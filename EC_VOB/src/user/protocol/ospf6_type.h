/******************************************************************************
 * Filename: ospf6_type.h
 * File description:
 *        
 * Copyright (C) 2005-2007 unknown, Inc. All Rights Reserved.
 *        
 * author: steven.gao
 * Create Date: Tuesday, July 14, 2009 
 *        
 * Modify History
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 *   Modifyer: 
 *   Date:     
 *   Description:
 *        
 * Version: 
 ******************************************************************************/
#ifndef _OSPF6_TYPE_H
#define _OSPF6_TYPE_H
/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_pal.h"  

#include "l_prefix.h"
#include "l_inet.h"
#include "l_bitmap.h"

#define OSPF6_UNIT_TEST

#define OSPF6_TAG_LEN       40
#define OSPF6_ROUTEMAP_LEN  20

#define OSPF6_TYPE_TIMER_STR_MAXLEN          9
#define OSPF6_TYPE_NAME                      10

#define OSPF6_TIMER_STR_MAXLEN          9

#define OSPF6_MAX_TAG_LENGTH            7

/* copy from ospf6_lsa.h */
/* LSA Function code. */
#define OSPF6_LSA_MIN                   1
#define OSPF6_LSA_ROUTER                1
#define OSPF6_LSA_NETWORK               2
#define OSPF6_LSA_INTER_PREFIX          3
#define OSPF6_LSA_INTER_ROUTER          4
#define OSPF6_LSA_AS_EXTERNAL           5
#define OSPF6_LSA_GROUP_MEMBERSHIP      6
#define OSPF6_LSA_NSSA                  7
#define OSPF6_LSA_LINK                  8
#define OSPF6_LSA_INTRA_PREFIX          9
#define OSPF6_LSA_LAST_KNOWN            10
#ifdef HAVE_OSPF6_TE
#define OSPF6_LSA_INTRA_AREA_TE         10
#define OSPF6_LSA_LAST_KNOWN            11
#endif /* HAVE_OSPF6_TE */
#define OSPF6_LSA_MAX                   (OSPF6_LSA_LAST_KNOWN + 1)
#define OSPF6_LSA_TYPE_MASK             0x1FFF

#define LSA_FLOODING_SCOPE_MASK         0x6000
#define LSA_FLOODING_SCOPE_UNKNOWN_MASK 0x8000
#define LSA_FLOODING_SCOPE_LINKLOCAL    0
#define LSA_FLOODING_SCOPE_AREA         1
#define LSA_FLOODING_SCOPE_AS           2
#define LSA_FLOODING_SCOPE_RESERVED     3

#define LSA_LINK_TYPE_POINTOPOINT       1
#define LSA_LINK_TYPE_TRANSIT           2
#define LSA_LINK_TYPE_STUB              3
#define LSA_LINK_TYPE_VIRTUALLINK       4
#define LSA_LINK_TYPE_MAX               5

#define ROUTER_LSA_BIT_B        (1 << 0)
#define ROUTER_LSA_BIT_E        (1 << 1)
#define ROUTER_LSA_BIT_V        (1 << 2)
#define ROUTER_LSA_BIT_W        (1 << 3)

#define AS_EXTERNAL_LSA_BIT_T       (1 << 0)
#define AS_EXTERNAL_LSA_BIT_F       (1 << 1)
#define AS_EXTERNAL_LSA_BIT_E       (1 << 2)

/* Macros. */
#define CODE2TYPE(C)              ospfv3_lsa_code2type[(C)]
#define LSA_CODE(T)                                                           \
        (((pal_ntoh16(T)  & OSPF6_LSA_TYPE_MASK)                              \
        >= OSPF6_LSA_LAST_KNOWN )  ? OSPF6_LSA_LAST_KNOWN :                   \
        (pal_ntoh16(T)  & OSPF6_LSA_TYPE_MASK))

/* Router-LSA. */
#define LSA_ROUTER_FLAGS_OFFSET         20
#define LSA_ROUTER_OPTIONS_OFFSET       21
#define LSA_ROUTER_LINK_DESC_OFFSET     24
#define LSA_ROUTER_LINK_DESC_SIZE       16

/* Network-LSA. */
#define LSA_NETWORK_OPTIONS_OFFSET      21
#define LSA_NETWORK_ROUTERS_OFFSET      24
#define LSA_NETWORK_ROUTER_ID_SIZE      4

/* Inter-Area-Prefix-LSA. */
#define LSA_IA_PREFIX_METRIC_OFFSET     21
#define LSA_IA_PREFIX_PREFIX_OFFSET     24

/* Inter-Area-Router-LSA. */
#define LSA_IA_ROUTER_OPTIONS_OFFSET        21
#define LSA_IA_ROUTER_METRIC_OFFSET         25
#define LSA_IA_ROUTER_ROUTER_ID_OFFSET      28

/* AS-external-LSA. */
#define LSA_AS_EXTERNAL_FLAGS_OFFSET        20
#define LSA_AS_EXTERNAL_METRIC_OFFSET       21
#define LSA_AS_EXTERNAL_PREFIX_OFFSET       24
#define LSA_AS_EXTERNAL_NEXTHOP_OFFSET(P)                                     \
    (LSA_AS_EXTERNAL_PREFIX_OFFSET + LSA_PREFIX_SPACE (P))

/* Link-LSA. */
#define LSA_LINK_PRIORITY_OFFSET        20
#define LSA_LINK_OPTIONS_OFFSET         21
#define LSA_LINK_LINKLOCAL_OFFSET       24
#define LSA_LINK_NUM_PREFIXES_OFFSET    40
#define LSA_LINK_PREFIX_OFFSET          44

/* Intra-Area-Prefix-LSA. */
#define LSA_INTRA_PREFIX_NUM_PREFIXES_OFFSET    20
#define LSA_INTRA_PREFIX_REF_LS_TYPE_OFFSET     22
#define LSA_INTRA_PREFIX_REF_LS_ID_OFFSET       24
#define LSA_INTRA_PREFIX_REF_ADV_ROUTER_OFFSET  28
#define LSA_INTRA_PREFIX_PREFIX_OFFSET          32

/* Externs. */
extern u_int16_t ospfv3_lsa_code2type[];

/* copy from opsf6_prefix.h */
#define OSPFV3_PREFIX_OPTION_NU     (1 << 0)    /* No Unicast.       */
#define OSPFV3_PREFIX_OPTION_LA     (1 << 1)    /* Local Address.    */
#define OSPFV3_PREFIX_OPTION_MC     (1 << 2)    /* Multicast.        */
#define OSPFV3_PREFIX_OPTION_P      (1 << 3)    /* Propagate (NSSA). */

typedef struct
{
  /* PrefixLength. */
  u_char length;

  /* PrefixOptions. */
  u_char options;

  /* Misc. field. */
  union
  {
    /* Referenced LS Type for Type-5. */
    u_int16_t ref_ls_type;

    /* Metric for Type-9. */
    u_int16_t metric;

    /* Always zero for Type-3, Type-8. */
    u_int16_t zero;

  } u;

  /* Address Prefix. */
  struct pal_in6_addr addr;
} OSPF6_TYPE_LSA_Prefix_T; // identical to ospf6_lsa_prefix, but to avoid redefinition.

/* copy from ospf6_debug.h */
#define OSPF6_ROUTER_LSA_BITS_STR_MAXLEN    8
#define OSPF6_PREFIX_OPTIONS_STRING_MAXLEN  12
#define OSPF6_OPTIONS_STR_MAXLEN    15

/* copy from ospf6d.h */
/* OSPFv3 options. */
#define OSPFV3_OPTION_V6 (1 << 0)   /* IPv6 forwarding. */
#define OSPFV3_OPTION_E  (1 << 1)   /* External routing. */
#define OSPFV3_OPTION_MC (1 << 2)   /* Multicast forwarding. */
#define OSPFV3_OPTION_N  (1 << 3)   /* Type-7(NSSA) LSA. */
#define OSPFV3_OPTION_R  (1 << 4)   /* Forwarding (Any Protocol). */
#define OSPFV3_OPTION_DC (1 << 5)   /* Demand Circuit handling. */


/**********************************
** definitions for return value **
**********************************
*/
enum
{
    OSPF6_TYPE_RESULT_SUCCESS = 0,
    OSPF6_TYPE_RESULT_FAIL,
    OSPF6_TYPE_RESULT_FAIL_ON_OM,
    OSPF6_TYPE_RESULT_WRONG_VALUE,
    OSPF6_TYPE_RESULT_INVALID_ARG,
    OSPF6_TYPE_RESULT_INVALID_NETWORK,
    OSPF6_TYPE_RESULT_INVALID_AREAID,
    OSPF6_TYPE_RESULT_INVALID_PROC_TAG,
    OSPF6_TYPE_RESULT_NETWORK_TYPE_INVALID,
    OSPF6_TYPE_RESULT_ABR_TYPE_INVALID,
    OSPF6_TYPE_RESULT_TIMER_VALUE_INVALID,
    OSPF6_TYPE_RESULT_COST_INVALID,
    OSPF6_TYPE_RESULT_ROUTER_ID_CONFIGURED, 
    OSPF6_TYPE_RESULT_VR_NOT_EXIST = 20,
    OSPF6_TYPE_RESULT_VRF_NOT_EXIST,
    OSPF6_TYPE_RESULT_VRF_ALREADY_BOUND,      
    OSPF6_TYPE_RESULT_PROCESS_NOT_EXIST,
    OSPF6_TYPE_RESULT_VLINK_CANT_GET,
    OSPF6_TYPE_RESULT_VLINK_NOT_EXIST,
    OSPF6_TYPE_RESULT_IF_NOT_EXIST,         
    OSPF6_TYPE_RESULT_AREA_NOT_EXIST = 30,
    OSPF6_TYPE_RESULT_AREA_IS_BACKBONE,
    OSPF6_TYPE_RESULT_AREA_IS_DEFAULT,
    OSPF6_TYPE_RESULT_AREA_IS_STUB,
    OSPF6_TYPE_RESULT_AREA_NOT_DEFAULT,
    OSPF6_TYPE_RESULT_AREA_ID_NOT_MATCH,
    OSPF6_TYPE_RESULT_AREA_RANGE_NOT_EXIST,
    OSPF6_TYPE_RESULT_AREA_HAS_VLINK,
    OSPF6_TYPE_RESULT_AREA_LIMIT,
    OSPF6_TYPE_RESULT_NBR_CONFIG_INVALID,
    OSPF6_TYPE_RESULT_NBR_P2MP_CONFIG_REQUIRED,
    OSPF6_TYPE_RESULT_NBR_P2MP_CONFIG_INVALID,
    OSPF6_TYPE_RESULT_NBR_NBMA_CONFIG_INVALID,
    OSPF6_TYPE_RESULT_NBR_STATIC_NOT_EXIST,
    OSPF6_TYPE_RESULT_CSPF_INSTANCE_EXIST,
    OSPF6_TYPE_RESULT_CSPF_INSTANCE_NOT_EXIST,
    OSPF6_TYPE_RESULT_CSPF_CANT_START,
    OSPF6_TYPE_RESULT_IF_PARAM_NOT_CONFIGURED,
    OSPF6_TYPE_RESULT_PROCESS_ID_INVALID,
    OSPF6_TYPE_RESULT_INCONSISTENT_VALUE,
    OSPF6_TYPE_RESULT_SEND_MSG_FAIL
};

typedef struct OSPF6_TYPE_Identity_S
{
    BOOL_T router_flag;

    struct pal_in4_addr router_id;

    struct prefix address;
}OSPF6_TYPE_Identity_T;

typedef struct OSPF6_TYPE_IfParam_S
{
    /* indexed by {vr_id, tag, ifindex, instance_id} */

    UI32_T  vr_id;                  /* VR ID. */
    char    tag[OSPF6_TAG_LEN];     /* OSPF6 process tag. */
    int     ifindex;                /* Interface index. */
    UI32_T  instance_id;            /* Instance ID */

    /*
     * Attributes 
     */
    UI32_T  area_id;
    /* OSPF area ID format. */
    UI8_T format;
  
    UI32_T  network_type;
    UI32_T  admin_status;
    
    UI8_T   priority;               /* Router priority. */
    UI32_T  output_cost;            /* Interface output cost. */

    UI32_T  transmit_delay;          /* Interface transmit delay. */
    UI32_T  retransmit_interval;     /* Retransmit interval. */
    UI32_T  hello_interval;          /* Hello interval. */
    UI32_T  dead_interval;           /* Router dead interval. */
    UI32_T  poll_interval;           /* Poll interval */

    /* IFSM State. */
    UI8_T   state;
    
    /* OSPF6 identity for DR election. */
    UI32_T  d_router;
    UI32_T  bd_router;

    UI32_T  events;                 /* ospfv3IfEvents */
    UI32_T  row_status;             /* ospfv3IfRowStatus */

    UI32_T  linklsa_count;          /* ospfv3IfLinkScopeLsaCount */
    UI32_T  linklsa_cksum;          /* ospfv3IfLinkLsaCksumSum */
#if 0
    UI32_T  ifdemand;               /* ospfv3IfDemand */
    UI32_T  demand_nbr_probe;       /* ospfv3IfDemandNbrProbe */
    UI32_T  demand_nbr_probe_retrans_limit; /* ospfv3IfDemandNbrProbeRetransLimit */
    UI32_T  demand_nbr_probe_interval;      /* ospfv3IfDemandNbrProbeInterval */
    UI32_T  te_status;                      /* ospfv3IfTEDisabled */
    UI32_T  link_lsa_suppress;           /* ospfv3IfLinkLSASuppression */
#endif
}OSPF6_TYPE_IfParam_T;

typedef struct OSPF6_TYPE_OspfInterfac_S
{
    /* indexed by {vr_id, tag, ifindex} */

    UI32_T  vr_id;                  /* VR ID. */
    char    tag[OSPF6_TAG_LEN];     /* OSPF6 process tag. */
    int     ifindex;                /* Interface index. */

    /*
     * Attributes 
     */
    UI32_T  instance_id;            /* Instance ID */
    UI32_T  area_id;
    UI32_T  network_type;
    UI32_T  admin_status;
    
    UI8_T   priority;             /* Router priority. */
    UI32_T  router_id;

    /* Interface output cost. */
    UI32_T  output_cost;

    UI32_T  transmit_delay;          /* Interface transmit delay. */
    UI32_T  retransmit_interval;     /* Retransmit interval. */
    UI32_T  hello_interval;          /* Hello interval. */
    UI32_T  dead_interval;           /* Router dead interval. */
    UI32_T  poll_interval;           /* Poll interval */

    UI8_T   flags;    
#define OSPF6_IF_UP                 (1 << 0)
#define OSPF6_IF_DESTROY            (1 << 1)
#define OSPF6_IF_WRITE_Q            (1 << 2)
#define OSPF6_IF_PASSIVE            (1 << 3)
#define OSPF6_IF_TRANSIT            (1 << 4)
#define OSPF6_IF_JOIN_MULTICAST     (1 << 5)

    /* IFSM State. */
    UI8_T   state;
    UI8_T   ostate;
    
    /* OSPF6 identity for DR election. */
    OSPF6_TYPE_Identity_T ident;
    OSPF6_TYPE_Identity_T designated_ident;
    OSPF6_TYPE_Identity_T backup_designated_ident;

    UI32_T  events;                 /* ospfv3IfEvents */
    UI32_T  row_status;             /* ospfv3IfRowStatus */

    UI32_T  linklsa_count;          /* ospfv3IfLinkScopeLsaCount */
    UI32_T  linklsa_cksum;          /* ospfv3IfLinkLsaCksumSum */
#if 0
    UI32_T  ifdemand;               /* ospfv3IfDemand */
    UI32_T  demand_nbr_probe;       /* ospfv3IfDemandNbrProbe */
    UI32_T  demand_nbr_probe_retrans_limit; /* ospfv3IfDemandNbrProbeRetransLimit */
    UI32_T  demand_nbr_probe_interval;      /* ospfv3IfDemandNbrProbeInterval */
    UI32_T  te_status;                      /* ospfv3IfTEDisabled */
    UI32_T  link_lsa_suppress;           /* ospfv3IfLinkLSASuppression */
#endif
    BOOL_T  passive_flag;

    UI32_T  neighbor_count;
    UI32_T  adjacent_neighbor_count;

    /* Statistics. */
    UI32_T  hello_in;           /* Hello packet input count. */
    UI32_T  hello_out;          /* Hello packet output count. */
    UI32_T  db_desc_in;         /* database desc. packet input count. */
    UI32_T  db_desc_out;        /* database desc. packet output count. */
    UI32_T  ls_req_in;          /* LS request packet input count. */
    UI32_T  ls_req_out;         /* LS request packet output count. */
    UI32_T  ls_upd_in;          /* LS update packet input count. */
    UI32_T  ls_upd_out;         /* LS update packet output count. */
    UI32_T  ls_ack_in;          /* LS Ack packet input count. */
    UI32_T  ls_ack_out;         /* LS Ack packet output count. */
    UI32_T  discarded;          /* Discarded input count by error. */
    UI32_T  state_change;       /* Number of IFSM state change. */

    char    timer_str[OSPF6_TYPE_TIMER_STR_MAXLEN];
}OSPF6_TYPE_Interface_T;




typedef struct OSPF6_TYPE_Area_S
{
    UI32_T  vr_id;
    char    tag[OSPF6_TAG_LEN];
    UI32_T  area_id;

    BOOL_T  first_flag; /* if it is to get the first */

    /* External routing capability. */
    UI8_T  external_routing;        /* normal area(1), STUB area(2) or NSSA area(3) */
    /* OSPF6 area type.  */
#define OSPF6_AREA_DEFAULT          0
#define OSPF6_AREA_STUB             1
#define OSPF6_AREA_TYPE_MAX         2
    /* Rowstatus*/
    UI8_T  status;
    
    /* Administrative flags. */
    UI8_T flags;
#define OSPF6_AREA_UP                 (1 << 0)
#define OSPF6_AREA_TRANSIT            (1 << 1)  /* TransitCapability. */
#define OSPF6_AREA_SHORTCUT           (1 << 2)  /* Other ABR agree on S-bit. */

      /* area->top Administrative flags. */
    UI16_T top_flags;
#define OSPF6_PROC_UP                   (1 << 0)
#define OSPF6_PROC_DESTROY              (1 << 1)
#define OSPF6_ROUTER_ABR                (1 << 2)
#define OSPF6_ROUTER_ASBR               (1 << 3)
#define OSPF6_ROUTER_DB_OVERFLOW        (1 << 4)
#define OSPF6_ROUTER_RESTART            (1 << 5)
#define OSPF6_LSDB_EXCEED_OVERFLOW_LIMIT    (1 << 6)
#define OSPF6_ASE_CALC_SUSPENDED        (1 << 7)
#define OSPF6_GRACE_LSA_ACK_RECD        (1 << 8)
#define OSPF6_ROUTER_RESTART_UPDATING    (1 << 9) /* process is up again when restart */

    UI32_T spf_calc_count;
    UI32_T abr_count;
    UI32_T asbr_count;
    UI32_T lsa_count;
    UI32_T lsa_checksum;
    /* Configuration flags.  */
    UI16_T config;
#define OSPF6_AREA_CONF_EXTERNAL_ROUTING   (1 << 0) /* external routing. */
#define OSPF6_AREA_CONF_NO_SUMMARY         (1 << 1) /* stub no import summary.*/
#define OSPF6_AREA_CONF_DEFAULT_COST       (1 << 2) /* stub default cost. */
#define OSPF6_AREA_CONF_AUTH_TYPE          (1 << 3) /* Area auth type. */
#define OSPF6_AREA_CONF_SHORTCUT_ABR       (1 << 10)/* Shortcut ABR. */
#define OSPF6_AREA_CONF_ACCESS_LIST_IN     (1 << 11)/* Access-List in. */
#define OSPF6_AREA_CONF_ACCESS_LIST_OUT    (1 << 12)/* Access-List out. */
#define OSPF6_AREA_CONF_PREFIX_LIST_IN    (1 << 13)/* Prefix-List in. */
#define OSPF6_AREA_CONF_PREFIX_LIST_OUT   (1 << 14)/* Prefix-List out. */
#define OSPF6_AREA_CONF_SNMP_CREATE        (1 << 15)/* ospfAreaStatus.  */

    /* Area ID format */
    UI8_T format;

    UI32_T  summary;

    UI32_T  stub_metric;
    UI32_T  stub_metric_type;
#define OSPF6_AREA_STUB_OSPFV3_METRIC   1
#define OSPF6_AREA_STUB_EXT_METRIC      2
#define OSPF6_AREA_STUB_EXT2_METRIC     3
}OSPF6_TYPE_Area_T;



typedef struct OSPF6_TYPE_Area_Range_S
{
    BOOL_T      is_first; 
    UI32_T      vr_id;
    char        tag[OSPF6_TAG_LEN];
    UI32_T      area_id;
    int         lsa_type;   /* InterAreaPrefixLSA or NSSAExternalLSA */
#define OSPF6_T_AREA_RANGE_TYPE_NSSAEXT_LSA       1
#define OSPF6_T_AREA_RANGE_TYPE_INTERAREA_LSA     2
    L_INET_AddrIp_T range;  /* including: PrefixType, Prefix, PrefixLength */
    
    UI32_T      format;
    UI32_T      effect;  /* Advertising or DoNotAdvertising */
#define OSPF6_T_AREA_RANGE_ADVERTISE   (1 << 0)
} OSPF6_TYPE_Area_Range_T;


/* OSPF6 nexthop. */
typedef struct OSPF6_TYPE_Route_Nexthop_S
{
  /* Flags. */
  UI32_T flags;
#define OSPF6_TYPE_NEXTHOP_INVALID      (1 << 0)
#define OSPF6_TYPE_NEXTHOP_CONNECTED    (1 << 1)
#define OSPF6_TYPE_NEXTHOP_UNNUMBERED   (1 << 2)
#define OSPF6_TYPE_NEXTHOP_VIA_TRANSIT  (1 << 3)

  /* Nexthop interface name. */
  char ifname[20];

  UI32_T area_flag;
#define OSPF6_TYPE_AREA_UP                 (1 << 0)
#define OSPF6_TYPE_AREA_TRANSIT            (1 << 1) /* TransitCapability. */
#define OSPF6_TYPE_AREA_SHORTCUT           (1 << 2) /* Other ABR agree on S-bit. */
  UI32_T area_id;
  char area_id_str[16];

  /* Neighbor IP address. */
  L_INET_AddrIp_T nbr_id;
}OSPF6_TYPE_Route_Nexthop_T;

typedef struct OSPF6_TYPE_Route_S
{
    UI32_T      vr_id;
    char        tag[OSPF6_TAG_LEN];
    BOOL_T      is_first;
    L_INET_AddrIp_T addr;
    
    UI32_T path_code_type;
    /* Path type code.  */
#define OSPF6_TYPE_PATH_CODE_UNKNOWN        0
#define OSPF6_TYPE_PATH_CODE_CONNECTED      1
#define OSPF6_TYPE_PATH_CODE_DISCARD        2
#define OSPF6_TYPE_PATH_CODE_INTRA_AREA     3
#define OSPF6_TYPE_PATH_CODE_INTER_AREA     4
#define OSPF6_TYPE_PATH_CODE_E1             5
#define OSPF6_TYPE_PATH_CODE_E2             6
#define OSPF6_TYPE_PATH_CODE_N1             7
#define OSPF6_TYPE_PATH_CODE_N2             8
    UI32_T flag;
    UI32_T type2_cost;
    UI32_T path_cost;

    UI32_T vec_num;
    OSPF6_TYPE_Route_Nexthop_T nh[SYS_ADPT_OSPF6_MAX_NEXTHOP];
}OSPF6_TYPE_Route_T;

/* OSPF Virtual-Link. */
typedef struct OSPF6_TYPE_Vlink_S
{
    /* Description. */
    char name[OSPF6_TYPE_NAME];
  
    UI32_T  vr_id;
    char    tag[OSPF6_TAG_LEN];
    /* Transit area for this virtual-link. */
    struct pal_in4_addr area_id;  

    /* Router-ID of virtual-link peer. */
    struct pal_in4_addr peer_id;
  
    /* Virtual-Link index. */
    UI32_T ifindex;

    UI32_T instance_id; 
    
    /* Area ID format. */
    UI32_T format;
  
    /* Flags. */
    BOOL_T flags;
  
    /* Row Status. */
    UI8_T   status;
    UI8_T   oi_state;
    UI32_T  events;
    UI8_T   nbr_state;
    BOOL_T  nbr_flag;
    BOOL_T  addr_flag;
    BOOL_T  dest_flag;
    
    struct prefix address;
  
    struct prefix destination;

    /* Interface transmit delay. */
    UI32_T transmit_delay;

    /* Retransmit interval. */
    UI32_T retransmit_interval;

    /* Hello interval. */
    UI32_T hello_interval;

    /* Router dead interval. */
    UI32_T dead_interval;

    UI32_T link_lsa_count;
    UI32_T link_lsa_cksum;

    char timer_str[OSPF6_TYPE_TIMER_STR_MAXLEN];

    /* Configured flags. */
    UI32_T config;
}OSPF6_TYPE_Vlink_T;

#define OSPF6_TYPE_AREA_VLINK_CREATE                0
#define OSPF6_TYPE_AREA_VLINK_DEAD_INTERVAL         1
#define OSPF6_TYPE_AREA_VLINK_HELLO_INTERVAL        2
#define OSPF6_TYPE_AREA_VLINK_RETRANSMIT_INTERVAL   3
#define OSPF6_TYPE_AREA_VLINK_TRANSMIT_DELAY        4
#define OSPF6_TYPE_AREA_VLINK_DELETE                5

typedef struct OSPF6_TYPE_Multi_Proc_Redist_S
{
    UI32_T vr_id;
    char   tag[OSPF6_TAG_LEN];
    UI32_T proto;
#define OSPF6_TYPE_REDISTRIBUTE_DEFAULT                0
#define OSPF6_TYPE_REDISTRIBUTE_KERNEL                 1
#define OSPF6_TYPE_REDISTRIBUTE_CONNECTED              2
#define OSPF6_TYPE_REDISTRIBUTE_STATIC                 3
#define OSPF6_TYPE_REDISTRIBUTE_RIP                    4
#define OSPF6_TYPE_REDISTRIBUTE_RIPNG                  5

    UI32_T metric;
    UI32_T metric_type;
    UI32_T status;
    char mapname[OSPF6_ROUTEMAP_LEN];
}OSPF6_TYPE_Multi_Proc_Redist_T;





#define OSPF6_TYPE_LSDB_ADVERTISE_SIZE  SYS_ADPT_IF_MTU





typedef struct 
{
    UI32_T      vr_id;
    char        tag[OSPF6_TAG_LEN];

    BOOL_T      is_first; 
    UI32_T      abr_type;
    UI32_T      default_metric;
    UI32_T      max_concurrent_dd;
    UI32_T      router_id;          /* 0: for un-configured */
    UI32_T      spf_delay;
    UI32_T      spf_hold;
} OSPF6_TYPE_PROC_PARAM_T; 

typedef struct 
{
    BOOL_T      status; /* TRUE for enable, FALSE for disable */
    UI32_T      metric;
    UI32_T      metric_type;
} OSPF6_TYPE_REDISTRIBUTE_PARAM_T; 

typedef struct 
{
    UI32_T  area_id;
    UI32_T  format;
    UI16_T  type;
    BOOL_T  is_first; 
    BOOL_T  summary;     /* TRUE for SendSummary */
    UI32_T  default_cost; 
} OSPF6_TYPE_AREA_PARAM_T;


typedef struct 
{
    BOOL_T          is_first;       /* used by GetNext */
    L_INET_AddrIp_T addr;
    UI32_T          effect;
} OSPF6_TYPE_RANGE_PARAM_T;

typedef struct 
{
    UI32_T  nbr_id;

    UI32_T  c_flags;
#define OSPF6_TYPE_VLINK_CFLAGS_HELLO       (1<<0)
#define OSPF6_TYPE_VLINK_CFLAGS_DEAD        (1<<1)
#define OSPF6_TYPE_VLINK_CFLAGS_RETRANS     (1<<2)
#define OSPF6_TYPE_VLINK_CFLAGS_TRANSMIT    (1<<3)
    UI32_T  hello_interval;
    UI32_T  dead_interval;
    UI32_T  retransmit_interval;
    UI32_T  transmit_delay;
} OSPF6_TYPE_VLINK_PARAM_T;


typedef struct
{
    UI32_T      ifindex;    /* key */
    
    BOOL_T      passive_if;
} OSPF6_TYPE_L3IF;


typedef struct 
{
    UI32_T  ifindex;            /* key 1*/
    UI32_T  instance_id;        /* key 2*/

    UI32_T  c_flags;
#define OSPF6_TYPE_IFP_CFLAGS_HELLO       (1<<0)
#define OSPF6_TYPE_IFP_CFLAGS_DEAD        (1<<1)
#define OSPF6_TYPE_IFP_CFLAGS_RETRANS     (1<<2)
#define OSPF6_TYPE_IFP_CFLAGS_TRANSMIT    (1<<3)
#define OSPF6_TYPE_IFP_CFLAGS_PRIORIFY    (1<<4)
#define OSPF6_TYPE_IFP_CFLAGS_COST        (1<<5)
#define OSPF6_TYPE_IFP_CFLAGS_ROUTER      (1<<6)

    UI32_T  type; /* only broadcast is provided */
    
    UI32_T  priority;
    UI32_T  cost;

    UI32_T  hello_interval;
    UI32_T  dead_interval;
    UI32_T  retransmit_interval;
    UI32_T  transmit_delay;

    char        tag[OSPF6_TAG_LEN];
    UI32_T      area_id;
} OSPF6_OM_IF_PARAMS_T;


/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif /*_OSPF6_TYPE_H*/

