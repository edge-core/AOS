/*
 * authorization
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION == TRUE) \
 || (SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD == SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION)

#include <string.h>
#include "tacacs_type.h"
#include "libtacacs.h"
#include "l_stdlib.h"
/* #include "socket.h" */
#include "tacacs_type.h"
#include "sys_module.h"
#include "l_mm.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* define naming constant TAC_AUTHOR_DEBUG_MODE
 * to build tac_packet.c with DEBUG version
 * And let following macros print debug messages
 */

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef TAC_AUTHOR_DEBUG_MODE

    #define TAC_AUTHOR_TRACE(msg)                       (printf(msg))
    #define TAC_AUTHOR_TRACE1(msg, arg)                 (printf(msg, arg))
    #define TAC_AUTHOR_TRACE2(msg, arg1, arg2)          (printf(msg, arg1, arg2))
    #define TAC_AUTHOR_TRACE3(msg, arg1, arg2, arg3)    (printf(msg, arg1, arg2, arg3))

#else

    #define TAC_AUTHOR_TRACE(msg)                       ((void)0)
    #define TAC_AUTHOR_TRACE1(msg, arg)                 ((void)0)
    #define TAC_AUTHOR_TRACE2(msg, arg1, arg2)          ((void)0)
    #define TAC_AUTHOR_TRACE3(msg, arg1, arg2, arg3)    ((void)0)

#endif /* TAC_AUTHOR_DEBUG_MODE */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T TACACS_LIB_CreateAvPairs(
                    UI32_T arg_cnt,
                    const UI8_T *arg_len_list,
                    const UI8_T *arg_buf,
                    char **av_pair_list_p);


/* EXPORTED SUBPROGRAM BODIES
 */

/*************************************************
       The AV-pairs list depends from Cisco IOS version
char *avpair[]=
{
   "service=(*)slip|ppp|arap|shell|tty-daemon|
          connection|system|firewall|multilink|...",
     this attribute MUST always be included !

   "protocol=(*)lcp|ip|ipx|atalk|vines|lat|xremote|
           tn3270|telnet|rlogin|pad|vpdn|ftp|
           http|deccp|osicp|unknown|multilink",
   "cmd=(*)command, if service=shell",
This attribute MUST be specified if service equals "shell".
A NULL value (cmd=NULL) indicates that the shell itself is being referred to.

   "cmd-arg=(*)argument to command",
Multiple cmd-arg attributes may be specified

   "acl=(*)access list, if service=shell ?cmd=NULL",
Used only when service=shell and cmd=NULL

   "inacl=(*)input access list",
   "outacl=(*)output access list",
   "zonelist=(*)numeric zonelist value to AppleTalk only",
   "addr=(*)network address",
   "addr-pool=(*)address pool",
   "routing=(*)true|false, routing propagated",
   "route=(*)<dst_address> <mask> [<routing_addr>]",
MUST be of the form "<dst_address> <mask> [<routing_addr>]"

   "timeout=(*)timer for the connection (in minutes)",
zero - no timeout

   "idletime=(*)idle-timeout (in minutes)",
   "autocmd=(*)auto-command, service=shell and cmd=NULL",
   "noescape=(*)true|false, deny using symbol escape",
service=shell and cmd=NULL

   "nohangup=(*)true|false, Do no disconnect after autocmd",
service=shell and cmd=NULL

   "priv_lvl=(*)privilege level",
   "remote_user=(*)remote userid, for AUTHEN_METH_RCMD",
   "remote_host=(*)remote host, for AUTHEN_METH_RCMD",
   "callback-dialstring=(*)NULL, or a dialstring",
   "callback-line=(*)line number to use for a callback",
   "callback-rotary=(*)rotary number to use for a callback",
   "nocallback-verify=(*)not require authen after callback"

     ...

   This list can increase for new versions of Cisco IOS

   NULL - end of array

   = - mandatory argument
   * - optional argument

   maximum length of 1 AV-pair is 255 chars
};
*/

/*
 methods:
TAC_PLUS_AUTHEN_METH_NOT_SET    := 0x00
TAC_PLUS_AUTHEN_METH_NONE       := 0x01
TAC_PLUS_AUTHEN_METH_KRB5       := 0x02
TAC_PLUS_AUTHEN_METH_LINE       := 0x03
TAC_PLUS_AUTHEN_METH_ENABLE     := 0x04
TAC_PLUS_AUTHEN_METH_LOCAL      := 0x05
TAC_PLUS_AUTHEN_METH_TACACSPLUS := 0x06     * use this *
TAC_PLUS_AUTHEN_METH_GUEST      := 0x08
TAC_PLUS_AUTHEN_METH_RADIUS     := 0x10
TAC_PLUS_AUTHEN_METH_KRB4       := 0x11
TAC_PLUS_AUTHEN_METH_RCMD       := 0x20

priv_lvl:
TAC_PLUS_PRIV_LVL_MAX   := 0x0f               ?
TAC_PLUS_PRIV_LVL_ROOT  := 0x0f               ?
TAC_PLUS_PRIV_LVL_USER  := 0x01               ?
TAC_PLUS_PRIV_LVL_MIN   := 0x00               ?

authen_type:
TAC_PLUS_AUTHEN_TYPE_ASCII      := 0x01       ascii
TAC_PLUS_AUTHEN_TYPE_PAP        := 0x02       pap
TAC_PLUS_AUTHEN_TYPE_CHAP       := 0x03       chap
TAC_PLUS_AUTHEN_TYPE_ARAP       := 0x04       arap
TAC_PLUS_AUTHEN_TYPE_MSCHAP     := 0x05       mschap

authen_service:
TAC_PLUS_AUTHEN_SVC_NONE        := 0x00
TAC_PLUS_AUTHEN_SVC_LOGIN       := 0x01
TAC_PLUS_AUTHEN_SVC_ENABLE      := 0x02
TAC_PLUS_AUTHEN_SVC_PPP         := 0x03
TAC_PLUS_AUTHEN_SVC_ARAP        := 0x04
TAC_PLUS_AUTHEN_SVC_PT          := 0x05
TAC_PLUS_AUTHEN_SVC_RCMD        := 0x06
TAC_PLUS_AUTHEN_SVC_X25         := 0x07
TAC_PLUS_AUTHEN_SVC_NASI        := 0x08
TAC_PLUS_AUTHEN_SVC_FWPROXY     := 0x09
*/
#define TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE 8
/* An authorization request packet */
struct author
{
    UI8_T authen_method;
    UI8_T priv_lvl;
    UI8_T authen_type;
    UI8_T service;

    UI8_T user_len;
    UI8_T port_len;
    UI8_T rem_addr_len;
    UI8_T arg_cnt;             /* the number of args */
    /* <arg_cnt u_chars containing the lengths of args 1 to arg n> */
    /* <user_len bytes of char data> */
    /* <port_len bytes of char data> */
    /* <rem_addr_len bytes of u_char data> */
    /* <char data for each arg> */
};

/****************************************
    send request (client finction)
****************************************/
int tac_author_send_request(struct session *session,
                        const int method,
                        const int priv_lvl,
                        const int authen_type,
                        const int authen_service,
                        const char *user,
                        const char *port,
                        const char *rem_addr,
                        char **avpair,
                        UI8_T *secret)
{
    TACACS_LIB_Packet_T     *tacacs_packet_p;
    int                     i, body_len;
    int arglens=0;
    UI8_T                   *buf;
    HDR *hdr;
    struct author *auth;
    char *lens;

    TAC_AUTHOR_TRACE2("\r\n[tac_author_send_request] user(%s) port(%s)",
        user, port);

    /*tacacs_packet_p = (TACACS_LIB_Packet_T*)L_MEM_Allocate(sizeof(TACACS_LIB_Packet_T));*/
    if ((tacacs_packet_p = (TACACS_LIB_Packet_T*)L_MM_Malloc(sizeof(TACACS_LIB_Packet_T), L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_AUTHOR_SEND_REQUEST))) == NULL)
        return 0;

    /* header */
    buf = (UI8_T*)tacacs_packet_p;
    hdr = &tacacs_packet_p->packet_header;

    /* datas */
    auth = (struct author *)(buf+TAC_PLUS_HDR_SIZE);
    lens = (char *)(buf+TAC_PLUS_HDR_SIZE+TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE);

    hdr->version = TAC_PLUS_VER_0;
    hdr->type = TAC_PLUS_AUTHOR;      /* set packet type to authorization */
    hdr->seq_no = ++session->seq_no;
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/

    /* in packet, use network byte order */
    hdr->session_id = L_STDLIB_Hton32(session->session_id);

    /* count length */
    for (i=0; avpair[i]!=NULL ; i++)
    {
        TAC_AUTHOR_TRACE2("\r\n[tac_author_send_request] avpair(%d): %s",
            i, avpair[i]);

        if (strlen(avpair[i])>255)
        {
            L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
            return 0;
        }
        arglens += strlen(avpair[i]);
    }

    body_len = TAC_AUTHOR_REQ_FIXED_FIELDS_SIZE + i + strlen(user) +
                strlen(port) + strlen(rem_addr) + arglens;

    /* check buffer size */
    if (TACACS_LIB_MAX_LEN_OF_PACKET_BODY < body_len)
    {
        TAC_AUTHOR_TRACE2("\r\n[tac_author_send_request] body len(%d) can not large than %u",
            body_len, TACACS_LIB_MAX_LEN_OF_PACKET_BODY);
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return 0;
    }

    /* in packet, use network byte order */
    hdr->datalength = L_STDLIB_Hton32(body_len);

    auth->authen_method = (UI8_T) method;
    auth->priv_lvl = (UI8_T) priv_lvl;
    auth->authen_type = (UI8_T) authen_type;
    auth->service = (UI8_T) authen_service;
    auth->user_len = (UI8_T) strlen(user);
    auth->port_len = (UI8_T) strlen(port);
    auth->rem_addr_len = (UI8_T) strlen(rem_addr);
    auth->arg_cnt = (UI8_T) i;

    for (i=0; avpair[i]!=NULL ; i++)
    {
        *lens = (UI8_T) strlen(avpair[i]);
        lens+=1;
    }

    /* now filling some data */
    if (strlen(user) > 0)
    {
        strcpy(lens,user);
        lens += strlen(user);
    }
    if (strlen(port) > 0)
    {
        strcpy(lens,port);
        lens += strlen(port);
    }
    if (strlen(rem_addr) > 0)
    {
        strcpy(lens,rem_addr);
        lens += strlen(rem_addr);
    }
    for (i=0; avpair[i]!=NULL ; i++)
    {
        strcpy(lens,avpair[i]);
        lens += (UI8_T)strlen(avpair[i]);
    }

    /* send packet */
    if (FALSE == TACACS_LIB_WritePacket(session, tacacs_packet_p, secret))
    {
        TAC_AUTHOR_TRACE("\r\n[tac_author_send_request] call TACACS_LIB_WritePacket() failed");
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return 0;
    }

    L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
    return 1;
}


/* RESPONSEs processing *
status =
TAC_PLUS_AUTHOR_STATUS_PASS_ADD  := 0x01
TAC_PLUS_AUTHOR_STATUS_PASS_REPL := 0x02
TAC_PLUS_AUTHOR_STATUS_FAIL      := 0x10
TAC_PLUS_AUTHOR_STATUS_ERROR     := 0x11
TAC_PLUS_AUTHOR_STATUS_FOLLOW    := 0x21
*/
#define TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE 6
/* An authorization reply packet */
struct author_reply
{
    UI8_T status;
    UI8_T arg_cnt;
    UI16_T msg_len;
    UI16_T data_len;

    /* <arg_cnt u_chars containing the lengths of arg 1 to arg n> */
    /* <msg_len bytes of char data> */
    /* <data_len bytes of char data> */
    /* <char data for each arg> */
};


/*********************************************************
*     get RESPONSE (client function)  return status      *
**********************************************************/
int tac_author_get_response(struct session *session,
                            char *server_msg,
                            char *data,
                            char **avpair,
                            UI8_T *secret)
{
    TACACS_LIB_Packet_T     *tacacs_packet_p;
    UI8_T                   *buf, *arg_len_list_p;
    UI8_T                   *server_msg_p, *data_p, *arg_list_p;
    struct author_reply     *auth;
    HDR                     *hdr;
    UI32_T                  body_len, arg_index;
    UI32_T                  server_msg_len, data_len, args_len;

    if (session == NULL)
    {
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    /*maggie liu for authorization*/
    /*tacacs_packet_p = (TACACS_LIB_Packet_T*)L_MEM_Allocate(sizeof(TACACS_LIB_Packet_T));*/
    tacacs_packet_p = (TACACS_LIB_Packet_T*)L_MM_Malloc(sizeof(TACACS_LIB_Packet_T), L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_AUTHOR_GET_RESPONSE)) ;
    if (NULL == tacacs_packet_p)
        return TAC_PLUS_AUTHOR_STATUS_ERROR;

    buf = (UI8_T*)tacacs_packet_p;

    /* recv response from TACACS+ server */
    if (FALSE == TACACS_LIB_ReadPacket(session, tacacs_packet_p, secret))
    {
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    hdr = &tacacs_packet_p->packet_header;

    /* check packet type */
    if (hdr->type != TAC_PLUS_AUTHOR)
    {
        TAC_AUTHOR_TRACE1("\r\n[tac_author_get_response] wrong header type (%d)",
            hdr->type);
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    /* TACACS+ header size = 12
     * so the following code does not have alignment problem
     */
    auth = (struct author_reply *)(buf + TAC_PLUS_HDR_SIZE);

    TAC_AUTHOR_TRACE1("\r\n[tac_author_get_response] return status(%s)",
        (auth->status == TAC_PLUS_AUTHOR_STATUS_PASS_ADD) ? "PASS_ADD" :
        (auth->status == TAC_PLUS_AUTHOR_STATUS_PASS_REPL) ? "PASS_REPL" :
        (auth->status == TAC_PLUS_AUTHOR_STATUS_FAIL) ? "FAIL" :
        (auth->status == TAC_PLUS_AUTHOR_STATUS_ERROR) ? "ERROR" :
        (auth->status == TAC_PLUS_AUTHOR_STATUS_FOLLOW) ? "FOLLOW" :
        "unknown status");

    /* check return status */
    if (TAC_PLUS_AUTHOR_STATUS_ERROR == auth->status)
    {
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return auth->status;
    }

    /* check Attribute-Value pairs number */
    if (TACACS_LIB_MAX_NBR_OF_AV_PAIRS < auth->arg_cnt)
    {
        TAC_AUTHOR_TRACE2("\r\n[tac_author_get_response] argument(%d) can not large than %u",
            auth->arg_cnt, TACACS_LIB_MAX_LEN_OF_PACKET_BODY);
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    /* in packet, use network byte order */
    body_len = L_STDLIB_Ntoh32(hdr->datalength);
    server_msg_len = L_STDLIB_Ntoh16(auth->msg_len);
    data_len = L_STDLIB_Ntoh16(auth->data_len);

    arg_len_list_p = buf + TAC_PLUS_HDR_SIZE + TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE;
    server_msg_p = arg_len_list_p + auth->arg_cnt;
    data_p = server_msg_p + server_msg_len;
    arg_list_p = data_p + data_len;

    /* sum of argument length */
    for (arg_index = args_len = 0; arg_index < auth->arg_cnt; arg_index++)
        args_len += arg_len_list_p[arg_index];

    if (body_len != (TAC_AUTHOR_REPLY_FIXED_FIELDS_SIZE +
            auth->arg_cnt + server_msg_len + data_len + args_len))
    {
        TAC_AUTHOR_TRACE("\r\n[tac_author_get_response] error! body length mismatch");
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    /* if server_msg exist, copy it */
    if (0 < server_msg_len)
    {
        /* server_msg len is defined in two octects.
         * As a result, input buffer may be not large enough.
         * If server_msg length is too long, just truncate it
         */
        if (TACACS_LIB_MAX_LEN_OF_SERVER_MSG < server_msg_len)
            server_msg_len = TACACS_LIB_MAX_LEN_OF_SERVER_MSG;

        memcpy(server_msg, server_msg_p, server_msg_len);
    }
    server_msg[server_msg_len] = '\0'; /* force end a string */
    TAC_AUTHOR_TRACE1("\r\n[tac_author_get_response] server_msg:\r\n%s", server_msg);

    /* if data exist, copy it */
    if (0 < data_len)
    {
        /* data len is defined in two octects.
         * As a result, input buffer may be not large enough.
         * If data length is too long, just truncate it
         */
        if (TACACS_LIB_MAX_LEN_OF_DATA < data_len)
            data_len = TACACS_LIB_MAX_LEN_OF_DATA;

        memcpy(data, data_p, data_len);
    }
    data[data_len] = '\0';
    TAC_AUTHOR_TRACE1("\r\n[tac_author_get_response] data:\r\n%s", data);

    /* if failed to create av pairs */
    if (FALSE == TACACS_LIB_CreateAvPairs(auth->arg_cnt, arg_len_list_p,
            arg_list_p, avpair))
    {
        L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/
        return TAC_PLUS_AUTHOR_STATUS_ERROR;
    }

    L_MM_Free(tacacs_packet_p); /*maggie liu for authorization*/

    return auth->status;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - TACACS_LIB_CreateAvPairs
 *---------------------------------------------------------------------------
 * PURPOSE  : create Attribute-Value pairs by packet argument buffer
 * INPUT    : arg_cnt           -- how many arguments in arg_len
 *            arg_len           -- how long every argument is
 *            arg_buf           -- the argument content in packet buffer
 * OUTPUT   : av_pair_list_p
 * RETURN   : None
 * NOTE     : the size of av_pair_list_p MUST be not large than
 *            TACACS_LIB_MAX_NBR_OF_AV_PAIRS
 *---------------------------------------------------------------------------
 */
static BOOL_T TACACS_LIB_CreateAvPairs(
                    UI32_T arg_cnt,
                    const UI8_T *arg_len_list,
                    const UI8_T *arg_buf,
                    char **av_pair_list_p)
{
    UI32_T  arg_index;

    /* if argument too many */
    if (TACACS_LIB_MAX_NBR_OF_AV_PAIRS < arg_cnt)
    {
        TAC_AUTHOR_TRACE2("\r\n[TACACS_LIB_CreateAvPairs] argument(%lu) can not more than %u",
            arg_cnt, TACACS_LIB_MAX_LEN_OF_PACKET_BODY);
        return FALSE;
    }

    memset(av_pair_list_p, 0, sizeof(char*) * TACACS_LIB_MAX_NBR_OF_AV_PAIRS);

    for (arg_index = 0; arg_index < arg_cnt; arg_index++)
    {
        /* check arg len, is it possible zero? */
        if (0 >= arg_len_list[arg_index])
            continue;

        /* 1, char MAX value is 255
         * 2, need one octect '\0' to terminate a string
         */

        /*maggie liu for authorization*/
       /* *av_pair_list_p = (char*)L_MEM_Allocate(((int)arg_len_list[arg_index]) + 1);*/
        *av_pair_list_p = (char*)L_MM_Malloc(((int)arg_len_list[arg_index]) +1, L_MM_USER_ID2(SYS_MODULE_TACACS, 0)) ;

        /* if failed to allocate memory */
        if (NULL == *av_pair_list_p)
        {
            TACACS_LIB_FreeAvPairs(av_pair_list_p);
            TAC_AUTHOR_TRACE("\r\n[TACACS_LIB_CreateAvPairs] memory is not enough");
            return FALSE;
        }

        memcpy(*av_pair_list_p, arg_buf, arg_len_list[arg_index]);
        (*av_pair_list_p)[arg_len_list[arg_index]] = '\0'; /* force to end a string */

        TAC_AUTHOR_TRACE2("\r\n[tac_author_get_response] arg(%lu): %s",
            arg_index, *av_pair_list_p);

        arg_buf += arg_len_list[arg_index]; /* move pointer to next arg */
        ++av_pair_list_p; /* move pointer to next av pair */
    }

    return TRUE;
}
#endif
