#include "dhcp_type.h"

#ifdef	SYS_CPNT_DHCP_SERVER
/* FUNCTION NAME : dhcp
 * PURPOSE:
 *       DHCP protocol for dhcp server.
 *
 * INPUT:
 *      packet -- the raw packet we receive
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. Be aware, this function now only for dhcp client and server use
 *		2. For relay, we will call relay() instead.
 */
void dhcp(struct packet *packet);

void dhcpdiscover(struct packet *packet);
void dhcprequest(struct packet *packet);
void dhcprelease(struct packet *packet);
void dhcpdecline(struct packet *packet);
void dhcpinform(struct packet *packet);
void nak_lease(struct packet *packet, struct iaddr *cip);
void ack_lease(struct packet *packet, struct lease *lease, unsigned intoffer, TIME when);
void dhcp_reply(struct lease *lease);
struct lease *find_lease(struct packet *packet, struct shared_network *share, int *ours);
struct lease *mockup_lease(struct packet *packet, struct shared_network *share, struct host_decl *hp);
void lease_ping_timeout(void *vlp);
	
#endif /* end of #ifdef	SYS_CPNT_DHCP_SERVER */

