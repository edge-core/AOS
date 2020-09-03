/*
 *   AUTHENTICATION
 *
 */
#include "sys_adpt.h"
#include "tacacs_type.h"
#include "libtacacs.h"
#include "l_stdlib.h"
/* #include "socket.h" */
#include <string.h>
/*
          types of authentication
TACACS_ENABLE_REQUEST  1    Enable Requests
TACACS_ASCII_LOGIN     2    Inbound ASCII Login
TACACS_PAP_LOGIN       3    Inbound PAP Login
TACACS_CHAP_LOGIN      4    Inbound CHAP login
TACACS_ARAP_LOGIN      5    Inbound ARAP login
TACACS_PAP_OUT         6    Outbound PAP request
TACACS_CHAP_OUT        7    Outbound CHAP request
TACACS_ASCII_ARAP_OUT  8    Outbound ASCII and ARAP request
TACACS_ASCII_CHPASS    9    ASCII change password request
TACACS_PPP_CHPASS      10   PPP change password request
TACACS_ARAP_CHPASS     11   ARAP change password request
TACACS_MSCHAP_LOGIN    12   MS-CHAP inbound login
TACACS_MSCHAP_OUT      13   MS-CHAP outbound login

	tac_authen_send_start - ending start authentication packet
		(we are as client initiate connection)
		port		tty10 or Async10
		username
		type
		data		external data to tacacs+ server
	return
		1       SUCCESS
		0       FAILURE
*/
#define TAC_AUTHEN_START_FIXED_FIELDS_SIZE 8
struct authen_start {
    UI8_T action;
    UI8_T priv_lvl;
/*
#define TAC_PLUS_PRIV_LVL_MIN 0x0
#define TAC_PLUS_PRIV_LVL_MAX 0xf
*/
    UI8_T authen_type;

#define TAC_PLUS_AUTHEN_TYPE_ASCII  1
#define TAC_PLUS_AUTHEN_TYPE_PAP    2
#define TAC_PLUS_AUTHEN_TYPE_CHAP   3
#define TAC_PLUS_AUTHEN_TYPE_ARAP   4
#define TAC_PLUS_AUTHEN_TYPE_MSCHAP 5

    UI8_T service;

#define TAC_PLUS_AUTHEN_SVC_LOGIN  1
#define TAC_PLUS_AUTHEN_SVC_ENABLE 2
#define TAC_PLUS_AUTHEN_SVC_PPP    3
#define TAC_PLUS_AUTHEN_SVC_ARAP   4
#define TAC_PLUS_AUTHEN_SVC_PT     5
#define TAC_PLUS_AUTHEN_SVC_RCMD   6
#define TAC_PLUS_AUTHEN_SVC_X25    7
#define TAC_PLUS_AUTHEN_SVC_NASI   8

    UI8_T user_len;
    UI8_T port_len;
    UI8_T rem_addr_len;
    UI8_T data_len;
    /* <user_len bytes of char data> */
    /* <port_len bytes of char data> */
    /* <rem_addr_len bytes of u_char data> */
    /* <data_len bytes of u_char data> */
};
/***************************************************/
int tac_authen_send_start(struct session* session, char* port, char* addr, char* username, int type, char* data,int auth_priv_lvl,UI8_T *secret)
{
#define REMOTE_ADDR_LEN 57
  char buf[256];
  HDR *hdr = (HDR *)buf;
  struct authen_start *ask=(struct authen_start *)(buf+TAC_PLUS_HDR_SIZE);
  /* username */
  char *u=NULL;
  /* port */
  char *p=NULL;
  /* addr */
  char *a=NULL;
  /* data */
  char *d;

  if (session == NULL)
	return 0;

  /* clear */
  memset(buf,'\0',sizeof(buf));

  if(256 < (TAC_PLUS_HDR_SIZE+TAC_AUTHEN_START_FIXED_FIELDS_SIZE+strlen(username)+strlen(port)+ strlen(addr)+strlen(data)))
  	return 0;

  u=(char *)(buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_START_FIXED_FIELDS_SIZE);
  p=(char *)(u+strlen(username));
  a=(char *)(p+strlen(port));
  d = (char *)(a + strlen(addr));

  /*** header ***/
  /* version (TAC_PLUS_MAJOR_VER | TAC_PLUS_MINOR_VER_0) */
  if  (type == TACACS_ENABLE_REQUEST || type == TACACS_ASCII_LOGIN )
	   hdr->version = TAC_PLUS_VER_0;
  else
	   hdr->version = TAC_PLUS_VER_1;

  /* type of packet - TAC_PLUS_AUTHEN */
  hdr->type = TAC_PLUS_AUTHEN;
  /* set sequence, for first request it will be 1 */
  hdr->seq_no = ++(session->seq_no);
  /* encryption TAC_PLUS_ENCRYPTED || TAC_PLUS_CLEAR */
  hdr->encryption = TAC_PLUS_CLEAR;  /*TAC_PLUS_ENCRYPTED;*/
 /* in packet, use network byte order */
    hdr->session_id = L_STDLIB_Hton32(session->session_id);

  /* data length */
  if (type == TACACS_CHAP_LOGIN || type == TACACS_MSCHAP_LOGIN)
      hdr->datalength = L_STDLIB_Hton32(TAC_AUTHEN_START_FIXED_FIELDS_SIZE+strlen(username)+strlen(port)+strlen(addr)+1+strlen(data));
  else if (type == TACACS_PAP_LOGIN || type == TACACS_ARAP_LOGIN)
      hdr->datalength = L_STDLIB_Hton32(TAC_AUTHEN_START_FIXED_FIELDS_SIZE+strlen(username)+strlen(port)+strlen(addr)+strlen(data));
  else
      hdr->datalength = L_STDLIB_Hton32(TAC_AUTHEN_START_FIXED_FIELDS_SIZE+strlen(username)+strlen(port)+strlen(addr));

  /* privilege level */
   ask->priv_lvl = auth_priv_lvl;
  switch (type)
  {
    case TACACS_ENABLE_REQUEST:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->service = TAC_PLUS_AUTHEN_SVC_ENABLE;
       break;
    case TACACS_ASCII_LOGIN:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
       ask->service = TAC_PLUS_AUTHEN_SVC_LOGIN;
       break;
    case TACACS_PAP_LOGIN:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
       break;
    case TACACS_PAP_OUT:
       ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_PAP;
       break;
    case TACACS_CHAP_LOGIN:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
       break;
    case TACACS_CHAP_OUT:
       ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_CHAP;
       break;
    case TACACS_MSCHAP_LOGIN:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
       break;
    case TACACS_MSCHAP_OUT:
       ask->action = TAC_PLUS_AUTHEN_SENDAUTH;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_MSCHAP;
       break;
    case TACACS_ARAP_LOGIN:
       ask->action = TAC_PLUS_AUTHEN_LOGIN;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ARAP;
       break;
    case TACACS_ASCII_CHPASS:
       ask->action = TAC_PLUS_AUTHEN_CHPASS;
       ask->authen_type = TAC_PLUS_AUTHEN_TYPE_ASCII;
       break;
  }
  /*
   * the length of fields in start packet
   * using without convertation ntohs or htons
   * (this is not clean in RFC)
   */
  /* username length */
  ask->user_len = strlen(username);
  if (strlen(username)>128) return 0;
  /* portname length */
  ask->port_len = strlen(port);
  /* addr length */
  ask->rem_addr_len = strlen(addr);
  /* data length */
  ask->data_len = strlen(data);

  /* join data */
  if ((strlen(username) > 0) && (strlen(username) <= SYS_ADPT_MAX_USER_NAME_LEN))
    strncpy(u, username, SYS_ADPT_MAX_USER_NAME_LEN); /* user */
  if ((strlen(port) > 0) &&(strlen(port) <= MAX_LEN_OF_NAS_PORT))
    strncpy(p, port, MAX_LEN_OF_NAS_PORT);     /* port */
  if ((strlen(addr) > 0) && (strlen(addr) <= REMOTE_ADDR_LEN))
    strncpy(a, addr, REMOTE_ADDR_LEN);     /* addr */

  if (type == TACACS_CHAP_LOGIN)
  {
     *d++ = 1;
     strcpy(d,data);
  }
  if (type == TACACS_ARAP_LOGIN || type == TACACS_PAP_LOGIN)
     strcpy(d,data);

  /* write_packet encripting datas */
  if (FALSE == TACACS_LIB_WritePacket(session, (TACACS_LIB_Packet_T *)buf, secret)) return 0;
  return 1;
}

/*********************************************
   send REPLY packet (server function)
   return status packet
   and set variables
	return
		0	SUCCESS
		-1	FAILURE
Status:
TAC_PLUS_AUTHEN_STATUS_PASS     1
TAC_PLUS_AUTHEN_STATUS_FAIL     2
TAC_PLUS_AUTHEN_STATUS_GETDATA  3
TAC_PLUS_AUTHEN_STATUS_GETUSER  4
TAC_PLUS_AUTHEN_STATUS_GETPASS  5
TAC_PLUS_AUTHEN_STATUS_RESTART  6
TAC_PLUS_AUTHEN_STATUS_ERROR    7
TAC_PLUS_AUTHEN_STATUS_FOLLOW   0x21
***********************************************/
#define TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE 6

struct authen_reply {
    UI8_T status;
    UI8_T flags;
#define TAC_PLUS_AUTHEN_FLAG_NOECHO     0x1
    UI16_T msg_len;
    UI16_T data_len;
    /* <msg_len bytes of char data> */
    /* <data_len bytes of u_char data> */
};
/*************************************/
/* get REPLY reply (client function) */
/* return status packet and set variables
	return
		-1	FAILURE
Status:

   TAC_PLUS_AUTHEN_STATUS_PASS     := 0x01
   TAC_PLUS_AUTHEN_STATUS_FAIL     := 0x02
   TAC_PLUS_AUTHEN_STATUS_GETDATA  := 0x03
   TAC_PLUS_AUTHEN_STATUS_GETUSER  := 0x04
   TAC_PLUS_AUTHEN_STATUS_GETPASS  := 0x05
   TAC_PLUS_AUTHEN_STATUS_RESTART  := 0x06
   TAC_PLUS_AUTHEN_STATUS_ERROR    := 0x07
   TAC_PLUS_AUTHEN_STATUS_FOLLOW   := 0x21

*/
int tac_authen_get_reply(struct session* session, char* server, char* datas, UI8_T *secret)
{
   UI8_T buf[256] = {0};
   /* header */
   HDR *hdr;
   /* static datas */
   struct authen_reply *rep;
   /* server message */
   char *serv_msg = NULL;
   /* server datas */
   char *dat_pak = NULL;
   int mlen=0,dlen=0;

   if (FALSE == TACACS_LIB_ReadPacket(session, (TACACS_LIB_Packet_T *)buf, secret)) return 0;

  if(session == NULL) return 0;
   hdr = (HDR *)buf;
   rep = (struct authen_reply *)(buf+TAC_PLUS_HDR_SIZE);
   serv_msg = (char *)
       (buf+TAC_PLUS_HDR_SIZE+TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE);
   dat_pak = (char *)(serv_msg + L_STDLIB_Ntoh16(rep->msg_len));

   /* fields length */
   mlen = L_STDLIB_Ntoh16(rep->msg_len);
   dlen = L_STDLIB_Ntoh16(rep->data_len);

   if (hdr->datalength != L_STDLIB_Hton32(TAC_AUTHEN_REPLY_FIXED_FIELDS_SIZE +
	       mlen + dlen))
   {
       return 0;
   }
   session->session_id = L_STDLIB_Ntoh32(hdr->session_id);

   if (mlen > 0)
   {
      memset(server,'\0',mlen);
      strncpy(server,serv_msg,mlen);
   }
   if (dlen > 0)
   {
      memset(datas,'\0',dlen);
      strncpy(datas,dat_pak,dlen);
   }
   
   return (rep->status);
}

/************************************
   Send CONTINUE packet
	  (client function)

   tac_authen_send_cont

	return
		1       SUCCESS
		0       FAILURE
*************************************/
#define TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE 5
struct authen_cont {
    UI16_T user_msg_len;
    UI16_T user_data_len;
    UI8_T flags;

#define TAC_PLUS_CONTINUE_FLAG_ABORT 0x1

    /* <user_msg_len bytes of u_char data> */
    /* <user_data_len bytes of u_char data> */
};
/* --------------------------------------------------- */
int
tac_authen_send_cont(struct session* session, char* user_msg,char* data, UI8_T *secret)
{
#if (SYS_ADPT_MAX_USER_NAME_LEN >= SYS_ADPT_MAX_PASSWORD_LEN)
#define USER_MSG_LEN SYS_ADPT_MAX_USER_NAME_LEN
#else
#define USER_MSG_LEN SYS_ADPT_MAX_PASSWORD_LEN
#endif
  char buf[512];
  /* header */
  HDR *hdr = (HDR *)buf;
  /* datas */
  struct authen_cont *ask = (struct authen_cont *)(buf + TAC_PLUS_HDR_SIZE);
  /* packet */
  char *p = (char *)
    (buf + TAC_PLUS_HDR_SIZE + TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE);
  char *d = (char *)(p + strlen(user_msg));

  /* zero */
  memset(buf,'\0', sizeof(buf));
  /* version */
  hdr->version = TAC_PLUS_VER_0;
  /* packet type */
  hdr->type = TAC_PLUS_AUTHEN;
  /* sequence number */
  hdr->seq_no = ++session->seq_no;
  /* set encryption */
  hdr->encryption = TAC_PLUS_CLEAR; /*TAC_PLUS_ENCRYPTED;*/
  /* in packet, use network byte order */
    hdr->session_id = L_STDLIB_Hton32(session->session_id);
  /* packet length */
  hdr->datalength = L_STDLIB_Hton32(TAC_AUTHEN_CONT_FIXED_FIELDS_SIZE+strlen(user_msg)
			   + strlen(data));
  /* data length */
  ask->user_msg_len = L_STDLIB_Hton16(strlen(user_msg));
  ask->user_data_len = L_STDLIB_Hton16(strlen(data));

  /* set datas */
  if ((strlen(user_msg) > 0) && (strlen(user_msg) <= USER_MSG_LEN))
    strncpy(p, user_msg, USER_MSG_LEN);
  if (strlen(data) > 0)
    strcpy(d, data);

  /* send packet */
  if (FALSE == TACACS_LIB_WritePacket(session, (TACACS_LIB_Packet_T *)buf, secret)) return 0;
  return 1;
}
