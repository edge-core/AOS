#ifndef RIP2TIME_H
#define RIP2TIME_H


#include "sys_pal.h"
#include "netcfg_type.h"
#include <netinet/in.h>

#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"


/*
 * function declarations
 */
#define INET_ADDRESS_MAX_SIZE 4
#define INET_ADDRESS_IPV4_SIZE 4

#define RIPMIB 1,3,6,1,4,1,259,6,10,104,1,18,2

/*wei.zhang add */
/***ripNetworkByInetAddrTable****/
#define RIPNETWORKBYINETADDRADDRESSTYPE         1
#define RIPNETWORKBYINETADDRADDRESS             2
#define RIPNETWORKBYINETADDRPFXLEN              3
#define RIPNETWORKBYINETADDRSTATUS              4


/********************************************
 ****ripInstabilityPreventingTable***********
 ********************************************
 */
#define RIPVLANINDEX                    1
#define RIPSPLITHORIZONSTATUS           2

/********************************************
 **********ripNetworkAddrTable***************
 ********************************************
 */
#define RIPNETWORKIFINDEX           1
#define RIPNETWORKADDRSTATUS            2


/********************************************
 **********ripPassiveInterfaceTable***************
 ********************************************
 */
#define RIPINTERFACEINDEX           1
#define RIPPASSIVEINTERFACESTATUS   2


/********************************************
ripNeighborTable
********************************************
*/
#define RIPNEIGHBORADDRESSTYPE          1
#define RIPNEIGHBORADDRESS				2
#define RIPNEIGHBORADDRESSSTATUS		3



/***ripDistanceTable****/
#define RIPDISTANCEINETADDRTYPE         1
#define RIPDISTANCEINETADDR             2
#define RIPDISTANCEPFXLEN              3
#define RIPDISTANCEVALUE               4
#define RIPDISTANCEALISENAME           5
#define RIPDISTANCEROWSTATUS           6


/*var_ripRouteClearByNetworkTable*/
#define RIPROUTECLEARBYNETWORKINETADDRADDRTYPE         1
#define RIPROUTECLEARBYNETWORKINETADDRADDR             2
#define RIPROUTECLEARBYNETWORKPFXLEN              3
#define RIPROUTECLEARBYNETWORKSTATUS              4
/*var_ripRedistribueTable*/
#define RIPREDISTRIBUTEPROTOCOL				1
#define RIPREDISTRIBUTEMETRIC               2
#define RIPREDISTRIBUTESTATUS               3
#define RIPREDISTRIBUTEREMAPNAME            4

/*wei.zhang add */
/*rip originate */
#define RIP_SNMP_ROUTING_INFORMATION_NOORIGINATE    1
#define RIP_SNMP_ROUTING_INFORMATION_ORIGINATE      2


#define RIP_SNMP_ROUTING_PROCESS_STATUS_VALID             1
#define RIP_SNMP_ROUTING_PROCESS_STATUS_INVALID           2
    
    
#define RIP_SNMP_STATISTICS_RESET                         1
#define RIP_SNMP_STATISTICS_NO_RESET                      2
    
#define RIP_SNMP_NETWORKADDR_VALID                        1
#define RIP_SNMP_NETWORKADDR_INVALID                      2

/*rip distance table*/
#define MIN_RIP_SNMP_DISTANCE                        1
#define MAX_RIP_SNMP_DISTANCE                      255


/*route clear by network*/  
#define RIP_SNMP_ROUTECLEARBYNETWORK_NOCLEAR                        1
#define RIP_SNMP_ROUTECLEARBYNETWORK_CLEAR                      2

  
#define RIP_SNMP_NEIGHBOR_VALID                           1
#define RIP_SNMP_NEIGHBOR_INVALID                         2
    
#define RIP_SNMP_PASSIVE_INTERFACE_VALID                  1
#define RIP_SNMP_PASSIVE_INTERFACE_INVALID                2

/*wei.zhang add for information originate*/
#define RIPDEFAULTINFORMATIONORIGINATE 1
    
/*rip distance*/
#define MAXRIPDISTANCEALISENAME 16


#define STRING	      ASN_OCTET_STR

#define INTEGER32     ASN_INTEGER
#define INTEGER       ASN_INTEGER

#define RIP_API_REDIST_STATUS_VALID           1
#define RIP_API_REDIST_STATUS_INVALID         2

#define RIP_API_DLIST_STATUS_VALID         1
#define RIP_API_DLIST_STATUS_INVALID       2

#define RIP_SNMP_SPLIT_HORIZON                      1
#define RIP_SNMP_POISON_REVERSE                     2
#define RIP_SNMP_SPLIT_NONE                         3


#define RIP_SNMP_DEFAULT_VRF_ID 0

#define MINSIZE_RIP_DEFAULTMETRIC	1L
#define MAXSIZE_RIP_DEFAULTMETRIC	16L
/*wei.zhang add for distance*/ 
#define MIN_DISTANCE_VALUE 1
#define MAX_DISTANCE_VALUE 255
#define MAX_DISTANCE_ALISENAME 16L

/*wei.zhang add for distance */
#define VAL_ripDistanceStatus_active	1L
#define VAL_ripDistanceStatus_notInService	2L
#define VAL_ripDistanceStatus_notReady	3L
#define VAL_ripDistanceStatus_createAndGo	4L
#define VAL_ripDistanceStatus_createAndWait	5L
#define VAL_ripDistanceStatus_destroy	6L


#define VAL_RIP_RouteClearByType_noClear 		1
#define VAL_RIP_RouteClearByType_all			2
#define VAL_RIP_RouteClearByType_connected	3
#define VAL_RIP_RouteClearByType_ospf			4
#define VAL_RIP_RouteClearByType_rip			5
#define VAL_RIP_RouteClearByType_static		6
#define VAL_RIP_RouteClearByNetwork_noClear 		1
#define VAL_RIP_RouteClearByNetwork_clear 		2

/* RIP version number. */
#define RIPv1				1
#define RIPv2				2
#define RIP_PORT_DEFAULT		520
#define RIP_MAX_PACKET_SIZE		1500

/* RIP timers */
#define RIP_UPDATE_TIMER_DEFAULT	30
#define RIP_TIMEOUT_TIMER_DEFAULT	180
#define RIP_GARBAGE_TIMER_DEFAULT	120

#define RIP_TIMER_MIN			5
#define RIP_TIMER_MAX			2147483647

#define RIP_INTERFACE_WAKEUP_DELAY	1
#define RIP_PREMATURE_TIMEOUT_DELAY	1
#define RIP_TRIGGERED_UPDATE_DELAY	2

#define RIP_DEFAULT_DISTANCE        120
#define RIP_DEFAULT_METRIC          1
#define RIP_MAX_PREFIX              SYS_ADPT_MAX_NBR_OF_RIP_ROUTE_ENTRY
#define RIP_ROUTER_VERSION          3
#define RIP_DEFAULT_INFORMATION_ORIGINATE   1


/* Default value for "default-metric" command. */
#define RIP_DEFAULT_METRIC_DEFAULT	1

/* RIP metric and multicast group address. */
#define RIP_METRIC_INFINITY		16

#define PREFIX_STRING_SIZE     20
/*wei.zhang 7/24*/
/*rip maxPrefix*/
#define RIP_MIN_MAXPREFIX    1
#define RIP_MAX_MAXPREFIX    SYS_ADPT_MAX_NBR_OF_RIP_ROUTE_ENTRY
/*rip default information originate*/
#define RIP_NO_ORIGINATE    1
#define RIP_ORIGINATE       2

/*wei.zhang add for debug*/
#define RIP_SNMP_DEBUG FALSE 


#define VAL_Status_active	1L
#define VAL_Status_notInService	2L
#define VAL_Status_notReady	3L
#define VAL_Status_createAndGo	4L
#define VAL_Status_createAndWait	5L
#define VAL_Status_destroy	6L


#define L_RSTATUS_NOT_EXIST      0  /* state         */ /* Not RFC value */
#define L_RSTATUS_ACTIVE         1  /* state, action */
#define L_RSTATUS_NOTINSERVICE   2  /*        action */
#define L_RSTATUS_NOTREADY       3  /* state         */
#define L_RSTATUS_CREATEANDGO    4  /*        action */
#define L_RSTATUS_CREATEANDWAIT  5  /*        action */
#define L_RSTATUS_DESTROY        6  /*        action */
#define L_RSTATUS_SET_OTHER      7  /*        action */ /* Not RFC value */
#define L_RSTATUS_ALLOCATED      8


/* SNMP value return using static variable.  */
#define RIP_SNMP_RETURN_INTEGER(V) \
  do { \
    *var_len = sizeof (int); \
    rip_int_val = V; \
    return (u_char *) &rip_int_val; \
  } while (0)
      /* IPv4 prefix structure. */
#if 0
struct prefix_ipv4
{
    u_int8_t family;
    u_int8_t prefixlen;
    u_int8_t pad1;
    u_int8_t pad2;
    struct pal_in4_addr prefix;
};
#endif

/* RIP default instance. */
#define RIP_DEFAULT_INSTANCE		0

enum RIP_API_LISTTYPE_E
{
    /* modify for EPR# ES4827G-20-00928. Jian Rong. */
    ACCESS_LIST = 1,
    PREFIX_LIST = 2
};

typedef struct RIP_DLIST_INDEX_S
{
    /*interface index*/
    u_int32_t ifindex;
    /*access_list or prefix_list*/
    enum RIP_API_LISTTYPE_E type;
} RIP_DLIST_INDEX_T;
/* NetworkAddrTable*/
typedef struct RIP_SNMP_NetworkAddrTableIndex_S
{
    UI32_T	   type;
    UI32_T	   addr;
    UI8_T        pfx_len;
} RIP_SNMP_NetworkAddrTableIndex_T;

/* RouteClearByNetworkTable*/
typedef struct RIP_SNMP_RouteClearByNetworkTableIndex_S
{
    UI32_T	   type;
    UI32_T	   addr;
    UI8_T        pfx_len;
} RIP_SNMP_RouteClearByNetworkTableIndex_T;

/*wei.zhang add for neighbor table*/
typedef struct RIP_SNMP_NeighborAddr_S
{
    char      addr[INET_ADDRESS_MAX_SIZE];

} RIP_SNMP_NeighborAddr_T;

/*wei.zhang add for network table*/
typedef struct RIP_SNMP_NeighborAddrTableIndex_S
{
    char       addr[INET_ADDRESS_MAX_SIZE];

} RIP_SNMP_NeighborAddrTableIndex_T;

/*rip distance entry */

typedef struct RIP_Distance_S
{
    UI32_T  distance;
    char    alist_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    UI32_T  status;
}RIP_Distance_T;

/* RIPDistanceTableIndex*/
typedef struct RIP_SNMP_DistanceTableIndex_S
{
    UI32_T	   addr;
    UI8_T        pfx_len;
} RIP_SNMP_DistanceTableIndex_T;

typedef struct RIP_Distance_snmp_S
{
    UI32_T pfxlen;
    UI32_T ip_addr;
    UI32_T distance;
    char   alist_name[SYS_ADPT_ACL_MAX_NAME_LEN + 1];
    UI32_T status;
    BOOL_T used;
}RIP_Distance_snmp_T;


  
  /*ripMgt*/
  void    init_ripMgt(void);
  
  /*ripNeighborTable*/
  void    init_ripNeighborTable(void);
  
  /*ripInstabilityPreventingTable*/
  void    init_ripInstabilityPreventingTable(void);
  
  /*ripNetworkByInterfaceTable*/
  void    init_ripNetworkByInterfaceTable(void);
  
  /*ripNetworkByInetAddrTable*/
  void	init_ripNetworkByInetAddrTable(void);
  
  /*ripPassiveInterfaceTable*/
  void    init_ripPassiveInterfaceTable(void);
  /*ripRouteClearByNetworkTable*/
  void     init_ripRouteClearByNetworkTable(void);
  
  /*ripDistanceTable*/
  void    init_ripDistanceTable(void);
  
  /*rip redistributeTable*/
  void    init_ripRedistributeTable(void);

//rip private mib declaration
Netsnmp_Node_Handler do_ripUpdateTime;
Netsnmp_Node_Handler do_ripRouterVersion;
Netsnmp_Node_Handler do_ripStatisticsReset;
Netsnmp_Node_Handler do_ripTimeoutTime;
Netsnmp_Node_Handler do_ripGarbageCollectionTime;
Netsnmp_Node_Handler do_ripRoutingProcessStatus;

Netsnmp_Node_Handler do_ripDefaultMetric;
Netsnmp_Node_Handler do_ripMaxPrefix;
Netsnmp_Node_Handler do_ripDefaultInformationOriginate;
Netsnmp_Node_Handler do_ripDefaultDistance;
Netsnmp_Node_Handler do_ripClearByType;

FindVarMethod   var_ripNeighborTable;
WriteMethod     write_ripNeighborStatus;

FindVarMethod   var_ripInstabilityPreventingTable;
WriteMethod     write_ripSplitHorizonStatus;


FindVarMethod   var_ripNetworkByInterfaceTable;
WriteMethod     write_ripNetworkByInterfaceStatus;


FindVarMethod   var_ripNetworkByInetAddrTable;
WriteMethod     write_ripNetworkByInetAddrStatus;

FindVarMethod   var_ripPassiveInterfaceTable;
WriteMethod     write_ripPassiveInterfaceStatus;

FindVarMethod   var_ripRouteClearByNetworkTable;
WriteMethod     write_ripRouteClearByNetworkStatus;


FindVarMethod   var_ripDistanceTable;
WriteMethod     write_ripDistanceValue;
WriteMethod     write_ripDistanceAliseName;
WriteMethod     write_ripDistanceRowStatus;

FindVarMethod   var_ripRedistribueTable;
WriteMethod     write_ripRedistributeMetric;
WriteMethod     write_ripRedistributeStatus;
WriteMethod     write_ripRedistributeRmap;




#endif

