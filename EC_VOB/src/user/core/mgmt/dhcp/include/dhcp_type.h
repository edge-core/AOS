/* Module Name: DHCP_TYPE.H
 * Purpose:
 *      This module declares all structures used in DHCP/IAD, structure name and
 *      field name do not be changed.
 *
 * Notes:
 *      1. change struct in_add to IPA in interfaec)info, Primary interface address.
 *
 * History:
 *       Date       -- Modifier,  Reason
 *  0.1 2001.12.25  --  William, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001.
 */

#ifndef _DHCP_TYPE_H
#define _DHCP_TYPE_H
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "leaf_es3626a.h"
#include "l_mm.h"
#include "sysfun.h"
#include "sys_module.h"

/* NAMING CONSTANT DECLARATIONS
 */
typedef UI32_T  TIME;
/*#include "tree.h"*//*Timon*/

#if !defined (TIME_MAX)
# define TIME_MAX 2147483647
#endif

#define MAX_TIME 0xffffffff 
#define MIN_TIME 0
#define MAX_RELAY_SERVER SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER

#define dhcp_malloc(x) malloc(x)
#define dhcp_free(x) free(x)

/* for debug use; allocate and free memory in P2MEM */
//#define dhcp_malloc(x) DHCP_TEMP_MemoryAlloc(x)
//#define dhcp_free(x) DHCP_TEMP_MemoryFree(x)

#define PROTO2 (x)  x

#define DHCP_OPTIONS_COOKIE "\143\202\123\143"
/*  -- New Definition in Mercury --
 *  Interface configuration tag.
 *  For each interface, the combination is one of
 *      { CLIENT + RELAY | SERVER | RELAY | CLIENT }
 */
#define	DHCP_TYPE_BIND_NONE     	0 
#define DHCP_TYPE_BIND_CLIENT       BIT_1
#define DHCP_TYPE_BIND_SERVER       BIT_2
#define DHCP_TYPE_BIND_RELAY        BIT_3

/*  Interface mode : DHCP/BOOTP/USER-DEFINED    */
#define DHCP_TYPE_INTERFACE_MODE_USER_DEFINE    VAL_vlanAddressMethod_user
#define DHCP_TYPE_INTERFACE_MODE_DHCP           VAL_vlanAddressMethod_dhcp
#define DHCP_TYPE_INTERFACE_MODE_BOOTP          VAL_vlanAddressMethod_bootp

/* Restart object */
#define DHCP_TYPE_RESTART_NONE      BIT_0
#define DHCP_TYPE_RESTART_CLIENT    BIT_1
#define DHCP_TYPE_RESTART_SERVER    BIT_2
#define DHCP_TYPE_RESTART_RELAY     BIT_3

#define DHCP_TYPE_CID_BUF_MAX_SIZE  32

#define DHCP_TYPE_CLASSID_BUF_MAX_SIZE   MAXSIZE_dhcpcIfVendorClassId      /* 2007-12, Joseph */

/* DHCP Server Pool Type */
#define DHCP_TYPE_POOL_NONE     VAL_dhcpPoolPoolType_notSpecify
#define DHCP_TYPE_POOL_NETWORK  VAL_dhcpPoolPoolType_netWork
#define DHCP_TYPE_POOL_HOST     VAL_dhcpPoolPoolType_host

/* DHCP Server Hash Table type */
#define DHCP_TYPE_HASH_HOST_DECL        1
#define DHCP_TYPE_HASH_LEASE            2

/* DHCP Proto UDP   */
#define DHCP_TYPE_IP_PROT_UDP          17

/* UDP destination port */
#define DHCP_TYPE_UDP_BOOTPC_PORT      68
#define DHCP_TYPE_UDP_BOOTPS_PORT      67

/*  -- Following definition is porting from DHCP/IAD -- */
/* BOOTP (rfc951) message types */
#define BOOTREQUEST                     1
#define BOOTREPLY                       2

/* Possible values for hardware type (htype) field... */
#define HTYPE_ETHER                     1               /* Ethernet 10Mbps              */
#define HTYPE_IEEE802                   6               /* IEEE 802.2 Token Ring... */
#define HTYPE_FDDI                      8               /* FDDI...          */

#define DHCP_TYPE_UC_MAGIC_WORD         0x44484350      /* Magic Word = "DHCP" */

/*#define INADDR_BROADCAST 0xFFFFFFFF*//*Timon*/
#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
/* DHCP Option82 Relay Forward Flag */
#define DHCP_TYPE_FLOODING                  1
#define DHCP_TYPE_RELAY_BROADCAST           2
#define DHCP_TYPE_RELAY_UNICAST             3
#define DHCP_TYPE_RELAY_LAYER2_UNICAST      4
#endif



#define	DHCP_FRAME_HEADER_LEN               22
#define DHCP_UDP_PSEUDO_HEADER_LENGTH       12
#define DHCP_PACKET_LEN                     576
#define	DHCP_FRAME_FIX_LEN                  236
#define	DHCP_COOKIE_LEN                     4


#if ((SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE) || (SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE))
#define	DHCP_OPTION82_LENGTH_MAC_MODE       18
#define DHCP_OPTION82_LENGTH_IP_MODE        16
#define	DHCP_CIRCUIT_ID_SUBOPTION           1
#define	DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT  6
#define	DHCP_CID_TYPE_VLAN_UNIT_PORT              0
#define DHCP_CID_TYPE_CONFIGURED_STRING           1
#define	DHCP_CID_LENGTH_VLAN_UNIT_PORT            4
#define	DHCP_REMOTE_ID_SUBOPTION            2
#define	DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_HEX    8
#define	DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_ASCII 14
#define DHCP_RID_SUBOPTION_LENGTH_IP_MODE_HEX     6
#define	DHCP_RID_TYPE_MAC_MODE_HEX                0
#define DHCP_RID_TYPE_IP_MODE_HEX                 1
#define	DHCP_RID_TYPE_MAC_MODE_ASCII              2
#define DHCP_RID_TYPE_IP_MODE_ASCII               3
#define DHCP_RID_TYPE_CONFIGURED_STRING           4
#define	DHCP_RID_LENGTH_MAC_MODE_HEX              6
#define DHCP_RID_LENGTH_MAC_MODE_ASCII           12
#define DHCP_RID_LENGTH_IP_MODE_HEX               4
#endif

/* DHCP Option codes: */

#define DHO_PAD                         0
#define DHO_SUBNET_MASK                 1
#define DHO_TIME_OFFSET                 2
#define DHO_ROUTERS                     3
#define DHO_TIME_SERVERS                4
#define DHO_NAME_SERVERS                5
#define DHO_DOMAIN_NAME_SERVERS         6
#define DHO_LOG_SERVERS                 7
#define DHO_COOKIE_SERVERS              8
#define DHO_LPR_SERVERS                 9
#define DHO_IMPRESS_SERVERS             10
#define DHO_RESOURCE_LOCATION_SERVERS   11
#define DHO_HOST_NAME                   12
#define DHO_BOOT_SIZE                   13
#define DHO_MERIT_DUMP                  14
#define DHO_DOMAIN_NAME                 15
#define DHO_SWAP_SERVER                 16
#define DHO_ROOT_PATH                   17
#define DHO_EXTENSIONS_PATH             18
#define DHO_IP_FORWARDING               19
#define DHO_NON_LOCAL_SOURCE_ROUTING    20
#define DHO_POLICY_FILTER               21
#define DHO_MAX_DGRAM_REASSEMBLY        22
#define DHO_DEFAULT_IP_TTL              23
#define DHO_PATH_MTU_AGING_TIMEOUT      24
#define DHO_PATH_MTU_PLATEAU_TABLE      25
#define DHO_INTERFACE_MTU               26
#define DHO_ALL_SUBNETS_LOCAL           27
#define DHO_BROADCAST_ADDRESS           28
#define DHO_PERFORM_MASK_DISCOVERY      29
#define DHO_MASK_SUPPLIER               30
#define DHO_ROUTER_DISCOVERY            31
#define DHO_ROUTER_SOLICITATION_ADDRESS 32
#define DHO_STATIC_ROUTES               33
#define DHO_TRAILER_ENCAPSULATION       34
#define DHO_ARP_CACHE_TIMEOUT           35
#define DHO_IEEE802_3_ENCAPSULATION     36
#define DHO_DEFAULT_TCP_TTL             37
#define DHO_TCP_KEEPALIVE_INTERVAL      38
#define DHO_TCP_KEEPALIVE_GARBAGE       39
#define DHO_NIS_DOMAIN                  40
#define DHO_NIS_SERVERS                 41
#define DHO_NTP_SERVERS                 42
#define DHO_VENDOR_ENCAPSULATED_OPTIONS 43
#define DHO_NETBIOS_NAME_SERVERS        44
#define DHO_NETBIOS_DD_SERVER           45
#define DHO_NETBIOS_NODE_TYPE           46
#define DHO_NETBIOS_SCOPE               47
#define DHO_FONT_SERVERS                48
#define DHO_X_DISPLAY_MANAGER           49
#define DHO_DHCP_REQUESTED_ADDRESS      50
#define DHO_DHCP_LEASE_TIME             51
#define DHO_DHCP_OPTION_OVERLOAD        52
#define DHO_DHCP_MESSAGE_TYPE           53
#define DHO_DHCP_SERVER_IDENTIFIER      54
#define DHO_DHCP_PARAMETER_REQUEST_LIST 55
#define DHO_DHCP_MESSAGE                56
#define DHO_DHCP_MAX_MESSAGE_SIZE       57
#define DHO_DHCP_RENEWAL_TIME           58
#define DHO_DHCP_REBINDING_TIME         59
#define DHO_DHCP_CLASS_IDENTIFIER       60
#define DHO_DHCP_CLIENT_IDENTIFIER      61
#define DHO_DHCP_TFTP_SERVER    		66      /* 2007-03, Joseph */
#define DHO_DHCP_BOOTFILE_NAME  		67      /* 2007-03, Joseph */
#define DHO_DHCP_USER_CLASS_ID          77
#define DHO_DHCP_OPTION_82              82      /* 2006-07, Joseph */
#define DHO_END                         255




#if ((SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)||(SYS_CPNT_DHCPSNP_INFORMATION_OPTION == TRUE))	
typedef enum
{
    DHCP_OPTION82_ENABLE = VAL_dhcp_Option82_enabled,
	DHCP_OPTION82_DISABLE = VAL_dhcp_Option82_disabled
} DHCP_OPTON82_E;


typedef enum
{
    DHCP_OPTION82_POLICY_DROP = VAL_dhcp_Option82_Policy_drop,
	DHCP_OPTION82_POLICY_REPLACE = VAL_dhcp_Option82_Policy_replace,
	DHCP_OPTION82_POLICY_KEEP = VAL_dhcp_Option82_Policy_keep
} DHCP_OPTON82_POLICY_E;	

typedef enum
{
    DHCP_OPTION82_RID_MAC_HEX =1,
    DHCP_OPTION82_RID_MAC_ASCII,
    DHCP_OPTION82_RID_IP_HEX,
    DHCP_OPTION82_RID_IP_ASCII,
    DHCP_OPTION82_RID_CONFIGURED_STRING
    
} DHCP_OPTION82_RID_E;

/* for trace_id of user_id when allocate buffer with l_mem/l_mm
 */
enum
{
    DHCP_TYPE_TRACE_ID_DHCP_ALGO_HANDLE_OPTION82 = 0,
    DHCP_TYPE_TRACE_ID_DHCP_ALGO_REPLYPACKETPROCESS,
};
#endif

/* end 2006-07 */
/* end porting, rich */

/* DHCP message types. */
#define DHCPDISCOVER                    1
#define DHCPOFFER                       2
#define DHCPREQUEST                     3
#define DHCPDECLINE                     4
#define DHCPACK                         5
#define DHCPNAK                         6
#define DHCPRELEASE                     7
#define DHCPINFORM                      8

/*  DHCP used data length   */
#define DHCP_UDP_OVERHEAD               (14 + /* Ethernet header */     \
                                         20 + /* IP header */           \
                                         8)   /* UDP header */
#define DHCP_SNAME_LEN                  64
#define DHCP_FILE_LEN                   SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN
#define DHCP_FIXED_NON_UDP              236
#define DHCP_FIXED_LEN                  (DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
                        /* Everything but options. */
#define DHCP_MTU_MAX                    1500
#define DHCP_OPTION_LEN                 (DHCP_MTU_MAX - DHCP_FIXED_LEN)

#define BOOTP_MIN_LEN                   300
#define DHCP_MIN_LEN                    548

/* Possible modes in which discover_interfaces can run. */

#define DISCOVER_RUNNING    0
#define DISCOVER_SERVER     1
#define DISCOVER_UNCONFIGURED   2
#define DISCOVER_RELAY      3
#define DISCOVER_REQUESTED  4

#define ROOT_GROUP  0
#define HOST_DECL   1
#define SHARED_NET_DECL 2
#define SUBNET_DECL 3
#define CLASS_DECL  4
#define GROUP_DECL  5

#define BOOTP_BROADCAST 32768L

/* time seconds */
#define DHCP_TYPE_ONE_DAY_SEC                  (60*60*24)
#define DHCP_TYPE_ONE_HOUR_SEC                 (60*60)
#define DHCP_TYPE_ONE_MIN_SEC                  60

/* moved from task.c and renamed */
#define DHCP_TYPE_EVENT_TIMER_1_SEC     BIT_2


/* static panic flag */
#define DHCP_TYPE_STATE_PANIC_DHCP_DISCOVER     1
#define DHCP_TYPE_STATE_PANIC_DHCP_INFORM       2
/* Function number */
enum
{
    /*  function_no
        function_no
     */
    DHCP_TYPE_STATE_PANIC = 0,
    DHCP_TYPE_BIND_LEASE,
    DHCP_TYPE_STORE_CONFIG_FROM_ACK,
};

/* Error number */
enum
{
    DHCP_TYPE_EH_Unknown = 0,
    DHCP_TYPE_EH_Dmalloc,
    DHCP_TYPE_EH_Dfree,
    DHCP_TYPE_EH_Add_protocol,
    DHCP_TYPE_EH_Add_hash,
    DHCP_TYPE_EH_Tree_const,
    DHCP_TYPE_EH_Tree_concat,
    DHCP_TYPE_EH_DHCP_MGR_do_packet,
    DHCP_TYPE_EH_Write_client_lease,
    DHCP_TYPE_EH_Parse_option_buffer,
    DHCP_TYPE_EH_State_panic,
    DHCP_TYPE_EH_Bind_lease,
    DHCP_TYPE_EH_State_panic_dhcp_inform,
    DHCP_TYPE_EH_Store_config_from_ack,

};

/* Possible states in which the client can be. */
enum dhcp_state {
    S_REBOOTING,
    S_INIT,
    S_SELECTING,
    S_REQUESTING,
    S_BOUND,
    S_RENEWING,
    S_REBINDING
};



/* MACRO DECLARATIONS
 */
/*  -- New Definition in Mercury --
 *  u_intxx_t is used in DHCP/IAD.
 *  For porting issue, redefine the type to sys_type defined type.
 */
/*#define u_int32_t   UI32_T
#define u_int16_t   UI16_T
#define u_int8_t    UI8_T
*/
/*  #define PROTO(x)    x   */

/* TYPE DECLARATIONS
 */
/*  -- New Definition in Mercury --
 *  The task state translation is same as whole system state,
 *  and provision complete is a state in master and after CLI configuration
 *  command is provisioned.
 */
typedef UI32_T  DHCP_TIME;              /*  for TIME in DHCP/IAD    */

typedef void (*DHCP_FUNC)(void*);
#define VOIDPTR void *

/*typedef struct DHCP_TYPE_ADDRESS_LIST_S
{
    UI32_T ip_address;
    struct DHCP_TYPE_ADDRESS_LIST_S *next;
    //int count;
}DHCP_TYPE_ADDRESS_LIST_T;*/

typedef enum
{
    DHCP_TYPE_SYSTEM_STATE_TRANSITION = SYS_TYPE_STACKING_TRANSITION_MODE,
    DHCP_TYPE_SYSTEM_STATE_MASTER   = SYS_TYPE_STACKING_MASTER_MODE,
    DHCP_TYPE_SYSTEM_STATE_SLAVE    = SYS_TYPE_STACKING_SLAVE_MODE,
    DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE,
    DHCP_TYPE_SYSTEN_STATE_RUNNING
}   DHCP_TYPE_SYSTEM_STATE_T;

#pragma pack(1)
typedef struct DHCP_TYPE_IpHeader_S
{
    UI8_T   ip_ver_hlen;    /*  0x45    */
    UI8_T   tos;
    UI16_T  tlen;
    UI16_T  id;
    UI16_T  offset;
    UI8_T   ttl;
    UI8_T   proto;
    UI16_T  csum;
    UI32_T  sip;
    UI32_T  dip;
}   DHCP_TYPE_IpHeader_T;


typedef struct DHCP_TYPE_UdpHeader_S
{
    UI16_T  src_port;
    UI16_T  dst_port;
    UI16_T  udp_header_len;
    UI16_T  chksum;
}   DHCP_TYPE_UdpHeader_T;
#pragma pack()

/*  --  Original defined in DHCP/IAD    --
 *  reference DHCP/IAD definition.
 *  For porting issue, redefine the type to int.
 */
#ifdef ssize_t
typedef int     ssize_t;
#endif
//typedef   UI32_T  size_t;
#ifdef u_long
typedef UI32_T  u_long;
#endif
/*
 * Internet address (a structure for historical reasons)
 *
struct in_addr {
    u_long s_addr;
};
 */


/*  -- Following definition is porting from DHCP/IAD -- */
/* An internet address of up to 128 bits. */
struct iaddr {
    int len;
    unsigned char iabuf [16];
};

struct iaddrlist {
    struct iaddrlist *next;
    struct iaddr addr;
};

struct option_data {
    int len;
    u_int8_t *data;
};

struct string_list {
    struct string_list *next;
    char string [1];
};

/* 2002-7-16: Penny 1st created for cid issue (JBOS) */
typedef struct DHCP_TYPE_ClientId_S
{
    UI32_T id_mode;                             /* DHCP_MGR_CID_HEX | DHCP_MGR_CID_TEXT */
    UI32_T id_len;                              /* len of buffer */
    UI8_T id_buf[MAXSIZE_dhcpcIfClientId+1];    /* content of CID */
}DHCP_TYPE_ClientId_T;

/* begin 2007-12, Joseph */
typedef struct DHCP_TYPE_Vendor_S 
{
    UI32_T vendor_mode;                                     /* DHCP_MGR_CLASSID_HEX | DHCP_MGR_CLASSID_TEXT */
	UI32_T vendor_len;								        /* len of buffer */
	UI8_T  vendor_buf[DHCP_TYPE_CLASSID_BUF_MAX_SIZE+1];   	/* content of Vendor */
}DHCP_TYPE_Vendor_T;
/* end 2007-12 */ 

/***
 * Configuration information from the config file...
 ***/


struct client_config {
    struct option_data defaults [256]; /* Default values for options. */
    enum {
        ACTION_DEFAULT,     /* Use server value if present,
                       otherwise default. */
        ACTION_SUPERSEDE,   /* Always use default. */
        ACTION_PREPEND,     /* Prepend default to server. */
        ACTION_APPEND,      /* Append default to server. */
    } default_actions [256];

    struct option_data send_options [256]; /* Send these to server. */
    u_int8_t required_options [256]; /* Options server must supply. */
    u_int8_t requested_options [256]; /* Options to request from server. */
    int requested_option_count; /* Number of requested options. */
    DHCP_TIME timeout;          /* Start to panic if we don't get a
                       lease in this time period when
                       SELECTING. */
    DHCP_TIME initial_interval;     /* All exponential backoff intervals
                       start here. */
    DHCP_TIME retry_interval;       /* If the protocol failed to produce
                       an address before the timeout,
                       try the protocol again after this
                       many seconds. */
    DHCP_TIME select_interval;      /* Wait this many seconds from the
                       first DHCPDISCOVER before
                       picking an offered lease. */
    DHCP_TIME reboot_timeout;       /* When in INIT-REBOOT, wait this
                       long before giving up and going
                       to INIT. */
    DHCP_TIME backoff_cutoff;       /* When doing exponential backoff,
                       never back off to an interval
                       longer than this amount. */
    struct string_list *media;  /* Possible network media values. */
    char *script_name;      /* Name of config script. */
    enum { IGNORE, ACCEPT, PREFER } bootp_policy;
                    /* Ignore, accept or prefer BOOTP
                       responses. */
    struct string_list *medium; /* Current network medium. */

    struct iaddrlist *reject_list;  /* Servers to reject. */
};

/***
 * DHCP client lease structure...
 ***/
struct client_lease {
    struct client_lease *next;            /* Next lease in list. */
    TIME expiry, renewal, rebind;         /* Lease timeouts. */
    struct iaddr address;                 /* Address being leased. */
    char *server_name;                    /* Name of boot server. */
    char *filename;                       /* Name of file we're supposed to boot. */
    struct string_list *medium;           /* Network medium. */

    unsigned int is_static : 1;           /* If set, lease is from config file. */
    unsigned int is_bootp: 1;             /* If set, lease was aquired with BOOTP. */

    struct option_data options [256];     /* Options supplied with lease. */
};
struct class {
    char *name;

    struct group *group;
};


/*** Short lease stored in UC
 * Created by Penny 2002/2/6
 */
struct dhcp_uc_lease{
    TIME expiry, renewal, rebind;             /* Lease timeouts. */
    struct iaddr address;
    unsigned int is_bootp: 1;
    UI32_T vid_ifIndex;
    struct dhcp_uc_lease *next;
};


/***
 *  Declare DHCP packet structure and field name
 ***/
struct dhcp_packet {
    u_int8_t  op;       /* Message opcode/type */
    u_int8_t  htype;    /* Hardware addr type (see net/if_types.h) */
    u_int8_t  hlen;     /* Hardware addr length */
    u_int8_t  hops;     /* Number of relay agent hops from client */
    u_int32_t xid;      /* Transaction ID */
    u_int16_t secs;     /* Seconds since client started looking */
    u_int16_t flags;    /* Flag bits */
    UI32_T ciaddr;  /* Client IP address (if already in use) */
    UI32_T yiaddr;  /* Client IP address */
    UI32_T siaddr;  /* IP address of next server to talk to */
    UI32_T giaddr;  /* DHCP relay agent IP address */
    unsigned char chaddr [16];  /* Client hardware address */
    char sname [DHCP_SNAME_LEN];    /* Server name */
    char file [DHCP_FILE_LEN];  /* Boot filename */
    unsigned char options [DHCP_OPTION_LEN];
                /* Optional parameters
                   (actual length dependent on MTU). */
};

/* rich porting to linux from 28-MO-38, 2007/12/19 */
typedef struct PSEUDO_HEADER_S {
    UI8_T     src_ip[4];
    UI8_T     dst_ip[4];
    UI8_T     reserved;
    UI8_T     protocol;
    UI16_T    udp_length;
} PSEUDO_HEADER_T;

typedef struct UDP_CHKECSUM_S {
    PSEUDO_HEADER_T           pseudo_header;
    DHCP_TYPE_UdpHeader_T     udp_header;
    struct dhcp_packet        dhcp_packet;
} UDP_CHECKSUM_T;

/* end 2006-07 */
/* end rich porting */

/***
 * Per-interface state used in the dhcp client...
 ***/
struct client_state {
    struct client_lease *active;          /* Currently active lease. */
    struct client_lease *new;                  /* New lease. */
    struct client_lease *offered_leases;        /* Leases offered to us. */
    struct client_lease *leases;        /* Leases we currently hold. */
    struct client_lease *alias;              /* Alias lease. */

    enum dhcp_state state;      /* Current state for this interface. */
    struct iaddr destination;           /* Where to send packet. */
    u_int32_t xid;                    /* Transaction ID. */
    u_int16_t secs;             /* secs value from DHCPDISCOVER. */
    TIME first_sending;         /* When was first copy sent? */
    TIME interval;            /* What's the current resend interval? */
    struct string_list *medium;        /* Last media type tried. */

    struct dhcp_packet packet;          /* Outgoing DHCP packet. */
    int packet_length;         /* Actual length of generated packet. */

    struct iaddr requested_address;     /* Address we would like to get. */

    struct client_config *config;       /* Information from config file. */

    struct string_list *env;           /* Client script environment. */
    int envc;           /* Number of entries in environment. */
};

/***
 *  Structure for hardware address declaration, similiar as SNAD in Phase2.
 ***/
struct hardware {
    u_int8_t htype;
    u_int8_t hlen;
    u_int8_t haddr [16];

};


typedef struct DHCP_TYPE_ServerOptions_S
{
   DHCP_TYPE_ClientId_T cid;	
   UI32_T   default_router[SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER];  
   UI32_T   dns_server[SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER];  
   UI32_T   next_server; 
   UI32_T   netbios_name_server[SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER]; 
   UI32_T  	netbios_node_type;  										 
   char  	domain_name[SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN+1];  
   char	    bootfile[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN+1];						 /* the string len of bootfile name */
   UI32_T  	lease_time;													 /* lease time in sec */
   
}DHCP_TYPE_ServerOptions_T;

typedef struct DHCP_TYPE_PoolConfigEntry_S
{
	struct DHCP_TYPE_PoolConfigEntry_S *next;
	struct DHCP_TYPE_PoolConfigEntry_S *clarified_next;
	struct DHCP_TYPE_PoolConfigEntry_S *previous;
	struct DHCP_TYPE_PoolConfigEntry_S *clarified_previous;
	
	char	pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1]; 	/* pool name (string): key */
	UI32_T	pool_type; 								/* DHCP_TYPE_POOL_NETWORK / DHCP_TYPE_POOL_HOST */
	UI32_T  network_address;						/* network ip address */
	UI32_T  sub_netmask;							/* subnet mask  */
	UI32_T	host_address;							/* host ip address */
	DHCP_TYPE_ServerOptions_T  options; 				/* various configurable options */
	struct hardware  hardware_address; 				/* Mac address */

}DHCP_TYPE_PoolConfigEntry_T;


/***
 *  Information about each network interface. (interface_info)
 *  The key of this structure is vid_ifIndex, based on 802.1Q available.
 *
 ***/
struct interface_info {
    struct interface_info *next;            /* Next interface in list... */
    struct shared_network *shared_network;  /* Networks connected to this interface. */
    struct hardware hw_address;             /* Its physical address. */
    /*  2002.01.06, William,    change using Phase2's type.
     *                          keep original for reference.
     *  struct in_addr primary_address;
     */
    UI32_T  primary_address;                /* Primary interface address.   */

    /*  char name [IFNAMSIZ];   */          /* Its name... */
    int rfdesc;                             /* Its read file descriptor. */
    int wfdesc;                             /* Its write file descriptor, if
                                                different. */
    unsigned char *rbuf;                    /* Read buffer, if required. */
    UI32_T rbuf_max;        /* Size of read buffer. */
    UI32_T rbuf_offset;     /* Current offset into buffer. */
    UI32_T rbuf_len;        /* Length of data in buffer. */

    /*  struct ifreq *ifp;  */      /* Pointer to ifreq struct. */
    //u_int32_t flags;      /* Control flags... */
#define INTERFACE_REQUESTED 1
#define INTERFACE_AUTOMATIC 2

    /*  New fields for Mercury project  */
    UI32_T  vid_ifIndex;            /*  interface associated with ifIndex   */
    /*  DHCP_TYPE_INTERFACE_MODE_USER_DEFINE means disable DHCP and BOOTP,
     *  no binding on the interface.
     */
    UI32_T  mode;                   /*  BOOTP/DHCP/USER_DEFINE  */
    UI32_T  role;                   /*  Client/Server/Relay     */
    UI32_T  client_port;            /*  DHCP client, default=68 */
    UI32_T  server_port;            /*  DHCP server, default=67 */
    UI32_T  server_ip;              /*  for client, DHCP/BOOTP server ip    */
    UI32_T  gateway_ip;             /*  for client, the relay agent's ip    */

    UI32_T relay_server_list[MAX_RELAY_SERVER]; /* the server address list for relay agent */
    DHCP_TYPE_ClientId_T  cid;

    /* end of new field for Mercury */

    DHCP_TYPE_Vendor_T  classid;            /* 2007-12, Joseph */
    	
    /* Only used by DHCP client code. */
    struct client_state *client;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    BOOL_T dhcp_inform;
#endif
};

/***
 *  Declare protocol associated structure.
 ***/
struct protocol {
    struct protocol *next;
    int fd;
    void (*handler) (struct protocol *);
    void *local;
    //UI32_T role;
};



/***
 * A dhcp packet and the pointers to its option values.
 ***/
struct packet {
    struct dhcp_packet *raw;
    int packet_length;
    int packet_type;
    int options_valid;
    int client_port;
    struct iaddr client_addr;
    struct interface_info *interface;   /* Interface on which packet
                           was received. */
    struct hardware *haddr;     /* Physical link address
                       of local sender (maybe gateway). */
    struct shared_network *shared_network;
    struct option_data options [256];
    int got_requested_address;  /* True if client sent the
                       dhcp-requested-address option. */
};



/* Group of declarations that share common parameters. */
struct group {
    struct group *next;
    struct subnet *subnet;
    struct shared_network *shared_network;
    TIME default_lease_time;
    TIME max_lease_time;
    TIME bootp_lease_cutoff;
    TIME bootp_lease_length;
    //char *filename;
    UI8_T filename[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN];
    char *server_name;
    struct iaddr next_server;
    int boot_unknown_clients;
    int dynamic_bootp;
    int allow_bootp;
    int allow_booting;
    int one_lease_per_client;
    int get_lease_hostnames;
    int use_host_decl_names;
    int use_lease_addr_for_default_route;
    int authoritative;
    int always_reply_rfc1048;
    struct tree_cache *options [256];
};


struct subnet {
    struct subnet *next_subnet;
    struct subnet *next_sibling;
    struct shared_network *shared_network;
    struct interface_info *interface;
    struct iaddr interface_address;
    struct iaddr net;
    struct iaddr netmask;
    struct group *group;
};

struct shared_network {
    struct shared_network *next;
    char *name;
    struct subnet *subnets;
    struct interface_info *interface;
    struct lease *leases;
    struct lease *insertion_point;
    struct lease *last_lease;

    struct group *group;
};

struct lease_state {
    struct lease_state *next;
    struct interface_info *ip;
    TIME offered_expiry;
    struct tree_cache *options [256];
    UI32_T expiry, renewal, rebind;
    char filename [DHCP_FILE_LEN];
    char *server_name;
    struct iaddr from;
    int max_message_size;
    UI8_T *prl;
    int prl_len;
    int got_requested_address;  /* True if client sent the
                       dhcp-requested-address option. */
    int got_server_identifier;  /* True if client sent the
                       dhcp-server-identifier option. */
    struct shared_network *shared_network;  /* Shared network of interface
                           on which request arrived. */
    UI32_T xid;
    UI16_T secs;
    UI16_T bootp_flags;
    //struct in_addr ciaddr;
    //struct in_addr giaddr;
    UI32_T ciaddr;
    UI32_T giaddr;

    UI8_T hops;
    UI8_T offer;
};

/* A dhcp host declaration structure. */
struct host_decl {
    struct host_decl *n_ipaddr;
    char *name;
    struct hardware interface;
	BOOL_T encoded_hw_addr;    
    struct tree_cache *fixed_addr;
	BOOL_T encoded_cid;
    struct group *group;
};

/* A dhcp lease declaration structure. */
struct lease {
    struct lease *next;
    struct lease *prev;
    struct lease *n_uid, *n_hw;
    struct lease *waitq_next;
    //struct iaddr ip_addr;
    UI32_T ip_addr;
    TIME starts, ends, timestamp;
    unsigned char *uid;
    int uid_len;
    int uid_max;
    unsigned char uid_buf [32];
    char *hostname;
    char *client_hostname;
    struct host_decl *host;
    struct subnet *subnet;
    struct shared_network *shared_network;
    struct hardware hardware_addr;
    int flags;
#define STATIC_LEASE                    1
#define BOOTP_LEASE                     2
#define DYNAMIC_BOOTP_OK                4
#define PERSISTENT_FLAGS                (DYNAMIC_BOOTP_OK)
#define EPHEMERAL_FLAGS             (BOOTP_LEASE)
#define MS_NULL_TERMINATION         8
#define ABANDONED_LEASE             16
    struct lease_state *state;
};


struct domain_search_list
{
    UI32_T  dummy;
};

struct name_server
{
    UI32_T  dummy;
};

/* moved from dhcp_task.c */
#pragma pack(1)
typedef struct DHCP_TYPE_MSG_S
{
    L_MM_Mref_Handle_T *mref_handle_p;
    UI16_T      packet_length;
    UI16_T      rxRifNum;
    UI8_T       dst_mac[6];
    UI8_T       src_mac[6];
    UI16_T      vid;
    UI16_T      src_lport_ifIndex;
}   DHCP_TYPE_MSG_T;
#pragma pack()


#endif   /* _DHCP_TYPE_H */
