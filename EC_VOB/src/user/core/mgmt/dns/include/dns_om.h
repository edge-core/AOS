/* MODULE NAME: dns_om.h
 * PURPOSE:
 *   Initialize the database resource and provide some get/set functions for accessing the
 *   dns database.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-10-23    -- Wiseway , created for convention.
 *
 * Copyright(C)      Accton Corporation, 2002
 */
#ifndef DNS_OM_H
#define DNS_OM_H

#include "dns_type.h"
#include "dns_hostlib.h"
#include "sysfun.h"


#define DNS_OM_EnterCriticalSection()   SYSFUN_OM_ENTER_CRITICAL_SECTION()
#define DNS_OM_LeaveCriticalSection()   SYSFUN_OM_LEAVE_CRITICAL_SECTION()
/*************************************************************************************
 *  terms:
 *  routine name  :the c function name;
 *  purpose       :Who this routine (function) is provided for.In this file,all the
 *                  function  is provided for snmp.
 *  function      :What does the function (routine) do.
 *  returns       :The returned value of this function (routine).
 *                 In this file, usually there are two possible returned values:
 *                 DNS_FAILURE          -1;
 *                 DNS_SUCCESS          0
 *************************************************************************************/
/* MACRO FUNCTION DECLARATIONS
 */

 /* The commands for IPC message.
 */
enum {
    DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_UNPARSES,
    DNS_OM_IPC_CMD_GET_SERVER_OPT_COUNTER_FRIENDS_AUTH_ANS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_AUTH_NO_DATE_RESPS,
    DNS_OM_IPC_CMD_GET_SEROPT_COUNTER_FRIENDS_AUTH_NO_NAMES,
    DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_ERRORS,
    DNS_OM_IPC_CMD_GET_DNS_SERVOPT_COUNTER_FRIENDS_NON_AUTH_DATES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_NON_AUTH_NO_DATAS,
    DNS_OM_IPC_GETSERVOPT_COUNTER_FRIENDS_OTHER_ERRORS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDSREFERRALS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQREFUSALS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_REQUNPARSES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTHANS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_FRIENDS_RELNAMES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_DATA_RESPS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_AUTH_NO_NAMES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_ERRORS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_DATAS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_NON_AUTH_NO_DATAS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REFERRALS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_RELNAMES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_REFUSALS,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_REQ_UNPARSES,
    DNS_OM_IPC_CMD_GET_SERVOPT_COUNTER_SELF_OTHER_ERRORS,
    DNS_OM_IPC_CMD_GET_NAME_SERVER_BY_INDEX,
    DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST,
    DNS_OM_IPC_CMD_GET_DNS_RES_COUNTER_BY_OPCODE_ENTRY,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_BY_RCODE_ENTRY,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_FALLBACKS,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_MARTIANS,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_DATA_RESPS,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_NON_AUTH_NO_DATA_RESPS,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_RECD_RESPONSES,
    DNS_OM_IPC_CMD_GET_RES_COUNTER_UNPARSE_RESPS,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNALS,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_INTERNAL_TIMEOUTS,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_NO_RESPONSES,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_REFERALS,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_RETRANS,
    DNS_OM_IPC_CMD_GET_RES_OPT_COUNTER_ROOT_RETRANS,
    DNS_OM_IPC_CMD_GET_SERV_CONFIG_OMPLEMENT_IDENT,
    DNS_OM_IPC_CMD_GET_SERV_CONFIG_RECURS,
    DNS_OM_IPC_CMD_GET_SERV_CONFIG_RESET_TIME,
    DNS_OM_IPC_CMD_GET_SERV_CONFIG_UP_TIME,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_ANS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_DATA_RESPS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_AUTH_NO_NAMES,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_ENTRY,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_ERRORS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_DATAS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_NON_AUTH_NO_DATAS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_OTHER_ERRORS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_REFERRALS,
    DNS_OM_IPC_CMD_GET_SERV_COUNTER_RELNAMES,
    DNS_OM_IPC_CMD_GET_NEXT_SERVER_LIST,
    DNS_OM_IPC_CMD_GET_SERVER_COUNTER_REQ_REFUSALS,
    DNS_OM_IPC_CMD_GET_DOMAIN_NAME_LIST_ENTRY,
    DNS_OM_IPC_CMD_GET_NEXT_DOMAIN_NAME_LIST_ENTRY,
};


#define DNS_OM_MSG_CMD(msg_p)    (((DNS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define DNS_OM_MSG_RETVAL(msg_p) (((DNS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_OM_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DNS message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of DNS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_OM_GET_MSGBUFSIZE(type_name) \
    ((uintptr_t)&((DNS_OM_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of DNS message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of DNS message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA() \
    sizeof(DNS_OM_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_OM_MSG_CMD
 *              DNS_OM_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the DNS command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The DNS command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_OM_MSG_CMD(msg_p)    (((DNS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define DNS_OM_MSG_RETVAL(msg_p) (((DNS_OM_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - DNS_OM_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define DNS_OM_MSG_DATA(msg_p)   ((void *)&((DNS_OM_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    int    dnsResCounterByRcodeCode;        /* not-accessible.The index to this table.  The Response Codes*/
    int    dnsResCounterByRcodeResponses;   /* read-only. Number of responses the resolver has received for the
                                              response code value which identifies this row of the table.*/
} DNS_OM_IPCMsg_ResCounterByRcodeEntry_T;


typedef struct
{
    int       dnsResCounterByOpcodeCode;
    int       dnsResCounterByOpcodeQueries;
    int       dnsResCounterByOpcodeResponses;
} DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T;

typedef struct {
    UI32_T            dnsResCounterNonAuthDataResps;/*Number of requests made by the resolver for which a
                                                                  non-authoritative answer (cached data) was received.*/
    UI32_T            dnsResCounterNonAuthNoDataResps;/*Number of requests made by the resolver for which a
                                                                    non-authoritative answer - no such data response (empty answer) was received*/
    UI32_T            dnsResCounterMartians;/*Number of responses received which were received from
                                                         servers that the resolver does not think it asked*/
    UI32_T            dnsResCounterRecdResponses;/*Number of responses received to all queries*/
    UI32_T            dnsResCounterUnparseResps;/*Number of responses received which were unparseable*/
    UI32_T            dnsResCounterFallbacks;/*umber of times the resolver had to fall back to its seat belt information*/
    UI32_T            dnsResOptCounterReferals;/*Number of responses which were received from servers redirecting query to another server.*/
    UI32_T            dnsResOptCounterRetrans;/*Number requests retransmitted for all reasons*/
    UI32_T            dnsResOptCounterNoResponses;/*Number of queries that were retransmitted because of no response*/
    UI32_T            dnsResOptCounterRootRetrans;/*Number of queries that were retransmitted that were to root servers.*/
    UI32_T            dnsResOptCounterInternals;/*Number of requests internally generated by the resolver.*/
    UI32_T            dnsResOptCounterInternalTimeOuts;/*Number of requests internally generated which timed out*/
} DNS_OM_IPCMsg_ResCounter_T;

typedef struct
{
    UI8_T	dnsServConfigImplementIdent[MAXSIZE_dnsServConfigImplementIdent+1]; /*The implementation identification string for the DNS
                                                                              server software in use on the system, for example;
                                                                               "DNS-2.1'" */

    int		dnsServConfigRecurs;  /*This represents the recursion services offered by this
                                          name server.  The values that can be read or written are:
                                          available(1) - performs recursion on requests from clients.
                                          restricted(2) - recursion is performed on requests only
                                          from certain clients, for example; clients on an access control list.
                                          It is not supported in our system.
                                          unavailable(3) - recursion is not available.  */
   UI32_T	dnsServConfigUpTime;  /*If the server has a persistent state (e.g., a process),
                                          this value will be the time elapsed since it started.
                                          For software without persistant state, this value will  be zero */
   UI32_T	dnsServConfigResetTime;
   int		dnsServConfigReset;    /*Status/action object to reinitialize any persistant name
                                          server state.  When set to reset(2), any persistant
                                          name server state (such as a process) is reinitialized as
                                          if the name server had just been started.  This value
                                          will never be returned by a read operation.  When read,
                                          one of the following values will be returned:
                                          other(1) - server in some unknown state;
                                          initializing(3) - server (re)initializing;
                                          running(4) - server currently running */
   int		dnsServConfigMaxRequests; /*proxy can receive up to 20 requests simultaneously.range 1:20. default:10*/
   int		dnsServConfigCurrentNumberOfRequests; /*currently received number of requests in proxy*/
   int		dnsServEnabled;      /*0 disabled,1 enabled, 2 stopped; added by wiseway 2002-10-09*/
   int		dnsServStatus;

} DNS_OM_IPCMsg_ProxyConfig_T;

typedef struct {
    UI32_T	dnsResConfigSbeltAddr;  /* not-accessible.The IP address of the Sbelt name server */
    I8_T	dnsResConfigSbeltName[MAXSIZE_dnsResConfigSbeltName+1];  /*read-create.The DNS name */
    int		dnsResConfigSbeltRecursion;   /*read-create .Kind of queries resolver will be sending to the name
                                                  server identified in this row of the table*/
    int		dnsResConfigSbeltPref;        /* read-create .This value identifies the preference for the name server*/
    I8_T	dnsResConfigSbeltSubTree[MAXSIZE_dnsResConfigSbeltSubTree+1]; /* not-accessible */
    int		dnsResConfigSbeltClass;        /*not-accessible.not-accessible.The class of DNS queries that will be sent to the server*/
    int		dnsResConfigSbeltStatus;       /* read-create. Row status column for this row of the Sbelt table.      */
} DNS_OM_IPCMsg_ResConfigSbeltEntry_T;


typedef union
{
    UI32_T cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T result; /* for response */
} DNS_OM_IPCMsg_Type_T;

typedef struct  {
 I8_T 	DnsIpDomainName[DNS_MAX_NAME_LENGTH+1];
} DNS_OM_IPCMsg_IpDomain_T;

typedef  struct
{
    UI32_T     time;
}DNS_OM_IPCMsg_TIME_T;

typedef struct
{
   L_INET_AddrIp_T   	name_server_ip;
   UI32_T		index;
}DNS_OM_IPCMsg_Name_Server_T;

typedef struct  {
  UI32_T            dnsServCounterAuthAns;  /*Number of queries which were authoritatively answered*/
  UI32_T            dnsServCounterAuthNoNames; /*Number of queries for which `authoritative no such name responses were made*/
  UI32_T            dnsServCounterAuthNoDataResps;/* Number of queries for which `authoritative no such data (empty answer) responses were made.*/
  UI32_T            dnsServCounterNonAuthDatas;/*Number of queries which were non-authoritatively answered (cached data)*/
  UI32_T            dnsServCounterNonAuthNoDatas; /*Number of queries which were non-authoritatively answered with no data (empty answer).*/
  UI32_T            dnsServCounterReferrals; /*Number of requests that were referred to other servers.*/
  UI32_T            dnsServCounterErrors;    /*Number of requests the server has processed that were answered with errors (RCODE values other than 0 and 3)*/
  UI32_T            dnsServCounterRelNames;  /*Number of requests received by the server for names that are only 1 label long (text form - no internal dots).*/
  UI32_T            dnsServCounterReqRefusals; /*Number of DNS requests refused by the server*/
  UI32_T            dnsServCounterReqUnparses;/*Number of requests received which were unparseable*/
  UI32_T            dnsServCounterOtherErrors;/*Number of requests which were aborted for other (local) server errors*/
  DNS_ServCounterEntry_T    dnsServCounterTable[DNS_MAX_OP_CODE_NUMBER];   /*Counter information broken down by DNS class and type*/
  UI32_T            dnsServOptCounterSelfAuthAns;/*Number of requests the server has processed which originated from a resolver on the same host for
                                                             whichthere has been an authoritative answer.*/
  UI32_T            dnsServOptCounterSelfAuthNoNames;/*Number of requests the server has processed which
                                                                   originated from a resolver on the same host for which
                                                                   there has been an authoritative no such name answer
                                                                   given.*/
  UI32_T            dnsServOptCounterSelfAuthNoDataResps;/*Number of requests the server has processed which
                                                                      originated from a resolver on the same host for which
                                                                      there has been an authoritative no such data answer
                                                                      (empty answer) made*/
  UI32_T            dnsServOptCounterSelfNonAuthDatas;/*Number of requests the server has processed which
                                                                    originated from a resolver on the same host for which a
                                                                    non-authoritative answer (cached data) was made.*/
  UI32_T            dnsServOptCounterSelfNonAuthNoDatas;/*Number of requests the server has processed which
                                                                      originated from a resolver on the same host for which a
                                                                     `non-authoritative, no such data' response was made
                                                                    (empty answer).*/
  UI32_T            dnsServOptCounterSelfReferrals;/*Number of queries the server has processed which
                                                                originated from a resolver on the same host and were
                                                                referred to other servers*/
  UI32_T            dnsServOptCounterSelfErrors;/*Number of requests the server has processed which
                                                             originated from a resolver on the same host which have
                                                             been answered with errors (RCODEs other than 0 and 3)*/
  UI32_T            dnsServOptCounterSelfRelNames;/*Number of requests received for names that are only 1
                                                               label long (text form - no internal dots) the server has
                                                               processed which originated from a resolver on the same
                                                               host.*/
  UI32_T            dnsServOptCounterSelfReqRefusals;/*Number of DNS requests refused by the server which
                                                                   originated from a resolver on the same host.*/
  UI32_T            dnsServOptCounterSelfReqUnparses; /*Number of requests received which were unparseable and
                                                                    which originated from a resolver on the same host*/
  UI32_T            dnsServOptCounterSelfOtherErrors;/*Number of requests which were aborted for other (local)
                                                                   server errors and which originated on the same host*/
  UI32_T            dnsServOptCounterFriendsAuthAns;/*Number of queries originating from friends which were
                                                                 authoritatively answered.  The definition of friends is
                                                                 a locally defined matte*/
  UI32_T            dnsServOptCounterFriendsAuthNoNames;/*Number of queries originating from friends, for which
                                                                      authoritative `no such name' responses were made.  The
                                                                      definition of friends is a locally defined matter.*/
  UI32_T            dnsServOptCounterFriendsAuthNoDataResps;/*Number of queries originating from friends for which
                                                                          authoritative no such data (empty answer) responses were
                                                                           made.  The definition of friends is a locally defined
                                                                           matter*/
  UI32_T            dnsServOptCounterFriendsNonAuthDatas;/*Number of queries originating from friends which were
                                                                       non-authoritatively answered (cached data). The
                                                                        definition of friends is a locally defined matter*/
  UI32_T            dnsServOptCounterFriendsNonAuthNoDatas;/*Number of queries originating from friends which were
                                                                        non-authoritatively answered with no such data (empty
                                                                        answer).*/
  UI32_T            dnsServOptCounterFriendsReferrals;/*Number of requests which originated from friends that
                                                                    were referred to other servers.  The definition of
                                                                     friends is a locally defined matter.*/
  UI32_T            dnsServOptCounterFriendsErrors;/*Number of requests the server has processed which
                                                                originated from friends and were answered with errors
                                                                (RCODE values other than 0 and 3).  The definition of
                                                                 friends is a locally defined matter.*/
  UI32_T            dnsServOptCounterFriendsRelNames;/*Number of requests received for names from friends that
                                                                   are only 1 label long (text form - no internal dots) the
                                                                   server has processed.*/
  UI32_T            dnsServOptCounterFriendsReqRefusals;/*Number of DNS requests refused by the server which were
                                                                      received from `friends'.*/
  UI32_T            dnsServOptCounterFriendsReqUnparses;/*Number of requests received which were unparseable and
                                                                      which originated from `friends'.*/
  UI32_T            dnsServOptCounterFriendsOtherErrors;/*Number of requests which were aborted for other (local)
                                                                      server errors and which originated from `friends'.*/
} DNS_OM_IPCMsg_ProxyCounter_T;


typedef struct  {
    int   dnsServCounterOpCode;    /*not-accessible The DNS OPCODE being counted in this row of the table.*/
    int   dnsServCounterQClass;    /*not-accessible The class of record being counted in this row of the table*/
    int   dnsServCounterQType;     /*not-accessible The type of record which is being counted in this row in the table*/
    int   dnsServCounterTransport; /*not-accessible
                                              A value of udp(1) indicates that the queries reported on this row were sent using UDP.
                                              A value of tcp(2) indicates that the queries reported on this row were sent using TCP.
                                              A value of other(3) indicates that the queries reported on this row were sent using a transport that was neither TCP nor UDP.
                                           */
    UI32_T dnsServCounterRequests;   /*read-only Number of requests (queries) that have been recorded in this row of the table*/
    UI32_T dnsServCounterResponses;  /*read-only Number of responses made by the server since initialization for the kind of query identified on this row of the table.*/
} DNS_OM_IPCMsg_ServCounterEntry_T;

typedef struct
{
    UI32_T  index;
    char 	name_str[DNS_MAX_NAME_LENGTH+1];
} DNS_OM_IPCMsg_IdxNameStr_T;

typedef union
{
    DNS_OM_IPCMsg_ResCounterByOpcodeEntry_T         rescounterbyopcode;
    DNS_OM_IPCMsg_ResCounterByRcodeEntry_T           rescounterbyrcode;
    DNS_OM_IPCMsg_ResCounter_T                               rescounter;
    DNS_OM_IPCMsg_ProxyConfig_T                              dnsproxy;
    DNS_OM_IPCMsg_TIME_T                                         time;
    DNS_OM_IPCMsg_ProxyCounter_T                            proxycounter;
    DNS_OM_IPCMsg_ServCounterEntry_T                       severcounter;
    DNS_OM_IPCMsg_Name_Server_T                            serverip;
    DNS_OM_IPCMsg_ResConfigSbeltEntry_T                     resconfigsbeltentry;
    DNS_OM_IPCMsg_IdxNameStr_T                  idx_namestr;
} DNS_OM_IPCMsg_Data_T;

typedef struct
{
    DNS_OM_IPCMsg_Type_T type;
    DNS_OM_IPCMsg_Data_T data;
} DNS_OM_IPCMsg_T;


/*the following APIs is used for Cache Mibs  */

/* FUNCTION NAME:  SSHD_OM_Init
 * PURPOSE:
 *          Initiate the semaphore for DNS objects
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_OM_Init.
 */
BOOL_T DNS_OM_Init(void);

/*maggie liu*/
/* FUNCTION NAME:  DNS_OM_ResetConfig
 * PURPOSE:
 *          Set the configure information to default for DNS
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DNS_MGR_ResetConfig().
 */
void DNS_OM_ResetConfig();

#if 0
/* FUNCTION NAME:  DNS_OM_EnterCriticalSection
 * PURPOSE:
 *          Enter critical section before a task invokes the dns objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T DNS_OM_EnterCriticalSection(void);

/* FUNCTION NAME:  DNS_OM_LeaveCriticalSection
 * PURPOSE:
 *          Leave critical section after a task invokes the sshd objects.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          .
 */
BOOL_T DNS_OM_LeaveCriticalSection(void);
#endif


/* FUNCTION NAME : DNS_OM_SetDnsResCacheStatus
 * PURPOSE:
 *		This function is used for initializing dns cache
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *		int * -- 1 enable cache,
 *				 2 enable cache,
 *				 3 clear the contents in the cache
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheStatus (int *dns_res_cache_status_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCacheStatus
 * PURPOSE:
 *		This function is used for getting the cache status.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the returned cache status
 *				 enabled(1),
 *				 disabled(2)
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheStatus(int *dns_res_cache_status_p);


/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxTTL
 * PURPOSE:
 *		This function is used for setting Maximum Time-To-Live for RRs in this cache.
 *		If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing Maximum Time-To-Live for RRs in the cache
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheMaxTTL(int *dns_res_cache_max_ttl_p);




 /* FUNCTION NAME : DNS_OM_GetDnsResCacheMaxTTL
 * PURPOSE:
 *		This fuction will be called by snmp module.
 *		If the resolver does not implement a TTL ceiling, the value of this field should be zero.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable for storing the returned Maximum Time-To-Live for RRs in the cache
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
 int DNS_OM_GetDnsResCacheMaxTTL(UI32_T *dns_res_cache_max_ttl_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCacheGoodCaches
 * PURPOSE:
 *		This function is used for getting the number of rrs the resolver has cached successfully.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the number of rrs the resolver has cache successfully
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheGoodCaches(int *dns_res_good_caches_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCacheBadCaches
 * PURPOSE:
 *		This function is used for getting the number of RRs the resolver has refused to cache.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing the returned number of rrs the resolver has refused to cache
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCacheBadCaches(int *dns_res_cache_bad_caches_p);


/*the following APIs is used for Proxy Mibs                                                   */


/* FUNCTION NAME : DNS_OM_GetDnsServConfigImplementIdent
 * PURPOSE:
 *		This function gets the implementation identification string for the DNS server software in use on the system
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		I8_T * -- a pointer to a string to storing  the implementation
 *				  identification string for the DNS server software
 *				  in use on the system.
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigImplementIdent(I8_T *dns_serv_config_implement_ident_p);

/* FUNCTION NAME : DNS_OM_SetDnsServConfigRecurs
 * PURPOSE:
 *		This function sets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *		int * -- A pointer to a variable to store the value to be set.
 *				This represents the recursion services offered by this
 *				name server.  The values that can be read or written are:
 *				available(1) - performs recursion on requests from clients.
 *				restricted(2) - recursion is performed on requests only
 *				from certain clients, for example; clients on an access
 *				control list.  It is not supported currently.
 *				unavailable(3) - recursion is not available.
 * OUTPUT:
 *
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDnsServConfigRecurs(int *config_recurs_p);


/* FUNCTION NAME : DNS_OM_SetDnsServConfigRecurs
 * PURPOSE:
 *		This function gets the recursion services offered by this name server.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		int * -- A pointer to a variable to store the returned value.
 *				This represents the recursion services offered by this
 *				name server.  The values that can be read or written are:
 *				available(1) - performs recursion on requests from clients.
 *				restricted(2) - recursion is performed on requests only
 *				from certain clients, for example; clients on an access
 *				control list.  It is not supported currently.
 *				unavailable(3) - recursion is not available.
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigRecurs(int *config_recurs_p);



/* FUNCTION NAME : DNS_OM_GetDnsServConfigUpTime
 * PURPOSE:
 *		This function get the up time since the server started.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		    UI32_T* -- a pointer to a variable to store the returned
 *					   value about the up time since the server started
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigUpTime
    (
    UI32_T *dns_serv_config_up_time_p  /*a pointer to a variable to store the returned value about the up time since the server started*/
    );


/* FUNCTION NAME : DNS_OM_GetDnsServConfigResetTime
 * PURPOSE:
 *		This function gets the time elapsed since the last time the name server was `reset.'
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		UI32_T* -- a pointer to a variable to stored the returned
 *				   value about the time elapsed since the last time
 *				   the name server was reset.
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigResetTime(UI32_T *dns_serv_config_reset_time);


/* FUNCTION NAME : DNS_OM_SetDnsServConfigReset
 * PURPOSE:
 *		This function reinitialize any persistant name server state.
 *
 *
 * INPUT:
 *		int * -- When set to reset(2), any persistant name server state
 *				(such as a process) is reinitialized as if the name
 *				server had just been started
 *
 * OUTPUT:
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDnsServConfigReset(int *dns_serv_config_reset_p);


/* FUNCTION NAME : DNS_OM_GetDnsServConfigReset
 * PURPOSE:
 *		This funtion gets any persistant name server state.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *				other(1) - server in some unknown state;
 *				initializing(3) - server (re)initializing;
 *				running(4) - server currently running
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetDnsServConfigReset(int *dns_serv_config_reset_p);

 /*
 * FUNCTION NAME : DNS_OM_GetDnsServConfigResetTime
 *
 * PURPOSE:
 *		This function gets the time elapsed since the last time the name server was `reset.'
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterAuthAns(int *dns_serv_counter_auth_ans_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterAuthNoNames
 *
 * PURPOSE:
 *		This function gets the Number of queries for which `authoritative no such name'
 *      responses were made.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterAuthNoNames(UI32_T *dns_serv_counter_auth_no_names_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterAuthNoDataResps
 *
 * PURPOSE:
 *		This function gets the Number of queries for which `authoritative no such data'
 *       (empty answer) responses were made
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterAuthNoDataResps(int *dns_serv_counter_auth_no_data_resps_p);


 /*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterNonAuthDatas
 *
 * PURPOSE:
 *		This function gets the Number of queries which were non-authoritatively
 *      answered (cached data)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterNonAuthDatas(int *dns_serv_counter_non_auth_datas_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterNonAuthNoDatas
 *
 * PURPOSE:
 *		This funciton gets the number of Number of queries which were non-authoritatively
         answered with no data (empty answer)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterNonAuthNoDatas(UI32_T *dns_serv_counter_non_auth_no_datas_p);

 /*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReferrals
 *
 * PURPOSE:
 *		This function gets the Number of requests that were referred to other servers
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterReferrals(int *dns_serv_counter_referrals_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterErrors
 *
 * PURPOSE:
 *		This function gets the Number of requests the server has processed that were
 *      answered with errors (RCODE values other than 0 and 3).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterErrors(int *dns_serv_counter_errors_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterRelNames
 *
 * PURPOSE:
 *		This function gets the Number of requests received by the server for names that
 *       are only 1 label long (text form - no internal dots)
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterRelNames(int *dns_serv_counter_rel_names_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReqRefusals
 *
 * PURPOSE:
 *		This function gets the Number of DNS requests refused by the server.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterReqRefusals(int *dns_serv_counter_req_refusals_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterReqUnparses
 *
 * PURPOSE:
 *		This function gets the Number of requests received which were unparseable
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterReqUnparses(int *dns_serv_counter_req_unparses_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServCounterOtherErrors
 *
 * PURPOSE:
 *		This function gets the Number of requests which were aborted for other (local)
 *      server errors
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		 int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServCounterOtherErrors(UI32_T *dns_serv_counter_other_errors_p);





/* FUNCTION NAME : DNS_OM_GetDnsServCounterEntry
 * PURPOSE:
 *		This function gets the dnsServCounterEntry according the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ServCounterEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *		DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);

/*
 * FUNCTION NAME : DNS_OM_GetNextDnsServCounterEntry
 *
 * PURPOSE:
 *		This function gets the dnsServCounterEntry next to the specified index.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetNextDnsServCounterEntry(DNS_ServCounterEntry_T *dns_serv_counter_entry_t_p);


/* FUNCTION NAME : DNS_OM_GetNextDnsServCounterEntry
 * PURPOSE:
 *		This function gets the dnsServCounterEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ServCounterEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *		DNS_ServCounterEntry_T * -- a pointer to a DNS_ServCounterEntry_T variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterSelfAuthAns(int *dns_serv_opt_counter_self_auth_ans_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfAuthNoNames
 *
 * PURPOSE:
 *		This function gets the number of requests the server has processed which
        originated from a resolver on the same host for which
        there has been an authoritative no such name answer given.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfAuthNoNames(int *dns_serv_opt_counter_self_auth_no_names_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps
 *
 * PURPOSE:
 *		This function gets the number of requests the server has processed which
 *       originated from a resolver on the same host for which
 *       there has been an authoritative no such data answer (empty answer) made.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfAuthNoDataResps(int *dns_serv_opt_counter_self_auth_no_data_resps);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfNonAuthDatas
 *
 * PURPOSE:
 *		 Number of requests the server has processed which
          originated from a resolver on the same host for which a
          non-authoritative answer (cached data) was made
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfNonAuthDatas(int *dns_serv_opt_counter_self_non_auth_datas_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas
 *
 * PURPOSE:
 *		Number of requests the server has processed which
 *      originated from a resolver on the same host for which a
 *      non-authoritative, no such data' response was made
 *      (empty answer).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfNonAuthNoDatas(int *dns_serv_opt_counter_self_non_auth_no_datas_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReferrals
 *
 * PURPOSE:
 *		Number of queries the server has processed which
 *       originated from a resolver on the same host and were
 *       referred to other servers
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfReferrals(int *dns_serv_opt_counter_self_referrals);



/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfErrors
 *
 * PURPOSE:
 *		Number of requests the server has processed which
 *       originated from a resolver on the same host which have
 *      been answered with errors (RCODEs other than 0 and 3).
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfErrors(int *dns_serv_opt_counter_self_errors_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfRelNames
 *
 * PURPOSE:
 *		Number of requests received for names that are only 1
 *      label long (text form - no internal dots) the server has
 *       processed which originated from a resolver on the same
 *       host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfRelNames(int *dns_serv_opt_counter_self_rel_names_p);



/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReqRefusals
 *
 * PURPOSE:
 *		Number of DNS requests refused by the server which
 *      originated from a resolver on the same host
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfReqRefusals(int *dns_serv_opt_counter_self_req_refusals_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfReqUnparses
 *
 * PURPOSE:
 *		Number of requests received which were unparseable and
 *      which originated from a resolver on the same host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfReqUnparses(int *dns_serv_opt_counter_self_req_unparses_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterSelfOtherErrors
 *
 * PURPOSE:
 *		Number of requests which were aborted for other (local)
 *      server errors and which originated on the same host.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterSelfOtherErrors(int *dns_serv_opt_counter_self_other_errors_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthAns
 *
 * PURPOSE:
 *		Number of queries originating from friends which were
 *      authoritatively answered.  The definition of friends is
 *      a locally defined matter
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --   a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthAns(int *dns_serv_opt_counter_friends_auth_ans_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthNoNames
 *
 * PURPOSE:
 *		Number of queries originating from friends, for which
 *      authoritative `no such name' responses were made.  The
 *      definition of friends is a locally defined matter.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthNoNames(int *dns_serv_opt_counter_friends_auth_no_names_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps
 *
 * PURPOSE:
 *		Number of queries originating from friends for which
 *      authoritative no such data (empty answer) responses were
 *      made.  The definition of friends is a locally defined
 *      matter
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		UI32_T * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsServOptCounterFriendsAuthNoDataResps(UI32_T *dns_serv_opt_counter_friends_auth_no_data_resps_p);


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * PURPOSE:
 *		Number of queries originating from friends which were
 *		non-authoritatively answered with no such data (empty
 *		answer)
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsNonAuthDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_datas_p);


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas
 * PURPOSE:
 *		Number of queries originating from friends which were
 *		non-authoritatively answered with no such data (empty
 *		answer)
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsNonAuthNoDatas(UI32_T *dns_serv_opt_counter_friends_non_auth_no_datas_p);


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReferrals
 * PURPOSE:
 *		Number of requests which originated from friends that
 *		were referred to other servers.  The definition of
 *		friends is a locally defined matter.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReferrals(UI32_T *dns_serv_opt_counter_friends_referrals_p);


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsErrors
 * PURPOSE:
 *		Number of requests the server has processed which
 *		originated from friends and were answered with errors
 *		(RCODE values other than 0 and 3).  The definition of
 *		friends is a locally defined matter.
 * INPUT:
 *
 *
 * OUTPUT:
 *		UI32_T* -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsErrors(UI32_T *dns_serv_opt_counter_friends_errors_p);

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsRelNames
 * PURPOSE:
 *		Number of requests received for names from friends that
 *		are only 1 label long (text form - no internal dots) the
 *		server has processed
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable to store the result.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsRelNames(UI32_T *dns_serv_opt_counter_friends_rel_names_p);

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReqRefusals
 * PURPOSE:
 *		This function gets the number of DNS requests refused by the server which were
 *		received from `friends'.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReqRefusals(UI32_T *dns_serv_opt_counter_friends_req_refusals_p);

/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsReqUnparses
 * PURPOSE:
 *		Number of requests received which were unparseable and
 *		which originated from `friends'
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsReqUnparses(UI32_T *dns_serv_opt_counter_friends_req_unparses_p);


/* FUNCTION NAME : DNS_OM_GetDnsServOptCounterFriendsOtherErrors
 * PURPOSE:
 *		Number of requests which were aborted for other (local)
 *		server errors and which originated from `friends'.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsServOptCounterFriendsOtherErrors(UI32_T *dns_serv_opt_counter_friends_other_errors_p);

/*the following APIs is used for Resolver Mibs                                                   */

/* FUNCTION NAME : DNS_OM_GetDnsResConfigImplementIdent
 * PURPOSE:
 *		The implementation identification string for the
 *		resolver software in use on the system.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		I8_T * -- a pointer to the variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigImplementIdent(I8_T *dns_res_config_implement_ident_p);

/* FUNCTION NAME : DNS_OM_GetDnsResConfigService
 * PURPOSE:
 *		Get kind of DNS resolution service provided	.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *				recursiveOnly(1) indicates a stub resolver.
 *				iterativeOnly(2) indicates a normal full service resolver.
 *				recursiveAndIterative(3) indicates a full-service
 *				resolver which performs a mix of recursive and iterative queries.
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigService(int *dns_res_config_service_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigMaxCnames
 * PURPOSE:
 *		Limit on how many CNAMEs the resolver should allow
 *		before deciding that there's a CNAME loop.  Zero means
 *		that resolver has no explicit CNAME limit.
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigMaxCnames(int *dns_resconfig_max_cnames_p);

/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T * --  a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *
 * RETURN:
 *
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);

/* FUNCTION NAME : DNS_OM_GetNextDnsResConfigSbeltEntry
 * PURPOSE:
 *		This function gets the DNS_ResConfigSbeltEntry_T  next to the specified index.
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetNextDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigUpTime
 * PURPOSE:
 *		If the resolver has a persistent state (e.g., a
 *		process), this value will be the time elapsed since it
 *		started.  For software without persistant state, this
 *		value will be 0.
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing config up time
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigUpTime(int *config_up_time_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigResetTime
 * PURPOSE:
 *		This function gets the time elapsed since it started.
 *
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to storing the returned config reset time
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigResetTime(int *config_reset_time_p);


/* FUNCTION NAME : DNS_OM_SetDnsResConfigReset
 * PURPOSE:
 *		This function reinitialize any persistant resolver state.
 *
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable stored with the reset value,2 means reset
 *
 * OUTPUT:
 *
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResConfigReset(int *config_reset_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigReset
 * PURPOSE:
 *		This function gets any persistant resolver state.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned resolver state
 *				other(1) - resolver in some unknown state;
 *				initializing(3) - resolver (re)initializing;
 *				running(4) - resolver currently running.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigReset(int *config_reset_p);

/* FUNCTION NAME : DNS_OM_SetDnsResConfigQueryOrder
 * PURPOSE:
 *		This function  set the query order.
 *
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable storing the value to be set
 *				DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *				DNS_QUERY_DNS_FIRST: Query DNS server first
 *				default: DNS_QUERY_LOCAL_FIRST
 *				0 means uses default configuration
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResConfigQueryOrder(int *dns_res_config_query_order_p);


/* FUNCTION NAME : DNS_OM_GetDnsResConfigQueryOrder
 * PURPOSE:
 *		This function  get the query order.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the returned value about query order
 *				DNS_QUERY_LOCAL_FIRST:Query static host table first;
 *				DNS_QUERY_DNS_FIRST: Query DNS server first
 *				DNS_QUERY_ DNS_ONLY: Query DNS server only
 *				default: DNS_QUERY_LOCAL_FIRST
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResConfigQueryOrder(int *dns_res_config_query_order_p);


/* FUNCTION NAME : DNS_OM_GetDnsResCounterByOpcodeEntry
 * PURPOSE:
 *		This function gets dnsResCounterByOpcodeEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- Given as the index of the entry.
 *
 * OUTPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable t store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);

/* FUNCTION NAME : DNS_OM_GetNextDnsResCounterByOpcodeEntry
 * PURPOSE:
 *		This function gets dnsResCounterByOpcodeEntry next to the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- Given to provide the index of the entry.
 *
 * OUTPUT:
 *		DNS_ResCounterByOpcodeEntry_T * -- a pointer to a variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK   :succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetNextDnsResCounterByOpcodeEntry(DNS_ResCounterByOpcodeEntry_T *dns_res_counter_by_opcode_entry_p);


/* FUNCTION NAME : DNS_OM_GetDnsResCounterByRcodeEntry
 * PURPOSE:
 *		This function gets the number of responses for the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- Given as a index for the entry.
 *										  INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *		DNS_ResCounterByRcodeEntry_T * --a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);

/* FUNCTION NAME : DNS_OM_GetNextDnsResCounterByRcodeEntry
 * PURPOSE:
 *		This function gets the number of responses for the index next to the specified index.
 *
 *
 *
 * INPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- Given as the index of the entry.
 *										 INDEX { dnsResCounterByRcodeCode }
 * OUTPUT:
 *		DNS_ResCounterByRcodeEntry_T * -- a pointer to a DnsResCounterByRcodeEntry variable to store the returned entry
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetNextDnsResCounterByRcodeEntry(DNS_ResCounterByRcodeEntry_T *dns_res_counter_by_rcode_entry_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterReferals
 *
 * PURPOSE:
 *		This function gets the number of responses which were
 *		eived from servers redirecting query to another server.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterReferals(int *dns_res_opt_counter_referals_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterRetrans
 *
 * PURPOSE:
 *		This function gets the number requests retransmitted for all reasons
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterRetrans(int *dns_res_opt_counter_retrans_p);

 /*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterNoResponses
 *
 * PURPOSE:
 *		This function gets the number of queries that were retransmitted because of no response
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterNoResponses(UI32_T *dns_res_opt_counter_no_responses_p);


/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterRootRetrans
 *
 * PURPOSE:
 *		This function gets the number of queries that were retransmitted that were to root servers.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterRootRetrans(int *dns_res_opt_counter_root_retrans_p);

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterInternals
 *
 * PURPOSE:
 *		This function gets the number of requests internally generated by the resolver.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * --  a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterInternals(int *dns_res_opt_counter_internals);

/*
 * FUNCTION NAME : DNS_OM_GetDnsResOptCounterInternalTimeOuts
 *
 * PURPOSE:
 *		This function gets the number of requests internally generated which timed out.
 *
 * INPUT:
 *		none
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_OK/DNS_ERROR
 *
 * NOTES:
 *		none
 */
int DNS_OM_GetDnsResOptCounterInternalTimeOuts(int *dns_res_opt_counter_internal_time_outs_p);


/* FUNCTION NAME : DNS_OM_EnableDomainLookup
 * PURPOSE:
 *		This fcuntion enable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *
 */
void DNS_OM_EnableDomainLookup(void);

/* FUNCTION NAME : DNS_OM_DisableDomainLookup
 * PURPOSE:
 *		This fcuntion disable DNS including resolver and proxy.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *
 */
void DNS_OM_DisableDomainLookup(void);

int DNS_OM_AddDomainName
     (
      char *domain_name_p   /*a domain name to be added */
     );


/* FUNCTION NAME : DNS_OM_DeleteDomainName
 * PURPOSE:
 *		Delete the default domain name.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_OK:success;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_DeleteDomainName(void);


/* FUNCTION NAME : DNS_OM_AddDomainNameToList
 * PURPOSE:
 *		If there is no domain list, the domain name that you specified with the ip
 *		domain-name global configuration command is used. If there is a domain list,
 *		the default domain name is not used. The ip domain-list command is similar
 *		to the ip domain-name command, except that with the ip domain-list command
 *      you can define a list of domains, each to be tried in turn.
 * INPUT:
 *		I8_T * -- a domain name to be added to the domain name lsit.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_AddDomainNameToList
     (
     	char *domain_name_p   /*a domain name to be added to the domain name lsit */
     );

/* FUNCTION NAME : DNS_OM_DeleteDomainNameToList
 * PURPOSE:
 *		If there is no domain list, the domain name that you specified with the ip
 *		domain-name global configuration command is used. If there is a domain list,
 *		the default domain name is not used. The ip domain-list command is similar
 *		to the ip domain-name command, except that with the ip domain-list command
 *		you can define a list of domains, each to be tried in turn.
 * INPUT:
 *		I8_T * -- a domain name to be deleted to the domain name lsit
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_DeleteDomainNameFromList
     (
     	char *domain_name_p   /*a domain name to be deleted to the domain name lsit */
     );


/* FUNCTION NAME : DNS_OM_AddNameServer
 * PURPOSE:
 *		This function adds a name server IP address to the name server list.
 *
 *
 * INPUT:
 *		I8_T * -- a pointer to a ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_AddNameServer (L_INET_AddrIp_T *addr_p);


/* FUNCTION NAME : DNS_OM_DeleteNameServer
 * PURPOSE:
 *		This function deletes a name server entry from
 *		the name server list accordint to the IP address of the name server.
 *
 * INPUT:
 *		I8_T * -- a pointer to a ip addr , whose related name server will be deleted;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_DeleteNameServer
    (
    L_INET_AddrIp_T *addr_p
    );


/* FUNCTION NAME : DNS_OM_ClearDnsCache
 * PURPOSE:
 *     This function delete the contents of the dns cache.The default
 *     configuration will be used.
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *		DNS_OK :success
 * NOTES:
 *      This function will be called by configuration sub moudle.
 */
void DNS_OM_ClearDnsCache(void);


/* FUNCTION NAME : DNS_OM_ShowDnsConfig
 * PURPOSE:
 *		This function shows the dns configuration.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by CLI module.
 */
void DNS_OM_ShowDnsConfig(void);

/* FUNCTION NAME : DNS_OM_ShowDnsDatabase
 * PURPOSE:
 *		This funciton is used for showing  cache database and status.
 *		Every field except link should be displayed, and the index of this cache
 *		entry should also be displayed.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
void DNS_OM_ShowDnsDatabase(void);

/* FUNCTION NAME : DNS_OM_ShowDnsCache
 * PURPOSE:
 *		This funciton is used for showing  cache database and status.
 *		Every field except link should be displayed, and the index of this cache
 *		entry should also be displayed.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
void  DNS_OM_ShowDnsCache(void);


/* FUNCTION NAME : DNS_OM_DebugOpen
 * PURPOSE:
 *		This fcuntion enables the displaying of debugging information.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *
 */
void  DNS_OM_DebugOpen(void);

/* FUNCTION NAME : DNS_OM_DebugClose
 * PURPOSE:
 *		This fcuntion disables the displaying of debugging information.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *
 */
void  DNS_OM_DebugClose(void);


/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxEntries
 * PURPOSE:
 *		This fcuntion set the max number of entries in cache.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *				max entries in cache. range:1280-6400. default:2560
 *
 * OUTPUT:
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_SetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p);


/* FUNCTION NAME : DNS_OM_SetDnsResCacheMaxEntries
 * PURPOSE:
 *		This fcuntion get the max number of entries in cache.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned value
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int  DNS_OM_GetDnsResCacheMaxEntries(int *dns_res_cache_max_entries_p);

/* FUNCTION NAME : DNS_OM_SetDnsServConfigMaxRequests
 * PURPOSE:
 *		This fcuntion set the max number of requests in proxy.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *				max requests in proxy. range:1-20. default:10
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDnsServConfigMaxRequests(int *dns_serv_config_max_requests_p);


/* FUNCTION NAME :DNS_OM_GetDnsServConfigMaxRequests
 * PURPOSE:
 *		This fcuntion gets the max number of requests in proxy.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned value
 *
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *
 */
int  DNS_OM_GetDnsServConfigMaxRequests(int *dns_res_cache_max_entries_p);


 /* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This funciton set the max number of local requests that resolver
 *		can deal with.
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable for storing the number of
 *				local max requests. range:1..10. default:5
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsLocalMaxRequests(int *dns_config_local_max_requests_p);


/* FUNCTION NAME : DNS_OM_GetDnsLocalMaxRequests
 * PURPOSE:
 *		This funciton set the max number of local requests that resolver
 *		can deal with.
 *
 *
 * INPUT:
 *		none.
 * OUTPUT:
 *		int * -- a pointer to a variable for storing the returned number of
 *				local max requests. range:1..10. default:5
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int  DNS_OM_GetDnsLocalMaxRequests(int *dns_config_local_max_requests_p);


/* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This fcuntion sets time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *		UI32_T* --  a pointer to a variable for storing timeout value
 *					for requests. range:1..20 . default:5 seconds
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int  DNS_OM_SetDnsTimeOut(UI32_T *dns_time_out_p);


/* FUNCTION NAME : DNS_OM_SetDnsLocalMaxRequests
 * PURPOSE:
 *		This fcuntion gets the time out value (seconds) for requests.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T * -- a pointer to a variable for storing the returned value
 *					range 1:20; default: 5 seconds
 * RETURN:
 *		DNS_ERROR:failure;
 *		DNS_OK:succsess;
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_GetDnsTimeOut(UI32_T *dns_time_out_p);


/* FUNCTION NAME : DNS_OM_ShowNameServerList
 * PURPOSE:
 *		This fcuntion displays the nam server informaion.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *
 */
void DNS_OM_ShowNameServerList(void);


/* FUNCTION NAME : DNS_OM_ShowDomainNameList
 * PURPOSE:
 *		This function shows the dns domain name list.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		 This function will be called by CLI module.
 */
BOOL_T DNS_OM_ShowDomainNameList(void);


/* FUNCTION NAME : DNS_OM_GetDnsResCounterFallbacks
 * PURPOSE:
 *		This function gets the number of times the resolver had to fall back to its seat belt information.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterFallbacks(int *dns_res_counter_fallbacks_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCounterUnparseResps
 * PURPOSE:
 *		This function gets Number of responses received which were unparseable.
 *
 *
 *
 * INPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterUnparseResps
    (
    UI32_T *dns_res_counter_unparse_resps_p /*a pointer to a variable to store the result*/
    );

/* FUNCTION NAME : DNS_OM_GetDnsResCounterRecdResponses
 * PURPOSE:
 *		This function gets Number of responses received to all queries.
 *
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterRecdResponses(UI32_T *dns_res_counter_recd_responses_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCounterMartians
 * PURPOSE:
 *		This function gets the number of responses received which were received from
 *		servers that the resolver does not think it asked.
 *
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterMartians(UI32_T *dns_res_counter_martians_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCounterNonAuthNoDataResps
 * PURPOSE:
 *		This fucniton gets Number of requests made by the resolver for which a
 *		non-authoritative answer - no such data response (empty answer) was received
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterNonAuthNoDataResps(int *dns_res_counter_non_auth_no_data_resps_p);

/* FUNCTION NAME : DNS_OM_GetDnsResCounterNonAuthDataResps
 * PURPOSE:
 *		This functin gets the number of requests made by the resolver for which a
 *		non-authoritative answer (cached data) was received.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *		int * -- a pointer to a variable to store the result
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDnsResCounterNonAuthDataResps(int *dns_res_counter_non_auth_data_resps_p);

/* FUNCTION NAME : DNS_OM_ServCounterInc
 *
 * PURPOSE:
 *		Server's counter add 1(index with leaf)
 *
 * INPUT:
 *		int.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ServCounterInc(int leaf);

/* FUNCTION NAME : DNS_OM_ServOptCounterInc
 *
 * PURPOSE:
 * 		Server counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- .
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ServOptCounterInc(int leaf);


/* FUNCTION NAME : DNS_OM_ResOptCounterInc
 *
 * PURPOSE:
 * 		Resolver's opt counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- leaf.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ResOptCounterInc(int leaf);

/* FUNCTION NAME : DNS_OM_ResCounterInc
 *
 * PURPOSE:
 *		Resolver counter add 1 (index by leaf)
 *
 * INPUT:
 *		int -- leaf.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ResCounterInc(int leaf);

/* FUNCTION NAME : DNS_OM_ResCounterByOpcodeInc
 *
 * PURPOSE:
 *		resolver counter add 1 index by opcode
 *
 * INPUT:
 *		int -- identify whether a request or response
 *		int -- index
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ResCounterByOpcodeInc(int qr,int opcode);

/* FUNCTION NAME : DNS_OM_ResCounterByRcodeInc
 *
 * PURPOSE:
 *		resolver counter add 1 (index by rcode)
 *
 * INPUT:
 *		int -- index
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ResCounterByRcodeInc(int rcode);

/* FUNCTION NAME : DNS_OM_GetDnsDebugStatus
 *
 * PURPOSE:
 * 		Get whether server debug status is open.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_GetDnsDebugStatus(int *status);

/* FUNCTION NAME : DNS_OM_ShowCounter
 *
 * PURPOSE:
 *		show dns counter.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ShowCounter(void);

/* FUNCTION NAME : DNS_OM_GetDnsStatus
 *
 * PURPOSE:
 * 		Get dns status
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void  DNS_OM_GetDnsStatus(int *status);

/* FUNCTION NAME : DNS_OM_SetDnsStatus
 *
 * PURPOSE:
 * 		set dns status
 *
 * INPUT:
 *		int -- status.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_SetDnsStatus(int status);

/* FUNCTION NAME : DNS_OM_GetDnsIpDomain
 *
 * PURPOSE:
 *		Get dns ip domain name.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      char* -- ipdomain name .
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
int DNS_OM_GetDnsIpDomain(char* ipdomain);

/* FUNCTION NAME : DNS_OM_DnsUptimeInit
 *
 * PURPOSE:
 *		Initiate dns uptime
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_DnsUptimeInit(void);

/* FUNCTION NAME : DNS_OM_DnsResetTimeInit
 *
 * PURPOSE:
 *		initiate dns reset time
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_DnsResetTimeInit(void);

/* FUNCTION NAME : DNS_OM_GetDnsSbelt
 *
 * PURPOSE:
 *		Get dns sbelt.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      DNS_ResConfigSbelt_T* -- dns sbelt
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
DNS_ResConfigSbelt_T* DNS_OM_GetDnsSbelt(void);

/* FUNCTION NAME : DNS_OM_ResCounterInit
 *
 * PURPOSE:
 * 		initiate dns resolver counter
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ResCounterInit(void);

/* FUNCTION NAME : DNS_OM_ServCounterInit
 *
 * PURPOSE:
 *		initiate dns server counter.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ServCounterInit(void);

/* FUNCTION NAME : DNS_OM_GetServStatus
 *
 * PURPOSE:
 *		get dns server status
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		int -- dns server status
 *
 * NOTES:
 *		none.
 */
int DNS_OM_GetServStatus(void);

/* FUNCTION NAME : DNS_OM_SetServStatus
 *
 * PURPOSE:
 *		ser dns server status
 *
 * INPUT:
 *		int -- dns server status
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_SetServStatus(int status);

/* FUNCTION NAME : DNS_OM_GetServCurrentRequestNumber
 *
 * PURPOSE:
 *		get server current request number.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		int -- current server request number
 *
 * NOTES:
 *		none.
 */
int DNS_OM_GetServCurrentRequestNumber(void);

/* FUNCTION NAME : DNS_OM_ServCurrentRequestNumberInc
 *
 * PURPOSE:
 *		current server request number add 1
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ServCurrentRequestNumberInc(void);

/* FUNCTION NAME : DNS_OM_ServCurrentRequestNumberDec
 *
 * PURPOSE:
 *		Dec current request number
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_ServCurrentRequestNumberDec(void);

/* FUNCTION NAME : DNS_OM_SetServResetStatus
 *
 * PURPOSE:
 *		Ser server reset status.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_SetServResetStatus(int status);

/* FUNCTION NAME :
 * PURPOSE:
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_SetServServiceEnable(int enable);

/* FUNCTION NAME : DNS_OM_GetServServiceEnable
 *
 * PURPOSE:
 *		Get whether server is enable
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
int DNS_OM_GetServServiceEnable(void);

/* FUNCTION NAME : DNS_OM_ServUpTimeInit
 *
 * PURPOSE:
 *		initiate server uptime
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      none.
 */
void DNS_OM_ServUpTimeInit(void);

/* FUNCTION NAME : DNS_OM_ServResetTimeInit
*
 * PURPOSE:
 *		initiate server reset time
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none
 */
void DNS_OM_ServResetTimeInit(void);



/* FUNCTION NAME : DNS_OM_GetNextDomainNameList
 * PURPOSE:
 *		This function get next the dns domain name from list.
 *
 *
 *
 * INPUT:
 *		I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *		I8_T *dns_ip_domain_name    --  next dommain name of list.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		 the initial name is empty string.
 */
BOOL_T DNS_OM_GetNextDomainNameList(char *dns_ip_domain_name);



/* FUNCTION NAME : DNS_OM_GetDomainNameList
 * PURPOSE:
 *		This function get the dns domain name from list.
 *
 *
 *
 * INPUT:
 *		I8_T *dns_ip_domain_name    --  current dommain name of list.
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *
 */
BOOL_T DNS_OM_GetDomainNameList(I8_T *dns_ip_domain_name);



/* FUNCTION NAME : DNS_OM_GetNextNameServerList
 * PURPOSE:
 *		This function get next the domain name server from list.
 *
 *
 * INPUT:
 *		UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      UI32_T *ip  --  next doamin name server ip.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *		The initial ip value is zero.
 */
BOOL_T DNS_OM_GetNextNameServerList(L_INET_AddrIp_T *ip_p);



/* FUNCTION NAME : DNS_OM_GetNameServerList
 * PURPOSE:
 *		This function get next the domain name server from list.
 *
 *
 * INPUT:
 *		UI32_T *ip  --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *
 */
BOOL_T DNS_OM_GetNameServerList(L_INET_AddrIp_T *ip_p);



/* FUNCTION NAME : DNS_OM_SetDnsResConfigMaxCnames
 * PURPOSE:
 *		Limit on how many CNAMEs the resolver should allow
 *		before deciding that there's a CNAME loop.  Zero means
 *		that resolver has no explicit CNAME limit.
 * INPUT:
 *		UI32_T  -- dns_resconfig_max_cnames
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigMaxCnames(UI32_T dns_resconfig_max_cnames_p);



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltName
 * PURPOSE:
 *		This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *		DNS_OK :success
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltName(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltRecursion
 * PURPOSE:
 *		This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *		DNS_OK :success
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltRecursion(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltPref
 * PURPOSE:
 *		This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *		DNS_OK :success
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltPref(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_OM_SetDnsResConfigSbeltStatus
 * PURPOSE:
 *		This funciton set the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T * -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the value to be set
 *									 INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		none.
 *
 * RETURN:
 *      DNS_ERROR : failure,
 *		DNS_OK :success
 *
 * NOTES:
 *		 This function will be called by snmp module.
 */
int DNS_OM_SetDnsResConfigSbeltStatus(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_OM_GetDefaultDnsResConfigSbeltEntry
 * PURPOSE:
 *		This funciton get the specified DnsResConfigSbeltEntry according to the index.
 *
 *
 * INPUT:
 *		DNS_ResConfigSbeltEntry_T* -- INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass
 *
 * OUTPUT:
 *		DNS_ResConfigSbeltEntry_T* -- a pointer to a DNS_ResConfigSbeltEntry_T variable to store the returned value
 *
 * RETURN:
 *		DNS_ERROR : failure,
 *		DNS_OK    : success.
 * NOTES:
 *		 This function will be called by DNS_CACHE_Init and snmp module.
 */
int DNS_OM_GetDefaultDnsResConfigSbeltEntry(DNS_ResConfigSbeltEntry_T *dns_res_config_sbelt_entry_t_p);



/* FUNCTION NAME : DNS_OM_SetNameServerByIndex
 * PURPOSE:
 *  This function adds a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *      UI32_T  ip_addr --  ip addr will be added as a name server;
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_OM_SetNameServerByIndex(UI32_T index, L_INET_AddrIp_T *ip_addr);



/* FUNCTION NAME : DNS_OM_GetNameServerByIndex
 * PURPOSE:
 *		This function get the domain name server by index from list.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *		UI32_T  *ip     --  current doamin name server ip.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module.
 */
BOOL_T DNS_OM_GetNameServerByIndex(UI32_T index,  L_INET_AddrIp_T *ip);

 /*maggie liu, ES4827G-FLF-ZZ-00244*/
/* FUNCTION NAME : DNS_OM_DeleteNameServerAll
 * PURPOSE:
 *  This function delete all name server IP address from the name server list.
 *
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by CLI and CGI.
 */
int DNS_OM_DeleteNameServerAll();


/*maggie liu for es4827g-FLF-00244*/
/* FUNCTION NAME : DNS_OM_DeleteNameServerByIndex
 * PURPOSE:
 *  This function delete a name server IP address to the name server list by index.
 *
 *
 * INPUT:
 *      UI32_T  index   --  index of name server.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_OM_DeleteNameServerByIndex(UI32_T index);



/* FUNCTION NAME : DNS_OM_SetResResetStatus
 *
 * PURPOSE:
 *		Set resolver reset status.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *		none.
 *
 * NOTES:
 *		none.
 */
void DNS_OM_SetResResetStatus(int status);



/* FUNCTION	NAME : DNS_OM_SetResolverTaskId
 * PURPOSE:
 *		Set resolver task id.
 *
 * INPUT:
 *		UI32_T  tid --  Resolver task id.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *          TRUE
 *
 * NOTES:
 *		None.
 */
BOOL_T DNS_OM_SetResolverTaskId(UI32_T tid);



/* FUNCTION	NAME : DNS_OM_GetResolverTaskId
 * PURPOSE:
 *		Get resolver task id.
 *
 * INPUT:
 *		none.
 *
 * OUTPUT:
 *		UI32_T *tid --  Resolver task id.
 *
 * RETURN:
 *          TRUE
 *
 * NOTES:
 *		None.
 */
BOOL_T DNS_OM_GetResolverTaskId(UI32_T *tid);

/* FUNCTION NAME : DNS_OM_GetNextLookupCtlTable
 * PURPOSE:
 *  This function get next Nslookup control table .
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);

/* FUNCTION NAME : DNS_OM_GetNextLookupCtlTableAndIndex
 * PURPOSE:
 *  This function get next Nslookup control table .
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *     index_p      --  pointer to get 0-based OM index
 *
 * OUTPUT:
 *     *index_p     -- 0-based OM index
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupCtlTableAndIndex(DNS_Nslookup_CTRL_T *CTRL_table, UI32_T *index_p);

/* FUNCTION NAME : DNS_OM_FindLookupCtlTableWithIndex
 * PURPOSE:
 *  This function get next Nslookup control table.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *     index_p      --  pointer to get 0-based OM index
 *
 * OUTPUT:
 *     *index_p     -- 0-based OM index
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupCtlTableAndIndex(DNS_Nslookup_CTRL_T *CTRL_table, UI32_T *index_p);


/* FUNCTION NAME : DNS_OM_GetLookupCtlTable
 * PURPOSE:
 *  This function get next Nslookup control table.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupCtlTable(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_OM_SetDNSCtlTable_TargetAddressType
 * PURPOSE:
 *  This function set DNS control table TagetAddress Type.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDNSCtlTable_TargetAddressType(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_OM_SetDNSCtlTable_TargetAddress
 * PURPOSE:
 *  This function set DNS control table TagetAddress Type.
 *
 *
 * INPUT:
 *     CTRL_table   --  Nslookup control table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetDNSCtlTable_TargetAddress(DNS_Nslookup_CTRL_T *CTRL_table);


/* FUNCTION NAME : DNS_OM_SetLookupCtlOperStatusByIndex
 * PURPOSE:
 *  This function sets lookupCtlOperStatus by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlOperStatus
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlOperStatusByIndex(UI32_T index, UI32_T oper_status);

/* FUNCTION NAME : DNS_OM_SetLookupCtlRcByIndex
 * PURPOSE:
 *  This function sets lookupCtlRc by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlRc
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlRcByIndex(UI32_T index, UI32_T rc);

/* FUNCTION NAME : DNS_OM_SetLookupCtlTimeByIndex
 * PURPOSE:
 *  This function sets lookupCtlTime by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlTime
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlTimeByIndex(UI32_T index, UI32_T time);

/* FUNCTION NAME : DNS_OM_SetLookupCtlRowStatusByIndex
 * PURPOSE:
 *  This function sets lookupCtlRowStatus by index.
 *
 * INPUT:
 *      index       -- 0-based OM index of the entry
 *      oper_status -- new value of lookupCtlRowStatus
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupCtlRowStatusByIndex(UI32_T index, UI32_T row_status);

/* FUNCTION NAME : DNS_OM_SetLookupResultsIndexByIndex
 * PURPOSE:
 *  This function sets lookupResultsIndex by index.
 *
 *
 * INPUT:
 *      ctl_index       -- 0-based OM index of the control table
 *      result_index    -- 0-based OM index of the results table
 *      user_index      -- 1-based user index of the results table
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupResultsIndexByIndex(UI32_T ctl_index, UI32_T result_index,
    UI32_T user_index);

/* FUNCTION NAME : DNS_OM_SetLookupResultsAddressByIndex
 * PURPOSE:
 *  This function sets lookupResultsAddress by index.
 *
 *
 * INPUT:
 *      ctl_index       -- 0-based OM index of the control table
 *      result_index    -- 0-based OM index of the results table
 *      addr_str_p      -- pointer to L_INET_AddrIp_T structure
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 *
 * NOTES:
 *
 */
int DNS_OM_SetLookupResultsAddressByIndex(UI32_T ctl_index, UI32_T result_index,
    L_INET_AddrIp_T *addr_str_p);

/* FUNCTION NAME : DNS_OM_GetNextLookupResultTable
 * PURPOSE:
 *  This function get next nslookup result table.
 *
 *
 * INPUT:
 *     intable   --  nslookup result table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetNextLookupResultTable(DNS_Nslookup_Result_T *Result_table);


/* FUNCTION NAME : DNS_OM_GetLookupResultTable
 * PURPOSE:
 *  This function This function get nslookup result table.
 *
 *
 * INPUT:
 *     Result_table   --  nslookup result table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_GetLookupResultTable(DNS_Nslookup_Result_T *Result_table);



/* FUNCTION NAME : DNS_Nslookup_TargetAddrCheck
 * PURPOSE:
 *  This function  check nslookup control table length.
 *
 *
 * INPUT:
 *     addr_len   --  nslookup target address length.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
BOOL_T DNS_Nslookup_TargetAddrCheck(void *addr_len);

/* FUNCTION NAME : DNS_OM_SetNslookupTimeOut
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_SetNslookupTimeOut(UI32_T index, UI32_T timeout);

/* FUNCTION NAME : DNS_OM_GetNslookupTimeOut
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
void DNS_OM_GetNslookupTimeOut(UI32_T *timeout, UI32_T index);


/* FUNCTION NAME : DNS_OM_Nslookup_DeleteEntry
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_OM_Nslookup_DeleteEntry(UI32_T index);



/* FUNCTION NAME : DNS_Nslookup_DeleteEntry
 * PURPOSE:
 *  This function delete nslookup control and result tables.
 *
 *
 * INPUT:
 *     index        --  index of nslookup table that is been deleted.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *
 */
int DNS_Nslookup_DeleteEntry(UI32_T index);



/* FUNCTION NAME : DNS_OM_GetNslookupPurgeTime
 * PURPOSE:
 *  This function get nslookup PurgeTime.
 *
 *
 * INPUT:
 *     purge_time   --  nslookup PurgeTime.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
void DNS_OM_GetNslookupPurgeTime(UI32_T *purge_time);


/* FUNCTION NAME : DNS_OM_SetNslookupPurgeTime
 * PURPOSE:
 *  This function set nslookup PurgeTime.
 *
 *
 * INPUT:
 *     purge_time   --  nslookup PurgeTime.
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
void DNS_OM_SetNslookupPurgeTime(UI32_T purge_time);



/* FUNCTION NAME : DNS_Nslookup_PurgeTime
 * PURPOSE:
 *  This function return nslookup PurgeTime.
 *
 *
 * INPUT:
 *
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  TRUE :failure,
 *  FALSE:success.
 * NOTES:
 *
 */
UI32_T DNS_Nslookup_PurgeTime(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_IsNslookupCtlTableFull
 *-------------------------------------------------------------------------
 * PURPOSE  : Check nslookup control table is full or not.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : TRUE/FALSE
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
BOOL_T DNS_OM_IsNslookupCtlTableFull(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_GetNslookupCtlEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup control entry by index.
 * INPUT    : ctl_index  -- 0-based nslookup control entry index
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_GetNslookupCtlEntryByIndex(
    UI32_T ctl_index,
    DNS_Nslookup_CTRL_T *ctl_entry_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_CreateNslookupCtlEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup control entry.
 * INPUT    : ctl_entry_p  -- nslookup control entry
 * OUTPUT   : ctl_index_p  -- 0-based nslookup control entry index
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_CreateNslookupCtlEntry(
    DNS_Nslookup_CTRL_T *ctl_entry_p,
    UI32_T *ctl_index_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_GetNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Get nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index    -- 0-based nslookup result entry index
 *            result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_GetNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *result_entry_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_CreateNslookupResultEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : Create nslookup result entry by index.
 * INPUT    : ctl_index       -- 0-based nslookup control entry index
 *            result_index    -- 0-based nslookup result entry index
 *            result_entry_p  -- nslookup result entry
 * OUTPUT   : None
 * RETUEN   : DNS_OK/DNS_ERROR
 * NOTES    : None
 * ------------------------------------------------------------------------
 */
int
DNS_OM_CreateNslookupResultEntryByIndex(
    UI32_T ctl_index,
    UI32_T result_index,
    DNS_Nslookup_Result_T *result_entry_p
);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - DNS_OM_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for DNS om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T DNS_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p);


/* FUNCTION NAME : DNS_OM_CreateDomainNameListEntry
 * PURPOSE: To create a new dnsDomainListEntry in dnsDomainListEntry table.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. default name is 'domainname1', 'domainname2', ...
 */
BOOL_T DNS_OM_CreateDomainNameListEntry(UI32_T idx);

/* FUNCTION NAME : DNS_OM_DestroyDomainNameListEntry
 * PURPOSE: To destroy adnsDomainListEntry in dnsDomainListEntry table.
 * INPUT  : idx   -- index of dnsDomainListEntry
 *                   (1-based, key to search the entry)
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_DestroyDomainNameListEntry(UI32_T idx);

/* FUNCTION NAME : DNS_OM_SetDomainNameListEntry
 * PURPOSE: To modify a domain name in the dnsDomainListEntry table.
 * INPUT  : idx           -- index of entry.
 *                           (1-based, key to search the entry)
 *		    domain_name_p -- domain name content.
 * OUTPUT : none.
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 *          2. domain_name_p[0] == '\0' is not valid
 */
BOOL_T DNS_OM_SetDomainNameListEntry(UI32_T idx, char *domain_name_p);

/* FUNCTION NAME : DNS_OM_GetDomainNameListEntry
 * PURPOSE: To get entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *                           (1-based, key to search the entry)
 * OUTPUT : domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_GetDomainNameListEntry(UI32_T idx, char *domain_name_p);

/* FUNCTION NAME : DNS_OM_GetNextDomainNameListEntry
 * PURPOSE: To get next entry from the dnsDomainListEntry table.
 * INPUT  : idx           -- index of dnsDomainListEntry
 *                           (1-based, 0 to get the first,
 *                            key to search the entry)
 * OUTPUT : idx           -- next index of dnsDomainListEntry
 *          domain_name_p -- pointer to domain name content
 * RETURN : FALSE -- failure,
 *          TRUE  -- success.
 * NOTES  : 1. for SNMP.
 */
BOOL_T DNS_OM_GetNextDomainNameListEntry(UI32_T *idx, char *domain_name_p);

/* FUNCTION NAME : DNS_OM_ClearDatabase
 * PURPOSE: To clear database in DNS_OM.
 * INPUT  : none.
 * OUTPUT : none.
 * RETURN : none.
 * NOTES  : 1. for entering transition mode, etc...
 */
void DNS_OM_ClearDatabase(void);

/* FUNCTION NAME : DNS_OM_DeleteAllNameServer
 * PURPOSE:
 *  This function delete all name server IP address to the name server list.
 *
 *
 * INPUT:
 *      none
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *  DNS_ERROR : failure,
 *  DNS_OK    : success.
 * NOTES:
 *      This function will be called by SNMP.
 */
int DNS_OM_DeleteAllNameServer();

#endif  /* #ifndef DNS_OM_H */
