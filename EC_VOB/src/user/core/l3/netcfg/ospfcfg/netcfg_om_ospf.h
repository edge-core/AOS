/*-----------------------------------------------------------------------------
 * FILE NAME: NETCFG_OM_OSPF.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2008/11/27     --- Lin.Li, Create
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */
#ifndef NETCFG_OM_OSPF_H
#define NETCFG_OM_OSPF_H

#include <sys_type.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "netcfg_type.h"
#include "netcfg_mgr_ospf.h"
#include "l_ls_prefix.h"
#include "l_ls_table.h"
#include "l_linklist.h"

/* Table definition. */
#define NETCFG_OM_OSPF_GLOBAL_IF_TABLE_DEPTH		 8

/* Table depth. */
#define NETCFG_OM_OSPF_AREA_TABLE_DEPTH                  4
#define NETCFG_OM_OSPF_STUB_AREA_TABLE_DEPTH             5
#define NETCFG_OM_OSPF_LSDB_TABLE_DEPTH                 13
#define NETCFG_OM_OSPF_AREA_RANGE_TABLE_DEPTH            8
#define NETCFG_OM_OSPF_HOST_TABLE_DEPTH                  5
#define NETCFG_OM_OSPF_IF_TABLE_DEPTH                    8
#define NETCFG_OM_OSPF_IF_METRIC_TABLE_DEPTH             9
#define NETCFG_OM_OSPF_VIRT_IF_TABLE_DEPTH               8
#define NETCFG_OM_OSPF_NBR_TABLE_DEPTH                   8
#define NETCFG_OM_OSPF_VIRT_NBR_TABLE_DEPTH              8
#define NETCFG_OM_OSPF_EXT_LSDB_TABLE_DEPTH              9
#define NETCFG_OM_OSPF_AREA_AGGREGATE_TABLE_DEPTH       13
#define NETCFG_OM_OSPF_LSDB_UPPER_TABLE_DEPTH	       5
#define NETCFG_OM_OSPF_LSDB_LOWER_TABLE_DEPTH	       8
#define NETCFG_OM_OSPF_AREA_AGGREGATE_UPPER_TABLE_DEPTH  5
#define NETCFG_OM_OSPF_AREA_AGGREGATE_LOWER_TABLE_DEPTH  8
#define NETCFG_OM_OSPF_NEXTHOP_TABLE_DEPTH	8

/* Default bandwidth.  */
#define NETCFG_OM_OSPF_DEFAULT_BANDWIDTH		     10000	/* Kbps. */
#define NETCFG_OM_OSPF_DEFAULT_REF_BANDWIDTH	    100000	/* Kbps. */

#define NETCFG_OM_OSPF_LSA_MAXAGE_INTERVAL_DEFAULT        10
#define NETCFG_OM_OSPF_LSA_REFRESH_INTERVAL_DEFAULT	10
#define NETCFG_OM_OSPF_EXTERNAL_LSA_ORIGINATE_DELAY	 1
#define NETCFG_OM_OSPF_LSA_REFRESH_EVENT_INTERVAL	    500000

#define NETCFG_OM_OSPF_AREA_LIMIT_DEFAULT                 (~0 - 1)
#define NETCFG_OM_OSPF_MAX_CONCURRENT_DD_DEFAULT	5

#define NETCFG_OM_OSPF_ROUTE_MAX    10

#define NETCFG_OM_OSPF_AREA_AGGREGATE_UPPER_TABLE_DEPTH  5
#define NETCFG_OM_OSPF_AREA_AGGREGATE_LOWER_TABLE_DEPTH  8


#define NETCFG_OM_OSPF_GLOBAL_IF_TABLE_DEPTH		 8

#define NETCFG_OM_OSPF_GENERAL_GROUP		  1
#define NETCFG_OM_OSPF_AREA_TABLE			  2
#define NETCFG_OM_OSPF_STUB_AREA_TABLE		  3
#define NETCFG_OM_OSPF_LSDB_TABLE			  4
#define NETCFG_OM_OSPF_AREA_RANGE_TABLE		  5
#define NETCFG_OM_OSPF_HOST_TABLE			  6
#define NETCFG_OM_OSPF_IF_TABLE			  7
#define NETCFG_OM_OSPF_IF_METRIC_TABLE		  8
#define NETCFG_OM_OSPF_VIRT_IF_TABLE		  9
#define NETCFG_OM_OSPF_NBR_TABLE		         10
#define NETCFG_OM_OSPF_VIRT_NBR_TABLE	         11
#define NETCFG_OM_OSPF_EXT_LSDB_TABLE	         12
#define NETCFG_OM_OSPF_ROUTE_GROUP	         13
#define NETCFG_OM_OSPF_AREA_AGGREGATE_TABLE        14

#define NETCFG_OM_OSPF_NBR_STATIC_TABLE		 16
#define NETCFG_OM_OSPF_LSDB_UPPER_TABLE	         17
#define NETCFG_OM_OSPF_LSDB_LOWER_TABLE	         18
#define NETCFG_OM_OSPF_AREA_AGGREGATE_LOWER_TABLE  19

#define NETCFG_OM_OSPF_API_INDEX_VARS_MAX		  8

/* OSPF area type.  */
#define NETCFG_OM_OSPF_AREA_DEFAULT			0
#define NETCFG_OM_OSPF_AREA_STUB				1
#define NETCFG_OM_OSPF_AREA_NSSA				2
#define NETCFG_OM_OSPF_AREA_TYPE_MAX			3


/* Interface Parameter default values. */
#define NETCFG_OM_OSPF_OUTPUT_COST_DEFAULT		1
#define NETCFG_OM_OSPF_OUTPUT_COST_MIN			1
#define NETCFG_OM_OSPF_OUTPUT_COST_MAX			65535
#define NETCFG_OM_OSPF_STUB_DEFAULT_COST_DEFAULT		1
#define NETCFG_OM_OSPF_STUB_DEFAULT_COST_MIN		0
#define NETCFG_OM_OSPF_STUB_DEFAULT_COST_MAX		16777215
#define NETCFG_OM_OSPF_HELLO_INTERVAL_DEFAULT		10
#define NETCFG_OM_OSPF_HELLO_INTERVAL_NBMA_DEFAULT	30
#define NETCFG_OM_OSPF_HELLO_INTERVAL_MIN			1
#define NETCFG_OM_OSPF_HELLO_INTERVAL_MAX			65535
#define NETCFG_OM_OSPF_HELLO_INTERVAL_JITTER		0.1
#define NETCFG_OM_OSPF_ROUTER_DEAD_INTERVAL_DEFAULT	40
#define NETCFG_OM_OSPF_ROUTER_DEAD_INTERVAL_NBMA_DEFAULT	120
#define NETCFG_OM_OSPF_ROUTER_DEAD_INTERVAL_MIN		1
#define NETCFG_OM_OSPF_ROUTER_DEAD_INTERVAL_MAX		65535
#define NETCFG_OM_OSPF_ROUTER_PRIORITY_DEFAULT		1
#define NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_DEFAULT	5
#define NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_MIN		1
#define NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_MAX		65535
#define NETCFG_OM_OSPF_RETRANSMIT_INTERVAL_JITTER		0.2
#define NETCFG_OM_OSPF_TRANSMIT_DELAY_DEFAULT		1
#define NETCFG_OM_OSPF_POLL_INTERVAL_DEFAULT		120
#define NETCFG_OM_OSPF_POLL_INTERVAL_MIN			1
#define NETCFG_OM_OSPF_POLL_INTERVAL_MAX			65535
#define NETCFG_OM_OSPF_NEIGHBOR_PRIORITY_DEFAULT		0

#define NETCFG_OM_OSPF_AUTH_SIMPLE_SIZE           8
#define NETCFG_OM_OSPF_AUTH_MD5_SIZE             16

/* OSPF Network Type in struct ospf_interface. */
#define NETCFG_OM_OSPF_IFTYPE_NONE			0
#define NETCFG_OM_OSPF_IFTYPE_POINTOPOINT			1
#define NETCFG_OM_OSPF_IFTYPE_BROADCAST			2
#define NETCFG_OM_OSPF_IFTYPE_NBMA			3
#define NETCFG_OM_OSPF_IFTYPE_POINTOMULTIPOINT		4
#define NETCFG_OM_OSPF_IFTYPE_POINTOMULTIPOINT_NBMA	5
#define NETCFG_OM_OSPF_IFTYPE_VIRTUALLINK			6
#define NETCFG_OM_OSPF_IFTYPE_LOOPBACK            	7
#define NETCFG_OM_OSPF_IFTYPE_MAX				8


#define INTERFACE_NAMSIZ 20

#define IPV4_ADDRESS_UNSPEC             0x00000000      /* 0.0.0.0. */
#define IPV4_ADDRESS_LOOPBACK           0x7F000001	/* 127.0.0.1. */

#define NETCFG_OM_ROUTE_DEFAULT                0
#define NETCFG_OM_ROUTE_KERNEL                 1
#define NETCFG_OM_ROUTE_CONNECT                2
#define NETCFG_OM_ROUTE_STATIC                 3
#define NETCFG_OM_ROUTE_RIP                    4
#define NETCFG_OM_ROUTE_RIPNG                  5
#define NETCFG_OM_ROUTE_OSPF                   6
#define NETCFG_OM_ROUTE_OSPF6                  7
#define NETCFG_OM_ROUTE_BGP                    8
#define NETCFG_OM_ROUTE_ISIS                   9
#define NETCFG_OM_ROUTE_MAX                   10

#define OSPF_DEFAULT_BANDWIDTH		     10000	/* Kbps. */
#define OSPF_DEFAULT_REF_BANDWIDTH	    100000	/* Kbps. */

#define EXTERNAL_METRIC_TYPE_UNSPEC		 0
#define EXTERNAL_METRIC_TYPE_1			 1
#define EXTERNAL_METRIC_TYPE_2			 2
#define EXTERNAL_METRIC_TYPE_DEFAULT		 2

#define EXTERNAL_METRIC_VALUE_UNSPEC		0xFFFFFF

#define NETCFG_OM_OSPF_API_INDEX_VARS_MAX		  8
struct L_ls_table_index
{
  unsigned int len;
  unsigned int octets;
  char vars[NETCFG_OM_OSPF_API_INDEX_VARS_MAX];
};

/*OSPF master structure*/
typedef struct NETCFG_OM_OSPF_Master_S
{
  /* VR ID. */
  UI32_T vr_id;

  /* OSPF instance list. */
  struct L_list *ospf;

  /* Virtual interface index. */
  int vlink_index;

  /* OSPF global interface table. */
  struct L_ls_table *if_table;

  /* OSPF interface parameter pool. */
  struct L_list *if_params;
}NETCFG_OM_OSPF_Master_T;

/* OSPF VRF. */
typedef struct NETCFG_OM_OSPF_Vrf_S
{
  /* VRF ID. */
  UI32_T vrf_id;

  /* OSPF instance list. */
  struct L_list *ospf;

  /* Redistribute info. */
  struct L_ls_table *redist_table;

  /* Redistribute count. */
  UI32_T redist_count[NETCFG_OM_OSPF_ROUTE_MAX];
}NETCFG_OM_OSPF_Vrf_T;

/* Configuration redistribute. */
typedef struct NETCFG_OM_OSPF_REDIST_CONF_S
{
  /* Flags. */
  u_int32_t flags;
#define OSPF_REDIST_ENABLE		    (1 << 0)
#define OSPF_REDIST_METRIC_TYPE		(1 << 1)
#define OSPF_REDIST_METRIC		    (1 << 2)
#define OSPF_REDIST_ROUTE_MAP		(1 << 3)
#define OSPF_REDIST_TAG			    (1 << 4)
#define OSPF_REDIST_FILTER_LIST		(1 << 5)
#define OSPF_DIST_LIST_IN           (1 << 6)

  /* Metric value (24-bit). */
  u_int32_t metric;

  /* Route tag. */
  u_int32_t tag;

  /* Route-map. */
  struct route_map
  {
    char *name;
    struct route_map *map;

  } route_map;

  /* Distribute-list. */
  struct
  {
    char *name;
    struct access_list *list;

  } distribute_list;
}NETCFG_OM_OSPF_REDIST_CONF_T;

/* OSPF instance structure. */
typedef struct NETCFG_OM_OSPF_Instance_S
{
  /* OSPF process ID. */
  UI32_T ospf_id;

  /* VR ID. */
  UI32_T vr_id;

  /*VRF ID*/
  UI32_T vrf_id;
  
  /* OSPF start time. */
  /*pal_time_t start_time;*/

  /* OSPF Router ID. */
  struct pal_in4_addr router_id_config;		/* Router-ID configured. */

  /* Administrative flags. */
  UI16_T flags;
#define NETCFG_OM_OSPF_PROC_UP			(1 << 0)
#define NETCFG_OM_OSPF_PROC_DESTROY		(1 << 1)
#define NETCFG_OM_OSPF_ROUTER_ABR			(1 << 2)
#define NETCFG_OM_OSPF_ROUTER_ASBR		(1 << 3)
#define NETCFG_OM_OSPF_ROUTER_DB_OVERFLOW		(1 << 4)
#define NETCFG_OM_OSPF_ROUTER_RESTART		(1 << 5)
#define NETCFG_OM_OSPF_LSDB_EXCEED_OVERFLOW_LIMIT	(1 << 6)
#define NETCFG_OM_OSPF_ASE_CALC_SUSPENDED		(1 << 7)
#define NETCFG_OM_OSPF_GRACE_LSA_ACK_RECD		(1 << 8)
#define NETCFG_OM_OSPF_ROUTER_RESTART_UPDATING    (1 << 9) /* djd: process is up again when restart */

  /* Configuration variables. */
  UI16_T config;
#define NETCFG_OM_OSPF_CONFIG_ROUTER_ID		(1 << 0)
#define NETCFG_OM_OSPF_CONFIG_DEFAULT_METRIC	(1 << 1)
#define NETCFG_OM_OSPF_CONFIG_MAX_CONCURRENT_DD	(1 << 2)
#define NETCFG_OM_OSPF_CONFIG_RFC1583_COMPATIBLE	(1 << 3)
#define NETCFG_OM_OSPF_CONFIG_OPAQUE_LSA		(1 << 4)
#define NETCFG_OM_OSPF_CONFIG_TRAFFIC_ENGINEERING	(1 << 5)
#define NETCFG_OM_OSPF_CONFIG_RESTART_METHOD	(1 << 6)
#define NETCFG_OM_OSPF_CONFIG_OVERFLOW_LSDB_LIMIT	(1 << 7)
#define NETCFG_OM_OSPF_CONFIG_ROUTER_ID_USE	(1 << 8)

  /* ABR type. */
  UI8_T abr_type;

  /* Default information originate. */
  UI8_T default_origin;
#define NETCFG_OM_OSPF_DEFAULT_ORIGINATE_UNSPEC	0
#define NETCFG_OM_OSPF_DEFAULT_ORIGINATE_NSM	1
#define NETCFG_OM_OSPF_DEFAULT_ORIGINATE_ALWAYS	2

  /* SPF timer config. */
  NETCFG_TYPE_OSPF_Timer_T timer;/*Lin.Li, used in netcfg*/

  /* Reference bandwidth (Kbps).  */
  UI32_T ref_bandwidth;

  /* Max concurrent DD. */
  UI16_T max_dd;			/* Maximum concurrent DD.  */
  #define NETCFG_OM_OSPF_MAX_CONCURRENT_DD_DEFAULT	5

  /* Configuration tables. */
  struct L_ls_table *networks;		/* Network area tables. */
  struct L_ls_table *summary;		/* Address range for external-LSAs. */
  struct L_ls_table *nbr_static;		/* Static neighbor for NBMA. */
  struct L_list *passive_if;	        /* Pasive interfaces. */

  /* Redistriribute configuration. */
  NETCFG_OM_OSPF_REDIST_CONF_T redist[NETCFG_OM_OSPF_ROUTE_MAX];

  /* Redistribute default metric. */
  UI32_T default_metric;

  /* Redistribute timer argument. */
  int redist_update;

  /* OSPF specific object tables. */
  struct L_ls_table *area_table;		/* Area table. */
  struct L_ls_table *nexthop_table;	/* Nexthop table. */
  struct L_ls_table *redist_table;	/* Redistribute map table. */
  struct L_ls_table *vlink_table;		/* Virtual interface table. */

  /* Pointer to the Backbone Area. */
  NETCFG_TYPE_OSPF_Area_T *backbone;/*Lin.Li, used in netcfg*/

  /* LSDB of AS-external-LSAs. */
  /*struct ospf_lsdb *lsdb;*/

  /* Limit of number of LSAs */
  UI32_T lsdb_overflow_limit;	
  int lsdb_overflow_limit_type;		/* Soft or hard limit. */
#define NETCFG_OM_OSPF_LSDB_OVERFLOW_LIMIT_OSCILATE_RANGE		0.95
#define NETCFG_OM_OSPF_LSDB_OVERFLOW_SOFT_LIMIT	1  
#define NETCFG_OM_OSPF_LSDB_OVERFLOW_HARD_LIMIT	2
#define NETCFG_OM_OSPF_LSDB_OVERFLOW_LIMIT_DEFAULT	AMS_SYS_DFLT_OSPF_LSDB_OVERFLOW_LIMIT  
  
#define NETCFG_OM_OSPF_AREA_LIMIT_DEFAULT                 (~0 - 1)
  UI32_T max_area_limit;

  /* Routing tables. */
  struct L_ls_table *rt_asbr;		/* ASBR routing table. */
  struct L_ls_table *rt_network;		/* IP routing table. */

  /* Interface list of queue. */
  struct L_list *oi_write_q;
  
  /* Distance parameter. */
  struct L_ls_table *distance_table;
  UI8_T distance_all;
  UI8_T distance_intra;
  UI8_T distance_inter;
  UI8_T distance_external;

  /* Time stamp. */
  /*struct pal_timeval tv_redist;*/

  /* OSPF hello timer jitter seed.  */
  UI32_T jitter_seed;

  /* Concurrent DD handling.  */
  /*struct ospf_neighbor *dd_nbr_head;*/
  //struct ospf_neighbor *dd_nbr_tail;
  UI16_T dd_count_in;		/* Incomming DD neighbors.  */
  UI16_T dd_count_out;		/* Outgoing DD neighbors.  */

  UI32_T nssa_count;			/* NSSA attached. */

  /* Statistics. */
  UI32_T lsa_originate_count;	/* LSA origination. */
  UI32_T rx_lsa_count;		/* LSA used for new instantiation. */

  /* API related tables. */
  struct L_ls_table *stub_table;		/* OSPF stub area table. */
  struct L_ls_table *lsdb_table;		/* OSPF LSDB upper table. */
  struct L_ls_table *host_table;		/* OSPF Host table. */
  struct L_ls_table *area_range_table;	/* OSPF Area Range table. */
  struct L_ls_table *nbr_table;		/* OSPF Neighbor table. */
}NETCFG_OM_OSPF_Instance_T;

/* OSPF identity for DR Election. */
typedef struct NETCFG_OM_OSPF_Identity_S
{
  /* IPv4 prefix for the router. */
  struct prefix address;

  /* Router ID. */
  struct pal_in4_addr router_id;

  /* Router ID for DR. */
  struct pal_in4_addr d_router;

  /* Router ID for Backup. */
  struct pal_in4_addr bd_router;

  /* Router Priority. */
  UI8_T priority;
}NETCFG_OM_OSPF_Identity_T;

typedef struct NETCFG_OM_OSPF_IfParams_S
{
  /* Pointer to ospf_if_param_table's desc. */
  char *desc;

  /* Interface index. */
  UI32_T ifindex;
  
  /* VR ID. */
  UI32_T vr_id;

   /* OSPF process ID. */
  UI32_T ospf_id;
     
  /* Destination prefix for PPP and virtual link. */
  struct prefix destination;

  
   /* OSPF identity for DR election. */
  NETCFG_OM_OSPF_Identity_T ident;
  /* Configured flags. */
  u_int32_t config;
#define NETCFG_OM_OSPF_IF_PARAM_NETWORK_TYPE		(1 << 0)
#define NETCFG_OM_OSPF_IF_PARAM_AUTH_TYPE			(1 << 1)
#define NETCFG_OM_OSPF_IF_PARAM_PRIORITY			(1 << 2)
#define NETCFG_OM_OSPF_IF_PARAM_OUTPUT_COST		(1 << 3)
#define NETCFG_OM_OSPF_IF_PARAM_TRANSMIT_DELAY		(1 << 4)
#define NETCFG_OM_OSPF_IF_PARAM_RETRANSMIT_INTERVAL	(1 << 5)
#define NETCFG_OM_OSPF_IF_PARAM_HELLO_INTERVAL		(1 << 6)
#define NETCFG_OM_OSPF_IF_PARAM_DEAD_INTERVAL		(1 << 7)
#define NETCFG_OM_OSPF_IF_PARAM_AUTH_SIMPLE		(1 << 8)
#define NETCFG_OM_OSPF_IF_PARAM_AUTH_CRYPT		(1 << 9)
#define NETCFG_OM_OSPF_IF_PARAM_DATABASE_FILTER		(1 << 10)
#ifdef HAVE_OSPF_TE
#define NETCFG_OM_OSPF_IF_PARAM_TE_METRIC                 (1 << 11)
#endif /* HAVE_OSPF_TE */
#define NETCFG_OM_OSPF_IF_PARAM_DISABLE_ALL		(1 << 12)
#define NETCFG_OM_OSPF_IF_PARAM_MTU			(1 << 13)
#define NETCFG_OM_OSPF_IF_PARAM_MTU_IGNORE		(1 << 14)
#define NETCFG_OM_OSPF_IF_PARAM_RESYNC_TIMEOUT		(1 << 15)

  /* Number of interfaces which have the same network addresses. */
  int clone;

  /* Interface type. */
  UI8_T type;

  /* OSPF Network Type. */
  UI8_T network_type;	
  
  /* Authentication type. */
  UI8_T auth_type;

  /* Router priority. */
  UI8_T priority;

  /* MTU size. */
  u_int16_t mtu;

  /* Interface output cost. */
  u_int32_t output_cost;

#ifdef HAVE_OSPF_TE
  /* TE metric. */
  u_int32_t te_metric;
#endif /* HAVE_OSPF_TE */

  /* Interface transmit delay. */
  u_int32_t transmit_delay;

  /* Retransmit interval. */
  u_int32_t retransmit_interval;

  /* Hello interval. */
  u_int32_t hello_interval;

  /* Router dead interval. */
  u_int32_t dead_interval;

  /* Resync timeout. */
  u_int32_t resync_timeout;

  /* Password for simple. */
  char auth_simple[NETCFG_TYPE_OSPF_AUTH_SIMPLE_SIZE + 1];

  /* Password list for crypt. */
  struct L_list *auth_crypt;

  /* Neighbor List. */
  struct L_ls_table *nbrs;
}NETCFG_OM_OSPF_IfParams_T;

typedef struct NETCFG_OM_OSPF_CryptKey_S
{
  UI8_T flags;
#define NETCFG_OM_OSPF_AUTH_MD5_KEY_PASSIVE   (1 << 0)
  
  UI8_T key_id;
  char auth_key[NETCFG_TYPE_OSPF_AUTH_MD5_SIZE + 1];
}NETCFG_OM_OSPF_CryptKey_T;
typedef struct NETCFG_OM_OSPF_SummaryAddress_S
{
  /* OSPF instance. */
  NETCFG_OM_OSPF_Instance_T *top;

  /* Summary address prefix. */
  struct ls_prefix *lp;
}NETCFG_OM_OSPF_SummaryAddress_T;


/* FUNCTION NAME : NETCFG_OM_OSPF_Init
 * PURPOSE:Init NETCFG_OM_OSPF database, create semaphore
 *
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
void NETCFG_OM_OSPF_Init(void);

/* FUNCTION NAME : NETCFG_OM_OSPF_DeleteAllOspfMasterEntry
 * PURPOSE:
 *          Remove all OSPF master entries.
 *
 * INPUT:  None.
 *
 * OUTPUT: None.
 *
 * RETURN: None.
 *
 * NOTES:  None.
 */
void NETCFG_OM_OSPF_DeleteAllOspfMasterEntry(void);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceEntry
 * PURPOSE:
 *      Get a ospf Instance entry with specific vr_id and proc_id.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: 
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetInstanceEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_Instance_T *top);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetOspfInterfaceEntry
 * PURPOSE:
 *      Get a OSPF interface.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *      oip
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetOspfInterfaceEntry(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_IfParams_T *oip);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetCryptKeyEntry
 * PURPOSE:
 *      Get crypt key entry .
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_GetCryptKeyEntry(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr, NETCFG_OM_OSPF_CryptKey_T *ck);
/* FUNCTION NAME : NETCFG_OM_OSPF_GetRunningIfEntryByIfindex
* PURPOSE:
*     Get ospf interface config information by ifindex.
*
* INPUT:
*      vr_id,
*      entry
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetRunningIfEntryByIfindex(UI32_T vr_id, NETCFG_TYPE_OSPF_IfConfig_T *entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_CheckCryptKeyExist
 * PURPOSE:
 *      Check if crypt key exist .
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *     
 */
UI32_T NETCFG_OM_OSPF_CheckCryptKeyExist(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag,
                                                    struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_InstanceAdd
 * PURPOSE:
 *      Add a OSPF instance entry when router ospf enable .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_InstanceAdd(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_InstanceDelete
 * PURPOSE:
 *      Delete a OSPF instance entry when router ospf disable .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_InstanceDelete(UI32_T vr_id, UI32_T vrf_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationTypeSet
 * PURPOSE:
 *     Set OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      type
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationTypeSet(UI32_T vr_id, UI32_T ifindex, UI8_T type, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationTypeUnset
 * PURPOSE:
 *     Unset OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationTypeUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationKeySet
 * PURPOSE:
 *      Set OSPF interface authentication key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      auth_key
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationKeySet(UI32_T vr_id, UI32_T ifindex, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfAuthenticationKeyUnset
 * PURPOSE:
 *     Unset OSPF interface authentication type.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfAuthenticationKeyUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfMessageDigestKeySet
 * PURPOSE:
 *      Set OSPF interface message digest key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      key_id
 *      auth_key
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfMessageDigestKeySet(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, char *auth_key, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfMessageDigestKeyUnset
 * PURPOSE:
 *      Unset OSPF interface message digest key.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      key_id
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfMessageDigestKeyUnset(UI32_T vr_id, UI32_T ifindex, UI8_T key_id, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfPrioritySet
 * PURPOSE:
 *      Set OSPF interface priority.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      priority
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfPrioritySet(UI32_T vr_id, UI32_T ifindex, UI8_T priority, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfPriorityUnset
 * PURPOSE:
 *      Unset OSPF interface priority.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfPriorityUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfCostSet
 * PURPOSE:
 *      Set OSPF interface cost.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      cost
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfCostSet(UI32_T vr_id, UI32_T ifindex, UI32_T cost, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_IfCostUnset
 * PURPOSE:
 *      Unset OSPF interface cost.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfCostUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfDeadIntervalSet
 * PURPOSE:
 *      Set OSPF interface dead interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr

 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfDeadIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfDeadIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface dead interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfDeadIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfHelloIntervalSet
 * PURPOSE:
 *      Set OSPF interface hello interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfHelloIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfHelloIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface hello interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfHelloIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfRetransmitIntervalSet
 * PURPOSE:
 *      Set OSPF interface retransmit interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      interval
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfRetransmitIntervalSet(UI32_T vr_id, UI32_T ifindex, UI32_T interval, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfRetransmitIntervalUnset
 * PURPOSE:
 *      Unset OSPF interface retransmit interval.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfRetransmitIntervalUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfTransmitDelaySet
 * PURPOSE:
 *      Set OSPF interface transmit delay.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      delay
 *      addr_ flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfTransmitDelaySet(UI32_T vr_id, UI32_T ifindex, UI32_T delay, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME :  	NETCFG_OM_OSPF_IfTransmitDelayUnset
 * PURPOSE:
 *      Unset OSPF interface transmit delay.
 *
 * INPUT:
 *      vr_id,
 *      ifindex 
 *      addr_flag
 *      addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_IfTransmitDelayUnset(UI32_T vr_id, UI32_T ifindex, BOOL_T addr_flag, struct pal_in4_addr addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetRouterId
 * PURPOSE:
 *      Get ospf configured Router ID.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      config_router_id.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *     1. if return NETCFG_TYPE_OK, it means you can get configured_router_id
 *     2. if return NETCFG_TYPE_CAN_NOT_GET, it means user not configured router id.
 *
 */
UI32_T NETCFG_OM_OSPF_GetRouterId(UI32_T vr_id, UI32_T proc_id, struct pal_in4_addr *config_router_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_RouterIdSet
 * PURPOSE:
 *      Set ospf Router ID.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      router_id.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
 *     
 *     
 */
UI32_T NETCFG_OM_OSPF_RouterIdSet(UI32_T vr_id, UI32_T proc_id, UI32_T router_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_RouterIdUnset
 * PURPOSE:
 *      Set ospf Router ID to default.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *     
 *     
 */
UI32_T NETCFG_OM_OSPF_RouterIdUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetTimer
 * PURPOSE:
 *      Get ospf timer value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      timer.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_GetTimer(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Timer_T *timer);

/* FUNCTION NAME : NETCFG_OM_OSPF_TimerSet
 * PURPOSE:
 *      Set ospf timer value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      timer
 * OUTPUT:
 *      
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_TimerSet(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Timer_T timer);

/* FUNCTION NAME : NETCFG_OM_OSPF_TimerUnset
 * PURPOSE:
 *      Set ospf timer value to default value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *      
 * OUTPUT:
 *      
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
 *
 * NOTES:
*
*
*/
UI32_T NETCFG_OM_OSPF_TimerUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultMetric
 * PURPOSE:
 *      Get ospf default metric value.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      flag,
 *      metric.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetDefaultMetric(UI32_T vr_id, UI32_T proc_id, BOOL_T *flag, UI32_T *metric);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultMetricSet
* PURPOSE:
*     Set ospf default metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      metric.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultMetricUnset
* PURPOSE:
*     Set ospf default metric to default value.
*
* INPUT:
*      vr_id,
*      proc_id.
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultMetricUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetCompatibleRfc1583Status
 * PURPOSE:
 *      Get ospf compatible rfc1583 status.
 *
 * INPUT:
 *      vr_id
 *      proc_id
 *
 * OUTPUT:
 *      status.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetCompatibleRfc1583Status(UI32_T vr_id, UI32_T proc_id, BOOL_T *status);

/* FUNCTION NAME : NETCFG_OM_OSPF_CompatibleRfc1583Set
* PURPOSE:
*     Set ospf  compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_CompatibleRfc1583Set(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_CompatibleRfc1583Unset
* PURPOSE:
*     Unset ospf  compatible rfc1583.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_CompatibleRfc1583Unset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetPassiveIf
 * PURPOSE:
 *      Get passive interface status.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry->ifname,
 *      entry->addr
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextPassiveIf
 * PURPOSE:
 *      Get next passive interface .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextPassiveIf(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Passive_If_T *entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_PassiveIfSet
* PURPOSE:
*     Set ospf  passive interface.
*
* INPUT:
*      vr_id
*      proc_id,
*      ifindex,
*      addr
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_PassiveIfSet(UI32_T vr_id, UI32_T proc_id, UI32_T ifindex, UI32_T addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_PassiveIfUnset
* PURPOSE:
*     Unset ospf  passive interface.
*
* INPUT:
*      vr_id
*      proc_id,
*      ifindex,
*      addr
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_SET
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_PassiveIfUnset(UI32_T vr_id, UI32_T proc_id, UI32_T ifindex, UI32_T addr);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNetworkEntry
 * PURPOSE:
 *      Get network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNetworkEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextNetworkEntry
 * PURPOSE:
 *      Get next network .
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      entry.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextNetworkEntry(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T *entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_NetworkSet
 * PURPOSE:
 *      Set network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_NetworkSet(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T entry);

/* FUNCTION NAME : NETCFG_OM_OSPF_NetworkUnset
 * PURPOSE:
 *      Unset network entry.
 *
 * INPUT:
 *      vr_id
 *      proc_id,
 *      entry
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_NetworkUnset(UI32_T vr_id, UI32_T proc_id, NETCFG_TYPE_OSPF_Network_T entry);


/* FUNCTION NAME : NETCFG_OM_OSPF_GetSummaryAddr
* PURPOSE:
*      Get Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen);

/* FUNCTION NAME : NETCFG_OM_OSPF_AddSummaryAddr
* PURPOSE:
*      Add Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_AddSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen);

/* FUNCTION NAME : NETCFG_OM_OSPF_DelSummaryAddr
* PURPOSE:
*      Delete Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_DelSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T addr, UI32_T masklen);

/* FUNCTION NAME : NETCFG_OM_OSPF_SetAutoCostRefBandwidth
* PURPOSE:
*      Set auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_SetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T refbw);

/* FUNCTION NAME : NETCFG_OM_OSPF_UnsetAutoCostRefBandwidth
* PURPOSE:
*      Unset auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_UnsetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id);



/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeProtoSet
* PURPOSE:
*     Set ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeProtoSet(UI32_T vr_id, UI32_T proc_id, char *type);



/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeProtoUnset
* PURPOSE:
*     Unset ospf redistribute.
*
* INPUT:
*      vr_id,
*      proc_id,
*      type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeProtoUnset(UI32_T vr_id, UI32_T proc_id, char *type);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricTypeSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric_type);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricTypeUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);



/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T metric);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeMetricUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeTagSet
* PURPOSE:
*     Set ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      tag
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeTagSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, UI32_T tag);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeTagUnset
* PURPOSE:
*     Unset ospf redistribute tag.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeTagUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *proto_type, char *route_map);

/* FUNCTION NAME : NETCFG_OM_OSPF_RedistributeRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_RedistributeRoutemapUnset(UI32_T vr_id, UI32_T proc_id, char *proto_type);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricTypeSet
* PURPOSE:
*     Set ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric_type
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricTypeSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric_type);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricTypeUnset
* PURPOSE:
*     Unset ospf redistribute metric type.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricTypeUnset(UI32_T vr_id, UI32_T proc_id );

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricSet
* PURPOSE:
*     Set ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      metric
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricSet(UI32_T vr_id, UI32_T proc_id, UI32_T metric);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoMetricUnset
* PURPOSE:
*     Unset ospf redistribute metric.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoMetricUnset(UI32_T vr_id, UI32_T proc_id );

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoRoutemapSet
* PURPOSE:
*     Set ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      route_map
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoRoutemapSet(UI32_T vr_id, UI32_T proc_id, char *route_map);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoRoutemapUnset
* PURPOSE:
*     Unset ospf redistribute route map.
*
* INPUT:
*      vr_id,
*      proc_id,
*      proto_type,
*      
*
* OUTPUT:
*      None
*
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoRoutemapUnset( UI32_T vr_id, UI32_T proc_id );

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoAlwaysSet
* PURPOSE:
*     Set ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoAlwaysSet(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoAlwaysUnset
* PURPOSE:
*     Unset ospf default information to "always".
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoAlwaysUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoSet
* PURPOSE:
*     Set ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoSet(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_DefaultInfoUnset
* PURPOSE:
*     Unset ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_DefaultInfoUnset(UI32_T vr_id, UI32_T proc_id);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetNextSummaryAddr
* PURPOSE:
*      Getnext Summary Address.
*
* INPUT:
*      vr_id
*      proc_id,
*      entry
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetNextSummaryAddr(UI32_T vr_id, UI32_T proc_id, UI32_T *addr, UI32_T *masklen);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetAutoCostRefBandwidth
* PURPOSE:
*      Get auto cost reference bandwidth.
*
* INPUT:
*      vr_id
*      proc_id,
*      refbw
*
* OUTPUT:
*      entry.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetAutoCostRefBandwidth(UI32_T vr_id, UI32_T proc_id, UI32_T *refbw);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetRedistributeConfig
* PURPOSE:
*      Get redistribute configuration inormation.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetRedistributeConfig(UI32_T vr_id, UI32_T proc_id, char *type, NETCFG_OM_OSPF_REDIST_CONF_T *redist_config);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultInfoConfig
* PURPOSE:
*      Get default information configuration.
*
* INPUT:
*      vr_id
*      proc_id,
*      
*
* OUTPUT:
*      redist_config.
*
* RETURN: NULL/NETCFG_TYPE_CAN_NOT_GET
*
* NOTES:
*
*/
UI32_T NETCFG_OM_OSPF_GetDefaultInfoConfig(UI32_T vr_id, UI32_T proc_id, NETCFG_OM_OSPF_REDIST_CONF_T *redist_config);

/* FUNCTION NAME : NETCFG_OM_OSPF_GetDefaultInfoAlways
* PURPOSE:
*     Get ospf default information.
*
* INPUT:
*      vr_id,
*      proc_id,
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetDefaultInfoAlways(UI32_T vr_id, UI32_T proc_id, UI32_T *originate);

/* FUNCTION NAME : NETCFG_OM_OSPF_OspfInterfaceAdd
 * PURPOSE:
 *      Add default ospf -interface .
 *
 * INPUT:
 *      vr_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_DefaultOspfInterfaceAdd(UI32_T vr_id, UI32_T ifindex);

BOOL_T NETCFG_OM_OSPF_DefaultOspfInterfaceDelete(UI32_T vr_id, UI32_T ifindex);

/* FUNCTION NAME : NETCFG_OM_OSPF_OspfInterfaceAdd
 * PURPOSE:
 *      Add ospf-interface .
 *
 * INPUT:
 *      vr_id
 *      vrf_id
 *      ifindex
 *
 * OUTPUT:
 *      None.
 *
 * RETURN: TRUE/FALSE
 *
 * NOTES:
 *      
 */
BOOL_T NETCFG_OM_OSPF_OspfInterfaceAdd(UI32_T vr_id, UI32_T ifindex, UI32_T ip_addr, UI32_T ip_mask);

BOOL_T NETCFG_OM_OSPF_OspfInterfaceDelete (UI32_T vr_id, UI32_T ifindex, UI32_T ip_addr);

#if 0
/* FUNCTION NAME : NETCFG_OM_OSPF_AreaDefaultCostSet
* PURPOSE:
*     Set ospf area default cost value.
*
* INPUT:
*      vr_id,
*      proc_id,
*      area_id,
*      format,
*      cost.
*
* OUTPUT:
*      None
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_AreaDefaultCostSet(UI32_T vr_id, UI32_T proc_id, UI32_T area_id, UI32_T format, UI32_T cost);
#endif

/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceStatistics
* PURPOSE:
*     Get ospf instance some parameters.
*
* INPUT:
*      entry->vr_id,
*      entry->proc_id.
*
* OUTPUT:
*      entry
* RETURN:
*       NETCFG_TYPE_OK/NETCFG_TYPE_FAIL
*
* NOTES:
*      None.
*/
UI32_T NETCFG_OM_OSPF_GetInstancePara(NETCFG_TYPE_OSPF_Instance_Para_T *entry);


/* FUNCTION NAME : NETCFG_OM_OSPF_GetInstanceCount
 * PURPOSE:
 *     Get total number of ospf instances
 *
 * INPUT:
 *      vr_id
 *
 * OUTPUT:
 *      count  -- Total instance count
 *
 * RETURN:
 *      RETURN: NETCFG_TYPE_OK/NETCFG_TYPE_CAN_NOT_GET
 *
 * NOTES:
 *      None
 */
UI32_T NETCFG_OM_OSPF_GetInstanceCount(UI32_T vr_id, UI32_T *count);

#endif

