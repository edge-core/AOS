/* Module Name: IP_LIB.H
 * Purpose:
 *      IP_LIB provides some library for network layer, include loopback IP, zero IP,
 *      overlapping...
 *
 * Notes:
 *   1. API.
 *      IP_LIB_IsZeroNetwork() : 00.xx.xx.xx/8
 *      IP_LIB_IsLoopBackIp() : 127.xx.xx.xx/8
 *      IP_LIB_IsClassBReserveIp : 191.255.xx.xx/16
 *      IP_LIB_IsClassCReservedIp : 192.00.00.xx/24, 223.255.255.0/24
 *      IP_LIB_IsTestingIp : 240.00.00.00 ~ 255.255.255.254
 *      IP_LIB_IsBroadcastIp : 255.255.255.255
 *      IP_LIB_IsValidForNetworkInterface : no above IP.
 *      IP_LIB_IsMulticastIp : 224.00.00.00 ~ 239.255.255.255
 *      IP_LIB_GetSubnetBroadcastIp (ip,mask, &bcast_ip)
 *      IP_LIB_IsIpInClassA : 0~127
 *      IP_LIB_IsIpInClassB : 128~191
 *      IP_LIB_IsIpInClassC : 192~223
 *      IP_LIB_IsIpInClassD : 224~239
 *      IP_LIB_IsIpInClassE : 240~255
 *
 * History:
 *       Date       --  Modifier,   Reason
 *  0.1 2002.06.23  --  William,    First Created.
 *      2007.7      --  peter_yu    Port to Linux Platform
 *
 * Copyright(C)      Accton Corporation, 2002-2007.
 */

#ifndef     _IP_LIB_H
#define     _IP_LIB_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "l_inet.h"
#include <net/if.h>


/* NAME CONSTANT DECLARATIONS
 */
/*
 *  Define constant used in IP_LIB.
 */
//#define IP_LIB_BROADCAST_IP                     0x0ffffffff
#define IP_LIB_BROADCAST_IP_BYTE                0xff    /* for byte0 - byte3 */

/*  Function returned value */
#define IP_LIB_OK                               0
#define IP_LIB_INVALID_IP                       0x80000010
#define IP_LIB_INVALID_IP_LOOPBACK_IP           0x80000011
#define IP_LIB_INVALID_IP_ZERO_NETWORK          0x80000012
#define IP_LIB_INVALID_IP_BROADCAST_IP          0x80000013
#define IP_LIB_INVALID_IP_IN_CLASS_D            0x80000014
#define IP_LIB_INVALID_IP_IN_CLASS_E            0x80000015
#define IP_LIB_INVALID_IP_SUBNET_NETWORK_ID     0x80000016
#define IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR 0x80000017
#define IP_LIB_INVALID_ARG                      0x80000008
#define IP_LIB_INVALID_IPV6_UNSPECIFIED         0x80000009
#define IP_LIB_INVALID_IPV6_LOOPBACK            0x8000000A
#define IP_LIB_INVALID_IPV6_MULTICAST           0x8000000B


/* MACRO FUNCTION DECLARATIONS
 */
#define IP_LIB_CHKAPP_ENUM_APP(a)       a,
#define IP_LIB_CHKAPP_NAME_APP(a)       IP_LIB_CHKAPP_NAME_APP_X(a)
#define IP_LIB_CHKAPP_NAME_APP_X(a)     #a,
#define IP_LIB_CHKAPP_CNV_BIT(a)        a##_BIT = (1 << a),

#define IP_LIB_SKTTYPE_LST(_)               \
            _(IP_LIB_SKTTYPE_TCP        )   \
            _(IP_LIB_SKTTYPE_UDP        )

/*  APP ID list for socket port checking
 */
#define IP_LIB_CHKAPP_LST(_)                \
            _(IP_LIB_CHKAPP_TCPMUX     )    \
            _(IP_LIB_CHKAPP_ECHO       )    \
            _(IP_LIB_CHKAPP_DISCARD    )    \
            _(IP_LIB_CHKAPP_SYSTAT     )    \
            _(IP_LIB_CHKAPP_DAYTIM     )    \
            _(IP_LIB_CHKAPP_NETSTA     )    \
            _(IP_LIB_CHKAPP_QOTD       )    \
            _(IP_LIB_CHKAPP_CHARGEN    )    \
            _(IP_LIB_CHKAPP_FTP_DATA   )    \
            _(IP_LIB_CHKAPP_FTP_ACCESS )    \
            _(IP_LIB_CHKAPP_SSH        )    \
            _(IP_LIB_CHKAPP_TELNET     )    \
            _(IP_LIB_CHKAPP_SMTP       )    \
            _(IP_LIB_CHKAPP_TIME       )    \
            _(IP_LIB_CHKAPP_NAME       )    \
            _(IP_LIB_CHKAPP_NICNAME    )    \
            _(IP_LIB_CHKAPP_TACACSP    )    \
            _(IP_LIB_CHKAPP_DOMAIN     )    \
            _(IP_LIB_CHKAPP_PRIV_RJS   )    \
            _(IP_LIB_CHKAPP_FINGER     )    \
            _(IP_LIB_CHKAPP_HTTP       )    \
            _(IP_LIB_CHKAPP_TTYLINK    )    \
            _(IP_LIB_CHKAPP_SUPDUP     )    \
            _(IP_LIB_CHKAPP_HOSTRIAME  )    \
            _(IP_LIB_CHKAPP_ISO_TSAP   )    \
            _(IP_LIB_CHKAPP_GPPITNP    )    \
            _(IP_LIB_CHKAPP_ACR_NEMA   )    \
            _(IP_LIB_CHKAPP_POP2       )    \
            _(IP_LIB_CHKAPP_POP3       )    \
            _(IP_LIB_CHKAPP_SUNRPC     )    \
            _(IP_LIB_CHKAPP_AUTH       )    \
            _(IP_LIB_CHKAPP_SFTP       )    \
            _(IP_LIB_CHKAPP_UUCP_PATH  )    \
            _(IP_LIB_CHKAPP_NNTP       )    \
            _(IP_LIB_CHKAPP_NTP        )    \
            _(IP_LIB_CHKAPP_LOC_SRV    )    \
            _(IP_LIB_CHKAPP_NETBIOS    )    \
            _(IP_LIB_CHKAPP_IMAP2      )    \
            _(IP_LIB_CHKAPP_SNMP       )    \
            _(IP_LIB_CHKAPP_SNMPTRAP   )    \
            _(IP_LIB_CHKAPP_BGP        )    \
            _(IP_LIB_CHKAPP_LDAP       )    \
            _(IP_LIB_CHKAPP_HTTPS      )    \
            _(IP_LIB_CHKAPP_SMTP_SSL   )    \
            _(IP_LIB_CHKAPP_PRINT      )    \
            _(IP_LIB_CHKAPP_LOGIN      )    \
            _(IP_LIB_CHKAPP_SHELL      )    \
            _(IP_LIB_CHKAPP_PRINTER    )    \
            _(IP_LIB_CHKAPP_TEMPO      )    \
            _(IP_LIB_CHKAPP_COURIER    )    \
            _(IP_LIB_CHKAPP_CHAT       )    \
            _(IP_LIB_CHKAPP_NETNEWS    )    \
            _(IP_LIB_CHKAPP_UUCP       )    \
            _(IP_LIB_CHKAPP_DHCPV6_CLN )    \
            _(IP_LIB_CHKAPP_DHCPV6_SVR )    \
            _(IP_LIB_CHKAPP_REMOTEFS   )    \
            _(IP_LIB_CHKAPP_NNTP_SSL   )    \
            _(IP_LIB_CHKAPP_STMP       )    \
            _(IP_LIB_CHKAPP_LDAP_SSL   )    \
            _(IP_LIB_CHKAPP_FTPS_DATA  )    \
            _(IP_LIB_CHKAPP_FTPS_CLNT  )    \
            _(IP_LIB_CHKAPP_POP3_SSL   )    \
            _(IP_LIB_CHKAPP_RMON       )    \
            _(IP_LIB_CHKAPP_NFS        )    \
            _(IP_LIB_CHKAPP_APPLE_SASL )    \
            _(IP_LIB_CHKAPP_LOCKD      )    \
            _(IP_LIB_CHKAPP_X11        )    \
            _(IP_LIB_CHKAPP_ALT_IRC    )    \
            _(IP_LIB_CHKAPP_STD_IRC    )

#define IP_LIB_SKTTYPE_TCP_AND_UDP_BIT      (IP_LIB_SKTTYPE_TCP_BIT | IP_LIB_SKTTYPE_UDP_BIT)

/* must sort in ascending order by the socket port number
 * each row contains
 *      PORT NO,   TCP/UDP_TYPE_USED_BY_APP,        APP_ALLOWED_TO_USE
 *                 (refer to IP_LIB_SKTTYPE_LST)    (refer to IP_LIB_CHKAPP_LST)
 */
#define IP_LIB_CHKAPP_SKPORT_LST(_)                                     \
    _(1,    /* tcpmux     */ TCP_AND_UDP,   IP_LIB_CHKAPP_TCPMUX    )   \
    _(7,    /* echo       */ TCP_AND_UDP,   IP_LIB_CHKAPP_ECHO      )   \
    _(9,    /* discard    */ TCP_AND_UDP,   IP_LIB_CHKAPP_DISCARD   )   \
    _(11,   /* systat     */ TCP_AND_UDP,   IP_LIB_CHKAPP_SYSTAT    )   \
    _(13,   /* daytim     */ TCP_AND_UDP,   IP_LIB_CHKAPP_DAYTIM    )   \
    _(15,   /* netsta     */ TCP_AND_UDP,   IP_LIB_CHKAPP_NETSTA    )   \
    _(17,   /* qotd       */ TCP_AND_UDP,   IP_LIB_CHKAPP_QOTD      )   \
    _(19,   /* chargen    */ TCP_AND_UDP,   IP_LIB_CHKAPP_CHARGEN   )   \
    _(20,   /* ftp data   */ TCP,           IP_LIB_CHKAPP_FTP_DATA  )   \
    _(21,   /* ftp access */ TCP,           IP_LIB_CHKAPP_FTP_ACCESS)   \
    _(22,   /* ssh        */ TCP_AND_UDP,   IP_LIB_CHKAPP_SSH       )   \
    _(23,   /* telnet     */ TCP,           IP_LIB_CHKAPP_TELNET    )   \
    _(25,   /* smtp       */ TCP,           IP_LIB_CHKAPP_SMTP      )   \
    _(37,   /* time       */ TCP_AND_UDP,   IP_LIB_CHKAPP_TIME      )   \
    _(42,   /* name       */ TCP_AND_UDP,   IP_LIB_CHKAPP_NAME      )   \
    _(43,   /* nicname    */ TCP,           IP_LIB_CHKAPP_NICNAME   )   \
    _(49,   /* tacacs+    */ TCP_AND_UDP,   IP_LIB_CHKAPP_TACACSP   )   \
    _(53,   /* domain     */ TCP_AND_UDP,   IP_LIB_CHKAPP_DOMAIN    )   \
    _(77,   /* priv-rjs   */ TCP,           IP_LIB_CHKAPP_PRIV_RJS  )   \
    _(79,   /* finger     */ TCP,           IP_LIB_CHKAPP_FINGER    )   \
    _(80,   /* http       */ TCP_AND_UDP,   IP_LIB_CHKAPP_HTTP      )   \
    _(87,   /* ttylink    */ TCP_AND_UDP,   IP_LIB_CHKAPP_TTYLINK   )   \
    _(95,   /* supdup     */ TCP_AND_UDP,   IP_LIB_CHKAPP_SUPDUP    )   \
    _(101,  /* hostriame  */ TCP,           IP_LIB_CHKAPP_HOSTRIAME )   \
    _(102,  /* iso-tsap   */ TCP,           IP_LIB_CHKAPP_ISO_TSAP  )   \
    _(103,  /* gppitnp    */ TCP_AND_UDP,   IP_LIB_CHKAPP_GPPITNP   )   \
    _(104,  /* acr-nema   */ TCP_AND_UDP,   IP_LIB_CHKAPP_ACR_NEMA  )   \
    _(109,  /* pop2       */ TCP,           IP_LIB_CHKAPP_POP2      )   \
    _(110,  /* pop3       */ TCP,           IP_LIB_CHKAPP_POP3      )   \
    _(111,  /* sunrpc     */ TCP_AND_UDP,   IP_LIB_CHKAPP_SUNRPC    )   \
    _(113,  /* auth       */ TCP_AND_UDP,   IP_LIB_CHKAPP_AUTH      )   \
    _(115,  /* sftp       */ TCP,           IP_LIB_CHKAPP_SFTP      )   \
    _(117,  /* uucp-path  */ TCP,           IP_LIB_CHKAPP_UUCP_PATH )   \
    _(119,  /* nntp       */ TCP,           IP_LIB_CHKAPP_NNTP      )   \
    _(123,  /* NTP        */ UDP,           IP_LIB_CHKAPP_NTP       )   \
    _(135,  /* loc-srv    */ TCP_AND_UDP,   IP_LIB_CHKAPP_LOC_SRV   )   \
    _(139,  /* netbios    */ TCP_AND_UDP,   IP_LIB_CHKAPP_NETBIOS   )   \
    _(143,  /* imap2      */ TCP,           IP_LIB_CHKAPP_IMAP2     )   \
    _(161,  /* snmp       */ UDP,           IP_LIB_CHKAPP_SNMP      )   \
    _(162,  /* snmptrap   */ TCP_AND_UDP,   IP_LIB_CHKAPP_SNMPTRAP  )   \
    _(179,  /* BGP        */ TCP,           IP_LIB_CHKAPP_BGP       )   \
    _(389,  /* ldap       */ TCP_AND_UDP,   IP_LIB_CHKAPP_LDAP      )   \
    _(443,  /* https      */ TCP,           IP_LIB_CHKAPP_HTTPS     )   \
    _(465,  /* smtp+ssl   */ TCP,           IP_LIB_CHKAPP_SMTP_SSL  )   \
    _(512,  /* print      */ TCP_AND_UDP,   IP_LIB_CHKAPP_PRINT     )   \
    _(513,  /* login      */ TCP_AND_UDP,   IP_LIB_CHKAPP_LOGIN     )   \
    _(514,  /* shell      */ TCP_AND_UDP,   IP_LIB_CHKAPP_SHELL     )   \
    _(515,  /* printer    */ TCP,           IP_LIB_CHKAPP_PRINTER   )   \
    _(526,  /* tempo      */ TCP_AND_UDP,   IP_LIB_CHKAPP_TEMPO     )   \
    _(530,  /* courier    */ TCP_AND_UDP,   IP_LIB_CHKAPP_COURIER   )   \
    _(531,  /* chat       */ TCP_AND_UDP,   IP_LIB_CHKAPP_CHAT      )   \
    _(532,  /* netnews    */ TCP,           IP_LIB_CHKAPP_NETNEWS   )   \
    _(540,  /* uucp       */ TCP,           IP_LIB_CHKAPP_UUCP      )   \
    _(546,  /* dhcpv6 cln */ TCP_AND_UDP,   IP_LIB_CHKAPP_DHCPV6_CLN)   \
    _(547,  /* dhcpv6 svr */ TCP_AND_UDP,   IP_LIB_CHKAPP_DHCPV6_SVR)   \
    _(556,  /* remotefs   */ TCP,           IP_LIB_CHKAPP_REMOTEFS  )   \
    _(563,  /* nntp+ssl   */ TCP_AND_UDP,   IP_LIB_CHKAPP_NNTP_SSL  )   \
    _(587,  /* stmp       */ TCP,           IP_LIB_CHKAPP_STMP      )   \
    _(636,  /* ldap+ssl   */ TCP_AND_UDP,   IP_LIB_CHKAPP_LDAP_SSL  )   \
    _(989,  /* ftps data  */ TCP_AND_UDP,   IP_LIB_CHKAPP_FTPS_DATA )   \
    _(990,  /* ftps clnt  */ TCP_AND_UDP,   IP_LIB_CHKAPP_FTPS_CLNT )   \
    _(993,  /* ldap+ssl   */ TCP,           IP_LIB_CHKAPP_LDAP_SSL  )   \
    _(995,  /* pop3+ssl   */ TCP,           IP_LIB_CHKAPP_POP3_SSL  )   \
    _(1042, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(1043, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(1044, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(1045, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(1046, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(1047, /* rmon       */ TCP_AND_UDP,   IP_LIB_CHKAPP_RMON      )   \
    _(2049, /* nfs        */ UDP,           IP_LIB_CHKAPP_NFS       )   \
    _(3659, /* apple-sasl */ TCP_AND_UDP,   IP_LIB_CHKAPP_APPLE_SASL)   \
    _(4045, /* lockd      */ TCP_AND_UDP,   IP_LIB_CHKAPP_LOCKD     )   \
    _(6000, /* X11        */ TCP,           IP_LIB_CHKAPP_X11       )   \
    _(6665, /* ALT IRC    */ TCP,           IP_LIB_CHKAPP_ALT_IRC   )   \
    _(6666, /* ALT IRC    */ TCP,           IP_LIB_CHKAPP_ALT_IRC   )   \
    _(6667, /* STD IRC    */ TCP,           IP_LIB_CHKAPP_STD_IRC   )   \
    _(6668, /* ALT IRC    */ TCP,           IP_LIB_CHKAPP_ALT_IRC   )   \
    _(6669, /* ALT IRC    */ TCP,           IP_LIB_CHKAPP_ALT_IRC   )


/* DATA TYPE DECLARATIONS
 */
enum IP_LIB_ChkAppId_E
{
    IP_LIB_CHKAPP_LST(IP_LIB_CHKAPP_ENUM_APP)
    IP_LIB_CHKAPP_MAX               /* maximum no, for boundary checking  */
};

enum IP_LIB_SktType_E
{
    IP_LIB_SKTTYPE_LST(IP_LIB_CHKAPP_ENUM_APP)
    IP_LIB_SKTTYPE_MAX              /* maximum no, for boundary checking  */
};

enum IP_LIB_SktTypeBit_E
{
    IP_LIB_SKTTYPE_LST(IP_LIB_CHKAPP_CNV_BIT)
    IP_LIB_SKTTYPE_BIT_MAX          /* maximum no, for boundary checking  */
};

typedef struct
{
    UI16_T  skt_port;       /* socket port number           */
    UI16_T  skt_type_bmp;   /* bitmap of socket type used   */
    UI32_T  allow_app;      /* which app id allowed to use  */
}   IP_LIB_SkPortInfo_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : IP_LIB_Init
 * PURPOSE:
 *      Initialize IP_LIB used system resource, eg. protection semaphore.
 *      Clear all working space.
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
void IP_LIB_Init(void);


/* FUNCTION NAME : IP_LIB_IsZeroNetwork
 * PURPOSE:
 *      Is the ip belonging to zero network ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip belong zero network.
 *      FALSE -- ip not belong no zero network.
 *
 * NOTES:
 *      1. It's class A subnet : 00.xx.xx.xx.
 */
BOOL_T IP_LIB_IsZeroNetwork(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsLoopBackIp
 * PURPOSE:
 *      Is the ip a loop back IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip belong loopback network.
 *      FALSE -- ip not belong no loopback network.
 *
 * NOTES:
 *      1. It's class A subnet : 127.xx.xx.xx
 */
BOOL_T IP_LIB_IsLoopBackIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsClassBReserveIp
 * PURPOSE:
 *      Is the ip in reserved Class B IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is Class B reserved ip.
 *      FALSE -- ip not in Class B reserved ip.
 *
 * NOTES:
 *      1. It's class B subnet : 191.255.xx.xx/16
 */
BOOL_T IP_LIB_IsClassBReserveIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsClassCReservedIp
 * PURPOSE:
 *      Is the ip in reserved Class C IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is Class C reserved ip.
 *      FALSE -- ip not in Class C reserved ip.
 *
 * NOTES:
 *      1. It's class C subnet : 192.00.00.xx/24, 223.255.255.0/24
 */
BOOL_T IP_LIB_IsClassCReservedIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsTestingIp
 * PURPOSE:
 *      Is the ip is in reserved test-IP.
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a reserved test-ip.
 *      FALSE -- ip not a reserved test-ip.
 *
 * NOTES:
 *      1. It's reserved for testing : 240.00.00.00 ~ 255.255.255.254
 */
BOOL_T IP_LIB_IsTestingIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsBroadcastIp
 * PURPOSE:
 *      Is the ip is broadcast IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a broadcast IP.
 *      FALSE -- ip not a broadcast IP.
 *
 * NOTES:
 *      1. It's reserved for testing :  255.255.255.255
 */
BOOL_T IP_LIB_IsBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsValidForNetworkInterface
 * PURPOSE:
 *      Is the ip can used as network interface's IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip can be used as network interface ip.
 *      FALSE -- the ip can't be network interface ip.
 *
 * NOTES:
 *      1. This IP can't in zero network, loop back ip, multicast ip, broadcast IP.
 *         But can be a reserved ip for private network inner IP.
 */
BOOL_T IP_LIB_IsValidForNetworkInterface(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsValidForIpConfig
 * PURPOSE:
 *      Check if the ip address is valid for ip configuration.
 *
 * INPUT:
 *      ip_address  -- ip address
 *      mask        -- mask
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      IP_LIB_OK
 *      IP_LIB_INVALID_IP_LOOPBACK_IP
 *      IP_LIB_INVALID_IP_ZERO_NETWORK
 *      IP_LIB_INVALID_IP_BROADCAST_IP
 *      IP_LIB_INVALID_IP_IN_CLASS_D
 *      IP_LIB_INVALID_IP_IN_CLASS_E
 *      IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR
 *      IP_LIB_INVALID_IP_SUBNET_NETWORK_ID
 *
 * NOTES:
 *      1. This address could not in zero network, or be loopback, multicast, broadcast address.
 *      2. Check subnet network id or subnet broadcast address.
 *      3. return value is UI32_T.
 */
UI32_T IP_LIB_IsValidForIpConfig(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsValidForRemoteIp
 * PURPOSE:
 *      Check if the ip address is valid for remote server ip.
 *
 * INPUT:
 *      ip_address  -- ip address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      IP_LIB_OK
 *      IP_LIB_INVALID_IP_LOOPBACK_IP
 *      IP_LIB_INVALID_IP_ZERO_NETWORK
 *      IP_LIB_INVALID_IP_BROADCAST_IP
 *      IP_LIB_INVALID_IP_IN_CLASS_D
 *      IP_LIB_INVALID_IP_IN_CLASS_E
 *
 * NOTES:
 *      1. This address could not in zero network, or be loopback, multicast, broadcast address.
 *      2. return value is UI32_T.
 *      3. we don't check with ip/mask of DUT's IP interface (RIF).
 */
UI32_T IP_LIB_IsValidForRemoteIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsMulticastIp
 * PURPOSE:
 *      Is the ip a multicast IP ?
 *
 * INPUT:
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a multicast IP.
 *      FALSE -- ip not a multicast IP.
 *
 * NOTES:
 *      1. This IP is in 224.00.00.00 ~ 239.255.255.255.
 *
 */
BOOL_T IP_LIB_IsMulticastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsMulticastIp
 * PURPOSE:
 *      Get subnet direct broadcast IP, which host is all 1.
 *
 * INPUT:
 *      ip_address  -- the ip of subnet.
 *      ip_mask     -- mask of subnet.
 *
 * OUTPUT:
 *      bcast_ip   -- the broadcast ip of subnet.
 *
 * RETURN:
 *      IP_LIB_OK - successfully get bcast ip.
 *      IP_LIB_INVALID_ARG -- invalid bcast_ip address or subnet (ip,mask).
 *
 * NOTES:
 *      None.
 *
 */
UI32_T IP_LIB_GetSubnetBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI8_T bcast_ip[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsIpInClassA
 * PURPOSE:
 *      Check the IP is in Class A or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class A; 0~127
 *      FALSE -- no, not in Class A.
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_LIB_IsIpInClassA(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsIpInClassB
 * PURPOSE:
 *      Check the IP is in Class B or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class B; 128~191
 *      FALSE -- no, not in Class B.
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_LIB_IsIpInClassB(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsIpInClassC
 * PURPOSE:
 *      Check the IP is in Class C or not ?
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class C; 192~223
 *      FALSE -- no, not in Class C.
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_LIB_IsIpInClassC(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsIpInClassD
 * PURPOSE:
 *      Check the IP is in Class D or not ? (the multicast group)
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class D; 224~239
 *      FALSE -- no, not in Class D.
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_LIB_IsIpInClassD(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);


/* FUNCTION NAME : IP_LIB_IsIpInClassE
 * PURPOSE:
 *      Check the IP is in Class E or not ? (Testing group)
 * INPUT:
 *      ip_address  -- ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- yes, in Class E; 240~255
 *      FALSE -- no, not in Class E.
 *
 * NOTES:
 *      None.
 *
 */
BOOL_T IP_LIB_IsIpInClassE(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_CompareIp
 * PURPOSE:
 *      Compare two UI32_T ip address, and return result.
 * INPUT:
 *      ip_address_1  -- first IP address.
*       ip_address_2  -- second IP address.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      1   -- ip_address_1 great than ip_address_2
 *      0   -- ip_address_1 equal to   ip_address_2
*       -1  -- ip_address_1 less  tahn ip_address_2
 *
 * NOTES:
 *      None.
 *
 */
int IP_LIB_CompareIp(UI8_T ip_address_1[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_address_2[SYS_ADPT_IPV4_ADDR_LEN]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: IP_LIB_MaskToCidr
 * -------------------------------------------------------------------------
 * PURPOSE:  Translate the mask to cidr ( prefix_length )
 * INPUT:    mask
 * OUTPUT:   none.
 * RETURN:   cidr ( prefix_length)
 * NOTES:
 * -------------------------------------------------------------------------*/
UI32_T IP_LIB_MaskToCidr(UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME: IP_LIB_CidrToMask
 * -------------------------------------------------------------------------
 * PURPOSE:  Translate the prefix length to mask
 * INPUT:    prefix_length
 * OUTPUT:   mask.
 * RETURN:   none
 * NOTES:
 * -------------------------------------------------------------------------*/
void IP_LIB_CidrToMask(UI32_T prefix_length, UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_GetNetworkID
 * PURPOSE:
 *      Get network ID from ip and mask.
 *
 * INPUT:
 *      ip_address  -- the ip of subnet.
 *      ip_mask     -- mask of subnet.
 *
 * OUTPUT:
 *      network_id   -- the network ID of subnet.
 *
 * RETURN:
 *      IP_LIB_OK           -- successfully.
 *
 * NOTES:
 *      None.
 *
 */
UI32_T IP_LIB_GetNetworkID(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T ip_mask[SYS_ADPT_IPV4_ADDR_LEN], UI8_T network_id[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsValidNetworkMask
 * PURPOSE:
 *      Verify the IP mask is continue bit 1.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE      -- It is a valid Network mask
 *      FALSE     -- It is not a valid Network mask
 *
 * NOTES:
 *      1. true if "hole" in mask; true because x&-x always has
 *         exactly one bit set, which should be equal -x
 */
BOOL_T IP_LIB_IsValidNetworkMask(UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsSubnetBroadcastIp
 * PURPOSE:
 *      Is the ip is a broadcast IP for subnet ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a subnet broadcast IP.
 *      FALSE -- ip not a subnet broadcast IP.
 *
 * NOTES:
 *      1. It's classless checking.
 *          (ip&~mask)==(b'cast&~mask)
 */
BOOL_T IP_LIB_IsSubnetBroadcastIp(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsHostIdZero
 * PURPOSE:
 *      Is the ip is a network IP for subnet ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the ip is a network-ip, can't as interface-ip.
 *      FALSE -- ip not a network IP.
 *
 * NOTES:
 *      1. It's classless checking.
 *          (ip&~mask)==(b'cast&~mask)
 */
BOOL_T IP_LIB_IsHostIdZero(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIpMaskZero
 * PURPOSE:
 *      Is the ip&network mask is zero ?
 *
 * INPUT:
 *      subnet_mask - mask of subnet
 *      ip_address -- the ip be verified.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *
 */
BOOL_T IP_LIB_IsIpMaskZero(UI8_T ip_address[SYS_ADPT_IPV4_ADDR_LEN], UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIpBelongToSubnet
 * PURPOSE:
 *      Verify the IP is belong to the subnet.
 *
 * INPUT:
 *      subnet_ip
 *      subnet_mask_len
 *      ip_addr             -- address under verification
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE -- yes, ip_addr belong to subnet.
 *      FALSE-- no, ip_addr is another subnet.
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIpBelongToSubnet(const UI8_T* subnet_ip,
                                UI32_T subnet_mask_len,
                                UI8_T* ip_address);

/* FUNCTION NAME : IP_LIB_UI32toByteArray
 * PURPOSE:
 *      Convert IP address from UI32_T type to UI8_T[4].
 *
 * INPUT:
 *      in                  -- IP address in UI32_T format.
 *
 * OUTPUT:
 *      out                 -- IP address in UI8_T[4] format.
 *
 * RETURN:
 *      IP_LIB_OK           -- Converted successfully.
 *      IP_LIB_INVALID_ARG  -- Invalid IP address inputed.
 *
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_UI32toArray(UI32_T in, UI8_T out[SYS_ADPT_IPV4_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_ArraytoUI32
 * PURPOSE:
 *      Convert IP address from UI8_T[4] to UI32_T type in network order.
 * INPUT:
 *      byte_ip             -- IP address in UI8_T[4] format.
 * OUTPUT:
 *      ui32_ip             -- IP address in UI32_T format (network order).
 * RETURN:
 *      IP_LIB_OK           -- Converted successfully.
 *      IP_LIB_INVALID_ARG  -- Invalid IP address inputed.
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_ArraytoUI32(UI8_T byte_ip[SYS_ADPT_IPV4_ADDR_LEN], UI32_T *ui32_ip);


/* FUNCTION NAME : IP_LIB_IsIpInterfaceUp
 * PURPOSE:
 *      Is the l3 interface is up or administrative down ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the l3 interface is UP.
 *      FALSE -- the l3 interface is administrative DOWN.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsIpInterfaceUp(UI16_T flags);


/* FUNCTION NAME : IP_LIB_IsIpInterfaceRunning
 * PURPOSE:
 *      Is the l3 interface is Running ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the l3 interface is RUNNING.
 *      FALSE -- the l3 interface is DOWN.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsIpInterfaceRunning(UI16_T flags);

/* FUNCTION NAME : IP_LIB_IsIPv6UnspecifiedAddr
 * PURPOSE:
 *      Check if this ipv6 address is unspecified address (::).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6UnspecifiedAddr(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIPv6LoopbackAddr
 * PURPOSE:
 *      Check if this ipv6 address is loopback address (::1).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6LoopbackAddr(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIPv6Multicast
 * PURPOSE:
 *      Check if this ipv6 address is multicast address (FF00::/8).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6Multicast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIPv6LinkLocal
 * PURPOSE:
 *      Check if this ipv6 address is link local address (FE80::/10).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None. 
 */
BOOL_T IP_LIB_IsIPv6LinkLocal(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIPv6SolicitedMulticast
 * PURPOSE:
 *      Check if this ipv6 address is solicited multicast address (FF02::1:FF/104).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None. 
 */
BOOL_T IP_LIB_IsIPv6SolicitedMulticast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_IsIPv6LinkLocalAllNodeMulticast
 * PURPOSE:
 *      Check if this ipv6 address is link local scope all node multicast address (FF02::1).
 *
 * INPUT:
 *      addr    -- ipv6 address
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE 
 *      FALSE
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIPv6LinkLocalAllNodeMulticast(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN]);

/* FUNCTION NAME : IP_LIB_CheckIPv6PrefixForInterface
 * PURPOSE:
 *      Check if this ipv6 address is good for the interface while address configuration.
 *
 * INPUT:
 *      addr    -- ipv6 address
 *      preflen -- prefix length
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None.
 */
UI32_T IP_LIB_CheckIPv6PrefixForInterface(UI8_T addr[SYS_ADPT_IPV6_ADDR_LEN], UI32_T prefix_len);

/* FUNCTION NAME : IP_LIB_GetPrefixAddr
 * PURPOSE:
 *      Mask the addr with prefix length to get the prefix address
 *
 * INPUT:
 *      addr        -- addr
 *      addr_len    -- size is 4 or 16
 *      preflen     -- prefix length
 *
 * OUTPUT:
 *      addr_out    -- the prefix address
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      1. For IPv4/IPv6.
 *      2. The prefix length assume the same as INPUT: preflen.
 *      3. Bit copy is used if (preflen%8)
 */
BOOL_T IP_LIB_GetPrefixAddr(UI8_T addr[], UI16_T addr_len, UI32_T preflen, UI8_T addr_out[]);


/* FUNCTION NAME : IP_LIB_IsLoopbackInterface
 * PURPOSE:
 *      Is the interface is Loopback ?
 *
 * INPUT:
 *      flags
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- the interface is Loopback.
 *      FALSE -- the interface isn't Loopback.
 *
 * NOTES:
 */
BOOL_T IP_LIB_IsLoopbackInterface(UI16_T flags);


/* FUNCTION NAME : IP_LIB_ConvertLoopbackIdToIfindex
 * PURPOSE:
 *      This function will convert a loopback ID to a ifindex.
 *
 * INPUT:
 *      lo_id
 *
 * OUTPUT:
 *      ifindex
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T  IP_LIB_ConvertLoopbackIdToIfindex(UI32_T lo_id, UI32_T *ifindex);


/* FUNCTION NAME : IP_LIB_ConvertLoopbackIfindexToId
 * PURPOSE:
 *      This function will convert a ifindex to a Loopback ID.
 *
 * INPUT:
 *      ifindex
 *
 * OUTPUT:
 *      lo_id
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 */
BOOL_T  IP_LIB_ConvertLoopbackIfindexToId(UI32_T ifindex, UI32_T *lo_id);

/* FUNCTION NAME : IP_LIB_IsIpPrefixEqual
 * PURPOSE:
 *      Check where the ip prefix is equal
 *
 * INPUT:
 *      ip_addr1_p  -- ip address 1
 *      ip_addr2_p  -- ip address 2
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE   -- the prefix of ip addr 1 and ip addr 2 are equal
 *      FALSE  -- otherwise
 *
 * NOTES:
 *      None.
 */
BOOL_T IP_LIB_IsIpPrefixEqual(L_INET_AddrIp_T *ip_addr1_p, L_INET_AddrIp_T *ip_addr2_p);

/* FUNCTION NAME : IP_LIB_IsValidSocketPortForServerListen
 * PURPOSE:
 *      To verify if specified IP socket port and type is valid for
 *          specified APP ID to use as server's listening port
 *
 * INPUT:
 *      chk_skt_port - port number for IP socket to check (host order)
 *      chk_skt_type - socket type for IP socket to check
 *                                     (refer to IP_LIB_SktType_E)
 *      chk_app_id   - APP ID to check (refer to IP_LIB_ChkAppId_E)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE  -- It is a valid socket port for specified APP ID
 *      FALSE -- It is not a valid socket port for specified APP ID
 *
 * NOTES:
 *      1. checking for local server's listening port
 */
BOOL_T IP_LIB_IsValidSocketPortForServerListen(UI16_T chk_skt_port, UI16_T chk_skt_type, UI32_T chk_app_id);

#if (SYS_CPNT_IP_TUNNEL == TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_LIB_ConvertTunnelIdToIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function  convert a tid to a matching tid_ifindex existing in the IF_table.
 * INPUT    : tid       -- specify the vid input by management
 * OUTPUT   : *tid_ifindex -- returns the matching tid_ifindex in the iftable
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  IP_LIB_ConvertTunnelIdToIfindex(UI32_T tid, UI32_T *tid_ifindex);

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_LIB_ConvertTunnelIdFromIfindex
 *-----------------------------------------------------------------------------
 * PURPOSE  : This function  convert a tid from a matching tid_ifindex existing in the IF_table.
 * INPUT  : tid_ifindex -- returns the matching tid_ifindex in the iftable
 * OUTPUT    : *tid       -- specify the vid input by management
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
 BOOL_T  IP_LIB_ConvertTunnelIdFromIfindex(UI32_T tid_ifindex, UI32_T *tid);

/*
    to be discuss: originally, zone ID means vlan id, but now we have tunnel!
*/
 BOOL_T  IP_LIB_ConvertZoneIDFromTunnelIfindex(UI32_T tid_ifindex, UI32_T *zone_id);

#define IS_TUNNEL_IFINDEX(tid_ifindex)\
    ((tid_ifindex < SYS_ADPT_TUNNEL_1_IF_INDEX_NUMBER||tid_ifindex > SYS_ADPT_MAX_TUNNEL_ID+SYS_ADPT_TUNNEL_1_IF_INDEX_NUMBER-1)? FALSE : TRUE)

#define IS_TUNNEL_ID_VAILD(tid)    ((tid > SYS_ADPT_MAX_TUNNEL_ID||tid == 0)?FALSE:TRUE)
/*-----------------------------------------------------------------------------
 * FUNCTION NAME - IP_LIB_GetTunnelAddress
 *-----------------------------------------------------------------------------
 * PURPOSE  : convert dynamic tunnel address to correspoding destination address
 * INPUT  : input_addr -- support only ISATAP and 6to4
 * OUTPUT    : output_addr : tunnel destination IPv4 address
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *-----------------------------------------------------------------------------
 */
BOOL_T  IP_LIB_GetTunnelAddress(L_INET_AddrIp_T * input_addr, L_INET_AddrIp_T* output_addr );
#endif /*SYS_CPNT_IP_TUNNEL*/

#if (SYS_CPNT_VXLAN == TRUE)
#define IS_VXLAN_IFINDEX(ifindex)\
    ((ifindex < SYS_ADPT_VXLAN_FIRST_IF_INDEX_NUMBER || ifindex > SYS_ADPT_VXLAN_FIRST_IF_INDEX_NUMBER+SYS_ADPT_MAX_VXLAN_VFI_ID-SYS_ADPT_MIN_VXLAN_VFI_ID)? FALSE : TRUE)

BOOL_T IP_LIB_ConvertVfiIdToIfindex(UI32_T vfi_id, UI32_T *ifindex_p);
BOOL_T IP_LIB_ConvertVfiIdFromIfindex(UI32_T ifindex, UI32_T *vfi_id_p);
#endif

#define  IP_LIB_IS_6TO4_ADDR(addr) (addr[0] == 0x20 && addr[1]== 0x02)
#define  IP_LIB_IS_ISATAP_ADDR(addr) (addr[9] == 0 && addr[10] == 0x5E && addr[11]== 0xFE)

#endif   /* _IP_LIB_H */


