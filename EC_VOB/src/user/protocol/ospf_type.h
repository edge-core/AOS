/* MODULE NAME:  ospf_type.h
 * PURPOSE:
 *     Define common types used in OSPF.
 *
 * NOTES:
 *
 * HISTORY
 *    11/27/2008 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef OSPF_TYPE_H
#define OSPF_TYPE_H

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_pal.h"  
#include "l_prefix.h"
#include "l_inet.h" /* L_INET_AddrIp_T */

#define OSPF_TYPE_TIMER_STR_MAXLEN          9
#define OSPF_TYPE_NAME                      10

/*if auth key */
#define OSPF_AUTH_SIMPLE_SIZE        8


/**********************************
** definitions for return value **
**********************************
*/
enum
{
    OSPF_TYPE_RESULT_SUCCESS = 0,
    OSPF_TYPE_RESULT_FAIL,
    OSPF_TYPE_RESULT_WRONG_VALUE,
    OSPF_TYPE_RESULT_INVALID_ARG,
    OSPF_TYPE_RESULT_INVALID_NETWORK,
    OSPF_TYPE_RESULT_NETWORK_TYPE_INVALID,
    OSPF_TYPE_RESULT_ABR_TYPE_INVALID,
    OSPF_TYPE_RESULT_TIMER_VALUE_INVALID,
    OSPF_TYPE_RESULT_COST_INVALID,
    OSPF_TYPE_RESULT_VR_NOT_EXIST,
    OSPF_TYPE_RESULT_VRF_NOT_EXIST,
    OSPF_TYPE_RESULT_VRF_ALREADY_BOUND,      
    OSPF_TYPE_RESULT_NETWORK_EXIST,
    OSPF_TYPE_RESULT_NETWORK_NOT_EXIST,
    OSPF_TYPE_RESULT_VLINK_CANT_GET,
    OSPF_TYPE_RESULT_DISTANCE_NOT_EXIST,
    OSPF_TYPE_RESULT_HOST_ENTRY_EXIST,       
    OSPF_TYPE_RESULT_PROCESS_NOT_EXIST,
    OSPF_TYPE_RESULT_IF_NOT_EXIST,         
    OSPF_TYPE_RESULT_AREA_NOT_EXIST,
    OSPF_TYPE_RESULT_AREA_IS_BACKBONE,
    OSPF_TYPE_RESULT_AREA_IS_DEFAULT,
    OSPF_TYPE_RESULT_AREA_IS_STUB,
    OSPF_TYPE_RESULT_AREA_IS_NSSA,
    OSPF_TYPE_RESULT_AREA_NOT_DEFAULT,
    OSPF_TYPE_RESULT_AREA_ID_NOT_MATCH,
    OSPF_TYPE_RESULT_AREA_RANGE_NOT_EXIST,
    OSPF_TYPE_RESULT_AREA_HAS_VLINK,
    OSPF_TYPE_RESULT_AREA_LIMIT,
    OSPF_TYPE_RESULT_NBR_CONFIG_INVALID,
    OSPF_TYPE_RESULT_NBR_P2MP_CONFIG_REQUIRED,
    OSPF_TYPE_RESULT_NBR_P2MP_CONFIG_INVALID,
    OSPF_TYPE_RESULT_NBR_NBMA_CONFIG_INVALID,
    OSPF_TYPE_RESULT_NBR_STATIC_NOT_EXIST,
    OSPF_TYPE_RESULT_CSPF_INSTANCE_EXIST,
    OSPF_TYPE_RESULT_CSPF_INSTANCE_NOT_EXIST,
    OSPF_TYPE_RESULT_CSPF_CANT_START,
    OSPF_TYPE_RESULT_AUTH_TYPE_INVALID,
    OSPF_TYPE_RESULT_IF_PARAM_NOT_CONFIGURED,
    OSPF_TYPE_RESULT_PROCESS_ID_INVALID,
    OSPF_TYPE_RESULT_AREA_NOT_NSSA,
    OSPF_TYPE_RESULT_MD5_KEY_EXIST,
    OSPF_TYPE_RESULT_MD5_KEY_NOT_EXIST,
    OSPF_TYPE_RESULT_INCONSISTENT_VALUE,

    OSPF_TYPE_RESULT_SEND_MSG_FAIL
};

typedef struct
{
    BOOL_T router_flag;

    struct pal_in4_addr router_id;

    struct prefix address;
}OSPF_TYPE_Identity_T;

typedef struct
{
    /* Interface index. */
    UI32_T ifindex;

    /* VR ID. */
    UI32_T vr_id;

    /* OSPF process ID. */
    UI32_T ospf_id;
    char auth_key[OSPF_AUTH_SIMPLE_SIZE + 1];

    UI32_T mtu_ignore;

    BOOL_T passive_if_flag; 

    BOOL_T unnumbered_flag;

    /* OSPF identity for DR election. */
    OSPF_TYPE_Identity_T ident;

    OSPF_TYPE_Identity_T designated_ident;

    OSPF_TYPE_Identity_T backup_designated_ident;

    BOOL_T filter_flag;

    BOOL_T link_flag;
     
    struct pal_in4_addr area_id;
    
    UI8_T external_routing; /*NSSA , STUB, default*/

    /* IFSM State. */
    UI8_T state;
    UI8_T ostate;
    
    /* Destination prefix for PPP and virtual link. */
    struct prefix destination;
   
    /* Number of interfaces which have the same network addresses. */
    int clone;
  
    /* OSPF Network Type. */
    UI8_T network_type;	
    
    /* Authentication type. */
    UI8_T auth_type;
  
    /* Router priority. */
    UI8_T priority;
  
    /* MTU size. */
    UI16_T mtu;
  
    /* Interface output cost. */
    UI32_T output_cost;
  
    /* Interface transmit delay. */
    UI32_T transmit_delay;
  
    /* Retransmit interval. */
    UI32_T retransmit_interval;
  
    /* Hello interval. */
    UI32_T hello_interval;
  
    /* Router dead interval. */
    UI32_T dead_interval;

    UI32_T poll_interval;
  
    /* Resync timeout. */
    UI32_T resync_timeout;

    UI32_T neighbor_count;

    UI32_T adjacent_neighbor_count;

      /* Statistics. */
    UI32_T hello_in;           /* Hello packet input count. */
    UI32_T hello_out;          /* Hello packet output count. */
    UI32_T db_desc_in;         /* database desc. packet input count. */
    UI32_T db_desc_out;        /* database desc. packet output count. */
    UI32_T ls_req_in;          /* LS request packet input count. */
    UI32_T ls_req_out;         /* LS request packet output count. */
    UI32_T ls_upd_in;          /* LS update packet input count. */
    UI32_T ls_upd_out;         /* LS update packet output count. */
    UI32_T ls_ack_in;          /* LS Ack packet input count. */
    UI32_T ls_ack_out;         /* LS Ack packet output count. */
    UI32_T discarded;          /* Discarded input count by error. */
    UI32_T state_change;       /* Number of IFSM state change. */

    /* Cryptographic Sequence Number. */ 
    UI32_T crypt_seqnum;
    char timer_str[OSPF_TYPE_TIMER_STR_MAXLEN];
}OSPF_TYPE_OspfInterfac_T;

typedef struct
{
    /* Interface index. */
    UI32_T ifindex;

    struct pal_in4_addr if_addr;
    UI32_T addressless_if;

    UI32_T output_cost;

    UI16_T mtu;
    
    UI32_T mtu_ignore;

    struct pal_in4_addr area_id;
    UI16_T type; /* if_type */
    UI16_T admin_status;

    UI8_T priority;

    UI32_T transmit_delay;

    UI32_T retransmit_interval;

    UI32_T hello_interval; 

    UI32_T dead_interval;

    UI32_T poll_interval;

    UI8_T state;

    struct pal_in4_addr dr_addr;

    struct pal_in4_addr bdr_addr;

    UI32_T events;
    
    char auth_key[OSPF_AUTH_SIMPLE_SIZE + 1];   

    UI16_T status;
    UI16_T multicast_forwarding;
    UI16_T demand;

    UI8_T auth_type;

    struct pal_in4_addr dr_id;

    struct pal_in4_addr bdr_id; 

} OSPF_TYPE_Msg_OspfInterfac_T;


typedef struct
{
    /* VR ID. */
    UI32_T vr_id;

    /* OSPF process ID. */
    UI32_T proc_id;
 
    UI32_T ip_address;
    UI32_T addressless_if;
    UI32_T tos;
    UI32_T value;
    UI32_T status;

} OSPF_TYPE_IfMetricEntry_T;

enum OSPF_TYPE_AreaNssa_TranslatorRole_Type_E
{
    OSPF_TYPE_TRANSLATOR_ALAWAYS = 0,
    OSPF_TYPE_TRANSLATOR_CANDIDATE,
    OSPF_TYPE_TRANSLATOR_NEVER
};

enum OSPF_TYPE_AreaNssa_Metric_Type_E
{
    OSPF_TYPE_METRIC_TYPE_1 = 1,
    OSPF_TYPE_METRIC_TYPE_2
};

typedef struct
{
	/* Configuration flags.  */
	UI32_T config;
#define OSPF_TYPE_AREA_NSSA_CONF_TRANSLATOR_ROLE   				(1 << 0) 
#define OSPF_TYPE_AREA_NSSA_CONF_NO_REDISTRIBUTION   			(1 << 1) 
#define OSPF_TYPE_AREA_NSSA_CONF_NO_SUMMARY       				(1 << 2) 
#define OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION    			(1 << 3)
#define OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC    		(1 << 4)
#define OSPF_TYPE_AREA_NSSA_CONF_DFLT_INFORMATION_METRIC_TYPE    (1 << 5)
    
	enum OSPF_TYPE_AreaNssa_TranslatorRole_Type_E translator_role;
    
	UI32_T metric;    

	enum OSPF_TYPE_AreaNssa_Metric_Type_E metric_type;
}OSPF_TYPE_Area_Nssa_Para_T;

#define OSPF_TYPE_VLINK_AUTH_SIMPLE_SIZE        8
#define OSPF_TYPE_VLINK_AUTH_MD5_SIZE          16

/* for OSPF_TYPE_Area_Virtual_Link_Para_T.config */
#define OSPF_TYPE_AREA_VLINK_DEAD_INTERVAL   				(1 << 0) 
#define OSPF_TYPE_AREA_VLINK_HELLO_INTERVAL  			    (1 << 1) 
#define OSPF_TYPE_AREA_VLINK_RETRANSMIT_INTERVAL      		(1 << 2) 
#define OSPF_TYPE_AREA_VLINK_TRANSMIT_DELAY    			    (1 << 3)
#define OSPF_TYPE_AREA_VLINK_AUTHENTICATION   		        (1 << 4)
#define OSPF_TYPE_AREA_VLINK_AUTHENTICATIONKEY              (1 << 5)
#define OSPF_TYPE_AREA_VLINK_MESSAGEDIGESTKEY               (1 << 6)

/* for OSPF_TYPE_Area_Virtual_Link_Para_T.auth_type */
#define OSPF_TYPE_AUTH_NULL              0
#define OSPF_TYPE_AUTH_SIMPLE            1
#define OSPF_TYPE_AUTH_CRYPTOGRAPHIC     2

/* OSPF area virtual-link parameters. */
typedef struct
{
    UI32_T vlink_addr;
    /* Configuration flags.  */
    UI32_T config; /* OSPF_TYPE_AREA_VLINK_... */
    UI32_T dead_interval;
    UI32_T hello_interval;
    UI32_T retransmit_interval;
    UI32_T transmit_delay;
    char   auth_key[OSPF_TYPE_VLINK_AUTH_SIMPLE_SIZE + 1];
    char   md5_key[OSPF_TYPE_VLINK_AUTH_MD5_SIZE + 1];
    UI32_T key_id;
    UI32_T auth_type; /* OSPF_TYPE_AUTH_... */

}OSPF_TYPE_Area_Virtual_Link_Para_T;

typedef struct
{
    BOOL_T first_flag;/*for get next, if the flag is true, get first*/
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T area_id;
    UI32_T format;
    char area_id_str[16];
    UI32_T area_type;
#define OSPF_TYPE_AREA_DEFAULT			0
#define OSPF_TYPE_AREA_STUB				1
#define OSPF_TYPE_AREA_NSSA				2
#define OSPF_TYPE_AREA_TYPE_MAX			3
    OSPF_TYPE_Area_Nssa_Para_T  nssa;
    BOOL_T stub_no_summary_flag;
    BOOL_T default_cost_flag;
    UI32_T default_cost;
    UI32_T auth_type;
} OSPF_TYPE_Area_Para_T;

typedef struct
{
    UI32_T indexlen;
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T area_id;
    /* OSPF Authentication Type. */
    UI8_T  auth_type;
#define OSPF_AREA_AUTH_NONE             0
#define OSPF_AREA_AUTH_SIMPLE_PASSWORD  1
#define OSPF_AREA_AUTH_MD5              2
    /* External routing capability. */
    UI8_T  external_routing;
    /* OSPF area type.  */
#define OSPF_AREA_DEFAULT			0
#define OSPF_AREA_STUB				1
#define OSPF_AREA_NSSA				2
#define OSPF_AREA_TYPE_MAX			3
    /* Rowstatus*/
    UI8_T  status;
    
    /* Administrative flags. */
    UI8_T flags;
#define OSPF_AREA_UP                 (1 << 0)
#define OSPF_AREA_TRANSIT            (1 << 1)	/* TransitCapability. */
#define OSPF_AREA_SHORTCUT           (1 << 2)	/* Other ABR agree on S-bit. */

      /* area->top Administrative flags. */
    UI16_T top_flags;
#define OSPF_PROC_UP			(1 << 0)
#define OSPF_PROC_DESTROY		(1 << 1)
#define OSPF_ROUTER_ABR			(1 << 2)
#define OSPF_ROUTER_ASBR		(1 << 3)
#define OSPF_ROUTER_DB_OVERFLOW		(1 << 4)
#define OSPF_ROUTER_RESTART		(1 << 5)
#define OSPF_LSDB_EXCEED_OVERFLOW_LIMIT	(1 << 6)
#define OSPF_ASE_CALC_SUSPENDED		(1 << 7)
#define OSPF_GRACE_LSA_ACK_RECD		(1 << 8)
#define OSPF_ROUTER_RESTART_UPDATING    (1 << 9) /* process is up again when restart */

    UI32_T spf_calc_count;
    UI32_T abr_count;
    UI32_T asbr_count;
    UI32_T lsa_count;
    UI32_T lsa_checksum;
    /* Configuration flags.  */
    UI16_T config;
#define OSPF_AREA_CONF_EXTERNAL_ROUTING   (1 << 0) /* external routing. */
#define OSPF_AREA_CONF_NO_SUMMARY         (1 << 1) /* stub no import summary.*/
#define OSPF_AREA_CONF_DEFAULT_COST       (1 << 2) /* stub default cost. */
#define OSPF_AREA_CONF_AUTH_TYPE          (1 << 3) /* Area auth type. */
#define OSPF_AREA_CONF_NSSA_TRANSLATOR    (1 << 4) /* NSSA translate role. */
#define OSPF_AREA_CONF_STABILITY_INTERVAL (1 << 5) /* NSSA StabilityInterval.*/
#define OSPF_AREA_CONF_NO_REDISTRIBUTION  (1 << 6) /* NSSA redistribution. */
#define OSPF_AREA_CONF_DEFAULT_ORIGINATE  (1 << 7) /* NSSA default originate.*/
#define OSPF_AREA_CONF_METRIC		      (1 << 8) /* NSSA default metric. */
#define OSPF_AREA_CONF_METRIC_TYPE	      (1 << 9) /* NSSA default mtype. */
#define OSPF_AREA_CONF_SHORTCUT_ABR       (1 << 10)/* Shortcut ABR. */
#define OSPF_AREA_CONF_ACCESS_LIST_IN     (1 << 11)/* Access-List in. */
#define OSPF_AREA_CONF_ACCESS_LIST_OUT    (1 << 12)/* Access-List out. */
#define OSPF_AREA_CONF_PREFIX_LIST_IN	  (1 << 13)/* Prefix-List in. */
#define OSPF_AREA_CONF_PREFIX_LIST_OUT	  (1 << 14)/* Prefix-List out. */
#define OSPF_AREA_CONF_SNMP_CREATE        (1 << 15)/* ospfAreaStatus.  */
} OSPF_TYPE_Area_T;

typedef struct
{
    UI32_T indexlen;
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T area_id;
    UI32_T stub_tos;
    UI32_T default_cost; // i.e metric
    UI8_T  metric_type;
    UI8_T  status;
} OSPF_TYPE_Stub_Area_T;


typedef struct
{
    UI32_T indexlen;
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T area_id;
    UI32_T type;
    UI32_T range_addr;
    UI32_T range_mask;
    
    char area_id_str[16];
    UI32_T status;
    /* Flag for advertise or not. */
    UI32_T flags;
#define OSPF_AREA_RANGE_ADVERTISE   (1 << 0)
#define OSPF_AREA_RANGE_SUBSTITUTE  (1 << 1)
}OSPF_TYPE_Area_Range_T;

typedef struct
{
    UI32_T indexlen;
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T area_id;
    
    /* Configuration flags.  */
    UI16_T config;
#define OSPF_AREA_CONF_EXTERNAL_ROUTING   (1 << 0) /* external routing. */
#define OSPF_AREA_CONF_NO_SUMMARY         (1 << 1) /* stub no import summary.*/
#define OSPF_AREA_CONF_DEFAULT_COST       (1 << 2) /* stub default cost. */
#define OSPF_AREA_CONF_AUTH_TYPE          (1 << 3) /* Area auth type. */
#define OSPF_AREA_CONF_NSSA_TRANSLATOR    (1 << 4) /* NSSA translate role. */
#define OSPF_AREA_CONF_STABILITY_INTERVAL (1 << 5) /* NSSA StabilityInterval.*/
#define OSPF_AREA_CONF_NO_REDISTRIBUTION  (1 << 6) /* NSSA redistribution. */
#define OSPF_AREA_CONF_DEFAULT_ORIGINATE  (1 << 7) /* NSSA default originate.*/
#define OSPF_AREA_CONF_METRIC		      (1 << 8) /* NSSA default metric. */
#define OSPF_AREA_CONF_METRIC_TYPE	      (1 << 9) /* NSSA default mtype. */
#define OSPF_AREA_CONF_SHORTCUT_ABR       (1 << 10)/* Shortcut ABR. */
#define OSPF_AREA_CONF_ACCESS_LIST_IN     (1 << 11)/* Access-List in. */
#define OSPF_AREA_CONF_ACCESS_LIST_OUT    (1 << 12)/* Access-List out. */
#define OSPF_AREA_CONF_PREFIX_LIST_IN	  (1 << 13)/* Prefix-List in. */
#define OSPF_AREA_CONF_PREFIX_LIST_OUT	  (1 << 14)/* Prefix-List out. */
#define OSPF_AREA_CONF_SNMP_CREATE        (1 << 15)/* ospfAreaStatus.  */
    UI32_T metric_type;
    UI32_T metric;
    UI32_T translator_role;
#define OSPF_NSSA_TRANSLATE_NEVER	0
#define OSPF_NSSA_TRANSLATE_ALWAYS	1
#define OSPF_NSSA_TRANSLATE_CANDIDATE	2
    UI32_T translator_state;
#define OSPF_NSSA_TRANSLATOR_DISABLED	0
#define OSPF_NSSA_TRANSLATOR_ENABLED	1
#define OSPF_NSSA_TRANSLATOR_ELECTED	2
    

}OSPF_TYPE_Nssa_Area_T;

typedef struct
{
  /* CLI argument. */
  int proc_id;

  /* Flags. */
  int flags;
#define OSPF_SHOW_ROUTE_STATE_INIT	(1 << 0)
  
  /* Prefix. */
  UI32_T addr;
  UI32_T masklen;
}OSPF_TYPE_Route_Arg_T;

/* OSPF nexthop. */
typedef struct
{
  /* Flags. */
  UI32_T flags;
#define OSPF_TYPE_NEXTHOP_INVALID		(1 << 0)
#define OSPF_TYPE_NEXTHOP_CONNECTED		(1 << 1)
#define OSPF_TYPE_NEXTHOP_UNNUMBERED	(1 << 2)
#define OSPF_TYPE_NEXTHOP_VIA_TRANSIT	(1 << 3)

  /* Nexthop interface name. */
  char ifname[20];

  UI32_T area_flag;
#define OSPF_TYPE_AREA_UP                 (1 << 0)
#define OSPF_TYPE_AREA_TRANSIT            (1 << 1)	/* TransitCapability. */
#define OSPF_TYPE_AREA_SHORTCUT           (1 << 2)	/* Other ABR agree on S-bit. */
  UI32_T area_id;
  char area_id_str[16];

  /* Neighbor IP interface address. */
  UI32_T nbr_id;
}OSPF_TYPE_Route_Nexthop_T;

typedef struct
{
    UI32_T vr_id;
    UI32_T proc_id;
    OSPF_TYPE_Route_Arg_T arg;
    UI32_T path_code_type;
    /* Path type code.  */
#define OSPF_TYPE_PATH_CODE_UNKNOWN			 0
#define OSPF_TYPE_PATH_CODE_CONNECTED		 1
#define OSPF_TYPE_PATH_CODE_DISCARD			 2
#define OSPF_TYPE_PATH_CODE_INTRA_AREA		 3
#define OSPF_TYPE_PATH_CODE_INTER_AREA		 4
#define OSPF_TYPE_PATH_CODE_E1			 5
#define OSPF_TYPE_PATH_CODE_E2			 6
#define OSPF_TYPE_PATH_CODE_N1			 7
#define OSPF_TYPE_PATH_CODE_N2			 8
    UI32_T flag;
    UI32_T type2_cost;
    UI32_T path_cost;
    OSPF_TYPE_Route_Nexthop_T nh;
    UI32_T vec_num;
}OSPF_TYPE_Route_T;

/* OSPF Virtual-Link. */
typedef struct
{
    /* Description. */
    char name[OSPF_TYPE_NAME];
  
    char ifname[OSPF_TYPE_NAME];
  
    /* Virtual-Link index. */
    UI32_T index;

    UI32_T proc_id;

    UI32_T vr_id;
    /* Transit area for this virtual-link. */
    struct pal_in4_addr area_id;  

    char area_id_str[16];
    
    /* Router-ID of virtual-link peer. */
    struct pal_in4_addr peer_id;
  
    /* Interface MTU. */
    int mtu;
  
    /* Area ID format. */
    UI32_T format;
  
    /* Flags. */
    BOOL_T flags;
  
    /* Row Status. */
    UI8_T status;
    UI8_T oi_state;
    UI32_T events;
    UI8_T nbr_state;
    BOOL_T nbr_flag;
    BOOL_T addr_flag;
    BOOL_T dest_flag;
    
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

    char timer_str[OSPF_TYPE_TIMER_STR_MAXLEN];

    /* Authentication type. */
    UI32_T auth_type;
    char auth_simple[9];
    
    /* Password list for crypt. */
    char auth_crypt[256][17];
    UI32_T key_id;
      /* Configured flags. */
    UI32_T config;
#define OSPF_IF_PARAM_NETWORK_TYPE		(1 << 0)
#define OSPF_IF_PARAM_AUTH_TYPE			(1 << 1)
#define OSPF_IF_PARAM_PRIORITY			(1 << 2)
#define OSPF_IF_PARAM_OUTPUT_COST		(1 << 3)
#define OSPF_IF_PARAM_TRANSMIT_DELAY		(1 << 4)
#define OSPF_IF_PARAM_RETRANSMIT_INTERVAL	(1 << 5)
#define OSPF_IF_PARAM_HELLO_INTERVAL		(1 << 6)
#define OSPF_IF_PARAM_DEAD_INTERVAL		(1 << 7)
#define OSPF_IF_PARAM_AUTH_SIMPLE		(1 << 8)
#define OSPF_IF_PARAM_AUTH_CRYPT		(1 << 9)
#define OSPF_IF_PARAM_DATABASE_FILTER		(1 << 10)
#define OSPF_IF_PARAM_TE_METRIC                 (1 << 11)
#define OSPF_IF_PARAM_DISABLE_ALL		(1 << 12)
#define OSPF_IF_PARAM_MTU			(1 << 13)
#define OSPF_IF_PARAM_MTU_IGNORE		(1 << 14)
#define OSPF_IF_PARAM_RESYNC_TIMEOUT		(1 << 15)

}OSPF_TYPE_Vlink_T;

typedef struct
{
    UI32_T proc_id;
    UI32_T vr_id;
    struct pal_in4_addr area_id;
    struct pal_in4_addr router_id;
    struct pal_in4_addr ipaddr;
    UI32_T options;
    UI32_T state;
    UI32_T event;
    UI32_T lsretransqlen;  
}OSPF_TYPE_MultiProcessVirtNbr_T;

#define OSPF_TYPE_SUMMARY_CONFIG_TYPE_CLI       1
#define OSPF_TYPE_SUMMARY_CONFIG_TYPE_SNMP      2      
#define OSPF_TYPE_SUMMARY_CONFIG_TYPE_WEB       3

typedef struct
{
    UI32_T proc_id;
    UI32_T summary_address_type;
    UI32_T summary_address;    
    UI32_T summary_pfxlen;
    UI32_T indexlen;
    UI32_T config_type;
    BOOL_T summary_status;
}OSPF_TYPE_Multi_Proc_Summary_Addr_T;

typedef struct
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T proto;
#define OSPF_TYPE_REDISTRIBUTE_DEFAULT    0
#define OSPF_TYPE_REDISTRIBUTE_RIP        1
#define OSPF_TYPE_REDISTRIBUTE_STATIC     2
#define OSPF_TYPE_REDISTRIBUTE_CONNECTED  3
#define OSPF_TYPE_REDISTRIBUTE_BGP        4
#define OSPF_TYPE_REDISTRIBUTE_MAX        5

    UI32_T flags;
    UI32_T metric;
    UI32_T metric_type;
    UI32_T tag;
    UI32_T status;
    char mapname[17];
    char listname[17];
    UI8_T  route_map[20];
    UI8_T  default_origin;
    UI8_T  origin_type;    
#define OSPF_DEFAULT_ORIGINATE_UNSPEC	0
#define OSPF_DEFAULT_ORIGINATE_NSM	1
#define OSPF_DEFAULT_ORIGINATE_ALWAYS	2

}OSPF_TYPE_Multi_Proc_Redist_T;


/* OSPF Network. */
typedef struct
{

    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T network_addr;
    UI32_T network_pfx;
    UI32_T area_id;
    UI32_T format;
    UI32_T indexlen;
}OSPF_TYPE_Network_Area_T;

/* OSPF Network. */
typedef struct
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T ifindex;
    UI32_T addr;
}OSPF_TYPE_Passive_If_T;


/* wang.tong add */
#define OSPF_IF_PARAM_VALID                  1
#define OSPF_IF_PARAM_INVALID              2

enum OSPF_CONFIGURE_TYPE_E
{
    OSPF_CONFIGURATION_TYPE_CLI_WEB = 1,
    OSPF_CONFIGURATION_TYPE_SNMP,
    OSPF_CONFIGURATION_TYPE_DYNAMIC
};

#define OSPF_TYPE_IF_AUTH_MD5__KEY_SIZE    16
typedef struct
{
    /* Interface index. */
    UI32_T ifindex;

    UI32_T tos;

    UI8_T config_type;

    UI32_T ip_address;
    UI8_T mask_len;

    /* Configured flags. */
    UI32_T config;
#define OSPF_IF_PARAMS_NETWORK_TYPE		(1 << 0)
#define OSPF_IF_PARAMS_AUTH_TYPE			(1 << 1)
#define OSPF_IF_PARAMS_PRIORITY			(1 << 2)
#define OSPF_IF_PARAMS_OUTPUT_COST		(1 << 3)
#define OSPF_IF_PARAMS_TRANSMIT_DELAY		(1 << 4)
#define OSPF_IF_PARAMS_RETRANSMIT_INTERVAL	(1 << 5)
#define OSPF_IF_PARAMS_HELLO_INTERVAL		(1 << 6)
#define OSPF_IF_PARAMS_DEAD_INTERVAL		(1 << 7)
#define OSPF_IF_PARAMS_AUTH_SIMPLE		(1 << 8)
#define OSPF_IF_PARAMS_AUTH_CRYPT		(1 << 9)
#define OSPF_IF_PARAMS_DATABASE_FILTER		(1 << 10)
#ifdef HAVE_OSPF_TE
#define OSPF_IF_PARAMS_TE_METRIC                 (1 << 11)
#endif /* HAVE_OSPF_TE */
#define OSPF_IF_PARAMS_DISABLE_ALL		(1 << 12)
#define OSPF_IF_PARAMS_MTU			(1 << 13)
#define OSPF_IF_PARAMS_MTU_IGNORE		(1 << 14)
#define OSPF_IF_PARAMS_RESYNC_TIMEOUT		(1 << 15)


    /* OSPF Network Type. */
    UI8_T network_type;	

    /* Authentication type. */
    UI8_T auth_type;

    /* Router priority. */
    UI8_T priority;

    /* MTU size. */
    UI16_T mtu;

    /* Interface output cost. */
    UI32_T output_cost;

    /* Interface transmit delay. */
    UI32_T transmit_delay;

    /* Retransmit interval. */
    UI32_T retransmit_interval;

    /* Hello interval. */
    UI32_T hello_interval;

    /* Router dead interval. */
    UI32_T dead_interval;

    /* Resync timeout. */
    UI32_T resync_timeout;

    /* Password for simple. */
    char auth_simple[OSPF_AUTH_SIMPLE_SIZE + 1];

    /* Password list for crypt. */
    char auth_crypt[256][OSPF_TYPE_IF_AUTH_MD5__KEY_SIZE + 1];
}OSPF_TYPE_IfParam_T;


#define OSPF_TYPE_DEFAULTROUTEMAP_SIZE    15
typedef struct
{
    UI32_T proc_id;
    
    UI32_T routerId_type;
#define OSPF_ROUTER_ID_TYPE_AUTO      1
#define OSPF_ROUTER_ID_TYPE_MANUAL    2

    UI32_T rfc1583CompatibleState;
#define OSPF_RFC1583_COMPATIBLE_STATE_ENABLED        1
#define OSPF_RFC1583_COMPATIBLE_STATE_DISABLED       2

    UI32_T autoCost;

    UI32_T originateDefaultRoute;
#define OSPF_ORIGINATE_DEFAULT_ROUTE_ENABLED       1
#define OSPF_ORIGINATE_DEFAULT_ROUTE_DISABLED      2

    UI32_T advertiseDefaultRoute;
#define OSPF_ADVERTISE_DEFAULT_ROUTE_ALWAYS          1
#define OSPF_ADVERTISE_DEFAULT_ROUTE_NOT_ALWAYS      2

    UI32_T externalMetricType;
#define OSPF_EXTERNAL_METRIC_TYPE_1     1
#define OSPF_EXTERNAL_METRIC_TYPE_2     2

    UI32_T defaultExternalMetric;
    UI32_T spfHoldTime;
    UI32_T areaNumber;
    UI32_T areaLimit;
    UI32_T capabilityOpaque;
#define OSPF_CAPABILITY_OPAQUE_LSA_ENABLED    1
#define OSPF_CAPABILITY_OPAQUE_LSA_DISABLED   2

    UI32_T overflowDatabaseNumber;
    UI32_T overflowDatabaseType;
    UI32_T overflowExternalDBMaxsize;
    UI32_T overflowExternalDBWaittime;
    UI32_T systemStatus;
    struct pal_in4_addr routerId;
    UI32_T adminStat;
    UI32_T versionNumber;
    UI32_T areaBdrRtrStatus;
    UI32_T asbdrRtrStatus;
    UI32_T externLsaCount;
    UI32_T externLsaCksumSum;
    UI32_T originateNewLsas;
    UI32_T rxNewLsas;
    UI32_T restartSupport;
    UI32_T restartInterval;
    UI32_T restartStatus;
    UI32_T asLsaCount;
    UI32_T spfDelayTime;
    UI8_T   defaultRouteMap[OSPF_TYPE_DEFAULTROUTEMAP_SIZE + 1];
    UI32_T defaultMetric;
}OSPF_TYPE_MultiProcessSystem_T;

typedef struct
{
    UI32_T proc_id;
    struct pal_in4_addr NbrIpAddr;
    struct pal_in4_addr router_id;
    UI32_T NbrOptions;
    UI32_T NbrPriority;
    UI32_T NbrState;
    UI32_T NbrEvent;
    UI32_T NbrLsRetransQlen;
    UI32_T NbmaNbrStatus;
}OSPF_TYPE_MultiProcessNbr_T;

#define OSPF_TYPE_LSDB_ADVERTISE_SIZE  255
typedef struct
{
    UI32_T proc_id;
    struct pal_in4_addr LsdbArea_id;
    int LsdbType;
    struct pal_in4_addr LsdbLsid;
    struct pal_in4_addr LsdbRouter_id;
    UI32_T LsdbSeqence;
    UI32_T LsdbAge;
    UI32_T LsdbChecksum;
    UI8_T   LsdbAdvertise[OSPF_TYPE_LSDB_ADVERTISE_SIZE + 1];
    UI32_T LsdbAdvertise_size;
} OSPF_TYPE_MultiProcessLsdb_T;

#define OSPF_TYPE_EXT_LSDB_ADVERTISE_SIZE  36
typedef struct
{
    UI32_T proc_id;
    int ExtLsdbType;
    struct pal_in4_addr ExtLsdbLsid;
    struct pal_in4_addr ExtLsdbRouter_id;
    UI32_T ExtLsdbSeqence;
    UI32_T ExtLsdbAge;
    UI32_T ExtLsdbChecksum;
    UI8_T   ExtLsdbAdvertise[OSPF_TYPE_EXT_LSDB_ADVERTISE_SIZE + 1];
    UI32_T ExtLsdbAdvertise_size;
}OSPF_TYPE_MultiProcessExtLsdb_T;


typedef struct
{
    int address_type;
    UI32_T network_type;
    struct pal_in4_addr addr;
    UI32_T key_id;
    UI8_T key[OSPF_TYPE_IF_AUTH_MD5__KEY_SIZE + 1];
}OSPF_TYPE_MultiProcessIfAuthMd5_T;

/* end. wang.tong*/

typedef struct
{
    UI32_T proc_id;
    L_INET_AddrIp_T route_dest;

    L_INET_AddrIp_T nexthop;
    /* Nexthop interface name. */
    char ifname[20];    
    UI32_T path_cost;
    UI32_T path_code_type; /* Path type code.  OSPF_TYPE_PATH_CODE_XXX */
    UI32_T area_id;
    UI32_T istransit;
} OspfMultiProcessRouteNexthopEntry_T;

typedef struct
{
    UI32_T vr_id;
    UI32_T proc_id;
    UI32_T ip_address;
    UI32_T tos;
    UI32_T metric;
    UI32_T status;
    UI32_T area_id;
} OSPF_TYPE_HostEntry_T;

#endif    /* End of OSPF_TYPE_H */


