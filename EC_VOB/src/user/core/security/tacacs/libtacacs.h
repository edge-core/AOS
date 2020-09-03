/*#include "sys_bld.h"*/
#include "sys_type.h"
#include "sysfun.h"
/*#include "skt_vx.h"*/
/*#include <stdlib.h>*/
/*#include <ctype.h>*/
#include <sys/types.h>
/*#include <string.h>*/
#include "tacacs_mgr.h" /* for TACACS_Authentic_T */

#ifndef __LIBTACACS_H__
#define __LIBTACACS_H__

/*for EH *//*Mercury_V2-00030*/
#define TACACS_OM_SET_SERVER_PORT_FUNC_NO   1
#define TACACS_OM_SET_SERVER_SECRET_FUNC_NO 2
#define TACACS_MGR_SET_SERVER_IP_FUNC_NO    3
#define TACACS_MY_STRDUP_FUNC_NO        4
#define TACACS_TAC_CONNECT_FUNC_NO      5
#define TACACS_CREATE_MD5_HASH_FUNC_NO      6
#define TACACS_READ_PACKET_FUNC_NO      7
#define TACACS_OM_SET_SERVER_PORT_FUNC_NO   1
#define TACACS_OM_SET_SERVER_PORT_FUNC_NO   1




#define MD5_LEN           16

#define MSCHAP_DIGEST_LEN 49

#ifndef TAC_PLUS_PORT
#define TAC_PLUS_PORT           49
#endif

#define TAC_PLUS_READ_TIMEOUT       180 /* seconds */
#define TAC_PLUS_WRITE_TIMEOUT      180 /* seconds */
#define MAX_LEN_OF_NAS_PORT             16/*255*/
#define PORT                            "tty10"

struct session {
                  UI32_T session_id;             /* Host Byte Order, rhost specific unique session id */
               /* int aborted; */                /* have we received an abort flag? */
                  int seq_no;                    /* seq. no. of last packet exchanged */
                  time_t last_exch;              /* time of last packet exchange */
                  int sock;                      /* socket for this connection */
                  UI32_T timeout;                /* seconds for waiting to read or write packet */
               /* char *key;  */                 /* the key */
               /* int keyline; */                /* line number key was found on */
               /* char *peer;  */                /* name of connected peer */
                  char *cfgfile;                 /* config file name */
                  char *acctfile;                /* name of accounting file */
                  char port[MAX_LEN_OF_NAS_PORT+1]; /* For error reporting */
                 unsigned char version;          /* version of last packet read */
};

/*extern struct session session; */   /* the session */

struct tacacs_server {
    char *ip;
    char *key;
    int mode;
#define TAC_SERVER_MASTER       1   /* check all master servers */
#define TAC_SERVER_SLAVE        1   /* if no responce, check slave */
};
extern struct server *tac_server;

/* types of authentication */
#define TACACS_ENABLE_REQUEST  1    /* Enable Requests */
#define TACACS_ASCII_LOGIN     2    /* Inbound ASCII Login */
#define TACACS_PAP_LOGIN       3    /* Inbound PAP Login */
#define TACACS_CHAP_LOGIN      4    /* Inbound CHAP login */
#define TACACS_ARAP_LOGIN      5    /* Inbound ARAP login */
#define TACACS_PAP_OUT         6    /* Outbound PAP request */
#define TACACS_CHAP_OUT        7    /* Outbound CHAP request */
#define TACACS_ASCII_ARAP_OUT  8    /* Outbound ASCII and ARAP request */
#define TACACS_ASCII_CHPASS    9    /* ASCII change password request */
#define TACACS_PPP_CHPASS      10   /* PPP change password request */
#define TACACS_ARAP_CHPASS     11   /* ARAP change password request */
#define TACACS_MSCHAP_LOGIN    12   /* MS-CHAP inbound login */
#define TACACS_MSCHAP_OUT      13   /* MS-CHAP outbound login */

#define TAC_PLUS_AUTHEN_LOGIN      1
#define TAC_PLUS_AUTHEN_CHPASS     2
#define TAC_PLUS_AUTHEN_SENDPASS   3    /* deprecated */
#define TAC_PLUS_AUTHEN_SENDAUTH   4

/* status of reply packet, that client get from server in authen */
#define TAC_PLUS_AUTHEN_CONNECT_FAIL    -1
#define TAC_PLUS_AUTHEN_STATUS_PASS     1
#define TAC_PLUS_AUTHEN_STATUS_FAIL     2
#define TAC_PLUS_AUTHEN_STATUS_GETDATA  3
#define TAC_PLUS_AUTHEN_STATUS_GETUSER  4
#define TAC_PLUS_AUTHEN_STATUS_GETPASS  5
#define TAC_PLUS_AUTHEN_STATUS_RESTART  6
#define TAC_PLUS_AUTHEN_STATUS_ERROR    7
#define TAC_PLUS_AUTHEN_STATUS_FOLLOW   0x21

/* methods of authorization */
#define TAC_PLUS_AUTHEN_METH_NOT_SET     0  /*0x00*/
#define TAC_PLUS_AUTHEN_METH_NONE        1  /*0x01*/
#define TAC_PLUS_AUTHEN_METH_KRB5        2  /*0x03*/
#define TAC_PLUS_AUTHEN_METH_LINE        3  /*0x03*/
#define TAC_PLUS_AUTHEN_METH_ENABLE      4  /*0x04*/
#define TAC_PLUS_AUTHEN_METH_LOCAL       5  /*0x05*/
#define TAC_PLUS_AUTHEN_METH_TACACSPLUS  6  /*0x06*/   /* use this ? */
#define TAC_PLUS_AUTHEN_METH_GUEST       8  /*0x08*/
#define TAC_PLUS_AUTHEN_METH_RADIUS      16 /*0x10*/
#define TAC_PLUS_AUTHEN_METH_KRB4        17 /*0x11*/
#define TAC_PLUS_AUTHEN_METH_RCMD        32 /*0x20*/

/* priv_levels */
#define TAC_PLUS_PRIV_LVL_MAX    15 /*0x0f*/
#define TAC_PLUS_PRIV_LVL_ROOT   15 /*0x0f*/
#define TAC_PLUS_PRIV_LVL_USER   1  /*0x01*/
#define TAC_PLUS_PRIV_LVL_MIN    0  /*0x00*/

/* authen types */
#define TAC_PLUS_AUTHEN_TYPE_ASCII     1  /*0x01*/    /*  ascii  */
#define TAC_PLUS_AUTHEN_TYPE_PAP       2  /*0x02*/    /*  pap    */
#define TAC_PLUS_AUTHEN_TYPE_CHAP      3  /*0x03*/    /*  chap   */
#define TAC_PLUS_AUTHEN_TYPE_ARAP      4  /*0x04*/    /*  arap   */
#define TAC_PLUS_AUTHEN_TYPE_MSCHAP    5  /*0x05*/    /*  mschap */

/* authen services */
#define TAC_PLUS_AUTHEN_SVC_NONE       0  /*0x00*/
#define TAC_PLUS_AUTHEN_SVC_LOGIN      1  /*0x01*/
#define TAC_PLUS_AUTHEN_SVC_ENABLE     2  /*0x02*/
#define TAC_PLUS_AUTHEN_SVC_PPP        3  /*0x03*/
#define TAC_PLUS_AUTHEN_SVC_ARAP       4  /*0x04*/
#define TAC_PLUS_AUTHEN_SVC_PT         5  /*0x05*/
#define TAC_PLUS_AUTHEN_SVC_RCMD       6  /*0x06*/
#define TAC_PLUS_AUTHEN_SVC_X25        7  /*0x07*/
#define TAC_PLUS_AUTHEN_SVC_NASI       8  /*0x08*/
#define TAC_PLUS_AUTHEN_SVC_FWPROXY    9  /*0x09*/

/* authorization status */
#define TAC_PLUS_AUTHOR_STATUS_PASS_ADD  1  /*0x01*/
#define TAC_PLUS_AUTHOR_STATUS_PASS_REPL 2  /*0x02*/
#define TAC_PLUS_AUTHOR_STATUS_FAIL      16 /*0x10*/
#define TAC_PLUS_AUTHOR_STATUS_ERROR     17 /*0x11*/
#define TAC_PLUS_AUTHOR_STATUS_FOLLOW    33 /*0x21*/

/* accounting flag */
#define TAC_PLUS_ACCT_FLAG_MORE     0x1     /* deprecated */
#define TAC_PLUS_ACCT_FLAG_START    0x2
#define TAC_PLUS_ACCT_FLAG_STOP     0x4
#define TAC_PLUS_ACCT_FLAG_WATCHDOG 0x8

/* accounting status */
#define TAC_PLUS_ACCT_STATUS_SUCCESS     1   /*0x01*/
#define TAC_PLUS_ACCT_STATUS_ERROR       2   /*0x02*/
#define TAC_PLUS_ACCT_STATUS_FOLLOW     33   /*0x21*/


/* All tacacs+ packets have the same header format */
struct tac_plus_pak_hdr {
    unsigned char version;

#define TAC_PLUS_MAJOR_VER_MASK 0xf0
#define TAC_PLUS_MAJOR_VER      0xc0

#define TAC_PLUS_MINOR_VER_0    0x0
#define TAC_PLUS_VER_0  (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_0)

#define TAC_PLUS_MINOR_VER_1    0x01
#define TAC_PLUS_VER_1  (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_1)

    unsigned char type;

#define TAC_PLUS_AUTHEN         1
#define TAC_PLUS_AUTHOR         2
#define TAC_PLUS_ACCT           3

    unsigned char seq_no;       /* packet sequence number */
    unsigned char encryption;       /* packet is encrypted or cleartext */

#define TAC_PLUS_ENCRYPTED 0x0      /* packet is encrypted */
#define TAC_PLUS_CLEAR     0x1      /* packet is not encrypted */

    UI32_T session_id;      /* session identifier FIXME: Is this needed? */
    int datalength;     /* length of encrypted data following this
                 * header */
    /* datalength bytes of encrypted data */
};

typedef struct TACACS_LIB_Packet_S
{
    struct tac_plus_pak_hdr     packet_header;
    UI8_T                       packet_body[TACACS_LIB_MAX_LEN_OF_PACKET_BODY];
} TACACS_LIB_Packet_T;

#define HASH_TAB_SIZE 157        /* user and group hash table sizes */

#define TAC_PLUS_HDR_SIZE 12

typedef struct tac_plus_pak_hdr HDR;

/** TACACS client Authentication Function -- kevin **/
extern I32_T tacacs_main_ascii_login(UI32_T server_ip,
                                     UI8_T *secret,
                                     UI32_T server_port,
                                     UI32_T retransmit,
                                     UI32_T timeout,
                                     char *user,
                                     char *passwd,
                                     int auth_type,
                                     int auth_priv_lvl,
                                     TACACS_SessType_T sess_type,
                                     UI32_T sess_id,
                                     const L_INET_AddrIp_T *caller_addr);

extern I32_T tacacs_main_enable_requests(UI32_T server_ip,
                                         UI8_T *secret,
                                         UI32_T server_port,
                                         UI32_T retransmit,
                                         UI32_T timeout,
                                         char *username,
                                         char *passwd,
                                         int auth_type,
                                         int auth_priv_lvl,
                                         TACACS_SessType_T sess_type,
                                         UI32_T sess_id,
                                         const L_INET_AddrIp_T *caller_addr);

/**
    LIBRARY FUNCTIONS
**/
extern char *tac_getipfromname(char *name);

/**
    CLIENT FUNCTIONS
**/
extern BOOL_T tac_connect( UI32_T server_ip, UI8_T *key, UI32_T port,struct session *session );
extern int  tac_connect_nonblocking(UI32_T server_ip, UI8_T *key, UI32_T port, UI32_T timeout, struct session *session_s);
extern void tac_close(struct session* session);
extern int tac_authen_send_start(struct session* session,
                                 char* port,
                                 char* addr,
                                 char* username,
                                 int type,
                                 char* data,
                                 int auth_priv_lvl,
                                 UI8_T *secret);
extern int tac_authen_get_reply(struct session* session,
      char* server, char* datas, UI8_T *secret);
extern int tac_authen_send_cont(struct session* session,
      char* user_msg,  char* data, UI8_T *secret);

extern int tac_account_send_request(struct session *session,
                                    const int flag,
                                    const int method,
                                    const int priv_lvl,
                                    const int authen_type,
                                    const int authen_service,
                                    const char *user,
                                    const char *port,
                                    const char *rem_addr,
                                    char **avpair,
                                    UI8_T *secret);

extern int tac_account_get_reply(struct session *session,
                                 char *server_msg,
                                 char *data,
                                 UI8_T *secret);

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) || (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)

extern int tac_author_send_request(struct session *session,
                                   const int method,
                                   const int priv_lvl,
                                   const int authen_type,
                                   const int authen_service,
                                   const char *user,
                                   const char *port,
                                   const char *rem_addr,
                                   char **avpair,
                                   UI8_T *secret);

extern int tac_author_get_response(struct session *session,
                                   char *server_msg,
                                   char *data,
                                   char **avpair,
                                   UI8_T *secret);

#endif /* SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE || SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_ReadPacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Read data packet from TACACS+ server
 * INPUT    : session_p, secret_p
 * OUTPUT   : pkt_p
 * RETURN   : TRUE -- success / FALSE -- failure
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
/*extern unsigned char *read_packet(struct session* session ,UI8_T *pkt, UI8_T *secret);*/
BOOL_T TACACS_LIB_ReadPacket(struct session* session_p, TACACS_LIB_Packet_T *pkt_p, UI8_T *secret_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_WritePacket
 *---------------------------------------------------------------------------
 * PURPOSE  : Send a data packet to TACACS+ server
 *            pak pointer to packet data to send
 * INPUT    : session_p, pkt_p, secret_p
 * OUTPUT   : None.
 * RETURN   : TRUE -- success / FALSE -- failure
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
/*extern int write_packet(struct session* session, unsigned char *pak, UI8_T *secret);*/
BOOL_T TACACS_LIB_WritePacket(struct session *session_p, TACACS_LIB_Packet_T *pkt_p, UI8_T *secret_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FreeAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : free Attribute-Value pairs
 * INPUT    : av_pair_list_p
 * OUTPUT   : av_pair_list_p
 * RETURN   : None
 * NOTE     : the last element of av_pair_list_p MUST be NULL or
 *            the size of av_pair_list_p MUST be not large than
 *            TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *---------------------------------------------------------------------------
 */
void TACACS_LIB_FreeAvPairs(char **av_pair_list_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindAttributeInAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : search specified attribute in av_pair_list_p
 * INPUT    : search_attribute_p, av_pair_list_p
 * OUTPUT   : None
 * RETURN   : return the array index in av_pair_list_p if attribute found
 *            if return value < 0, implies the attribute not found
 * NOTE     : the last element of av_pair_list_p MUST be NULL or
 *            the size of av_pair_list_p MUST be not large than
 *            TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *---------------------------------------------------------------------------
 */
I32_T TACACS_LIB_FindAttributeInAvPairs(const char *search_attribute_p, char **av_pair_list_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindAttributeInAttributeList
 *---------------------------------------------------------------------------
 * PURPOSE  : search specified attribute in a attribute list
 * INPUT    : search_attribute_p, list_p
 * OUTPUT   : None
 * RETURN   : return the array index in list_p if attribute found
 *            if return value < 0, implies the attribute not found
 * NOTE     : the last element of list_p MUST be NULL
 *---------------------------------------------------------------------------
 */
I32_T TACACS_LIB_FindAttributeInAttributeList(const char *search_attribute_p, const char **list_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FindSeparatedCharInAvPair
 *---------------------------------------------------------------------------
 * PURPOSE  : find av-pair separated character
 * INPUT    : av_pair_p
 * OUTPUT   : None
 * RETURN   : return the address of seprated character in av_pair_p
 *            return NULL if not found
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
char* TACACS_LIB_FindSeparatedCharInAvPair(char *av_pair_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_CheckReturnedAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : check Attribute-Value pairs by attribute check list
 * INPUT    : check_list_p, av_pair_list_p
 * OUTPUT   : None
 * RETURN   : TRUE -- success / FALSE -- failure
 * NOTE     : 1, return FALSE if a mandatory attribute exist and it is not in
 *               check-list
 *            2, return FALSE if malformed pair is found
 *            3, the last element of av_pair_list_p MUST be NULL or
 *               the size of av_pair_list_p MUST be not large than
 *               TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *            4, the last element of check_list_p MUST be NULL
 *---------------------------------------------------------------------------
 */
BOOL_T TACACS_LIB_CheckReturnedAvPairs(const char **check_list_p, char **av_pair_list_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_FormatPortString
 *---------------------------------------------------------------------------
 * PURPOSE  : free Attribute-Value pairs
 * INPUT    : ifindex
 * OUTPUT   : port_str_p
 * RETURN   : None
 * NOTE     : None.
 *---------------------------------------------------------------------------
 */
void TACACS_LIB_FormatPortString(UI32_T ifindex, char *port_str_p);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TAC_UTILS_GetNasPort
 *---------------------------------------------------------------------------
 * PURPOSE  : Get NAS port description from caller type
 * INPUT    : caller_type - Caller type
 *            line_no     - (optional) Connected line number
 * OUTPUT   : nas_port    - nas port description
 * RETURN   : TRUE/Succeeded, FALSE/Failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
BOOL_T TAC_UTILS_GetNasPort(
    TACACS_SessType_T sess_type,
    UI32_T line_no,
    char *nas_port,
    UI32_T nas_port_size
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TAC_UTILS_GetRemAddr
 *---------------------------------------------------------------------------
 * PURPOSE  : Get Remote address from caller type
 * INPUT    : caller_type - Caller type
 *            caller_addr - (optional) Caller address
 * OUTPUT   : rem_addr    - remote address
 * RETURN   : TRUE/Succeeded, FALSE/Failed
 * NOTE     : None
 *---------------------------------------------------------------------------
 */
BOOL_T TAC_UTILS_GetRemAddr(
    TACACS_SessType_T sess_type,
    const L_INET_AddrIp_T *caller_addr,
    char *rem_addr,
    UI32_T rem_addr_size
);

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_ConvertAuthMethod
 *---------------------------------------------------------------------------
 * PURPOSE  : convert TACACS_Authentic_T to auth_method
 * INPUT    : auth_by_whom
 * OUTPUT   : None
 * RETURN   : auth_method
 * NOTE     : aute_method defined in TACACS+ draft could be:
 *                  TAC_PLUS_AUTHEN_METH_RADIUS;
 *                  TAC_PLUS_AUTHEN_METH_TACACSPLUS;
 *                  TAC_PLUS_AUTHEN_METH_LOCAL;
 *                  TAC_PLUS_AUTHEN_METH_NOT_SET;
 *---------------------------------------------------------------------------
 */
UI32_T TACACS_LIB_ConvertAuthMethod(TACACS_Authentic_T auth_by_whom);




/**
        Kevin Functions
**/
char * tac_print_authen_status(int status);

#if 0
/* obsoleted by L_INET_Aton() */
UI32_T my_inet_addr(UI8_T *cp);
#endif

void tac_md5_calc (UI8_T *output, UI8_T *input, unsigned int inlen);
unsigned int call_time(void);
#endif   /* __LIBTACACS_H__ */


