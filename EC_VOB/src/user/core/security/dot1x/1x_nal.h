#ifndef LIB1X_NAL_H
#define LIB1X_NAL_H

#include "sys_cpnt.h"
#include "1x_common.h"
#include "1x_ethernet.h"
#include "sys_type.h"

/* The address types*/
#define     LIB1X_NAL_MACADDR   1
#define         LIB1X_NAL_IPADDR    2

/* Interface types.*/
#define         LIB1X_NAL_IT_ETHLAN 1
#define         LIB1X_NAL_IT_WLAN   2

#define     LIB1X_MAXDEVLEN     16   /* IFNAMSIZ + 10  */   

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE) 
    #define         LIB1X_MAXEAPLEN     1600//512  /* suger, 05-04-2004, enlarge to 1600 */
#else
#define         LIB1X_MAXEAPLEN     512  
#endif    

#define     LIB1X_MAX_RAD_STATE_LEN 255

/* This structure defines an "address". This could
 * be an IP address or a MAC address, thus doing it
 * this way  gives us a "generic" interface.
 */
struct lib1x_nal_addr
{
    unsigned char * addr;
    int len;
    int addr_type;
};

/* Abstracts the notion of an interface, which could be a socket
 * or an actual device
 */
struct lib1x_nal_intdev
{
    unsigned char * interface;
    int type;
};


/* Generic packet struct .. passed to the handler */

struct lib1x_packet
{
    UI8_T  *data;
    int caplen;
};



/* We shall use the Berkeley Packet Capture Utility.*/
#define  LIB1X_LSTNR_PROMISCMODE        1
#define  LIB1X_LSTNR_SNAPLEN            512 
#define  LIB1X_LSTNR_RDTIMEOUT          1000    

#define  LIB1X_IT_PKTSOCK           1
#define  LIB1X_IT_UDPSOCK           2

struct lib1x_nal_intfdesc;

typedef void lib1x_nal_genpkt_handler( Global_Params * , struct lib1x_nal_intfdesc * , struct lib1x_packet * ); 

/* Interface descriptor*/
struct lib1x_nal_intfdesc 
{
     struct  lib1x_packet  packet;
     UI8_T       inttype;    /* interface type, packet socket or udp socket */

#if 0 /* (SAVE MEMORY) only two data members are meanful  */
/*1. The listener datastructures*/

    BOOLEAN     promisc_mode;
    int     snaplen;
    int     read_timeout;
    int     pf_sock;    /* socket : PF_PACKET   since libpcap needs to get discarded*/
    /*UI8_T     device[ LIB1X_MAXDEVLEN + 1];*/
    lib1x_nal_genpkt_handler * packet_handler;  /* not using currently, it is dynamic */
    UI8_T       packet_buffer[LIB1X_MAXEAPLEN];
    struct  lib1x_packet  packet;
/*2. The Xmitter datastructures*/

     struct             libnet_link_int * libnet_desc;

/*3. general ..*/
     UI8_T      ouraddr[ ETHER_ADDRLEN ];
     UI8_T      inttype;    /* interface type, packet socket or udp socket */

/*4. If we are having a UDP socket */
     int        udpsock;
     struct sockaddr_in *radsvraddr;
#endif /* if 0 */
};

BOOL_T DOT1X_Send_Packet(struct Auth_Pae_tag * auth_pae,UI8_T *packet_data,UI32_T packet_len);
BOOLEAN lib1x_nal_send( struct Auth_Pae_tag * auth_pae, struct lib1x_nal_intfdesc * desc,  char * packet , int size);
void lib1x_nal_receivepoll( Global_Params * global, struct lib1x_nal_intfdesc * desc , lib1x_nal_genpkt_handler * pkt_handler, UI8_T *  info);
void lib1x_nal_close( struct lib1x_nal_intfdesc * desc );
void lib1x_nal_EAPOL_header( struct Auth_Pae_tag * auth_pae,UI32_T port);
BOOL_T lib1x_nal_EAP_filter(UI32_T lport,UI8_T *mac,UI32_T vid);
#endif /*LIB1X_NAL_H*/
