
#ifndef DOT1X_PKT_XMIT_H
#define DOT1X_PKT_XMIT_H


#include "1x_common.h"


typedef struct PKT_XMIT_tag
{


     struct 		libnet_link_int	* network;
     unsigned char		*device;
     unsigned char		errbuf[256];

     Global_Params 	* global;
	
} PKT_XMIT;

#endif/*DOT1X_PKT_XMIT_H*/
