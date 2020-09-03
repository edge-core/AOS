/*-----------------------------------------------------------------------------
 * Module Name: lan.c
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of lan.c
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. All those files which needs to receive packet need to register his
 *    receiving API function to lan.c
 * 2. lan.c will not call relative upper layer API to notify that the relative
 *    packet he is interested has arrived.
 * 3. In order to reduce memory copy, we pass L_MM_Mref_Handle_T as an argument.
 * 4. For receiving, we asked NIC driver to allocate buffer by our MAlloc().
 *    The NIC driver can pass the buffer to LAN.C and then we can repackage
 *    the buffer to be L_MM_Mref_Handle_T which allows different layer component to
 *    see the portion what they really care(by changing pointer of pdu) without
 *    changing the real buf pointer(this is really free routine wants)
 * 5. For transimittion, we will ask that upper layer component to allocate
 *    buffer by supported utility and pass the L_MM_Mref_Handle_T to lower lever
 *    code.  And also needs to reserve enough space for lower layer component
 *    to add their header.
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    06/26/2001 - Jason Hsue, Created
 *    02/23/2002 - Add LAN_BufferAdjustment() for the purpose that Allayer chip
 *                 might send a untagged packet.  But if this is an IGMP packet
 *                 and need to be flooded to some other ports, IGMPSnp will use
 *                 MultiSend packet but it will be treated as a tagged packet.
 *                 Then the preallocate buffer for ethernet header won't be
 *                 enough.
 *    02/23/2002 - Add a workaround for LAN_RecvPacket()
 *                 For Allayer chip, the packet receive from a trunk will be only
 *                 be reported by trunk id.  We need to know if it is a trunk
 *                 id and translate it back to the original port.
 *    02/25/2002 - There is a Boradcom Allayer chip limitation.  Whenever a trunk
 *                 is formed, the packet received will never carry the normal port
 *                 as his reciving port, instead it will report trunk port from
 *                 32 - 37.  We need to patch the LAN driver here.
 *                 Here is what we do:
 *                 1, Add a lot #ifdef ALLAYER_SWITCH
 *                 2. Whenever reciving a packet from LACP and the port number is
 *                    normal, learn the link partner MAC address and his port number.
 *                 3. When reciving a packet which port number is 32-37
 *                    if this is an LACP packet, search the MAC and get port number
 *                       and change the port number as the real port number.
 *                    else call swctrl to get the primary port number from trunk
 *                       logical port and pass the trunk primary port to upper layer.
 *    02/26/2002 - Correct the bug for LAN_CheckBuffer() at LAN_sendPkt
 *    07/11/2002 - To add broadcast storm control for Graham serious and move the
 *                 storm constant to sys_adpt.h
 *    10/31/2002 - Modify to support 802.1x
 *    06/12/2003 - Add unicast storm protection mechanism
 *    2004-11-26   wuli, remove class storm protection by using FFP
 *                       remove trap enable/disable functions
 *    2005-02-23   wuli, use SYS_DFLT_MYMACMYIP_PACKET_TO_CPU_PRIORITY for reply
 *                       packet ( call DEV_NICDRV_SendPacketToPort )
 *    2005-09-01   kh_shi, remove task and storm protection in LAN and refine the code
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2005
 *-----------------------------------------------------------------------------
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_hwcfg.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "xstp_pom.h"
#include "swctrl_pom.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "stktplg_pom.h"
#include "backdoor_mgr.h"
#include "lan_type.h"
#include "dev_swdrv.h"
#include "dev_nicdrv_pmgr.h"
#include "dev_nicdrv_lib.h"
#include "lan.h"
#include "lan_om.h"
#include "l_threadgrp.h"
#include "sysrsc_mgr.h"
#include "sys_callback_mgr.h"
#include "l_proto.h"
#include "l_bitmap.h"
#include "l_sort_lst.h"
#include "vlan_om.h"

#if (SYS_CPNT_MGMT_PORT == TRUE)
#include "adm_nic.h"
#endif

#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif

#if(SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
#include "swctrl_pom.h"
#include "vlan_lib.h"
#include "vlan_pom.h"
#include "amtrdrv_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
#define LAN_STACKING_BACKDOOR_OPEN              /* toggle stacking backdoor */
#endif

/* Now design for authenticating packet via SYS_CALLBACK
 */
#if (SYS_CPNT_IPSG_MAC_MODE == TRUE)
#define LAN_SYS_CALLBACK_FOR_AUTH_PACKET    TRUE
#endif

#define LAN_FORCE_TRANSFER_TO_TAGGED_PACKET         /* for bcm drvier, force tagged when send single packet */

#define LAN_MAX_PDU_SIZE                1500         /* SYS_ADPT_IF_MTU ?? */
#define LAN_UNTAG_FRAME_HEADER_SIZE     14           /* DA, SA and ether type */
#define LAN_TAG_FRAME_HEADER_SIZE       18           /* DA, SA, 802.1q tag and ether type */
#define LAN_QINQ_FRAME_HEADER_SIZE      22           /* double taging header size */
#define LAN_ONE_TAG_FIELD_SIZE          4            /* tag type + tag info       */
#define LAN_ETHERTYPE_FIELD_SIZE        2            /* ethertype/tag type field  */
#define LAN_NOT_PRESENT_STACKING_PORT   0xffffffff   /* stacking port not exist   */
#define LAN_NOT_PRESENT_MGMT_PORT       0xffffffff   /* management port not exist */
#define LAN_PKT_HIGH_TXCOS              (SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE - 1) /* the highest priority */
#define LAN_IPCIO_BUFFER_SZ             80
#define LAN_ISC_TRY_COUNT               10
#define LAN_ISC_TIMEOUT                 20 /* in ticks*/

/*
 *  Packet           |     DA       | SA | Frame Type | DSAP | SSAP | CTRL | P_ID |
 * ------------------+--------------+----+------------+------+------+------+------+---
 *  BPDU             | 0180C2000000 | SA |   < 1500   |  42  |  42  |  ??  |  00  |...
 *  LLDP             | 0180C200000E | SA |   < 1500   | ..............................
 *  GMRP             | 0180C2000020 | SA |   < 1500   |  42  |  42  |  ??  |  01  |...
 *  GVRP             | 0180C2000021 | SA |   < 1500   |  42  |  42  |  ??  |  01  |...
 *  ARP              | FFFFFFFFFFFF | SA |    0806    | ..............................
 *  RARP             | ............ | SA |    0835    | ..............................
 *  IP               | ............ | SA |    0800    | ..............................
 *  MIP              | 01005Exxxxxx | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  IGMP (Query)     | 01005E000001 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  IGMP (Report)    | 01005E000001 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  IGMP (Leave)     | 01005E000001 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  DVMRP Router     | 01005E000004 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  OSPFIGP Router   | 01005E000005 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  OSPFIGP D Router | 01005E000006 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  ST Router        | 01005E000007 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  ST Host          | 01005E000008 | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  RIP/RIP2 Router  | 01005E00000A | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  IGRP Router      | 01005E00000A | SA |    0800    |  42  |  42  |  ??  |  01  |...
 *  R-APS            | 0119A7000001 | SA |    8902    | ..............................
 *  R-APS (v2)       | 0119A70000XX | SA |    8902    | ..............................
 *  ERPS Health      | 0012CF000000 | SA |   < 1500   |  AA  |  AA  |  03  |0012CF|8032
 *  OAM Loopback     | FFFFFFFFFFFF | SA |    9000    | ..............................
 *  PPPoE Discover   | ............ | SA |    8863    | ..............................
 *  LBD              | 0012CF000000 | SA |   < 1500   |  AA  |  AA  |  03  |0012CF|0002
 *  UDLD (Cisco)     | 01000CCCCCCC | SA |   < 1500   |  AA  |  AA  |  03  |00000C|0111
 *  UDLD (Accton)    | 0112CF000003 | SA |   < 1500   |  AA  |  AA  |  03  |0012CF|0003
 */
#define LAN_TAG_FRAME_ORG_OFFSET            21   /* for SNAP use */
#define LAN_TAG_FRAME_SNAP_TYPE_OFFSET      24   /* for SNAP use */

#define LAN_ADDR_IPV6MC_WORD1   L_STDLIB_Hton16(0x3333)

#define LAN_ADDR_BPDU_WORD1     L_STDLIB_Hton16(0x0180)
#define LAN_ADDR_BPDU_WORD2     L_STDLIB_Hton16(0xC200)
#define LAN_ADDR_BPDU_WORD3     L_STDLIB_Hton16(0x0000)

#define LAN_ADDR_LACP_WORD1     L_STDLIB_Hton16(0x0180)
#define LAN_ADDR_LACP_WORD2     L_STDLIB_Hton16(0xC200)
#define LAN_ADDR_LACP_WORD3     L_STDLIB_Hton16(0x0002)

#define LAN_ADDR_DOT1X_WORD1    L_STDLIB_Hton16(0x0180)
#define LAN_ADDR_DOT1X_WORD2    L_STDLIB_Hton16(0xC200)
#define LAN_ADDR_DOT1X_WORD3    L_STDLIB_Hton16(0x0003)

#define LAN_ADDR_LLDP_WORD1     L_STDLIB_Hton16(0x0180)
#define LAN_ADDR_LLDP_WORD2     L_STDLIB_Hton16(0xC200)
#define LAN_ADDR_LLDP_WORD3     L_STDLIB_Hton16(0x000E)

#define LAN_ADDR_CLUSTER_WORD1  L_STDLIB_Hton16(SYS_DFLT_CLUSTER_ADDR_WORD1)
#define LAN_ADDR_CLUSTER_WORD2  L_STDLIB_Hton16(SYS_DFLT_CLUSTER_ADDR_WORD2)
#define LAN_ADDR_CLUSTER_WORD3  L_STDLIB_Hton16(SYS_DFLT_CLUSTER_ADDR_WORD3)

#define LAN_ADDR_GVRP_WORD1     L_STDLIB_Hton16(0x0180)
#define LAN_ADDR_GVRP_WORD2     L_STDLIB_Hton16(0xC200)
#define LAN_ADDR_GVRP_WORD3     L_STDLIB_Hton16(0x0021)

#define DSAP_TYPE               0x42
#define SSAP_TYPE               0x42
#define CTRL_TYPE               0x03
#define PROTOCOL_ID_BPDU        L_STDLIB_Hton16(0x0000)
#define PROTOCOL_ID_GVRP        L_STDLIB_Hton16(0x0001)

#define LACP_DSAP_TYPE          0x1
#define MARKER_DSAP_TYPE        0x2
#define OAM_DSAP_TYPE           0x3
#define LAN_DSAP_TYPE_ESMC      0xA

#define CLUSTER_MAX_LENGTH      0x0600
#define CLUSTER_DSAP_TYPE       0xAA
#define CLUSTER_SSAP_TYPE       0xAA
#define CLUSTER_CTRL_TYPE       0x03
#define CLUSTER_ORG_BYTE1       0x00
#define CLUSTER_ORG_BYTE2       0x12
#define CLUSTER_ORG_BYTE3       0xCF
#define CLUSTER_SNAP_TYPE       L_STDLIB_Hton16(0x0001)
#define ERPS_HEALTH_SNAP_TYPE   L_STDLIB_Hton16(0x8032)

#define SNAP_DSAP_TYPE          0xAA
#define SNAP_SSAP_TYPE          0xAA
#define SNAP_CTRL_TYPE          0x03

#define LAN_ADDR_ORG_SPEC1_WORD1  L_STDLIB_Hton16(0x0012)
#define LAN_ADDR_ORG_SPEC1_WORD2  L_STDLIB_Hton16(0xCF00)
#define LAN_ADDR_ORG_SPEC1_WORD3  L_STDLIB_Hton16(0x0001)

#define LAN_ADDR_ORG_SPEC3_WORD1  L_STDLIB_Hton16(0x0112)
#define LAN_ADDR_ORG_SPEC3_WORD2  L_STDLIB_Hton16(0xCF00)
#define LAN_ADDR_ORG_SPEC3_WORD3  L_STDLIB_Hton16(0x0004)

#define SNAP_ORG_BYTE1          0x00
#define SNAP_ORG_BYTE2          0x12
#define SNAP_ORG_BYTE3          0xCF
#define SNAP_TYPE_LBD           L_STDLIB_Hton16(0x0002)
#if (SYS_CPNT_MLAG == TRUE)
#define SNAP_TYPE_MLAG          L_STDLIB_Hton16(0x0004)
#endif

#if (SYS_CPNT_UDLD_PDU_FORMAT == SYS_CPNT_UDLD_PDU_FORMAT_ACCTON)
    /* UDLD PDU format for ACCTON */
    #define LAN_ADDR_UDLD_WORD1     L_STDLIB_Hton16(0x0112)
    #define LAN_ADDR_UDLD_WORD2     L_STDLIB_Hton16(0xCF00)
    #define LAN_ADDR_UDLD_WORD3     L_STDLIB_Hton16(0x0003)
    #define SNAP_ORG_UDLD_BYTE1     0x00
    #define SNAP_ORG_UDLD_BYTE2     0x12
    #define SNAP_ORG_UDLD_BYTE3     0xCF
    #define SNAP_TYPE_UDLD          L_STDLIB_Hton16(0x0003)
#else
    /* UDLD PDU format for CISCO */
    #define LAN_ADDR_UDLD_WORD1     L_STDLIB_Hton16(0x0100)
    #define LAN_ADDR_UDLD_WORD2     L_STDLIB_Hton16(0x0CCC)
    #define LAN_ADDR_UDLD_WORD3     L_STDLIB_Hton16(0xCCCC)
    #define SNAP_ORG_UDLD_BYTE1     0x00
    #define SNAP_ORG_UDLD_BYTE2     0x00
    #define SNAP_ORG_UDLD_BYTE3     0x0C
    #define SNAP_TYPE_UDLD          L_STDLIB_Hton16(0x0111)
#endif /* #if (SYS_CPNT_UDLD_PDU_FORMAT == SYS_CPNT_UDLD_PDU_FORMAT_ACCTON) */

#define ETHER_IPV4_TYPE         L_STDLIB_Hton16(0x0800)
#define ETHER_IPV6_TYPE         L_STDLIB_Hton16(0x86DD)
#define ETHER_ARP_TYPE          L_STDLIB_Hton16(0x0806)
#define ETHER_RARP_TYPE         L_STDLIB_Hton16(0x0835)

#define ETHER_TAG_TYPE          L_STDLIB_Hton16(0x8100)
#define ETHER_LACP_TYPE         L_STDLIB_Hton16(0x8809)
#define ETHER_DOT1X_TYPE        L_STDLIB_Hton16(0x888E)
#define ETHER_LLDP_TYPE         L_STDLIB_Hton16(0x88CC)
#define ETHER_CFM_TYPE          L_STDLIB_Hton16(0x8902)
#define ETHER_EFM_OAM_LOOPBACK_TYPE     L_STDLIB_Hton16(0x9000) /* OAM loopback(ETHER_TYPE) */
#define ETHER_PPPOED_TYPE       L_STDLIB_Hton16(0x8863)
#define ETHER_PTP_TYPE          L_STDLIB_Hton16(0x88f7)

/* for management port use
 */
#define LAN_ADM_PKT_ENQUEUE_EVENT    BIT_0
#define LAN_ADM_PACKET_THRESHOLD     10
#define LAN_ADM_TASK_MSG_Q_LEN       50

/* MACRO FUNCTION DECLARATIONS
 */
#define LAN_UNIT_BMP(unit)                 BIT_VALUE((unit)-1)
#define LAN_UNIT_PORT_TO_LPORT(unit,port)  ((SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT*((unit)-1))+port)
#define LAN_LPORT_LOCATE_BYTE(lport)       (((lport)-1)/8)
#define LAN_LPORT_LOCATE_BIT(lport)        (0x80 >> ((lport-1)%8))

#define LAN_PREPEND_ETHERTYPE(buf,type) do {\
        buf -= LAN_ETHERTYPE_FIELD_SIZE;\
        *(UI16_T *)buf = L_STDLIB_Hton16(type);\
        } while (0)

#define LAN_PREPEND_TAGGED_INFO(buf,tpid,tag_info) do {\
        buf -= 2;\
        *(UI16_T *)buf = L_STDLIB_Hton16(tag_info);\
        buf -= 2;\
        *(UI16_T *)buf = L_STDLIB_Hton16(tpid);\
        } while (0)

#define LAN_PREPEND_DA_SA(buf,dst_mac,src_mac) do {\
        I32_T i;\
        for (i=5; i>=0; i--)\
            *(--buf) = src_mac[i];\
        for (i=5; i>=0; i--)\
            *(--buf) = dst_mac[i];\
        } while (0)

#if (SYS_CPNT_MGMT_PORT == TRUE)
#define ADM_NIC_Send(a, b, c, d)    ADM_NIC_Send(a, b, c, d)
#else
#define ADM_NIC_Send(a, b, c, d)    TRUE
#define SYS_ADPT_MGMT_PORT          LAN_NOT_PRESENT_MGMT_PORT
#endif

#define MDATA_BITMAP_SET_INDEX(bitmap, index, vid)           \
	(bitmap)[index / DEV_NIC_MDATA_WIDTH] |= BIT_VALUE(index % DEV_NIC_MDATA_WIDTH); \
	(bitmap)[(DEV_NIC_MDATA_HASH_SIZE/DEV_NIC_MDATA_WIDTH)-1] = vid;

#define MDATA_BITMAP_ISSET_INDEX(bitmap, index, vid)        \
    ((bitmap)[index / DEV_NIC_MDATA_WIDTH] & BIT_VALUE(index % DEV_NIC_MDATA_WIDTH))    \
    && ((bitmap)[(DEV_NIC_MDATA_HASH_SIZE/DEV_NIC_MDATA_WIDTH)-1] == vid)

/* Opcode for send isc packet use
 */
enum{
    LAN_ISC_RX_TX_PACKET = 1,  /* TX: master->slave, RX: slave->master */
    LAN_ISC_LOOPBACK_PACKET,
    LAN_ISC_SET_OAM_LOOPBACK,
    LAN_ISC_SET_INTERNAL_LOOPBACK,
    LAN_ISC_SET_PORT_TPID,
    LAN_ISC_SET_PORT_LEARNING,
    LAN_ISC_SET_PORT_SECURITY,
    LAN_ISC_SET_PORT_DISCARD_UNTAGGED_FRAME,
    LAN_ISC_SET_PORT_DISCARD_TAGGED_FRAME,
    LAN_ISC_ACK,
    LAN_ISC_NAK,
    LAN_ISC_SET_VLAN_LEARNING_STATUS,
    LAN_TOTAL_REMOTE_SERVICES
};

/* DATA TYPE DECLARATIONS
 */

/* For packet data translation, we need the content be packed instead of word alignment.
 */
typedef union
{
    UI8_T   addr[6];
    struct
    {
        UI16_T  word1;
        UI16_T  word2;
        UI16_T  word3;
    } __attribute__((packed, aligned(1))) a;
} __attribute__((packed, aligned(1)))MacAddress_T;

#define word1  a.word1
#define word2  a.word2
#define word3  a.word3

typedef union
{
    UI8_T   raw_data[SYS_ADPT_MAX_FRAME_SIZE];      /* 1522 */
    struct  pkt_S
    {
        MacAddress_T da;               /* destination hardware address */
        MacAddress_T sa;               /* source hardware address      */
        UI16_T       tag_type;         /* tagged type 0x8100           */
        UI16_T       tag_info;         /* tagged info                  */
        UI16_T       type;             /* IP, ARP, or RARP             */
        union
        {
            UI8_T   data_buf[LAN_MAX_PDU_SIZE+4];   /* PDU data + CRC (4 bytes) */
            struct
            {
                UI8_T   dsap;
                UI8_T   ssap;
                UI8_T   ctrl;
                UI16_T  p_id;
                UI8_T   sap_data[LAN_MAX_PDU_SIZE-1];
            } __attribute__((packed, aligned(1))) eth802_2;
        } __attribute__((packed, aligned(1))) pkt;
    } __attribute__((packed, aligned(1))) raw;
} __attribute__((packed, aligned(1))) RxFrame_T;

#define da              raw.da
#define sa              raw.sa
#define frame_tag_type  raw.tag_type
#define frame_tag_info  raw.tag_info
#define frame_type      raw.type
#define pkt_buf         raw.pkt.data_buf
#define dsap            raw.pkt.eth802_2.dsap
#define ssap            raw.pkt.eth802_2.ssap
#define ctrl            raw.pkt.eth802_2.ctrl
#define pid             raw.pkt.eth802_2.p_id
#define sap_data        raw.pkt.eth802_2.sap_data

/* Control header of LAN for stacking
 */
typedef struct
{
    UI32_T service_id;
} LanIscHeader_T;

/* Send packet to remote unit(s)
 * service_id = LAN_ISC_RX_TX_PACKET
 */
typedef struct
{
    UI32_T  service_id; /* to dispatch the lan normal packet or x_off_pkt             */
    UI32_T  src_unit;   /* source unit, only used when Slave receives packet from NIC */
    UI32_T  src_port;   /* source port, only used when Slave receives packet from NIC */
                        /* from Nic_drv and send it to remote master unit             */
    UI32_T  pkt_length; /* ethernet frame length, header is not included */
    UI32_T  reason;
    UI8_T   uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   uport_untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T   tx_port_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
} __attribute__((packed, aligned(1)))StackingHeader_T; /* rename ?? */

/* Enable/disable remote OAM/Internal loopback status
 * service_id = LAN_ISC_SET_OAM_LOOPBACK or
 *              LAN_ISC_SET_INTERNAL_LOOPBACK
 *              LAN_ISC_SET_PORT_TPID
 *              LAN_ISC_SET_PORT_LEARNING
 *              LAN_ISC_SET_PORT_SECURITY
 *              LAN_ISC_SET_VLAN_LEARNING_STATUS
 */
typedef struct
{
    UI32_T service_id; /* to enable/disable remote lan port into OAM/Internal loopback mode */
    UI32_T unit;       /* target remote unit */
    UI32_T port;       /* target remote port */
    UI32_T vid;        /* target vid */
    union
    {
        BOOL_T enable; /* enable/disable oam loopback */
        UI32_T tpid;   /* tpid of a port */
    }data;
} __attribute__((packed, aligned(1)))RemoteOperationHeader_T;

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
typedef struct LAN_QinQ_Tag_S
{
    UI16_T tpid;
    UI16_T tag_info;
    UI16_T inner_tag_info;
    UI8_T uportlist[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T countperunit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
}LAN_QinQ_Tag_T;
#endif

#if (SYS_CPNT_MGMT_PORT == TRUE)
typedef struct
{
    I32_T   unit;
    UI32_T  port;
    void    *cookie;
    UI32_T  reason;
    void    *raw_data;
    I32_T   size;
} AdmTaskMsgBlock_T;

typedef struct
{
    UI32_T  mtype;      /* encoding message type, source    */
    UI8_T   *mtext;     /* point message block              */
    UI32_T  mreserved;  /* not defined for future extension */
} AdmTaskMsg_T;
#endif

/* LOCAL FUNCTION DECLARATIONS
 */
static void   LAN_Init(void);
static void   LAN_InitStackingInfo(void);
static BOOL_T LAN_BufferAdjustment(L_MM_Mref_Handle_T **mref_handle_pp, UI32_T packet_length, UI32_T header_length);
static LAN_TYPE_PacketClass_T LAN_ClassifyPkt(RxFrame_T *frame, UI32_T number_of_tag, BOOL_T *mdata);
static UI32_T LAN_GetPortTpid(UI32_T unit, UI32_T port);
static BOOL_T LAN_CorrectIngressPacketInfo(
    BOOL_T              rx_is_tagged,
    BOOL_T              workaround_vlan_xlate,
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              *tag_info_p,
    UI16_T              *inner_tag_info_p,
    UI16_T              type,
    UI32_T              unit,
    UI32_T              port);
static BOOL_T LAN_CorrectEgressPacketInfo(
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              *tag_info_p,
    UI16_T              *inner_tag_info_p,
    UI16_T              type,
    UI32_T              unit,
    UI32_T              port);
static void   LAN_ProcessPacket(UI32_T unit, UI32_T port,  UI32_T *cookie, UI32_T reason, L_MM_Mref_Handle_T *mref_handle_p);
void   LAN_RecvPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, L_MM_Mref_Handle_T *mref_handle_p);
static BOOL_T LAN_AnnounceRxPktToUpperLayer(L_MM_Mref_Handle_T *mref_handle_p, UI8_T *dst_mac, UI8_T *src_mac, UI16_T tag_info, UI16_T ether_type,
                                            UI32_T pdu_length, UI32_T src_unit, UI32_T src_port, UI32_T q_class);
static BOOL_T LAN_SendPacketToRemoteUnit(UI32_T service_id, L_MM_Mref_Handle_T *mref_handle_p, UI32_T port, UI32_T packet_length, UI32_T reason,
                                         UI16_T dst_bmp, UI8_T *uport_list, UI8_T *uport_untagged_list, UI8_T *tx_port_count_per_unit);
static BOOL_T LAN_RecvIscPacket(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static BOOL_T LAN_RecvIscSetting(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
static BOOL_T LAN_IscService_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id);
static BOOL_T LAN_SendIscReply(ISC_Key_T *key, UI32_T service_id);

/*fuzhimin,20090505*/
#if 0
#if(SYS_CPNT_DHCPSNP == TRUE)
static BOOL_T LAN_isDhcpPkt(L_MM_Mref_Handle_T *mref_handle_p);
#endif
#endif
/*fuzhimin,20090505,end*/

#if (SYS_CPNT_MAINBOARD == TRUE)
static BOOL_T LAN_BlockingPortBufferFreed(UI16_T tag_info, UI32_T unit, UI32_T port, UI32_T packet_type);
#endif

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
static I32_T LAN_QinQ_TagCompare(void *inlist, void *newtag);
static UI32_T LAN_QinQ_BuildList(L_SORT_LST_List_T *list,
                                 UI8_T     dst_mac[6],
                                 UI8_T     src_mac[6],
                                 UI16_T    tag_info,
                                 UI16_T    inner_tag_info,
                                 UI16_T    type,
                                 UI8_T     *uport_list,
                                 UI8_T     *uport_untagged_list,
                                 UI8_T     *tx_port_count_per_unit,
                                 UI32_T    *count);
#endif

/* function for management port
 */
#if (SYS_CPNT_MGMT_PORT == TRUE)
static void LAN_ADM_Return2Pool (void *buf);
static void LAN_ADM_RecvPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, void *raw_data, I32_T size);
static void LAN_ADM_MonitorTaskProcessPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, void *raw_data, I32_T size);
static void LAN_ADM_Task(void);
#endif

/* APIs for chip workaround
 */
#if defined(BCM56514) && 0 /* obsolete, has another solution */
static BOOL_T LAN_ChipWorkaround_FrameWithInnerTagOnlyReceivedFromAccessPort(UI32_T unit, UI32_T port, BOOL_T has_tag, UI32_T number_of_tag, UI16_T *tag_info_p);
#endif
#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
static BOOL_T LAN_ChipWorkaround_IncorrectReasonForIntruder(UI8_T *dst_mac, UI8_T *src_mac, UI16_T tag_info, UI16_T ether_type, UI32_T src_unit, UI32_T src_port, UI32_T *reason_p);
#endif

/* functions for backdoor
 */
#if 0 /* unused function */
static void LAN_DisplayDataByHex(UI8_T *data, UI32_T length);
#endif
static void LAN_BackdoorInputMAC(UI8_T mac_addr[6]);
static void LAN_BackdoorPrintSendPkt(UI32_T unit, UI32_T port, UI32_T packet_length, UI8_T *data);
static void LAN_BackdoorPrePrintSendMultiPkt(L_MM_Mref_Handle_T *mref_handle_p,UI8_T dst_mac[6],UI8_T src_mac[6],UI16_T type,UI16_T tag_info,
                                     UI32_T packet_length,UI8_T *uport_list,UI8_T *uport_untagged_list);
static void LAN_BackdoorPrintSendMultiPkt(UI8_T *uport_list, UI32_T unit_bmp, UI8_T *uport_untagged_list, UI32_T packet_length, UI8_T *data);
static void LAN_BackdoorPrintRxDebug(RxFrame_T *frame, UI32_T unit, UI32_T port, UI32_T size, UI32_T reason);
static void LAN_BackdoorPrintRxTagParsing(RxFrame_T *frame, UI32_T ingress_vlan, BOOL_T rx_is_tagged, BOOL_T has_tag, UI16_T tag_info, UI16_T inner_tag_info, UI16_T packet_frame_type, BOOL_T pkt_is_truncated);

#ifdef LAN_STACKING_BACKDOOR_OPEN
static void LAN_STACKING_Backdoor(void);
static void LAN_STACKING_SendPacket();
static void LAN_STACKING_SendIpPkt(UI32_T unit, UI32_T port);
static void LAN_STACKING_SendStaPkt(UI32_T unit, UI32_T port);
static void LAN_STACKING_SendGvrpPkt(UI32_T unit, UI32_T port);
static void LAN_STACKING_SendLacpPkt(UI32_T unit, UI32_T port);
static void LAN_STACKING_SendMultiIpPkt(void);
static void LAN_STACKING_DisplayRecvPacket(UI8_T *data, UI32_T length);
static void LAN_STACKING_DisplayXmitPacket(L_MM_Mref_Handle_T *mref_handle_p);
static void LAN_STACKING_DisplayIscMref(UI16_T dst_bmp, L_MM_Mref_Handle_T *mref_handle_p, UI32_T priority);
#endif
#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
static UI16_T LAN_GetValidDrvUnitBmp(void);
#endif

/* STATIC LOCAL GLOBAL VARIABLE DECLARATIONS
 */
static struct
{
    BOOL_T chk_stp;
    BOOL_T bypass_auth;                     /* bypass NA/Security check */
    SYS_MODULE_ID_T module_id;
    LAN_AnnouncePktFunction_T announce_cb;
}
lan_packet_info[LAN_TYPE_NUMBER_OF_PACKET_CLASS] =
{                                          /* chk_stp   bypass_auth     module_id                       announce_cb */
    [LAN_TYPE_IP_PACKET] =                  { TRUE,     FALSE,          SYS_MODULE_IML,                 SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_STA_PACKET] =                 { FALSE,    FALSE,          SYS_MODULE_STA,                 SYS_CALLBACK_MGR_ReceiveStaPacketCallback,      },
    [LAN_TYPE_GVRP_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_GVRP,                SYS_CALLBACK_MGR_ReceiveGvrpPacketCallback,     },
    [LAN_TYPE_LACP_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_LACP,                SYS_CALLBACK_MGR_ReceiveLacpPacketCallback,     },
    [LAN_TYPE_OAM_PACKET] =                 { FALSE,    FALSE,          SYS_MODULE_EFM_OAM,             SYS_CALLBACK_MGR_ReceiveOamPacketCallback,      },
    [LAN_TYPE_DOT1X_PACKET] =               { FALSE,    FALSE,          SYS_MODULE_DOT1X,               SYS_CALLBACK_MGR_ReceiveDot1xPacketCallback,    },
    [LAN_TYPE_LLDP_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_LLDP,                SYS_CALLBACK_MGR_ReceiveLldpPacketCallback,     },
    [LAN_TYPE_CLUSTER_PACKET] =             { FALSE,    FALSE,          SYS_MODULE_CLUSTER,             SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_INTERNAL_LOOPBACK_PACKET] =   { FALSE,    TRUE,           SYS_MODULE_SWCTRL,              SYS_CALLBACK_MGR_ReceiveLoopbackPacketCallback, },
    [LAN_TYPE_CFM_PACKET] =                 { FALSE,    FALSE,          SYS_MODULE_CFM,                 SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_SFLOW_PACKET] =               { FALSE,    TRUE,           SYS_MODULE_SFLOW,               SYS_CALLBACK_MGR_ReceiveSflowPacketCallback,    },
    [LAN_TYPE_RAPS_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_ERPS,                SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_OAM_LOOPBACK_PACKET] =        { FALSE,    TRUE,           SYS_MODULE_EFM_OAM_LOOPBACK,    SYS_CALLBACK_MGR_ReceiveOamLbkPacketCallback,   },
    [LAN_TYPE_ITRI_MIM_PACKET] =            { TRUE,     FALSE,          SYS_MODULE_L2MUX,               SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_L2PT_PACKET] =                { TRUE,     FALSE,          SYS_MODULE_L2MUX,               SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_ERPS_HEALTH_PACKET] =         { FALSE,    FALSE,          SYS_MODULE_ERPS,                SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_PPPOED_PACKET] =              { TRUE,     FALSE,          SYS_MODULE_PPPOE_IA,            SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback,    },
    [LAN_TYPE_LBD_PACKET] =                 { TRUE,     TRUE,           SYS_MODULE_LBD,                 SYS_CALLBACK_MGR_ReceiveLbdPacketCallback,      },
    [LAN_TYPE_UDLD_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_UDLD,                SYS_CALLBACK_MGR_ReceiveUdldPacketCallback,     },
    [LAN_TYPE_PTP_PACKET] =                 { FALSE,    FALSE,          SYS_MODULE_PTP,                 SYS_CALLBACK_MGR_ReceiveL2muxPacketCallback     },
    [LAN_TYPE_ESMC_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_SYNCE,               SYS_CALLBACK_MGR_ReceiveESMCPacketCallback,     },
#if(SYS_CPNT_MLAG == TRUE)
    [LAN_TYPE_MLAG_PACKET] =                { FALSE,    FALSE,          SYS_MODULE_MLAG,                SYS_CALLBACK_MGR_ReceiveMlagPacketCallback,     },
#endif
    [LAN_TYPE_UNKNOWN_PACKET] =             { TRUE,     FALSE,          SYS_MODULE_UNKNOWN,             NULL,      },
};


static LAN_IntrusionCallBackFunction_T lan_announce_na_and_security_check_callback;

#if (SYS_CPNT_RUNTIME_SWITCH_DIAG == TRUE)
static const LAN_AnnouncePktDebugFunction_T  lan_announce_pkt_debug_callback = NULL; /* for SWDIAG */
static BOOL_T lan_debug_rx_int; /* for SWDIAG */
#endif

#if (SYS_CPNT_MGMT_PORT == TRUE)
static UI32_T  lan_adm_msgQ_id;      /* message queue id              */
static UI32_T  lan_adm_packet_alloc; /* total packet buffer allacated */
static Lcb_T   lan_adm_lcb;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */

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
void LAN_Register_NA_N_SecurityCheck_Handler(LAN_IntrusionCallBackFunction_T fun)
{
    lan_announce_na_and_security_check_callback = fun;
}

void LAN_InitiateSystemResources(void)
{
    LAN_OM_InitiateSystemResources();
    LAN_Init();
}

void LAN_AttachSystemResources(void)
{
    LAN_OM_AttachSystemResources();
}

void LAN_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    LAN_OM_GetShMemInfo(segid_p, seglen_p);
}

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
static void LAN_Init(void)
{
    LAN_OM_ClearAllBackdoorCounter();
    LAN_OM_ClearAllBackdoorToggle();
#if (SYS_CPNT_EFM_OAM == TRUE)
    LAN_OM_ClearAllOamLoopback();
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
    LAN_OM_ClearAllInternalLoopback();
#endif

    LAN_OM_ClearAllPortLearning();
    LAN_OM_ClearAllPortSecurity();
    LAN_OM_ClearAllVlanLearning();

    lan_announce_na_and_security_check_callback = NULL;

#if (SYS_CPNT_MGMT_PORT == TRUE)
    lan_adm_packet_alloc = 0;
    lan_adm_lcb.task_id  = 0;
    lan_adm_lcb.sem_id   = 0;
    ADM_NIC_DriverInit(SYS_BLD_MAX_LAN_RX_BUF_SIZE_PER_PACKET,(void*)LAN_ADM_RecvPacket,(void*)NULL);

    if (SYSFUN_CreateMsgQ(LAN_ADM_TASK_MSG_Q_LEN, SYSFUN_MSG_FIFO, &lan_adm_msgQ_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("LAN_Init: SYSFUN_CreateMsgQ for ADM fail\n");
        while(1);  /* wait forever if fail to create message queue */
    }
#endif
    return;
}

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
void LAN_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("lan",
                                                       SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
                                                       LAN_BackdoorEntrance);
    ISC_Register_Service_CallBack(ISC_LAN_DIRECTCALL_SID, LAN_IscService_Callback);
    //DEV_NICDRV_Register_RecvPacket_Handler(DEV_NICDRV_PROTOCOL_LAN, (void*)LAN_RecvPacket);
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_CreateTask
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Init create task
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. LAN has no task, we create the task in DEV_NICDRV
 * ----------------------------------------------------------------------------------*/
void LAN_CreateTask(void)
{

#if (SYS_CPNT_MGMT_PORT == TRUE)
    if (SYSFUN_SpawnTask(SYS_BLD_LAN_ADM_TASK, SYS_BLD_LAN_ADM_TASK_PRIORITY,
                         SYS_BLD_TASK_COMMON_STACK_SIZE, SYSFUN_TASK_NO_FP,
                         LAN_ADM_Task, 0,
                         &lan_adm_lcb.task_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("LAN_Create_Task:SYSFUN_SpawnTask return != SYSFUN_OK\n");
    }
#endif

    return;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SetTransitionMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : the LAN set into transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_SetTransitionMode(void)
{
    LAN_OM_SetTransitionMode();
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterTransitionMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterTransitionMode(void)
{
    LAN_OM_EnterTransitionMode();
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterMasterMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as master mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterMasterMode(void)
{
    UI8_T my_mac[SYS_ADPT_MAC_ADDR_LEN];

    memset(my_mac, 0, SYS_ADPT_MAC_ADDR_LEN);
#if (SYS_CPNT_MAINBOARD == TRUE)
    STKTPLG_POM_GetLocalUnitBaseMac(my_mac);
#endif
    LAN_OM_SetMyMac(my_mac);
    LAN_InitStackingInfo();
    LAN_OM_ClearAllVlanLearning();
    LAN_OM_EnterMasterMode();
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_EnterSlaveMode
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Enable the LAN activities as slave mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_EnterSlaveMode(void)
{
    LAN_InitStackingInfo();
    LAN_OM_EnterSlaveMode();
}

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
 *            port          -- the port number that packet to go
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
void LAN_SendPacket(L_MM_Mref_Handle_T *mref_handle_p,
                    UI8_T               dst_mac[6],
                    UI8_T               src_mac[6],
                    UI16_T              type,
                    UI16_T              tag_info,
                    UI32_T              packet_length,
                    UI32_T              unit,
                    UI32_T              port,
                    BOOL_T              is_tagged,
                    UI32_T              cos_value)
{
    UI8_T   *buf = NULL;
    UI32_T  drv_unit, pdu_len;
    BOOL_T  transfer_to_tagged_format;
    UI32_T  header_size;
    UI16_T  inner_tag_info;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)) && (SYS_ADPT_STACKING_PORT != port))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__,(unsigned long) port);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* For SendPacket, packet is tagged or not is decided by upper layer.
     * If the packet should be tagged, the ethernet header will be 18 bytes long.
     * Otherwise, it will be 14 bytes long only.
     * Some chip's sendpacket, the tagged or not is decided by eggress rule. For the
     * case, nic driver won't care the tagged info, but the raw packet we pass to the
     * nic to send should always be tagged.
     * for BCM API :bcm_packet_send_port_async, send single packet should be tagged,
     * so is_tagged always TRUE and ethernet header will be 18 bytes long
     */
#ifdef LAN_FORCE_TRANSFER_TO_TAGGED_PACKET
    transfer_to_tagged_format = TRUE;
#else
    transfer_to_tagged_format = FALSE;
#endif

    if (port == SYS_ADPT_MGMT_PORT)
    {
        transfer_to_tagged_format = FALSE; /* no need to force transfer to tagged for ADM chip driver */
    }

    inner_tag_info = mref_handle_p->pkt_info.inner_tag_info;

    LAN_CorrectEgressPacketInfo(
        dst_mac,
        src_mac,
        &tag_info,
        &inner_tag_info,
        type,
        unit,
        port);

    header_size = LAN_UNTAG_FRAME_HEADER_SIZE;
    header_size += (is_tagged || transfer_to_tagged_format) ? LAN_ONE_TAG_FIELD_SIZE : 0;
    header_size += (inner_tag_info != 0) ? LAN_ONE_TAG_FIELD_SIZE : 0;

    if (LAN_BufferAdjustment(&mref_handle_p, packet_length, header_size) == FALSE)
    {
        SYSFUN_Debug_Printf("%s: LAN_BufferAdjustment(tagged) return false\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    buf = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    /* reset the pdu address to 14~22 bytes ahead and prepend tag info
     */
    L_MM_Mref_MovePdu(mref_handle_p, -header_size, &pdu_len);
    packet_length += header_size;

    LAN_PREPEND_ETHERTYPE(buf, type);

    if (inner_tag_info != 0)
    {
        LAN_PREPEND_TAGGED_INFO(buf, SYS_DFLT_DOT1Q_PORT_TPID_FIELD, inner_tag_info);
    }

    if (is_tagged || transfer_to_tagged_format)
    {
        LAN_PREPEND_TAGGED_INFO(buf, LAN_GetPortTpid(unit, port), tag_info);
    }

    LAN_PREPEND_DA_SA(buf, dst_mac, src_mac);

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX) && !LAN_OM_IsBackdoorPortFilter(unit, port))
    {
        LAN_BackdoorPrintSendPkt(unit, port, packet_length, (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len));
    }

    /* JBOS, EN5251 nic driver, yjlin
     */
    if (port == SYS_ADPT_MGMT_PORT)
    {
        UI16_T  *temp_p = (UI16_T*)&dst_mac[0];

        /* Because JBOS doesn't support BPDU, so if this packet is not BPDU, than send it out.
         * Oherwise, don't send it. modified by tedtai, 020912
         * Here we compare data in network order (instead of host order)
         */
        if (*(temp_p)     != LAN_ADDR_BPDU_WORD1 ||
            *(temp_p + 1) != LAN_ADDR_BPDU_WORD2 ||
            *(temp_p + 2) != LAN_ADDR_BPDU_WORD3 )
        {
            if ( !ADM_NIC_Send((UI32_T)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len), packet_length, 0, 0 ) )
            {
                SYSFUN_Debug_Printf("LAN_SendPacket: ADM_NIC_Send return FALSE\n");
            }
        }
        L_MM_Mref_Release(&mref_handle_p);
    }
    else
    {
        /* STKTPLG_OM_IsLocalUnit do not check illegal unit & option module exist or not !! ??
         */
        if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
        {
            /* For control packet (multicast DA), send it to remote unit via ISC
             */
            if (dst_mac[0] & 1)
            {
                UI32_T   i, lport;
                UI16_T   dst_bmp = LAN_UNIT_BMP(drv_unit); /* get unit bitmap */
                UI8_T    remote_uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI8_T    remote_uport_untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
                UI8_T    tx_port_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

                for (i=0 ; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                {
                    tx_port_count_per_unit[i] = 0;
                }

                /* Send packet to single port of remote unit
                 */
                tx_port_count_per_unit[unit-1] = 1;

                memset(remote_uport_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                lport = LAN_UNIT_PORT_TO_LPORT(unit, port);
                remote_uport_list[LAN_LPORT_LOCATE_BYTE(lport)] = LAN_LPORT_LOCATE_BIT(lport);

                if (TRUE == is_tagged)
                {
                    memset(remote_uport_untagged_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                }
                else
                {
                    memset(remote_uport_untagged_list, 0xff, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                }

                if ( LAN_SendPacketToRemoteUnit(LAN_ISC_RX_TX_PACKET,
                                                mref_handle_p,
                                                port,
                                                packet_length,
                                                DEV_NICDRV_LIB_MAKE_REASON_FROM_COS(LAN_PKT_HIGH_TXCOS-1),
                                                dst_bmp,
                                                remote_uport_list,
                                                remote_uport_untagged_list,
                                                tx_port_count_per_unit) == FALSE )
                {
                     SYSFUN_Debug_Printf("%s:LAN_SendPacketToRemoteUnit return false\n",__FUNCTION__);
                }
            }
            else  /* For unicast packet, send it through stacklink port */
            {
                /* Add semaphore is for packet can be send out without disturb by other send packet function
                 */
                if (DEV_NICDRV_PMGR_SendPacketToPort(unit,
                                                port,
                                                is_tagged,
                                                tag_info,
                                                L_MM_Mref_GetPdu(mref_handle_p, &pdu_len),
                                                packet_length,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                                SYS_DFLT_MYMACMYIP_PACKET_TO_CPU_PRIORITY,
#else
                                                SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                                (void*)LAN_MREF_ReleaseReference,
                                                mref_handle_p) == FALSE)
                {
                    SYSFUN_Debug_Printf("%s:DEV_NICDRV_PMGR_SendPacketToPort return false\n",__FUNCTION__);
                }
            }
        }
        else /* if local */
        {
#ifdef LAN_STACKING_BACKDOOR_OPEN
            if (TRUE == LAN_OM_GetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET))
            {
                LAN_STACKING_DisplayXmitPacket(mref_handle_p);
            }
#endif

            /* Add semaphore is for packet can be send out without disturb by other send packet function
             */
            if (FALSE == DEV_NICDRV_PMGR_SendPacketByPort(unit,
                                                     port,
                                                     is_tagged,
                                                     L_MM_Mref_GetPdu(mref_handle_p, &pdu_len),
                                                     packet_length,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                                     SYS_DFLT_MYMACMYIP_PACKET_TO_CPU_PRIORITY,
#else
                                                     SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                                     (void*)LAN_MREF_ReleaseReference,
                                                     mref_handle_p))
            {
                SYSFUN_Debug_Printf("LAN_SendPacket:DEV_NICDRV_PMGR_SendPacketByPort return false\n");
            }
        }
    }
}

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
                     UI32_T              in_port)
{
    UI32_T pdu_len;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX))
    {
        BACKDOOR_MGR_Printf("\n--sendPkt to Pipeline --\n");
        LAN_BackdoorPrintSendPkt(0, in_port, packet_length, (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len));
    }
    if (FALSE == DEV_NICDRV_PMGR_SendPacketPipeline(in_port,
                                                     L_MM_Mref_GetPdu(mref_handle_p, &pdu_len),
                                                     packet_length,
                                                     mref_handle_p))
    {
        SYSFUN_Debug_Printf("LAN_SendPacketPipeline:DEV_NICDRV_PMGR_SendPacketPipeline returns false\n");
    }
}

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
/* Compare function for L_SORT_LIST
 */
static I32_T LAN_QinQ_TagCompare(void *inlist, void *newtag)
{
    LAN_QinQ_Tag_T *p1 = inlist;
    LAN_QinQ_Tag_T *p2 = newtag;
    int diff;

    if( (inlist==NULL) || (newtag==NULL) )
        return -1;

    if ((diff = p1->tpid - p2->tpid))
        return diff;

    if ((diff = p1->tag_info - p2->tag_info))
        return diff;

    if ((diff = p1->inner_tag_info - p2->inner_tag_info))
        return diff;

    return 0;
}

/* FUNCTION NAME: LAN_QinQ_BuildList
 *----------------------------------------------------------------------------------------
 * PURPOSE: This procedure build the list for QinQ double tags packet.
 *----------------------------------------------------------------------------------------
 * INPUT:   list        -- a list which will save
 *          uport_list  -- port list including unit and port number
 *                           It's a bit map and port 1 is LSB of first byte.
 *                           tagged_list - port tagged list for the corresponding port list.
 *                           It's a bit map and port 1 is LSB of first byte.
 *          uport_untagged_list    -- tell the specified uport_list should be sent with tagged or not
 *          tx_port_count_per_unit -- how many ports need to send for each unit
 *          count       -- return the element numbers in the list plus 1, if uport_list is not zero
 * RETURN:  TRUE    -- success
 *          FALSE   -- failure
 *----------------------------------------------------------------------------------------
 * NOTE:
 *  1. The port need to send double tags which:
 *     -- the port is tagged
 *     -- the port is uplink port
 *  2. Because each port may has different tpid and pvid (include priority and CFI).
 *     Each different outer tag (tpid+pvid)must send out by different cycle. so, we build a
 *     list to save different outer tag and its uport_list and tx_port_count_per_unit.  Then,
 *     we send out each element in the list one by one.
 *  3. To add the 2nd (outer) tag. The tpid is QinQ port's tpid and the tag_info is its pvid.
 *     We don't process priority and CFI, because these data are zero in this project.
 *  4. This function is built the list and count number.
 *  5. Remove the bit and count in uport_list and tx_port_count_per_unit for double tags.
 *     When this function is done, uport_list and tx_port_count_per_unit is updated for
 *     untag and one tag.
 */
static UI32_T LAN_QinQ_BuildList(L_SORT_LST_List_T *list,
                                 UI8_T     dst_mac[6],
                                 UI8_T     src_mac[6],
                                 UI16_T    tag_info,
                                 UI16_T    inner_tag_info,
                                 UI16_T    type,
                                 UI8_T     *uport_list,
                                 UI8_T     *uport_untagged_list,
                                 UI8_T     *tx_port_count_per_unit,
                                 UI32_T    *count)
{
    LAN_QinQ_Tag_T qtag;
    UI32_T unit, port;
    UI32_T byte_idx, bit_mask;
    UI32_T qtag_count;
    UI32_T untag_count;
    BOOL_T is_tagged;

    if (list == NULL ||
        uport_list == NULL || uport_untagged_list == NULL || tx_port_count_per_unit == NULL ||
        count == NULL)
    {
        return FALSE;
    }

    untag_count = 0;
    qtag_count = 0;

    for (unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        for (port = 1; port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; port++)
        {
            byte_idx = ((SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT+7)/8) * (unit-1) + ((port-1)/8);
            bit_mask = BIT_VALUE(7 - ((port - 1) % 8));

            if (!(uport_list[byte_idx] & bit_mask))
            {
                continue;
            }

            is_tagged = (uport_untagged_list[byte_idx] & bit_mask) == 0;

            qtag.tpid = LAN_GetPortTpid(unit, port);
            qtag.tag_info = tag_info;
            qtag.inner_tag_info = inner_tag_info;

            if (!LAN_CorrectEgressPacketInfo(
                    dst_mac,
                    src_mac,
                    &qtag.tag_info,
                    &qtag.inner_tag_info,
                    type,
                    unit,
                    port))
            {
                /* if no tag has been modified
                 * and is outer untagged,
                 * it doesn't need to add to list.
                 */
                if (!is_tagged)
                {
                    untag_count++;
                    continue;
                }
            }

            if (L_SORT_LST_Get(list, &qtag) == TRUE)
            {
                qtag.uportlist[byte_idx] |= bit_mask;
                qtag.countperunit[unit-1]++;
            }
            else
            {
                memset(qtag.uportlist, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
                memset(qtag.countperunit, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
                qtag.uportlist[byte_idx] = bit_mask;
                qtag.countperunit[unit-1] = 1;
                qtag_count++;
            }
            uport_list[byte_idx] &= ~bit_mask;
            tx_port_count_per_unit[unit-1]--;

            L_SORT_LST_Set(list, &qtag);
        }
    }

    *count = qtag_count + (untag_count > 0);

    return TRUE;
}
#endif /* (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE) */


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
                          UI16_T              tag_info,
                          UI32_T              packet_length,
                          UI8_T               *uport_list,
                          UI8_T               *uport_untagged_list,
                          UI8_T               *tx_port_count_per_unit,
                          UI32_T              cos_value)
{
    L_MM_Mref_Handle_T   *untag_mref_handle_p = NULL;
    BOOL_T               mgmt_untag_list_flag = FALSE;
    UI8_T                *buf = NULL;
    UI32_T               unit_bmp = 0;
    L_MM_Mref_Handle_T   *mref_handle_remote_p;
    UI32_T               remote_unit_bmp = 0;
    UI32_T               pdu_len;

    UI32_T               packet_length_send_out;
    UI32_T               header_length;
    L_MM_Mref_Handle_T   *mref_p_send_out;
    UI8_T                *uport_list_send_out;
    UI8_T                *tx_port_count_per_unit_send_out;

    L_MM_Mref_Handle_T  *new_mref_p;
    UI32_T               tpid;
    UI32_T               count;
    UI16_T               inner_tag_info;
    UI16_T               tag_info_send_out, inner_tag_info_send_out;

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
    LAN_QinQ_Tag_T      qtag;
    L_SORT_LST_List_T   list;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DATA) && !LAN_OM_IsBackdoorPortListFilter(uport_list))
    {
        LAN_BackdoorPrePrintSendMultiPkt(mref_handle_p,dst_mac,src_mac,type,tag_info,packet_length,
                                         uport_list,uport_untagged_list);
    }

    inner_tag_info = mref_handle_p->pkt_info.inner_tag_info;

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
    /* create a list to save different outer tag and its uport_list and tx_port_count_per_unit.
     */
    L_SORT_LST_Create(&list,
                      SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST*8,
                      sizeof(LAN_QinQ_Tag_T),
                      (void*)LAN_QinQ_TagCompare);

    /* After LAN_QinQ_BuildList, uport_list and tx_port_count_per_unit will
     * exclude the ports that have to send double tag, i.e. it is left
     * of the ports which send single tag
     */
    if( LAN_QinQ_BuildList(&list,
                           dst_mac,
                           src_mac,
                           tag_info,
                           inner_tag_info,
                           type,
                           uport_list,
                           uport_untagged_list,
                           tx_port_count_per_unit,
                           &count) == FALSE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        L_SORT_LST_Delete_All(&list);
        return;
    }
#else
    count = 1;
#endif /* (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE) */

    /* has list need be sent
     */
    while (count--)
    {
        /* if count > 0, need to allocate/duplicate mem_ref for sending packet.
         */
        if (count > 0)
        {
            /* allocate new buffer for below send
             */
            new_mref_p = L_MM_AllocateTxBuffer(packet_length,
                                               L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SENDMULTIPACKET));

            if ( new_mref_p != NULL )
            {
                UI8_T   *new_buf;

                buf     = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
                new_buf = L_MM_Mref_GetPdu(new_mref_p, &pdu_len);
                memcpy (new_buf, buf, packet_length);

                new_mref_p->current_usr_id = SYS_MODULE_LAN;
            }
            else
            {
                SYSFUN_Debug_Printf("%s:L_MM_AllocateTxBuffer return NULL\n",__FUNCTION__);
                L_MM_Mref_Release(&mref_handle_p);
                break;
            }
            /* Use new create memory reference to send packet
             */
            mref_p_send_out = new_mref_p;

        }  /* (count > 0) */
        else
        {
            /* if count == 0, this is the last time to send packet, use mref_handle_p
             */
            mref_p_send_out = mref_handle_p;
        }

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
        /* Get each element from list one by one
         */
        if (L_SORT_LST_Remove_1st(&list, &qtag))
        {
            if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DA_DATA) == TRUE && !LAN_OM_IsBackdoorPortListFilter(qtag.uportlist))
            {
                UI32_T i;

                BACKDOOR_MGR_Printf("\n[list.numb]%ld ",(long)list.nbr_of_element);
                BACKDOOR_MGR_Printf("[tpid]%04hx ",qtag.tpid);
                BACKDOOR_MGR_Printf("[portList]");
                for (i=0; i<=SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1; i++)
                    BACKDOOR_MGR_Printf("%02x", qtag.uportlist[i]);
                BACKDOOR_MGR_Printf("[tx_port_count_per_unit]");
                for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                    BACKDOOR_MGR_Printf("%d ", qtag.countperunit[i]);
            }

            /* send out this doule tags package
             */
            uport_list_send_out             = qtag.uportlist;
            tx_port_count_per_unit_send_out = qtag.countperunit;
            tpid                            = qtag.tpid;
            tag_info_send_out               = qtag.tag_info;
            inner_tag_info_send_out         = qtag.inner_tag_info;
            packet_length_send_out          = packet_length;
        }
        else /* else part happen when count = 0 */
#endif /* (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE) */
        {
            /* send out this one tag package
             */
            uport_list_send_out             = uport_list;
            tx_port_count_per_unit_send_out = tx_port_count_per_unit;
            tpid                            = SYS_DFLT_DOT1Q_PORT_TPID_FIELD;
            tag_info_send_out               = tag_info;
            inner_tag_info_send_out         = inner_tag_info;
            packet_length_send_out          = packet_length;
        }

        /* convert port list to unit bitmap
         * if the unit bmp is empty just release mref_p and return
         */
        STKTPLG_POM_PortList2DriverUnitList(uport_list_send_out, &unit_bmp);
        if (0 == unit_bmp)
        {
            L_MM_Mref_Release(&mref_p_send_out);
            continue;
        }

        header_length = LAN_TAG_FRAME_HEADER_SIZE;
        header_length += (inner_tag_info_send_out != 0) ? LAN_ONE_TAG_FIELD_SIZE : 0;

        if (LAN_BufferAdjustment(&mref_p_send_out, packet_length_send_out, header_length) == FALSE)
        {
            SYSFUN_Debug_Printf("%s:LAN_BufferAdjustment return false\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }

        buf = L_MM_Mref_GetPdu(mref_p_send_out, &pdu_len);

        /* reset the pdu address to 18~22 bytes ahead
         */
        L_MM_Mref_MovePdu(mref_p_send_out, -header_length, &pdu_len);
        packet_length_send_out += header_length;

        LAN_PREPEND_ETHERTYPE(buf, type);

        if (inner_tag_info_send_out != 0)
        {
            LAN_PREPEND_TAGGED_INFO(buf, SYS_DFLT_DOT1Q_PORT_TPID_FIELD, inner_tag_info_send_out);
        }

        /* send multi packet will always be tagged
         */
        LAN_PREPEND_TAGGED_INFO(buf, tpid, tag_info_send_out);
        LAN_PREPEND_DA_SA(buf, dst_mac, src_mac);

        if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX) && !LAN_OM_IsBackdoorPortListFilter(uport_list))
        {
            LAN_BackdoorPrintSendMultiPkt(uport_list_send_out, unit_bmp, uport_untagged_list, packet_length_send_out,
                                          (UI8_T*)L_MM_Mref_GetPdu(mref_p_send_out, &pdu_len));
        }

#if (SYS_CPNT_MGMT_PORT == TRUE)
        if ((uport_list_send_out[LAN_LPORT_LOCATE_BYTE(SYS_ADPT_MGMT_PORT)] & (LAN_LPORT_LOCATE_BIT(SYS_ADPT_MGMT_PORT))) &&
            (uport_untagged_list[LAN_LPORT_LOCATE_BYTE(SYS_ADPT_MGMT_PORT)] & (LAN_LPORT_LOCATE_BIT(SYS_ADPT_MGMT_PORT))) )
        {
            UI8_T *untag_pdu = NULL;

            untag_mref_handle_p = L_MM_AllocateTxBuffer(packet_length_send_out - header_length,
                                                        L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SENDMULTIPACKET));
            L_MM_Mref_MovePdu(untag_mref_handle_p, -LAN_UNTAG_FRAME_HEADER_SIZE, &pdu_len);
            untag_pdu = L_MM_Mref_GetPdu(untag_mref_handle_p, &pdu_len);

            if ( untag_pdu != NULL )
            {
                /* copy dmac, smac
                 */
                memcpy(untag_pdu, buf, LAN_UNTAG_FRAME_HEADER_SIZE);

                /* copy ether-type, payload
                 */
                memcpy(untag_pdu+LAN_UNTAG_FRAME_HEADER_SIZE, buf + header_length, packet_length_send_out-header_length);

                mgmt_untag_list_flag = TRUE;
                untag_mref_handle_p->current_usr_id = SYS_MODULE_LAN;
            }
        }
#endif

        /* Clone multi-reference to send packet to both local and remote unit
         */
        mref_handle_remote_p = mref_p_send_out;
        L_MM_Mref_AddRefCount(mref_handle_remote_p, 1);

        /* Check if this multicast packet should be sent to local unit
         */
        if (unit_bmp & LAN_UNIT_BMP(LAN_OM_GetMyUnitId())) /* if local unit */
        {
#ifdef LAN_STACKING_BACKDOOR_OPEN
            if (TRUE == LAN_OM_GetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET))
            {
                LAN_STACKING_DisplayXmitPacket(mref_p_send_out);
            }
#endif

            /* Add semaphore is for packet can be send out without disturb by other send packet function
             */
            /* we use mgmt_untag_list_flag == TRUE to replace #if SYS_CPNT_MGMT_PORT == TRUE here
             * for readability. mgmt_untag_list_flag will set to TRUE if mgmt port is supported
             */
            if (TRUE == mgmt_untag_list_flag)
            {
                uport_untagged_list[LAN_LPORT_LOCATE_BYTE(SYS_ADPT_MGMT_PORT)] &= ~(LAN_LPORT_LOCATE_BIT(SYS_ADPT_MGMT_PORT));
                uport_list_send_out[LAN_LPORT_LOCATE_BYTE(SYS_ADPT_MGMT_PORT)] &= ~(LAN_LPORT_LOCATE_BIT(SYS_ADPT_MGMT_PORT));
                tx_port_count_per_unit_send_out[LAN_OM_GetMyUnitId()-1]--;

                /* -LAN_ONE_TAG_FIELD_SIZE is because tagged to untagged
                 */
                if ( FALSE == ADM_NIC_Send((UI32_T)L_MM_Mref_GetPdu(untag_mref_handle_p, &pdu_len), packet_length_send_out-LAN_ONE_TAG_FIELD_SIZE, 0, 0) )
                {
                    SYSFUN_Debug_Printf("%s:ADM_NIC_Send return FALSE\n",__FUNCTION__);
                }

                L_MM_Mref_Release(&untag_mref_handle_p);
            }

            if (tx_port_count_per_unit_send_out[LAN_OM_GetMyUnitId()-1] != 0)
            {
                /* PROBLEM: JASON WRITE HERE FOR MEMERIZE ???
                 * If product is 48 port, we should not care which unit the packet should be sent to.
                 * And we don't need to clone memory buffer before we call Broadcom's nic send.
                 * THIS NEED TO CLARIFY WITH BROADCOM.
                 */
                if (DEV_NICDRV_PMGR_SendPacketByPortList(tx_port_count_per_unit_send_out[LAN_OM_GetMyUnitId()-1],
                                                    uport_list_send_out,
                                                    uport_untagged_list,
                                                    L_MM_Mref_GetPdu(mref_p_send_out, &pdu_len),
                                                    packet_length_send_out,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                                    DEV_NICDRV_LIB_CosValue(LAN_PKT_HIGH_TXCOS), /* use the high cos for packets from cpu */
#else
                                                    SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                                    (void*)LAN_MREF_ReleaseReference,
                                                    mref_p_send_out) == FALSE)
                {
                    SYSFUN_Debug_Printf("%s:DEV_NICDRV_PMGR_SendPacketByPortList return FALSE 1\n",__FUNCTION__);
                }
            }
            else
            {
                L_MM_Mref_Release(&mref_p_send_out);
            }
        }
        else
        {
             /* if no need to send packet to local, just release the mref_p
              */
             L_MM_Mref_Release(&mref_p_send_out);
        }

        /* remove local unit from unit bitmap, since local unit don't need to send thru ISC
         */
        remote_unit_bmp = unit_bmp & (~LAN_UNIT_BMP(LAN_OM_GetMyUnitId()));

        /* Check if this multicast packet should be sent to remote unit
         */
        if (remote_unit_bmp != 0)
        {
            if ( LAN_SendPacketToRemoteUnit(LAN_ISC_RX_TX_PACKET,
                                            mref_handle_remote_p,
                                            0,              /* src port:no use in this case */
                                            packet_length_send_out,
#if 1/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                            DEV_NICDRV_LIB_MAKE_REASON_FROM_COS(LAN_PKT_HIGH_TXCOS-1),
#else
                                            LAN_PKT_HIGH_TXCOS,
#endif
                                            remote_unit_bmp,
                                            uport_list_send_out,
                                            uport_untagged_list,
                                            tx_port_count_per_unit_send_out) == FALSE )
            {
                 SYSFUN_Debug_Printf("%s:LAN_SendPacketToRemoteUnit return FALSE\n",__FUNCTION__);
            }
        }
        else
        {
             /* no need to send packet to remote, just release this mref_p
              */
             L_MM_Mref_Release(&mref_handle_remote_p);
        }
    } /* while loop */

#if (LAN_ADJUST_TAG_FOR_SEND_MULTI_PKT == TRUE)
    L_SORT_LST_Delete_All(&list);
#endif
}

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
                            UI32_T              cos_value)
{
#if (SYS_CPNT_MGMT_PORT == TRUE)
    L_MM_Mref_Handle_T *untag_mref_handle_p;
    UI8_T              *untag_pdu, *pdu;
#endif
    UI8_T    *buf = NULL;
    UI32_T   vid, pdu_len;
    UI32_T   header_size;
    UI16_T   inner_tag_info;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    vid = tag_info & 0x0fff;

    if ((vid == 0) || (vid > SYS_ADPT_MAX_VLAN_ID))
    {
        SYSFUN_Debug_Printf("LAN_SendPacketByVlanId:invalid vid=[%lu]\n", (unsigned long)vid);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* For LAN_SendPacketByVlanId, packet is treated as tagged always.  So unlike SendPacket,
     * this function doesn't need to specify this packet is tagged.
     */
    inner_tag_info = mref_handle_p->pkt_info.inner_tag_info;

    header_size = LAN_TAG_FRAME_HEADER_SIZE;
    header_size += (inner_tag_info != 0) ? LAN_ONE_TAG_FIELD_SIZE : 0;

    if (LAN_BufferAdjustment(&mref_handle_p, packet_length, header_size) == FALSE)
    {
        SYSFUN_Debug_Printf("%s: LAN_BufferAdjustment(tagged) return false\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    buf = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    /* reset the pdu address to 18 bytes ahead
     */
    L_MM_Mref_MovePdu(mref_handle_p, -header_size, &pdu_len);
    packet_length += header_size;

    LAN_PREPEND_ETHERTYPE(buf, type);

    if (inner_tag_info != 0)
    {
        LAN_PREPEND_TAGGED_INFO(buf, SYS_DFLT_DOT1Q_PORT_TPID_FIELD, inner_tag_info);
    }

    /* TODO:
     *   if tpid of ports are different,
     *   this API will works incorrectly. (depend on ch
     * p.s. this API is unused now.
     */
    LAN_PREPEND_TAGGED_INFO(buf, SYS_DFLT_DOT1Q_PORT_TPID_FIELD, tag_info);
    LAN_PREPEND_DA_SA(buf, dst_mac, src_mac);

#if (SYS_CPNT_MGMT_PORT == TRUE)
    /* EN5251 nic driver
     */
    {
        pdu       = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        untag_pdu = L_MEM_Allocate(packet_length-4, L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SENDPACKETBYVLANID));

        if ( untag_pdu != NULL )
        {
            memcpy (untag_pdu, pdu, 12);                        /* copy dmac, smac */
            memcpy (untag_pdu+12, pdu+16, packet_length-16);    /* copy type, payload */

            untag_mref_handle_p = L_MM_Mref_Construct(untag_pdu,
                                                      packet_length-4,
                                                      0,
                                                      packet_length-4,
                                                      (L_MM_Mref_FreeFun_T)L_MEM_Free,
                                                      NULL);

            if ( untag_mref_handle_p == NULL )
            {
                SYSFUN_Debug_Printf("LAN_SendPacketByVlanId:L_MM_Mref_Construct for untag_mref return NULL\n");
                L_MEM_Free(untag_pdu);
            }
            else
            {
                if ( !ADM_NIC_Send((UI32_T)L_MM_Mref_GetPdu(untag_mref_handle_p, &pdu_len), packet_length-4, 0, 0) )
                {
                    SYSFUN_Debug_Printf("LAN_SendPacketByVlanId:ADM_NIC_Send return false\n");
                }
                L_MM_Mref_Release(&untag_mref_handle_p);
            }
         }
    }
#endif
    {
        /* Add semaphore is for packet can be send out without disturb by other send packet function
         */
        if (DEV_NICDRV_PMGR_SendPacketByVid(vid,
                                       L_MM_Mref_GetPdu(mref_handle_p, &pdu_len),
                                       packet_length,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                       DEV_NICDRV_LIB_CosValue(LAN_PKT_HIGH_TXCOS),
#else
                                       SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                       (void*)LAN_MREF_ReleaseReference,
                                       mref_handle_p) == FALSE)

        {
            SYSFUN_Debug_Printf("LAN_SendPacketByVlanId:DEV_NICDRV_SendPacketByVid return false\n");
        }
    }
}

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
BOOL_T LAN_CallByAgent_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_p)
{
    return LAN_IscService_Callback(key, mref_p, ISC_LAN_CALLBYAGENT_SID);
}

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
void LAN_Register_Debug_Handler(LAN_AnnouncePktDebugFunction_T callBackFunction)
{
    lan_announce_pkt_debug_callback = callBackFunction;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RxDebug_Enable
 * ----------------------------------------------------------------------------------
 * PURPOSE  : set lan_debug_rx_int to TRUE, all packet arrive LAN will pass to SWDIAG
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_RxDebug_Enable(void)
{
    lan_debug_rx_int = TRUE;
    return;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RxDebug_Enable
 * ----------------------------------------------------------------------------------
 * PURPOSE  : set lan_debug_rx_int to FASLE, packets arrive LAN will not pass to SWDIAG
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_RxDebug_Disable(void)
{
    lan_debug_rx_int = FALSE;
    return;
}

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
 *           uport_list     -- port list including unit and port number
 *                             It's a bit map and port 1 is LSB of first byte.
 *                             tagged_list - port tagged list for the corresponding port list.
 *                             It's a bit map and port 1 is LSB of first byte.
 *      uport_untagged_list -- tell the specified uport_list should be sent with tagged or not
 *      port_count_per_unit -- number of port that have to send for each unit
 *            cos_value     -- the cos value when sending this packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
void LAN_SendLoopBackPacket (L_MM_Mref_Handle_T  *mref_handle_p,
                             UI8_T               dst_mac[6],
                             UI8_T               src_mac[6],
                             UI16_T              type,
                             UI16_T              tag_info,                /* 802.1p & vid */
                             UI32_T              packet_length,
                             UI8_T               *uport_list,
                             UI8_T               *uport_untagged_list,
                             UI8_T               *tx_port_count_per_unit, /* important:caller is &tx_port_count_per_unit ?? */
                             UI32_T              cos_value)
{
    UI8_T    *buf;
    UI32_T   pdu_len;

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DATA) && !LAN_OM_IsBackdoorPortListFilter(uport_list))
    {
        LAN_BackdoorPrePrintSendMultiPkt(mref_handle_p,dst_mac,src_mac,type,tag_info,packet_length,
                                         uport_list,uport_untagged_list);
    }

    if (LAN_BufferAdjustment(&mref_handle_p, packet_length, 18) == FALSE)
    {
        SYSFUN_Debug_Printf("LAN_SendLoopBackPacket:LAN_BufferAdjustment return false\n");
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (0 == tx_port_count_per_unit[LAN_OM_GetMyUnitId()-1])
    {
        SYSFUN_Debug_Printf("LAN_SendLoopBackPacket:tx_port_count_per_unit=0\n");
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    /* For SendMultiPacket, packet is treated as tagged always.  So unlike SendPacket,
     * this function doesn't need to specify this packet is tagged.
     */
    /* reset the pdu address to 18 bytes ahead
     */
    buf = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    L_MM_Mref_MovePdu(mref_handle_p, -18, &pdu_len);
    packet_length += 18;
    LAN_PREPEND_TAGGED_INFO(buf, SYS_DFLT_DOT1Q_PORT_TPID_FIELD, tag_info, type)
    LAN_PREPEND_DA_SA(buf, dst_mac, src_mac)

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX) && !LAN_OM_IsBackdoorPortListFilter(uport_list))
    {
        LAN_BackdoorPrintSendMultiPkt(uport_list, 0, uport_untagged_list, packet_length, (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len));
    }

    /* PROBLEM: JASON WRITE HERE FOR MEMERIZE ???
     * If product is 48 port, we should not care which unit the packet should be sent to.
     * And we don't need to clone memory buffer before we call Broadcom's nic send.
     * THIS NEED TO CLARIFY WITH BROADCOM.
     */

    /* Add semaphore is for packet can be send out without disturb by other send packet function
     */
    if (!DEV_NICDRV_SendLoopBackPacketByPortList (tx_port_count_per_unit[my_unit_id-1],
                                                  uport_list,
                                                  uport_untagged_list,
                                                  L_MM_Mref_GetPdu(mref_handle_p, &pdu_len),
                                                  packet_length,
                                                  DEV_NICDRV_LIB_CosValue(LAN_PKT_HIGH_TXCOS), /* use the high cos for packets from cpu */
                                                  (void*)LAN_MREF_ReleaseReference,
                                                  mref_handle_p))
    {
        SYSFUN_Debug_Printf("LAN_SendLoopBackPacket:DEV_NICDRV_SendLoopBackPacketByPortList return FALSE\n");
    }
}

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
BOOL_T LAN_SetOamLoopback(UI32_T unit, UI32_T port, BOOL_T enable)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T      *mref_handle_p;
        RemoteOperationHeader_T *frame_ptr;
        UI32_T                  pdu_len;
        UI32_T                  isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETOAMLOOPBACKOFREMOTEUNIT) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_OAM_LOOPBACK;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it take short time only
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }
        if (LAN_ISC_ACK == isc_reply)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
#endif /* #if (SYS_CPNT_STACKING == TRUE) */
    {
        return LAN_OM_SetOamLoopback(unit, port, enable);
    }
}
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
BOOL_T LAN_SetInternalLoopback(UI32_T unit, UI32_T port, BOOL_T enable)
{
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T   *mref_handle_p;
        RemoteOperationHeader_T *frame_ptr;
        UI32_T               pdu_len;
        UI32_T               isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETOAMLOOPBACKOFREMOTEUNIT) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_INTERNAL_LOOPBACK;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it take short time only
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }

        if (LAN_ISC_ACK == isc_reply)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
#endif /* #if (SYS_CPNT_STACKING == TRUE) */
    {
        return LAN_OM_SetInternalLoopback(unit, port, enable);
    }
}
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
BOOL_T LAN_SetPortLearning(UI32_T unit, UI32_T port, BOOL_T enable)
{
    UI32_T retvel = TRUE;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

    if (FALSE == LAN_OM_SetPortLearning(unit, port, enable))
    {
        SYSFUN_Debug_Printf("%s: LAN_OM_SetPortLearning fail unit=%lu port=%lu tpid=%lu\n", __FUNCTION__, (unsigned long)unit, (unsigned long)port, (unsigned long)tpid);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T       *mref_handle_p;
        RemoteOperationHeader_T  *frame_ptr;
        UI32_T                   pdu_len;
        UI32_T                   isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETPORTLEARNING) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_PORT_LEARNING;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it won't take too much time
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }

        if (LAN_ISC_ACK == isc_reply)
        {
            retvel = TRUE;
        }
        else
        {
            retvel = FALSE;
        }
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

    return retvel;
}

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
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
BOOL_T LAN_SetVlanLearningStatus(UI32_T vid, BOOL_T learning)
{
    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if (0 == vid || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        SYSFUN_Debug_Printf("%s: invalid vid = [%lu]\n", __FUNCTION__, (unsigned long)vid);
        return FALSE;
    }

    if (FALSE == LAN_OM_SetVlanLearningStatus(vid, learning))
    {
        SYSFUN_Debug_Printf("%s: LAN_OM_SetVlanLearningStatus fail vid=%lu tpid=%lu\n", __FUNCTION__, (unsigned long)vid, (unsigned long)tpid);
        return FALSE;
    }

    {
#if (SYS_CPNT_STACKING == TRUE)
        L_MM_Mref_Handle_T       *mref_handle_p;
        RemoteOperationHeader_T  *frame_ptr;
        UI32_T                   pdu_len;
        UI16_T                   unit_bmp = 0;

        unit_bmp = LAN_GetValidDrvUnitBmp();

        if (unit_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                    L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETVLANLEARNINGSTATUS) /* user_id */);
            frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            if (NULL == frame_ptr)
            {
                SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
                L_MM_Mref_Release(&mref_handle_p);
                return FALSE;
            }
            frame_ptr->service_id  = LAN_ISC_SET_VLAN_LEARNING_STATUS;
            frame_ptr->vid         = vid;
            frame_ptr->data.enable = learning;

            /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it won't take too much time
             */
            if (ISC_SendMcastReliable(unit_bmp, ISC_LAN_DIRECTCALL_SID,
                                     mref_handle_p,
                                     SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                     LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT, FALSE)!=0)
            {
                SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
                return FALSE;
            }
        }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */
    }

    return TRUE;
}
#endif

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
BOOL_T LAN_SetPortSecurity(UI32_T unit, UI32_T port, BOOL_T enable)
{
    UI32_T retvel = TRUE;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

    if (FALSE == LAN_OM_SetPortSecurity(unit, port, enable))
    {
        SYSFUN_Debug_Printf("%s: LAN_OM_SetPortSecurity fail unit=%lu port=%lu tpid=%lu\n", __FUNCTION__, (unsigned long)unit, (unsigned long)port, (unsigned long)tpid);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T       *mref_handle_p;
        RemoteOperationHeader_T  *frame_ptr;
        UI32_T                   pdu_len;
        UI32_T                   isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETPORTSECURITY) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_PORT_SECURITY;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it won't take too much time
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }

        if (LAN_ISC_ACK == isc_reply)
        {
            retvel = TRUE;
        }
        else
        {
            retvel = FALSE;
        }
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

    return retvel;
}

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
BOOL_T LAN_SetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable)
{
    UI32_T retvel = TRUE;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

    if (FALSE == LAN_OM_SetPortDiscardUntaggedFrame(unit, port, enable))
    {
        SYSFUN_Debug_Printf("%s: LAN_OM_SetPortDiscardUntaggedFrame fail unit=%lu port=%lu tpid=%lu\n", __FUNCTION__, (unsigned long)unit, (unsigned long)port, (unsigned long)tpid);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T       *mref_handle_p;
        RemoteOperationHeader_T  *frame_ptr;
        UI32_T                   pdu_len;
        UI32_T                   isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETPORTDISCARDUNTAGGEDFRAME) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_PORT_DISCARD_UNTAGGED_FRAME;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it won't take too much time
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }

        if (LAN_ISC_ACK == isc_reply)
        {
            retvel = TRUE;
        }
        else
        {
            retvel = FALSE;
        }
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

    return retvel;
}

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
BOOL_T LAN_SetPortDiscardTaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable)
{
    UI32_T retvel = TRUE;
#if (SYS_CPNT_STACKING == TRUE)
    UI32_T drv_unit;
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        return FALSE;
    }

    if ((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
    {
        SYSFUN_Debug_Printf("%s: invalid port = [%lu]\n", __FUNCTION__, (unsigned long)port);
        return FALSE;
    }

    if (port == SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("%s: can't operate on management port\n", __FUNCTION__);
        return FALSE;
    }

    if (FALSE == LAN_OM_SetPortDiscardTaggedFrame(unit, port, enable))
    {
        SYSFUN_Debug_Printf("%s: LAN_OM_SetPortDiscardUntaggedFrame fail unit=%lu port=%lu tpid=%lu\n", __FUNCTION__, (unsigned long)unit, (unsigned long)port, (unsigned long)tpid);
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if (STKTPLG_POM_IsLocalUnit(unit, port, &drv_unit) == FALSE) /* if remote unit */
    {
        L_MM_Mref_Handle_T       *mref_handle_p;
        RemoteOperationHeader_T  *frame_ptr;
        UI32_T                   pdu_len;
        UI32_T                   isc_reply;


        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(RemoteOperationHeader_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SETPORTDISCARDTAGGEDFRAME) /* user_id */);
        frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL == frame_ptr)
        {
            SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
        }
        frame_ptr->service_id  = LAN_ISC_SET_PORT_DISCARD_TAGGED_FRAME;
        frame_ptr->unit        = unit;
        frame_ptr->port        = port;
        frame_ptr->data.enable = enable;

        /* Send to slave unit, use ISC_LAN_DIRECTCALL_SID because it won't take too much time
         */
        if (!ISC_RemoteCall((UI8_T)drv_unit, ISC_LAN_DIRECTCALL_SID, mref_handle_p,
                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                            sizeof(isc_reply), (UI8_T *) &isc_reply,
                            LAN_ISC_TRY_COUNT, LAN_ISC_TIMEOUT) )
        {
            SYSFUN_Debug_Printf("%s:ISC_Send return false.\n",__FUNCTION__);
            return FALSE;
        }

        if (LAN_ISC_ACK == isc_reply)
        {
            retvel = TRUE;
        }
        else
        {
            retvel = FALSE;
        }
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

    return retvel;
}

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
    UI32_T packet_type)
{
    if (packet_type >= LAN_TYPE_UNKNOWN_PACKET)
    {

        L_MM_Mref_Release(&mref_handle_p);
        LAN_OM_IncreaseBackdoorCounter(LAN_RX_UNKNOWN_CLASS_DROP); /* only lan task call, no protection */
        return;
    }

    LAN_AnnounceRxPktToUpperLayer(
        mref_handle_p,
        dst_mac,
        src_mac,
        tag_info,
        ether_type,
        pdu_length,
        src_unit,
        src_port,
        packet_type);
}

/* LOCAL SUBPROGRAM BODIES
 */

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_InitStackingInfo
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Initial stacking info (variable lan_stack_info)
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_InitStackingInfo(void)
{
    UI32_T my_unit_id;
    UI8_T  master_unit_id;
    UI32_T my_stacking_port;

#if (SYS_CPNT_RUNTIME_SWITCH_DIAG == TRUE)
    LAN_OM_SetMyUnitId(1);
    LAN_OM_SetMasterUnitId(1);
    LAN_OM_SetMyStackingPort(LAN_TYPE_NOT_PRESENT_STACKING_PORT);
#else
    STKTPLG_POM_GetMyUnitID(&my_unit_id);
    STKTPLG_POM_GetMasterUnitId(&master_unit_id);
    if (FALSE == STKTPLG_POM_GetSimplexStackingPort(my_unit_id, &my_stacking_port))
    {
        my_stacking_port = LAN_TYPE_NOT_PRESENT_STACKING_PORT;
    }
    LAN_OM_SetMyUnitId(my_unit_id);
    LAN_OM_SetMasterUnitId(master_unit_id);
    LAN_OM_SetMyStackingPort(my_stacking_port);
#endif
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_MREF_ReleaseReference
 *----------------------------------------------------------------------------------
 * PURPOSE  : The function is called by STA task to tell LAN doesn't need to pass
 *            STA packet to STA task.
 * INPUT    : unit   -- dummy argument(just try to fit to Broadcom's nic driver)
 *            buf    -- dummy argument(not use now)
 *            cookie -- the cookie that we pass when calling send packet function
 *                      it must be a L_MM_Mref_Handle_T* type
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function is just used for Broadcom NIC drvier to match arguments.
 * ----------------------------------------------------------------------------------*/
void LAN_MREF_ReleaseReference(I32_T unit, void *packet, void *cookie)
{
    L_MM_Mref_Release((L_MM_Mref_Handle_T**)&cookie);
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_BufferAdjustment
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Check if the LAN buffer prepared by upper layer is enough.
 *            If the preallocate header is not enough, this routine will
 *            do some adjustment for memory movement if the total buffer
 *            length is enough.
 * INPUT    : mref          -- memory reference of input packet
 *            pdu_length    -- length of pdu
 *            header_length -- length of header
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_BufferAdjustment(L_MM_Mref_Handle_T **mref_handle_pp, UI32_T pdu_length, UI32_T header_length)
{
    L_MM_Mref_Handle_T *mref_handle_p = *mref_handle_pp;
    UI32_T  task_id, size_before_pdu;
    char    task_name[80];
    BOOL_T  ret_val = TRUE;

    size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(mref_handle_p);
    if(size_before_pdu < header_length)
    {
        ret_val = L_MM_Mref_ShiftPdu(mref_handle_p, (header_length-size_before_pdu));
    }

    /* allocate new buffer if original size is not enough
     */
    if (!ret_val)
    {
        L_MM_Mref_Handle_T *new_mref_handle_p;
        void *new_pdu, *pdu;
        UI32_T pdu_len, new_buf_len;

        new_buf_len = pdu_length + header_length - SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN;
        new_mref_handle_p = L_MM_AllocateTxBuffer(new_buf_len, L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_BUFFERADJUSTMENT));

        if ((ret_val = (new_mref_handle_p != NULL)))
        {
            size_before_pdu = L_MM_Mref_GetAvailSzBeforePdu(new_mref_handle_p);
            L_MM_Mref_MovePdu(new_mref_handle_p, (header_length - size_before_pdu), &pdu_len);

            /* pdu_len of new_mref_handle_p shall be equal to pdu_length
             */
            new_pdu = L_MM_Mref_GetPdu(new_mref_handle_p, &pdu_len);
            pdu = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
            memcpy(new_pdu, pdu, pdu_length);

            L_MM_Mref_Release(&mref_handle_p);
            *mref_handle_pp = mref_handle_p = new_mref_handle_p;
        }
    }

    if(ret_val==TRUE)
    {
        return TRUE;
    }

    task_id = SYSFUN_TaskIdSelf();
    if (SYSFUN_TaskIDToName(task_id, task_name, 80)!=SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("LAN_BufferAdjustment: Task %s pre allocate bytes is not enough!\n", (char *)task_name);
    }
    else
    {
        SYSFUN_Debug_Printf("LAN_BufferAdjustment: Task %lu pre allocate bytes is not enough!\n", (unsigned long)task_id);
    }
    return FALSE;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ClassifyPkt
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Classify the packet into pre-defined type
 * INPUT    : frame  -- rx packet pointer
 *            number_of_tag -- number of vlan tag
 *            mdata -- multicast data
 * OUTPUT   : None
 * RETURN   : One of the PacketType_T
 * NOTE     : 1. All variables are compare in network order except variable
 *               packet_frame_type_value which is in host order
 * ----------------------------------------------------------------------------------*/
static LAN_TYPE_PacketClass_T LAN_ClassifyPkt(RxFrame_T *frame, UI32_T number_of_tag, BOOL_T *mdata)
{
    RxFrame_T   *shifted_frame;
    UI16_T      packet_frame_type;
    UI16_T      packet_frame_type_value;
    UI32_T dip;

    /* frame->xxx is using for access DA, SA
     * other fields use shifted_frame->xxx to access
     */
    if( number_of_tag == 2 ) /* double tags */
    {
        shifted_frame = (RxFrame_T*)((UI8_T*)frame+4);
    }
    else if( number_of_tag == 1 ) /* single tag */
    {
        shifted_frame = frame;
    }
    else /* tag_number == 0, no tag */
    {
        shifted_frame = (RxFrame_T*)((UI8_T*)frame-4);
    }

    packet_frame_type = shifted_frame->frame_type;

    packet_frame_type_value = L_STDLIB_Ntoh16(packet_frame_type);

    if ((frame->sa.word1 & L_STDLIB_Hton16(0x0100)) == L_STDLIB_Hton16(0x0100)) /* multicast SA */
    {
        LAN_OM_IncreaseBackdoorCounter(LAN_MULTICAST_SA_ERROR);
        return LAN_TYPE_UNKNOWN_PACKET;
    }

    if ((0 == frame->sa.word1) && (0 == frame->sa.word2) && (0 == frame->sa.word3)) /* zero SA */
    {
        LAN_OM_IncreaseBackdoorCounter(LAN_ZERO_SA_ERROR);
        return LAN_TYPE_UNKNOWN_PACKET;
    }

#if (SYS_CPNT_CFM == TRUE)
    /*put before the protocol which DA start from 0x0180c200, because CCM and LBM use the same addr
      but LBM is unicast, so wee only check the ether type*/
    if (ETHER_CFM_TYPE == packet_frame_type)
    {
        return LAN_TYPE_CFM_PACKET;
    }
#endif /* (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
    if (ETHER_PPPOED_TYPE == packet_frame_type)
    {
        return LAN_TYPE_PPPOED_PACKET;
    }
#endif /* (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
    if (ETHER_DOT1X_TYPE == packet_frame_type)
    {
        return LAN_TYPE_DOT1X_PACKET;
    }
#endif /* (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
    if (ETHER_EFM_OAM_LOOPBACK_TYPE == packet_frame_type)
    {
        /* 802.3ah EFM OAM Loopback
         * OAM Loopback type: 0x9000
         */
        return LAN_TYPE_OAM_LOOPBACK_PACKET;
    }
#endif
#if (SYS_CPNT_SYNCE == TRUE)
    if(shifted_frame->dsap == LAN_DSAP_TYPE_ESMC)
    {
        return LAN_TYPE_ESMC_PACKET;
    }
#endif

    /* BPDU(STA)/GVRP/LACP/LLDP packets have same first two words
     */
    if ((LAN_ADDR_BPDU_WORD1 == frame->da.word1) &&
        (LAN_ADDR_BPDU_WORD2 == frame->da.word2))
    {
        if (LAN_ADDR_BPDU_WORD3 == frame->da.word3) /* BPDU */
        {
            if (packet_frame_type_value <= LAN_MAX_PDU_SIZE)
            {
                return LAN_TYPE_STA_PACKET;
            }
        }
        else if (LAN_ADDR_LACP_WORD3 == frame->da.word3) /* LACP */
        {
            if (ETHER_LACP_TYPE == packet_frame_type)
            {
#if (SYS_CPNT_LACP == TRUE)
                if (shifted_frame->dsap == LACP_DSAP_TYPE ||
                    shifted_frame->dsap == MARKER_DSAP_TYPE)
                {
                    return LAN_TYPE_LACP_PACKET;
                }
#endif /* (SYS_CPNT_LACP == TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
                if (shifted_frame->dsap == OAM_DSAP_TYPE)
                {
                    return LAN_TYPE_OAM_PACKET;
                }
#endif /* (SYS_CPNT_EFM_OAM == TRUE) */
            }
        }

#if (SYS_CPNT_LLDP == TRUE)
        else if (LAN_ADDR_LLDP_WORD3 == frame->da.word3) /* LLDP */
        {
            if (ETHER_LLDP_TYPE == packet_frame_type)
            {
                return LAN_TYPE_LLDP_PACKET;
            }
        }
#endif /* (SYS_CPNT_LLDP == TRUE) */
    }
    else if (ETHER_ARP_TYPE  == packet_frame_type || /* ARP packet  */
             ETHER_RARP_TYPE == packet_frame_type)   /* RARP packet */
    {
        return LAN_TYPE_IP_PACKET;
    }
    else if (ETHER_IPV4_TYPE   == packet_frame_type) /* IP packet   */
    {
        /*multicast data packet, exclude protocol packet*/
        if ((L_STDLIB_Hton16(0x0100) == frame->da.word1)
          && ((frame->da.word2 & L_STDLIB_Hton16(0xff00)) == L_STDLIB_Hton16(0x5e00))
          && (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_PROTO] != L_PROTO_IGMP)
          && (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_PROTO] != L_PROTO_PIM))
        {
            dip = (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_DIP + 0] << 24) |
                    (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_DIP + 1] << 16) |
                    (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_DIP + 2] << 8) |
                    (shifted_frame->pkt_buf[L_PROTO_IP_HDR_OFFSET_DIP + 3]) ;

            /*ignore reserved multicast ip addr*/
            if (dip > L_PROTO_MAX_LOCAL_GROUP)
                *mdata = TRUE;
        }

        return LAN_TYPE_IP_PACKET;
    }
#if (SYS_CPNT_IPV6 == TRUE)
    else if (ETHER_IPV6_TYPE == packet_frame_type) /* IPV6 packet */
    {
        /*multicast data packet, exclude protocol packet*/
        if ((L_STDLIB_Hton16(0x3333) == frame->da.word1)
          && (shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_PROTO] == 0
          || shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_PROTO]  == L_PROTO_UDP
          || shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_PROTO]  == L_PROTO_TCP))
        {
            if(shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_DIP]==0xff
               &&shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_PROTO]==0x11
               &&!((shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_UDP_OFFSET_DST_PORT]==0x01 /*ptp*/
                   &&shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_UDP_OFFSET_DST_PORT+1]==0x3f)
                  ||(shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_UDP_OFFSET_DST_PORT]==0x01  /*ptp*/
                     &&shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_UDP_OFFSET_DST_PORT+1]==0x40)
                 )
               )
            {
                if((shifted_frame->pkt_buf[L_PROTO_IPV6_HDR_OFFSET_DIP+1]&0x10) == 0)
                {
                    /*This is a well-known multicast data, should be forwarded by mldsnp, do not do rate limit*/
                }
                else
                {
                    *mdata = TRUE;
                }
            }
        }

        return LAN_TYPE_IP_PACKET;
    }
#endif
    else if ((L_STDLIB_Hton16(0x0100) == frame->da.word1) && ((frame->da.word2 & L_STDLIB_Hton16(0xff00)) == L_STDLIB_Hton16(0x5e00))) /* multicast but not ETHER_IP_TYPE */
    {
        return LAN_TYPE_IP_PACKET;
    }

#if (SYS_CPNT_DOT1X == TRUE)
    else if (ETHER_DOT1X_TYPE == packet_frame_type) /* unicast dot1x packet */
    {
        return LAN_TYPE_DOT1X_PACKET;
    }
#endif /* (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_CLUSTER == TRUE)
    else if ((frame->da.word1 == LAN_ADDR_CLUSTER_WORD1) &&
             (frame->da.word2 == LAN_ADDR_CLUSTER_WORD2) &&
             (frame->da.word3 == LAN_ADDR_CLUSTER_WORD3) &&
             (packet_frame_type_value < CLUSTER_MAX_LENGTH))
    {
        if ((CLUSTER_DSAP_TYPE == shifted_frame->dsap) &&
            (CLUSTER_SSAP_TYPE == shifted_frame->ssap) &&
            (CLUSTER_CTRL_TYPE == shifted_frame->ctrl) &&
            (CLUSTER_ORG_BYTE1 == shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET]) &&
            (CLUSTER_ORG_BYTE2 == shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET+1]) &&
            (CLUSTER_ORG_BYTE3 == shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET+2]))
        {
            UI16_T frame_snap_type;

            memcpy(&frame_snap_type,&(shifted_frame->raw_data[LAN_TAG_FRAME_SNAP_TYPE_OFFSET]),2);/* data alignment issue */
            switch(frame_snap_type)
            {
    #if (SYS_CPNT_CLUSTER == TRUE)
            case CLUSTER_SNAP_TYPE:
                return LAN_TYPE_CLUSTER_PACKET;
    #endif
            default:
                ;
            }
        }
    }
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_MLAG == TRUE)
    else if ((frame->da.word1 == LAN_ADDR_ORG_SPEC3_WORD1) &&
             (frame->da.word2 == LAN_ADDR_ORG_SPEC3_WORD2) &&
             (frame->da.word3 == LAN_ADDR_ORG_SPEC3_WORD3))
    {
        if ((shifted_frame->dsap == SNAP_DSAP_TYPE) &&
            (shifted_frame->ssap == SNAP_SSAP_TYPE) &&
            (shifted_frame->ctrl == SNAP_CTRL_TYPE) &&
            (shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET] == SNAP_ORG_BYTE1) &&
            (shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET+1] == SNAP_ORG_BYTE2) &&
            (shifted_frame->raw_data[LAN_TAG_FRAME_ORG_OFFSET+2] == SNAP_ORG_BYTE3))
        {
            UI16_T frame_snap_type;

            memcpy(&frame_snap_type,&(shifted_frame->raw_data[LAN_TAG_FRAME_SNAP_TYPE_OFFSET]),2);/* data alignment issue */
            switch(frame_snap_type)
            {
    #if (SYS_CPNT_MLAG == TRUE)
            case SNAP_TYPE_MLAG:
                return LAN_TYPE_MLAG_PACKET;
    #endif
            }
        }
    }
#endif /* #if (SYS_CPNT_MLAG == TRUE) */

#if (SYS_CPNT_ITRI_MIM == TRUE)
    /* all unclassifiable unicast belongs to ITRI MIM frames
     */
    if ((frame->da.addr[0] & 0x1) == 0x0)
    {
        return LAN_TYPE_ITRI_MIM_PACKET;
    }
#endif /* (SYS_CPNT_ITRI_MIM == TRUE) */

    return LAN_TYPE_UNKNOWN_PACKET;
}

/*
avoid sending multi same multicast data packets to upper layer in a short period
dev_nic_mflt, keep hash bitmap, one packet recv, set the hash bit. nic task clear the bitmap every 10 ticks
return true, drop this packet
return faluse, send this packet to upper layer, igmp snooping and ip stack...
*/
static BOOL_T
LAN_MdataRateCtrl(RxFrame_T *frame, struct dev_nic_mdata *dev_nic_mflt)
{
  if (!dev_nic_mflt)
      return FALSE;

  if (MDATA_BITMAP_ISSET_INDEX((dev_nic_mflt->dnic_mdata), DEV_NIC_MDATA_HASH_COMP(frame), ((frame)->frame_tag_info&0x0fff)))
      return TRUE;/*drop this packet*/

  MDATA_BITMAP_SET_INDEX((dev_nic_mflt->dnic_mdata), DEV_NIC_MDATA_HASH_COMP(frame), ((frame)->frame_tag_info&0x0fff));

  return FALSE;
}

/*fuzhimin,20090505*/
#if 0
#if(SYS_CPNT_DHCPSNP == TRUE)
static BOOL_T LAN_isDhcpPkt(L_MM_Mref_Handle_T *mref_handle_p)
{

typedef struct LAN_Ipv4PktFormat_S
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
} __attribute__((packed, aligned(1)))LAN_Ipv4PktFormat_T;
typedef struct LAN_Udphdr_S
{
        UI16_T  uh_sport;           /* source port       */
        UI16_T  uh_dport;           /* destination port  */
        UI16_T  uh_ulen;            /* udp length        */
        UI16_T  uh_sum;             /* udp checksum      */
} __attribute__((packed, aligned(1)))LAN_Udphdr_T;

#define LAN_IPPROTO_UDP         17          /* user datagram protocol       */
#define LAN_BOOTP_PORT_S        67          /* Bootp server udp port number */
#define LAN_BOOTP_PORT_C        68          /* Bootp client udp port number */


    UI32_T pdu_len;
    LAN_Ipv4PktFormat_T *iphdr = NULL;
    LAN_Udphdr_T        *udp_header_p = NULL;
    UI16_T              udp_dstport;

    if((iphdr=(LAN_Ipv4PktFormat_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len))==NULL)
    {
        SYSFUN_Debug_Printf("%s[%d] : L_MM_Mref_GetPdu fail\n", __FUNCTION__, __LINE__);
        return FALSE;
    }
    if (iphdr->protocol == LAN_IPPROTO_UDP)
    {
        udp_header_p = (LAN_Udphdr_T *)((char *)iphdr+sizeof(LAN_Ipv4PktFormat_T));
        udp_dstport = L_STDLIB_Ntoh16(udp_header_p->uh_dport);
        if((udp_dstport == LAN_BOOTP_PORT_C) || (udp_dstport == LAN_BOOTP_PORT_S))
        {
            return TRUE;
        }
    }
    return FALSE;
}
#endif
#endif
/*fuzhimin,20090505,end*/

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_GetPortTpid
 * ----------------------------------------------------------------------------------
 * PURPOSE  : get TPID of the specified port.
 * INPUT    : unit, port
 * OUTPUT   : None
 * RETURN   : TPID (host order)
 * NOTE     : for slave unit, can only get local port info.
 * ----------------------------------------------------------------------------------*/
static UI32_T LAN_GetPortTpid(UI32_T unit, UI32_T port)
{
    return SYS_DFLT_DOT1Q_PORT_TPID_FIELD;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_CorrectIngressPacketInfo
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Correct packet info according to upper layer CSC's config
 * INPUT    : rx_is_tagged
 *            mref_handle_p
 *            dst_mac
 *            src_mac
 *            tag_info_p
 *            inner_tag_info_p
 *            type
 *            unit
 *            port
 * OUTPUT   : mref_handle_p
 *            tag_info_p
 *            inner_tag_info_p
 * RETURN   : TRUE if one of parameters has been changed.
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_CorrectIngressPacketInfo(
    BOOL_T              rx_is_tagged,
    BOOL_T              workaround_vlan_xlate,
    L_MM_Mref_Handle_T  *mref_handle_p,
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              *tag_info_p,
    UI16_T              *inner_tag_info_p,
    UI16_T              type,
    UI32_T              unit,
    UI32_T              port)
{
    if (!rx_is_tagged)
    {
#if (SYS_CPNT_MAC_VLAN == TRUE && SYS_CPNT_MAC_VLAN_IMPLEMENTED_BY_RULE== TRUE )
        if (VLAN_OM_CorrectIngressVidFromMacBasedVlan(src_mac, tag_info_p))
        {
            if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX) && !LAN_OM_IsBackdoorPortFilter(unit, port))
                BACKDOOR_MGR_Printf("RX correct: MAC_VLAN [vid]%hu\r\n",  (*tag_info_p & 0x0fff));
            return TRUE;
        }
#endif
    }

    return FALSE;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_CorrectEgressPacketInfo
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Correct packet info according to upper layer CSC's config
 * INPUT    : dst_mac
 *            src_mac
 *            tag_info_p
 *            inner_tag_info_p
 *            type
 *            unit
 *            port
 * OUTPUT   : mref_handle_p
 *            tag_info_p
 * RETURN   : TRUE if one of parameters has been changed.
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_CorrectEgressPacketInfo(
    UI8_T               dst_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI8_T               src_mac[SYS_ADPT_MAC_ADDR_LEN],
    UI16_T              *tag_info_p,
    UI16_T              *inner_tag_info_p,
    UI16_T              type,
    UI32_T              unit,
    UI32_T              port)
{
    return FALSE;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ProcessPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Classify and then announce packet to upper layer (or send by ISC)
 * INPUT    : unit          -- logical source unit number
 *            port          -- logical source port number (include SYS_ADPT_STACKING_PORT)
 *            cookie        -- not used
 *            reason        -- reason of the packet trap to CPU
 *            mref_handle_p -- memory reference of receive packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_ProcessPacket(UI32_T unit, UI32_T port,  UI32_T *cookie, UI32_T reason, L_MM_Mref_Handle_T *mref_handle_p)
{
    UI32_T     packet_type;
    UI32_T     ether_header_length = 18;
    UI16_T     packet_frame_type;
    UI16_T     tag_info;
    UI16_T     inner_tag_info;
    RxFrame_T  *frame;
    UI32_T     pdu_len;
    UI32_T     number_of_tag;
    UI8_T      my_mac[SYS_ADPT_MAC_ADDR_LEN];
    BOOL_T     has_tag;
    UI64_T     rx_timestamp = 0;
    UI32_T     ingress_vlan = 0;
    BOOL_T     is_single_inner_tagged = FALSE;
    BOOL_T     rx_is_tagged = TRUE;
    BOOL_T     pkt_is_truncated = FALSE;
    BOOL_T     workaround_vlan_xlate = FALSE;
    BOOL_T     pkt_info_is_corrected = FALSE;

    BOOL_T mdata = FALSE;/*recv packet is multicast data*/
    struct dev_nic_mdata *mhash = NULL;

#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
    UI32_T     workaround_reason = DEV_NICDRV_LIB_ReceivePacketReason(reason);
    #define DEV_NICDRV_LIB_ReceivePacketReason(reason)  workaround_reason
#endif

    /* extract extra packet info from cookie
     */
    if (cookie && *(UI32_T *)cookie == DEV_NICDRV_RX_PACKET_INFO_MAGIC_WORD)
    {
        DEV_NICDRV_RxPacketInfo_T *pkt_info_p = (DEV_NICDRV_RxPacketInfo_T *)cookie;

        rx_timestamp = pkt_info_p->rx_timestamp;
        ingress_vlan = pkt_info_p->ingress_vlan;
        is_single_inner_tagged = pkt_info_p->is_single_inner_tagged;
        rx_is_tagged = pkt_info_p->rx_is_tagged;
        pkt_is_truncated = pkt_info_p->pkt_is_truncated;
        workaround_vlan_xlate = pkt_info_p->workaround_vlan_xlate;
        cookie = pkt_info_p->cookie;
        mhash = pkt_info_p->mhash;
    }
    else
    {
        mhash = (struct dev_nic_mdata *)cookie;
    }

    frame = (RxFrame_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (NULL == frame)
    {
        printf("%s: L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX) && !LAN_OM_IsBackdoorPortFilter(unit, port))
    {
        LAN_BackdoorPrintRxDebug(frame, unit, port, pdu_len, reason);
    }

    /* check whether packet contain outer tag
     * note that if tpid = 0x8100 in double tag mode, a single 0x8100 type
     * tag info will be classified as outer tag
     *
     * NOTE:
     *   always use 0x8100 to identify outer tag
     */
    if (!is_single_inner_tagged &&
        frame->frame_tag_type == ETHER_TAG_TYPE)
    {
        has_tag       = TRUE;
        number_of_tag = 1;
    }
    else
    {
        has_tag       = FALSE;
        number_of_tag = 0;
    }

    LAN_OM_GetMyMac(my_mac);

    packet_type = LAN_ClassifyPkt(frame,number_of_tag, &mdata);

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
    /* If the port is in internal loopback mode,
     * all packet from this port will be classified
     * as internal loopback packet and trap to swctrl
     */
    if (TRUE == LAN_OM_GetInternalLoopback(unit, port))
    {
        packet_type = LAN_TYPE_INTERNAL_LOOPBACK_PACKET;
    }
    else
#endif
#if (SYS_CPNT_SFLOW == TRUE)
    if ((DEV_NICDRV_LIB_ReceivePacketReason(reason) & LAN_TYPE_RECV_REASON_INGRESS_SAMPLED))
    {
        packet_type = LAN_TYPE_SFLOW_PACKET;
    }
    else
#endif
    {
        /*avoid send multi same multicast data to upper layer in a short period*/
        if (mdata && LAN_MdataRateCtrl(frame, mhash))
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }

#if (SYS_CPNT_EFM_OAM == TRUE)
#if (SYS_ADPT_OAM_HW_SUPPORT_LOOPBACK == TRUE)
    /* SYS_ADPT_OAM_HW_SUPPORT_LOOPBACK == TRUE, means hardware
     * support oam loopback. In this case, NIC should only
     * accept OAMPDU and drop all other packets.
     */
    if ((TRUE == LAN_OM_GetOamLoopback(unit, port))&&
        (packet_type != LAN_TYPE_OAM_PACKET))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
#else
    /* SYS_ADPT_OAM_HW_SUPPORT_LOOPBACK == FALSE, means hardware is
     * not support or partially support oam loopback. In this case,
     * NIC should only accept OAMPDU and tx back all other
     * packets in loopback mode (do software loopback).
     */
    if ((TRUE == LAN_OM_GetOamLoopback(unit, port))&&
        (packet_type != LAN_TYPE_OAM_PACKET))
    {
        if (FALSE == DEV_NICDRV_PMGR_SendPacketByPort(unit,
                                                      port,
                                                      FALSE,
                                                      frame,
                                                      pdu_len,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                                      SYS_DFLT_MYMACMYIP_PACKET_TO_CPU_PRIORITY,
#else
                                                      SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                                      (void*)LAN_MREF_ReleaseReference,
                                                      mref_handle_p))
        {
            SYSFUN_Debug_Printf("%s:DEV_NICDRV_PMGR_SendPacketByPort return false\n",__FUNCTION__);
        }
        return;
    }
#endif
#endif

    if (has_tag)
    {
        tag_info = L_STDLIB_Ntoh16(frame->frame_tag_info);
    }
    else
    {
        tag_info = ingress_vlan;
    }
    if ((number_of_tag - (has_tag ? 1 : 0)) > 0)
    {
        inner_tag_info = L_STDLIB_Ntoh16(*(UI16_T*)(&frame->raw_data[LAN_UNTAG_FRAME_HEADER_SIZE + (LAN_ONE_TAG_FIELD_SIZE * (has_tag ? 1 : 0))]));
    }
    else
    {
        inner_tag_info = 0;
    }
    ether_header_length = LAN_UNTAG_FRAME_HEADER_SIZE + (LAN_ONE_TAG_FIELD_SIZE * number_of_tag);
    packet_frame_type   = L_STDLIB_Ntoh16(*(UI16_T*)(&frame->raw_data[ether_header_length-LAN_ETHERTYPE_FIELD_SIZE]));

    #if defined(BCM56514) && 0 /* obsolete, has another solution */
    LAN_ChipWorkaround_FrameWithInnerTagOnlyReceivedFromAccessPort(unit, port, has_tag, number_of_tag, &tag_info);
    #endif

    pkt_info_is_corrected = LAN_CorrectIngressPacketInfo(
        rx_is_tagged,
        workaround_vlan_xlate,
        mref_handle_p,
        frame->da.addr,
        frame->sa.addr,
        &tag_info,
        &inner_tag_info,
        packet_frame_type,
        unit,
        port);

    /* fill pkt_info of mref
     */
    mref_handle_p->pkt_info.timestamp = rx_timestamp;
    mref_handle_p->pkt_info.inner_tag_info = inner_tag_info;
    mref_handle_p->pkt_info.pkt_is_truncated = pkt_is_truncated;
    mref_handle_p->pkt_info.rx_is_tagged = rx_is_tagged;
    mref_handle_p->pkt_info.ether_header_length = ether_header_length;
    mref_handle_p->pkt_info.rx_reason = DEV_NICDRV_LIB_ReceivePacketReason(reason);
    mref_handle_p->pkt_info.rx_unit = unit;
    mref_handle_p->pkt_info.rx_port = port;

#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
    /* if port security is enabled and reason doesn't contain
     * intrusder, do workaround.
     */
    if ((LAN_OM_GetPortSecurity(unit, port) ||
         (LAN_OM_GetPortLearning(unit, port) && LAN_OM_GetVlanLearningStatus(tag_info))) &&
        0 == (workaround_reason & (LAN_TYPE_RECV_REASON_INTRUDER | LAN_TYPE_RECV_REASON_STATION_MOVE)))
    {
        LAN_ChipWorkaround_IncorrectReasonForIntruder(
                                    frame->da.addr,
                                    frame->sa.addr,
                                    (tag_info & 0xfff),
                                    packet_frame_type,
                                    unit,
                                    port,
                                    &workaround_reason);
    }
#endif

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX) && !LAN_OM_IsBackdoorPortFilter(unit, port))
    {
        LAN_BackdoorPrintRxTagParsing(frame, ingress_vlan, rx_is_tagged, has_tag, tag_info, inner_tag_info, packet_frame_type, pkt_is_truncated);
    }

    /* Discard untagged frame if
     *   rx frame is untagged and
     *   rx packet_type bypass bridge and
     *   rx port is configured to discard untagged frame.
     *
     * NOTE
     *   here use chk_stp to indicate bypass bridge mechanism or not,
     *   might be changed if lan has new flag for bypass_bridge.
     */
    if (lan_packet_info[packet_type].chk_stp)
    {
        if (rx_is_tagged)
        {
            if (LAN_OM_GetPortDiscardTaggedFrame(unit, port))
            {
                L_MM_Mref_Release(&mref_handle_p);
                LAN_OM_IncreaseBackdoorCounter(LAN_RX_TAGGED_DROP);
                return;
            }
        }
        else
        {
            if (LAN_OM_GetPortDiscardUntaggedFrame(unit, port))
            {
                L_MM_Mref_Release(&mref_handle_p);
                LAN_OM_IncreaseBackdoorCounter(LAN_RX_UNTAGGED_DROP);
                return;
            }
        }
    }

    /* --------------pseudo code to support AMTR software learn ----------
     *  [NICDRV in Slave Unit]
     *
     *  if  ( protocol packet )
     *  {
     *      Relay Protocol packet to Master
     *  }
     *  else
     *  {
     *      if (NA or Port-Move)
     *      {
     *          notify AMTR to put in NA buffer
     *      }
     *      Drop
     *  }
     *
     *  [NICDRV in Master Unit]
     *
     *  if (NA or Port-Move)
     *  {
     *      return_value = Notify AMTR
     *      if (return_value == TRUE)
     *      {
     *          Drop
     *          continue
     *      }
     *  }
     *  if (!protocol)
     *  {
     *      Drop
     *  }
     *  AnnounceProtocol to upper layer
     */

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
#if (LAN_SYS_CALLBACK_FOR_AUTH_PACKET != TRUE)
        if (packet_type >= LAN_TYPE_UNKNOWN_PACKET)
        {
            if (DEV_NICDRV_LIB_ReceivePacketReason(reason) & (LAN_TYPE_RECV_REASON_INTRUDER | LAN_TYPE_RECV_REASON_STATION_MOVE))
            {
                if (lan_announce_na_and_security_check_callback != NULL)
                {
                    lan_announce_na_and_security_check_callback(frame->da.addr,
                                                                frame->sa.addr,
                                                                (tag_info & 0xfff),
                                                                packet_frame_type,
                                                                unit,
                                                                port);
                }
            }
            L_MM_Mref_Release(&mref_handle_p);
            LAN_OM_IncreaseBackdoorCounter(LAN_RX_UNKNOWN_CLASS_DROP); /* only lan task call, no protection */
            return;
        }
        else
#endif /* (LAN_SYS_CALLBACK_FOR_AUTH_PACKET != TRUE) */
        {
#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
            if (packet_type == LAN_TYPE_INTERNAL_LOOPBACK_PACKET)
            {
                LAN_SendPacketToRemoteUnit(LAN_ISC_LOOPBACK_PACKET,mref_handle_p, port,
                                           pdu_len, reason,
                                           LAN_UNIT_BMP(LAN_OM_GetMasterUnitId()),
                                           NULL, NULL, NULL);
            }
            else
#endif
            {
                /* In slave mode, send rx protocol packet to master by ISC.
                 */
                LAN_SendPacketToRemoteUnit(LAN_ISC_RX_TX_PACKET,mref_handle_p, port,
                                           pdu_len, reason,
                                           LAN_UNIT_BMP(LAN_OM_GetMasterUnitId()),
                                           NULL, NULL, NULL);
            }
        }
    }
    else /* if master */
    {
        L_MM_Mref_MovePdu(mref_handle_p, ether_header_length, &pdu_len);

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
        /* If it is internal loopback packet, no need to make the following
         * checking and dispatch to swctrl directly
         */
        if (packet_type == LAN_TYPE_INTERNAL_LOOPBACK_PACKET)
        {
            LAN_AnnounceRxPktToUpperLayer(mref_handle_p, frame->da.addr, frame->sa.addr, tag_info,
                                          packet_frame_type, pdu_len, unit, port, packet_type);
            return;
        }
#endif

        LAN_OM_GetMyMac(my_mac);
#if (SYS_CPNT_MAINBOARD == TRUE)

#ifdef ASF4526B_FLF_P5
    if(!(DEV_NICDRV_LIB_ReceivePacketReason(reason) &
          (LAN_TYPE_RECV_REASON_INTRUDER | LAN_TYPE_RECV_REASON_STATION_MOVE) ) )
    {
#endif

        /* 2005/03/28 phoebe modified for EPR ES4649-ZZ-01103
         * if the packet need workaround(da==sa==my_mac), don't call LAN_BlockingPortBufferFreed
         * since SYS_ADPT_STACKING_PORT is a constant, not a legal port and it needn't have
         * to check it is in blocking mode or not
         */
        if(!((memcmp(my_mac, &frame->da, SYS_ADPT_MAC_ADDR_LEN)==0) &&
             (memcmp(my_mac, &frame->sa, SYS_ADPT_MAC_ADDR_LEN)==0) &&
             (port != LAN_OM_GetMyStackingPort())))
        {
            if (LAN_BlockingPortBufferFreed(tag_info, unit, port, packet_type) == TRUE)
            {
                L_MM_Mref_Release(&mref_handle_p);
                LAN_OM_IncreaseBackdoorCounter(LAN_RX_BLOCKING_PORT_DROP);
                return;
            }
        }

#ifdef ASF4526B_FLF_P5
    }
#endif

#endif

        /*L3 protocol won't process itself send out packet */
        if(LAN_TYPE_IP_PACKET == packet_type)
        {
            if(0 == memcmp(my_mac, frame->sa.addr, SYS_ADPT_MAC_ADDR_LEN))
            {
                 if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX))
                     BACKDOOR_MGR_Printf ("Recieve packet but src mac is the same as this switch cpu mac\r\n");

                 L_MM_Mref_Release(&mref_handle_p);
                return;
            }
        }

        /* we check the announce NA and security check callback function is NULL
         * or not instead of use #ifdefine SYS_CPNT_INTRUSION_MSG_TRAP at below
         * to determind whether support security check in upper layer
         */
        /* EPR: ES3510MA-FLF-38-00592    (2012/7/19)
         * Problem Description:
         *     PC with Win XP in MAC vlan and dhcp snooping doesn't receive IP address from DHCP server
         * Root Cause:
         *     Pkts to cpu by rule can't match the rule of mac vlan,
         *     so SA will not learn on correct vlan.
         *     DHCPSNP cant work well without correct mac learnt.
         * Solution:
         *     In LAN, all the pkts that match mac vlan config will
         *     be taken as NA pkts, i.e. LAN will notify these pkts
         *     to AMTR and security CSC to determine if learn/drop
         *     them or not. A correct mac entry will be written to
         *     chip if needed.
         */
        if (!lan_packet_info[packet_type].bypass_auth)
        {
#if (LAN_SYS_CALLBACK_FOR_AUTH_PACKET == TRUE)
            if (pkt_info_is_corrected ||
                (DEV_NICDRV_LIB_ReceivePacketReason(reason)&
                (LAN_TYPE_RECV_REASON_INTRUDER | LAN_TYPE_RECV_REASON_STATION_MOVE)))
            {
                SYS_CALLBACK_MGR_AuthenticatePacket(
                    SYS_MODULE_LAN,
                    mref_handle_p,
                    frame->da.addr,
                    frame->sa.addr,
                    tag_info,
                    packet_frame_type,
                    pdu_len,
                    unit,
                    port,
                    packet_type);

                return;
            }
#else /* (LAN_SYS_CALLBACK_FOR_AUTH_PACKET == TRUE) */
            if ((lan_announce_na_and_security_check_callback != NULL) &&
                (pkt_info_is_corrected ||
                (DEV_NICDRV_LIB_ReceivePacketReason(reason)&
                (LAN_TYPE_RECV_REASON_INTRUDER | LAN_TYPE_RECV_REASON_STATION_MOVE))))
            {
                 /* if lan_announce_na_and_security_check_callback() return TRUE, implies
                  * this packet is an intrusion packet and LAN has to free it, otherwise
                  * LAN can do advance processing.
                  */


                 if (TRUE == lan_announce_na_and_security_check_callback(frame->da.addr,
                                                                         frame->sa.addr,
                                                                         (tag_info & 0xfff),
                                                                         packet_frame_type,
                                                                         unit,
                                                                         port))
                 {
                     L_MM_Mref_Release(&mref_handle_p);
                     LAN_OM_IncreaseBackdoorCounter(LAN_RX_INTRUSION_DROP);
                     return;
                 }
            }
#endif /* (LAN_SYS_CALLBACK_FOR_AUTH_PACKET == TRUE) */
        }

        /*
         * if the packet is a IP packet, and it destination MAC address is unicast address,
         * and it is not my MAC, then drop it
         */
#if 0
        if (   (packet_type == LAN_TYPE_IP_PACKET)
            && (!(frame->da.addr[0] & 0x1))
            && (memcmp(my_mac, &frame->da, SYS_ADPT_MAC_ADDR_LEN) != 0))
        {
/*fuzhimin,20090505*/
#if(SYS_CPNT_DHCPSNP == TRUE)
            /*get global dhcp snooping status, and check if it's dhcp pkt*/
            DHCPSNP_PMGR_GetGlobalDhcpSnoopingStatus(&dhcpsnp_status);
            isDhcpPkt = LAN_isDhcpPkt(mref_handle_p);
            if((dhcpsnp_status == TRUE)&&(isDhcpPkt))
            {
                /*do nothing*/
            }
            else
            {
                L_MM_Mref_Release(&mref_handle_p);
                LAN_OM_IncreaseBackdoorCounter(LAN_RX_INTRUSION_DROP);
                return;
            }
#else
/*fuzhimin,20090505,end*/
            L_MM_Mref_Release(&mref_handle_p);
            LAN_OM_IncreaseBackdoorCounter(LAN_RX_INTRUSION_DROP);
            return;
#endif
        }
#endif

        if (packet_type >= LAN_TYPE_UNKNOWN_PACKET)
        {

            L_MM_Mref_Release(&mref_handle_p);
            LAN_OM_IncreaseBackdoorCounter(LAN_RX_UNKNOWN_CLASS_DROP); /* only lan task call, no protection */
            return;
        }

        if (port == LAN_OM_GetMyStackingPort())
        {
            port = SYS_ADPT_STACKING_PORT;
        }

        LAN_AnnounceRxPktToUpperLayer(mref_handle_p, frame->da.addr, frame->sa.addr, tag_info,
                                      packet_frame_type, pdu_len, unit, port, packet_type);
    }

#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
    #undef DEV_NICDRV_LIB_ReceivePacketReason
#endif

    return;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RecvPackcet
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Whenever DEV_NICDRV receive a packet it will call this function
 *            to handle the packet (pass as a memory reference type)
 * INPUT    : unit          -- logical source unit number
 *            port          -- logical source port number (include SYS_ADPT_STACKING_PORT)
 *            cookie        -- not used
 *            reason        -- reason of the packet trap to CPU
 *            mref_handle_p -- memory reference of receive packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function is registered to DEV_NICDRV as a client
 *            2. Packet must be freed in this function
 * ----------------------------------------------------------------------------------*/
void LAN_RecvPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, L_MM_Mref_Handle_T *mref_handle_p)
{
    if (NULL == mref_handle_p)
    {
        SYSFUN_Debug_Printf("%s:mref_handle_p == NULL\n",__FUNCTION__);
        return;
    }

    if (((0 == port) || (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)) && (port != SYS_ADPT_STACKING_PORT))
    {
        printf("%s:invalid port=[%lu]\n",__FUNCTION__,(unsigned long)port);
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

#if (SYS_CPNT_RUNTIME_SWITCH_DIAG == TRUE)

    /*call back when doing external loopback - added by mikeliu.110904*/
    if (TRUE == lan_debug_rx_int)
    {
        UI8_T    *pdu;
        UI32_T   pdu_len;

        pdu = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (NULL != pdu)
        {
            lan_announce_pkt_debug_callback(port, pdu, pdu_len);
        }
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
#endif

    LAN_OM_IncreaseBackdoorCounter(LAN_RX_PACKET_TOTAL);

    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_DROP) && !LAN_OM_IsBackdoorPortFilter(unit, port))
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }

    LAN_ProcessPacket(unit, port, cookie, reason, mref_handle_p);

    return;
}


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
BOOL_T LAN_GetStackingHeaderLen(UI16_T *length)
{
    if (NULL == length)
        return FALSE;

    *length = (UI16_T)sizeof(StackingHeader_T);
    return TRUE;
}


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
BOOL_T LAN_GetEthHeaderLen(BOOL_T untagged, UI16_T *length)
{
    if (NULL == length)
        return FALSE;

    if (untagged == TRUE)
        *length = (UI16_T)LAN_UNTAG_FRAME_HEADER_SIZE;
    else
        *length = (UI16_T)(LAN_UNTAG_FRAME_HEADER_SIZE + LAN_ONE_TAG_FIELD_SIZE);

    return TRUE;
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_AnnounceRxPktToUpperLayer
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Anounce packet to upper layer according to their class
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
 * RETURN   : TRUE  -- announce success
 *            FALSE -- announce fail
 * NOTE     : 1. array type_to_module_id is set according to
 *               enum{LAN_IP_PACKET = 0, LAN_STA_PACKET, LAN_GVRP_PACKET, LAN_LACP_PACKET,
 *                    LAN_DOT1X_PACKET, LAN_LLDP_PACKET, LAN_CLUSTER_PACKET, LAN_MAX_PACKET_TYPE} PacketType_T
 *            2. no need to check packet_type>=LAN_MAX_PACKET_TYPE since this function
 *               is call in master mode, and this type of packet will be drop before call
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_AnnounceRxPktToUpperLayer(L_MM_Mref_Handle_T *mref_handle_p, UI8_T *dst_mac, UI8_T *src_mac,
                                            UI16_T tag_info, UI16_T ether_type, UI32_T pdu_length,
                                            UI32_T src_unit, UI32_T src_port, UI32_T packet_type)
{
    /* SetMyId and SetNextId is for L_MREF debugging purpose
     */
    mref_handle_p->current_usr_id = SYS_MODULE_LAN;
    mref_handle_p->next_usr_id    = lan_packet_info[packet_type].module_id;

    if (lan_packet_info[packet_type].announce_cb != NULL)
    {
        if(!lan_packet_info[packet_type].announce_cb(SYS_MODULE_LAN,mref_handle_p, dst_mac, src_mac, tag_info, ether_type,
                                               pdu_length, src_unit, src_port, packet_type))
            L_MM_Mref_Release(&mref_handle_p);
        return TRUE;
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_SendPacketToRemoteUnit
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This function is used for LAN to send packet to remote unit via ISC.
 * INPUT    : service_id             -- lan serivce id, to identify different type of packet
 *            mref_handle_p          -- a descriptor which has a pointer to indicate
 *                                      packet buffer and memory reference number
 *            port                   -- port number on remote unit.
 *            packet_length          -- the length of total ethernet frame, including header.
 *            unit                   -- remote unit number.
 *            uport_list             -- port list
 *            uport_untagged_list    -- untagged list
 *            tx_port_count_per_unit -- active port count per unit.
 * OUTPUT   : None
 * RETURN   : TRUE  -- successful
 *            FALSE -- failed
 * NOTE     : 1. This function will allocate a new buffer for ISC frame, copy ethernet frame
 *               to the buffer, construct a new memory reference for this buffer, release old
 *               memory reference and send frame to remote unit via ISC.
 *            2. This function will free mref_p by itself, caller function don't free it twice
 *            3. For master unit, port is not used, the packet send to slave unit is for send
 *               out by front port
 *            4. For slave unit, port is the src port that the packet is received
 *            5. If there is performance issue, there is a way to avoid memory copy:
 *               Increase NIC packet buffer from 1600 bytes to 1800 bytes and append
 *               LAN Stacking Header to the end.
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_SendPacketToRemoteUnit(UI32_T   service_id,
                                         L_MM_Mref_Handle_T *mref_handle_p,
                                         UI32_T   port,
                                         UI32_T   packet_length,
                                         UI32_T   reason,
                                         UI16_T   dst_bmp,
                                         UI8_T    *uport_list,
                                         UI8_T    *uport_untagged_list,
                                         UI8_T    *tx_port_count_per_unit)
{
    L_MM_Mref_Handle_T  *new_mref_handle_p;
    UI32_T              pdu_len;
    UI8_T               *frame_ptr;
    UI8_T               *ether_frame_ptr;
    StackingHeader_T    *lan_stack_hdr;
    UI32_T              cpu_queue, priority;

    /* prepare new frame buffer
     */
    new_mref_handle_p = L_MM_AllocateTxBuffer(sizeof(StackingHeader_T) + packet_length, /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_SENDPACKETTOREMOTEUNIT) /* user_id */);
    frame_ptr = L_MM_Mref_GetPdu(new_mref_handle_p, &pdu_len);
    if (NULL == frame_ptr)
    {
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    /* set pointer of LAN stack header within ISC buffer
     */
    lan_stack_hdr = (StackingHeader_T *)frame_ptr;

    /* Copy payload of LAN to ISC buffer
     */
    ether_frame_ptr = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    memcpy((UI8_T *)&frame_ptr[sizeof(StackingHeader_T)], ether_frame_ptr, packet_length);

    /* cpu_queue = (reason>>28) & 0x7; */  /* Lewis temp modified for a quick solution: assume queue#=(reason) >> 28) & 0x7 */
    cpu_queue = mref_handle_p->pkt_info.cos;
    switch (cpu_queue)
    {
        case 0:
            priority = 1;
            break;
        case 1:
            priority = 2;
            break;
        case 2:
            priority = 0;
            break;
        default:
            priority = cpu_queue;
            break;
    }

    L_MM_Mref_Release(&mref_handle_p);

    /* Prepare LAN stack header
     */
    lan_stack_hdr->service_id = service_id;
    lan_stack_hdr->src_unit   = LAN_OM_GetMyUnitId();
    lan_stack_hdr->src_port   = port;
    lan_stack_hdr->pkt_length = packet_length;
    lan_stack_hdr->reason     = reason;

    if (NULL != uport_list)
    {
        memcpy(lan_stack_hdr->uport_list, uport_list, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }

    if (NULL != uport_untagged_list)
    {
        memcpy(lan_stack_hdr->uport_untagged_list, uport_untagged_list, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    }

    if (NULL != tx_port_count_per_unit)
    {
        memcpy(lan_stack_hdr->tx_port_count_per_unit, tx_port_count_per_unit, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    }

    /* Deferred procedure mechanism is used for passing packet to BCM driver
     * from interrupt context, since semaphore cannot be taken within an ISR.
     */

#ifdef LAN_STACKING_BACKDOOR_OPEN
    if (TRUE == LAN_OM_GetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE))
    {
        printf("ISC Packet that will be sent by calling ISC_Send():\n");
        LAN_STACKING_DisplayIscMref(dst_bmp, new_mref_handle_p, priority);
    }
#endif

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        if (FALSE == ISC_Send(dst_bmp, ISC_LAN_DIRECTCALL_SID, new_mref_handle_p, priority))
        {
            SYSFUN_Debug_Printf("LAN_SendPacketToRemoteUnit:ISC_Send return false. priority:%lu\n",(unsigned long)priority);
            return FALSE;
        }
    }
    else
    {
        if (FALSE == ISC_Send(dst_bmp, ISC_LAN_CALLBYAGENT_SID, new_mref_handle_p, priority))
        {
            SYSFUN_Debug_Printf("LAN_SendPacketToRemoteUnit:ISC_Send return false. priority:%lu\n",(unsigned long)priority);
            return FALSE;
        }
    }

    return TRUE;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RecvIscPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The function is used as a callback function for ISC, when ISC has
 *            a packet for LAN.
 * INPUT    : key     -- a key(unit, seq_no) to identify the caller.
 *            mref_p  -- a descriptor which has a pointer to indicate packet buffer
 *                       and memory reference number.
 * OUTPUT   : None
 * RETURN   : NULL    -- no more space
 *            others  -- the starting address of buffer.
 * NOTE     : 1.This function will process control information from remote unit and pass
 *              ethernet frame to CPU(LAN_RecvPacket()) in master mode ,or to Nic_drv
 *              (DEV_NICDRV_SendPacketByPortList()) in slave mode.
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_RecvIscPacket(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    StackingHeader_T    *stk_header;
    UI32_T              pdu_len;
    UI8_T               *pdu_p;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        /* This unit is in Transition mode. Drop packet and return.
         */
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    /* Extract LAN stack header from ISC packet
     */
    stk_header = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len); /* The NULL test is done in LAN_RecvIscPacket */
    if (0 == stk_header->pkt_length)
    {
        SYSFUN_Debug_Printf("LAN_RecvIscPacket:pkt_length == 0\n");
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

#ifdef LAN_STACKING_BACKDOOR_OPEN
    if (TRUE == LAN_OM_GetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE))
    {
        printf("LAN Receive ISC packet:\n");
        LAN_STACKING_DisplayIscMref(0, mref_handle_p, key->priority);
    }
#endif

    /* Reset the pdu address
     */
    pdu_p = L_MM_Mref_MovePdu(mref_handle_p, sizeof(StackingHeader_T), &pdu_len);

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
#ifdef LAN_STACKING_BACKDOOR_OPEN
        if (TRUE == LAN_OM_GetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET))
        {
            LAN_STACKING_DisplayXmitPacket(mref_handle_p);
        }
#endif
        /* Extract uport list and untagged list from active port list for this unit
         *
         * uportlist :
         *           +---------+---------+------+---------+
         *  unit     |    1    |    2    | ...  |    8    |
         *           +---------+---------+------+---------+
         *  portlist | 4 bytes | 4 bytes | ...  | 4 bytes |
         *           +---------+---------+------+---------+
         */

        /* This unit is in Slave mode. Send packet directly to NIC driver.
         * Add semaphore is for packet can be send out without disturb by other send packet function
         */
        if ( !DEV_NICDRV_PMGR_SendPacketByPortList(stk_header->tx_port_count_per_unit[LAN_OM_GetMyUnitId()-1],
                                              stk_header->uport_list,
                                              stk_header->uport_untagged_list,
                                              pdu_p,
                                              stk_header->pkt_length,
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
                                              DEV_NICDRV_LIB_CosValue(LAN_PKT_HIGH_TXCOS),
#else
                                              SYS_DFLT_CPU_SEND_PACKET_PRIORITY,
#endif
                                              (void*)LAN_MREF_ReleaseReference,
                                              mref_handle_p))
        {
            SYSFUN_Debug_Printf("LAN_RecvIscPacket:DEV_NICDRV_PMGR_SendPacketByPortList return false\n");
        }
    }
    else
    {

#ifdef LAN_STACKING_BACKDOOR_OPEN
        if (TRUE == LAN_OM_GetBackdoorStackingToggle(DISPLAY_RECEIVE_PACKET))
        {
            LAN_STACKING_DisplayRecvPacket(pdu_p, stk_header->pkt_length);
        }
#endif

        /* This unit is in Master mode. Announce packet to CPU.
         */

        /*  the mref_p here have problem (from ISC differ from those from LAN)?? */
        LAN_RecvPacket(stk_header->src_unit, stk_header->src_port, NULL,
                       stk_header->reason, mref_handle_p);
     }

     return TRUE;
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_RecvIscSetting
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The function is used as a callback function for ISC, when ISC has
 *            a setting to oam/internal loopback for LAN.
 * INPUT    : key     -- a key(unit, seq_no) to identify the caller.
 *            mref_p  -- a descriptor which has a pointer to indicate packet buffer
 *                       and memory reference number.
 * OUTPUT   : None
 * RETURN   : TRUE   -- handle success
 *            FALSE  -- handle fail
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_RecvIscSetting(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    RemoteOperationHeader_T  *operation_header_p;
    UI32_T                   pdu_len;
    BOOL_T                   ret = FALSE;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        /* This unit is in Transition mode. Drop packet and return.
         */
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    operation_header_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len); /* The NULL test is done in LAN_RecvIscPacket */

    if (LAN_OM_GetMyUnitId() != operation_header_p->unit)
    {
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    switch (operation_header_p->service_id)
    {
#if (SYS_CPNT_EFM_OAM == TRUE)
        case LAN_ISC_SET_OAM_LOOPBACK:
            ret = LAN_OM_SetOamLoopback(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
        case LAN_ISC_SET_INTERNAL_LOOPBACK:
            ret = LAN_OM_SetInternalLoopback(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;
#endif

        case LAN_ISC_SET_PORT_LEARNING:
            ret = LAN_OM_SetPortLearning(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;

        case LAN_ISC_SET_PORT_SECURITY:
            ret = LAN_OM_SetPortSecurity(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;

        case LAN_ISC_SET_PORT_DISCARD_UNTAGGED_FRAME:
            ret = LAN_OM_SetPortDiscardUntaggedFrame(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;

        case LAN_ISC_SET_PORT_DISCARD_TAGGED_FRAME:
            ret = LAN_OM_SetPortDiscardTaggedFrame(operation_header_p->unit, operation_header_p->port, operation_header_p->data.enable);
            break;

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
        case LAN_ISC_SET_VLAN_LEARNING_STATUS:
            ret = LAN_OM_SetVlanLearningStatus(operation_header_p->vid, operation_header_p->data.enable);
            break;
#endif

         default:
            /* do nothing
             */
            break;
    }

    if (ret == TRUE)
    {
        LAN_SendIscReply(key, LAN_ISC_ACK);
    }
    else
    {
        LAN_SendIscReply(key, LAN_ISC_NAK);
    }

    L_MM_Mref_Release(&mref_handle_p);

    return TRUE;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_IscService_Callback
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This function is ISC callback function that register to ISC
 * INPUT    : key     -- key of ISC
 *            mref_p  -- a descriptor which has a pointer to indicate packet buffer
 *                       and memory reference number.
 *            svc_id  -- service id
 * OUTPUT   : None
 * RETURN   : TRUE    -- service id is for LAN
 *            FALSE   -- service id is not for LAN
 * NOTE     : 1.This function is register to ISC to callback when there is packet for LAN
 *              and the service id is ISC_LAN_DIRECTCALL_SID
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_IscService_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id)
{
    UI8_T   *buf;
    UI32_T  service_id, pdu_len;

    buf = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (NULL == buf)
    {
        SYSFUN_Debug_Printf("%s:L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        L_MM_Mref_Release(&mref_handle_p);
        return FALSE;
    }

    service_id = ((LanIscHeader_T*)buf)->service_id;
    switch(service_id)
    {
        case LAN_ISC_RX_TX_PACKET:
            return LAN_RecvIscPacket(key, mref_handle_p);
            break;
        case LAN_ISC_SET_OAM_LOOPBACK:
        case LAN_ISC_SET_INTERNAL_LOOPBACK:
        case LAN_ISC_SET_PORT_TPID:
        case LAN_ISC_SET_PORT_LEARNING:
        case LAN_ISC_SET_PORT_SECURITY:
        case LAN_ISC_SET_PORT_DISCARD_UNTAGGED_FRAME:
        case LAN_ISC_SET_PORT_DISCARD_TAGGED_FRAME:
        case LAN_ISC_SET_VLAN_LEARNING_STATUS:
            return LAN_RecvIscSetting(key, mref_handle_p);
            break;
        default :
            SYSFUN_Debug_Printf("LAN_IscService_Callback: unknown service_id=[%lu]\n", (unsigned long)service_id);
            L_MM_Mref_Release(&mref_handle_p);
            return FALSE;
            break;
    }
    return TRUE;
}

/* ------------------------------------------------------------------------
 * ROUTINE NAME - LAN_SendIscReply
 * ------------------------------------------------------------------------
 * FUNCTION : This function sends a ACK or NAK back to master's isc remote call
 * INPUT    : key        - key for ISC service
 *            service_id - LAN_ISC_ACK or LAN_ISC_NAK
 * OUTPUT   : None
 * RETURN   : TRUE  -- success
 *            FALSE -- fail
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static BOOL_T LAN_SendIscReply(ISC_Key_T *key, UI32_T service_id)
{
    L_MM_Mref_Handle_T   *mref_handle_p;
    LanIscHeader_T       *lan_isc_packet_p;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(LanIscHeader_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_ISCREPLY) /* user_id */
                                          );
    lan_isc_packet_p = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (NULL == lan_isc_packet_p)
    {
        SYSFUN_Debug_Printf("%s():L_MM_Mref_GetPdu return NULL\n",__FUNCTION__);
        return FALSE;
    }

    lan_isc_packet_p->service_id = service_id;

    return ISC_RemoteReply(mref_handle_p, key);
}

#if (SYS_CPNT_MAINBOARD == TRUE)

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_BlockingPortBufferFreed
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Check whether the input packet is from port that in blocking mode(MST)
 * INPUT    : tag_info    -- raw tagged info of the packet
 *            unit        -- logical unit
 *            port        -- logical port
 *            packet_type -- packet type
 * OUTPUT   : None
 * RETURN   : TRUE      -- blocking port
 *            FALSE     -- not blocking port
 * NOTE     : 1. tagged aware
 *            2. bpdu, lacp, gvrp, dot1x, lldp and cluster can be rx in blocking mode,
 *               and LAN_MAX_PACKET_TYPE will drop in master mode, so only LAN_IP_PACKET is checked
 *            3. multiple stp tree (can't diable a port by FFP , because port state is
 *               different in different spanning tree)
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_BlockingPortBufferFreed(UI16_T tag_info, UI32_T unit, UI32_T port, UI32_T packet_type)
{
    UI32_T vid, ifindex, state;

    if ((tag_info != 0) && lan_packet_info[packet_type].chk_stp)
    {
        vid = tag_info & (0x0fff); /* get vlan id from tag_info */
#if 0
        if(SWCTRL_POM_UserPortToLogicalPort(unit, port, &ifindex) == SWCTRL_LPORT_UNKNOWN_PORT)
        {
           SYSFUN_Debug_Printf("LAN_MonitorTask_ProcessPacket:SWCTRL_UserPortToLogicalPort == SWCTRL_LPORT_UNKNOWN_PORT, unit=[%lu], port=[%lu]\n",unit, port);
           return TRUE;
        }
#else
        if ((unit < 1) || (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK))
            return FALSE;

        if (port == 0 || port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
            return FALSE;
#define SWCTRL_UPORT_TO_IFINDEX(unit, port)         ( ((unit)-1) * (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) + (port)  )

        ifindex = SWCTRL_UPORT_TO_IFINDEX (unit, port);
#endif
        if(XSTP_POM_GetPortStateByVlan(vid, ifindex, &state) == FALSE)
        {
            SYSFUN_Debug_Printf("LAN_MonitorTask_ProcessPacket:XSTP_SVC_GetPortStateByVlan return FALSE, vid=[%lu], ifindex=[%lu]\n",(unsigned long)vid, (unsigned long)ifindex);
            return TRUE;
        }
        if(VAL_dot1dStpPortState_forwarding != state)
        {
            return TRUE;
        }
    }
    return FALSE;
}

#endif

/* ----------functions for management port----------
 */
#if (SYS_CPNT_MGMT_PORT == TRUE)
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ADM_Return2Pool
 * ----------------------------------------------------------------------------------
 * PURPOSE  : This function free packet buffer by call ADM_NIC_L_PT_Free and update
 *            coresponding counter
 * INPUT    : buf     -- packet buffer address that want to free
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1.This function is use to register to L_MM_Mref_Construct only
 * ----------------------------------------------------------------------------------*/
static void LAN_ADM_Return2Pool(void *buf)
{
    if (lan_adm_packet_alloc > 0)
    {
        lan_adm_packet_alloc--;
    }

    ADM_NIC_L_PT_Free (buf);
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ADM_RecvPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Whenever DEV_NICDRV receive a packet it will call this function
 *            to handle the packet (pass as a memory reference type)
 * INPUT    : unit     -- logical source unit number
 *            port     -- logical source port number (include SYS_ADPT_STACKING_PORT)
 *            cookie   -- not used
 *            reason   -- reason of the packet trap to CPU
 *            raw_data -- buffer address of receive packet
 *            size     -- length (in bytes) of packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : 1. This function is registered to ADM_NIC as a client
 * ----------------------------------------------------------------------------------*/
static void LAN_ADM_RecvPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, void *raw_data, I32_T size)
{
    AdmTaskMsg_T       msg;
    AdmTaskMsgBlock_T  *p_msg_data_blk;
    UI8_T              *buf_free_p;

    if (NULL == raw_data)
    {
        return;
    }

    /* In ADM_NIC, allocated buffer address will added 4 before passed to ADM driver.
     * This is for untagged packet can move DA&SA(12 bytes) 4 bytes to front (instead of rear)
     * of the packet and insert tag type and tag info into that 4 bytes. (detail in adm_nic.c)
     * So all packets arrive are tagged, the reason is only indecate its original type
     */
    if ( ADM_RECV_TAG_TYPE == reason )
    {
        buf_free_p = raw_data - 4;
    }
    else if ( ADM_RECV_UNTAG_TYPE == reason )
    {
        buf_free_p = raw_data;
    }

    /* mgmt port only support receive packet in master mode
     */
    if ((LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)||
        (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE))
    {
        ADM_NIC_L_PT_Free (buf_free_p);
        return;
    }

    p_msg_data_blk = (AdmTaskMsgBlock_T *)L_MEM_Allocate(sizeof(AdmTaskMsgBlock_T), L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_ADM_RECVPACKET));

    if (NULL == p_msg_data_blk)
    {
        ADM_NIC_L_PT_Free(buf_free_p);
        return;
    }

    p_msg_data_blk->unit     = (I32_T) unit;
    p_msg_data_blk->port     = (UI32_T)port;
    p_msg_data_blk->cookie   = (void *)cookie;
    p_msg_data_blk->reason   = (UI32_T)reason;
    p_msg_data_blk->raw_data = (void *)raw_data;
    p_msg_data_blk->size     = (I32_T) size;

    msg.mtype       = 0;
    msg.mreserved   = 0;
    msg.mtext       = (UI8_T*) p_msg_data_blk;

    if ( SYSFUN_SendMsgQ(lan_adm_msgQ_id, (UI32_T *)&msg, 0, 0) == SYSFUN_OK )
    {
        if (SYSFUN_SendEvent(lan_adm_lcb.task_id, LAN_ADM_PKT_ENQUEUE_EVENT) != SYSFUN_OK)
        {
            ADM_NIC_L_PT_Free(buf_free_p);
            L_MEM_Free((void*)p_msg_data_blk);
        }
    }
    else
    {
        ADM_NIC_L_PT_Free(buf_free_p);
        L_MEM_Free((void*)p_msg_data_blk);

        /* send event again to push the IGMP to clear up the queue
         * in case it lost last event flag ??
         */
        SYSFUN_SendEvent(lan_adm_lcb.task_id, LAN_ADM_PKT_ENQUEUE_EVENT);
    }
    return;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ADM_MonitorTaskProcessPacket
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Process dequeued packet, classify and then dispatch or drop
 * INPUT    : unit     -- logical source unit number
 *            port     -- logical source port number (include SYS_ADPT_STACKING_PORT)
 *            cookie   -- not used
 *            reason   -- reason of the packet trap to CPU
 *            raw_data -- buffer address of receive packet
 *            size     -- length (in bytes) of packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_ADM_MonitorTaskProcessPacket(I32_T unit, UI32_T port, void *cookie, UI32_T reason, void *raw_data, I32_T size)
{
    UI32_T              ether_header_length = 18;
    L_MM_Mref_Handle_T  *mref_handle_p;
    static UI32_T       lan_adm_rx_count = 0;
    RxFrame_T           *frame = raw_data;
    UI16_T              packet_frame_type, packet_frame_type_value, tag_info;
    void                (*free_fun)(void *);
    UI8_T               *buf_free_p;

    /* record that we have get one more packet from ADM_NIC
     */
    lan_adm_rx_count++;

    LAN_OM_IncreaseBackdoorCounter(LAN_RX_PACKET_TOTAL);

    /* In ADM_NIC, allocated buffer address will added 4 before passed to ADM driver.
     * This is for untagged packet can move DA&SA(12 bytes) 4 bytes to front (instead of rear)
     * of the packet and insert tag type and tag info into that 4 bytes. (detail in adm_nic.c)
     * So all packets arrive are tagged, the reason is only indecate its original type
     */
    if (ADM_RECV_TAG_TYPE == reason)
    {
        buf_free_p = raw_data - 4;
    }
    else if (ADM_RECV_UNTAG_TYPE == reason)
    {
        buf_free_p = raw_data;
    }

    if (TRUE == LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX) && !LAN_OM_IsBackdoorPortFilter(unit, port))
    {
        LAN_BackdoorPrintRxDebug(raw_data, unit, port, size, reason);
    }

    if (port != SYS_ADPT_MGMT_PORT)
    {
        SYSFUN_Debug_Printf("LAN_ADM_MonitorTaskProcessPacket: Illegal port number %lu\n", (unsigned long)port);
        ADM_NIC_L_PT_Free (buf_free_p);
        return;
    }

    /* all packet will make tagged in ADM_NIC, so untagged packet is illegal
     */
    if (frame->frame_tag_type != ETHER_TAG_TYPE)
    {
        ADM_NIC_L_PT_Free (buf_free_p);
        return;
    }

    /* limit the total packet number that announce to upper
     */
    if (lan_adm_packet_alloc < LAN_ADM_PACKET_THRESHOLD )
    {
        lan_adm_packet_alloc++;
        free_fun = LAN_ADM_Return2Pool;
    }
    else
    {
        ADM_NIC_L_PT_Free(buf_free_p);
        return;
    }

    packet_frame_type       = frame->frame_type;                     /* network order */
    packet_frame_type_value = L_STDLIB_Ntoh16(packet_frame_type);    /* host order    */
    tag_info                = L_STDLIB_Ntoh16(frame->frame_tag_info);/* host order    */

    /* MGMT port support only IP packet type
     */
    if (!((ETHER_IP_TYPE   == packet_frame_type) ||
          (ETHER_IPV6_TYPE   == packet_frame_type) ||
          (ETHER_ARP_TYPE  == packet_frame_type) ||
          (ETHER_RARP_TYPE == packet_frame_type) ))
    {
        ADM_NIC_L_PT_Free (buf_free_p);
        return;
    }

    if (NULL == lan_packet_info[LAN_IP_PACKET].announce_cb)
    {
        SYSFUN_Debug_Printf("LAN_ADM_MonitorTaskProcessPacket: lan_packet_info[LAN_IP_PACKET].announce_cb is NULL\n");
        ADM_NIC_L_PT_Free(buf_free_p);
        return;
    }

    /* The frame we send to upper layer is using mref_p instead of raw data.
     * Therefore, we need to construct the raw data to a mref_p, and pass the
     * mref_p to upper layer.
     */
    mref_handle_p = L_MM_Mref_Construct(buf_free_p,
                                        SYS_BLD_MAX_LAN_RX_BUF_SIZE_PER_PACKET,
                                        (UI8_T*)frame->pkt_buf - (UI8_T*)buf_free_p,
                                        size - ether_header_length,
                                        (L_MM_Mref_FreeFun_T)free_fun,
                                        NULL);
    if (NULL == mref_handle_p)
    {
        ADM_NIC_L_PT_Free(buf_free_p);
        SYSFUN_Debug_Printf("LAN_ADM_MonitorTaskProcessPacket:L_MM_Mref_Construct return NULL\n");
        return;
    }

    /* for L_MREF debugging purpose
     */
    mref_handle_p->current_usr_id = SYS_MODULE_LAN;
    mref_handle_p->next_usr_id    = SYS_MODULE_IML;

    lan_packet_info[packet_type].announce_cb(
        mref_handle_p,
        frame->da.addr,
        frame->sa.addr,
        tag_info,
        packet_frame_type_value,
        size - ether_header_length,
        lan_stack_info.my_unit_id,
        port);

    return;
}

/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ADM_Task
 * ----------------------------------------------------------------------------------
 * PURPOSE  : The task that process enqueued packets in adm message queue
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_ADM_Task(void)
{
    UI32_T  msg_buf[4];
    UI32_T  wait_events, all_events = 0, rcv_event = 0;

    wait_events = LAN_ADM_PKT_ENQUEUE_EVENT;

    while(TRUE)
    {
        SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY,
                            (all_events!=0)? SYSFUN_TIMEOUT_NOWAIT : SYSFUN_TIMEOUT_WAIT_FOREVER,
                            &rcv_event);

        all_events |= rcv_event;
        if ( all_events == 0 )
        {
            SYSFUN_Debug_Printf("LAN_ADM_Task: all_events == 0\n");
            continue;
        }

        if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            while(SYSFUN_ReceiveMsgQ (lan_adm_msgQ_id, (UI32_T *)&msg_buf, SYSFUN_TIMEOUT_NOWAIT)==SYSFUN_OK)
            {
                AdmTaskMsgBlock_T  *data_blk;

                data_blk = (AdmTaskMsgBlock_T *)((AdmTaskMsg_T *)msg_buf)->mtext;

                if ( data_blk->reason == ADM_RECV_TAG_TYPE )
                {
                    ADM_NIC_L_PT_Free(data_blk->raw_data - 4);
                }
                else if ( data_blk->reason == ADM_RECV_UNTAG_TYPE )
                {
                    ADM_NIC_L_PT_Free(data_blk->raw_data);
                }
            }
            all_events = 0;
            continue;
        }

        if (all_events & LAN_ADM_PKT_ENQUEUE_EVENT)
        {
            if (SYSFUN_ReceiveMsgQ (lan_adm_msgQ_id,(UI32_T *)&msg_buf, SYSFUN_TIMEOUT_NOWAIT)==SYSFUN_RESULT_NO_MESSAGE)
            {
                all_events ^= LAN_ADM_PKT_ENQUEUE_EVENT;
            }
            else
            {
                AdmTaskMsgBlock_T  *data_blk;

                data_blk = (AdmTaskMsgBlock_T *)((AdmTaskMsg_T *)msg_buf)->mtext;

                LAN_ADM_MonitorTaskProcessPacket(data_blk->unit, data_blk->port, data_blk->cookie,
                                                 data_blk->reason, data_blk->raw_data, data_blk->size);

                L_MEM_Free((void *)((AdmTaskMsg_T*)msg_buf)->mtext);
            }
        }
    }
}

#endif


/*-----------------------------------------------------------------------------------*
 * APIs for chip workaround                                                          *
 *-----------------------------------------------------------------------------------*/
#if defined(BCM56514) && 0 /* obsolete, has another solution */
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ChipWorkaround_FrameWithInnerTagOnlyReceivedFromAccessPort
 * ----------------------------------------------------------------------------------
 * PURPOSE  : see note.
 * INPUT    : unit, port
 *            has_tag, number_of_tag
 *            tag_info_p
 * OUTPUT   : tag_info_p
 * RETURN   : TRUE if the input tag_info had been updated.
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_ChipWorkaround_FrameWithInnerTagOnlyReceivedFromAccessPort(
                UI32_T unit, UI32_T port, BOOL_T has_tag, UI32_T number_of_tag, UI16_T *tag_info_p)
{
    /* EPR: ASF4526B-FLF-P5-00112
     * Problem: packet received from access port, can't be learn with SPVID.
     *
     * BCM56514 chip issue, workaround
     *
     * Upper layer assume the packets to CPU with double tagged when  "Double Tag mode" is enabled
     * But chip behavior is
     * If packets are copy-to-cpu(CML=5), the packet retains the original tag with the exception
     * that the ingress may insert a default tag if the packed is untagged.
     *
     * If PKT.TPID ==VLAN_CTRL.INNER_TPID, the packet is tagged from chip's viewpoint
     * and copied to CPU (not double tagged)
     *
     * solution: for access port(UNI) and packets received with single tagged, give a SVID (i.e. PVID) as tag_info
     *
     * NOTE: this workaround does not really work on SW learn system,
     *       because it only work on master unit,
     */
    BOOL_T tag_info_updated = FALSE;
    UI32_T ifindex;
    UI32_T dot1q_tunnel_status;
    UI32_T dot1q_tunnel_mode;

    if (LAN_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (has_tag && number_of_tag == 1)
    {
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_POM_UserPortToIfindex(unit, port, &ifindex))
        {
            SYSFUN_Debug_Printf("%s:SWCTRL_POM_UserPortToIfindex return SWCTRL_LPORT_UNKNOWN_PORT, unit=[%lu], port=[%lu]\n",__FUNCTION__, (unsigned long)unit, (unsigned long)port);
            return FALSE;
        }
        if (FALSE == SWCTRL_POM_GetDot1qTunnelStatus(&dot1q_tunnel_status))
        {
            SYSFUN_Debug_Printf("%s:SWCTRL_POM_GetDot1qTunnelStatus return FALSE\n",__FUNCTION__);
            return FALSE;
        }
        if (FALSE == SWCTRL_POM_GetPortDot1qTunnelMode(ifindex, &dot1q_tunnel_mode))
        {
            SYSFUN_Debug_Printf("%s:SWCTRL_POM_GetPortDot1qTunnelMode return FALSE\n",__FUNCTION__);
            return FALSE;
        }

        if (dot1q_tunnel_status == VAL_vlanDot1qTunnelStatus_enabled &&
            dot1q_tunnel_mode == SWCTRL_QINQ_PORT_MODE_ACCESS)
        {
            /* just set tag_info to 0, indicates untagged frame.
             */
            *tag_info_p = 0;
            tag_info_updated = TRUE;
        }
    }

    return tag_info_updated;
}
#endif /* defined(BCM56514) */

#if (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE)
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_ChipWorkaround_IncorrectReasonForIntruder
 * ----------------------------------------------------------------------------------
 * PURPOSE  : workaround for the chip cannot support correct reason to
 *            determine if the received packets is intruder or not.
 * INPUT    : dst_mac, src_mac, tag_info, ether_type, src_unit, src_port
 *            reason_p - original reason
 * OUTPUT   : reason_p - patched reason
 * RETURN   : TRUE if the input reason had been updated.
 * NOTE     : NOT work for slave units.
 * ----------------------------------------------------------------------------------*/
static BOOL_T LAN_ChipWorkaround_IncorrectReasonForIntruder(UI8_T *dst_mac, UI8_T *src_mac, UI16_T tag_info, UI16_T ether_type, UI32_T src_unit, UI32_T src_port, UI32_T *reason_p)
{
    extern BOOL_T AMTRDRV_OM_GetExactRecord(UI8_T *addr_entry);

    AMTRDRV_TYPE_Record_T get_entry;
    VLAN_OM_Vlan_Port_Info_T vlan_port_info;
    UI32_T workaround_reason = 0;
    UI32_T ifindex;
    UI32_T vid;

    if (LAN_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        /* NOT work for slave units.
         */
        return FALSE;
    }

#if 0
/* ES4626F-SW-FLF-38-01769
 * Behavior change:
 *     Shall learn SA if DA == 01-80-c2-00-00-00~01-80-c2-00-00-1f
 */
    /* shall not learn reserved mcast SA,
     *
     * here is simple patch:
     * if DA == 01-80-c2-00-00-00~01-80-c2-00-00-1f,
     * will not learn SA.
     */
    {
        if (dst_mac[0] == 0x01 &&
            dst_mac[1] == 0x80 &&
            dst_mac[2] == 0xc2 &&
            dst_mac[3] == 0x00 &&
            dst_mac[4] == 0x00 &&
            (dst_mac[5] & 0x1f) == dst_mac[5])
        {
            return FALSE;
        }
    }
#endif

    if (tag_info != 0)
    {
        vid = tag_info & 0xfff;
    }
    else
    {
        /* get pvid
         */
        vlan_port_info.lport_ifindex = ifindex;
        if (!VLAN_POM_GetVlanPortEntry(&vlan_port_info))
        {
            return FALSE;
        }

        VLAN_OM_ConvertFromIfindex(vlan_port_info.port_item.dot1q_pvid_index, &vid);
    }

    get_entry.address.vid = vid;
    memcpy(get_entry.address.mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* if source MAC is known and learned in the same port,
     * this packet MUST be from an authorized device.
     * NOTE: do not allow station move condition so check port
     *       number here
     */
    if (AMTRDRV_OM_GetExactRecord((UI8_T *)&get_entry))
    {
        SWCTRL_POM_UserPortToLogicalPort(src_unit, src_port, &ifindex);

        if (get_entry.address.ifindex != ifindex)
        {
            workaround_reason |= LAN_TYPE_RECV_REASON_STATION_MOVE;
        }
    }
    else
    {
        workaround_reason |= LAN_TYPE_RECV_REASON_INTRUDER;
    }

    if (workaround_reason == 0)
    {
        return FALSE;
    }

    *reason_p |= workaround_reason;

    return TRUE;
}
#endif /* (SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION == TRUE) */


/* ----------functions for backdoor----------
 */

#if 0 /* unused function */
/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_DisplayDataByHex
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Print out raw data in HEX format
 * INPUT    : data   -- pointer to the data
 *            length -- data length that want to print
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_DisplayDataByHex(UI8_T *data, UI32_T length)
{
    UI32_T i;

    #define BYTES_IN_GROUP  4
    #define BYTES_IN_LINE   (5*BYTES_IN_GROUP)

    for (i=0; i<length; i++)
    {
        if ((i%BYTES_IN_LINE) == 0)
        {
            BACKDOOR_MGR_Printf("%04lx:",i);
        }
        if (((i%BYTES_IN_GROUP) == 0) && ((i%BYTES_IN_LINE) != 0))
        {
            BACKDOOR_MGR_Printf("  ");
        }
        if ((i%BYTES_IN_GROUP) == 0)
        {
            BACKDOOR_MGR_Printf("0x");
        }
        BACKDOOR_MGR_Printf("%02x ", data[i]);
        if (((i+1)%BYTES_IN_LINE) == 0)
        {
            BACKDOOR_MGR_Printf("\r\n");
        }
    }
}
#endif

static void LAN_BackdoorInputMAC(UI8_T mac_addr[6])
{
    UI32_T k;
    UI8_T  tmpstr[12];
    int ch;

    BACKDOOR_MGR_Printf("Input specific address. ex:000080e61234\n");
    BACKDOOR_MGR_Printf("Input MAC address:");

    for (k=0; k<12; k++)
    {
        if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
        {
            printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
            return;
        }
        tmpstr[k] = ch;
        BACKDOOR_MGR_Printf("%c",tmpstr[k]);
    }
    BACKDOOR_MGR_Printf("\n");

    for (k=0; k<12; k++)
    {
        if(tmpstr[k]>='0' && tmpstr[k]<='9')
            tmpstr[k] = tmpstr[k]-'0';
        else if(tmpstr[k]>='a' && tmpstr[k]<='z')
            tmpstr[k] = tmpstr[k]-'a'+10;
        else if(tmpstr[k]>='A' && tmpstr[k]<='Z')
            tmpstr[k] = tmpstr[k]-'A'+10;
        else tmpstr[k] = 0;
    }

    for (k=0; k<6; k++)
    {
        mac_addr[k] = (tmpstr[k*2]*16)+tmpstr[k*2+1];
    }
}

static void LAN_BackdoorSendPkt(UI16_T port)
{
    L_MM_Mref_Handle_T      *mref_handle_p;
    UI8_T                   *payload_p;
    UI32_T                  pdu_len;
    UI16_T                  i, pkt_size;
    UI8_T                   my_mac[6], dst_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    char                    buf[8];

    if(STKTPLG_POM_GetLocalUnitBaseMac(my_mac)==FALSE)
    {
        BACKDOOR_MGR_Printf("\nCannnot get my mac.");
        return;
    }
    BACKDOOR_MGR_Printf("\nInput packet size:");
    BACKDOOR_MGR_RequestKeyIn(buf, 7);
    pkt_size = atoi(buf);

    mref_handle_p = L_MM_AllocateTxBuffer(pkt_size,
                                          L_MM_USER_ID2(SYS_MODULE_LAN, 1));
    if(mref_handle_p)
    {
        BACKDOOR_MGR_Printf("\nmref_handle_p=%p", mref_handle_p);
    }
    else
    {
        BACKDOOR_MGR_Printf("\nmref_handle_p is null");
        return;
    }

    payload_p = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    BACKDOOR_MGR_Printf("\npayload_p=%p", payload_p);

    for(i=0; i<pkt_size; i++)
    {
        payload_p[i]=(UI8_T)(i&0xFF);
    }

    BACKDOOR_MGR_Printf("\nSend out packet(size=%d)", (int)pkt_size);

    LAN_SendPacket(mref_handle_p, dst_mac, my_mac, pkt_size, 0x1, pkt_size, 1, (UI32_T)port, FALSE, 0);
    return;
}

static void LAN_BackdoorSendPktByPortList(UI16_T port)
{
    L_MM_Mref_Handle_T   *mref_handle_p;
    UI8_T                uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T                untagged_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T                active_uport_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T                *payload_p;
    UI32_T               pdu_len;
    UI16_T               i, pkt_size;
    UI8_T                my_mac[6], dst_mac[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
    char                 buf[8];

    if(STKTPLG_POM_GetLocalUnitBaseMac(my_mac)==FALSE)
    {
        BACKDOOR_MGR_Printf("\nCannnot get my mac.");
        return;
    }

    BACKDOOR_MGR_Printf("\nInput packet size:");
    BACKDOOR_MGR_RequestKeyIn(buf, 7);
    pkt_size = atoi(buf);

    memset(active_uport_count_per_unit, 0, sizeof(active_uport_count_per_unit));
    active_uport_count_per_unit[0]=1;
    memset(uport_list, 0, sizeof(uport_list));
    memset(untagged_list, 0, sizeof(untagged_list));
    uport_list[((port-1) / 8)] |= ((0x01) << (7 - ((port - 1) % 8)));
    untagged_list[((port-1) / 8)] |= ((0x01) << (7 - ((port - 1) % 8)));
    mref_handle_p = L_MM_AllocateTxBuffer(pkt_size,
                                          L_MM_USER_ID2(SYS_MODULE_LAN, 1));
    if(mref_handle_p)
    {
        BACKDOOR_MGR_Printf("\nmref_handle_p=%p", mref_handle_p);
    }
    else
    {
        BACKDOOR_MGR_Printf("\nmref_handle_p is null");
        return;
    }

    payload_p = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    BACKDOOR_MGR_Printf("\npayload_p=%p", payload_p);

    for(i=0; i<pkt_size; i++)
    {
        payload_p[i]=(UI8_T)(i&0xFF);
    }

    BACKDOOR_MGR_Printf("\nSend out packet(size=%d)", (int)pkt_size);

    LAN_SendMultiPacket(mref_handle_p, dst_mac, my_mac, pkt_size, 0x1, pkt_size, uport_list, untagged_list, active_uport_count_per_unit, 0);
    return;

}

void LAN_BackdoorEntrance(void)
{
    int     ch;
    BOOL_T  backdoor_continue = TRUE;
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];

/*
    L_THREADGRP_Handle_T  tg_handle;
    UI32_T  backdoor_member_id;
*/
    //kh_shi tg_handle = DRIVER_PROC_COMM_GetTDriverhreadGroupHandle();

    /* Join thread group
     */
    /*kh_shi if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }*/

    while (backdoor_continue)
    {
        BACKDOOR_MGR_Printf("\r\n-----LAN BACKDOOR MENU:-----");
        BACKDOOR_MGR_Printf("\n\t[0]:Show lan counter");
        BACKDOOR_MGR_Printf("\n\t[1]:Reset lan counter");
        BACKDOOR_MGR_Printf("\n\t[2]:Display rx packet");
        BACKDOOR_MGR_Printf("\n\t[3]:Display tx packet");
        BACKDOOR_MGR_Printf("\n\t[4]:Display tx data");
        BACKDOOR_MGR_Printf("\n\t[5]:Disable display of all rx/tx");
        BACKDOOR_MGR_Printf("\n\t[6]:Send out a packet");
        BACKDOOR_MGR_Printf("\n\t[7]:Send out a packet by portlist fun");
        BACKDOOR_MGR_Printf("\n\t[8]:Set port mask for packet display");
        BACKDOOR_MGR_Printf("\n\t[9]:Drop rx packet: %d", LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_DROP));
        BACKDOOR_MGR_Printf("\n\t[S]:Open stacking backdoor");
        BACKDOOR_MGR_Printf("\n\t[Q]:Quit\n");
        BACKDOOR_MGR_Printf("\r----------------------------\n");

        if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
        {
            printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
            break;
        }

        //L_THREADGRP_Execution_Request(tg_handle, backdoor_member_id);
        switch (ch)
        {
            case 'q':
            case 'Q':
                backdoor_continue = FALSE;
                break;

            case '0':
                BACKDOOR_MGR_Printf("RX packet total    = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_RX_PACKET_TOTAL));
                BACKDOOR_MGR_Printf("unknown class drop = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_RX_UNKNOWN_CLASS_DROP));
                BACKDOOR_MGR_Printf("blocking port drop = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_RX_BLOCKING_PORT_DROP));
                BACKDOOR_MGR_Printf("intrusion drop     = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_RX_INTRUSION_DROP));
                BACKDOOR_MGR_Printf("untagged drop      = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_RX_UNTAGGED_DROP));
                BACKDOOR_MGR_Printf("zero sa error      = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_ZERO_SA_ERROR));
                BACKDOOR_MGR_Printf("multicast sa error = %lu\n", (unsigned long)LAN_OM_GetBackdoorCounter(LAN_MULTICAST_SA_ERROR));
                break;

            case '1':
                LAN_OM_ClearAllBackdoorCounter();
                break;

            case '2':
                BACKDOOR_MGR_Printf("\nPress (0=rx all, 1=rx specific DA, 2=rx specific SA)");
                if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
                {
                    printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
                    backdoor_continue = FALSE;
                    break;
                }
                if (ch == '0')
                {
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX,TRUE);
                }
                else if (ch == '1')
                {
                    LAN_BackdoorInputMAC(mac_addr);
                    LAN_OM_SetBackdoorMac(LAN_DEBUG_RX_DA_ADDR, mac_addr);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX,TRUE);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX_DA,TRUE);
                }
                else if (ch == '2')
                {
                    LAN_BackdoorInputMAC(mac_addr);
                    LAN_OM_SetBackdoorMac(LAN_DEBUG_RX_SA_ADDR, mac_addr);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX,TRUE);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX_SA,TRUE);
                }
                break;

            case '3':
                BACKDOOR_MGR_Printf("\nPress (0=tx all, 1=tx specific DA)");
                if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
                {
                    printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
                    backdoor_continue = FALSE;
                    break;
                }
                if (ch == '0')
                {
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX, TRUE);
                }
                else if (ch == '1')
                {
                    LAN_BackdoorInputMAC(mac_addr);
                    LAN_OM_SetBackdoorMac(LAN_DEBUG_TX_DA_ADDR, mac_addr);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX, TRUE);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DA, TRUE);
                }
                break;

            case '4':
                BACKDOOR_MGR_Printf("\nPress (0=tx all, 1=tx specific DA)");
                if ((ch = BACKDOOR_MGR_GetChar()) == EOF)
                {
                    printf("%s: BACKDOOR_MGR_GetChar fail\n",__FUNCTION__);
                    backdoor_continue = FALSE;
                    break;
                }
                if (ch == '0')
                {
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DATA,TRUE);
                }
                else if (ch == '1')
                {
                    LAN_BackdoorInputMAC(mac_addr);
                    LAN_OM_SetBackdoorMac(LAN_DEBUG_TX_DA_ADDR, mac_addr);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DATA,TRUE);
                    LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DA_DATA,TRUE);
                }
                break;

            case '5':
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX_SA, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX_DA, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DA, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DATA, FALSE);
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_TX_DA_DATA, FALSE);
                break;
            case '6':
            {
                UI16_T port;

                BACKDOOR_MGR_Printf("\nSend to which port?");
                BACKDOOR_MGR_RequestKeyIn(mac_addr,5);
                port = (UI16_T)(atoi((char*)(mac_addr)));
                LAN_BackdoorSendPkt(port);
            }
                break;
            case '7':
            {
                UI16_T port;

                BACKDOOR_MGR_Printf("\nSend to which port?");
                BACKDOOR_MGR_RequestKeyIn(mac_addr,5);
                port = (UI16_T)(atoi((char*)(mac_addr)));
                LAN_BackdoorSendPktByPortList(port);
            }
                break;

            case '8':
            {
                UI8_T port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
                UI32_T i, j, unit, port;

                BACKDOOR_MGR_Printf("\nWhich unit: ");
                BACKDOOR_MGR_RequestKeyIn(mac_addr,5);
                unit = (atoi((char*)(mac_addr)));

                BACKDOOR_MGR_Printf("\nWhich port: ");
                BACKDOOR_MGR_RequestKeyIn(mac_addr,5);
                port = (atoi((char*)(mac_addr)));

                LAN_OM_GetBackdoorPortFilterMask(port_filter_mask);

                BACKDOOR_MGR_Printf("\nOld filtered port: ");
                for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                    for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; j++)
                        if (port_filter_mask[i][j/8] & BIT_VALUE(7 - (j % 8)))
                            BACKDOOR_MGR_Printf("%2d/%2d, ", i+1, j+1);
                BACKDOOR_MGR_Printf("\b\b ");

                if (unit > 0 && unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK &&
                    port > 0 && port <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
                {
                    port_filter_mask[unit-1][(port-1)/8] ^= BIT_VALUE(7 - ((port-1) % 8));
                }

                BACKDOOR_MGR_Printf("\nNew filtered port: ");
                for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
                    for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; j++)
                        if (port_filter_mask[i][j/8] & BIT_VALUE(7 - (j % 8)))
                            BACKDOOR_MGR_Printf("%2d/%2d, ", i+1, j+1);
                BACKDOOR_MGR_Printf("\b\b");

                LAN_OM_SetBackdoorPortFilterMask(port_filter_mask);
            }
                break;

            case '9':
                LAN_OM_SetBackdoorToggle(LAN_DEBUG_RX_DROP,
                    !LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_DROP));
                break;

            case 's':
            case 'S':
#if (SYS_CPNT_STACKING == TRUE)
    #ifdef LAN_STACKING_BACKDOOR_OPEN
                LAN_STACKING_Backdoor();
    #else
                BACKDOOR_MGR_Printf("Not supported in this build\n");
    #endif
#endif
                break;

            default:
                break;
        }
        //L_THREADGRP_Execution_Release(tg_handle, backdoor_member_id);
    }

    //L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
}

static void LAN_BackdoorPrintSendPkt(UI32_T unit, UI32_T port, UI32_T packet_length, UI8_T *data)
{
    int     i;
    UI32_T  time = SYSFUN_GetSysTick();
    BOOL_T  skip = FALSE;
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];

    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DA) == TRUE)    /*tx specific DA address*/
    {
        LAN_OM_GetBackdoorMac(LAN_DEBUG_TX_DA_ADDR, mac_addr);
        if (memcmp(mac_addr, data, 6)!=0)
            skip=TRUE;
    }

    if (!skip)
    {
        BACKDOOR_MGR_Printf("\n--sendPkt--\n");
        BACKDOOR_MGR_Printf("[unit]%ld,[port]%ld,[size]%ld,[DA]%02x%02x-%02x%02x-%02x%02x,"
                             "[SA]%02x%02x-%02x%02x-%02x%02x,[pkt]",
                             (long)unit, (long)port, (long)packet_length,
                             data[0], data[1], data[2], data[3], data[4], data[5],
                             data[6], data[7], data[8], data[9], data[10], data[11]);
        for (i=12; i<=packet_length-1; i++)
            BACKDOOR_MGR_Printf("%02x", data[i]);
        BACKDOOR_MGR_Printf("[time]%ld\n", (long)time);
    }
}

static void LAN_BackdoorPrePrintSendMultiPkt(L_MM_Mref_Handle_T *mref_handle_p,
                                             UI8_T               dst_mac[6],
                                             UI8_T               src_mac[6],
                                             UI16_T              type,
                                             UI16_T              tag_info,
                                             UI32_T              packet_length,
                                             UI8_T              *uport_list,
                                             UI8_T              *uport_untagged_list)
{
    UI8_T   *data;
    UI32_T  time  = SYSFUN_GetSysTick();
    UI32_T  i, pdu_len;
    BOOL_T  skip = FALSE;
    UI8_T   mac_addr[SYS_ADPT_MAC_ADDR_LEN];

    data = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DA_DATA) == TRUE)    /*tx specific DA address*/
    {
        LAN_OM_GetBackdoorMac(LAN_DEBUG_TX_DA_ADDR, mac_addr);
        if (memcmp(mac_addr, data, 6)!=0)
            skip = TRUE;
    }

    if (FALSE == skip)
    {
        BACKDOOR_MGR_Printf("\n--------------- data from upper layer-----------------------\n");
        BACKDOOR_MGR_Printf("[portList]");
        for (i=0; i<=SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1; i++)
            BACKDOOR_MGR_Printf("%02x", uport_list[i]);
        BACKDOOR_MGR_Printf("[untaggedList]");
        for (i=0; i<=SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1; i++)
            BACKDOOR_MGR_Printf("%02x", uport_untagged_list[i]);
        BACKDOOR_MGR_Printf("[size]%ld"
               "[DA]%02x%02x-%02x%02x-%02x%02x"
               "[SA]%02x%02x-%02x%02x-%02x%02x"
               "[type]%04x[tag_info]%04x",
               (long)packet_length,
               dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4],  dst_mac[5],
               src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5],
               type, tag_info);
        BACKDOOR_MGR_Printf("[PK]");
        for (i=0; i<=packet_length-1; i++)
            BACKDOOR_MGR_Printf("%02x", data[i]);
        BACKDOOR_MGR_Printf("[time]%ld\n", (long)time);
    }
}

static void LAN_BackdoorPrintSendMultiPkt(UI8_T *uport_list, UI32_T unit_bmp, UI8_T *uport_untagged_list, UI32_T packet_length, UI8_T *data)
{
    int i;
    UI32_T time = SYSFUN_GetSysTick();
    BOOL_T skip = FALSE;
    UI8_T  mac_addr[SYS_ADPT_MAC_ADDR_LEN];

    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_TX_DA) == TRUE)    /*tx specific DA address*/
    {
        LAN_OM_GetBackdoorMac(LAN_DEBUG_TX_DA_ADDR, mac_addr);
        if (memcmp(mac_addr, data, 6)!=0)
            skip = TRUE;
    }

    if (!skip)
    {
        BACKDOOR_MGR_Printf("\n--sendMultiPkt--\n");
        BACKDOOR_MGR_Printf("[portList]");
        for (i=0; i<=SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1; i++)
            BACKDOOR_MGR_Printf("%02x", uport_list[i]);
        BACKDOOR_MGR_Printf(",[unitBmp]=%lu", (unsigned long)unit_bmp);
        BACKDOOR_MGR_Printf(",[untaggedList]");
        for (i=0; i<=SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST-1; i++)
            BACKDOOR_MGR_Printf("%02x", uport_untagged_list[i]);
        BACKDOOR_MGR_Printf("[Size]=%ld,"
               "[DA]%02x%02x-%02x%02x-%02x%02x,"
               "[SA]%02x%02x-%02x%02x-%02x%02x,[PK]",
               (long)packet_length,
               data[0], data[1], data[2], data[3], data[4],  data[5],
               data[6], data[7], data[8], data[9], data[10], data[11]);
        for (i=12; i<=packet_length-1; i++)
            BACKDOOR_MGR_Printf("%02x", data[i]);
        BACKDOOR_MGR_Printf(",[time]=%ld\n", (long)time);
    }
}

/* FUNCTION NAME : LAN_BackdoorPrintRxDebug
 * PURPOSE:
 *      Debug received packets
 *
 */
static void LAN_BackdoorPrintRxDebug(RxFrame_T *frame, UI32_T unit, UI32_T port, UI32_T size, UI32_T reason)
{
    UI32_T  time = SYSFUN_GetSysTick();
    UI8_T   *data = (UI8_T*)frame;
    BOOL_T  log_data = FALSE;
    UI8_T   mac_da[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   mac_sa[SYS_ADPT_MAC_ADDR_LEN];

    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_DA) == TRUE)
    {
        if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_SA) == FALSE)
        {
            /*rx specific DA*/
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_DA_ADDR, mac_da);
            if ((memcmp(mac_da, data,6)==0))
            {
                log_data = TRUE;
            }
        }
        else
        {
            /*rx specific DA and SA*/
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_DA_ADDR, mac_da);
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_SA_ADDR, mac_sa);
            if ((memcmp(mac_da, data, 6)==0) &&
                (memcmp(mac_sa, data+6, 6)==0))
            {
                log_data = TRUE;
            }
        }
    }
    else
    {
        if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_SA) == TRUE)
        {
            /* rx specific SA */
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_SA_ADDR, mac_sa);
            if ((memcmp(mac_sa, data+6, 6)==0))
            {
                log_data = TRUE;
            }
        }
        else
        {
            /* rx all */
            log_data = TRUE;
        }
    }

    if (TRUE == log_data)
    {
        int i;
        BACKDOOR_MGR_Printf("\n--RxPkt--\n");
        BACKDOOR_MGR_Printf("[unit]%lu[port]%lu[size]%lu[reason]%08lx[DA]%02x%02x-%02x%02x-%02x%02x[SA]%02x%02x-%02x%02x-%02x%02x[Pkt]",
                             (unsigned long)unit, (unsigned long)port, (unsigned long)size, (unsigned long)reason, data[0], data[1], data[2], data[3], data[4], data[5],
                             data[6], data[7], data[8], data[9], data[10], data[11]);
        for (i=12; i<=size-1; i++)
            BACKDOOR_MGR_Printf("%02x", data[i]);
        BACKDOOR_MGR_Printf("[time]%ld\n", (long)time);
    }
}

static void LAN_BackdoorPrintRxTagParsing(RxFrame_T *frame, UI32_T ingress_vlan, BOOL_T rx_is_tagged, BOOL_T has_tag, UI16_T tag_info, UI16_T inner_tag_info, UI16_T packet_frame_type, BOOL_T pkt_is_truncated)
{
    UI8_T   *data = (UI8_T*)frame;
    BOOL_T  log_data = FALSE;
    UI8_T   mac_da[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   mac_sa[SYS_ADPT_MAC_ADDR_LEN];

    if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_DA) == TRUE)
    {
        if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_SA) == FALSE)
        {
            /*rx specific DA*/
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_DA_ADDR, mac_da);
            if ((memcmp(mac_da, data,6)==0))
            {
                log_data = TRUE;
            }
        }
        else
        {
            /*rx specific DA and SA*/
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_DA_ADDR, mac_da);
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_SA_ADDR, mac_sa);
            if ((memcmp(mac_da, data, 6)==0) &&
                (memcmp(mac_sa, data+6, 6)==0))
            {
                log_data = TRUE;
            }
        }
    }
    else
    {
        if (LAN_OM_GetBackdoorToggle(LAN_DEBUG_RX_SA) == TRUE)
        {
            /* rx specific SA */
            LAN_OM_GetBackdoorMac(LAN_DEBUG_RX_SA_ADDR, mac_sa);
            if ((memcmp(mac_sa, data+6, 6)==0))
            {
                log_data = TRUE;
            }
        }
        else
        {
            /* rx all */
            log_data = TRUE;
        }
    }

    if (TRUE == log_data)
    {
        BACKDOOR_MGR_Printf("\n--RxTag--\n");
        BACKDOOR_MGR_Printf("[ingress_vlan]%lu[rx_is_tagged]%d[has_tag]%d[tag_info]%04hx[inner_tag_info]%04hx[packet_frame_type]%04hx[pkt_is_truncated]%d\n",
                             (unsigned long)ingress_vlan, rx_is_tagged,
                             has_tag, tag_info, inner_tag_info, packet_frame_type,
                             pkt_is_truncated);
    }
}

#ifdef LAN_STACKING_BACKDOOR_OPEN
#define MAXLINE                                      255

/* FUNCTION NAME: LAN_STACKING_Backdoor
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_Backdoor(void)
{
    UI8_T    ch;

    while(1)
    {
        BACKDOOR_MGR_Printf("\r\n");
        BACKDOOR_MGR_Printf("\r\n---------------------------------------------------");
        BACKDOOR_MGR_Printf("\r\n-----       LAN Stacking Engineer Menu        -----");
        BACKDOOR_MGR_Printf("\r\n---------------------------------------------------");
        BACKDOOR_MGR_Printf("\r\n [0] Exit");
        BACKDOOR_MGR_Printf("\r\n [1] Send packet to an unit");
        BACKDOOR_MGR_Printf("\r\n [2] Display transmitted packet for stacking");
        if (LAN_OM_GetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET))
            BACKDOOR_MGR_Printf("[enable]");
        else
            BACKDOOR_MGR_Printf("[disable]");
        BACKDOOR_MGR_Printf("\r\n [3] Display received packet for stacking");
        if (LAN_OM_GetBackdoorStackingToggle(DISPLAY_RECEIVE_PACKET))
            BACKDOOR_MGR_Printf("[enable]");
        else
            BACKDOOR_MGR_Printf("[disable]");
        BACKDOOR_MGR_Printf("\r\n [4] Display isc packet information for debug");
        if (LAN_OM_GetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE))
            BACKDOOR_MGR_Printf("[enable]");
        else
            BACKDOOR_MGR_Printf("[disable]");
        BACKDOOR_MGR_Printf("\r\n [5] Multicasting to port 1,2,3 on unit 2");
        BACKDOOR_MGR_Printf("\r\n [6] lan.c stop send packet to upper layer for debug");
        BACKDOOR_MGR_Printf("\r\r\n\n Enter Selection: ");

        ch = BACKDOOR_MGR_GetChar();
        switch(ch)
        {
            case '0':
                BACKDOOR_MGR_Printf("\r\n Exit LAN Stacking Engineer Menu\r\n");
                return;
            case '1':
                LAN_STACKING_SendPacket();
                break;
            case '2':
                if (LAN_OM_GetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET))
                    LAN_OM_SetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET, FALSE);
                else
                    LAN_OM_SetBackdoorStackingToggle(DISPLAY_TRANSMIT_PACKET, TRUE);
                continue;
            case '3':
                if (LAN_OM_GetBackdoorStackingToggle(DISPLAY_RECEIVE_PACKET))
                    LAN_OM_SetBackdoorStackingToggle(DISPLAY_RECEIVE_PACKET, FALSE);
                else
                    LAN_OM_SetBackdoorStackingToggle(DISPLAY_RECEIVE_PACKET, TRUE);
                continue;
            case '4':
                if (LAN_OM_GetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE))
                    LAN_OM_SetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE, FALSE);
                else
                    LAN_OM_SetBackdoorStackingToggle(ISC_DEBUG_SHOW_MESSAGE, TRUE);
                continue;
            case '5':
                LAN_STACKING_SendMultiIpPkt();
            default:
                continue;
        }
    }

} /* End of LAN_STACKING_Backdoor() */

/* FUNCTION NAME: LAN_STACKING_SendPacket
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_SendPacket(void)
{
    char      line_buffer[MAXLINE];
    UI32_T    unit=0, port=0;
    UI8_T     ch;

    BACKDOOR_MGR_Printf("\r\r\n\n------------- Send Packet to an Unit ---------------");
    BACKDOOR_MGR_Printf("\r\n Enter Unit ID : ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, sizeof(line_buffer)-1) > 0)
        unit = atoi(line_buffer);

    BACKDOOR_MGR_Printf("\r\n Enter Port ID : ");

    if (BACKDOOR_MGR_RequestKeyIn(line_buffer, sizeof(line_buffer)-1) > 0)
        port = atoi(line_buffer);

    BACKDOOR_MGR_Printf("\r\n Enter Packet Type [1]IP [2]STA [3]GVRP [4]LACP : ");
    ch = getchar();
    getchar(); /* to take away \n */

    BACKDOOR_MGR_Printf("\r\n Select Unit ID:%ld,  Port ID:%ld,", (long)unit, (long)port); /* Debug message */

    switch (ch)
    {
       case '1':
              BACKDOOR_MGR_Printf("  IP packet");
              LAN_STACKING_SendIpPkt(unit, port);
              break;
       case '2':
              BACKDOOR_MGR_Printf("  STA packet");
              LAN_STACKING_SendStaPkt(unit, port);
              break;
       case '3':
              BACKDOOR_MGR_Printf("  GVRP packet");
              LAN_STACKING_SendGvrpPkt(unit, port);
              break;
       case '4':
              BACKDOOR_MGR_Printf("  LACP packet");
              LAN_STACKING_SendLacpPkt(unit, port);
              break;
       default:
              BACKDOOR_MGR_Printf("\r\n Unknown packet type");
    }

    BACKDOOR_MGR_Printf("\r\n----------------------------------------------------");
    BACKDOOR_MGR_Printf("\r\n");

} /* End of LAN_STACKING_SendPacket */

/* FUNCTION NAME: LAN_STACKING_SendIpPkt
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_SendIpPkt(UI32_T unit, UI32_T port)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *frame;
    RxFrame_T           *ether_frame;
    UI32_T              pdu_len, i;
    UI16_T              taginfo = 0x2001;
    UI8_T               damac[6]={0x00,0x00,0xf1,0x0d,0x00,0x02};
    UI8_T               samac[6]={0x00,0x00,0xf1,0x0d,0x00,0x00};


    mref_handle_p = L_MM_AllocateTxBuffer(64, /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_STACKING_SENDIPPKT));

    frame = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (frame ==NULL)
        return;

    for (i=0; i<64; i++)
    {
        frame[i] = (UI8_T)i;
    }

    ether_frame = (RxFrame_T *)frame;
    ether_frame->frame_tag_type = ETHER_TAG_TYPE;
    ether_frame->frame_tag_info = L_STDLIB_Hton16(0x2001);
    ether_frame->frame_type = ETHER_IPV4_TYPE;

    L_MM_Mref_MovePdu(mref_handle_p, 18, &pdu_len);

    BACKDOOR_MGR_Printf("\r\n---->>> IP packet:\r\n");
    for (i=0; i<64; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", frame[i]);
        if ( ((i+1)%8) == 0 )
             BACKDOOR_MGR_Printf("\r\n");
    }
    BACKDOOR_MGR_Printf("\r\n");

    LAN_SendPacket(mref_handle_p, damac, samac, L_STDLIB_Ntoh16(ETHER_IPV4_TYPE), taginfo, 46, unit, port, TRUE, 1);

} /* End of LAN_STACKING_SendIpPkt */

/* FUNCTION NAME: LAN_STACKING_SendStaPkt
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_SendStaPkt(UI32_T unit, UI32_T port)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *frame;
    RxFrame_T           *ether_frame;
    UI32_T              pdu_len, i;
    UI16_T              taginfo = 0x2001;
    UI8_T               damac[6]={0x01,0x80,0xC2,0x00,0x00,0x00};
    UI8_T               samac[6]={0x00,0xE0,0xDE,0xAD,0x02,0x00};

    mref_handle_p = L_MM_AllocateTxBuffer(64, /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_STACKING_SENDSTAPKT));

    frame = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (frame == NULL)
        return;

    for (i=0; i<64; i++)
    {
        frame[i] = (UI8_T)i;
    }

    ether_frame = (RxFrame_T *)frame;

    ether_frame->frame_tag_type = ETHER_TAG_TYPE;
    ether_frame->frame_tag_info = L_STDLIB_Hton16(0x2001);
    ether_frame->frame_type = L_STDLIB_Hton16(46); /* length */

    ether_frame->dsap = DSAP_TYPE;
    ether_frame->ssap = SSAP_TYPE;
    ether_frame->ctrl = CTRL_TYPE;
    ether_frame->pid = PROTOCOL_ID_BPDU;

    L_MM_Mref_MovePdu(mref_handle_p, 18, &pdu_len);

    BACKDOOR_MGR_Printf("\r\n---->>> BPDU packet:\r\n");
    for (i=0; i<64; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", frame[i]);
        if ( ((i+1)%8) == 0 )
             BACKDOOR_MGR_Printf("\r\n");
    }
    BACKDOOR_MGR_Printf("\r\n");

    LAN_SendPacket(mref_handle_p, damac, samac, 46, taginfo, 46, unit, port, TRUE, 1);

} /* End of LAN_STACKING_SendStaPkt */

/* FUNCTION NAME: LAN_STACKING_SendGvrpPkt
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 */
static void LAN_STACKING_SendGvrpPkt(UI32_T unit, UI32_T port)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *frame;
    RxFrame_T           *ether_frame;
    UI32_T              pdu_len, i;
    UI16_T              taginfo = 0x2001;
    UI8_T               damac[6]={0x01,0x80,0xC2,0x00,0x00,0x21};
    UI8_T               samac[6]={0x00,0xE0,0xDE,0xAD,0x02,0x00};


    mref_handle_p = L_MM_AllocateTxBuffer(64, /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_STACKING_SENDGVRPPKT));

    frame = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (frame == NULL)
        return;

    for (i=0; i<64; i++)
    {
        frame[i] = (UI8_T)i;
    }

    ether_frame = (RxFrame_T *)frame;

    ether_frame->frame_tag_type = ETHER_TAG_TYPE;
    ether_frame->frame_tag_info = L_STDLIB_Hton16(0x2001);
    ether_frame->frame_type = L_STDLIB_Hton16(46); /* length */

    ether_frame->dsap = DSAP_TYPE;
    ether_frame->ssap = SSAP_TYPE;
    ether_frame->ctrl = CTRL_TYPE;
    ether_frame->pid = PROTOCOL_ID_GVRP;

    L_MM_Mref_MovePdu(mref_handle_p, 18, &pdu_len);

    BACKDOOR_MGR_Printf("\r\n---->>> GVRP packet:\r\n");
    for (i=0; i<64; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", frame[i]);
        if ( ((i+1)%8) == 0 )
             BACKDOOR_MGR_Printf("\r\n");
    }
    BACKDOOR_MGR_Printf("\r\n");

    LAN_SendPacket(mref_handle_p, damac, samac, 46, taginfo, 46, unit, port, TRUE, 1);

} /* End of LAN_STACKING_SendGvrpPkt */

/* FUNCTION NAME: LAN_STACKING_SendLacpPkt
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_SendLacpPkt(UI32_T unit, UI32_T port)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *frame;
    RxFrame_T           *ether_frame;
    UI32_T              pdu_len, i;
    UI16_T              taginfo = 0x2001;
    UI8_T               damac[6]={0x01,0x80,0xC2,0x00,0x00,0x02};
    UI8_T               samac[6]={0x00,0xE0,0xDE,0xAD,0x02,0x00};


    mref_handle_p = L_MM_AllocateTxBuffer(64, /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_STACKING_SENDLACPPKT));

    frame = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (frame == NULL)
        return;

    for (i=0; i<64; i++)
    {
        frame[i] = (UI8_T)i;
    }

    ether_frame = (RxFrame_T *)frame;

    ether_frame->frame_tag_type = ETHER_TAG_TYPE;
    ether_frame->frame_tag_info = L_STDLIB_Hton16(0x2001);
    ether_frame->frame_type = ETHER_LACP_TYPE;

    L_MM_Mref_MovePdu(mref_handle_p, 18, &pdu_len);

    BACKDOOR_MGR_Printf("\r\n---->>> LACP packet:\r\n");
    for (i=0; i<64; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", frame[i]);
        if ( ((i+1)%8) == 0 )
             BACKDOOR_MGR_Printf("\r\n");
    }
    BACKDOOR_MGR_Printf("\r\n");
#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
    LAN_SendPacket(mref_handle_p, damac, samac, L_STDLIB_Ntoh16(ETHER_LACP_TYPE), taginfo, 46, unit, port, TRUE, 1);
#else
    LAN_SendPacket(mref_handle_p, damac, samac, L_STDLIB_Ntoh16(ETHER_LACP_TYPE), taginfo, 46, unit, port, TRUE, LAN_PKT_HIGH_TXCOS);
#endif

} /* End of LAN_STACKING_SendLacpPkt */


static void LAN_STACKING_SendMultiIpPkt(void)
{
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T               *frame;
    RxFrame_T           *ether_frame;
    UI32_T              pdu_len, i;
    UI16_T              taginfo = 0x2001;
    UI8_T               damac[6]={0x01,0x80,0xC2,0x00,0x00,0x05};
    UI8_T               samac[6]={0x00,0xE0,0xDE,0xAD,0x02,0x00};
    UI8_T               tx_port_count_per_unit[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI8_T               uport_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI8_T               untag_port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];


    memset(tx_port_count_per_unit, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
    memset(uport_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);
    memset(untag_port_list, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    tx_port_count_per_unit[1] = 3;          /* multicasting to 3 ports on unit 2 */
    uport_list[0] = 0xE0;                   /* port 1,2,3 */
    untag_port_list[0] = 0x00;              /* tagged: port 1,2,3 */

    mref_handle_p = L_MM_AllocateTxBuffer(64, /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_LAN, LAN_TYPE_TRACE_ID_LAN_STACKING_SENDMULTIIPPKT));

    frame = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if (frame == NULL)
        return;

    for (i=0; i<64; i++)
    {
        frame[i] = (UI8_T)i;
    }

    ether_frame = (RxFrame_T *)frame;

    ether_frame->frame_tag_type = ETHER_TAG_TYPE;
    ether_frame->frame_tag_info = L_STDLIB_Hton16(0x2001);
    ether_frame->frame_type = ETHER_IPV4_TYPE;

    L_MM_Mref_MovePdu(mref_handle_p, 18, &pdu_len);

    BACKDOOR_MGR_Printf("\r\n---->>> Multicast packet:\r\n");

#if 0/* Current solution, CPU packet enqueue 7. anzhen.zheng, 2008-8-15 */
    LAN_SendMultiPacket(mref_handle_p, damac, samac, L_STDLIB_Hton16(ETHER_IP_TYPE), taginfo, 46, uport_list, untag_port_list, tx_port_count_per_unit, 1);
#else
    LAN_SendMultiPacket(mref_handle_p, damac, samac, L_STDLIB_Hton16(ETHER_IPV4_TYPE), taginfo, 46, uport_list, untag_port_list, tx_port_count_per_unit, LAN_PKT_HIGH_TXCOS);
#endif
} /* End of LAN_STACKING_SendMultiIpPkt */

/* FUNCTION NAME: LAN_STACKING_DisplayRecvPacket
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_DisplayRecvPacket(UI8_T *data, UI32_T length)
{
    UI32_T      i;

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n------------- Received Packet for LAN on stacking ------------");
    BACKDOOR_MGR_Printf("\r\n");
    for (i=0; i<length; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", data[i]);
        if ( ((i+1)%8) == 0 )
        {
            BACKDOOR_MGR_Printf("\r\n");
        }
    }
    BACKDOOR_MGR_Printf("\r\n--------------------------------------------------------------");
    BACKDOOR_MGR_Printf("\r\n");
}

/* FUNCTION NAME: LAN_STACKING_DisplayXmitPacket
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
static void LAN_STACKING_DisplayXmitPacket(L_MM_Mref_Handle_T *mref_handle_p)
{
    UI8_T     *data;
    UI32_T    length = 0;
    UI32_T    i;

    data = L_MM_Mref_GetPdu(mref_handle_p, &length);

    BACKDOOR_MGR_Printf("\r\n");
    BACKDOOR_MGR_Printf("\r\n---------- Xmitted Packet to NIC for LAN on stacking ----------");
    BACKDOOR_MGR_Printf("\r\n");
    for (i=0; i<length; i++)
    {
        BACKDOOR_MGR_Printf("0x%02x ", data[i]);
        if ( ((i+1)%8) == 0 )
        {
            BACKDOOR_MGR_Printf("\r\n");
        }
    }
    BACKDOOR_MGR_Printf("\r\n--------------------------------------------------------------");
    BACKDOOR_MGR_Printf("\r\n");
}


/* ----------------------------------------------------------------------------------
 * FUNCTION : LAN_STACKING_DisplayIscMref
 * ----------------------------------------------------------------------------------
 * PURPOSE  : Display ISC packet data which is sent by LAN
 * INPUT    : dst_bmp  -- destination unit bitmap
 *            mref_p   -- mref of packet
 *            priority -- send packet priority
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ----------------------------------------------------------------------------------*/
static void LAN_STACKING_DisplayIscMref(UI16_T dst_bmp, L_MM_Mref_Handle_T *mref_handle_p, UI32_T priority)
{
    StackingHeader_T  *stk_header;
    UI8_T             *frame;
    UI32_T             pdu_len;

    BACKDOOR_MGR_Printf("ISC Packet Data:\n");
    BACKDOOR_MGR_Printf("dst_bmp = %u\n",dst_bmp);
    BACKDOOR_MGR_Printf("priority = %lu\n",(unsigned long)priority);

    frame = (UI8_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    stk_header = (StackingHeader_T*)frame;
    BACKDOOR_MGR_Printf("stk_header.service_id =%ld\n", (long)stk_header->service_id);
    BACKDOOR_MGR_Printf("stk_header.src_unit =%ld\n", (long)stk_header->src_unit);
    BACKDOOR_MGR_Printf("stk_header.src_port =%ld\n", (long)stk_header->src_port);
    BACKDOOR_MGR_Printf("stk_header.pkt_length =%ld\n", (long)stk_header->pkt_length);
    BACKDOOR_MGR_Printf("stk_header.reason =%ld\n", (long)stk_header->reason);
    BACKDOOR_MGR_Printf("stk_header.uport_list = ");
    BACKDOOR_MGR_DumpHex("", SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST,(char*)(stk_header->uport_list));
    BACKDOOR_MGR_Printf("stk_header.uport_untagged_list = ");
    BACKDOOR_MGR_DumpHex("", SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, (char*)(stk_header->uport_untagged_list));
    BACKDOOR_MGR_Printf("stk_header.tx_port_count_per_unit = ");
    BACKDOOR_MGR_DumpHex("", SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, (char*)(stk_header->tx_port_count_per_unit));
    BACKDOOR_MGR_DumpHex("", stk_header->pkt_length, (char*)(&frame[sizeof(StackingHeader_T)]));
}

#endif /* LAN_STACKING_BACKDOOR_OPEN */

#if (SYS_CPNT_AMTR_VLAN_MAC_LEARNING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: LAN_GetValidDrvUnitBmp
 *------------------------------------------------------------------------------
 * PURPOSE: Getting UnitBmp for all exist unit or module.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : unit_bmp    -- which driver units are exist
 * NOTES  : This function is to search all exist unit id by stktplg
 *          then ISC can forward the information to all exist unit.
 *------------------------------------------------------------------------------*/
static UI16_T LAN_GetValidDrvUnitBmp(void)
{
    UI32_T unit_index;
    UI32_T stack_id;
    UI16_T unit_bmp=0;

    stack_id = LAN_OM_GetMyUnitId();
    for (unit_index = 0; STKTPLG_POM_GetNextDriverUnit(&unit_index); )
    {
        if (unit_index == stack_id)
        {
            continue;
        }
        unit_bmp |= BIT_VALUE(unit_index-1);
    }
    return unit_bmp;
}
#endif

