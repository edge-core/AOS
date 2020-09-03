#ifndef LIB1X_RADIUS_H
#define LIB1X_RADIUS_H

#include "1x_nal.h"

#define  LIB1X_ETH_IP   0x0800      /* Internet Protocol packet */


#define  LIB1X_RAD_ACCREQ   1   /* Access Request*/
#define  LIB1X_RAD_ACCACT   2   /* Access Accept*/
#define  LIB1X_RAD_ACCREJ   3   /* Access Reject*/
#define  LIB1X_RAD_ACCCHL   11  /* Access Challenge*/

#define  LIB1X_LIL_ENDIAN


#define  LIB1X_IPHDRLEN     20  /* Assume for now    TODO*/
#define  LIB1X_UDPHDRLEN    8
#define  LIB1X_RADHDRLEN    20  /* RADIUS Header length*/
#define  LIB1X_RADATTRLEN   2   /* length of attr field without data part*/

/* RADIUS attribute definitions. Also from RFC 2138 */
#define LIB1X_RAD_USER_NAME              1
#define LIB1X_RAD_PASSWORD               2
#define LIB1X_RAD_NAS_IP_ADDRESS         4
#define LIB1X_RAD_NAS_PORT           5
#define LIB1X_RAD_SERVICE_TYPE           6
#define LIB1X_RAD_FRAMED_MTU         12
#define LIB1X_RAD_REPLY_MESSAGE          18
#define LIB1X_RAD_STATE                  24
#define LIB1X_RAD_SESSION_TIMEOUT        27
#define LIB1X_RAD_CALLED_STID        30
#define LIB1X_RAD_CALLING_STID       31
#define LIB1X_RAD_NAS_IDENTIFIER         32
#define LIB1X_RAD_NAS_PORTTYPE       61
#define LIB1X_RAD_CONNECTINFO        77
#define LIB1X_RAD_EAP_MESSAGE        79 /* eap message .. from RFC 2869*/
#define LIB1X_RAD_MESS_AUTH      80 /* Message Authenticator*/

#define LIB1X_IPPROTO_UDP        17

#define LIB1X_80211_NAS_PORTTYPE    19  /* port type for 802.11 */




#define LIB1X_CKSUM_CARRY(x) \
    (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

struct lib1x_radiushdr
{
    UI8_T   code;
    UI8_T   identifier;
    UI16_T  length;    /* suger, 05-04-2004, radius length field is 2 octets */
    UI8_T   authenticator_str[16+1];
}__attribute__((packed, aligned(1)));

struct lib1x_radiusattr
{
    UI8_T  type;
    UI8_T  length;  /* is the lengh of entire attribute including the type and length fields*/
}__attribute__((packed, aligned(1)));

struct lib1x_udphdr
{
    UI32_T sport;   /* soure port */
    UI32_T dport;   /* destination port */
    UI32_T len;    /* length */
    UI32_T sum;     /* checksum */
}__attribute__((packed, aligned(1)));

struct lib1x_radiuspkt  /* this struct is used for parsing only */
{
    UI8_T  s_ethaddr[6];
    UI8_T  d_ethaddr[6];
    UI32_T dst_port, src_port;
    struct lib1x_radiushdr *rhdr;    /* pointer to the radius start in the packet*/
}__attribute__((packed, aligned(1)));

struct lib1x_radius_const   /* this struct is used for cosntructing packets */
{
    UI8_T    * pkt;
    struct lib1x_radiushdr * rhdr;
    UI32_T   pktlen;    /* length of the complete packet */
    UI8_T    * ptr_messauth;
    int       * nas_porttype;
}__attribute__((packed, aligned(1)));

struct lib1x_iphdr
{
#ifdef LIB1X_LIL_ENDIAN
       UI8_T ip_hl:4,         /* header length */
              ip_v:4;         /* version */
#endif
#ifdef LIB1X_BIG_ENDIAN
           UI8_T ip_v:4,          /* version */
                 ip_hl:4;        /* header length */
#endif
          UI8_T ip_tos;          /* type of service */
          UI32_T ip_len;         /* total length */
          UI32_T ip_id;          /* identification */
          UI32_T ip_off;
#ifndef IP_RF
#define IP_RF 0x8000        /* reserved fragment flag */
#endif
#ifndef IP_DF
#define IP_DF 0x4000        /* dont fragment flag */
#endif
#ifndef IP_MF
#define IP_MF 0x2000        /* more fragments flag */
#endif
#ifndef IP_OFFMASK
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
#endif
          UI8_T ip_ttl;          /* time to live */
          UI8_T ip_p;            /* protocol */
          UI32_T ip_sum;         /* checksum */
}__attribute__((packed, aligned(1)));

#define LIB1X_FRMSUPP_RESPID        1
#define LIB1X_FRMSUPP_RESPOTH       2

struct radius_info   /* one struct for radius related bookkeeping */
{
    UI8_T     identifier;                /* the identifier field in a radius packet */
    /* needs to be changed for every unique packet */
#if 0 /* (SAVE MEMORY) no function use this buffer: eap_message_frmsupp */
    UI8_T     eap_message_frmsupp[ LIB1X_MAXEAPLEN+1 ]; /* defined in 1x_nal.h */
    UI8_T     eap_messtype_frmsupp;		   /* we store the received eap message from supp*/
    int       eap_messlen_frmsupp;
#endif
    UI16_T    eap_messlen_frmserver;
    char      username[DOT1X_USERNAME_LENGTH + 1];            /* the username attribute is what the supplicant sends in *//*Ricky*/
    UI32_T    username_len;              /* length of the username attribute */
    UI8_T     radius_state[ LIB1X_MAX_RAD_STATE_LEN+1 ]; /* State attribute .. needs to be copied back */
    BOOLEAN   rad_stateavailable;        /* available or not */
    UI32_T    rad_statelength;           /* length of the State attribute .. type 24 */
}__attribute__((packed, aligned(1)));



struct Auth_Pae_tag;


void lib1x_create_reqauth( struct Auth_Pae_tag * auth_pae );
void lib1x_rad_eapresp_svr( struct Auth_Pae_tag * auth_pae, struct lib1x_packet * srcpkt);
void lib1x_rad_eapresp_supp( struct Auth_Pae_tag * auth_pae, struct lib1x_packet * pkt);
#endif /*LIB1X_RADIUS_H*/
