#ifndef SNMP_MGR_H
#define SNMP_MGR_H

/* system
 */
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "leaf_2576.h"
#include "leaf_3411.h"
#include "leaf_3412.h"
#include "leaf_3413n.h"
#include "leaf_3413t.h"
#include "leaf_3414.h"
#include "leaf_3415.h"
#include "leaf_2021.h"
#include "leaf_2819.h"
#include "sysfun.h"

/* common library
 */
#include "l_stdlib.h"

/* core layer
 */
#include "trap_mgr.h"

/* SNMP itself
 */
#include "snmp_type.h"

#ifndef SNMP_MGR_MAX
#define SNMP_MGR_MAX(x1,x2) ((x1) >= (x2) ? (x1) : (x2))
#endif

/*debug flage, release code must set FALSE, test can set TRUE*/
#define SNMP_MGR_DEBUG_CONTROL TRUE

/* for error code*/
#define SNMP_MGR_ERROR_OK                           0
#define SNMP_MGR_ERROR_EXCEED_LIMIT                 0x80000001
#define SNMP_MGR_ERROR_MEM_ALLOC_FAIL               0x80000002
#define SNMP_MGR_ERROR_PARAMETER_NOT_MATCH          0x80000003
#define SNMP_MGR_ERROR_FAIL                         0x80000004
#define SNMP_MGR_ERROR_NO_ENTRY_EXIST               0x80000005

/* for rmon
 */
#define SNMP_MGR_MIN_RMON_ALARM_RISING_THRESHOLD    0
#define SNMP_MGR_MAX_RMON_ALARM_RISING_THRESHOLD    2147483647
#define SNMP_MGR_MIN_RMON_ALARM_FALLING_THRESHOLD   0
#define SNMP_MGR_MAX_RMON_ALARM_FALLING_THRESHOLD   2147483647

#define SNMP_MGR_MAX_SECURITY_NAME_LEN              SNMP_MGR_MAX(SYS_ADPT_MAX_COMM_STR_NAME_LEN, MAXSIZE_usmUserSecurityName)
#define SNMP_MGR_MAX_PASSWORD_LEN 32
#define SNMP_MGR_MAX_SEND_TRAP_RETRY_TIMES   10

/* In linux environment, the startup time of network layer of switch is
 * usually earlier than PC (After PC to send Gratuitous ARP, it is
 * ready).
 * When traps are sent at switch startup, switch will send 3 ARP
 * requests to request the MAC address of the trap server. If PC is
 * ready after the ARP requests timeout, the traps will lost. So that
 * we need to delay few seconds to wait for PC ready after the
 * spanning tree of the switch is stable.
 */
#define SNMP_MGR_SEND_TRAP_DELAY_TIME 10 /* in seconds */

/* for max number of community, user, group and view */

#define SNMP_MGR_MAX_NBR_OF_SNMPV3_USER              16
#define SNMP_MGR_MAX_NBR_OF_SNMPV3_SECURITYTOGROUP   64
#define SNMP_MGR_MAX_NBR_OF_SNMPV3_GROUP             26 /* community*2 +16 = 26*/
#define SNMP_MGR_MAX_NBR_OF_SNMPV3_VIEW              32
#define SNMP_MGR_MAX_NBR_OF_TARGET_ADDR_ENTRY        SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
#define SNMP_MGR_MAX_NBR_OF_TARGET_ADDR_PARAMS_ENTRY SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
#define SNMP_MGR_MAX_NBR_OF_NOTIFY_ENTRY             SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
#define SNMP_MGR_MAX_NBR_OF_NOTIFY_FILTER_PROFILE_ENTRY SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
#define SNMP_MGR_MAX_NBR_OF_NOTIFY_FILTER_ENTRY         SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
#define SNMP_MGR_MAX_NBR_OF_REMOTE_ENGINEID          5
#define SNMP_MGR_MAX_NBR_OF_REMOTE_USER              5


#define MAXSIZE_VIEW_SUBTREE 64
#define MAXSIZE_VIEW_MASK 64
#define MIN_SNMPV3_PASSWORD_LEN 8
#define MAX_SNMPV3_PASSWORD_LEN 32

#define SNMP_MGR_SNMPV3_MD5_KEY_LEN 16
#define SNMP_MGR_SNMPV3_SHA_KEY_LEN 20
#define SNMP_MGR_SNMPV3_AES192_KEY_LEN 24
#define SNMP_MGR_SNMPV3_AES256_KEY_LEN 32
#define SNMP_MGR_SNMPV3_MAX_PRIV_KEY_LEN SNMP_MGR_SNMPV3_AES256_KEY_LEN
#define SNMP_MGR_SNMPV3_MAX_KEY_LEN 40
#define SNMP_MGR_SNMPV3_AES_STR_LEN 3
#define SNMP_MGR_SNMPV3_AES128 128
#define SNMP_MGR_SNMPV3_AES192 192
#define SNMP_MGR_SNMPV3_AES256 256
#define SNMP_MGR_SNMPV3_3DES   256

/* Define the default timeout and retry count of Inform Request*/
#define SNMP_MGR_INFORM_REQUEST_TIMEOUT            1500
#define SNMP_MGR_INFORM_REQUEST_RETRY_CNT          3
#define SNMP_MGR_INFORM_PORT                       1042
#define SNMP_MGR_TADDR_IPV6LINKLOCAL_LEN 22
#define SNMP_MGR_TADDR_IPV6_LEN          18
#define SNMP_MGR_TADDR_IPV4_LEN          6
#define SNMP_MGR_TADDR_PORT_LEN          2
#define SNMP_MGR_TADDR_SCOPE_LEN         4
#define SNMP_MGR_TADDR_IPV6LINKLOCAL_SHIFT 6

/* added for AES extend key length
 */
#define SNMP_MGR_MAX_AES256_KEY_LEN 32

/* Convert UI64_T to counter64 */
#define SNMP_MGR_UI64_T_TO_COUNTER64(c64Output,ui64_tInputValue) \
{ \
    c64Output.high=(UI32_T)(ui64_tInputValue>>32); \
    c64Output.low =(UI32_T)ui64_tInputValue; \
}

/* Define SNMP v3  security model  type.
 */
 typedef enum
{
 SNMP_MGR_SNMPV3_MODEL_ANY   =    0,
 SNMP_MGR_SNMPV3_MODEL_V1    =    1, /*SNMP_SEC_MODEL_SNMPv1*/
 SNMP_MGR_SNMPV3_MODEL_V2C   =    2, /*SNMP_SEC_MODEL_SNMPv2c*/
 SNMP_MGR_SNMPV3_MODEL_V3    =    3 /*SNMP_SEC_MODEL_USM*/
}SNMP_MGR_Snmpv3_Model_T;

/* Define SNMPV2c community access right
*/
typedef enum
{
    SNMP_MGR_ACCESS_RIGHT_GROUP_SPECIFIC = 0,
    SNMP_MGR_ACCESS_RIGHT_READ_ONLY  = 1,
    SNMP_MGR_ACCESS_RIGHT_READ_WRITE = 2
} SNMP_MGR_Snmp_Comm_Access_Right_T;


/* Define SNMP v3 auth  type.
 */
typedef enum
{
    SNMP_MGR_SNMPV3_AUTHTYPE_NONE = 0,
    SNMP_MGR_SNMPV3_AUTHTYPE_MD5 = 1,
    SNMP_MGR_SNMPV3_AUTHTYPE_SHA = 2
}  SNMP_MGR_V3_Auth_Type_T;

/* Define SNMP v3 priv  type.
 */
typedef enum
{
    SNMP_MGR_SNMPV3_PRIVTYPE_NONE   = 0,
    SNMP_MGR_SNMPV3_PRIVTYPE_DES    = 1,
    SNMP_MGR_SNMPV3_PRIVTYPE_AES128 = 2,
    SNMP_MGR_SNMPV3_PRIVTYPE_AES192 = 3,
    SNMP_MGR_SNMPV3_PRIVTYPE_AES256 = 4,
    SNMP_MGR_SNMPV3_PRIVTYPE_3DES   = 5

} SNMP_MGR_V3_Priv_Type_T;

/* definitions of command in SNMP which will be used in ipc message
 */
enum
{
    SNMP_MGR_IPCCMD_SETSNMPCOMMUNITYSTATUS,
    SNMP_MGR_IPCCMD_SETTRAPDESTADDRESSBYINDEX,
    SNMP_MGR_IPCCMD_SETDEFAULTSNMPENGINEID,
    SNMP_MGR_IPCCMD_GETSNMPSTATUS,
    SNMP_MGR_IPCCMD_CREATESNMPV3GROUP,
    SNMP_MGR_IPCCMD_CREATESNMPV3USER,
    SNMP_MGR_IPCCMD_CREATESNMPV3VIEW,
    SNMP_MGR_IPCCMD_DELETESNMPV3VIEW,
    SNMP_MGR_IPCCMD_DELETESNMPV3VIEWBYNAME,
    SNMP_MGR_IPCCMD_DELETESNMPV3GROUP,
    SNMP_MGR_IPCCMD_GETENGINEID,
    SNMP_MGR_IPCCMD_GETRUNNINGENGINEID,
    SNMP_MGR_IPCCMD_GETENGINEBOOTS,
    SNMP_MGR_IPCCMD_DELETESNMPV3USER,
    SNMP_MGR_IPCCMD_GETSNMPV3USER,
    SNMP_MGR_IPCCMD_GETNEXTSNMPV3USER,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPV3USER,
    SNMP_MGR_IPCCMD_GETNEXTSNMPV3GROUP,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPV3GROUP,
    SNMP_MGR_IPCCMD_GETNEXTSNMPV3VIEWNAME,
    SNMP_MGR_IPCCMD_GETNEXTSNMPV3VIEW,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPV3VIEW,
    SNMP_MGR_IPCCMD_SET_ENGINEID,
    SNMP_MGR_IPCCMD_GETNEXTSNMPCOMMUNITY,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPCOMMUNITY,
    SNMP_MGR_IPCCMD_CREATESNMPCOMMUNITY,
    SNMP_MGR_IPCCMD_REMOVESNMPCOMMUNITY,
    SNMP_MGR_IPCCMD_ENABLE_SNMP_AGENT,
    SNMP_MGR_IPCCMD_DISABLE_SNMP_AGENT,
    SNMP_MGR_IPCCMD_GET_AGENTSTATUS,
    SNMP_MGR_IPCCMD_GETRUNNINGSNMPAGENTSTATUS,
    SNMP_MGR_IPCCMD_GETNEXTTRAPRECEIVER,
    SNMP_MGR_IPCCMD_GETTRAPRECEIVERBYINDEX,
    SNMP_MGR_IPCCMD_SETTRAPRECEIVERCOMMSTRINGNAMEBYINDEX,
    SNMP_MGR_IPCCMD_SETTRAPRECEIVERSTATUSBYINDEX,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGTRAPRECEIVER,
    SNMP_MGR_IPCCMD_GETTRAPRECEIVER,
    SNMP_MGR_IPCCMD_DELETETRAPRECEIVER,
    SNMP_MGR_IPCCMD_DELETETRAPRECEIVERWITHTARGETADDRNAME,
    SNMP_MGR_IPCCMD_SETTRAPRECEVIER,
    SNMP_MGR_IPCCMD_SETTRAPRECEIVERWITHTARGETADDRNAME,
    SNMP_MGR_IPCCMD_CREATESNMPREMOTEUSER,
    SNMP_MGR_IPCCMD_DELETESNMPREMOTEUSER,
    SNMP_MGR_IPCCMD_GETSNMPREMOTEUSERENTRY,
    SNMP_MGR_IPCCMD_GETNEXTSNMPREMOTEUSERENTRY,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPREMOTEUSERENTRY,
    SNMP_MGR_IPCCMD_CREATEREMOTEENGINEID,
    SNMP_MGR_IPCCMD_DELETEREMOTEENGINEID,
    SNMP_MGR_IPCCMD_DELETEREMOTEENGINEIDENTRY,
    SNMP_MGR_IPCCMD_GETSNMPREMOTEENGINEIDENTRY,
    SNMP_MGR_IPCCMD_GETNEXTSNMPREMOTEENGINEIDENTRY,
    SNMP_MGR_IPCCMD_GETNEXTRUNNINGSNMPREMOTEENGINEIDENTRY,
    SNMP_MGR_IPCCMD_GETNEXTSNMPTARGETADDRTABLE,
    SNMP_MGR_IPCCMD_GETSNMPTARGETADDRTABLE,
    SNMP_MGR_IPCCMD_CREATERSNMPNOTIFYPROFILEFILTERTABLE,
    SNMP_MGR_IPCCMD_DELETESNMPNOTIFYPROFILEFILTERTABLE,
    SNMP_MGR_IPCCMD_SETLOGADMINSTATUS,
    SNMP_MGR_IPCCMD_GETNEXTSNMPNOTIFYPROFILEFILTERTABLE,
    SNMP_MGR_IPCCMD_GETNLMCFGLOGTABLE,
    SNMP_MGR_IPCCMD_SETLOGFILTERNAME,

    SNMP_MGR_IPCCMD_GET_RMONALARMTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONALARMTABLE,
    SNMP_MGR_IPCCMD_CREATE_RMONALARMENTRY,
    SNMP_MGR_IPCCMD_DELETE_RMONALARMENTRY,
    SNMP_MGR_IPCCMD_MODIFY_RMONALARMENTRY,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONALARMTABLE,

    SNMP_MGR_IPCCMD_GET_RMONEVENTTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONEVENTTABLE,
    SNMP_MGR_IPCCMD_CREATE_RMONEVENTENTRY,
    SNMP_MGR_IPCCMD_DELETE_RMONEVENTENTRY,
    SNMP_MGR_IPCCMD_MODIFY_RMONEVENTENTRY,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONEVENTTABLE,

    SNMP_MGR_IPCCMD_GET_RMONSTATISTICSTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONSTATISTICSTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONSTATISTICSTABLE_BYLPORT,
    SNMP_MGR_IPCCMD_CREATE_RMONSTATISTICSENTRY,
    SNMP_MGR_IPCCMD_DELETE_RMONSTATISTICSENTRY,
    SNMP_MGR_IPCCMD_MODIFY_RMONSTATISTICSENTRY,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONSTATISTICSTABLE_BYLPORT,

    SNMP_MGR_IPCCMD_GET_RMONHISTORYCONTROLTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONHISTORYCONTROLTABLE,
    SNMP_MGR_IPCCMD_GETNEXT_RMONHISTORYCONTROLTABLE_BYLPORT,
    SNMP_MGR_IPCCMD_CREATE_RMONHISTORYCONTROLENTRY,
    SNMP_MGR_IPCCMD_DELETE_RMONHISTORYCONTROLENTRY,
    SNMP_MGR_IPCCMD_MODIFY_RMONHISTORYCONTROLENTRY,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONHISTORYCONTROLTABLE_BYLPORT,
    SNMP_MGR_IPCCMD_GETNEXT_RMONHISTORYTABLE_BYCONTROLINDEX,

    SNMP_MGR_IPCCMD_PACKETS1H,
    SNMP_MGR_IPCCMD_OCTETS1H,
    SNMP_MGR_IPCCMD_BCASTS1H,
    SNMP_MGR_IPCCMD_MCASTS1H,
    SNMP_MGR_IPCCMD_FRAGMENTS1H,
    SNMP_MGR_IPCCMD_COLLISIONS1H,
    SNMP_MGR_IPCCMD_ERRORS1H,
    SNMP_MGR_IPCCMD_PACKETS6H,
    SNMP_MGR_IPCCMD_OCTETS6H,
    SNMP_MGR_IPCCMD_BCASTS6H,
    SNMP_MGR_IPCCMD_MCASTS6H,
    SNMP_MGR_IPCCMD_FRAGMENTS6H,
    SNMP_MGR_IPCCMD_COLLISIONS6H,
    SNMP_MGR_IPCCMD_ERRORS6H,
    SNMP_MGR_IPCCMD_SNMPNOTIFYCREATE,
    SNMP_MGR_IPCCMD_SNMPNOTIFYDEL,
    SNMP_MGR_IPCCMD_SNMPNOTIFYMODIFY,
    SNMP_MGR_IPCCMD_GETSNMPENABLEAUTHENTRAPS,
    SNMP_MGR_IPCCMD_SETSNMPENABLEAUTHENTRAPS,
    SNMP_MGR_IPCCMD_SETSNMPENABLELINKUPDOWNTRAPS,
    SNMP_MGR_IPCCMD_SETCLUSTERROLE,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONALARMDELETEDDEFAULTENTRY,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONSTATISTICSDELETEDDEFAULTENTRY_BYLPORT,
    SNMP_MGR_IPCCMD_GETNEXT_RUNNING_RMONHISTORYCONTROLDELETEDEFAULTENTRY_BYLPORT,
    SNMP_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,
    SNMP_MGR_IPCCMD_ASYN_REQSENDTRAP,
    SNMP_MGR_IPCCMD_ASYN_REQSENDTRAPOPTIONAL,
    SNMP_MGR_IPCCMD_ASYN_NOTIFYSTATPLGCHANGED,
    SNMP_MGR_IPCCMD_ASYN_NOTIFYSTATPLGSTABLED
};

typedef struct  SNMP_MGR_STATS_S
        {
        unsigned long   snmpInPkts;
        unsigned long   snmpInBadVersions;
        unsigned long   snmpInBadCommunityNames;
        unsigned long   snmpInBadCommunityUses;
        unsigned long   snmpInASNParseErrs;
        unsigned short  snmpEnableAuthTraps;
        unsigned long   snmpOutPkts;
        unsigned long   snmpInBadTypes;
        unsigned long   snmpInTooBigs;
        unsigned long   snmpInNoSuchNames;
        unsigned long   snmpInBadValues;
        unsigned long   snmpInReadOnlys;
        unsigned long   snmpInGenErrs;
        unsigned long   snmpInTotalReqVars;
        unsigned long   snmpInTotalSetVars;
        unsigned long   snmpInGetRequests;
        unsigned long   snmpInGetNexts;
        unsigned long   snmpInSetRequests;
        unsigned long   snmpInGetResponses;
        unsigned long   snmpInTraps;
        unsigned long   snmpOutTooBigs;
        unsigned long   snmpOutNoSuchNames;
        unsigned long   snmpOutBadValues;
        unsigned long   snmpOutReadOnlys;
        unsigned long   snmpOutGenErrs;
        unsigned long   snmpOutGetRequests;
        unsigned long   snmpOutGetNexts;
        unsigned long   snmpOutSetRequests;
        unsigned long   snmpOutGetResponses;
        unsigned long   snmpOutTraps;
        unsigned long   snmpSilentDrops;
        unsigned long   snmpProxyDrops;
#if 0
        unsigned long   usecStatsUnsupportedQoS;
        unsigned long   usecStatsNotInWindows;
        unsigned long   usecStatsUnknownUserNames;
        unsigned long   usecStatsWrongDigestValues;
        unsigned long   usecStatsUnknownContexts;
        unsigned long   usecStatsBadParameters;
        unsigned long   usecStatsUnauthorizedOperations;
        unsigned long   usecStats;
#endif
} SNMP_MGR_STATS_T;

typedef struct
{
    UI32_T if_index;
    UI32_T id;
    char data_source[512];
    UI32_T drop_events;
    UI32_T octets;
    UI32_T packets;
    UI32_T bcast_pkts;
    UI32_T mcast_pkts;
    UI32_T crc_align;
    UI32_T undersize;
    UI32_T oversize;
    UI32_T fragments;
    UI32_T jabbers;
    UI32_T collisions;
    UI32_T pkts_64 ;
    UI32_T pkts_65_127;
    UI32_T pkts_128_255;
    UI32_T pkts_256_511;
    UI32_T pkts_512_1023;
    UI32_T pkts_1024_1518;
    char owner[SYS_ADPT_MAX_RMON_OWNER_STR_LEN + 1];
    UI32_T status;
} SNMP_MGR_RmonStatisticsEntry_T;

typedef struct
{
    UI32_T if_index;
    UI32_T id;
    char data_source[512];
    UI32_T buckets_requested;
    UI32_T buckets_granted;
    UI32_T interval;
    char owner[SYS_ADPT_MAX_RMON_OWNER_STR_LEN + 1];
    UI32_T status;
} SNMP_MGR_RmonHistoryControlEntry_T;

typedef struct
{
    UI32_T control_index;
    UI32_T data_index;
    UI32_T start_interval;
    UI32_T drop_events;
    UI32_T octets;
    UI32_T packets;
    UI32_T bcast_pkts;
    UI32_T mcast_pkts;
    UI32_T crc_align;
    UI32_T undersize;
    UI32_T oversize;
    UI32_T fragments;
    UI32_T jabbers;
    UI32_T collisions;
    UI32_T utilization;
} SNMP_MGR_RmonHistoryEntry_T;

typedef struct
{
    UI32_T id;
    UI32_T interval;
    char variable[512];
    UI32_T sample_type;
    UI32_T value;
    UI32_T startup_alarm;
    UI32_T rising_threshold;
    UI32_T falling_threshold;
    UI32_T rising_event_index;
    UI32_T falling_event_index;
    char owner[SYS_ADPT_MAX_RMON_OWNER_STR_LEN + 1];
    UI32_T status;
} SNMP_MGR_RmonAlarmEntry_T;

typedef struct
{
    UI32_T id;
    char description[MAXSIZE_eventDescription + 1];
    UI32_T type;
    char community[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI32_T last_time_sent;
    char owner[SYS_ADPT_MAX_RMON_OWNER_STR_LEN + 1];
    UI32_T status;
} SNMP_MGR_RmonEventEntry_T;

typedef struct
{
    /* key - A unique value, greater than zero, to indentify a unique interface.
     */
    char snmpv3_group_name[MAXSIZE_vacmGroupName+1];
    UI8_T  snmpv3_group_context_prefix[MAXSIZE_vacmAccessContextPrefix+1];
    SNMP_MGR_Snmpv3_Model_T  snmpv3_group_model;
    UI32_T  snmpv3_group_security_level;

    char   snmpv3_group_readview [ MAXSIZE_vacmAccessReadViewName+1];
    char   snmpv3_group_writeview [ MAXSIZE_vacmAccessWriteViewName+1];
    char   snmpv3_group_notifyview[MAXSIZE_vacmAccessNotifyViewName+1];
    UI32_T snmpv3_group_storage_type;
    UI32_T snmpv3_group_status;

} SNMP_MGR_SnmpV3GroupEntry_T;


typedef struct
{

    UI8_T   snmpv3_user_engine_id[MAXSIZE_snmpEngineID];   /*key*/
    UI32_T  snmpv3_user_engineIDLen;
    char   snmpv3_user_name[MAXSIZE_usmUserName+1];  /*key*/

    SNMP_MGR_Snmpv3_Model_T  snmpv3_user_security_model;  /* key of vacmSecurityToGroupEntry */

    char   snmpv3_user_group_name[MAXSIZE_vacmGroupName+1];
    char   snmpv3_user_auth_password [ MAX_SNMPV3_PASSWORD_LEN+1];
    UI8_T   snmpv3_user_auth_key[SNMP_MGR_SNMPV3_SHA_KEY_LEN];
    /*SNMP_MGR_SNMPV3_SHA_KEY_LEN= 20 > SNMP_MGR_SNMPV3_MD5_KEY_LEN= 16 , use the largest one*/
    UI32_T   snmpv3_user_auth_key_len;
    char   snmpv3_user_priv_password [ MAX_SNMPV3_PASSWORD_LEN+1];
    /* for AES 192 and 256
     */
    UI8_T  snmpv3_user_priv_key[SNMP_MGR_SNMPV3_MAX_PRIV_KEY_LEN];
       /*SNMP_MGR_SNMPV3_SHA_KEY_LEN= 20 > SNMP_MGR_SNMPV3_MD5_KEY_LEN= 16 , use the largest one*/
    UI32_T  snmpv3_user_priv_key_len;
    UI32_T  snmpv3_user_security_level;
    SNMP_MGR_V3_Auth_Type_T snmpv3_user_auth_type;
    SNMP_MGR_V3_Priv_Type_T  snmpv3_user_priv_type;
    UI32_T snmpv3_user_storage_type;
    UI32_T snmpv3_user_status;
    BOOL_T  password_from_config;

} SNMP_MGR_SnmpV3UserEntry_T;


typedef struct
{

    char   snmpv3_view_name[MAXSIZE_vacmViewTreeFamilyViewName+1]; /*key*/
    char   snmpv3_view_subtree [ MAXSIZE_VIEW_SUBTREE+1];         /*key*/
    UI32_T   snmpv3_view_subtree_len;

    char   snmpv3_view_mask [ MAXSIZE_VIEW_MASK+1];
    char   snmpv3_wildcard_subtree[ MAXSIZE_VIEW_SUBTREE+1];

    UI32_T snmpv3_view_type;
    UI32_T snmpv3_view_storage_type;
    UI32_T snmpv3_view_status;

} SNMP_MGR_SnmpV3ViewEntry_T;

/* Define SNMP Community data type
 *
 * NOTES: 1. The SNMP community string can only be accessed by CLI and Web.
 *           SNMP management station CAN NOT access the SNMP community string.
 *        2. There is no MIB to define the SNMP community string.
 *        3. Status of each SNMP community is defined as following:
 *              - invalid(0): the entry of this community string is deleted/purged
 *              - enabled(1): this community string is enabled
 *              - disabled(2): this community string is disabled

 *           Set status to invalid(0) will delete/purge a community string.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *           Access right of community string could be
 *              - READ_ONLY(1)
 *              - READ_WRITE(2)
 *
 *        4. The total number of SNMP community string supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *        5. By default, two SNMM community string are configued:
 *              - "PUBLIC"      READ-ONLY       enabled
 *              - "PRIVATE"     READ-WRITE      enabled
 *        6. For any SNMP request command, all of enabled community string
 *           shall be used to authenticate/authorize if this SNMP request
 *           is legal/permit.
 */
typedef struct
{
    /* comm_string_name - (key) to specify a unique SNMP community string
     */
    char   comm_string_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI32_T  access_right;
    UI32_T  status;

} SNMP_MGR_SnmpCommunity_T;


/*Define the Rfc2576 COMMUNITY MIB Structure*/
typedef struct
{
    char   snmp_community_index[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    char   snmp_community_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    char   snmp_community_security_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI8_T   snmp_community_context_engine_id[MAXSIZE_snmpEngineID];
    UI32_T  snmp_community_engine_id_length;
    UI8_T   snmp_community_context_name[MAXSIZE_snmpCommunityContextName+1];
    UI8_T   snmp_community_transport_tag[MAXSIZE_snmpCommunityTransportTag+1];
    UI32_T  snmp_community_access_right;
    UI32_T  snmp_community_storage_type;
    UI32_T  snmp_community_status;
} SNMP_MGR_SnmpCommunityEntry_T;

/*eli,SNMP_MGR_SnmpCommunityEntryForCfgdb_T for cfgdb, we should not modify anything
 */
#define MAXSIZE_SNMPV3_ENGINE_ID_LEN            13
typedef struct
{
    UI8_T   snmp_community_index[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI8_T   snmp_community_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI8_T   snmp_community_security_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI8_T   snmp_community_context_engine_id[MAXSIZE_SNMPV3_ENGINE_ID_LEN];
    UI8_T   snmp_community_context_name[MAXSIZE_snmpCommunityContextName+1];
    UI8_T   snmp_community_transport_tag[MAXSIZE_snmpCommunityTransportTag+1];
    UI32_T  snmp_community_access_right;
    UI32_T  snmp_community_storage_type;
    UI32_T  snmp_community_status;
} SNMP_MGR_SnmpCommunityEntryForCfgdb_T;

typedef struct
{
    /* snmp_notify_name - (key)
     */
    char   snmp_notify_name[MAXSIZE_snmpNotifyName + 1];
    char   snmp_notify_tag[MAXSIZE_snmpNotifyTag +1 ];
    UI32_T snmp_notify_type;
    UI32_T snmp_notify_storage_type;
    UI32_T snmp_notify_row_status;
} SNMP_MGR_SnmpNotifyEntry_T;

typedef struct
{
    L_INET_AddrIp_T  snmp_notify_filter_profile_ip; /* key for local entry */
    char   snmp_target_params_name[MAXSIZE_snmpTargetParamsName + 1]; /* key for snmpNotifyFilterProfileTable */
    char   snmp_notify_filter_profile_name[MAXSIZE_snmpNotifyFilterProfileName +1 ];
    UI32_T snmp_notify_filter_profile_stor_type;
    UI32_T snmp_notify_filter_profile_row_status;
} SNMP_MGR_SnmpNotifyFilterProfileEntry_T;

typedef struct
{
    UI8_T       filter_name[MAXSIZE_snmpNotifyFilterProfileName+1];
    UI32_T      admin_status;
    UI32_T      oper_status;
} SNMP_MGR_NlmConfigLog_T;

#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
typedef struct
{
   /*snmpNotifyFilterProfileName  - (key1), this key is import from snmpNotifyFilterProfileTable*/
   /*snmpNotifyFilterSubtree      - (key2) */
    char   snmp_notify_filter_profile_name[MAXSIZE_snmpNotifyFilterProfileName + 1];
    oid     snmp_notify_filter_subtree[MAXSIZE_VIEW_SUBTREE +1 ];
    UI32_T  snmp_notify_filter_subtree_len;
    UI8_T   snmp_notify_filter_mask[ MAXSIZE_VIEW_MASK+1];
    UI32_T  snmp_notify_filter_mask_len;
    UI32_T  snmp_notify_filter_type;
    UI32_T  snmp_notify_filter_storage_type;
    UI32_T  snmp_notify_filter_row_status;
} SNMP_MGR_SnmpNotifyFilterEntry_T;

typedef struct
{
    /* snmp_target_params_name - (key)

     */
    char   snmp_target_params_name[MAXSIZE_snmpTargetParamsName + 1];
    UI32_T   snmp_target_params_mp_model;
    SNMP_MGR_Snmpv3_Model_T  snmp_target_params_security_model;
    UI8_T  snmp_target_params_security_name[MAXSIZE_snmpTargetParamsSecurityName+1];
    UI32_T snmp_target_params_security_level;
    UI32_T snmp_target_params_storage_type;
    UI32_T snmp_target_params_row_status;
} SNMP_MGR_SnmpTargetParamsEntry_T;

typedef struct
{
    /* snmp_target_addr_name - (key)
     */
    char   snmp_target_addr_name[MAXSIZE_snmpTargetAddrName + 1];
    oid     snmp_target_addr_tdomain[10]; /*netsnmp define this as MAX_OID_LEN*/
    UI32_T  snmp_target_addr_tdomain_len;

    union SNMP_TARGET_ADDR_TADDRESS_TYPE_U
    {
        char    snmp_target_addr_taddress[SNMP_MGR_TADDR_IPV6LINKLOCAL_LEN];
        struct
        {
            UI8_T ipaddress[SYS_ADPT_IPV4_ADDR_LEN];
            UI16_T udp_port;
        }ipv4_taddr;

        struct
        {
            UI8_T ipaddress[SYS_ADPT_IPV6_ADDR_LEN];
            UI16_T udp_port;
        }ipv6_taddr;

        struct
        {
            UI8_T ipaddress[SYS_ADPT_IPV6_ADDR_LEN];
            UI8_T scope_id[SNMP_MGR_TADDR_SCOPE_LEN];
            UI16_T udp_port;
        }ipv6z_taddr;
    }taddr;
    UI16_T  snmp_target_addr_len;
    UI32_T  snmp_target_addr_timeout;
    UI32_T  snmp_target_addr_retry_count;
    UI8_T   snmp_target_addr_tag_list[MAXSIZE_snmpTargetAddrTagList+1];
    char    snmp_target_addr_params[MAXSIZE_snmpTargetAddrParams+1];
    UI32_T  snmp_target_addr_storage_type;
    UI32_T  snmp_target_addr_row_status;
} SNMP_MGR_SnmpTargetAddrEntry_T;
#endif

typedef struct
{
    L_INET_AddrIp_T  trap_dest_address;
    char        trap_dest_community[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI32_T      trap_dest_status;
    UI32_T      trap_dest_port;
    SNMP_MGR_Snmpv3_Model_T      trap_dest_version;
    UI32_T      trap_dest_type;
    UI32_T      trap_dest_security_level;
    char        trap_dest_owner[MAXSIZE_trapDestOwner+1];
    UI32_T      trap_dest_protocol;
    UI32_T      trap_inform_request_timeout;
    UI32_T      trap_inform_request_retry_cnt;
    char        trap_dest_target_addr_name[MAXSIZE_snmpTargetAddrName + 1];
    char        trap_dest_target_params_name[MAXSIZE_snmpTargetParamsName + 1];
} SNMP_MGR_TrapDestEntry_T;

typedef struct
{
    L_INET_AddrIp_T  trap_dest_address;
    UI32_T      trap_dest_port;
    char        trap_dest_owner[MAXSIZE_trapDestOwner + 1];
    UI32_T      trap_dest_protocol;
    UI32_T      trap_dest_status;
} SNMP_MGR_TrapDestByIndexEntry_T;

typedef struct
{
    UI32_T      snmp_remote_engineID_host; /*key*/
    UI8_T       snmp_remote_engineID[MAXSIZE_snmpEngineID];
    UI32_T      snmp_remote_engineIDLen;
    UI32_T      snmp_remote_storage_type;
    UI32_T      snmp_remote_engineID_status;
}SNMP_MGR_SnmpRemoteEngineID_T;

#if (SYS_CPNT_CFGDB == TRUE)
typedef struct
{
    UI32_T      engine_boots;
    UI8_T       engineid[MAXSIZE_snmpEngineID];
} SNMP_MGR_SnmpEngineInfo_T;
#endif

/* structure for the request ipc message in SNMP_TYPE
 */
typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
    }type;

    union
    {

        BOOL_T bool_v;
        UI8_T  ui8_v;
        I8_T   i8_v;
        UI32_T ui32_v;
        UI16_T ui16_v;
        I32_T i32_v;
        I16_T i16_v;
        L_INET_AddrIp_T ip_v;

        struct
        {
            UI32_T ui32_1;
            UI32_T ui32_2;
        } ui32_ui32;

        struct
        {
            char comm_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
            UI32_T row_status;
        }set_snmp_community_status;

        SNMP_MGR_STATS_T stats_entry;
        SNMP_MGR_SnmpV3GroupEntry_T group_entry;
        SNMP_MGR_SnmpV3UserEntry_T user_entry;
        SNMP_MGR_SnmpV3ViewEntry_T view_entry;

        struct
        {
            UI8_T snmpv3_view_name[MAXSIZE_vacmViewTreeFamilyViewName+1];
            UI8_T snmpv3_wildcard_subtree[MAXSIZE_VIEW_SUBTREE+1];
        } delete_snmp_v3_view;

        struct
        {
            UI8_T snmpv3_view_name[MAXSIZE_vacmViewTreeFamilyViewName+1];
        } delete_snmp_v3_view_by_name;

        struct
        {
           char snmpv3_group_name[MAXSIZE_vacmGroupName+1];
           UI8_T  security_model;
           UI8_T security_level;
        }delete_snmp_v3_group;

        struct
        {
            UI8_T engineID[MAXSIZE_snmpEngineID];
            UI32_T engineIDLen;
        }engine_id_with_len;

        struct
        {
            SNMP_MGR_Snmpv3_Model_T  security_model;
            UI8_T snmpv3_user_name[MAXSIZE_usmUserName+1];
        }delete_snmp_v3_user;

        char snmp_v3_view_name[MAXSIZE_vacmViewTreeFamilyViewName+1];

        SNMP_MGR_SnmpCommunity_T com_entry;

        struct
        {
            char comm_string_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
            UI32_T access_right;
        }com_with_access;

        SNMP_MGR_TrapDestEntry_T trap_entry;

        struct
        {
            UI32_T trap_index;
            SNMP_MGR_TrapDestEntry_T trap_receiver;
        }tray_entry_by_index;

        struct
        {
            UI32_T trap_index;
            UI32_T port;
            L_INET_AddrIp_T addr;
        } set_trap_by_index;

        struct
        {
            UI32_T trap_index;
            UI8_T comm_string_name[SYS_ADPT_MAX_COMM_STR_NAME_LEN+1];
        } trap_with_comm;

        struct
        {
            UI32_T trap_index;
            UI32_T status;
        } trap_with_status;

        struct
        {
            L_INET_AddrIp_T ip_addr;
            UI8_T   target_name[MAXSIZE_snmpTargetAddrName+1];
        } trap_ip_with_name;

        struct
        {
            SNMP_MGR_TrapDestEntry_T entry;
            UI8_T target_name[MAXSIZE_snmpTargetAddrName+1];
        }tray_entry_with_name;

        struct
        {
            UI32_T remote_ip;
            SNMP_MGR_SnmpV3UserEntry_T entry;
        }remote_user;

        SNMP_MGR_SnmpRemoteEngineID_T remote_id_entry;

#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
        SNMP_MGR_SnmpTargetAddrEntry_T target_addr_entry;
#endif
#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
        SNMP_MGR_SnmpNotifyFilterProfileEntry_T notify_filter_profile_entry;

        SNMP_MGR_NlmConfigLog_T                          nlm_config_log_entry;

        struct
        {
            UI8_T   nlm_index[33];
            UI32_T  admin_status;
        }nlm_log_admin_status;

        struct
        {
            UI8_T    log_index[33];
            UI8_T    log_filter_name[33];
        }nlm_log_filter_name;

#endif
        struct
        {
            UI32_T unit;
            UI32_T port;
            UI32_T value;
        }statistics;

#if(SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)
        struct
        {
            UI8_T target_addr_name[MAXSIZE_snmpTargetAddrName+ 1];
            UI32_T notify_type;
            UI8_T user_name[MAXSIZE_usmUserName+ 1];
            UI32_T dest_ip_addres;
            UI32_T dest_port;
            UI8_T remote_engineID[MAXSIZE_snmpEngineID+ 1];
            UI32_T  remote_engineIDLen;
            BOOL_T password_from_config;
            UI32_T auth_protocol;
            UI32_T auth_key_len;
            UI8_T authentication_password[MAX_SNMPV3_PASSWORD_LEN+ 1];
            UI32_T  priv_protocol;
            UI32_T priv_key_len;
            UI8_T privacy_password[MAX_SNMPV3_PASSWORD_LEN+ 1];
            UI32_T retry_interval;
            UI32_T retry_count;
        }notify;

        UI8_T snmp_target_addr_name[MAXSIZE_snmpTargetAddrName+ 1];
#endif /* (SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)            */

        TRAP_EVENT_TrapData_T trap_data_entry;

        struct
        {
            TRAP_EVENT_TrapData_T trap_data;
            TRAP_EVENT_SendTrapOption_E flag;
        }trap_event_with_flag;

        struct
        {
            UI32_T ui32;
            SNMP_MGR_RmonStatisticsEntry_T entry;
        } ui32_rmon_statistics_entry;

        struct
        {
            UI32_T ui32;
            SNMP_MGR_RmonHistoryControlEntry_T entry;
        } ui32_rmon_history_control_entry;

        SNMP_MGR_RmonAlarmEntry_T rmon_alarm_entry;
        SNMP_MGR_RmonEventEntry_T rmon_event_entry;
        SNMP_MGR_RmonStatisticsEntry_T rmon_statistics_entry;
        SNMP_MGR_RmonHistoryControlEntry_T rmon_history_control_entry;
        SNMP_MGR_RmonHistoryEntry_T rmon_history_entry;
    } data; /* contains the supplemntal data for the corresponding cmd */
} SNMP_MGR_IPCMsg_T;

#define SNMP_MGR_MSGBUF_TYPE_SIZE    sizeof(((SNMP_MGR_IPCMsg_T *)0)->type)

///////////////////////////////////////////////////////////////////////////
/*
 * SNMP_MGR_SHM_TRAP_QUEUE_BUF
 *
 * +----------------------+
 * |       magic          |
 * +----------------------+
 * |       size           |
 * +----------------------+
 * |     max_entries      |
 * +----------------------+
 * |      max_size        |
 * +----------------------+
 * |      entries         |
 * +----------------------+
 * |       flags          |
 * +----------------------+
 * |        errcnt        |
 * +----------------------+
 * |        wr_off        |
 * +----------------------+
 * |        rd_off        |
 * +----------------------+<---------
 * |                      |         |
 * ~   trap entries       ~        size
 * ~                      ~         |
 * |                      |         |
 * +----------------------+<--------
 *
 */
#define SNMP_MGR_SHM_TRAP_QUEUE_FULL        1
#define SNMP_MGR_SHM_TRAP_QUEUE_ERROR       2
#define SNMP_MGR_SHM_TRAP_QUEUE_OVERLAP     4

typedef struct SNMP_MGR_SHM_TRAP_QUEUE_BUF
{
             UI32_T         magic;      /* magic word to reflect it is initialized or not           */
             UI32_T         size;       /* size of shared memory buffer, do not contian this header */
    volatile UI32_T         max_entries;/* maximum entries ever stored in buffer                    */
    volatile UI32_T         max_size;   /* maximum size of space ever consumed                      */
    volatile UI32_T         entries;    /* entry count contained in buffer                          */
    volatile UI16_T         flags;      /* SNMP_MGR_TRAP_QUEUE_FULL, etc                            */
    volatile UI16_T         errcnt;     /* count of errors happened                                 */
    volatile UI32_T         wr_off;     /* offset to write next byte of date                        */
    volatile UI32_T         rd_off;     /* offset to read next byte of data                         */
    volatile UI32_T         mtime;      /* timestamp of last modification                           */
} SNMP_MGR_SHM_TRAP_QUEUE_BUF_T;

typedef struct SNMP_MGR_SHM_TRAP_QUEUE
{
    UI32_T                          sem;        /* mutex to protect shared data                             */
    SNMP_MGR_SHM_TRAP_QUEUE_BUF_T * buf;
} SNMP_MGR_SHM_TRAP_QUEUE_T;


#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_ONLY           1
#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_COMMUNITY          2
#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOGGED             4
#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_AND_TRAP       8
#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_TRAP_ONLY          0x10
#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_DEFAULT            SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_LOG_AND_TRAP

typedef struct SNMP_MGR_SHM_TRAP_QUEUE_ENTRY
{
    UI16_T                  size;       /* size of current trap entry                               */
    UI16_T                  type;       /* type of current trap                                     */
    UI32_T                  time;       /* timestamp of this trap                                   */
    UI8_T                   retries;    /* remained retry times                                     */
    UI8_T                   flags;      /* flags of trap data                                       */
    union {
        struct {
            // AMS_SYS_ADPT_MAX_COMM_STR_NAME_LEN: 32
            UI8_T   len;        /* length of name include NUL                               */
            // char name[AMS_SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
            UI8_T   data[1];
        }           comm;
        UI8_T       data[1];
    }                       content;
} SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T;


#define SNMP_MGR_SHM_TRAP_QUEUE_PEEK         1

#define SNMP_MGR_SHM_TRAP_QUEUE_OVER_WRITE   1

#define SNMP_MGR_SHM_TRAP_QUEUE_BUF_SIZE             (1 * 1024 * 1024)

#define SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_HDR_SIZE       ((size_t)&((SNMP_MGR_SHM_TRAP_QUEUE_ENTRY_T *)0)->content)
#define SNMP_MGR_SHM_TRAP_QUEUE_BUF_MAGIC            (((UI32_T)'T' << 24) | ((UI32_T)'R' << 16) | ((UI32_T)'P' << 8) | (UI32_T)'Q')

#define SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(buf)    ((buf) && ((buf)->magic == SNMP_MGR_SHM_TRAP_QUEUE_BUF_MAGIC))
#define SNMP_MGR_SHM_TRAP_QUEUE_IS_VALID(Q)          ((Q) && ((Q)->sem != (UI32_T)(-1)) && SNMP_MGR_SHM_TRAP_QUEUE_BUF_IS_VALID(((Q)->buf)))
#define SNMP_MGR_SHM_TRAP_QUEUE_LOCK(Q)              do { (void) SYSFUN_TakeSem(((Q)->sem), SYSFUN_TIMEOUT_WAIT_FOREVER); } while (0)
#define SNMP_MGR_SHM_TRAP_QUEUE_UNLOCK(Q)            do { (void) SYSFUN_GiveSem(((Q)->sem)); } while (0)

#define SNMP_MGR_NTRAPS                              1

// #define SNMP_MGR_SHM_TRAP_QUEUE_DEBUG
#ifdef SNMP_MGR_SHM_TRAP_QUEUE_DEBUG
# define dprintf                        printf
// # define dprintf(...)                do {} while (0)
#else
# define dprintf(...)                   do {} while (0)
#endif


UI32_T                      SNMP_MGR_GetDynamicDataSize(UI32_T trap_type);
UI32_T                      SNMP_MGR_GetTrapDataSize(TRAP_EVENT_TrapData_T *trap);
int                         SNMP_MGR_ShmTrapQueueInit(SNMP_MGR_SHM_TRAP_QUEUE_T *queue);
void                        SNMP_MGR_ShmTrapQueueReset(SNMP_MGR_SHM_TRAP_QUEUE_T *queue);
SNMP_MGR_SHM_TRAP_QUEUE_T * SNMP_MGR_GetShmTrapQueue(void);
int                         SNMP_MGR_ShmTrapQueueRead(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int n, int flag);
int                         SNMP_MGR_ShmTrapQueueWrite(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int flag);
UI32_T                      SNMP_MGR_GetShmTrapQueueEntryCount(SNMP_MGR_SHM_TRAP_QUEUE_T *queue);
int                         SNMP_MGR_ShmTrapQueueGetUnloggedEntries(SNMP_MGR_SHM_TRAP_QUEUE_T *queue, TRAP_EVENT_TrapData_T *trap, int n);

///////////////////////////////////////////////////////////////////////////

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ConvertOidToStr
 * ---------------------------------------------------------------------
 * PURPOSE : Convert object id to string.
 * INPUT   : src_p
 *           src_len
 *           dst_len
 * OUTPUT  : dst_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_ConvertOidToStr(UI32_T *src_p, UI32_T src_len, char *dst_p, UI32_T dst_len);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ConvertStrToOid
 * ---------------------------------------------------------------------
 * PURPOSE : Convert string to object id.
 * INPUT   : src_p
 *           dst_p
 *           dst_max_len
 * OUTPUT  : dst_len_p
 * RETURN  : TRUE/FALSE
 * NOTES   : None.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_ConvertStrToOid(char *src_p, UI32_T *dst_p, UI32_T dst_max_len, UI32_T *dst_len_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ConvertLInetAddrToInetAddrAndType
 * ---------------------------------------------------------------------
 * PURPOSE : Convert L_INET_Addr_T to InetAddressType/InetAddress pair
 *           and prefix length.
 *
 * INPUT   : in_addr_str_p          - pointer to virtual type L_INET_Addr_T
 *           out_inet_type_p        - pointer to get InetAddressType
 *           out_inet_addr_len_p    - pointer to get length of InetAddress
 *           out_inet_addr_ar_p     - pointer to array to get InetAddress
 *           out_preflen_p          - pointer to get prefix length
 *
 * OUTPUT  : *out_inet_type_p       - InetAddressType
 *           *out_inet_addr_len_p   - length of InetAddress
 *           out_inet_addr_ar_p     - buffer content will get InetAddress
 *           *out_preflen_p         - prefix length
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTE    : 1. Because this is not an export function, buffer size
 *              check is not performed.  The caller must ensure that
 *              input and output buffers have the proper sizes.
 *
 *           2. The output *out_inet_type_p is the same as L_INET_Addr_T.type.
 *              The output *out_preflen_p is the same as L_INIT_Addr_T.preflen.
 *              So, if you only need to get these, you don't need to
 *              call this function.
 *
 *           3. For IP addresses, input L_INET_AddrIp_T.preflen is
 *              converted to the output.  For DNS, this is ignored.
 *
 *           4. For DNS, the output InetAddress is an octet string,
 *              not a NUL-terminated C-language text string.
 *
 *           5. If this function returns FALSE, some output variables
 *              may already have been modified.  Don't expect them to
 *              remain the same as before calling this function.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_ConvertLInetAddrToInetAddrAndType(L_INET_Addr_T *in_addr_str_p,
    UI32_T *out_inet_type_p,
    UI32_T *out_inet_addr_len_p, UI8_T *out_inet_addr_ar_p, UI32_T *out_preflen_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ConvertInetAddrAndTypeToLInetAddr
 * ---------------------------------------------------------------------
 * PURPOSE : Convert InetAddressType/InetAddress pair and prefix length
 *           to L_INET_Addr_T.
 *
 * INPUT   : in_inet_type       - InetAddressType
 *           in_inet_addr_len   - length of InetAddress
 *           in_inet_addr_ar_p  - pointer to array with InetAddress
 *           in_preflen         - prefix length
 *           out_addr_str_p     - pointer to virtual type L_INET_Addr_T
 *                                to get output
 *
 * OUTPUT  : *out_addr_str_p    - cast pointer of virtual type to real type
 *                                to get output structure
 *
 * RETURN  : TRUE/FALSE
 *
 * NOTE    : 1. Because this is not an export function, buffer size
 *              check is not performed.  The caller must ensure that
 *              input and output buffers have the proper sizes.
 *
 *           2. If "in_preflen" is not needed, fill 0.
 *
 *           3. For DNS, the input InetAddress is an octet string,
 *              C-language text string NUL-termination is not needed.
 *
 *           4. If this function returns FALSE, some output variables
 *              may already have been modified.  Don't expect them to
 *              remain the same as before calling this function.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_ConvertInetAddrAndTypeToLInetAddr(UI32_T in_inet_type,
    UI32_T in_inet_addr_len, UI8_T *in_inet_addr_ar_p, UI32_T in_preflen,
    L_INET_Addr_T *out_addr_str_p);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNMP_MGR_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : Initialize SNMP_MGR.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void  SNMP_MGR_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_IsDebugMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Check if it is in debug mode now
 * INPUT:    None.
 * OUTPUT:
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
 BOOL_T SNMP_MGR_IsDebugMode(void);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_IsEHMsgEnable
 *---------------------------------------------------------------------------
 * PURPOSE:  Check if the EH message is enable now
 * INPUT:    None.
 * OUTPUT:
 * RETURN:   True/False
 * NOTE:
 *---------------------------------------------------------------------------
 */
 BOOL_T SNMP_MGR_IsEHMsgEnable(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_EnterMasterMode();



/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_EnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_EnterSlaveMode();


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_EnterTransition Mode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_EnterTransitionMode();


/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_SetTransitionMode
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP MGR Set Transition mode.
 *  INPUT    : none.
 *  OUTPUT   : none.
 *  RETURN   : none.
 *  NOTE     : none.
 * ------------------------------------------------------------------------*/
void SNMP_MGR_SetTransitionMode();


 /*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_GetOperationMode()                +
 *---------------------------------------------------------------------------  +
 * Purpose  :    To Get the current SNMP operation mode              +
 * Input    :       None                                                                        +
 * Output   :     None                                                                        +
 * Return   :      SYS_TYPE_Stacking_Mode_T                             +
 * Note     :       None                                                                        +
 *---------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T SNMP_MGR_GetOperationMode(void);

 /*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_Gen_Auth_Failure_Trap()                  +
 *---------------------------------------------------------------------------+
 * Purpose  :   This function will generate a authentication failure trap   +
 * Input    :   None                                                                                 +
 * Output   :   None                                                                                 +
 * Return   :   SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK                                                                                 +
 * Note     :   None                                                                                +
 *---------------------------------------------------------------------------*/
UI32_T  SNMP_MGR_Gen_Auth_Failure_Trap(void);


#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
/* Function Name: SNMP_MGR_CheckCompl
 * Purpose      : This function is used to check Octet 's type index, eg IP, Mac Address index
 *                if the data is exceed 255, we will convert it to 255(MAX) and the remain byte will
 *                also set to 255(MAX). This function is for getNext index check issue.
 *                For example, if input index is 10.7.4294967295.5(compl[0]=10, compl[1]=7, compl[2]=
 *                4294967295, compl[3]=5, we will convert it to 10.7.255.255(compl[0]=10, compl[1]=7,
 *                compl[2] = 255, compl[3]=255). Then we can get the right index from GetNext Operation.
 *
 *
 * Input        : begin (The first byte of the compl want to check)
 *              : end   (The End byte of the compl that want to check)
 *              : compl
 *              : max_value (The max value of this index)
 *
 * Output       : compl
 * Return       : None
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: James, 2002/6/20
 */
void SNMP_MGR_checkCompl(int begin, int end, oid *compl, UI32_T max_value);


/* Function Name: SNMP_MGR_ConvertRemainToZero
 * Purpose      : This function is used to convert the remaing index to zero, if
 *                the compl is not given enough to the number or index, This was
 *                use to solve getnext issue of index that is not enough.
 *
 *
 * Input        : compc  The number of index binding into agent.
 *              : exact  The desired number of index of this table
 *              : compl  The data in which the index store in.
 *
 * Output       : compl
 * Return       : None
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: James, 2002/6/20
 */
void SNMP_MGR_ConvertRemainToZero(int compc, int exact, oid *compl);

/* Function Name: SNMP_MGR_ReadStrFromCompl
 * Purpose      : This function is used to convert string that store in an integer Array,
 *                  to a Normal form of string
 *                here is an example of what is a string store in integer Array:
 *                  //ip: abcd store in iStr:
 *                  UI32_T iStr[4];
 *                  iIp[0]=97;//a
 *                  iIP[1]=98;//b
 *                  iIp[2]=99;//c
 *                  iIp[3]=100;//d
 *
 * Output       : result:the result string in normal form that after the conversion
 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_ReadStrFromCompl(oid* compl,UI32_T start, UI32_T end, UI8_T* result);

/* Function Name: SNMP_MGR_ReadOctetFromCompl
 * Purpose      : This function is used to convert string that store in an integer Array,
 *                  to a Normal form of string
 *                here is an example of what is a string store in integer Array:
 *                  //ip: abcd store in iStr:
 *                  UI32_T iStr[4];
 *                  iIp[0]=97;//a
 *                  iIP[1]=98;//b
 *                  iIp[2]=99;//c
 *                  iIp[3]=100;//d
 *
 * Output       : result:the result string in normal form that after the conversion
 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_ReadOctetFromCompl(oid* compl,UI32_T start, UI32_T end, UI8_T* result);

/* Function Name: SNMP_MGR_ReadIpFromCompl
 * Purpose      : This function is used to convert ip that store in an integer Array,
 *                  to a UI32_T form of IP
 *                here is an example of what is Ip store in integer Array:
 *                  //ip: 10.2.48.55 store in iIp:
 *                  UI32_T iIp[4];
 *                  iIp[0]=10;
 *                  iIP[1]=2;
 *                  iIp[2]=48;
 *                  iIp[3]=55;
 *
 * Output       : result:the ip in UI32_T form that after the convertion
 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_ReadIpFromCompl(oid* compl,UI32_T start, UI32_T* result);

/* Function Name: SNMP_MGR_BindIpInstance
 * Purpose      : This function is used to bind the Ip instance back to the agent
 *
 *
 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_BindIpInstance( UI32_T ip, UI32_T start, oid* inst);

/* Function Name: SNMP_MGR_BindStrInstance
 * Purpose      : This function is used to bind the string instance back to the agent
 *
 *

 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_BindStrInstance( char* str, UI32_T start, oid* inst);

/* Function Name: SNMP_MGR_BindOctetInstance
 * Purpose      : This function is used to bind the string instance back to the agent
 *
 *

 * Return       : TRUE/FALSE
 * Note         : This fuction is only for SNMP agent internal use only
 * First Created: Phoebe, 2003/1/05
 */
BOOL_T SNMP_MGR_BindOctetInstance( UI8_T* str, UI32_T start, UI32_T end, oid* inst);


/* Function Name: SNMP_MGR_RetrieveCompl
 * Purpose      : This function is used to retreive the compc and compl for Net-SNMP
 *                agent.
 *
 * Input        : *actual_name  It stand for the actually oid of the MIB varible that we desired to get.
 *              : actual_name_length The length of the *actual_name.
 *              : *name   The originally oid(plus index) of the variable in which we press Get/GetNext
 *                        from the SNMP browser.
 *              : *length The length of the *name.
 *              : *compc  Output, we will return the length of the compl for output.
 *              : *compl  Output, the compl pointer that we use for output.
 *              : instance_len  The total length of the compl, for example, if a table have three index
 *                              of UI32_T, then this instance_len will be 3*4 = 12;
 *
 * Output       : compc, compl
 * Return       : None
 * Note         : This fuction is only for NET-SNMP agent only.
 * First Created: James, 2003/11/28
 */
void SNMP_MGR_RetrieveCompl(oid *actual_name,
       UI32_T actual_name_length,
       oid *name,
       UI32_T length,
       UI32_T *compc,
       oid *compl,
       UI32_T instance_len);
#endif /* NET_SNMP_INCLUDES_H */

#if   (SYS_CPNT_DBSYNC_TXT == TRUE)

/* for 3 com autosave*/
/* 3 com auto save */
/* FUNCTION NAME : SNMP_MGR_GetTmpDirty
 * PURPOSE:
 *      Get current tmp_dirty flag.
 *
 * INPUT:
 *      *tmp_dirty
 *
 * OUTPUT:
 *      *tmp_dirty
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T  SNMP_MGR_GetTmpDirty(BOOL_T  *tmp_dirty);


/* FUNCTION NAME : SNMP_MGR_SetTmpDirty
 * PURPOSE:
 *      Get current tmp_dirty flag.
 *
 * INPUT:
 *       tmp_dirty
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None.
 */
 BOOL_T  SNMP_MGR_SetTmpDirty(BOOL_T  tmp_dirty);
#endif


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified SNMP community
 *          string can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community string
 * OUTPUT: comm_string                  - SNMP community info
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTES: 1. The SNMP community string can only be accessed by CLI and Web.
 *                     SNMP management station CAN NOT access the SNMP community string.
 *                2. There is no MIB to define the SNMP community string.
 *
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *           Access right of community string could be
 *              - READ_ONLY(1)
 *              - READ_WRITE(2)
 *
 *        4. The total number of SNMP community string supported by the system
 *           is defined by SNMP_MGR_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *        5. By default, two SNMM community string are configued:
 *              - "PUBLIC"      READ-ONLY       enabled
 *              - "PRIVATE"     READ-WRITE      enabled
 *        6. For any SNMP request command, all of enabled community string
 *             shall be used to authorize if this SNMP request is legal/permit.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpCommunity(SNMP_MGR_SnmpCommunity_T *comm_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available SNMP community
 *          string can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community string
 * OUTPUT: comm_string                  - next available SNMP community info
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTES: 1. Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *                2. Any invalid(0) community string will be skip duing the GetNext operation.
 *                3. comm_entry->comm_string_name = "" to get the first entry.
 */
UI32_T SNMP_MGR_GetNextSnmpCommunity(SNMP_MGR_SnmpCommunity_T *comm_entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextRunningSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default SNMP community can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: comm_entry->comm_string_name - (key) to specify a unique SNMP community
 * OUTPUT: snmp_comm                  - next available non-default SNMP community
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *                    "running configuration" to local or remote files.
 *                2. Since only non-default configuration will be saved, this
 *                function shall return non-default SNMP community.
 *               3. Community string name is an "ASCII Zero" string (char array ending with '\0').
 *               4. Any invalid(0) SNMP community will be skip during the GetNext operation.
  *              5. comm_entry->comm_string_name = "" to get the first entry.
 */
UI32_T SNMP_MGR_GetNextRunningSnmpCommunity(SNMP_MGR_SnmpCommunity_T *comm_entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The  Get  Function of Snmp V3 user/
 * INPUT:     entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T  *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 user.
 * INPUT:        entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:        1.entry->snmpv3_user_engine_id and entry->snmpv3_user_name = "" to get the first entry.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T  *entry);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpV3UserByGroup
 *---------------------------------------------------------------------------
 * PURPOSE:  he GetNext Function of Snmp V3 user name by index of group.
 * INPUT:    group_name, group_model, group_level, user_name
 * OUTPUT:   user_name, user_model
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_EXCEED_LIMIT
 *                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                         SNMP_MGR_ERROR_FAIL
 * NOTE:   1. group_name, group_model and group_level is the primary key.
 *         2. user_name is secondary key and set it to null to get the first entry.
 *         3. After GetNext, will return the next secondary key whereas the primary key
 *            will not return the next value.
 *         4. For Web Only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpV3UserByGroup(UI8_T *group_name,
                                         SNMP_MGR_Snmpv3_Model_T group_model,
                                         UI32_T group_level,
                                         UI8_T *user_name,
                                         SNMP_MGR_V3_Auth_Type_T *auth_type,
                                         SNMP_MGR_V3_Priv_Type_T *priv_type);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextRunningSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Running Function of Snmp V3 user, to get the first entry with the key of 0.
 * INPUT:        entry->snmpv3_user_engine_id, entry->snmpv3_user_name
 * OUTPUT:   entry.
 * RETURN:   RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:      1.entry->snmpv3_user_engine_id and entry->snmpv3_user_name = "" to get the first entry.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetNextRunningSnmpV3User(SNMP_MGR_SnmpV3UserEntry_T  *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the Snmp V3 group
 * INPUT:    entry->snmpv3_group_name, entry->snmpv3_group_context_prefix, entry->snmpv3_group_model, entry->snmpv3_group_security_level.
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3Group( SNMP_MGR_SnmpV3GroupEntry_T  *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of SNMP V3 Group.
 * INPUT:    entry->snmpv3_group_name, entry->snmpv3_group_context_prefix, entry->snmpv3_group_model, entry->snmpv3_group_security_level.
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:    1.To get the first entry, input all the key to 0 or NULL
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpV3Group( SNMP_MGR_SnmpV3GroupEntry_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextRunningSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext  Running Function of SNMP V3 Group.
 * INPUT:    entry->snmpv3_group_name, entry->snmpv3_group_context_prefix, entry->snmpv3_group_model, entry->snmpv3_group_security_level.
 * OUTPUT:   entry.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:      1.To get the first entry, input all the key to 0 or NULL
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetNextRunningSnmpV3Group( SNMP_MGR_SnmpV3GroupEntry_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMPV3 View
 * INPUT:    entry->snmpv3_view_name, entry->snmpv3_view_subtree
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T  *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 View
 * INPUT:      entry->snmpv3_view_name, entry->snmpv3_view_subtree
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T  *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpV3ViewName
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Function of Snmp V3 View Name.
 * INPUT:   view_name,
 * OUTPUT:  view_name
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_EXCEED_LIMIT
 *                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *           2. For Web only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpV3ViewName(UI8_T *view_name);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextRunningSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Ruuning Function of Snmp V3 View.
 * INPUT:      entry->snmpv3_view_name, entry->snmpv3_view_subtree
 * OUTPUT:   entry.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:     1. To get the first entry with the key of 0 or null.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetNextRunningSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T  *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetEngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMP Engine ID, the Engine ID can be 5-32 octets.
 * INPUT:    engineID, engineIDLen.
 * OUTPUT:   engineID, engineIDLen.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetEngineID (UI8_T *engineID, UI32_T *engineIDLen);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetEngineBoots
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Get the SNMP Engine boots.
 * INPUT:    engineBoots.
 * OUTPUT:   engineBoots
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetEngineBoots ( UI32_T *engineBoots);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetRunningEngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This Get running function of the  SNMP Engine ID, the Engine ID can be 5-32 octets.
 * INPUT:    engineID, engineIDLen.
 * OUTPUT:   engineID, engineIDLen.
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                     SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetRunningEngineID (UI8_T *engineID, UI32_T *engineIDLen);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_CreateSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if we  can  successfully
 *         create  the specified community string. Otherwise, false is returned.
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *        access_right      - the access level for this SNMP community
 * OUTPUT: None
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new community string to the system
 *           if the specified comm_string_name does not exist, and total number
 *           of community string configured is less than
 *           SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING.
 *        2. This function will update the access right an existed community
 *           string if the specified comm_string_name existed already.
 *        3. False is returned if total number of community string configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_CreateSnmpCommunity(char *comm_string_name, UI32_T access_right);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_RemoveSnmpCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if we can successfully remove a Snmp community
 *
 * INPUT: comm_string_name  - (key) to specify a unique SNMP community string
 *
 * OUTPUT: None
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will remove a snmp community.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_RemoveSnmpCommunity(char *comm_string_name);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetDefaultSnmpCommunityEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the Default entry of SnmpCommunityEntry
 * INPUT:    comm_entry
 * OUTPUT:   comm_entry
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                 SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetDefaultSnmpCommunityEntry(SNMP_MGR_SnmpCommunityEntry_T *comm_entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the SnmpCommunityEntry by entry
 * INPUT:    comm_entry
 * OUTPUT:   comm_entry
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *                 SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   This API only for SetbyEntry with RowStatus = 4(createAndGo) and 5(createAndWait).
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityEntry(SNMP_MGR_SnmpCommunityEntry_T *comm_entry_ptr);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_DeleteSnmpV3User
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete a SNMP V3 User
 * INPUT:    secourity_model, snmpv3_user_name[].
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     1. None
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteSnmpV3User(SNMP_MGR_Snmpv3_Model_T  security_model,  UI8_T * snmpv3_user_name);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_DeleteSnmpV3Group
 *---------------------------------------------------------------------------
 * PURPOSE: This function will delete a SNMP V3 Group that is not auto created.
 * INPUT:   snmpv3_group_name[], security_model, security_level
 * OUTPUT:  None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_FAIL
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteSnmpV3Group(char *snmpv3_group_name, UI8_T  security_model ,  UI8_T security_level);



/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_CreateSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the Snmp 's agent enter the transition mode.
 * INPUT:    entry.
 * OUTPUT:   None..
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:        The entry.snmpv3_view_subtree is in the ASCII from of an OID. and the mask is also in the ASCII from.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_CreateSnmpV3View(SNMP_MGR_SnmpV3ViewEntry_T *entry);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_DeleteSnmpV3View
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete a SNMP V3 View
 * INPUT:    snmpv3_view_name[], snmpv3_view_subtree[]
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteSnmpV3View(UI8_T *snmpv3_view_name, UI8_T *snmpv3_wildcard_subtree);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_DeleteSnmpV3ViewByName
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will delete all the SNMP V3 View by a specific  view Name
 * INPUT:    snmpv3_view_name[]
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
  UI32_T SNMP_MGR_DeleteSnmpV3ViewByName(UI8_T *snmpv3_view_name );


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_Set_EngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will Set the SNMP V3 EngineID
 * INPUT:    newengineID, engineIDLen
 * OUTPUT:   None.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     All uiser will delete when an new engineID is set..
 *---------------------------------------------------------------------------
 */
UI32_T    SNMP_MGR_Set_EngineID(UI8_T *newengineID, UI32_T engineIDLen);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetDefaultSnmpEngineID
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set the SNMP engineID to default value.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  1. success: SNMP_MGR_ERROR_OK
 *          2. failure: SNMP_MGR_ERROR_EXCEED_LIMIT
 *                      SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                      SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                      SNMP_MGR_ERROR_FAIL
 * NOTE:    This function is called when use the no snmp-server engineID,
 *          it will restore the engineID to default.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetDefaultSnmpEngineID();

  /*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_Enable_Snmp_Agent            +
 *---------------------------------------------------------------------------+
 * Purpose :      This function will enable the snmp_agent              +
 * Input    :     None                                                                       +
 * Output   :     None                                                                        +
 * Return   :     SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK                                                                        +
 * Note     :     None                                                                        +
 *---------------------------------------------------------------------------*/
UI32_T  SNMP_MGR_Enable_Snmp_Agent(void);


  /*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_Disable_Snmp_Agent               +
 *---------------------------------------------------------------------------+
 * Purpose :      This function will disable the snmp_agent    +
 * Input    :     None                                                                                 +
 * Output   :     None                                                                                 +
 * Return   :     SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK                                                                                +
 * Note     :     None                                                                                +
 *---------------------------------------------------------------------------*/
UI32_T  SNMP_MGR_Disable_Snmp_Agent(void);

/* The below is for Target MIB and Notify MIB implemation */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverCommunity
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the community string name can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, error code is returned.(as defined in snmp_mgr.h)
 *
 * INPUT: ip_address - (key) to specify a unique trap receiver
 *        community      - the SNMP community string for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_NO_ENTRY_EXIST
 * NOTES: 1. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverCommunity(L_INET_AddrIp_T *ip_address, UI8_T *community);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverVersion
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap version can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, error code is returned.(as defined in snmp_mgr.h)
 *
 * INPUT: ip_address - (key) to specify a unique trap receiver
 *        version      - the SNMP version of the trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_NO_ENTRY_EXIST
 * NOTES: 1. This function will update an existed trap receiver if
 *           the specified version existed already.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverVersion(L_INET_AddrIp_T *ip_address, UI32_T version);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverPort
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap port can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, error code is returned.(as defined in snmp_mgr.h)
 *
 * INPUT: ip_address - (key) to specify a unique trap receiver
 *        udp_port      - the SNMP version of the trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_NO_ENTRY_EXIST
 * NOTES: 1. This function will update an existed trap receiver if
 *           the specified version existed already.
 *        2. Default udp_port is 162 when created the trap receiver is created.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverPort(L_INET_AddrIp_T *ip_address, UI32_T udp_port);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverType
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap type can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, error code is returned.(as defined in snmp_mgr.h)
 *
 * INPUT: ip_address - (key) to specify a unique trap receiver
 *        type      - the SNMP type of the trap receiver
 *                    SNMP_MGR_SNMPV3_NOTIFY_TRAP; SNMP_MGR_SNMPV3_NOTIFY_INFORM
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_NO_ENTRY_EXIST
 * NOTES: 1. This function will update an existed trap receiver if
 *           the specified version existed already.
 *        2. Default type is trap when the trap receiver is created.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverType(L_INET_AddrIp_T *ip_address, UI32_T type);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the next available trap receiver
 *          can be retrieved successfully. Otherwise, error code is returned.
 *
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK;   SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_FAIL;
 * NOTES:1.This function will return trap receiver from the smallest ip addr.
 *       2.To get the first entry, input the key as 0.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be retrieved successfully. Otherwise, error code is returned.
 *
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK;   SNMP_MGR_ERROR_NO_ENTRY_EXIST; SNMP_MGR_ERROR_FAIL;
 * NOTES:1.This function will return trap receiver from the smallest ip addr.
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_Marked_Inform_Request_Socket
 * ---------------------------------------------------------------------------
 *  FUNCTION: This function is used to mark the correpsonding sock num to whether
 *            available or not.
 *
 *  INPUT    : infrom_sock_num -- socket_id;  flag--- available or not
 *  OUTPUT   : None.
 *  RETURN   : TRUE/FALSE.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_Marked_Inform_Request_Socket( UI32_T inform_sock_num, BOOL_T flag);

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_GetAvailableInformSocket
 * ---------------------------------------------------------------------------
 *  FUNCTION: This function is used to get the available inform/trap socket.
 *
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : the avaialble socket id, -1 is none available.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
I32_T SNMP_MGR_GetAvailableInformSocket();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_CreateSnmpRemoteUser
 * ---------------------------------------------------------------------
 * PURPOSE: This function will Create the Snmp Remote User
 * INPUT : remote ip, entry
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_CreateSnmpRemoteUser(UI32_T remote_ip, SNMP_MGR_SnmpV3UserEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_DeleteSnmpRemoteUser
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the Snmp Remote User
 * INPUT : entry
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteSnmpRemoteUser(SNMP_MGR_SnmpV3UserEntry_T *entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get the Snmp Remote User entry
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will getnext the Snmp Remote User entry
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextRunningSnmpRemoteUserEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will getnext  the Snmp Remote User entry for cli running config
 * INPUT : user_entry
 * OUTPUT: user_entry
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_FAIL/SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextRunningSnmpRemoteUserEntry (SNMP_MGR_SnmpV3UserEntry_T *user_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_CreateRemoteEngineID
 * ---------------------------------------------------------------------
 * PURPOSE: This function will create the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_CreateRemoteEngineID(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_DeleteRemoteEngineID
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK
 * NOTES : remove the remote EngineID Entry and all users which engineID assoiate on it
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteRemoteEngineID(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_DeleteRemoteEngineIDEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will delete the remote engineID Entry
 * INPUT : user_entry
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_FAIL/SNMP_MGR_ERROR_OK
 * NOTES : Only remove the remote EngineID Entry
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteRemoteEngineIDEntry(SNMP_MGR_SnmpRemoteEngineID_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetSnmpRemoteEngineIDEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get the remote engineID Entry
 * INPUT : entry
 * OUTPUT: entry
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES : none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpRemoteEngineIDEntry (SNMP_MGR_SnmpRemoteEngineID_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpRemoteEngineIDEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext function of the Remote EngineID entry.
 * INPUT:    entry.
 * OUTPUT:   entry
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpRemoteEngineIDEntry( SNMP_MGR_SnmpRemoteEngineID_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextRunningSnmpRemoteEngineIDEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext Running Function of Remote EngineID Entry
 * INPUT:    entry.
 * OUTPUT:   entry
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL/SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextRunningSnmpRemoteEngineIDEntry( SNMP_MGR_SnmpRemoteEngineID_T *entry);


#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpTargetAddrTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will retrieve the snmpTargetAddrTable in Rfc3413 Target MIB
 * INPUT:    entry->snmp_target_addr_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_NO_ENTRY_EXIST
 *                         SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpTargetAddrTable(SNMP_MGR_SnmpTargetAddrEntry_T *entry);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpTargetAddrTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will retrieve the next snmpTargetAddrTable in Rfc3413 Target MIB
 * INPUT:    entry->snmp_target_addr_name
 * OUTPUT:   entry.
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *          2. failure:    SNMP_MGR_ERROR_NO_ENTRY_EXIST
 *                         SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpTargetAddrTable(SNMP_MGR_SnmpTargetAddrEntry_T *entry);
#endif


/*
=========================================
Get/GetNext TrapReceiver By Index API:
==========================================
*/
#if (SYS_CPNT_SNMP_CONTIGUOUS_TRAP_DEST_TABLE == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetTotalTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the total number of
 *          trap receivers in the numerical-index table
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:  None.
 * OUTPUT: *total_p - total number of trap receivers
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetTotalTrapReceiverByIndex(UI32_T *total_p);
#endif  /* (SYS_CPNT_SNMP_CONTIGUOUS_TRAP_DEST_TABLE == TRUE) */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified trap receiver
 *          can be retrieved successfully.
 *          Otherwise, false is returned.
 * INPUT:   snmp_index          - 1-based SNMP user index
 * OUTPUT:  *trap_receiver_p    - trap receiver info
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 *
 * NOTE:  1. Status of each trap receiver is defined as SNMP_MGR_Snmp_RowStatus_Type_T.
 *        2. Setting status to SNMP_MGR_SNMPV3_ROWSTATUS_TYPE_DESTROY will delete/purge a trap receiver.
 *        3. Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *        4. The total number of trap receivers supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *        5. By default, there is no trap receiver configued in the system.
 *        6. For any trap event raised by any subsystem, a SNMP Trap shall be
 *           sent to all of the enabled trap receivers.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetTrapReceiverByIndex(UI32_T snmp_index, SNMP_MGR_TrapDestEntry_T *trap_receiver_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available trap receiver
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT:   *snmp_index_p       - 1-based SNMP user index
 * OUTPUT:  *snmp_index_p       - next available 1-based SNMP user index
 *          *trap_receiver_p    - next available trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextTrapReceiverByIndex(UI32_T *snmp_index_p, SNMP_MGR_TrapDestEntry_T *trap_receiver_p);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverCommStringNameByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the community string name can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT:   snmp_index          - 1-based SNMP user index
 *          comm_string_name    - the SNMP community string for this trap receiver
 * OUTPUT:  None
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:  1. This function will create a new trap receiver to the system if
 *           the specified trap_dest_address does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           status of this new trap receiver will be set to disabled(2)
 *           by default.
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverCommStringNameByIndex(UI32_T snmp_index, UI8_T *comm_string_name);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapDestProtocolByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the TrapDestProtocol can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT:   snmp_index          - 1-based SNMP user index
 *          protocol            - protocol
 * OUTPUT:  None
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapDestProtocolByIndex(UI32_T snmp_index, UI32_T protocol);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapDestAddressByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the TrapDestProtocol can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT:   snmp_index          - 1-based SNMP user index
 *          addr                - address
 * OUTPUT:  None
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES:
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapDestAddressByIndex(UI32_T snmp_index, UI32_T port, L_INET_AddrIp_T *addr);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverSecLevelByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the security level can be
 *          successfully set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT:   snmp_index          - 1-based SNMP user index
 *          level               - the SNMP security level for this trap receiver
 * OUTPUT:  None
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified trap_dest_address does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           status of this new trap receiver will be set to disabled(2)
 *           by default.
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverSecLevelByIndex(UI32_T snmp_index, UI32_T level);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverStatusByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the status can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        status                - the status for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverStatusByIndex(UI32_T snmp_index, UI32_T status);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverVersionByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the version can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        version                - the version for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: None
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverVersionByIndex(UI32_T index, UI32_T version);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapDestOwnerByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the trapDestOwner can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        version                - the version for this trap receiver
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: None
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapDestOwnerByIndex(UI32_T index, UI8_T *owner);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetDefaultTrapDestEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns the default value of trap receiver
 *
 * INPUT: entry  -> the structure of trap receiver
 * OUTPUT: entry -> the default structure of trap receiver
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES:   1. This function is particular design for snmpv2 row-created mechamism.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetDefaultTrapDestEntry(SNMP_MGR_TrapDestEntry_T *entry);

 /* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapDestEntryByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the trap receiver can be created successfully
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        entry --> the structure of trapDestEntry
 * OUTPUT: None
 * RETURN: SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 *        4. This function is "Set by Record", which is particular design
 *           for snmpv2 row-created mechamism.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapDestEntryByIndex(UI32_T index, SNMP_MGR_TrapDestEntry_T *entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_VerifyAndCreateProbeConfigDB
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will check every entry in Target MIB database,
 *           and will create the probe config MIB entry to if
 *           the entry is active due to target/Notificaiton MIB
 *           creation.
 * INPUT:    none
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_VerifyAndCreateProbeConfigDB();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextRunningTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default trap receiver can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available non-default trap receiver info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default trap receiver.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextRunningTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_DeleteTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be deleted successfully. Otherwise, error code is returned.
 *
 * INPUT:   ip_addr    - (key) to specify a unique trap receiver
 * OUTPUT:  none
 * RETURN:  SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTE:    none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteTrapReceiver(L_INET_AddrIp_T *ip_addr);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_DeleteTrapReceiverWithTargetAddrName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be deleted successfully. Otherwise, error code is returned.
 *
 * INPUT:   ip_addr    - (key) to specify a unique trap receiver
 *          target_addr_name
 * OUTPUT:  none
 * RETURN:  SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTE:    none
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_DeleteTrapReceiverWithTargetAddrName(L_INET_AddrIp_T *ip_addr, UI8_T *target_name);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be created successfully. Otherwise, error code is returned.
 *
 * INPUT: entry    - (key) to specify a unique trap receiver
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *         2. error code is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiver(SNMP_MGR_TrapDestEntry_T *entry);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetTrapReceiverWithTargetAddressName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the trap receiver
 *          can be created successfully. Otherwise, error code is returned.
 *
 * INPUT: entry    - (key) to specify a unique trap receiver
 *        target_addr_name
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *         2. error code is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetTrapReceiverWithTargetAddrName(SNMP_MGR_TrapDestEntry_T *entry, UI8_T *target_name);


/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_Get_AgentStatus                                   +
 *---------------------------------------------------------------------------+
 * Purpose :      This function will get the agent status                    +
 * Input    :     status                                                     +
 * Output   :     status                                                     +
 * Return   :     True/False                                                 +
 * Note     :     none                                                       +
 *---------------------------------------------------------------------------*/
BOOL_T SNMP_MGR_Get_AgentStatus(BOOL_T *status);

/*---------------------------------------------------------------------------+
 * Routine Name : SNMP_MGR_GetRunningSnmpAgentStatus                         +
 *---------------------------------------------------------------------------+
 * Purpose  :     This function will get the agent status                    +
 * Input    :     status                                                     +
 * Output   :     status                                                     +
 * RETURN   :     SYS_TYPE_GET_RUNNING_CFG_SUCCESS,                          +
 *                SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or                     +
 *                SYS_TYPE_GET_RUNNING_CFG_FAIL                              +
 * Note     :     for cli get running                                        +
 *---------------------------------------------------------------------------*/
UI32_T SNMP_MGR_GetRunningSnmpAgentStatus(BOOL_T *status);

/* FUNCTION NAME : SNMP_MGR_CreateDefault
 * PURPOSE:
 *      Greate the default SNMP v3 entry
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      operation_mode
 *
 * NOTES:
 *      For SNMP internal use only.
 */
void SNMP_MGR_CreateDefault(void);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the SNMP status .
 * INPUT:    None.
 * OUTPUT:   snmpstats
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:     none
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpStatus( SNMP_MGR_STATS_T   *snmpstats  );


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpCommCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the comm counter/
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpCommCounter(UI32_T  *snmp_counter);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3UserCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 user  counter/
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3UserCounter(UI32_T  *snmp_counter);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3SecurityToGroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 user  counter/
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3SecurityToGroupCounter(UI32_T  *snmp_counter);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3GroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 group  counter
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3GroupCounter(UI32_T  *snmp_counter);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3ViewCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 view counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3ViewCounter(UI32_T  *snmp_counter);



 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustCommCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the comm counter when a community  is created.
 * INPUT:    mode. (TRUE increase, FALSE descrease)
 * OUTPUT:   None.
 * RETURN:  SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustCommCounter(BOOL_T  mode);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3UserCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 user  counter when a user  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3UserCounter(BOOL_T  mode);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3SecurityToGroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 securityToGroup  counter when a user  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3SecurityToGroupCounter(BOOL_T  mode);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3GroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 group  counter when a group  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3GroupCounter(BOOL_T  mode);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3ViewCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 view  counter when a view  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3ViewCounter(BOOL_T  mode);


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetCommCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the comm counter when a community  is created.
 * INPUT:    mode. (TRUE increase, FALSE descrease)
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetCommCounter();


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3UserCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 user  counter when a user  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3UserCounter();


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3SecurityToGroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 securityToGroup  counter when a user  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3SecurityToGroupCounter();


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3GroupCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 group  counter when a group  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3GroupCounter();


 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3ViewCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 view  counter when a view  is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3ViewCounter();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3TargetAddrCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will reset the target addr entry counter.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3TargetAddrCounter();

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3TargetParamsCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will reset the target params counter.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3TargetParamsCounter();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_ResetSnmpV3NotifyCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will reset the notify counter
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_ResetSnmpV3NotifyCounter();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_IncrementTrapSendCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the trap_send counter when a trap is sent
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     snmp_stats is an global variable that store in snmp_mgr, we need
 *           to support an API for trap_mgr to increase this counters
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_IncrementTrapSendCounter();

#ifdef NET_SNMP_INCLUDES_H /* workaround: avoid to include net-snmp */
/*---------------------------------------------------------------------------------
 * Routine Name : SNMP_MGR_StringToObejctID
 *---------------------------------------------------------------------------
 * Purpose :      This function will transform the string to a ObjectID
 * Input    :       text_p
 * Output   :     oid_P, obj_length
 * Return   :      TRUE/FALSE
 * Note     :       None
 *---------------------------------------------------------------------------*/
BOOL_T SNMP_MGR_StringToObejctID(oid *oid_P, I8_T *text_p, UI32_T *obj_length);
#endif


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetRmonInitFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the RMON init flag.
 * INPUT:    flag.
 * OUTPUT:   flag
 * RETURN:   True/False
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_GetRmonInitFlag(BOOL_T *flag);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetRmonInitFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the RMON init flag.
 * INPUT:    flag.
 * OUTPUT:   none
 * RETURN:   True/False
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_SetRmonInitFlag(BOOL_T flag);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpCommunityEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The Get function of  the Rfc2576 CommunityEntry.
 * INPUT:    comm_entry.
 * OUTPUT:   comm_entry
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetSnmpCommunityEntry (SNMP_MGR_SnmpCommunityEntry_T *comm_entry);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNetSnmpInitFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the NetSnmp init flag.
 * INPUT:    flag.
 * OUTPUT:   flag
 * RETURN:   True/False
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_GetNetSnmpInitFlag(BOOL_T *flag);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetNetSnmpInitFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the NetSnmp init flag.
 * INPUT:    flag.
 * OUTPUT:   none
 * RETURN:   True/False
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_SetNetSnmpInitFlag(BOOL_T flag);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetNextSnmpCommunityEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The GetNext function of the Rfc2576 CommunityEntry.
 * INPUT:    comm_entry.
 * OUTPUT:   comm_entry
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextSnmpCommunityEntry( SNMP_MGR_SnmpCommunityEntry_T *comm_entry);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunitySecurityName
 *---------------------------------------------------------------------------
 * PURPOSE:  The Set function of the Rfc2576 MIB communuitSecurityName.
 * INPUT:    comm_string_name, sec_name
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunitySecurityName(char  *comm_string_name,  char *sec_name);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  The funtion to set the Rfc2576 Community MIB status.
 * INPUT:    comm_string_name, sec_name
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityStatus(char *comm_name, UI32_T row_status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityContextEngineID
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the Rfc2576 Community MIB EngineID
 * INPUT:    comm_name, engineID
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTES:    we not support to set engineID in present.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityContextEngineID(char *comm_name, UI8_T *engineID);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityContextName
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the Rfc2576 Community MIB ContextName
 * INPUT:    comm_name, context_name
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTES:    we not support to set engineID in present.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityContextName(char *comm_name, UI8_T *context_name);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityTransportTag
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the Rfc2576 Community MIB TransportTag
 * INPUT:    comm_name, transport_tag
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTES:    we not support to set engineID in present.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityTransportTag(char *comm_name, UI8_T *transport_tag);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityStorageType
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the Rfc2576 Community MIB StorageType
 * INPUT:    comm_name, storage_type
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTES:    we not support to set engineID in present.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityStorageType(char *comm_name, UI32_T storage_type);

#if  (SYS_CPNT_SNMP_MIB_STYLE == SYS_CPNT_SNMP_MIB_STYLE_ACCTON)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_Get_PrivateMibRootLen
 * ---------------------------------------------------------------------
 * PURPOSE: This function is for SNMP internal use to get the private Mib Root len.
 *
 * INPUT: none
 * OUTPUT: none
 * RETURN: SNMP_MGR_ERROR_OK;SNMP_MGR_ERROR_PARAMETER_NOT_MATCH;SNMP_MGR_ERROR_FAIL;
 * NOTES:1.none
 *
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_Get_PrivateMibRootLen();
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_CalPartialVarLen
 * ---------------------------------------------------------------------
 * PURPOSE: This function is for SNMP internal use to interpret the SSH
 *          use key from 1024 bytes to a small len and output.
 *
 * INPUT:  wholeVarMaxLen   wholeVarLen.  unitLen,         unitIndex,
 * OUTPUT: start,partialVarLen
 * RETURN: TRUE/FALSE;
 * NOTES:1.none
 *
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_CalPartialVarLen(UI32_T wholeVarMaxLen, UI32_T wholeVarLen,
                                 UI32_T unitLen, UI32_T unitIndex,
                     UI32_T *start, UI32_T *partialVarLen);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_CheckUsmUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE:  The function will check if the usmuserentry is exist
 * INPUT:      entry
 * OUTPUT:  none
 * RETURN:  1. success:    SNMP_MGR_ERROR_OK
 *                    2. failure:      SNMP_MGR_ERROR_EXCEED_LIMIT
 *                                         SNMP_MGR_ERROR_MEM_ALLOC_FAIL
 *                                         SNMP_MGR_ERROR_PARAMETER_NOT_MATCH
 *                                         SNMP_MGR_ERROR_FAIL
 * NOTE:
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_CheckUsmUserEntry( SNMP_MGR_SnmpV3UserEntry_T  *entry);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_ReqSendTrap
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : Variable's instance and value that should be bound in
 *             trap PDU.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before SNMP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
// void SNMP_MGR_ReqSendTrap(TRAP_EVENT_TrapData_T *trap_data_p);

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_CheckTrapEvent
 * ---------------------------------------------------------------------------
 *  FUNCTION: This function is ported from TRAP_MGR_TASK, we will check if there
 *            is any event and send the trap if any.
 *
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
void SNMP_MGR_CheckTrapEvent(void);

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_CheckSoftwareWatchdogEvent
 * ---------------------------------------------------------------------------
 *  FUNCTION: This function is used to check and response to software watchdog
 *            event.
 *
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
void SNMP_MGR_CheckSoftwareWatchdogEvent(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 * PURPOSE: This function is for SNMP provision complete.
 *
 * INPUT:  none
 * OUTPUT: none
 * RETURN: none;
 * NOTES:1.none
 *
 * ---------------------------------------------------------------------
 */
void SNMP_MGR_ProvisionComplete();

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_Print_snmpoid
 *---------------------------------------------------------------------------
 * PURPOSE:  This function use to print out the snmp oid.
 * INPUT:   *snmpoid
 * OUTPUT:  *snmpoid
 * RETURN:  none.
 *
 * NOTE:    none.
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_Print_snmpoid(SNMP_TYPE_Oid_T *snmpoid);

/*------------------------------------------------------------------|
 * ROUTINE NAME - SNMP_MGR_Packets1H                                    |
 * ROUTINE NAME - SNMP_MGR_Octets1H                                     |
 * ROUTINE NAME - SNMP_MGR_BCasts1H                                     |
 * ROUTINE NAME - SNMP_MGR_MCasts1H                                     |
 * ROUTINE NAME - SNMP_MGR_Fragments1H                                  |
 * ROUTINE NAME - SNMP_MGR_Collisions1H                                 |
 * ROUTINE NAME - SNMP_MGR_Errors1H                                     |
 *------------------------------------------------------------------|
 * FUNCTION: RMON history for 1 hour                                |
 *                                                                  |
 * INPUT   : UI32_T logical_unit - which unit to get                |
 *           UI32_T port         - which port to get                |
 * OUTPUT  : UI32_T *value       - value                            |
 * RETURN  : BOOL_T True : Successfully, False : Failed             |
 *------------------------------------------------------------------*/

BOOL_T SNMP_MGR_Packets1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Octets1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_BCasts1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_MCasts1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Fragments1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Collisions1H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Errors1H (UI32_T unit, UI32_T port, UI32_T *value);


/*------------------------------------------------------------------|
 * ROUTINE NAME - SNMP_MGR_Packets6H                                   |
 * ROUTINE NAME - SNMP_MGR_Octets6H                                    |
 * ROUTINE NAME - SNMP_MGR_BCasts6H                                    |
 * ROUTINE NAME - SNMP_MGR_MCasts6H                                    |
 * ROUTINE NAME - SNMP_MGR_Fragments6H                                 |
 * ROUTINE NAME - SNMP_MGR_Collisions6H                                |
 * ROUTINE NAME - SNMP_MGR_Errors6H                                    |
 *------------------------------------------------------------------|
 * FUNCTION: RMON history for 6 hours                              |
 *                                                                  |
 * INPUT   : UI32_T logical_unit - which unit to get                |
 *           UI32_T port         - which port to get                |
 * OUTPUT  : UI32_T *value       - value                            |
 * RETURN  : BOOL_T True : Successfully, False : Failed             |
 *------------------------------------------------------------------*/
BOOL_T SNMP_MGR_Packets6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Octets6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_BCasts6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_MCasts6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Fragments6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Collisions6H (UI32_T unit, UI32_T port, UI32_T *value);
BOOL_T SNMP_MGR_Errors6H (UI32_T unit, UI32_T port, UI32_T *value);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_ReqSendTrap
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : Variable's instance and value that should be bound in
 *             trap PDU.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before SNMP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
void SNMP_MGR_ReqSendTrap(TRAP_EVENT_TrapData_T *trap_data_p);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_GetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 *  FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           is successfully retrieved.  Otherwise, return false.
 * INPUT   : None.
 * OUTPUT  : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_GetSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_SetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_SetSnmpEnableAuthenTraps(UI8_T snmp_enable_authen_traps);
/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_SetSnmpEnableLinkUpDownTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : link_up_down_traps - VAL_ifLinkUpDownTrapEnable_enabled /
 *                                      VAL_ifLinkUpDownTrapEnable_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_SetSnmpEnableLinkUpDownTraps(UI8_T link_up_down_traps);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetSnmpCommunityName
 *---------------------------------------------------------------------------
 * PURPOSE:  The Set function of the Rfc2576 MIB communuitName.
 * INPUT:    comm_string_name, sec_name
 * OUTPUT:   none.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL.
 * NOTE:     For SNMP Use only.
 *---------------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SetSnmpCommunityName(char  *comm_string_name,  char *com_name);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_MGR_Get_PrivateMibRootLen
 * ---------------------------------------------------------------------
 *  FUNCTION: This API will get the private root len.
 *
 *  INPUT    : none
 *  OUTPUT   : None.
 *  RETURN   : the lenght of the private mib root.
 *  NOTE     : This procedure shall not be invoked before SNMP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_Get_PrivateMibRootLen();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SetNetSnmpParseErr
 * ---------------------------------------------------------------------
 * PURPOSE: This function for patching the Netsnmp5.1 memory leak issue.
 *
 * INPUT: *flag
 * OUTPUT: *flag
 * RETURN: TRUE;FALSE;
 * NOTES:   We provided a flag in snmp_mgr, when this flag is on, we will
 *          ignore line4790, snmp_api.c, function snmp_free_pdu
 *          if (pdu->command ==0) and go on to free the pdu.
 *          Since we do not know what case  will make the pdu->command = 0,
 *          we will set this flag on in function _sess_process_packet, ln
 *          ret = snmp_parse(sessp, sp, pdu, packetptr, length)(ln4907),then
 *          when function snmp_free_pdu see this flag is true, can go on
 *          to free the pdu.
 *
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_SetNetSnmpParseErr( BOOL_T flag);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_IsNetSnmpParseErr
 * ---------------------------------------------------------------------
 * PURPOSE: This function for patching the Netsnmp5.1 memory leak issue.
 *
 * INPUT: *flag
 * OUTPUT: none.
 * RETURN: TRUE;FALSE;
 * NOTES:   We provided a flag in snmp_mgr, when this flag is on, we will
 *          ignore line4790, snmp_api.c, function snmp_free_pdu
 *          if (pdu->command ==0) and go on to free the pdu.
 *          Since we do not know what case  will make the pdu->command = 0,
 *          we will set this flag on in function _sess_process_packet, ln
 *          ret = snmp_parse(sessp, sp, pdu, packetptr, length)(ln4907),then
 *          when function snmp_free_pdu see this flag is true, can go on
 *          to free the pdu.
 *
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_IsNetSnmpParseErr();


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_IsInetAddrLengthValid
 * ---------------------------------------------------------------------
 * PURPOSE: This function check whether an InetAddress length is valid,
 *          based on a given InetAddressType.
 *
 * INPUT:   type - InetAddressType value
 *          len  - length (bytes) of InetAddress value (octet string)
 *
 * OUTPUT:  None
 *
 * RETURN:  TRUE/FALSE
 *
 * NOTE:    1. This function checks IPv4, IPv4z, IPv6, IPv6z, and DNS.
 *
 *          2. If "type" is unknown(0), this function accepts only empty address.
 *
 *          3. If the caller needs to specify only some of these address types,
 *             we will pass another parameter in, as a support type bitmask.
 *             However, if, before calling this function, address type check
 *             has already been done, e.g. in the corresponding InetAddressType
 *             MIB node, then bitmasking may not be needed.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_IsInetAddrLengthValid(UI32_T type, UI32_T len);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_IsInetAddrValueValid
 * ---------------------------------------------------------------------
 * PURPOSE: This function check whether an InetAddress value is valid,
 *          based on a given InetAddressType.
 *
 * INPUT:   type        - InetAddressType value
 *          addr_ar_p   - length (bytes) of InetAddress value (octet string)
 *
 * OUTPUT:  None
 *
 * RETURN:  TRUE/FALSE
 *
 * NOTE:    1. Because this is not an export function, the length of the
 *             the buffer in "addr_ar_p" is not passed in.  The caller
 *             of this function must ensure proper length of the content.
 *
 *          1. This function checks IPv4, IPv4z, IPv6, IPv6z, and DNS.
 *
 *          2. If "type" is unknown(0), this function always return TRUE.
 *
 *          3. If the caller needs to specify only some of these address types,
 *             we will pass another parameter in, as a support type bitmask.
 *             However, if, before calling this function, address type check
 *             has already been done, e.g. in the corresponding InetAddressType
 *             MIB node, then bitmasking may not be needed.
 * ---------------------------------------------------------------------
 */
BOOL_T SNMP_MGR_IsInetAddrValueValid(UI32_T type, UI8_T *addr_ar_p);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_GetNextTrapReceiverForV1V2
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the next available v1v2 trap receiver
 *          can be retrieved successfully. Otherwise, error code is returned.
 *
 * INPUT: entry->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: entry            - next available v1 v2 trap receiver info
 * RETURN: SNMP_MGR_ERROR_OK;   SNMP_MGR_ERROR_FAIL;
 * NOTES:1.This function will return trap receiver from the smallest ip addr.
 *       2.To get the first entry, input the key as 0.
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_GetNextTrapReceiverForV1V2(SNMP_MGR_TrapDestEntry_T *entry);

#if(SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)
enum SNMP_MGR_CONFIG_ERROR_E
{
       SNMP_MGR_CONFIG_ERROR_OK = 0,
       SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
       SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
       SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
       SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
       SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
       SNMP_MGR_CONFIG_ERROR_SET_TRAP,
       SNMP_MGR_CONFIG_ERROR_GET_TRAP
};

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SnmpNotifyCreate
 * ---------------------------------------------------------------------
 * PURPOSE: This function create v3 inform and trap by one function.
 * INPUT:target_addr_name,notify_type,
 *             user_name, dest_ip_addres, dest_port,
 *             remote_engineID,  remote_engineIDLen,
 *             password_from_config,  auth_protocol,
 *             auth_key_len,  authentication_password,
 *             priv_protocol, priv_key_len,
 *             privacy_password,retry_interval,
 *             retry_count
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */
UI32_T SNMP_MGR_SnmpNotifyCreate(UI8_T *target_addr_name,UI32_T notify_type,
            UI8_T *user_name, UI32_T dest_ip_addres, UI32_T dest_port,
            UI8_T *remote_engineID, UI32_T  remote_engineIDLen,
            BOOL_T password_from_config,  UI32_T auth_protocol,
            UI32_T auth_key_len, UI8_T * authentication_password,
            UI32_T  priv_protocol, UI32_T priv_key_len,
            UI8_T *privacy_password,UI32_T retry_interval,
            UI32_T retry_count );

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SnmpNotifyDel
 * ---------------------------------------------------------------------
 * PURPOSE: This function del v3 inform and trap by one function.
 * INPUT:target_addr_name
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */

UI32_T SNMP_MGR_SnmpNotifyDel(UI8_T *target_addr_name );


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_MGR_SnmpNotifyModify
 * ---------------------------------------------------------------------
 * PURPOSE: This function modify v3 inform and trap by one function.
 * INPUT:target_addr_name,notify_type,
 *             user_name, dest_ip_addres, dest_port,
 *             remote_engineID,  remote_engineIDLen,
 *             password_from_config,  auth_protocol,
 *             auth_key_len,  authentication_password,
 *             priv_protocol, priv_key_len,
 *             privacy_password,retry_interval,
 *             retry_count
 * OUTPUT: SNMP_MGR_CONFIG_ERROR_E
 * RETURN: SNMP_MGR_ERROR_OK;
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_CREATE_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_USER,
 *        SNMP_MGR_CONFIG_ERROR_DEL_REMOTE_ENGINEID,
 *        SNMP_MGR_CONFIG_ERROR_DEL_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_SET_TRAP,
 *        SNMP_MGR_CONFIG_ERROR_GET_TRAP
 * NOTE:  target_addr_name , dest_ip_addres , ( remote_engineID + user_name ) is key
 * ---------------------------------------------------------------------
 */

UI32_T SNMP_MGR_SnmpNotifyModify(UI8_T *target_addr_name,UI32_T notify_type,
            UI8_T *user_name, UI32_T dest_ip_addres, UI32_T dest_port,
            UI8_T *remote_engineID, UI32_T  remote_engineIDLen,
            BOOL_T password_from_config,  UI32_T auth_protocol,
            UI32_T auth_key_len, UI8_T * authentication_password,
            UI32_T  priv_protocol, UI32_T priv_key_len,
            UI8_T *privacy_password,UI32_T retry_interval,
            UI32_T retry_count);
#endif /* (SYS_CPNT_SNMPV3_TARGET_NAME_STYLE == SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM)            */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_BitsFromCoreToSnmp
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the convert from core layer to snmp is ok.
 *          Otherwise, SNMP_MGR_ERROR_FAIL is returned.
 *
 * INPUT: core_layer_p - variable pointer
 *        snmp_p       - variable pointer
 *        var_size - memory size in bytes for core_layer_p and snmp_p
 * OUTPUT: snmp_p
 * RETURN:  1. success: SNMP_MGR_ERROR_OK
 *          2. failure: SNMP_MGR_ERROR_FAIL
 * NOTES: 1.The var_size of snmp_p must be the same as core_layer_p
 */
UI32_T SNMP_MGR_BitsFromCoreToSnmp(void *core_layer_p, UI8_T *snmp_p, UI32_T var_size);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_BitsFromSnmpToCore
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SNMP_MGR_ERROR_OK if the convert from core layer to snmp is ok.
 *          Otherwise, SNMP_MGR_ERROR_FAIL is returned.
 *
 * INPUT: core_layer_p - variable pointer
 *        snmp_p       - variable pointer
 *        var_size - memory size in bytes for core_layer_p and snmp_p
 * OUTPUT: snmp_p
 * RETURN:  1. success: SNMP_MGR_ERROR_OK
 *          2. failure: SNMP_MGR_ERROR_FAIL
 * NOTES: 1.The var_size of snmp_p must be the same as core_layer_p
 */
UI32_T SNMP_MGR_BitsFromSnmpToCore(void *core_layer_p, UI8_T *snmp_p, UI32_T var_size);

#if(SNMP_MGR_DEBUG_CONTROL==TRUE)

void SNMP_MGR_Debug(I8_T *str,I8_T type ,I8_T *value, UI32_T length);

#define SNMP_MGR_DEBUG_M(str) \
{\
SNMP_MGR_Debug((str),'m',(str),0);\
}
#define SNMP_MGR_DEBUG_S(str_m,str) \
{ \
SNMP_MGR_Debug((str_m),'s',(str),0);\
}
#define SNMP_MGR_DEBUG_I(str,value) \
{ \
UI32_T SNMP_MGR_Debug_TEMP_I;\
SNMP_MGR_Debug_TEMP_I = value; \
SNMP_MGR_Debug((str),'i',(I8_T *)(&(SNMP_MGR_Debug_TEMP_I)),0);\
}
#define SNMP_MGR_DEBUG_O(str,value,length) \
{ \
SNMP_MGR_Debug((str),'o',(I8_T *)(value),(length));\
}
#define SNMP_MGR_DEBUG_V(str,value,length) \
{ \
SNMP_MGR_Debug((str),'v',(I8_T *)(value),(length));\
}
#define SNMP_MGR_DEBUG_F() \
{ \
    UI32_T SNMP_MGR_Debug_TEMP_I;\
    SNMP_MGR_Debug_TEMP_I = __LINE__; \
    SNMP_MGR_Debug((__FUNCTION__),'i',(I8_T *)(&(SNMP_MGR_Debug_TEMP_I)),0);\
}
#define SNMP_MGR_DEBUG_T(str) \
{\
SNMP_MGR_Debug((str),'t',(str),0);\
}

#define SNMP_MGR_DEBUG_LINE() \
{  \
  printf("\r\n %s %d",__FUNCTION__,__LINE__); \
  fflush(stdout); \
}

#define SNMP_MGR_DEBUG_MSG(a,b...)   \
{ \
  printf(a,##b); \
  fflush(stdout); \
}

#else
#define SNMP_MGR_DEBUG_M(str)
#define SNMP_MGR_DEBUG_S(str_m,str)
#define SNMP_MGR_DEBUG_I(str,value)
#define SNMP_MGR_DEBUG_O(str,value,length)
#define SNMP_MGR_DEBUG_V(str,value,length)
#define SNMP_MGR_DEBUG_F()
#define SNMP_MGR_DEBUG_T(str)
#define SNMP_MGR_DEBUG_LINE()
#define SNMP_MGR_DEBUG_MSG(a,b...)
#endif

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3NotifyFilterProfileCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 notify filter profile counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   None.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_GetSnmpV3NotifyFilterProfileCounter(UI32_T  *snmp_counter);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3NotifyCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 notify counter when a
 *           notify entry is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3NotifyCounter(BOOL_T  mode);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3NotifyFilterProfileCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 notify counter when a
 *           notify entry is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
void  SNMP_MGR_AdjustSnmpV3NotifyFilterProfileCounter(BOOL_T  mode);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3NotifyFilterCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 notify filter counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   None.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
void  SNMP_MGR_GetSnmpV3NotifyFilterCounter(UI32_T  *snmp_counter);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3NotifyFilterCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 notify counter when a
 *           notify entry is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
void  SNMP_MGR_AdjustSnmpV3NotifyFilterCounter(BOOL_T  mode);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3NotifyCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 view counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3NotifyCounter(UI32_T  *snmp_counter);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3TargetAddrCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 target addr  counter when
 *           a target addr entry is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3TargetAddrCounter(BOOL_T  mode);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3TargetAddrCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 target addr counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3TargetAddrCounter(UI32_T  *snmp_counter);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_AdjustSnmpV3TargetParamsCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will increase the snmpv3 target params  counter
 *           when a target params is created.
 * INPUT:    mode.
 * OUTPUT:   None.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_AdjustSnmpV3TargetParamsCounter(BOOL_T  mode);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_GetSnmpV3TargetParamsCounter
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get  the snmpv3 target params counter.
 * INPUT:    counter.
 * OUTPUT:   counter.
 * RETURN:   SNMP_MGR_ERROR_OK/SNMP_MGR_ERROR_FAIL
 * NOTE:     For SNMP agent internal use only.
 *---------------------------------------------------------------------------
 */
UI32_T  SNMP_MGR_GetSnmpV3TargetParamsCounter(UI32_T  *snmp_counter);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SNMP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SNMP_MGR_Create_InterCSC_Relation(void);

#if(SYS_CPNT_CLUSTER==TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SNMP_MGR_SetClusterRole
 *---------------------------------------------------------------------------
 * PURPOSE:  This function is set snmp next cluster role.
 * INPUT:    role
 * OUTPUT:
 * RETURN:
 * NOTE:
 *---------------------------------------------------------------------------
 */
void SNMP_MGR_SetClusterRole (UI32_T role);
#endif /*end of #if(SYS_CPNT_CLUSTER==TRUE)*/

/* need to move the inclusion of sysfun.h before the macro definition of
 * dprintf above. If include sysfun.h here, it will lead to compile error
 * because the function dprintf is defined in lib C header file stdio.h
 * and it will be included through sysfun.h.
 * When the macro definition of dprintf is before the inclusion of stdio.h,
 * it will lead to errorneous expansion of dprintf in stdio.h
 */
#if 0
/*********eli test*****************/
#include "sysfun.h"
#endif

BOOL_T SNMP_MGR_HandleIPCReqMsg(   SYSFUN_Msg_T* req_msgbuf_p);

#if (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE)
/* Notification Log MIB -- Meiling Hou 2008.04.15 */
BOOL_T SNMP_MGR_CheckExistNotifyFilter(UI32_T trap_type, UI32_T specific_type,
                    UI32_T *enterprise, UI32_T enterprise_length, char *filter_name);
BOOL_T SNMP_MGR_CheckNotifyFilterName(char *filter_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SNMP_MGR_GetSnmpNotifyFilterProfileTableByProfileName
 *-------------------------------------------------------------------------
 * PURPOSE : Get notify filter profile entry by profile name
 * INPUT   : entry->snmp_notify_filter_profile_name -- to specify a profile name
 * OUTPUT  : entry  -- notify filter profile entry
 * RETURN  : SNMP_MGR_ERROR_OK    -- success
 *           SNMP_MGR_ERROR_FAIL  -- fail
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
UI32_T
SNMP_MGR_GetSnmpNotifyFilterProfileTableByProfileName(
    SNMP_MGR_SnmpNotifyFilterProfileEntry_T *entry
);
#endif  /* (SYS_CPNT_NOTIFICATIONLOG_MIB == TRUE) */
char *SNMP_MGR_GetToken (char *s, char *Token, char delemiters[]);
UI16_T SNMP_MGR_CheckNullStr(char *buff);
BOOL_T SNMP_MGR_GetLowerUpperValue(char *buff, UI32_T *lower_val, UI32_T *upper_val, UI32_T *err_idx);
BOOL_T SNMP_MGR_IsMeetDelemiter(char c, char delemiters[]);
void SNMP_MGR_BitmapToString(UI8_T *bitmap, UI32_T bitmap_size, char *string, UI32_T str_size);
BOOL_T SNMP_MGR_StringToBitmap(char *string, UI8_T *bitmap, UI32_T bitmap_size);

#endif  /* SNMP_MGR_H */
