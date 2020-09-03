/* Module Name: DHCP_ALGO.C
 * Purpose:
 *      This module keeps original function of DHCP/IAD, eg. discover_Interface,
 *      dispatch, got_one, do_packet, relay. In DHCP_ALGO, really processes DHCP
 *      protocol function, except timeout management.
 *
 * Notes:
 *      1. Timeout mechanism is checked in DHCP_TASK, and handling routine is keeping
 *         in this module.
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *  0.2 2002.02.23  --  Penny , Modified
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <sys_cpnt.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "dhcp_type.h"
#include "tree.h"
#include "dhcp.h"
#if (SYS_CPNT_BOOTP == TRUE)
#include "bootp.h"
#endif
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "dhcp_txrx.h"
#include "leaf_es3626a.h"
#include "uc_mgr.h"
#include "netcfg_pmgr_main.h"
#include "netcfg_pom_main.h"
#include "netcfg_type.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_ip.h"
#include "dhcp_algo.h"
#include "dhcp_time.h"
#include "dhcp_wa.h"
#include "dhcp_om.h"
#include "dhcp_error_print.h"
#include "dhcp_mgr.h"
#include "dhcp_backdoor.h"
#include "memory.h"
#include "hash.h"
#include "tree.h"
#include "alloc.h"
#include "syslog_pmgr.h"
#include "syslog_type.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "netcfg_type.h"
#include "l_stdlib.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"
#include "ip_lib.h"
#include "swctrl_pmgr.h"
#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)
#include "sys_callback_mgr.h"
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "swctrl_pom.h"
#include "netcfg_pmgr_route.h"
#include "ipal_neigh.h"
#include "iml_pmgr.h"
#endif

#if (SYS_CPNT_ROUTING == TRUE)
#include "amtrl3_pmgr.h"
#endif

#if (SYS_CPNT_AUTO_CONFIG_STATIC_IP == TRUE)
#include "cli_pom.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define UDP_PACKET_BUFFER   2048 		/* for DHCP receive packet buf */
#if !defined (TIME_MAX)
# define TIME_MAX 2147483647
#endif


#define DEFAULT_GATEWAY 0x00000000

#if 0
#define INADDR_ANY 0x0
#endif
#define AF_INET4 AF_INET

static struct iaddr iaddr_broadcast = { 4, { 255, 255, 255, 255 } };
static BOOL_T had_been_logged_to_syslog = FALSE;

#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)
#define DHCP_UDP_PSEUDO_HEADER_LENGTH 12
#define DHCP_Option82_PACKET_LEN		576
#define	DHCP_FRAME_HEADER_LEN		22

static BOOL_T DHCP_ALGO_RelayExceptionCheck(UI32_T vid, struct dhcp_packet *dhcp_packet_p, UI32_T relay_server_ip1);
static BOOL_T DHCP_ALGO_RequestPacketProcess(UI8_T *buffer_request_p, UI32_T *buffer_dhcp_len_p, struct dhcp_packet *dhcp_packet_p,
                                            UI32_T dhcp_packet_len, UI32_T  vid, UI32_T src_lport_ifindex, UI32_T mgmt_intf_ip);
static BOOL_T DHCP_ALGO_FindActiveRelayServer(DHCP_TYPE_IpHeader_T *ip_header_p, UI8_T *destinate_mac,UI32_T *send_out_ifindex, UI32_T *active_relay_server, UI32_T relay_server_list[]);
static BOOL_T DHCP_ALGO_FillInOption82CircuitId(struct dhcp_packet *dhcp_packet_p, UI32_T index, UI32_T vid, UI32_T src_lport_ifIndex, BOOL_T subtype);
static void DHCP_ALGO_FillInOption82RemoteId(struct dhcp_packet *dhcp_packet_p, UI32_T index, UI32_T mode, UI32_T ip_address,BOOL_T subtype);
static BOOL_T DHCP_ALGO_ReplyPacketProcess(struct dhcp_packet *dhcp_packet_p, UI32_T dhcp_packet_len,
                                           UI32_T *client_vlan, UI32_T *client_unit, UI32_T *client_port, UI32_T mgmt_intf_ip);
static BOOL_T DHCP_ALGO_Option82ExistentCheck(struct dhcp_packet *dhcp_packet_p, UI32_T option_len, UI32_T *index, BOOL_T *last_option);
static UI32_T DHCP_ALGO_PacketForward(UI16_T forward_flag, L_MM_Mref_Handle_T *mem_ref_p, UI32_T packet_length, UI8_T *destinate_mac,
                                      UI8_T *source_mac, UI32_T output_vid, UI32_T exclude_lport, UI32_T output_lport);
static unsigned short ipchksum(unsigned short *ip, int len);
static BOOL_T DHCP_ALGO_CalculateOptionLen(struct dhcp_packet *dhcp_packet_p,UI32_T *option_len);
static BOOL_T DHCP_ALGO_CalculateTxBufferSize(
                        struct dhcp_packet *dhcp_packet_p,
                        UI32_T ip_header_len,
                        UI32_T udp_header_len,
                        UI32_T dhcp_packet_len,
                        UI32_T mgmt_intf_ip,
                        UI32_T *buffer_size_p);
static BOOL_T DHCP_ALGO_GetRidValueLen(UI32_T rid_mode, UI32_T mgmt_intf_ip,UI32_T *rid_value_len_p);
#endif
static UI16_T DHCP_ALGO_ChkSumCalculate(UI16_T *calculateDataBufferPtr, UI32_T calculateLengthInHalfWord);

#if (SYS_CPNT_DHCP_INFORM == TRUE)
static void DHCP_ALGO_StoreConfigFromAck(struct packet *packet);
#endif /* SYS_CPNT_DHCP_INFORM */

/* MACRO FUNCTION DECLARATIONS
 */
#define CALCULATE_LENGTH_NO_TRUNCATE(C_LENGTH); {         \
        C_LENGTH = (C_LENGTH % 2) ? C_LENGTH / 2 + 1 : C_LENGTH / 2;             \
    }
/* DATA TYPE DECLARATIONS
 */
typedef void (*BOOTP_PACKET_HANDLER_T) (struct interface_info *,
				     struct dhcp_packet *, int, unsigned int,
				     struct iaddr, struct hardware *);

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void DHCP_ALGO_LeaseExpired(void *ipp);
static void DHCP_ALGO_DeleteLease(void *ipp);
/* STATIC VARIABLE DECLARATIONS
 */
 /* The following 2 structures are used for make_discover() */

 static struct tree_cache *options [256];
 static struct tree_cache option_elements [256];
union
{
	unsigned char packbuf [UDP_PACKET_BUFFER];
	struct dhcp_packet packet;
} u;

/* global lease for read
 */
static struct dhcp_uc_lease *uc_lease_P;

/* global lease for write
 */
static struct dhcp_uc_lease *uc_write_lease_P;
 static int interfaces_invalidated;
 static int socket_num;

 static BOOL_T socket_dirty = FALSE;

/*Now about the inferfaces info is stored in OM */
 static struct protocol *protocols;

 static	BOOTP_PACKET_HANDLER_T bootp_packet_relay_handler, bootp_packet_server_handler;
#if (SYS_CPNT_DHCP_SERVER == TRUE)

 static struct group root_group;
#endif
 static unsigned char buffer [1024];

/* Default dhcp option priority list (this is ad hoc and should not be
   mistaken for a carefully crafted and optimized list). */
static unsigned char dhcp_option_default_priority_list [] =
{
	DHO_DHCP_REQUESTED_ADDRESS,
	DHO_DHCP_OPTION_OVERLOAD,
	DHO_DHCP_MAX_MESSAGE_SIZE,
	DHO_DHCP_RENEWAL_TIME,
	DHO_DHCP_REBINDING_TIME,
	DHO_DHCP_CLASS_IDENTIFIER,
	DHO_DHCP_CLIENT_IDENTIFIER,
	DHO_SUBNET_MASK,
	DHO_TIME_OFFSET,
	DHO_ROUTERS,
	DHO_TIME_SERVERS,
	DHO_NAME_SERVERS,
	DHO_DOMAIN_NAME_SERVERS,
	DHO_HOST_NAME,
	DHO_LOG_SERVERS,
	DHO_COOKIE_SERVERS,
	DHO_LPR_SERVERS,
	DHO_IMPRESS_SERVERS,
	DHO_RESOURCE_LOCATION_SERVERS,
	DHO_HOST_NAME,
	DHO_BOOT_SIZE,
	DHO_MERIT_DUMP,
	DHO_DOMAIN_NAME,
	DHO_SWAP_SERVER,
	DHO_ROOT_PATH,
	DHO_EXTENSIONS_PATH,
	DHO_IP_FORWARDING,
	DHO_NON_LOCAL_SOURCE_ROUTING,
	DHO_POLICY_FILTER,
	DHO_MAX_DGRAM_REASSEMBLY,
	DHO_DEFAULT_IP_TTL,
	DHO_PATH_MTU_AGING_TIMEOUT,
	DHO_PATH_MTU_PLATEAU_TABLE,
	DHO_INTERFACE_MTU,
	DHO_ALL_SUBNETS_LOCAL,
	DHO_BROADCAST_ADDRESS,
	DHO_PERFORM_MASK_DISCOVERY,
	DHO_MASK_SUPPLIER,
	DHO_ROUTER_DISCOVERY,
	DHO_ROUTER_SOLICITATION_ADDRESS,
	DHO_STATIC_ROUTES,
	DHO_TRAILER_ENCAPSULATION,
	DHO_ARP_CACHE_TIMEOUT,
	DHO_IEEE802_3_ENCAPSULATION,
	DHO_DEFAULT_TCP_TTL,
	DHO_TCP_KEEPALIVE_INTERVAL,
	DHO_TCP_KEEPALIVE_GARBAGE,
	DHO_NIS_DOMAIN,
	DHO_NIS_SERVERS,
	DHO_NTP_SERVERS,
	DHO_VENDOR_ENCAPSULATED_OPTIONS,
	DHO_NETBIOS_NAME_SERVERS,
	DHO_NETBIOS_DD_SERVER,
	DHO_NETBIOS_NODE_TYPE,
	DHO_NETBIOS_SCOPE,
	DHO_FONT_SERVERS,
	DHO_X_DISPLAY_MANAGER,
	DHO_DHCP_PARAMETER_REQUEST_LIST,

	/* Presently-undefined options... */
	62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76,
	78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92,
	93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106,
	107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118,
	119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130,
	131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
	143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
	155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166,
	167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178,
	179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190,
	191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202,
	203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
	215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226,
	227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238,
	239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250,
	251, 252, 253, 254,
};

static  int sizeof_dhcp_option_default_priority_list =
	sizeof dhcp_option_default_priority_list;

/* top_level_config is for read_client_conf() */
static struct client_config top_level_config;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : DHCP_ALGO_Init
 * PURPOSE:
 *      Initialize data used in ALGO module.
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
 *      1. All data will re-init in ReStart stage. (Master --> ProvisionComplete)
 */
void DHCP_ALGO_Init(void)
{
    bootp_packet_relay_handler = NULL;
    bootp_packet_server_handler = NULL;
    protocols = NULL;
    uc_lease_P = NULL;
    uc_write_lease_P = NULL;

    if (socket_num > 0)
    {
    	/* 1. Close the socket if we created before for this interface */
		close(socket_num);
    }
    socket_num = 0;
    socket_dirty = FALSE;
}   /*  end of DHCP_ALGO_Init   */

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
 void DHCP_ALGO_SetDispatchFlag(BOOL_T flag)
 {
 	if (flag == FALSE)
 	{
 		/* close socket before freeing protocol */
		if (socket_num > 0)
	    {
	    	/* 1. Close the socket if we created before for this interface */
			close(socket_num);
	    }

 		socket_num = 0;
 	}
 }

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
void DHCP_ALGO_SetupRelayHandler()
{
    bootp_packet_relay_handler = (BOOTP_PACKET_HANDLER_T)relay;

} /* end of DHCP_ALGO_SetupRelayHandler */

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
void DHCP_ALGO_SetupServerHandler()
{
    bootp_packet_server_handler = (BOOTP_PACKET_HANDLER_T)do_packet;

} /* end of DHCP_ALGO_SetupServerHandler */

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
void relay(struct interface_info *ip,
                struct dhcp_packet *packet,
                int length,
                unsigned int from_port,
                struct iaddr from,
                struct hardware *hfrom)
{
#if (SYS_CPNT_ROUTING == TRUE)
    int i;
    struct interface_info *if_ptr_addr, *if_pointer;
	struct  sockaddr_in to, sto;
	struct in_addr from_sockaddr;
	NETCFG_TYPE_InetRifConfig_T rif_config;
	struct  hardware hto;

    if (packet->hlen > sizeof(packet->chaddr))
	{
        DHCP_BD(RELAY, "Discarding packet with invalid hlen[%u]", packet->hlen);
		return;
	}

    DHCP_BD(RELAY, "packet op[%u],flag[%u]", packet->op, packet->flags);

	/* If it's a bootreply, forward it to the client. */
	if (packet->op == BOOTREPLY)
	{
        /* Find the interface that corresponds to the giaddr in the packet. */
    	if_ptr_addr = NULL;
    	if_pointer = DHCP_OM_GetNextInterface(if_ptr_addr);

    	while (if_pointer != NULL)
    	{
    		if(if_pointer->primary_address == packet->giaddr)
    		    break;

    		if_pointer = if_pointer->next;
    	}

        if (if_pointer == NULL)
        	return;

		if (!(packet->flags & L_STDLIB_Hton16(BOOTP_BROADCAST)) )
		{
			to.sin_addr.s_addr = packet->yiaddr;
			to.sin_port = L_STDLIB_Hton16((UI16_T)ip->client_port);
		}
		else
		{

			to.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_BROADCAST);
			to.sin_port = L_STDLIB_Hton16((UI16_T)ip->client_port);

			/* Penny, 2002-6-20: This is a temporary patch for the problem to
			** broadcast packet thru socket
			*/
			to.sin_family = AF_INET;
			/* Set up the hardware destination address. */
			hto.hlen = packet->hlen;

			if (hto.hlen > sizeof hto.haddr)
				hto.hlen = sizeof hto.haddr;

			memcpy(hto.haddr, packet->chaddr, hto.hlen);
			hto.htype = packet->htype;

            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = if_pointer->vid_ifIndex;
			if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) != NETCFG_TYPE_OK)
				return;

            if (memcmp(rif_config.addr.addr, &if_pointer->primary_address,
                        SYS_ADPT_IPV4_ADDR_LEN) != 0)
            {
                return;
            }

			from_sockaddr.s_addr = if_pointer->primary_address;

            DHCP_BD(RELAY, "Send packet to %u.%u.%u.%u, port %u",
                    L_INET_EXPAND_IP(to.sin_addr.s_addr), to.sin_port);
			DHCP_TXRX_SendPktThruIMLToClientPort(if_pointer,
					 		(struct packet *)0,
					 		packet,
					 		length,
					 		from_sockaddr,
							&to,
					 		&hto);
			return;

			/* End of the Patch.Penny, 2002-6-20: This is a temporary patch
			** for the problem to broadcast packet thru socket
			*/
		}

		to.sin_family = AF_INET;

		#ifdef HAVE_SA_LEN
			to.sin_len = sizeof to;
		#endif

		/* Set up the hardware destination address. */
		hto.hlen = packet->hlen;

		if (hto.hlen > sizeof hto.haddr)
			hto.hlen = sizeof hto.haddr;

		memcpy(hto.haddr, packet->chaddr, hto.hlen);

		hto.htype = packet->htype;

         /* if reply packet's bootp flag is unicast,
          * we should add host entry to AMTRL3, then it will add arp entry to kernel to avoid send without arp
          */
		{
			AMTRL3_TYPE_InetHostRouteEntry_T inet_host_route_entry;

		    memset(&inet_host_route_entry, 0, sizeof(inet_host_route_entry));
			inet_host_route_entry.dst_vid_ifindex = if_pointer->vid_ifIndex;
			inet_host_route_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
            inet_host_route_entry.dst_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
			memcpy(inet_host_route_entry.dst_inet_addr.addr, &to.sin_addr, SYS_ADPT_IPV4_ADDR_LEN);
			memcpy(inet_host_route_entry.dst_mac, packet->chaddr, SYS_ADPT_MAC_ADDR_LEN);
			inet_host_route_entry.lport = SYS_ADPT_DESTINATION_PORT_UNKNOWN;
            DHCP_BD(RELAY, "Add host route to AMTRL3\r\n"
                    "vid_ifindex[%lu],ip[%u.%u.%u.%u],mac[%02x-%02x-%02x-%02x-%02x-%02x]",
                    (unsigned long)if_pointer->vid_ifIndex, L_INET_EXPAND_IP(to.sin_addr.s_addr), L_INET_EXPAND_MAC(packet->chaddr));
			AMTRL3_PMGR_SetHostRoute(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB,&inet_host_route_entry , VAL_ipNetToPhysicalExtType_dynamic);
		}

        DHCP_BD(RELAY, "Send packet to %u.%u.%u.%u, port %u",
                L_INET_EXPAND_IP(to.sin_addr.s_addr), to.sin_port);
		send_packet(if_pointer, (struct packet *)0, packet, length, if_pointer->primary_address, &to, &hto);

		return;
	} /* if (packet->op == BOOTREPLY)  */

	/* RFC 1542 4.1.1
     * The relay agent MUST silently discard BOOTREQUEST messages whose
’h   * 'hops' field exceeds the value 16
	*/
	if (packet->hops > 16)
	{
        DHCP_BD(RELAY, "Discarding packet with invalid hops(%u)", packet->hops);
		return;
	}
	else
	{
		packet->hops++;
	}

	/* Penny, 2002-11-21: we still forward this packet to DHCP Server without
	** changing anything in giaddr field.
	*/
	if (packet->giaddr == 0x0)
	{
		/* 2002-11-28, Penny: Check if interface IP has been changed or not
		**	Sync. OM's interface IP with NetCfg
		*/
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ip->vid_ifIndex;
		if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) != NETCFG_TYPE_OK)
			return;

        if (memcmp(rif_config.addr.addr, &ip->primary_address,
                        SYS_ADPT_IPV4_ADDR_LEN) != 0)
		{
            UI32_T ip_address;
            memcpy(&ip_address, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN);
			DHCP_OM_SetIfIp(ip->vid_ifIndex, ip_address);
		}

		/* Set the giaddr so the server can figure out what net it's from and so that
		** we can later forward the response to the correct net.
		*/
		packet->giaddr = ip->primary_address;
	}

	/* Otherwise, it's a BOOTREQUEST, so forward it to all the servers. */

	if (ip->relay_server_list[0] == 0)
		return;

	for(i=0; i<MAX_RELAY_SERVER;i++)
	{
		if (ip->relay_server_list[i] ==0)
			continue;

		sto.sin_addr.s_addr = ip->relay_server_list[i];
		sto.sin_port = L_STDLIB_Hton16((UI16_T)ip->server_port);
		sto.sin_family = AF_INET;

        DHCP_BD(RELAY, "Send packet to %u.%u.%u.%u, port %u",
                L_INET_EXPAND_IP(sto.sin_addr.s_addr), sto.sin_port);
		send_packet(ip, (struct packet *)0, packet, length, ip->primary_address, &sto,
						(struct hardware *)0 ) ;

	}
 #endif
} /* end of relay */

/* FUNCTION NAME : do_packet
 * PURPOSE:
 *      Process DHCP/Bootp packet and call handling routining (dhcp-client, dhcpd,
 *      or bootp-client, bootpd).
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
 *      1. do_packet is static function in ALGO, not export function.
 *      2. client_dhcp, dhcpd, client_bootp, and bootpd are static function in ALGO.
 *
 */
void do_packet(struct interface_info *interface,
                struct dhcp_packet *packet,
                int len,
                unsigned int from_port,
                struct iaddr from,
                struct hardware *hfrom)
{

    struct packet tp;
    UI32_T current_mode;
    int i;

    if (packet->hlen > sizeof(packet->chaddr))
    {
        DHCP_BD(PACKET, "Discarding packet with invalid hlen[%u]", packet->hlen);
        return;
    }

    memset (&tp, 0, sizeof tp);
    tp.raw = packet;
    tp.packet_length = len;
    tp.client_port = from_port;
    tp.client_addr = from;
    tp.interface = interface;
    tp.haddr = hfrom;

    parse_options (&tp);

    if (tp.options_valid &&
        tp.options [DHO_DHCP_MESSAGE_TYPE].data)
        tp.packet_type =
            tp.options [DHO_DHCP_MESSAGE_TYPE].data [0];

    DHCP_BD(PACKET, "Receive packet from %u.%u.%u.%u, port %u\r\n"
            "packet type[%u],op[%u]",
            L_INET_EXPAND_IP(tp.client_addr.iabuf), tp.client_port,
            tp.packet_type, packet->op);

/* check OM's DHCP mode. If the mode == dhcp, we should disgard bootp packet */
 	if(!DHCP_OM_GetIfMode(interface->vid_ifIndex, &current_mode))
 	{
        DHCP_BD(PACKET, "Failed to get vid_ifindex[%lu] address mode", (unsigned long)interface->vid_ifIndex);
 		return;
 	}
 	if (current_mode == DHCP_TYPE_INTERFACE_MODE_DHCP)
 	{
 		if (tp.packet_type)
        	client_dhcp(&tp);
 	}
#if (SYS_CPNT_BOOTP == TRUE)
 	else if (current_mode == DHCP_TYPE_INTERFACE_MODE_BOOTP)
 	{
 		if (tp.packet_type == 0) /*Timon*/
        	client_bootp(&tp);
 	}
#endif
	else if (current_mode == DHCP_TYPE_INTERFACE_MODE_USER_DEFINE)
	{
		/* user-defined address mode*/
#if (SYS_CPNT_DHCP_INFORM == TRUE)
        /* if dhcp inform is enable and recieve dhcp reply packet */
        if((TRUE == interface->dhcp_inform)&&
           (BOOTREPLY == packet->op))
        {
            if(tp.packet_type)
                client_dhcp(&tp);
        }
#endif
		/* role : DHCP Server */
#if	(SYS_CPNT_DHCP_SERVER == TRUE)
	    /* dhcp server only needs to receive dhcp request packet */
        if(BOOTREQUEST == packet->op)
        {
	    if (tp.packet_type)
	    {
	        dhcp (&tp);
	    }
#if (SYS_CPNT_BOOTP == TRUE)
	    else
	        bootp (&tp);
#endif
        }
#endif
    }

    /* Free the data associated with the options. */
    for (i = 0; i < 256; i++)
    {
        if (tp.options [i].len && tp.options [i].data)
            dhcp_free (tp.options [i].data);
    }


}   /*  end of do_packet    */

/* FUNCTION NAME : parse_options
 * PURPOSE:
 *       Parse all available options out of the specified packet.
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
 */
void parse_options(struct packet *packet)
{
    /* Initially, zero all option pointers. */
    memset (packet->options, 0, sizeof (packet->options));

    /* If we don't see the magic cookie, there's nothing to parse. */

    if (memcmp(packet->raw->options, DHCP_OPTIONS_COOKIE, 4))
    {
        packet->options_valid = 0;
        return;
    }

    /* Go through the options field, up to the end of the packet
       or the End field. */
    parse_option_buffer (packet, &packet->raw->options [4],
                 packet->packet_length - DHCP_FIXED_NON_UDP - 4);
    /* If we parsed a DHCP Option Overload option, parse more
       options out of the buffer(s) containing them. */
    if (packet->options_valid
            && packet->options [DHO_DHCP_OPTION_OVERLOAD].data)
    {
        if (packet->options [DHO_DHCP_OPTION_OVERLOAD].data [0] & 1)
            parse_option_buffer (packet,
                         (unsigned char *)
                         packet->raw->file,
                         sizeof packet->raw->file);
        if (packet->options [DHO_DHCP_OPTION_OVERLOAD].data [0] & 2)
            parse_option_buffer (packet,
                         (unsigned char *)
                         packet->raw->sname,
                         sizeof packet->raw->sname);
    }
}

/* FUNCTION NAME : parse_option_buffer
 * PURPOSE:
 *      Parse options out of the specified buffer, storing addresses of option
 *      values in packet->options and setting packet->options_valid if no
 *      errors are encountered.
 *
 * INPUT:
 *      packet -- the raw packet we receive
 *      buffer -- the option field buffur
 *      length -- length of the buffer
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
void parse_option_buffer(struct packet *packet, unsigned char *buffer,int length)
{
    unsigned char *s, *t;
    unsigned char *end = buffer + length;
    int len;
    int code;

    for (s = buffer; *s != DHO_END && s < end; )
    {
        code = s [0];
        /* Pad options don't have a length - just skip them. */
        if (code == DHO_PAD)
        {
            ++s;
            continue;
        }
        /* All other fields (except end, see above) have a
           one-byte length. */
        len = s [1];

        /* If the length is outrageous, the options are bad. */
        if (s + len + 2 > end)
        {
        	/* syslog: overflows input buffer*/
            DHCP_BD(PACKET, "Option length overflows input buffer");
            packet->options_valid = 0;
            return;
        }
        /* If we haven't seen this option before, just make
           space for it and copy it there. */
        if (!packet->options [code].data)
        {
            if (!(t = ((unsigned char *) dhcp_malloc(len + 1) )))
            {
                /* add to syslog */
                DHCP_BD(PACKET, "Failed to allocate storage for option");
                return;
            }
            else
            {
	            /* Copy and NUL-terminate the option (in case it's an
	               ASCII string. */
	            memcpy (t, &s [2], len);
	            t [len] = 0;
	            packet->options [code].len = len;
	            packet->options [code].data = t;
			}
        }
        else
        {
            /* If it's a repeat, concatenate it to whatever
               we last saw.   This is really only required
               for clients, but what the heck... */
            t = ((unsigned char *)
                 dhcp_malloc (len + packet->options [code].len + 1));
            if (!t)
            {
                /* Eventually replace with syslog */
                DHCP_BD(PACKET, "Failed to expand storage for option");
                return;
            }
            else
            {
	            memcpy (t, packet->options [code].data,
	                packet->options [code].len);
	            memcpy (t + packet->options [code].len,
	                &s [2], len);
	            packet->options [code].len += len;
	            t [packet->options [code].len] = 0;
	            dhcp_free(packet->options [code].data);
	            packet->options [code].data = t;
	        }
        }
        s += len + 2;
    }
    packet->options_valid = 1;

}

/* FUNCTION NAME : client_dhcp
 * PURPOSE:
 *       DHCP protocol
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
void client_dhcp(packet)
    struct packet *packet;
{
    /* struct iaddrlist *ap; */
    void (*handler) (struct packet *);

	/* Check if this packet is for client */

    switch (packet->packet_type)
    {
    case DHCPOFFER:
        handler = dhcpoffer;
        break;

    case DHCPNAK:
        handler = dhcpnak;
        break;

    case DHCPACK:
        handler = dhcpack;
        break;

    default:
        return;
    }


#if 0 /* Penny 2002-1-4 */
	    /* If there's a reject list, make sure this packet's sender isn't
	       on it. */
	for (ap = packet -> interface -> client -> config -> reject_list;
            ap; ap = ap->next)
    {
        if (addr_eq(packet->client_addr, ap->addr))
        {
            return;
        }
    }
#endif

	(*handler) (packet);
} /* end of client_dhcp */

#if (SYS_CPNT_BOOTP == TRUE)
/* FUNCTION NAME : client_bootp
 * PURPOSE:
 *       BOOTP protocol
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
 *      Be aware, this function now only for dhcp client use
 */
void client_bootp(packet)
    struct packet *packet;
{
    /*struct iaddrlist *ap;*/

    if (packet->raw->op != BOOTREPLY)
        return;
#if 0 /* Penny 2002-1-4 */
    /* If there's a reject list, make sure this packet's sender isn't
       on it. */
    for (ap = packet->interface->client->config->reject_list;
            ap; ap = ap->next)
    {
        if (addr_eq(packet->client_addr, ap->addr))
        {
            return;
        }
    }
#endif
    dhcpoffer(packet);
}
#endif /* #if (SYS_CPNT_BOOTP == TRUE) */


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
void dispatch()
{

#if ((SYS_CPNT_DHCP_SERVER == TRUE) || (SYS_CPNT_DHCP_RELAY == TRUE))
    fd_set r;
	int max = 0;
	int count;
	int on =1;
	struct timeval	wait_time;
	struct sockaddr_in name;

    /* Check if socket has been closed in error condition, create again for DHCP */

	if (socket_num == 0)
	{
		name.sin_family = AF_INET;
		name.sin_port 	= 	L_STDLIB_Hton16(67);
		name.sin_addr.s_addr 	= L_STDLIB_Hton32(INADDR_ANY);

		if ((socket_num = socket (AF_INET, SOCK_DGRAM, DHCP_TYPE_IP_PROT_UDP)) < 0)
		{
            DHCP_BD(PACKET, "Failed to create DHCP socket.");
    	    return;
		}
		if(setsockopt(socket_num, SOL_SOCKET, SO_REUSEADDR,(void *)&on, sizeof(on)) < 0)
	    {
            DHCP_BD(PACKET, "Failed to set SO_REUSEADDR socket option.");
	        close(socket_num);
	        return;
	    }

        if (setsockopt(socket_num, IPPROTO_IP, IP_PKTINFO, (void *)&on, sizeof(on)) < 0)
        {
            DHCP_BD(PACKET, "Failed to set IP_PKTINFO socket option.");
            close(socket_num);
	        return;
        }



		if (bind (socket_num, (struct sockaddr *)&name, sizeof name) < 0)
		{
            DHCP_BD(PACKET, "Failed to bind socket.");
            close(socket_num);
			return;
		}

	}

    memset(&wait_time, 0, sizeof wait_time);
	//wait_time.tv_sec = 0;
    //wait_time.tv_usec = 600000;

		/* Set up the read mask. */
	FD_ZERO (&r);
	FD_SET (socket_num, &r);

	if (socket_num > max)
		max = socket_num;


	/* Wait for a packet for a timeout... XXX */
	count = select (max + 1, &r, NULL, NULL, &wait_time);

	/* Not likely to be transitory... */
	if (count <= 0)
	{
                socket_dirty = FALSE;
		return;
	}
	else
	{

		if (!FD_ISSET (socket_num, &r))
		{
	                socket_dirty = FALSE;
			return;
		}

		got_one();

		interfaces_invalidated = 0;
	}
#endif

} /* end of dispath */

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
void reinitialize_interfaces ()
{
	interfaces_invalidated = 1;
}

/* FUNCTION NAME : got_one
 * PURPOSE:
 *      Retrieve one packet from specified interface and call do_packet() to handle
 *      this packet.
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
void got_one()
{
    struct sockaddr_storage from;
	struct hardware hfrom;
	struct iaddr ifrom;
	struct interface_info *ip;
	char rbuf[UDP_PACKET_BUFFER]={0};
    struct msghdr mhdr;
    struct cmsghdr *cmsg;
    struct iovec iov;
    ssize_t len;
    struct cmsghdr *cm;
    struct in_pktinfo *pktinfo=NULL;
    char pktinfo_buf[sizeof (*cmsg) + sizeof (*pktinfo)];
    struct dhcp_packet *dhcp_packet;
    struct sockaddr_in *sa;
	NETCFG_TYPE_InetRifConfig_T rif_config;

    iov.iov_base = (caddr_t) rbuf;
    iov.iov_len = sizeof(rbuf);
    mhdr.msg_name = (caddr_t) &from;
    mhdr.msg_namelen = sizeof(from);
    mhdr.msg_iov = &iov;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = pktinfo_buf;
    mhdr.msg_controllen = sizeof (*cmsg) + sizeof (*pktinfo);


    if ((len = recvmsg(socket_num, &mhdr, 0)) < 0)
    {
		socket_dirty = FALSE;
		return;
	}

    /* detect receiving interface */
    for (cm = (struct cmsghdr *) CMSG_FIRSTHDR(&mhdr); cm;
            cm = (struct cmsghdr *) CMSG_NXTHDR(&mhdr, cm))
    {
        if ((cm->cmsg_level == IPPROTO_IP)&&
            (cm->cmsg_type == IP_PKTINFO))
	    {
            pktinfo = (struct in_pktinfo *)(CMSG_DATA(cm));
        }
    }

    if (pktinfo == NULL)
	{
        DHCP_BD(PACKET, "null packet info.");
	    socket_dirty = FALSE;
		return;
	}
	else
    {
	    socket_dirty = TRUE;
    }

    /* get dhcp packet */
    dhcp_packet = (struct dhcp_packet *) rbuf;

    /* get sa */
    sa = (struct sockaddr_in *) &from;

    /* get incoming interface */
    rif_config.ifindex = pktinfo->ipi_ifindex;

    DHCP_BD(PACKET, "Incoming interface[%lu]", (unsigned long)rif_config.ifindex);


	if ((bootp_packet_relay_handler) && (DHCP_OM_IsRelayOn() == TRUE) )
	{
		/* 1. Check if it is a bootrequest, and incoming interface is not
		** 	the one we expect, drop the packet
		*/
        if (dhcp_packet->op == BOOTREQUEST)
		{
			/* 2003-5-20, Penny: Find the packet-incoming interface.
			*/

			if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) != NETCFG_TYPE_OK)
            {
                DHCP_BD(PACKET, "Failed to get primary rif from interface.");
				return;
            }

			if ((ip = DHCP_OM_FindInterfaceByVidIfIndex(rif_config.ifindex)) == NULL)
			{
                DHCP_BD(PACKET, "Failed to get dhcp interface.");
				return;
			}
		}
		else
		{
            memset(&rif_config, 0, sizeof(rif_config));
            memcpy(rif_config.addr.addr, &(dhcp_packet->giaddr), SYS_ADPT_IPV4_ADDR_LEN);

            rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
            rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
			if (NETCFG_POM_IP_GetRifFromIp (&rif_config) == NETCFG_TYPE_OK)
			{
				if ((ip = DHCP_OM_FindInterfaceByVidIfIndex(rif_config.ifindex)) == NULL)
				{
                    DHCP_BD(PACKET, "Failed to get dhcp interface.");
					return;
				}
			}
			else
            {
                DHCP_BD(PACKET, "Failed to get rif from interface.");
				return;
	    	}
		}


		ifrom.len = 4;
		memcpy (ifrom.iabuf, &(sa->sin_addr), ifrom.len);
		(*bootp_packet_relay_handler) (ip, dhcp_packet, len,
					 sa->sin_port, ifrom, &hfrom);
	}

	/* Penny, 2002/6/3: enable this part while we implement server */
	/* Penny, 2002/11/28: Record the packet-incoming interface which will help
	** 	server to choose the best lease for client
	*/

	if ((bootp_packet_server_handler) && (DHCP_OM_IsServerOn() == TRUE) )
	{
		if (NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) != NETCFG_TYPE_OK)
        {
            DHCP_BD(PACKET, "Failed to get primary rif from interface.");
			return;
        }

		if ((ip = DHCP_OM_FindInterfaceByVidIfIndex(rif_config.ifindex)) == NULL)
        {
            DHCP_BD(PACKET, "Failed to get dhcp interface.");
			return;
        }

		ifrom.len = 4;

		memcpy (ifrom.iabuf, &(sa->sin_addr), ifrom.len);

		(*bootp_packet_server_handler) (ip, dhcp_packet, len,
					 sa->sin_port, ifrom, &hfrom);
	}


}    /*  end of got_one  */



/* FUNCTION NAME : discover_interfaces
 * PURPOSE:
 *      Discover all available interfaces for DHCP.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE -- successfully discover at lease one interface
 *		FALSE -- Fail to discover any interface
 *
 * NOTES:
 *      1. This function retrieve interface from NETCFG and build up protocol list.
 *      2. This function must go with send_packet, receive_packet.
 *      3. Notes from DHCP/IAD :
 *          Use the SIOCGIFCONF ioctl to get a list of all the attached interfaces.
 *          For each interface that's of type INET and not the loopback interface,
 *          register that interface with the network I/O software, figure out what
 *          subnet it's on, and add it to the list of interfaces.
 *      4. After get interface ip from WA (working area), this function will also set the
 *          value to DHCP_OM
 *
 */
BOOL_T discover_interfaces(UI32_T restart_object)
{

    struct interface_info *tmp, *ifp;
    struct hardware hw_address;
    struct iaddr addr;
    UI32_T ret, tmp_vidIfIndex;
    UI8_T mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T addr_len = SYS_ADPT_MAC_ADDR_LEN; /* for MAC is 6 */
    UI32_T vid_ifIndex;
    UI32_T primary_address;
    UI32_T address_mode;
    DHCP_WA_InterfaceDhcpInfo_T if_config;
	DHCP_TYPE_ClientId_T cid;
    DHCP_TYPE_Vendor_T   classid;
#if (SYS_CPNT_DHCP_SERVER == TRUE)
    struct subnet *subnet;
    struct shared_network *share;
#endif

	NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T     zero_ip[SYS_ADPT_IPV4_ADDR_LEN] = {0};

    DHCP_BD(EVENT, "restart object[%lu]", (unsigned long)restart_object);

    memset(mac, 0, SYS_ADPT_MAC_ADDR_LEN);
    memset(&hw_address,0, sizeof(hw_address));
    memset(&addr, 0, sizeof(addr));

    /*1. Build up the interface info for client first; for client, an interface could
    **	have no ip bind to it yet. So we find out the interface which user has been
    **  config to DHCP Client or BOOTP Client.
    */
 	if(restart_object == DHCP_TYPE_RESTART_CLIENT)
 	{
 	    vid_ifIndex = 0; /* get the 1st entry */
            while (NETCFG_POM_IP_GetNextIpAddressMode(&vid_ifIndex, &address_mode)== NETCFG_TYPE_OK)
	    {
		    /* We only configure the interface which is DHCP or BOOTP address mode
             */
                if ((address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_DHCP)
#if (SYS_CPNT_BOOTP == TRUE)
                    ||(address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP)
#endif
                    )
                {
                    ifp = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);

    		    if (ifp == NULL)
    		    {
    		        /*  Create a interface and link to it */
    		        if (! DHCP_OM_CreateIf (vid_ifIndex))
    		        {
    		            return FALSE;
    		        }
    		    }

    		    DHCP_OM_SetIfMode(vid_ifIndex, address_mode);

    		    if (VLAN_PMGR_GetVlanMac(vid_ifIndex, mac) == FALSE)
    		     	return FALSE;

    		    addr_len = SYS_ADPT_MAC_ADDR_LEN;
    		    hw_address.htype = HTYPE_ETHER;
    		    hw_address.hlen  = addr_len;
    		    memcpy (hw_address.haddr, mac, addr_len);
    		    DHCP_OM_SetIfHwAddress(vid_ifIndex, hw_address);

    		    /* Now move WA interface info to OM */
    		    memset(&if_config, 0, sizeof(DHCP_WA_InterfaceDhcpInfo_T));
    		    DHCP_WA_GetIfConfig(vid_ifIndex,&if_config);

    		    if (if_config.if_ip && (if_config.if_ip != 0))
    		    	primary_address = if_config.if_ip;
    		    else
    		    	primary_address = 0x0;   /*penny added 2002-1-5 */

    		    DHCP_OM_SetClientIfConfig(vid_ifIndex, primary_address, if_config.if_server_ip, if_config.if_gateway);
    		    DHCP_OM_SetIfPort (vid_ifIndex, if_config.client_port, if_config.server_port);
    		    DHCP_OM_SetIfBindingRole(vid_ifIndex, DHCP_TYPE_BIND_CLIENT);

    		    /* 2002-7-16, Penny added for CID issue */
    		    if (DHCP_WA_C_GetIfClientId(vid_ifIndex, (DHCP_MGR_ClientId_T *)&cid))
    		    {
    		    	/* User did the configuration for CID */
    		    	if(cid.id_mode != 0 && cid.id_len != 0)
    		    	{
    		    		DHCP_OM_SetIfClientId(vid_ifIndex, cid);
    		    	}
    		    }
    		         /* begin 2007-12, Joseph */
    		    if (DHCP_WA_C_GetIfVendorClassId(vid_ifIndex, (DHCP_MGR_Vendor_T *)&classid))
    		    {
    		        /* User did the configuration for Class ID */
    			DHCP_OM_SetIfVendorClassId(vid_ifIndex, classid);
    		    }
                }
#if (SYS_CPNT_DHCP_INFORM == TRUE)
            if (address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
            {
                BOOL_T dhcp_inform = FALSE;
                /* check if dhcp inform is set and if it has primary address */
                memset(&rif_config, 0, sizeof(rif_config));
                rif_config.ifindex =vid_ifIndex;
                if((NETCFG_TYPE_OK == NETCFG_POM_IP_GetDhcpInform(vid_ifIndex, &dhcp_inform))&&
                   (NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config)))
                {
                    /* if dhcp inform is set, we should create dhcp interface */
                    if(dhcp_inform)
                    {
                        ifp = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);

            		    if (ifp == NULL)
            		    {
            		        /*  Create a interface and link to it */
            		        if (! DHCP_OM_CreateIf (vid_ifIndex))
            		        {
            		            return FALSE;
            		        }
            		    }

            		    DHCP_OM_SetIfMode(vid_ifIndex, address_mode);

            		    if (VLAN_PMGR_GetVlanMac(vid_ifIndex, mac) == FALSE)
            		     	return FALSE;

            		    addr_len = SYS_ADPT_MAC_ADDR_LEN;
            		    hw_address.htype = HTYPE_ETHER;
            		    hw_address.hlen  = addr_len;
            		    memcpy (hw_address.haddr, mac, addr_len);
            		    DHCP_OM_SetIfHwAddress(vid_ifIndex, hw_address);

            		    /* Now move WA interface info to OM */
            		    memset(&if_config, 0, sizeof(DHCP_WA_InterfaceDhcpInfo_T));
            		    if(FALSE == DHCP_WA_GetIfConfig(vid_ifIndex,&if_config))
                            DHCP_BD(EVENT, "Failed to get interface from WA");

                        /* set primary ip address to om interface */
                        memcpy(&primary_address, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN);

            		    DHCP_OM_SetClientIfConfig(vid_ifIndex, primary_address, if_config.if_server_ip, if_config.if_gateway);
            		    DHCP_OM_SetIfPort (vid_ifIndex, if_config.client_port, if_config.server_port);

            		    DHCP_OM_SetIfBindingRole(vid_ifIndex, DHCP_TYPE_BIND_CLIENT);
                        DHCP_OM_SetIfDhcpInform(vid_ifIndex, dhcp_inform);

            		    if (DHCP_WA_C_GetIfClientId(vid_ifIndex, (DHCP_MGR_ClientId_T *)&cid))
            		    {
            		    	/* User did the configuration for CID */
            		    	if(cid.id_mode != 0 && cid.id_len != 0)
            		    	{
            		    		DHCP_OM_SetIfClientId(vid_ifIndex, cid);
            		    	}
            		    }

            		    if (DHCP_WA_C_GetIfVendorClassId(vid_ifIndex, (DHCP_MGR_Vendor_T *)&classid))
            		    {
            		        /* User did the configuration for Class ID */
            			    DHCP_OM_SetIfVendorClassId(vid_ifIndex, classid);

            		    }

                    }
                }
            }
#endif /* SYS_CPNT_DHCP_INFORM */
	    } /* end of while (NETCFG_MGR_GetNextIpAddressMode(&vid_ifIndex, &address_mode)) */
 	}


  	/* 2. Now set the interface for Server and Relay (setting up socket): Due to for
  	** server and relay the interface must contains IP, so we get the vid_ifIndex from
  	** Rif config table
  	*/
  	if(restart_object == DHCP_TYPE_RESTART_RELAY || restart_object == DHCP_TYPE_RESTART_SERVER)
  	{
        memset(&rif_config, 0, sizeof(rif_config));
   		while (NETCFG_POM_IP_GetNextRifConfig(&rif_config)==NETCFG_TYPE_OK)
  		{
	      	tmp_vidIfIndex = rif_config.ifindex;

		    ifp = DHCP_OM_FindInterfaceByVidIfIndex(tmp_vidIfIndex);

		    if (ifp == NULL)
		    {
			    if (! DHCP_OM_CreateIf (tmp_vidIfIndex))
			    {
			        return FALSE;
			    }
		    }

		    /* find MAC address of that vlan */
		    if (VLAN_PMGR_GetVlanMac(tmp_vidIfIndex, mac) == FALSE)
		    	return FALSE;
	        addr_len = SYS_ADPT_MAC_ADDR_LEN;

			 /* Penny: 2002-10-07:
		    ** Check this interface's ip address method, if it is a bootp /
		    ** DHCP, we can't register any subnet on it and can't bind any socket
		    ** on it. Reason: We don't allow interface which is a DHCP/BOOTP client
		    ** perform any DHCP Server behavior.
		    */
			ret = NETCFG_POM_IP_GetIpAddressMode(rif_config.ifindex, &address_mode);
			if (ret != NETCFG_TYPE_OK)
			{
                DHCP_BD(EVENT, "Failed to get ip address mode for vid_ifIndex[%lu]", (unsigned long)rif_config.ifindex);
				continue;
			}
			else
			{
				if (address_mode != NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
					continue;

			}

		    DHCP_OM_SetIfMode(tmp_vidIfIndex, address_mode);

		    hw_address.htype = HTYPE_ETHER;
		    hw_address.hlen  = addr_len;
		    memcpy (hw_address.haddr, mac, addr_len);
		    DHCP_OM_SetIfHwAddress(tmp_vidIfIndex, hw_address);

			memset(&if_config, 0 , sizeof(DHCP_WA_InterfaceDhcpInfo_T));
		    DHCP_WA_GetIfConfig(tmp_vidIfIndex, &if_config);

		    if (memcmp(rif_config.addr.addr, zero_ip, SYS_ADPT_IPV4_ADDR_LEN)== 0)
		    	continue;
	        memcpy(&primary_address, rif_config.addr.addr, SYS_ADPT_IPV4_ADDR_LEN);

		    if (!DHCP_OM_SetIfIp(tmp_vidIfIndex, primary_address))
				return FALSE;

			/* Grab the address... */
			addr.len = 4;
			memcpy(addr.iabuf, &primary_address, addr.len);

			/*retrieve interface info from dhcp om  */
			tmp = DHCP_OM_FindInterfaceByVidIfIndex(tmp_vidIfIndex);
			if(NULL == tmp)
			{
				DHCP_BD(EVENT, "Can't retrive dhcp interface from DHCP_OM.");
				continue;
			}

			DHCP_OM_SetIfBindingRole(rif_config.ifindex, DHCP_TYPE_BIND_NONE);
			/* If there's a registered subnet for this address,
		     * connect it together...
		     */
#if (SYS_CPNT_DHCP_SERVER == TRUE)
			if(restart_object == DHCP_TYPE_RESTART_SERVER)
			{
				if ((subnet = find_subnet (addr)))
				{
					/*If subnet can be find */
					/* If this interface has multiple aliases
					 * on the same subnet, ignore all but the
					 * first we encounter.
					*/
					if (!subnet->interface)
					{
						subnet->interface = tmp;
						subnet->interface_address = addr;
					}
					else if (subnet->interface != tmp)
                        DHCP_BD(EVENT, "Multiple interfaces match the same subnet.");

					share = subnet->shared_network;
					if (tmp->shared_network &&
						tmp->shared_network != share)
					{
                        DHCP_BD(EVENT, "Interface matches multiple shared networks.");
					}
					else
					{
						tmp->shared_network = share;
					}

					if (!share->interface)
					{
						share->interface = tmp;
					}
					else if (share->interface != tmp)
					{
                        DHCP_BD(EVENT, "Multiple interfaces match the same shared network");
					}

					/* Penny: 2002-10-07: Mark this interface role as server */
					DHCP_OM_SetIfBindingRole(rif_config.ifindex, DHCP_TYPE_BIND_SERVER);
				}
				else
					DHCP_OM_SetIfBindingRole(rif_config.ifindex, DHCP_TYPE_BIND_NONE);
			}
#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
			if(restart_object == DHCP_TYPE_RESTART_RELAY)
			{
				UI32_T relay_server[MAX_RELAY_SERVER] = {0};

				if (if_config.if_binding_role & DHCP_TYPE_BIND_RELAY)
			    {
			    	memcpy(relay_server, if_config.relay_server, sizeof(UI32_T)*MAX_RELAY_SERVER);

			    	if (!DHCP_OM_SetIfRelayServerAddress(tmp_vidIfIndex, relay_server))
						return FALSE;

					DHCP_OM_SetIfBindingRole(tmp_vidIfIndex, DHCP_TYPE_BIND_RELAY);
			    }
			}
#endif
#if 0
		    /* If we're just trying to get a list of interfaces that we might
		       be able to configure, we can quit now. */
		       /*   if (state == DISCOVER_UNCONFIGURED)
		        return; */

		    /* Find subnets that don't have valid interface
			   addresses... */
			for (subnet = (tmp->shared_network
				       ? tmp->shared_network->subnets
				       : (struct subnet *)0);
			     subnet; subnet = subnet->next_sibling)
			{
				if (!subnet->interface_address.len)
				{
					/* Set the interface address for this subnet
					   to the first address we found. */
					subnet->interface_address.len = 4;
					memcpy (subnet->interface_address.iabuf,
						&primary_address, 4);
				}
			}
#endif

		}
  	}


    /* Set rule to trap to cpu */
    if(FALSE == DHCP_ALGO_SetDhcpPacketRule())
        return FALSE;

	return TRUE;
}   /*  end of discover_interfaces  */


/* FUNCTION NAME : DHCP_ALGO_LeaseExpired
 * PURPOSE:
 *      Set interface Ip to default IP and mask value while
 *			dhcp lease time expired.
 *
 * INPUT:
 *  	ipp -- the pointer to interface info.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 */
static void DHCP_ALGO_LeaseExpired(void *ipp)
{
    struct interface_info *ip = ipp;
    NETCFG_TYPE_InetRifConfig_T rif_config;
	UI32_T address_mode = 0;

    if (ip == NULL)
    	return;

    DHCP_BD(CLIENT, "vid_ifindex[%lu],ip[%u.%u.%u.%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(ip->primary_address));

    rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
    rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    rif_config.addr.preflen =0;
    if (ip->vid_ifIndex != 0)
    {
        if ((ip->client != NULL) && (ip->client->active != NULL))
        	memcpy (rif_config.addr.addr, ip->client->active->address.iabuf, SYS_ADPT_IPV4_ADDR_LEN);
        if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ip->vid_ifIndex, &address_mode))
        {
            return;
        }
        if(NETCFG_TYPE_IP_ADDRESS_MODE_DHCP !=address_mode
#if (SYS_CPNT_BOOTP == TRUE)
           && NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP!=address_mode
#endif
          )
        {
            return;
        }
        /* Get Rif from IP */
        if (NETCFG_POM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
        {
            rif_config.row_status  = VAL_netConfigStatus_2_destroy;
        	rif_config.ipv4_role   = NETCFG_TYPE_MODE_PRIMARY;

        	if (NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC) != NETCFG_TYPE_OK)
            {
                DHCP_BD(CLIENT, "Failed to bind ip to the system.");
            }
            ip->primary_address = 0;
        }
        DHCP_TIME_cancel_timeout (DHCP_ALGO_LeaseExpired, ip);
        DHCP_MGR_DeleteClientDefaultGateway(ip->vid_ifIndex);
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
        DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex);
#endif /*#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)*/
        /* free lease in UC */
        DHCP_ALGO_FreeGlobalUCWriteLease(ip->vid_ifIndex);
    }

}/* end of DHCP_ALGO_LeaseExpired */


/* FUNCTION NAME : DHCP_ALGO_DeleteLease
 * PURPOSE:
 *      Set interface Ip to default IP and mask value while
 *			delete dhcp lease.
 *
 * INPUT:
 *  	ipp -- the pointer to interface info.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      This API is only used for dhcp release
 */
static void DHCP_ALGO_DeleteLease(void *ipp)
{
    struct interface_info *ip = ipp;
    NETCFG_TYPE_InetRifConfig_T rif_config;

    if (ip == NULL)
    	return;

    DHCP_BD(CLIENT, "vid_ifindex[%lu],ip[%u.%u.%u.%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(ip->primary_address));

    rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
    rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    rif_config.addr.preflen =0;
    if (ip->vid_ifIndex != 0)
    {
        /* copy active lease ip */
        if ((ip->client != NULL) && (ip->client->active != NULL))
        	memcpy (rif_config.addr.addr, ip->client->active->address.iabuf, SYS_ADPT_IPV4_ADDR_LEN);

        /* Get Rif from active lease ip */
        if (NETCFG_POM_IP_GetRifFromIp(&rif_config) == NETCFG_TYPE_OK)
        {
            rif_config.row_status  = VAL_netConfigStatus_2_destroy;
        	rif_config.ipv4_role   = NETCFG_TYPE_MODE_PRIMARY;

        	if (NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC) != NETCFG_TYPE_OK)
            {
                DHCP_BD(CLIENT, "Failed to bind ip to the system.");
            }
                    ip->primary_address = 0;
        }
        DHCP_TIME_cancel_timeout (DHCP_ALGO_LeaseExpired, ip);
        DHCP_MGR_DeleteClientDefaultGateway(ip->vid_ifIndex);
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)
        DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex);
#endif /*#if (SYS_CPNT_DNS_FROM_DHCP == TRUE || SYS_CPNT_DNS == TRUE)*/
        /* free lease in UC */
        DHCP_ALGO_FreeGlobalUCWriteLease(ip->vid_ifIndex);
    }

}/* end of DHCP_ALGO_LeaseExpired */


/* Allocate a client_lease structure and initialize it from the parameters
   in the specified packet. */

struct client_lease *packet_to_lease(packet)
    struct packet *packet;
{
    struct client_lease *lease;
    int i;

    lease = (struct client_lease *)dhcp_malloc (sizeof (struct client_lease));

    if (!lease)
    {
        DHCP_BD(CLIENT, "No memory to record lease");
        return (struct client_lease *)0;
    }

    memset (lease, 0, sizeof *lease);

    /* Copy the lease options. */

    for (i = 0; i < 256; i++)
    {
        if (packet->options[i].len)
        {
            lease->options [i].data =
                (unsigned char *)
                    dhcp_malloc (packet->options [i].len + 1);

            if (!lease->options[i].data)
            {
                DHCP_BD(CLIENT, "No memory for option");
                free_client_lease (lease);
                return (struct client_lease *)0;
            }
            else
            {
                memcpy (lease->options [i].data,
                    packet->options [i].data,
                    packet->options [i].len);
                lease->options [i].len =
                    packet->options [i].len;
                lease->options [i].data
                    [lease->options [i].len] = 0;
            }
        }
    }

    lease->address.len = sizeof (packet->raw->yiaddr);

    memcpy(lease->address.iabuf, &packet->raw->yiaddr, lease->address.len);

    /* If the server name was filled out, copy it. */

    if ((!packet->options [DHO_DHCP_OPTION_OVERLOAD].len ||
         !(packet->options [DHO_DHCP_OPTION_OVERLOAD].data [0] & 2)) &&
            packet->raw->sname [0])
    {
        int len;
        /* Don't count on the NUL terminator. */
        for (len = 0; len < 64; len++)
            if (!packet->raw->sname [len])
                break;
        lease->server_name = (char *) dhcp_malloc (len + 1);

        if (!lease->server_name)
        {
            DHCP_BD(CLIENT, "No memory for filename");
            free_client_lease (lease);
            return (struct client_lease *)0;
        }
        else
        {
            memcpy(lease->server_name, packet->raw->sname, len);
            lease->server_name [len] = 0;
        }
    }

    /* Ditto for the filename. */
    if ((!packet->options [DHO_DHCP_OPTION_OVERLOAD].len ||
         !(packet->options [DHO_DHCP_OPTION_OVERLOAD].data [0] & 1)) &&
            packet->raw->file [0])
    {
        int len;
        /* Don't count on the NUL terminator. */
        for (len = 0; len < 64; len++)
            if (!packet->raw->file [len])
                break;
        lease->filename = (char *)dhcp_malloc (len + 1);

        if (!lease->filename)
        {
            DHCP_BD(CLIENT, "No memory for filename");
            free_client_lease (lease);
            return (struct client_lease *)0;
        }
        else
        {
            memcpy(lease->filename, packet->raw->file, len);
            lease->filename [len] = 0;
        }
    }
    return lease;
}
void free_client_lease(lease)
    struct client_lease *lease;
{
    int i;

    if (lease == NULL)
    	return;
    if (lease->next != NULL)
    {
    	free_client_lease(lease->next);
        lease->next = NULL;
    }

    if (lease->server_name)
        dhcp_free (lease->server_name);
    if (lease->filename)
        dhcp_free (lease->filename);

    for (i = 0; i < 256; i++)
    {
        if ((lease->options[i].data)&&(lease->options [i].len))
            dhcp_free (lease->options [i].data);
    }
    dhcp_free(lease);
}

void dhcpoffer(packet)
    struct packet *packet;
{
    struct interface_info *ip = packet->interface;
    struct client_lease *lease, *lp;
    int i;
    int arp_timeout_needed;
    TIME cur_time;
    I32_T stop_selecting;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }


    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    /* If we're not receptive to an offer right now, or if the offer
       has an unrecognizable transaction id, then just drop it. */
    if (ip->client->state != S_SELECTING ||
        packet->interface->client->xid != packet->raw->xid ||
        (packet->interface->hw_address.hlen !=
         packet->raw->hlen) ||
        (memcmp (packet->interface->hw_address.haddr,
             packet->raw->chaddr, packet->raw->hlen)))
        return;

    /* If this lease doesn't supply the minimum required parameters,
     *  blow it off.
     */
    for (i = 0; ip->client->config->required_options [i]; i++)
    {
        if (!packet->options [ip->client->config->required_options[i]].len)
            return;
    }

    /* If we've already seen this lease, don't record it again. */
    for (lease = ip->client->offered_leases;
            lease; lease = lease->next)
    {
        if (lease->address.len == sizeof packet->raw->yiaddr &&
            !memcmp (lease->address.iabuf,
                        &packet->raw->yiaddr, lease->address.len))
        {
            DHCP_BD(CLIENT, "Lease already seen. Don't record it again.");
            return;
        }
    }

    lease = packet_to_lease (packet);

    if (!lease)
    {
        DHCP_BD(CLIENT, "Failed to transfer packet to lease");
        return;
    }


    /* If this lease was acquired through a BOOTREPLY, record that
       fact. */
    if (!lease->options [DHO_DHCP_MESSAGE_TYPE].len)
        lease->is_bootp = 1;

    /* Record the medium under which this lease was offered. */
    lease->medium = ip->client->medium;

    arp_timeout_needed = 0;

    /* Figure out when we're supposed to stop selecting. */
    stop_selecting = (ip->client->first_sending +
              ip->client->config->select_interval);

    /* If this is the lease we asked for, put it at the head of the
       list, and don't mess with the arp request timeout. */
    if (lease->address.len == ip->client->requested_address.len &&
        !memcmp (lease->address.iabuf,
             ip->client->requested_address.iabuf,
                    ip->client->requested_address.len))
    {
        lease->next = ip->client->offered_leases;
        ip->client->offered_leases = lease;
    }
    else
    {
        /* If we already have an offer, and arping for this
           offer would take us past the selection timeout,
           then don't extend the timeout - just hope for the
           best. */
        if (ip->client->offered_leases &&
            (cur_time + arp_timeout_needed) > stop_selecting)
            arp_timeout_needed = 0;

        /* Put the lease at the end of the list. */
        lease->next = (struct client_lease *)0;
        if (!ip->client->offered_leases)
            ip->client->offered_leases = lease;
        else
        {
            for (lp = ip->client->offered_leases; lp->next;lp = lp->next)
                ;
            lp->next = lease;
        }
    }

    /* If we're supposed to stop selecting before we've had time
       to wait for the ARPREPLY, add some delay to wait for
       the ARPREPLY. */
#if 0 /* Penny, 2002/3/11: In order to avoid sending to many dhcpdiscover, comment out here
	   *  	to go to state_selecting directly
	   */
       deltaTime = stop_selecting - cur_time;
    if (deltaTime < arp_timeout_needed)
        stop_selecting = (cur_time + arp_timeout_needed);

    /* If the selecting interval has expired, go immediately to
       state_selecting().  Otherwise, time out into
       state_selecting at the select interval. */
    if (stop_selecting <= 0)
        state_selecting (ip);
    else
    {
        DHCP_TIME_add_timeout (stop_selecting, state_selecting, ip);
        DHCP_TIME_cancel_timeout (send_discover, ip);
    }
#endif /* end of #if 0 */

	state_selecting (ip);


} /* End of void dhcpoffer() */

void dhcpack(packet)
    struct packet *packet;
{
    struct interface_info *ip = packet->interface;
    struct client_lease *lease;
    TIME cur_time;
    UI32_T option66_length = 0, option67_length = 0;
    UI8_T tftp_server_p[SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN]={0};
    char  bootfile_name_p[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN]={0};
    UI8_T *pkt_option_p;
    UI32_T option_code;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND); /* cur_time in second */

    /* If we're not receptive to an offer right now, or if the offer
       has an unrecognizable transaction id, then just drop it. */
    if (packet->interface->client->xid != packet->raw->xid ||
        (packet->interface->hw_address.hlen !=
         packet->raw->hlen) ||
        (memcmp (packet->interface->hw_address.haddr,
             packet->raw->chaddr, packet->raw->hlen)))
        return;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    /* if interface is configured to dhcp inform , we should ack it */
    if(ip->dhcp_inform)
    {
        /* state should always in select */
        if(ip->client->state == S_SELECTING)
        {
            DHCP_ALGO_StoreConfigFromAck(packet);
        }

        /* Stop resending DHCPINFORM. */
        DHCP_TIME_cancel_timeout (DHCP_ALGO_SendInform, ip);
        return;
    }
#endif

    if (ip->client->state != S_REBOOTING &&
        ip->client->state != S_REQUESTING &&
        ip->client->state != S_RENEWING &&
        ip->client->state != S_REBINDING)
        return;

    lease = packet_to_lease (packet);
    if (!lease)
        return;


    ip->client->new = lease;

    /* Stop resending DHCPREQUEST. */
    DHCP_TIME_cancel_timeout (send_request, ip);

    /* Figure out the lease time. */
    ip->client->new->expiry =
        (getULong (ip->client->new->options [DHO_DHCP_LEASE_TIME].data));
    /* A number that looks negative here is really just very large,
       because the lease expiry offset is unsigned. */
    if (ip->client->new->expiry < 0)
        ip->client->new->expiry = TIME_MAX;

    /* Take the server-provided renewal time if there is one;
       otherwise figure it out according to the spec. */
    if (ip->client->new->options [DHO_DHCP_RENEWAL_TIME].len)
        ip->client->new->renewal =
            (getULong (ip->client ->new->options [DHO_DHCP_RENEWAL_TIME].data));
    else
        ip->client->new->renewal =
            ip->client->new->expiry / 2;

    /* Same deal with the rebind time. */
    if (ip->client->new->options [DHO_DHCP_REBINDING_TIME].len)
        ip->client->new->rebind =
            (getULong (ip->client->new ->options [DHO_DHCP_REBINDING_TIME].data));
    else
        ip->client->new->rebind =
            ip->client->new->renewal +
                ip->client->new->renewal / 2 +
                    ip->client->new->renewal / 4;

    ip->client->new->expiry += cur_time;
    /* Lease lengths can never be negative. */
    if (ip->client->new->expiry < cur_time)
        ip->client->new->expiry = TIME_MAX;
    ip->client->new->renewal += cur_time;
    if (ip->client->new->renewal < cur_time)
        ip->client->new->renewal = TIME_MAX;
    ip->client->new->rebind += cur_time;
    if (ip->client->new->rebind < cur_time)
        ip->client->new->rebind = TIME_MAX;

    bind_lease (ip);

    option66_length = 0;
    option67_length = 0;

    if (packet->options[DHO_VENDOR_ENCAPSULATED_OPTIONS].len > 1)
    {
    	UI32_T option43_offset=0;

        pkt_option_p = packet->options[DHO_VENDOR_ENCAPSULATED_OPTIONS].data;
        while(option43_offset < packet->options[DHO_VENDOR_ENCAPSULATED_OPTIONS].len)
        {
	        option_code = pkt_option_p[option43_offset];
	        if (option_code == 67)
	        {
                /* ASCII option 67 */
	            option67_length = pkt_option_p[option43_offset+1];
	            memcpy(bootfile_name_p, (pkt_option_p + option43_offset + 2),
                    (option67_length > sizeof(bootfile_name_p)) ? sizeof(bootfile_name_p) : option67_length);

	        }

	        if (option_code == 66)
	        {
                /* ASCII option 66 */
	            option66_length = pkt_option_p[option43_offset+1];
	            memcpy(tftp_server_p, (pkt_option_p +option43_offset + 2),
                    (option66_length > sizeof(tftp_server_p)) ? sizeof(tftp_server_p) : option66_length);
                /*Hex option 66 */
	            /*sprintf((char *)tftp_server_p, "%d.%d.%d.%d\n",pkt_option_p[option43_offset + 2 + 0],pkt_option_p[option43_offset + 2 + 1],pkt_option_p[option43_offset + 2 + 2],pkt_option_p[option43_offset + 2 + 3]);*/

	        }


	        option43_offset += (pkt_option_p[option43_offset+1]+2/*for opcode and length*/);
	  }/*end of while*/
    }/*end of option43*/

    if (option66_length == 0)
    {

        if (packet->options[DHO_DHCP_TFTP_SERVER].len > 1)
        {
            option66_length = packet->options[DHO_DHCP_TFTP_SERVER].len;

            memcpy(tftp_server_p, packet->options[DHO_DHCP_TFTP_SERVER].data,
                (option66_length > sizeof(tftp_server_p)) ? sizeof(tftp_server_p) : option66_length);
        }

    }

    if (option67_length == 0)
    {

        if (packet->options[DHO_DHCP_BOOTFILE_NAME].len > 1)
        {
            option67_length = packet->options[DHO_DHCP_BOOTFILE_NAME].len;

            memcpy(bootfile_name_p, packet->options[DHO_DHCP_BOOTFILE_NAME].data,
                (option67_length > sizeof(bootfile_name_p)) ? sizeof(bootfile_name_p) : option67_length);

        }
    }

#if (SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP== TRUE)

    if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
        DHCP_BD_MSG("option66(tftp server)[%s],len[%lu]\r\n", tftp_server_p, (unsigned long)option66_length);
        DHCP_BD_MSG("option67(bootfile)[%s],len[%lu]\r\n", bootfile_name_p, (unsigned long)option67_length);
    }

    SYS_CALLBACK_MGR_DHCP_RxOptionConfigCallback(SYS_MODULE_DHCP, option66_length, tftp_server_p,
                                option67_length, bootfile_name_p);
#endif

}

void dhcpnak(packet)
    struct packet *packet;
{
    struct interface_info *ip = packet->interface;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }
    /* If we're not receptive to an offer right now, or if the offer
       has an unrecognizable transaction id, then just drop it. */
    if (packet->interface->client->xid != packet->raw->xid ||
        (packet->interface->hw_address.hlen !=
         packet->raw->hlen) ||
        (memcmp (packet->interface->hw_address.haddr,
             packet->raw->chaddr, packet->raw->hlen)))
        return;

    if (ip->client->state != S_REBOOTING &&
        ip->client->state != S_REQUESTING &&
        ip->client->state != S_RENEWING &&
        ip->client->state != S_REBINDING)
        return;

    if (!ip->client->active)
        return;

    free_client_lease (ip->client->active);
    ip->client->active = (struct client_lease *)0;

    /* Stop sending DHCPREQUEST packets... */
    DHCP_TIME_cancel_timeout (send_request, ip);

    ip->client->state = S_INIT;
    state_init (ip);
}


/* FUNCTION NAME : state_reboot
 * PURPOSE:
 *      Change dhcp client lease state to REBOOT for specified interface.
 *
 * INPUT:
 *      ipp --  interface_info_pointer point to interface_info structure
 *              which would be rebooted (REINIT) for dhcp client.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. If the interface had not configured ip, broadcast 'discover' packet,
 *         else send unicast 'dhcprequest' to server.
 *
 */

void state_reboot(void *ipp)
{
    struct interface_info *ip = ipp;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* If we don't remember an active lease, go straight to INIT. */
    if (!ip->client->active ||
            ip->client->active->is_bootp)
    {

        state_init (ip);
        return;
    }

#if (SYS_CPNT_DHCP_INFORM == TRUE)

    if((DHCP_TYPE_INTERFACE_MODE_USER_DEFINE == ip->mode)&&
       (TRUE == ip->dhcp_inform))
    {
        if(0!=ip->primary_address)
        {
            state_init (ip);
        }

        return;
    }
#endif

    /* We are in the rebooting state. */
    ip->client->state = S_REBOOTING;

    /* make_request doesn't initialize xid because it normally comes
       from the DHCPDISCOVER, but we haven't sent a DHCPDISCOVER,
       so pick an xid now. */
    ip->client->xid = rand();

    /* Make a DHCPREQUEST packet, and set appropriate per-interface
       flags. */

    make_request (ip, ip->client->active);
    ip->client->destination = iaddr_broadcast;
    ip->client->first_sending = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    ip->client->interval = ip->client->config->initial_interval;
    /* Zap the medium list... */
    ip->client->medium = (struct string_list *)0;

    /* Send out the first DHCPREQUEST packet. */
    send_request (ip);
}

/* Called when a lease has completely expired and we've been unable to
   renew it. */

void state_init(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;
#if (SYS_CPNT_DHCP_INFORM == TRUE)
    BOOL_T dhcp_inform = FALSE;
#endif

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* if address mode is user configured, enabled dhcp inform, and has configured ip address,
     * use dhcp inform instead of dhcp discover
     */
#if (SYS_CPNT_DHCP_INFORM == TRUE)
    if((DHCP_TYPE_INTERFACE_MODE_USER_DEFINE == ip->mode)&&
       (TRUE == ip->dhcp_inform)&&
       (0!=ip->primary_address))
    {
        dhcp_inform = TRUE;
    }
    else
    {
        dhcp_inform = FALSE;
    }

    if(TRUE == dhcp_inform)
        DHCP_ALGO_MakeInform(ip);
    else
#endif
    make_discover (ip, ip->client->active);

    ip->client->xid = ip->client->packet.xid;
    ip->client->destination = iaddr_broadcast;
    ip->client->state = S_SELECTING;
    ip->client->first_sending = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    ip->client->interval = ip->client->config->initial_interval;

#if (SYS_CPNT_DHCP_INFORM == TRUE)
    if(TRUE == dhcp_inform)
        DHCP_ALGO_SendInform(ip);
    else
#endif
    send_discover (ip);
}

/* state_selecting is called when one or more DHCPOFFER packets have been
   received and a configurable period of time has passed. */

void state_selecting(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;


    TIME cur_time;
    struct client_lease *lp, *next, *picked;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* Cancel state_selecting and send_discover timeouts, since either
       one could have got us here. */
    DHCP_TIME_cancel_timeout(state_selecting, ip);
    DHCP_TIME_cancel_timeout(send_discover, ip);

    /* We have received one or more DHCPOFFER packets.   Currently,
       the only criterion by which we judge leases is whether or
       not we get a response when we arp for them. */
    picked = (struct client_lease *)0;
    for (lp = ip->client->offered_leases; lp; lp = next)
    {
        next = lp->next;

        /* Check to see if we got an ARPREPLY for the address
           in this particular lease. */
        if (!picked)
        {
//            L_STDLIB_Ntoh32(*(UI32_T*)(lp->address.iabuf));

            picked = lp;
            picked->next = (struct client_lease *)0;
        } /*end if (!picked)  */
        else
        {
            free_client_lease (lp);
        }
    } /* end of for (lp = ip->client->offered_leases; lp; lp = next)  */
    ip->client->offered_leases = (struct client_lease *)0;

    /* If we just tossed all the leases we were offered, go back
       to square one. */
    if (!picked)
    {
        ip->client->state = S_INIT;
        state_init (ip);
        return;
    }

    /* If it was a BOOTREPLY, we can just take the address right now. */
    if (!picked->options [DHO_DHCP_MESSAGE_TYPE].len)
    {
        ip->client->new = picked;

        /* Make up some lease expiry times
           XXX these should be configurable. */
        cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
#if 0 /* Penny 2002/2/27: BOOTP lease time should be infinite */
        ip->client->new->expiry = cur_time + 12000;
        ip->client->new->renewal += cur_time + 8000;
        ip->client->new->rebind += cur_time + 10000;
#endif
		ip->client->new->expiry = TIME_MAX;
        ip->client->new->renewal = (TIME_MAX*0.5);
        ip->client->new->rebind = (TIME_MAX*0.875);

        ip->client->state = S_REQUESTING;

        /* Bind to the address we received. */
        bind_lease (ip);
        return;
    }

    /* Go to the REQUESTING state. */
    ip->client->destination = iaddr_broadcast;
    ip->client->state = S_REQUESTING;
    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    ip->client->first_sending = cur_time;
    ip->client->interval = ip->client->config->initial_interval;

    /* Make a DHCPREQUEST packet from the lease we picked. */
    make_request (ip, picked);
    ip->client->xid = ip->client->packet.xid;

    /* Toss the lease we picked - we'll get it back in a DHCPACK. */
    free_client_lease (picked);

    /* Add an immediate timeout to send the first DHCPREQUEST packet. */
    send_request (ip);
}

/* state_panic gets called if we haven't received any offers in a preset
   amount of time.   When this happens, we try to use existing leases that
   haven't yet expired, and failing that, we call the client script and
   hope it can do something. */

void state_panic(ipp, flag)
    void *ipp;
    UI8_T flag;
{
    TIME cur_time;
    SYSLOG_OM_RecordOwnerInfo_T owner_info;
    struct interface_info *ip = ipp;
	cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    const DHCP_TIME default_retry_interval = 300;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

	/* 2003-2-19, Penny: Modify for Sun request for do syslog and prompting error msg
    **		to console while the failure of receving any lease
    */
    if(DHCP_TYPE_STATE_PANIC_DHCP_DISCOVER == flag)
    {
        if (had_been_logged_to_syslog == FALSE)
        {
            DHCP_BD(CLIENT, "DHCP request failed - will retry later");

	        /* syslog */
	        owner_info.level 		= SYSLOG_LEVEL_WARNING;
	        owner_info.module_no 	= SYS_MODULE_DHCP;
	        owner_info.function_no 	= DHCP_TYPE_STATE_PANIC;
	        owner_info.error_no 	= DHCP_TYPE_EH_State_panic;
    	   SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, DHCP_IP_RETRIEVE_FAILURE_INDEX, 0, 0, 0);
		    had_been_logged_to_syslog = TRUE;
	    }
    }
#if (SYS_CPNT_DHCP_INFORM == TRUE)
    if(DHCP_TYPE_STATE_PANIC_DHCP_INFORM == flag)
    {
        UI32_T log_vid=0;
        /* DHCP Inform doesn't receive DHCP Ack, write a log and retry after 300 secs */
        owner_info.level = SYSLOG_LEVEL_WARNING;
        owner_info.module_no = SYS_MODULE_DHCP;
        owner_info.function_no = DHCP_TYPE_STATE_PANIC;
        owner_info.error_no = DHCP_TYPE_EH_State_panic_dhcp_inform;
        VLAN_IFINDEX_CONVERTTO_VID(ip->vid_ifIndex, log_vid);
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, DHCP_INFORM_RETRIEVE_FAILURE_INDEX, (void *)&log_vid, 0, 0);
    }
#endif

#if (SYS_CPNT_AUTO_CONFIG_STATIC_IP == TRUE)
{
    CLI_TYPE_WorkingFlag_T cli_flag;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    /* if the default vlan filed to get ip address,
     * and DUT use the Factory_default.cfg to boot,
     * we change to static ip
     */
     if(ip->vid_ifIndex == SYS_DFLT_AUTO_STATIC_IP_VLAN_IF_INDEX)
     {
        memset(&cli_flag, 0, sizeof(cli_flag));
        CLI_POM_GetWorkingFlag(&cli_flag);
        if(cli_flag.startup_by_factory_default)
        {
            NETCFG_PMGR_IP_SetIpAddressMode (SYS_DFLT_AUTO_STATIC_IP_VLAN_IF_INDEX, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE);
            memset(&rif_config, 0, sizeof(rif_config));
            rif_config.ifindex = ip->vid_ifIndex;
            rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
            rif_config.row_status = VAL_netConfigStatus_2_createAndGo;
            L_INET_StringToInaddr(L_INET_FORMAT_IP_UNSPEC, SYS_DFLT_FACTORY_DFLT_STATIC_IP, (L_INET_Addr_T *)&rif_config.addr, sizeof(rif_config.addr));
            rif_config.addr.preflen = SYS_DFLT_FACTORY_DFLT_PREFIX_LEN;
            NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC);

            return;
        }
     }
}
#endif
    /*note ("No DHCPOFFERS received. Now in state_panic."); */
#if 0 /* 2002/3/21 Penny:Mask off for dhcp exception */
    /* We may not have an active lease, but we may have some
       predefined leases that we can try. */
    if (!ip->client->active && ip->client->leases)
        goto activate_next;

    /* Run through the list of leases and see if one can be used. */
    while (ip->client->active)
    {
        if (ip->client->active->expiry > cur_time)
        {
/*          note ("Trying recorded lease %s",
                  piaddr (ip->client->active->address));
*/
            /* Run the client script with the existing
               parameters. */
/*
            script_init (ip, "TIMEOUT",
                     ip->client->active->medium);
            script_write_params (ip, "new_",
                         ip->client->active);
            if (ip->client->alias)
                script_write_params (ip, "alias_",
                             ip->client->alias);
*/
            /* If the old lease is still good and doesn't
               yet need renewal, go into BOUND state and
               timeout at the renewal time. */

            temp = ip->primary_address;
/* 2002/2/20 mask off til we use arp to check --> assuming no one use that ip */
#if 0
            if (!IP_MGR_PingOut(temp, &delta_time, &result))
            {
                DHCP_ERROR_Print("Ping fails.....\n");
                return;
            }
#endif
result = 0x01;

            if (result == 0x01)
            {
                if (cur_time <
                    ip->client->active->renewal)
                {
                    ip->client->state = S_BOUND;
/*                  note ("bound: renewal in %d seconds.",
                          ip->client->active->renewal
                          - cur_time);
                          */
                    DHCP_TIME_add_timeout((ip->client ->
                              active->renewal),
                             state_bound, ip);
                }
                else
                {
                    ip->client->state = S_BOUND;
/*                  note ("bound: immediate renewal."); */
                    state_bound (ip);
                }
                reinitialize_interfaces ();
                /*go_daemon ();*/
                return;
            }

        }

        /* If there are no other leases, give up. */
        if (!ip->client->leases)
        {
            ip->client->leases = ip->client->active;
            ip->client->active = (struct client_lease *)0;
            break;
        }

    activate_next:
        /* Otherwise, put the active lease at the end of the
           lease list, and try another lease.. */
        for (lp = ip->client->leases; lp->next; lp = lp->next)
            ;
        lp->next = ip->client->active;
        if (lp->next)
        {
            lp->next->next = (struct client_lease *)0;
        }
        ip->client->active = ip->client->leases;
        ip->client->leases = ip->client->leases->next;

        /* If we already tried this lease, we've exhausted the
           set of leases, so we might as well give up for
           now. */
        if (ip->client->active == loop)
            break;
        else if (!loop)
            loop = ip->client->active;
    } /* end of while (ip->client->active) */

    /* No leases were available, or what was available didn't work, so
       tell the shell script that we failed to allocate an address,
       and try again later. */
    /*DHCP_ERROR_Print("***DHCP***(State_panic): No working leases in persistent database - sleeping.\n");*/
 #endif /*end of #if 0 */
    ip->client->state = S_INIT;

   /* retry interval is set when read_client_conf() */
    if(ip->client->config)
        DHCP_TIME_add_timeout (cur_time + ip->client->config->retry_interval,state_init, ip);
    else
        DHCP_TIME_add_timeout (cur_time + default_retry_interval,state_init, ip);


}

void bind_lease(ip)
    struct interface_info *ip;
{
	UI32_T address_mode;
	SYSLOG_OM_RecordOwnerInfo_T owner_info;
	UI32_T om_mode;
    NETCFG_TYPE_InetRifConfig_T rif_config;
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE|| SYS_CPNT_DNS == TRUE )
        int i,name_srv_count;
#endif

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset(&rif_config, 0, sizeof(rif_config));

	/* Penny, 2003-9-5: cancel the timeout for the previous lease expired  */
	 DHCP_TIME_cancel_timeout (DHCP_ALGO_LeaseExpired, ip);

	/* 2002/2/9 Penny: Check if dhcp OM == Dynamic address mode, but Netcfg OM == Static,
	 * Don't bind DHCP lease to system
	 */
	if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ip->vid_ifIndex, &address_mode))
    {
    	return;
    }

    if (!DHCP_OM_GetIfMode(ip->vid_ifIndex, &om_mode))
    {
    	return;
    }

    /* We detect that system just change from dynamic to static address mode, but
     * user did not key in "ip dhcp restart", we do the dhcp-restart for user.
     */
    else if ( (address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) &&
    		  (address_mode != om_mode) )
    {
    	DHCP_TIME_ClearList();
    	DHCP_MGR_Restart(DHCP_TYPE_RESTART_CLIENT); /* we automatic trigger our dhcp to update info */
    	return;
    }

	rif_config.ifindex = ip->vid_ifIndex;
    rif_config.row_status  = VAL_netConfigStatus_2_createAndGo;
    rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY;
    rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
    rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

    memcpy(rif_config.addr.addr, ip->client->new->address.iabuf, SYS_ADPT_IPV4_ADDR_LEN);
    if(ip->client->new->options[DHO_SUBNET_MASK].data)
    {
        rif_config.addr.preflen =IP_LIB_MaskToCidr( ip->client->new->options[DHO_SUBNET_MASK].data );
    }
    else
    {
        if(IP_LIB_IsIpInClassA(rif_config.addr.addr))
        {
            rif_config.addr.preflen = 8;
        }
        else if(IP_LIB_IsIpInClassB(rif_config.addr.addr))
        {
            rif_config.addr.preflen = 16;
        }
        else if(IP_LIB_IsIpInClassC(rif_config.addr.addr))
        {
            rif_config.addr.preflen = 24;
        }
        else
        {
            /* Invalid unicast IP addresss
             */
            DHCP_BD(CLIENT, "Invalid IP address");
            return;
        }
    }

    if(NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_DYNAMIC) != NETCFG_TYPE_OK)
        DHCP_BD(CLIENT, "Failed to bind ip to the system.");

    if (ip->client->new->options [DHO_ROUTERS].data != NULL)
    {
        int i=0;
        for(i=0;i < (ip->client->new->options [DHO_ROUTERS].len/4);i++)
        {
            if(IP_LIB_IsIpBelongToSubnet(rif_config.addr.addr, rif_config.addr.preflen , ip->client->new->options [DHO_ROUTERS].data))
                if(DHCP_MGR_SetClientDefaultGateway(ip->vid_ifIndex, ip->client->new->options [DHO_ROUTERS].data+i*4))
                    break;
        }
    }
    else
    {
        DHCP_MGR_DeleteClientDefaultGateway(ip->vid_ifIndex );
    }
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE|| SYS_CPNT_DNS == TRUE )
    /* Add DNS Servers. */
    name_srv_count = (ip->client->new->options [DHO_DOMAIN_NAME_SERVERS].len)/4;

    if (name_srv_count > 0)
    {
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE )
        if( DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex))
            DHCP_BD(CLIENT, "Failed to delete all dns IPs.");

#elif (SYS_CPNT_DNS == TRUE)
        if(DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex))
            DHCP_BD(CLIENT, "Failed to delete all dns IPs.");

#endif
        for(i=0;i<name_srv_count;i++)
        {
            if(ip->client->new->options [DHO_DOMAIN_NAME_SERVERS].data)
            {
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE )
                if(DHCP_MGR_SetClientNameServer(ip->vid_ifIndex, (ip->client->new->options [DHO_DOMAIN_NAME_SERVERS].data)+(i*4)))
                    DHCP_BD(CLIENT, "Failed to add a DNS IP.");

#elif (SYS_CPNT_DNS == TRUE)
                if(DHCP_MGR_SetClientNameServer(ip->vid_ifIndex, (ip->client->new->options [DHO_DOMAIN_NAME_SERVERS].data)+(i*4)))
                    DHCP_BD(CLIENT, "Failed to add a DNS IP.");
#endif
            }
        }
    }
    else
    {
        DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex);
    }
#endif

    /* Record the server ip and the default gateway in dhcp interface */
    if(ip->client->new->options[DHO_DHCP_SERVER_IDENTIFIER].data)
    {
        ip->server_ip = getULong(ip->client->new->options [DHO_DHCP_SERVER_IDENTIFIER].data);
    }
    else
    {
        ip->server_ip = 0;
    }

    if(ip->client->new->options[DHO_ROUTERS].data)
    {
        ip->gateway_ip = getULong(ip->client->new->options [DHO_ROUTERS].data);
    }
    else
    {
        ip->gateway_ip = 0;
    }

    /* Remember the medium */
    ip->client->new->medium = ip->client->medium;

    /* Write out the new lease to UC.
    ** Compare the new ip to current one
    ** if it is different, write client lease
    ** otherwise do not need to write it
    ** Penny: 2002-2-5 */

    /* When the state from Requesting to Bound(first DHCP ACK), we need to log it.  */
    if (ip->client->active == NULL)
    {
        DHCP_BD(CLIENT, "DHCP server responded");

        /* syslog */
        owner_info.level        = SYSLOG_LEVEL_NOTICE;
        owner_info.module_no    = SYS_MODULE_DHCP;
        owner_info.function_no 	= DHCP_TYPE_BIND_LEASE;
        owner_info.error_no 	= DHCP_TYPE_EH_Bind_lease;
        SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, DHCP_IP_RETRIEVE_SUCCESS_INDEX, "DHCP(bind_lease)", 0, 0);
        had_been_logged_to_syslog = FALSE;
        /* It must write acitve lease to UC */
        write_client_lease(ip->vid_ifIndex,ip->client->new);
	}
	/* When the state from Renewing to Bound or from Rebinding to Bound, we will log according to had_been_logged_to_syslog */
	else
	{
	    if (strcmp((char*)ip->client->active->address.iabuf,
	    	        (char*)ip ->client->new->address.iabuf))
	    {
	    	write_client_lease(ip->vid_ifIndex,ip->client->new);
	    }
		/* syslog : successfully get IP from server in IP failure retrieving cycle */
	   	if (had_been_logged_to_syslog)
	   	{
            DHCP_BD(CLIENT, "DHCP server responded");

		    /* syslog */
		    owner_info.level 		= SYSLOG_LEVEL_NOTICE;
		    owner_info.module_no 	= SYS_MODULE_DHCP;
		    owner_info.function_no 	= DHCP_TYPE_BIND_LEASE;
		    owner_info.error_no 	= DHCP_TYPE_EH_Bind_lease;
		    SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, DHCP_IP_RETRIEVE_SUCCESS_INDEX, "DHCP(bind_lease)", 0, 0);
			had_been_logged_to_syslog = FALSE;
	   	}

	    /* Replace the old active lease with the new one. */
	    free_client_lease (ip->client->active);
	}

    ip->client->active = ip->client->new;
    ip->client->new = (struct client_lease *)0;

    /* Set up a timeout to start the renewal process. */
    DHCP_TIME_add_timeout (ip->client->active->renewal,
             state_bound, ip);

	/* Penny, 2003-9-5: add the timeout for the new lease expired  */
	DHCP_TIME_add_timeout (ip->client->active->expiry,
             DHCP_ALGO_LeaseExpired, ip);
	/* Penny, 2002-10-7: update current ip address to primary_address */
	ip->primary_address = getULong(ip->client->active->address.iabuf);

    ip->client->state = S_BOUND;

    /* configure this interface with IP information
    */

    reinitialize_interfaces();
    /*go_daemon ();*/
}

/* state_bound is called when we've successfully bound to a particular
   lease, but the renewal time on that lease has expired.   We are
   expected to unicast a DHCPREQUEST to the server that gave us our
   original lease. */

void state_bound(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;
    TIME cur_time;
    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* T1 has expired. */
    make_request (ip, ip->client->active);
    ip->client->xid = ip->client->packet.xid;

    if (ip->client->active->options[DHO_DHCP_SERVER_IDENTIFIER].len == 4)
    {
        memcpy (ip->client->destination.iabuf,
               ip->client->active->options[DHO_DHCP_SERVER_IDENTIFIER].data, 4);
        ip->client->destination.len = 4;
    }
    else
        ip->client->destination = iaddr_broadcast;

    ip->client->first_sending = cur_time;
    ip->client->interval = ip->client->config->initial_interval;
    ip->client->state = S_RENEWING;

    /* Send the first packet immediately. */
    send_request (ip);
}

void make_discover(ip, lease)
    struct interface_info *ip;
    struct client_lease *lease;
{
    unsigned char discover = DHCPDISCOVER;
    int i;
    BOOL_T is_bootp = FALSE;
    UI32_T address_mode;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset (option_elements, 0, sizeof option_elements);
    memset (options, 0, sizeof options);
    memset (&ip->client->packet, 0, sizeof (ip->client->packet));

#if (SYS_CPNT_BOOTP == TRUE)
    /* 2002/2/21 Penny: In order to send out a "pure" bootp packet if address mode == bootp,
    					skip DHCP Message type
     */
    if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ip->vid_ifIndex, &address_mode))
    {
    	return;
    }

    /* We detect that system just change to bootp address mode, but
     * user not key in "ip dhcp restart", skip to fill-in in the option[dhcp message type]
     */
    else if (address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP)
    {
    	is_bootp = TRUE;
    }
#endif

	if (!is_bootp)
	{
		/* 2002-7-17, Penny: to insert cid into option field if user specified */
		if (ip ->cid.id_len != 0)
		{
			i = DHO_DHCP_CLIENT_IDENTIFIER;
			options [i] = &option_elements [i];
		    options [i]->value = (unsigned char *)ip->cid.id_buf;
		    options [i]->len 	 = ip->cid.id_len;
		    options [i]->buf_size = ip->cid.id_len;
		    options [i]->timeout = 0xFFFFFFFF;
		    options [i]->tree = (struct tree *)0;
        }

        if (ip ->classid.vendor_len != 0)
        {
            i = DHO_DHCP_CLASS_IDENTIFIER;
            options [i] = &option_elements [i];
            options[i]->value = ip ->classid.vendor_buf;
            options[i]->len 	 = ip ->classid.vendor_len;
            options[i]->buf_size = sizeof(ip ->classid.vendor_buf);
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
        }


        /* Set DHCP_MESSAGE_TYPE to DHCPDISCOVER */
        i = DHO_DHCP_MESSAGE_TYPE;
        options [i] = &option_elements [i];
        options [i] -> value = &discover;
        options [i] -> len = sizeof discover;
        options [i] -> buf_size = sizeof discover;
        options [i] -> timeout = 0xFFFFFFFF;
        options [i] -> tree = (struct tree *)0;

        /* Request the options we want */
        i  = DHO_DHCP_PARAMETER_REQUEST_LIST;
        options [i] = &option_elements [i];
        options [i] -> value = ip -> client -> config -> requested_options;
        options [i] -> len = ip -> client -> config -> requested_option_count;
        options [i] -> buf_size = ip -> client -> config -> requested_option_count;
        options [i] -> timeout = 0xFFFFFFFF;
        options [i] -> tree = (struct tree *)0;

        /* If we had an address, try to get it again. */
        if (lease)
        {
            ip -> client -> requested_address = lease -> address;
            i = DHO_DHCP_REQUESTED_ADDRESS;
            options [i] = &option_elements [i];
            options [i] -> value = lease -> address.iabuf;
            options [i] -> len = lease -> address.len;
            options [i] -> buf_size = lease -> address.len;
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
        }
        else
        {
            ip -> client -> requested_address.len = 0;
            /* Penny disabled 1-25-2002 */

        }


        /* Send any options requested in the config file. */
        for (i = 0; i < 256; i++)
        {
            if (!options [i])
            {
                options [i] = &option_elements [i];
                options [i] -> value = 0;
                options [i] -> len = 0;
                options [i] -> buf_size = 0;
                options [i] -> timeout = 0xFFFFFFFF;
                options [i] -> tree = (struct tree *)0;
            }
        }


    } /* end of if (!is_bootp) */

    /* Set up the option buffer... */
    ip->client->packet_length = cons_options ((struct packet *)0, &ip->client->packet, 0,
                                                options, 0, 0, 0, (UI8_T *)0, 0);
    if (ip->client->packet_length < BOOTP_MIN_LEN)
        ip->client->packet_length = BOOTP_MIN_LEN;

    ip->client->packet.op = BOOTREQUEST;
    ip->client->packet.htype = ip->hw_address.htype;
    ip->client->packet.hlen = ip->hw_address.hlen;
    ip->client->packet.hops = 0;
    ip->client->packet.xid = (UI32_T)rand();
    ip->client->packet.secs = 0; /* filled in by send_discover. */

    ip->client->packet.flags = L_STDLIB_Hton16 (BOOTP_BROADCAST);

    memset(&(ip->client->packet.ciaddr), 0, sizeof(ip->client->packet.ciaddr));
    memset(&(ip->client->packet.yiaddr), 0, sizeof(ip->client->packet.yiaddr));
    memset(&(ip->client->packet.siaddr), 0, sizeof(ip->client->packet.siaddr));
    memset(&(ip->client->packet.giaddr), 0, sizeof(ip->client->packet.giaddr));
    memcpy(ip->client->packet.chaddr, ip->hw_address.haddr, ip->hw_address.hlen);

    if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        int i;
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
        DHCP_BD_MSG("HW address:");

        for (i = 0;i < ip->hw_address.hlen;i++)
            DHCP_BD_MSG("%02x ", ip->client->packet.chaddr[i]);

        DHCP_BD_MSG("\r\n");
    }

#ifdef DEBUG_PACKET
    dump_packet (sendpkt);
    dump_raw ((unsigned char *)ip->client->packet,
          sendpkt->packet_length);
#endif
}


void make_request(ip, lease)
    struct interface_info *ip;
    struct client_lease *lease;
{
    unsigned char request = DHCPREQUEST;
    UI32_T address_mode;
    int i;
    BOOL_T is_bootp = FALSE;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset (options, 0, sizeof options);
    memset (&ip->client->packet, 0, sizeof (ip->client->packet));

#if (SYS_CPNT_BOOTP == TRUE)
    /* 2002/2/21 Penny: In order to send out a "pure" bootp packet if address mode == bootp,
     *					skip DHCP Message type
     */
	if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ip->vid_ifIndex, &address_mode))
    {
    	return;
    }

    /* We detect that system just change to bootp address mode, but
     * user not key in "ip dhcp restart", skip to fill-in in the option[dhcp message type]
     */
    if (address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP)
    {
    	is_bootp = TRUE;
    }
#endif

	if (!is_bootp)
	{
		/* 2002-7-17, Penny: to insert cid into option field if user specified */
		if (ip ->cid.id_len != 0)
		{
			i = DHO_DHCP_CLIENT_IDENTIFIER;
			options [i] = &option_elements [i];
		    options [i]->value = (unsigned char *)ip->cid.id_buf;
		    options [i]->len 	 = ip->cid.id_len;
		    options [i]->buf_size = ip->cid.id_len;
		    options [i]->timeout = 0xFFFFFFFF;
		    options [i]->tree = (struct tree *)0;
        }

        if (ip ->classid.vendor_len != 0)
        {
#if 0      /* should we send option 43 in request ?*/
            i = DHO_VENDOR_ENCAPSULATED_OPTIONS;
            options [i] = &option_elements [i];
            options [i] -> value = ip ->classid.vendor_buf; // vendor_encap.vendor_buf;
            options [i] -> len 	 = ip ->classid.vendor_len; // vendor_encap.vendor_len;
            options [i] -> buf_size = sizeof(ip ->classid.vendor_buf); // sizeof(vendor_encap.vendor_buf);
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
#endif
            i = DHO_DHCP_CLASS_IDENTIFIER;
            options [i] = &option_elements [i];
            options[i]->value = ip ->classid.vendor_buf;
            options[i]->len 	 = ip ->classid.vendor_len;
            options[i]->buf_size = sizeof(ip ->classid.vendor_buf);
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
        }

        /* Set DHCP_MESSAGE_TYPE to DHCPREQUEST */
        i = DHO_DHCP_MESSAGE_TYPE;
        options [i] = &option_elements [i];
        options [i] -> value = &request;
        options [i] -> len = sizeof request;
        options [i] -> buf_size = sizeof request;
        options [i] -> timeout = 0xFFFFFFFF;
        options [i] -> tree = (struct tree *)0;

        /* IsBootp: */
        /* Request the options we want */
        i = DHO_DHCP_PARAMETER_REQUEST_LIST;
        options [i] = &option_elements [i];
        options [i] -> value = ip -> client -> config -> requested_options;
        options [i] -> len = ip -> client -> config -> requested_option_count;
        options[i]->buf_size = ip->client->config->requested_option_count;
        options [i] -> timeout = 0xFFFFFFFF;
        options [i] -> tree = (struct tree *)0;

        /* If we are requesting an address that hasn't yet been assigned
           to us, use the DHCP Requested Address option. */
        if (ip->client->state == S_REQUESTING)
        {
            /* Send back the server identifier... */
            i = DHO_DHCP_SERVER_IDENTIFIER;
            options [i] = &option_elements [i];
            options [i] -> value = lease -> options [i].data;
            options [i] -> len = lease -> options [i].len;
            options [i] -> buf_size = lease -> options [i].len;
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
        }
        if (ip -> client -> state == S_REQUESTING ||
                ip -> client -> state == S_REBOOTING)
        {
            ip -> client -> requested_address = lease -> address;
            i = DHO_DHCP_REQUESTED_ADDRESS;
            options [i] = &option_elements [i];
            options [i] -> value = lease -> address.iabuf;
            options [i] -> len = lease -> address.len;
            options [i] -> buf_size = lease -> address.len;
            options [i] -> timeout = 0xFFFFFFFF;
            options [i] -> tree = (struct tree *)0;
        }
        else
        {
            ip -> client -> requested_address.len = 0;
        }

        /* Send any options requested in the config file. */
        for (i = 0; i < 256; i++)
        {
            if (!options [i] &&
                    ip->client->config->send_options[i].data)
            {
                options [i] = &option_elements [i];
                options[i]->value = ip->client->config->send_options[i].data;
                options[i]->len = ip->client->config->send_options[i].len;
                options[i]->buf_size = ip->client->config->send_options[i].len;
                options [i] -> timeout = 0xFFFFFFFF;
                options [i] -> tree = (struct tree *)0;
            }
        }
    } /* end of if(!is_bootp)*/

    /* Set up the option buffer... */
    ip->client->packet_length =
        cons_options ((struct packet *)0, &ip->client->packet, 0,
                  options, 0, 0, 0, (UI8_T *)0, 0);
    if (ip->client->packet_length < BOOTP_MIN_LEN)
        ip->client->packet_length = BOOTP_MIN_LEN;

    ip->client->packet.op = BOOTREQUEST;
    ip->client->packet.htype = ip->hw_address.htype;
    ip->client->packet.hlen = ip->hw_address.hlen;
    ip->client->packet.hops = 0;
    ip->client->packet.xid = ip->client->xid;
    ip->client->packet.secs = 0; /* Filled in by send_request. */

    /* If we own the address we're requesting, put it in ciaddr;
       otherwise set ciaddr to zero. */
    if (ip->client->state == S_BOUND ||
        ip->client->state == S_RENEWING ||
            ip->client->state == S_REBINDING)
    {
        memcpy (&ip->client->packet.ciaddr,
            lease->address.iabuf, lease->address.len);
        ip->client->packet.flags = 0;
    }
    else
    {
        memset (&ip->client->packet.ciaddr, 0,
            sizeof ip->client->packet.ciaddr);
        ip->client->packet.flags = L_STDLIB_Hton16 (BOOTP_BROADCAST);
    }

    memset(&ip->client->packet.yiaddr, 0, sizeof(ip->client->packet.yiaddr));
    memset(&ip->client->packet.siaddr, 0, sizeof(ip->client->packet.siaddr));
    memset(&ip->client->packet.giaddr, 0, sizeof(ip->client->packet.giaddr));
    memcpy(ip->client->packet.chaddr, ip->hw_address.haddr, ip->hw_address.hlen);

}

void make_decline(ip, lease)
    struct interface_info *ip;
    struct client_lease *lease;
{
    unsigned char decline = DHCPDECLINE;
    int i;
    struct tree_cache message_type_tree;
    struct tree_cache requested_address_tree;
    struct tree_cache server_id_tree;
    struct tree_cache client_id_tree;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset (options, 0, sizeof options);
    memset (&ip->client->packet, 0, sizeof (ip->client->packet));

    /* Set DHCP_MESSAGE_TYPE to DHCPDECLINE */
    i = DHO_DHCP_MESSAGE_TYPE;
    options [i] = &message_type_tree;
    options [i]->value = &decline;
    options [i]->len = sizeof decline;
    options [i]->buf_size = sizeof decline;
    options [i]->timeout = 0xFFFFFFFF;
    options [i]->tree = (struct tree *)0;

    /* Send back the server identifier... */
    i = DHO_DHCP_SERVER_IDENTIFIER;
        options [i] = &server_id_tree;
        options [i]->value = lease->options [i].data;
        options [i]->len = lease->options [i].len;
        options [i]->buf_size = lease->options [i].len;
        options [i]->timeout = 0xFFFFFFFF;
        options [i]->tree = (struct tree *)0;

    /* Send back the address we're declining. */
    i = DHO_DHCP_REQUESTED_ADDRESS;
    options [i] = &requested_address_tree;
    options [i]->value = lease->address.iabuf;
    options [i]->len = lease->address.len;
    options [i]->buf_size = lease->address.len;
    options [i]->timeout = 0xFFFFFFFF;
    options [i]->tree = (struct tree *)0;

    /* Send the uid if the user supplied one. */
    i = DHO_DHCP_CLIENT_IDENTIFIER;
    if (ip->client->config->send_options[i].len)
    {
        options [i] = &client_id_tree;
        options[i]->value = ip->client->config->send_options[i].data;
        options[i]->len = ip->client->config->send_options[i].len;
        options[i]->buf_size = ip->client->config->send_options[i].len;
        options [i]->timeout = 0xFFFFFFFF;
        options [i]->tree = (struct tree *)0;
    }


    /* Set up the option buffer... */
    ip->client->packet_length =
        cons_options ((struct packet *)0, &ip->client->packet, 0,
                  options, 0, 0, 0, (UI8_T *)0, 0);
    if (ip->client->packet_length < BOOTP_MIN_LEN)
        ip->client->packet_length = BOOTP_MIN_LEN;

    ip->client->packet.op = BOOTREQUEST;
    ip->client->packet.htype = ip->hw_address.htype;
    ip->client->packet.hlen = ip->hw_address.hlen;
    ip->client->packet.hops = 0;
    ip->client->packet.xid = ip->client->xid;
    ip->client->packet.secs = 0; /* Filled in by send_request. */
    ip->client->packet.flags = 0;

    /* ciaddr must always be zero. */
    memset(&ip->client->packet.ciaddr, 0, sizeof(ip->client->packet.ciaddr));
    memset(&ip->client->packet.yiaddr, 0, sizeof(ip->client->packet.yiaddr));
    memset(&ip->client->packet.siaddr, 0, sizeof(ip->client->packet.siaddr));
    memset(&ip->client->packet.giaddr, 0, sizeof(ip->client->packet.giaddr));
    memcpy(ip->client->packet.chaddr, ip->hw_address.haddr, ip->hw_address.hlen);

#ifdef DEBUG_PACKET
    dump_packet (sendpkt);
    dump_raw ((unsigned char *)ip->client->packet, sendpkt->packet_length);
#endif
}

void make_release(ip, lease)
    struct interface_info *ip;
    struct client_lease *lease;
{
    unsigned char request = DHCPRELEASE;
    int i;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset (options, 0, sizeof options);
    memset (&ip->client->packet, 0, sizeof (ip->client->packet));

    /* Set DHCP_MESSAGE_TYPE to DHCPRELEASE */
    i = DHO_DHCP_MESSAGE_TYPE;
    options [i] = &option_elements[i];
    options [i]->value = &request;
    options [i]->len = sizeof request;
    options [i]->buf_size = sizeof request;
    options [i]->timeout = 0xFFFFFFFF;
    options [i]->tree = (struct tree *)0;

    /* Send back the server identifier... */
    i = DHO_DHCP_SERVER_IDENTIFIER;
        options [i] = &option_elements[i];
        options [i]->value = lease->options [i].data;
        options [i]->len = lease->options [i].len;
        options [i]->buf_size = lease->options [i].len;
        options [i]->timeout = 0xFFFFFFFF;
        options [i]->tree = (struct tree *)0;

    /* Send back the client identifier may be the optional */
    /* i = DHO_DHCP_CLIENT_IDENTIFIER; */


    /* Set up the option buffer... */
    ip->client->packet_length =
        cons_options ((struct packet *)0, &ip->client->packet, 0,
                  options, 0, 0, 0, (UI8_T *)0, 0);
    if (ip->client->packet_length < BOOTP_MIN_LEN)
        ip->client->packet_length = BOOTP_MIN_LEN;

    ip->client->packet.op = BOOTREQUEST;
    ip->client->packet.htype = ip->hw_address.htype;
    ip->client->packet.hlen = ip->hw_address.hlen;
    ip->client->packet.hops = 0;
    ip->client->packet.xid = rand();
    ip->client->packet.secs = 0;
    ip->client->packet.flags = 0;

    ip->client->packet.ciaddr = L_STDLIB_Hton32(ip->primary_address);
    memset(&ip->client->packet.yiaddr, 0, sizeof(ip->client->packet.yiaddr));
    memset(&ip->client->packet.siaddr, 0, sizeof(ip->client->packet.siaddr));
    memset(&ip->client->packet.giaddr, 0, sizeof(ip->client->packet.giaddr));
    memcpy(ip->client->packet.chaddr, ip->hw_address.haddr, ip->hw_address.hlen);

#ifdef DEBUG_PACKET
    dump_packet (sendpkt);
    dump_raw ((unsigned char *)ip->client->packet,
          ip->client->packet_length);
#endif
}

/* Send out a DHCPDISCOVER packet, and set a timeout to send out another
   one after the right interval has expired.  If we don't get an offer by
   the time we reach the panic interval, call the panic function. */

void send_discover(ipp)
    void *ipp;
{

    struct interface_info *ip = ipp;

    /*int interval;*/
    UI32_T interval;
    int increase = 1;
    TIME cur_time;
    struct in_addr inaddr_any;
    struct sockaddr_in sockaddr_broadcast;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    sockaddr_broadcast.sin_family= AF_INET;
    sockaddr_broadcast.sin_port = L_STDLIB_Hton16(67);
    sockaddr_broadcast.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_BROADCAST);
    inaddr_any.s_addr=0;
    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    DHCP_TIME timeout = 0;
    /* Figure out how long it's been since we started transmitting. */

    interval = ((cur_time - ip->client->first_sending) > 0)? (cur_time - ip->client->first_sending)
    															:(ip->client->first_sending - cur_time);

    /* If we're past the panic timeout, call the script and tell it
       we haven't found anything for this interface yet. */

    /* timeout value is set when read_client_conf()*/
    if(ip->client->config)
        timeout = ip->client->config->timeout;
    else
        timeout = 60;

    if (interval > timeout)
    {

        state_panic (ip, DHCP_TYPE_STATE_PANIC_DHCP_DISCOVER);
        return;
    }
    /* If we're selecting media, try the whole list before doing
       the exponential backoff, but if we've already received an
       offer, stop looping, because we obviously have it right. */

    /* If we're supposed to increase the interval, do so.  If it's
       currently zero (i.e., we haven't sent any packets yet), set
       it to one; otherwise, add to it a random number between
       zero and two times itself.  On average, this means that it
       will double with every transmission. */
    if (increase)
    {
        if (!ip->client->interval)
            ip->client->interval = ip->client->config->initial_interval;
        else
        {
            ip->client->interval += ((rand() >> 2) % (2 * ip->client->interval));
        }

        /* Don't backoff past cutoff. */
        if (ip->client->interval > ip->client->config->backoff_cutoff)
            ip->client->interval =
                ((ip->client->config->backoff_cutoff / 2)
                 + ((rand() >> 2) %
                    ip->client->config->backoff_cutoff));
    }
    else if (!ip->client->interval)
        ip->client->interval = ip->client->config->initial_interval;

    /* If the backoff would take us to the panic timeout, just use that
       as the interval. */

    if (cur_time + ip->client->interval >
        ip->client->first_sending + ip->client->config->timeout)
        ip->client->interval =
            (ip->client->first_sending +
             ip->client->config->timeout) - cur_time + 1;

    /* Record the number of seconds since we started sending. */
    if (interval < 65536)
        ip->client->packet.secs = L_STDLIB_Hton16 (interval);
    else
        ip->client->packet.secs = L_STDLIB_Hton16 (65535);
    ip->client->secs = ip->client->packet.secs;


    /* Send out a packet. */
    DHCP_BD(CLIENT, "Send out discover");
    DHCP_TXRX_SendPktThruIML(ip, (struct packet *)0,
                  &ip->client->packet,
                  ip->client->packet_length,
                  inaddr_any, &sockaddr_broadcast,
                  (struct hardware *)0);

    DHCP_TIME_add_timeout (cur_time + ip->client->interval, send_discover, ip);

}

void send_request(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;
	UI32_T interval;
    struct sockaddr_in destination;
    struct in_addr from;
    TIME cur_time;
    UI16_T remote_port = 67;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);
    /* Figure out how long it's been since we started transmitting. */
	interval = ((cur_time - ip->client->first_sending) > 0)? (cur_time - ip->client->first_sending)
    															:(ip->client->first_sending - cur_time);
    /* If we're in the INIT-REBOOT or REQUESTING state and we're
       past the reboot timeout, go to INIT and see if we can
       DISCOVER an address... */
    /* XXX In the INIT-REBOOT state, if we don't get an ACK, it
       means either that we're on a network with no DHCP server,
       or that our server is down.  In the latter case, assuming
       that there is a backup DHCP server, DHCPDISCOVER will get
       us a new address, but we could also have successfully
       reused our old address.  In the former case, we're hosed
       anyway.  This is not a win-prone situation. */
    if ((ip->client->state == S_REBOOTING ||
         ip->client->state == S_REQUESTING) &&
            interval > ip->client->config->reboot_timeout)
    {

        ip->client->state = S_INIT;
        DHCP_TIME_cancel_timeout (send_request, ip);
        /* If client has active lease, we should free it */
        if(ip->client->active)
        {
            /* destroy client's lease ip */
            DHCP_ALGO_LeaseExpired(ip);

            /* free active lease in engine */
            free_client_lease(ip->client->active);
            ip->client->active = NULL;
        }
        state_init (ip);
        return;
    }

    /* If we're in the reboot state, make sure the media is set up
       correctly. */
    if (ip->client->state == S_REBOOTING &&
        !ip->client->medium &&
            ip->client->active->medium)
    {
#if 0
        /*script_init (ip, "MEDIUM", ip->client->active->medium);*/

        /* If the medium we chose won't fly, go to INIT state. */
        /*
        if (script_go (ip))
            goto cancel;
        */
#endif
        /* Record the medium. */
        ip->client->medium = ip->client->active->medium;
    }

    /* If the lease has expired, relinquish the address and go back
       to the INIT state. */
    if (ip->client->state != S_REQUESTING &&
            cur_time > ip->client->active->expiry)
    {
#if 0
        /* Run the client script with the new parameters. */
        /*
        script_init (ip, "EXPIRE", (struct string_list *)0);
        script_write_params (ip, "old_", ip->client->active);
        if (ip->client->alias)
            script_write_params (ip, "alias_",
                         ip->client->alias);
        script_go (ip);
        */
        /* Now do a preinit on the interface so that we can
           discover a new address. */
        /*
        script_init (ip, "PREINIT", (struct string_list *)0);
        if (ip->client->alias)
            script_write_params (ip, "alias_",
                         ip->client->alias);
        script_go (ip);
        */
#endif
        ip->client->state = S_INIT;
        state_init (ip);
        return;
    }

    /* Do the exponential backoff... */
    if (!ip->client->interval)
        ip->client->interval =
            ip->client->config->initial_interval;
    else
    {
        ip->client->interval +=
            ((rand() >> 2) %
             (2 * ip->client->interval));
    }

    /* Don't backoff past cutoff. */
    if (ip->client->interval >
        ip->client->config->backoff_cutoff)
        ip->client->interval =
            ((ip->client->config->backoff_cutoff / 2)
             + ((rand() >> 2)
                % ip->client->interval));

    /* If the backoff would take us to the expiry time, just set the
       timeout to the expiry time. */
    if  (ip->client->state != S_REQUESTING &&
        cur_time + ip->client->interval >
        ip->client->active->expiry)
        ip->client->interval =
            ip->client->active->expiry - cur_time + 1;

    /* If the lease T2 time has elapsed, or if we're not yet bound,
       broadcast the DHCPREQUEST rather than unicasting. */
    if(ip->client->state == S_REQUESTING ||
        ip->client->state == S_REBOOTING ||
        cur_time > ip->client->active->rebind)
        destination.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_BROADCAST);
    else
        memcpy(&destination.sin_addr, ip->client->destination.iabuf, sizeof(destination.sin_addr));

    destination.sin_port = L_STDLIB_Hton16(remote_port);
    destination.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
    destination.sin_len = sizeof destination;
#endif

    if (ip->client->state != S_REQUESTING)
        memcpy (&from, ip->client->active->address.iabuf,
            sizeof from);
    else
        from.s_addr = INADDR_ANY;

    /* Record the number of seconds since we started sending. */
    if (ip->client->state == S_REQUESTING)
        ip->client->packet.secs = ip->client->secs;
    else
    {
        if (interval < 65536)
            ip->client->packet.secs = L_STDLIB_Hton16 (interval);
        else
            ip->client->packet.secs = L_STDLIB_Hton16 (65535);
    }

    DHCP_BD(CLIENT, "DHCPREQUEST on vid_ifindex[%lu] to %u.%u.%u.%u, port[%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(destination.sin_addr.s_addr), destination.sin_port);

    if (destination.sin_addr.s_addr != INADDR_BROADCAST )
        DHCP_TXRX_SendPktThruIML (ip,
                      (struct packet *)0,
                      &ip->client->packet,
                      ip->client->packet_length,
                      from, &destination,
                      (struct hardware *)0);
    else
        /* Send out a packet. */
        DHCP_TXRX_SendPktThruIML (ip, (struct packet *)0,
                      &ip->client->packet,
                      ip->client->packet_length,
                      from, &destination,
                      (struct hardware *)0);

    DHCP_TIME_add_timeout (cur_time + ip->client->interval,
             send_request, ip);
}

void send_decline(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;
    struct in_addr inaddr_any;
    struct sockaddr_in sockaddr_broadcast;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    inaddr_any.s_addr = 0;
    sockaddr_broadcast.sin_family = AF_INET;
    sockaddr_broadcast.sin_port = L_STDLIB_Hton16(67);
    sockaddr_broadcast.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_BROADCAST);

    DHCP_BD(CLIENT, "DHCPDECLINE on vid_ifindex[%lu] to %u.%u.%u.%u, port[%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(sockaddr_broadcast.sin_addr.s_addr), sockaddr_broadcast.sin_port);

    /* Send out a packet. */
    DHCP_TXRX_SendPktThruIML (ip, (struct packet *)0,
                  &ip->client->packet,
                  ip->client->packet_length,
                  inaddr_any, &sockaddr_broadcast,
                  (struct hardware *)0);
}

void send_release(ipp)
    void *ipp;
{
    struct interface_info *ip = ipp;
    struct sockaddr_in destination;
    struct in_addr from;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* set dip */
    destination.sin_addr.s_addr = L_STDLIB_Hton32(ip->server_ip);
    destination.sin_port = L_STDLIB_Hton16(67);
    destination.sin_family = AF_INET4;
#ifdef HAVE_SA_LEN
    destination.sin_len = sizeof(destination);
#endif

    /* set sip */
    memcpy (&from, ip->client->active->address.iabuf,sizeof from);

    DHCP_BD(CLIENT, "DHCPRELEASE on vid_ifindex[%lu] to %u.%u.%u.%u, port[%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(destination.sin_addr.s_addr), destination.sin_port);

    /* Send out a packet. */
    DHCP_TXRX_SendPktThruIML (ip, (struct packet *)0,
                  &ip->client->packet,
                  ip->client->packet_length,
                  from, &destination,
                  (struct hardware *)0);
}

/* support DHCP INFORM */
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
void DHCP_ALGO_MakeInform(struct interface_info *ip)
{
    unsigned char inform = DHCPINFORM;
    int i;
    u_int8_t requested_options [256] = {0};

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset (option_elements, 0, sizeof option_elements);
    memset (options, 0, sizeof options);
    memset (&ip->client->packet, 0, sizeof (ip->client->packet));

	/* Client identifier */
	if(ip ->cid.id_len != 0)
    {
	    i = DHO_DHCP_CLIENT_IDENTIFIER;
		options[i] = &option_elements [i];
		options[i]->value = (unsigned char *)ip->cid.id_buf;
		options[i]->len = ip->cid.id_len;
		options[i]->buf_size = ip->cid.id_len;
		options[i]->timeout = 0xFFFFFFFF;
		options[i]->tree = (struct tree *)0;
    }


    /* Set DHCP_MESSAGE_TYPE to DHCPINFORM */
    i = DHO_DHCP_MESSAGE_TYPE;
    options[i] = &option_elements [i];
    options[i]->value = &inform;
    options[i]->len = sizeof(inform);
    options[i]->buf_size = sizeof(inform);
    options[i]->timeout = 0xFFFFFFFF;
    options[i]->tree = (struct tree *)0;

    /* Request the options we want */
    i  = DHO_DHCP_PARAMETER_REQUEST_LIST;
    /* we only support router, domain name server */
    requested_options[0] = DHO_ROUTERS;
    requested_options[1] = DHO_DOMAIN_NAME_SERVERS;

    options[i] = &option_elements [i];
    options[i]->value = requested_options;
    options[i]->len = 2;      /* 2 options */
    options[i]->buf_size = 2; /* 2 options */
    options[i]->timeout = 0xFFFFFFFF;
    options[i]->tree = (struct tree *)0;

    for (i = 0; i < 256; i++)
    {
        if (!options[i])
        {
            options[i] = &option_elements [i];
            options[i]->value = 0;
            options[i]->len = 0;
            options[i]->buf_size = 0;
            options[i]->timeout = 0xFFFFFFFF;
            options[i]->tree = (struct tree *)0;
        }
    }


    /* Set up the option buffer... */
    ip->client->packet_length = cons_options ((struct packet *)0, &ip->client->packet, 0,
                                                options, 0, 0, 0, (UI8_T *)0, 0);
    if (ip->client->packet_length < BOOTP_MIN_LEN)
        ip->client->packet_length = BOOTP_MIN_LEN;

    ip->client->packet.op = BOOTREQUEST;
    ip->client->packet.htype = ip->hw_address.htype;
    ip->client->packet.hlen = ip->hw_address.hlen;
    ip->client->packet.hops = 0;
    ip->client->packet.xid = (UI32_T)rand();
    ip->client->packet.secs = 0; /* filled in by DHCP_ALGO_SendInform. */

    ip->client->packet.flags = 0; /* use unicast flag */

    if(ip->primary_address)
        memcpy (&(ip->client->packet.ciaddr), &(ip->primary_address),sizeof(ip->client->packet.ciaddr));
    else
        memset (&(ip->client->packet.ciaddr),0, sizeof(ip->client->packet.ciaddr));
    memset (&(ip->client->packet.yiaddr),0, sizeof(ip->client->packet.yiaddr));
    memset (&(ip->client->packet.siaddr),0, sizeof(ip->client->packet.siaddr));
    memset (&(ip->client->packet.giaddr),0, sizeof(ip->client->packet.giaddr));
    memcpy (ip->client->packet.chaddr,ip->hw_address.haddr, ip->hw_address.hlen);

}

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
void DHCP_ALGO_SendInform(void *ipp)
{
    struct interface_info *ip = ipp;
    struct in_addr from;
    struct sockaddr_in sockaddr_broadcast;
    int result;
    int increase = 1;
    TIME cur_time;
    UI32_T interval;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    sockaddr_broadcast.sin_family = AF_INET;
    sockaddr_broadcast.sin_port = L_STDLIB_Hton16(67);
    sockaddr_broadcast.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_BROADCAST);

    cur_time = (TIME)(SYSFUN_GetSysTick()/SYS_BLD_TICKS_PER_SECOND);

    /* Figure out how long it's been since we started transmitting. */

    interval = ((cur_time - ip->client->first_sending) > 0)? (cur_time - ip->client->first_sending)
    													:(ip->client->first_sending - cur_time);

    /* If we're past the panic timeout, call the script and tell it
       we haven't found anything for this interface yet. */
    /* timeout value is set when read_client_conf()*/
	if(!ip->client->config)
	{
		DHCP_BD(EVENT,"can't read client config");
		return;
	}

    if (interval > ip->client->config->timeout)
    {
        state_panic (ip, DHCP_TYPE_STATE_PANIC_DHCP_INFORM);
        return;
    }
    /* If we're selecting media, try the whole list before doing
       the exponential backoff, but if we've already received an
       offer, stop looping, because we obviously have it right. */

    /* If we're supposed to increase the interval, do so.  If it's
       currently zero (i.e., we haven't sent any packets yet), set
       it to one; otherwise, add to it a random number between
       zero and two times itself.  On average, this means that it
       will double with every transmission. */
    if (increase)
    {
        if (!ip->client->interval)
            ip->client->interval = ip->client->config->initial_interval;
        else
        {
            ip->client->interval += ((rand() >> 2) % (2 * ip->client->interval));
        }

        /* Don't backoff past cutoff. */
        if (ip->client->interval > ip->client->config->backoff_cutoff)
            ip->client-> interval =
                ((ip->client->config->backoff_cutoff / 2)
                 + ((rand() >> 2) %
                    ip->client->config->backoff_cutoff));
    }
    else if (!ip->client->interval)
        ip->client->interval = ip->client->config->initial_interval;

    /* If the backoff would take us to the panic timeout, just use that
       as the interval. */

    if (cur_time + ip->client->interval >
        ip->client->first_sending + ip->client->config->timeout)
        ip->client->interval =
            (ip->client->first_sending +
             ip->client->config->timeout) - cur_time + 1;

    /* Record the number of seconds since we started sending. */
    if (interval < 65536)
        ip->client->packet.secs = L_STDLIB_Hton16 (interval);
    else
        ip->client->packet.secs = L_STDLIB_Hton16 (65535);
    ip->client->secs = ip->client->packet.secs;

    /* set sip */
    memset(&from, 0, sizeof(from));
    if(ip->primary_address)
        memcpy (&from, &(ip->primary_address),sizeof(from));

    DHCP_BD(CLIENT, "DHCPINFORM on vid_ifindex[%lu] to %u.%u.%u.%u, port[%u]",
            (unsigned long)ip->vid_ifIndex, L_INET_EXPAND_IP(sockaddr_broadcast.sin_addr.s_addr), sockaddr_broadcast.sin_port);

    /* Send out a packet. */
    result = DHCP_TXRX_SendPktThruIML (ip, (struct packet *)0,
                  &ip->client->packet,
                  ip->client->packet_length,
                  from, &sockaddr_broadcast,
                  (struct hardware *)0);

    DHCP_TIME_add_timeout (cur_time + ip->client->interval, DHCP_ALGO_SendInform, ip);
}

/* FUNCTION NAME : DHCP_ALGO_StoreConfigFromAck
 * PURPOSE:
 *      To parse dhcpack's option to store dns server and default gateway
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
 *		This api is used only for dhcp inform client
 */
static void DHCP_ALGO_StoreConfigFromAck(struct packet *packet)
{
    struct interface_info *ip = packet->interface;
    UI32_T default_router_count=0;
    UI32_T name_srv_count=0;
    UI32_T i=0;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    struct client_lease *lease=NULL;
    SYSLOG_OM_RecordOwnerInfo_T owner_info;
    UI32_T log_vid=0;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_CLIENT))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    memset(&rif_config, 0, sizeof(rif_config));
    rif_config.ifindex = ip->vid_ifIndex;

    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
    {
        return;
    }

    /* parse option to set default gateway */
    default_router_count = (packet->options [DHO_ROUTERS].len)/4;

    if (default_router_count > 0)
    {
        for(i=0;i <default_router_count;i++)
        {
            /* we can only support one dhcp default geteway now */
            if(IP_LIB_IsIpBelongToSubnet(rif_config.addr.addr, rif_config.addr.preflen , packet->options[DHO_ROUTERS].data))
                if(FALSE == DHCP_MGR_SetClientDefaultGateway(ip->vid_ifIndex, packet->options[DHO_ROUTERS].data+i*4))
                {
                    break;
                }
        }
    }
    else
    {
        /* if we can't find default gateway, should we delete configuration ?*/
        DHCP_MGR_DeleteClientDefaultGateway(ip->vid_ifIndex );
    }

    /* parse option to set dns server */
#if (SYS_CPNT_DNS_FROM_DHCP == TRUE|| SYS_CPNT_DNS == TRUE )
    /* Add DNS Servers. */
    name_srv_count = (packet->options[DHO_DOMAIN_NAME_SERVERS].len)/4;

    if (name_srv_count > 0)
    {
        /* clear interface's name configured name server */
        DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex);

        for(i=0;i<name_srv_count;i++)
        {
            if(FALSE == DHCP_MGR_SetClientNameServer(ip->vid_ifIndex, (packet->options[DHO_DOMAIN_NAME_SERVERS].data)+(i*4)))
            {
                break;
            }
        }
    }
    else
    {
        /* if we can't find dns server, should we delete configuration ?*/
        DHCP_MGR_DeleteClientNameServer(ip->vid_ifIndex);
    }
#endif

    /* Record the server ip and the default gateway in dhcp interface */
    if(packet->options[DHO_DHCP_SERVER_IDENTIFIER].len >0)
        ip->server_ip = getULong(packet->options[DHO_DHCP_SERVER_IDENTIFIER].data);
    if(packet->options[DHO_ROUTERS].len >0)
        ip->gateway_ip = getULong(packet->options [DHO_ROUTERS].data);


    lease = packet_to_lease (packet);
    if (!lease)
        return;

    /* Replace the old active lease with the new one. */
    if(ip->client->active)
        free_client_lease (ip->client->active);

    ip->client->active = lease;

    /* Record server respond to syslog */
    memset(&owner_info, 0, sizeof(owner_info));
    owner_info.level   = SYSLOG_LEVEL_NOTICE;
    owner_info.module_no  = SYS_MODULE_DHCP;
    owner_info.function_no  = DHCP_TYPE_STORE_CONFIG_FROM_ACK;
    owner_info.error_no  = DHCP_TYPE_EH_Store_config_from_ack;
    VLAN_IFINDEX_CONVERTTO_VID(ip->vid_ifIndex, log_vid);
    SYSLOG_PMGR_AddFormatMsgEntry(&owner_info, DHCP_INFORM_RETRIEVE_SUCCESS_INDEX,(void *)&log_vid , 0, 0);
    return;
}

#endif

/* This function will still remain in mercury-> future: append to clparse.c*/
void read_client_conf(UI32_T vid_ifIndex)
{
    struct interface_info *ip, *interfaces, *ifp;


    /* Initialize the top level client configuration. */
    memset (&top_level_config, 0, sizeof top_level_config);

    /* Set some defaults... */
    top_level_config.timeout = 60;
    top_level_config.select_interval = 0;
    top_level_config.reboot_timeout = 10;
    top_level_config.retry_interval = 300;
    top_level_config.backoff_cutoff = 15;
    top_level_config.initial_interval = 3;
    top_level_config.bootp_policy = ACCEPT;
    top_level_config.script_name = 0; /* No script to use--> Penny */
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_SUBNET_MASK;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_BROADCAST_ADDRESS;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_TIME_OFFSET;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_ROUTERS;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_DOMAIN_NAME;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_DOMAIN_NAME_SERVERS;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_HOST_NAME;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_DHCP_TFTP_SERVER;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_DHCP_BOOTFILE_NAME;
    top_level_config.requested_options
        [top_level_config.requested_option_count++] =
            DHO_VENDOR_ENCAPSULATED_OPTIONS;

    ifp = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (ifp != NULL)
        interfaces = ifp;
    else
        return; /*interfaces = NULL;*/
    ip = interfaces;

    if (!ip->client)
    {
        ip->client = (struct client_state *) dhcp_malloc (sizeof (struct client_state));
        if (!ip->client)
        {
            DHCP_BD(CLIENT, "No memory for client state");
            return;
        }
        memset (ip->client, 0, sizeof(struct client_state));
    }

    if (!ip->client->config)
    {
		ip->client->config = (struct client_config *)dhcp_malloc(sizeof(struct client_config));
		if(!ip->client->config)
		{
			DHCP_BD(CLIENT, "No memory for client config");
			return;
		}

		memcpy(ip->client->config, &top_level_config, sizeof(struct client_config));
    }

    DHCP_OM_SetIfClientState(vid_ifIndex, ip->client);


}

#if (SYS_CPNT_DHCP_SERVER == TRUE)
/* read config file for DHCP Server in hornet */
void readconf()
{

	/* Set up the global defaults... */
	root_group.default_lease_time 		= DHCP_WA_DEFAULT_LEASE_TIME;
	root_group.max_lease_time			= DHCP_WA_INFINITE_LEASE_TIME;
	root_group.bootp_lease_cutoff 		= DHCP_WA_INFINITE_LEASE_TIME;
	root_group.boot_unknown_clients 	= 1;
	root_group.allow_bootp 				= 1;
	root_group.allow_booting 			= 1;
	root_group.authoritative 			= 1;
    root_group.dynamic_bootp             = 1;

	/* 1. check network config */
	DHCP_ALGO_ParseNetworkConfig(&root_group);

	/* 2. check host config */
	DHCP_ALGO_ParseHostConfig(&root_group);

}

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
BOOL_T locate_network(struct packet *packet)
{
    struct iaddr ia;
    struct subnet *subnet;

	/* If this came through a gateway, find the corresponding subnet... */
    if (packet->raw->giaddr)
    {
        DHCP_BD(SERVER, "From relay agent %u.%u.%u.%u", L_INET_EXPAND_IP(packet->raw->giaddr));

		ia.len = 4;
		memcpy (ia.iabuf, &packet->raw->giaddr, 4);
		subnet = find_subnet (ia);

		if (subnet)
			packet->shared_network = subnet->shared_network;
		else
			packet->shared_network = (struct shared_network *)0;
	}
    else
    {
        packet->shared_network = packet->interface->shared_network;
    }

	if (packet->shared_network)
		return TRUE;
	return FALSE;

} /*  end of locate_network */

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
void DHCP_ALGO_ParseNetworkConfig(struct group *group)
{
    DHCP_TYPE_PoolConfigEntry_T *tmp_pool;
    struct shared_network *share;
    UI32_T ip_address, sub_netmask;

    ip_address = 0;
    sub_netmask = 0;
    tmp_pool = DHCP_WA_GetNextNetworkPoolbyIpMask(ip_address, sub_netmask);
    while (tmp_pool)
    {
    	share = new_shared_network ("parse_statement");
		if (!share)
		{
            DHCP_BD(SERVER, "No memory for shared subnet");
			return;
		}
		share->group = clone_group (group, "parse_statement:subnet");
		share->group->shared_network = share;


		parse_subnet_declaration(tmp_pool, share);
#if 0
		/*share->group = clone_group (share->subnet->group, "parse_statement:subnet");
		share->group->shared_network = share;*/
#endif
		/* share->subnets is the subnet we just parsed. */

		if (share->subnets)
		{
            share->interface = share->subnets->interface;

			/* Make the shared network name from network number. */
			if(share->name)
				memcpy(share->name, &share->subnets->net, sizeof(UI32_T));

			/* Copy the authoritative parameter from the subnet,
			   since there is no opportunity to declare it here. */
			share->group->authoritative =
				share->subnets->group->authoritative;
			enter_shared_network (share);
		}
    	tmp_pool = DHCP_WA_GetNextNetworkPoolbyIpMask(tmp_pool->network_address, tmp_pool->sub_netmask);

	} /* end of while (tmp_pool) */

} /* end of DHCP_ALGO_ParseNetworkConfig */

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
void DHCP_ALGO_ParseHostConfig(struct group *group)
{
	struct host_decl *host;
	unsigned char addr [4];
	int len = sizeof addr;
	DHCP_TYPE_PoolConfigEntry_T *pool_config;
	char *pool_name = NULL;
	struct hardware hardware;
	struct tree *rv, *tree = (struct tree *)0;

    pool_config = DHCP_WA_GetNextHostPoolbyPoolName(pool_name);
    while (pool_config)
    {
    	/* 1. Malloc host_decl structure in order to store host config from WA */
    	host = (struct host_decl *)dmalloc (sizeof (struct host_decl),
					    "parse_host_declaration");
		if (!host)
		{
            DHCP_BD(SERVER, "Failed to allocate host decl struct.");
			return;
		}

		/* 2. get pool name as a host name, and clone group as default */
		if(host->name)
			memcpy(host->name, pool_name, sizeof(*pool_name));
		host->group = clone_group (group, "parse_host_declaration");

		/* 3. 3. get Mac address (as parsing 'Hardware') or CID if any */
		if (pool_config->hardware_address.hlen != 0)
		{
			hardware.htype = pool_config->hardware_address.htype;
			hardware.hlen = pool_config->hardware_address.hlen;
			memcpy(hardware.haddr, pool_config->hardware_address.haddr, pool_config->hardware_address.hlen );
    		host->interface = hardware;
    	}
    	/* 4. get fixed address (as parsing 'FIXED_ADDR') 	 */
    	if (pool_config->host_address != 0)
    	{
    		memcpy(addr, &pool_config->host_address, sizeof(UI32_T));
    		rv = tree_const (addr, len);
    		/*if (tree)
				tree = tree_concat (tree, rv);
			else*/
				tree = rv;

			host->fixed_addr = tree_cache (tree);

    	}

    	/* 5. Get Option config */
		DHCP_WA_ParseOptionConfig(pool_config, host->group);
    	enter_host (host);
    	pool_config = DHCP_WA_GetNextHostPoolbyPoolName(pool_config->pool_name);
    } /* end of while (pool_config)*/

} /* end of DHCP_ALGO_ParseHostConfig */

struct tree *parse_option_param(UI32_T option_code, DHCP_TYPE_PoolConfigEntry_T *pool_config)
{
	struct tree *tree = (struct tree *)0;
	struct tree *t;
	UI8_T tmp_type_str[2]={0};
	UI8_T addr[4]={0};
	int i = 0;

	switch (option_code)
	{
		case DHO_DOMAIN_NAME:
			t = tree_const((unsigned char *)pool_config->options.domain_name,
								 strlen(pool_config->options.domain_name));
			tree = tree_concat(tree, t);
			break;
		case DHO_NETBIOS_NODE_TYPE:
			*tmp_type_str = (UI8_T)pool_config->options.netbios_node_type;
			t = tree_const(tmp_type_str, 1);
			tree = tree_concat(tree,t);
			break;
		case DHO_NETBIOS_NAME_SERVERS:
			for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER; i++)
			{
				if (pool_config->options.netbios_name_server[i] != 0)
				{
					memcpy(addr, &pool_config->options.netbios_name_server[i], sizeof(UI32_T));
					tree = tree_concat(tree, tree_const (addr, 4));
				}
				else
					break;
			}
			break;
		case DHO_DOMAIN_NAME_SERVERS:
			for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER; i++)
			{
				if (pool_config->options.dns_server[i] != 0)
				{
					memcpy(addr, &pool_config->options.dns_server[i], sizeof(UI32_T));
					tree = tree_concat(tree, tree_const (addr, 4));
				}
				else
					break;
			}
			break;
		case DHO_ROUTERS:
			for (i = 0; i < SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER; i++)
			{
				if (pool_config->options.default_router[i] != 0)
				{
					memcpy(addr, &pool_config->options.default_router[i], sizeof(UI32_T));
					tree = tree_concat(tree, tree_const (addr, 4));
				}
				else
					break;
			}
			break;
		case DHO_DHCP_CLIENT_IDENTIFIER:
			tree = tree_concat(tree, tree_const((unsigned char *)pool_config->options.cid.id_buf,
								 (int)pool_config->options.cid.id_len));
			break;

	}
	return tree;

} /*end of parse_option_param */

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
 *		1. Range calculation:
 */
void parse_subnet_declaration(DHCP_TYPE_PoolConfigEntry_T *pool, struct shared_network *share)
{
	struct subnet *subnet, *t, *u;
	struct iaddr iaddr;
	UI32_T temp_address;

	subnet = new_subnet ("parse_subnet_declaration");
	if (!subnet)
	{
        DHCP_BD(SERVER, "No memory for new subnet");
		return;
	}

	subnet->shared_network = share;
    subnet->group = clone_group(share->group, "parse_subnet_declaration");
	subnet->group->subnet = subnet;

	/* Get the network number... */
	/* Penny, 2002-9-17: we suppose only take the network address which already mask off
     * the host bit
	*/
	memset (iaddr.iabuf, 0, 16);
	temp_address = pool->network_address & pool->sub_netmask;
	memcpy (iaddr.iabuf, &temp_address, sizeof(UI32_T));
	iaddr.len = sizeof(UI32_T);
	subnet->net = iaddr;

	/* Get the netmask... */
	memcpy (iaddr.iabuf, &pool->sub_netmask, sizeof(UI32_T));
	iaddr.len = sizeof(UI32_T);
	subnet->netmask = iaddr;

	enter_subnet (subnet);

	/* Parse the rest of option config for network pool including calculation RANGE */

	/* a. Calculate out the RANGE for this subnet and set it to subnet */
	DHCP_WA_GetNetworkRange(subnet, pool->network_address, pool->sub_netmask);

	/* b. Get Option config */
	DHCP_WA_ParseOptionConfig(pool, subnet->group);

	/* If this subnet supports dynamic bootp, flag it so in the
	   shared_network containing it. */
	if (subnet->group->dynamic_bootp)
		share->group->dynamic_bootp = 1;
	if (subnet->group->one_lease_per_client)
		share->group->one_lease_per_client = 1;

	/* Add the subnet to the list of subnets in this shared net. */
	if (!share->subnets)
		share->subnets = subnet;
    else
    {
		u = (struct subnet *)0;

        for (t = share->subnets; t; t = t->next_sibling)
        {
            if (subnet_inner_than(subnet, t, 0))
            {
				if (u)
					u->next_sibling = subnet;
				else
					share->subnets = subnet;
				subnet->next_sibling = t;
				return;
			}
			u = t;
		}
		u->next_sibling = subnet;
	}

} /* end of parse_subnet_declaration */
#endif

UI32_T getULong(buf)
    unsigned char *buf;
{
    unsigned long ibuf;
    int size;

    if (buf == NULL)
        return 0;

    size = (sizeof(buf) > 4)? 4:sizeof(buf);
    memcpy (&ibuf, buf, size);
    return L_STDLIB_Ntoh32 (ibuf);
}

UI16_T getUShort(buf)
    unsigned char *buf;
{
    unsigned short ibuf;
    int size;

    if (buf == NULL)
        return 0;

    size = (sizeof(buf) > 2)? 2:sizeof(buf);
    memcpy (&ibuf, buf, size);
    return L_STDLIB_Ntoh16 (ibuf);
}


/* cons options into a big buffer, and then split them out into the
   three seperate buffers if needed.  This allows us to cons up a set
   of vendor options using the same routine. */
int cons_options(inpacket, outpacket, mms, options, overload, terminate, bootpp, prl, prl_len)
    struct packet *inpacket;
    struct dhcp_packet *outpacket;
    int mms;
    struct tree_cache **options;
    int overload;   /* Overload flags that may be set. */
    int terminate;
    int bootpp;
    UI8_T *prl;
    int prl_len;
{
    unsigned char priority_list [300];
    int priority_len;
    /*unsigned char buffer [1024]; */   /* Penny change from 4096 to 1024: Really big buffer... */
    int main_buffer_size;
    int mainbufix, bufix;
    int option_size;
    int length =0;

    /* If the client has provided a maximum DHCP message size,
       use that; otherwise, if it's BOOTP, only 64 bytes; otherwise
       use up to the minimum IP MTU size (576 bytes). */
    /* XXX if a BOOTP client specifies a max message size, we will
       honor it. */
    if (!mms &&
        inpacket &&
        inpacket->options [DHO_DHCP_MAX_MESSAGE_SIZE].data &&
        (inpacket->options [DHO_DHCP_MAX_MESSAGE_SIZE].len >=
         sizeof (UI16_T)))
	{

            mms = getUShort (inpacket->options
                 [DHO_DHCP_MAX_MESSAGE_SIZE].data);
	}
    /* If the client has provided a maximum DHCP message size,
       use that; otherwise, if it's BOOTP, only 64 bytes; otherwise
       use up to the minimum IP MTU size (576 bytes). */
    /* XXX if a BOOTP client specifies a max message size, we will
       honor it. */
    if (mms)
        main_buffer_size = mms - DHCP_FIXED_LEN;
    else if (bootpp)
        main_buffer_size = 64;
    else
        main_buffer_size = 576 - DHCP_FIXED_LEN;

    if (main_buffer_size > sizeof buffer)
        main_buffer_size = sizeof buffer;

    /* Preload the option priority list with mandatory options. */
    priority_len = 0;
    priority_list [priority_len++] = DHO_DHCP_MESSAGE_TYPE;
    priority_list [priority_len++] = DHO_DHCP_SERVER_IDENTIFIER;
    priority_list [priority_len++] = DHO_DHCP_LEASE_TIME;
    priority_list [priority_len++] = DHO_DHCP_MESSAGE;

    /* If the client has provided a list of options that it wishes
       returned, use it to prioritize.  Otherwise, prioritize
       based on the default priority list. */

    if (inpacket &&
            inpacket->options[DHO_DHCP_PARAMETER_REQUEST_LIST].data)
    {
        int prlen = (inpacket->options[DHO_DHCP_PARAMETER_REQUEST_LIST].len);

        if (prlen + priority_len > sizeof priority_list)
            prlen = (sizeof priority_list) - priority_len;

        memcpy(&priority_list [priority_len], (inpacket->options[DHO_DHCP_PARAMETER_REQUEST_LIST].data), prlen);

        priority_len += prlen;
        prl = priority_list;
    }

    if (prl)
    {
        if (prl_len + priority_len > sizeof priority_list)
            prl_len = (sizeof priority_list) - priority_len;

        memcpy (&priority_list [priority_len], prl, prl_len);

        priority_len += prl_len;
        prl = priority_list;
    }
    else
    {
        memcpy(&priority_list [priority_len], dhcp_option_default_priority_list, sizeof_dhcp_option_default_priority_list);
        priority_len += sizeof_dhcp_option_default_priority_list;

    }


    /* Copy the options into the big buffer... */

    option_size = store_options(buffer,
                     (main_buffer_size - 7 +
                      ((overload & 1) ? DHCP_FILE_LEN : 0) +
                      ((overload & 2) ? DHCP_SNAME_LEN : 0)),
                     options, priority_list, priority_len,
                     main_buffer_size,
                     (main_buffer_size +
                      ((overload & 1) ? DHCP_FILE_LEN : 0)),
                     terminate);


    /* Put the cookie up front... */
    memcpy (outpacket->options, DHCP_OPTIONS_COOKIE, 4);

    mainbufix = 4;

    /* If we're going to have to overload, store the overload
       option at the beginning.  If we can, though, just store the
       whole thing in the packet's option buffer and leave it at
       that. */
    if (option_size <= main_buffer_size - mainbufix)
    {

        memcpy (&outpacket->options [mainbufix], buffer, option_size);
        mainbufix += option_size;
        if (mainbufix < main_buffer_size)
            outpacket->options [mainbufix++] = DHO_END;
        length = DHCP_FIXED_NON_UDP + mainbufix;
    }
    else
    {
        outpacket->options [mainbufix++] = DHO_DHCP_OPTION_OVERLOAD;
        outpacket->options [mainbufix++] = 1;
        if (option_size > main_buffer_size - mainbufix + DHCP_FILE_LEN)
            outpacket->options [mainbufix++] = 3;
        else
            outpacket->options [mainbufix++] = 1;

        memcpy (&outpacket->options [mainbufix],buffer, main_buffer_size - mainbufix);
        bufix = main_buffer_size - mainbufix;
        length = DHCP_FIXED_NON_UDP + mainbufix;

		/* if bufix is not in the valid range, return option length = 0 */
		if((bufix<0)||(bufix>(sizeof(buffer)-1)))
		{
			DHCP_BD(EVENT, "option buffer access out of range.");
			return 0;
		}

        if (overload & 1)
        {
            if (option_size - bufix <= DHCP_FILE_LEN)
            {
                memcpy (outpacket->file, &buffer [bufix], option_size - bufix);
                mainbufix = option_size - bufix;
                if (mainbufix < DHCP_FILE_LEN)
                    outpacket->file [mainbufix++] = DHO_END;
                while (mainbufix < DHCP_FILE_LEN)
                    outpacket->file [mainbufix++] = DHO_PAD;
            }
            else
            {
                memcpy (outpacket->file, &buffer [bufix], DHCP_FILE_LEN);
                bufix += DHCP_FILE_LEN;
            }
        }
        if ((overload & 2) && option_size < bufix)
        {
            memcpy (outpacket->sname, &buffer [bufix], option_size - bufix);
            mainbufix = option_size - bufix;

            if (mainbufix < DHCP_SNAME_LEN)
                outpacket->file [mainbufix++] = DHO_END;
            while (mainbufix < DHCP_SNAME_LEN)
                outpacket->file [mainbufix++] = DHO_PAD;
        }

    }/*end else*/

    return (length);
}

/* Store all the requested options into the requested buffer. */
int store_options(buffer, buflen, options, priority_list, priority_len, first_cutoff, second_cutoff, terminate)
    unsigned char *buffer;
    int buflen;
    struct tree_cache **options;
    unsigned char *priority_list;
    int priority_len;
    int first_cutoff, second_cutoff;
    int terminate;
{
    int bufix = 0;
    int option_stored [256];
    int i;
    int ix;
    int tto;

    /* Zero out the stored-lengths array. */
    memset (option_stored, 0, sizeof option_stored);

    /* Copy out the options in the order that they appear in the
       priority list... */

    for (i = 0; i < priority_len; i++)
    {
        /* Code for next option to try to store. */
        int code = priority_list [i];
        int optstart;

        /* Number of bytes left to store (some may already
           have been stored by a previous pass). */
        int length;
/*DHCP_ERROR_Print("options [code]->len is %d", options [code]->len);*/

        /* If no data is available for this option, skip it. */

        if (!options [code])
        {
            continue;
        }

        /* The client could ask for things that are mandatory,
           in which case we should avoid storing them twice... */
        if (option_stored [code])
            continue;
        option_stored [code] = 1;

        /* Find the value of the option... */
        if (!tree_evaluate(options [code]))
        {
            continue;
        }

        /* We should now have a constant length for the option. */
        length = options [code]->len;

		tto = 0;
        /* Try to store the option. */

        /* If the option's length is more than 255, we must store it
           in multiple hunks.   Store 255-byte hunks first.  However,
           in any case, if the option data will cross a buffer
           boundary, split it across that boundary. */

        ix = 0;

        optstart = bufix;

        while (length)
        {
            unsigned char incr = length > 255 ? 255 : length;

            /* If this hunk of the buffer will cross a
               boundary, only go up to the boundary in this
               pass. */
            if (bufix < first_cutoff &&
                bufix + incr > first_cutoff)
                incr = first_cutoff - bufix;
            else if (bufix < second_cutoff &&
                 bufix + incr > second_cutoff)
                incr = second_cutoff - bufix;

            /* If this option is going to overflow the buffer,
               skip it. */
            if (bufix + 2 + incr > buflen)
            {
                bufix = optstart;
                break;
            }

            /* Everything looks good - copy it in! */
            buffer [bufix] = code;
            buffer [bufix + 1] = incr;

            if (tto && incr == length)
            {
                memcpy (buffer + bufix + 2,
                    options [code]->value + ix,
                    incr - 1);
                buffer [bufix + 2 + incr - 1] = 0;
            }
            else
            {
                memcpy (buffer + bufix + 2,
                    options [code]->value + ix, incr);
            }
            length -= incr;
            ix += incr;
            bufix += 2 + incr;
        }
    }
    return bufix;
}


/* FUNCTION	NAME : read_client_leases
 * PURPOSE:
 *		Read client lease from UC or from FS. The lease contains the ip
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
 *		This function will let us retrieve the ip server gave us before and
 *		request the same ip again to the server.
 *
 */
void read_client_leases(UI32_T vid_ifIndex)
{
    struct interface_info *ip;
    struct client_lease *lease;
    DHCP_OM_UC_LEASE_DATA_T uc_data;
    UI32_T vid;

    /* get interface
     */
    ip = DHCP_OM_FindInterfaceByVidIfIndex(vid_ifIndex);
    if (!ip)
    	return;

    VLAN_IFINDEX_CONVERTTO_VID(vid_ifIndex, vid);
    if(!VLAN_POM_IsVlanExisted(vid))
    {
        ip->client->active = NULL;
        goto exit;
    }

    memset(&uc_data, 0, sizeof(uc_data));
    if(!DHCP_OM_GetUCLeaseData(&uc_data))
    {
        ip->client->active = NULL;
        goto exit;
    }

    if(uc_data.vid_ifindex != vid_ifIndex)
    {
        ip->client->active = NULL;
        goto exit;
    }

    lease = (struct client_lease *)dhcp_malloc(sizeof (struct client_lease));
    if (!lease)
    {
        DHCP_BD(CLIENT, "No memory for allocating lease");
        ip->client->active = NULL;
	goto exit;
    }

    memset (lease, 0, sizeof *lease);
    lease->address.len = (int)uc_data.address.len;
    memcpy(lease->address.iabuf, uc_data.address.iabuf, sizeof(lease->address.iabuf));
    lease->expiry  = (TIME) uc_data.expiry;
    lease->renewal = (TIME) uc_data.renewal;
    lease->rebind  = (TIME) uc_data.rebind;
    lease->is_bootp &= (uc_data.is_bootp)?1:0;
    ip->client->active = lease;

exit:
    DHCP_OM_SetIfClientState(ip->vid_ifIndex, ip->client);
    return;
}

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
void write_client_lease(UI32_T vid_ifIndex, struct client_lease *lease)
{
   DHCP_OM_UC_LEASE_DATA_T uc_data;

    if(!lease)
        return;

    memset(&uc_data, 0, sizeof(uc_data));
    uc_data.vid_ifindex = vid_ifIndex;
    uc_data.address.len = (UI32_T)lease->address.len;
    memcpy(uc_data.address.iabuf, lease->address.iabuf, sizeof(uc_data.address.iabuf));
    uc_data.expiry  = (UI32_T)lease->expiry;
    uc_data.renewal = (UI32_T)lease->renewal;
    uc_data.rebind  = (UI32_T)lease->rebind;
    uc_data.is_bootp = (lease->is_bootp)?TRUE:FALSE;
    DHCP_OM_SetUCLeaseData(&uc_data);
    return;
}

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
UI32_T DHCP_ALGO_GetNaturalSubnetMask(UI32_T ip)
{
	UI32_T mask, sub_netmask = 0;

    mask = (ip >> 24) & 0xff;
	if ( (0x0 <= mask) && (mask <= 0x7f))
		sub_netmask = 0xff000000;
	else if ((0x80 <= mask) && (mask <= 0xbf))
		sub_netmask = 0xffff0000;
	else if ((0xc0 <= mask) && (mask <= 0xdf))
		sub_netmask = 0xffffff00;

	return sub_netmask;

} /*  end of DHCP_ALGO_GetNaturalSubnetMask */

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
BOOL_T DHCP_ALGO_IsSocketDirty(void)
{
    if(socket_num == 0)
        return FALSE;
    else
        return(socket_dirty);
}

/* FUNCTION	NAME : DHCP_ALGO_ReleaseClientLease
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
BOOL_T DHCP_ALGO_ReleaseClientLease(UI32_T ifindex)
{
    struct interface_info	*if_p;

    if_p = DHCP_OM_FindInterfaceByVidIfIndex(ifindex);

    if(if_p == NULL)
    {
        return FALSE;
    }

    if ((if_p->client == NULL) ||(if_p->client->active == NULL))
    {
        return TRUE;
    }

    /* make DHCPRELEASE packet and send out */
    make_release(if_p, if_p->client->active);
    send_release(if_p);
#if 0  /* move to DHCP_ALGO_DeleteLease() */
    /* free global write uc lease*/
    DHCP_ALGO_FreeGlobalUCWriteLease(ifindex);
    /*DHCP_ALGO_WriteClientLeasesToUC();*/  /* UC design must be changed */
#endif
    /* destroy client's lease ip */
    DHCP_ALGO_DeleteLease(if_p);

    /* free active lease in engine */
    free_client_lease(if_p->client->active);
    if_p->client->active = NULL;

    return TRUE;
}


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
BOOL_T DHCP_ALGO_FreeGlobalUCWriteLease(UI32_T ifindex)
{
    struct dhcp_uc_lease  *tmp_lease_p,*previous_lease_p;

    tmp_lease_p = uc_write_lease_P;
    previous_lease_p = NULL;
    while(tmp_lease_p)
    {
        if(tmp_lease_p->vid_ifIndex == ifindex)
        {
            /* remove head */
            if(previous_lease_p == NULL)
            {
                uc_write_lease_P = tmp_lease_p->next;
                dhcp_free(tmp_lease_p);

                return TRUE;
            }
            else
            {
                tmp_lease_p->next = previous_lease_p->next;
                dhcp_free(tmp_lease_p);

                return TRUE;
            }

        }
        previous_lease_p = tmp_lease_p;
        tmp_lease_p = tmp_lease_p->next;

    }

    return FALSE;
}

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
BOOL_T DHCP_ALGO_SetDhcpPacketRule(void)
{
    UI32_T system_role = DHCP_TYPE_BIND_NONE;
    DHCP_OM_GetSystemRole(&system_role);
    if(system_role & DHCP_TYPE_BIND_CLIENT)
    {
        if(FALSE == SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_DHCP_CLIENT))
            return FALSE;
    }
    else
    {
        if(FALSE == SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_DHCP_CLIENT))
            return FALSE;
    }
    /* L3 switch support DHCP relay and server */
#if(SYS_CPNT_ROUTING == TRUE)
    if(system_role & DHCP_TYPE_BIND_SERVER)
    {
        if(FALSE == SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_DHCP_SERVER))
            return FALSE;
    }
    else
    {
        if(FALSE == SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_DHCP_SERVER))
            return FALSE;
    }

    if(system_role & DHCP_TYPE_BIND_RELAY)
    {
        if(FALSE == SWCTRL_PMGR_EnableDhcpTrap(SWCTRL_DHCP_TRAP_BY_L3_RELAY))
            return FALSE;
    }
    else
    {
        if(FALSE == SWCTRL_PMGR_DisableDhcpTrap(SWCTRL_DHCP_TRAP_BY_L3_RELAY))
            return FALSE;
    }
#endif
    return TRUE;
}

#if 0
struct lease *mockup_lease (packet, share, hp)
	struct packet *packet;
	struct shared_network *share;
	struct host_decl *hp;
{

	static struct lease mock;
	struct iaddr addr;

	memcpy(addr.iabuf, &mock.ip_addr, 4);
	addr.len = 4;

	mock.subnet = find_host_for_network(&hp, &addr, share);
	if (!mock.subnet)
		return (struct lease *)0;
	mock.next = mock.prev = (struct lease *)0;
	mock.shared_network = mock.subnet->shared_network;
	mock.host = hp;

    if (hp->group->options [DHO_DHCP_CLIENT_IDENTIFIER])
    {
		mock.uid = hp->group ->
			options [DHO_DHCP_CLIENT_IDENTIFIER]->value;
		mock.uid_len = hp->group ->
			options [DHO_DHCP_CLIENT_IDENTIFIER]->len;
    }
    else
    {
		mock.uid = (unsigned char *)0;
		mock.uid_len = 0;
	}

	mock.hardware_addr = hp->interface;
	mock.starts = mock.timestamp = mock.ends = MIN_TIME;
	mock.flags = STATIC_LEASE;
	return &mock;

}
#endif

int addr_eq(addr1, addr2)
	struct iaddr addr1, addr2;
{
	if (addr1.len != addr2.len)
		return 0;
	return memcmp (addr1.iabuf, addr2.iabuf, addr1.len) == 0;
}

struct iaddr subnet_number(addr, mask)
	struct iaddr addr;
	struct iaddr mask;
{
	int i;
	struct iaddr rv;

	rv.len = 0;

	/* Both addresses must have the same length... */
	if (addr.len != mask.len)
		return rv;

	rv.len = addr.len;
	for (i = 0; i < rv.len; i++)
		rv.iabuf [i] = addr.iabuf [i] & mask.iabuf [i];
	return rv;
}

char *piaddr(addr)
	struct iaddr addr;
{
	static char pbuf [4 * 16];
	char *s = pbuf;
	int i;

    if (addr.len == 0)
    {
		strcpy (s, "<null address>");
	}

    for (i = 0; i < addr.len; i++)
    {
		sprintf (s, "%s%d", i ? "." : "", addr.iabuf [i]);
		s += strlen (s);
	}
	return pbuf;
}

UI32_T host_addr(addr, mask)
	struct iaddr addr;
	struct iaddr mask;
{
	int i;
	UI32_T swaddr;
	struct iaddr rv;

	rv.len = 0;

	/* Mask out the network bits... */
	rv.len = addr.len;
	for (i = 0; i < rv.len; i++)
		rv.iabuf [i] = addr.iabuf [i] & ~mask.iabuf [i];

	/* Copy out up to 32 bits... */
	memcpy (&swaddr, &rv.iabuf [rv.len - sizeof swaddr], sizeof swaddr);

	/* Swap it and return it. */
	return L_STDLIB_Ntoh32 (swaddr);
}

/* Combine a network number and a integer to produce an internet address.
   This won't work for subnets with more than 32 bits of host address, but
   maybe this isn't a problem. */

struct iaddr ip_addr (subnet, mask, host_address)
	struct iaddr subnet;
	struct iaddr mask;
	UI32_T host_address;
{
	int i, j, k;
	UI32_T swaddr;
	struct iaddr rv;
	unsigned char habuf [sizeof swaddr];

	swaddr = L_STDLIB_Hton32 (host_address);
	memcpy (habuf, &swaddr, sizeof swaddr);

	/* Combine the subnet address and the host address.   If
	   the host address is bigger than can fit in the subnet,
	   return a zero-length iaddr structure. */
	rv = subnet;
	j = rv.len - sizeof habuf;

    for (i = sizeof habuf - 1; i >= 0; i--)
    {
        if (mask.iabuf [i + j])
        {
            if (habuf[i] > (mask.iabuf [i + j] ^ 0xFF))
            {
				rv.len = 0;
				return rv;
			}

            for (k = i - 1; k >= 0; k--)
            {
                if (habuf [k])
                {
					rv.len = 0;
					return rv;
				}
			}
			rv.iabuf [i + j] |= habuf [i];
			break;
        }
        else
			rv.iabuf [i + j] = habuf [i];
	}

	return rv;
}


#if (SYS_CPNT_DHCP_RELAY_OPTION82 ==TRUE)
/* FUNCTION	NAME : DHCP_ALGO_Handle_Option82
 * PURPOSE:
 *
 *
 * INPUT:
 		vidIfIndex             --  vlan ifindex
 *		mem_ref_p			--  holder of received packet buffer.
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
 *		This routine will insert option82 field in REQUEST packet and delete the field from
              REPLY packet.
 */
void DHCP_ALGO_Handle_Option82(
				UI32_T vidIfIndex,
				L_MM_Mref_Handle_T *mem_ref_p,
				UI32_T packet_length,
				UI32_T rxRifNum,
				UI8_T *dst_mac,
				UI8_T *src_mac,
				const UI32_T vid,
				UI32_T src_lport_ifIndex)
{
    DHCP_TYPE_IpHeader_T        *ip_header_p, *buf_ip_header_p;
    DHCP_TYPE_UdpHeader_T       *udp_header_p, *buf_udp_header_p;
    UI32_T                      ip_header_len, udp_header_len, dhcp_packet_len;
    struct dhcp_packet	        *dhcp_packet_p, *buf_dhcp_pkt_p;
    UI8_T                       *buffer_p, *buffer_request_p;
    UI32_T                      buffer_size=0, buffer_dhcp_len=0, buffer_packet_len;
    L_MM_Mref_Handle_T                    *mref_p;
    NETCFG_TYPE_InetRifConfig_T rifNode;
    UI8_T                       destinate_mac[SYS_ADPT_MAC_ADDR_LEN], source_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T                       broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    UI8_T                       cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T                      mgmt_intf_ip = 0;
    UI32_T                      client_vid = 0, client_unit = 0, client_port = 0;
    UI32_T                      calculate_length;
    UI32_T                      send_out_ifindex = 0, send_out_vid = 0;
    UI32_T                      ingress_ifindex=0;
    UI32_T                      active_relay_server = 0, relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER]={0};  /* 2006-11, Joseph */
    UI32_T                      pdu_len = 0;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* check if input pointer is NULL */
    if(NULL == mem_ref_p)
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return;
    }

    if((NULL == dst_mac)||
       (NULL == src_mac))
    {
        L_MM_Mref_Release(&mem_ref_p);
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return;
    }

    memcpy(destinate_mac, dst_mac, SYS_ADPT_MAC_ADDR_LEN);
    memcpy(source_mac, src_mac, SYS_ADPT_MAC_ADDR_LEN);

    /* prepare point to IP header, UDP header, DHCP packet, and calculate their length */
    ip_header_p = (DHCP_TYPE_IpHeader_T*) L_MM_Mref_GetPdu(mem_ref_p,&pdu_len);
    ip_header_len = (0x0f & ip_header_p->ip_ver_hlen) * 4;
    udp_header_p = (DHCP_TYPE_UdpHeader_T*) ((UI8_T*)ip_header_p + ip_header_len);
    udp_header_len = sizeof(DHCP_TYPE_UdpHeader_T);
    dhcp_packet_p = (struct dhcp_packet*) ((UI8_T*)udp_header_p + udp_header_len);
    dhcp_packet_len	= packet_length - ip_header_len - udp_header_len; /* dhcp_packet_len will include the zero padding  */

    DHCP_BD(RELAY, "dhcp packet len[%lu], packet length[%lu],ip header len[%lu],udp header len[%lu]",
        (unsigned long)dhcp_packet_len, (unsigned long)packet_length, (unsigned long)ip_header_len, (unsigned long)udp_header_len);

    if ( DHCP_OM_OK == DHCP_OM_GetRelayServerAddress(relay_server))
    {
        DHCP_BD(RELAY, "After get relay server address");
    }

    /* exception cases checking, if a case happened , flood the packet */
    if (DHCP_ALGO_RelayExceptionCheck(vid, dhcp_packet_p, relay_server[0]) != TRUE)
    {
        DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
        return;
    }

    if (dhcp_packet_p->op == BOOTREQUEST)
    {
        /* RFC 1542 4.1.1
         * The relay agent MUST silently discard BOOTREQUEST messages whose
’h       * 'hops' field exceeds the value 16
         */
        if (dhcp_packet_p->hops > 16)
    	{
            DHCP_BD(RELAY, "Discarding packet with invalid hops(%u)", dhcp_packet_p->hops);
            L_MM_Mref_Release(&mem_ref_p);
    		return;
    	}

        /* if it's BOOTREQUEST packet */
        DHCP_BD(RELAY, "--- REQUEST ---");
        if (DHCP_ALGO_FindActiveRelayServer(ip_header_p, destinate_mac, &send_out_ifindex, &active_relay_server, relay_server) != TRUE)
        {
            DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
            return;
        }

        /* get ip address from egress interface as relay agent ip address */
        memset(&rifNode, 0, sizeof(rifNode));
        rifNode.ifindex = send_out_ifindex;
        if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rifNode))
        {
            memcpy(&mgmt_intf_ip, rifNode.addr.addr, sizeof(mgmt_intf_ip));
            DHCP_BD(RELAY, "mgmt intf ip[%u.%u.%u.%u], rifnode addr[%u.%u.%u.%u]",
                    L_INET_EXPAND_IP(mgmt_intf_ip), L_INET_EXPAND_IP(rifNode.addr.addr));
        }

        /* need to calculate the tx buffer size */
        if(FALSE == DHCP_ALGO_CalculateTxBufferSize(dhcp_packet_p, ip_header_len, udp_header_len,dhcp_packet_len,mgmt_intf_ip, &buffer_size))
        {
            DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
            return;
        }

        mref_p = L_MM_AllocateTxBuffer(buffer_size, L_MM_USER_ID2(SYS_MODULE_DHCP, DHCP_TYPE_TRACE_ID_DHCP_ALGO_HANDLE_OPTION82));

        if (mref_p == NULL)
        {
            DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
            return;
        }

        mref_p->current_usr_id = SYS_MODULE_DHCP;
        buffer_p = L_MM_Mref_GetPdu(mref_p,&pdu_len);
        /* initial packet buffer */
        memset(buffer_p, 0, buffer_size);

    	buffer_request_p = buffer_p + ip_header_len + udp_header_len;


        if (DHCP_ALGO_RequestPacketProcess(buffer_request_p, &buffer_dhcp_len, dhcp_packet_p, dhcp_packet_len, vid, src_lport_ifIndex, mgmt_intf_ip) != TRUE)
        {
        	L_MM_Mref_Release(&mref_p);
            DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
            return;
    	}

        DHCP_BD(RELAY, "buffter dhcp packet len[%lu]", (unsigned long)buffer_dhcp_len);

    	memcpy(buffer_p, ip_header_p, ip_header_len);
    	memcpy(buffer_p + ip_header_len, udp_header_p, udp_header_len);

    	buf_ip_header_p = (DHCP_TYPE_IpHeader_T*) (buffer_p);
    	buf_udp_header_p = (DHCP_TYPE_UdpHeader_T*) (buffer_p + ip_header_len);
        buf_dhcp_pkt_p = (struct dhcp_packet*) ((UI8_T*)buf_udp_header_p + udp_header_len);

    	/* fill in dest ip, dest mac, dest port, source ip, source mac, source port */
    	buf_ip_header_p->sip = mgmt_intf_ip;
        buf_ip_header_p->dip = active_relay_server;
        SWCTRL_POM_GetCpuMac(cpu_mac);
	    memcpy(source_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
	    /* destinate_mac has been set in arpfind */

        /* RFC 2131, 4.7.2 Relay agent should use port 67 as the source port number
         */
        buf_udp_header_p->src_port = htons(SYS_DFLT_DHCP_SERVER_PORT);
        buf_udp_header_p->dst_port = htons(SYS_DFLT_DHCP_SERVER_PORT);
    	buf_udp_header_p->udp_header_len = udp_header_len + buffer_dhcp_len;
        buf_udp_header_p->udp_header_len = htons(buf_udp_header_p->udp_header_len);

    	if (buf_udp_header_p->chksum != 0)
    	{
        	buf_udp_header_p->chksum = 0;

         	calculate_length = DHCP_UDP_PSEUDO_HEADER_LENGTH + udp_header_len + buffer_dhcp_len;
         	CALCULATE_LENGTH_NO_TRUNCATE(calculate_length);

            DHCP_BD(RELAY, "buffer_dhcp_len[%ld],udp_header_len[%lu],cal_len[%lu]",
                    (long)buffer_dhcp_len, (unsigned long)udp_header_len, (unsigned long)calculate_length);

        	buf_udp_header_p->chksum = DHCP_ALGO_UdpChkSumCalculate(buf_ip_header_p, calculate_length);
    	}


        /*Ip checksum is header checksum */
    	buf_ip_header_p->tlen = buffer_dhcp_len + udp_header_len + ip_header_len;
        buf_ip_header_p->tlen = htons(buf_ip_header_p->tlen);

	    buf_ip_header_p->csum = 0;
  		buf_ip_header_p->csum = ipchksum((unsigned short *)buf_ip_header_p, ip_header_len);

    	/* calculate packet length	*/
    	buffer_packet_len = ip_header_len + udp_header_len + buffer_dhcp_len;
        L_MM_Mref_SetPduLen(mref_p,buffer_packet_len);
        L_MM_Mref_Release(&mem_ref_p);
        VLAN_IFINDEX_CONVERTTO_VID(send_out_ifindex, send_out_vid);
		DHCP_ALGO_PacketForward(DHCP_TYPE_RELAY_UNICAST, mref_p, buffer_packet_len, destinate_mac, source_mac, send_out_vid, 0, 0 );

    }
    else
    {
        /* if it's BOOTREPLY packet */
        DHCP_BD(RELAY, "--- REPLY ---");

        /* get ip address from ingress interface as relay agent ip address */
        VLAN_VID_CONVERTTO_IFINDEX(vid, ingress_ifindex);
        memset(&rifNode, 0, sizeof(rifNode));
        rifNode.ifindex = ingress_ifindex;
        if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rifNode))
        {
            memcpy(&mgmt_intf_ip, rifNode.addr.addr, sizeof(mgmt_intf_ip));
            DHCP_BD(RELAY, "mgmt intf ip[%u.%u.%u.%u], rifnode addr[%u.%u.%u.%u]",
                    L_INET_EXPAND_IP(mgmt_intf_ip), L_INET_EXPAND_IP(rifNode.addr.addr));
        }

        if (DHCP_ALGO_ReplyPacketProcess(dhcp_packet_p, dhcp_packet_len, &client_vid, &client_unit, &client_port, mgmt_intf_ip) != TRUE)
        {
            DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
            return;
        }
        else
        {
            /* there is no circuit id information, flood the packet */
            if (client_vid == 0 && client_unit == 0 && client_port == 0)
            {
                DHCP_ALGO_PacketForward(DHCP_TYPE_FLOODING, mem_ref_p, packet_length, destinate_mac, source_mac, vid, src_lport_ifIndex, 0);
                return;
            }

            /* fill in dest ip, dest mac, dest port, source ip, source mac, source port */
            if ( !(dhcp_packet_p->flags & htons(BOOTP_BROADCAST)) )
            {
                ip_header_p->dip = dhcp_packet_p->yiaddr;
                memcpy(destinate_mac, dhcp_packet_p->chaddr, dhcp_packet_p->hlen);
            }
            else
            {
                ip_header_p->dip = INADDR_BROADCAST;
                memcpy(destinate_mac, broadcast, SYS_ADPT_MAC_ADDR_LEN);
            }

            ip_header_p->sip = mgmt_intf_ip;
            SWCTRL_POM_GetCpuMac(cpu_mac);
	        memcpy(source_mac, cpu_mac, SYS_ADPT_MAC_ADDR_LEN);
	        udp_header_p->src_port = htons(SYS_DFLT_DHCP_SERVER_PORT);
	        udp_header_p->dst_port = htons(SYS_DFLT_DHCP_CLIENT_PORT);

	        /* calculate the udp checksum */
            if (udp_header_p->chksum != 0)
            {
                udp_header_p->chksum = 0;

                calculate_length = DHCP_UDP_PSEUDO_HEADER_LENGTH + udp_header_len + dhcp_packet_len;
                CALCULATE_LENGTH_NO_TRUNCATE(calculate_length);

                udp_header_p->chksum = DHCP_ALGO_UdpChkSumCalculate(ip_header_p, calculate_length);
            }

            /* calculate the ip checksum */
            ip_header_p->csum = 0;
		    ip_header_p->csum = ipchksum((unsigned short *)ip_header_p, ip_header_len);

		    /* forward according to the dhcp flag */
		    if ( !(dhcp_packet_p->flags & htons(BOOTP_BROADCAST)) )
            {
                DHCP_ALGO_PacketForward(DHCP_TYPE_RELAY_LAYER2_UNICAST, mem_ref_p, packet_length, destinate_mac, source_mac, client_vid, src_lport_ifIndex, client_port);
            }
            else
            {
                DHCP_ALGO_PacketForward(DHCP_TYPE_RELAY_BROADCAST, mem_ref_p, packet_length, destinate_mac, source_mac, client_vid, src_lport_ifIndex, 0);
            }

        }
    }

}   /* End of DHCP_ALGO_Handle_Option82() */

/* FUNCTION	NAME : DHCP_ALGO_RelayExceptionCheck
 * PURPOSE:
 *      Exception cases checking for DHCP Option82 L2 Relay
 *
 * INPUT:
 *		vid                 --  vlan the packet coming from
 *      dhcp_packet_p       --  pointer point to dhcp packet
 *      mgmt_vid            --  management vlan id
 *      mgmt_intf_ip        --  management vlan interface IP
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE    - success, no exception case happened
 *      FALSE   - failure, exception case happened
 *
 * NOTES:
 *		None.
 *
 */
static BOOL_T DHCP_ALGO_RelayExceptionCheck(UI32_T vid, struct dhcp_packet *dhcp_packet_p,UI32_T relay_server_ip1)
{

    /* check if input pointer is NULL */
    if(NULL == dhcp_packet_p)
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return FALSE;
    }

    if (relay_server_ip1 == 0)
    {
        return FALSE;
    }

    if (dhcp_packet_p->op == BOOTREQUEST)
    {
        if ((dhcp_packet_p->giaddr != 0) || (dhcp_packet_p->hops >= 16))
        {
            return FALSE;
        }
    }

    if (dhcp_packet_p->op == BOOTREPLY)
    {
        if (dhcp_packet_p->giaddr == 0)
        {
            return FALSE;
        }
    }

    return TRUE;
}   /* End of DHCP_ALGO_RelayExceptionCheck() */


/* FUNCTION	NAME : DHCP_ALGO_RequestPacketProcess
 * PURPOSE:
 *      process DHCP Request packet for Option82 L2 Relay
 *
 * INPUT:
 *      dhcp_packet_p       --  pointer point to dhcp packet
 *      dhcp_packet_len     --  length of dhcp packet received
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE    - success
 *      FALSE   - failure
 *
 * NOTES:
 *		None.
 *
 */
static BOOL_T DHCP_ALGO_RequestPacketProcess(UI8_T *buffer_request_p, UI32_T *buffer_dhcp_len_p, struct dhcp_packet *dhcp_packet_p, UI32_T dhcp_packet_len, UI32_T  vid, UI32_T src_lport_ifindex, UI32_T mgmt_intf_ip)
{
    UI32_T                  option_len, i = 0;
    UI32_T                  policy;
    struct dhcp_packet	    temp_dhcppkt;
    BOOL_T                  option82_exist = FALSE;
    UI32_T                  dhcp_rid_mode = 0;
    UI32_T                  rid_value_len = 0;
    UI32_T                  option82_value_len = 0;
    UI32_T                  ori_option82_value_len = 0;
    UI32_T                  cid_length = 0;
    UI32_T                  rid_length = 0;
    BOOL_T                  last_option = TRUE;
    UI32_T                  real_dhcp_packet_len=0;
    BOOL_T                  subtype_format=FALSE;

    if(DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
    {
        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
    }

    /* check if input pointer is NULL */

    if((NULL == buffer_request_p)||
       (NULL == buffer_dhcp_len_p)||
       (NULL == dhcp_packet_p))
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return FALSE;
    }

    option_len = dhcp_packet_len - DHCP_FRAME_FIX_LEN;
    /* dhcp packet length may have paddnig in the last, so option length may not be correct */
    if(FALSE == DHCP_ALGO_CalculateOptionLen(dhcp_packet_p, &option_len))
    {
        DHCP_BD(RELAY, "Failed to re-calculate option length");
        return FALSE;
    }
    real_dhcp_packet_len = option_len + DHCP_FRAME_FIX_LEN;

    option82_exist = DHCP_ALGO_Option82ExistentCheck(dhcp_packet_p, option_len, &i, &last_option);

    if(DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82RidMode(&dhcp_rid_mode))
    {
        DHCP_BD(RELAY, "Failed to get dhcp relay option 82 remote id mode");
        return FALSE;
    }

    /* get remote id length */
    if(FALSE == DHCP_ALGO_GetRidValueLen(dhcp_rid_mode, mgmt_intf_ip, &rid_value_len))
    {
        DHCP_BD(RELAY, "Failed to get remote id length");
        return FALSE;
    }

    /* get optoin82 subtype format */
    if (DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Format(&subtype_format))
    {
        DHCP_BD(RELAY, "Failed to get subtype foramt");
        return FALSE;
    }

    /* calculate RID value length */
    if (subtype_format)
    {
        cid_length = DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT + 2;
        rid_length = rid_value_len + 4;
    }
    else
    {
        cid_length = DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT;
        rid_length = rid_value_len + 2;
    }

    if (option82_exist == FALSE)
    {
        /* the option82 is nonexistent, nee to append */


        memset(&temp_dhcppkt, 0,sizeof(struct dhcp_packet) );
        memcpy(&temp_dhcppkt, dhcp_packet_p, real_dhcp_packet_len);

        option82_value_len = cid_length + rid_length;;
	    /* so far i at the END of the DHCP options */
        temp_dhcppkt.options[i] = DHO_DHCP_OPTION_82;
        temp_dhcppkt.options[i+1] = option82_value_len;

        if(FALSE == DHCP_ALGO_FillInOption82CircuitId(&temp_dhcppkt, i+2, vid, src_lport_ifindex,subtype_format))
            return FALSE;

        DHCP_ALGO_FillInOption82RemoteId(&temp_dhcppkt,i+2+cid_length,dhcp_rid_mode,mgmt_intf_ip,subtype_format);

        temp_dhcppkt.options[i+option82_value_len+2] = DHO_END;  /* End of Option */
        temp_dhcppkt.giaddr = mgmt_intf_ip;
        temp_dhcppkt.hops++;

        *buffer_dhcp_len_p = real_dhcp_packet_len + option82_value_len + 2;

        /*	prepare DHCP packet	*/

        memcpy (buffer_request_p, (char*)&temp_dhcppkt, *buffer_dhcp_len_p);

        return TRUE;
    }
    else
    {
        /* the option82 is existent */
        if(DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Policy(&policy))
        {
            DHCP_BD(RELAY, "Failed to get dhcp relay option 82 policy");
            return FALSE;
        }
        switch (policy)
        {

            case VAL_dhcp_Option82_Policy_drop:
                /* FALSE for flooding */
                return FALSE;
				break;

            case VAL_dhcp_Option82_Policy_keep:
                /* do nothing, TRUE for unicast */
                *buffer_dhcp_len_p = real_dhcp_packet_len;
                dhcp_packet_p->giaddr = mgmt_intf_ip;
                dhcp_packet_p->hops++;
                memcpy (buffer_request_p, (char*)dhcp_packet_p, *buffer_dhcp_len_p);
                return TRUE;
				break;

            case VAL_dhcp_Option82_Policy_replace:
            {
                UI32_T option82_index=0;
                struct dhcp_packet * buffer_request_dhcp_p = NULL;


                ori_option82_value_len = dhcp_packet_p->options[i+1];

                /* copy the option before option 82 to buffer */
                memcpy(buffer_request_p, dhcp_packet_p, sizeof(UI8_T)*(DHCP_FRAME_FIX_LEN+i));
                buffer_request_dhcp_p = (struct dhcp_packet *)buffer_request_p;
                if(!last_option)
                {
                    memcpy(&(buffer_request_dhcp_p->options[i]),&(dhcp_packet_p->options[i+ori_option82_value_len+2]),sizeof(UI8_T)*(option_len-(i+ori_option82_value_len+2)-1));
                    option82_index = option_len - (ori_option82_value_len + 2) -1;  /* remove the last option DHO_END */
                }
                else
                    option82_index = i;

                if (dhcp_packet_p->options[i+1] != (dhcp_packet_p->options[i+3]+2)) /* Both CID and RID */
                {
                        DHCP_BD(RELAY, "Policy replace: Both CID and RID");

                    if(FALSE == DHCP_ALGO_FillInOption82CircuitId(buffer_request_dhcp_p, option82_index+2, vid, src_lport_ifindex,subtype_format))
                        return FALSE;

                    DHCP_ALGO_FillInOption82RemoteId(buffer_request_dhcp_p, option82_index+cid_length+2,dhcp_rid_mode,mgmt_intf_ip,subtype_format);
                    option82_value_len = cid_length + rid_length;

                }
                else   /* option 82 only has one sub-option */
                {

                    if(dhcp_packet_p->options[i+2] == DHCP_CIRCUIT_ID_SUBOPTION)    /* Only CID*/
                    {
                        DHCP_BD(RELAY, "Policy replace: only CID");

                        if(FALSE == DHCP_ALGO_FillInOption82CircuitId(buffer_request_dhcp_p, option82_index+2, vid, src_lport_ifindex,subtype_format))
                            return FALSE;

                        option82_value_len = cid_length;

                    }
                    else if(dhcp_packet_p->options[i+2] == DHCP_REMOTE_ID_SUBOPTION) /* Only RID*/
                    {
                        DHCP_BD(RELAY, "Policy replace: only RID");
                        DHCP_ALGO_FillInOption82RemoteId(buffer_request_dhcp_p, option82_index+2,dhcp_rid_mode, mgmt_intf_ip,subtype_format);
                        option82_value_len = rid_length;

                    }
                    else
                    {
                        /* incorrect format ! */
                        DHCP_BD(RELAY, "Policy_replace but incorrect format");
                        return FALSE;
                    }
                }

                /* replace the option82 length */
                buffer_request_dhcp_p->options[option82_index] = DHO_DHCP_OPTION_82;
                buffer_request_dhcp_p->options[option82_index+1] = option82_value_len;
                buffer_request_dhcp_p->options[option82_index+option82_value_len+2] = DHO_END;

                /* update dhcp packet length */
                real_dhcp_packet_len = real_dhcp_packet_len - ori_option82_value_len + option82_value_len;

                *buffer_dhcp_len_p = real_dhcp_packet_len;
                buffer_request_dhcp_p->giaddr = mgmt_intf_ip;
                buffer_request_dhcp_p->hops++;
                return TRUE;
            }
				break;

            default:
                /* FALSE for flooding */
                DHCP_BD(RELAY, "No matched Policy ");

                return FALSE;
				break;
        }

    }


}   /* end of DHCP_ALGO_RequestPacketProcess() */

/* FUNCTION	NAME : DHCP_ALGO_FillInOption82CircuitId
 * PURPOSE:
 *
 * INPUT:
 *      dhcp_packet_p       --  pointer point to dhcp packet
 *      index               --  start position of circuit id suboption
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE/FALSE.

 * NOTES:
 *		None.
 *
 */
static BOOL_T DHCP_ALGO_FillInOption82CircuitId(struct dhcp_packet *dhcp_packet_p, UI32_T index, UI32_T vid, UI32_T src_lport_ifIndex, BOOL_T subtype)
{
    UI32_T              unit, port, trunkid;
    UI32_T              lport_type;

    /* check if input pointer is NULL */
    if(NULL == dhcp_packet_p)
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return FALSE;
    }

    lport_type = SWCTRL_POM_LogicalPortToUserPort(src_lport_ifIndex,&unit,&port,&trunkid);

    /* If ingress port is trunk port, we use the first active member as its user port */
    if(SWCTRL_LPORT_TRUNK_PORT == lport_type)
    {
        UI32_T active_lport[SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK]={0};
        UI32_T active_count=0;
        SWCTRL_POM_GetActiveTrunkMember(src_lport_ifIndex, active_lport, &active_count);
        if(active_count!=0)
        {   /* get first trunk member */
            SWCTRL_POM_LogicalPortToUserPort(active_lport[0],&unit,&port,&trunkid);
        }
        else
        {
            return FALSE;
        }
    }

    if(subtype)
    {
        dhcp_packet_p->options[index] = DHCP_CIRCUIT_ID_SUBOPTION;                    /* Circuit ID */
        dhcp_packet_p->options[index+1] = DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT;   /* CID Length */
        dhcp_packet_p->options[index+2] = DHCP_CID_TYPE_VLAN_UNIT_PORT;               /* CID type */
        dhcp_packet_p->options[index+3] = DHCP_CID_LENGTH_VLAN_UNIT_PORT;             /* length */
        dhcp_packet_p->options[index+4] = (vid & 0xFF00)>>8;
        dhcp_packet_p->options[index+5] = vid & 0x00FF;
        dhcp_packet_p->options[index+6] = unit;   /* Module ID */
        dhcp_packet_p->options[index+7] = port;   /* Port id */
    }
    else
    {
        /* fill in CID with T-L-V format */
        dhcp_packet_p->options[index] = DHCP_CIRCUIT_ID_SUBOPTION;                      /* Circuit ID */
        dhcp_packet_p->options[index+1] = DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT - 2; /* CID Length */
        dhcp_packet_p->options[index+2] = (vid & 0xFF00) >> 8;
        dhcp_packet_p->options[index+3] = vid & 0x00FF;
        dhcp_packet_p->options[index+4] = unit;   /* Module ID */
        dhcp_packet_p->options[index+5] = port;   /* Port id */
    }
    return TRUE;

}   /* end of DHCP_ALGO_FillInOption82CircuitId() */

/* FUNCTION	NAME : DHCP_ALGO_FillInOption82RemoteId
 * PURPOSE:
 *
 * INPUT:
 *      dhcp_packet_p       --  pointer point to dhcp packet
 *      index               --  start position of remote id suboption
 *      mode                --  insert mac / ip address in remote id
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.

 * NOTES:
 *		None.
 *
 */
static void DHCP_ALGO_FillInOption82RemoteId(struct dhcp_packet *dhcp_packet_p, UI32_T index, UI32_T mode, UI32_T ip_address, BOOL_T subtype)
{
    UI8_T               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];


    /* check if input pointer is NULL */
    if(NULL == dhcp_packet_p)
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return;
    }

    dhcp_packet_p->options[index] = DHCP_REMOTE_ID_SUBOPTION;        /* Remote ID */

    if(subtype)
    {
        switch(mode)
        {
            case DHCP_OPTION82_RID_MAC_HEX:
            {
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_HEX;       /* RID Length */
                dhcp_packet_p->options[index+2] = DHCP_RID_TYPE_MAC_MODE_HEX;                   /* RID type */
                dhcp_packet_p->options[index+3] = DHCP_RID_LENGTH_MAC_MODE_HEX;                 /* length */
                SWCTRL_POM_GetCpuMac(cpu_mac);
                memcpy(&(dhcp_packet_p->options[index+4]), cpu_mac, SYS_ADPT_MAC_ADDR_LEN);   /* MAC Address */

            }
            break;
            case DHCP_OPTION82_RID_MAC_ASCII:   /* encoded format: "XXXXXXXXXXXX"*/
            {
                UI8_T       cpu_mac_ascii[DHCP_RID_LENGTH_MAC_MODE_ASCII+1] = {0};
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_ASCII;
                dhcp_packet_p->options[index+2] = DHCP_RID_TYPE_MAC_MODE_ASCII;
                dhcp_packet_p->options[index+3] = DHCP_RID_LENGTH_MAC_MODE_ASCII;
                SWCTRL_POM_GetCpuMac(cpu_mac);
                snprintf((char *)cpu_mac_ascii, DHCP_RID_LENGTH_MAC_MODE_ASCII+1,"%02X%02X%02X%02X%02X%02X",
                    cpu_mac[0],cpu_mac[1],cpu_mac[2],cpu_mac[3],cpu_mac[4],cpu_mac[5]);
                memcpy(&(dhcp_packet_p->options[index+4]), cpu_mac_ascii, DHCP_RID_LENGTH_MAC_MODE_ASCII);
            }
            break;
            case DHCP_OPTION82_RID_IP_HEX:
            {
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_IP_MODE_HEX;
                dhcp_packet_p->options[index+2] = DHCP_RID_TYPE_IP_MODE_HEX;
                dhcp_packet_p->options[index+3] = DHCP_RID_LENGTH_IP_MODE_HEX;
                memcpy(&(dhcp_packet_p->options[index+4]), &ip_address, DHCP_RID_LENGTH_IP_MODE_HEX);
            }
            break;
            case DHCP_OPTION82_RID_IP_ASCII:    /* encoded format: xxx.xxx.xxx.xxx */
            {
                UI8_T      ip_address_ascii[16] = {0};   /*xxx.xxx.xxx.xxx  length = 15 */
                UI32_T      addr_len = 0;

                snprintf((char *)ip_address_ascii, 16,"%u.%u.%u.%u",L_INET_EXPAND_IP(ip_address));
                addr_len = strlen((char *)ip_address_ascii);
                dhcp_packet_p->options[index+1] = addr_len + 2;
                dhcp_packet_p->options[index+2] = DHCP_RID_TYPE_IP_MODE_ASCII;
                dhcp_packet_p->options[index+3] = addr_len;
                memcpy(&(dhcp_packet_p->options[index+4]), ip_address_ascii, addr_len);
            }
            break;
            case DHCP_OPTION82_RID_CONFIGURED_STRING:
            {
                UI8_T      rid_value[SYS_ADPT_MAX_LENGTH_OF_RID+1] = {0};
                UI32_T     string_len = 0;

                if(DHCP_OM_OK == DHCP_OM_GetDhcpRelayOption82RidValue(rid_value))
                {
                    string_len = strlen((char *)rid_value);
                    dhcp_packet_p->options[index+1] = string_len + 2;
                    dhcp_packet_p->options[index+2] = DHCP_RID_TYPE_CONFIGURED_STRING;
                    dhcp_packet_p->options[index+3] = string_len;
                    memcpy(&(dhcp_packet_p->options[index+4]), rid_value, string_len);
                }

            }
            break;

        }
    }
    else
    {
        /* fill in RID with T-L-V format */
        switch (mode)
        {
            case DHCP_OPTION82_RID_MAC_HEX:
            {
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_HEX - 2;   /* RID Length */
                SWCTRL_POM_GetCpuMac(cpu_mac);
                memcpy(&(dhcp_packet_p->options[index+2]), cpu_mac, SYS_ADPT_MAC_ADDR_LEN);   /* MAC Address */
            }
            break;
            case DHCP_OPTION82_RID_MAC_ASCII:   /* encoded format: "XXXXXXXXXXXX"*/
            {
                UI8_T       cpu_mac_ascii[DHCP_RID_LENGTH_MAC_MODE_ASCII+1] = {0};
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_MAC_MODE_ASCII - 2;
                SWCTRL_POM_GetCpuMac(cpu_mac);
                sprintf((char *)cpu_mac_ascii, "%02X%02X%02X%02X%02X%02X",
                        cpu_mac[0], cpu_mac[1], cpu_mac[2], cpu_mac[3], cpu_mac[4], cpu_mac[5]);
                memcpy(&(dhcp_packet_p->options[index+2]), cpu_mac_ascii, DHCP_RID_LENGTH_MAC_MODE_ASCII);
            }
            break;
            case DHCP_OPTION82_RID_IP_HEX:
            {
                dhcp_packet_p->options[index+1] = DHCP_RID_SUBOPTION_LENGTH_IP_MODE_HEX - 2;
                memcpy(&(dhcp_packet_p->options[index+2]), &ip_address, DHCP_RID_LENGTH_IP_MODE_HEX);
            }
            break;
            case DHCP_OPTION82_RID_IP_ASCII:    /* encoded format: xxx.xxx.xxx.xxx */
            {
                UI8_T      ip_address_ascii[16] = {0};   /*xxx.xxx.xxx.xxx  length = 15 */
                UI32_T      addr_len = 0;
                sprintf((char *)ip_address_ascii, "%u.%u.%u.%u", L_INET_EXPAND_IP(ip_address));
                addr_len = strlen((char *)ip_address_ascii);
                dhcp_packet_p->options[index+1] = addr_len;
                memcpy(&(dhcp_packet_p->options[index+2]), ip_address_ascii, addr_len);
            }
            break;
            case DHCP_OPTION82_RID_CONFIGURED_STRING:
            {
                UI8_T      rid_value[SYS_ADPT_MAX_LENGTH_OF_RID+1] = {0};
                UI32_T     string_len = 0;

                if (DHCP_OM_OK == DHCP_OM_GetDhcpRelayOption82RidValue(rid_value))
                {
                    string_len = strlen((char *)rid_value);
                    dhcp_packet_p->options[index+1] = string_len;
                    memcpy(&(dhcp_packet_p->options[index+2]), rid_value, string_len);
                }

            }
            break;

        }
    }

    return;

}   /* end of DHCP_ALGO_FillInOption82RemoteId() */

/* FUNCTION	NAME : DHCP_ALGO_ReplyPacketProcess
 * PURPOSE:
 *      process DHCP Reply packet for Option82 L2 Relay
 *
 * INPUT:
 *      dhcp_packet_p       --  pointer point to dhcp packet
 *      dhcp_packet_len     --  length of dhcp packet received
 *      client_vid          --  the vlan which dhcp client connected
 *      client_unit         --  the unit which dhcp client connected
 *      client_port         --  the port which dhcp client connected
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE    - success
 *      FALSE   - failure
 *
 * NOTES:
 *		None.
 *
 */
static BOOL_T DHCP_ALGO_ReplyPacketProcess(struct dhcp_packet *dhcp_packet_p, UI32_T dhcp_packet_len, UI32_T *client_vlan, UI32_T *client_unit, UI32_T *client_port, UI32_T mgmt_intf_ip)
{
    UI32_T              option_len, i = 0;
    BOOL_T              option82_exist = FALSE, subtype_format = TRUE;
    UI32_T              dhcp_rid_mode;
    UI8_T               cpu_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T               rid_value[SYS_ADPT_MAX_LENGTH_OF_RID+1]={0};
    UI32_T              rid_value_len=0;
    BOOL_T              last_option = TRUE;
    SWCTRL_Lport_Type_T lport_type;
    UI32_T              egress_lport=0;

    DHCP_BD(RELAY, "original dhcp packet len[%lu]", (unsigned long)dhcp_packet_len);

    /* check if input pointer is NULL */
    if((NULL == dhcp_packet_p)||
       (NULL == client_vlan)||
       (NULL == client_unit)||
       (NULL == client_port))
    {
        DHCP_BD(RELAY, "Invalid NULL input pointer");
        return FALSE;
    }

    option_len = dhcp_packet_len - DHCP_FRAME_FIX_LEN;
    /* dhcp packet length may have paddnig in the last, so option length may not be correct */
    if(FALSE == DHCP_ALGO_CalculateOptionLen(dhcp_packet_p, &option_len))
    {
        DHCP_BD(RELAY, "Failed to re-calculate option length");
        return FALSE;
    }

    option82_exist = DHCP_ALGO_Option82ExistentCheck(dhcp_packet_p, option_len, &i,&last_option);


    /* get subtype format */
    if (DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Format(&subtype_format))
        return FALSE;

    /* if DHCP relay information option 82 is enabled, get rid mode */

    if(DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82RidMode(&dhcp_rid_mode))
    {
        DHCP_BD(RELAY, "Failed to get dhcp relay option 82 remote id mode");
        return FALSE;
    }

    if(dhcp_rid_mode == DHCP_OPTION82_RID_MAC_HEX)
    {
        SWCTRL_POM_GetCpuMac(cpu_mac);
        rid_value_len = DHCP_RID_LENGTH_MAC_MODE_HEX;
        memcpy(rid_value, cpu_mac, rid_value_len);
    }
    else if(dhcp_rid_mode == DHCP_OPTION82_RID_MAC_ASCII)
    {
        SWCTRL_POM_GetCpuMac(cpu_mac);
        rid_value_len = DHCP_RID_LENGTH_MAC_MODE_ASCII;
        snprintf((char *)rid_value, SYS_ADPT_MAX_LENGTH_OF_RID+1,"%02X%02X%02X%02X%02X%02X",
                cpu_mac[0],cpu_mac[1],cpu_mac[2],cpu_mac[3],cpu_mac[4],cpu_mac[5]);
    }
    else if((dhcp_rid_mode == DHCP_OPTION82_RID_IP_HEX)||
            (dhcp_rid_mode == DHCP_OPTION82_RID_IP_ASCII))
    {

        if(dhcp_rid_mode == DHCP_OPTION82_RID_IP_HEX)
        {
            rid_value_len = DHCP_RID_LENGTH_IP_MODE_HEX;
            memcpy(rid_value, &mgmt_intf_ip, DHCP_RID_LENGTH_IP_MODE_HEX);
        }
        else if(dhcp_rid_mode == DHCP_OPTION82_RID_IP_ASCII)
        {
            snprintf((char *)rid_value, SYS_ADPT_MAX_LENGTH_OF_RID+1,"%u.%u.%u.%u",L_INET_EXPAND_IP(mgmt_intf_ip));
            rid_value_len = strlen((char *)rid_value);
        }
    }
    else if(dhcp_rid_mode == DHCP_OPTION82_RID_CONFIGURED_STRING)
    {
        if(DHCP_OM_OK == DHCP_OM_GetDhcpRelayOption82RidValue(rid_value))
        {
            rid_value_len = strlen((char *)rid_value);
        }
    }

    if (option82_exist == FALSE)
    {
        DHCP_BD(RELAY, "Option82 is not existed");
        return FALSE;
    }
    else
    {
         /* It has Both CID and RID  or CID only*/
        if(dhcp_packet_p->options[i+2] == DHCP_CIRCUIT_ID_SUBOPTION)
        {
            /* extract vlan-unit-port information based on local configuration */
            if (subtype_format)
            {
                /* check CID length first */
                if (dhcp_packet_p->options[i+3] != DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT)
                {
                    /* flood but don't remove option 82 */
                   return FALSE;
                }
                /* if it doesn't have the cid type matched vlan-unit-port, flood */
                if(dhcp_packet_p->options[i+4] == DHCP_CID_TYPE_VLAN_UNIT_PORT)
                {
                    *client_vlan = (dhcp_packet_p->options[i+6] << 8) + dhcp_packet_p->options[i+7];
                    *client_unit = dhcp_packet_p->options[i+8];
                    *client_port = dhcp_packet_p->options[i+9];
                }
                else
                {
                   /* flood but don't remove option 82 */
                    return FALSE;
                }

                if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
                {
                    DHCP_BD_MSG("%s[%d],CID information\r\n", __FUNCTION__, __LINE__);
                    DHCP_BD_MSG("Type:%u\r\n", dhcp_packet_p->options[i+2]);
                    DHCP_BD_MSG("Length:%u\r\n", dhcp_packet_p->options[i+3]);
                    DHCP_BD_MSG("Subtype:%u\r\n", dhcp_packet_p->options[i+4]);
                    DHCP_BD_MSG("Sublength:%u\r\n", dhcp_packet_p->options[i+5]);
                    DHCP_BD_MSG("Vlan:%lu, Unit:%lu, Port:%lu\r\n", (unsigned long)*client_vlan, (unsigned long)*client_unit, (unsigned long)*client_port);
                }
            }
            else
            {
                /* check CID length first */
                if (dhcp_packet_p->options[i+3] != (DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT - 2))
                return FALSE;

                *client_vlan = (dhcp_packet_p->options[i+4] << 8) + dhcp_packet_p->options[i+5];
                *client_unit = dhcp_packet_p->options[i+6];
                *client_port = dhcp_packet_p->options[i+7];
            }
        }
        else      /* it has RID only*/
        {
            /* flood but remove option 82*/
            *client_vlan = 0;
            *client_unit = 0;
            *client_port = 0;
        }

        /* Check if circuit-id is trunk port member,
         * If yes, we should sent it to trunk port
         */
        lport_type = SWCTRL_POM_UserPortToLogicalPort(*client_unit,*client_port,&egress_lport);
        if(SWCTRL_LPORT_UNKNOWN_PORT != lport_type)
        {
            *client_port = egress_lport;
        }

        if (dhcp_packet_p->options[i+1] != (dhcp_packet_p->options[i+3]+2)) /* Both CID and RID */
        {

            if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
            {
                UI8_T rid_index=0;
                UI8_T k=0;
                rid_index = i+4+DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT;
                DHCP_BD_MSG("%s[%d],RID information\r\n", __FUNCTION__, __LINE__);
                DHCP_BD_MSG("Type:%u\r\n", dhcp_packet_p->options[rid_index]);
                DHCP_BD_MSG("Length:%u\r\n", dhcp_packet_p->options[rid_index+1]);
                DHCP_BD_MSG("Subtype:%u\r\n", dhcp_packet_p->options[rid_index+2]);
                DHCP_BD_MSG("Sublength:%u\r\n", dhcp_packet_p->options[rid_index+3]);
                DHCP_BD_MSG("Content:");

                for(k=0;k<rid_value_len;k++)
                    DHCP_BD_MSG("%02X", dhcp_packet_p->options[rid_index+4+k]);

                DHCP_BD_MSG("\r\n");
            }

            UI8_T  compare_index = 0;
            /* compare RID value based on local configuration */
            if (subtype_format)
            {
                /* check RID length first */
                if (dhcp_packet_p->options[i+DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT+5] != (rid_value_len + 2))
                {
                    /* flood but don't remove option 82 */

                    return FALSE;
                }
                compare_index = i + DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT + 8;
            }
            else
            {
                /* check RID length first */
                if (dhcp_packet_p->options[i+(DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT-2)+5] != rid_value_len)
                {
                    /* flood but don't remove option 82 */

                    return FALSE;
                }
                compare_index = i + (DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT - 2) + 6;
            }
            /* compare rid value */
            if (memcmp(rid_value, &dhcp_packet_p->options[compare_index], rid_value_len))
            {
                /* flood but don't remove option 82 */
                DHCP_BD(RELAY, "Rid value isn't matched");
                    return FALSE;
            }
        }

        /* remove option 82 */
        if (dhcp_packet_p->options[i] == DHO_DHCP_OPTION_82)
        {
            if(last_option)
            {
                UI32_T option82_len=0;

                option82_len = dhcp_packet_p->options[i+1]+2;
                /* remove option82 and end option */
                memset(&(dhcp_packet_p->options[i]),0, sizeof(UI8_T)*(option82_len+1));
                dhcp_packet_p->options[i] = DHO_END;
            }
            else
            {
                UI32_T other_option_index=0;
                UI32_T other_option_len=0;
                UI8_T *tmp_buf = NULL;

                other_option_index = i + dhcp_packet_p->options[i+1] + 2;
                other_option_len = option_len - other_option_index;

                DHCP_BD(RELAY, "allocate buffer size[%lu]", (unsigned long)option_len);

                tmp_buf = (UI8_T *)L_MM_Malloc(option_len, L_MM_USER_ID2(SYS_MODULE_DHCP, DHCP_TYPE_TRACE_ID_DHCP_ALGO_REPLYPACKETPROCESS));
                if(NULL == tmp_buf)
                {
                    DHCP_BD(RELAY, "Failed to allocate option buffer");
                    return FALSE;
                }

                memset(tmp_buf, 0, sizeof(UI8_T)*option_len);
                /* copy the other options after option 82 */
                memcpy(tmp_buf, &(dhcp_packet_p->options[other_option_index]), sizeof(UI8_T)*other_option_len);
                memset(&(dhcp_packet_p->options[i]),0,sizeof(UI8_T)*other_option_len);

                /* copy back the other options */
                memcpy(&(dhcp_packet_p->options[i]),tmp_buf,sizeof(UI8_T)*other_option_len);
                L_MM_Free((void *)tmp_buf);

            }
        }
    }

    return TRUE;

}   /* end of DHCP_ALGO_ReplyPacketProcess() */

/* FUNCTION	NAME : DHCP_ALGO_Option82ExistentCheck
 * PURPOSE:
 *      check to find whether the Option82 is existent in DHCP packet
 *
 * INPUT:
 *      dhcp_packet_p  --  pointer point to dhcp packet
 *      option_len     --  length of this dhcp packet options
 *
 * OUTPUT:
 *      index          --  the option82 position (if found option82)
 *                         or the end position (if not found option82)
 *      last optoin         --  if option 82 is the last option in dhcp packet
 * RETURN:
 *		TRUE        -   the option82 is existent
 *      FALSE       -   the option82 is nonexistent
 *
 * NOTES:
 *		None.
 *
 */
static BOOL_T DHCP_ALGO_Option82ExistentCheck(struct dhcp_packet *dhcp_packet_p, UI32_T option_len, UI32_T *index,BOOL_T *last_option)
{
    UI32_T              i = DHCP_COOKIE_LEN;
    UI32_T              other_option_index=0;

    /* check if input pointer is NULL */
    if((NULL == dhcp_packet_p)||
       (NULL == index)||
       (NULL == last_option))
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return FALSE;
    }

    while (dhcp_packet_p->options[i] != DHO_END)
    {
        i = i + dhcp_packet_p->options[i+1] + 2;
        if (i < option_len)
        {
            if (dhcp_packet_p->options[i] == DHO_DHCP_OPTION_82)
            {
                *index = i;
                other_option_index = i+ dhcp_packet_p->options[i+1] + 2;
                if(dhcp_packet_p->options[other_option_index] == DHO_END)
                    *last_option = TRUE;
                else
                    *last_option = FALSE;
                return TRUE;
            }
        }
        else
        {
            *index = i;
            return FALSE;
        }
    }

    *index = i;
    return FALSE;

}   /* end of DHCP_ALGO_Option82ExistentCheck() */

/* FUNCTION	NAME : DHCP_ALGO_FindActiveRelayServer
 * PURPOSE:
 *      find mac of dhcp server or next hop
 *
 * INPUT:
 *      ip_header_p     --  pointer point to ip header
 *
 * OUTPUT:
 *		destinate_mac   --  mac of dhcp server or next hop
 *      rif             --  routing interface to dhcp server or next hop
 *
 * RETURN:
 *		TRUE            -   find mac of dhcp server or next hop successfully
 *      FALSE           -   find destination mac fail
 *
 * NOTES:
 *		None.
 *
 */
/* begin 2006-11, Joseph */
static BOOL_T DHCP_ALGO_FindActiveRelayServer(DHCP_TYPE_IpHeader_T *ip_header_p, UI8_T *destinate_mac, UI32_T *send_out_ifindex, UI32_T *active_relay_server, UI32_T relay_server_list[])
{
    UI32_T      retry = 0;
    UI32_T      i = 0;
    BOOL_T      rtn_value = FALSE;
    UI32_T      out_ifindex = 0;
    IPAL_NeighborEntry_T  neighbor;
    L_INET_AddrIp_T dst_addr, src_addr, next_hop_addr;

    /* check if input pointer is NULL */
    if((NULL == ip_header_p)||
       (NULL == destinate_mac)||
       (NULL == send_out_ifindex)||
       (NULL == active_relay_server)||
       (NULL == relay_server_list))
    {
        DHCP_BD(RELAY, "invalid NULL input pointer");
        return FALSE;
    }
    /* get nexthop mac*/
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER; i++)
    {
        if (relay_server_list[i] == 0)
        {
            continue;
        }

        memset(&dst_addr, 0, sizeof(L_INET_AddrIp_T));
        memset(&src_addr, 0, sizeof(L_INET_AddrIp_T));
        memset(&next_hop_addr, 0, sizeof(L_INET_AddrIp_T));
        dst_addr.type = L_INET_ADDR_TYPE_IPV4;
        dst_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        memcpy(dst_addr.addr, &relay_server_list[i],SYS_ADPT_IPV4_ADDR_LEN);
        /* It can find best routing interface */
        if(NETCFG_TYPE_OK ==  NETCFG_PMGR_ROUTE_GetBestRoutingInterface(&dst_addr,&src_addr,&next_hop_addr,&out_ifindex))
        {

            if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
            {
                DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
                DHCP_BD_MSG("src addr[%u.%u.%u.%u]\r\n", L_INET_EXPAND_IP(src_addr.addr));
                DHCP_BD_MSG("out interface[%lu]\r\n", (unsigned long)L_INET_EXPAND_IP(out_ifindex));
                DHCP_BD_MSG("next hop[%u.%u.%u.%u]\r\n", L_INET_EXPAND_IP(next_hop_addr.addr));
                DHCP_BD_MSG("addr type[%u]\r\n", next_hop_addr.type);
            }

            for(retry = 0;retry < 2;retry++)
            {
                memset(&neighbor, 0, sizeof(IPAL_NeighborEntry_T));
                /* get destination mac from dip */
                if(IPAL_RESULT_OK ==  IPAL_NEIGH_GetNeighbor(out_ifindex, &next_hop_addr, &neighbor))
                {
                    if (DHCP_BACKDOOR_GetDebugFlag(DHCP_BD_FLAG_RELAY))
                    {
                        DHCP_BD_MSG("%s[%d]\r\n", __FUNCTION__, __LINE__);
                        DHCP_BD_MSG("ifindex[%lu],state[%lu]\r\n", (unsigned long)neighbor.ifindex, (unsigned long)neighbor.state);
                        DHCP_BD_MSG("phy addr[%02x-%02x-%02x-%02x-%02x-%02x]\r\n", L_INET_EXPAND_MAC(neighbor.phy_address));
                        DHCP_BD_MSG("ip addr[%u.%u.%u.%u]\r\n", L_INET_EXPAND_IP(neighbor.ip_addr.addr));
                    }

                    memcpy(destinate_mac,neighbor.phy_address, SYS_ADPT_MAC_ADDR_LEN);
                    *send_out_ifindex = out_ifindex;
                    *active_relay_server = relay_server_list[i];
                    rtn_value = TRUE;
                    break;
                }
                else
                {
                    /* if can't find arp entry in ipal, send arp request */
                    if(IPAL_RESULT_FAIL == IPAL_NEIGH_SendArpRequest(out_ifindex, &next_hop_addr))
                    {
                        DHCP_BD(RELAY, "Failed to send arp request");
                    }
                }
            }



        }
        else
        {
            DHCP_BD(RELAY, "Can't find best routing interface");
            continue;
        }


    }

    return rtn_value;

}   /* end of DHCP_ALGO_FindActiveRelayServer() */


/* FUNCTION	NAME : DHCP_ALGO_PacketForward
 * PURPOSE:
 *
 *
 * INPUT:
 *      forward_flag    --  DHCP_TYPE_FLOODING
 *                          DHCP_TYPE_RELAY_BROADCAST
 *                          DHCP_TYPE_RELAY_UNICAST
 *                          DHCP_TYPE_RELAY_LAYER2_UNICAST
 *		mem_ref_p			--  holder of received packet buffer.
 *		packet_length	--	received packet length.
 *		destinate_mac	--	the destination hardware mac address.
 *		source_mac		--	the source hardware mac address.
 *		output_vid		--	the vlan to send out packet.
 *		exclude_lport   --  the exclude port ifIndex.
 *      output_lport    --  the output port ifIndex.
 *      rif             --  the routing interface to send out packet.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		successful (0), failed (-1)
 *      [according to the IML definition]
 *
 * NOTES:
 *
 */
static UI32_T DHCP_ALGO_PacketForward(UI16_T forward_flag, L_MM_Mref_Handle_T *mem_ref_p, UI32_T packet_length, UI8_T *destinate_mac,
                                      UI8_T *source_mac, UI32_T output_vid, UI32_T exclude_lport, UI32_T output_lport)
{
    UI32_T                  out_vid_ifindex;

    /* check if input pointer is NULL */
    if(NULL == mem_ref_p)
    {
        DHCP_BD(RELAY, "Invalid NULL input pointer");
        return (-1);
    }

    if((NULL == destinate_mac)||
       (NULL == source_mac))
    {
        L_MM_Mref_Release(&mem_ref_p);
        DHCP_BD(RELAY, "Invalid NULL input pointer");
        return (-1);
    }

    VLAN_VID_CONVERTTO_IFINDEX(output_vid, out_vid_ifindex);

    switch (forward_flag)
    {

        case DHCP_TYPE_FLOODING:
            /* flooding */
            DHCP_BD(RELAY, "flooding to vlan ifindex %lu except lport %lu", (unsigned long)out_vid_ifindex, (unsigned long)exclude_lport);
            return IML_PMGR_BroadcastBootPPkt_ExceptInport(mem_ref_p, out_vid_ifindex, packet_length, destinate_mac, source_mac, exclude_lport);
            break;

        case DHCP_TYPE_RELAY_UNICAST:
            /* unicast */
            DHCP_BD(RELAY, "unicast to vlan ifindex %lu ", (unsigned long)out_vid_ifindex);
            return IML_PMGR_SendPkt(mem_ref_p, out_vid_ifindex, packet_length, destinate_mac, source_mac, 0x0800, FALSE);
            break;

        case DHCP_TYPE_RELAY_BROADCAST:
            /* broadcast */
            DHCP_BD(RELAY, "broadcast to vlan ifindex %lu except lport %lu", (unsigned long)out_vid_ifindex, (unsigned long)exclude_lport);
	   		return IML_PMGR_BroadcastBootPPkt_ExceptInport(mem_ref_p, out_vid_ifindex, packet_length, destinate_mac, source_mac, exclude_lport);
            break;

        case DHCP_TYPE_RELAY_LAYER2_UNICAST:
            /* unicast on non-management vlan */
            DHCP_BD(RELAY, "L2 unicast to lport %lu on vlan %lu", (unsigned long)output_lport, (unsigned long)output_vid);
            return IML_PMGR_SendPktToPort(mem_ref_p, packet_length, destinate_mac, source_mac, output_vid, output_lport, 0x0800);
            break;

        default:
            L_MM_Mref_Release(&mem_ref_p);
            return (-1);
            break;
    }

}   /* end of DHCP_ALGO_PacketForward() */

/* FUNCTION : DHCP_ALGO_CalculateOptionLen
 * PURPOSE  : The function calculate the option length of the DHCP payload
 * INPUT    : dhcp_packet_p  --  dhcp packet
 *            option_len     --  original receive option length
 * OUTPUT   : option_len     --  option length of the DHCP payload
 * RETUEN   : TRUE/FALSE
 * NOTES    : Becasue incoming packet may have padding in the last,
 *            we can't use packet length to calculate the option length.
 *            we must find out where is the DHO_END option and use it to calculate the real length
 */
static BOOL_T DHCP_ALGO_CalculateOptionLen(struct dhcp_packet *dhcp_packet_p,UI32_T *option_len)
{
    UI32_T  tmp_index = DHCP_COOKIE_LEN;
    UI32_T  original_option_len;
    BOOL_T  end_found = TRUE;
    /* check if input pointer is NULL */
    if((NULL == dhcp_packet_p)||
       (NULL == option_len))
    {
        return FALSE;
    }

    original_option_len = *option_len;
    while (dhcp_packet_p->options[tmp_index] != DHO_END)
    {
        /* move to next option */
        tmp_index = tmp_index + dhcp_packet_p->options[tmp_index+1] + 2;
        if (tmp_index >= original_option_len)
        {
            end_found = FALSE;
            break;
        }
    }

    if(end_found)
    {
        *option_len = tmp_index + 1;
        return TRUE;
    }
    else
        return FALSE;

}

/* FUNCTION : DHCP_ALGO_CalculateTxBufferSize
 * PURPOSE  : The function calculate the Tx buffer size
 * INPUT    : dhcp_packet_p   --  original dhcp packet
 *            ip_header_len   --  original ip header length
 *            udp_header_len  --  original udp header length
 *            mgmt_intf_ip    --  management interface ip address
 *            dhcp_packet_len --  original dhcp packet length (including zero padding)
 * OUTPUT   : buffer_size_p   -- tx buffer size
 * RETUEN   : TRUE/FALSE
 * NOTES    :
 */
static BOOL_T DHCP_ALGO_CalculateTxBufferSize(
                        struct dhcp_packet *dhcp_packet_p,
                        UI32_T ip_header_len,
                        UI32_T udp_header_len,
                        UI32_T dhcp_packet_len,
                        UI32_T mgmt_intf_ip,
                        UI32_T *buffer_size_p)
{
    UI32_T option_len=0;
    UI32_T new_dhcp_packet_len=0;
    UI32_T option82_index=0;
    BOOL_T last_option=FALSE;
    BOOL_T option82_exist=FALSE;
    UI32_T ori_option82_total_len=0;
    UI32_T rid_mode=0;
    UI32_T rid_value_len=0;
    UI32_T cid_len=0, rid_len=0,option82_total_len=0;
    UI32_T cal_buffer_size=0;
    UI32_T policy=0;
    /* null pointer checking */
    if((NULL == dhcp_packet_p)||
       (NULL == buffer_size_p))
    {
        return FALSE;
    }

    /* calculate real dhcp packet length */
    option_len = dhcp_packet_len - DHCP_FRAME_FIX_LEN;
    if(FALSE == DHCP_ALGO_CalculateOptionLen(dhcp_packet_p, &option_len))
    {
        DHCP_BD(RELAY, "Failed to re-calculate option length");
        return FALSE;
    }

    new_dhcp_packet_len = option_len + DHCP_FRAME_FIX_LEN;

    /* check if option 82 exist and get its length */
    option82_exist = DHCP_ALGO_Option82ExistentCheck(dhcp_packet_p, option_len, &option82_index, &last_option);
    if(TRUE == option82_exist)
    {
        ori_option82_total_len = (UI32_T)(dhcp_packet_p->options[option82_index+1])+2;
    }

    DHCP_BD(RELAY, "original option82 total length[%lu]", (unsigned long)ori_option82_total_len);

    /* get configured option82 length */

    if(DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82RidMode(&rid_mode))
    {
        DHCP_BD(RELAY, "Failed to get dhcp relay option 82 remote id mode");
        return FALSE;
    }

    if(FALSE == DHCP_ALGO_GetRidValueLen(rid_mode, mgmt_intf_ip, &rid_value_len))
    {
        DHCP_BD(RELAY, "Failed to get remote id length");
        return FALSE;
    }

    cid_len = DHCP_CID_SUBOPTION_LENGTH_VLAN_UNIT_PORT + 2;
    rid_len = rid_value_len + 4;
    option82_total_len = cid_len + rid_len + 2;

    /* get the relay option82 policy */
    if(DHCP_OM_OK != DHCP_OM_GetDhcpRelayOption82Policy(&policy))
    {
        DHCP_BD(RELAY, "Failed to get dhcp relay option 82 policy");
       return FALSE;
    }

    cal_buffer_size = ip_header_len + udp_header_len;
    /* calculate the buffer size */
    if(TRUE == option82_exist)
    {
        if(VAL_dhcp_Option82_Policy_replace == policy)
        {
            cal_buffer_size = cal_buffer_size + new_dhcp_packet_len - ori_option82_total_len + option82_total_len;
        }
        else
        {
            cal_buffer_size = cal_buffer_size + new_dhcp_packet_len;
        }
    }
    else
    {
        cal_buffer_size = cal_buffer_size + new_dhcp_packet_len + option82_total_len;
    }

    if(cal_buffer_size < BOOTP_MIN_LEN)
    {
        cal_buffer_size = ip_header_len + udp_header_len + BOOTP_MIN_LEN;
    }

    *buffer_size_p = cal_buffer_size;

    return TRUE;

}

/* FUNCTION : DHCP_ALGO_GetRidValueLen
 * PURPOSE  : The function calculate the option length of the DHCP payload
 * INPUT    : rid_mode        -- configured remote id mode
 *            mgmt_intf_ip    -- management interface ip address
 * OUTPUT   : rid_value_len_p -- remote id value length
 * RETUEN   : TRUE/FALSE
 * NOTES    :
 */
static BOOL_T DHCP_ALGO_GetRidValueLen(UI32_T rid_mode,UI32_T mgmt_intf_ip,UI32_T *rid_value_len_p )
{
    UI8_T rid_value[SYS_ADPT_MAX_LENGTH_OF_RID + 1] = {0};

    if(NULL == rid_value_len_p)
        return FALSE;
    /* if rid mode is ip address, but DUT has no ip address configured, flood the packet*/
    if((rid_mode == DHCP_OPTION82_RID_IP_HEX)||
       (rid_mode == DHCP_OPTION82_RID_IP_ASCII))
    {

        if(rid_mode == DHCP_OPTION82_RID_IP_HEX)
            *rid_value_len_p = DHCP_RID_LENGTH_IP_MODE_HEX;


        if(rid_mode == DHCP_OPTION82_RID_IP_ASCII)    /* encode ip address to ascii code */
        {
            UI8_T      ip_address_ascii[16] = {0};

            snprintf((char *)ip_address_ascii, 16,"%u.%u.%u.%u",L_INET_EXPAND_IP(mgmt_intf_ip));
            *rid_value_len_p = strlen((char *)ip_address_ascii);
        }

    }
    else if(rid_mode == DHCP_OPTION82_RID_MAC_HEX)
    {
    	*rid_value_len_p = DHCP_RID_LENGTH_MAC_MODE_HEX;
    }
    else if(rid_mode == DHCP_OPTION82_RID_MAC_ASCII)
    {
        *rid_value_len_p = DHCP_RID_LENGTH_MAC_MODE_ASCII;
    }
    else if(rid_mode == DHCP_OPTION82_RID_CONFIGURED_STRING)
    {
        if(DHCP_OM_OK == DHCP_OM_GetDhcpRelayOption82RidValue(rid_value))
        {
            *rid_value_len_p = strlen((char *)rid_value);
        }
        else
            *rid_value_len_p = 0;
    }

    return TRUE;
}


/* FUNCTION : ipchksum
 * PURPOSE  : The function calculate the checksum of ip header
 * INPUT    : ip    -- pointer to ip header
 *            len   -- calculate length
 * OUTPUT   :
 * RETUEN   : checksum value
 * NOTES    :
 */
static unsigned short ipchksum(unsigned short *ip, int len)
{
	unsigned long sum = 0;

    /* check if input pointer is NULL */
    if(NULL == ip)
    {
        DHCP_BD(RELAY, "Invalid NULL input pointer");
        return 0;
    }

	len >>= 1;

    while (len--)
    {
		sum += *(ip++);
		if (sum > 0xFFFF)
			sum -= 0xFFFF;
	}
	return(((unsigned short)(~sum) & 0x0000FFFF));
}
#endif

/* FUNCTION : DHCP_ALGO_ChkSumCalculate
 * PURPOSE  : The function calculate dhcp checksum
 * INPUT    : *calculateDataBufferPtr     --  Data buffer which want to calculate checksum.
 *            calculateLengthInHalfWord   --  length of data buffer (in 2bytes length).
 * OUTPUT   : None.
 * RETUEN   : checksum result.
 * NOTES    : None.
 */
static UI16_T DHCP_ALGO_ChkSumCalculate(UI16_T *calculateDataBufferPtr, UI32_T calculateLengthInHalfWord)
{
    UI32_T  i, temp;
    UI16_T  checkSum;

    /* check if input pointer is NULL */
    if(NULL == calculateDataBufferPtr)
    {
        return 0;
    }

    checkSum = *calculateDataBufferPtr;
    calculateDataBufferPtr++;
    for(i = 1; i < calculateLengthInHalfWord; i++)
    {
        temp = checkSum + *calculateDataBufferPtr;
        if(temp > 0xffff)
        {   /* carry */
            checkSum = checkSum + *calculateDataBufferPtr + 1;
        }
        else
        {
            checkSum = (UI16_T)temp;
        }
        calculateDataBufferPtr++;
    }
    checkSum = checkSum^0xffff;
    return checkSum;
}

/* FUNCTION : DHCP_ALGO_UdpChkSumCalculate
 * PURPOSE  : The function calculate the checksum of the UDP header
 * INPUT    : *calculateDataBufferPtr     --  Data buffer which want to calculate checksum.
 *            calculateLengthInHalfWord   --  length of data buffer (in 2bytes length).
 * OUTPUT   : None.
 * RETUEN   : checksum result.
 * NOTES    : None.
 */
UI16_T DHCP_ALGO_UdpChkSumCalculate(DHCP_TYPE_IpHeader_T *ipPktStructure, UI32_T calculateLengthInHalfWord)
{
    UDP_CHECKSUM_T             udp_checksum;
    DHCP_TYPE_UdpHeader_T    *udppktbufferptr  = (DHCP_TYPE_UdpHeader_T*)((UI8_T*)ipPktStructure + sizeof(DHCP_TYPE_IpHeader_T));
    struct dhcp_packet               *dhcppktptr = (struct dhcp_packet*)((UI8_T*)udppktbufferptr + sizeof(DHCP_TYPE_UdpHeader_T));

    /* check if input pointer is NULL */

    if(NULL == ipPktStructure)
    {
        return 0;
    }

    /*initial udp checksum to zero */
    memset(&udp_checksum, 0, sizeof(UDP_CHECKSUM_T));
    memcpy(udp_checksum.pseudo_header.src_ip, &ipPktStructure->sip, sizeof(udp_checksum.pseudo_header.src_ip));
    memcpy(udp_checksum.pseudo_header.dst_ip, &ipPktStructure->dip, sizeof(udp_checksum.pseudo_header.dst_ip));

    DHCP_BD(RELAY, "pseudo header src ip[%u.%u.%u.%u]", L_INET_EXPAND_IP(udp_checksum.pseudo_header.src_ip));
    DHCP_BD(RELAY, "pseudo header dst ip[%u.%u.%u.%u]", L_INET_EXPAND_IP(udp_checksum.pseudo_header.dst_ip));

    udp_checksum.pseudo_header.reserved = 0;
    udp_checksum.pseudo_header.protocol = 17;
    udp_checksum.pseudo_header.udp_length = udppktbufferptr->udp_header_len;

    DHCP_BD(RELAY, "psudo header udp length[%u]", udp_checksum.pseudo_header.udp_length);

    udp_checksum.udp_header.src_port = udppktbufferptr->src_port;

    DHCP_BD(RELAY, "udp header src port[%u]", udp_checksum.udp_header.src_port);

    udp_checksum.udp_header.dst_port = udppktbufferptr->dst_port;

    DHCP_BD(RELAY, "udp header dst port[%u]", udp_checksum.udp_header.dst_port);

    udp_checksum.udp_header.udp_header_len = udppktbufferptr->udp_header_len;

    DHCP_BD(RELAY, "udp header len[%u]", udp_checksum.udp_header.udp_header_len);

    udp_checksum.udp_header.chksum = 0;

    memcpy(&udp_checksum.dhcp_packet, dhcppktptr, ntohs(udppktbufferptr->udp_header_len)-sizeof(DHCP_TYPE_UdpHeader_T));

    return DHCP_ALGO_ChkSumCalculate((UI16_T *)&udp_checksum, calculateLengthInHalfWord);
}


