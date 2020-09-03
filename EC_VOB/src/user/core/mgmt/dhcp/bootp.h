#include "sys_cpnt.h"
#if (SYS_CPNT_DHCP_SERVER == TRUE && SYS_CPNT_BOOTP == TRUE)
#include "dhcp_type.h"

void bootp(struct packet *packet);

#endif 

