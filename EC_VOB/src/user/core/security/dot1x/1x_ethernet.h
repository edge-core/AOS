
#ifndef LIB1X_ETHERNET_H
#define LIB1X_ETHERNET_H

#define ETHER_ADDRLEN		6
#define ETHER_HDRLEN		14
#define LIB1X_ETHER_EAPOL_TYPE	0x888E
#define LIB1X_ETHER_IP		0x800
#define LIB1X_ETHER_TAG_INFO	0

struct lib1x_ethernet
{
	unsigned char  ether_dhost[ETHER_ADDRLEN];    /* destination ethernet address */
	unsigned char  ether_shost[ETHER_ADDRLEN];    /* source ethernet address */
	unsigned short ether_type;                     /* packet type ID */
};    

#endif /*LIB1X_ETHERNET_H*/
