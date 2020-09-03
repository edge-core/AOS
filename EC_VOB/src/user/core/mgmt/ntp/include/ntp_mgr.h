/* static char SccsId[] = "+-<>?!NTP_MGR.H   22.1  22/04/02  15:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_MGR.H
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created    Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */

#ifndef _NTP_MGR_H
#define _NTP_MGR_H


/* INCLUDE FILE DECLARATIONS
 */
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include "sys_type.h"
#include "sys_adpt.h"
#include "leaf_es3626a.h"
#include "l_inet.h"
#include "sysfun.h"
#include "ntp_type.h"
#include "ntp_mgr.h"


/* The key to get keygen mgr msgq.
 */
#define NTP_MGR_IPCMSG_KEY SYS_BLD_APP_PROTOCOL_GROUP_IPCMSGQ_KEY

/* The commands for IPC message. */
enum
{
    NTP_MGR_IPC_CMD_SET_STATUS,
    NTP_MGR_IPC_CMD_GET_STATUS,
    NTP_MGR_IPC_CMD_GET_RUNN_STATUS,
    NTP_MGR_IPC_CMD_GET_LAST_UPDATE_TIME,
    NTP_MGR_IPC_CMD_GET_POLL_TIME,
    NTP_MGR_IPC_CMD_SET_SV_OPMODE,
    NTP_MGR_IPC_CMD_GET_SV_OPMODE,
    NTP_MGR_IPC_CMD_GET_RUNN_SV_OPMODE,
    NTP_MGR_IPC_CMD_ADD_SVR_IP,
    NTP_MGR_IPC_CMD_DEL_SVR_IP,
    NTP_MGR_IPC_CMD_DEL_ALL_SVR_IP,
    NTP_MGR_IPC_CMD_GET_NEXT_SVR,
    NTP_MGR_IPC_CMD_GET_LAST_UPDATE_SVR,
    NTP_MGR_IPC_CMD_FIND_SVR,
    NTP_MGR_IPC_CMD_FIND_NEXT_SVR,
    NTP_MGR_IPC_CMD_GET_AUTH_STATUS,
    NTP_MGR_IPC_CMD_SET_AUTH_STATUS,
    NTP_MGR_IPC_CMD_GET_RUNN_AUTH_STATUS,
    NTP_MGR_IPC_CMD_ADD_AUTH_KEY,
    NTP_MGR_IPC_CMD_ADD_AUTH_KEY_ENCRYPTED,
    NTP_MGR_IPC_CMD_SET_AUTH_KEY_STATUS,
    NTP_MGR_IPC_CMD_DEL_AUTH_KEY,
    NTP_MGR_IPC_CMD_DEL_ALL_AUTH_KEY,
    NTP_MGR_IPC_CMD_GET_NEXT_KEY,
    NTP_MGR_IPC_CMD_FIND_KEY
};

/* MACRO DEFINITIONS
 */
/*-------------------------------------------------------------------------
 * MACRO NAME - NTP_MGR_GET_MSGBUFSIZE
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of NTP message that contains the specified
 *           type of data.
 * INPUT   : type_name - the type name of data for the message.
 * OUTPUT  : None
 * RETURN  : The size of NTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NTP_MGR_GET_MSGBUFSIZE(type_name) \
        ((uintptr_t) & ((NTP_MGR_IPCMsg_T *)0)->data + sizeof(type_name))

/*-------------------------------------------------------------------------
 * MACRO NAME - NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the size of NTP message that has no data block.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The size of NTP message.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NTP_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA()    sizeof(NTP_MGR_IPCMsg_Type_T)

/*-------------------------------------------------------------------------
 * MACRO NAME - NTP_MGR_MSG_CMD
 *              NTP_MGR_MSG_RETVAL
 *-------------------------------------------------------------------------
 * PURPOSE : Get the NTP command/return value of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The NTP command/return value; it's allowed to be used as lvalue.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NTP_MGR_MSG_CMD(msg_p)    (((NTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.cmd)
#define NTP_MGR_MSG_RETVAL(msg_p) (((NTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->type.result)

/*-------------------------------------------------------------------------
 * MACRO NAME - NTP_MGR_MSG_DATA
 *-------------------------------------------------------------------------
 * PURPOSE : Get the data block of an IPC message.
 * INPUT   : msg_p - the IPC message.
 * OUTPUT  : None
 * RETURN  : The pointer of the data block.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
#define NTP_MGR_MSG_DATA(msg_p)   ((void *)&((NTP_MGR_IPCMsg_T *)(msg_p)->msg_buf)->data)


/* defines */
/* Begin for ntp macro - QingfengZhang, 22 February, 2005 11:16:17 */
/*
 * NTP uses two fixed point formats.  The first (l_fp) is the "long"
 * format and is 64 bits long with the decimal between bits 31 and 32.
 * This is used for time stamps in the NTP packet header (in network
 * byte order) and for internal computations of offsets (in local host
 * byte order). We use the same structure for both signed and unsigned
 * values, which is a big hack but saves rewriting all the operators
 * twice. Just to confuse this, we also sometimes just carry the
 * fractional part in calculations, in both signed and unsigned forms.
 * Anyway, an l_fp looks like:
 *
 *    0           1           2           3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                 Integral Part                 |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |                 Fractional Part               |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */
typedef struct L_FP_FORMAT{
    union {
        UI32_T Xl_ui;
        I32_T Xl_i;
    } Ul_i;
    union {
        UI32_T Xl_uf;
        I32_T Xl_f;
    } Ul_f;
}    l_fp;

/*
 * Clock filter algorithm tuning parameters
 */
#define MINDISPERSE .01 /* min dispersion */
#define MAXDISPERSE 16. /* max dispersion */
#define NTP_SHIFT   8   /* clock filter stages */
#define NTP_FWEIGHT .5  /* clock filter weight */

#define NTP_MGR_PASS_TAG_YES 7
#define NTP_MGR_PASS_TAG_NO 0


/* DATA TYPE DECLARATIONS
 */   /* Add struct - QingfengZhang, 22 February, 2005 10:59:01 */

/*
 * NTP packet format.  The mac field is optional.  It isn't really
 * an l_fp either, but for now declaring it that way is convenient.
 * See Appendix A in the specification.
 *
 * Note that all u_fp and l_fp values arrive in network byte order
 * and must be converted (except the mac, which isn't, really).
 */
typedef struct NTP_PKT {
    unsigned char   li_vn_mode; /* leap indicator, version and mode */
    unsigned char   stratum;    /* peer stratum */
    unsigned char   ppoll;      /* peer poll interval */
    I8_T    precision;  /* peer clock precision */
    UI32_T   rootdelay; /* distance to primary clock */
    UI32_T   rootdispersion;    /* clock dispersion */
    UI32_T   refid;     /* reference clock ID */
    l_fp    reftime;    /* time peer clock was last updated */
    l_fp    org;        /* originate time stamp */
    l_fp    rec;        /* receive time stamp */
    l_fp    xmt;        /* transmit time stamp */

#define LEN_PKT_NOMAC   12 * sizeof(UI32_T) /* min header length */
#define LEN_PKT_MAC LEN_PKT_NOMAC +  sizeof(UI32_T)
#define MIN_MAC_LEN 3 * sizeof(UI32_T)  /* DES */
#define MAX_MAC_LEN 5 * sizeof(UI32_T)  /* MD5 */
#ifdef OPENSSL
        u_int32 exten[NTP_MAXEXTEN / 4]; /* max extension field */
#else /* OPENSSL */
        UI32_T exten[1];   /* misused */
#endif /* OPENSSL */
    unsigned char   mac[MAX_MAC_LEN]; /* mac */
}__attribute__((__packed__)) NTP_PACKET;

typedef struct NTP_MGR_SERVER_S
{
    struct NTP_MGR_SERVER_S  *next_server;  /* next server in build list */
    struct sockaddr_in  srcadr; /* address of remote host */
    unsigned char  version;         /* version to use */
    unsigned char  leap;            /* leap indicator */
    unsigned char  stratum;         /* stratum of remote server */
    I8_T    precision;      /* server's clock precision */
    unsigned char  trust;           /* trustability of the filtered data */
    UI32_T  rootdelay;          /* distance from primary clock */
    UI32_T  rootdispersion;     /* peer clock dispersion */
    UI32_T  refid;          /* peer reference ID */
    l_fp  reftime;          /* time of peer's last update */
    unsigned long  event_time;      /* time for next timeout */
    unsigned long  last_xmit;       /* time of last transmit */
    unsigned short  xmtcnt;         /* number of packets transmitted */
    unsigned short  rcvcnt;         /* number of packets received */
    unsigned char  reach;           /* reachability, NTP_WINDOW bits */
    unsigned short  filter_nextpt;      /* index into filter shift register */
    I32_T  filter_delay[NTP_SHIFT]; /* delay part of shift register */
    l_fp  filter_offset[NTP_SHIFT]; /* offset part of shift register */
    I32_T  filter_soffset[NTP_SHIFT]; /* offset in s_fp format, for disp */
    UI32_T  filter_error[NTP_SHIFT];    /* error part of shift register */
    l_fp  org;          /* peer's originate time stamp */
    l_fp  xmt;          /* transmit time stamp */
    UI32_T  delay;          /* filter estimated delay */
    UI32_T  dispersion;     /* filter estimated dispersion */
    l_fp  offset;           /* filter estimated clock offset */
    I32_T  soffset;         /* fp version of above */
    UI32_T keyid;           /*key id of when config server*/
    unsigned char ntpServerStatus; /*for snmp express server status*/

    /* when the value is TRUE, should send request packet to server.
     */
    BOOL_T send_request;
}   NTP_MGR_SERVER_T;

/* This struct is used for cli_runcfg.c
 */
typedef struct NTP_RUNCFG_SERVER_S
{
    struct NTP_RUNCFG_SERVER_S  *next_server;  /* next server in build list */
    UI32_T ipadr; /* address of remote host */
    unsigned char  version;         /* version to use */
    UI32_T keyid;           /*key id of when config server*/
}   NTP_RUNCFG_SERVER_T;


/* Structure to store keys in in the hash table.*/
typedef struct NTP_MGR_AUTHKEY_S {
    UI32_T keyid;       /* key identifier */
    union {
        long bogon;     /* Make sure nonempty */
        char  MD5_key[MAXSIZE_ntpAuthKeyWord+1];  /* MD5 key */
    }__attribute__((__packed__))k;
    char         password[MAXSIZE_ntpAuthKeyWord*3 + 4];
    unsigned long flags;       /* flags that wave */
    unsigned long lifetime; /* remaining lifetime */
    UI32_T        keylen;      /* key length */
    unsigned long ntpAuthKeyStatus;
/*  unsigned long trustflag;*/
    struct NTP_MGR_AUTHKEY_S *next;
}__attribute__((__packed__))NTP_MGR_AUTHKEY_T;

/* End - QingfengZhang, 22 February, 2005 11:18:29 */




#define NTP_VERSION 3  /*current version number*/
#define NTP_OLDVERSION 1 /*the min version number*/

#define NTP_PORT 123
#define MAX_AF  2  /*The max number of sockets we can open*/

#define KEY_TRUSTED 0x001 /*this key is trusted*/
#define KEY_MD5     0x200 /*this is a MD5 key type*/

/*
 * The hash table. This is indexed by the low order bits of the
 * keyid. We make this fairly big for potentially busy servers.
 */
#define HASHSIZE    64
#define HASHMASK    ((HASHSIZE)-1)
#define KEYHASH(keyid)  ((keyid) & HASHMASK)


/*
 * Selection algorithm tuning parameters
 */
#define NTP_MINCLOCK    4   /* minimum survivors */
#define NTP_MAXCLOCK    3  /* maximum candidates */
#define NTP_MAXAUTHKEY  255  /* maximum authentication keys */
#define MAXDISTANCE 1.  /* max root distance */
#define CLOCK_SGATE 3.  /* popcorn spike gate */
#define HUFFPUFF    900 /* huff-n'-puff sample interval (s) */
#define ADJUSTDELAY 1 /*adjust the delay time*/
#define HYST        .5  /* anti-clockhop hysteresis */
#define HYST_TC     .875    /* anti-clockhop hysteresis decay */
#define MAX_TTL     8   /* max ttl mapping vector size */
#define NTP_MAXEXTEN    1024    /* maximum extension field size */

/* Values for mode */
#define MODE_UNSPEC 0   /* unspecified (old version) */
#define MODE_ACTIVE 1   /* symmetric active */
#define MODE_PASSIVE    2   /* symmetric passive */
#define MODE_CLIENT 3   /* client mode */
#define MODE_SERVER 4   /* server mode */
#define MODE_BROADCAST  5   /* broadcast mode */
#define MODE_CONTROL    6   /* control mode packet */
#define MODE_PRIVATE    7   /* implementation defined function */
#define MODE_BCLIENT    8   /* broadcast client mode */


/* Values for peer.stratum, sys_stratum */
#define STRATUM_REFCLOCK ((unsigned char)0) /* default stratum */
/* A stratum of 0 in the packet is mapped to 16 internally */
#define STRATUM_PKT_UNSPEC ((unsigned char)0) /* unspecified in packet */
#define STRATUM_UNSPEC  ((unsigned char)16) /* unspecified */

/* Some defaults */
#define DEFTIMEOUT  5       /* 5 timer increments */
#define DEFSAMPLES  4       /* get 4 samples per server */
#define DEFPRECISION    (-5)        /* the precision we claim */
#define DEFMAXPERIOD    60      /* maximum time to wait */
#define DEFMINSERVERS   3       /* minimum responding servers */
#define DEFMINVALID 1       /* mimimum servers with valid time */

/*
 * Values for peer.leap, sys_leap
 */
#define LEAP_NOWARNING  0x0 /* normal, no leap second warning */
#define LEAP_ADDSECOND  0x1 /* last minute of day has 61 seconds */
#define LEAP_DELSECOND  0x2 /* last minute of day has 59 seconds */
#define LEAP_NOTINSYNC  0x3 /* overload, clock is free running */


/*
 * Poll interval parameters
 */
#define NTP_UNREACH 16  /* poll interval backoff count */
#define NTP_MINPOLL 4   /* log2 min poll interval (16 s) */
#define NTP_MINDPOLL    6   /* log2 default min poll (64 s) */
#define NTP_MAXDPOLL    10  /* log2 default max poll (~17 m) */
#define NTP_MAXPOLL 17  /* log2 max poll interval (~36 h) */
#define NTP_BURST   8   /* packets in burst */
#define BURST_DELAY 2   /* interburst delay (s) */
#define RESP_DELAY  1   /* crypto response delay (s) */


/*
 * Since ntpdate isn't aware of some of the things that normally get
 * put in an NTP packet, we fix some values.
 */
#define NTPDATE_PRECISION   (-6)        /* use this precision */
#define NTPDATE_DISTANCE    FP_SECOND   /* distance is 1 sec */
#define NTPDATE_DISP        FP_SECOND   /* so is the dispersion */
#define NTPDATE_REFID       (0)     /* reference ID to use */
#define PEER_MAXDISP    (64*FP_SECOND)  /* maximum dispersion (fp 64) */

#define FP_SECOND   (0x10000)  /* 1 second*/

/* end - QingfengZhang, 22 February, 2005 11:16:21 */


#define NTP_WAIT_TIMEOUT           3     /* Seconds before resending       */
#define NTP_MAX_SERVER             3
#define NTP_1900_TO_1970_SECS    0x83aa7e80     /* 1970 - 1900 in seconds */
#define NTP_1900_TO_1990_SECS       0xa9491c00  /* Seconds between 1900 and 1990*/
#define NTP_1900_TO_2001_SECS        0xBDFA4700     /* Seconds between 1900 and 2001*/

#define NtpAuthkeyTrusted_enabled   1L
#define NtpAuthkeyTrusted_disabled  2L


 /*Define the default value of NTP
  */
 #define NTP_MAX_POLLTIME        MAX_ntpPollInterval          /* Max polling time */
 #define NTP_MIN_POLLTIME        MIN_ntpPollInterval        /* Min polling time */
#define NTP_DEFAULT_OPERATIONMODE    VAL_ntpServiceMode_unicast        /* unicast mode */
 #define NTP_DEFAULT_STATUS      VAL_ntpStatus_disabled
 #define NTP_DEFAULT_AUTHSTATUS           VAL_ntpAuthenticateStatus_disabled


#define NTP_MAXSKW  0x28f   /* 0.01 sec in fp format */
#define NTP_MINDIST 0x51f   /* 0.02 sec in fp format */
#define PEER_MAXDISP    (64*FP_SECOND)  /* maximum dispersion (fp 64) */
#define NTP_INFIN   15  /* max stratum, infinity a la Bellman-Ford */
#define NTP_MAXWGT  (8*FP_SECOND)   /* maximum select weight 8 seconds */
#define NTP_MAXLIST 5   /* maximum select list size */
#define PEER_SHIFT  8   /* 8 suitable for crystal time base */
#define NTP_MAXAGE  86400   /* one day in seconds */

/* TYPE DECLARATIONS
 */
typedef enum
{
  NTP_TYPE_SYSTEM_STATE_TRANSITION = SYS_TYPE_STACKING_TRANSITION_MODE,
  NTP_TYPE_SYSTEM_STATE_MASTER    = SYS_TYPE_STACKING_MASTER_MODE,
  NTP_TYPE_SYSTEM_STATE_SLAVE     = SYS_TYPE_STACKING_SLAVE_MODE,
} NTP_TYPE_SYSTEM_STATE_T;

/* EXPORTED SUBPROGRAM BODIES
 */
    /* TYPE DECLARATIONS
     */
    typedef struct
    {
        UI32_T polling_inteval;
        NTP_OPERATION_MODE_E config_mode;
        NTP_SERVICE_E   service_status ;    /* show if NTP ON or OFF */
        NTP_AUTHENTICATE_E authenticate_status;  /* NTP authenticate enable or not */
        NTP_MGR_SERVER_T *server_entry;    /*ntp server list*/
        UI32_T num_servers;    /*number of servers to poll*/
        NTP_MGR_AUTHKEY_T *authKey_entry[HASHSIZE];  /*ntp authKey hash table*/
        UI32_T num_authkeys;   /*number of authentication keys*/
        UI32_T ntp_version;    /*system current version*/
        UI32_T sync_method; /* reserved */

    }__attribute__((__packed__)) NTP_OM_MIB_ENTRY_T;

/* Message declarations for IPC.
 */
typedef union
{
    UI32_T  cmd;    /* for sending IPC request. CSCA_MGR_IPC_CMD1 ... */
    UI32_T  result; /* for response */
} NTP_MGR_IPCMsg_Type_T;

typedef struct
{
    UI32_T  status;
} NTP_MGR_IPCMsg_Status_T;

typedef struct
{
    I32_T  time;
} NTP_MGR_IPCMsg_LastUpdateTime_T;

typedef struct
{
    UI32_T  poll_time;
} NTP_MGR_IPCMsg_PollTime_T;

typedef struct
{
    UI32_T  serv_mode;
} NTP_MGR_IPCMsg_ServMode_T;

typedef struct
{
    UI32_T  ip_addr;
} NTP_MGR_IPCMsg_IpAddr_T;

typedef struct
{
    UI32_T  ip_addr;
    UI32_T  version;
    UI32_T  keyid;
} NTP_MGR_IPCMsg_Server_T;

typedef struct
{
    NTP_MGR_SERVER_T  server_entry;
} NTP_MGR_IPCMsg_Server_Entry_T;

typedef struct
{
    UI32_T            ip_addr;
    NTP_MGR_SERVER_T  server_entry;
} NTP_MGR_IPCMsg_Find_Server_Entry_T;

typedef struct
{
    UI32_T  status;
} NTP_MGR_IPCMsg_Auth_Status_T;

typedef struct
{
    UI32_T  keyid;
    char    auth_key[MAXSIZE_ntpAuthKeyWord+1];
} NTP_MGR_IPCMsg_Auth_Key_T;

typedef struct
{
    UI32_T  keyid;
    char    auth_key_encrypted[MAXSIZE_ntpAuthKeyWord*3 + 4];
} NTP_MGR_IPCMsg_Auth_Key_Encrypted_T;

typedef struct
{
    UI32_T  keyid;
    UI32_T  status;
} NTP_MGR_IPCMsg_Auth_Key_Status_T;

typedef struct
{
    NTP_MGR_AUTHKEY_T    auth_entry;
} NTP_MGR_IPCMsg_Auth_Key_Entry_T;

typedef struct
{
    UI32_T  keyid;
    NTP_MGR_AUTHKEY_T    auth_entry;
} NTP_MGR_IPCMsg_Find_Key_Entry_T;

typedef struct
{
    UI32_T  index;
} NTP_MGR_IPCMsg_SvrIndex_T;

typedef union
{
    NTP_MGR_IPCMsg_Status_T             stats;
    NTP_MGR_IPCMsg_PollTime_T           ptime;
    NTP_MGR_IPCMsg_LastUpdateTime_T     lastuptime;
    NTP_MGR_IPCMsg_ServMode_T           srmod;
    NTP_MGR_IPCMsg_SvrIndex_T           svrid;
    NTP_MGR_IPCMsg_IpAddr_T             ipadr;
    NTP_MGR_IPCMsg_Server_T             server;
    NTP_MGR_IPCMsg_Server_Entry_T       svrentry;
    NTP_MGR_IPCMsg_Find_Server_Entry_T  findsvrentry;
    NTP_MGR_IPCMsg_Auth_Status_T        authstatus;
    NTP_MGR_IPCMsg_Auth_Key_Entry_T     authentry;
} NTP_MGR_IPCMsg_Data_T;

typedef struct
{
    NTP_MGR_IPCMsg_Type_T  type;
    NTP_MGR_IPCMsg_Data_T  data;
} NTP_MGR_IPCMsg_T;

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize NTP_MGR used system resource, eg. protection semaphore.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_Init(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will make the NTP_MGR enter the slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 void NTP_MGR_EnterSlaveMode(void);
/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_SetTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
void NTP_MGR_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_GetOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return the NTP_MGR  mode. (slave/master/transition)
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T NTP_MGR_GetOperationMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME -  NTP_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
* PURPOSE  : Handle the ipc request message for ntp mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpStatus_enabled, 2 :VAL_ntpStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T NTP_MGR_SetStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpStatus_enabled, 2: VAL_ntpStatus_disabled
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetStatus(UI32_T *status);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpStatus_enabled/VAL_ntpStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:     default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
 SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningStatus(UI32_T *stuats);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Set NTP authenticate status : enable/disable
 * INPUT    : status you want to set. 1 :VAL_ntpAuthenticateStatus_enabled, 2 :VAL_ntpAuthenticateStatus_disabled
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T NTP_MGR_SetAuthStatus(UI32_T status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetAuthStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get NTP authenticate status  : enable/disable
 * INPUT    : none
 * OUTPUT   : current NTP status.1 :VAL_ntpAuthenticateStatus_enabled, 2: VAL_ntpAuthenticateStatus_disabled
 * RETURN   : TRUE : If success
 *            FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetAuthStatus(UI32_T *status);


/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningAuthStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the authenticate status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   status -- VAL_ntpAuthenticateStatus_enabled/VAL_ntpAuthenticateStatus_disabled
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:   default value is disable  1:enable, 2:disable
 *---------------------------------------------------------------------------*/
 SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningAuthStatus(UI32_T *stuats);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , unicast/brocast/anycast/disable
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 *
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_SetServiceOperationMode(UI32_T mode);
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetServiceOperationMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Set ntp service operation mode , Unicast/brocast/anycast
 * INPUT    : VAL_ntpServiceMode_unicast = 1
 *            VAL_ntpServiceMode_broadcast = 2
 *            VAL_ntpServiceMode_anycast = 3
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetServiceOperationMode(UI32_T *mode);
/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningServiceMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the operation mode mapping of system
 * INPUT:    None
 * OUTPUT:   VAL_ntpServiceMode_unicast = 1
 *           VAL_ntpServiceMode_broadcast = 2
 *           VAL_ntpServiceMode_anycast = 3
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE: default value is unicast
 *---------------------------------------------------------------------------*/
 SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningServiceMode(UI32_T *servicemode);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Set the polling interval
 * INPUT    : polling interval used in unicast mode
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_SetPollTime(UI32_T polltime);
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetPollTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get the polling interval
 * INPUT    : A buffer is used to be put data
 * OUTPUT   : polling interval used in unicast mode
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_GetPollTime(UI32_T *polltime);
 /*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_MGR_GetRunningPollTime
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the status of disable/enable mapping of system
 * INPUT:    None
 * OUTPUT:   polling time
 * RETURN:   SYS_TYPE_GET_RUNNING_CFG_FAIL -- error (system is not in MASTER mode)
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 *           SYS_TYPE_GET_RUNNING_CFG_SUCCESS -- different from default value
 * NOTE:  default value is 16 secconds
 *---------------------------------------------------------------------------*/
 SYS_TYPE_Get_Running_Cfg_T  NTP_MGR_GetRunningPollTime(UI32_T *polltime);
 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_AddServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Add a ntp server ip to OM
 * INPUT    : 1. ip address 2. ntp version 3. key id
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_AddServerIp(UI32_T ipaddress,UI32_T version,UI32_T keyid);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete a server ip from OM
 * INPUT    : ipaddress
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : 1.Delete a designated server.
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_DeleteServerIp(UI32_T ipaddress);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of last update-time
 * INPUT    : buffer pointer stored time information
 * OUTPUT   : time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *        FALSE: NTP never get time from server.
 * NOTES    :
 *------------------------------------------------------------------------------*/
 BOOL_T NTP_MGR_GetLastUpdateUTCTime(UI32_T *time);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetCurrentUTCTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Get time information of GMT
 * INPUT    : Buffer of  UTC time
 * OUTPUT   : 1.time in seconds from 2001/01/01 00:00:00
 * RETURN   : TRUE : If success
 *        FALSE: If failed
 * NOTES    :
 *------------------------------------------------------------------------------*/
 BOOL_T NTP_MGR_GetCurrentUTCTime(UI32_T *time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_InTimeServiceMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Perform time serice mode,e.g, unicast, broadcast mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : Called by NTP_TASK
 *------------------------------------------------------------------------------*/
 void NTP_MGR_InTimeServiceMode(void);


/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_DeleteAllServerIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Delete all servers ip from OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_DeleteAllServerIp(void);

 /*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_MGR_FindServer
  *------------------------------------------------------------------------------
  * PURPOSE  : Get a server entry  from OM using ip address as index
  * INPUT    : ip address
  * OUTPUT   : server find
  * RETURN   : TRUE : If find
  *            FALSE: If not found
  * NOTES    : none
  *------------------------------------------------------------------------------*/
  BOOL_T  NTP_MGR_FindServer(UI32_T ipaddress,NTP_MGR_SERVER_T *server);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_AddAuthKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Add an authentication key to the list
 * INPUT    : 1.index 2. md5_key
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    : none
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_AddAuthKey(UI32_T index, char *md5_key);


 /*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_MGR_DeleteAllAuthKey
  *------------------------------------------------------------------------------
  * PURPOSE  : Delete all Authenticaion key
  * INPUT    : none
  * OUTPUT   : none
  * RETURN   : TRUE : If success
  *            FALSE:
  * NOTES    :
  *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_DeleteAllAuthKey(void);


 /*------------------------------------------------------------------------------
  * FUNCTION NAME - NTP_MGR_DeleteAuthKey
  *------------------------------------------------------------------------------
  * PURPOSE  : Delete a designed Authentication key
  * INPUT    : keyid
  * OUTPUT   : none
  * RETURN   : TRUE : If success
  *            FALSE:
  * NOTES    :
  *------------------------------------------------------------------------------*/
  BOOL_T  NTP_MGR_DeleteAuthKey(UI32_T keyid);

 /*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_SetTrustedKey
 *------------------------------------------------------------------------------
 * PURPOSE  : Set an authentication key to be a trusted one
 * INPUT    : keyid
              status
 * OUTPUT   : none
 * RETURN   : TRUE : If success
 *        FALSE:
 * NOTES    :
 *------------------------------------------------------------------------------*/
 BOOL_T  NTP_MGR_SetTrustedKey(UI32_T keyid,UI32_T status);

/*------------------------------------------------------------------------------
   * FUNCTION NAME - NTP_MGR_FindKey
   *------------------------------------------------------------------------------
   * PURPOSE  : check whether the key exist
   * INPUT    : keyid
   * OUTPUT   :
   * RETURN   : TRUE : If find
   *            FALSE: If not found
   * NOTES    : none
   *------------------------------------------------------------------------------*/

BOOL_T  NTP_MGR_FindKey(UI32_T keyid, NTP_MGR_AUTHKEY_T *authkey);
;

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetNextServer(UI32_T *ipadd, UI32_T *ver,UI32_T *key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update server for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetLastUpdateServer(NTP_MGR_SERVER_T *serv);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_GetLastUpdateTime
 *------------------------------------------------------------------------------
 * PURPOSE  : Return the last update time for show ntp
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_GetLastUpdateTime(I32_T *time);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_AddAuthKey_Encrypted
*------------------------------------------------------------------------------
* PURPOSE  : Add an authentication encrypted key to the list ,use in provison
* INPUT    : 1.index 2. encryptedkey
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_AddAuthKey_Encrypted(UI32_T index, char *encryptedkey);

/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_FindKey
*------------------------------------------------------------------------------
* PURPOSE  : check whether the key exist
* INPUT    : keyid
* OUTPUT   :
* RETURN   : TRUE : If find
*            FALSE: If not found
* NOTES    : none
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_GetNextKey(NTP_MGR_AUTHKEY_T *authkey);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_FindNextServer
 *------------------------------------------------------------------------------
 * PURPOSE  : Get a next server entry  from OM using ip address as index
 * INPUT    : 1.point to the server list.
 * OUTPUT   : buffer contain information
 * RETURN   : TRUE : If success
 *            FALSE: If get failed, or the assigned ip was cleared.
 * NOTES    : none
 *------------------------------------------------------------------------------*/
BOOL_T NTP_MGR_FindNextServer(UI32_T ipadd, NTP_MGR_SERVER_T *serv);



BOOL_T NTP_MGR_Encode(int flag,char *buf,char *encode_passwd);

BOOL_T NTP_MGR_Decode(char *buf,char *passwd);
/*------------------------------------------------------------------------------
* FUNCTION NAME - NTP_MGR_SetAuthKeyStatus
*------------------------------------------------------------------------------
* PURPOSE  : Set an authentication key status
* INPUT    : 1.index 2. status : VAL_ntpAuthKeyStatus_valid/VAL_ntpAuthKeyStatus_invalid
* OUTPUT   : none
* RETURN   : TRUE : If success
*            FALSE:
* NOTES    : only used for snmp do set authentication key status
*------------------------------------------------------------------------------*/
BOOL_T  NTP_MGR_SetAuthKeyStatus(UI32_T index, UI32_T status);
/*Follow three routine is used for backdoor*/
void NTP_MGR_Show_Delay();
void NTP_MGR_Debug_Enable();
void NTP_MGR_Debug_Disable();

/* Digests a string and prints the result. Add for NTP - QingfengZhang, 06 April, 2005 2:19:08 */
/*  FUNCTION NAME : L_MD5_MDStringAndPxt
 *  PURPOSE:
 *      Encode a string and packet by MD5.
 *  INPUT:
 *      string       -- input string for digest
 *      stringlen    -- input string length
 *      pkt          -- input packet for digest
 *      len          -- input packet length
 *  OUTPUT:
 *      digest -- output digest string.
 *
 *  RETURN:
 *      None
 *
 *  NOTES:
 *      This routine is used in NTP.
 */
void L_MD5_MDStringAndPxt (UI8_T *digest, UI8_T *string,UI32_T stringlen,UI32_T *pkt,UI32_T len);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_Alarming
 *------------------------------------------------------------------------------
 * PURPOSE  : when interrutp occur , set the flag
 * INPUT    :
 * OUTPUT   :
 * RETURN   :
 * NOTES    : every 20 ticks ,the function will be called.
 *------------------------------------------------------------------------------*/
void NTP_MGR_Alarming();

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_RifUp_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : RIF up callback handler.
 * INPUT    : ifindex   -- The ifindex of active rif.
 *            addr_p    -- The IP address of active rif.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_RifUp_Callback(UI32_T ifindex, L_INET_AddrIp_T *addr_p);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_LPortEnterForwarding_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : Lport enters forwarding callback handler.
 * INPUT    : xstid -- Index of the spanning tree.
 *            lport -- Logical port number.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_LPortEnterForwarding_Callback(UI32_T xstp_id, UI32_T lport);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - NTP_MGR_LPortLeaveForwarding_Callback
 *------------------------------------------------------------------------------
 * PURPOSE  : Lport leaves forwarding callback handler.
 * INPUT    : xstid -- Index of the spanning tree.
 *            lport -- Logical port number.
 * OUTPUT   : None.
 * RETURN   : None.
 * NOTES    : None.
 *------------------------------------------------------------------------------
 */
void NTP_MGR_LPortLeaveForwarding_Callback(UI32_T xstp_id, UI32_T lport);

#endif /* NTP_MGR_H */
