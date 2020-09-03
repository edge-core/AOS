/* MODULE NAME: dns_type.h
 * PURPOSE:
 *		This file provides some data types and macros used in dns module.
 * NOTES:
 *
 * History:
 *		Date		-- Modifier,	Reason
 *		2002-10-23	-- Wiseway,	created.
 *		2002-10-27	-- Wiseway	modified for deleting unused type.
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef DNS_TYPE_H
#define DNS_TYPE_H

#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "leaf_1611.h"
#include "leaf_1612.h"
#include "sys_type.h"
//#include "socket.h"

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "leaf_es3626a.h"
#include "leaf_2925.h"
#include "l_inet.h"

#define  DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_CLI    "SYSTEM_CLI"
#define  DNS_TYPE_NSLOOKUP_CTL_OWNER_INDEX_SYSTEM_WEB    "SYSTEM_WEB"
#define  DNS_TYPE_NSLOOKUP_TTL_TICKS                     (30 * DNS_SYS_CLK)

#define DNS_DEFAULT_STATUS  SYS_DFLT_IP_DOMAIN_LOOKUP
#define MAX_NBR_OF_HOST_TABLE_SIZE  SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE
#define MAX_NBR_OF_NAME_SERVER_TABLE_SIZE  SYS_ADPT_DNS_MAX_NBR_OF_NAME_SERVER_TABLE_SIZE
#define MAX_NBR_OF_DOMAIN_NAME_LIST  SYS_ADPT_DNS_MAX_NBR_OF_DOMAIN_NAME_LIST


#define  DNS_MAX_NAME_LENGTH		SYS_ADPT_DNS_MAX_NAME_LENGTH
/*#define  DNS_MAX_NAME_LENGTH		255*/
#define  DNS_CACHE_MAX_NAME_LENGTH	64

#define DNS_SERV_DISABLED		0
#define DNS_SERV_ENABLED		1

#define DNS_SERV_INIT			1			/*old value 0*/
#define DNS_SERV_STARTED		2			/*old value 1*/
#define DNS_SERV_STATUS_STOP           	3		/*old value 2*/
#define DNS_SERV_STATUS_STOPPED        	4		/*old value 3*/

#define  DNS_QUERY_LOCAL_FIRST		1     /* Query static host table first */
#define  DNS_QUERY_DNS_FIRST		2     /* Query DNS server first */
#define  DNS_QUERY_DNS_ONLY		3     /* Query DNS server only */


#define  DNS_MAX_TIME_OUT		SYS_ADPT_DNS_MAX_TIME_OUT
#define  DNS_MIN_TIME_OUT		SYS_ADPT_DNS_MIN_TIME_OUT
#define	DNS_DEF_TIME_OUT		SYS_DFLT_DNS_DEFAULT_TIME_OUT

/* QUERY,IQUERY,NOTSUP */
#define DNS_MAX_OP_CODE_NUMBER		3

/* refer to dns.h */
#define DNS_MAX_R_CODE_NUMBER		7

#define DNS_MAX_LOCAL_REQUEST		SYS_ADPT_DNS_MAX_NBR_OF_LOCAL_REQUEST
#define DNS_DEF_LOCAL_REQUEST		SYS_DFLT_DNS_DEFAULT_LOCAL_REQUEST
#define DNS_MIN_LOCAL_REQUEST		SYS_ADPT_DNS_MIN_NBR_OF_LOCAL_REQUEST


#define DNS_MAX_SERVER_REQUEST		SYS_ADPT_DNS_MAX_NBR_OF_SERVER_REQUEST
#define DNS_DEF_SERVER_REQUEST		SYS_DFLT_DNS_DEFAULT_SERVER_REQUEST
#define DNS_MIN_SERVER_REQUEST		SYS_ADPT_DNS_MIN_NBR_OF_SERVER_REQUEST

#define DNS_MAX_REQ_TIMEOUT		20
#define DNS_DEF_REQ_TIMEOUT		10
#define DNS_MIN_REQ_TIMEOUT		1


#define DNS_MIN_CACHE_SIZE			SYS_ADPT_DNS_MIN_NBR_OF_CACHE_SIZE
#define DNS_DEF_CACHE_SIZE			SYS_DFLT_DNS_DEFAULT_CACHE_SIZE
#define DNS_MAX_CACHE_SIZE			SYS_ADPT_DNS_MAX_NBR_OF_CACHE_SIZE

#define  DNS_ENABLE			1						/* dns is enabled*/
#define  DNS_DISABLE		2						/*dns is disabled*/

#define	DNS_OK                  (0)					/*operation is successful*/
#define	DNS_ERROR               (-1)					/*operation is failed*/
#define	DNS_OUT_OF_RANGE        (-2)					/*the parameter is out of range*/
#define DNS_SERVER_EXISTED      (-3)
#define DNS_RR_EXISTED          (-4)
#define DNS_PROCESSING          (-5)

#define DNS_RES_CFG_IMPL_ID			"DNS-RSV 0.1a"		/*implementation identifier string of dns resolver*/
#define DNS_SERV_CFG_IMPL_ID		"DNS-PROXY 0.1a"	/*implementation identifier string of dns proxy*/

/*the following macros from dns.h*/
#define DNS_SYS_CLK			100	/*(sysClkRateGet ())	*/
#define RAND16				(rand()&0xffff)

#define DNS_DONE			1

#define ZERO				0

#define MAXHOSTNAMELEN 			SYS_ADPT_DNS_MAX_NAME_LENGTH
#define MAXHOSTIPNUM			SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP_PER_HOST_NAME /*maggie liu, ES3628BT-FLF-ZZ-00147*/
#define MAXHOSTNAMENUM	  		SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP
/* River@May 7, 2008, add nslookup mib */
#define DNS_MAX_NSLOOKUP_REQUEST    (DNS_MAX_LOCAL_REQUEST/2)
#define DNS_DEF_NSLOOKUP_REQUEST    DNS_MAX_NSLOOKUP_REQUEST
#define DNS_MIN_NSLOOKUP_REQUEST    DNS_MIN_LOCAL_REQUEST
#define DNS_MAXNSLOOKUPHOSTIPNUM     8 /*MAXHOSTIPNUM*/ /*maggie liu, NSLOOKUP*/

#define DNS_MAX_NSLOOKUP_PTIME      SYS_ADPT_DNS_MAX_NSLOOKUP_PTIME
#define DNS_DEF_NSLOOKUP_PTIME      SYS_DFLT_DNS_NSLOOKUP_PTIME
#define DNS_MIN_NSLOOKUP_PTIME      SYS_ADPT_DNS_MIN_NSLOOKUP_PTIME

#define DNS_TYPE_INET_ADDR_TYPE_IPV4            VAL_dnsHostAddrInetAddressType_ipv4
#define DNS_TYPE_DEFAULT_DOMAIN_NAME_FORMAT     "DomainName%ld"
#define DNS_TYPE_DEFAULT_HOST_NAME_FORMAT       "HostName%ld"
#define DNS_TYPE_DEFAILT_NAME_SERVER_IP         0x0A010001   /* 10.1.0.1 */

/*the above macros from dns.h*/

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    DNS_TYPE_TRACE_ID_DNS_CACHE_GETRR = 0,
    DNS_TYPE_TRACE_ID_DNS_MGR_HOSTSHOW,
    DNS_TYPE_TRACE_ID_DNS_MGR_ASYNCSTARTNSLOOKUP,
    DNS_TYPE_TRACE_ID_DNS_RXTCPMSG,
    DNS_TYPE_TRACE_ID_DNS_RXUDPMSG,
    DNS_TYPE_TRACE_ID_DNS_PROXY_CALLBACK,
    DNS_TYPE_TRACE_ID_DNS_PROXY_DAEMON,
    DNS_TYPE_TRACE_ID_DNS_HOSTLIB_HOSTADD,
    DNS_TYPE_TRACE_ID_DNS_HOSTLIB_ADDHOSTENTRY,
    DNS_TYPE_TRACE_ID_DNS_HOSTLIB_SETDNSHOSTENTRYBYNAMEANDINDEX,
    DNS_TYPE_TRACE_ID_DNS_OM_SBELT_TABLE_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_OM_IP_DOMAIN_LIST_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_TMP_BUFFER,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SBELT_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_SENDBUF_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SLIST_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_CACHERR_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_RECVBUF_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_DNS_SEND,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_CACHEDREPLY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_LOCALREPLY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_ERRORREPLY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_ERRORMAKE,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_HOSTENT_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ADDR_LIST_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ADDR_LIST_ENTRY_ELM,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_NAME_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ALIASES_ENTRY,
    DNS_TYPE_TRACE_ID_DNS_RESOLVER_H_ALIASES_ENTRY_ELM
};

typedef struct DNS_ServCounterEntry_S {
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
} DNS_ServCounterEntry_T;

/* INDEX { dnsServCounterOpCode, dnsServCounterQClass, dnsServCounterQType, dnsServCounterTransport } */

typedef struct DNS_ResConfigSbeltEntry_S {
    L_INET_AddrIp_T dnsResConfigSbeltAddr;  /* not-accessible.The IP address of the Sbelt name server */
    I8_T	dnsResConfigSbeltName[MAXSIZE_dnsResConfigSbeltName+1];  /*read-create.The DNS name */
    int		dnsResConfigSbeltRecursion;   /*read-create .Kind of queries resolver will be sending to the name
                                                  server identified in this row of the table*/
    int		dnsResConfigSbeltPref;        /* read-create .This value identifies the preference for the name server*/
    I8_T	dnsResConfigSbeltSubTree[MAXSIZE_dnsResConfigSbeltSubTree+1]; /* not-accessible */
    int		dnsResConfigSbeltClass;        /*not-accessible.not-accessible.The class of DNS queries that will be sent to the server*/
    int		dnsResConfigSbeltStatus;       /* read-create. Row status column for this row of the Sbelt table.      */
} DNS_ResConfigSbeltEntry_T;

/*INDEX:dnsResConfigSbeltAddr,dnsResConfigSbeltSubTree,dnsResConfigSbeltClass              */



typedef struct DNS_ResCounterByOpcodeEntry_S {
    int       dnsResCounterByOpcodeCode;      /*not-accessible. not-accessible. The index to this table.  */
    int       dnsResCounterByOpcodeQueries;   /*read-only. Total number of queries that have sent out by the resolver  for the OpCode
                                                which is the index to this row of the table*/
    int       dnsResCounterByOpcodeResponses; /*read-only. Total number of responses that have been received by the resolver
                                                for the OpCode which is the index to this row of the table*/
} DNS_ResCounterByOpcodeEntry_T;
/* INDEX { dnsResCounterByOpcodeCode } */



typedef struct DNS_ResCounterByRcodeEntry_S {
    int    dnsResCounterByRcodeCode;        /* not-accessible.The index to this table.  The Response Codes*/
    int    dnsResCounterByRcodeResponses;   /* read-only. Number of responses the resolver has received for the
                                              response code value which identifies this row of the table.*/
} DNS_ResCounterByRcodeEntry_T;
/* INDEX { dnsResCounterByRcodeCode } */



/* the following structure are used for proxy configuration*/

typedef struct DNS_ProxyConfig_S
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

} DNS_ProxyConfig_T;

/* the above structure are used for proxy configuration*/

/* the following structure are used for resolver configuration*/
typedef struct DNS_ResConfigSbelt_S {
    DNS_ResConfigSbeltEntry_T	dns_res_config_sbelt_entry;
    struct DNS_ResConfigSbelt_S	*next_p;
} DNS_ResConfigSbelt_T;


typedef struct DNS_ResConfig_S
{
  I8_T		dnsResConfigImplementIdent[MAXSIZE_dnsResConfigImplementIdent+1]; /*The implementation identification string for the
                                                                      resolver software in use on the system, for example; `RES-2.1'"*/
  int		dnsResConfigService;   /* Kind of DNS resolution service provided:
                                          recursiveOnly(1) indicates a stub resolver.
                                          iterativeOnly(2) indicates a normal full service resolver.
                                          recursiveAndIterative(3) indicates a full-service
                                          resolver which performs a mix of recursive and iterative queries*/
  int		dnsResConfigMaxCnames; /*Limit on how many CNAMEs the resolver should allow
                                         before deciding that there's a CNAME loop.  Zero means
                                         that resolver has no explicit CNAME limit.*/
  DNS_ResConfigSbelt_T	*dns_res_config_sbelt_table_p; /*able of safety belt information used by the resolver
                                                          when it hasn't got any better idea of where to send a
                                                          query, such as when the resolver is booting or is a stub resolver.*/
  UI32_T	dnsResConfigUpTime;    /*If the resolver has a persistent state (e.g., a
                                         process), this value will be the time elapsed since it
                                         started.  For software without persistant state, this value will be 0  */
  UI32_T	dnsResConfigResetTime;
  int		dnsResConfigReset;     /*Status/action object to reinitialize any persistant
                                         resolver state.  When set to reset(2), any persistant
                                         resolver state (such as a process) is reinitialized as if
                                         the resolver had just been started.  This value will
                                         never be returned by a read operation.  When read, one of
                                         the following values will be returned:
                                         other(1) - resolver in some unknown state;
                                         initializing(3) - resolver (re)initializing;
                                         running(4) - resolver currently running*/
 int		dnsResConfigQueryOrder;  /* DNS_QUERY_LOCAL_FIRST:Query static host table first;
                                              DNS_QUERY_DNS_FIRST: Query DNS server first
                                              DNS_QUERY_ DNS_ONLY: Query DNS server only
                                              default: DNS_QUERY_LOCAL_FIRST*/
} DNS_ResConfig_T;


typedef struct DNS_CacheConfig_S
{
   int		cache_status;				/* 1 enable cache, 2 disable cache */
   UI32_T	cache_max_ttl;			/* the max ttl that cache permits  */
   int		cache_good_caches;
   int		cache_bad_caches;
   int		cache_max_entries;            /*the max  entries in cache, range: 1280..6400 default :2560        */
} DNS_CacheConfig_T;


typedef struct DNS_IpDomain_S {
 char 	DnsIpDomainName[DNS_MAX_NAME_LENGTH+1];
 I8_T   idx;                        /* 1-based, for snmp */
 struct DNS_IpDomain_S *next_p;
} DNS_IpDomain_T;

/* the above structure are used for resolver configuration */


typedef struct DNS_Config_S
{
  int 		DnsOptimize;
  int		DnsStatus;             /*default (1) enabled; (0) disabled*/ 	/* pass */
  I8_T		DnsIpDomainName[DNS_MAX_NAME_LENGTH+1];
  int 		DnsMaxLocalRequests;       /*resolver can deal with up to 10 requests simultaneously.range 1:10 default 5*/
  int 		DnsDebugStatus;        /*default (0) disabled ; (1)enabled  */
  UI32_T 	DnsTimeOut;  /*range 1:15 (in seconds).default 12*/
  DNS_IpDomain_T     *DnsIpDomainList_p;
  DNS_ResConfig_T    DnsResConfig;
  DNS_ProxyConfig_T  DnsProxyConfig;
  DNS_CacheConfig_T  DnsResCache;
} DNS_Config_T;


typedef struct DNS_ProxyCounter_S {
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
} DNS_ProxyCounter_T;

typedef struct DNS_ResCounter_S {
    DNS_ResCounterByOpcodeEntry_T       dnsResCounterByOpcodeTable[DNS_MAX_OP_CODE_NUMBER]; /*Table of the current count of resolver queries and answers.*/
    DNS_ResCounterByRcodeEntry_T        dnsResCounterByRcodeTable[DNS_MAX_R_CODE_NUMBER];   /*Table of the current count of responses to resolver queries*/
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
} DNS_ResCounter_T;


/* the following type DNS_Hdr_S is used for the header of DNS packet*/
typedef struct DNS_Hdr_S
{

	UI16_T id;

#if (SYS_HWCFG_LITTLE_ENDIAN_CPU == TRUE)
	/* third */
	unsigned int   rd:1;
	unsigned int   tc:1;
	unsigned int   aa:1;
	unsigned int   opcode:4;
	unsigned int   qr:1;
	/* fourth */
	unsigned int   rcode:4;
	unsigned int   z:3;
	unsigned int   ra:1;
#else
	/* third */
	unsigned int   qr:1;		  /* it's a query or responce? */
	unsigned int   opcode:4;	  /* do whar operation? query?inverse query? or oter?*/
	unsigned int   aa:1; 		  /* whether answer is authorized ? if answer has
					     			 multi-part it just correspond for who match
					     			 query name or first in answer section */
	unsigned int   tc:1;		  /* does this pkt truncated because length is over-long.*/
	unsigned int   rd:1;		  /* recursive desired or not*/
	/* fourth */
	unsigned int   ra:1;		  /* recursive available ??*/
	unsigned int   z:3; 		  /* should alway be zero, if not,ERROR!!  */
	unsigned int   rcode:4;  	  /* responce code*/
#endif

	UI16_T qdcount;
	UI16_T ancount;
	UI16_T nscount;
	UI16_T arcount;
}DNS_Hdr_T;
/* the above type DNS_Hdr_S is used for the header of DNS packet*/


enum DNS_Qr_E
{
	DNS_QR_QUERY = 0,
	DNS_QR_RESP
};

enum DNS_Opcode_E
{
	DNS_OP_QUERY = 0,
	DNS_OP_IQUERY,
	DNS_OP_STATUS
};

enum DNS_Rcode_E
{
	DNS_RC_OK = 0,
	DNS_RC_FORMAT,
	DNS_RC_SERVFAIL,
	DNS_RC_NAMEERR,
	DNS_RC_NOTSUPP,
	DNS_RC_REFUSED,
	DNS_RC_TCPREFUSED = 254,
	DNS_RC_TRUNC
};

enum DNS_Qtype_E
{
	/*special rr type codes for queries */
	DNS_QT_IXFR = 	251,
	DNS_QT_AXFR,
	DNS_QT_MAILB,
	DNS_QT_MAILA,
	DNS_QT_ALL,
	/* special classes for queries */
	DNS_QC_ALL
};

typedef struct DNS_RR_S{
	/* the name is the first field. It has variable length, so it can't be put in the struct */
	/* char name[?]*/
	UI16_T rtype;
	UI16_T rclass;
	UI32_T  rttl;
	UI16_T rdlength;
	/* rdata follows */
}DNS_RR_T;

enum DNS_RRType_E
{
	DNS_RRT_A = 1,
	DNS_RRT_NS,
	DNS_RRT_MD,
	DNS_RRT_MF,
	DNS_RRT_CNAME,
	DNS_RRT_SOA,
	DNS_RRT_MB,
	DNS_RRT_MG,
	DNS_RRT_MR,
	DNS_RRT_NULL,
	DNS_RRT_WKS,
	DNS_RRT_PTR,
	DNS_RRT_HINFO,
	DNS_RRT_MINFO,
	DNS_RRT_MX,
	DNS_RRT_TXT,
	DNS_RRT_AAAA = 28
};

#define DNS_RRT_MIN      	DNS_RRT_A
#if (SYS_CPNT_IPV6 == TRUE)
#define DNS_RRT_MAX 	DNS_RRT_AAAA
#else
#define DNS_RRT_MAX 	    DNS_RRT_TXT
#endif

/* rr classes */
enum DNS_RRClass_E
{
	DNS_RRC_IN = 1,
	DNS_RRC_CS,
	DNS_RRC_CH,
	DNS_RRC_HS
};

/*
 * Types for compression buffers.
 */
typedef struct DNS_Compress_S {
	int    index;
	UI8_T string[DNS_MAX_NAME_LENGTH+1];
} DNS_Compress_T;

typedef struct HostName_S
{
  	char  name[MAXHOSTNAMELEN]; /*maggie liu, ES4827G-FLF-ZZ-00243*/
} HostName_T, *HostName_PTR;

typedef struct HostEntry_S
{
    HostName_T			hostName[MAXHOSTNAMENUM];
    L_INET_AddrIp_T     netAddr[MAXHOSTIPNUM];
	int					port;
	struct HostEntry_S*	next_P;

} HostEntry_T, *HostEntry_PTR;

typedef struct DNS_CacheRecord_S
{
	short	flag;								/* flag of this Cache node */
	char 	name[DNS_CACHE_MAX_NAME_LENGTH];/* domain name */
	L_INET_AddrIp_T ip;									/* IP address */
	UI16_T	type;
	UI32_T  arrived_time;                   	/* time when RR was received */
	UI32_T  ttl;                            	/* This is the TTL value*/
} DNS_CacheRecord_T;

#define DNS_CACHE_ENTRY_TYPE_CNAME  1
#define DNS_CACHE_ENTRY_TYPE_ALIAS  5

/* River@May 7, 2008, add nslookup mib */
typedef struct DNS_Nslookup_CTRL_Head_S
{
    UI32_T          CtlOwnerIndexLen;
    UI8_T           CtlOwnerIndex[MAXSIZE_dnsCtlOwnerIndex+1];
    UI32_T          OperationNameLen;
    UI8_T           OperationName[MAXSIZE_dnsCtlOperationName+1];

}DNS_Nslookup_CTRL_Head_T;

typedef struct DNS_Nslookup_CTRL_S
{
    UI32_T          CtlOwnerIndexLen;
    UI8_T           CtlOwnerIndex[MAXSIZE_dnsCtlOwnerIndex+1];
    UI32_T          OperationNameLen;
    UI8_T           OperationName[MAXSIZE_dnsCtlOperationName+1];

    /* The following currently only supports taking a DNS address to look up
     * its IP address.
     * If the input will support an IP address to look up its DNS name,
     * then we will change to L_INET_AddrDns_T and L_INET_AddrIp_T
     * to communicate with the core layer.
     * The SNMP layer is responsible for converting between an InetAddressType
     * and InetAddress pair for SNMP, and L_INET for the core layer.
     */
    UI32_T          TargetAddressType;
    UI32_T          TargetAddressLen;
    UI8_T           TargetAddress[MAXSIZE_dnsCtlTargetAddress +1];

    UI32_T          OperStatus;
    UI32_T          Time;
    I32_T           Rc;
    UI32_T          RowStatus;

    UI32_T          af_family;
    UI32_T          created_time;     /* ticks */
}DNS_Nslookup_CTRL_T;

typedef struct DNS_Nslookup_Result_Head_S
{
    UI32_T                  CtlOwnerIndexLen;
    UI8_T                   CtlOwnerIndex[MAXSIZE_dnsCtlOwnerIndex+1];
    UI32_T                  OperationNameLen;
    UI8_T                   OperationName[MAXSIZE_dnsCtlOperationName+1];
    UI32_T                  ResultsIndex;
}DNS_Nslookup_Result_Head_T;


typedef struct DNS_Nslookup_Result_S
{
    UI32_T                  CtlOwnerIndexLen;
    UI8_T                   CtlOwnerIndex[MAXSIZE_dnsCtlOwnerIndex+1];
    UI32_T                  OperationNameLen;
    UI8_T                   OperationName[MAXSIZE_dnsCtlOperationName+1];
    UI32_T                  ResultsIndex;

    /* L_INET_AddrIp_T stores the resulting address of a "name-to-address"
     * look-up as an IP address.
     * If we will need resulting name of an "address-to-name" look-up,
     * then we will need an L_INET_AddrDns_T structure.
     * The SNMP layer is responsible for converting between an InetAddressType
     * and InetAddress pair for SNMP, and L_INET for the core layer.
     */
    L_INET_AddrIp_T         ResultsAddress_str;
}DNS_Nslookup_Result_T;

enum DNS_Nslookup_OperStatus_Type_E
{
    NSLOOKUP_OPSTATUS_ENABLE =1,
    NSLOOKUP_OPSTATUS_NOTSTARTED,
    NSLOOKUP_OPSTATUS_COMPLETED
};

typedef struct DNS_TYPE_SockAddr_S
{
    union
    {
        struct sockaddr_in  inaddr_4;
        struct sockaddr_in6 inaddr_6;
    } s_addr_u;

    UI8_T saddr_len;

}DNS_TYPE_SockAddr_T;

#endif  /* #ifndef DNS_TYPE_H */
