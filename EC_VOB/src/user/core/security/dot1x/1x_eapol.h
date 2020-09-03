#ifndef LIB1X_EAPOL_H
#define LIB1X_EAPOL_H
#include "sys_cpnt.h"

#define LIB1X_EAPOL_HDRLEN	4		/* Just the header Note:*/
						/* is different from struct*/
#define	LIB1X_EAPOL_LOGOFF	2               /*0000 0010B*/
#define LIB1X_EAPOL_EAPPKT	0               /*0000 0000B*/
#define LIB1X_EAPOL_START	1		/*0000 0001B*/
#define LIB1X_EAPOL_KEY		3		/*0000 0011B*/
#define LIB1X_EAPOL_ENCASFALERT 4		/*0000 0100B*/
#if 0
#define	LIB1X_EAP_REQUEST	1
#define LIB1X_EAP_RESPONSE	2
#define LIB1X_EAP_SUCCESS	3
#define LIB1X_EAP_FAILURE	4
#endif
#define	LIB1X_EAP_HDRLEN	4

#define LIB1X_EAP_RRIDENTITY	1
#define LIB1X_EAP_RRNOTIF	2
#define LIB1X_EAP_RRNAK		3
#define LIB1X_EAP_RRMD5		4
#define LIB1X_EAP_RROTP		5
#define LIB1X_EAP_RRGEN		6

#if ( SYS_CPNT_DOT1X_TTLS_TLS_PEAP == TRUE)
#define LIB1X_EAP_TLS		13		/* suger, 05-04-2004, add support for TLS/TTLS/PEAP */
#define LIB1X_EAP_TTLS		21
#define LIB1X_EAP_PEAP		25
#endif
#define LIB1X_EAP_ZLXEAP    44      /* ZoneLabs EAP */

#define LIB1x_EAP_RRLEN 	1

#define	LIB1X_EAPOL_VER		1		/*00000001B*/


struct lib1x_eapol
{
	/*u_short	ether_type; */   
	unsigned char	protocol_version;
        unsigned char	packet_type;	
	unsigned short  packet_body_length;
}__attribute__((packed, aligned(1)));

struct lib1x_eap
{
	unsigned char  code;	
	unsigned char  identifier;
	unsigned short length; 
}__attribute__((packed, aligned(1)));

struct lib1x_eap_rr
{
	unsigned char	type;	
}__attribute__((packed, aligned(1)));


#endif /*LIB1X_EAPOL_H*/
