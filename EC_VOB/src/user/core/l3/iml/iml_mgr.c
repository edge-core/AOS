/*-----------------------------------------------------------------------------
 *   File Name: IML.c  --> Interface Mapping Layer
 *   This file handles mapping between physical interfaces and vlan, subnet
 *   Histroy:
 *   Create:  Nike Chen
 *       2003.04.09, William, add Patch for BCM5615 Copy-To-Cpu issue.
 *       2004.02.06, Penny: for incoming packet parsing, we use IML_MGR_GetPacketPriority()
 *          to determine the packet type then based on the packet type to give the priority to put
 *          into our priority queue. The original msg queue feature is obsolete.
 *          SYSFUN_ReceiveMsgQ and SYSFUN_SendMsgQ is replacing by L_MSG_GetFromPriorityQueue and
 *          L_MSG_PutToPriorityQueue.
 *       2004.04.23, Penny, add IML_MGR_IsArpValid() to check the arp validation.
 *                      If valid, then can be writen to AMTRL3.
 *       2004.08.23, Penny, add one more parameter -- method for L_MSG_CreatePriorityQueue()
 *   Notes:
 *      1. IML_Init(), IML_CreateTask () is called by root ().
 *      2. IML_EnterMasterMode, IML_EnterSlaveMode,
 *         IML_EnterTransitionMode is called by STK_CTRL.
 *      3. Change to event based design instead of msg Q only. -- Penny, 2002-9-13
 *      4. Drop loopback ingress packet due to FFP will trap blocking packet. -- William, 2003-9-4
 *-----------------------------------------------------------------------------*/
/* INCLUDE FILE DECLARATIONS
 */
/*
 *  <<< Define some DIRECTORYs for special purpose >>>
 *
 *      IP_MGR_DEVELOP : used when developing stage, prevent mis-order of calling.
 *                 for debugging purpose, dump Phase2 inner arp, circuit,
 *                 static-route table.
 *      BACKDOOR_OPEN : used for connecting backdoor functions with Console CLI.
 *                      defined   -- to active backdoor function.
 *                      undefined -- to inactive backdoor function.
 *      _SNMP_SOCKET_SHOW : display socket info.
 */

#define BACKDOOR_OPEN
#define _SNMP_SOCKET_SHOW   1


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "l_mm.h"
#include "l_mpool.h"
#include "l_msg.h" /* Priority Queue Issue */
#include "l_stdlib.h"
#include "l_ipcmem.h"
#include "l_math.h"
#include "l_cvrt.h"
#include "netcfg_type.h"
#include "netcfg_om_ip.h"
#include "netcfg_pom_ip.h"
#include "lan.h"
#include "vlan_pmgr.h"
#include "vlan_om.h"
#include "vlan_pom.h"
#include "vlan_lib.h"
#include "swctrl.h"
#include "mac_cache.h"
#include "amtr_pmgr.h"
#include "iml_mgr.h"
#include "iml_type.h"
#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pmgr.h"
#include "amtrl3_type.h"
#endif
#include "leaf_es3626a.h"
#include "sys_module.h"
#include "l2mux_mgr.h"
#include "stktplg_pom.h"
#include "sys_callback_mgr.h"
#include "l2_l4_proc_comm.h"
#include "leaf_4001.h"
#include "ip_lib.h"
#include "ipal_neigh.h"

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
#include "http_mgr.h"
#include "telnet_mgr.h"
#include "sshd_mgr.h"
#include "Snmp_mgr.h" /* for Snmp Trap */
#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

#ifdef  BACKDOOR_OPEN
#include "backdoor_mgr.h"
#endif

#if (SYS_CPNT_STACKING == TRUE)
#include "iuc.h"
#include "isc.h"
#endif /* SYS_CPNT_STACKING */

#if (SYS_CPNT_DAI == TRUE)
#include "dai_om.h"
#include "dai_mgr.h"
#endif

#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
#include "dhcpv6_om.h"
#include "dhcpv6_pom.h"
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "dhcp_pom.h"
#include "dhcp_om.h"
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
#include "amtrl3_om.h"
#include "leaf_2096.h"
#endif

#if (SYS_CPNT_VRRP == TRUE)
#include "netcfg_om_nd.h"
#include "vrrp_pom.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
#include "netcfg_pom_nd.h"
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
#include "dhcpv6snp_type.h"
#include "dhcpv6snp_pom.h"
#endif

#if (SYS_CPNT_L2MCAST == TRUE)
#include "igv3snp_om.h"
#endif

#if (SYS_CPNT_DEBUG == TRUE)
#include "debug_type.h"
#include "debug_mgr.h"
#include "sys_time.h"
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_om.h"
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef  BACKDOOR_OPEN
    #define IML_MGR_BD_MSG(...)                             \
    {                                                       \
        if (debug_enable)                                   \
        {                                                   \
            printf("(%d):%s, ", __LINE__, __FUNCTION__);    \
            printf(__VA_ARGS__);                            \
            printf("\r\n");                                 \
        }                                                   \
    }
#else

    #define IML_MGR_BD_MSG(...)

#endif

#if (SYS_CPNT_DEBUG == TRUE)
    #define ARP_DEBUG_MSG(format, ...)  \
              DEBUG_MGR_Printf(DEBUG_TYPE_ARP, \
                DEBUG_TYPE_MATCH_ANY, \
                DEBUG_TYPE_ARP_PACKETS,  \
                0, \
                format,\
                ##__VA_ARGS__)
#else
    #define ARP_DEBUG_MSG(format, ...)
#endif

#define GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR(l_iml_msg_ptr) &((l_iml_msg_ptr)->iml_msg)

/* LOCAL DATATYPE DECLARATION
 */
#define IPV4_FORMAT        0x0800
#define IPV6_FORMAT        0x86DD
#define ARP_FORMAT         0x0806

/* Priority Queue Issue */
/* since dev_nicdrv will do storm control, it needn't be too large
 */
#define IML_ADPT_PRIORITY_QUEUE_LENGTH     128

/* Task Event */
#define IML_TASK_EVENT_RECEIVE_PACKET       BIT_1
#define IML_TASK_EVENT_ENTER_TRANSITION     BIT_2

/* ----------------------------------------------- */
/* IP protocol constant definitions                */
/* ----------------------------------------------- */
#define IML_IPPROTO_IP          0           /* dummy for IP                 */
#define IML_IPPROTO_ICMP        1           /* control message protocol     */
#define IML_IPPROTO_IGMP        2           /* group mgmt protocol          */
#define IML_IPPROTO_TCP         6           /* tcp                          */
#define IML_IPPROTO_UDP         17          /* user datagram protocol       */
#define IML_IPPROTO_IPV6        41          /* IPv6-in-IPv4 tunnelling      */
#define IML_IPPROTO_ICMPV6      58          /* ICMPv6                       */
#define IML_IPPROTO_NONE        59          /* IPv6 no next header          */
#define IML_IPPROTO_OSPF        89          /* OSPF                         */
#define IML_IPPROTO_PIM         103         /* PIM Dense Mode               */
#define IML_IPPROTO_VRRP        112         /* VRRP                         */

#define IML_ICMP_ECHOREPLY      0           /* Echo Reply               */
#define IML_ICMP_DEST_UNREACH   3           /* Destination Unreachable  */
#define IML_ICMP_SOURCE_QUENCH  4           /* Source Quench            */
#define IML_ICMP_REDIRECT       5           /* Redirect (change route)  */
#define IML_ICMP_ECHO           8           /* Echo Request             */
#define IML_ICMP_TIME_EXCEEDED  11          /* Time Exceeded            */
#define IML_ICMP_PARAMETERPROB  12          /* Parameter Problem        */
#define IML_ICMP_TIMESTAMP      13          /* Timestamp Request        */
#define IML_ICMP_TIMESTAMPREPLY 14          /* Timestamp Reply          */
#define IML_ICMP_INFO_REQUEST   15          /* Information Request      */
#define IML_ICMP_INFO_REPLY     16          /* Information Reply        */
#define IML_ICMP_ADDRESS        17          /* Address Mask Request     */
#define IML_ICMP_ADDRESSREPLY   18          /* Address Mask Reply       */

#define IML_IPV6_EXT_HDR_HOP_BY_HOP     0   /* Hop-by-hop options header  */
#define IML_IPV6_EXT_HDR_DESTINATION    60  /* Destination options header */
#define IML_IPV6_EXT_HDR_ROUTING        43  /* Routing header             */
#define IML_IPV6_EXT_HDR_FRAGMENT       44  /* Fragment header            */
#define IML_IPV6_EXT_HDR_AUTHENTICATION 51  /* Authentication header      */
#define IML_IPV6_EXT_HDR_SECURITY       50  /* Encapsulating security payload */

#define IML_ICMPV6_MGM_QUERY            130 /* Multicast Listener Query     */
#define IML_ICMPV6_MGM_REPORT           131 /* Multicast Listener Report    */
#define IML_ICMPV6_MGM_REDUCTION        132 /* Multicast Listener Done      */
#define IML_ICMPV6_NDISC_ROUTER_SOL     133 /* ICMP Router Solicitation     */
#define IML_ICMPV6_NDISC_ROUTER_ADVT    134 /* ICMP Router Advertisement    */
#define IML_ICMPV6_NDISC_NEIGH_SOL      135 /* ICMP Neighbor Solicitation   */
#define IML_ICMPV6_NDISC_NEIGH_ADVT     136 /* ICMP Neighbor Advertisement  */
#define IML_ICMPV6_NDISC_REDIRECT       137 /* ICMP Redirect                */
#define IML_ICMPV6_MLD2_REPORT          143 /* ???? */

#define IML_BOOTP_PORT_S        67          /* Bootp server udp port number */
#define IML_BOOTP_PORT_C        68          /* Bootp client udp port number */
#define IML_DHCP6_PORT_S        547         /* dhcpv6 server udp port number */
#define IML_DHCP6_PORT_C        546         /* dhcpv6 client udp port number */

#define IML_RIP_PORT            520         /* UDP port for RIP             */
#define IML_HSRP_PORT           1985        /* HSRP port number             */
#define IML_TFTP_PORT           SYS_DFLT_TFTP_PORT   /* TFTP port number    */

#define IML_ARP_OPCODE_REQUEST  1           /* ARP request op code */
#define IML_ARP_OPCODE_REPLY    2           /* ARP reply op code */

#define IML_MSG_SIZE        sizeof(IML_L_MSG_T)

enum
{
    IML_CLASSIFIED_ARP_REPLY = 0,
    IML_CLASSIFIED_ARP_REQUEST,
    IML_CLASSIFIED_IP_MGMT,
    IML_CLASSIFIED_IGMP,
    IML_CLASSIFIED_PIM,
    IML_CLASSIFIED_RIP,
    IML_CLASSIFIED_OSPF,
    IML_CLASSIFIED_VRRP,
    IML_CLASSIFIED_HSRP,
    IML_CLASSIFIED_TFTP,
    IML_CLASSIFIED_OTHER,
    IML_CLASSIFIED_BOOTP_CLIENT,
    IML_CLASSIFIED_BOOTP_SERVER,
    IML_CLASSIFIED_UDPHELPER,
    IML_CLASSIFIED_ICMP_MLD,
    IML_CLASSIFIED_ICMP_ND_NEIGH_SOL,
    IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT,
    IML_CLASSIFIED_ICMP_ND_ROUTER_SOL,
    IML_CLASSIFIED_ICMP_ND_ROUTER_ADVT,
    IML_CLASSIFIED_ICMP_ND_REDIRECT,
    IML_CLASSIFIED_ENCAP_V6,
    IML_CLASSIFIED_DHCP_V6,
    IML_CLASSIFIED_MCAST_V6_DATA,
    IML_CLASSIFIED_ICMP_ECHO_LOCAL,
    IML_MAX_NO_OF_PKT_TYPE, /* Maximum number of packet type */
}IML_Classified_Pkt_Type_T;

/* Penny,2004-2-6: added for Priority Queue Issue.
 */
#define IML_ADPT_MAX_NUM_OF_QUEUE      8
#define IML_ADPT_MAX_NUM_OF_PRIORITY   8
/* end of Penny added for priority Q issue
 */

/* added by jinwang for revising WFQ 2004-11-02 */
#define     IML_USING_WFQ_MODE                  TRUE
#define     IML_QUEUE0_LENGTH                   5   /*HSRP*/
#define     IML_QUEUE1_LENGTH                   20  /*OTHER*/
#define     IML_QUEUE2_LENGTH                   20  /*UDPHELPER*/
#define     IML_QUEUE3_LENGTH                   60  /*BOOTP,ARP REQUEST*/
#define     IML_QUEUE4_LENGTH                   20  /*VRRP*/
#define     IML_QUEUE5_LENGTH                   60  /*PIM,IGMP SNP*/
#define     IML_QUEUE6_LENGTH                   80  /*OSPF,RIP,IP MGMT*/
#define     IML_QUEUE7_LENGTH                   50  /*TFTP,ARP REPLY*/

#define     IML_IP_HANDLE_TYPE_IGMPSNP          1
#define     IML_IP_HANDLE_TYPE_TCPIPSTK         2
#define     IML_IP_HANDLE_TYPE_BOTH             3

#define     IML_IPV6_OPTLEN(p)  (((p)->hdr_len+1) << 3)

/* define flags and offset field in iphdr->frag_off
 */
#define     IML_IP_FLAG_RESERVED 0x8000	    /* Flag: Reserved           */
#define     IML_IP_FLAG_DF       0x4000	    /* Flag: "Don't Fragment"   */
#define     IML_IP_FLAG_MF       0x2000	    /* Flag: "More Fragments"   */
#define     IML_IP_FRAG_OFFSET   0x1FFF	    /* "Fragment Offset" part   */


typedef struct IML_MGR_COUNTER_S
{
    UI32_T   rx_from_lower_layer_cnt;        /* rcv from NIC */
    UI32_T   rx_mcast_cnt;
    UI32_T   rx_arp_cnt;
    UI32_T   rx_ip_cnt;
    UI32_T   rx_cnt;                         /* to packet handler */
    UI32_T   send_to_bootp_cnt;
    UI32_T   send_to_dhcpsnp_cnt;
    UI32_T   send_to_dhcpv6snp_cnt;
    UI32_T   send_to_ndsnp_cnt;
    UI32_T   send_to_kernel_ip_cnt;
    UI32_T   send_to_kernel_arp_cnt;
    UI32_T   send_to_amtrl3_arp_cnt;
    UI32_T   send_to_kernel_nd_cnt;
    UI32_T   send_to_amtrl3_nd_cnt;
    UI32_T   drop_rx_cnt;
    UI32_T   drop_tx_cnt;
    UI32_T   drop_tx_null_mac_cnt;
    UI32_T   drop_no_msg_buf_cnt;            /* no msg buf for incoming packet */
    UI32_T   drop_msg_q_full_cnt;            /* counter for dropping pkt while msg q full */
    UI32_T   inner_error_cnt;                /* processing error in IML */
    UI32_T   tx_cnt;                         /* to NIC */
    UI32_T   tx_ip_cnt;
    UI32_T   tx_arp_cnt;
    UI32_T   tx_bcast_cnt;
    UI32_T   drop_rx_unicast_da_invalid_cnt;   /* drop unicast packet whose dst_mac is invalid */
} IML_MGR_COUNTER_T;

typedef struct IML_Ipv4PktFormat_S
{
    UI8_T   ver_len;
    UI8_T   tos;
    UI16_T  length;
    UI16_T  identification;
    UI16_T  frag_off;
    UI8_T   ttl;
    UI8_T   protocol;
    UI16_T  checksum;
    UI32_T  srcIp;
    UI32_T  dstIp;
    UI8_T   payload[0];
} __attribute__((packed, aligned(1)))IML_Ipv4PktFormat_T;

typedef struct IML_IcmpHdr_S
{
    UI8_T   type;
    UI8_T   code;
    UI16_T  checksum;
    union
    {
        struct
        {
            UI16_T id;
            UI16_T sequence;
        } echo;
        UI32_T  gateway;
        struct
        {
            UI16_T __unused;
            UI16_T mtu;
        } frag;
    } un;
} __attribute__((packed, aligned(1)))IML_IcmpHdr_T;

typedef struct IML_Ipv6PktFormat_S
{
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU==TRUE)
    UI32_T      flow_label:20,
                traffic_class:8,
                version:4;
#else
    UI32_T      version:4,
                traffic_class:8,
                flow_label:20;
#endif
    UI16_T      payload_len;
    UI8_T       next_hdr;
    UI8_T       hop_limit;

    UI8_T       src_addr[16];
    UI8_T       dst_addr[16];
    UI8_T       pay_load[0];
} __attribute__((packed, aligned(1)))IML_Ipv6PktFormat_T;


typedef struct IML_Icmp6Hdr_S {
    UI8_T       icmp6_type;
    UI8_T       icmp6_code;
    UI16_T      icmp6_cksum;
} __attribute__((packed, aligned(1)))IML_Icmp6Hdr_T;

typedef struct IML_Icmp6NdNeighSolAdvtHdr_S {
    IML_Icmp6Hdr_T icmp6_hdr;
    struct icmpv6_nd_advt
    {
#if (SYS_HWCFG_LITTLE_ENDIAN_CPU==TRUE)
    UI32_T      reserved:5,
                override:1,
                solicited:1,
                router:1,
                reserved2:24;
#else
    UI32_T      router:1,
                solicited:1,
                override:1,
                reserved:29;
#endif
    } u_nd_advt;
    UI8_T       target[16];
    UI8_T       option[0];
} __attribute__((packed, aligned(1)))IML_Icmp6NdNeighSolAdvtHdr_T;

typedef struct IML_Icmp6OptHdr_S   /* ICMPv6 option header */
{
    UI8_T  opt_type;
    UI8_T  opt_len;        /* in units of 8 octets */
    UI8_T  opt_data[0];
    /* followed by option specific data */
}__attribute__((packed, aligned(1)))IML_Icmp6OptHdr_T;

typedef struct IML_ArpPktFormat_S
{
    UI16_T  hardwareType;           /* Hardware type                 */
    UI16_T  protocolType;           /* Protocol type                 */
    UI8_T   hardwareLen;            /* Hardware address length       */
    UI8_T   protocolLen;            /* Protocol address length       */
    UI16_T  opcode;                 /* Request or Reply              */
    UI8_T   srcMacAddr[6];          /* Source hardware address       */
    UI32_T  srcIp;                  /* Source IP address             */
    UI8_T   dstMacAddr[6];          /* Destination hardware address  */
    UI32_T  dstIp;                  /* Destination IP address        */
} __attribute__((packed, aligned(1)))IML_ArpPktFormat_T;

typedef struct IML_Udphdr_S
{
        UI16_T  uh_sport;           /* source port       */
        UI16_T  uh_dport;           /* destination port  */
        UI16_T  uh_ulen;            /* udp length        */
        UI16_T  uh_sum;             /* udp checksum      */
} __attribute__((packed, aligned(1)))IML_Udphdr_T;

typedef struct IML_Tcphdr_S
{
        UI16_T  th_sport;           /* source port       */
        UI16_T  th_dport;           /* destination port  */
        UI32_T  th_seq;             /* tcp seq number     */
        UI32_T  th_acknum;          /* tcp ack number   */
        UI32_T  dont_care1;
        UI16_T  th_csum;
        UI16_T  dont_care2;
} __attribute__((packed, aligned(1)))IML_Tcphdr_T;

typedef struct IML_PACKET_RX_S
{
        L_MM_Mref_Handle_T *mref_handle_p;
        UI8_T    *dst_mac;
        UI8_T    *src_mac;
        UI32_T   ingress_vid;
        UI32_T   etherType;
        UI32_T   packet_length;
        UI32_T   src_port;
        UI32_T   classified_packet_type;
        UI32_T   extension_header_len;
        NETCFG_TYPE_InetRifConfig_T src_rif;
} IML_PACKET_RX_T;

typedef struct  IML_MSG_S
{
    UI32_T  mtype;       /* encoding message type, source   */
    void*   mtext;       /* point message block             */
    UI32_T  muser;       /* user defined                    */
    UI32_T  mreserved;   /* reserve for future              */
} IML_MSG_T;

/* This data type is used to pass as the message buffer to L_MSG.
 * Note that sizeof(L_MSG_Message_T)>=sizeof(IML_MSG_T) must be hold to ensure
 * that the data in IML_MSG_T can be kept by L_MSG without problem.
 */
SYSFUN_STATIC_ASSERT(sizeof(L_MSG_Message_T)>=sizeof(IML_MSG_T), "The size of L_MSG_Message_T must be larger or equal to size of IML_MSG_T");
typedef struct  IML_L_MSG_S
{
    union
    {
        UI8_T data[sizeof(L_MSG_Message_T)]; /* to ensure IML_L_MSG_T is large enough to pass to L_MSG */
        IML_MSG_T iml_msg;
    };
} IML_L_MSG_T;

typedef struct  IML_WEBAUTH_S
{
    UI32_T  time_stamp;  // 0.01 sec
    UI32_T  in_lport;
    UI32_T  src_ip;
    UI32_T  dst_ip;
    UI16_T  src_tcpport;
    UI8_T   dst_mac[SYS_ADPT_MAC_ADDR_LEN];
} IML_WEBAUTH_T;


typedef BOOL_T (*IML_MGR_TxSnoopPacketFunction_T)(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI32_T              pkt_length,
    UI32_T              egr_vidifindex,
    UI32_T              egr_lport,
    UI32_T              ing_lport,
    UI16_T              pkt_type);


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void IML_MGR_DataInit();
static void IML_MGR_PacketDispatch(L_MM_Mref_Handle_T *mref_handle_p,
                                   UI8_T  *dst_mac, UI8_T  *src_mac,
                                   UI32_T ingress_vid, UI32_T etherType,
                                   UI32_T packet_length, UI32_T src_port,
                                   UI32_T classified_packet_type,
                                   UI32_T extension_header_len,
                                   NETCFG_TYPE_InetRifConfig_T *src_rif_p);
static BOOL_T IML_MGR_GetSrcIpFromHdr(UI16_T ethertype, UI8_T *payload, L_INET_AddrIp_T *ip_adr_p);
static BOOL_T IML_MGR_GetDstIpFromHdr(UI16_T ethertype, UI8_T *payload, L_INET_AddrIp_T *ip_adr_p);
//static BOOL_T IML_MGR_GetSrcRif (UI32_T *vid_ifindex_p, UI8_T *payload, UI16_T etherType, UI32_T classified_packet_type, NETCFG_TYPE_InetRifConfig_T *ingress_rif, BOOL_T is_translated);
#if (SYS_CPNT_IPV6 == TRUE || SYS_CPNT_MLDSNP == TRUE)
static BOOL_T IML_MGR_GetExtHdrLen(UI8_T *payload_p, UI8_T ext_hdr_type, UI32_T *ext_hdr_len_p);
static BOOL_T IML_MGR_GetTotalExtHdrLen(UI8_T *payload_p, UI32_T payload_len, UI32_T *total_ext_hdr_len_p, UI32_T *next_hdr_type_p);
#endif
static BOOL_T IML_MGR_PacketClassification(UI8_T *payload, UI32_T payload_len, UI16_T etherType, UI32_T ingress_vid, UI32_T lport, UI8_T  dst_mac[6], UI32_T *classified_packet_type, UI32_T *ext_hdr_len_p);
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
static BOOL_T IML_MGR_FilterMgmtVlanPacket(UI32_T packet_type, void *payload, UI32_T ingress_vid);
#endif

static void   IML_MGR_InitPriorityQueue(void);
static BOOL_T IML_MGR_IsDhcpPacket(L_MM_Mref_Handle_T *mref_handle_p, UI16_T ether_type);
static BOOL_T IML_MGR_IsDhcp6Packet(L_MM_Mref_Handle_T *mref_handle_p, UI16_T ether_type);
static BOOL_T IML_MGR_IsArpValid(IML_ArpPktFormat_T *arp_pkt_p);
static BOOL_T IML_MGR_IsNdAdvtSolValid(IML_Icmp6NdNeighSolAdvtHdr_T *nd_pkt_p);
static void   IML_MGR_PrepareTask (BOOL_T is_init);   /*  Prepare IML needs space, queue  */
static void   IML_MGR_Reset (void);                   /*  Recreate IML used resource      */
static void   IML_MGR_RxTaskMain(void* arg);          /*  IML RX task body                */
static void   IML_MGR_TxTaskMain(void* arg);
static void   IML_MGR_ProcessTxPacket(L_MM_Mref_Handle_T *mref_handle_p,IML_MGR_RecvPktFromTCPIPStackArgs_T packet_args);
static BOOL_T IML_MGR_IsNullMac(UI8_T *mac);
static void   IML_MGR_PrintPacketType(UI32_T packet_type);
static UI32_T IML_MGR_SendPkt(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid_ifindex, UI32_T packet_length,
                              UI8_T *dst_mac, UI8_T *src_mac, UI16_T packet_type, BOOL_T forward);
static UI32_T IML_MGR_SendPktToTCPIPStack(UI32_T mref_handle_offset, UI16_T tag_info, UI32_T process_packet_type);
static UI32_T IML_MGR_RecvPktFromTCPIPStack(UI32_T *mref_handle_offset_p, IML_MGR_RecvPktFromTCPIPStackArgs_T *args_p);
static BOOL_T IML_MGR_GetPortOperStatus(UI32_T unit_no, UI32_T port_no, UI32_T *lport);
static void IML_MGR_ProcessRxPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    ether_type,
                                       UI32_T    packet_length,
                                       UI32_T    l_port);

static BOOL_T
IML_MGR_TxSnoopPacket_Callback_Func(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI32_T              pkt_length,
    UI32_T              egr_vidifindex,
    UI32_T              egr_lport,
    UI32_T              ing_lport,
    UI16_T              pkt_type
);

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
static UI32_T IML_MGR_BroadcastBootPPkt_ExceptInport(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid_ifindex,
    UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,UI32_T src_lport_ifindex);
static UI32_T IML_MGR_SendPktToPort(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,
    UI32_T vid, UI32_T out_lport, UI16_T packet_type);
#endif

#if (SYS_CPNT_IP_TUNNEL == TRUE)
static BOOL_T IML_MGR_SetDynamicTunnelRoute(UI8_T   *payload, UI32_T vid_ifindex, UI16_T packet_type);
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
static BOOL_T IML_MGR_TxSnoopDhcp_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI32_T              pkt_length,
    UI32_T              egr_vidifindex,
    UI32_T              egr_lport,
    UI32_T              ing_lport,
    UI16_T              ether_type
);
#endif

#if (SYS_CPNT_VRRP == TRUE)
static BOOL_T IML_MGR_IsVirtualMac(UI8_T *mac);
static BOOL_T IML_MGR_IsMasterVirtualIp(UI32_T ifindex, UI32_T ipaddr);
static BOOL_T IML_MGR_IsVrrpMaster(UI32_T ifindex, UI32_T vrid);
static BOOL_T IML_MGR_ProcessRxVirtualMacAddressPacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T    *payload,
    UI32_T    ingress_vid,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     vlan_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T    ether_type
);

static void IML_MGR_ProcessTxVirtualMacAddressPacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T *payload,
    IML_MGR_RecvPktFromTCPIPStackArgs_T *packet_args
);
#endif

#if (SYS_CPNT_VIRTUAL_IP == TRUE)
static BOOL_T IML_MGR_IsAcctonVirtualMac(UI8_T *mac);
#endif

#ifdef  BACKDOOR_OPEN
static void IML_MGR_BackDoorMain(void);
static void IML_MGR_ShowCounter(void);
static void IML_MGR_ShowPacket(UI8_T *da, UI8_T *sa, UI32_T pkt_type, UI32_T pkt_len, void *payload);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* Assign priority to the classified packet
 * wuli,adjust priority 2004-11-04
 */
static const UI8_T  priority_table[IML_MAX_NO_OF_PKT_TYPE] = {7, /*IML_CLASSIFIED_ARP_REPLY   */
                                                              3, /*IML_CLASSIFIED_ARP_REQUEST */
                                                              6, /*IML_CLASSIFIED_IP_MGMT     */
                                                              5, /*IML_CLASSIFIED_IGMP  */
                                                              5, /*IML_CLASSIFIED_PIM         */
                                                              6, /*IML_CLASSIFIED_RIP         */
                                                              6, /*IML_CLASSIFIED_OSPF        */
                                                              4, /*IML_CLASSIFIED_VRRP        */
                                                              0, /*IML_CLASSIFIED_HSRP        */
                                                              7, /*IML_CLASSIFIED_TFTP        */
                                                              1, /*IML_CLASSIFIED_OTHER       */
                                                              3, /*IML_CLASSIFIED_BOOTP_CLIENT*/
                                                              3, /*IML_CLASSIFIED_BOOTP_SERVER*/
                                                              2};/*IML_CLASSIFIED_UDPHELPER   */

static IML_MGR_COUNTER_T                iml_counter;

/*modified by jinwang for using WFQ,2004-11-2*/
#if IML_USING_WFQ_MODE      /* WFQ mode */
static L_PRIORITY_QUEUE_MAP_T  priority_queue_map[IML_ADPT_MAX_NUM_OF_PRIORITY]=        /* here array-length initialize according to LAN_MAX_NUMBER_OF_PRIORITY */
{
        {IML_QUEUE0_LENGTH,0,0},        {IML_QUEUE1_LENGTH,0,1},
        {IML_QUEUE2_LENGTH,0,2},        {IML_QUEUE3_LENGTH,0,3},
        {IML_QUEUE4_LENGTH,0,4},        {IML_QUEUE5_LENGTH,0,5},
        {IML_QUEUE6_LENGTH,0,6},        {IML_QUEUE7_LENGTH,0,7}
};
#else                       /*  SPQ mode */
static L_PRIORITY_QUEUE_MAP_T  priority_queue_map[IML_ADPT_MAX_NUM_OF_PRIORITY]=
{
        {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,0},   {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,1},
        {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,2},   {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,3},
        {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,4},   {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,5},
        {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,6},   {IML_ADPT_PRIORITY_QUEUE_LENGTH,0,7}
};
#endif

static BOOL_T                           debug_enable=FALSE;
static BOOL_T                           show_incoming_packet_type = FALSE;
static SYSFUN_MsgQ_T                    iml_msg_q_id;
static BOOL_T                           iml_using_priority;
static UI32_T                           iml_start_time = 0;
static UI32_T                           iml_end_time = 0;
static UI32_T                           iml_delta_time = 0;
static BOOL_T                           IML_MGR_BackDoor_testingTick = FALSE;

/* STATIC VARIABLE DECLARATIONS
 */
SYSFUN_DECLARE_CSC

static  BOOL_T                  is_transition_done;
static  UI32_T                  iml_rx_task_id;
static  UI32_T                  iml_tx_task_id;
static  L_MPOOL_HANDLE          iml_msg_blk_pool;       /*  message block pool descriptor   */
static  L_MSG_PriorityQueue_T   priority_queue;         /* Priority Queue Issue */
/*modified by jinwang for using WFQ,2004-11-04*/
#if !IML_USING_WFQ_MODE
static  UI16_T                  schedule[] = {0};                               /* schedule is not use in strict priority */
#else
static  UI16_T                  schedule[] = {1,2,2,3,3,5,6,6};             /* schedule is used in WFQ */
#endif

enum    IML_MESSAGE_ACTION_MODE_E
{
        LAN_PACKET_RX_CALLBACK=1
};

#if (SYS_CPNT_WEBAUTH == TRUE)
/* may need to improve the database operation for WEBAUTH
 * in current design, always replace the oldest one...
 */
#define     IML_MGR_WEBAUTH_ENTRY_MAX         200   //total entries
#define     IML_MGR_WEBAUTH_AGEOUT_TIME     12000   //in ticks (0.01 sec)

static UI16_T        iml_watbl_head = 0,
                     iml_watbl_cnt  = 0;
static IML_WEBAUTH_T iml_webauth_tbl[IML_MGR_WEBAUTH_ENTRY_MAX];
static BOOL_T        iml_webauth_sts[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */


static IML_MGR_TxSnoopPacketFunction_T iml_mgr_tx_snoop_packet_func_list[] = {
#if (SYS_CPNT_DHCPSNP == TRUE)
    IML_MGR_TxSnoopDhcp_CallbackFunc,
#endif
    NULL
};


/* EXPORTED FUNCTIONS
 */

void IML_MGR_InitiateProcessResources(void)
{
    IML_MGR_DataInit();
    IML_MGR_PrepareTask (TRUE); /* Initialize IML Task */
    return;
}

void IML_MGR_InitiateSystemResources(void)
{
    return;
}

void IML_MGR_AttachSystemResources(void)
{
    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void IML_MGR_Create_InterCSC_Relation(void)
{
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("IML",
                                                      SYS_BLD_L2MUX_GROUP_IPCMSGQ_KEY,
                                                      IML_MGR_BackDoorMain);
#endif
}


BOOL_T IML_MGR_CreateTask(void)
{
    /* create a thread for IML task only when it needs to take care of timer
     * event.
     */
    if(SYSFUN_SpawnThread(SYS_BLD_IML_RX_THREAD_PRIORITY,
                          SYS_BLD_IML_RX_THREAD_SCHED_POLICY,
                          SYS_BLD_IML_RX_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          IML_MGR_RxTaskMain,
                          NULL,
                          &iml_rx_task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread IML_MGR_RxTaskMain fail.\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread ( SW_WATCHDOG_IML_RX, iml_rx_task_id, SYS_ADPT_IML_RX_SW_WATCHDOG_TIMER );
#endif

    /* create a thread for IML task only when it needs to take care of timer
     * event.
     */
    if(SYSFUN_SpawnThread(SYS_BLD_IML_TX_THREAD_PRIORITY,
                          SYS_BLD_IML_TX_THREAD_SCHED_POLICY,
                          SYS_BLD_IML_TX_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          IML_MGR_TxTaskMain,
                          NULL,
                          &iml_tx_task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread IML_MGR_TxTaskMain fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}


static void IML_MGR_InitPriorityQueue(void)
{
    BOOL_T ret;

    /* int queue_id   = 0;*/
    /* int i, group, num_of_item_per_group = 0;*/
    L_MSG_PriorityQueueMethod_T priority_queue_method;


    /* Modified by jinwang for using WFQ,2004-11-02
     */
#if IML_USING_WFQ_MODE
    priority_queue_method = L_MSG_WFQ;
#else
    priority_queue_method = L_MSG_STRICT;
#endif

    /* Penny,2004.08.23:
     * Spec changed for L_MSG_CreatePriorityQueue() -- add one more parameter method
     * For strict priority queue, method is L_MSG_STRICT
     */
    ret = L_MSG_CreatePriorityQueue(&priority_queue,
                                    IML_ADPT_MAX_NUM_OF_QUEUE, /* number_of Q */
                                    IML_ADPT_MAX_NUM_OF_PRIORITY,
                                    /* priority_queue_mapping */priority_queue_map,
                                    0,
                                    schedule, /* L_MSG_STRICT */ priority_queue_method);

    if (ret != TRUE)
        return;

}/* end of IML_MGR_InitPriorityQueue */

static void IML_MGR_PrepareTask (BOOL_T is_init)
{
    /* Allocate new message block pool
     */
    if (NULL==(iml_msg_blk_pool=L_MPOOL_CreateBlockPool (sizeof(IML_PACKET_RX_T), IML_ADPT_PRIORITY_QUEUE_LENGTH)))
    {
        /* DBG_printf ("ERR--Can't create message block pool.\n");*/
        return;
    }

    IML_MGR_InitPriorityQueue();
}

static void IML_MGR_Reset (void)
{
    L_MPOOL_RecreateBlockPool (iml_msg_blk_pool);
}

/*
 * FUNCTION : IML_DataInit(void)
 * Description -- To init IML init data and register callback function in LAN.C.
 * caller:
 */
static void IML_MGR_DataInit(void)
{
    memset (&iml_counter, 0, sizeof(IML_MGR_COUNTER_T));
    iml_using_priority = SYS_CPNT_IML_PRIORITY_Q_ENABLED;

#if (SYS_CPNT_WEBAUTH == TRUE)
    memset (&iml_webauth_tbl, 0, sizeof(iml_webauth_tbl));
    memset (&iml_webauth_sts, FALSE, sizeof(iml_webauth_sts));
    iml_watbl_head = iml_watbl_cnt = 0;
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */
    MAC_CACHE_Init();
}


/* FUNCTION NAME: IML_EnterMasterMode
 * PURPOSE: Enable the IML activities as master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IML_MGR_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE();
    return;
}


/* FUNCTION NAME: IML_EnterSlaveMode
 * PURPOSE: Enable the IML activities as slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IML_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
    return;
}


/* FUNCTION NAME: IML_SetTransitionMode
 * PURPOSE: Set the IML activities as transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IML_MGR_SetTransitionMode(void)
{
    /* MGR's set transition mode */
    SYSFUN_SET_TRANSITION_MODE();

    /* TASK's set transition mode */
    is_transition_done = FALSE;
    SYSFUN_SendEvent(iml_rx_task_id, IML_TASK_EVENT_ENTER_TRANSITION);
    MAC_CACHE_Init();
}

/* FUNCTION NAME: IML_EnterTransitionMode
 * PURPOSE: Enable the IML activities as transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IML_MGR_EnterTransitionMode (void)
{
    /* enter transition mode for MGR */
    SYSFUN_ENTER_TRANSITION_MODE();

    /* enter transition mode for TASK*/
    /*  want task release all resources */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);

    // can't send/receive packet!
    IML_MGR_Reset();
    IML_MGR_DataInit();
}

#if (SYS_CPNT_WEBAUTH == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_AgeOutOldEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : To age our the old entry
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  - age out some entries
 *            FALSE - age out nothing
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
static BOOL_T IML_MGR_AgeOutOldEntry(void)
{
    UI32_T  tbl_idx, curr_tick, diff_tick;
    UI16_T  loop_cnt =0;
    BOOL_T  ret = FALSE;

    curr_tick = SYSFUN_GetSysTick();

    tbl_idx = iml_watbl_head;
    while (loop_cnt < iml_watbl_cnt)
    {
        if (iml_webauth_tbl[tbl_idx].time_stamp < curr_tick)
        {
            diff_tick = curr_tick - iml_webauth_tbl[tbl_idx].time_stamp;
        }
        else
        {
            diff_tick = 0xffffffff - iml_webauth_tbl[tbl_idx].time_stamp + curr_tick + 1;
        }

        if (diff_tick < IML_MGR_WEBAUTH_AGEOUT_TIME)
        {
            break;
        }

        loop_cnt ++;
        if (++tbl_idx >= IML_MGR_WEBAUTH_ENTRY_MAX)
        {
            tbl_idx = 0;
        }
    }

    if (tbl_idx != iml_watbl_head)
    {
        iml_watbl_head  = tbl_idx;
        iml_watbl_cnt  -= loop_cnt;
        ret             = TRUE;
    }

    IML_MGR_BD_MSG(" head/cnt-%d/%d",
        iml_watbl_head, iml_watbl_cnt);

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_GetWebauthEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : To get table index for specified src ip/tcp port
 * INPUT    : src_ip, src_tcpport
 * OUTPUT   : none
 * RETURN   : table index
 *            IML_MGR_WEBAUTH_ENTRY_MAX if failed
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
static UI32_T IML_MGR_GetWebauthEntry(
    UI32_T  src_ip,
    UI16_T  src_tcpport)
{
    UI32_T  tbl_idx, ret = IML_MGR_WEBAUTH_ENTRY_MAX;
    UI16_T  loop_cnt =0;

    tbl_idx = iml_watbl_head;
    while (loop_cnt < iml_watbl_cnt)
    {
        if (  (iml_webauth_tbl[tbl_idx].src_ip      == src_ip)
            &&(iml_webauth_tbl[tbl_idx].src_tcpport == src_tcpport)
           )
        {
            ret = tbl_idx;
            break;
        }

        loop_cnt ++;
        if (++tbl_idx >= IML_MGR_WEBAUTH_ENTRY_MAX)
        {
            tbl_idx = 0;
        }
    }

#if 0
    for (tbl_idx = 0; tbl_idx < IML_MGR_WEBAUTH_ENTRY_MAX; tbl_idx++)
    {
        if (  (iml_webauth_tbl[tbl_idx].src_ip   == src_ip)
            &&(iml_webauth_tbl[tbl_idx].src_port == src_port)
           )
        {
            ret = tbl_idx;
            break;
        }
    }
#endif
    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_SetWebauthEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : To set a entry for specified src ip/tcp port.
 * INPUT    : in_lport, src_ip, src_tcpport, dst_ip, dst_mac_p
 * OUTPUT   : none
 * RETURN   : table index
 *            IML_MGR_WEBAUTH_ENTRY_MAX if failed
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
static UI32_T IML_MGR_SetWebauthEntry(
    UI32_T  in_lport,
    UI32_T  src_ip,
    UI16_T  src_tcpport,
    UI32_T  dst_ip,
    UI8_T   *dst_mac_p)
{
    UI16_T  tbl_idx;

    /* try to age out old entries before finding
     * to avoid finding the entry recorded long long ago...
     */
    IML_MGR_AgeOutOldEntry();

    tbl_idx = IML_MGR_GetWebauthEntry(src_ip, src_tcpport);
    if (IML_MGR_WEBAUTH_ENTRY_MAX == tbl_idx)
    {
        /* replace the oldest one
         */
        if (IML_MGR_WEBAUTH_ENTRY_MAX == iml_watbl_cnt)
        {
            tbl_idx = iml_watbl_head;

            if (++iml_watbl_head >= IML_MGR_WEBAUTH_ENTRY_MAX)
            {
                iml_watbl_head = 0;
            }
        }
        else
        {
            tbl_idx = iml_watbl_head + iml_watbl_cnt;
            iml_watbl_cnt++;

            if (tbl_idx >= IML_MGR_WEBAUTH_ENTRY_MAX)
            {
                tbl_idx -= IML_MGR_WEBAUTH_ENTRY_MAX;
            }
        }

        IML_MGR_BD_MSG(" head/cnt/idx-%d/%d/%d",
            iml_watbl_head, iml_watbl_cnt, tbl_idx);

        if (tbl_idx < IML_MGR_WEBAUTH_ENTRY_MAX)
        {
            IML_MGR_BD_MSG(" in/sip/sport/dip-%ld/%08lx/%d/%08lx",
                (long)in_lport, (unsigned long)src_ip, src_tcpport, (unsigned long)dst_ip);

            iml_webauth_tbl[tbl_idx].in_lport    = in_lport;
            iml_webauth_tbl[tbl_idx].src_ip      = src_ip;
            iml_webauth_tbl[tbl_idx].src_tcpport = src_tcpport;
            iml_webauth_tbl[tbl_idx].dst_ip      = dst_ip;
            memcpy (iml_webauth_tbl[tbl_idx].dst_mac, dst_mac_p, SYS_ADPT_MAC_ADDR_LEN);
            iml_webauth_tbl[tbl_idx].time_stamp  = SYSFUN_GetSysTick();
        }
    }

    return tbl_idx;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_CheckSumAdjust
 *--------------------------------------------------------------------------
 * PURPOSE  : To adjust checksum for partial change. (for IP/TCP)
 * INPUT    : chksum_p - pointer to the chksum in the packet
 *            optr_p   - pointer to the old data in the packet
 *            olen     - length of the old data
 *            nptr_p   - pointer to the new data in the packet
 *            nlen     - length of the new data
 * OUTPUT   : chesum_p - pointer to the chksum in the packet
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
static void IML_MGR_CheckSumAdjust (
    UI8_T   *chksum_p,
    UI8_T   *optr_p,
    UI16_T   olen,
    UI8_T   *nptr_p,
    UI16_T   nlen)
{
    I32_T   x,   old,   new;

    x = (chksum_p[0] << 8) + chksum_p[1];
    x = ~x & 0xFFFF;

    while (olen)
    {
        old = (optr_p[0] << 8) + optr_p[1];
        optr_p += 2;
        x -= old & 0xffff;
        if (x<=0)
        {
            x--;
            x &= 0xffff;
        }
        olen-=2;
    }

    while (nlen)
    {
        new = (nptr_p[0] << 8) + nptr_p[1];
        nptr_p += 2;
        x += new & 0xffff;
        if (x & 0x10000)
        {
            x++;
            x&=0xffff;
        }
        nlen-=2;
    }

    x = ~x & 0xFFFF;
    chksum_p[0]=x >> 8;
    chksum_p[1]=x & 0xff;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_DoWebAuthReplaceForIng
 *--------------------------------------------------------------------------
 * PURPOSE  : To get original dip and ingress lport
 *              by specified src ip/tcp port.
 * INPUT    : mref_handle_p - pointer to handle of L_MM_Mref
 *            payload_p     - pointer to payload of pdu
 *            new_dip_p     - pointer to new dip
 *            new_mac_p     - pointer to new destination mac
 * OUTPUT   : payload_p     - pointer to payload of pdu
 *                            (da,dip,ip/tcp checksum will be modified)
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
static BOOL_T IML_MGR_DoWebAuthReplaceForIng(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *payload_p,
    UI8_T               *new_dip_p,
    UI8_T               *new_mac_p)
{
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T *ipv6_pkt_p   = (IML_Ipv6PktFormat_T*)payload_p;
#endif
    IML_Ipv4PktFormat_T *ipv4_pkt_p   = (IML_Ipv4PktFormat_T*)payload_p;
    IML_Tcphdr_T        *tcp_header_p = NULL;
    UI32_T              ipHeaderLen, avail_size_before_pdu;
    BOOL_T              ret = FALSE;

    if (NULL != payload_p)
    {
        ipHeaderLen = (0x0f & ipv4_pkt_p->ver_len) * 4;
        tcp_header_p = (IML_Tcphdr_T*)((UI8_T*)ipv4_pkt_p+ipHeaderLen);

        IML_MGR_CheckSumAdjust(
            (UI8_T *) &ipv4_pkt_p->checksum,
            (UI8_T *) &ipv4_pkt_p->dstIp,
            4,
            new_dip_p,
            4);

        IML_MGR_CheckSumAdjust(
            (UI8_T *) &tcp_header_p->th_csum,
            (UI8_T *) &ipv4_pkt_p->dstIp,
            4,
            new_dip_p,
            4);

        memcpy (&ipv4_pkt_p->dstIp, new_dip_p, 4);

        /* to replace the da in pdu
         *   bcz new version of kernel will always check the da and mark the
         *   packet to other host if it's not for us in eth_type_trans.
         *
         * assume the da is at the beginning of the pdu
         */
        avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
        memcpy(payload_p - avail_size_before_pdu, new_mac_p, SYS_ADPT_MAC_ADDR_LEN);

        IML_MGR_BD_MSG("dip-%d.%d.%d.%d",
            new_dip_p[0], new_dip_p[1], new_dip_p[2], new_dip_p[3]);

        ret = TRUE;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_DoWebAuthReplaceForEgr
 *--------------------------------------------------------------------------
 * PURPOSE  : To replace the src ip and src mac for webauth.
 * INPUT    : payload_p
 * OUTPUT   : payload_p, src_mac_p
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
static BOOL_T IML_MGR_DoWebAuthReplaceForEgr(
    UI8_T   *payload_p,
    UI8_T   *src_mac_p)
{
#if (SYS_CPNT_IPV6 == TRUE)
        IML_Ipv6PktFormat_T *ipv6_pkt_p   = (IML_Ipv6PktFormat_T*)payload_p;
#endif
        IML_Ipv4PktFormat_T *ipv4_pkt_p   = (IML_Ipv4PktFormat_T*)payload_p;
        IML_Tcphdr_T        *tcp_header_p = NULL;
        IML_WEBAUTH_T       *webauth_ent_p;
        UI32_T              ipHeaderLen, tbl_idx;
        BOOL_T              ret = FALSE;

    if (  (NULL != payload_p)
        &&(IML_IPPROTO_TCP == ipv4_pkt_p->protocol)
       )
    {
        /* need to improve for fragment packets
         * i think it's a rare case
         */
        ipHeaderLen = (0x0f & ipv4_pkt_p->ver_len) * 4;
        tcp_header_p = (IML_Tcphdr_T*)((UI8_T*)ipv4_pkt_p+ipHeaderLen);

        tbl_idx = IML_MGR_GetWebauthEntry(ipv4_pkt_p->dstIp, tcp_header_p->th_dport);
        if (tbl_idx != IML_MGR_WEBAUTH_ENTRY_MAX)
        {
            webauth_ent_p = &iml_webauth_tbl[tbl_idx];

            IML_MGR_CheckSumAdjust(
                (UI8_T *) &ipv4_pkt_p->checksum,
                (UI8_T *) &ipv4_pkt_p->srcIp,
                4,
                (UI8_T *) &webauth_ent_p->dst_ip,
                4);

            IML_MGR_CheckSumAdjust(
                (UI8_T *) &tcp_header_p->th_csum,
                (UI8_T *) &ipv4_pkt_p->srcIp,
                4,
                (UI8_T *) &webauth_ent_p->dst_ip,
                4);

            ipv4_pkt_p->srcIp = webauth_ent_p->dst_ip;
            memcpy(src_mac_p, webauth_ent_p->dst_mac, SYS_ADPT_MAC_ADDR_LEN);
            ret = TRUE;
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_DoWebAuthChecking
 *--------------------------------------------------------------------------
 * PURPOSE  : To do webauth checking.
 *           (e.g, if dip is not for us && dport == http do fllowing for webauth
 *                  1. record  the sip+sport data
 *                  2. replace the dip with dut's ip and update ip/tcp checksum)
 * INPUT    : in_lport      - ingress lport
 *            vid_ifidx     - vid ifindex
 *            mref_handle_p - pointer to handle of L_MM_Mref
 *            payload_p     - pointer to payload of pdu
 *            dst_mac_p     - pointer to original destination mac
 *            new_dst_mac_p - pointer to new destination mac
 * OUTPUT   : payload_p     - pointer to payload of pdu
 *                            (da,dip,ip/tcp checksum will be modified)
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
static BOOL_T IML_MGR_DoWebAuthChecking(
    UI32_T              in_lport,
    UI32_T              vid_ifidx,
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *payload_p,
    UI8_T               *dst_mac_p,
    UI8_T               *new_dst_mac_p)
{
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T         *ipv6_pkt_p   = (IML_Ipv6PktFormat_T*)payload_p;
#endif
    IML_Ipv4PktFormat_T         *ipv4_pkt_p   = (IML_Ipv4PktFormat_T*)payload_p;
    IML_Tcphdr_T                *tcp_header_p = NULL;
    NETCFG_TYPE_InetRifConfig_T rif_config = {0};
    UI32_T                      ipHeaderLen, tbl_idx;
    BOOL_T                      ret = FALSE;

    if (  (NULL != payload_p) && (NULL != dst_mac_p)
        &&(IML_IPPROTO_TCP == ipv4_pkt_p->protocol)
       )
    {
        /* need to improve for fragment packets
         * i think it's a rare case
         */
        ipHeaderLen = (0x0f & ipv4_pkt_p->ver_len) * 4;
        tcp_header_p = (IML_Tcphdr_T*)((UI8_T*)ipv4_pkt_p+ipHeaderLen);

        memcpy(rif_config.addr.addr, &(ipv4_pkt_p->dstIp), SYS_ADPT_IPV4_ADDR_LEN);
        rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
        rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

        /* if lport's webauth is enabled && dport == http do fllowing for webauth
         *   1. record  the sip+sport data
         *   2. replace the dip with dut's ip and update ip/tcp checksum
         *
         * bcz webauth need lport to check if sip + lport is pass the authentication
         */
        if ((in_lport <= sizeof(iml_webauth_sts))
            && (TRUE == iml_webauth_sts[in_lport -1])
            &&(L_STDLIB_Ntoh16(tcp_header_p->th_dport) == 80)
            && (FALSE == WEBAUTH_OM_IsIPValidByLPort(ipv4_pkt_p->srcIp, in_lport))
           )
        {
            IML_MGR_BD_MSG("->");

            memset (&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex    = vid_ifidx;
            rif_config.addr.type  = L_INET_ADDR_TYPE_IPV4;

            /* need to improve,
             * call om api directly will fail...
             */
            if (NETCFG_TYPE_OK ==
                    NETCFG_POM_IP_GetNextRifFromInterface(&rif_config))
            {
                tbl_idx = IML_MGR_SetWebauthEntry(
                            in_lport,
                            ipv4_pkt_p->srcIp,
                            tcp_header_p->th_sport,
                            ipv4_pkt_p->dstIp,
                            dst_mac_p);

                if (IML_MGR_WEBAUTH_ENTRY_MAX != tbl_idx)
                {
                    IML_MGR_DoWebAuthReplaceForIng(
                        mref_handle_p,
                        payload_p,
                        &rif_config.addr.addr[0],
                        new_dst_mac_p);
                    ret = TRUE;
                }
            }
        }
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort
 *--------------------------------------------------------------------------
 * PURPOSE  : To get original dip and ingress lport
 *              by specified src ip/tcp port.
 * INPUT    : src_ip, src_tcpport
 * OUTPUT   : org_dip_p, lport_p
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. all args are in network order
 *            2. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_MGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort(
    UI32_T  src_ip,
    UI16_T  src_tcpport,
    UI32_T  *org_dip_p,
    UI32_T  *lport_p)
{
    UI32_T  tbl_index;
    BOOL_T  ret = FALSE;

    IML_MGR_BD_MSG("src_ip/src_tcpport-%08lx/%d", (unsigned long)src_ip, src_tcpport);

    tbl_index = IML_MGR_GetWebauthEntry(src_ip, L_STDLIB_Hton16(src_tcpport));
    if (IML_MGR_WEBAUTH_ENTRY_MAX != tbl_index)
    {
        *lport_p   = iml_webauth_tbl[tbl_index].in_lport;
        *org_dip_p = iml_webauth_tbl[tbl_index].dst_ip;
        IML_MGR_BD_MSG("lport/dip-%ld/%08lx", (long)*lport_p, (unsigned long)*org_dip_p);
        ret = TRUE;
    }

    return ret;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_SetWebauthStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : To set webauth status by specified logical port.
 * INPUT    : lport (1-based), status
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : 1. for webauth
 *--------------------------------------------------------------------------*/
BOOL_T IML_MGR_SetWebauthStatus(
    UI32_T  lport,
    BOOL_T  status)
{
    BOOL_T  ret = FALSE;

    IML_MGR_BD_MSG("lport/wbstats-%ld/%d", (long)lport, status);

    if ((0 < lport) && (lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT))
    {
        /* may need to clear the iml_webauth_tbl for this lport if
         * this lport is disabled...
         */
        iml_webauth_sts[lport -1] = status;
        ret = TRUE;
    }

    return ret;
}

#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

static void IML_MGR_RxTaskMain(void* arg)
{
    UI32_T  event_var;
    UI32_T  wait_events;
    UI32_T  rcv_events;
    UI32_T  ret_value;
    UI8_T   msg_buf[SYSFUN_SIZE_OF_MSG(IML_MSG_SIZE)];

    IML_MSG_T     *msg;
    SYSFUN_Msg_T  *msgbuf_p = (SYSFUN_Msg_T*)msg_buf;

    IML_PACKET_RX_T *p_msg_data_blk;
    UI32_T  priority;
    UI32_T  member_id;

    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetL2muxGroupTGHandle();

    /* join the thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_IML_RX_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    /* Create IML message queue
     */
    if (SYSFUN_CreateMsgQ (SYSFUN_MSGQKEY_PRIVATE, SYSFUN_MSGQ_UNIDIRECTIONAL, &iml_msg_q_id)!=SYSFUN_OK)
    {
        iml_msg_q_id = 0;

        /* DBG_printf ("ERR---Can't create Message Q in IML.\n");*/
        return;
    }

    /* Prepare waiting event and init. event var.
     */
    wait_events = IML_TASK_EVENT_RECEIVE_PACKET |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  IML_TASK_EVENT_ENTER_TRANSITION;
    event_var = 0;

    while(1)
    {
        if ((ret_value = SYSFUN_ReceiveEvent (wait_events, SYSFUN_EVENT_WAIT_ANY,
                                             (event_var==0)?SYSFUN_TIMEOUT_WAIT_FOREVER:SYSFUN_TIMEOUT_NOWAIT,
                                             &rcv_events))!=SYSFUN_OK)
        {
            rcv_events = 0;  /* William, added to clear the var. */
            /* SYSFUN_Debug_Printf("\nIML:ReceiveEvent != SYSFUN_OK\n"); */
        }

        event_var |= rcv_events;

        if (event_var == 0)
        {
            /* Log to system : ERR--Receive Event Failure
             */
            SYSFUN_Debug_Printf("\nIML_TaskMain:Receive event failure\n");
            continue;
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (event_var  & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_IML_RX);
            event_var  ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            }

            if (iml_using_priority == TRUE)
            {
                while (L_MSG_GetFromPriorityQueue(&priority_queue, (L_MSG_Message_T*)msgbuf_p->msg_buf, &priority))
                {
                    msg = GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR((IML_L_MSG_T*)(msgbuf_p->msg_buf));
                    p_msg_data_blk = (IML_PACKET_RX_T*) msg->mtext;

                    L_MM_Mref_Release(&(p_msg_data_blk->mref_handle_p));
                    L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
                }
            }
            else
            {
                /* task in transition mode, should clear resource (msgQ) in task
                 */
                while(SYSFUN_ReceiveMsg(iml_msg_q_id, 0, SYSFUN_TIMEOUT_NOWAIT, IML_MSG_SIZE, msgbuf_p)==SYSFUN_OK)
                {
                    msg            = GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR((IML_L_MSG_T*)(msgbuf_p->msg_buf));
                    p_msg_data_blk = (IML_PACKET_RX_T*) msg->mtext;

                    L_MM_Mref_Release(&(p_msg_data_blk->mref_handle_p));
                    L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
                }
            }

            if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }

            if (event_var & IML_TASK_EVENT_ENTER_TRANSITION )
                is_transition_done = TRUE;  /* Turn on the transition done flag */

            event_var = 0;
            continue;
        }

        if (event_var & IML_TASK_EVENT_RECEIVE_PACKET)
        {
            if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            }

            if (iml_using_priority == TRUE)
            {
                if (L_MSG_GetFromPriorityQueue(&priority_queue, (L_MSG_Message_T *)msgbuf_p->msg_buf, &priority) != TRUE)
                {
                    event_var ^= IML_TASK_EVENT_RECEIVE_PACKET;
                    if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    {
                        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                    }
                    continue;
                }
            }
            else
            {
                if(SYSFUN_ReceiveMsg(iml_msg_q_id, 0, SYSFUN_TIMEOUT_NOWAIT, IML_MSG_SIZE, msgbuf_p) == SYSFUN_RESULT_NO_MESSAGE)
                {
                    event_var ^= IML_TASK_EVENT_RECEIVE_PACKET;
                    if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
                    {
                        SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
                    }
                    continue;
                }
            }

            {
                msg            = GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR((IML_L_MSG_T*)(msgbuf_p->msg_buf));
                p_msg_data_blk = (IML_PACKET_RX_T*) msg->mtext;
                switch (msg->mtype)
                {
                    case LAN_PACKET_RX_CALLBACK:
                        IML_MGR_PrintPacketType(p_msg_data_blk->classified_packet_type);
                        IML_MGR_PacketDispatch(p_msg_data_blk->mref_handle_p,
                                               p_msg_data_blk->dst_mac,
                                               p_msg_data_blk->src_mac,
                                               p_msg_data_blk->ingress_vid,
                                               p_msg_data_blk->etherType,
                                               p_msg_data_blk->packet_length,
                                               p_msg_data_blk->src_port,
                                               p_msg_data_blk->classified_packet_type,
                                               p_msg_data_blk->extension_header_len,
                                               &p_msg_data_blk->src_rif);

                        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
                        break;

                    default :
                        break;
                }
            }
            if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
            {
                SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }
        } /* if (event_var & IML_TASK_EVENT_RECEIVE_PACKET) */
    }/* end while */

    L_THREADGRP_Leave(tg_handle, member_id);
} /* end of IML_RxTaskMain() */


static void IML_MGR_TxTaskMain(void* arg)
{
    UI32_T mref_handle_offset;
    L_MM_Mref_Handle_T *mref_handle_p;
    IML_MGR_RecvPktFromTCPIPStackArgs_T packet_args;
    UI32_T member_id;

    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetL2muxGroupTGHandle();

    /* join the thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_IML_RX_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(TRUE)
    {
        /* The calling process will suspend until packet is received
         */
        if(IML_MGR_RecvPktFromTCPIPStack(&mref_handle_offset, &packet_args) != IML_TYPE_RETVAL_OK)
            continue;
        mref_handle_p = (L_MM_Mref_Handle_T*)L_IPCMEM_GetPtr(mref_handle_offset);

        if ((SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) ||
            (mref_handle_p == NULL))
        {
            iml_counter.drop_tx_cnt++;
            L_MM_Mref_Release(&mref_handle_p);
            continue;
        }

        if(L_THREADGRP_Execution_Request(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
        }

        IML_MGR_ProcessTxPacket(mref_handle_p, packet_args);

        if(L_THREADGRP_Execution_Release(tg_handle, member_id) != TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
        }
    }

    L_THREADGRP_Leave(tg_handle, member_id);
} /* end of IML_TxTaskMain() */

static void IML_MGR_ProcessTxPacket(
    L_MM_Mref_Handle_T   *mref_handle_p,
    IML_MGR_RecvPktFromTCPIPStackArgs_T packet_args)
{
    UI32_T vlan_id;
    UI32_T vlan_ifindex;
    UI32_T pdu_len;
    void   *pdu_p;
    BOOL_T is_forward;
    IML_Ipv4PktFormat_T *ipv4_hdr_p;
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T *ipv6_hdr_p;
#endif
    NETCFG_TYPE_InetRifConfig_T rif_conf;

    if (IML_MGR_IsNullMac(packet_args.dst_mac)|| IML_MGR_IsNullMac(packet_args.src_mac))
    {
        iml_counter.drop_tx_null_mac_cnt++;
        goto release;
    }

    if (NULL == (pdu_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len)))
    {
        SYSFUN_Debug_Printf("%s: L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        goto release;
    }

    vlan_id  = packet_args.tag_info & 0x1FFF;
    if (FALSE == VLAN_OM_ConvertToIfindex(vlan_id, &vlan_ifindex))
    {
        SYSFUN_Debug_Printf("%s: VLAN_OM_ConvertToIfindex return FALSE\n",__FUNCTION__);
        goto release;
    }

    is_forward = FALSE;
    if (packet_args.packet_type == IPV4_FORMAT)
    {
        ipv4_hdr_p = (IML_Ipv4PktFormat_T*)pdu_p;
        memcpy(rif_conf.addr.addr, &(ipv4_hdr_p->srcIp), SYS_ADPT_IPV4_ADDR_LEN);
        rif_conf.addr.type    = L_INET_ADDR_TYPE_IPV4;
        rif_conf.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        rif_conf.ifindex      = vlan_ifindex;
        if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetRifFromExactIpAndIfindex(&rif_conf, TRUE))
        {
            is_forward = FALSE;
        }
        else
        {
            is_forward = TRUE;
        }
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (packet_args.packet_type ==IPV6_FORMAT)
    {
        ipv6_hdr_p = (IML_Ipv6PktFormat_T*)pdu_p;
        memcpy(rif_conf.addr.addr, &(ipv6_hdr_p->src_addr), SYS_ADPT_IPV6_ADDR_LEN);
        /* how about L_INET_ADDR_IS_IPV6_LINK_LOCAL? */
        if((rif_conf.addr.addr[0] == 0xFE)
         && (rif_conf.addr.addr[1] == 0x80))
        {
            rif_conf.addr.type = L_INET_ADDR_TYPE_IPV6Z;
        }
        else
        {
            rif_conf.addr.type = L_INET_ADDR_TYPE_IPV6;
        }
        rif_conf.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        if (NETCFG_TYPE_OK == NETCFG_OM_IP_GetIPv6RifConfig(&rif_conf))
        {
            is_forward = FALSE;
        }
        else
        {
            is_forward = TRUE;
        }
    }
#endif /*#if (SYS_CPNT_IPV6 == TRUE)*/

#if (SYS_CPNT_VRRP == TRUE)
    IML_MGR_ProcessTxVirtualMacAddressPacket(mref_handle_p, (UI8_T*)pdu_p, &packet_args);
#endif
    IML_MGR_SendPkt(mref_handle_p, vlan_ifindex, pdu_len,
                    packet_args.dst_mac, packet_args.src_mac, packet_args.packet_type,
                    is_forward);
    return;
release:
    iml_counter.drop_tx_cnt++;
    L_MM_Mref_Release(&mref_handle_p);
    return;
}

#if (SYS_CPNT_IPV6 == TRUE || SYS_CPNT_MLDSNP == TRUE)
static BOOL_T IML_MGR_GetExtHdrLen(UI8_T *payload_p, UI8_T ext_hdr_type, UI32_T *ext_hdr_len_p)
{
    switch (ext_hdr_type)
    {
        case IML_IPV6_EXT_HDR_HOP_BY_HOP:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IML_IPV6_EXT_HDR_DESTINATION:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IML_IPV6_EXT_HDR_ROUTING:
            *ext_hdr_len_p = (*(payload_p+1)+1)<<3;
            break;
        case IML_IPV6_EXT_HDR_FRAGMENT:
            *ext_hdr_len_p = 8;
            break;
        case IML_IPV6_EXT_HDR_AUTHENTICATION:
            *ext_hdr_len_p = (*(payload_p+1)+2)<<2;
            break;
        case IML_IPV6_EXT_HDR_SECURITY: /* because payload is encrypted */
        default: /* not an IPV6 extension header */
            return FALSE;
    }

    return TRUE;
}

static BOOL_T IML_MGR_GetTotalExtHdrLen(UI8_T *payload_p, UI32_T payload_len, UI32_T *total_ext_hdr_len_p, UI32_T *next_hdr_type_p)
{
    IML_Ipv6PktFormat_T *ipv6_pkt_p   = (IML_Ipv6PktFormat_T*)payload_p;

    if (ipv6_pkt_p->next_hdr != IML_IPPROTO_ICMPV6 &&
        ipv6_pkt_p->next_hdr != IML_IPPROTO_TCP &&
        ipv6_pkt_p->next_hdr != IML_IPPROTO_UDP &&
        ipv6_pkt_p->next_hdr != IML_IPPROTO_NONE)
    {
        UI8_T   *ipv6_opt_hdr_p;
        UI32_T  ext_hdr_len;
        UI8_T   next_hdr_type;

        next_hdr_type  = ipv6_pkt_p->next_hdr;
        ipv6_opt_hdr_p = ipv6_pkt_p->pay_load;
        *total_ext_hdr_len_p = 0;
        while (IML_MGR_GetExtHdrLen(ipv6_opt_hdr_p, next_hdr_type, &ext_hdr_len) == TRUE)
        {
            next_hdr_type = *ipv6_opt_hdr_p;
            ipv6_opt_hdr_p = ipv6_opt_hdr_p + ext_hdr_len;
            *total_ext_hdr_len_p += ext_hdr_len;

            if ((sizeof(IML_Ipv6PktFormat_T) + *total_ext_hdr_len_p) > payload_len)
            {
                *total_ext_hdr_len_p = 0;
                return FALSE;
            }

            /* Enough to check next IPv6 extension header?
             */
            if ((sizeof(IML_Ipv6PktFormat_T) + *total_ext_hdr_len_p + 2) > payload_len)
            {
                break;
            }
        }
        if (next_hdr_type == IML_IPPROTO_ICMPV6 ||
            next_hdr_type == IML_IPPROTO_TCP ||
            next_hdr_type == IML_IPPROTO_UDP ||
            next_hdr_type == IML_IPPROTO_PIM ||
            next_hdr_type == IML_IPPROTO_NONE)
        {
            *next_hdr_type_p = next_hdr_type;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else /* no extension header */
    {
        *next_hdr_type_p = ipv6_pkt_p->next_hdr;
        *total_ext_hdr_len_p = 0;
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_IPV6 == TRUE || SYS_CPNT_MLDSNP == TRUE) */

#if 0 /* checking is moved to SYS_CALLBACK_MGR_HandleReceiveNdPacket */
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - IML_MGR_CheckRaGuard
 *--------------------------------------------------------------------------
 * PURPOSE  : To check if packet should be dropped by IPv6 RA GUARD.
 * INPUT    : ing_lport, classified_packet_type
 * OUTPUT   : None
 * RETURN   : TRUE  - packet should be dropped by RA GUARD.
 *            FALSE - packet need further processing.
 * NOTES    : None
 *--------------------------------------------------------------------------*/
static BOOL_T IML_MGR_CheckRaGuard(
    UI32_T  ing_lport,
    UI32_T  classified_packet_type)
{
    UI32_T  pkt_type = NETCFG_TYPE_RG_PKT_RR;
    BOOL_T  ret = FALSE;

    /* only check RA/RR
     */
    switch (classified_packet_type)
    {
    case IML_CLASSIFIED_ICMP_ND_ROUTER_ADVT:
        pkt_type = NETCFG_TYPE_RG_PKT_RA;

    case IML_CLASSIFIED_ICMP_ND_REDIRECT:
        if (TRUE == NETCFG_POM_ND_RAGUARD_IsEnabled(
                        ing_lport, pkt_type))
        {
            ret = TRUE;
        }
        break;
    default:
        break;
    }

    return ret;
}
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */
#endif

/* FUNCTION NAME : IML_MGR_PacketClassification
 * PURPOSE:
 *      Classify the received packet to find out the packet type.
 * INPUT:
 *      payload     -- payload of frame, excluded ether frame header
 *      etherType   -- either IP type or ARP type
 *      ingress_vid -- vid in frame header (or from which interface received).
 *      dst_mac     -- destination mac address (from ether header)
 *
 * OUTPUT:
 *      classified_packet_type -- classfied packet type
 *      ext_hdr_len_p         --  extension header length (for ipv6)
 *
 * RETURN:
 *      TRUE  -- succefully parse out the packet type
 *      FALSE -- fail to parse out the packet type
 *
 * NOTES:
 *      None.
 */
static BOOL_T IML_MGR_PacketClassification(UI8_T  *payload,
                                           UI32_T payload_len,
                                           UI16_T etherType,
                                           UI32_T ingress_vid,
                                           UI32_T lport,
                                           UI8_T  dst_mac[6],
                                           UI32_T *classified_packet_type,
                                           UI32_T *ext_hdr_len_p)
{

    UI32_T ingress_vid_ifindex = 0;
    UI32_T next_hdr_type       = 0;
    //UI32_T src_rif             = 0;
    UI16_T dst_port            = 0;
    UI16_T opcode              = 0;
    BOOL_T my_packet           = FALSE;
    IML_Udphdr_T        *udp_header_p = NULL;
    IML_Ipv4PktFormat_T *ipv4_pkt_p   = (IML_Ipv4PktFormat_T*)payload;
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T *ipv6_pkt_p   = (IML_Ipv6PktFormat_T*)payload;
#endif
    IML_ArpPktFormat_T  *arp_pkt_p    =(IML_ArpPktFormat_T *)payload;
    NETCFG_TYPE_InetRifConfig_T  rif_config;

    VLAN_OM_ConvertToIfindex(ingress_vid, &ingress_vid_ifindex);
    *classified_packet_type = IML_CLASSIFIED_OTHER;
    *ext_hdr_len_p = 0;

    /* Find out dispatch components by examining packet
     */
    if (etherType == IPV4_FORMAT )
    {
        if ((L_STDLIB_Ntoh32(ipv4_pkt_p->dstIp) & 0xe0000000) == 0xe0000000)
        {
            iml_counter.rx_mcast_cnt++;
        }
        else
        {
            memcpy(rif_config.addr.addr, &(ipv4_pkt_p->dstIp), SYS_ADPT_IPV4_ADDR_LEN);
            rif_config.addr.type    = L_INET_ADDR_TYPE_IPV4;
            rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            rif_config.ifindex      = ingress_vid_ifindex;
            if (NETCFG_OM_IP_GetRifFromExactIpAndIfindex(&rif_config, TRUE)==NETCFG_TYPE_OK)
            {
                my_packet = TRUE;
                *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
            }
        }

        /* for fragmented IPv4 packet, it should be sent to kernel.
         * flags:| 00 | 01 | 02 | 13 bits |
         *       |  R | DF | MF | OffSET  |
         * R: Reserved, DF: Don't Fragment, MF: More Fragement
         * We check if MF is 0 and framgent offset is 0
         */
        if (((L_STDLIB_Ntoh16(ipv4_pkt_p->frag_off) & IML_IP_FLAG_MF) != 0) ||
            ((L_STDLIB_Ntoh16(ipv4_pkt_p->frag_off) & IML_IP_FRAG_OFFSET) != 0))
        {
            return TRUE;
        }

        switch (ipv4_pkt_p->protocol)
        {
            case IML_IPPROTO_OSPF:
                *classified_packet_type = IML_CLASSIFIED_OSPF;
                break;
            case IML_IPPROTO_IGMP:
                *classified_packet_type = IML_CLASSIFIED_IGMP;
                break;
            case IML_IPPROTO_PIM:
                *classified_packet_type = IML_CLASSIFIED_PIM;
                break;
            case IML_IPPROTO_VRRP:
                *classified_packet_type = IML_CLASSIFIED_VRRP;
                break;
            case IML_IPPROTO_UDP:
                udp_header_p = (IML_Udphdr_T *)((char *)ipv4_pkt_p+sizeof(IML_Ipv4PktFormat_T));
                dst_port = L_STDLIB_Ntoh16(udp_header_p->uh_dport);

#if (SYS_CPNT_UDP_HELPER == TRUE)
                UI32_T if_addr;
                UI32_T ret = UDPHELPER_OM_CheckHelper(ingress_vid_ifindex, ipv4_pkt_p->dstIp,
                                                      dst_port, &if_addr);
                /* record this is UDP packet */
                if ( ret )
                {/* There is port scan function */
                    *classified_packet_type = IML_CLASSIFIED_UDPHELPER;
                    break;
                }

#endif

                if(dst_port == IML_BOOTP_PORT_C)
                {
                    *classified_packet_type = IML_CLASSIFIED_BOOTP_CLIENT;
                    break;
                }

                if(dst_port == IML_BOOTP_PORT_S)
                {
                    *classified_packet_type = IML_CLASSIFIED_BOOTP_SERVER;
                    break;
                }

                if (dst_port == IML_HSRP_PORT)
                {
                    *classified_packet_type = IML_CLASSIFIED_HSRP;
                    break;
                }

                if (dst_port == IML_RIP_PORT)  /* RIP */
                {
                    *classified_packet_type = IML_CLASSIFIED_RIP;
                    break;
                }

                if ((dst_port == IML_TFTP_PORT) &&
                        (my_packet == TRUE))
                {
                    *classified_packet_type = IML_CLASSIFIED_TFTP;
                    break;
                }
                else
                {
                    if (my_packet == TRUE)
                    {
                        /* DHCP, BOOTP, DNS, IP data, IP Mcast data ... */
                        *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
                    }
                    else
                    {
                        /* DHCP, BOOTP, DNS, IP data, IP Mcast data ... */
                        *classified_packet_type = IML_CLASSIFIED_OTHER;
                    }
                }
                break;
            case IML_IPPROTO_IPV6:
                *classified_packet_type = IML_CLASSIFIED_ENCAP_V6;
                break;
            case IML_IPPROTO_ICMP:
                if (my_packet == TRUE)
                {
                    IML_IcmpHdr_T *icmp_hdr_p;
                    icmp_hdr_p = (IML_IcmpHdr_T *)ipv4_pkt_p->payload;
                    if (icmp_hdr_p->type == IML_ICMP_ECHO ||
                        icmp_hdr_p->type == IML_ICMP_ECHOREPLY)
                    {
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ECHO_LOCAL;
                        break;
                    }
                }
                /* Let IML_IPPROTO_ICMP pass through to default if not IML_CLASSIFIED_ICMP_ECHO_LOCAL
                 */
            default:
                if (my_packet == TRUE)
                {
                    /* DHCP, BOOTP, DNS, IP data, IP Mcast data ... */
                    *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
                }
                else
                    /* DHCP, BOOTP, DNS, IP data, IP Mcast data ... */
                    *classified_packet_type = IML_CLASSIFIED_OTHER;

                break;

        }/* end of switch (ipv4_pkt_p->protocol) */
        return TRUE;
    } /* end of if (etherType == IPV4_FORMAT ) */
    else if (etherType == ARP_FORMAT )
    {
        opcode = L_STDLIB_Ntoh16(arp_pkt_p->opcode);

        /* is Arp reply? */
        if (opcode==IML_ARP_OPCODE_REPLY)
        {
            *classified_packet_type = IML_CLASSIFIED_ARP_REPLY;
        }
        else if (opcode==IML_ARP_OPCODE_REQUEST)
        {
            *classified_packet_type = IML_CLASSIFIED_ARP_REQUEST;
        }

        return TRUE;
    }
#if (SYS_CPNT_IPV6 == TRUE || SYS_CPNT_MLDSNP == TRUE)
    else if (etherType == IPV6_FORMAT)
    {
        IML_Icmp6Hdr_T  *icmp6_hdr_p;

#if (SYS_CPNT_IP_TUNNEL == TRUE)
#if (SYS_CPNT_IPV6 == TRUE)
      SYS_CALLBACK_MGR_AnnounceIpv6PacketCallback(SYS_MODULE_IML,ipv6_pkt_p->src_addr,ipv6_pkt_p->dst_addr);
#endif/*SYS_CPNT_IPV6*/
#endif/*SYS_CPNT_IP_TUNNEL*/

        if (TRUE == IML_MGR_GetTotalExtHdrLen(payload, payload_len, ext_hdr_len_p, &next_hdr_type))
        {
            if (next_hdr_type == IML_IPPROTO_ICMPV6)
            {
                icmp6_hdr_p = (IML_Icmp6Hdr_T*)(payload + sizeof(IML_Ipv6PktFormat_T) + *ext_hdr_len_p);
                switch (icmp6_hdr_p->icmp6_type)
                {
#if (SYS_CPNT_IPV6 == TRUE)
                    case IML_ICMPV6_NDISC_NEIGH_SOL:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ND_NEIGH_SOL;
                        break;
                    case IML_ICMPV6_NDISC_NEIGH_ADVT:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT;
                        break;
                    case IML_ICMPV6_NDISC_ROUTER_SOL:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ND_ROUTER_SOL;
                        break;
                    case IML_ICMPV6_NDISC_ROUTER_ADVT:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ND_ROUTER_ADVT;
                        break;
                    case IML_ICMPV6_NDISC_REDIRECT:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_ND_REDIRECT;
                        break;

#if (SYS_CPNT_MLDSNP == TRUE)
                    case IML_ICMPV6_MGM_QUERY:
                      #if(SYS_CPNT_MLDSNP_QUERY_DROP == TRUE)
                        if(MLDSNP_OM_IsQueryDropEnable(lport))
                          return FALSE;
                      #endif
                    case IML_ICMPV6_MGM_REPORT:
                    case IML_ICMPV6_MGM_REDUCTION:
                    case IML_ICMPV6_MLD2_REPORT:
                        *classified_packet_type = IML_CLASSIFIED_ICMP_MLD;
                        break;
#endif
                    default:
                        *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
                        break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE)*/
                }
            }
            else if(next_hdr_type == IML_IPPROTO_UDP)
            {
                IML_Udphdr_T *udp_hdr_p = NULL;

                udp_hdr_p = (IML_Udphdr_T*)(payload + sizeof(IML_Ipv6PktFormat_T) + *ext_hdr_len_p);

#if (SYS_CPNT_IPV6 == TRUE)
                *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
                if (L_STDLIB_Ntoh16(udp_hdr_p->uh_dport) == SYS_DFLT_DHCPv6_CLIENT_PORT ||
                    L_STDLIB_Ntoh16(udp_hdr_p->uh_dport) == SYS_DFLT_DHCPv6_SERVER_PORT)
                {
                    *classified_packet_type = IML_CLASSIFIED_DHCP_V6;
                }
#endif
#if (SYS_CPNT_MLDSNP == TRUE)
                else if (dst_mac[0] == 0x33 && dst_mac[1] == 0x33)
                {
                    *classified_packet_type = IML_CLASSIFIED_MCAST_V6_DATA;
                }

#endif

            }
            else if (next_hdr_type == IML_IPPROTO_PIM)
            {
                *classified_packet_type = IML_CLASSIFIED_PIM;
            }
            else  /* not ICMP6 packet */
            {
#if (SYS_CPNT_MLDSNP == TRUE)
                /* If ethertype=ipv6 and !icmpv6 and da=multicast, then classified as
                 * ipv6 multicast data packet
                 */
                if (dst_mac[0] == 0x33 && dst_mac[1] == 0x33)
                {
                    *classified_packet_type = IML_CLASSIFIED_MCAST_V6_DATA;
                }
                else
#endif
                {
#if (SYS_CPNT_IPV6 == TRUE)
                    *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
#endif
                }
            }
        }
        else  /* Can't get extension header length */
        {
#if (SYS_CPNT_IPV6 == TRUE)
            *classified_packet_type = IML_CLASSIFIED_IP_MGMT;
#endif
        }

        /* if ethertype=ipv6 and !multicast data and !support_ipv6
         * then we don't handle this packet
         */
        if (*classified_packet_type == IML_CLASSIFIED_OTHER)
            return FALSE;
        else
            return TRUE;
    }
#endif /* #if (SYS_CPNT_IPV6 == TRUE || SYS_CPNT_MLDSNP == TRUE) */
    else
    {
        return FALSE;
    }

}/* end of IML_MGR_PacketClassification */


void IML_MGR_RecvPacket(L_MM_Mref_Handle_T *mref_handle_p,
                               UI8_T dst_mac[6],
                               UI8_T src_mac[6],
                               UI16_T tag_info,
                               UI16_T ether_type,
                               UI32_T packet_length,
                               UI32_T l_port)
{
    UI8_T         buf[SYSFUN_SIZE_OF_MSG(IML_MSG_SIZE)];
    SYSFUN_Msg_T  *msgbuf_p = (SYSFUN_Msg_T*)buf;
    IML_MSG_T     *msg_p    = GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR((IML_L_MSG_T*)msgbuf_p->msg_buf);

    IML_PACKET_RX_T       *p_msg_data_blk;
    void                  *payload;
    UI32_T                payload_len;
    MAC_CACHE_MacEntry_T  mac_entry;
    NETCFG_TYPE_InetRifConfig_T  ingress_rif;

    VLAN_OM_Dot1qPortVlanEntry_T port_vlan_entry;

    UI32_T          classified_packet_type;
    UI32_T          extension_header_len;
    UI32_T          priority, ingress_vid;
    UI32_T          ingress_vid_ifindex;
    UI16_T          checksum=0;
    BOOL_T          is_translated=FALSE; /* The packet has been routed */
    UI8_T           vlan_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T           hsrp_mac[5] = {0x00, 0x00, 0x0C, 0x07, 0xAC};
    UI8_T           vrrp_mac[5] = {0x00, 0x00, 0x5E, 0x00, 0x01};


    iml_counter.rx_from_lower_layer_cnt++;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* IML not in master mode, discard the packet.
         */
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* If the packet is unicast and the dst_mac is not VRRP or HSRP and not equal to vlan mac
     * (device mac). It should not go toward CPU, so discard it.
     * The reason why processing here is to do before entering queuing process.
     */
    STKTPLG_POM_GetLocalUnitBaseMac(vlan_mac);
    if ((dst_mac[0] & 0x01)!=0x01)
    {
#if (SYS_CPNT_DAI == TRUE)
        UI8_T status;
#endif
        /* if packet is unicast
         */
#if (SYS_CPNT_DHCPSNP == TRUE)
        if ((memcmp(dst_mac, hsrp_mac, 5) != 0) && (memcmp(dst_mac, vrrp_mac, 5) != 0) &&
            (memcmp(vlan_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN) != 0)&& (FALSE==IML_MGR_IsDhcpPacket(mref_handle_p, ether_type))
#if (SYS_CPNT_DAI == TRUE)
            && !( ether_type == ARP_FORMAT
                  && DAI_TYPE_OK == DAI_OM_GetGlobalDaiStatus(&status)
                  && status == DAI_TYPE_GLOBAL_ENABLED )
#endif
           )
#else
        if ((memcmp(dst_mac, hsrp_mac, 5) != 0) && (memcmp(dst_mac, vrrp_mac, 5) != 0) &&
            (memcmp(vlan_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN) != 0))
#endif
        {
            /* if packet DA is not VRRP or HSRP, and DA != vlan mac, then drop it.
             */
            L_MM_Mref_Release(&mref_handle_p);
            iml_counter.drop_rx_cnt ++;
            iml_counter.drop_rx_unicast_da_invalid_cnt ++;
            return;
        }
    }

    /*  Allocate message data block from Message Block Pool
     */
    if (!L_MPOOL_AllocateBlock (iml_msg_blk_pool, (void **)(&p_msg_data_blk)))
    {
        /*  No more message data block, ie. queue is full !
         */
        L_MM_Mref_Release(&mref_handle_p);
        iml_counter.drop_rx_cnt++;
        iml_counter.drop_no_msg_buf_cnt++;
        return;
    }

    if (ether_type == IPV4_FORMAT
#if (SYS_CPNT_IPV6 == TRUE)
        || ether_type == IPV6_FORMAT
#endif
        )
    {
        iml_counter.rx_ip_cnt++;
    }
    else if (ether_type == ARP_FORMAT)
    {
        iml_counter.rx_arp_cnt++;
    }

    /* find ingress vid
     */
    ingress_vid = tag_info & 0x0fff; /* 12 bit vid */
    if (0 == ingress_vid)
    {
        /* Use PVID as the interface
         */
        if (VLAN_POM_GetDot1qPortVlanEntry (l_port ,&port_vlan_entry) == FALSE)
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
            iml_counter.drop_rx_cnt++;
            return;
        }

        ingress_vid = port_vlan_entry.dot1q_pvid_index;
    }

    /* ingress_vid should be non-zeno here */
    VLAN_OM_ConvertToIfindex(ingress_vid, &ingress_vid_ifindex);

    payload = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);

    /* Classify packet type
     */
    if (IML_MGR_PacketClassification(payload,
                                     payload_len,
                                     ether_type,
                                     ingress_vid,
                                     l_port,
                                     dst_mac,
                                     &classified_packet_type,
                                     &extension_header_len) == FALSE)
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
        return;
    }

/* already checked in L2MUX_GROUP_L2muxReceivePacketCallbackHandler
 */
#if 0
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    if (TRUE == IML_MGR_CheckRaGuard(l_port, classified_packet_type))
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
        return;
    }
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */
#endif

    /* Get CPU Mac and check if the packet is routed packet
     */
    if ( (memcmp(vlan_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN) == 0) &&
         (memcmp(vlan_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0) )
    {
        is_translated = TRUE;
    }

#if 0 /* VaiWang, Wednesday, April 16, 2008 4:34:31 */
    /* Simply comment out this part as the translation is useless by now with BCM chip */
    /* Get ingress rif number
     */
    if (IML_MGR_GetSrcRif(&ingress_vid_ifindex, payload, ether_type, classified_packet_type,&ingress_rif,is_translated) == FALSE)
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
        return;
    }

    /* For translated packet, the ingress vid may be changed.
     * If the vid_ifindex is change, we need to change it in vid too
     */
    if (is_translated == TRUE)
    {
        VLAN_OM_ConvertFromIfindex (ingress_vid_ifindex,&new_ingress_vid);
        if (new_ingress_vid != ingress_vid)
        {
            ingress_vid = new_ingress_vid;
        }
    }
#endif /* vai */

    /* iml add check source MAC :
     * if it is one of my router MAC then (a) no mac cache learning
     *                                    (b) workaround TTL
     */
    /* TODO: need to add decreasement for IPv6's TTL ?? */
    if ( (ether_type == IPV4_FORMAT) &&
         (memcmp(vlan_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN) == 0))
    {
        /* original formula
         *
         * unsigned long sum;
         * ipptr->ttl--;
         * sum = ipptr->Checksum + 0x100;
         * ipptr->Checksum = (sum + (sum>>16))
         */
        IML_Ipv4PktFormat_T *ipptr = (IML_Ipv4PktFormat_T*)payload;

        ipptr->ttl++; /* increment ttl */
        checksum = L_STDLIB_Ntoh16(ipptr->checksum) - 0x100;   /* decrement checksum high byte*/
        if ((checksum & 0xff00) == 0xff0)
            checksum--;
            //ipptr->checksum--;  /* reduce carry */

        ipptr->checksum = L_STDLIB_Hton16(checksum);
    }
    else
    {
        /* mac_learning
         */
        mac_entry.vid_ifindex = ingress_vid_ifindex;
        memcpy(mac_entry.mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);
        mac_entry.lport = l_port;
        MAC_CACHE_SetLearnedMacEntry(mac_entry);
    }

    p_msg_data_blk->mref_handle_p           = mref_handle_p;
    p_msg_data_blk->dst_mac                 = dst_mac;
    p_msg_data_blk->src_mac                 = src_mac;
    p_msg_data_blk->ingress_vid             = ingress_vid;
    p_msg_data_blk->etherType               = ether_type;
    p_msg_data_blk->packet_length           = packet_length;
    p_msg_data_blk->src_port                = l_port;
    p_msg_data_blk->classified_packet_type  = classified_packet_type;
    p_msg_data_blk->extension_header_len    = extension_header_len;
    memcpy(&p_msg_data_blk->src_rif,&ingress_rif, sizeof(NETCFG_TYPE_InetRifConfig_T));

    if (iml_using_priority == TRUE)
    {
        /* Determine the incoming packet priority for giving corresponding packet type
         */
        priority = priority_table[classified_packet_type];

        /* Prepare message block for sending
         */
        msg_p->mtype     = LAN_PACKET_RX_CALLBACK;
        msg_p->mreserved = 0;
        msg_p->mtext     = p_msg_data_blk;
        msg_p->muser     = 0;

        if (L_MSG_PutToPriorityQueue(&priority_queue, (L_MSG_Message_T*)msg_p, priority) == FALSE)
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
            iml_counter.drop_rx_cnt++;
            iml_counter.drop_msg_q_full_cnt++;
            return;
        }
        SYSFUN_SendEvent (iml_rx_task_id, IML_TASK_EVENT_RECEIVE_PACKET);
    }
    else
    {
        /* Prepare message block for sending
         */
        msg_p->mtype     = LAN_PACKET_RX_CALLBACK;
        msg_p->mreserved = 0;
        msg_p->mtext     = p_msg_data_blk;
        msg_p->muser     = 0;

        /* Send message and event
         */
        msgbuf_p->msg_size = IML_MSG_SIZE;
        msgbuf_p->msg_type = 1 ; /* any value other than 0 */
        if (SYSFUN_SendRequestMsg(iml_msg_q_id, msgbuf_p, SYSFUN_TIMEOUT_NOWAIT,
                                  IML_TASK_EVENT_RECEIVE_PACKET, 0,NULL)!=SYSFUN_OK)
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
            iml_counter.drop_rx_cnt++;
            iml_counter.drop_msg_q_full_cnt++;
            return;
        }
    }

    return;
}


/*added bu jinwang for debug,2004-11-04*/
void IML_MGR_ShowQueue()
{

    typedef struct L_MSG_QueueElement_S
    {
        L_MSG_Message_T msg;
        struct L_MSG_QueueElement_S *next;
        UI32_T priority;
    }L_MSG_QueueElement_T;
    typedef struct
    {
        L_MSG_QueueElement_T *head;
        L_MSG_QueueElement_T *tail;
    }L_MSG_QueueList_T;

    typedef struct
    {
        L_PT_Descriptor_T   l_msg_priority_queue_desc;    /* for l_pt descriptor */
        L_MSG_QueueList_T   *queue_list;
        UI16_T              *counter_for_scheduling;      /* the remaining counter for scheduling
                                                          **  deduction
                                                          */
        UI32_T              dequeue_starting_index;       /* for WFQ use only */

    }L_MSG_InternalUsedBuffer_T;
    UI8_T i,total_enque=0;
        L_MSG_InternalUsedBuffer_T *internal_use_buffer;
        L_MSG_QueueElement_T  *element, *element_backup;
        L_MSG_QueueList_T *queue_list;
    IML_MSG_T *msg;
     IML_PACKET_RX_T *p_msg_data_blk;
     UI32_T classified_packet_type;
    internal_use_buffer=priority_queue.internal_use_buffer;
    queue_list=internal_use_buffer->queue_list;
    show_incoming_packet_type =TRUE;
    memcpy(internal_use_buffer->counter_for_scheduling, priority_queue.scheduling, priority_queue.nbr_of_queue * sizeof(*internal_use_buffer->counter_for_scheduling));
    for(i=0;i<IML_ADPT_MAX_NUM_OF_QUEUE;i++)
        {
            printf ("\n queue %d, schedule value %d: ", i,internal_use_buffer->counter_for_scheduling[i]);
            element=queue_list[i].head;
            while(element != NULL)
                {
                    element_backup=element->next;
                    msg=(IML_MSG_T*)&element->msg;
                    p_msg_data_blk=(IML_PACKET_RX_T*)(msg->mtext);
                    classified_packet_type=p_msg_data_blk->classified_packet_type;
                    IML_MGR_PrintPacketType(classified_packet_type);
                    element=element_backup;
                    total_enque++;
                }
            printf ("\n");
        }
    printf ("total enqueue packet is : %d\n",total_enque);
    show_incoming_packet_type =FALSE;
}


static BOOL_T IML_MGR_GetSrcIpFromHdr(UI16_T ethertype, UI8_T *payload, L_INET_AddrIp_T *ip_adr_p)
{
    IML_Ipv4PktFormat_T *ipv4_hdr_p = (IML_Ipv4PktFormat_T*)payload;
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T *ipv6_hdr_p = (IML_Ipv6PktFormat_T*)payload;
#endif
    IML_ArpPktFormat_T  *arp_pkt_p  = (IML_ArpPktFormat_T *)payload;

    switch (ethertype)
    {
        case IPV4_FORMAT:
            memcpy(ip_adr_p->addr, &(ipv4_hdr_p->srcIp), SYS_ADPT_IPV4_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV4;
            ip_adr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case IPV6_FORMAT:
            memcpy(ip_adr_p->addr, &(ipv6_hdr_p->src_addr), SYS_ADPT_IPV6_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV6;
            ip_adr_p->addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            break;
#endif
        case ARP_FORMAT:
            memcpy(ip_adr_p->addr, &(arp_pkt_p->srcIp), SYS_ADPT_IPV4_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV4;
            ip_adr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            break;
        default:
            return FALSE;
    }
    return TRUE;
}


static BOOL_T IML_MGR_GetDstIpFromHdr(UI16_T ethertype, UI8_T *payload, L_INET_AddrIp_T *ip_adr_p)
{
    IML_Ipv4PktFormat_T *ipv4_hdr_p = (IML_Ipv4PktFormat_T*)payload;
#if (SYS_CPNT_IPV6 == TRUE)
    IML_Ipv6PktFormat_T *ipv6_hdr_p = (IML_Ipv6PktFormat_T*)payload;
#endif
    IML_ArpPktFormat_T  *arp_pkt_p  = (IML_ArpPktFormat_T *)payload;

    switch (ethertype)
    {
        case IPV4_FORMAT:
            memcpy(ip_adr_p->addr, &(ipv4_hdr_p->dstIp), SYS_ADPT_IPV4_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV4;
            ip_adr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            break;
#if (SYS_CPNT_IPV6 == TRUE)
        case IPV6_FORMAT:
            memcpy(ip_adr_p->addr, &(ipv6_hdr_p->dst_addr), SYS_ADPT_IPV6_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV6;
            ip_adr_p->addrlen = SYS_ADPT_IPV6_ADDR_LEN;
            break;
#endif
        case ARP_FORMAT:
            memcpy(ip_adr_p->addr, &(arp_pkt_p->dstIp), SYS_ADPT_IPV4_ADDR_LEN);
            ip_adr_p->type = L_INET_ADDR_TYPE_IPV4;
            ip_adr_p->addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            break;
        default:
            return FALSE;
    }
    return TRUE;
}


#if 0
/* FUNCTION NAME : IML_MGR_GetSrcRif
 * PURPOSE:
 *      Get the ingress rif of packet. If the packet is routed packet.
 *      The ingress_vid may need to be modified too.
 * INPUT:
 *      vid_ifindex_p -- ingress vid
 *      payload     -- payload of frame, excluded ether frame header
 *      etherType   -- either IP type or ARP type
 *      classified_packet_type -- the buffer to store the classfied packet type
 *      ingress_rif -- ingress rif of the packet
 *      is_translated -- If the packet is a routed packet
 *
 * OUTPUT:
 *      vid_ifindex -- ingress vlan id of packet
 *      ingress_rif -- ingress rif of packet
 *
 * RETURN:
 *      TRUE  -- succefully get the source rif
 *      FALSE -- fail to get the source rif
 *
 * NOTES:
 *      1)When the packet is a routed packet, the ingress vid will be corrected
 *        to correct vlan id.
 *      2)If the packet is original packet, the ingress vid will not be changed.
 */

static BOOL_T IML_MGR_GetSrcRif (UI32_T *vid_ifindex_p,
                                 UI8_T  *payload,
                                 UI16_T ether_type,
                                 UI32_T classified_packet_type,
                                 NETCFG_TYPE_InetRifConfig_T *ingress_rif,
                                 BOOL_T is_translated)
{
    IML_Ipv4PktFormat_T *ip_pkt_p  =(IML_Ipv4PktFormat_T*)payload;
    IML_ArpPktFormat_T  *arp_pkt_p =(IML_ArpPktFormat_T *)payload;
    L_INET_AddrIp_T     sip;
    UI32_T              ingress_vid_ifindex = *vid_ifindex_p;
    BOOL_T              found_src_rif = FALSE;
    UI8_T               zero[16] = {0};


    /* Get source IP
     */
    memset(&sip, 0, sizeof(L_INET_AddrIp_T));
    if (!IML_MGR_GetSrcIpFromHdr(ether_type, payload, &sip))
    {
            return FALSE;
    }

    if (memcmp(sip.addr, zero, sip.addrlen)==0)
    {
        /* for DUT having dhcp relay feature, dhcp clint-IP is 0,
         * we must find one interface, which is used as gid.
         * if we can't find, this interface is not config. yet.
         */

        /* If the SIP is zero, we will get 1st rif of ingress vid to be src rif
         */
        ingress_rif->ifindex = ingress_vid_ifindex;
        while (NETCFG_POM_IP_GetNextInetRifOfInterface(ingress_rif)==NETCFG_TYPE_OK)
        {
            if (memcmp(ingress_rif->addr.addr, zero, ingress_rif->addr.addrlen)!=0)
            {
                /* find one rif is not (0,0)
                 */
                found_src_rif = TRUE;
                break;
            }
        }
    }
    else
    {
        /* Step-1, check if the packet is translated packet
         */
        memcpy(&(ingress_rif->addr), &sip, sizeof(L_INET_AddrIp_T));

        if (NETCFG_POM_IP_GetRifFromIp(ingress_rif) == NETCFG_TYPE_OK)
        {
            /* Get interface associated with the RIF
             */
            found_src_rif = TRUE;

            /* Check if the vid of rif is same as ingress vid
             */
            if (ingress_rif->ifindex != ingress_vid_ifindex)
            {
                if (is_translated == TRUE)
                {
                    /* If the packet is routed, we will use vid of ingress_rif
                     * as ingress vid
                     */
                    *vid_ifindex_p = ingress_rif->ifindex;
                }
                else
                {
                    /* If the packet is original packet, then we will use 1st rif of
                     * ingress vlan as ingress rif
                     */
                    found_src_rif = FALSE;
                    memset(ingress_rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                    ingress_rif->ifindex = ingress_vid_ifindex;

                    /* 2005.11.23 Willy modify: check if the get rif is ip=0, then get next until not 0.
                     */
                    while (NETCFG_POM_IP_GetNextInetRifOfInterface(ingress_rif) == NETCFG_TYPE_OK)
                    {

                        //if (0 != *((UI32_T*)ingress_rif->ip_addr))
                        if (memcmp(ingress_rif->addr.addr, zero, ingress_rif->addr.addrlen)!=0)
                        {
                            /* find one rif is not (0,0)
                             */
                            found_src_rif = TRUE;
                            break;
                        }
                    }
                }
            }
        }
    }

    if (found_src_rif == FALSE)
    {
        /* If we can not find any proper rif as source rif
         * we will use the primary rif on ingress vlan as source rif
         */

        /* Get the 1st Rif on that ingress vlan
         */
        memset(ingress_rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
        ingress_rif->ifindex = ingress_vid_ifindex;

        /* 2005.11.23 Willy modify: check if the get rif is ip=0, then get next until not 0.
         */
        while (NETCFG_POM_IP_GetNextInetRifOfInterface(ingress_rif)==NETCFG_TYPE_OK)
        {
            //if (0 != *((UI32_T*)ingress_rif->ip_addr))
            if (memcmp(ingress_rif->addr.addr, zero, ingress_rif->addr.addrlen)!=0)
            {
                /* find one rif is not (0,0)
                 */
                found_src_rif = TRUE;
                break;
            }
        }

        /* If still can not get the 1st RIF from ingress vlan without IP=0
         */
        if (found_src_rif == FALSE)
        {
#if(SYS_CPNT_DHCPSNP == TRUE)
            if ((classified_packet_type == IML_CLASSIFIED_BOOTP_CLIENT) ||
                (classified_packet_type == IML_CLASSIFIED_DHCPSNP))
#else
            if (classified_packet_type == IML_CLASSIFIED_BOOTP_CLIENT)
#endif
            {
                /* If it is BOOTP packet and can not find any rif on Device,
                 * we will still return true.
                 */
                memset(ingress_rif, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }

    return TRUE;

}/* end of IML_MGR_GetSrcRif */
#endif

/* FUNCTION NAME : IML_MGR_PacketDispatch
 * PURPOSE:
 *      The packet handling routine for received packet, called by IML task.
 * INPUT:
 *      mem_ref -- the holder for received packet, the pdu is excluding frame header.
 *      dst_mac -- destination mac in frame.
 *      src_mac -- source mac in frame.
 *      ingress_vid-- ingress vlan id
 *      packet_type -- it should be one of {0x0800, 0x0806}
 *      packet_length--total length of packet, excluding frame header.
 *      src_port-- ingress-port.
 *      classified_packet_type -- classified packet type
 *      extension_header_len   -- ipv6 extension header length
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. In BCM5615 chip, CopyToCpu may cause da,sa,tag is egress-info, not ingress-info;
 *         but src-port is ingress-port. (This need special processing)
 *
 */
static void IML_MGR_PacketDispatch(L_MM_Mref_Handle_T *mref_handle_p,
                                   UI8_T  *dst_mac,
                                   UI8_T  *src_mac,
                                   UI32_T ingress_vid,
                                   UI32_T etherType,
                                   UI32_T packet_length,
                                   UI32_T src_port,
                                   UI32_T classified_packet_type,
                                   UI32_T extension_header_len,
                                   NETCFG_TYPE_InetRifConfig_T *src_rif_p)
{
    UI32_T  ingress_vid_ifindex;
    void    *payload;
    UI32_T  payload_len;
    UI32_T  avail_size_before_pdu;
    UI16_T  ethernet_header_len = 0;
#if (SYS_CPNT_ROUTING == TRUE)
    AMTRL3_TYPE_InetHostRouteEntry_T  host_entry;
    UI32_T action_flags;
    UI32_T fib_id = SYS_ADPT_DEFAULT_FIB;
#endif

    /* Step-1. arguments verification
     */
    if ((mref_handle_p == NULL)||(NULL == dst_mac)||(NULL == src_mac))
    {
        iml_counter.inner_error_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* Step-2. Stacking mode checking, this step is used in platform 2.0
     *         (deleted here)
     * Step-3. Packet received, increase counter
     */
    iml_counter.rx_cnt++;
    payload = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);

#ifdef BACKDOOR_OPEN
    if (debug_enable)
    {
        printf ("\n Rx (vid=%ld,port=%ld,type=%lu), ", (long)ingress_vid, (long)src_port, (unsigned long)classified_packet_type);
        IML_MGR_ShowPacket(dst_mac, src_mac, etherType, packet_length, payload);
    }
#endif

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    if (IML_MGR_FilterMgmtVlanPacket(etherType, payload, ingress_vid) == TRUE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        iml_counter.drop_rx_cnt++;
        return;
    }
#endif

    VLAN_OM_ConvertToIfindex(ingress_vid, &ingress_vid_ifindex);

    avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);

    ethernet_header_len = mref_handle_p->pkt_info.ether_header_length;


	/* if available size before pdu is smaller then ethernet header,
	 * we can't move pdu to ethernet head to send packet to TCP/IP stack.
	 * This mref regarded as invalid, free it.
 		 */
    if(avail_size_before_pdu < ethernet_header_len)
    {
        L_MM_Mref_Release(&mref_handle_p);
        iml_counter.drop_rx_cnt++;
        return;
    }

    /* Step-4. Pass the packet to proper components
     */
    switch (classified_packet_type)
    {

        case IML_CLASSIFIED_BOOTP_SERVER:
        {
#if (SYS_CPNT_ROUTING == TRUE)
            /* DHCP server/relay received packet via socket,
             * send to TCP/IP stack
             */
            {

                L_MM_Mref_Handle_T *mref_handle_clone_p;

                mref_handle_clone_p = L_MM_AllocateTxBufferForPktForwarding(mref_handle_p,payload_len,L_MM_USER_ID2(SYS_MODULE_IML, IML_TYPE_TRACE_ID_ENQ_TO_IP_PKT_LIST));

                if(mref_handle_clone_p == NULL)
                {
                    L_MM_Mref_Release(&mref_handle_clone_p);
                    return;
                }

                {
                    UI32_T pdu_len;
                    void  *clone_payload_p;

                    if (NULL==L_MM_Mref_MovePdu(mref_handle_clone_p, (0-ethernet_header_len), &pdu_len))
                    {
                        L_MM_Mref_Release(&mref_handle_clone_p);
                        return;
                    }

                    clone_payload_p = L_MM_Mref_GetPdu(mref_handle_clone_p,&pdu_len);
                    memcpy(clone_payload_p,(payload-ethernet_header_len),(payload_len+ethernet_header_len));
                }
                iml_counter.send_to_kernel_ip_cnt++;

                IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_clone_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
            }
#else
            /* DHCP L2 relay received packet via syscall_back,
             */
            L_MM_Mref_AddRefCount(mref_handle_p, 1);
            iml_counter.send_to_bootp_cnt++;
            SYS_CALLBACK_MGR_ReceiveBootpPacketCallback(SYS_MODULE_IML,
                                                        mref_handle_p,
                                                        packet_length,
                                                        ingress_vid_ifindex,
                                                        dst_mac,
                                                        src_mac,
                                                        ingress_vid,
                                                        src_port);
#endif
        }
        break;
        /* for DHCP client*/
        case IML_CLASSIFIED_BOOTP_CLIENT:
		{
            /* callback to DHCP
             */
            L_MM_Mref_AddRefCount(mref_handle_p, 1);
            iml_counter.send_to_bootp_cnt++;
            SYS_CALLBACK_MGR_ReceiveBootpPacketCallback(SYS_MODULE_IML,
                                                        mref_handle_p,
                                                        packet_length,
                                                        ingress_vid_ifindex,
                                                        dst_mac,
                                                        src_mac,
                                                        ingress_vid,
                                                        src_port);

        }
            break;

#if (SYS_CPNT_UDP_HELPER == TRUE)
        case IML_CLASSIFIED_UDPHELPER:
            L_MM_Mref_AddRefCount(mref_handle_p, 1);
            SYS_CALLBACK_MGR_ReceiveUdpHelperPacketCallback(SYS_MODULE_IML,
                                                            mref_handle_p,
                                                            packet_length,
                                                            ingress_vid_ifindex,
                                                            dst_mac,
                                                            src_mac,
                                                            ingress_vid,
                                                            src_port);
            break;
#endif
#if (SYS_CPNT_MLDSNP == TRUE)
        case IML_CLASSIFIED_ICMP_MLD:
        case IML_CLASSIFIED_MCAST_V6_DATA:
            {
                L_MM_Mref_Handle_T*  du_mref_p = NULL;
                UI32_T length;
                void *src_pkt_p = NULL;
                void *dst_pkt_p = NULL;

                src_pkt_p = L_MM_Mref_GetPdu(mref_handle_p, &length);

                if(mref_handle_p->pkt_info.pkt_is_truncated
                   &&classified_packet_type == IML_CLASSIFIED_MCAST_V6_DATA)
                {
                   IML_Ipv6PktFormat_T *iphdr = ((UI8_T *)src_pkt_p);
                   IML_Udphdr_T *udphdr = ((UI8_T *)src_pkt_p + (40 + extension_header_len)/*ip hdr*/);

                   udphdr->uh_sum  = 0;
                   udphdr->uh_ulen = L_STDLIB_Hton16(length - (40 + extension_header_len));
                   iphdr->payload_len = L_STDLIB_Hton16(length - 40);
                }

                #if (SYS_CPNT_MLDSNP == TRUE)
                /*send to mldsnp*/
                L_MM_Mref_AddRefCount(mref_handle_p, 1);
                SYS_CALLBACK_MGR_ReceiveMldsnpPacketCallback(SYS_MODULE_IML,
                                                               mref_handle_p,
                                                               dst_mac,
                                                               src_mac,
                                                               ingress_vid,
                                                               IPV6_FORMAT,
                                                               packet_length,
                                                               extension_header_len,
                                                               src_port);
                #endif

            }
            break;
#endif /* #if (SYS_CPNT_MLDSNP == TRUE)*/
        case IML_CLASSIFIED_HSRP:
            L_MM_Mref_AddRefCount(mref_handle_p, 1);
            SYS_CALLBACK_MGR_ReceiveHsrpPacketCallback(SYS_MODULE_IML,
                                                       mref_handle_p,
                                                       packet_length,
                                                       ingress_vid_ifindex,
                                                       dst_mac,
                                                       src_mac,
                                                       ingress_vid,
                                                       src_port);
            break;

#if (SYS_CPNT_VRRP == TRUE)
        case IML_CLASSIFIED_VRRP:
            L_MM_Mref_AddRefCount(mref_handle_p, 1);
            SYS_CALLBACK_MGR_ReceiveVrrpPacketCallback(SYS_MODULE_IML,
                                                       mref_handle_p,
                                                       packet_length,
                                                       ingress_vid_ifindex,
                                                       dst_mac,
                                                       src_mac,
                                                       ingress_vid,
                                                       src_port);
            break;
#endif /* #if (SYS_CPNT_VRRP == TRUE) */

        case IML_CLASSIFIED_DHCP_V6:
            {

#if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))

                    UI32_T ingress_vid_ifindex = 0;
                    BOOL_T drop = FALSE;
                    UI8_T  cpu_mac[SYS_ADPT_MAC_ADDR_LEN] ={0};
                    UI32_T length = 0;


                    SWCTRL_GetCpuMac(cpu_mac);
                    /* DHCPv6 request packet, only server and relay need */
                    VLAN_VID_CONVERTTO_IFINDEX(ingress_vid, ingress_vid_ifindex);

#if (SYS_CPNT_DHCPV6SNP == TRUE)
                    {
                        UI8_T global_status=0;

                        /* check if global snooping and vlan snooping is enabled */
                        if(DHCPV6SNP_TYPE_OK!=DHCPV6SNP_POM_GetGlobalSnoopingStatus(&global_status))
                        {
                            L_MM_Mref_Release(&mref_handle_p);
                            return;
                        }

                        if(DHCPV6SNP_TYPE_GLOBAL_SNOOPING_ENABLE == global_status)
                        {
                            /* if it support dhcpv6snp,
                             * we should callback to dhcpv6snp to handle it
                             */

                            L_MM_Mref_AddRefCount(mref_handle_p, 1);
                            iml_counter.send_to_dhcpv6snp_cnt++;
                            SYS_CALLBACK_MGR_ReceiveDhcpv6snpPacketCallback(
                                                           SYS_MODULE_IML,
                                                           mref_handle_p,
                                                           packet_length,
                                                           extension_header_len,
                                                           dst_mac,
                                                           src_mac,
                                                           ingress_vid,
                                                           src_port);
                            drop = TRUE;
                        }
                        else
                        {
                            /* we should send this packet to TCP/IP stack */
                            drop = FALSE;
                        }

                    }
#endif /* #if (SYS_CPNT_DHCPV6SNP == TRUE) */


                    /* check if there's need to relay */
#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
                    {
                        BOOL_T relay_status = FALSE;

                        if(DHCPv6_OM_OK == DHCPv6_POM_GetRelayStatus(ingress_vid_ifindex, &relay_status))
                        {

                            if(TRUE == relay_status)
                            {

                                drop = FALSE;
                            }
                            else
                            {
                                /* check if this packet is for me */
                                if(!memcmp(cpu_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN))
                                {

                                    drop = FALSE;
                                }
                                else
                                {
                                    drop = TRUE;
                                }
                            }

                        }
                    }
#endif
                    if(FALSE == drop)
                    {
                        /* send to TCP/IP stack */
                        /*move duplicate pointer to da*/
                        if (NULL == L_MM_Mref_MovePdu(mref_handle_p, (0 - ethernet_header_len), &length))
                        {
                            L_MM_Mref_Release(&mref_handle_p);
                            return;
                        }
                        L_MM_Mref_AddRefCount(mref_handle_p, 1);
                        IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
                    }


#endif  /* end of #if ((SYS_CPNT_DHCPV6 == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))  */
            }
            break;
        case IML_CLASSIFIED_ICMP_ECHO_LOCAL:
            {

#if (SYS_CPNT_STACKING == TRUE)
                {
                    UI16_T iuc_ethernet_header_len = 0;
                    UI16_T isc_header_len = 0;
                    UI16_T stacking_header_len = 0;
                    UI16_T ethernet_header_len = 0;
                    ICU_GetIUCEthHeaderLen(&iuc_ethernet_header_len);
                    ISC_GetISCHeaderLen(&isc_header_len);
                    LAN_GetStackingHeaderLen(&stacking_header_len);
                    LAN_GetEthHeaderLen(FALSE, &ethernet_header_len);

                    if ((avail_size_before_pdu > ethernet_header_len) &&
                            (avail_size_before_pdu <= (ethernet_header_len + iuc_ethernet_header_len +
                                                 isc_header_len + stacking_header_len)))
                    {
                        avail_size_before_pdu -= (iuc_ethernet_header_len + isc_header_len + stacking_header_len);
                    }
                }
#endif /* SYS_CPNT_STACKING */

                UI32_T pdu_len;
                if (NULL==L_MM_Mref_MovePdu(mref_handle_p, (0-avail_size_before_pdu), &pdu_len))
                {
                    L_MM_Mref_Release(&mref_handle_p);
                    return;
                }
                iml_counter.send_to_kernel_ip_cnt++;
                L_MM_Mref_AddRefCount(mref_handle_p, 1);
                /* use IML_TYPE_PROCESS_PACKET_DIRECT_CALL to make ping response time shorter
                 */
                IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_DIRECT_CALL);
            }
            break;
        case IML_CLASSIFIED_IGMP:
        case IML_CLASSIFIED_IP_MGMT:
        case IML_CLASSIFIED_RIP:
        case IML_CLASSIFIED_OSPF:
        case IML_CLASSIFIED_TFTP:
        case IML_CLASSIFIED_OTHER:
        default:
            {
                {
                    UI32_T pdu_len;
                    if (NULL==L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len))
                    {
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }
                }
                iml_counter.send_to_kernel_ip_cnt++;
                L_MM_Mref_AddRefCount(mref_handle_p, 1);
                IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
            }
            break;

        case IML_CLASSIFIED_PIM:
           {
                /*move mref pointer to da*/
                {
                    UI32_T pdu_len;
                    if (NULL==L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len))
                    {
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }
                }

                #if (SYS_CPNT_MLDSNP == TRUE)
                 if (etherType == IPV6_FORMAT)
                {
                    L_MM_Mref_Handle_T*  du_mref_p = NULL;
                    UI32_T length;
                    void *src_pkt_p = NULL;
                    void *dst_pkt_p = NULL;

                    /* create a new MREF for sending to mldsnp. */
                    if(NULL == (du_mref_p = L_MM_AllocateTxBufferForPktForwarding(mref_handle_p, packet_length + ethernet_header_len, L_MM_USER_ID2(SYS_MODULE_IML, IML_TYPE_TRACE_ID_PACKET_DISPATCH_MLD))))
                    {
                        SYSFUN_Debug_Printf("%s[%d] : L_MM_AllocateTxBufferForPktForwarding fail\n",  __FUNCTION__, __LINE__);
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }

                    /*move mref pointer to da*/
                    if (NULL == L_MM_Mref_MovePdu(du_mref_p, (0 - ethernet_header_len), &length))
                    {
                            L_MM_Mref_Release(&du_mref_p);
                            L_MM_Mref_Release(&mref_handle_p);
                            return;
                    }

                    /*do copy from ip layer to the end*/
                    dst_pkt_p = L_MM_Mref_GetPdu(du_mref_p, &length);
                    src_pkt_p = L_MM_Mref_GetPdu(mref_handle_p, &length);
                    memcpy(dst_pkt_p, src_pkt_p, length);

                    /*move mref poninter back to ip layer*/
                    if (NULL == L_MM_Mref_MovePdu(du_mref_p, ethernet_header_len, &length))
                    {
                            L_MM_Mref_Release(&du_mref_p);
                            L_MM_Mref_Release(&mref_handle_p);
                            return;
                    }

                    /*send to mldsnp*/
                    SYS_CALLBACK_MGR_ReceiveMldsnpPacketCallback(SYS_MODULE_IML,
                                                                   du_mref_p,
                                                                   dst_mac,
                                                                   src_mac,
                                                                   ingress_vid,
                                                                   IPV6_FORMAT,
                                                                   packet_length,
                                                                   extension_header_len,
                                                                   src_port);
                }
                #endif


                iml_counter.send_to_kernel_ip_cnt++;
                L_MM_Mref_AddRefCount(mref_handle_p, 1);
                IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);

            }
            break;

        case IML_CLASSIFIED_ARP_REPLY:
        case IML_CLASSIFIED_ARP_REQUEST:
            {
                IML_ArpPktFormat_T     *arp_pkt_p = (IML_ArpPktFormat_T*)payload;
                NETCFG_TYPE_InetRifConfig_T rif_config;
                UI32_T    pdu_len;
                BOOL_T    ret = TRUE;
                BOOL_T    is_set_amtrl3 = FALSE;
                BOOL_T    is_to_kernel = TRUE;
#if (SYS_CPNT_DEBUG == TRUE)
                UI8_T  arp_packet_sip[18]={0};
                UI8_T  arp_packet_dip[18]={0};
                UI8_T  arp_src_mac[SYS_ADPT_MAC_ADDR_LEN]={0};
                UI8_T  arp_packet_type[4]={0};
                int    year,month,day,hour,minute,second;

                memcpy (arp_src_mac, arp_pkt_p->srcMacAddr, 6);
                L_INET_Ntoa(arp_pkt_p->srcIp,arp_packet_sip);
                L_INET_Ntoa(arp_pkt_p->dstIp,arp_packet_dip);
                SYS_TIME_GetRealTimeClock(&year,&month,&day,&hour,&minute,&second);
                if (classified_packet_type == IML_CLASSIFIED_ARP_REPLY)
                    strcpy((char*)arp_packet_type, "rep");
                else
                    strcpy((char*)arp_packet_type, "req");
#endif
                /* 2009-10-16, Jimi
                 * we should check if global dai is enabled, call back to dai to process this packet */
#if (SYS_CPNT_DAI == TRUE)
                {
                    UI8_T    dai_status = 0 ;
                    if (DAI_TYPE_OK == DAI_MGR_GetGlobalDaiStatus(&dai_status))
                    {
                        if(DAI_TYPE_GLOBAL_ENABLED == dai_status)
                        {
                            L_MM_Mref_AddRefCount(mref_handle_p, 1);
                            if(FALSE == SYS_CALLBACK_MGR_ReceiveArpPacketCallback(SYS_MODULE_IML,
                                                                      mref_handle_p,
                                                                      packet_length,
                                                                      ingress_vid_ifindex,
                                                                      dst_mac,
                                                                      src_mac,
                                                                      ingress_vid,
                                                                      src_port))
                            {
                                ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                             hour, minute, second, arp_packet_type, arp_packet_sip,
                                             arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                             arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                            }
                            else
                            {
                                ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: received %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                             hour, minute, second, arp_packet_type, arp_packet_sip,
                                             arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                             arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                            }
                            break;
                        }
                    }
                    else
                    {
                        ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                             hour, minute, second, arp_packet_type, arp_packet_sip,
                                             arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                             arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                        break;
                    }
                }
#endif

                if( (arp_pkt_p->srcMacAddr[0] & 0x01) == 0x01  ||
                    (*(UI32_T *)arp_pkt_p->srcMacAddr) == 0xffffffff )
                {
                    iml_counter.inner_error_cnt++;
                    L_MM_Mref_Release(&mref_handle_p);
                    ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                         hour, minute, second, arp_packet_type, arp_packet_sip,
                                         arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                         arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    return;
                }

                /* 2005.6.22 Willy Add to Check for any valid ARP packets, if the ingress_vid of
                 * the incomming ARP is not belong to Any local interfaces (for L2 products, it
                 * will be only 1 interface), will not process this packets.
                 */
                if(FALSE == IML_MGR_IsArpValid(arp_pkt_p))
                {
                    iml_counter.inner_error_cnt++;
                    L_MM_Mref_Release(&mref_handle_p);
                    ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                         hour, minute, second, arp_packet_type, arp_packet_sip,
                                         arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                         arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    return;
                }

#if (SYS_CPNT_ROUTING == TRUE)
                IPAL_NeighborEntry_T neigh_entry;
                L_INET_AddrIp_T      dst_addr;
                UI32_T  sip = arp_pkt_p->srcIp;
                UI32_T  dip = arp_pkt_p->dstIp;

                memcpy(rif_config.addr.addr,&(arp_pkt_p->srcIp),SYS_ADPT_IPV4_ADDR_LEN);
                rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
                rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                rif_config.ifindex = ingress_vid_ifindex;
                if (NETCFG_OM_IP_GetRifFromIpAndIfindex(&rif_config) == NETCFG_TYPE_OK)
                {
                    is_set_amtrl3 = TRUE;
                }

#if (SYS_CPNT_VRRP == TRUE)
                if(IML_MGR_IsMasterVirtualIp(ingress_vid_ifindex, dip))
                {
                    is_set_amtrl3 = TRUE;
                }
#endif
                if(is_set_amtrl3)
                {
                    /* we have to ensure kernel handle ARP packet before AMTRL3 set entry
                     */
                    L_MM_Mref_AddRefCount(mref_handle_p, 1);
                    iml_counter.send_to_kernel_arp_cnt++;
                    L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len);
                    if(IML_TYPE_RETVAL_OK == IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_WAIT))
                    {
                        is_to_kernel = FALSE;
                        ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: received %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                 hour, minute, second, arp_packet_type, arp_packet_sip,
                                 arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                 arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    }
                    else
                    {
                        ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                 hour, minute, second, arp_packet_type, arp_packet_sip,
                                 arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                 arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    }

                    memset(&dst_addr, 0, sizeof(dst_addr));
                    memcpy(dst_addr.addr, &sip, SYS_ADPT_IPV4_ADDR_LEN);
                    dst_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                    dst_addr.type = L_INET_ADDR_TYPE_IPV4;

                    if (IPAL_RESULT_OK == IPAL_NEIGH_GetNeighbor(ingress_vid_ifindex, &dst_addr, &neigh_entry) &&
                        0 == memcmp(neigh_entry.phy_address, arp_pkt_p->srcMacAddr, SYS_ADPT_MAC_ADDR_LEN))
                    {
                        memset(&host_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
                        memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                        host_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
                        host_entry.dst_inet_addr.addrlen=SYS_ADPT_IPV4_ADDR_LEN;
                        memcpy(host_entry.dst_inet_addr.addr, &sip, SYS_ADPT_IPV4_ADDR_LEN);
                        memcpy(&host_entry.dst_mac, arp_pkt_p->srcMacAddr, SYS_ADPT_MAC_ADDR_LEN);
                        host_entry.lport = src_port;
                        host_entry.dst_vid_ifindex = ingress_vid_ifindex;
                        action_flags = AMTRL3_TYPE_FLAGS_IPV4;
                        ret = FALSE;
                        if(classified_packet_type == IML_CLASSIFIED_ARP_REPLY)
                        {
                            iml_counter.send_to_amtrl3_arp_cnt++;
                            ret = AMTRL3_PMGR_SetHostRoute(action_flags, fib_id, &host_entry, VAL_ipNetToPhysicalExtType_dynamic);
                        }
                        else
                        {
                            memcpy(rif_config.addr.addr, &dip, SYS_ADPT_IPV4_ADDR_LEN);
                            rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                            rif_config.addr.type    = L_INET_ADDR_TYPE_IPV4;
                            rif_config.ifindex      = ingress_vid_ifindex;

                            /* if dip is the ip address of the DUT's rif,
                             *     then set the host route
                             * else (means this ARP request is not for DUT)
                             *     check whether packet's src ip in one of the DUT's rif subnet
                             *     if yes, call AMTRL3_PMGR_ReplaceExistHostRoute, this function
                             *     will replace exist entry and do nothing if the entry not found
                             *     it is because too many ARP request in the subnet, DUT don't
                             *     do anything if the src ip doesn't exist in the host route table
                             */
                             if (NETCFG_OM_IP_GetRifFromExactIpAndIfindex(&rif_config, TRUE) == NETCFG_TYPE_OK)
                             {
                                 iml_counter.send_to_amtrl3_arp_cnt++;
                                 ret = AMTRL3_PMGR_SetHostRoute(action_flags, fib_id, &host_entry, VAL_ipNetToPhysicalExtType_dynamic);
                             }
#if (SYS_CPNT_VRRP == TRUE)
                             else if(IML_MGR_IsMasterVirtualIp(ingress_vid_ifindex, dip))
                             {
                                iml_counter.send_to_amtrl3_arp_cnt++;
                                ret = AMTRL3_PMGR_SetHostRoute(action_flags, fib_id, &host_entry, VAL_ipNetToPhysicalExtType_dynamic);
                             }
#endif
                             else
                             {
                                 memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                                 memcpy(rif_config.addr.addr, &sip, SYS_ADPT_IPV4_ADDR_LEN);
                                 rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                                 rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
                                 rif_config.ifindex = ingress_vid_ifindex;
                                 if (NETCFG_OM_IP_GetRifFromIpAndIfindex(&rif_config) == NETCFG_TYPE_OK)
                                 {
                                     iml_counter.send_to_amtrl3_arp_cnt++;
                                     ret = AMTRL3_PMGR_ReplaceExistHostRoute(action_flags, fib_id, &host_entry, VAL_ipNetToPhysicalExtType_dynamic);
                                 }
                             }
                         }

                         if (!ret)
                         {
                             /* To prevent kernel always do software route
                              */
                             IPAL_NEIGH_DeleteNeighbor(ingress_vid_ifindex, &dst_addr);
                         }
                     }
                 }

#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

                if(is_to_kernel)
                {
                    /* don't need to wait kernel respond
                     */
                    L_MM_Mref_AddRefCount(mref_handle_p, 1);
                    iml_counter.send_to_kernel_arp_cnt++;
                    L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len);
                    if(IML_TYPE_RETVAL_OK == IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT))
                    {
                        ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: received %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                hour, minute, second, arp_packet_type, arp_packet_sip,
                                arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    }
                    else
                    {
                        ARP_DEBUG_MSG("\r\n%d:%d:%d: ARP: filtered %s src %s %02x-%02x-%02x-%02x-%02x-%02x, dst %s",
                                hour, minute, second, arp_packet_type, arp_packet_sip,
                                arp_src_mac[0], arp_src_mac[1], arp_src_mac[2],
                                arp_src_mac[3], arp_src_mac[4], arp_src_mac[5], arp_packet_dip);
                    }
                }
            }
            break;

/* if packet is trapped to cpu by per port register/rule,
 * the processing below may be not necessary...
 */
#if (SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_GLOBAL == TRUE)
        case IML_CLASSIFIED_ICMP_ND_ROUTER_ADVT:
        case IML_CLASSIFIED_ICMP_ND_REDIRECT:

            /* 1. send to TCP/IP STACK if it's for us
             * 2. send to RG_GROUP for software relay
             */
            {
                UI8_T   *src_pkt_p, *dst_pkt_p;
                UI32_T  src_len, ing_cos, pkt_type = NETCFG_TYPE_RG_PKT_RA;
                UI8_T   cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
                BOOL_T  is_to_tcp = FALSE, is_sw_relay;

#if  (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
                /* need sw relay if this is packet is trapped to cpu (not mirrored)
                 */
                is_sw_relay = NETCFG_POM_ND_RAGUARD_IsAnyPortEnabled();
#else
                /* for bcm platform, RA Guard
                 *  1. enabled  port, packet is dropped before coming here
                 *  2. disabled port, packet is mirroed to cpu, no software relay needed
                 */
                is_sw_relay = FALSE;
#endif /* #if  (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE) */

                /* 1. multicast, to tcp and sw relay
                 * 2.    to dut, to tcp only
                 * 3.   unicast, sw relay only
                 */
                if (dst_mac[0] & 0x01)
                {
                    is_to_tcp = TRUE;
                }
                else
                {
                    SWCTRL_GetCpuMac(cpu_mac);

                    if (0 == memcmp(cpu_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN))
                    {
                        is_to_tcp   = TRUE;
                        is_sw_relay = FALSE;
                    }
                }

                if (classified_packet_type  == IML_CLASSIFIED_ICMP_ND_REDIRECT)
                    pkt_type == NETCFG_TYPE_RG_PKT_RR;

                IML_MGR_BD_MSG("pkt_type/to_tcp/relay-%d/%d/%d",
                                pkt_type, is_to_tcp, is_sw_relay);

                src_pkt_p = L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &src_len);
                if (NULL == src_pkt_p)
                    break;

                /* get cos from the tag of original packet...
                 */
                ing_cos = src_pkt_p[14] >> 5;

                /* a. send to TCP/IP stack
                 */
                if (TRUE == is_to_tcp)
                {
                    L_MM_Mref_Handle_T  *tmp_mref_p;
                    UI32_T              dst_len;

                    /* clone the mref
                     */
                    if (TRUE == is_sw_relay)
                    {
                        tmp_mref_p = L_MM_AllocateTxBufferForPktForwarding(mref_handle_p, payload_len,
                                L_MM_USER_ID2(SYS_MODULE_IML, IML_TYPE_TRACE_ID_PACKET_DISPATCH));

                        dst_pkt_p = L_MM_Mref_MovePdu(tmp_mref_p, (0-ethernet_header_len), &dst_len);
                        if ((NULL != tmp_mref_p) && (NULL != dst_pkt_p))
                        {
                            /* clone success */
                            memcpy(dst_pkt_p, src_pkt_p, src_len);
                        }
                        else
                        {
                            /* clone failed, send to tcp without sw relay
                             */
                            if (NULL != tmp_mref_p)
                            {
                                iml_counter.inner_error_cnt++;
                                L_MM_Mref_Release(&tmp_mref_p);
                            }
                            tmp_mref_p  = mref_handle_p;
                            is_sw_relay = FALSE;

                            IML_MGR_BD_MSG("pkt_type -%d, clone failed", pkt_type);
                        }
                    }
                    else
                    {
                        tmp_mref_p = mref_handle_p;
                    }

                    /* add ref count of old mref, if old mref is used here
                     */
                    if (tmp_mref_p == mref_handle_p)
                        L_MM_Mref_AddRefCount(mref_handle_p, 1);

                    IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(tmp_mref_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
                }

#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)
                /* b. send to RG_GROUP for software relay
                 */
                if (TRUE == is_sw_relay)
                {
                    /* move the original pdu pointer back to ip header
                     */
                    src_pkt_p = L_MM_Mref_MovePdu(mref_handle_p, ethernet_header_len, &src_len);
                    if (NULL != src_pkt_p)
                    {
                        /* RA Guard need to use the whole packet...
                         */
                        L_MM_Mref_AddRefCount(mref_handle_p, 1);

                        SYS_CALLBACK_MGR_ReceiveRaGuardPacketCallback(
                            SYS_MODULE_IML,
                            mref_handle_p,
                            dst_mac,
                            src_mac,
                            ingress_vid,
                            ing_cos,
                            pkt_type,
                            packet_length,
                            src_port);
                    }
                }
#endif /*#if (SYS_CPNT_IPV6_RA_GUARD_SW_RELAY == TRUE)*/
            }
            break;
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_GLOBAL == TRUE) */

#if (SYS_CPNT_IPV6 == TRUE)
        case IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT:
        case IML_CLASSIFIED_ICMP_ND_NEIGH_SOL:
            {
                IML_Icmp6NdNeighSolAdvtHdr_T  *nd_pkt_p;
                IML_Icmp6OptHdr_T        *nd_opt;
                IML_Ipv6PktFormat_T      *ipv6_pkt_p = (IML_Ipv6PktFormat_T*)payload;
                UI8_T                    *lladdr = NULL;
                NETCFG_TYPE_InetRifConfig_T rif_config;
                UI32_T                   pdu_len, opt_left;
                BOOL_T    ret = TRUE;

                nd_pkt_p = (IML_Icmp6NdNeighSolAdvtHdr_T *)(((UI8_T*)payload)+sizeof(IML_Ipv6PktFormat_T)+extension_header_len);

                if (ipv6_pkt_p->payload_len < extension_header_len ||
                    (ipv6_pkt_p->payload_len-extension_header_len) < sizeof(IML_Icmp6NdNeighSolAdvtHdr_T))
                {
                    iml_counter.inner_error_cnt++;
                    L_MM_Mref_Release(&mref_handle_p);
                    return;
                }

                /* In current implementation, we send NA to Kernel's TCP/IP stack first
                 * in order to let it perform the validation of NA packet and then set
                 * to AMTRL3 only if we can get the neigbor entry from Kernel after that.
                 * So we skip the validation step here.
                 */
                /*if (!IML_MGR_IsNdAdvtSolValid(nd_pkt_p))
                {
                    iml_counter.inner_error_cnt++;
                    L_MM_Mref_Release(&mref_handle_p);
                    return;
                }*/

                /* Get Target Link Layer address for Neighbor Advertisement
                 */
                if (classified_packet_type == IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT)
                {
                    /* Check whether it has ICMP6's option
                     */
                    if ((ipv6_pkt_p->payload_len-extension_header_len) >= (sizeof(IML_Icmp6NdNeighSolAdvtHdr_T)+sizeof(IML_Icmp6OptHdr_T)))
                    {
                        nd_opt   = (IML_Icmp6OptHdr_T*)nd_pkt_p->option;
                        opt_left = ipv6_pkt_p->payload_len - extension_header_len - sizeof(IML_Icmp6NdNeighSolAdvtHdr_T);
                        while (TRUE)
                        {
                            if (nd_opt->opt_type == 0 || nd_opt->opt_len == 0)
                            {
                                break;
                            }

                            if ((nd_opt->opt_len << 3)>opt_left)
                            {
                                break;
                            }

                            if (nd_opt->opt_type == 1 || /* source link-layer address */
                                nd_opt->opt_type == 2)   /* target link-layer address */
                            {
                                if (nd_opt->opt_len != 1) /* 1 means 8 bytes */
                                {
                                    iml_counter.inner_error_cnt++;
                                    L_MM_Mref_Release(&mref_handle_p);
                                    return;
                                }

                                if (nd_opt->opt_type == 2)
                                {
                                    lladdr = nd_opt->opt_data;
                                    break;
                                }
                            }

                            opt_left -= (nd_opt->opt_len << 3);
                            if (opt_left<8) /* 8 is the smallest size of option TLV */
                            {
                                break;
                            }

                            nd_opt = (IML_Icmp6OptHdr_T*)(((UI8_T*)nd_opt)+(nd_opt->opt_len<<3));
                        }
                    }

                    if( lladdr && ((lladdr[0] & 0x01) == 0x01  ||
                        (*(UI32_T *)lladdr) == 0xffffffff ))
                    {
                        iml_counter.inner_error_cnt++;
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }
                }

                memset(rif_config.addr.addr,0x0,sizeof(L_INET_AddrIp_T));
                memcpy(rif_config.addr.addr,&(ipv6_pkt_p->src_addr),SYS_ADPT_IPV6_ADDR_LEN);
                if (rif_config.addr.addr[0]==0xFE && rif_config.addr.addr[1]==0x80)
                {
                    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6Z;
                    VLAN_OM_ConvertFromIfindex(ingress_vid_ifindex, &rif_config.addr.zoneid);
                }
                else
                {
                    rif_config.addr.type = L_INET_ADDR_TYPE_IPV6;
                }
                rif_config.addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                if (NETCFG_OM_IP_GetRifFromSubnet(&rif_config) == NETCFG_TYPE_OK)
                {
                    /* modified by steven.gao */
                    if(rif_config.addr.type == L_INET_ADDR_TYPE_IPV6
                    && rif_config.ifindex != ingress_vid_ifindex)
                    {
                        iml_counter.inner_error_cnt++;
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                    }

#if (SYS_CPNT_IPV6_ROUTING == TRUE)
                    if (classified_packet_type == IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT &&
                        nd_pkt_p->u_nd_advt.solicited == TRUE)
                    {
                        IPAL_NeighborEntry_T neigh_entry;
                        L_INET_AddrIp_T      dst_addr;

                        L_MM_Mref_AddRefCount(mref_handle_p, 1);
                        iml_counter.send_to_kernel_nd_cnt++;
                        L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len);
                        IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_WAIT);

                        memset(&dst_addr, 0, sizeof(dst_addr));
                        memcpy(dst_addr.addr, nd_pkt_p->target, SYS_ADPT_IPV6_ADDR_LEN);
                        dst_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                        if(L_INET_ADDR_IS_IPV6_LINK_LOCAL(dst_addr.addr))
                        {
                            dst_addr.type = L_INET_ADDR_TYPE_IPV6Z;
                            VLAN_OM_ConvertFromIfindex(ingress_vid_ifindex, &(dst_addr.zoneid));
                        }
                        else
                            dst_addr.type = L_INET_ADDR_TYPE_IPV6;
                        if (IPAL_RESULT_OK == IPAL_NEIGH_GetNeighbor(ingress_vid_ifindex, &dst_addr, &neigh_entry) &&
                            (lladdr == NULL || 0 == memcmp(neigh_entry.phy_address, lladdr, SYS_ADPT_MAC_ADDR_LEN)))
                        {
                            memset(&host_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
                            host_entry.dst_inet_addr = dst_addr;
                            memcpy(&host_entry.dst_mac, neigh_entry.phy_address, SYS_ADPT_MAC_ADDR_LEN);
                            host_entry.lport = src_port;
                            host_entry.dst_vid_ifindex = ingress_vid_ifindex;
                            action_flags = AMTRL3_TYPE_FLAGS_IPV6;
                            iml_counter.send_to_amtrl3_nd_cnt++;

                            if (nd_pkt_p->u_nd_advt.override != TRUE)
                                action_flags |= AMTRL3_TYPE_FLAGS_NOT_OVERRIDE;

                            /* DUT only receive neighbor solicitation for itself, so don't need to consider
                             * using AMTRL3_PMGR_ReplaceExistHostRoute() as ARP request
                             * See comment at IML_CLASSIFIED_ARP_REQUEST above
                             */
                            if (FALSE == (ret = AMTRL3_PMGR_SetHostRoute(action_flags, fib_id, &host_entry, VAL_ipNetToPhysicalExtType_dynamic)))
                            {
                                /* To prevent kernel always do software route
                                 */
                                IPAL_NEIGH_DeleteNeighbor(ingress_vid_ifindex, &dst_addr);
                            }
                        }
                    }
                    else
#endif
                    {
                        L_MM_Mref_AddRefCount(mref_handle_p, 1);
                        iml_counter.send_to_kernel_nd_cnt++;
                        L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len);
                        IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
                    }
                }
                else
                {
                    /* send to kernel */
                    L_MM_Mref_AddRefCount(mref_handle_p, 1);
                    iml_counter.send_to_kernel_nd_cnt++;
                    L_MM_Mref_MovePdu(mref_handle_p, (0-ethernet_header_len), &pdu_len);

                    IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);
                }
            }
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
            case IML_CLASSIFIED_ENCAP_V6:
            {
                /*UI8_T  cpu_mac[SYS_ADPT_MAC_ADDR_LEN] ={0};*/
                NETCFG_TYPE_L3_Interface_T l3_if;
                UI32_T                     ingress_vid_ifindex = 0;
                UI32_T                     length = 0;



                /*move src mref pointer to da*/
                if (NULL == L_MM_Mref_MovePdu(mref_handle_p, (0- ethernet_header_len), &length))
                {
                        L_MM_Mref_Release(&mref_handle_p);
                        return;
                }

                /* We receive encapsulated ipv6 packet because we need to software forward at first time,
                 * and then we will write a host/net route to chip to do hardware forward;
                 * no matter this packet is for me or need to forward, we send it into TCP/IP stack to
                 * let kernel decide how to route
                 */

                L_MM_Mref_AddRefCount(mref_handle_p, 1);
                IML_MGR_SendPktToTCPIPStack(L_IPCMEM_GetOffset(mref_handle_p), ingress_vid, IML_TYPE_PROCESS_PACKET_NETIF_RX_NO_WAIT);


            }
            break;
#endif
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
    }

    L_MM_Mref_Release(&mref_handle_p);
    return;

}/* end of IML_MGR_PacketDispatch */


/* FUNCTION NAME: IML_SendPkt
 * PURPOSE: Send packet
 * INPUT:  *mref_handle_p -- L_MREF descriptor
 *         vid_ifindex    -- vid ifindex
 *         packet_length  -- packet length
 *         *dst_mac --
 *         *src_mac -- this mac will not be used!
 *         packet_type --
 *         forward --
 * OUTPUT: none
 * RETURN: successful (0), failed (-1)
 * NOTES:  forward will be obsolete when L3AMTR is well defined
 */
static UI32_T IML_MGR_SendPkt(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI32_T vid_ifindex,
    UI32_T packet_length,
    UI8_T *dst_mac,
    UI8_T *src_mac,
    UI16_T packet_type,
    BOOL_T forward)
{
    UI32_T  cos_value=0;
    UI32_T  rc = MAC_CACHE_NOT_EXIST;
    UI8_T   *payload;
    UI32_T  payload_len;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    UI32_T  out_lport = 0;
    BOOL_T  is_tagged, bcast_send;
    UI32_T  vid;


    /*  Record IML are using the MREF now
     */
#ifdef  L_MREF_DEBUG_ID
    mref_handle_p->current_usr_id = SYS_MODULE_IML;
#endif

    if (FALSE == VLAN_OM_ConvertFromIfindex(vid_ifindex, &vid))
    {
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return (-1);
    }
    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry) == FALSE)
    {
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return (-1);
    }


    /* L2 snooping protocol check here */
    if(IML_MGR_TxSnoopPacket_Callback_Func(
            mref_handle_p,
            dst_mac,
            src_mac,
            packet_length,
            vid_ifindex,
            0,
            0,
            packet_type))
    {
        return (0);
    }


    /*  Step3.
     *  If (ARP packet) use vlan mac as source mac. (replace caller's mac).
     */
    payload = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);

#ifdef BACKDOOR_OPEN
    if (debug_enable)
    {
        printf ("\n Tx (vid-ifindex=%ld), ",(long)vid_ifindex);
        IML_MGR_ShowPacket(dst_mac, src_mac, (UI32_T)packet_type, packet_length, payload);
    }
#endif

#if(SYS_CPNT_IP_TUNNEL == TRUE)
    {
        if(FALSE == IML_MGR_SetDynamicTunnelRoute(payload, vid_ifindex, packet_type))
        {
             iml_counter.drop_tx_cnt++;
             L_MM_Mref_Release(&mref_handle_p);
             return (-1);
        }
    }
#endif

    bcast_send = TRUE;
    /* Step4.
     * Depend on unicast/multicast/broadcast, do something :
     *     Unicast       : verify the egress vlan & port is correct.
     *     B'cast/M'cast : send to all the ports.
     */
    if ((dst_mac[0] & 0x01)!=0x01) /* unicast */
    {
        bcast_send = FALSE;
        /*  Find out the (lport, ip, rif) had learnt
         */
        if (forward == FALSE)
            rc = MAC_CACHE_GetLport(vid_ifindex, dst_mac, &out_lport);
        else
            rc = MAC_CACHE_NOT_EXIST;

        /* 2003.09.07, William, fixing Mac Cache age-out, then flooding always issue.
         *             For BCM5615, only ARP can update mac cache.
         */
        if (MAC_CACHE_OK != rc)
        {
            /* try to get from AMTR
             */
            AMTR_TYPE_AddrEntry_T addr_entry;

            memcpy(addr_entry.mac, dst_mac, 6);
            addr_entry.vid  = vid;
            if (AMTR_PMGR_GetExactAddrEntry(&addr_entry)==TRUE)
            {
                rc = MAC_CACHE_OK;
                out_lport = addr_entry.ifindex;
            }
        }
        /* 2003.09.07, William, end of patch Mac-Cache age-out issue */

#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf (" Tx-->lport=%d rc=%lu forward=%u\n", (int)out_lport, (unsigned long)rc, forward);
        }
#endif

        /* suger, 03-11-2004, add checking of (out_lport == SYS_ADPT_STACKING_PORT).
         * Otherwise, if out_lport is stacking port, the packet will send by multicast.
         */
        if ((rc == MAC_CACHE_OK)&&
            ((out_lport>0)&&
             ((out_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)||
             (out_lport == SYS_ADPT_STACKING_PORT))))
        {
            /*  the mac had learned.
             */
            if (SYS_ADPT_STACKING_PORT != out_lport)
            {
                if (out_lport > SYS_ADPT_TOTAL_NBR_OF_LPORT)
                {
                    //logMsg (" IML : hugh lport number=%d, lock you\n", (int)out_lport, 0,0,0,0,0);
                }

                /*  1. Verify the port in port list of vlan
                 */
                if (vlan_entry.dot1q_vlan_current_egress_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
                {
                    /*  2. Check the port is tagged or untagged ?
                     */
                    if (vlan_entry.dot1q_vlan_current_untagged_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
                        is_tagged = FALSE;
                    else
                        is_tagged = TRUE;

#ifdef  L_MREF_DEBUG_ID
                    mref_handle_p->next_usr_id = SYS_MODULE_LAN;
#endif

                    if (IPV4_FORMAT == packet_type
#if (SYS_CPNT_IPV6 == TRUE)
                        || IPV6_FORMAT == packet_type
#endif
                        )
                        iml_counter.tx_ip_cnt++;
                    else
                        iml_counter.tx_arp_cnt++;

                    iml_counter.tx_cnt++;

#if (SYS_CPNT_WEBAUTH == TRUE)
                    if (TRUE == IML_MGR_DoWebAuthReplaceForEgr(payload, src_mac))
                    {

                    }
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

                    L2MUX_MGR_SendPacket(mref_handle_p, dst_mac, src_mac, packet_type,
                                          (UI16_T) vid, packet_length, out_lport, is_tagged,
                                          cos_value,FALSE);
                }
                else
                {
                    /* Jason Chen, 12-12-2000. not in the same subnet -> discard it
                     */
                    iml_counter.drop_tx_cnt++;
                    L_MM_Mref_Release(&mref_handle_p);
                }
                return (0);
            }
            else
            {   /* special handle stacking port */

#ifdef  L_MREF_DEBUG_ID

                mref_handle_p->next_usr_id = SYS_MODULE_LAN;
#endif

                /*  4. Send out the packet
                 */
#ifdef BACKDOOR_OPEN
                if (debug_enable)
                {
                    printf ("\n (Tx:vid=%d, lport=%d), ", (int)vid, (int)out_lport);
                }
#endif

                L2MUX_MGR_SendPacket(mref_handle_p, dst_mac, src_mac, packet_type,
                                      (UI16_T) vid, packet_length, out_lport, TRUE,
                                      cos_value,FALSE);
                return (0);
            }

        }
        else
        {
            bcast_send = TRUE;
        }
    }

    if(bcast_send) /* broadcast, m'cast */
    {
#ifdef  L_MREF_DEBUG_ID
        mref_handle_p->next_usr_id = SYS_MODULE_LAN;
#endif

        /* 2002.07.04. William, original code. use lport-list to send packet, but there is counter in arg.
         * LAN_SendMultiPacket (mem_ref, dst_mac, src_mac, packet_type, (UI16_T)vid, packet_length,
         * port_list, untag_port_list, active_uport_count_per_unit, cos_value);
         */
        L2MUX_MGR_SendMultiPacket (mref_handle_p, dst_mac, src_mac, packet_type, (UI16_T)vid, packet_length,
                   vlan_entry.dot1q_vlan_current_egress_ports, vlan_entry.dot1q_vlan_current_untagged_ports,
                   cos_value);

#ifdef BACKDOOR_OPEN
        if (debug_enable)
            printf ("\n (Tx: multi-dest), ");
#endif

        iml_counter.tx_bcast_cnt++;
        iml_counter.tx_cnt++;
    } /* end of broadcast */

    return (0);
} /* end of IML_SendPkt */



UI32_T IML_MGR_SetManagementVid(UI32_T vid_ifIndex)
{
    UI32_T  vid;

#if (SYS_CPNT_ROUTING == FALSE)
    UI32_T  prev_vid, prev_vid_ifIndex;
    UI8_T   cpu_mac[6];
#endif
    VLAN_OM_ConvertFromIfindex (vid_ifIndex, &vid);

    if (VLAN_OM_IsVlanExisted(vid) != TRUE)
    {
        return (-1);
    }

    /* For L2 only product, need remove previous entry from mac table
     */
#if (SYS_CPNT_ROUTING == FALSE)
    VLAN_OM_GetManagementVlan(&prev_vid); /* check previous Mgmt_Vlan id */
#endif

    /* Set Mgmt Vlan to VLAN_mgr before we delete previous mgmt vlan
     */
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    if (VLAN_PMGR_SetIpInterface (vid)==FALSE)
#else
    if (VLAN_PMGR_SetManagementVlan(vid)==FALSE)
#endif
    {
        return (-1);
    }

    /* in L2 only product, remove the previous CPU mac from previous Mac table
     */
#if (SYS_CPNT_ROUTING == FALSE)
    if (prev_vid != vid)
    {
        /*  1. Delete CPU MAC. with previous vlan and mac.
         */
        VLAN_OM_ConvertToIfindex(prev_vid, &prev_vid_ifIndex);
        VLAN_PMGR_GetVlanMac(prev_vid_ifIndex, cpu_mac);
        AMTR_PMGR_DeleteCpuMac(prev_vid, cpu_mac);
        VLAN_PMGR_LeaveManagementVlan(prev_vid);

        /*  2. change to new vlan.
         *  3. Get management vlan's mac as CPU MAC
         *  4. Set CPU MAC with new vlan and mac.
         */
        VLAN_PMGR_GetVlanMac(vid_ifIndex, cpu_mac);
        AMTR_PMGR_SetCpuMac(vid, cpu_mac, FALSE);
    }
#endif

    return (0);
} /* end of IML_SetManagementVid */


void IML_MGR_GetManagementVid(UI32_T *vid_ifIndex)
{
    UI32_T vid;

    /*  2001.12.27, William,
     *  UI32_T sys_manage_vid;
     *  VLAN_MGR_ConvertFromIfindex (sys_manage_vid_ifIndex, &sys_manage_vid);
     *  *vid = sys_manage_vid;
     */
     /* Penny 2002/3/28: check mgmt vlan from VLAN */
    if (VLAN_OM_GetManagementVlan(&vid) == FALSE)
    {
        vid = SYS_DFLT_SWITCH_MANAGEMENT_VLAN;
    }

    VLAN_OM_ConvertToIfindex(vid, vid_ifIndex);
}


#if ((SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE))
/* FUNCTION NAME: IML_MGR_SendPktByVlanEntry
 * PURPOSE: Send  packet by DHCPV6SNP according to vlan entry
 * INPUT:  mref_handle_p -- L_MREF descriptor
 *             packet_length  -- packet length
 *             vid                  -- vid
 *             dst_mac_p      -- destination mac address
 *             src_mac_p      -- source mac address
 *             vlan_entry      -- the main purpose is to get the egress port list.
 * OUTPUT: none
 * RETURN: TRUE/FALSE
 * NOTES:  called by DHCPV6SNP  which is in dhcpsnp_engine.c
 */
BOOL_T IML_MGR_SendPktByVlanEntry(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length,
      UI32_T vid,UI8_T *dst_mac_p, UI8_T *src_mac_p, VLAN_OM_Dot1qVlanCurrentEntry_T *vlan_entry_p)
{
    const UI16_T  packet_type=0x86dd;   /* IPV6 type in ethernet header */
    UI32_T  cos_value=0;



    /* check null pointer */
    if(NULL==mref_handle_p)
        return FALSE;

    if((NULL == dst_mac_p)||(NULL==src_mac_p)||(NULL==vlan_entry_p))
    {
        /* free mref */
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    if (FALSE == L_MM_Mref_SetPduLen(mref_handle_p, packet_length))
    {
        /* free mref */
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    /* send to L2MUX */
    L2MUX_MGR_SendMultiPacket(mref_handle_p, dst_mac_p, src_mac_p, packet_type, (UI16_T)vid, packet_length,
         vlan_entry_p->dot1q_vlan_current_egress_ports, vlan_entry_p->dot1q_vlan_current_untagged_ports,
         cos_value);

    /* increase tx counter */
    iml_counter.tx_bcast_cnt++;
    iml_counter.tx_cnt++;
    return TRUE;

}
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
                                           int *queue_full_drop)
{
    if (ip_packet_from_lan   == NULL ||
        ip_packet_to_p2      == NULL ||
        arp_packet_from_lan  == NULL ||
        arp_packet_to_p2     == NULL ||
        queue_full_drop      == NULL)
    {
        return FALSE;
    }

    *ip_packet_from_lan   = (int)iml_counter.rx_ip_cnt;
    *ip_packet_to_p2      = (int)iml_counter.send_to_kernel_ip_cnt;
    *arp_packet_from_lan  = (int)iml_counter.rx_arp_cnt;
    *arp_packet_to_p2     = (int)iml_counter.send_to_kernel_arp_cnt;
    *arp_packet_to_amtrl3 = (int)iml_counter.send_to_amtrl3_arp_cnt;
    *queue_full_drop      = (int)iml_counter.drop_msg_q_full_cnt;

    return TRUE;

}/* end of IML_MGR_GetSystemPerformanceCounter */


/* FUNCTION NAME: IML_MGR_ClearSystemPerformanceCounter
 * PURPOSE: export function to clear the packet counters in IML
 * INPUT:  none
 * OUTPUT: none
 * RETURN: void
 * NOTES:
 */
void IML_MGR_ClearSystemPerformanceCounter(void)
{
    /* Initial all internal data structure
     */
    memset (&iml_counter, 0, sizeof(IML_MGR_COUNTER_T));
}


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
BOOL_T IML_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    IML_MGR_IPCMsg_T *mgr_msg_p;

    if(ipcmsg_p == NULL)
        return FALSE;

    mgr_msg_p = (IML_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        mgr_msg_p->type.result = IML_TYPE_RESULT_FAIL;
        ipcmsg_p->msg_size = IML_MGR_MSGBUF_TYPE_SIZE;
        return TRUE;
    }

    switch(mgr_msg_p->type.cmd)
    {
        case IML_MGR_IPC_SEND_PACKET:
            mgr_msg_p->type.result = IML_MGR_SendPkt(L_IPCMEM_GetPtr(mgr_msg_p->data.SendPacket_data.mref_handle_offset),
                            mgr_msg_p->data.SendPacket_data.ifindex,
                            mgr_msg_p->data.SendPacket_data.packet_length,
                            mgr_msg_p->data.SendPacket_data.dst_mac,
                            mgr_msg_p->data.SendPacket_data.src_mac,
                            mgr_msg_p->data.SendPacket_data.packet_type,
                            mgr_msg_p->data.SendPacket_data.forward);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;
#if (SYS_CPNT_DHCPV6SNP == TRUE)
        case IML_MGR_IPC_SEND_PACKET_BY_VLAN_ENTRY:
             mgr_msg_p->type.result = IML_MGR_SendPktByVlanEntry(L_IPCMEM_GetPtr(mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.mref_handle_offset),
                            mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.packet_length,
                            mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.vid,
                            mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.dst_mac,
                            mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.src_mac,
                            &(mgr_msg_p->data.SendBootPPacketByDhcpSnp_data.dot1q_entry));
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif
        case IML_MGR_IPC_GET_MANAGEMENT_VID:
            IML_MGR_GetManagementVid(&mgr_msg_p->data.ManagementVid_data.vid_ifindex);
            ipcmsg_p->msg_size=IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_ManagementVid_Data_S);
            break;
        case IML_MGR_IPC_SET_MANAGEMENT_VID:
            mgr_msg_p->type.result = IML_MGR_SetManagementVid(mgr_msg_p->data.ManagementVid_data.vid_ifindex);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;

#if (SYS_CPNT_DAI == TRUE)
        case IML_MGR_IPC_SEND_ARP_INSPECTION_PACKET:
            mgr_msg_p->type.result = IML_MGR_ARP_Inspection_SendPkt(
                L_IPCMEM_GetPtr(mgr_msg_p->data.ArpInspection_data.mref_handle_offset),
                mgr_msg_p->data.ArpInspection_data.vid, mgr_msg_p->data.ArpInspection_data.packet_length,
                mgr_msg_p->data.ArpInspection_data.dst_mac, mgr_msg_p->data.ArpInspection_data.src_mac,
                mgr_msg_p->data.ArpInspection_data.packet_type, mgr_msg_p->data.ArpInspection_data.src_lport_ifindex);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
        case IML_MGR_IPC_GETORGDIPNLOPORTBYSIPSTCPPORT:
            mgr_msg_p->type.result_bool = IML_MGR_GetOrgDipAndIngLportBySrcIpAndSrcTcpPort(
                mgr_msg_p->data.WebauthClient_data.src_ip,
                mgr_msg_p->data.WebauthClient_data.src_tcpport,
                &mgr_msg_p->data.WebauthClient_data.org_dip,
                &mgr_msg_p->data.WebauthClient_data.lport);
            ipcmsg_p->msg_size=IML_MGR_GET_MSGBUFSIZE(IML_MGR_IPCMsg_WebauthClient_Data_S);
            break;
        case IML_MGR_IPC_SETWEBAUTHSTATUS:
            mgr_msg_p->type.result_bool = IML_MGR_SetWebauthStatus(
                mgr_msg_p->data.WebauthStatus_data.lport,
                mgr_msg_p->data.WebauthStatus_data.status);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
        case IML_MGR_IPC_BROADCAST_EXCEPT_IN_PORT:
            mgr_msg_p->type.result = IML_MGR_BroadcastBootPPkt_ExceptInport(
                            L_IPCMEM_GetPtr(mgr_msg_p->data.BroadcastExceptInport_data.mref_handle_offset),
                            mgr_msg_p->data.BroadcastExceptInport_data.vid_ifindex,
                            mgr_msg_p->data.BroadcastExceptInport_data.packet_length,
                            mgr_msg_p->data.BroadcastExceptInport_data.dst_mac,
                            mgr_msg_p->data.BroadcastExceptInport_data.src_mac,
                            mgr_msg_p->data.BroadcastExceptInport_data.src_lport_ifindex);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;

        case IML_MGR_IPC_SEND_TO_PORT:
            mgr_msg_p->type.result = IML_MGR_SendPktToPort(
                            L_IPCMEM_GetPtr(mgr_msg_p->data.SendPktToPort_data.mref_handle_offset),
                            mgr_msg_p->data.SendPktToPort_data.packet_length,
                            mgr_msg_p->data.SendPktToPort_data.dst_mac,
                            mgr_msg_p->data.SendPktToPort_data.src_mac,
                            mgr_msg_p->data.SendPktToPort_data.vid,
                            mgr_msg_p->data.SendPktToPort_data.out_lport,
                            mgr_msg_p->data.SendPktToPort_data.packet_type);
            ipcmsg_p->msg_size=IML_MGR_MSGBUF_TYPE_SIZE;
            break;
#endif

        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            mgr_msg_p->type.result = IML_TYPE_RESULT_FAIL;
            ipcmsg_p->msg_size = IML_MGR_MSGBUF_TYPE_SIZE;
    }

    return TRUE;
}


/*  LOCAL FUNCTION BODY
 */

 /*------------------------------------------------------------------------------
 * ROUTINE NAME : IML_MGR_IsDhcpPacket
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Check if this is dhcp packet
 * INPUT:
 *    mref_handle_p  --  mref pointer
 *    ether_type     --  ether type
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsDhcpPacket(L_MM_Mref_Handle_T *mref_handle_p, UI16_T ether_type)
{
    IML_Udphdr_T        *udp_header_p = NULL;
    IML_Ipv4PktFormat_T *ip_pkt_p     = NULL;
    UI16_T              udp_dstport;    /* 2006-05, Joseph */
    UI32_T              pdu_len;

    if (ether_type == IPV4_FORMAT)
    {
        ip_pkt_p=(IML_Ipv4PktFormat_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if(ip_pkt_p->protocol == IML_IPPROTO_UDP)
        {
            udp_header_p = (IML_Udphdr_T *)((char *)ip_pkt_p+sizeof(IML_Ipv4PktFormat_T));
            /* begin 2006-05, Joseph */
            udp_dstport = L_STDLIB_Ntoh16(udp_header_p->uh_dport);
            if((udp_dstport == IML_BOOTP_PORT_C) || (udp_dstport == IML_BOOTP_PORT_S))
            {
                return TRUE;
            }
            /* end 2006-05 */
        }
    }
    return FALSE;
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : IML_MGR_IsDhcp6Packet
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Check if this is dhcpv6 packet
 * INPUT:
 *    mref_handle_p  --  mref pointer
 *    ether_type     --  ether type
 * OUTPUT:
 *    None
 *
 * RETURN:
 *    TRUE/FALSE
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsDhcp6Packet(L_MM_Mref_Handle_T *mref_handle_p, UI16_T ether_type)
{
    IML_Udphdr_T        *udp_header_p = NULL;
    IML_Ipv6PktFormat_T  *ip_pkt_p     = NULL;
    UI16_T              udp_dstport;
    UI32_T              pdu_len;

    if (ether_type == IPV6_FORMAT)
    {
        ip_pkt_p=(IML_Ipv6PktFormat_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if(ip_pkt_p->next_hdr == IML_IPPROTO_UDP)
        {
            udp_header_p = (IML_Udphdr_T *)((char *)ip_pkt_p+sizeof(IML_Ipv6PktFormat_T));

            udp_dstport = L_STDLIB_Ntoh16(udp_header_p->uh_dport);
            if((udp_dstport == IML_DHCP6_PORT_C) || (udp_dstport == IML_DHCP6_PORT_S))
            {
                return TRUE;
            }

        }
    }
    return FALSE;
}

static BOOL_T IML_MGR_IsArpValid(IML_ArpPktFormat_T *arp_pkt_p)
{
    #define IML_ARP_REQ     1
    #define IML_ARP_RSP     2
    #define IML_INARP_REQ   8
    #define IML_INARP_RSP   9
    #define IML_IP_LAN_TYPE 0x0800
    #define IML_ARP_ETHER   1

    UI32_T sip    = 0;
    UI16_T opcode = 0;

    /* 1. Verify src protocol addr (check 0.0.0.0 and 255.255.255.255)
     */
    sip = L_STDLIB_Ntoh32(arp_pkt_p->srcIp);
    if ((sip == 0) || (sip == 0xffffffff) )
    {
        return FALSE;
    }

    /* 2. Check the type is handlable ?
     *  if not defined type, discard the packet
     */
    opcode = L_STDLIB_Ntoh16(arp_pkt_p->opcode);
    if ((opcode != IML_ARP_REQ) &&
        (opcode != IML_ARP_RSP) &&
        (opcode != IML_INARP_REQ) &&
        (opcode != IML_INARP_RSP))
    {
        return FALSE;
    }

    /* 3. verify protocol
     */
    if(L_STDLIB_Ntoh16(arp_pkt_p->protocolType) != IML_IP_LAN_TYPE)
    {
        return FALSE;
    }

    /* 4. verify protocol address len
     */
    if(arp_pkt_p->protocolLen != 4)
    {
        return FALSE;
    }

    /* 5. verify subnet type
     */
    if(L_STDLIB_Ntoh16(arp_pkt_p->hardwareType) == IML_ARP_ETHER)
    {
        /* verify subnet len
         */
        if( arp_pkt_p->hardwareLen != 6)
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}/* end of IML_MGR_IsArpValid */

static BOOL_T IML_MGR_IsNdAdvtSolValid(IML_Icmp6NdNeighSolAdvtHdr_T *nd_pkt_p)
{
    /* TODO: check whether NS/NA header format is correct
     */
    return TRUE;
}


/*
 *  IML_IsNullMac
 *
 *  Return :
 *      TRUE -- mac[0]..[5] all is 0.
 *      FALSE-- others
 */
static BOOL_T IML_MGR_IsNullMac(UI8_T *mac)
{
    UI16_T  *twobyte = (UI16_T*)mac;

    return ((0==twobyte[0])&&(0==twobyte[1])&&(0==twobyte[2]));
}

#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
/* FUNCTION NAME : IML_MGR_IsMgmtPacket
 * PURPOSE:
 *      Check if the packet is management packet (TELNET/WEB/SNMP).
 * INPUT:
 *      payload     -- payload of frame, excluded frame header
 *      in_or_out_pkt -- IML_OUT_MGMT_PKT : Check source port, but destionation port for SNMP Trap
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
BOOL_T IML_MGR_IsMgmtPacket(void *payload, UI8_T in_or_out_pkt)
{
    IML_Ipv4PktFormat_T   *ip_pkt_p=(IML_Ipv4PktFormat_T*)payload;
    UI16_T  d_port=0, s_port=0;
    IML_Udphdr_T  *udp_header_p=NULL;
    IML_Tcphdr_T  *tcp_header_p=NULL;
    UI32_T ipHeaderLen;
    UI32_T http_port, telnet_port, ssh_port;

#if (SYS_CPNT_HTTPS == TRUE)
    UI32_T https_port;
#endif

    UI32_T snmp_port=161; /*current SNMP do not provide API, we set it with hard code*/
    SNMP_MGR_TrapDestEntry_T trapEntry;


    http_port = HTTP_MGR_Get_Http_Port();

#if (SYS_CPNT_HTTPS == TRUE)
    HTTP_MGR_Get_Secure_Port(&https_port);
#endif

    TELNET_MGR_GetTnpdPort(&telnet_port);
    ssh_port = SSHD_MGR_GetSshdPort();

    ipHeaderLen = (0x0f & ip_pkt_p->ver_len) * 4;

    /* For current state, we only check UDP for SNMP & SNMP Trap packet,
     * and TCP for TELNET/WEB packet
     * Although according to IANA, these protocol are registered both on TCP and UDP
     */
    if (ip_pkt_p->protocol == IML_IPPROTO_UDP)
    {
        udp_header_p = (IML_Udphdr_T*)((UI8_T*)ip_pkt_p+ipHeaderLen);
        d_port = L_STDLIB_Ntoh16(udp_header_p->uh_dport);
        s_port = L_STDLIB_Ntoh16(udp_header_p->uh_sport);
    }
    else if (ip_pkt_p->protocol == IML_IPPROTO_TCP)
    {
        tcp_header_p = (IML_Tcphdr_T*)((UI8_T*)ip_pkt_p+ipHeaderLen);
        d_port = L_STDLIB_Ntoh16(tcp_header_p->th_dport);
        s_port = L_STDLIB_Ntoh16(tcp_header_p->th_sport);
    }

    if (in_or_out_pkt == IML_IN_MGMT_PKT)
    {
        if (ip_pkt_p->protocol == IML_IPPROTO_UDP)
        {
            if (d_port == snmp_port)
            {
                return TRUE;
            }
        }
        else if (ip_pkt_p->protocol == IML_IPPROTO_TCP)
        {
            if ((d_port == http_port)   ||
                (d_port == telnet_port) ||
                (d_port == ssh_port)    ||
                (d_port == https_port)  ||
                (d_port == snmp_port))
            {
                return TRUE;
            }
        }
    }
    else if (in_or_out_pkt == IML_OUT_MGMT_PKT)
    {
        if (ip_pkt_p->protocol == IML_IPPROTO_UDP)
        {
            if (s_port == snmp_port)
            {
                return TRUE;
            }

            /* For SNMP Trap packet, Trap port is in Dst Port, not Src Port
             */
            memset(&trapEntry, 0, sizeof(trapEntry));
            while (SNMP_MGR_GetNextTrapReceiver(&trapEntry) == SNMP_MGR_ERROR_OK)
            {
                if (d_port == trapEntry.trap_dest_port)
                {
                    return TRUE;
                }
            }
        }
        else if (ip_pkt_p->protocol == IML_IPPROTO_TCP)
        {
            if ((s_port == http_port)   ||
                (s_port == telnet_port) ||
                (s_port == ssh_port)    ||
                (s_port == https_port)  ||
                (s_port == snmp_port))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
} /* IML_MGR_IsMgmtPacket */


/* FUNCTION NAME : IML_MGR_FilterMgmtVlanPacket
 * PURPOSE:
 *      Check if the packet should be filtered. This is for management vlan.
 * INPUT:
 *      packet_type -- type in frame
 *      payload     -- payload of frame, excluded frame header
 *      ingress_vid-- ingress-vlan id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE --- The packet should be filtered
 *      FALSE ---- Leave the packet alone
 *
 * NOTES:
 *      None.
 */
static BOOL_T IML_MGR_FilterMgmtVlanPacket(UI32_T packet_type, void *payload, UI32_T ingress_vid)
{
    UI32_T  mgmt_vid=0, mgmt_vid_ifindex=0;
    UI32_T  ingress_vid_ifindex=0;
    IML_Ipv4PktFormat_T   *ip_pkt_p=(IML_Ipv4PktFormat_T*)payload;
    UI32_T  sip, dip, b_addr;
    UI32_T  rif_num;
    NETIF_MGR_RifConfig_T  rif_config;


    VLAN_OM_GetManagementVlan(&mgmt_vid);
    if (VLAN_OM_ConvertToIfindex(mgmt_vid, &mgmt_vid_ifindex) == FALSE)
    {
        return FALSE;
    }
    if (NETIF_MGR_IsActiveManagementVid(mgmt_vid_ifindex) == FALSE)
    {
        return FALSE;
    }

    if (packet_type == IPV4_FORMAT)
    {
        sip = L_STDLIB_Ntoh32(ip_pkt_p->srcIp);
        dip = L_STDLIB_Ntoh32(ip_pkt_p->dstIp);

        VLAN_OM_ConvertToIfindex(ingress_vid, &ingress_vid_ifindex);

        /*come from mgmt vlan
         */
        if (ingress_vid == mgmt_vid)
        {
            /* multicast or broadcast
             */
            if (IP_LIB_IsMulticastIp(dip)||IP_LIB_IsBroadcastIp(dip))
            {
                return FALSE;
            }

            /* subnet broadcast
             */
            if (NETIF_MGR_GetRifFromIp(dip, &rif_num) == NETIF_MGR_OK)
            {
                if (NETIF_MGR_GetRifConfig(rif_num, &rif_config) == NETIF_MGR_OK)
                {
                    if (IP_LIB_GetSubnetBroadcastIp(rif_config.ip_address, rif_config.ip_mask, &b_addr) == IP_LIB_OK)
                    {
                        if (b_addr == dip)
                        {
                            return FALSE;
                        }
                    }
                }
            }

            /* for unicast
             */
            if(NETIF_MGR_IsManagementIp(dip) == FALSE)
            {
                /* drop the unicast packet which dip is not mgmt-ip
                 */
                return TRUE;
            }
        }
        else
        {
            /* come from nornmal vlan
             */
            if(NETIF_MGR_IsManagementIp(dip) == TRUE)
            {
                return TRUE;
            }
            else
            {
                if (NETIF_MGR_GetRifFromExactIp(dip, &rif_num) == NETIF_MGR_OK)
                {
                    if (IML_MGR_IsMgmtPacket(payload, IML_IN_MGMT_PKT) == TRUE)
                    {
                        return TRUE;
                    }
                }
            }
        }
    } /* end of if (packet_type == IPV4_FORMAT) */

    return FALSE;

} /* IML_MGR_ParsePacket */

#endif /* end of #if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */


void IML_MGR_RxLanPacket(L_MM_Mref_Handle_T  *mref_handle_p,
                              UI8_T     *dst_mac,
                              UI8_T     *src_mac,
                              UI16_T    tag_info,
                              UI16_T    type,
                              UI32_T    pkt_length,
                              UI32_T    unit_no,
                              UI32_T    port_no)
{
    UI32_T          lport = 0;
    L_MM_Mref_Handle_T*  mref_stack = NULL;

#if (SYS_CPNT_L2MCAST == TRUE)
    IML_Ipv4PktFormat_T *iphdr;
    IML_Udphdr_T        *udphdr;
    void *src_pkt_p = NULL;
    void *dst_pkt_p = NULL;
    UI32_T pdu_len;
    int handle_type = 0;
    UI32_T size_before_pdu, length;

#if (SYS_CPNT_STACKING == TRUE)
    UI16_T iuc_ethernet_header_len = 0;
    UI16_T isc_header_len = 0;
    UI16_T stacking_header_len = 0;
    UI16_T ethernet_header_len = 0;
#endif /* SYS_CPNT_STACKING */
#endif /* SYS_CPNT_L2MCAST */

    iml_counter.rx_from_lower_layer_cnt++;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* IML not in master mode, discard the packet.
         */
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* Check port status.
     * If port operation status is down, discard this received packet and
     * do not dispatch it to upper layer component.
     */
    if ( IML_MGR_GetPortOperStatus(unit_no, port_no, &lport) == FALSE )
    {
         L_MM_Mref_Release(&mref_handle_p);
         return;
    }

#if (SYS_CPNT_L2MCAST == TRUE)
    if((src_pkt_p=L_MM_Mref_GetPdu(mref_handle_p, &pdu_len))==NULL)
    {
        SYSFUN_Debug_Printf("%s[%d] : L_MM_Mref_GetPdu fail\n", __FUNCTION__, __LINE__);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* Check DA to see if this packet is for IGMPSNP(multicast)
     */
    if (0 == memcmp(dst_mac, (char[]) {0x01,0x00,0x5e}, 3))
    {
        iphdr = (IML_Ipv4PktFormat_T *)src_pkt_p;
        if ((iphdr->protocol == IML_IPPROTO_IGMP) ||
            (iphdr->protocol == IML_IPPROTO_PIM))
        {
            /*
             * For IGMP, MRD, DVMRP and PIM packet: if IPMC is disabled, we
             * don't need send the packet to TCP/IP Stack.
             */
            handle_type = IML_IP_HANDLE_TYPE_IGMPSNP;
        }
        else if ((L_STDLIB_Ntoh32(iphdr->dstIp) & 0xffffff00) == 0xe0000000)
        {
#if (SYS_CPNT_IGMPSNP_RESERVE_ADDRESS_PACKET_CHIP_TRAP_TO_CPU_BUT_NOT_FORWARD == TRUE)
            UI32_T classified_packet_type, extension_header_len;
            UI8_T *payload;

            payload = L_MM_Mref_GetPdu(mref_handle_p, &pkt_length);
            if(FALSE == IML_MGR_PacketClassification(payload,
                                         pkt_length,
                                         type,
                                         (tag_info & 0x0fff),
                                         lport,
                                         dst_mac,
                                         &classified_packet_type,
                                         &extension_header_len))
            {
                L_MM_Mref_Release(&mref_handle_p);
                return;
            }

            if((iphdr->protocol == IML_IPPROTO_UDP)
                && (classified_packet_type == IML_CLASSIFIED_OTHER))
                handle_type = IML_IP_HANDLE_TYPE_IGMPSNP;
            else
#endif
            /*
             * For known multicast packet, e.g., OSPF, RIP and so on, IGMPSNP
             * doesn't care it.
             */
            handle_type = IML_IP_HANDLE_TYPE_TCPIPSTK;
        }
        else
        {
            /*
             * For IGMP, MRD, DVMRP and PIM packet: if IPMC is disabled, we
             * don't need send the packet to TCP/IP Stack.
             */
            handle_type = IML_IP_HANDLE_TYPE_IGMPSNP;
        }
        if(mref_handle_p->pkt_info.pkt_is_truncated)
        {
           iphdr = (IML_Ipv4PktFormat_T *)src_pkt_p;

           if (iphdr->protocol == IML_IPPROTO_UDP)
           {
             udphdr = (IML_Udphdr_T *)((UI8_T *)src_pkt_p+ ((iphdr->ver_len&0x0f)*4));
             udphdr->uh_sum  = 0;
             udphdr->uh_ulen = L_STDLIB_Hton16(pdu_len - ((iphdr->ver_len&0x1f)*4));
             iphdr->checksum = 0;
             iphdr->length   = L_STDLIB_Hton16(pdu_len);
             iphdr->checksum = L_MATH_CheckSum16((UI16_T *)iphdr, ((iphdr->ver_len&0x0f)*4));
           }
        }
    }
    else
    {
        /* Unicast Stream */
        handle_type = IML_IP_HANDLE_TYPE_TCPIPSTK;
    }

    if (handle_type == IML_IP_HANDLE_TYPE_IGMPSNP)
    {
        /* Call IGMPSNP callback function
         */
        if(FALSE == SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback(SYS_MODULE_IML,
                                                                mref_handle_p,
                                                                dst_mac,
                                                                src_mac,
                                                                tag_info,
                                                                type,
                                                                pkt_length,
                                                                lport))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }
    else if (handle_type == IML_IP_HANDLE_TYPE_BOTH)
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

        /* create a new MREF for TCP/IP stack. */
        mref_stack = L_MM_AllocateTxBufferForPktForwarding(mref_handle_p, pdu_len-32+18, L_MM_USER_ID2(SYS_MODULE_IML, 0));
        if (NULL == mref_stack)
        {
            L_MM_Mref_Release(&mref_handle_p);
            SYSFUN_Debug_Printf("%s[%d] : L_MM_AllocateTxBufferForPktForwarding fail\n",
                        __FUNCTION__, __LINE__);
            return;
        }

        /* Include ISC header if neccesary */
        if (NULL == L_MM_Mref_MovePdu(mref_handle_p, (0 - size_before_pdu), &length))
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MM_Mref_Release(&mref_stack);
            return;
        }

        if (NULL == L_MM_Mref_MovePdu(mref_stack, (0 - 32), &length))
        {
            L_MM_Mref_Release(&mref_handle_p);
            L_MM_Mref_Release(&mref_stack);
            return;
        }

        dst_pkt_p = L_MM_Mref_GetPdu(mref_stack, &length);
        src_pkt_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        memcpy(dst_pkt_p, src_pkt_p, length);

        /* Exclude ISC header if neccessary */
        L_MM_Mref_MovePdu(mref_handle_p, size_before_pdu, &pdu_len);
        if (FALSE == SYS_CALLBACK_MGR_ReceiveIgmpsnpPacketCallback(SYS_MODULE_IML,
                                                       mref_handle_p,
                                                       dst_mac,
                                                       src_mac,
                                                       tag_info,
                                                       type,
                                                       pkt_length,
                                                       lport))
        {
            L_MM_Mref_Release(&mref_handle_p);
        }

        /* Exclude ISC header if neccessary */
        L_MM_Mref_MovePdu(mref_stack, 18, &length);

        IML_MGR_ProcessRxPacket(mref_stack, dst_mac, src_mac, tag_info, type,
                                pkt_length, lport);
    }
    else
    {
        IML_MGR_ProcessRxPacket(mref_handle_p, dst_mac, src_mac, tag_info, type,
                                pkt_length, lport);
    }
#else
    IML_MGR_ProcessRxPacket(mref_handle_p, dst_mac, src_mac, tag_info, type,
                            pkt_length, lport);
#endif /* SYS_CPNT_L2MCAST */

    return;
}

/* FUNCTION NAME: IML_MGR_GetPortOperStatus
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
static BOOL_T IML_MGR_GetPortOperStatus(UI32_T unit_no, UI32_T port_no, UI32_T *lport)
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

}


static void IML_MGR_ProcessRxPacket(L_MM_Mref_Handle_T *mref_handle_p,
                                       UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI8_T     src_mac[SYS_ADPT_MAC_ADDR_LEN],
                                       UI16_T    tag_info,
                                       UI16_T    ether_type,
                                       UI32_T    packet_length,
                                       UI32_T    l_port)
{
    VLAN_OM_Dot1qPortVlanEntry_T port_vlan_entry;
    UI8_T           vlan_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T           hsrp_mac[5] = {0x00, 0x00, 0x0C, 0x07, 0xAC};
    UI8_T           vrrp_mac[5] = {0x00, 0x00, 0x5E, 0x00, 0x01};
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
    UI8_T           accton_virtual_mac[5] =
    {
    VIRTUAL_MAC_BYTE0,
    VIRTUAL_MAC_BYTE1,
    VIRTUAL_MAC_BYTE2,
    VIRTUAL_MAC_BYTE3,
    VIRTUAL_MAC_BYTE4,
    };
#endif
    UI32_T          ingress_vid_ifindex;
    UI32_T          ingress_vid;
    UI32_T          classified_packet_type;
    UI32_T          extension_header_len;
    UI32_T          payload_len;
    UI16_T          checksum = 0;
    void            *payload = NULL;
    BOOL_T          is_drop = FALSE;

    /* find ingress vid
     */
    ingress_vid = tag_info & 0x0fff; /* 12 bit vid */
    if (0 == ingress_vid)
    {
        /* Use PVID as the interface
         */
#if 0
        if (VLAN_PMGR_GetDot1qPortVlanEntry (l_port ,&port_vlan_entry) == FALSE)
#else
    if (VLAN_POM_GetDot1qPortVlanEntry (l_port ,&port_vlan_entry) == FALSE)
#endif
        {
            L_MM_Mref_Release(&mref_handle_p);
            iml_counter.drop_rx_cnt++;
            return;
        }
        else
        {
            ingress_vid = port_vlan_entry.dot1q_pvid_index;
        }
    }

    VLAN_OM_ConvertToIfindex(ingress_vid, &ingress_vid_ifindex);

    payload = L_MM_Mref_GetPdu(mref_handle_p, &payload_len);
    if (NULL == payload)
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    STKTPLG_POM_GetLocalUnitBaseMac(vlan_mac);

#if (SYS_CPNT_WEBAUTH == TRUE)
    if (FALSE == IML_MGR_DoWebAuthChecking(
                    l_port, ingress_vid_ifindex, mref_handle_p,
                    payload, dst_mac, vlan_mac))
#endif
    {
        /* If the packet is unicast and the dst_mac is not VRRP or HSRP or Accton virtual MAC and not equal to vlan mac
         * (device mac). It should not go toward CPU, so discard it.
         * The reason why processing here is to do before entering queuing process.
         */
        if ((dst_mac[0] & 0x01)!=0x01)
        {
            is_drop = TRUE;

            if (memcmp(dst_mac, hsrp_mac, 5) == 0)
            {
                is_drop = FALSE;
            }
            else if (memcmp(dst_mac, vrrp_mac, 5) == 0)
            {
                is_drop = FALSE;
            }
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
            else if (memcmp(dst_mac, accton_virtual_mac, 5) == 0)
            {
                is_drop = FALSE;
            }
#endif
            else if (memcmp(vlan_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN) == 0)
            {
                is_drop = FALSE;
            }
#if (SYS_CPNT_DHCPSNP == TRUE)
            else if (TRUE == IML_MGR_IsDhcpPacket(mref_handle_p, ether_type))
            {
                is_drop = FALSE;
            }
#endif
#if (SYS_CPNT_DHCPV6SNP == TRUE)
            else if(TRUE == IML_MGR_IsDhcp6Packet(mref_handle_p, ether_type))
            {
                is_drop = FALSE;
            }
#endif
#if (SYS_CPNT_DAI == TRUE)
            else if (ether_type == ARP_FORMAT)
            {
                UI8_T status;

                if (  (DAI_TYPE_OK == DAI_OM_GetGlobalDaiStatus(&status))
                    &&(DAI_TYPE_GLOBAL_ENABLED == status)
                   )
                {
                    is_drop = FALSE;
                }
            }
#endif
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
            else if (ether_type == IPV6_FORMAT)
            {
                /* need to do softway realy
                 */
                is_drop = FALSE;
            }
#endif
        } /* if ((dst_mac[0] & 0x01)!=0x01) */
    } /* if (FALSE == IML_MGR_DoWebAuthChecking(...) */

    if (TRUE == is_drop)
    {
        L_MM_Mref_Release(&mref_handle_p);
        iml_counter.drop_rx_cnt ++;
        iml_counter.drop_rx_unicast_da_invalid_cnt ++;
        return;
    }

    if (ether_type == IPV4_FORMAT)
    {
        iml_counter.rx_ip_cnt++;
    }
    else if (ether_type == ARP_FORMAT)
    {
        iml_counter.rx_arp_cnt++;
    }

    /* Classify packet type
     */
    if (IML_MGR_PacketClassification(payload,
                                     payload_len,
                                     ether_type,
                                     ingress_vid,
                                     l_port,
                                     dst_mac,
                                     &classified_packet_type,
                                     &extension_header_len) == FALSE)
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

#if (SYS_CPNT_VRRP == TRUE)
    if(IML_MGR_IsVirtualMac(dst_mac) || IML_MGR_IsAcctonVirtualMac(dst_mac))
    {
        if(!IML_MGR_ProcessRxVirtualMacAddressPacket(
                mref_handle_p,
                payload,
                ingress_vid,
                dst_mac,
                vlan_mac,
                ether_type))
        {
            iml_counter.drop_rx_cnt++;
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }
#endif

/* already checked in L2MUX_GROUP_L2muxReceivePacketCallbackHandler
 */
#if 0
#if (SYS_CPNT_IPV6_RA_GUARD == TRUE)
    if (TRUE == IML_MGR_CheckRaGuard(l_port, classified_packet_type))
    {
        iml_counter.drop_rx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
#endif /* #if (SYS_CPNT_IPV6_RA_GUARD == TRUE) */
#endif

    IML_MGR_PrintPacketType(classified_packet_type);

    /* iml add check source MAC :
     * if it is one of my router MAC then (a) no mac cache learning
     *                                    (b) workaround TTL
     */
    if ( (ether_type == IPV4_FORMAT) &&
         (memcmp(vlan_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN) == 0))
    {
        /* original formula
         *
         * unsigned long sum;
         * ipptr->ttl--;
         * sum = ipptr->Checksum + 0x100;
         * ipptr->Checksum = (sum + (sum>>16))
         */
        IML_Ipv4PktFormat_T *ipptr = (IML_Ipv4PktFormat_T*)payload;

        ipptr->ttl++; /* increment ttl */
        checksum = L_STDLIB_Ntoh16(ipptr->checksum) - 0x100;   /* decrement checksum high byte*/
        if ((checksum & 0xff00) == 0xff0)
            checksum--;
            //ipptr->checksum--;  /* reduce carry */

        ipptr->checksum = L_STDLIB_Hton16(checksum);
    }

    IML_MGR_PacketDispatch(mref_handle_p,
                            dst_mac,
                            src_mac,
                            ingress_vid,
                            ether_type,
                            packet_length,
                            l_port,
                            classified_packet_type,
                            extension_header_len,
                            NULL);

    return;
}


/*--------------------------------------------------------------------------
 *  FUNCTION NAME : IML_MGR_SendPktToTCPIPStack
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *      This function will send packet in format of mref to linux kernel tcp/ip
 *      stack.
 *
 *  INPUT:
 *   mref_handle_offset -- offset of mref_handle_p for L_IPCMEM to get pointer
 *   tag_info           -- vlan tag info of the incoming packet which is contained
 *                         in mref
 *   is_wait_finish     -- whether need to wait for finish processing packet before return
 *   process_packet_type -- indicate how packet is processed
 *
 *  OUTPUT:
 *   None.
 *
 *  RETURN:
 *   IML_TYPE_RETVAL_OK                    --  Success
 *   IML_TYPE_RETVAL_NET_DEVICE_NOT_EXIST  --  The net device of the correspoding vlan
 *                                             from where the packet comes cannot be found
 *   IML_TYPE_RETVAL_MREF_ERROR            --  An error occurs when operates on mref handle
 *   IML_TYPE_RETVAL_ALLOC_SKB_FAIL        --  Fail to allocate socket buffer
 *
 *  NOTES:
 *   Pdu of the received mref_handle must point to ethernet header.
 *--------------------------------------------------------------------------
 */
static UI32_T IML_MGR_SendPktToTCPIPStack(UI32_T mref_handle_offset, UI16_T tag_info, UI32_T process_packet_type)
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI8_T*              pkt_buf_p;
    UI32_T              pdu_len;
    BOOL_T              is_tagged_packet;

    mref_handle_p = L_IPCMEM_GetPtr(mref_handle_offset);
    pkt_buf_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);


    /* Might need to consider QinQ case here.
     * Only consider single tag now.
     */
    if(pkt_buf_p[12]==0x81 && pkt_buf_p[13]==0x0)
        is_tagged_packet=TRUE;
    else
        is_tagged_packet=FALSE;

    return SYSFUN_Syscall(SYSFUN_SYSCALL_IML_MGR,
                          L_CVRT_UINT_TO_PTR(IML_TYPE_SYSCALL_CMD_SEND_PKT_TO_TCP_IP_STACK),
                          L_CVRT_UINT_TO_PTR(mref_handle_offset),
                          L_CVRT_UINT_TO_PTR(tag_info),
                          L_CVRT_UINT_TO_PTR(is_tagged_packet),
                          L_CVRT_UINT_TO_PTR(process_packet_type));
}

/*--------------------------------------------------------------------------
 *  FUNCTION NAME : IML_MGR_RecvPktFromTCPIPStack
 *--------------------------------------------------------------------------
 *  PURPOSE:
 *      This function will receive the packets coming from linux kernel tcp/ip
 *      stack.
 *
 *  INPUT:
 *   None.
 *
 *  OUTPUT:
 *   mref_handle_offset_p  -- offset of mref_handle_p for L_IPCMEM to get pointer
 *   args_p->tag_info      -- the vlan tag info for the outgoing ip packet
 *   args_p->packet_type   -- the packet type of the outgoing ip packet
 *   args_p->dst_mac       -- destination mac addr
 *   args_p->src_mac       -- source mac addr
 *
 *
 *  RETURN:
 *   IML_TYPE_RETVAL_OK                    --  Success
 *   IML_TYPE_RETVAL_INVALID_ARG           --  Invalid input arguments
 *                                             from where the packet comes cannot be found
 *   IML_TYPE_RETVAL_INTR                  --  The operation is interrupted by a signal
 *   IML_TYPE_RETVAL_MREF_ERROR            --  An error occurs when operates on mref handle
 *
 *  NOTES:
 *   Pdu of the output mref_handle will point to the ip header.
 *--------------------------------------------------------------------------
 */

static UI32_T IML_MGR_RecvPktFromTCPIPStack(UI32_T *mref_handle_offset_p, IML_MGR_RecvPktFromTCPIPStackArgs_T *args_p)
{
    if(mref_handle_offset_p==NULL || args_p==NULL)
        return IML_TYPE_RETVAL_INVALID_ARG;

    return SYSFUN_Syscall(SYSFUN_SYSCALL_IML_MGR,
                          L_CVRT_UINT_TO_PTR(IML_TYPE_SYSCALL_CMD_RECV_PKT_FROM_TCP_IP_STACK),
                          L_CVRT_UINT_TO_PTR(mref_handle_offset_p),
                          L_CVRT_UINT_TO_PTR(args_p),
                          NULL, NULL);

}

static void IML_MGR_PrintPacketType(UI32_T packet_type)
{
    if (show_incoming_packet_type == TRUE)
    {
        switch (packet_type)
        {
            case IML_CLASSIFIED_OTHER:
                BACKDOOR_MGR_Printf("\n**Other**");
                break;
            case IML_CLASSIFIED_ARP_REQUEST:
                BACKDOOR_MGR_Printf("\n**Arp Request**");
                break;
            case IML_CLASSIFIED_IP_MGMT:
                BACKDOOR_MGR_Printf("\n**IP MGMT**");
                break;
            case IML_CLASSIFIED_HSRP:
                BACKDOOR_MGR_Printf("\n**HSRP**");
                break;
            case IML_CLASSIFIED_VRRP:
                BACKDOOR_MGR_Printf("\n**VRRP**");
                break;
            case IML_CLASSIFIED_OSPF:
                BACKDOOR_MGR_Printf("\n**OSPF**");
                break;
            case IML_CLASSIFIED_PIM:
                BACKDOOR_MGR_Printf("\n**PIM**");
                break;
            case IML_CLASSIFIED_IGMP:
                BACKDOOR_MGR_Printf("\n**IGMP**");
                break;
            case IML_CLASSIFIED_RIP:
                BACKDOOR_MGR_Printf("\n**RIP**");
                break;
            case IML_CLASSIFIED_ARP_REPLY:
                BACKDOOR_MGR_Printf("\n**ARP Reply**");
                break;
            case IML_CLASSIFIED_BOOTP_CLIENT:
                BACKDOOR_MGR_Printf("\n**BOOTP Client**");
                break;
            case IML_CLASSIFIED_BOOTP_SERVER:
                BACKDOOR_MGR_Printf("\n**BOOTP Server**");
                break;
            case IML_CLASSIFIED_UDPHELPER:
                BACKDOOR_MGR_Printf("\n**Udphelper**");
                break;
            case IML_CLASSIFIED_TFTP:
                BACKDOOR_MGR_Printf("\n**TFTP**");
                break;
            case IML_CLASSIFIED_DHCP_V6:
                BACKDOOR_MGR_Printf("\n**DHCPv6**");
                break;
            case IML_CLASSIFIED_MCAST_V6_DATA:
                BACKDOOR_MGR_Printf("\n**IPv6 Multicast data**");
                break;
            case IML_CLASSIFIED_ENCAP_V6:
                BACKDOOR_MGR_Printf("\n**IPv6-in-IPv4 tunnelling**");
                break;
            case IML_CLASSIFIED_ICMP_ND_NEIGH_SOL:
                BACKDOOR_MGR_Printf("\n**IPv6 ND Neighbor Solicitation**");
                break;
            case IML_CLASSIFIED_ICMP_ND_NEIGH_ADVT:
                BACKDOOR_MGR_Printf("\n**IPv6 ND Neighbor Advertisement**");
                break;
            case IML_CLASSIFIED_ICMP_ND_ROUTER_SOL:
                BACKDOOR_MGR_Printf("\n**IPv6 ND Router Solicitation**");
                break;
            case IML_CLASSIFIED_ICMP_ND_ROUTER_ADVT:
                BACKDOOR_MGR_Printf("\n**IPv6 ND Router Advertisement**");
                break;
            case IML_CLASSIFIED_ICMP_ND_REDIRECT:
                BACKDOOR_MGR_Printf("\n**IPv6 ND Redirect**");
                break;
            default:
                BACKDOOR_MGR_Printf("\n **Error **= %lu", packet_type);
                break;
        }
    }
}

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
    UI16_T packet_type,UI32_T src_lport_ifIndex)
{
    UI32_T  cos_value=0;
    BOOL_T  is_exist = FALSE;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    UI32_T  out_lport =0;
    BOOL_T  is_tagged;


    if (NULL == mref_handle_p)
    {
        iml_counter.drop_tx_cnt++;
        return  (-1);
    }

    if (IML_MGR_IsNullMac(dst_mac)|| IML_MGR_IsNullMac(src_mac))
    {
        iml_counter.drop_tx_null_mac_cnt++;
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
        return (-1);
    }

    /*  Record IML use the MREF */

     mref_handle_p->current_usr_id = SYS_MODULE_IML;



    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry) == FALSE)
    {
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release (&mref_handle_p);
        return (-1);
    }

    /*
     *  Depend on unicast/multicast/broadcast, do something :
     *      Unicast : verify the egress vlan & port is correct.
     *      B'cast/M'cast : send to all the ports.
     */
    {
        if ((dst_mac[0] & 0x01)!=0x01)
        {   /*  unicast */

            /*  Find out the (lport, ip, rif) had leart */
            AMTR_TYPE_AddrEntry_T addr_entry;

            memcpy(addr_entry.mac, dst_mac, 6);
            addr_entry.vid  = vid;

            if (AMTR_MGR_GetExactAddrEntryFromChip(&addr_entry)==TRUE)
            {


                is_exist = TRUE;
                out_lport = addr_entry.ifindex;
            }

#ifdef BACKDOOR_OPEN
            if (debug_enable)
            {
                printf (" Tx-->lport=%d ", (int)out_lport);
            }
#endif
            /* suger, 03-11-2004, add checking of (out_lport == SYS_ADPT_STACKING_PORT).
             * Otherwise, if out_lport is stacking port, the packet will send by multicast.
             */
            if ((is_exist == TRUE)&&((out_lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT)||(out_lport == SYS_ADPT_STACKING_PORT)))
            {
                /* if output port is the same as source port, drop it */
                if(out_lport == src_lport_ifIndex)
                {
                    iml_counter.drop_tx_cnt++;
                    L_MM_Mref_Release (&mref_handle_p);
                    return (-1);
                }

                /*  the mac had learned.    */
                if (SYS_ADPT_STACKING_PORT != out_lport)
                {

                    /*  1. Verify the port in port list of vlan */
                    if (vlan_entry.dot1q_vlan_current_egress_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
                    {
                        /*  2. Check the port is tagged or untagged ?   */
                        if (vlan_entry.dot1q_vlan_current_untagged_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
                            is_tagged = FALSE;
                        else
                            is_tagged = TRUE;


                        /*  4. Send out the packet  */
                        iml_counter.tx_arp_cnt++;

                        iml_counter.tx_cnt++;
                        mref_handle_p->next_usr_id = SYS_MODULE_L2MUX;

                        L2MUX_MGR_SendPacket (mref_handle_p, dst_mac, src_mac, packet_type, (UI16_T) vid, packet_length,
                            out_lport, is_tagged, cos_value,FALSE);
                    }
                    else
                    {
                        /* Jason Chen, 12-12-2000. not in the same subnet -> discard it */
                        iml_counter.drop_tx_cnt++;
                        L_MM_Mref_Release(&mref_handle_p);
                    }
                    return (0);
                }
                else
                {   /* special handle stacking port */

                    /*  4. Send out the packet  */
#ifdef BACKDOOR_OPEN
                    if (debug_enable)
                    {
                        printf ("\n (Tx:vid=%d, lport=%d), ", (int)vid, (int)out_lport);
                    }
#endif

                    mref_handle_p->next_usr_id = SYS_MODULE_L2MUX;

                    L2MUX_MGR_SendPacket (mref_handle_p, dst_mac, src_mac, packet_type, (UI16_T) vid, packet_length,
                        out_lport, TRUE, cos_value,FALSE);
                    return (0);
                }
            }
        }
        /*  broadcast, m'cast   */
        {
            UI8_T  lport_position_mask = 0;
            UI32_T lport_row_num = 0;
            AMTR_TYPE_AddrEntry_T addr_entry;




            memcpy(addr_entry.mac, src_mac, 6);
            addr_entry.vid  = vid;


            lport_position_mask = (1 << (7 - ((src_lport_ifIndex - 1) & 7)));
            lport_row_num       = (src_lport_ifIndex - 1) >> 3;
            vlan_entry.dot1q_vlan_current_egress_ports[lport_row_num] &= ~lport_position_mask;


            mref_handle_p->next_usr_id = SYS_MODULE_L2MUX;

            L2MUX_MGR_SendMultiPacket (mref_handle_p, dst_mac, src_mac, packet_type, (UI16_T)vid, packet_length,
                 vlan_entry.dot1q_vlan_current_egress_ports, vlan_entry.dot1q_vlan_current_untagged_ports,
                 cos_value);


#ifdef BACKDOOR_OPEN
    if (debug_enable)
    {
        printf ("\n (Tx: multi-dest), ");
    }
#endif

            iml_counter.tx_bcast_cnt++;
            iml_counter.tx_cnt++;
        }   /*  end of broadcast    */
    }   /*  end of Step4    */

    return (0);

}   /*  end of IML_SendPkt  */

#endif

#if(SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* FUNCTION NAME: IML_MGR_BroadcastBootPPkt_ExceptInport
 * PURPOSE: Send broadcast packet
 * INPUT:  *mem_ref_handle_p -- L_MREF descriptor
 *         vid_ifIndex       -- vlan ifIndex.
 *         packet_length     -- packet length
 *         *dst_mac_p        -- destination mac
 *         *src_mac_p        -- source mac
 *         src_lport_ifindex -- broadcast packet except this port
 * OUTPUT: none
 * RETURN: successful (0), failed (-1)
 * NOTES:
 */
static UI32_T IML_MGR_BroadcastBootPPkt_ExceptInport(L_MM_Mref_Handle_T *mref_handle_p, UI32_T vid_ifindex,
    UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,UI32_T src_lport_ifindex)
{
    /*  LOCAL CONSTANT DECLARATION
     */
    #define PKT_TYPE_IP 0x0800
    /*  LOCAL VARIABLE DECLARATION
     */
    /*L_MM_Mref_Handle_T  *tmp_mem_ref;*/
    UI32_T  vid;
    UI16_T  packet_type=PKT_TYPE_IP;
    UI32_T  cos_value=0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;


    /*  Verfiy the parameter is valid.  */
    if (mref_handle_p == NULL)
    {
        iml_counter.drop_tx_cnt++;
#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because null mref");
        }
#endif
        return  (-1);
    }

    if ((dst_mac_p == NULL)||(src_mac_p == NULL))
    {
        iml_counter.drop_tx_null_mac_cnt++;
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because null dst/src mac pointer");
        }
#endif
        return (-1);
    }


    /* L2 snooping protocol check here */
    if(IML_MGR_TxSnoopPacket_Callback_Func(
            mref_handle_p,
            dst_mac_p,
            src_mac_p,
            packet_length,
            vid_ifindex,
            0,
            src_lport_ifindex,
            packet_type))
    {
        return (0);
    }


#ifdef  L_MREF_DEBUG_ID
    mref_handle_p->current_usr_id = SYS_MODULE_IML;
#endif

    /*  2001.12.25, William,
     *  Add tag to packet (0x8100, 0x0001)/(ETHER_TAG_TYPE, VLAN-1)
     */
    if (L_MM_Mref_SetPduLen(mref_handle_p, packet_length) == FALSE)
    {
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release (&mref_handle_p);
        return (-1);
    }

    /*  Find out vlan information   */
    VLAN_IFINDEX_CONVERTTO_VID(vid_ifindex, vid);
    memset(&vlan_entry, 0 , sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry) == FALSE)
    {
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release (&mref_handle_p);
        return (-1);
    }

#ifdef  L_MREF_DEBUG_ID
    mref_handle_p->next_usr_id = SYS_MODULE_L2MUX;
#endif


    vlan_entry.dot1q_vlan_current_egress_ports[(src_lport_ifindex-1)/8]=
    vlan_entry.dot1q_vlan_current_egress_ports[(src_lport_ifindex-1)/8] & (~(0x80>>((src_lport_ifindex-1)%8)));

    /* 2009-02-04 repalced by recursively send packet to ports */
    L2MUX_MGR_SendMultiPacket (mref_handle_p, dst_mac_p, src_mac_p, packet_type, (UI16_T)vid, packet_length,
         vlan_entry.dot1q_vlan_current_egress_ports, vlan_entry.dot1q_vlan_current_untagged_ports,cos_value);
#if 0
    /* 2009-02-04 Jimi, recursively send packet to resolve the problem that can't send multicast packets
     *            to ports on the same chip
     */
    for(tmp_lport=1;tmp_lport<=SYS_ADPT_TOTAL_NBR_OF_LPORT;tmp_lport++)
    {

        if(vlan_entry.dot1q_vlan_current_egress_ports[(tmp_lport-1)/8] & (0x80>>((tmp_lport-1)%8)))
        {

            /* Check the port is tagged or untagged ? */
            if (vlan_entry.dot1q_vlan_current_untagged_ports[(tmp_lport-1)/8] & (0x80>>((tmp_lport-1)%8)))
            {
                is_tagged = FALSE;
            }
            else
            {
                is_tagged = TRUE;
            }

            tmp_mem_ref = L_MREF_CloneReference(mem_ref);
            L_MREF_SetMyId(tmp_mem_ref, SYS_MODULE_IML, 0);
            if ( tmp_mem_ref != NULL)
            {
                L2MUX_MGR_SendPacket (tmp_mem_ref, dst_mac, src_mac, packet_type, (UI16_T) vid, packet_length,
                tmp_lport, is_tagged, cos_value,FALSE);
            }
        }

    }
#endif


    iml_counter.tx_bcast_cnt++;
    iml_counter.tx_cnt++;
    return (0);

}   /*  end of IML_BroadcastBootPPkt    */


/* FUNCTION NAME: IML_SendPktToPort
 * PURPOSE: Send packet
 * INPUT:  *mref_handle_p   -- L_MREF descriptor
 *         packet_length    -- packet length
 *         *dst_mac_p       -- destination mac
 *         *src_mac_p       -- source mac
 *         vid              -- vid
 *         out_lport        -- output port
 *         packet_type      --
 * OUTPUT: None
 * RETURN: successful (0), failed (-1)
 * NOTES:  None
 */
static UI32_T IML_MGR_SendPktToPort(L_MM_Mref_Handle_T *mref_handle_p, UI32_T packet_length, UI8_T *dst_mac_p, UI8_T *src_mac_p,
    UI32_T vid, UI32_T out_lport, UI16_T packet_type)
{
    /*  LOCAL CONSTANT DECLARATION
     */
    /*  LOCAL VARIABLE DECLARATION
     */
    UI32_T  cos_value=0;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_entry;
    UI32_T  vid_ifindex=0;
    BOOL_T  is_tagged;

    /*  BODY
     */
    /* Verfiy the parameter is valid.
     */
    if (mref_handle_p == NULL)
    {
        iml_counter.drop_tx_cnt++;
#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because null mref");
        }
#endif
        return  (-1);   /*  invalid mref structure  */
    }

    if ((dst_mac_p == NULL)||(src_mac_p == NULL))
    {
        iml_counter.drop_tx_null_mac_cnt++;
        iml_counter.drop_tx_cnt++;
        L_MM_Mref_Release(&mref_handle_p);
#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because null dst/src mac pointer");
        }
#endif
        return (-1);
    }

    /*  Check is system in stacking mode ?  */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        iml_counter.drop_tx_cnt++;
#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because not in master mode");
        }
#endif
        L_MM_Mref_Release (&mref_handle_p);
        return (-1);
    }   /*  end of if   */
    /*  2002.04.10, William, for invalid Mac (0:0:0:0:0:0), discard it  */
    if (IML_MGR_IsNullMac(dst_mac_p)|| IML_MGR_IsNullMac(src_mac_p))
    {

        iml_counter.drop_tx_null_mac_cnt++;
        iml_counter.drop_tx_cnt++;

#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because of null dst/src mac");
        }
#endif
        L_MM_Mref_Release(&mref_handle_p);
        return (-1);
    }


    VLAN_VID_CONVERTTO_IFINDEX(vid, vid_ifindex);
    /* L2 snooping protocol check here */
    if(IML_MGR_TxSnoopPacket_Callback_Func(
            mref_handle_p,
            dst_mac_p,
            src_mac_p,
            packet_length,
            vid_ifindex,
            out_lport,
            0,
            packet_type))
    {
        return (0);
    }


    memset(&vlan_entry, 0, sizeof(VLAN_OM_Dot1qVlanCurrentEntry_T));
    if (VLAN_OM_GetDot1qVlanCurrentEntry(0, vid, &vlan_entry) == FALSE)
    {
        iml_counter.inner_error_cnt++;
        iml_counter.drop_tx_cnt++;

#ifdef BACKDOOR_OPEN
        if (debug_enable)
        {
            printf ("\n Tx packet drop because get vlan entry error");
        }
#endif
        L_MM_Mref_Release (&mref_handle_p);
        return (-1);
    }


    /* Send Unicast packet */
    if (0x0800 == packet_type)
    {
        iml_counter.tx_ip_cnt++;
    }

    /* Verify the port in port list of vlan */
    if (vlan_entry.dot1q_vlan_current_egress_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
    {
        /* Check the port is tagged or untagged ? */
        if (vlan_entry.dot1q_vlan_current_untagged_ports[(out_lport-1)/8] & (0x80>>((out_lport-1)%8)))
        {
            is_tagged = FALSE;
        }
        else
        {
            is_tagged = TRUE;
        }

        L2MUX_MGR_SendPacket (mref_handle_p, dst_mac_p, src_mac_p, packet_type, (UI16_T) vid, packet_length,
            out_lport, is_tagged, cos_value,FALSE);
    }

    return (0);

}/* end of IML_SendPktToPort */
#endif


#if (SYS_CPNT_IP_TUNNEL == TRUE)
/* FUNCTION NAME: IML_MGR_SetDynamicTunnelRoute
 * PURPOSE: set dynamic tunnel route to amtrl3 and chip
 * INPUT:  vid_ifindex   -- sent out vlan ifindex
 *         packet_type   -- send out packet type
 *         payload       -- send out packet's payload
 * OUTPUT: none
 * RETURN: TRUE / FALSE
 * NOTES:  This api is for tunnel to set dynamic host route and net route,
 *         we receive kernel's routing and encapsulated packet to get enough information to
 *         set route to amtrl3 and chip
 */
static BOOL_T IML_MGR_SetDynamicTunnelRoute(UI8_T   *payload, UI32_T vid_ifindex, UI16_T packet_type)
{
    IML_Ipv4PktFormat_T   *ip_pkt_p=(IML_Ipv4PktFormat_T*)payload;
    IML_Ipv6PktFormat_T   *ipv6_pkt_p=(IML_Ipv6PktFormat_T*)(payload + sizeof(IML_Ipv4PktFormat_T));
    NETCFG_TYPE_L3_Interface_T tunnel_l3_intf;
    AMTRL3_TYPE_InetCidrRouteEntry_T    net_entry;
    AMTRL3_TYPE_InetHostRouteEntry_T    host_entry;
    AMTRL3_OM_NetRouteEntry_T  net_om_entry;
    L_INET_AddrIp_T            masked_host;
    UI8_T   i = 0;

    /* check if 6to4 or ISATAP,
     * 6to4 only needs to write net route to chip
     * ISATAP needs to write host route to chip */



    if((packet_type == IPV4_FORMAT)&&
       (ip_pkt_p->protocol == IML_IPPROTO_IPV6))
    {

        memset(&tunnel_l3_intf, 0, sizeof(tunnel_l3_intf));
        /* If there's two tunnel interface have the same source vlan , it would be a problem */
        if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_GetTunnelIfindexBySrcVidIfindex(vid_ifindex, &tunnel_l3_intf))
        {

            return FALSE;
        }
        if(debug_enable)
        {
            printf("%s[%d],tunnel ifindex = %lu, tunnel mode = %u\r\n",__FUNCTION__,__LINE__,
                tunnel_l3_intf.ifindex, tunnel_l3_intf.u.tunnel_intf.tunnel_mode);
        }

        /* check tunnel mode */
        switch(tunnel_l3_intf.u.tunnel_intf.tunnel_mode)
        {
            case NETCFG_TYPE_TUNNEL_MODE_6TO4:
            {
                if(debug_enable)
                {
                    printf("6to4 tunnel \r\n");
                    printf("%s[%d]: set 6to4 tunnel route\r\n",__FUNCTION__,__LINE__);
                    printf("tunnel ifindex = %lu\r\n",tunnel_l3_intf.ifindex);
                    printf("IPv4 Header:sip = %u.%u.%u.%u, dip = %u.%u.%u.%u\r\n",
                        L_INET_EXPAND_IPV4(&ip_pkt_p->srcIp),L_INET_EXPAND_IPV4(&ip_pkt_p->dstIp));
                    printf("IPv6 Header:sip = %lx:%lx:%lx:%lx, dip = %lx:%lx:%lx:%lx\r\n",
                        L_INET_EXPAND_IPV6(ipv6_pkt_p->src_addr),L_INET_EXPAND_IPV6(ipv6_pkt_p->dst_addr));
                }

                /* add 2002:XXXX:XXXX::/16 net route to chip */
                /* we should set to netcfg_mgr_ip to handle route change, not amtrl3
                 * we need set host route and net route */

                /* set special host route that create initiator and terminator */
                memset(&host_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
                host_entry.tunnel_entry_type = AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4;
                host_entry.dst_vid_ifindex = tunnel_l3_intf.ifindex;
                /* set host destination 2002:wwxx:yyzz::/64 */
                memcpy(host_entry.dst_inet_addr.addr,ipv6_pkt_p->dst_addr,(SYS_ADPT_TUNNEL_6to4_PREFIX_LEN/8));
                host_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
                host_entry.dst_inet_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                host_entry.dst_inet_addr.preflen = SYS_ADPT_TUNNEL_6to4_PREFIX_LEN;
                host_entry.lport = SYS_ADPT_DESTINATION_PORT_UNKNOWN;
                memset(host_entry.dst_mac, 0,SYS_ADPT_MAC_ADDR_LEN );
                host_entry.trunk_id=0;

                /* set source ipv4 address and source vlan interface */
                host_entry.u.ip_tunnel.src_vidifindex = vid_ifindex;
                memcpy(host_entry.u.ip_tunnel.src_inet_addr.addr, &(ip_pkt_p->srcIp), SYS_ADPT_IPV4_ADDR_LEN);
                host_entry.u.ip_tunnel.src_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
                host_entry.u.ip_tunnel.src_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                /* host_entry.u.ip_tunnel.src_inet_addr.preflen  should it need ?*/
                /* set destination ip */
                memcpy(host_entry.u.ip_tunnel.dest_inet_addr.addr, &(ip_pkt_p->dstIp),SYS_ADPT_IPV4_ADDR_LEN);
                host_entry.u.ip_tunnel.dest_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
                host_entry.u.ip_tunnel.dest_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                /* host_entry.u.ip_tunnel.dest_inet_addr.preflen  should it need ?*/
                /* we should find out nexthop address, if it can't find,
                 * we use destination ip as nexthop
                 */
                host_entry.u.ip_tunnel.nexthop_inet_addr = host_entry.u.ip_tunnel.dest_inet_addr;
                host_entry.u.ip_tunnel.nexthop_vidifindex = vid_ifindex;
                /* search for net route */
                memset(&net_om_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
                net_om_entry.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;
                while(AMTRL3_OM_GetNextNetRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB , &net_om_entry))
                {
                    /* mask with net route's prefix length to host entry */
                    memset(&masked_host, 0, sizeof(L_INET_AddrIp_T));
                    masked_host = host_entry.u.ip_tunnel.dest_inet_addr;
                    masked_host.preflen = net_om_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen;
                    L_INET_ApplyMask(&masked_host);

                    if(debug_enable)
                    {
                        printf("net route:%lx:%lx:%lx:%lx/%lu\r\n",
                            L_INET_EXPAND_IPV6(net_om_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr),
                            net_om_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen);
                        printf("masked host entry:%lx:%lx:%lx:%lx\r\n",L_INET_EXPAND_IPV6(masked_host.addr));
                    }

                    if(0 == memcmp(masked_host.addr, net_om_entry.inet_cidr_route_entry.inet_cidr_route_dest.addr, SYS_ADPT_IPV4_ADDR_LEN))
                    {
                        /* If found net route is local route, we use destination as tunne next hop */
                        if(net_om_entry.inet_cidr_route_entry.inet_cidr_route_type == VAL_ipCidrRouteType_local)
                            host_entry.u.ip_tunnel.nexthop_inet_addr = host_entry.u.ip_tunnel.dest_inet_addr;
                        else
                            host_entry.u.ip_tunnel.nexthop_inet_addr = net_om_entry.inet_cidr_route_entry.inet_cidr_route_next_hop;
                        host_entry.u.ip_tunnel.nexthop_vidifindex = net_om_entry.inet_cidr_route_entry.inet_cidr_route_if_index;
                        break;
                    }
                }

                if(debug_enable)
                {
                    printf("%s[%d]:Set Tunnel host route to AMTRL3\r\n",__FUNCTION__,__LINE__);
                    printf("route for %d.%d.%d.%d, get %d.%d.%d.%d on vlan %ld\r\n",
                        L_INET_EXPAND_IPV4(host_entry.u.ip_tunnel.dest_inet_addr.addr),
                        L_INET_EXPAND_IPV4(host_entry.u.ip_tunnel.nexthop_inet_addr.addr),
                        host_entry.u.ip_tunnel.nexthop_vidifindex);
                    printf("IP=[%ld]%lx:%lx:%lx:%lx\r\n",
                        host_entry.dst_vid_ifindex,L_INET_EXPAND_IPV6(host_entry.dst_inet_addr.addr));
                    printf("tunnel src=[%ld]%d.%d.%d.%d\r\n",
                        host_entry.u.ip_tunnel.src_vidifindex, L_INET_EXPAND_IPV4(host_entry.u.ip_tunnel.src_inet_addr.addr));
                    printf("tunnel next=[%ld]%d.%d.%d.%d\r\n",
                        host_entry.u.ip_tunnel.nexthop_vidifindex,L_INET_EXPAND_IPV4(host_entry.u.ip_tunnel.nexthop_inet_addr.addr));
                    printf("tunnel dest=%d.%d.%d.%d\r\n", L_INET_EXPAND_IPV4(host_entry.u.ip_tunnel.dest_inet_addr.addr));
                }
                if(FALSE == AMTRL3_MGR_SetHostRoute(AMTRL3_TYPE_FLAGS_IPV6, SYS_ADPT_DEFAULT_FIB,&host_entry ,NULL, VAL_ipNetToPhysicalExtType_other))
                {
                    /*printf(" Failed to write host route to amtrl3\r\n");*/
                    return FALSE;
                }
            }
            break;
            case NETCFG_TYPE_TUNNEL_MODE_ISATAP:
            {
                /*reserve for future work */
            }
            break;
        }
    }

    return TRUE;
}

#endif


/*------------------------------------------------------------------------------
 * ROUTINE NAME : IML_MGR_TxSnoopPacket_Callback_Func
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle snooping protocol
 * INPUT:
 *    mref_handle_p     --  L_MM_Mref descriptor
 *    dst_mac           --  destination mac address
 *    src_mac           --  source mac address
 *    pkt_length        --  packet length
 *    egr_vidifindex    --  egress vid ifindex
 *    egr_lport         --  egress logical port
 *    ing_lport         --  ingress logical port
 *    pkt_type          --  ethernet type
 * OUTPUT:
 *    None.
 * RETURN:
 *    TRUE       --  packet will be handled by dhcp snooping
 *    FALSE      --  packet will be handled by IML
 * NOTES:
 * (1)If egress port is specified, we should send out packet to this port;
 *    otherwise, we should find out egress port by destination mac address.
 * (2)If ingress port is specified, we shouldn't send out packet to this port;
 *    otherwise, this packet is sent by DUT
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_TxSnoopPacket_Callback_Func(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI32_T              pkt_length,
    UI32_T              egr_vidifindex,
    UI32_T              egr_lport,
    UI32_T              ing_lport,
    UI16_T              pkt_type)
{
    IML_MGR_TxSnoopPacketFunction_T *func_p;

    for (func_p = iml_mgr_tx_snoop_packet_func_list; *func_p; func_p++)
    {
        if ((*func_p)(
            mref_handle_p,
            dst_mac,
            src_mac,
            pkt_length,
            egr_vidifindex,
            egr_lport,
            ing_lport,
            pkt_type))
        {
            return TRUE;
        }
    }

    return FALSE;
}

#if (SYS_CPNT_DHCPSNP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME : IML_MGR_TxSnoopDhcp_CallbackFunc
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This function will handle dhcp snooping protocol
 * INPUT:
 *    mref_handle_p
 *    dst_mac
 *    src_mac
 *    pkt_length
 *    egr_vidifindex
 *    egr_lport
 *    ing_lport
 *    ether_type
 * OUTPUT:
 *    None.
 * RETURN:
 *    TRUE       --  packet will be handled by dhcp snooping
 *    FALSE      --  packet will be handled by IML
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_TxSnoopDhcp_CallbackFunc(
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               *dst_mac,
    UI8_T               *src_mac,
    UI32_T              pkt_length,
    UI32_T              egr_vidifindex,
    UI32_T              egr_lport,
    UI32_T              ing_lport,
    UI16_T              ether_type)
{
    UI32_T egr_vid=0;
    UI32_T pkt_type=0;
    UI32_T ext_hdr_len=0;
    UI32_T pdu_len=0;
    UI8_T *pdu_p = NULL;

    pdu_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(NULL == pdu_p)
        return FALSE;

    VLAN_IFINDEX_CONVERTTO_VID(egr_vidifindex, egr_vid);

    if(FALSE == IML_MGR_PacketClassification(pdu_p,pdu_len,ether_type,egr_vid,egr_lport,dst_mac,&pkt_type,&ext_hdr_len))
        return FALSE;

    /* check if it's dhcp packet */
    if((pkt_type == IML_CLASSIFIED_BOOTP_CLIENT)||
       (pkt_type == IML_CLASSIFIED_BOOTP_SERVER))
    {
        if(TRUE == SYS_CALLBACK_MGR_TxSnoopDhcpPacket(
                        SYS_MODULE_IML,
                        mref_handle_p,
                        pkt_length,
                        dst_mac,
                        src_mac,
                        egr_vidifindex,
                        egr_lport,
                        ing_lport))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

    return FALSE;
}
#endif

#if (SYS_CPNT_VRRP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_ProcessRxVirtualMacAddressPacket
 *------------------------------------------------------------------------------
 * PURPOSE: Process receive IPv4 packet with DA is VRRP virtual mac address
 * INPUT :  mref_handle_p   --  L_MM_Mref descriptor
 *          paylod          --  packet payload
 *          ingress_vid     --  ingress vlan id
 *          dst_mac         --  destination mac address
 *          vlan_mac        --  vlan mac address
 *          ether_type      --  ethernet type
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_ProcessRxVirtualMacAddressPacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T    *payload,
    UI32_T    ingress_vid,
    UI8_T     dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T     vlan_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T    ether_type)
{
    UI32_T avail_size_before_pdu = 0;
    UI16_T eth_hdr_len = 0;
    VRRP_OPER_ENTRY_T vrrp_entry;
    UI32_T vid_ifindex=0;
    IML_Ipv4PktFormat_T *ip_pkt = NULL;
    BOOL_T change_mac = FALSE;

    if((mref_handle_p == NULL)||(payload == NULL))
        return FALSE;

    avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
    LAN_GetEthHeaderLen(FALSE, &eth_hdr_len);
    if(avail_size_before_pdu < eth_hdr_len)
    {
        return TRUE;
    }

    VLAN_VID_CONVERTTO_IFINDEX(ingress_vid, vid_ifindex);

    /* only ARP packet non-VRRP's Ipv4 packet needs to be changed mac address
     */
    if(ether_type == IPV4_FORMAT)
    {
        ip_pkt = (IML_Ipv4PktFormat_T *)payload;
        if(ip_pkt->protocol == IML_IPPROTO_VRRP)
            return TRUE;

        if(IML_MGR_IsVirtualMac(dst_mac) &&
           IML_MGR_IsVrrpMaster(vid_ifindex, dst_mac[5]))
        {
#if (SYS_CPNT_VRRP_PING == TRUE)
            /* if VRRP ping is disabled, and ICMP echo request is sent, drop
             */
            if(ip_pkt->protocol == IML_IPPROTO_ICMP)
            {

                IML_IcmpHdr_T *icmp_hdr;
                icmp_hdr = (IML_IcmpHdr_T *)ip_pkt->payload;
                if((icmp_hdr->type == IML_ICMP_ECHO) &&
                    IML_MGR_IsMasterVirtualIp(vid_ifindex, ip_pkt->dstIp))
                {
                    UI32_T ping_status = 0;
                    if((VRRP_TYPE_OK == VRRP_POM_GetPingStatus(&ping_status))&&
                       (VRRP_TYPE_PING_STATUS_DISABLE == ping_status))
                    {
                        iml_counter.drop_rx_cnt++;
                        if(debug_enable)
                            printf("ping is disabled, drop ICMP echo request\r\n");

                        return FALSE;
                    }
                }
            }
#endif
            change_mac = TRUE;
       }
       else if(IML_MGR_IsAcctonVirtualMac(dst_mac))
       {
            change_mac = TRUE;
       }

       if(change_mac)
       {
            /* only change DA from virtual MAC address to CPU MAC address
             * if VRRP is master
             */
            if(debug_enable)
            {
                printf("change SA from %02X-%02X-%02X-%02X-%02X-%02X to %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],
                    vlan_mac[0],vlan_mac[1],vlan_mac[2],vlan_mac[3],vlan_mac[4],vlan_mac[5]);
            }

            memcpy(payload-eth_hdr_len, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(dst_mac,vlan_mac, SYS_ADPT_MAC_ADDR_LEN);
        }
    }
    else if(ether_type == ARP_FORMAT)
    {
        IML_ArpPktFormat_T  *arp_pkt  = (IML_ArpPktFormat_T *)payload;
        VRRP_ASSOC_IP_ENTRY_T assoc_ip_entry;
        UI8_T dip[SYS_ADPT_IPV4_ADDR_LEN];

        if(!memcmp(arp_pkt->dstMacAddr, dst_mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            memset(&assoc_ip_entry, 0, sizeof(assoc_ip_entry));
            IP_LIB_UI32toArray(arp_pkt->dstIp, assoc_ip_entry.assoc_ip_addr);
            if((VRRP_TYPE_OK == VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(&assoc_ip_entry)) &&
                IML_MGR_IsVrrpMaster(assoc_ip_entry.ifindex, assoc_ip_entry.vrid))
            {
                /* only change DA from virtual MAC address to CPU MAC address
                 * if VRRP is master
                 */
                change_mac = TRUE;
            }
            else if(IML_MGR_IsAcctonVirtualMac(dst_mac))
            {
                change_mac = TRUE;
            }
        }

        if(change_mac)
        {
                if(debug_enable)
                {
                    printf("change SA from %02X-%02X-%02X-%02X-%02X-%02X to %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                        dst_mac[0],dst_mac[1],dst_mac[2],dst_mac[3],dst_mac[4],dst_mac[5],
                        vlan_mac[0],vlan_mac[1],vlan_mac[2],vlan_mac[3],vlan_mac[4],vlan_mac[5]);
                }

                memcpy(payload-eth_hdr_len, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);
                memcpy(dst_mac,vlan_mac, SYS_ADPT_MAC_ADDR_LEN);

                /* if it's ARP packet, change destination hardware address from virtual MAC address to CPU MAC address
                 */
                if(debug_enable)
                {
                    printf("change ARP reply's target MAC address from %02X-%02X-%02X-%02X-%02X-%02X to %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                        arp_pkt->dstMacAddr[0],arp_pkt->dstMacAddr[1],arp_pkt->dstMacAddr[2],
                        arp_pkt->dstMacAddr[3],arp_pkt->dstMacAddr[4],arp_pkt->dstMacAddr[5],
                        vlan_mac[0],vlan_mac[1],vlan_mac[2],vlan_mac[3],vlan_mac[4],vlan_mac[5]);
                }

                memcpy(arp_pkt->dstMacAddr, vlan_mac, SYS_ADPT_MAC_ADDR_LEN);
            }
        }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_ProcessTxVirtualMacAddressPacket
 *------------------------------------------------------------------------------
 * PURPOSE: Process transmit IPv4 packet with DA is VRRP virtual mac address
 * INPUT :  mref_handle_p   --  L_MM_Mref descriptor
 *          paylod          --  packet paylod
 *          packet_args     --  information about packet
 * OUTPUT:  None
 * RETURN:  None
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static void IML_MGR_ProcessTxVirtualMacAddressPacket(
    L_MM_Mref_Handle_T *mref_handle_p,
    UI8_T *payload,
    IML_MGR_RecvPktFromTCPIPStackArgs_T *packet_args)
{
    UI32_T avail_size_before_pdu = 0;
    UI16_T eth_hdr_len = 0;
    VRRP_ASSOC_IP_ENTRY_T assoc_ip_entry;
    UI32_T vid_ifindex;
    UI8_T sip[SYS_ADPT_IPV4_ADDR_LEN];
    IML_Ipv4PktFormat_T *ip_pkt = NULL;
    UI8_T vmac[SYS_ADPT_MAC_ADDR_LEN] = {};
    BOOL_T change_mac = FALSE;

    /* only ARP packet non-VRRP's Ipv4 packet needs to be changed mac address
     */
    VLAN_VID_CONVERTTO_IFINDEX((packet_args->tag_info & 0x1FFF), vid_ifindex);
    avail_size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
    LAN_GetEthHeaderLen(TRUE, &eth_hdr_len);
    if(avail_size_before_pdu < eth_hdr_len)
    {
        return;
    }

    memset(&assoc_ip_entry, 0, sizeof(assoc_ip_entry));
    if(packet_args->packet_type == IPV4_FORMAT)
    {
        ip_pkt = (IML_Ipv4PktFormat_T *)payload;
        if(ip_pkt->protocol == IML_IPPROTO_VRRP)
            return;

        /* only change SA from CPU MAC address to virtual MAC address
         * if VRRP is master
         */
        IP_LIB_UI32toArray(ip_pkt->srcIp, assoc_ip_entry.assoc_ip_addr);
        if((VRRP_TYPE_OK == VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(&assoc_ip_entry)) &&
            IML_MGR_IsVrrpMaster(assoc_ip_entry.ifindex, assoc_ip_entry.vrid))
        {
            vmac[0] = 0x00;
            vmac[1] = 0x00;
            vmac[2] = 0x5e;
            vmac[3] = 0x00;
            vmac[4] = 0x01;
            vmac[5] = assoc_ip_entry.vrid;
            change_mac = TRUE;
        }
        else
        {
            NETCFG_TYPE_InetRifConfig_T rif_config;
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
            IP_LIB_UI32toArray(ip_pkt->srcIp, rif_config.addr.addr);
            if(debug_enable)
            {
                printf("checking if addr %d.%d.%d.%d is virtual?\r\n", rif_config.addr.addr[0], rif_config.addr.addr[1], rif_config.addr.addr[2], rif_config.addr.addr[3]);
            }
            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetVirtualRifByIpaddr(&rif_config))
            {
                if(debug_enable)
                {
                    printf("This is virtual ip addr %d.%d.%d.%d\r\n", rif_config.addr.addr[0], rif_config.addr.addr[1], rif_config.addr.addr[2], rif_config.addr.addr[3]);
                }
                vmac[0] = VIRTUAL_MAC_BYTE0;
                vmac[1] = VIRTUAL_MAC_BYTE1;
                vmac[2] = VIRTUAL_MAC_BYTE2;
                vmac[3] = VIRTUAL_MAC_BYTE3;
                vmac[4] = VIRTUAL_MAC_BYTE4;
                vmac[5] = rif_config.addr.addr[3];
                change_mac = TRUE;
            }

        }
        if(change_mac)
        {
            if(debug_enable)
            {
                printf("change SA from %02X-%02X-%02X-%02X-%02X-%02X to %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    packet_args->src_mac[0],packet_args->src_mac[1],packet_args->src_mac[2],
                    packet_args->src_mac[3],packet_args->src_mac[4],packet_args->src_mac[5],
                    vmac[0],vmac[1],vmac[2],vmac[3],vmac[4],vmac[5]);
            }

            memcpy(payload-eth_hdr_len+SYS_ADPT_MAC_ADDR_LEN, vmac, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(packet_args->src_mac,vmac, SYS_ADPT_MAC_ADDR_LEN);
        }
    }
    else if(packet_args->packet_type == ARP_FORMAT)
    {
        IML_ArpPktFormat_T  *arp_pkt  = (IML_ArpPktFormat_T *)payload;

        /* only change SA from CPU MAC address to virtual MAC address
         * if VRRP is master
         */
        IP_LIB_UI32toArray(arp_pkt->srcIp, assoc_ip_entry.assoc_ip_addr);
        if((VRRP_TYPE_OK == VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(&assoc_ip_entry)) &&
            IML_MGR_IsVrrpMaster(assoc_ip_entry.ifindex, assoc_ip_entry.vrid))
        {
            vmac[0] = 0x00;
            vmac[1] = 0x00;
            vmac[2] = 0x5e;
            vmac[3] = 0x00;
            vmac[4] = 0x01;
            vmac[5] = assoc_ip_entry.vrid;
            change_mac = TRUE;
        }
        else
        {
            NETCFG_TYPE_InetRifConfig_T rif_config;
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
            IP_LIB_UI32toArray(arp_pkt->srcIp, rif_config.addr.addr);
            if(debug_enable)
            {
                printf("checking if addr %d.%d.%d.%d is virtual?\r\n", rif_config.addr.addr[0], rif_config.addr.addr[1], rif_config.addr.addr[2], rif_config.addr.addr[3]);
            }
            //UI32_T tid = SYSFUN_TaskIdSelf();
            ///DEBUG
            //printf("%s, %d, task id is: %lu\r\n", __FUNCTION__, __LINE__, tid);

            if(NETCFG_TYPE_OK == NETCFG_OM_IP_GetVirtualRifByIpaddr(&rif_config))
            {
                if(debug_enable)
                {
                    printf("This is virtual ip addr %d.%d.%d.%d\r\n", rif_config.addr.addr[0], rif_config.addr.addr[1], rif_config.addr.addr[2], rif_config.addr.addr[3]);
                }
                vmac[0] = VIRTUAL_MAC_BYTE0;
                vmac[1] = VIRTUAL_MAC_BYTE1;
                vmac[2] = VIRTUAL_MAC_BYTE2;
                vmac[3] = VIRTUAL_MAC_BYTE3;
                vmac[4] = VIRTUAL_MAC_BYTE4;
                vmac[5] = rif_config.addr.addr[3];
                change_mac = TRUE;
            }
        }
        if(change_mac)
        {
            if(debug_enable)
            {
                printf("change SA and sender hardware address from %02X-%02X-%02X-%02X-%02X-%02X to %02X-%02X-%02X-%02X-%02X-%02X\r\n",
                    packet_args->src_mac[0],packet_args->src_mac[1],packet_args->src_mac[2],
                    packet_args->src_mac[3],packet_args->src_mac[4],packet_args->src_mac[5],
                    vmac[0],vmac[1],vmac[2],vmac[3],vmac[4],vmac[5]);
            }

            memcpy(payload-eth_hdr_len+SYS_ADPT_MAC_ADDR_LEN, vmac, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(packet_args->src_mac,vmac, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(arp_pkt->srcMacAddr, vmac, SYS_ADPT_MAC_ADDR_LEN);
        }
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_IsVrrpMaster
 *------------------------------------------------------------------------------
 * PURPOSE: check if it's VRRP master
 * INPUT :  ifindex     --  vlan ifindex
 *          vrid        --  vritual router id
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsVrrpMaster(UI32_T ifindex, UI32_T vrid)
{
    VRRP_OPER_ENTRY_T vrrp_entry;

    memset(&vrrp_entry, 0, sizeof(vrrp_entry));
    vrrp_entry.ifindex = ifindex;
    vrrp_entry.vrid = vrid;
    if((VRRP_TYPE_OK == VRRP_POM_GetVrrpOperEntry(&vrrp_entry)) &&
       (vrrp_entry.oper_state == VAL_vrrpOperState_master))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_IsMasterVirtualIp
 *------------------------------------------------------------------------------
 * PURPOSE: check if ip address is VRRP master virtual ip
 * INPUT :  ifindex     --  vid ifindex
 *          ipaddr      --  ip address
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsMasterVirtualIp(UI32_T ifindex, UI32_T ipaddr)
{
    VRRP_ASSOC_IP_ENTRY_T assoc_ip_entry;

    memset(&assoc_ip_entry, 0, sizeof(assoc_ip_entry));
    IP_LIB_UI32toArray(ipaddr, assoc_ip_entry.assoc_ip_addr);
    if((VRRP_TYPE_OK == VRRP_POM_GetVrrpAssoIpAddrEntryByIpaddr(&assoc_ip_entry)) &&
        IML_MGR_IsVrrpMaster(ifindex, assoc_ip_entry.vrid))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_IsVirtualMac
 *------------------------------------------------------------------------------
 * PURPOSE: check if MAC address is virtual MAC address
 * INPUT :  mac     --  MAC address
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsVirtualMac(UI8_T *mac)
{
    return ((mac[0]== 0x00)&&(mac[1]== 0x00)&&(mac[2]== 0x5e)&&(mac[3]== 0x00)&&(mac[4]== 0x01));
}
#endif
#if(SYS_CPNT_VIRTUAL_IP == TRUE)
/*------------------------------------------------------------------------------
 * ROUTINE NAME  - IML_MGR_IsAcctonVirtualMac
 *------------------------------------------------------------------------------
 * PURPOSE: check if MAC address is Accton virtual MAC address
 * INPUT :  mac     --  MAC address
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES :  None
 *------------------------------------------------------------------------------
 */
static BOOL_T IML_MGR_IsAcctonVirtualMac(UI8_T *mac)
{
    return ((mac[0] == VIRTUAL_MAC_BYTE0) &&
            (mac[1] == VIRTUAL_MAC_BYTE1) &&
            (mac[2] == VIRTUAL_MAC_BYTE2) &&
            (mac[3] == VIRTUAL_MAC_BYTE3) &&
            (mac[4] == VIRTUAL_MAC_BYTE4));
}
#endif

#ifdef BACKDOOR_OPEN
static void IML_MGR_ShowCounter()
{
    BACKDOOR_MGR_Printf (" IML packet handling counters :\n Rx :");
    BACKDOOR_MGR_Printf ("\n   Receive from LAN : %d,  IP/ARP/M'cast(%d,%d,%d), rx total=%d",
            (int)iml_counter.rx_from_lower_layer_cnt,
            (int)iml_counter.rx_ip_cnt,
            (int)iml_counter.rx_arp_cnt,
            (int)iml_counter.rx_mcast_cnt,
            (int)iml_counter.rx_cnt);
    BACKDOOR_MGR_Printf ("\n                  drop=%d, inner error=%d, drop U'cast DA invalid=%d",
            (int)iml_counter.drop_rx_cnt,
            (int)iml_counter.inner_error_cnt,
            (int)iml_counter.drop_rx_unicast_da_invalid_cnt);
    BACKDOOR_MGR_Printf ("\n   Send to IP/ARP/ND/Bootp(%d,%d,%d,%d)",
            (int)iml_counter.send_to_kernel_ip_cnt,
            (int)iml_counter.send_to_kernel_arp_cnt,
            (int)iml_counter.send_to_kernel_nd_cnt,
            (int)iml_counter.send_to_bootp_cnt);
    BACKDOOR_MGR_Printf ("\n   Send to DHCPSNP/DHCPv6SNP/NDSNP(%d,%d,%d)",
            (int)iml_counter.send_to_dhcpsnp_cnt,
            (int)iml_counter.send_to_dhcpv6snp_cnt,
            (int)iml_counter.send_to_ndsnp_cnt);
    BACKDOOR_MGR_Printf ("\n TX :\n   IP/ARP/B'cast(%d,%d,%d), total=%d",
            (int)iml_counter.tx_ip_cnt,
            (int)iml_counter.tx_arp_cnt,
            (int)iml_counter.tx_bcast_cnt,
            (int)iml_counter.tx_cnt);
    BACKDOOR_MGR_Printf ("\n   drop=%d, null mac=%d, no msg buf=%d",
            (int)iml_counter.drop_tx_cnt,
            (int)iml_counter.drop_tx_null_mac_cnt,
            (int)iml_counter.drop_no_msg_buf_cnt);
}   /* end  of IML_MGR_ShowCounter */

static void IML_MGR_ShowPacket(UI8_T *da, UI8_T *sa, UI32_T pkt_type, UI32_T pkt_len, void *payload)
{
    IML_Ipv4PktFormat_T   *ip_pkt_p=(IML_Ipv4PktFormat_T*)payload;
    IML_ArpPktFormat_T  *arp_pkt_p=(IML_ArpPktFormat_T*)payload;

    printf (" packet len=%ld. DA=%02x%02x%02x%02x%02x%02x, SA=%02x%02x%02x%02x%02x%02x.\n", (long)pkt_len,
            da[0],da[1],da[2],da[3],da[4],da[5], sa[0],sa[1],sa[2],sa[3],sa[4],sa[5]);
    switch(pkt_type)
    {
        case IPV4_FORMAT:
             if(ip_pkt_p->protocol == IML_IPPROTO_IPV6)
             {
                printf(" IPv6-in-IPv4 tunnel packet\n");
             }
             printf(" IP: sip=%08lx, dip=%08lx, proto=%d, TTL=%d\n",
                    (unsigned long)L_STDLIB_Ntoh32(ip_pkt_p->srcIp), (unsigned long)L_STDLIB_Ntoh32(ip_pkt_p->dstIp), ip_pkt_p->protocol, ip_pkt_p->ttl);
             break;
        case ARP_FORMAT:
             printf(" ARP: sip=%08lx, dip=%08lx, opcode=%d, sa=%02x%02x%02x%02x%02x%02x, da=%02x%02x%02x%02x%02x%02x\n",
                    (unsigned long)L_STDLIB_Ntoh32(arp_pkt_p->srcIp), (unsigned long)L_STDLIB_Ntoh32(arp_pkt_p->dstIp), L_STDLIB_Ntoh16(arp_pkt_p->opcode),
                    arp_pkt_p->srcMacAddr[0],arp_pkt_p->srcMacAddr[1],arp_pkt_p->srcMacAddr[2],
                    arp_pkt_p->srcMacAddr[3],arp_pkt_p->srcMacAddr[4],arp_pkt_p->srcMacAddr[5],
                    arp_pkt_p->dstMacAddr[0],arp_pkt_p->dstMacAddr[1],arp_pkt_p->dstMacAddr[2],
                    arp_pkt_p->dstMacAddr[3],arp_pkt_p->dstMacAddr[4],arp_pkt_p->dstMacAddr[5]);
             break;
        default:
             break;
    }

}   /* end of IML_MGR_ShowPacket */

BOOL_T IML_MGR_Dummy_Free(void* buf, void* cookie)
{
    return TRUE;
}

/* FUNCTION NAME : IML_MGR_BackDoor_Menu
 * PURPOSE:
 *      Display SYStem Function back door available function and accept user seletion.
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
 *      None
 */
static void IML_MGR_BackDoorMain(void)
{
    int     ch;
    BOOL_T  eof=FALSE;
    char    buf[16];
    char    *terminal;
    //kh_shi IML_MSG_T *msg;
    IML_PACKET_RX_T *p_msg_data_blk;
    UI32_T  priority;
    char buffer[300];
    UI8_T dst_mac[]  = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    UI8_T src_mac[] =  { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
    L_MM_Mref_Handle_T *mref_handle_p;
    int i;

    UI8_T   msg_buf[SYSFUN_SIZE_OF_MSG(IML_MSG_SIZE)];

    IML_MSG_T     *msg;
    SYSFUN_Msg_T  *msgbuf_p = (SYSFUN_Msg_T*)msg_buf;

    L_THREADGRP_Handle_T tg_handle;
    UI32_T               backdoor_member_id;

    tg_handle      = L2_L4_PROC_COMM_GetL2muxGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (! eof)
    {
        BACKDOOR_MGR_Printf("\n 0. Exit\n");
        BACKDOOR_MGR_Printf(" 1. Show counter.\n");
        BACKDOOR_MGR_Printf(" 2. Enable/Disable debug.(%s)\n", ((debug_enable)?"on":"off"));
        BACKDOOR_MGR_Printf(" 3. Suspend IML Task.\n");
        BACKDOOR_MGR_Printf(" 4. Resume IML Task.\n");
        BACKDOOR_MGR_Printf(" 5. Enable/Disable print packet type.(%s)\n",((show_incoming_packet_type == TRUE)?"on":"off") );
        BACKDOOR_MGR_Printf(" 6. Enable/Disable using Priority Queue.(%s)\n",((iml_using_priority == TRUE)?"on":"off") );
        BACKDOOR_MGR_Printf(" 7. Inject 10000 packets.\n");
        BACKDOOR_MGR_Printf(" 8. Enable/Disable flag for 7.(%s)\n",((IML_MGR_BackDoor_testingTick == TRUE)?"on":"off"));
        BACKDOOR_MGR_Printf(" 9. Set MAC cache age out time\n");
        BACKDOOR_MGR_Printf("    select =");
        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int) strtoul( buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");
        switch (ch)
        {
            case 0 :
                eof = TRUE;
                break;
            case 1 :
                IML_MGR_ShowCounter();
                break;
            case 2 :
                debug_enable = ~debug_enable;
                break;
            case 3 :
                if (SYSFUN_SuspendTaskWithTimeout(0) != SYSFUN_OK)
                    BACKDOOR_MGR_Printf("\nSuspend IML Task Failed.");
                break;
            case 4 :
                if (SYSFUN_ResumeTaskInTimeoutSuspense(iml_rx_task_id) != SYSFUN_OK)
                    BACKDOOR_MGR_Printf("\nResume IML Task Failed.");
                break;
            case 5 :
                if (show_incoming_packet_type)
                    show_incoming_packet_type = FALSE;
                else
                    show_incoming_packet_type = TRUE;

                break;
            case 6:
                if (iml_using_priority == TRUE)
                {
                    while (L_MSG_GetFromPriorityQueue(&priority_queue, (L_MSG_Message_T*)msgbuf_p->msg_buf, &priority))
                    {
                        msg = GET_IML_MSG_PTR_FROM_L_IML_MSG_PTR((IML_L_MSG_T*)msgbuf_p->msg_buf);
                        p_msg_data_blk = (IML_PACKET_RX_T*) msg->mtext;

                        L_MM_Mref_Release(&(p_msg_data_blk->mref_handle_p));
                        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
                    }

                    iml_using_priority = FALSE;
                }
                else
                {
                    while(SYSFUN_ReceiveMsg(iml_msg_q_id, 0, SYSFUN_TIMEOUT_NOWAIT, IML_MSG_SIZE, msgbuf_p)==SYSFUN_OK)
                    {
                        msg = (IML_MSG_T*) msg_buf;
                        p_msg_data_blk = (IML_PACKET_RX_T*) msg->mtext;

                        L_MM_Mref_Release(&(p_msg_data_blk->mref_handle_p));
                        L_MPOOL_FreeBlock (iml_msg_blk_pool, p_msg_data_blk);
                    }

                    iml_using_priority = TRUE;
                }
                break;
            case 7:
                iml_delta_time = 0;
                iml_end_time   = 0;
                iml_start_time = 0;

                mref_handle_p = L_MM_Mref_Construct(buffer, 300, 30, 250,
                                                    L_MM_MREF_FREE_FUN_CUSTOM,
                                                    NULL, NULL);
                memset(buffer,0xff,300);

                if (IML_MGR_BackDoor_testingTick == TRUE)
                {
                    iml_start_time = SYSFUN_GetSysTick();
                }

                for (i=0; i<10000; i++)
                {
                    IML_MGR_RecvPacket(mref_handle_p,
                                       dst_mac,
                                       src_mac,
                                       0x0001,
                                       IPV4_FORMAT,
                                       300,
                                       5);
                }
                if (IML_MGR_BackDoor_testingTick == TRUE)
                {
                    iml_end_time = SYSFUN_GetSysTick();
                }
                iml_delta_time = iml_end_time - iml_start_time;
                BACKDOOR_MGR_Printf("\nProcess Tick = %lu", iml_delta_time);
                break;

            case 8:
                if (IML_MGR_BackDoor_testingTick)
                    IML_MGR_BackDoor_testingTick = FALSE;
                else
                    IML_MGR_BackDoor_testingTick = TRUE;
                break;
            case 9:
                {
                    UI32_T age_time=0;
                    char line_buffer[255];
                    MAC_CACHE_GetAgeOutTime(&age_time);
                    BACKDOOR_MGR_Printf("\n MAC cache age out time = %u secs",(age_time/SYS_BLD_TICKS_PER_SECOND));
                    BACKDOOR_MGR_Printf("\n age out time (secs)= ");
                    if(BACKDOOR_MGR_RequestKeyIn(line_buffer, 255)>0)
                    {
                        age_time = atoi(line_buffer);
                    }

                    MAC_CACHE_SetAgeOutTime(age_time*SYS_BLD_TICKS_PER_SECOND);
                }
                break;
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */
}   /*  end of IML_MGR_BackDoor_Menu    */

#endif  /*  end of #ifdef BACKDOOR_OPEN    */
