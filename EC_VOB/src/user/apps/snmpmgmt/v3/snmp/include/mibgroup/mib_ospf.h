/* =====================================================================================
 *  Module Name: MIB_OSPF.H
 *  Purpose :
 *  Notes:
 *  History :
 *  Modify: 2009.02.06 by Donny.li
* =====================================================================================
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_pal.h"
#include "netcfg_type.h"
#include <netinet/in.h>
     
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "snmp_mgr.h"

#define INET_ADDRESS_IPV4_SIZE 4
#define INET_ADDRESS_TYPE_IPV4 1
#define OSPF_SNMP_AREA_ID_FORMAT_DECIMAL    2

#define OSPF_SNMP_ROW_STATUS_ACTIVE               1
#define OSPF_SNMP_ROW_STATUS_NOTINSERVICE         2
#define OSPF_SNMP_ROW_STATUS_NOTREADY             3
#define OSPF_SNMP_ROW_STATUS_CREATEANDGO          4
#define OSPF_SNMP_ROW_STATUS_CREATEANDWAIT        5
#define OSPF_SNMP_ROW_STATUS_DESTROY              6

#define OSPF_SNMP_IF_PARAM_VALID                  1
#define OSPF_SNMP_IF_PARAM_INVALID              2

/* OSPF Area ID format */
#define OSPF_SNMP_AREA_ID_FORMAT_DEFAULT		 0
#define OSPF_SNMP_AREA_ID_FORMAT_ADDRESS		 1
#define OSPF_SNMP_AREA_ID_FORMAT_DECIMAL		 2

void init_ospfMultiProcSummaryAddrTable(void);
void init_ospfMultiProcRedistTable(void);
void init_ospfMultiProcessIfTable(void);
void init_ospfMultiProcVirtIfMd5Table(void);

#define SUMMARY_ADDR_MAX_SIZE                   4
#define SUMMARY_MASK_MAX_SIZE                   4
#define PREFIX_STRING_SIZE                      20


/**************************************************
 ********ospfMultiProcessSummaryAddressTable*************
 **************************************************
 */
#define OSPFPROCESSID               1

#define OSPFMULTIPROCSUMMARYADDRTYPE      1
#define OSPFMULTIPROCSUMMARYADDR          2
#define OSPFMULTIPROCSUMMARYPFXLEN             3
#define OSPFMULTIPROCSUMMARYSTATUS           4
#define OSPFMULTIPROCSUMMARYADDROIDNAMELENGTH   16


FindVarMethod   var_ospfMultiProcessSummaryAddressTable;
WriteMethod     write_ospfMultiProcSummaryAddrEntry;

/**************************************************
 ***************ospfMultiProcessIfTable*****************
 **************************************************
 */
#define OSPF_MULTI_PROCESS_VIRT_IF_AUTHKEY_LENGTH_MAX     8

#define OSPFMULTIPROCESSVIRTIFAREAID                    1
#define OSPFMULTIPROCESSVIRTIFNEIGHBOR                  2
#define OSPFMULTIPROCESSVIRTIFTRANSITDELAY              3
#define OSPFMULTIPROCESSVIRTIFRETRANSINTERVAL           4
#define OSPFMULTIPROCESSVIRTIFHELLOINTERVAL             5
#define OSPFMULTIPROCESSVIRTIFRTRDEADINTERVAL           6
#define OSPFMULTIPROCESSVIRTIFSTATE                     7
#define OSPFMULTIPROCESSVIRTIFEVENTS                    8
#define OSPFMULTIPROCESSVIRTIFAUTHKEY                   9
#define OSPFMULTIPROCESSVIRTIFSTATUS                    10
#define OSPFMULTIPROCESSVIRTIFAUTHTYPE                  11

#define OSPF_MULTI_PROCESS_VIRT_IF_TRANSITDELAY_MIN      1
#define OSPF_MULTI_PROCESS_VIRT_IF_TRANSITDELAY_MAX      65535
#define OSPF_MULTI_PROCESS_VIRT_IF_RETRANSINTERVAL_MIN   1
#define OSPF_MULTI_PROCESS_VIRT_IF_RETRANSINTERVAL_MAX   65535
#define OSPF_MULTI_PROCESS_VIRT_IF_HELLOINTERVAL_MIN     1
#define OSPF_MULTI_PROCESS_VIRT_IF_HELLOINTERVAL_MAX     65535
#define OSPF_MULTI_PROCESS_VIRT_IF_RTRDEADINTERVAL_MIN   1
#define OSPF_MULTI_PROCESS_VIRT_IF_RTRDEADINTERVAL_MAX   65535
#define OSPF_MULTI_PROCESS_VIRT_IF_STATUS_MIN            1
#define OSPF_MULTI_PROCESS_VIRT_IF_STATUS_MAX            6
#define OSPF_MULTI_PROCESS_VIRT_IF_AUTHTYPE_MIN          0
#define OSPF_MULTI_PROCESS_VIRT_IF_AUTHTYPE_MAX          2

#define OSPF_AREA_VLINK_DEAD_INTERVAL                   (1 << 0) 
#define OSPF_AREA_VLINK_HELLO_INTERVAL                  (1 << 1) 
#define OSPF_AREA_VLINK_RETRANSMIT_INTERVAL             (1 << 2) 
#define OSPF_AREA_VLINK_TRANSMIT_DELAY                  (1 << 3)
#define OSPF_AREA_VLINK_AUTHENTICATION                  (1 << 4)
#define OSPF_AREA_VLINK_AUTHENTICATIONKEY               (1 << 5)
#define OSPF_AREA_VLINK_MESSAGEDIGESTKEY                (1 << 6)

#define OSPF_AREA_ID_FORMAT_DEFAULT      0
#define OSPF_AREA_ID_FORMAT_ADDRESS      1
#define OSPF_AREA_ID_FORMAT_DECIMAL      2
void
init_ospfMultiProcessVirtIfTable(void);

FindVarMethod   var_ospfMultiProcessVirtIfTable;
WriteMethod     write_ospfMultiProcVirtIfEntry;
WriteMethod     write_ospfMultiProcVirtIfEntryChar;

/**************************************************
 ***************ospfMultiProcessNbrTable*****************
 **************************************************
 */

#define OSPFMULTIPROCESSVIRTNBRAREA                     1
#define OSPFMULTIPROCESSVIRTNBRRTRID                    2
#define OSPFMULTIPROCESSVIRTNBRIPADDR                   3
#define OSPFMULTIPROCESSVIRTNBROPTIONS                  4
#define OSPFMULTIPROCESSVIRTNBRSTATE                    5
#define OSPFMULTIPROCESSVIRTNBREVENTS                   6
#define OSPFMULTIPROCESSVIRTNBRLSRETRANSQLEN            7


#define OSPF_AREA_VLINK_DEAD_INTERVAL                   (1 << 0) 
#define OSPF_AREA_VLINK_HELLO_INTERVAL                  (1 << 1) 
#define OSPF_AREA_VLINK_RETRANSMIT_INTERVAL             (1 << 2) 
#define OSPF_AREA_VLINK_TRANSMIT_DELAY                  (1 << 3)
#define OSPF_AREA_VLINK_AUTHENTICATION                  (1 << 4)
#define OSPF_AREA_VLINK_AUTHENTICATIONKEY               (1 << 5)
#define OSPF_AREA_VLINK_MESSAGEDIGESTKEY                (1 << 6)


void
init_ospfMultiProcessVirtNbrTable(void);

FindVarMethod   var_ospfMultiProcessVirtNbrTable;



/**************************************************
 ***************ospfMultiProcessIfTable*****************
 **************************************************
 */
#define OSPF_MULTI_PROCESS_IF_AUTHKEY_LENGTH_MAX     8

#define OSPFMULTIPROCESSIFIPADDRESSTYPE             1
#define OSPFMULTIPROCESSIFIPADDRESS                 2
#define OSPFMULTIPROCESSIFCOST                      3
#define OSPFMULTIPROCESSIFMTU                       4
#define OSPFMULTIPROCESSIFMTUIGNORE                 5
#define OSPFMULTIPROCESSIFAREAID                    6
#define OSPFMULTIPROCESSIFRTRPRIORITY               7
#define OSPFMULTIPROCESSIFTRANSITDELAY              8
#define OSPFMULTIPROCESSIFRETRANSINTERVAL           9
#define OSPFMULTIPROCESSIFHELLOINTERVAL             10
#define OSPFMULTIPROCESSIFRTRDEADINTERVAL           11
#define OSPFMULTIPROCESSIFSTATE                     12
#define OSPFMULTIPROCESSIFDESIGNATEDROUTER          13
#define OSPFMULTIPROCESSIFBACKUPDESIGNATEDROUTER    14
#define OSPFMULTIPROCESSIFEVENTS                    15
#define OSPFMULTIPROCESSIFAUTHKEY                   16
#define OSPFMULTIPROCESSIFSTATUS                    17
#define OSPFMULTIPROCESSIFAUTHTYPE                  18
#define OSPFMULTIPROCESSIFDESIGNATEDROUTERID        19
#define OSPFMULTIPROCESSIFBACKUPDESIGNATEDROUTERID  20

#define OSPF_MULTI_PROCESS_IF_COST_MIN              1
#define OSPF_MULTI_PROCESS_IF_COST_MAX              65535
#define OSPF_MULTI_PROCESS_IF_MTU_MIN               576
#define OSPF_MULTI_PROCESS_IF_MTU_MAX               65535
#define OSPF_MULTI_PROCESS_IF_MUTIGNORE_MIN         0
#define OSPF_MULTI_PROCESS_IF_MUTIGNORE_MAX         1   
#define OSPF_MULTI_PROCESS_IF_PRIORITY_MIN          0
#define OSPF_MULTI_PROCESS_IF_PRIORITY_MAX          255
#define OSPF_MULTI_PROCESS_IF_TRANSITDELAY_MIN      1
#define OSPF_MULTI_PROCESS_IF_TRANSITDELAY_MAX      65535
#define OSPF_MULTI_PROCESS_IF_RETRANSINTERVAL_MIN   1
#define OSPF_MULTI_PROCESS_IF_RETRANSINTERVAL_MAX   65535
#define OSPF_MULTI_PROCESS_IF_HELLOINTERVAL_MIN     1
#define OSPF_MULTI_PROCESS_IF_HELLOINTERVAL_MAX     65535
#define OSPF_MULTI_PROCESS_IF_RTRDEADINTERVAL_MIN   1
#define OSPF_MULTI_PROCESS_IF_RTRDEADINTERVAL_MAX   65535
#define OSPF_MULTI_PROCESS_IF_STATE_MIN             1
#define OSPF_MULTI_PROCESS_IF_STATE_MAX             7
#define OSPF_MULTI_PROCESS_IF_STATUS_MIN            1
#define OSPF_MULTI_PROCESS_IF_STATUS_MAX            6
#define OSPF_MULTI_PROCESS_IF_AUTHTYPE_MIN          0
#define OSPF_MULTI_PROCESS_IF_AUTHTYPE_MAX          2





FindVarMethod   var_ospfMultiProcessIfTable;
WriteMethod     write_ospfMultiProcIfEntry;
WriteMethod     write_ospfMultiProcIfEntryIP;
WriteMethod     write_ospfMultiProcIfEntryChar;

/************************************************
 ************ospfMultiProcessVirtIfAuthMd5Table*********
 **************************************************
 */

#define OSPFMULTIPROCVIRTIFAUTHMD5AREAID        1
#define OSPFMULTIPROCVIRTIFAUTHMD5NEIGHBOR      2
#define OSPFMULTIPROCVIRTIFAUTHMD5KEYID         3
#define OSPFMULTIPROCVIRTIFAUTHMD5KEY           4
#define OSPFMULTIPROCVIRTIFAUTHMD5KEYLENGTH     16
#define OSPFMULTIPROCVIRTIFAUTHMD5KEYIDMAX      255
#define OSPFMULTIPROCVIRTIFAUTHMD5KEYIDMIN      1



FindVarMethod   var_ospfMultiProcVirtIfAuthMd5Table;
WriteMethod     write_MultiProcVirtIfAuthMd5Key;


#define OSPF_MULTI_PROCESS_REDISTRIBUTE_PROTOCOL               1
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_TYPE            2
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC                 3
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG                    4
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_FILTER_LIST_NAME       5
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS                 6
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTE_MAP              7

#define OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_MIN             0
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_METRIC_MAX             16777214
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG_MIN                0
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_TAG_MAX                0xFFFFFFFF
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS_MIN             1
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_STATUS_MAX             6
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTEMAP_NAME_MIN      1
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_ROUTEMAP_NAME_MAX      15
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_LIST_NAME_MIN          1
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_LIST_NAME_MAX          16

#define OSPF_MULTI_PROCESS_REDISTRIBUTE_RIP         1
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_STATIC      2
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_CONNECTED   3
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_BGP         4
#define OSPF_MULTI_PROCESS_REDISTRIBUTE_MAX         5

#define OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE_1 1
#define OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE_2 2

FindVarMethod   var_ospfMultiProcessRedistTable;
WriteMethod     write_ospfMultiProcRedistributeEntryChar;
WriteMethod     write_ospfMultiProcRedistributeEntry;


#define OspfInetNetworkType              1

#define OSPFMULTIPROCESSNETWORKAREAADDRESSTYPE  1
#define OSPFMULTIPROCESSNETWORKAREAADDRESS      2
#define OSPFMULTIPROCESSNETWORKAREAPFXLEN       3
#define OSPFMULTIPROCESSNETWORKAREAAREAID       4
#define OSPFMULTIPROCESSNETWORKAREASTATUS       5
#define OSPFMULTIPROCESSNETWORKAREAAREAID2      6

#define OSPFMULTIPROCESSAREAID              1
#define OSPFMULTIPROCESSAUTHTYPE            2
#define OSPFMULTIPROCESSIMPORTASEXTERN      3
#define OSPFMULTIPROCESSSPFRUNS             4
#define OSPFMULTIPROCESSAREABDRRTRCOUNT     5
#define OSPFMULTIPROCESSASBDRRTRCOUNT       6
#define OSPFMULTIPROCESSAREALSACOUNT        7
#define OSPFMULTIPROCESSAREALSACKSUMSUM     8
#define OSPFMULTIPROCESSAREASUMMARY         9
#define OSPFMULTIPROCESSAREASTATUS          10

#define OSPFMULTIPROCESSSTUBAREAID      1
#define OSPFMULTIPROCESSSTUBTOS         2
#define OSPFMULTIPROCESSSTUBMETRIC      3
#define OSPFMULTIPROCESSSTUBSTATUS      4

#define OSPFMULTIPROCESSAREAAGGREGATEAREAID      1
#define OSPFMULTIPROCESSAREAAGGREGATELSDBTYPE    2
#define OSPFMULTIPROCESSAREAAGGREGATENET         3 
#define OSPFMULTIPROCESSAREAAGGREGETEMASK        4
#define OSPFMULTIPROCESSAREAAGGREGETESTATUS      5
#define OSPFMULTIPROCESSAREAAGGREGATEEFFECT      6

#define OSPFMULTIPROCESSNSSAAREAID                      1
#define OSPFMULTIPROCESSNSSATRANSLATORROLE              2
#define OSPFMULTIPROCESSNSSAREDISTRIBUTESTATUS          3 
#define OSPFMULTIPROCESSNSSAORIGINATEDEFAULTINFOSTATUS  4
#define OSPFMULTIPROCESSNSSAMETRICTYPE                  5
#define OSPFMULTIPROCESSNSSAMETRIC                      6
#define OSPFMULTIPROCESSNSSASTATUS                      7
#define OSPFMULTIPROCESSNSSATRANSLATORSTATE             8

#define OSPF_SNMP_NO_AREA_SUMMARY		 1
#define OSPF_SNMP_SEND_AREA_SUMMARY		 2

#define OSPF_SNMP_AREA_IMPORT_EXTERNAL	1
#define OSPF_SNMP_AREA_IMPORT_NO_EXTERNAL 2
#define OSPF_SNMP_AREA_IMPORT_NSSA	    3
#define OspfStubAreaTosValue             0
#define OspfAreaAggregateLsdbSummaryLink    3
#define OSPF_SNMP_RANGE_AREA_ADVERTISEMATCHING       1
#define OSPF_SNMP_RANGE_AREA_DONOTADVERTISEMATCHING  2
#define OSPF_SNMP_NSSA_TRANSLATE_NEVER	1
#define OSPF_SNMP_NSSA_TRANSLATE_ALWAYS	2
#define OSPF_SNMP_NSSA_TRANSLATE_CANDIDATE	3
#define OSPF_SNMP_NSSA_REDISTRIBUTE_ENABLED          1
#define OSPF_SNMP_NSSA_REDISTRIBUTE_DISABLED         2
#define OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_ENABLED     1
#define OSPF_SNMP_NSSA_ORIGINATE_DEFAULT_DISABLED    2
#define OSPF_SNMP_NSSA_EXTERNAL_METRIC_TYPE_1			 1
#define OSPF_SNMP_NSSA_EXTERNAL_METRIC_TYPE_2			 2


void
init_ospfMultiProcessAreaTable(void);

void
init_ospfMultiProcessNetworkAreaAddressTable(void);

void
init_ospfMultiProcessStubAreaTable(void);

void
init_ospfMultiProcessAreaAggregateTable(void);

void
init_ospfMultiProcessNssaTable(void);

FindVarMethod   var_ospfMultiProcessNetworkAreaAddressTable;
WriteMethod     write_ospfMultiProcessNetworkAreaStatus;
WriteMethod     write_ospfMultiProcessNetworkAreaId;
WriteMethod     write_ospfMultiProcessNetworkAreaId2;

FindVarMethod   var_ospfMultiProcessAreaTable;
WriteMethod     write_ospfMultiProcessAreaSummary;
WriteMethod     write_ospfMultiProcessAreaNssaTranslatorRole;

FindVarMethod   var_ospfMultiProcessStubAreaTable;
WriteMethod     write_ospfMultiProcessStubAreaMetric;
WriteMethod     write_ospfMultiProcessStubAreaStatus;

FindVarMethod   var_ospfMultiProcessAreaAggregateTable;
WriteMethod     write_ospfMultiProcessAreaAggregateStatus;
WriteMethod     write_ospfMultiProcessAreaAggregateEffect;

FindVarMethod   var_ospfMultiProcessNssaTable;
WriteMethod     write_ospfMultiProcessNssaTranslatorRole;
WriteMethod     write_ospfMultiProcessNssaRedistributeStatus;
WriteMethod     write_ospfMultiProcessNssaOriginateDefaultInfoStatus;
WriteMethod     write_ospfMultiProcessNssaMetricType;
WriteMethod     write_ospfMultiProcessNssaMetric;
WriteMethod     write_ospfMultiProcessNssaStatus;



/* wang.tong add*/
/* OSPF MIB ospfIfParamTable. */
#define OSPF_IFPARAM_IFINDEX                   1
#define OSPF_IFPARAM_IPADDRESS           2
#define OSPF_IFPARAM_TOS                           3
#define OSPF_IFPARAM_COST                        4
#define OSPF_IFPARAM_STATUS                   5

#define MIN_OSPF_IFPARAM_COST                1L
#define MAX_OSPF_IFPARAM_COST               65535L
#define DEFAULT_OSPF_IFPARAM_COST     10

/*
 * function declarations 
 */
void        init_ospfIfParamTable(void);
FindVarMethod   var_ospfIfParamTable;
WriteMethod     write_ospfIfParamCost;
WriteMethod     write_ospfIfParamStatus;
/* end  OSPF MIB ospfIfParamTable */

/*Ospf mib ospfMultiProcessSystemTable */

#define OSPF_MULTI_PROCESS_ID                                  1
#define OSPF_MULTI_PROCESS_ROUTER_ID_TYPE                      2
#define OSPF_MULTI_PROCESS_RFC1583_COMPATIBLE_STATE            3
#define OSPF_MULTI_PROCESS_AUTO_COST                           4
#define OSPF_MULTI_PROCESS_ORIGINATE_DEFAULT_ROUTE             5
#define OSPF_MULTI_PROCESS_ADVERTISE_DEFAULT_ROUTE             6
#define OSPF_MULTI_PROCESS_EXTERNAL_METRIC_TYPE                7
#define OSPF_MULTI_PROCESS_DEFAULT_EXTERNAL_METRIC             8
#define OSPF_MULTI_PROCESS_SPF_HOLD_TIME                       9
#define OSPF_MULTI_PROCESS_AREA_NUMBER                         10
#define OSPF_MULTI_PROCESS_AREA_LIMIT                          11
#define OSPF_MULTI_PROCESS_CAPABILITY_OPAQUE                   12
#define OSPF_MULTI_PROCESS_OVERFLOW_DATABASE_NUMBER            13
#define OSPF_MULTI_PROCESS_OVERFLOW_DATABASE_TYPE              14
#define OSPF_MULTI_PROCESS_OVERFLOW_EXTERNAL_DB_MAXSIZE        15
#define OSPF_MULTI_PROCESS_OVERFLOW_EXTERNAL_DB_WAITTIME       16
#define OSPF_MULTI_PROCESS_SYSTEM_STATUS                       17
#define OSPF_MULTI_PROCESS_ROUTER_ID                           18
#define OSPF_MULTI_PROCESS_ADMIN_STAT                          19
#define OSPF_MULTI_PROCESS_VERSION_NUMBER                      20
#define OSPF_MULTI_PROCESS_AREA_BDR_RTR_STATUS                 21
#define OSPF_MULTI_PROCESS_AS_BDR_RTR_STATUS                   22
#define OSPF_MULTI_PROCESS_EXTERN_LSA_COUNT                    23
#define OSPF_MULTI_PROCESS_EXTERN_LSA_CKSUM_SUM                24
#define OSPF_MULTI_PROCESS_ORIGINATE_NEW_LSAS                  25
#define OSPF_MULTI_PROCESS_RX_NEW_LSAS              26
#define OSPF_MULTI_PROCESS_RESTART_SUPPORT                     27
#define OSPF_MULTI_PROCESS_RESTART_INTERVAL                    28
#define OSPF_MULTI_PROCESS_RESTART_STATUS                      29
#define OSPF_MULTI_PROCESS_AS_LSA_COUNT                        30
#define OSPF_MULTI_PROCESS_SPF_DELAY_TIME                      31
#define OSPF_MULTI_PROCESS_DEFAULT_ROUTE_MAP                   32
#define OSPF_MULTI_PROCESS_DEFAULT_METRIC                   33

#define OSPF_MULTI_PROCESS_AUTOCOST_MIN   1
#define OSPF_MULTI_PROCESS_AUTOCOST_MAX   4294967

#define OSPF_DEFAULTEXTERNALMETRIC_MIN    -1
#define OSPF_DEFAULTEXTERNALMETRIC_MAX    16777214

#define OSPF_DEFAULTMETRIC_MIN    -1
#define OSPF_DEFAULTMETRIC_MAX    16777214

#define OSPF_SPF_TIMER_MIN    0
#define OSPF_SPF_TIMER_MAX     2147483647U

#define OSPF_AREA_LIMIT_MIN    1
#define OSPF_AREA_LIMIT_MAX    4294967294U
void        init_ospfMultiProcessSystemTable(void);
FindVarMethod   var_ospfMultiProcessSystemTable;
WriteMethod write_ospfMultiProcessSystemCompatibleRfc1853;
WriteMethod write_ospfMultiProcessSystemAutoCost;
WriteMethod write_ospfMultiProcessSystemOriginateDefaultRoute;
WriteMethod write_ospfMultiProcessSystemAdvertiseDefaultRoute;
WriteMethod write_ospfMultiProcessSystemExternalMetricType;
WriteMethod write_ospfMultiProcessSystemDefaultExternalMetric;
WriteMethod write_ospfMultiProcessSystemSpfHoldTimer;
WriteMethod write_ospfMultiProcessSystemAreaLimit;
WriteMethod write_ospfMultiProcessSystemRouterId;
WriteMethod write_ospfMultiProcessSystemStatus;
WriteMethod write_ospfMultiProcessSystemSpfdelayTimer;
WriteMethod write_ospfMultiProcessSystemDefaultMetric;
/*Ospf mib ospfMultiProcessSystemTable . end */



/*Ospf mib ospfMultiProcessNbrTable */

#define OSPF_MULTI_PROCESS_NBR_IPADDR                    1
#define OSPF_MULTI_PROCESS_NBR_RTR_ID                    2
#define OSPF_MULTI_PROCESS_NBR_OPTIONS                   3
#define OSPF_MULTI_PROCESS_NBR_PRIORITY                  4
#define OSPF_MULTI_PROCESS_NBR_STATE                     5
#define OSPF_MULTI_PROCESS_NBR_EVENTS                    6
#define OSPF_MULTI_PROCESS_NBR_LS_RETRANS_QLEN           7

void        init_ospfMultiProcessNbrTable(void);
FindVarMethod   var_ospfMultiProcessNbrTable;

/*Ospf mib ospfMultiProcesslsdbTable */
#define OSPF_MULTI_PROCESS_LSDB_AREA_ID                 1        
#define OSPF_MULTI_PROCESS_LSDB_TYPE                        2
#define OSPF_MULTI_PROCESS_LSDB_LSID                         3
#define OSPF_MULTI_PROCESS_LSDB_ROUTER_ID           4
#define OSPF_MULTI_PROCESS_LSDB_SQUENCE              5            
#define OSPF_MULTI_PROCESS_LSDB_AGE                          6
#define OSPF_MULTI_PROCESS_LSDB_CHK_SUM               7
#define OSPF_MULTI_PROCESS_LSDB_ADVERTISE           8

void        init_ospfMultiProcessLsdbTable(void);
FindVarMethod   var_ospfMultiProcessLsdbTable;


/*Ospf mib ospfMultiProcessExtlsdbTable */
#define OSPF_MULTI_PROCESS_EXT_LSDB_TYPE                     1
#define OSPF_MULTI_PROCESS_EXT_LSDB_LSID                      2
#define OSPF_MULTI_PROCESS_EXT_LSDB_ROUTER_ID        3
#define OSPF_MULTI_PROCESS_EXT_LSDB_SQUENCE           4
#define OSPF_MULTI_PROCESS_EXT_LSDB_AGE                       5
#define OSPF_MULTI_PROCESS_EXT_LSDB_CHK_SUM            6
#define OSPF_MULTI_PROCESS_EXT_LSDB_ADVERTISE        7

void        init_ospfMultiProcessExtLsdbTable(void);
FindVarMethod   var_ospfMultiProcessExtLsdbTable;


/*Ospf mib ospfMultiProcessIfAuthMd5Table */
#define OSPF_MULTI_PROCESS_IF_AUTH_MD5_ADDRESS_TYPE         1
#define OSPF_MULTI_PROCESS_IF_AUTH_MD5_ADDRESS                      2
#define OSPF_MULTI_PROCESS_IF_AUTH_MD5_KEY_ID                           3
#define OSPF_MULTI_PROCESS_IF_AUTH_MD5_KEY                                 4

void        init_ospfMultiProcessIfAuthMd5Table(void);
FindVarMethod   var_ospfMultiProcessIfAuthMd5Table;
WriteMethod write_ospfMultiProcessIfAuthMd5Key;

/*end. wang.tong*/

