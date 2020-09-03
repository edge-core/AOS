#include "sys_bld.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include <stdlib.h>
#include <ctype.h>
#include "radius_type.h"
#include "l_inet.h"

#ifndef  RADIUSCLIENT_H
#define  RADIUSCLIENT_H

#define  RADIUS_MAX_MAC_STRING_LENGTH       17 /*xx-xx-xx-xx-xx-xx*/
/*for EH*//*Mercury_V2-00030*/
#define  RADIUS_RC_AVPAIR_NEW_FUNC_NO       1
#define  RADIUS_RC_AVPAIR_GEN_FUNC_NO       2
#define  RADIUS_SET_OPTION_STR_FUNC_NO      3
#define  RADIUS_SET_OPTION_INIT_FUNC_NO     4
#define  RADIUS_SET_OPTION_SRV_FUNC_NO      5
#define  RADIUS_SET_OPTION_AUO_FUNC_NO      6
#define  RADIUS_MGR_SET_SERVER_IP_FUNC_NO   7
#define  RADIUS_OM_Set_Server_Secret_FUNC_NO    8
#define  RADIUS_OM_Set_Request_Timeout_FUNC_NO  9
#define  RADIUS_OM_Set_Server_Port_FUNC_NO  10
#define  RADIUS_OM_Set_Retransmit_Times_FUNC_NO 11
#define  RADIUS_TASK_CreateRadiusTask_FUNC_NO   12

#define  RADIUS_SUPPORT_RADA    2
#define  RADIUS_SUPPORT_EAP     1
#define  RADIUS_NOT_SUPPORT_EAP 0

#ifndef UCHAR_MAX /* UCHAR_MAX should be define in limits.h */
#define  UCHAR_MAX       255
#endif

#define  RADIUS_EAP_HDRLEN  4
#define  RADIUS_EAP_RRIDENTITY  1


#undef __BEGIN_DECLS
#undef __END_DECLS
#ifdef __cplusplus
# define __BEGIN_DECLS extern "C" {
# define __END_DECLS }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif



#undef __P
#if defined (__STDC__) || defined (_AIX) || (defined (__mips) && defined (_SYSTYPE_SVR4)) || defined(WIN32) || defined(__cplusplus)
# define __P(protos) protos
#else
# define __P(protos) ()
#endif
#define EAP_START               2
#define AUTH_ID_LEN     64

#define NAME_LENGTH     32
#define GETSTR_LENGTH       128 /* must be bigger than AUTH_PASS_LEN */

/* defines for config.c */
#define SERVER_MAX  SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS
#define AUTH_LOCAL_FST  (1<<0)
#define AUTH_RADIUS_FST (1<<1)
#define AUTH_LOCAL_SND  (1<<2)
#define AUTH_RADIUS_SND (1<<3)
/*******from rc_send_server() ***********/
#if 0
typedef struct server {
    int max;
    char *name[SERVER_MAX];
    unsigned short port[SERVER_MAX];
} SERVER;
#endif
#if 0
#define          MAX_SECRET_LENGTH      SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH /* MUST be multiple of 16 */
/************ For MIB *************************/
typedef struct radiusAuthtab
{
    UI32_T  radiusAuthServerIndex;
    UI32_T  radiusAuthServerAddress;
    UI32_T  radiusAuthClientServerPortNumber;
    UI32_T  radiusAuthClientRoundTripTime;
    UI32_T  radiusAuthClientAccessRequests;
    UI32_T  radiusAuthClientAccessRetransmissions;
    UI32_T  radiusAuthClientAccessAccepts;
    UI32_T  radiusAuthClientAccessRejects;
    UI32_T  radiusAuthClientAccessChallenges;
    UI32_T  radiusAuthClientMalformedAccessResponses;
    UI32_T  radiusAuthClientBadAuthenticators;
    UI32_T  radiusAuthClientPendingRequests;
    UI32_T  radiusAuthClientTimeouts;
    UI32_T  radiusAuthClientUnknownTypes;
    UI32_T  radiusAuthClientPacketsDropped;
}AuthServerEntry;

typedef struct RADIUS_Server_Host_S {
    BOOL_T   used_flag;
    UI32_T   server_ip;
    UI32_T   server_port;
        UI32_T   timeout;
        UI32_T   retransmit;
        UI32_T   server_index;
        UI8_T    secret[MAX_SECRET_LENGTH+1];
        AuthServerEntry server_table;
} RADIUS_Server_Host_T;
#endif
typedef struct pw_auth_hdr
{
    UI8_T          code;
    UI8_T          id;
    UI16_T         length;
    UI8_T          vector[AUTH_VECTOR_LEN];
    UI8_T          data[2];
} AUTH_HDR;

#define AUTH_HDR_LEN            20

#define CHAP_VALUE_LENGTH       16

#define PW_AUTH_UDP_PORT        1812   /*kevin 1645*/
#define PW_ACCT_UDP_PORT        1813   /*kevin 1646*/

#define PW_TYPE_STRING          0
#define PW_TYPE_INTEGER         1
#define PW_TYPE_IPADDR          2
#define PW_TYPE_DATE            3
#define PW_TYPE_IP6ADDR         4

/* standard RADIUS codes */

#define PW_ACCESS_REQUEST       1
#define PW_ACCESS_ACCEPT        2
#define PW_ACCESS_REJECT        3
#define PW_ACCOUNTING_REQUEST   4
#define PW_ACCOUNTING_RESPONSE  5
#define PW_ACCOUNTING_STATUS    6
#define PW_PASSWORD_REQUEST     7
#define PW_PASSWORD_ACK         8
#define PW_PASSWORD_REJECT      9
#define PW_ACCOUNTING_MESSAGE   10
#define PW_ACCESS_CHALLENGE     11
#define PW_STATUS_SERVER        12
#define PW_STATUS_CLIENT        13


/* standard RADIUS attribute-value pairs */

#define PW_USER_NAME            1   /* string */
#define PW_USER_PASSWORD        2   /* string */
#define PW_CHAP_PASSWORD        3   /* string */
#define PW_NAS_IP_ADDRESS       4   /* ipaddr */
#define PW_NAS_PORT             5   /* integer */
#define PW_SERVICE_TYPE         6   /* integer */
#define PW_FRAMED_PROTOCOL      7   /* integer */
#define PW_FRAMED_IP_ADDRESS    8   /* ipaddr */
#define PW_FRAMED_IP_NETMASK    9   /* ipaddr */
#define PW_FRAMED_ROUTING       10  /* integer */
#define PW_FILTER_ID            11  /* string */
#define PW_FRAMED_MTU           12  /* integer */
#define PW_FRAMED_COMPRESSION   13  /* integer */
#define PW_LOGIN_IP_HOST        14  /* ipaddr */
#define PW_LOGIN_SERVICE        15  /* integer */
#define PW_LOGIN_PORT           16  /* integer */
#define PW_OLD_PASSWORD         17  /* string */ /* deprecated */
#define PW_REPLY_MESSAGE        18  /* string */
#define PW_LOGIN_CALLBACK_NUMBER    19  /* string */
#define PW_FRAMED_CALLBACK_ID       20  /* string */
#define PW_EXPIRATION           21  /* date */ /* deprecated */
#define PW_FRAMED_ROUTE         22  /* string */
#define PW_FRAMED_IPX_NETWORK   23  /* integer */
#define PW_STATE                24  /* string */
#define PW_CLASS                25  /* string */
#define PW_VENDOR_SPECIFIC      26  /* string */
#define PW_SESSION_TIMEOUT      27  /* integer */
#define PW_IDLE_TIMEOUT         28  /* integer */
#define PW_TERMINATION_ACTION   29  /* integer */
#define PW_CALLED_STATION_ID    30  /* string */
#define PW_CALLING_STATION_ID   31  /* string */
#define PW_NAS_IDENTIFIER       32  /* string */
#define PW_PROXY_STATE          33  /* string */
#define PW_LOGIN_LAT_SERVICE    34  /* string */
#define PW_LOGIN_LAT_NODE       35  /* string */
#define PW_LOGIN_LAT_GROUP      36  /* string */
#define PW_FRAMED_APPLETALK_LINK    37  /* integer */
#define PW_FRAMED_APPLETALK_NETWORK 38  /* integer */
#define PW_FRAMED_APPLETALK_ZONE    39  /* string */
#define PW_CHAP_CHALLENGE       60      /* string */
#define PW_NAS_PORT_TYPE        61      /* integer */
#define PW_PORT_LIMIT           62      /* integer */
#define PW_LOGIN_LAT_PORT       63      /* string */
#define PW_TUNNEL_TYPE          64      /* integer */
#define PW_TUNNEL_MEDIUM_TYPE   65      /* integer */

/* RADIUS Extensions */
#define PW_ARAP_PASSWORD        70  /* string */
#define PW_ARAP_FEATURES        71  /* string */
#define PW_ARAP_ZONE_ACCESS     72  /* integer */
#define PW_ARAP_SECURITY        73  /* integer */
#define PW_ARAP_SECURITY_DATA   74  /* string */
#define PW_PASSWORD_RETRY       75  /* integer */
#define PW_PROMPT               76  /* integer */
#define PW_CONNECT_INFO         77  /* string */
#define PW_CONFIGURATION_TOKEN  78  /* string */
#define PW_EAP_MESSAGE          79  /* string */
#define PW_MESSAGE_AUTHENTICATOR    80  /* string */
#define PW_TUNNEL_PRIVATE_GROUP     81  /* string */
#define PW_ARAP_CHALLENGE_RESPONSE  84  /* string */
#define PW_NAS_PORT_ID_STRING       87  /* string */
#define PW_FRAMED_POOL          88  /* string */
#define PW_NAS_IPV6_ADDRESS     95  /* ip6addr */
/*  Accounting */

#define PW_ACCT_STATUS_TYPE     40  /* integer */
#define PW_ACCT_DELAY_TIME      41  /* integer */
#define PW_ACCT_INPUT_OCTETS    42  /* integer */
#define PW_ACCT_OUTPUT_OCTETS   43  /* integer */
#define PW_ACCT_SESSION_ID      44  /* string */
#define PW_ACCT_AUTHENTIC       45  /* integer */
#define PW_ACCT_SESSION_TIME    46  /* integer */
#define PW_ACCT_INPUT_PACKETS   47  /* integer */
#define PW_ACCT_OUTPUT_PACKETS  48  /* integer */
#define PW_ACCT_TERMINATE_CAUSE     49  /* integer */
#define PW_ACCT_MULTI_SESSION_ID    50  /* string */
#define PW_ACCT_LINK_COUNT      51  /* integer */

/*  Merit Experimental Extensions */

#define PW_USER_ID                      222     /* string */
#define PW_USER_REALM                   223     /* string */

/*  Integer Translations */

/*  SERVICE TYPES   */

#define PW_LOGIN            1
#define PW_FRAMED           2
#define PW_CALLBACK_LOGIN       3
#define PW_CALLBACK_FRAMED      4
#define PW_OUTBOUND         5
#define PW_ADMINISTRATIVE       6
#define PW_NAS_PROMPT                   7
#define PW_AUTHENTICATE_ONLY        8
#define PW_CALLBACK_NAS_PROMPT          9

/*  FRAMED PROTOCOLS    */

#define PW_PPP              1
#define PW_SLIP             2
#define PW_ARA                          3
#define PW_GANDALF                      4
#define PW_XYLOGICS                     5

/*  FRAMED ROUTING VALUES   */

#define PW_NONE             0
#define PW_BROADCAST            1
#define PW_LISTEN           2
#define PW_BROADCAST_LISTEN     3

/*  FRAMED COMPRESSION TYPES    */

#define PW_VAN_JACOBSON_TCP_IP      1
#define PW_IPX_HEADER_COMPRESSION   2

/*  LOGIN SERVICES  */

#define PW_TELNET                       0
#define PW_RLOGIN                       1
#define PW_TCP_CLEAR                    2
#define PW_PORTMASTER                   3
#define PW_LAT                          4
#define PW_X25_PAD                      5
#define PW_X25_T3POS                    6

/*  TERMINATION ACTIONS */

#define PW_DEFAULT          0
#define PW_RADIUS_REQUEST       1

/*  PROHIBIT PROTOCOL  */

#define PW_DUMB     0   /* 1 and 2 are defined in FRAMED PROTOCOLS */
#define PW_AUTH_ONLY    3
#define PW_ALL      255

/*  ACCOUNTING STATUS TYPES    */

#define PW_STATUS_START     1
#define PW_STATUS_STOP      2
#define PW_STATUS_ALIVE     3
#define PW_STATUS_MODEM_START   4
#define PW_STATUS_MODEM_STOP    5
#define PW_STATUS_CANCEL    6
#define PW_ACCOUNTING_ON    7
#define PW_ACCOUNTING_OFF   8

/*      ACCOUNTING TERMINATION CAUSES   */

#define PW_USER_REQUEST         1
#define PW_LOST_CARRIER         2
#define PW_LOST_SERVICE         3
#define PW_ACCT_IDLE_TIMEOUT    4
#define PW_ACCT_SESSION_TIMEOUT 5
#define PW_ADMIN_RESET          6
#define PW_ADMIN_REBOOT         7
#define PW_PORT_ERROR           8
#define PW_NAS_ERROR            9
#define PW_NAS_REQUEST          10
#define PW_NAS_REBOOT           11
#define PW_PORT_UNNEEDED        12
#define PW_PORT_PREEMPTED       13
#define PW_PORT_SUSPENDED       14
#define PW_SERVICE_UNAVAILABLE  15
#define PW_CALLBACK             16
#define PW_USER_ERROR           17
#define PW_HOST_REQUEST         18

/*     NAS PORT TYPES    */

#define PW_ASYNC        0
#define PW_SYNC         1
#define PW_ISDN_SYNC        2
#define PW_ISDN_SYNC_V120   3
#define PW_ISDN_SYNC_V110   4
#define PW_VIRTUAL      5

/*     AUTHENTIC TYPES */
#define PW_RADIUS   1
#define PW_LOCAL    2
#define PW_REMOTE   3
#define PW_ETHERNET 15
#define PW_802_1X      19

/*  TUNNEL TYPES */
#define PW_PPTP       1
#define PW_L2F        2
#define PW_L2TP       3
#define PW_ATMP       4
#define PW_VTP          5
#define PW_AH           6
#define PW_IP_IP        7
#define PW_MIN_IP_IP    8
#define PW_ESP          9
#define PW_GRE         10
#define PW_DVS         11
#define PW_IP_IN_IP    12
#define PW_VLAN        13

/*  TUNNEL MEDIUM TYPES */
#define PW_IPv4       1
#define PW_IPv6       2
#define PW_NSAP       3
#define PW_HDLC       4
#define PW_BBN_1822 5
#define PW_802        6
#define PW_E_163          7
#define PW_E_164          8

#define PW_CISCO_AVPAIR      ((9<<16)|1)

/* Server data structures */

typedef struct dict_attr
{
    char              name[NAME_LENGTH + 1];    /* attribute name */
    int               value;            /* attribute index */
    int               type;             /* string, int, etc. */
/*  struct dict_attr *next; kevin*/
} DICT_ATTR;

typedef struct dict_value
{
    char               attrname[NAME_LENGTH +1];
    char               name[NAME_LENGTH + 1];
    int                value;
/*  struct dict_value *next;*/
} DICT_VALUE;
/*****kevin**********/
#if 0
DICT_VALUE dict_value_table[]={
{"Service-Type","Login-User",          1},
{"Service-Type","Framed-User",         2},
{"Service-Type","Callback-Login-User", 3},
{"Service-Type","Callback-Framed-User",4},
{"Service-Type","Outbound-User",       5},
{"Service-Type","Administrative",      6},
{"Service-Type","NAS-Prompt-User",     7},
{"Framed-Protocol","PPPP",             1},
{"Framed-Protocol","SLIP",             2},
{"Framed-Routing","None",              0},
{"Framed-Routing","Broadcast",         1},
{"Framed-Routing","Listen",            2},
{"Framed-Routing","Broadcast-Listen",  3},
{"Framed-Compression","None",          0},
{"Framed-Compression","Van-Jacobson-TCP-IP",1},
{"Login-Service","Telnet",             0},
{"Login-Service","Rlogin",             1},
{"Login-Service","TCP-Clear",          2},
{"Login-Service","PortMaster",         3},
{"Acct-Status","TypeStart",            1},
{"Acct-Status","TypeStop",             2},
{"Acct-Status","TypeAccounting-On",    7},
{"Acct-Status","TypeAccounting-Off",   8},
{"Acct-Authentic","RADIUS",            1},
{"Acct-Authentic","Local",             2},
{"Acct-Authentic","PowerLink128",    100},
{"Termination-Action","Default",       0},
{"Termination-Action","RADIUS-Request",1},
{"NAS-Port-Type","Async",              0},
{"NAS-Port-Type","Sync",               1},
{"NAS-Port-Type","ISDN",               2},
{"NAS-Port-Type","ISDN-V120",          3},
{"NAS-Port-Type","ISDN-V110",          4},
{"Acct-Terminate-Cause","User-Request",1},
{"Acct-Terminate-Cause","Lost-Carrier",2},
{"Acct-Terminate-Cause","Lost-Service",3},
{"Acct-Terminate-Cause","Idle-Timeout",4},
{"Acct-Terminate-Cause","Session-Rimeout",5},
{"Acct-Terminate-Cause","Admin-Reset", 6},
{"Acct-Terminate-Cause","Admin-Reboot",7},
{"Acct-Terminate-Cause","Port-Error",  8},
{"Acct-Terminate-Cause","NAS-Error",   9},
{"Acct-Terminate-Cause","NAS-Request", 10},
{"Acct-Terminate-Cause","NAS-Reboot",  11},
{"Acct-Terminate-Cause","Port-Unneeded",12},
{"Acct-Terminate-Cause","Port-Preempted",13},
{"Acct-Terminate-Cause","Port-Suspended",14},
{"Acct-Terminate-Cause","Service-Unavailable",15},
{"Acct-Terminate-Cause","Callback",      16},
{"Acct-Terminate-Cause","User-Error",    17},
{"Acct-Terminate-Cause","Host-Request",  18},
{"Auth-Type","Local",0},
{"Auth-Type","System",1},
{"Auth-Type","SecurID",2},
{"Auth-Type","Crypt-Local",3},
{"Auth-Type","Reject",4},
{"Auth-Type","Pam",253},
{"Auth-Type","None",254},
{"Fall-Through","No",0},
{"Fall-Through","Yes",1},
{"Add-Port-To-IP-Address","No",0},
{"Add-Port-To-IP-Address","Yes",1},
{"Server-Config","Password-Expiration",30},
{"Server-Config","Password-Warning",5},
};
#endif
/*static int num_value=((sizeof(dict_value_table))/(sizeof(dict_value_table[0])));*/
/*int num_value=((sizeof(dict_value_table))/(sizeof(dict_value_table[0])));*/
/*******************/
typedef struct value_pair
{
    char               name[NAME_LENGTH + 1];
    int                attribute;
    int                type;
    UI32_T             lvalue;
    char               strvalue[AUTH_STRING_LEN + 1];
    struct value_pair *next;
} VALUE_PAIR;

/* don't change this, as it has to be the same as in the Merit radiusd code */
/*#define MGMT_POLL_SECRET  "Hardlyasecret"*/
#define MGMT_POLL_SECRET      "testing123"  /* modify by kevin */
/*  Define return codes from "SendServer" utility */
#if 0
#define NOT_IN_MASTER_MODE_RC  -3
#define BADRESP_RC  -2
#define ERROR_RC    -1
#define OK_RC       0
#define TIMEOUT_RC  1
#define CHALLENGE_RC    2
#endif
typedef struct send_data /* Used to pass information to sendserver() function */
{
    UI8_T          code;        /* RADIUS packet code */
    UI8_T          seq_nbr; /* Packet sequence number */
    UI32_T         server;      /* Name/addrress of RADIUS server */
    UI32_T         svc_port;    /* RADIUS protocol destination port */
    UI32_T         timeout; /* Session timeout in seconds */
    UI32_T      retries;
    VALUE_PAIR     *send_pairs;     /* More a/v pairs to send */
    VALUE_PAIR     *receive_pairs;  /* Where to place received a/v pairs */
} SEND_DATA;


#ifndef MAX
#define MAX(a, b)     ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)     ((a) < (b) ? (a) : (b))
#endif

/********************** for 802.1x *********************************/
#if 0 /* move to radius_type.h */
#define  EAP_MESSAGE_SIZE   AUTH_STRING_LEN
#endif

#define  STATE_MESSAGE_SIZE   AUTH_STRING_LEN

struct radius_eap
{
    unsigned char  code;
    unsigned char  identifier;
    unsigned short length;
};

struct radius_eap_rr
{
    unsigned char   type;
};
#if 0 /* move to radius_type.h */
typedef BOOL_T (*RadaAnnounceResult_T)(UI32_T lport,
                                    UI8_T *mac,
                                    int identifier,
                                    BOOL_T authorized_result,
                                    UI8_T *authorized_vlan_list,
                                    UI8_T *authorized_qos_list,
                                    UI32_T session_time);

typedef struct recv_eap_data
{
    UI8_T          data[EAP_MESSAGE_SIZE+1];
    UI32_T         len;
    UI32_T         src_port;
    UI8_T          src_mac[6];
    void           (*fun_ptr)();
} R_EAP_DATA;
#endif


struct Radius_pktbuf
{
    UI8_T    * pkt;
    UI32_T   length;    /* -1 indicates no packet is stored in this slot */
    UI8_T    eap_code;
    UI8_T    eap_id;
    UI8_T    eaprr_type;   /* = 255 if not valid i.e. the EAP packet is not a request/response pkt */
};

struct Radius_Auth_Pae_tag
{
    struct Radius_pktbuf   fromsrv;     /* buffers of length one to store latest packet  from server */
    struct radius_info    * rinfo; /* structure for radius related bookkeeping with respect
                     to sending packets to the radius server*/
};
typedef struct Radius_Auth_Pae_tag Radius_Auth_Pae;

typedef struct RADIUS_RADA_AUTH_MSG_S
{
    UI32_T  lport;
    UI32_T  cookie;     /* MSGQ_ID for return result */
    UI8_T   src_mac[6];
    char    rada_username[RADIUS_MAX_MAC_STRING_LENGTH+1];
    char    rada_passwd  [RADIUS_MAX_MAC_STRING_LENGTH+1];
}RADIUS_RADA_AUTH_MSG_T;

/*maggie liu for RADIUS authentication ansync*/
typedef struct RADIUS_ANSYNC_AUTH_MSG_S
{
    char    username[SYS_ADPT_MAX_USER_NAME_LEN +1];
    char    password  [SYS_ADPT_MAX_PASSWORD_LEN +1];
    I32_T  privilege;
    UI32_T  cookie;     /* MSGQ_ID for return result */
}RADIUS_ANSYNC_AUTH_MSG_T;


typedef struct  RADIUS_TASK_MSGQ_S
{
    UI8_T   radius_task_service_type;      /* encoding message type, source    */
    UI8_T   *mtext;     /* point message block  */
    UI16_T  mreserved1;
    UI32_T  mreserved2;  /* not defined for future extension */
    UI32_T  mreserved3;  /* not defined for future extension */
    UI32_T  mreserved4;  /* not defined for future extension */
}   RADIUS_TASK_MSGQ_T;

typedef struct RADIUS_AUTHORIZED_ATTRIBUTES_S
{
    UI32_T tunnel_type;
    UI32_T tunnel_medium_type;
    UI8_T   tunnel_private_group_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST+1];
    UI8_T   filter_id[SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE+1];
    UI32_T session_timeout;
}RADIUS_AUTHORIZED_ATTRIBUTES_T;

typedef enum
{
    RADIUSCLIENT_RESPONSE_PACKET_ERROR_NONE,
    RADIUSCLIENT_RESPONSE_PACKET_ERROR_UNKNOWN_TYPE,
    RADIUSCLIENT_RESPONSE_PACKET_ERROR_BAD_AUTHENTICATOR,
    RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_SEQ_NBR,
    RADIUSCLIENT_RESPONSE_PACKET_ERROR_MISMATCH_LENGTH,
} RADIUSCLIENT_ResponsePacketErrorType_T;

/*******************************************************************/

__BEGIN_DECLS

/*  Function prototypes */
/*      radexample.c            */
int radius_main(char user[],char password[],UI32_T src_port,UI8_T *src_mac,UI8_T eap_data[],UI32_T eap_datalen, VALUE_PAIR **received,UI32_T auth_service_type,UI32_T *auth_service,UI8_T *recv_data,UI32_T *recv_datalen,RADIUS_AUTHORIZED_ATTRIBUTES_T *authorized_attributes,R_STATE_DATA *state_message,UI32_T *server_ip);

#if 0
/*      inet_addr.c             */
unsigned int inet_addr(register const char *cp);
#endif

/*  avpair.c        */

VALUE_PAIR *rc_avpair_add __P((VALUE_PAIR **, int, void *, int));
int rc_avpair_assign __P((VALUE_PAIR *, void *, int));
VALUE_PAIR *rc_avpair_new __P((int, void *, int));
VALUE_PAIR *rc_avpair_gen __P((AUTH_HDR *));
VALUE_PAIR *rc_avpair_get __P((VALUE_PAIR *, UI32_T));
void rc_avpair_insert __P((VALUE_PAIR **, VALUE_PAIR *, VALUE_PAIR *));
void rc_avpair_free __P((VALUE_PAIR *));
int rc_avpair_parse __P((char *, VALUE_PAIR **));

/*  buildreq.c      */

void rc_buildreq __P((SEND_DATA *, int, UI32_T, UI32_T, UI32_T, UI32_T));
UI8_T rc_get_seqnbr __P((void));
UI8_T rc_guess_seqnbr(void);

int rc_auth __P((UI32_T, VALUE_PAIR *, VALUE_PAIR **,UI32_T *auth_service,UI8_T *,UI32_T *,UI32_T,RADIUS_AUTHORIZED_ATTRIBUTES_T *,UI32_T *server_ip));
void rc_random_vector (UI8_T *vector);
/*  config.c        */

//int rc_read_config __P((char *));
//char *rc_conf_str __P((char *));
//int rc_conf_int __P((char *));
//SERVER *rc_conf_srv __P((char *));
//int rc_find_server __P((char *, UI32_T *, char *));

//int get_request_timeout(void);
//void set_request_timeout(char *timeout);
//int get_server_port(void);
//void set_server_port(int serverport);
//int get_retansmit_times(void);
//void set_retransmit_times(char *retries);
//char * get_server_ip(void);
//void set_server_ip(char *serverip);

DICT_ATTR *rc_dict_getattr __P((int));


/*  ip_util.c       */

UI32_T rc_get_ipaddr __P((char *));
int rc_good_ipaddr __P((char *));
int rc_own_hostname __P((char *, int));
UI32_T rc_own_ipaddress __P((void));
void get_local_ip(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *local_ip_p);
/*  sendserver.c        */

int rc_send_server __P((SEND_DATA *,UI32_T *,UI8_T *,UI32_T *,UI32_T,RADIUS_Server_Host_T *,RADIUS_AUTHORIZED_ATTRIBUTES_T *));

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_SendRequestPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  Send the request packet.
 * INPUT:    request_index  - Request index
 *           socket_id      - Socket ID
 * OUTPUT:   vector_p       - Vector data in sent packet
 *           secret_key_p   - Secret key of the remote RADIUS server
 *           timeout_p      - Timeout of the remote RADIUS server
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
int SENDSERVER_SendRequestPacket(const UI32_T request_index,
    const int socket_id, UI8_T *vector_p, char *secret_key_p,
    UI32_T *timeout_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - SENDSERVER_ProcessReceivedPacket
 *---------------------------------------------------------------------------
 * PURPOSE:  Process the received packet.
 * INPUT:    request_index  - Request index
 *           packet_p       - Packet
 *           packet_len     - Packet length
 * OUTPUT:   code_p         - Code in the packet
 *           error_p        - Error type
 * RETURN:   OK_RC/ERROR_RC
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
int SENDSERVER_ProcessReceivedPacket(const UI32_T request_index,
    UI8_T *packet_p, int packet_len, UI32_T *code_p,
    RADIUSCLIENT_ResponsePacketErrorType_T *error_p);

/* md5.c            */
//void rc_md5_calc __P((UI8_T *, UI8_T *, unsigned int));
/* hmac.c */
//void hmac_md5(unsigned char *data, int data_len,
//              unsigned char *key,  int key_len,
//        unsigned char  digest[16]);


__END_DECLS

#endif /* RADIUSCLIENT_H */

