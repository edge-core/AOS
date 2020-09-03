#include "libtacacs.h"
#include "tacacs_mgr.h"
#include "l_stdlib.h"
/* #include "socket.h" */
#include <string.h>
#include "sys_module.h"
#include "l_mm.h"
/*
   send the accounting REQUEST  (client function)
*/
#define TAC_ACCT_REQ_FIXED_FIELDS_SIZE 9
#define TAC_ACCT_REPLY_FIXED_FIELDS_SIZE 5

struct acct
{
    UI8_T flags;
    UI8_T authen_method;
    UI8_T priv_lvl;
    UI8_T authen_type;
    UI8_T authen_service;
    UI8_T user_len;
    UI8_T port_len;
    UI8_T rem_addr_len;
    UI8_T arg_cnt; /* the number of cmd args */
    /* one UI8_T containing size for each arg */
    /* <user_len bytes of char data> */
    /* <port_len bytes of char data> */
    /* <rem_addr_len bytes of UI8_T data> */
    /* char data for args 1 ... n */
};

struct acct_reply
{
    UI16_T msg_len;
    UI16_T data_len;
    UI8_T status;      /* status */
};
/*
av-pairs:    (depends from IOS release)
  "task_id="
  "start_time="
  "stop_time="
  "elapsed_time="
  "timezone="
  "event=net_acct|cmd_acct|conn_acct|shell_acct|sys_acct|clock_change"
       Used only when "service=system"
  "reason="  - only for event attribute
  "bytes="
  "bytes_in="
  "bytes_out="
  "paks="
  "paks_in="
  "paks_out="
  "status="
    . . .
     The numeric status value associated with the action. This is a signed
     four (4) byte word in network byte order. 0 is defined as success.
     Negative numbers indicate errors. Positive numbers indicate non-error
     failures. The exact status values may be defined by the client.
  "err_msg="

   NULL - last

FLAGS:
TAC_PLUS_ACCT_FLAG_MORE     = 0x1     (deprecated)
TAC_PLUS_ACCT_FLAG_START    = 0x2
TAC_PLUS_ACCT_FLAG_STOP     = 0x4
TAC_PLUS_ACCT_FLAG_WATCHDOG = 0x8
*/
int tac_account_send_request(struct session *session,
                             const int flag,
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
    int i;
    char *buf;
    HDR *hdr;
    struct acct *acc;
    char *lens;
    int arglens=0;

    if ((buf = (char *)L_MM_Malloc(sizeof(char)*512, L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_SEND_REQUEST))) == NULL)
        return 1;
    hdr = (HDR *)buf;
    acc = (struct acct *)(buf + TAC_PLUS_HDR_SIZE);
    lens=(char*)(buf+TAC_PLUS_HDR_SIZE+
                 TAC_ACCT_REQ_FIXED_FIELDS_SIZE);

    memset(buf,'\0',sizeof(char)*512);
    hdr->version = TAC_PLUS_VER_0;
    hdr->type = TAC_PLUS_ACCT;
    hdr->seq_no = ++session->seq_no;
    hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
    hdr->session_id = session->session_id;

    for (i=0; avpair[i]!=NULL ; i++)
    {
        if (strlen(avpair[i])>255)    /* if lenght of AVP>255 set it to 255 */
            avpair[i][255]=0;
        arglens += strlen(avpair[i]);
    }
    hdr->datalength = L_STDLIB_Hton32(TAC_ACCT_REQ_FIXED_FIELDS_SIZE +
                           i+strlen(user)+strlen(port)+strlen(rem_addr)+arglens);

    acc->flags = (UI8_T) flag;
    acc->authen_method = (UI8_T) method;
    acc->priv_lvl = (UI8_T) priv_lvl;
    acc->authen_type = (UI8_T) authen_type;
    acc->authen_service = (UI8_T) authen_service;
    acc->user_len=(UI8_T)strlen(user);
    acc->port_len=(UI8_T)strlen(port);
    acc->rem_addr_len = (UI8_T) strlen(rem_addr);
    acc->arg_cnt = (UI8_T) i;

    for (i=0; avpair[i]!=NULL ; i++)
    {
        *lens=(UI8_T)strlen(avpair[i]);
        lens=lens+1;
    }
    /* filling data */
    if (strlen(user)>0)
    {
        strcpy(lens,user);
        lens += strlen(user);
    }
    if (strlen(port)>0)
    {
        strcpy(lens,port);
        lens += strlen(port);
    }
    if (strlen(rem_addr)>0)
    {
        strcpy(lens,rem_addr);
        lens += strlen(rem_addr);
    }
    for (i=0; avpair[i]!=NULL ; i++)
    {
        strcpy(lens,avpair[i]);
        lens += (UI8_T)strlen(avpair[i]);
    }
    if (FALSE == TACACS_LIB_WritePacket(session, (TACACS_LIB_Packet_T*)buf, secret))
    {
        L_MM_Free(buf);
        return 0;
    }
    L_MM_Free(buf);
    return 1;
}

/*************************************************
    get the accounting REPLY (client function)
       1  SUCCESS
       0  FAILURE
     Note: return value not 1 will be treat as fail
**************************************************/
int tac_account_get_reply(struct session *session,
                  char *server_msg,
                          char *data,
                          UI8_T *secret)
{
    int status;
    UI8_T *buf;

    HDR *hdr;
    struct acct_reply *acc;
    char *lens;

    if ((buf =
    (UI8_T *)L_MM_Malloc(
        sizeof(UI8_T)*TACACS_LIB_MAX_LEN_OF_PACKET_BODY,
        L_MM_USER_ID2(SYS_MODULE_TACACS, TACACS_TYPE_TRACE_ID_TACACS_ACCOUNT_GET_REPLY))) == NULL)
        return 0;
    hdr = (HDR *)buf;
    acc = (struct acct_reply *)(buf + TAC_PLUS_HDR_SIZE);
    lens=(char*)(buf + TAC_PLUS_HDR_SIZE +
              TAC_ACCT_REPLY_FIXED_FIELDS_SIZE);

    TACACS_LIB_ReadPacket(session, (TACACS_LIB_Packet_T*)buf, secret);
    /* some checks */
    if (hdr->type != TAC_PLUS_ACCT)
    {
        L_MM_Free(buf);
        return 0/*-1*/;
    }
    if (hdr->seq_no != 2)
    {
        L_MM_Free(buf);
        return 0;
    }
    session->session_id = hdr->session_id;

    if (hdr->datalength != L_STDLIB_Hton32(TAC_ACCT_REPLY_FIXED_FIELDS_SIZE+
        acc->msg_len + acc->data_len))
    {
        L_MM_Free(buf);
        return 0;
    }
    status=acc->status;

    memset(server_msg,'\0',acc->msg_len);
    strncpy(server_msg,lens,acc->msg_len);
    lens = lens + acc->msg_len;
    memset(data,'\0',acc->data_len);
    strncpy(data,lens,acc->data_len);
    L_MM_Free(buf);

    return status;
}
