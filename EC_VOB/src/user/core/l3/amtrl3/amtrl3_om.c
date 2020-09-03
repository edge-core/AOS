/* =====================================================================================
 * MODULE NAME: AMTRL3_OM.c
 *
 * PURPOSE: To store and manage AMTRL3 HostRoute and NetRoute Tables.
 *          It also provides HISAM utility for amtrl3 mgr and task.
 *          This database replaces the AMTRL3_VM and AMTRL3_ARPOM in
 *          the original
 *
 * NOTES:   This file is redesign base on the original function designed
 *          in AMTRL3_VM.h
 *
 *  History :
 *      Date            Modifier    Reason
 *  -----------------------------------------------------------------------
 *      04-28-2003        hyliao    First Created
 *      01-12-2004        amytu     Redesign
 *      01-15-2008        djd       Linux porting
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation, 2002,2003,2004,2008
 * =====================================================================================*/


/* INCLUDE FILE DECLARATIONS
 */
#include <sysfun.h>
#include "amtrl3_om.h"
#include "l_hisam.h"
#include "string.h"
#include "l_dlist.h"
#include "l_sort_lst.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "leaf_2096.h"
#include "leaf_4001.h"
#include "amtrl3_type.h"
//#include "xmalloc.h"
#include "stdlib.h"
#include "ip_lib.h"

/* NAME CONSTANT DECLARATIONS
 */
#define AMTRL3_OM_SIZEOF(s, memb)                   (sizeof(((s *)0)->memb))
#define AMTRL3_OM_OFFSETOF(s, memb)                 (((char *)&((s *)0)->memb-(char *)0))
#define AMTRL3_OM_KEYDEF(idx, s, memb)              .offset[idx] = AMTRL3_OM_OFFSETOF(s, memb), \
                                                    .length[idx] = AMTRL3_OM_SIZEOF(s,memb)

/* INTERFACE DEFINITION
 */
#define AMTRL3_OM_MAX_INTERFACE_NBR                 SYS_ADPT_MAX_NBR_OF_VLAN    /*  max vlan in this system */

/* HOST ROUTE DEFINITION
 */
#define AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_INET      (AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id))

#define AMTRL3_OM_HOST_ROUTE_KEYLEN_PORT_IFIDX_INET (AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.lport) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id))

#define AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_MAC_INET  (AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_mac) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr) + \
                                                     AMTRL3_OM_SIZEOF(AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id))

#define AMTRL3_OM_HOST_ROUTE_ENTRY_NBR              SYS_ADPT_MAX_NBR_OF_HOST_ROUTE   /* 2k routing entries (5615) */
#define AMTRL3_OM_HOST_ROUTE_INDEX_NBR              ((AMTRL3_OM_HOST_ROUTE_ENTRY_NBR * 2 / AMTRL3_OM_HOST_ROUTE_HISAM_N2) + 1) /* index number */
#define AMTRL3_OM_HOST_ROUTE_HISAM_N1               4                                           /* N1 */
#define AMTRL3_OM_HOST_ROUTE_HISAM_N2               (64 + 4)                                    /* N2 */
#define AMTRL3_OM_HOST_ROUTE_HASH_DEPTH             4                                           /* hash depth */
#define AMTRL3_OM_HOST_ROUTE_HASH_NBR               (AMTRL3_OM_HOST_ROUTE_ENTRY_NBR / 16 + 16)  /* hash number */
#define AMTRL3_OM_HOST_ROUTE_ENTRY_LEN              (sizeof(AMTRL3_OM_HostRouteEntry_T))

/* NET ROUTE DEFINITION
 */
#define SIZE_OF_INET_CIDR_ROUTE_FIBID               AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, fib_id)
#define SIZE_OF_INET_CIDR_ROUTE_DEST                AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_dest)
#define SIZE_OF_INET_CIDR_ROUTE_POLICY              AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_policy)
#define SIZE_OF_INET_CIDR_ROUTE_NHOP                AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_next_hop)
#define SIZE_OF_INET_CIDR_ROUTE_PROTO               AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_proto)
#define SIZE_OF_INET_ROUTE_PREFIX                   AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_pfxlen)
#define SIZE_OF_INET_CIDR_ROUTE_IFINDEX             AMTRL3_OM_SIZEOF(AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_if_index)

//#define SIZE_OF_NET_ROUTE_LPORT                     AMTRL3_OM_SIZEOF(AMTRL3_OM_NetRouteEntry_T, lport)  /*-*/
#define AMTRL3_OM_NET_ROUTE_ENTRY_LEN               (sizeof(AMTRL3_OM_NetRouteEntry_T))
#define AMTRL3_OM_NET_ROUTE_KEY_LEN                 (SIZE_OF_INET_CIDR_ROUTE_DEST + \
                                                     SIZE_OF_INET_ROUTE_PREFIX   + \
                                                     SIZE_OF_INET_CIDR_ROUTE_POLICY  + \
                                                     SIZE_OF_INET_CIDR_ROUTE_FIBID + \
                                                     SIZE_OF_INET_CIDR_ROUTE_NHOP)
#define AMTRL3_OM_NET_ROUTE_IFINDEX_KEY_LEN         (AMTRL3_OM_NET_ROUTE_KEY_LEN + \
                                                     SIZE_OF_INET_CIDR_ROUTE_FIBID + \
                                                     SIZE_OF_INET_CIDR_ROUTE_IFINDEX)

/*
#define AMTRL3_OM_NET_ROUTE_LPORT_KEY_LEN           (SIZE_OF_NET_ROUTE_LPORT       + \
                                                     SIZE_OF_INET_CIDR_ROUTE_PROTO   + \
                                                     AMTRL3_OM_NET_ROUTE_KEY_LEN )
*/
#define AMTRL3_OM_NET_ROUTE_ENTRY_NBR               (SYS_ADPT_MAX_NBR_OF_TOTAL_ROUTE_ENTRY + \
                                                     SYS_ADPT_MAX_NBR_OF_RIF)                   /* dynamic ROUTE + static ROUTE + local ROUTE */
#define AMTRL3_OM_NET_ROUTE_INDEX_NBR               ((AMTRL3_OM_NET_ROUTE_ENTRY_NBR * 2 / AMTRL3_OM_NET_ROUTE_HISAM_N2) + 1)        /* index number */
#define AMTRL3_OM_NET_ROUTE_HISAM_N1                4                                           /* N1 */
#define AMTRL3_OM_NET_ROUTE_HISAM_N2                (64 + 4)                                    /* N2 */
#define AMTRL3_OM_NET_ROUTE_HASH_DEPTH              4                                           /* hash depth */
#define AMTRL3_OM_NET_ROUTE_HASH_NBR                (AMTRL3_OM_NET_ROUTE_ENTRY_NBR / 16 + 16)   /* hash number */

#if (SYS_CPNT_VXLAN == TRUE)
#define AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR            256
#define AMTRL3_OM_VXLAN_TUNNEL_INDEX_NBR            ((AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR * 2 / AMTRL3_OM_VXLAN_TUNNEL_HISAM_N2) + 1)
#define AMTRL3_OM_VXLAN_TUNNEL_HISAM_N1             4
#define AMTRL3_OM_VXLAN_TUNNEL_HISAM_N2             (14 + 4)
#define AMTRL3_OM_VXLAN_TUNNEL_HASH_DEPTH           4
#define AMTRL3_OM_VXLAN_TUNNEL_HASH_NBR             (AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR / 16 + 16)
#define AMTRL3_OM_VXLAN_TUNNEL_ENTRY_LEN            sizeof(AMTRL3_OM_VxlanTunnelEntry_T)

#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_ENTRY_NBR    (256 * SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE)
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_INDEX_NBR    ((AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR * 2 / AMTRL3_OM_VXLAN_TUNNEL_HISAM_N2) + 1)
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HISAM_N1     4
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HISAM_N2     (14 + 4)
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HASH_DEPTH   4
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HASH_NBR     (AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR / 16 + 16)
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_ENTRY_LEN            sizeof(AMTRL3_OM_VxlanTunnelNexthopEntry_T)

#define SIZE_OF_VXLAN_TUNNEL_VFI_ID                 AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelEntry_T, vfi_id)
#define SIZE_OF_VXLAN_TUNNEL_LOCAL_VTEP             AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelEntry_T, local_vtep)
#define SIZE_OF_VXLAN_TUNNEL_REMOTE_VTEP            AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelEntry_T, remote_vtep)
#define AMTRL3_OM_VXLAN_TUNNEL_KEY_LEN              (SIZE_OF_VXLAN_TUNNEL_VFI_ID + SIZE_OF_VXLAN_TUNNEL_LOCAL_VTEP + SIZE_OF_VXLAN_TUNNEL_REMOTE_VTEP)

#define SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID         AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelNexthopEntry_T, vfi_id)
#define SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP     AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelNexthopEntry_T, local_vtep)
#define SIZE_OF_VXLAN_TUNNEL_NEXTHOP_REMOTE_VTEP    AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelNexthopEntry_T, remote_vtep)
#define SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR   AMTRL3_OM_SIZEOF(AMTRL3_OM_VxlanTunnelNexthopEntry_T, nexthop_addr)
#define AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_LEN      (SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_REMOTE_VTEP + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR)
#endif

/* RESOLVED NET ROUTE DEFINITION
 */
#define AMTRL3_OM_RESOLED_NET_ROUTE_FIBID           AMTRL3_OM_SIZEOF(AMTRL3_OM_ResolvedNetRouteEntry_T, fib_id)
#define AMTRL3_OM_RESOLED_NET_ROUTE_INVERSE_PFXLEN  AMTRL3_OM_SIZEOF(AMTRL3_OM_ResolvedNetRouteEntry_T, inverse_prefix_length)
#define AMTRL3_OM_RESOLED_NET_ROUTE_DEST            AMTRL3_OM_SIZEOF(AMTRL3_OM_ResolvedNetRouteEntry_T, inet_cidr_route_dest)
#define AMTRL3_OM_RESOLED_NET_ROUTE_NHOP            AMTRL3_OM_SIZEOF(AMTRL3_OM_ResolvedNetRouteEntry_T, inet_cidr_route_next_hop)

#define AMTRL3_OM_RESOLVED_NET_ROUTE_KEY_LEN        (AMTRL3_OM_RESOLED_NET_ROUTE_INVERSE_PFXLEN + \
                                                     AMTRL3_OM_RESOLED_NET_ROUTE_FIBID + \
                                                     AMTRL3_OM_RESOLED_NET_ROUTE_DEST + \
                                                     AMTRL3_OM_RESOLED_NET_ROUTE_NHOP)
#define AMTRL3_OM_RESOLVED_NET_ROUTE_ENTRY_LEN      (sizeof(AMTRL3_OM_ResolvedNetRouteEntry_T))

/*#define AMTRL3_OM_MASK(len)                      (~((1<<(32-len))-1))*/

/* DATA TYPE DECLARATIONS
 */

enum AMTRL3_OM_HOST_ROUTE_KEY_E
{
    AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX = 0,   /* key: fib_id + ifindex + inet_addr */
    AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX,  /* key: fib_id + lport + ifindex + inet_addr */
    AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX,   /* key: fib_id + ifindex + mac + inet_addr */
    AMTRL3_OM_HOST_ROUTE_NUMBER_OF_KEY
};

enum AMTRL3_OM_NET_ROUTE_KEY_E
{
    /* KEY: fib_id, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy, inet_cidr_route_next_hop
     */
    AMTRL3_OM_NET_ROUTE_KIDX = 0,

    /* KEY: fib_id, inet_cidr_route_next_hop, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy
     */
    AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP,

    /* KEY: fib_id, inet_cidr_route_if_index, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy
     */
    AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX,
    AMTRL3_OM_NET_ROUTE_NUMBER_OF_KEY
};

enum AMTRL3_OM_RESOLVED_NET_ROUTE_KEY_E
{
    AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX = 0, /* fib_id, inverse prefix length, dest_address, nexthop_address*/
    AMTRL3_OM_RESOLVED_NET_ROUTE_NUMBER_OF_KEY
};

enum AMTRL3_OM_NUM_OF_KEY_E
{
    AMTRL3_OM_NUM_OF_KEY_ONE = 1,
    AMTRL3_OM_NUM_OF_KEY_TWO,
    AMTRL3_OM_NUM_OF_KEY_THREE,
    AMTRL3_OM_NUM_OF_KEY_FOUR,
    AMTRL3_OM_NUM_OF_KEY_FIVE,
    AMTRL3_OM_NUM_OF_KEY_SIX
};

enum AMTRL3_OM_DOUBLE_LINK_LIST_OPERATION_E
{
    ENQUEUE_ENTRY,
    ENQUEUE_ENTRY_TO_HEAD,
    DEQUEUE_ENTRY,
    DELETE_ENTRY,
    DELETE_LIST
};

#if (SYS_CPNT_VXLAN == TRUE)
enum AMTRL3_OM_VXLAN_TUNNEL_KEY_E
{
    AMTRL3_OM_VXLAN_TUNNEL_KIDX = 0, /* KEY: vfi_id, local_vtep, remote_vtep */
    AMTRL3_OM_VXLAN_TUNNEL_NUMBER_OF_KEY
};

enum AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_E
{
    AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX = 0, /* KEY: vfi_id, local_vtep, remote_vtep, nexthop_addr */
    AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP, /* KEY: nexthop_addr, vfi_id, local_vtep, remote_vtep */
    AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_NUMBER_OF_KEY
};

const static L_HISAM_KeyDef_T amtrl3_om_vxlan_tunnel_key_def_table [AMTRL3_OM_VXLAN_TUNNEL_NUMBER_OF_KEY] = {
                    [AMTRL3_OM_VXLAN_TUNNEL_KIDX] = {
                        /* KEY: vfi_id, local_vtep, remote_vtep */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_THREE,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_VxlanTunnelEntry_T, vfi_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_VxlanTunnelEntry_T, local_vtep),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_VxlanTunnelEntry_T, remote_vtep),
                    },
                };

const static L_HISAM_KeyDef_T amtrl3_om_vxlan_tunnel_nexthop_key_def_table [AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_NUMBER_OF_KEY] = {
                    [AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX] = {
                        /* KEY: vfi_id, local_vtep, remote_vtep, nexthop_addr */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FOUR,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_VxlanTunnelNexthopEntry_T, vfi_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_VxlanTunnelNexthopEntry_T, local_vtep),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_VxlanTunnelNexthopEntry_T, remote_vtep),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_OM_VxlanTunnelNexthopEntry_T, nexthop_addr),
                    },

                    [AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP] = {
                        /* KEY: vfi_id, local_vtep, remote_vtep, nexthop_addr */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FOUR,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_VxlanTunnelNexthopEntry_T, nexthop_addr),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_VxlanTunnelNexthopEntry_T, vfi_id),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_VxlanTunnelNexthopEntry_T, local_vtep),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_OM_VxlanTunnelNexthopEntry_T, remote_vtep),
                    },
                };
#endif

typedef struct
{
    L_INET_AddrIp_T search_key1;
    UI32_T          search_key2;
    UI32_T          vid_ifindex;
    UI32_T          request_counter;
    UI32_T          retrieved_counter;
    UI32_T          fib_id;
    void            *retrieved_record;
    UI8_T           mac[SYS_ADPT_MAC_ADDR_LEN];
} AMTRL3_OM_HisamCookie_T;

static AMTRL3_OM_FIB_T  *amtrl3_om_fib_p[SYS_ADPT_MAX_NUMBER_OF_FIB] = {0};
static AMTRL3_OM_FIB_T  amtrl3_om_default_fib;
static UI32_T           amtrl3_om_sem_id;
static UI8_T            amtrl3_om_zero_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};
static UI32_T           amtrl3_om_vid2fib[SYS_ADPT_MAX_NBR_OF_VLAN]; // 0-based
static UI32_T           amtrl3_om_fib_vid_cnt[SYS_ADPT_MAX_NUMBER_OF_FIB]; // 0-based

const static L_HISAM_KeyType_T amtrl3_om_host_route_key_type_table[AMTRL3_OM_HOST_ROUTE_NUMBER_OF_KEY][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* fib_id, ifindex, inet_addr */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP, L_HISAM_NOT_INTEGER,
                     L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER },
                    /* fib_id, lport + ifindex + inet_addr */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP,
                     L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER },
                    /* fib_id, ifindex + mac + inet_addr */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER, L_HISAM_INET_ADDR_IP,
                     L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER, L_HISAM_NOT_INTEGER }
                };

const static L_HISAM_KeyDef_T amtrl3_om_host_route_key_def_table [AMTRL3_OM_HOST_ROUTE_NUMBER_OF_KEY] = {
                    [AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX] = {
                        /* key: fib_id + ifindex + inet_addr */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_THREE,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr),
                   },

                    [AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX] = {
                        /* key: fib_id + lport + ifindex + inet_addr */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FOUR,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_HostRouteEntry_T, key_fields.lport),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr),
                    },

                    [AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX] = {
                        /* key: fib_id + ifindex + mac + inet_addr */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FOUR,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_HostRouteEntry_T, key_fields.fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_vid_ifindex),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_mac),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_OM_HostRouteEntry_T, key_fields.dst_inet_addr),
                    },
                };

const static L_HISAM_KeyType_T amtrl3_om_net_route_key_type_table[AMTRL3_OM_NET_ROUTE_NUMBER_OF_KEY][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* fib_id, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy, inet_cidr_route_next_hop */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP, L_HISAM_4BYTE_INTEGER, L_HISAM_NOT_INTEGER,
                     L_HISAM_INET_ADDR_IP,  L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER, },
                    /* fib_id, inet_cidr_route_next_hop, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP, L_HISAM_INET_ADDR_IP, L_HISAM_4BYTE_INTEGER,
                     L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER,  },
                    /* fib_id, inet_cidr_route_if_index, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_next_hop, inet_cidr_route_policy */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP, L_HISAM_4BYTE_INTEGER,
                     L_HISAM_INET_ADDR_IP,  L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER, }
                };

const static L_HISAM_KeyDef_T amtrl3_om_net_route_key_def_table [AMTRL3_OM_NET_ROUTE_NUMBER_OF_KEY] = {
                    [AMTRL3_OM_NET_ROUTE_KIDX] = {
                        /* fib_id, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy, inet_cidr_route_next_hop */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FIVE,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_dest),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_pfxlen),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_policy),
                        AMTRL3_OM_KEYDEF(4, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_next_hop),
                    },

                    [AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP] = {
                        /* fib_id, inet_cidr_route_next_hop, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_policy */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FIVE,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_next_hop),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_dest),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_pfxlen),
                        AMTRL3_OM_KEYDEF(4, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_policy),
                    },

                    [AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX] = {
                        /* fib_id, inet_cidr_route_if_index, inet_cidr_route_dest, inet_cidr_route_pfxlen, inet_cidr_route_next_hop, inet_cidr_route_policy */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_SIX,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_if_index),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_dest),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_pfxlen),
                        AMTRL3_OM_KEYDEF(4, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_next_hop),
                        AMTRL3_OM_KEYDEF(5, AMTRL3_TYPE_InetCidrRoutePartialEntry_T, inet_cidr_route_policy),
                    },
                };

const static L_HISAM_KeyType_T amtrl3_om_resolved_net_route_key_type_table[AMTRL3_OM_RESOLVED_NET_ROUTE_NUMBER_OF_KEY][L_HISAM_MAX_FIELD_NUMBER_OF_A_KEY] =
                {
                    /* fib_id, inverse_prefix_length, inet_cidr_route_dest, inet_cidr_route_next_hop */
                    {L_HISAM_4BYTE_INTEGER, L_HISAM_4BYTE_INTEGER, L_HISAM_INET_ADDR_IP, L_HISAM_INET_ADDR_IP,
                     L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,   L_HISAM_NOT_INTEGER,  L_HISAM_NOT_INTEGER, }
                };

const static L_HISAM_KeyDef_T amtrl3_om_resolved_net_route_key_def_table [AMTRL3_OM_RESOLVED_NET_ROUTE_NUMBER_OF_KEY] = {
                    [AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX] = {
                        /* fib_id, inverse_prefix_length, inet_cidr_route_dest, inet_cidr_route_next_hop */
                        .field_number = AMTRL3_OM_NUM_OF_KEY_FOUR,
                        AMTRL3_OM_KEYDEF(0, AMTRL3_OM_ResolvedNetRouteEntry_T, fib_id),
                        AMTRL3_OM_KEYDEF(1, AMTRL3_OM_ResolvedNetRouteEntry_T, inverse_prefix_length),
                        AMTRL3_OM_KEYDEF(2, AMTRL3_OM_ResolvedNetRouteEntry_T, inet_cidr_route_dest),
                        AMTRL3_OM_KEYDEF(3, AMTRL3_OM_ResolvedNetRouteEntry_T, inet_cidr_route_next_hop),
                    },
                };

static AMTRL3_OM_FIB_T *AMTRL3_OM_GetFIBByID(UI32_T fib_id);
static void AMTRL3_OM_SetHostRouteKey(UI8_T *key, UI32_T key_index,  AMTRL3_OM_HostRouteEntry_T  *host_entry);
static void AMTRL3_OM_SetNetRouteKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_NetRouteEntry_T *net_route_entry);
static void AMTRL3_OM_SetResolvedNetRouteKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_ResolvedNetRouteEntry_T  *resolved_net_entry);
#if (SYS_CPNT_VXLAN == TRUE)
static void AMTRL3_OM_SetVxlanTunnelKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p);
static void AMTRL3_OM_SetVxlanTunnelNexthopKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p);
#endif
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie);
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByIfIndex_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie);
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByStatus_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie);
static UI32_T AMTRL3_OM_GetNextNHostRouteEntry_CallBack(AMTRL3_OM_HostRouteEntry_T *host_route_entry, void* cookie);
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie);
static BOOL_T AMTRL3_OM_DoulbleLinkListOperation(UI32_T action_flags,AMTRL3_OM_FIB_T  *om_fib, UI32_T list_operation, L_INET_AddrIp_T *host_route_ip, UI32_T host_route_status, UI32_T list_index);
static int AMTRL3_OM_CompareInterface(void* rec1, void* rec2);
static void AMTRL3_OM_LocalResetVid2Fib(UI32_T fib_id);


/*Simon's debug function*/
#define DEBUG_FLAG_BIT_DEBUG 0x01
#define DEBUG_FLAG_BIT_INFO  0x02
#define DEBUG_FLAG_BIT_NOTE  0x04
#define UNUSED __attribute__ ((__unused__))
/***************************************************************/
static UNUSED unsigned long DEBUG_FLAG = 0;//DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;
/***************************************************************/
#define DBGprintf(format,args...) ((DEBUG_FLAG_BIT_DEBUG & DEBUG_FLAG)==0)?:BACKDOOR_MGR_Printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define INFOprintf(format,args...) ((DEBUG_FLAG_BIT_INFO & DEBUG_FLAG)==0)?:BACKDOOR_MGR_Printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)
#define NOTEprintf(format,args...) ((DEBUG_FLAG_BIT_NOTE & DEBUG_FLAG)==0)?:BACKDOOR_MGR_Printf("%s:%d:"format"\r\n",__FUNCTION__,__LINE__ ,##args)


/*Simon's dump memory  function*/
#define DUMP_MEMORY_BYTES_PER_LINE 32
#define DUMP_MEMORY(mptr,size) do{\
    int i;\
    unsigned char * ptr=(unsigned char *)mptr;\
    for(i=0;i<size;i++){\
        if(i%DUMP_MEMORY_BYTES_PER_LINE ==0){\
            if(i>0)printf("\r\n");\
            printf("%0.4xH\t",i);\
        }\
        printf("%0.2x", *ptr++);\
    }\
    printf("\r\n");\
}while(0)
#define BACKDOOR_SET_DEBUG_FLAG() do{\
    UI8_T   ch;\
    BACKDOOR_MGR_Printf("Select debug level (1~3, set 0 to clear)\n");\
    ch = BACKDOOR_MGR_GetChar();\
    BACKDOOR_MGR_Printf ("%c\n",ch);\
    switch (ch)\
    {\
        case '0':\
            DEBUG_FLAG =0;\
            break;\
        case '1':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_NOTE;\
            break;\
        case '2':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO;\
            break;\
        case '3':\
            DEBUG_FLAG = DEBUG_FLAG_BIT_DEBUG|DEBUG_FLAG_BIT_INFO|DEBUG_FLAG_BIT_NOTE;\
            break;\
        default:\
            break;\
    }\
}while(0)
/*END Simon's debug function*/


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * PURPOSE:  Init the OM resouces.
 * INPUT:    none
 * OUTPUT:   none.
 * RETURN:
 * NOTES:
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_Initiate_System_Resources(void)
{
    UI32_T fib_id, vid;

    if(SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &amtrl3_om_sem_id)!= SYSFUN_OK)
        SYSFUN_Debug_Printf("AMTRL3 OM Create Semphore ID Fail\n");

    for(fib_id = 0; fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB; fib_id++)
    {
        amtrl3_om_fib_p[fib_id] = NULL;
        amtrl3_om_fib_vid_cnt[fib_id] = 0;
    }

    for(vid = 0; vid < SYS_ADPT_MAX_NBR_OF_VLAN; vid++)
        amtrl3_om_vid2fib[vid] = SYS_ADPT_DEFAULT_FIB;

    amtrl3_om_fib_vid_cnt[SYS_ADPT_DEFAULT_FIB] = SYS_ADPT_MAX_NBR_OF_VLAN;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetVid2Fib
 * -------------------------------------------------------------------------
 * PURPOSE:  Set vid2fib table by vid and fib id.
 * INPUT:    fib_id - FIB id (0 ~ 254).
 *           vid    - VLAN ID
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE.
 * NOTES:    None.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetVid2Fib(UI32_T vid, UI32_T fib_id)
{
    UI32_T  orig_priority, old_fib_id;
    BOOL_T  ret = FALSE;

    if (  (0 < vid)
        &&(vid < SYS_ADPT_MAX_NBR_OF_VLAN)
        &&(fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB)
       )
    {
        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
        old_fib_id = amtrl3_om_vid2fib[vid-1];
        if (amtrl3_om_fib_vid_cnt[old_fib_id] > 0)
            amtrl3_om_fib_vid_cnt[old_fib_id] -=1 ;

        amtrl3_om_vid2fib[vid-1] = fib_id;

        amtrl3_om_fib_vid_cnt[fib_id] +=1 ;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        ret = TRUE;
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetFibIdByVid
 * -------------------------------------------------------------------------
 * PURPOSE:  Get fib id by vid.
 * INPUT:    vid
 * OUTPUT:   None.
 * RETURN:   fib id
 * NOTES:    None.
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetFibIdByVid(UI32_T vid)
{
    UI32_T  orig_priority;
    UI32_T  ret = SYS_ADPT_DEFAULT_FIB;

    if ((0 < vid) && (vid < SYS_ADPT_MAX_NBR_OF_VLAN))
    {
        orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
        ret = amtrl3_om_vid2fib[vid-1];
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_CreateFIB
 * -------------------------------------------------------------------------
 * PURPOSE:  This function initialize amtrl3_om for the FIB.
 *           Information contain in this OM that shall be initialized
 *           are host_table, net_table, unresolve_list, resolve_list,
 *           sort_by_port_list.
 * INPUT:    fib_id:   FIB id.
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS ,
 *           AMTRL3_TYPE_FIB_FAIL,
 *           AMTRL3_TYPE_FIB_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_CreateFIB(UI32_T fib_id)
{
    BOOL_T          ret;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(fib_id >= SYS_ADPT_MAX_NUMBER_OF_FIB)
        return AMTRL3_TYPE_FIB_FAIL;

    /* bcz SYS_ADPT_DEFAULT_FIB's om is shared for all fib_id
     */
    if(SYS_ADPT_DEFAULT_FIB != fib_id)
        return AMTRL3_TYPE_FIB_SUCCESS;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) != NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FIB_ALREADY_EXIST;
    }

    if(SYS_ADPT_DEFAULT_FIB == fib_id)
    {
        om_fib = &amtrl3_om_default_fib;
        memset(om_fib, 0, sizeof(AMTRL3_OM_FIB_T));
    }
    else
    {
        if((om_fib = malloc(sizeof(AMTRL3_OM_FIB_T))) == NULL)
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return AMTRL3_TYPE_FIB_FAIL;
        }
        memset(om_fib, 0, sizeof(AMTRL3_OM_FIB_T));
    }

    amtrl3_om_fib_p[fib_id] = om_fib;
    om_fib->fib_id = fib_id;

    /* Create interface list */
    om_fib->intf_list = (L_SORT_LST_List_T *)malloc(sizeof(L_SORT_LST_List_T));
    if (NULL == om_fib->intf_list)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FIB_FAIL;
    }

    if (L_SORT_LST_Create (om_fib->intf_list,
                           AMTRL3_OM_MAX_INTERFACE_NBR,
                           sizeof(AMTRL3_OM_Interface_T),
                           AMTRL3_OM_CompareInterface)==FALSE)
    {
        /* DEBUG */
        SYSFUN_Debug_Printf("AMTRL3_OM_CreateFIB : Can't create interface List.\n");
    }

    /* Initiate global var */
    om_fib->amtrl3_om_ipv4_host_entry_ageout_time = SYS_DFLT_ARP_DEF_TTL * 60;
    om_fib->amtrl3_om_ipv4_super_netting_status = AMTRL3_TYPE_SUPER_NET_ENABLE;
    om_fib->amtrl3_om_ipv4_software_forwarding_status = AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE;
    om_fib->amtrl3_om_ipv6_host_entry_ageout_time = SYS_DFLT_NEIGHBOR_DEF_TTL * 60;
    om_fib->amtrl3_om_ipv6_super_netting_status = AMTRL3_TYPE_SUPER_NET_ENABLE;
    om_fib->amtrl3_om_ipv6_software_forwarding_status = AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE;
    //om_fib->amtrl3_om_ipv4_default_route_entry.action_type = SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE;

    /* initiate host route hisam hisam_descriptor
     */
    /* host route-ipv4 */
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.hash_depth      = AMTRL3_OM_HOST_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.N1              = AMTRL3_OM_HOST_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.N2              = AMTRL3_OM_HOST_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.record_length   = AMTRL3_OM_HOST_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.total_hash_nbr  = AMTRL3_OM_HOST_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.total_index_nbr = AMTRL3_OM_HOST_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv4_host_route_hisam_desc.total_record_nbr= AMTRL3_OM_HOST_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv4_host_route_hisam_desc,
                                  AMTRL3_OM_HOST_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_host_route_key_def_table,
                                  amtrl3_om_host_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv4 host_route HISAM failed.\n");
    }

    /* host route-ipv6 */
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.hash_depth      = AMTRL3_OM_HOST_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.N1              = AMTRL3_OM_HOST_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.N2              = AMTRL3_OM_HOST_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.record_length   = AMTRL3_OM_HOST_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.total_hash_nbr  = AMTRL3_OM_HOST_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.total_index_nbr = AMTRL3_OM_HOST_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv6_host_route_hisam_desc.total_record_nbr= AMTRL3_OM_HOST_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv6_host_route_hisam_desc,
                                  AMTRL3_OM_HOST_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_host_route_key_def_table,
                                  amtrl3_om_host_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv6 host_route HISAM failed.\n");
    }

    /* initiate net route hisam hisam_descriptor
     */
    /* net route-ipv4 */
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.hash_depth       = AMTRL3_OM_NET_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.N1               = AMTRL3_OM_NET_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.N2               = AMTRL3_OM_NET_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.record_length    = AMTRL3_OM_NET_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.total_hash_nbr   = AMTRL3_OM_NET_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.total_index_nbr  = AMTRL3_OM_NET_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv4_net_route_hisam_desc.total_record_nbr = AMTRL3_OM_NET_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv4_net_route_hisam_desc,
                                  AMTRL3_OM_NET_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_net_route_key_def_table,
                                  amtrl3_om_net_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv4 net_route HISAM failed.\n");
    }

    /* net route-ipv6 */
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.hash_depth       = AMTRL3_OM_NET_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.N1               = AMTRL3_OM_NET_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.N2               = AMTRL3_OM_NET_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.record_length    = AMTRL3_OM_NET_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.total_hash_nbr   = AMTRL3_OM_NET_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.total_index_nbr  = AMTRL3_OM_NET_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv6_net_route_hisam_desc.total_record_nbr = AMTRL3_OM_NET_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv6_net_route_hisam_desc,
                                  AMTRL3_OM_NET_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_net_route_key_def_table,
                                  amtrl3_om_net_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv6 net_route HISAM failed.\n");
    }

    /* initiate resolved net route hisam hisam_descriptor
     */
    /* resolved net route-ipv4 */
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.hash_depth      = AMTRL3_OM_NET_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.N1              = AMTRL3_OM_NET_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.N2              = AMTRL3_OM_NET_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.record_length   = AMTRL3_OM_RESOLVED_NET_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.total_hash_nbr  = AMTRL3_OM_NET_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.total_index_nbr = AMTRL3_OM_NET_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc.total_record_nbr= AMTRL3_OM_NET_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc,
                                  AMTRL3_OM_RESOLVED_NET_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_resolved_net_route_key_def_table,
                                  amtrl3_om_resolved_net_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv4 resolved_net_route HISAM failed.\n");
    }

    /* resolved net route-ipv6 */
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.hash_depth      = AMTRL3_OM_NET_ROUTE_HASH_DEPTH;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.N1              = AMTRL3_OM_NET_ROUTE_HISAM_N1;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.N2              = AMTRL3_OM_NET_ROUTE_HISAM_N2;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.record_length   = AMTRL3_OM_RESOLVED_NET_ROUTE_ENTRY_LEN;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.total_hash_nbr  = AMTRL3_OM_NET_ROUTE_HASH_NBR;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.total_index_nbr = AMTRL3_OM_NET_ROUTE_INDEX_NBR;
    om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc.total_record_nbr= AMTRL3_OM_NET_ROUTE_ENTRY_NBR;
    if (FALSE == L_HISAM_CreateV2(&om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc,
                                  AMTRL3_OM_RESOLVED_NET_ROUTE_NUMBER_OF_KEY,
                                  amtrl3_om_resolved_net_route_key_def_table,
                                  amtrl3_om_resolved_net_route_key_type_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create ipv6 resolved_net_route HISAM failed.\n");
    }

#if (SYS_CPNT_VXLAN == TRUE)
    /* vxlan tunnel */
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.hash_depth        = AMTRL3_OM_VXLAN_TUNNEL_HASH_DEPTH;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.N1                = AMTRL3_OM_VXLAN_TUNNEL_HISAM_N1;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.N2                = AMTRL3_OM_VXLAN_TUNNEL_HISAM_N2;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.record_length     = AMTRL3_OM_VXLAN_TUNNEL_ENTRY_LEN;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.total_hash_nbr    = AMTRL3_OM_VXLAN_TUNNEL_HASH_NBR;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.total_index_nbr   = AMTRL3_OM_VXLAN_TUNNEL_INDEX_NBR;
    om_fib->amtrl3_om_vxlan_tunnel_hisam_desc.total_record_nbr  = AMTRL3_OM_VXLAN_TUNNEL_ENTRY_NBR;

    if (FALSE == L_HISAM_Create(&(om_fib->amtrl3_om_vxlan_tunnel_hisam_desc), AMTRL3_OM_VXLAN_TUNNEL_NUMBER_OF_KEY, amtrl3_om_vxlan_tunnel_key_def_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create vxlan tunnel HISAM failed.\n");
    }

    /* vxlan tunnel nexthop */
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.hash_depth        = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HASH_DEPTH;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.N1                = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HISAM_N1;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.N2                = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HISAM_N2;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.record_length     = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_ENTRY_LEN;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.total_hash_nbr    = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_HASH_NBR;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.total_index_nbr   = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_INDEX_NBR;
    om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc.total_record_nbr  = AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_ENTRY_NBR;

    if (FALSE == L_HISAM_Create(&(om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc), AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_NUMBER_OF_KEY, amtrl3_om_vxlan_tunnel_nexthop_key_def_table))
    {
        SYSFUN_Debug_Printf("AMTRL3_OM create vxlan tunnel HISAM failed.\n");
    }
#endif

    /* Create Double link list for Unresolve_list, resolve_list, gateway list
     */
    ret = L_DLST_Indexed_Dblist_Create(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), SYS_ADPT_MAX_NBR_OF_HOST_ROUTE);
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv4_unresolve_host_entry_list fails\n");

    ret = L_DLST_Indexed_Dblist_Create(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), SYS_ADPT_MAX_NBR_OF_HOST_ROUTE);
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv6_unresolve_host_entry_list fails\n");

    ret = L_DLST_Indexed_Dblist_Clone(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), &(om_fib->amtrl3_om_ipv4_resolve_host_entry_list));
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv4_resolve_host_entry_list fails\n");

    ret = L_DLST_Indexed_Dblist_Clone(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), &(om_fib->amtrl3_om_ipv6_resolve_host_entry_list));
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv6_resolve_host_entry_list fails\n");

    ret = L_DLST_Indexed_Dblist_Clone(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), &(om_fib->amtrl3_om_ipv4_gateway_entry_list));
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv4_gateway_entry_list fails\n");

    ret = L_DLST_Indexed_Dblist_Clone(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), &(om_fib->amtrl3_om_ipv6_gateway_entry_list));
    if (!ret)
        SYSFUN_Debug_Printf("Initialize amtrl3_om_ipv6_gateway_entry_list fails\n");

    memset(&(om_fib->amtrl3_om_ipv4_first_unresolved_dip), 0, sizeof(L_INET_AddrIp_T));
    memset(&(om_fib->amtrl3_om_ipv6_first_unresolved_dip), 0, sizeof(L_INET_AddrIp_T));

    om_fib->amtrl3_om_ipv4_first_unresolved_dip.type = L_INET_ADDR_TYPE_IPV4;
    om_fib->amtrl3_om_ipv6_first_unresolved_dip.type = L_INET_ADDR_TYPE_IPV6;

    om_fib->amtrl3_om_ipv4_first_unresolved_fib = 0;
    om_fib->amtrl3_om_ipv6_first_unresolved_fib = 0;

    /* init */
    memset(&(om_fib->amtrl3_om_ipv4_first_gateway_dip), 0, sizeof(L_INET_AddrIp_T));
    memset(&(om_fib->amtrl3_om_ipv6_first_gateway_dip), 0, sizeof(L_INET_AddrIp_T));

    om_fib->amtrl3_om_ipv4_first_gateway_fib = 0;
    om_fib->amtrl3_om_ipv6_first_gateway_fib = 0;

    om_fib->amtrl3_om_ipv4_first_gateway_dip.type = L_INET_ADDR_TYPE_IPV4;
    om_fib->amtrl3_om_ipv6_first_gateway_dip.type = L_INET_ADDR_TYPE_IPV6;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return AMTRL3_TYPE_FIB_SUCCESS;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one FIB;
 * INPUT:    fib_id:   FIB ID.
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS ,
 *           AMTRL3_TYPE_FIB_FAIL,
 *           AMTRL3_TYPE_FIB_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_DeleteFIB(UI32_T fib_id)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    AMTRL3_OM_LocalResetVid2Fib(fib_id);

    /* bcz SYS_ADPT_DEFAULT_FIB's om is shared for all fib_id
     */
    if(SYS_ADPT_DEFAULT_FIB != fib_id)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FIB_SUCCESS;
    }

    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FIB_NOT_EXIST;
    }

    /* destroy host route hisam
     */
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc));
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc));


    /* destroy net route hisam
     */
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc));
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc));

    /* destroy resolve net route hisam
     */
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc));
    L_HISAM_Destroy(&(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc));

#if (SYS_CPNT_VXLAN == TRUE)
    L_HISAM_Destroy(&(om_fib->amtrl3_om_vxlan_tunnel_hisam_desc));
    L_HISAM_Destroy(&(om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc));
#endif

    /* destroy unresolve list (resolve list & gateway list are cloned from unresolve list)
     */
    L_DLST_Indexed_Dblist_Destroy(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list));
    L_DLST_Indexed_Dblist_Destroy(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list));

    /* Delete all interface record */
    if (NULL != om_fib->intf_list)
    {
        L_SORT_LST_Delete_All(om_fib->intf_list);
        free(om_fib->intf_list);
    }

    if(SYS_ADPT_DEFAULT_FIB == fib_id)
        memset(om_fib, 0 , sizeof(AMTRL3_OM_FIB_T));
    else
        free(om_fib);
    amtrl3_om_fib_p[fib_id] = NULL;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return AMTRL3_TYPE_FIB_SUCCESS;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearFIB()
 * -------------------------------------------------------------------------
 * PURPOSE:  Clear one FIB;
 * INPUT:    fib_id:   FIB ID.
 * OUTPUT:   none.
 * RETURN:   AMTRL3_TYPE_FIB_SUCCESS ,
 *           AMTRL3_TYPE_FIB_FAIL,
 *           AMTRL3_TYPE_FIB_EXCEPTION.
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_ClearFIB(UI32_T fib_id)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    /* bcz SYS_ADPT_DEFAULT_FIB's om is shared for all fib_id
     */
    if(SYS_ADPT_DEFAULT_FIB != fib_id)
        return AMTRL3_TYPE_FIB_SUCCESS;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FIB_NOT_EXIST;
    }

    /* clear host route hisam
     */
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc));

    /* clear net route hisam
     */
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc));

    /* clear resolve net route hisam
     */
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc));

#if (SYS_CPNT_VXLAN == TRUE)
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_vxlan_tunnel_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc));
#endif

    /* clear unresolve list, resolve list and gateway list (resolve list and gateway list are cloned)
     */
    L_DLST_Indexed_Dblist_DeleteAll_ByListArray(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list));
    L_DLST_Indexed_Dblist_DeleteAll_ByListArray(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list));

    /* Delete all interface record */
    if (NULL != om_fib->intf_list)
    {
        L_SORT_LST_Delete_All(om_fib->intf_list);
    }

    om_fib->amtrl3_om_ipv4_total_neighbor_counts    = 0;
    om_fib->amtrl3_om_ipv4_static_neighbor_counts   = 0;
    om_fib->amtrl3_om_ipv4_dynamic_neighbor_counts  = 0;
    om_fib->amtrl3_om_ipv4_resolved_net_route_count = 0;
    om_fib->amtrl3_om_ipv4_host_entry_ageout_time     = SYS_DFLT_ARP_DEF_TTL * 60;
    om_fib->amtrl3_om_ipv4_software_forwarding_status = AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE;
    om_fib->amtrl3_om_ipv4_super_netting_status       = AMTRL3_TYPE_SUPER_NET_ENABLE;

    om_fib->amtrl3_om_ipv6_host_entry_ageout_time = SYS_DFLT_NEIGHBOR_DEF_TTL * 60;
    om_fib->amtrl3_om_ipv6_super_netting_status = AMTRL3_TYPE_SUPER_NET_ENABLE;
    om_fib->amtrl3_om_ipv6_software_forwarding_status = AMTRL3_TYPE_SOFTWARE_FORWARDING_ENABLE;

    memset(&(om_fib->amtrl3_om_ipv4_first_unresolved_dip), 0, sizeof(L_INET_AddrIp_T));
    memset(&(om_fib->amtrl3_om_ipv6_first_unresolved_dip), 0, sizeof(L_INET_AddrIp_T));

    om_fib->amtrl3_om_ipv4_first_unresolved_dip.type = L_INET_ADDR_TYPE_IPV4;
    om_fib->amtrl3_om_ipv6_first_unresolved_dip.type = L_INET_ADDR_TYPE_IPV6;

    memset(&(om_fib->amtrl3_om_ipv4_first_gateway_dip), 0, sizeof(L_INET_AddrIp_T));
    memset(&(om_fib->amtrl3_om_ipv6_first_gateway_dip), 0, sizeof(L_INET_AddrIp_T));

    om_fib->amtrl3_om_ipv4_first_gateway_dip.type = L_INET_ADDR_TYPE_IPV4;
    om_fib->amtrl3_om_ipv6_first_gateway_dip.type = L_INET_ADDR_TYPE_IPV6;

    om_fib->amtrl3_om_ipv4_first_unresolved_fib = 0;
    om_fib->amtrl3_om_ipv6_first_unresolved_fib = 0;
    om_fib->amtrl3_om_ipv4_first_gateway_fib = 0;
    om_fib->amtrl3_om_ipv6_first_gateway_fib = 0;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return AMTRL3_TYPE_FIB_SUCCESS;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextFIBID
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next FIB id(0 ~ 254).
 * INPUT:    fib_id - AMTRL3_OM_GET_FIRST_FIB_ID to get first
 * OUTPUT:   fib_id
 * RETURN:   TRUE/FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextFIBID(UI32_T *fib_id)
{
    UI32_T  orig_priority, tmp_fib_id;
    BOOL_T  ret = FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if (*fib_id == AMTRL3_OM_GET_FIRST_FIB_ID)
        tmp_fib_id = 0;
    else
        tmp_fib_id = *fib_id+1;

    for (;tmp_fib_id <SYS_ADPT_MAX_NUMBER_OF_FIB; tmp_fib_id++)
    {
        if (amtrl3_om_fib_vid_cnt[tmp_fib_id] == 0)
            continue;

        ret = TRUE;
        *fib_id = tmp_fib_id;
        break;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Get config info or statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetDatabaseValue(UI32_T fib_id, UI32_T type)
{
    AMTRL3_OM_FIB_T *om_fib;
    UI32_T          ret = 0;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    om_fib = AMTRL3_OM_GetFIBByID(fib_id);

    /* fib not exists, return wrong value */
    if(om_fib == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return ret;
    }

    switch(type)
    {
        case AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv4_total_neighbor_counts;
            break;
        case AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv4_static_neighbor_counts;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv4_dynamic_neighbor_counts;
            break;
        case AMTRL3_OM_IPV4_HOST_ENTRY_AGEOUT_TIME:
            ret = om_fib->amtrl3_om_ipv4_host_entry_ageout_time;
            break;
        case AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT:
            ret = om_fib->amtrl3_om_ipv4_resolved_net_route_count;
            break;
        case AMTRL3_OM_IPV4_SOFTWARE_FORWARDING_STATUS:
            ret = om_fib->amtrl3_om_ipv4_software_forwarding_status;
            break;
        case AMTRL3_OM_IPV4_SUPER_NETTING_STATUS:
            ret = om_fib->amtrl3_om_ipv4_super_netting_status;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS:
            ret = om_fib->amtrl3_om_ipv4_dynamic_route_in_chip_counts;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_NUMBER:
            ret = om_fib->amtrl3_om_ipv4_cidr_route_number;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_DISCARDS:
            ret = om_fib->amtrl3_om_ipv4_cidr_route_discards;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE:
            ret = om_fib->amtrl3_om_ipv4_default_route_entry.action_type;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            ret = om_fib->amtrl3_om_ipv4_default_route_entry.multipath_counter;
            break;

        case AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv6_total_neighbor_counts;
            break;
        case AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv6_static_neighbor_counts;
            break;
        case AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            ret = om_fib->amtrl3_om_ipv6_dynamic_neighbor_counts;
            break;
        case AMTRL3_OM_IPV6_HOST_ENTRY_AGEOUT_TIME:
            ret = om_fib->amtrl3_om_ipv6_host_entry_ageout_time;
            break;
        case AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT:
            ret = om_fib->amtrl3_om_ipv6_resolved_net_route_count;
            break;
        case AMTRL3_OM_IPV6_SOFTWARE_FORWARDING_STATUS:
            ret = om_fib->amtrl3_om_ipv6_software_forwarding_status;
            break;
        case AMTRL3_OM_IPV6_SUPER_NETTING_STATUS:
            ret = om_fib->amtrl3_om_ipv6_super_netting_status;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_NUMBER:
            ret = om_fib->amtrl3_om_ipv6_cidr_route_number;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_DISCARDS:
            ret = om_fib->amtrl3_om_ipv6_cidr_route_discards;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE:
            ret = om_fib->amtrl3_om_ipv6_default_route_entry.action_type;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            ret = om_fib->amtrl3_om_ipv6_default_route_entry.multipath_counter;
            break;


        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_add_host_route_fail_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_delete_host_route_fail_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_host_route_in_chip_counter;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_add_host_route_fail_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_delete_host_route_fail_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_host_route_in_chip_counter;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv4_add_net_route_fail_counter;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_chip_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_om_counter;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL:
            ret = om_fib->debug_counter.amtrl3_om_ipv6_add_net_route_fail_counter;
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT:
            ret = om_fib->amtrl3_om_ipv6_6to4_tunnel_host_route_count;
            break;
        case AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT:
            ret = om_fib->amtrl3_om_ipv6_6to4_tunnel_net_route_count;
            break;
#endif

        default: /* wrong value */
            ret = 0;
            break;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Set the value of config info or statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_SetDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T value)
{
    AMTRL3_OM_FIB_T *om_fib;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    om_fib = AMTRL3_OM_GetFIBByID(fib_id);

    /* fib not exists, return */
    if(om_fib == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return ;
    }

    switch(type)
    {
        case AMTRL3_OM_IPV4_HOST_ENTRY_AGEOUT_TIME:
            om_fib->amtrl3_om_ipv4_host_entry_ageout_time = value;
            break;
        case AMTRL3_OM_IPV4_SOFTWARE_FORWARDING_STATUS:
            om_fib->amtrl3_om_ipv4_software_forwarding_status = value;
            break;
        case AMTRL3_OM_IPV4_SUPER_NETTING_STATUS:
            om_fib->amtrl3_om_ipv4_super_netting_status = value;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE:
            om_fib->amtrl3_om_ipv4_default_route_entry.action_type = value;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            om_fib->amtrl3_om_ipv4_default_route_entry.multipath_counter = value;
            break;

        case AMTRL3_OM_IPV6_HOST_ENTRY_AGEOUT_TIME:
            om_fib->amtrl3_om_ipv6_host_entry_ageout_time = value;
            break;
        case AMTRL3_OM_IPV6_SOFTWARE_FORWARDING_STATUS:
            om_fib->amtrl3_om_ipv6_software_forwarding_status = value;
            break;
        case AMTRL3_OM_IPV6_SUPER_NETTING_STATUS:
            om_fib->amtrl3_om_ipv6_super_netting_status = value;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE:
            om_fib->amtrl3_om_ipv6_default_route_entry.action_type = value;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            om_fib->amtrl3_om_ipv6_default_route_entry.multipath_counter = value;
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv6_6to4_tunnel_host_route_count = value;
            break;
        case AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv6_6to4_tunnel_net_route_count = value;
            break;
#endif

        default:
            break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Increase the statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_IncreaseDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T step)
{
    AMTRL3_OM_FIB_T *om_fib;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    om_fib = AMTRL3_OM_GetFIBByID(fib_id);

    /* fib not exists, return */
    if(om_fib == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return;
    }

    switch(type)
    {
        case AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv4_total_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv4_static_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv4_dynamic_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv4_resolved_net_route_count += step;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS:
            om_fib->amtrl3_om_ipv4_dynamic_route_in_chip_counts += step;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_NUMBER:
            om_fib->amtrl3_om_ipv4_cidr_route_number += step;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_DISCARDS:
            om_fib->amtrl3_om_ipv4_cidr_route_discards += step;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            om_fib->amtrl3_om_ipv4_default_route_entry.multipath_counter += step;
            break;

        case AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv6_total_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv6_static_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            om_fib->amtrl3_om_ipv6_dynamic_neighbor_counts += step;
            break;
        case AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv6_resolved_net_route_count += step;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_NUMBER:
            om_fib->amtrl3_om_ipv6_cidr_route_number += step;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_DISCARDS:
            om_fib->amtrl3_om_ipv6_cidr_route_discards += step;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            om_fib->amtrl3_om_ipv6_default_route_entry.multipath_counter += step;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv4_add_host_route_fail_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv4_delete_host_route_fail_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv4_host_route_in_chip_counter += step;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv6_add_host_route_fail_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv6_delete_host_route_fail_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv6_host_route_in_chip_counter += step;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv4_add_net_route_fail_counter += step;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP:
            om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_chip_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM:
            om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_om_counter += step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL:
            om_fib->debug_counter.amtrl3_om_ipv6_add_net_route_fail_counter += step;
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv6_6to4_tunnel_host_route_count += step;
            break;
        case AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT:
            om_fib->amtrl3_om_ipv6_6to4_tunnel_net_route_count += step;
            break;
#endif

        default:
            break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DecreaseDatabaseValue
 * -------------------------------------------------------------------------
 * PURPOSE:  Decrease the statistics in this FIB .
 * INPUT:    none.
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    MUST confirm the fib exists before calling this function.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_DecreaseDatabaseValue(UI32_T fib_id, UI32_T type, UI32_T step)
{
    AMTRL3_OM_FIB_T *om_fib;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    om_fib = AMTRL3_OM_GetFIBByID(fib_id);

    /* fib not exists, return */
    if(om_fib == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return;
    }

    switch(type)
    {
        case AMTRL3_OM_IPV4_TOTAL_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv4_total_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv4_total_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV4_STATIC_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv4_static_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv4_static_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv4_dynamic_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv4_dynamic_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV4_RESOLVED_NET_ROUTE_COUNT:
            if (om_fib->amtrl3_om_ipv4_resolved_net_route_count >= step)
                om_fib->amtrl3_om_ipv4_resolved_net_route_count -= step;
            break;
        case AMTRL3_OM_IPV4_DYNAMIC_ROUTE_IN_CHIP_COUNTS:
            if (om_fib->amtrl3_om_ipv4_dynamic_route_in_chip_counts >= step)
                om_fib->amtrl3_om_ipv4_dynamic_route_in_chip_counts -= step;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_NUMBER:
            if (om_fib->amtrl3_om_ipv4_cidr_route_number >= step)
                om_fib->amtrl3_om_ipv4_cidr_route_number -= step;
            break;
        case AMTRL3_OM_IPV4_CIDR_ROUTE_DISCARDS:
            if (om_fib->amtrl3_om_ipv4_cidr_route_discards >= step)
                om_fib->amtrl3_om_ipv4_cidr_route_discards -= step;
            break;
        case AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            if (om_fib->amtrl3_om_ipv4_default_route_entry.multipath_counter >= step)
                om_fib->amtrl3_om_ipv4_default_route_entry.multipath_counter -= step;
            break;
        case AMTRL3_OM_IPV6_TOTAL_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv6_total_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv6_total_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV6_STATIC_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv6_static_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv6_static_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV6_DYNAMIC_IPNET2PHYSICAL_COUNTS:
            if (om_fib->amtrl3_om_ipv6_dynamic_neighbor_counts >= step)
                om_fib->amtrl3_om_ipv6_dynamic_neighbor_counts -= step;
            break;
        case AMTRL3_OM_IPV6_RESOLVED_NET_ROUTE_COUNT:
            if (om_fib->amtrl3_om_ipv6_resolved_net_route_count >= step)
                om_fib->amtrl3_om_ipv6_resolved_net_route_count -= step;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_NUMBER:
            if (om_fib->amtrl3_om_ipv6_cidr_route_number >= step)
                om_fib->amtrl3_om_ipv6_cidr_route_number -= step;
            break;
        case AMTRL3_OM_IPV6_CIDR_ROUTE_DISCARDS:
            if (om_fib->amtrl3_om_ipv6_cidr_route_discards >= step)
                om_fib->amtrl3_om_ipv6_cidr_route_discards -= step;
            break;
        case AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER:
            if (om_fib->amtrl3_om_ipv6_default_route_entry.multipath_counter >= step)
                om_fib->amtrl3_om_ipv6_default_route_entry.multipath_counter -= step;
            break;

        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_HOST_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv4_add_host_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_add_host_route_fail_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_DEL_HOST_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv4_delete_host_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_delete_host_route_fail_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_HOST_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv4_host_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_host_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_HOST_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv6_add_host_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_add_host_route_fail_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_DEL_HOST_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv6_delete_host_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_delete_host_route_fail_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_HOST_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv6_host_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_host_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_NET_ROUTE_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_net_route_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_ROUTE_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_ecmp_route_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ECMP_NH_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_ecmp_nh_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV4_ADD_NET_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv4_add_net_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv4_add_net_route_fail_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_CHIP:
            if (om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_chip_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_chip_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_NET_ROUTE_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_net_route_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_ROUTE_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_ecmp_route_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ECMP_NH_IN_OM:
            if (om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_om_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_ecmp_nh_in_om_counter -= step;
            break;
        case AMTRL3_OM_DEBUG_COUNTER_IPV6_ADD_NET_ROUTE_FAIL:
            if (om_fib->debug_counter.amtrl3_om_ipv6_add_net_route_fail_counter >= step)
                om_fib->debug_counter.amtrl3_om_ipv6_add_net_route_fail_counter -= step;
            break;
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        case AMTRL3_OM_IPV6_6to4_TUNNEL_HOST_ROUTE_COUNT:
            if (om_fib->amtrl3_om_ipv6_6to4_tunnel_host_route_count >= step)
                om_fib->amtrl3_om_ipv6_6to4_tunnel_host_route_count -= step;
            break;
        case AMTRL3_OM_IPV6_6to4_TUNNEL_NET_ROUTE_COUNT:
            if (om_fib->amtrl3_om_ipv6_6to4_tunnel_net_route_count >= step)
                om_fib->amtrl3_om_ipv6_6to4_tunnel_net_route_count -= step;
            break;
#endif

        default:
            break;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return;
}

/* LOCAL SUBPROGRAM DEFINITION
 */
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetFIBByID
 * -------------------------------------------------------------------------
 * PURPOSE:  To get FIB by its id.
 * INPUT:    fib_id - FIB id (0 ~ 254).
 * OUTPUT:   none.
 * RETURN:   a pointer to the FIB just found
 * NOTES:    Must take semphore before calling this function.
 * -------------------------------------------------------------------------*/
static AMTRL3_OM_FIB_T *AMTRL3_OM_GetFIBByID(UI32_T fib_id)
{
    if (fib_id >= SYS_ADPT_MAX_NUMBER_OF_FIB)
        return NULL;

    return amtrl3_om_fib_p[SYS_ADPT_DEFAULT_FIB];
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_LocalResetVid2Fib
 * -------------------------------------------------------------------------
 * PURPOSE:  Reset vid2fib table by fib id.
 * INPUT:    fib_id - FIB id (0 ~ 254).
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTES:    None.
 * -------------------------------------------------------------------------*/
static void AMTRL3_OM_LocalResetVid2Fib(UI32_T fib_id)
{
    UI32_T  vid;

    if (fib_id < SYS_ADPT_MAX_NUMBER_OF_FIB)
    {
        for (vid =0; vid <SYS_ADPT_MAX_NBR_OF_VLAN; vid++)
        {
            if (amtrl3_om_vid2fib[vid] == fib_id)
            {
                amtrl3_om_vid2fib[vid] = SYS_ADPT_DEFAULT_FIB;
                amtrl3_om_fib_vid_cnt[SYS_ADPT_DEFAULT_FIB] += 1;
            }
        }

        amtrl3_om_fib_vid_cnt[fib_id] = 0;
    }
}

/* FUNCTION NAME : AMTRL3_OM_CompareInterface
 * PURPOSE:
 *      Compare two L3 interfaces by vid.
 *
 * INPUT:
 *      rec1    -- the pointer to the first entry.
 *      rec2    -- the pointer to the second entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      =0
 *      >0
 *      <0
 * NOTES:
 *      None.
 */
static int AMTRL3_OM_CompareInterface(void* rec1, void* rec2)
{
    AMTRL3_OM_Interface_T *p1 = (AMTRL3_OM_Interface_T *) rec1;
    AMTRL3_OM_Interface_T *p2 = (AMTRL3_OM_Interface_T *) rec2;

    if (p1->vid > p2->vid)
        return 1;
    else if (p1->vid < p2->vid)
        return -1;
    else
        return 0;
}

/* FUNCTION NAME : AMTRL3_OM_CreateInterface
 * PURPOSE:
 *          Store the new created ip interface entry.
 *
 * INPUT:
 *      fib_id
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE.
 *
 * NOTES:
 *      None.
 */
BOOL_T AMTRL3_OM_CreateInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (NULL == intf)
        return FALSE;

    orig_priority=SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if (L_SORT_LST_Set(om_fib->intf_list, intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return TRUE;
}


/* FUNCTION NAME : AMTRL3_OM_DeleteInterface
 * PURPOSE:
 *          Delete an ip interface entry.
 *
 * INPUT:
 *      fib_id
 *      intf  -- the interface entry.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE.
 *
 * NOTES:
 *      None.
 */
BOOL_T AMTRL3_OM_DeleteInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (NULL == intf)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if (L_SORT_LST_Delete(om_fib->intf_list, intf) == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return TRUE;
}

/* FUNCTION NAME : AMTRL3_OM_GetNextInterface
 * PURPOSE:
 *          Get Next IP interface entry
 *
 * INPUT:
 *      fib_id
 *      intf->vid
 *
 * OUTPUT:
 *      intf
 *
 * RETURN:
 *      AMTRL3_TYPE_SUCCESS
 *      AMTRL3_TYPE_FAIL
 *
 * NOTES:
 */
UI32_T AMTRL3_OM_GetNextIpInterface(UI32_T fib_id, AMTRL3_OM_Interface_T *intf)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    BOOL_T          rc;
    UI32_T          orig_priority;

    if (NULL == intf)
        return AMTRL3_TYPE_FAIL;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FAIL;
    }

    if (intf->vid == 0)
    {
        rc = L_SORT_LST_Get_1st(om_fib->intf_list, intf);
    }
    else
    {
        rc = L_SORT_LST_Get_Next(om_fib->intf_list, intf);
    }

    if (rc == FALSE)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return AMTRL3_TYPE_FAIL;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return AMTRL3_TYPE_SUCCESS;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set one route entry.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:    net route need be saved into OM database
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 * OUTPUT:  none.
 * RETURN:  TRUE  -- successfully insert to database.
 *          FALSE -- Cannot insert this entry to database
 * NOTES:  1. This function supports both create and set operation of net route
 *            entry.  A new entry will be created if net entry does not exist,
 *            otherwise, replace it.
 *         2. ipv4 and ipv6 cannot set at the same time
 *         3. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *            address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetNetRouteEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    UI32_T          rc = 0;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(net_route_entry == NULL)
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    /* add net route entry to HISAM database
     */
     if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        rc = L_HISAM_SetRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc), (UI8_T *)net_route_entry, TRUE);
     else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        rc = L_HISAM_SetRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc), (UI8_T *)net_route_entry,  TRUE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    if ((rc == L_HISAM_INSERT) || (rc == L_HISAM_REPLACE))
        return TRUE;
    else
        return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one route entry.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:    net route structure saved key values
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 * OUTPUT:   none.
 * RETURN:   TRUE                    -- Successfully remove from database
 *           FALSE                   -- This entry does not exist in chip/database.
 *                                   -- or cannot Delete this entry
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteNetRouteEntry(UI32_T action_flags,
                                     UI32_T fib_id,
                                     AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    UI8_T           key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(net_route_entry == NULL)
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }
    /* Delete record from net route table
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        rc = L_HISAM_DeleteRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc), key);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        rc = L_HISAM_DeleteRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc), key);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified cidr route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:   net route structure saved key value
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT:   net_route_entry -- record of forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNetRouteEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    UI8_T           key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(net_route_entry == NULL)
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6) &&
           (net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        rc = L_HISAM_GetRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc),
                           AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        rc = L_HISAM_GetRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetOneNetRouteEntryByDstIp
 * -------------------------------------------------------------------------
 * PURPOSE: get one cidr route entry which with the same dest IP, for ECMP purpose
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:   net route structure saved key value
 *                            KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *
 * OUTPUT:   net_route_entry -- record of forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetOneNetRouteEntryByDstIp(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    UI8_T                   key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T                  ret = FALSE;
    AMTRL3_OM_HisamCookie_T param;
    AMTRL3_OM_FIB_T         *om_fib = NULL;
    UI32_T                  orig_priority;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    /* set key index */
    memset(&param, 0, sizeof(AMTRL3_OM_HisamCookie_T));
    memset(&key, 0, sizeof(key));

    /* Net route key are stored in param.ip and param.counter
     */
    param.search_key1 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    param.search_key2 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    param.retrieved_record = (void *)net_route_entry;
    param.request_counter  = 1;
    param.retrieved_counter = 0;
    param.fib_id = fib_id;

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Use Search_WithCookie and callback function to fill appropriate net route entry
       to net route entry block
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        L_HISAM_Search_WithCookie(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack,
                              1, &param);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        L_HISAM_Search_WithCookie(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack,
                              1, &param);
    }

    if(param.retrieved_counter)
        ret = TRUE;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of cidr_route entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:    net route need be saved into OM database
 *                            Primary KEY :
 *                            inet_cidr_route_dest          -- destination inet address
 *                            inet_cidr_route_pfxlen        -- prefix length
 *                            inet_cidr_route_policy        -- serve as an additional index which
 *                                                             may delineate between multiple entries to
 *                                                             the same destination.
 *                                                             Default is {0,0}
 *                            inet_cidr_route_next_hop      -- next hop inet address.
 *
 * OUTPUT:  net_route_entry -- record of next forwarding table.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. if all keys are zero, means get first one.
 *          2. action_flags: if v4\v6 is set, only search in v4\v6 entry, but
 *                           it must consistent with address type in net_route_entry.
 *                           if both are set, and the address type is v4, search in
 *                           v4, then v6; if address type is v6, just search in v6.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNetRouteEntry(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          address_type;
    UI32_T          orig_priority;
    UI8_T           key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE, is_first = TRUE;

    if(net_route_entry == NULL)
        return FALSE;

    address_type = net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.type;

    /* set key index */
    if((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->inet_cidr_route_entry.inet_cidr_route_next_hop)) == FALSE) ||
 #if (SYS_CPNT_IP_TUNNEL == TRUE)
      ((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->inet_cidr_route_entry.inet_cidr_route_next_hop)) == TRUE)  &&  IS_TUNNEL_IFINDEX(net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index))||
 #endif /*SYS_CPNT_IP_TUNNEL*/
       ((AMTRL3_OM_IsAddressEqualZero(&(net_route_entry->inet_cidr_route_entry.inet_cidr_route_next_hop)) == TRUE)  &&
       (net_route_entry->inet_cidr_route_entry.inet_cidr_route_proto == VAL_ipCidrRouteProto_local)) )
    {
        is_first = FALSE;
    }

    if (is_first)
    {
        memset(net_route_entry, 0, sizeof(*net_route_entry));
    }

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && !CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type != L_INET_ADDR_TYPE_IPV4) && (address_type != L_INET_ADDR_TYPE_IPV4Z))
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return FALSE;
        }
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc), AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);

        rc = rc && (fib_id == net_route_entry->inet_cidr_route_entry.fib_id);
    }
    else if(!CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type != L_INET_ADDR_TYPE_IPV6) && (address_type != L_INET_ADDR_TYPE_IPV6Z))
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return FALSE;
        }
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc), AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);

        rc = rc && (fib_id == net_route_entry->inet_cidr_route_entry.fib_id);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
        {
            rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc), AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);

            rc = rc && (fib_id == net_route_entry->inet_cidr_route_entry.fib_id);
            if(!rc)
            {
                memset(net_route_entry, 0, sizeof(*net_route_entry));
                net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
                AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);
            }
        }

        if(!rc)
            rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc), AMTRL3_OM_NET_ROUTE_KIDX, key, (UI8_T *)net_route_entry);

        rc = rc && (fib_id == net_route_entry->inet_cidr_route_entry.fib_id);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByNextHop
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by next-hop.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_next_hop_type -- inet address type of nexthop
 *                                  inet_cidr_route_next_hop      -- next hop inet address.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByNextHop(UI32_T action_flags,
                                                UI32_T fib_id,
                                                UI32_T num_of_entry_requested,
                                                UI32_T *num_of_entry_retrieved,
                                                AMTRL3_OM_NetRouteEntry_T *net_route_entry_block)
{
    UI8_T                       key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T                      ret = FALSE;
    AMTRL3_OM_HisamCookie_T     param;
    AMTRL3_OM_NetRouteEntry_T   *net_route_entry;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if ((net_route_entry_block == NULL) || (num_of_entry_retrieved == NULL))
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    /* set key index */
    memset(&param, 0, sizeof(AMTRL3_OM_HisamCookie_T));
    memset(&key, 0, sizeof(key));
    net_route_entry = net_route_entry_block;

    /* Net route key are stored in param.ip and param.counter
     */
    param.search_key1 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_next_hop;
    param.retrieved_record = (void *)net_route_entry_block;
    param.request_counter  = num_of_entry_requested;
    param.retrieved_counter = 0;
    param.fib_id = fib_id;

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP, net_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Use Search_WithCookie and callback function to fill appropriate net route entry
       to net route entry block
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack,
                              num_of_entry_requested, &param);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack,
                              num_of_entry_requested, &param);
    }

    /* if there is at least one entry that matches the designated next hop interfaces,
       return TRUE
     */
    if(param.retrieved_counter)
    {
        *num_of_entry_retrieved = param.retrieved_counter;
        ret = TRUE;
    } /* end of if */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByIfIndex
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by ifindex
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id:             FIB ID.
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_if_index      -- route ifindex.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByIfIndex(UI32_T action_flags,
                                                UI32_T fib_id,
                                                UI32_T num_of_entry_requested,
                                                UI32_T *num_of_entry_retrieved,
                                                AMTRL3_OM_NetRouteEntry_T *net_route_entry_block)
{
    UI8_T                       key[AMTRL3_OM_NET_ROUTE_IFINDEX_KEY_LEN];
    BOOL_T                      ret = FALSE;
    AMTRL3_OM_HisamCookie_T     param;
    AMTRL3_OM_NetRouteEntry_T   *net_route_entry;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if ((net_route_entry_block == NULL) || (num_of_entry_retrieved == NULL))
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    /* set key index */
    memset(&param, 0, sizeof(AMTRL3_OM_HisamCookie_T));
    memset(&key, 0, sizeof(key));
    net_route_entry = net_route_entry_block;

    /* Net route key are stored in param.ip and param.counter
     */
    param.search_key2 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index;
    param.retrieved_record = (void *)net_route_entry_block;
    param.request_counter  = num_of_entry_requested;
    param.retrieved_counter = 0;
    param.fib_id = fib_id;

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX, net_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Use Search_WithCookie and callback function to fill appropriate net route entry
       to net route entry block
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByIfIndex_CallBack,
                              num_of_entry_requested, &param);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByIfIndex_CallBack,
                              num_of_entry_requested, &param);
    }

    /* if there is at least one entry that matches the designated next hop interfaces,
       return TRUE
     */
    if(param.retrieved_counter)
    {
        *num_of_entry_retrieved = param.retrieved_counter;
        ret = TRUE;
    } /* end of if */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByDstIp
 * -------------------------------------------------------------------------
 * PURPOSE:  Get next N entries of net routes by dest ip.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        num_of_entry_requested -- the number of entry to get at once
 *        net_route_entry_block  -- a specific gateway ip to be compared with
 *                                  KEY:
 *                                  inet_cidr_route_next_hop_type -- inet address type of nexthop
 *                                  inet_cidr_route_next_hop      -- next hop inet address.
 *                                  Other fields shall be zero.
 * OUTPUT:   num_of_entry_retrieved - the number of net route entry actually retrieved
 *           net_route_entry_block  - a memory block to store specific number of
 *                                    matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any net route
 * NOTES:    1. Buffer size for net_route_entry_block must be sufficient to store
 *           the number of net entries requested as indicated by the third input
 *           parameter.
 *           2. Only ipv4 or ipv6 flag shall be set.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNNetRouteEntryByDstIp(UI32_T action_flags,
                                              UI32_T fib_id,
                                              UI32_T  num_of_entry_requested,
                                              UI32_T  *num_of_entry_retrieved,
                                              AMTRL3_OM_NetRouteEntry_T  *net_route_entry_block)
{
    UI8_T                       key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T                      ret = FALSE;
    AMTRL3_OM_HisamCookie_T     param;
    AMTRL3_OM_NetRouteEntry_T   *net_route_entry;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if ((net_route_entry_block == NULL) || (num_of_entry_retrieved == NULL))
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    /* set key index */
    memset(&param, 0, sizeof(AMTRL3_OM_HisamCookie_T));
    memset(&key, 0, sizeof(key));
    net_route_entry = net_route_entry_block;

    /* Net route key are stored in param.ip and param.counter
     */
    param.search_key1 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest;
    param.search_key2 = net_route_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;
    param.retrieved_record = (void *)net_route_entry_block;
    param.request_counter  = num_of_entry_requested;
    param.retrieved_counter = 0;
    param.fib_id = fib_id;

    net_route_entry->inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, net_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Use Search_WithCookie and callback function to fill appropriate net route entry
       to net route entry block
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv4_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack,
                              num_of_entry_requested, &param);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv6_net_route_hisam_desc),
                              AMTRL3_OM_NET_ROUTE_KIDX, key,
                              (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack,
                              num_of_entry_requested, &param);
    }

    /* if there is at least one entry that matches the designated next hop interfaces,
       return TRUE
     */
    if(param.retrieved_counter)
    {
        *num_of_entry_retrieved = param.retrieved_counter;
        ret = TRUE;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: This function sets given host route entry to OM and adjust
 *          double link list location accordingly.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id:             FIB ID
 *        host_route_entry  -- host route need be saved in OM database
 *                             Primary KEY:
 *                             dst_vid_ifindex:    interface's ifindex
 *                             dst_inet_addr:      inet address
 * OUTPUT:  none.
 * RETURN:  TRUE \ FALSE
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *              address type in host_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetHostRouteEntry(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI32_T                      rc;
    AMTRL3_OM_HostRouteStatus_T old_status;
    L_HISAM_Desc_T              *host_route_hisam_desc = NULL;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    /* add net route entry to HISAM database
     */
    if(host_route_entry == NULL)
        return  FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6Z))
            return FALSE;

    host_route_entry->key_fields.fib_id = fib_id;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* EPR: ES3628BT-FLF-ZZ-001135 .
     * Somebody modified the HISAM key, but forget correct the
     * key value of "addrlen".
     */
    if ((host_route_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4) ||
        (host_route_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV4Z))
    {
        host_route_entry->key_fields.dst_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    }
    else if ((host_route_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6) ||
        (host_route_entry->key_fields.dst_inet_addr.type == L_INET_ADDR_TYPE_IPV6Z))
    {
        host_route_entry->key_fields.dst_inet_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }

    old_status = host_route_entry->old_status;
    host_route_entry->old_status = host_route_entry->status;

    /* Set operation replaces the previous entry.
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        host_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_host_route_hisam_desc);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        host_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_host_route_hisam_desc);

    rc = L_HISAM_SetRecord(host_route_hisam_desc, (UI8_T *)host_route_entry, TRUE);

    /* Update double link List according to host route status
    */
    if ((rc == L_HISAM_INSERT) || (rc == L_HISAM_REPLACE))
    {
        /* if host route status changed, removed host route index from unresolved_list
           or resolved_not_sync list accordingly
         */
        if ((host_route_entry->status != old_status) && (old_status != HOST_ROUTE_UNKNOWN))
        {
            AMTRL3_OM_DoulbleLinkListOperation(action_flags, om_fib, DELETE_ENTRY,
                                               &host_route_entry->key_fields.dst_inet_addr,
                                               old_status,
                                               host_route_hisam_desc->last_reference_record_index);
        } /* end of if */

        if (host_route_entry->status != old_status)
        {
            AMTRL3_OM_DoulbleLinkListOperation(action_flags, om_fib, ENQUEUE_ENTRY,
                                               &host_route_entry->key_fields.dst_inet_addr,
                                               host_route_entry->status,
                                               host_route_hisam_desc->last_reference_record_index);
        } /* end of if */
    } /* end of if */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    if ((rc == L_HISAM_CONFLICT) || (rc == L_HISAM_NO_BUFFER))
        return FALSE;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete one route entry by destination inet addresss.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id:             FIB ID.
 *        host_route_entry  -- target host route to delete
 *                             Primary KEY:
 *                             dst_vid_ifindex:    interface's ifindex
 *                             dst_inet_addr:      inet address
 * OUTPUT:   none.
 * RETURN:   TRUE           -- Successfully remove from database
 *           FALSE          -- This entry does not exist in chip/database.
 *                          -- or cannot Delete this entry
 * NOTES:   1. action_flags: ipv4 and ipv6 cannot be set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteHostRouteEntry(UI32_T action_flags,
                                      UI32_T fib_id,
                                      AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI8_T           key[AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_INET];
    BOOL_T          rc;
    L_HISAM_Desc_T  *host_route_hisam_desc = NULL;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    memset(&key, 0, sizeof(key));
    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        host_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_host_route_hisam_desc);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        host_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_host_route_hisam_desc);

    rc = L_HISAM_GetRecord(host_route_hisam_desc,
                           AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX,
                           key, (UI8_T *)host_route_entry);

    /* Host entry shall be remove from unresolved_list or resolved_list
       before it is removed from OM.
     */
    if (rc)
    {
        AMTRL3_OM_DoulbleLinkListOperation(action_flags, om_fib, DELETE_ENTRY,
                                            &host_route_entry->key_fields.dst_inet_addr,
                                            host_route_entry->status,
                                            host_route_hisam_desc->last_reference_record_index);
    } /* end of if */

    rc = L_HISAM_DeleteRecord(host_route_hisam_desc, key);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_inet_addr:      inet address
 *
 * OUTPUT:   host_route_entry -- record of host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. ipv4 and ipv6 cannot set at the same time
 *          2. action flag: AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *             address type in net_route_entry structure
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetHostRouteEntry(UI32_T action_flags,
                                   UI32_T fib_id,
                                   AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI8_T           key[AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_INET];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6) &&
            (host_route_entry->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    /* added by steven.gao */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        host_route_entry->key_fields.dst_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        host_route_entry->key_fields.dst_inet_addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    }

    /* set key index */
    memset(&key, 0, sizeof(key));
    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        rc = L_HISAM_GetRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc),
                                AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX,
                                key, (UI8_T *)host_route_entry);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        rc = L_HISAM_GetRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc),
                                AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX,
                                key, (UI8_T *)host_route_entry);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of host entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    ifindex
 *                             dst_inet_addr:      inet address
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is dst_inet_addr_type and dst_inet_addr in host_route_entry
 *          2. if key is (0,0), means get first one.
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry, but
 *                           it must consistent with address type in host_route_entry.
 *                           if both are set, and the address type is v4, search in
 *                           v4, then v6; if address type is v6, just search in v6.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntry(UI32_T action_flags,
                                       UI32_T fib_id,
                                       AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI8_T           key[AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_INET];
    BOOL_T          rc = FALSE;
    UI32_T          address_type;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }
    address_type = host_route_entry->key_fields.dst_inet_addr.type;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && !CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type != L_INET_ADDR_TYPE_IPV4) && (address_type != L_INET_ADDR_TYPE_IPV4Z))
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return FALSE;
        }
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }
    else if(!CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type != L_INET_ADDR_TYPE_IPV6) && (address_type != L_INET_ADDR_TYPE_IPV6Z))
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return FALSE;
        }

        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        if((address_type == L_INET_ADDR_TYPE_IPV4) || (address_type == L_INET_ADDR_TYPE_IPV4Z))
        {
            rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key, (UI8_T *)host_route_entry);
            rc = rc && (fib_id == host_route_entry->key_fields.fib_id);

            if(!rc)
            {
                memset(host_route_entry, 0, sizeof(*host_route_entry));
                host_route_entry->key_fields.fib_id = fib_id;
                AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);
            }
        }

        if(!rc)
            rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntryByVlanMac
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry by vid, inet addr and MAC addr
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    vlan ifindex
 *                             dst_mac:            destination mac addrs
 *                             dst_inet_addr:      inet address
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByVlanMac(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI8_T           key[AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_MAC_INET];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX, host_route_entry);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX, key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!rc))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX, key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return rc;
}

#if (SYS_CPNT_IP_TUNNEL == TRUE)
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByTunnelNexthop(
                                                UI32_T action_flags,
                                                UI32_T fib_id,
                                                L_INET_AddrIp_T *nexthop,
                                                AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  input=*host_route_entry;

    INFOprintf("Get unresolved tunnel for nexthop %lx:%lx:%lx:%lx; addrlen=%d", EXPAND_IPV6(nexthop->addr), nexthop->addrlen);
    while(AMTRL3_OM_GetNextHostRouteEntry(action_flags,fib_id,host_route_entry))
    {
        if(!IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
        {
            INFOprintf("%lx:%lx:%lx:%lx is not tunnel", EXPAND_IPV6(host_route_entry->key_fields.dst_inet_addr.addr));
            continue;
        }
        if(memcmp(host_route_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.addr, nexthop->addr, nexthop->addrlen)==0)
            return TRUE;
        INFOprintf("%lx:%lx:%lx:%lx nexthop %lx:%lx:%lx:%lx is not me", EXPAND_IPV6(host_route_entry->key_fields.dst_inet_addr.addr),EXPAND_IPV6(host_route_entry->key_fields.u.ip_tunnel.nexthop_inet_addr.addr));
    }
    *host_route_entry = input;
    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetHostRouteEntryByTunnelIfIndex
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry by tunnel ifindex and tunnel entry type
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    vlan ifindex
 *                             tunnel_entry_type:  tunnel mode
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetHostRouteEntryByTunnelIfIndex(UI32_T action_flags,
                                                  UI32_T fib_id,
                                                  AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  host_om;

    INFOprintf("inpute: action=%ld, fid=%ld, ifindex=%ld", action_flags,fib_id, host_route_entry->key_fields.dst_vid_ifindex);

    if(!IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
        return FALSE;
    memset(&host_om, 0, sizeof(host_om));
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        host_om.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
    else if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        host_om.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
    while(AMTRL3_OM_GetNextHostRouteEntry(action_flags,fib_id,&host_om))
    {
        INFOprintf("OM get %lx:%lx:%lx:%lx, vid=%ld", EXPAND_IPV6(host_om.key_fields.dst_inet_addr.addr), host_om.key_fields.dst_vid_ifindex);
        if(!IS_TUNNEL_IFINDEX(host_om.key_fields.dst_vid_ifindex))
        {
            continue;
        }

        /* it should check if tunnel entry type matched */
        if((host_route_entry->key_fields.dst_vid_ifindex == host_om.key_fields.dst_vid_ifindex)&&
           (host_route_entry->key_fields.tunnel_entry_type == host_om.key_fields.tunnel_entry_type))
        {
            *host_route_entry = host_om;
            INFOprintf("Get ifindex=%ld",host_route_entry->key_fields.dst_vid_ifindex);
            return TRUE;
        }
    }
    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetHostRouteEntryByTunnelPrefix
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified host route entry by tunnel ifindex and tunnel entry type
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             dst_vid_ifindex:    vlan ifindex
 *                             dst_inet_addr:      address and prefixlen
 *                             tunnel_entry_type:  tunnel mode
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetHostRouteEntryByTunnelPrefix(UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  host_om;
    UI32_T                      prefix_len = 0;

    INFOprintf("inpute: action=%ld, fid=%ld, ifindex=%ld", action_flags,fib_id, host_route_entry->key_fields.dst_vid_ifindex);

    if(!IS_TUNNEL_IFINDEX(host_route_entry->key_fields.dst_vid_ifindex))
        return FALSE;
    memset(&host_om, 0, sizeof(host_om));
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        host_om.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV6;
    else if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        host_om.key_fields.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;

    prefix_len = host_route_entry->key_fields.dst_inet_addr.preflen;

    while(AMTRL3_OM_GetNextHostRouteEntry(action_flags,fib_id,&host_om))
    {
        /* it should check if host entry's prefix match*/

        INFOprintf("OM get %lx:%lx:%lx:%lx, vid=%ld", EXPAND_IPV6(host_om.key_fields.dst_inet_addr.addr), host_om.key_fields.dst_vid_ifindex);
        if((!IS_TUNNEL_IFINDEX(host_om.key_fields.dst_vid_ifindex))||
           (memcmp(host_om.key_fields.dst_inet_addr.addr, host_route_entry->key_fields.dst_inet_addr.addr,(prefix_len/8))!=0))
        {
            continue;
        }

        /* it should check if tunnel entry type matched */
        if((host_route_entry->key_fields.dst_vid_ifindex == host_om.key_fields.dst_vid_ifindex)&&
           (host_route_entry->key_fields.tunnel_entry_type == host_om.key_fields.tunnel_entry_type))
        {
            *host_route_entry = host_om;
            INFOprintf("Get ifindex=%ld",host_route_entry->key_fields.dst_vid_ifindex);
            return TRUE;
        }
    }
    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNetRouteEntryByTunnelPrefix
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified net route entry by tunnel ifindex and tunnel entry type
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry  -- host route structure saved key value
 *                             KEY:
 *                             inet_cidr_route_if_index:  vlan ifindex
 *                             inet_cidr_route_dest:      address and prefixlen
 *                             tunnel_entry_type:  tunnel mode
 * OUTPUT:  host_route_entry -- record of host route entry.
 * RETURN:  TRUE    - Successfully,
 *          FALSE   - If not available or EOF.
 * NOTES:   1. If all keys equal to zero, means get the first record
 *          2. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNetRouteEntryByTunnelPrefix(UI32_T action_flags,
                                                UI32_T fib_id,
                                                AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    AMTRL3_OM_NetRouteEntry_T   net_om;
    UI32_T                      prefix_len = 0;

    INFOprintf("inpute: action=%ld, fid=%ld, ifindex=%ld", action_flags,fib_id, net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index);

    if(!IS_TUNNEL_IFINDEX(net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index))
        return FALSE;
    memset(&net_om, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        net_om.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV6;
    else if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        net_om.inet_cidr_route_entry.inet_cidr_route_dest.type = L_INET_ADDR_TYPE_IPV4;

    prefix_len = net_route_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen;

    while(AMTRL3_OM_GetNextNetRouteEntry(action_flags,fib_id,&net_om))
    {
        /* it should check if host entry's prefix match*/

        INFOprintf("OM get %lx:%lx:%lx:%lx, vid=%ld",
            EXPAND_IPV6(net_om.inet_cidr_route_entry.inet_cidr_route_dest.addr),
            net_om.inet_cidr_route_entry.inet_cidr_route_if_index);
        if((!IS_TUNNEL_IFINDEX(net_om.inet_cidr_route_entry.inet_cidr_route_if_index))||
           (memcmp(net_om.inet_cidr_route_entry.inet_cidr_route_dest.addr,
           net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest.addr,(prefix_len/8))!=0))
        {
            continue;
        }

        /* it should check if tunnel entry type matched */
        if((net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index == net_om.inet_cidr_route_entry.inet_cidr_route_if_index)&&
           (net_route_entry->tunnel_entry_type == net_om.tunnel_entry_type))
        {
            *net_route_entry = net_om;

            INFOprintf("Get ifindex=%ld",net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index);
            return TRUE;
        }
    }
    return FALSE;
}
#endif /*SYS_CPNT_IP_TUNNEL*/

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextHostRouteEntryByLport
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of host route entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        host_route_entry --  host route with key value
 *                             KEY:
 *                             lport: logical port index
 *                             dst_inet_addr:      inet address
 * OUTPUT:   host_route_entry -- record of next host route entry.
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (lport, dst_inet_addr_type, dst_inet_addr)
 *             in AMTRL3_OM_HostRouteEntry_T.
 *          2. if key is (0,0,0), means get first one.
 *          3. action_flags: if v4 and v6 are both set, search in v4 entry,
 *                           if not found, then search in v6 entry.
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextHostRouteEntryByLport(UI32_T action_flags,
                                              UI32_T fib_id,
                                              AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    UI8_T           key[AMTRL3_OM_HOST_ROUTE_KEYLEN_PORT_IFIDX_INET];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX, host_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc),
                                    AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX,
                                     key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    // Terry Liu(03/03/09): comment out the following two lines will avoid endless loop of caller of this function.
       // if(!rc)
       //     memset(&key, 0, sizeof(key));
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!rc))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc),
                                    AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX,
                                    key, (UI8_T *)host_route_entry);

        rc = rc && (fib_id == host_route_entry->key_fields.fib_id);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next N record of host entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        num_of_entry_requested  -- the number of entry to get at once
 *        host_route_entry_block  -- save a specific inet addr to be compared with
 *                                   KEY:
 *                                   dst_inet_addr:      inet address
 * OUTPUT:
 *        num_of_entry_retrieved - the number of host route entry actually retrieved
 *        host_route_entry_block - a memory block to store specific number of
 *                                 matching net route entries.
 * RETURN:   TRUE  -- Found available entries.
 *           FALSE -- can't find any host route
 * NOTES:    1. Buffer size for host_route_entry_block must be sufficient to store
 *              the number of host entries requested as indicated by the third input
 *              parameter.
 *           2. action_flags: if v4 and v6 are both set, get in v4 entry first, if num_of_entry_retrieved
 *                            is less than num_of_entry_requested, then get the rest in v6 entry.
 * ---------------------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextNHostRouteEntry(UI32_T action_flags,
                                        UI32_T fib_id,
                                        UI32_T num_of_entry_requested,
                                        UI32_T *num_of_entry_retrieved,
                                        AMTRL3_OM_HostRouteEntry_T  *host_route_entry_block)
{
    UI8_T                       key[AMTRL3_OM_HOST_ROUTE_KEYLEN_IFIDX_INET];
    BOOL_T                      ret = FALSE;
    AMTRL3_OM_HisamCookie_T     param;
    AMTRL3_OM_HostRouteEntry_T  *host_route_entry;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if((num_of_entry_retrieved == NULL) || (host_route_entry_block == NULL))
        return FALSE;

    /* set key index */
    memset(&param, 0, sizeof(AMTRL3_OM_HisamCookie_T));
    memset(&key, 0, sizeof(key));
    host_route_entry = host_route_entry_block;

    /* Net route key are stored in param.ip and param.counter
     */
    param.search_key1 = host_route_entry->key_fields.dst_inet_addr;  /* in fact, there is no need to set this search key */
    param.retrieved_record = (void *)host_route_entry_block;
    param.request_counter  = num_of_entry_requested;
    param.retrieved_counter = 0;
    param.fib_id = fib_id;

    host_route_entry->key_fields.fib_id = fib_id;
    AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Use Search_WithCookie and callback function to fill appropriate host route entry
       to host route entry block
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) &&
       (host_route_entry_block->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6) &&
       (host_route_entry_block->key_fields.dst_inet_addr.type != L_INET_ADDR_TYPE_IPV6Z))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc),
                                        AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key,
                                        (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNHostRouteEntry_CallBack,
                                        num_of_entry_requested, &param);
        if(param.retrieved_counter < num_of_entry_requested)
        {
            param.request_counter = num_of_entry_requested - param.retrieved_counter;

            host_route_entry = (AMTRL3_OM_HostRouteEntry_T *) param.retrieved_record;
            memset(host_route_entry, 0, sizeof(*host_route_entry));
            host_route_entry->key_fields.fib_id = fib_id;
            AMTRL3_OM_SetHostRouteKey(key, AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, host_route_entry);
        }
    }
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (param.retrieved_counter < num_of_entry_requested))
    {
        L_HISAM_SearchNext_WithCookie(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc),
                                        AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX, key,
                                        (UI32_T (*)(void*, void*))AMTRL3_OM_GetNextNHostRouteEntry_CallBack,
                                        num_of_entry_requested - param.retrieved_counter, &param);
    }

    /* if there is at least one entry that matches the designated next hop interfaces,
       return TRUE
     */
    if(param.retrieved_counter)
    {
        *num_of_entry_retrieved = param.retrieved_counter;
        ret = TRUE;
    } /* end of if */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearHostRouteDatabase
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all ipv4 and ipv6 host route entries.
 * INPUT:
 *           fib_id       -- FIB id
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:   1. If chip not support batch processing, should use one by one
 *              in this function using compile control to handle it.
 *          2. if any reason can't delete all, must work around in driver layer as possible.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_ClearHostRouteDatabase(UI32_T fib_id)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    /* delete all host route record */
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return;
    }

    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc));

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetTotalHostRouteNumber
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the total number of host route entries include ipv4 and ipv6
 * INPUT:
 *        action_flags:   AMTRL3_TYPE_FLAGS_IPV4 -- include ipv4 entries
 *                        AMTRL3_TYPE_FLAGS_IPV6 -- include ipv6 entries
 *        fib_id      :   FIB id
 * OUTPUT:
 * RETURN: total number of host route number which satisfying action_flags
 * NOTES:    none
 * -------------------------------------------------------------------------*/
UI32_T AMTRL3_OM_GetTotalHostRouteNumber(UI32_T action_flags, UI32_T fib_id)
{
    UI32_T          free_buffer = 0;
    UI32_T          used_buffer = 0;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return 0;
    }

    if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        free_buffer = L_HISAM_GetFreeBufNo(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc));
        used_buffer += om_fib->amtrl3_om_ipv4_host_route_hisam_desc.total_record_nbr - free_buffer;
    }

    if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        free_buffer = L_HISAM_GetFreeBufNo(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc));
        used_buffer += om_fib->amtrl3_om_ipv6_host_route_hisam_desc.total_record_nbr - free_buffer;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return(used_buffer);
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetResolvedNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Set net entry in Resolved state to a Resolved HISAM table
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- set ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- set ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry -- record of AMTRL3_OM_ResolvedNetRouteEntry_T
 *          key :
 *             ip_cidr_route_dest       - subnet ip address.
 *             inverse_prefix_length    - 32 minus prefix length for subnet mask
 * OUTPUT:  none.
 * RETURN:  TRUE  -- successfully insert to database.
 *          FALSE -- Cannot insert this entry to database
 * NOTES:   1. This function supports both create and set operation of net route
 *             entry.  A new entry will be created if net entry does not exist,
 *             otherwise, replace it.
 *          2. action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_SetResolvedNetEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry)
{
    UI32_T          rc;
    L_HISAM_Desc_T  *resolved_net_route_hisam_desc = NULL;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc);

    /* add net route entry to HISAM database
     */
    net_route_entry->fib_id = fib_id;
    rc = L_HISAM_SetRecord(resolved_net_route_hisam_desc, (UI8_T *)net_route_entry, TRUE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    if ((rc == L_HISAM_INSERT) || (rc == L_HISAM_REPLACE))
        return TRUE;
    else
        return FALSE;

    return TRUE;
} /* end of AMTRL3_OM_SetResolvedNetEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DeleteResolvedNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete net route entry from Resolved HISAM table
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- delete ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- delete ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:    the resolved net entry to be deleted
 * OUTPUT:   none.
 * RETURN:   TRUE                   -- Successfully remove from database
 *           FALSE                  -- This entry does not exist in chip/database.
 *                                  -- or cannot Delete this entry
 * NOTES:  action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_DeleteResolvedNetEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T  *net_route_entry)
{
    UI8_T           key[AMTRL3_OM_RESOLVED_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE;
    L_HISAM_Desc_T  *resolved_net_route_hisam_desc = NULL;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    net_route_entry->fib_id = fib_id;
    AMTRL3_OM_SetResolvedNetRouteKey(key, AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc);

    /* Delete record from net route table
     */
    rc = L_HISAM_DeleteRecord(resolved_net_route_hisam_desc, key);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return rc;

} /* end of AMTRL3_OM_DeleteResolvedNetEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetResolvedNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: get the specified cidr route entry
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *
 * OUTPUT:   net_route_entry -- record of Resolved Net entry
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (ip_cidr_route_dest, ip_cidr_route_mask) in
 *             AMTRL3_OM_ResolvedNetRouteEntry_T.
 *          2. action_flags: ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetResolvedNetRouteEntry(UI32_T action_flags, UI32_T fib_id, AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry)
{
    UI8_T           key[AMTRL3_OM_RESOLVED_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE;
    L_HISAM_Desc_T  *resolved_net_route_hisam_desc = NULL;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    net_route_entry->fib_id = fib_id;
    AMTRL3_OM_SetResolvedNetRouteKey(key, AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc);
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        resolved_net_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc);

    rc = L_HISAM_GetRecord(resolved_net_route_hisam_desc,
                           AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX,
                           key,
                           (UI8_T *)net_route_entry);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return rc;
} /* end of AMTRL3_OM_GetResolvedNetRouteEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextResolvedNetRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: To get the next record of Resolved net entry after the specified key
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *        net_route_entry:    include search key: inverse prefix length, dst address type,
 *                             dst address, nexthop_type, next hop address
 * OUTPUT:  net_route_entry  -- record of next Resolved Net Entry
 * RETURN:   TRUE    - Successfully,
 *           FALSE   - If not available or EOF.
 * NOTES:   1. Key is (ip_cidr_route_dest, ip_cidr_route_mask) in
 *             AMTRL3_OM_ResolvedNetRouteEntry_T.
 *          2. if key is (0,0), means get first one.
 *          3. action_flags can only be v4 or v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextResolvedNetRouteEntry(UI32_T action_flags,
                                              UI32_T fib_id,
                                              AMTRL3_OM_ResolvedNetRouteEntry_T *net_route_entry)
{
    UI8_T           key[AMTRL3_OM_RESOLVED_NET_ROUTE_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if(net_route_entry == NULL)
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    net_route_entry->fib_id = fib_id;
    AMTRL3_OM_SetResolvedNetRouteKey(key, AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX, net_route_entry);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* see the caller AMTRL3_MGR_CompensateResolvedNetRouteTable */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc),
                               AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX,  key, (UI8_T *)net_route_entry);

        rc = rc && (fib_id == net_route_entry->fib_id);

        if(!rc)
        {
            memset(net_route_entry, 0, sizeof(*net_route_entry));
            net_route_entry->fib_id = fib_id;
            AMTRL3_OM_SetResolvedNetRouteKey(key, AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX, net_route_entry);
        }
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!rc))
    {
        rc = L_HISAM_GetNextRecord(&(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc),
                               AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX,  key, (UI8_T *)net_route_entry);

        rc = rc && (fib_id == net_route_entry->fib_id);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return rc;
} /* end of AMTRL3_OM_GetNextResolvedNetRouteEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_ClearResolvedNetRouteDatabase
 * -------------------------------------------------------------------------
 * PURPOSE:  Delete all Resolved Net route entry from Resolved Net Table, include ipv4 and ipv6.
 * INPUT:
 *           fib_id       -- FIB id
 * OUTPUT:   none.
 * RETURN:   none.
 * NOTES:    Resolved Net Table should be clear if CLI disable superNetting feature.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_ClearResolvedNetRouteDatabase(UI32_T fib_id)
{
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return;
    }

    /* delete all host route record */
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv4_resolved_net_route_hisam_desc));
    L_HISAM_DeleteAllRecord(&(om_fib->amtrl3_om_ipv6_resolved_net_route_hisam_desc));

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
} /* end of AMTRL3_OM_GetNextResolvedNetRouteEntry() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextUnresolvedHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the next available host route entry from the Unresolve_List
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *
 * OUTPUT:  host_route_entry -- host route record
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the unresloved host route list
 *          2. return FALSE when list is empty or one loop has finished
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextUnresolvedHostEntry(UI32_T action_flags,
                                            UI32_T fib_id,
                                            AMTRL3_OM_HostRouteEntry_T *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  *host_route_entry_ptr;
    L_DLST_Index_T              unresolved_index = 0;
    BOOL_T                      entry_found = FALSE;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Check if there are unresolved_host entries waiting in queue for processing.
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        /* search in ipv4 entry */
        while(!entry_found)
        {
            if(!L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), &unresolved_index))
                break;

            /* Get Host record from Hisam using index retrieved from unresolved_list
            */
            host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), unresolved_index);

            /* Need to compare new entry with the first unresolved_entry to avoid infinite loop
            */
            if(AMTRL3_OM_IsAddressEqualZero(&(om_fib->amtrl3_om_ipv4_first_unresolved_dip)) && (host_route_entry_ptr->ref_count || host_route_entry_ptr->entry_type == VAL_ipNetToPhysicalExtType_static))
            {
                om_fib->amtrl3_om_ipv4_first_unresolved_dip = host_route_entry_ptr->key_fields.dst_inet_addr;
                om_fib->amtrl3_om_ipv4_first_unresolved_fib = host_route_entry_ptr->key_fields.fib_id;
            }
            else if(  (AMTRL3_OM_IsAddressEqual(&(host_route_entry_ptr->key_fields.dst_inet_addr),
                                                &(om_fib->amtrl3_om_ipv4_first_unresolved_dip)))
                    &&(host_route_entry_ptr->key_fields.fib_id == om_fib->amtrl3_om_ipv4_first_unresolved_fib)
                   )
            {
                /* Already walk thru unresolved_list completely and first entry is retreived for the second time.
                */
                L_DLST_Indexed_Dblist_Enqueue2Head(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), unresolved_index);
                memset(&(om_fib->amtrl3_om_ipv4_first_unresolved_dip.addr), 0, sizeof(om_fib->amtrl3_om_ipv4_first_unresolved_dip.addr));
                om_fib->amtrl3_om_ipv4_first_unresolved_fib = 0;
                break;
            }

            /* Only gateway entry need to remain in unresolved_list to retransmit ArpReq().
               Do not need to spend system resource to trasmit ArpReq() multiple times for
               host entry.
             */
            if(host_route_entry_ptr->ref_count || host_route_entry_ptr->entry_type == VAL_ipNetToPhysicalExtType_static)
            {
                if(L_DLST_Indexed_Dblist_Enqueue(&(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list), unresolved_index) == FALSE)
                    break;
            } /* end of if */
            else
            {
                host_route_entry_ptr->old_status = HOST_ROUTE_UNKNOWN;
            }

            memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
            entry_found = TRUE;
        } /* end of while */
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!entry_found))
    {
        /* search in ipv6 entry if ipv6 flag is set
           or both flags are set but not found in ipv4 entry
         */
        while(!entry_found)
        {
            if(!L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), &unresolved_index))
                break;

            /* Get Host record from Hisam using index retrieved from unresolved_list
            */
            host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), unresolved_index);

            /* Need to compare new entry with the first unresolved_entry to avoid infinite loop
            */

            if(AMTRL3_OM_IsAddressEqualZero(&(om_fib->amtrl3_om_ipv6_first_unresolved_dip))  &&
               (host_route_entry_ptr->ref_count || host_route_entry_ptr->entry_type == VAL_ipNetToPhysicalExtType_static))
            {
                om_fib->amtrl3_om_ipv6_first_unresolved_dip = host_route_entry_ptr->key_fields.dst_inet_addr;
                om_fib->amtrl3_om_ipv6_first_unresolved_fib = host_route_entry_ptr->key_fields.fib_id;
            }
            else if(  (AMTRL3_OM_IsAddressEqual(&(host_route_entry_ptr->key_fields.dst_inet_addr),
                                                &(om_fib->amtrl3_om_ipv6_first_unresolved_dip)))
                    &&(host_route_entry_ptr->key_fields.fib_id == om_fib->amtrl3_om_ipv6_first_unresolved_fib)
                   )
            {
                /* Already walk thru unresolved_list completely and first entry is retreived for the second time.
                */
                L_DLST_Indexed_Dblist_Enqueue2Head(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), unresolved_index);
                memset(&(om_fib->amtrl3_om_ipv6_first_unresolved_dip.addr), 0, sizeof(om_fib->amtrl3_om_ipv4_first_unresolved_dip.addr));
                om_fib->amtrl3_om_ipv6_first_unresolved_fib = 0;
                break;
            }

            /* Only gateway entry need to remain in unresolved_list to retransmit ArpReq().
               Do not need to spend system resource to trasmit ArpReq() multiple times for
               host entry.
             */
            if (host_route_entry_ptr->ref_count || host_route_entry_ptr->entry_type == VAL_ipNetToPhysicalExtType_static)
            {
                if (L_DLST_Indexed_Dblist_Enqueue(&(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list), unresolved_index) == FALSE)
                    break;
            } /* end of if */
            else
            {
                host_route_entry_ptr->old_status = HOST_ROUTE_UNKNOWN;
            }

            memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
            entry_found = TRUE;
        } /* end of while */
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return entry_found;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextResolvedNotSyncHostEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the host entry in Resolved_not_sync state. Net entry with
 *          nhop pointed to host_entry's ip need to be sync to OM and Chip
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *
 * OUTPUT:  host_route_entry  -- host route record
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the resloved_not_sync host route list
 *          2. return FALSE when list is empty
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextResolvedNotSyncHostEntry(UI32_T action_flags,
                                                 UI32_T fib_id,
                                                 AMTRL3_OM_HostRouteEntry_T  *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  *host_route_entry_ptr;
    L_DLST_Index_T              resolved_index = 0;
    BOOL_T                      entry_found = FALSE;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if (host_route_entry == NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        while (!entry_found)
        {
           /* No resolved_host entry waiting in queue for processing.
            */
            if (L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv4_resolve_host_entry_list), &resolved_index) == FALSE)
                break;
            else
            {
                /* Get Host record from Hisam using index retreived from resolved_list
                 */
                host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), resolved_index);
                memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
                // host_route_entry->old_status = HOST_ROUTE_UNKNOWN;
                host_route_entry->old_status = HOST_ROUTE_READY_NOT_SYNC;
                // printf("%s(%d): host route's old status has been set to HOST_ROUTE_READY_NOT_SYNC!\n", __FUNCTION__, __LINE__);
                entry_found = TRUE;
            }
        }
    }
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!entry_found))
    {
        while (!entry_found)
        {
           /* No resolved_host entry waiting in queue for processing.
            */
            if (L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv6_resolve_host_entry_list), &resolved_index) == FALSE)
                break;
            else
            {
                /* Get Host record from Hisam using index retreived from resolved_list
                 */
                host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), resolved_index);
                memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
//                host_route_entry->old_status = HOST_ROUTE_UNKNOWN;
                host_route_entry->old_status = HOST_ROUTE_READY_NOT_SYNC;
                // printf("%s(%d): host route's old status has been set to HOST_ROUTE_READY_NOT_SYNC!\n", __FUNCTION__, __LINE__);
                entry_found = TRUE;
            }
        }
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return entry_found;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextGatewayEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get the host entry in Gateway entry list.
 * INPUT:
 *        action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                            AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *        fib_id       -- FIB id
 *
 * OUTPUT:  host_route_entry  -- gateway host route record.
 * RETURN:  TRUE       --  Successfully,
 *          FALSE      --  Fail.
 * NOTES:   1. get the first entry from the gateway entry list
 *          2. return FALSE when list is empty or one loop has finished
 *          3. action_flags: if v4\v6 is set, only search in v4\v6 entry,
 *                           if both are set, search in v4, then v6
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetNextGatewayEntry(UI32_T action_flags,
                                     UI32_T fib_id,
                                     AMTRL3_OM_HostRouteEntry_T  *host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T  *host_route_entry_ptr;
    L_DLST_Index_T              gateway_index = 0;
    BOOL_T                      entry_found = FALSE;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if(host_route_entry == NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    /* Check if there are unresolved_host entries waiting in queue for processing.
     */
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        /* search in ipv4 entry */
        while (!entry_found)
        {
            if (!L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv4_gateway_entry_list), &gateway_index))
                break;

            /* Get Host record from Hisam using index retrieved from unresolved_list
            */
            host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv4_host_route_hisam_desc), gateway_index);

            /* Need to compare new entry with the first unresolved_entry to avoid infinite loop
             */
            if(AMTRL3_OM_IsAddressEqualZero(&(om_fib->amtrl3_om_ipv4_first_gateway_dip)))
            {
               om_fib->amtrl3_om_ipv4_first_gateway_dip = host_route_entry_ptr->key_fields.dst_inet_addr;
               om_fib->amtrl3_om_ipv4_first_gateway_fib = host_route_entry_ptr->key_fields.fib_id;
            }
            else if(  (AMTRL3_OM_IsAddressEqual(&(host_route_entry_ptr->key_fields.dst_inet_addr),
                                                &(om_fib->amtrl3_om_ipv4_first_gateway_dip)))
                    &&(host_route_entry_ptr->key_fields.fib_id == om_fib->amtrl3_om_ipv4_first_gateway_fib)
                   )
            {
                /* Already walk thru unresolved_list completely and first entry is retreived for the second time.
                 */
                L_DLST_Indexed_Dblist_Enqueue2Head(&(om_fib->amtrl3_om_ipv4_gateway_entry_list), gateway_index);
                memset(&(om_fib->amtrl3_om_ipv4_first_gateway_dip.addr), 0, sizeof(om_fib->amtrl3_om_ipv4_first_gateway_dip.addr));
                om_fib->amtrl3_om_ipv4_first_gateway_fib = 0;
                break;
            }

            if (L_DLST_Indexed_Dblist_Enqueue(&(om_fib->amtrl3_om_ipv4_gateway_entry_list), gateway_index) == FALSE)
                break;

            memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
            entry_found = TRUE;
        } /* end of while */
    }

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6) && (!entry_found))
    {
        /* search in ipv6 entry if ipv6 flag is set
           or both flags are set but not found in ipv4 entry
         */
        while (!entry_found)
        {
            if (!L_DLST_Indexed_Dblist_Dequeue(&(om_fib->amtrl3_om_ipv6_gateway_entry_list), &gateway_index))
                break;

            /* Get Host record from Hisam using index retrieved from unresolved_list
             */
            host_route_entry_ptr = L_HISAM_GetRecordAddressByIndex(&(om_fib->amtrl3_om_ipv6_host_route_hisam_desc), gateway_index);

            /* Need to compare new entry with the first unresolved_entry to avoid infinite loop
             */
            if(AMTRL3_OM_IsAddressEqualZero(&(om_fib->amtrl3_om_ipv6_first_gateway_dip)))
            {
               om_fib->amtrl3_om_ipv6_first_gateway_dip = host_route_entry_ptr->key_fields.dst_inet_addr;
               om_fib->amtrl3_om_ipv6_first_gateway_fib = host_route_entry_ptr->key_fields.fib_id;
            }
            else if(  (AMTRL3_OM_IsAddressEqual(&(host_route_entry_ptr->key_fields.dst_inet_addr),
                                                &(om_fib->amtrl3_om_ipv6_first_gateway_dip)))
                    &&(host_route_entry_ptr->key_fields.fib_id == om_fib->amtrl3_om_ipv6_first_gateway_fib)
                   )
            {
                /* Already walk thru unresolved_list completely and first entry is retreived for the second time.
                 */
                L_DLST_Indexed_Dblist_Enqueue2Head(&(om_fib->amtrl3_om_ipv6_gateway_entry_list), gateway_index);
                memset(&(om_fib->amtrl3_om_ipv6_first_gateway_dip.addr), 0, sizeof(om_fib->amtrl3_om_ipv6_first_gateway_dip.addr));
                om_fib->amtrl3_om_ipv6_first_gateway_fib = 0;
                break;
            }

            if (L_DLST_Indexed_Dblist_Enqueue(&(om_fib->amtrl3_om_ipv6_gateway_entry_list), gateway_index) == FALSE)
                break;

            memcpy(host_route_entry, host_route_entry_ptr, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);
            entry_found = TRUE;
        } /* end of while */
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return entry_found;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetSuperNetEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  Find any matching net route entry that is a supernet of the
 *           specific subnet entry
 * INPUT:
 *           action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                               AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *           fib_id       -- FIB id
 *           child_subnet - specific subnet to compare with.
 *           compare_length - specific length of the subnet.
 * OUTPUT:   supernet_entry - A net entry that is the parent of the given specific
 *                            subnet entry, which covers larger network than the
 *                            child_subnet route.
 * RETURN:   none.
 * NOTES:    ipv4 and ipv6 cannot set at the same time
 * -------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_GetSuperNetEntry(UI32_T action_flags,
                                  UI32_T fib_id,
                                  L_INET_AddrIp_T child_subnet,
                                  UI32_T compare_length,
                                  AMTRL3_OM_NetRouteEntry_T *supernet_entry)
{
    UI8_T                       key[AMTRL3_OM_NET_ROUTE_KEY_LEN];
    BOOL_T                      record_found = FALSE;
    AMTRL3_OM_NetRouteEntry_T   local_net_entry;
    L_HISAM_Desc_T              *net_route_hisam_desc = NULL;
    L_INET_AddrIp_T             local_subnet;
    AMTRL3_OM_FIB_T             *om_fib = NULL;
    UI32_T                      orig_priority;

    if(supernet_entry == NULL)
        return FALSE;
    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    /* set key index */
    memset(&key, 0, sizeof(key));
    memset(&local_net_entry, 0, sizeof(AMTRL3_OM_NetRouteEntry_T));

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    child_subnet.preflen = compare_length;
    L_INET_ApplyMask(&child_subnet);
    local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest = child_subnet;
    local_net_entry.inet_cidr_route_entry.inet_cidr_route_pfxlen = compare_length;

    local_net_entry.inet_cidr_route_entry.fib_id = fib_id;
    AMTRL3_OM_SetNetRouteKey(key, AMTRL3_OM_NET_ROUTE_KIDX, &local_net_entry);

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        net_route_hisam_desc = &(om_fib->amtrl3_om_ipv4_net_route_hisam_desc);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        net_route_hisam_desc = &(om_fib->amtrl3_om_ipv6_net_route_hisam_desc);
    }

    if (L_HISAM_GetNextRecord(net_route_hisam_desc,
                              AMTRL3_OM_NET_ROUTE_KIDX, key,
                              (UI8_T *)&local_net_entry))
    {
        child_subnet.preflen = compare_length;
        L_INET_ApplyMask(&child_subnet);
        local_subnet = child_subnet;

        if (  (AMTRL3_OM_IsAddressEqual(&(local_net_entry.inet_cidr_route_entry.inet_cidr_route_dest), &local_subnet))
            &&(fib_id == local_net_entry.inet_cidr_route_entry.fib_id)
           )
        {
            memcpy(supernet_entry, &local_net_entry, AMTRL3_OM_NET_ROUTE_ENTRY_LEN);
            record_found = TRUE;
        } /* end of if */
    } /* end of if */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);

    return record_found;
} /* end of AMTRL3_OM_GetSuperNetEntry () */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDoulbeLinkListCounter
 * -------------------------------------------------------------------------
 * PURPOSE: This function prints out the number of element currently exist in
 *          each double link list
 * INPUT:
 *      fib_id       -- FIB id
 *
 * OUTPUT:  none.
 * RETURN:  None.
 * NOTES:   None.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_GetDoulbeLinkListCounter (UI32_T fib_id)
{
    AMTRL3_OM_FIB_T  *om_fib = NULL;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return;
    }
    /* djd: data structure changed */    /*
    printf("IPV4 Unresolved list         \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list.number_of_items_count));
    printf("IPV6 Unresolved list         \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list.number_of_items_count));
    printf("IPV4 Resolved_not_sync list  \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv4_resolve_host_entry_list.number_of_items_count));
    printf("IPV6 Resolved_not_sync list  \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv6_resolve_host_entry_list.number_of_items_count));
    printf("IPV4 Gateway list            \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv4_gateway_entry_list.number_of_items_count));
    printf("IPV6 Gateway list            \t%d host entries\n", (int)(om_fib->amtrl3_om_ipv6_gateway_entry_list.number_of_items_count));
*/
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetHostRouteKey
 * -------------------------------------------------------------------------
 * PURPOSE:  Setting the key to the hisam
 * INPUT:    host_entry: include address type, address, lport, vlan id
 *                       which will be set to key according to key_index
 *           key_index:  to decide which field should be set in key.
 * OUTPUT:   key.
 * RETURN:   none.
 * NOTES:    none.
 * -------------------------------------------------------------------------*/
static void AMTRL3_OM_SetHostRouteKey(UI8_T *key, UI32_T key_index,  AMTRL3_OM_HostRouteEntry_T  *host_entry)
{
    const L_HISAM_KeyDef_T  *keydef_p;
    UI32_T                  idx, ofs =0;
    UI8_T                   *src_p = (UI8_T *) host_entry;

    if (NULL == src_p)
        return;

    switch (key_index)
    {
    case AMTRL3_OM_HOST_ROUTE_IFINDEX_INETADDR_KIDX:
    case AMTRL3_OM_HOST_ROUTE_PORT_IFINDEX_INETADDR_KIDX:
    case AMTRL3_OM_HOST_ROUTE_IFINDEX_MAC_INETADDR_KIDX:
        keydef_p = &amtrl3_om_host_route_key_def_table[key_index];
        break;

    default:
        return;
    } /* end of switch */

    for (idx =0; idx < keydef_p->field_number; idx++)
    {
        memcpy(&key[ofs], &src_p[keydef_p->offset[idx]], keydef_p->length[idx]);
        ofs += keydef_p->length[idx];
    }
} /* end of AMTRL3_OM_SetHostRouteKey() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetNetRouteKey
 * -------------------------------------------------------------------------
 * PURPOSE:  Setting the key to the hisam
 * INPUT:    net_route_entry: include inet_cidr_route_dest_type, inet_cidr_route_dest,
 *                            inet_cidr_route_pfxlen, inet_cidr_route_policy,
 *                            inet_cidr_route_next_hop_type, inet_cidr_route_next_hop,
 *                            which will be set to key according to key_index
 *           key_index:  to decide which field should be set in key.
 * OUTPUT:   key.
 * RETURN:   none.
 * NOTES:    none.
 * -------------------------------------------------------------------------*/
static void AMTRL3_OM_SetNetRouteKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_NetRouteEntry_T *net_route_entry)
{
    const L_HISAM_KeyDef_T  *keydef_p;
    UI32_T                  idx, ofs =0;
    UI8_T                   *src_p = (UI8_T *) net_route_entry;

    if (src_p == NULL)
        return;

    switch (key_index)
    {
    case AMTRL3_OM_NET_ROUTE_KIDX:
    case AMTRL3_OM_NET_ROUTE_KIDX_NEXT_HOP:
    case AMTRL3_OM_NET_ROUTE_KIDX_IFINDEX:
        keydef_p = &amtrl3_om_net_route_key_def_table[key_index];
        break;
    default:
        return;
    } /* end of switch */

    for (idx =0; idx < keydef_p->field_number; idx++)
    {
        memcpy(&key[ofs], &src_p[keydef_p->offset[idx]], keydef_p->length[idx]);
        ofs += keydef_p->length[idx];
    }
} /* end of AMTRL3_OM_SetNetRouteKey() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_SetResolvedNetRouteKey
 * -------------------------------------------------------------------------
 * PURPOSE:  Setting the key to the hisam
 * INPUT:    resolved_net_entry: inverse_pfxlen, dst_addr_type, dst_addr, next_hop_type, next_hop,
 *                              which will be set to key according to key_index
 *           key_index:  to decide which field should be set in key.
 * OUTPUT:   key.
 * RETURN:   none.
 * NOTES:    none.
 * -------------------------------------------------------------------------*/
static void AMTRL3_OM_SetResolvedNetRouteKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_ResolvedNetRouteEntry_T  *resolved_net_entry)
{
    const L_HISAM_KeyDef_T  *keydef_p;
    UI32_T                  idx, ofs =0;
    UI8_T                   *src_p = (UI8_T *) resolved_net_entry;

    if (src_p == NULL)
        return;

    switch (key_index)
    {
    case AMTRL3_OM_RESOLVED_NET_ROUTE_KIDX:
        keydef_p = &amtrl3_om_resolved_net_route_key_def_table[key_index];
        break;
    default:
        return;
    } /* end of switch */

    for (idx =0; idx < keydef_p->field_number; idx++)
    {
        memcpy(&key[ofs], &src_p[keydef_p->offset[idx]], keydef_p->length[idx]);
        ofs += keydef_p->length[idx];
    }
} /* end of AMTRL3_OM_SetResolvedNetRouteKey() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack
 * -------------------------------------------------------------------------
 * PURPOSE: HISAM CallBack function for get net route by next hop
 * INPUT:   cookie
 *
 * OUTPUT:  none.
 * RETURN:  L_HISAM_SEARCH_BREAK         --  tell this function to break searching
 *          L_HISAM_SEARCH_CONTINUE      --  tell this function continue searching
 * NOTES:
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie)
{
    AMTRL3_OM_HisamCookie_T *param = (AMTRL3_OM_HisamCookie_T *)cookie;

    if (  !AMTRL3_OM_IsAddressEqual(&(net_route_entry->inet_cidr_route_entry.inet_cidr_route_next_hop), &(param->search_key1))
        ||(param->fib_id != net_route_entry->inet_cidr_route_entry.fib_id)
       )
        return L_HISAM_SEARCH_BREAK;

    memcpy(param->retrieved_record, net_route_entry, AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    param->retrieved_record = (void *)((UI8_T*)param->retrieved_record + AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    /* Max number is reached for this getnext N function
     */
    if (param->retrieved_counter++ == param->request_counter)
        return L_HISAM_SEARCH_BREAK;

    return L_HISAM_SEARCH_CONTINUE;
} /* end of AMTRL3_OM_GetNextNNetRouteEntryByNextHop_CallBack() */

static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByIfIndex_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie)
{
    AMTRL3_OM_HisamCookie_T *param = (AMTRL3_OM_HisamCookie_T *)cookie;

    if (  (net_route_entry->inet_cidr_route_entry.inet_cidr_route_if_index!=param->search_key2)
        ||(param->fib_id != net_route_entry->inet_cidr_route_entry.fib_id)
       )
        return L_HISAM_SEARCH_BREAK;

    memcpy(param->retrieved_record, net_route_entry, AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    param->retrieved_record = (void *)((UI8_T*)param->retrieved_record + AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    /* Max number is reached for this getnext N function
     */
    if (param->retrieved_counter++ == param->request_counter)
        return L_HISAM_SEARCH_BREAK;

    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNHostRouteEntry_CallBack
 * -------------------------------------------------------------------------
 * PURPOSE: HISAM CallBack function for get host route
 * INPUT:   cookie
 *
 * OUTPUT:  none.
 * RETURN:  L_HISAM_SEARCH_BREAK         --  tell this function to break searching
 *          L_HISAM_SEARCH_CONTINUE      --  tell this function continue searching
 * NOTES:
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_OM_GetNextNHostRouteEntry_CallBack(AMTRL3_OM_HostRouteEntry_T *host_route_entry, void* cookie)
{
    AMTRL3_OM_HisamCookie_T *param = (AMTRL3_OM_HisamCookie_T *)cookie;

    if (param->fib_id != host_route_entry->key_fields.fib_id)
        return L_HISAM_SEARCH_BREAK;

    memcpy(param->retrieved_record, host_route_entry, AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);

    param->retrieved_record = (void *)((UI8_T*)param->retrieved_record + AMTRL3_OM_HOST_ROUTE_ENTRY_LEN);

    /* Max number is reached for this getnext N function
     */
    if (param->retrieved_counter++ == param->request_counter)
        return L_HISAM_SEARCH_BREAK;

    return L_HISAM_SEARCH_CONTINUE;
} /* end of AMTRL3_OM_GetNextNHostRouteEntry_CallBack() */

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack
 * -------------------------------------------------------------------------
 * PURPOSE: HISAM CallBack function for get net route by dest ip
 * INPUT:   cookie
 *
 * OUTPUT:  none.
 * RETURN:  L_HISAM_SEARCH_BREAK         --  tell this function to break searching
 *          L_HISAM_SEARCH_CONTINUE      --  tell this function continue searching
 * NOTES:
 * -------------------------------------------------------------------------*/
static UI32_T AMTRL3_OM_GetNextNNetRouteEntryByDstIp_CallBack(AMTRL3_OM_NetRouteEntry_T *net_route_entry, void* cookie)
{
    AMTRL3_OM_HisamCookie_T *param = (AMTRL3_OM_HisamCookie_T *)cookie;

    if (  !AMTRL3_OM_IsAddressEqual(&(net_route_entry->inet_cidr_route_entry.inet_cidr_route_dest), &(param->search_key1))
        ||(net_route_entry->inet_cidr_route_entry.inet_cidr_route_pfxlen != param->search_key2)
        ||(param->fib_id != net_route_entry->inet_cidr_route_entry.fib_id)
       )
        return L_HISAM_SEARCH_BREAK;

    memcpy(param->retrieved_record, net_route_entry, AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    param->retrieved_record = (void *)((UI8_T*)param->retrieved_record + AMTRL3_OM_NET_ROUTE_ENTRY_LEN);

    /* Max number is reached for this getnext N function
     */
    if (param->retrieved_counter++ == param->request_counter)
        return L_HISAM_SEARCH_BREAK;

    return L_HISAM_SEARCH_CONTINUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_DoulbleLinkListOperation
 * -------------------------------------------------------------------------
 * PURPOSE: This function handles various double link list operation.
 * INPUT:
 *          action_flags:       AMTRL3_TYPE_FLAGS_IPV4 -- get ipv4 entry
 *                              AMTRL3_TYPE_FLAGS_IPV6 -- get ipv6 entry
 *          om_fib:              pointer to om database of FIB .
 *          list_operation    - list operation
 *          host_route_status - host route status
 *          index             - optional, depends on list_operation
 * OUTPUT:  index           - optional, depends on list_operation
 * RETURN:  TRUE \ FALSE
 * NOTES:
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRL3_OM_DoulbleLinkListOperation(UI32_T action_flags,
                                                 AMTRL3_OM_FIB_T  *om_fib,
                                                 UI32_T list_operation,
                                                 L_INET_AddrIp_T *host_route_ip,
                                                 UI32_T host_route_status,
                                                 UI32_T list_index)
{
    L_DLST_Indexed_Dblist_T *unresolve_host_entry_list;
    L_DLST_Indexed_Dblist_T *resolve_host_entry_list;
    L_DLST_Indexed_Dblist_T *gateway_entry_list;
    L_INET_AddrIp_T         *first_unresolved_dip;
    L_INET_AddrIp_T         *first_gateway_dip;
    UI32_T                  *first_unresolved_fib;
    UI32_T                  *first_gateway_fib;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4))
    {
        unresolve_host_entry_list = &(om_fib->amtrl3_om_ipv4_unresolve_host_entry_list);
        resolve_host_entry_list   = &(om_fib->amtrl3_om_ipv4_resolve_host_entry_list);
        gateway_entry_list        = &(om_fib->amtrl3_om_ipv4_gateway_entry_list);
        first_unresolved_dip      = &(om_fib->amtrl3_om_ipv4_first_unresolved_dip);
        first_gateway_dip         = &(om_fib->amtrl3_om_ipv4_first_gateway_dip);
        first_unresolved_fib      = &(om_fib->amtrl3_om_ipv4_first_unresolved_fib);
        first_gateway_fib         = &(om_fib->amtrl3_om_ipv4_first_gateway_fib);
    }
    else if(CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
    {
        unresolve_host_entry_list = &(om_fib->amtrl3_om_ipv6_unresolve_host_entry_list);
        resolve_host_entry_list   = &(om_fib->amtrl3_om_ipv6_resolve_host_entry_list);
        gateway_entry_list        = &(om_fib->amtrl3_om_ipv6_gateway_entry_list);
        first_unresolved_dip      = &(om_fib->amtrl3_om_ipv6_first_unresolved_dip);
        first_gateway_dip         = &(om_fib->amtrl3_om_ipv6_first_gateway_dip);
        first_unresolved_fib      = &(om_fib->amtrl3_om_ipv6_first_unresolved_fib);
        first_gateway_fib         = &(om_fib->amtrl3_om_ipv6_first_gateway_fib);
    }

    switch (list_operation)
    {
        case DELETE_ENTRY:
            if (host_route_status == HOST_ROUTE_UNRESOLVED)
            {
                L_DLST_Indexed_Dblist_DeleteEntry(unresolve_host_entry_list, list_index);
                /* Need to clear this data to prevent infinite loop in GetNextUnresolvedHostEntry
                 */
                if(AMTRL3_OM_IsAddressEqual(first_unresolved_dip, host_route_ip))
                {
                    memset(&(first_unresolved_dip->addr), 0, sizeof(first_unresolved_dip->addr));
                    *first_unresolved_fib = 0;
                }
            } /* end of if */
            if (host_route_status == HOST_ROUTE_READY_NOT_SYNC)
                L_DLST_Indexed_Dblist_DeleteEntry(resolve_host_entry_list, list_index);

            if (host_route_status == HOST_ROUTE_GATEWAY_READY)
            {
                L_DLST_Indexed_Dblist_DeleteEntry(gateway_entry_list, list_index);
                /* Need to clear this data to prevent infinite loop in GetNextGatewayEntry
                 */
                if(AMTRL3_OM_IsAddressEqual(first_gateway_dip, host_route_ip))
                {
                    memset(&(first_gateway_dip->addr), 0, sizeof(first_gateway_dip->addr));
                    *first_gateway_fib = 0;
                }
            } /* end of if */

            break;
        case ENQUEUE_ENTRY:
            if (host_route_status == HOST_ROUTE_UNRESOLVED)
                L_DLST_Indexed_Dblist_Enqueue(unresolve_host_entry_list, list_index);
            if (host_route_status == HOST_ROUTE_READY_NOT_SYNC)
                L_DLST_Indexed_Dblist_Enqueue(resolve_host_entry_list, list_index);
            if (host_route_status == HOST_ROUTE_GATEWAY_READY)
                L_DLST_Indexed_Dblist_Enqueue(gateway_entry_list, list_index);
            break;
        default:
            break;
    } /* end of switch */
    return TRUE;
} /* end of AMTRL3_OM_DoulbleLinkListOperation() */

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_IsAddressEqual
 * -------------------------------------------------------------------------
 * PURPOSE: To check if the two address are equal
 * INPUT:   addr1 - the first address
 *          addr2 - the second address
 * OUTPUT:  None.
 * RETURN:  TRUE - equal
 *          FALSE - not eaual.
 * NOTES:   None.
 *--------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_IsAddressEqual(L_INET_AddrIp_T *addr1, L_INET_AddrIp_T *addr2)
{
    UI32_T  i;

    if(addr1->type != addr2->type)
        return FALSE;

    switch(addr1->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
            for(i = 0; i < 4; i++)
                if(addr1->addr[i] != addr2->addr[i])
                    return FALSE;
            return TRUE;

        case L_INET_ADDR_TYPE_IPV4Z:
            if(addr1->zoneid != addr2->zoneid)
                return FALSE;
            for(i = 0; i < 4; i++)
                if(addr1->addr[i] != addr2->addr[i])
                    return FALSE;
            return TRUE;

        case L_INET_ADDR_TYPE_IPV6:
            for(i = 0; i < 16; i++)
                if(addr1->addr[i] != addr2->addr[i])
                    return FALSE;
            return TRUE;

        case L_INET_ADDR_TYPE_IPV6Z:
            if(addr1->zoneid != addr2->zoneid)
                return FALSE;
            for(i = 0; i < 16; i++)
                if(addr1->addr[i] != addr2->addr[i])
                    return FALSE;
            return TRUE;

        default:
            return FALSE;
    }
    return FALSE;
}

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_IsAddressEqualZero
 * -------------------------------------------------------------------------
 * PURPOSE: To check if IP address is equal to zero.
 * INPUT:   addr - IP address
 * OUTPUT:  None.
 * RETURN:  TRUE - equal
 *          FALSE - not eaual.
 * NOTES:   we donot check it's address type. (Terry_Liu(11/7/2008):Now we do check address type)
 *--------------------------------------------------------------------------*/
BOOL_T AMTRL3_OM_IsAddressEqualZero(L_INET_AddrIp_T *addr)
{
    UI32_T  i;

    switch(addr->type)
    {
    case L_INET_ADDR_TYPE_IPV4:
    case L_INET_ADDR_TYPE_IPV4Z:
        for(i = 0; i < 4; i++)
            if(addr->addr[i] != 0)
                return FALSE;
        return TRUE;

    case L_INET_ADDR_TYPE_UNKNOWN:
    case L_INET_ADDR_TYPE_IPV6:
    case L_INET_ADDR_TYPE_IPV6Z:
        for(i = 0; i < 16; i++)
            if(addr->addr[i] != 0)
                return FALSE;
        return TRUE;

    default:
        return FALSE;
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get record of IpNetToPhysical entry matching specified key
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY:
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 *
 * OUTPUT:   ip_net_to_media_entry -- record of IpNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- can't get the specified record.
 *
 * NOTES:
 *      1. action flag AMTRL3_TYPE_FLAGS_IPV4/IPV6 must consistent with
 *         address type in ip_net_to_physical_entry structure.
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_OM_GetIpNetToPhysicalEntry(UI32_T action_flags,
                                          UI32_T fib_id,
                                          AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry)
{
    AMTRL3_OM_HostRouteEntry_T  host_entry;
    BOOL_T                      ret = FALSE;
    UI32_T                      address_type;

    if (ip_net_to_physical_entry == NULL)
        return FALSE;

    address_type = ip_net_to_physical_entry->ip_net_to_physical_net_address.type;
    if((action_flags == AMTRL3_TYPE_FLAGS_IPV4) &&
       (address_type != L_INET_ADDR_TYPE_IPV4) &&
       (address_type != L_INET_ADDR_TYPE_IPV4Z))
        return FALSE;
    if((action_flags == AMTRL3_TYPE_FLAGS_IPV6) &&
       (address_type != L_INET_ADDR_TYPE_IPV6) &&
       (address_type != L_INET_ADDR_TYPE_IPV6Z))
        return FALSE;

    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_entry.key_fields.dst_vid_ifindex = ip_net_to_physical_entry->ip_net_to_physical_if_index;
    host_entry.key_fields.dst_inet_addr = ip_net_to_physical_entry->ip_net_to_physical_net_address;

    if ((ret = AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_entry)) == TRUE)
    {
        ip_net_to_physical_entry->ip_net_to_physical_if_index = host_entry.key_fields.dst_vid_ifindex;
        ip_net_to_physical_entry->ip_net_to_physical_type = host_entry.entry_type;
        ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_type = 1;
        ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_len = SYS_ADPT_MAC_ADDR_LEN;
        memcpy(ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_octet_string,
               host_entry.key_fields.dst_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
    }
    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextIpNetToPhysicalEntry
 * -------------------------------------------------------------------------
 * PURPOSE:  To get the next record of ipNetToPhysical entry after the specified key
 *
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      ip_net_to_physical_entry -- get the entry match the key fields in this structure
 *                                  KEY:
 *                                  ip_net_to_physical_if_index
 *                                  ip_net_to_physical_net_address_type
 *                                  ip_net_to_physical_net_address
 *
 * OUTPUT:   ip_net_to_media_entry -- record of ipNetToPhysical table.
 *
 * RETURN:   TRUE - Successfully
 *           FALSE- No more record (EOF) or can't get other record.
 *
 * NOTES:
 *      1. If only AMTRL3_TYPE_FLAGS_IPV4 is set (address type must be IPV4),
 *         only traverse all ipv4 entries.
 *      2. If only AMTRL3_TYPE_FLAGS_IPV6 is set (address type must be IPV6)
 *         only traverse all ipv6 entries.
 *      3. If both IPV4 and IPV6 flag are set (address type may be IPV4 or IPV6),
 *         if address type is IPV4, traverse IPV4 entries at first then continue on IPV6 entries.
 *         if address type is IPV6, only traverse IPV6 entries.
 *      4. if all keys are zero, means get first one.
 *      5. filters dynamic host entry which is not ready
 * -------------------------------------------------------------------------*/
BOOL_T  AMTRL3_OM_GetNextIpNetToPhysicalEntry(UI32_T action_flags,
                                              UI32_T fib_id,
                                              AMTRL3_TYPE_ipNetToPhysicalEntry_T  *ip_net_to_physical_entry)
{
    AMTRL3_OM_HostRouteEntry_T  host_entry;
    BOOL_T                      ret = FALSE;

    if(ip_net_to_physical_entry == NULL)
        return FALSE;

    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_entry.key_fields.dst_vid_ifindex = ip_net_to_physical_entry->ip_net_to_physical_if_index;
    host_entry.key_fields.dst_inet_addr = ip_net_to_physical_entry->ip_net_to_physical_net_address;

    while((ret = AMTRL3_OM_GetNextHostRouteEntry(action_flags, fib_id, &host_entry)) == TRUE)
    {
        if(!memcmp(host_entry.key_fields.dst_mac, amtrl3_om_zero_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN))
            continue;

        if (host_entry.entry_type == VAL_ipNetToPhysicalExtType_dynamic &&
            host_entry.status != HOST_ROUTE_READY_NOT_SYNC &&
            host_entry.status != HOST_ROUTE_HOST_READY &&
            host_entry.status != HOST_ROUTE_GATEWAY_READY)
            continue;

#if (SYS_CPNT_IP_TUNNEL == TRUE)
        if (host_entry.entry_type == VAL_ipNetToPhysicalExtType_other &&
            (host_entry.key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_ISATAP ||
             host_entry.key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_6TO4 ||
             host_entry.key_fields.tunnel_entry_type == AMTRL3_TYPE_INETTOPHYSICALEXTTYPE_TUNNEL_MANUAL))
        {
            continue;
        }
#endif

        ip_net_to_physical_entry->ip_net_to_physical_if_index = host_entry.key_fields.dst_vid_ifindex;
        ip_net_to_physical_entry->ip_net_to_physical_net_address = host_entry.key_fields.dst_inet_addr;
        ip_net_to_physical_entry->ip_net_to_physical_type = host_entry.entry_type;
        ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_type = 1;
        ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_len = SYS_ADPT_MAC_ADDR_LEN;
        memcpy(ip_net_to_physical_entry->ip_net_to_physical_phys_address.phy_address_octet_string,
               host_entry.key_fields.dst_mac, sizeof(UI8_T)*SYS_ADPT_MAC_ADDR_LEN);
        return ret;
    }
    return ret;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get ipv4 or ipv6 Host Route Entry
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex --  ifindex (KEY)
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. Can not set these 2 flags at same time.
 *         flag must be consistent with inet_host_route_entry->inet_address_type.
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_OM_GetInetHostRouteEntry(UI32_T action_flags,
                                       UI32_T fib_id,
                                       AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T          host_entry;

    if (inet_host_route_entry == NULL)
        return FALSE;
    if (CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV4) && CHECK_FLAG(action_flags, AMTRL3_TYPE_FLAGS_IPV6))
        return FALSE;

    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_entry.key_fields.dst_vid_ifindex = inet_host_route_entry->dst_vid_ifindex;
    host_entry.key_fields.dst_inet_addr = inet_host_route_entry->dst_inet_addr;
    host_entry.key_fields.dst_inet_addr.type = AMTRL3_TYPE_FLAGS_IPV4;

    if (AMTRL3_OM_GetHostRouteEntry(action_flags, fib_id, &host_entry))
    {
        if(!host_entry.in_chip_status)
            return FALSE;

        inet_host_route_entry->lport = host_entry.key_fields.lport;
        inet_host_route_entry->dst_vid_ifindex = host_entry.key_fields.dst_vid_ifindex;
        memcpy(inet_host_route_entry->dst_mac, host_entry.key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
#if (SYS_CPNT_PBR == TRUE)
        inet_host_route_entry->hw_info = host_entry.hw_info;
#endif
        return TRUE;
    } /* end of if */
    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetNextInetHostRouteEntry
 * -------------------------------------------------------------------------
 * PURPOSE: Get next ipv4 or ipv6 Host Route Entry
 * INPUT:
 *      action_flags    -- AMTRL3_TYPE_FLAGS_IPV4 \
 *                         AMTRL3_TYPE_FLAGS_IPV6
 *      fib_id       -- FIB id
 *      inet_host_route_entry->dst_vid_ifindex -- ifindex
 *      inet_host_route_entry->inet_address --  Destination IPv4/v6 Address (KEY)
 * OUTPUT:  inet_host_route_entry
 * RETURN:  TRUE / FALSE
 * NOTES:
 *      1. If AMTRL3_TYPE_FLAGS_IPV4 is set, only search all ipv4 entries.
 *      2. If AMTRL3_TYPE_FLAGS_IPV6 is set, only search all ipv6 entries.
 *      3. If the inet_host_route_entry->inet_address = 0, get the first entry
 * -------------------------------------------------------------------------
 */
BOOL_T AMTRL3_OM_GetNextInetHostRouteEntry(UI32_T action_flags,
                                           UI32_T fib_id,
                                           AMTRL3_TYPE_InetHostRouteEntry_T *inet_host_route_entry)
{
    AMTRL3_OM_HostRouteEntry_T          host_entry;

    if (inet_host_route_entry == NULL)
        return FALSE;

    memset(&host_entry, 0, sizeof(AMTRL3_OM_HostRouteEntry_T));
    host_entry.key_fields.dst_vid_ifindex = inet_host_route_entry->dst_vid_ifindex;
    host_entry.key_fields.dst_inet_addr = inet_host_route_entry->dst_inet_addr;

    while(AMTRL3_OM_GetNextHostRouteEntry(action_flags, fib_id, &host_entry))
    {
        if (!host_entry.in_chip_status)
            continue;

        inet_host_route_entry->dst_inet_addr = host_entry.key_fields.dst_inet_addr;
        inet_host_route_entry->lport = host_entry.key_fields.lport;
        inet_host_route_entry->dst_vid_ifindex = host_entry.key_fields.dst_vid_ifindex;
        memcpy(inet_host_route_entry->dst_mac, host_entry.key_fields.dst_mac, sizeof(UI8_T) * SYS_ADPT_MAC_ADDR_LEN);
        return TRUE;
    } /* end of while */

    return FALSE;
}

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_GetDefaultRouteInfo
 * -------------------------------------------------------------------------
 * PURPOSE:  To get default route action type information.
 * INPUT:    action_flags   --  AMTRL3_TYPE_FLAGS_IPV4 \
 *                              AMTRL3_TYPE_FLAGS_IPV6
 *           fib_id     --   FIB id
 *
 * OUTPUT:   action_type    --  ROUTE / TRAP2CPU / DROP
 *           multipath_num  --  number of path for ECMP
 * RETURN:
 * NOTES:    Currently used in backdoor, to provide information.
 * -------------------------------------------------------------------------*/
void AMTRL3_OM_GetDefaultRouteInfo(UI32_T action_flags,
                                   UI32_T fib_id,
                                   UI32_T *action_type,
                                   UI32_T *multipath_num)
{
    if(action_flags == AMTRL3_TYPE_FLAGS_IPV4)
    {
        *action_type = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_ACTION_TYPE);
        *multipath_num = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV4_DEFAULT_ROUTE_MULTIPATH_COUNTER);
    }
    else if(action_flags == AMTRL3_TYPE_FLAGS_IPV6)
    {
        *action_type = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_ACTION_TYPE);
        *multipath_num = AMTRL3_OM_GetDatabaseValue(fib_id, AMTRL3_OM_IPV6_DEFAULT_ROUTE_MULTIPATH_COUNTER);
    }
}

#if (SYS_CPNT_VXLAN == TRUE)
BOOL_T AMTRL3_OM_SetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    UI32_T  rc = 0;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T orig_priority;

    if (tunnel_entry_p == NULL)
    {
        return FALSE;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if ((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_SetRecord(&om_fib->amtrl3_om_vxlan_tunnel_hisam_desc, (UI8_T *)tunnel_entry_p, TRUE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    if ((rc == L_HISAM_INSERT) || (rc == L_HISAM_REPLACE))
        return TRUE;
    else
        return FALSE;
}

BOOL_T AMTRL3_OM_DeleteVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    UI8_T           key[AMTRL3_OM_VXLAN_TUNNEL_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelKey(key, AMTRL3_OM_VXLAN_TUNNEL_KIDX, tunnel_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_DeleteRecord(&om_fib->amtrl3_om_vxlan_tunnel_hisam_desc, key);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    UI8_T key[AMTRL3_OM_VXLAN_TUNNEL_KEY_LEN];
    BOOL_T  rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T orig_priority;

    if (tunnel_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelKey(key, AMTRL3_OM_VXLAN_TUNNEL_KIDX, tunnel_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_GetRecord(&om_fib->amtrl3_om_vxlan_tunnel_hisam_desc,
                           AMTRL3_OM_VXLAN_TUNNEL_KIDX, key, (UI8_T *)tunnel_entry_p);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetNextVxlanTunnelEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    UI8_T           key[AMTRL3_OM_VXLAN_TUNNEL_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelKey(key, AMTRL3_OM_VXLAN_TUNNEL_KIDX, tunnel_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_GetNextRecord(&om_fib->amtrl3_om_vxlan_tunnel_hisam_desc,
                           AMTRL3_OM_VXLAN_TUNNEL_KIDX, key, (UI8_T *)tunnel_entry_p);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetVxlanTunnelEntryByVxlanPort(UI32_T fib_id, AMTRL3_TYPE_VxlanTunnelEntry_T *tunnel_entry_p)
{
    UI8_T                           key[AMTRL3_OM_VXLAN_TUNNEL_KEY_LEN];
    BOOL_T                          rc = FALSE;
    AMTRL3_OM_FIB_T                 *om_fib = NULL;
    UI32_T                          orig_priority;
    AMTRL3_OM_VxlanTunnelEntry_T    vxlan_tunnel;

    if (tunnel_entry_p == NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    memset(&vxlan_tunnel, 0, sizeof(AMTRL3_OM_VxlanTunnelEntry_T));
    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelKey(key, AMTRL3_OM_VXLAN_TUNNEL_KIDX, &vxlan_tunnel);

    while (L_HISAM_GetNextRecord(&om_fib->amtrl3_om_vxlan_tunnel_hisam_desc,
                           AMTRL3_OM_VXLAN_TUNNEL_KIDX, key, (UI8_T *)&vxlan_tunnel))
    {
        if (vxlan_tunnel.uc_vxlan_port == tunnel_entry_p->vxlan_port)
        {
            tunnel_entry_p->vfi_id = vxlan_tunnel.vfi_id;
            tunnel_entry_p->local_vtep = vxlan_tunnel.local_vtep;
            tunnel_entry_p->remote_vtep = vxlan_tunnel.remote_vtep;
            tunnel_entry_p->is_mc = vxlan_tunnel.is_mc;
            tunnel_entry_p->udp_port = vxlan_tunnel.udp_port;
            tunnel_entry_p->bcast_group = vxlan_tunnel.bcast_group;
            tunnel_entry_p->vxlan_port = vxlan_tunnel.uc_vxlan_port;
            rc = TRUE;
            break;
        }
        AMTRL3_OM_SetVxlanTunnelKey(key, AMTRL3_OM_VXLAN_TUNNEL_KIDX, &vxlan_tunnel);
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_SetVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p)
{
    UI32_T          rc = 0;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_nexthop_entry_p == NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);

    if ((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_SetRecord(&(om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc), (UI8_T *)tunnel_nexthop_entry_p, TRUE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    if ((rc == L_HISAM_INSERT) || (rc == L_HISAM_REPLACE))
        return TRUE;
    else
        return FALSE;
}

BOOL_T AMTRL3_OM_DeleteVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p)
{
    UI8_T           key[AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_nexthop_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelNexthopKey(key, AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX, tunnel_nexthop_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_DeleteRecord(&om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc, key);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p)
{
    UI8_T           key[AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_nexthop_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelNexthopKey(key, AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX, tunnel_nexthop_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_GetRecord(&om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc,
                           AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX, key, (UI8_T *)tunnel_nexthop_entry_p);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetNextVxlanTunnelNexthopEntry(UI32_T fib_id, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p)
{
    UI8_T           key[AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_LEN];
    BOOL_T          rc = FALSE;
    AMTRL3_OM_FIB_T *om_fib = NULL;
    UI32_T          orig_priority;

    if (tunnel_nexthop_entry_p == NULL)
        return FALSE;

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelNexthopKey(key, AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX, tunnel_nexthop_entry_p);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    rc = L_HISAM_GetNextRecord(&om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc,
                           AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX, key, (UI8_T *)tunnel_nexthop_entry_p);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return rc;
}

BOOL_T AMTRL3_OM_GetNextVxlanTunnelByNexthop(UI32_T fib_id,
                                             AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_p)
{
    AMTRL3_OM_VxlanTunnelNexthopEntry_T tunnel_nexthop;
    UI8_T                               key[AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KEY_LEN];
    AMTRL3_OM_FIB_T                     *om_fib = NULL;
    BOOL_T                              rc;
    UI32_T                              orig_priority;

    memcpy(&tunnel_nexthop, tunnel_nexthop_p, sizeof(AMTRL3_OM_VxlanTunnelNexthopEntry_T));

    memset(key, 0, sizeof(key));
    AMTRL3_OM_SetVxlanTunnelNexthopKey(key, AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP, &tunnel_nexthop);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(amtrl3_om_sem_id);
    if((om_fib = AMTRL3_OM_GetFIBByID(fib_id)) == NULL)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
        return FALSE;
    }

    do {
        rc = L_HISAM_GetNextRecord(&om_fib->amtrl3_om_vxlan_tunnel_nexthop_hisam_desc,
                                   AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP,
                                   key, (UI8_T *)&tunnel_nexthop);
        if (rc == FALSE)
            break;

        if (memcmp(tunnel_nexthop.nexthop_addr.addr, tunnel_nexthop_p->nexthop_addr.addr, SYS_ADPT_IPV4_ADDR_LEN) == 0 &&
            tunnel_nexthop.nexthop_ifindex == tunnel_nexthop_p->nexthop_ifindex)
        {
            memcpy(tunnel_nexthop_p, &tunnel_nexthop, sizeof(AMTRL3_OM_VxlanTunnelNexthopEntry_T));
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
            return TRUE;
        }
        else
        {
            break;
        }

        AMTRL3_OM_SetVxlanTunnelNexthopKey(key, AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP, &tunnel_nexthop);
    } while (rc);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(amtrl3_om_sem_id, orig_priority);
    return FALSE;
}

static void AMTRL3_OM_SetVxlanTunnelKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_VxlanTunnelEntry_T *tunnel_entry_p)
{
    switch (key_index)
    {
        /* KEY: vfi_id, local_vtep, remote_vtep
         */
        case AMTRL3_OM_VXLAN_TUNNEL_KIDX:
            memcpy(key, (UI8_T *)&tunnel_entry_p->vfi_id, SIZE_OF_VXLAN_TUNNEL_VFI_ID);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_VFI_ID, (UI8_T *)&tunnel_entry_p->local_vtep, SIZE_OF_VXLAN_TUNNEL_LOCAL_VTEP);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_VFI_ID + SIZE_OF_VXLAN_TUNNEL_LOCAL_VTEP, (UI8_T *)&tunnel_entry_p->remote_vtep, SIZE_OF_VXLAN_TUNNEL_REMOTE_VTEP);
            break;

        default:
            break;
    }
}

static void AMTRL3_OM_SetVxlanTunnelNexthopKey(UI8_T *key, UI32_T key_index, AMTRL3_OM_VxlanTunnelNexthopEntry_T *tunnel_nexthop_entry_p)
{
    switch (key_index)
    {
        /* KEY: vfi_id, local_vtep, remote_vtep, nexthop_addr
         */
        case AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX:
            memcpy(key, (UI8_T *)&tunnel_nexthop_entry_p->vfi_id, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID, (UI8_T *)&tunnel_nexthop_entry_p->local_vtep, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP, (UI8_T *)&tunnel_nexthop_entry_p->remote_vtep, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_REMOTE_VTEP);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_REMOTE_VTEP, (UI8_T *)&tunnel_nexthop_entry_p->nexthop_addr, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR);
            break;

        /* KEY: nexthop_addr, vfi_id, local_vtep, remote_vtep
         */
        case AMTRL3_OM_VXLAN_TUNNEL_NEXTHOP_KIDX_NEXTHOP:
            memcpy(key, (UI8_T *)&tunnel_nexthop_entry_p->nexthop_addr, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR, (UI8_T *)&tunnel_nexthop_entry_p->vfi_id, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID, (UI8_T *)&tunnel_nexthop_entry_p->local_vtep, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP);
            memcpy(key + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_NEXTHOP_ADDR + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_VFI_ID + SIZE_OF_VXLAN_TUNNEL_NEXTHOP_LOCAL_VTEP, (UI8_T *)&tunnel_nexthop_entry_p->remote_vtep, SIZE_OF_VXLAN_TUNNEL_NEXTHOP_REMOTE_VTEP);
            break;

        default:
            break;
    }
}

#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - AMTRL3_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for AMTRL3 OM.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRL3_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p)
{
    AMTRL3_OM_IPCMsg_T *msg_p;

    if (msgbuf_p == NULL)
        return FALSE;

    msg_p = (AMTRL3_OM_IPCMsg_T*)msgbuf_p->msg_buf;

    /* dispatch IPC message and call the corresponding AMTRL3_OM function
     */
    switch (msg_p->type.cmd)
    {
        case AMTRL3_OM_IPCCMD_GETIPNETTOPHYSICALENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetIpNetToPhysicalEntry(
                          msg_p->data.ip_net_to_physical.action_flags,
                          msg_p->data.ip_net_to_physical.fib_id,
                          &msg_p->data.ip_net_to_physical.ip_net_to_physical_entry);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(ip_net_to_physical);
            break;
        case AMTRL3_OM_IPCCMD_GETNEXTIPNETTOPHYSICALENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetNextIpNetToPhysicalEntry(
                          msg_p->data.ip_net_to_physical.action_flags,
                          msg_p->data.ip_net_to_physical.fib_id,
                          &msg_p->data.ip_net_to_physical.ip_net_to_physical_entry);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(ip_net_to_physical);
            break;
        case AMTRL3_OM_IPCCMD_GETINETHOSTROUTEENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetInetHostRouteEntry(
                          msg_p->data.host_route.action_flags,
                          msg_p->data.host_route.fib_id,
                          &msg_p->data.host_route.host_entry);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(host_route);
            break;
        case AMTRL3_OM_IPCCMD_GETNEXTINETHOSTROUTEENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetNextInetHostRouteEntry(
                          msg_p->data.host_route.action_flags,
                          msg_p->data.host_route.fib_id,
                          &msg_p->data.host_route.host_entry);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(host_route);
            break;
        case AMTRL3_OM_IPCCMD_GETTOTALHOSTROUTENUMBER:
            msg_p->type.result_ui32 = AMTRL3_OM_GetTotalHostRouteNumber(
                          msg_p->data.host_route.action_flags,
                          msg_p->data.host_route.fib_id);
            msgbuf_p->msg_size = AMTRL3_OM_MSGBUF_TYPE_SIZE;
            break;
#if (SYS_CPNT_VXLAN == TRUE)
        case AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY_BY_VXLAN_PORT:
            msg_p->type.result_bool = AMTRL3_OM_GetVxlanTunnelEntryByVxlanPort(
                          msg_p->data.vxlan_tunnel.fib_id,
                          &msg_p->data.vxlan_tunnel.tunnel);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel);
            break;
            
        case AMTRL3_OM_IPCCMD_GET_VXLAN_TUNNEL_ENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetVxlanTunnelEntry(
                          msg_p->data.vxlan_tunnel_entry.fib_id,
                          &msg_p->data.vxlan_tunnel_entry.tunnel);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel_entry);
            break;    
        case AMTRL3_OM_IPCCMD_GET_NEXT_VXLAN_TUNNEL_ENTRY:
            msg_p->type.result_bool = AMTRL3_OM_GetNextVxlanTunnelEntry(
                          msg_p->data.vxlan_tunnel_entry.fib_id,
                          &msg_p->data.vxlan_tunnel_entry.tunnel);
            msgbuf_p->msg_size = AMTRL3_OM_GET_MSG_SIZE(vxlan_tunnel_entry);
            break;        
#endif
        default:
            SYSFUN_Debug_Printf("\n%s(): Invalid cmd.\n", __FUNCTION__);
            msg_p->type.result_bool = FALSE;
            msgbuf_p->msg_size = AMTRL3_OM_MSGBUF_TYPE_SIZE;
    }

    return TRUE;
}

#if 0
/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_AssignInetAddress
 * -------------------------------------------------------------------------
 * PURPOSE: To copy an address to another address base on its address type.
 * INPUT:   dst_addr -  destination address
 *          src_addr -  source address
 *          addr_type - address type(ipv4\ipv4z\ipv6\ipv6z)
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 *--------------------------------------------------------------------------*/
void AMTRL3_OM_AssignInetAddress(AMTRL3_OM_InetAddress_T *dst_addr,
                                 AMTRL3_OM_InetAddress_T *src_addr,
                                 I32_T address_type)
{
    memset(dst_addr, 0, sizeof(AMTRL3_OM_InetAddress_T));

    switch(address_type)
    {
        case L_INET_ADDR_TYPE_IPV4:
            dst_addr->ipv4 = src_addr->ipv4;
            break;
        case L_INET_ADDR_TYPE_IPV4Z:
            dst_addr->ipv4z.ipv4 = src_addr->ipv4z.ipv4;
            dst_addr->ipv4z.zone_index = src_addr->ipv4z.zone_index;
            break;
        case L_INET_ADDR_TYPE_IPV6:
            memcpy(dst_addr->ipv6, src_addr->ipv6, SYS_ADPT_LENGTH_OF_INET_ADDRESS_TYPE_IPV6);
            break;
        case L_INET_ADDR_TYPE_IPV6Z:
            memcpy(dst_addr->ipv6z.ipv6, src_addr->ipv6z.ipv6, SYS_ADPT_LENGTH_OF_INET_ADDRESS_TYPE_IPV6);
            dst_addr->ipv6z.zone_index = src_addr->ipv6z.zone_index;
            break;
        default:
            break;
    }
}

/* --------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_OM_CleanInetAddress
 * -------------------------------------------------------------------------
 * PURPOSE: To set other fields to zero base on its address type.
 * INPUT:   inet_addr -  address
 *          addr_type - address type(ipv4\ipv4z\ipv6\ipv6z)
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 *--------------------------------------------------------------------------*/
void AMTRL3_OM_CleanInetAddress(AMTRL3_OM_InetAddress_T *inet_addr,
                                I32_T address_type)
{
    switch(address_type)
    {
        case L_INET_ADDR_TYPE_IPV4:
            memset((void *)inet_addr + sizeof(inet_addr->ipv4), 0, sizeof(AMTRL3_OM_InetAddress_T) - sizeof(inet_addr->ipv4));
            break;
        case L_INET_ADDR_TYPE_IPV4Z:
            memset((void *)inet_addr + sizeof(inet_addr->ipv4z), 0, sizeof(AMTRL3_OM_InetAddress_T) - sizeof(inet_addr->ipv4z));
            break;
        case L_INET_ADDR_TYPE_IPV6:
            memset((void *)inet_addr + sizeof(inet_addr->ipv6), 0, sizeof(AMTRL3_OM_InetAddress_T) - sizeof(inet_addr->ipv6));
            break;
        case L_INET_ADDR_TYPE_IPV6Z:
            memset((void *)inet_addr + sizeof(inet_addr->ipv6z), 0, sizeof(AMTRL3_OM_InetAddress_T) - sizeof(inet_addr->ipv6z));
            break;
        default:
            break;
    }
}
#endif
