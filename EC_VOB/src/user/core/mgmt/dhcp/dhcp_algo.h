/* Module Name:	DHCP_ALGO.H
 * Purpose:
 *		This module keeps original function of DHCP/IAD, eg. discover_Interface,
 *		dispatch, got_one, do_packet, relay. In DHCP_ALGO, really processes DHCP
 *		protocol function, except timeout management.
 *
 * Notes:
 *		1. Timeout mechanism is checked in DHCP_TASK, and handling routine is keeping
 *		   in this module.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.24	--	William, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */



#ifndef		_DHCP_ALGO_H
#define		_DHCP_ALGO_H


/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"
#include "dhcp_type.h"
#include "dhcp_wa.h"


/* NAME	CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */

/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION	NAME : DHCP_ALGO_Init
 * PURPOSE:
 *		Initialize data used in ALGO module.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. All data will re-init in ReStart stage. (Master --> ProvisionComplete)
 */
void	DHCP_ALGO_Init (void);

/* FUNCTION NAME : DHCP_ALGO_SetDispatchFlag
 * PURPOSE:
 *      Enable /Disable the dispatch (polling) function
 *
 * INPUT:
 *      flag -- TRUE / FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. While restarting Relay or Server, it will disable dispatch.
 *		2. After register socket (in discover_interfaces), it will be enable.
 */
 void DHCP_ALGO_SetDispatchFlag(BOOL_T flag);

/* FUNCTION	NAME : DHCP_ALGO_SetupRelayHandler
 * PURPOSE:
 *		Set up the bootp handler to do packet for relay
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_ALGO_SetupRelayHandler(void);

/* FUNCTION	NAME : DHCP_ALGO_SetupServerHandler
 * PURPOSE:
 *		Set up the bootp handler to do packet for Server
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *
 */
void DHCP_ALGO_SetupServerHandler();

/* FUNCTION NAME : relay
 * PURPOSE:
 *      Process DHCP/Bootp packet and call handling routining (dhcp relay).
 *
 * INPUT:
 *      interface   --  interface send to.
 *      packet      --  raw packet contains dhcp/bootp packet
 *      len         --  packet length
 *      from_port   --  src-port in udp packet.
 *      from        --  src-ip in packet.
 *      hfrom       --  source hardware-address.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *

 */
void relay(struct interface_info *interface,
                struct dhcp_packet *packet,
                int len,
                unsigned int from_port,
                struct iaddr from,
                struct hardware *hfrom);

/* FUNCTION	NAME : do_packet
 * PURPOSE:
 *		Process DHCP/Bootp packet and call handling routining (dhcp-client, dhcpd,
 *		or bootp-client, bootpd).
 *
 * INPUT:
 *		interface	--  interface send to.
 *		packet		--	raw packet contains dhcp/bootp packet
 *		len			--	packet length
 *		from_port	--	src-port in udp packet.
 *		from		--	src-ip in packet.
 *		hfrom		--	source hardware-address.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. do_packet is static function in ALGO, not export function.
 *		2. client_dhcp, dhcpd, client_bootp, and bootpd are static function in ALGO.
 */
void do_packet (
				struct interface_info *interface,
				struct dhcp_packet *packet,
				int	len,
				unsigned int from_port,
				struct iaddr from,
				struct hardware *hfrom);

/* FUNCTION NAME : parse_subnet_declaration
 * PURPOSE:
 *		This function mimics the parsing behavior for keyword 'subnet'.
 *
 * INPUT:
 *  	pool -- network pool config from WA
 *		share -- share network structure
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *
 */
void parse_subnet_declaration(DHCP_TYPE_PoolConfigEntry_T *pool, struct shared_network *share);

void parse_options(struct packet *packet);
void parse_option_buffer(struct packet *packet, unsigned char *buffer,int length);
void initialize_universes();
/* FUNCTION	NAME : locate_network
 * PURPOSE:
 *		find the corresponding subnet for incoming packet.
 *
 * INPUT:
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE -- able to locate shared_network
 *		FALSE -- fail to locate shared_network
 *
 * NOTES:
 *
 */
BOOL_T locate_network(struct packet *packet);
void client_dhcp(struct packet *packet);
void client_bootp(struct packet *packet);


/* FUNCTION NAME : reinitialize_interfaces
 * PURPOSE:
 *       Reinitialize interfaces.
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
 *      In hornet code, we do not close the socket and create
 *		it again. Instead, we remain on the same socket we create originally
 */
void reinitialize_interfaces ();

/* FUNCTION	NAME : got_one
 * PURPOSE:
 *		Retrieve one packet from specified interface and call do_packet() to handle
 *		this packet.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void got_one();

/* FUNCTION NAME : dispatch
 * PURPOSE:
 *      Wait for packet come in using select(); when one does, call receive_packet() to
 *		receive.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void dispatch();

/* FUNCTION	NAME : discover_interfaces
 * PURPOSE:
 *		Discover all available interfaces for DHCP.
 *
 * INPUT:
 *	 	None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE -- successfully discover at lease one interface
 *		FALSE -- Fail to discover any interface
 *
 * NOTES:
 *		1. This function retrieve interface from NETCFG and build up protocol list.
 *		2. This function must go with send_packet, receive_packet.
 *		3. Notes from DHCP/IAD :
 * 			Use the SIOCGIFCONF ioctl to get a list of all the attached interfaces.
 * 			For each interface that's of type INET and not the loopback interface,
 *	    	register that interface with the network I/O software, figure out what
 * 	    	subnet it's on, and add it to the list of interfaces.
 *
 */
BOOL_T discover_interfaces();

void add_protocol (UI32_T vid_ifIndex, int fd, void (*handler) (struct protocol *), void *local);




/* FUNCTION	NAME : state_reboot
 * PURPOSE:
 *		Change dhcp client lease state to REBOOT for specified interface.
 *
 * INPUT:
 *		ipp	--  interface_info_pointer point to interface_info structure
 *				which would be rebooted (REINIT) for dhcp client.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. If the interface had not configured ip, broadcast 'discover' packet,
 *		   else send unicast 'dhcprequest' to server.
 *
 */
void state_reboot (void *ipp);
struct client_lease *packet_to_lease(struct packet *packet);
void free_client_lease(struct client_lease *lease);
void dhcpoffer(struct packet *packet);
void dhcpack(struct packet *packet);
void dhcpnak(struct packet *packet);
void state_reboot(void *ipp);
void state_init(void *ipp);
void state_selecting(void *ipp);
void state_panic(void *ipp, UI8_T flag);
void bind_lease(struct interface_info *ip);
void state_bound(void *ipp);
void make_discover(struct interface_info *ip, struct client_lease *lease);
void make_request(struct interface_info *ip, struct client_lease *lease);
void make_decline(struct interface_info *ip, struct client_lease *lease);
void make_release(struct interface_info *ip, struct client_lease *lease);
void send_discover(void *ipp);
void send_request(void *ipp);
void send_decline(void *ipp);
void send_release(void *ipp);
void read_client_conf(UI32_T vid_ifIndex);
void readconf();
void read_client_leases(UI32_T vid_ifIndex);
UI32_T getULong(unsigned char *buf);
UI16_T getUShort(unsigned char *buf);
int cons_options(
	struct packet *inpacket,
	struct dhcp_packet *outpacket,
	int mms,
	struct tree_cache **options,
	int overload,	/* Overload flags that may be set. */
	int terminate,
	int bootpp,
	UI8_T *prl,
	int prl_len);
int store_options(
	unsigned char *buffer,
	int buflen,
	struct tree_cache **options,
	unsigned char *priority_list,
	int priority_len,
	int first_cutoff,
	int second_cutoff,
	int terminate);
struct tree *parse_option_param (UI32_T option_code, DHCP_TYPE_PoolConfigEntry_T *pool_config);

/*struct lease *find_lease (struct packet *packet, struct shared_network *share, int *ours);*/

/* FUNCTION NAME : DHCP_ALGO_ParseNetworkConfig
 * PURPOSE:
 *
 *
 * INPUT:
 *  	group -- Group of declarations that share common parameters
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *		1. This function parses 'subnet'statement in Hornet
 */
void DHCP_ALGO_ParseNetworkConfig(struct group *group);

/* FUNCTION NAME : DHCP_ALGO_ParseHostConfig
 * PURPOSE: Get the host config from WA to OM.
 *
 *
 * INPUT:
 *  	group -- Group of declarations that share common parameters
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *		1. This function does the job for parsing 'host'statement in Hornet
 *		   mimicking parse_host_declaration()
 */
void DHCP_ALGO_ParseHostConfig(struct group *group);

/* FUNCTION	NAME : write_client_lease
 * PURPOSE:
 *		Write client lease from UC or from FS. The lease contains the ip
 *		successfully bound to system before.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		-Penny Chang first created 2002/2/5-
 *		This function will let us set the ip which server gave us
 *
 */
void write_client_lease(UI32_T vid_ifIndex, struct client_lease *lease);

/* FUNCTION	NAME : DHCP_ALGO_GetNaturalSubnetMask
 * PURPOSE:
 *		Get natural subnet mask (class A, class B, and class C)
 * 		according to IP.
 *
 * INPUT:
 *		ip	--	ip address to get natural subnet mask
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		sub_netmask -- the natural subnet mask for the specifiedip
 *
 * NOTES:
 *
 */
UI32_T DHCP_ALGO_GetNaturalSubnetMask(UI32_T ip);

/* FUNCTION	NAME : DHCP_ALGO_IsSocketDirty
 * PURPOSE:
 *      Check socket status and tell DHCP Task to process it right away or not.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE  -- Data in socket, tell DHCP Task process the next right away.
 *      FALSE -- No data in socket, tell DHCP Task run as usual event trigger.
 *
 * NOTES:
 *
 */
BOOL_T DHCP_ALGO_IsSocketDirty(void);

/* FUNCTION NAME : DHCP_ALGO_RemoveProtocol
 * PURPOSE:
 *      Remove all protocols (the interface) to the list of protocols...
 *		
 *
 * INPUT:
 *  	None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
void DHCP_ALGO_RemoveProtocol();

int addr_eq (struct iaddr addr1, struct iaddr addr2);
struct iaddr subnet_number (struct iaddr addr, struct iaddr mask);
char *piaddr (struct iaddr addr);
UI32_T host_addr (struct iaddr addr, struct iaddr mask);
struct iaddr ip_addr (struct iaddr subnet, struct iaddr mask, UI32_T host_address);

#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)

/* FUNCTION	NAME : DHCP_ALGO_Handle_Option82
 * PURPOSE:
 *		
 *
 * INPUT:
 *           vidIfIndex             -- vlan id inindex
 *		mem_ref			--  holder of received packet buffer.
 *		packet_length	--	received packet length.
 *		rxRifNum			--	the RIF packet coming,
 *							-1 if not a configured interface.
 *		dst_mac			--	the destination hardware mac address in frame.
 *		src_mac			--	the source hardware mac address in frame.
 *		vid				--	the vlan the packet coming from.
 *		src_lport_ifIndex-- the ingress physical port ifIndex.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void DHCP_ALGO_Handle_Option82(
				UI32_T vidIfIndex,
				L_MM_Mref_Handle_T *mem_ref, 
				UI32_T packet_length, 
				UI32_T rxRifNum, 
				UI8_T *dst_mac, 
				UI8_T *src_mac, 
				UI32_T vid, 
				UI32_T src_lport_ifIndex);



#endif

/* FUNCTION	NAME : DHCP_MGR_ReleaseClientLease
 * PURPOSE:
 *		Release dhcp active client lease
 *
 * INPUT:
 *		ifindex         --  vlan interface index  
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		TRUE    --  successfully.
 *		FALSE	--  fail.    
 *      
 *
 * NOTES:
 *		this api will free dhcp engine's active lease and send DHCPRELEASE packet to dhcp server.
 *
 */
BOOL_T DHCP_ALGO_ReleaseClientLease(UI32_T ifindex);

/* FUNCTION	NAME : DHCP_ALGO_FreeGlobalUCWriteLease
 * PURPOSE:
 *		Release dhcp global UC write lease
 *
 * INPUT:
 *		ifindex         --  vlan interface index  
 * OUTPUT:
 *		none
 *
 * RETURN:
 *		TRUE    --  successfully.
 *		FALSE	--  fail.    
 *      
 *
 * NOTES:
 *		this api will free dhcp engine's active lease and send DHCPRELEASE packet to dhcp server.
 *
 */
BOOL_T DHCP_ALGO_FreeGlobalUCWriteLease(UI32_T ifindex);

void DHCP_ALGO_SetDebugFlag(UI32_T flag);
void DHCP_ALGO_GetDebugFlag(UI32_T *flag);
	
/* FUNCTION : DHCP_ALGO_UdpChkSumCalculate
 * PURPOSE  : The function calculate the checksum of the UDP header
 * INPUT    : *calculateDataBufferPtr     --  Data buffer which want to calculate checksum.
 *            calculateLengthInHalfWord   --  length of data buffer (in 2bytes length).
 * OUTPUT   : None.
 * RETUEN   : checksum result.
 * NOTES    : None.
 */
UI16_T DHCP_ALGO_UdpChkSumCalculate(DHCP_TYPE_IpHeader_T *ipPktStructure, UI32_T calculateLengthInHalfWord);

#if (SYS_CPNT_DHCP_INFORM)
/* FUNCTION NAME : DHCP_ALGO_MakeInform
 * PURPOSE:
 *      To make DHCP INFORM packet
 *
 * INPUT:
 *       
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *		
 */
void DHCP_ALGO_MakeInform(struct interface_info *ip);


/* FUNCTION NAME : DHCP_ALGO_SendInform
 * PURPOSE:
 *      To send DHCP INFORM packet
 *
 * INPUT:
 *       
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *		
 */
void DHCP_ALGO_SendInform(void *ipp);
#endif

/* FUNCTION	NAME : DHCP_ALGO_SetDhcpPacketRule
 * PURPOSE:
 *		set DHCP server and client packet trap to cpu according to system role
 * INPUT:
 *		None		
 * OUTPUT:
 *		None.
 * RETURN:
 *		TRUE/FALSE
 * NOTES:
 *      L2 product only need to check if it's client;
 *      L3 product need to check if it's client/server/relay
 */
BOOL_T DHCP_ALGO_SetDhcpPacketRule(void);

#endif	 /*	_DHCP_ALGO_H */
