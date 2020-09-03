/* Module Name:	_DHCP_TXRX.H
 * Purpose:
 *		This module provides all transmiting and receiving packet related functions,
 *		used in DHCP/IAD. Includes : if_register_send, if_register_receive,
 *		if_reinitialize_send, if_reinitialize_receive, send_packet, receive_packet.
 *		We keep function name same as DHCP/IAD used for reference.
 *
 * Notes:
 *		1.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.25	--	William, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */

#ifndef		_DHCP_TXRX_H
#define		_DHCP_TXRX_H


/* INCLUDE FILE	DECLARATIONS
 */
#include <netinet/in.h>
#include "sys_type.h"
#include "dhcp_type.h"
/*#include "skt_vx.h"*//*Timon*/
/*#include "socket.h"*//*Timon*/


/* NAME	CONSTANT DECLARATIONS
 */
#define ECONNREFUSED -1
#define ECONNRESET   -2
#define ECONNABORTED -3
#define ETIMEDOUT    -4
#define EMSGSIZE     -5
#define EHOSTUNREACH -6
#define ENETUNREACH  -7
#define ENETDOWN     -8
#define ENOBUFS      -9
#define EHOSTDOWN    -10
#define EINVAL       -11
#define EWOULDBLOCK  -12
#define EADDRINUSE   -13
#define ENOPROTOOPT  -14
#define EISCONN      -15
#define EAFNOSUPPORT -16
#define EADDRNOTAVAIL -17
#define EPROTONOSUPPORT -18
#define EPROTOTYPE      -19
#define EOPNOTSUPP    -20
#define ENOTCONN      -21
#define EALREADY      -22
#define EPIPE         -23
#define EDESTADDRREQ  -24
#define EINPROGRESS   -25


struct sockaddr_in2 {
	short int        sin_family;
	unsigned short	 sin_port;
	struct in_addr   sin_addr;
	unsigned int     sin_cid;
	char	         sin_zero[4];
};

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */
/* EXPORTED	SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION	NAME : DHCP_TXRX_Init
 * PURPOSE:
 *		Allocate resource used in DHCP_TXRX.
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
 *		1. All several blocks for sending packet out.
 */
void DHCP_TXRX_Init(void);


/* FUNCTION	NAME : if_reinitialize_send
 * PURPOSE:
 *		close previous register and re-registered.
 *
 * INPUT:
 *		info	-- interface list to be reinit.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. None.
 */
void if_reinitialize_send (struct interface_info *info);


/* FUNCTION	NAME : if_reinitialize_receive
 * PURPOSE:
 *		close previous register and re-registered.
 *
 * INPUT:
 *		info	-- interface list to be reinit.
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
void if_reinitialize_receive (struct interface_info *info);


/* FUNCTION	NAME : if_register_send
 * PURPOSE:
 *		Register interface-list to DHCP_OM for sending packet.
 *
 * INPUT:
 *		info	-- interface list to be registered.
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
void if_register_send (struct interface_info *info);


/* FUNCTION	NAME : if_register_receive
 * PURPOSE:
 *		Register interface-list to DHCP_OM for receiving packet.
 *
 * INPUT:
 *		info	-- interface list to be registered.
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
void if_register_receive (struct interface_info *info);


/* FUNCTION	NAME : send_packet
 * PURPOSE:
 *		Send out one packet from specified interface.
 *
 * INPUT:
 *		interface	--  interface send to.
 *		packet		--	keep dhcp packet and related options.
 *		raw			--	raw packet contains dhcp packet
 *		len			--	packet length
 *		from		--
 *		to			--	dest. address
 *		hto			--	destination address length.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		<0	-- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *		others -- Real transmitted packet size.
 *
 * NOTES:
 *		Error code please ref. socket define. (sendto)
 */
ssize_t send_packet (struct interface_info *interface,
					 struct packet *packet,
					 struct dhcp_packet *raw,
					 UI32_T len,
					 UI32_T from,
					 struct sockaddr_in *to,
					 struct hardware *hto);

/* FUNCTION	NAME : DHCP_TXRX_SendPktThruIML
 * PURPOSE:
 *		Send out one packet from specified interface thru IML.
 *
 * INPUT:
 *		interface	--  interface send to.
 *		packet		--	keep dhcp packet and related options.
 *		raw			--	raw packet contains dhcp packet
 *		len			--	packet length
 *		from		--	src ip-address.
 *		to			--	dest. ip address in structure.
 *		hto			--	destination address length.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		<0	-- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *			ENOBUFS -- no buffer
 *		others -- Real transmitted packet size.
 *
 * NOTES:
 *		1. Error code please ref. socket define. (sendto)
 *		2. Currently, in switch mode, use managed vlan only. All convertion will not used
 *		   when we change to router mode.
 */
ssize_t DHCP_TXRX_SendPktThruIML(struct interface_info *interface,
					 struct packet *packet,
					 struct dhcp_packet *raw,
					 UI32_T len,
					 struct in_addr from,
					 struct sockaddr_in *to,
					 struct hardware *hto);

/* FUNCTION	NAME : DHCP_TXRX_SendPktThruIMLToClientPort
 * PURPOSE:
 *		Send out one packet from specified interface thru IML to Client port.
 *
 * INPUT:
 *		interface	--  interface send to.
 *		packet		--	keep dhcp packet and related options.
 *		raw			--	raw packet contains dhcp packet
 *		len			--	packet length
 *		from		--	src ip-address.
 *		to			--	dest. ip address in structure.
 *		hto			--	destination address length.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		<0	-- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *			ENOBUFS -- no buffer
 *		others -- Real transmitted packet size.
 *
 * NOTES:
 *		1. Error code please ref. socket define. (sendto)
 *		2. Currently, in switch mode, use managed vlan only. All convertion will not used
 *		   when we change to router mode.
 */
ssize_t DHCP_TXRX_SendPktThruIMLToClientPort(struct interface_info *interface,
					 struct packet *packet,
					 struct dhcp_packet *raw,
					 UI32_T len,
					 struct in_addr from,
					 struct sockaddr_in *to,
					 struct hardware *hto);


/* FUNCTION	NAME : receive_packet
 * PURPOSE:
 *		Retrieve one packet from specified interface.
 *
 * INPUT:
 *		sockid		--	the socket we will receive from.
 *		buf			--	buffer keeping received packet.
 *		len			--	buffer size.
 *		from		--	pointer point to source address
 *		hfrom		--	source hardware address of this packet.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		0		-- end of file, remote is closed.
 *		< 0		-- error occurs. (EWOULDBLOCK, ENOTCONN)
 *		others	-- length of received data on success
 *
 *
 * NOTES:
 *		Error code please ref. socket define. (recvfrom)
 */
ssize_t receive_packet (int sockid,
						unsigned char *buf,
						UI32_T len,
						struct sockaddr_in2 *from,
						struct hardware *hfrom);


/* FUNCTION	NAME : DHCP_TXRX_SaveIpMac
 * PURPOSE:
 *		Save (ip, mac) pair in local table for future reference.
 *
 * INPUT:
 *		ip		--	the ip associated with mac. (key)
 *		rif		--	the Routing interface
 *		mac		--  the source mac which bring this ip packet from.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		TRUE	--	OK.
 *		FLASE	--	Table full.
 *
 *
 * NOTES:
 *		Error code please ref. socket define. (recvfrom)
 */
BOOL_T DHCP_TXRX_SaveIpMac(UI32_T ip, int rif, UI8_T *mac);


#endif	 /*	_DHCP_TXRX_H */
