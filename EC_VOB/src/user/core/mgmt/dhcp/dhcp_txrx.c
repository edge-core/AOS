/* Module Name: DHCP_TXRX.C
 * Purpose:
 *      .
 *
 * Notes:
 *      .
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef _DEBUG
#include <string.h>
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
/*#include "skt_vx.h"*//*Timon*/
/*#include "socket.h"*//*Timon*/
#include "sys_type.h"
#include "l_mm.h"
#include "l_stdlib.h"
#include "sysfun.h"
#include "dhcp_error_print.h"
#include "dhcp_mgr.h"
#include "dhcp_om.h"                /* ryan add */
#include "dhcp_time.h"              /* ryan add */
#include "dhcp_txrx.h"
#include "dhcp_txrx_type.h"
#include "iml_pmgr.h"
/*#include "iproute.h"*//*Timon*/

#include "netcfg_type.h"
#include "vlan_lib.h"
#include "vlan_pmgr.h"
#include "vlan_pom.h"

#if(SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#include "vlan_lib.h"
#endif

#include "netcfg_pom_ip.h"
#include "dhcp_algo.h"
#include "ip_lib.h"

/* NAMING CONSTANT DECLARATIONS
 */

//#define DHCP_TXRX_MAX_PACKET_BLOCK_NBR  8
//#define DHCP_TXRX_FRAME_HEADER_LEN      22
//#define DHCP_TXRX_IP_HEADER_LEN         20
//#define DHCP_TXRX_UDP_HEADER_LEN        8
//#define DHCP_TXRX_DHCP_PACKET_LEN       576
//#define DHCP_TXRX_FRAME_SIZE            (DHCP_TXRX_FRAME_HEADER_LEN+DHCP_TXRX_IP_HEADER_LEN+DHCP_TXRX_UDP_HEADER_LEN+DHCP_TXRX_DHCP_PACKET_LEN)

#define IP_UDP                          17
#define DHCP_TXRX_DEFAULT_CLIENT_PORT   68      /*  SYS_DFLT_DHCP_CLIENT_PORT   */
#define DHCP_TXRX_DEFAULT_SERVER_PORT   67

#define DHCP_TXRX_MAX_LEARN_IP          24

#define DHCP_TXRX_BROADCAST_IP         0xffffffff

/* MACRO FUNCTION DECLARATIONS
 */
#define CALCULATE_LENGTH_NO_TRUNCATE(C_LENGTH); {         \
        C_LENGTH = (C_LENGTH % 2) ? C_LENGTH / 2 + 1 : C_LENGTH / 2;             \
    }

/* DATA TYPE DECLARATIONS
 */
typedef struct  DHCP_TXRX_IpMac_S
{
    UI32_T  ip;             /*  key */
    int     ifindex;
    UI8_T   mac[6];
}   DHCP_TXRX_IpMac_T;
/*
typedef struct  DHCP_TXRX_Lcb_S
{
    L_MPOOL_HANDLE  tx_buf_pool;
    int             ip_mac_table_entry_used;
}   DHCP_TXRX_Lcb_T;
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static unsigned short ipchksum(unsigned short *ip, int len);
static BOOL_T DHCP_TXRX_GetMacByIp(UI32_T ip, int *ifindex, UI8_T *mac);
static int DHCP_TXRX_FindEntryByIp(UI32_T ip);
static BOOL_T DHCP_TXRX_FindDestination(
    UI32_T dst_ip,
    UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN]
); 


/* STATIC VARIABLE DECLARATIONS
 */

//static DHCP_TXRX_Lcb_T        dhcp_txrx_lcb;
static DHCP_TXRX_IpMac_T    ip_mac_table[DHCP_TXRX_MAX_LEARN_IP];
static UI32_T               ip_mac_table_entry_used;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : DHCP_TXRX_Init
 * PURPOSE:
 *      Allocate resource used in DHCP_TXRX.
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
 *      1. All several blocks for sending packet out.
 */
void DHCP_TXRX_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
//    memset((char*)&dhcp_txrx_lcb, 0, sizeof(DHCP_TXRX_Lcb_T));
    memset((char*)ip_mac_table, 0, sizeof(DHCP_TXRX_IpMac_T)*DHCP_TXRX_MAX_LEARN_IP);
//Timon    dhcp_txrx_lcb.tx_buf_pool = L_MPOOL_CreateBlockPool(DHCP_TXRX_FRAME_SIZE,DHCP_TXRX_MAX_PACKET_BLOCK_NBR);
    ip_mac_table_entry_used = 0;
}   /*  end of DHCP_TXRX_Init   */


/* FUNCTION NAME : if_reinitialize_send
 * PURPOSE:
 *      close previous register and re-registered.
 *
 * INPUT:
 *      info    -- interface list to be reinit.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. None.
 */
void if_reinitialize_send(struct interface_info *info)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
}   /*  end of if_reinitialize_send */


/* FUNCTION NAME : if_reinitialize_receive
 * PURPOSE:
 *      close previous register and re-registered.
 *
 * INPUT:
 *      info    -- interface list to be reinit.
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
void if_reinitialize_receive(struct interface_info *info)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
}   /*  end of if_reinitialize_receive  */

/* FUNCTION NAME : if_register_socket
 * PURPOSE:
 *      Register socket to the interface.
 *
 * INPUT:
 *      info    -- interface list to be registered.
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
int if_register_socket(struct interface_info *info)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    struct sockaddr_in name;
    int sock;
    int flag;

    /* BODY */

    /* Check if we already register socket for this interface.
    ** If yes, don't do it again.
    */

    if (info->rfdesc > 0)
        return (info->rfdesc);


    /* Set up the address we're going to bind to. */
    name.sin_family = AF_INET;
//  name.sin_port = local_port;
    name.sin_port = L_STDLIB_Hton16 (67);
    //name.sin_addr.s_addr = INADDR_ANY;
    name.sin_addr.s_addr = INADDR_ANY;
//  memset (name.sin_zero, 0, sizeof (name.sin_zero));
//("the sin_port => %d, %d, %d, %d\n", local_port, name.sin_port,L_STDLIB_Ntoh16 (local_port),L_STDLIB_Ntoh16 (name.sin_port));
    /* Make a socket... */

    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        error ("\nCan't create dhcp socket");

    /* Set the REUSEADDR option so that we don't fail to start if
       we're being restarted. */
//  flag = 1;
    flag = 2;
    if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR,
            (char *)&flag, sizeof flag) < 0)
        error ("\nCan't set SO_REUSEADDR option on dhcp socket");

    /* Set the BROADCAST option so that we can broadcast DHCP responses. */
    if (setsockopt (sock, SOL_SOCKET, SO_BROADCAST,
            (char *)&flag, sizeof flag) < 0)
        error ("\nCan't set SO_BROADCAST option on dhcp socket:");

    /* Bind the socket to this interface's IP address. */
    if (bind (sock, (struct sockaddr *)&name, sizeof name) < 0)
    {
        DHCP_ERROR_Print("\nCan't bind the socket to the interface(IP got from dhcp)");
        //exit(1);
        return 0;
    }
#if 0
#if defined (HAVE_SO_BINDTODEVICE)
    /* Bind this socket to this interface. */
//printf("HAVE_SO_BINDTODEVICE\n");
if (info->ifp) printf("Device=%s\n", (char *)(info->ifp));
    if (info -> ifp &&
        setsockopt (sock, SOL_SOCKET, SO_BINDTODEVICE,
            (char *)(info -> ifp), sizeof *(info -> ifp)) < 0) {
        error("setsockopt: SO_BINDTODEVICE: %m");
    }
#endif
#endif
    return sock;

}   /*  end of if_register_socket   */



/* FUNCTION NAME : if_register_send
 * PURPOSE:
 *      Register interface-list to DHCP_OM for sending packet.
 *
 * INPUT:
 *      info    -- interface list to be registered.
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
void if_register_send(struct interface_info *info)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */

    info -> wfdesc = info -> rfdesc;

}   /*  end of if_register_send */


/* FUNCTION NAME : if_register_receive
 * PURPOSE:
 *      Register interface-list to DHCP_OM for receiving packet.
 *
 * INPUT:
 *      info    -- interface list to be registered.
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
void if_register_receive(struct interface_info *info)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */

    /* If we're using the socket API for sending and receiving,
       we don't need to register this interface twice. */
    //info -> rfdesc = if_register_socket (info);
    info -> rfdesc = if_register_socket (info);

}   /*  end of if_register_receive  */


/* FUNCTION NAME : send_packet
 * PURPOSE:
 *      Send out one packet from specified interface.
 *
 * INPUT:
 *      interface   --  interface send to.
 *      packet      --  keep dhcp packet and related options.
 *      raw         --  raw packet contains dhcp packet
 *      len         --  packet length
 *      from        --  src ip-address.
 *      to          --  dest. ip address in structure.
 *      hto         --  destination address length.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      <0  -- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *          ENOBUFS -- no buffer
 *      others -- Real transmitted packet size.
 *
 * NOTES:
 *      1. Error code please ref. socket define. (sendto)
 *      2. Currently, in switch mode, use managed vlan only. All convertion will not used
 *         when we change to router mode.
 */
ssize_t send_packet(struct interface_info *interface,
                     struct packet *packet,
                     struct dhcp_packet *raw,
                     UI32_T len,
                     UI32_T from,
                     struct sockaddr_in *to,
                     struct hardware *hto)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    int result;
    int sockfd = 0;
	int on = 1;
	
	struct sockaddr_in sockaddress;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	    return -1;

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(void *)&on, sizeof(on)) < 0)
    {
        close(sockfd);	            		             
        return -1;
    }

	memset(&sockaddress, 0, sizeof(sockaddress));
	sockaddress.sin_family = AF_INET;
	sockaddress.sin_port = L_STDLIB_Hton16(67); 
	sockaddress.sin_addr.s_addr = L_STDLIB_Hton32(INADDR_ANY);
	
	if(bind(sockfd,(struct sockaddr *)&sockaddress,sizeof(sockaddress)) < 0)
	{
		close(sockfd);	            		
		return -1;             
	}
     
#ifdef IGNORE_HOSTUNREACH
    int retry = 0;
    do {
//printf("into send_packet loop %d %x %dn",
// interface->wfdesc,to->sin_addr.s_addr, sizeof *to);
#endif
        result = sendto (sockfd, (char *)raw, len, 0,
                 (struct sockaddr *)to, sizeof *to);
//printf("send end \n");
#ifdef IGNORE_HOSTUNREACH
    } while (to -> sin_addr.s_addr == L_STDLIB_Hton32 (INADDR_BROADCAST) &&
         result < 0 &&
         (errno == EHOSTUNREACH ||
          errno == ECONNREFUSED) &&
         retry++ < 10);
#endif
    if (result < 0) {
        /*if (errno == ENETUNREACH)
            warn ("send_packet: please consult README file %s",
                  "regarding broadcast address.");*/
        warn("\nDHCP:send_packet failed. ");
    }

	close(sockfd);
    return result;


} /* end of send_packet*/

/* FUNCTION NAME : DHCP_TXRX_SendPktThruIML
 * PURPOSE:
 *      Send out one packet from specified interface thru IML.
 *
 * INPUT:
 *      interface   --  interface send to.
 *      packet      --  keep dhcp packet and related options.
 *      raw         --  raw packet contains dhcp packet
 *      len         --  packet length
 *      from        --  src ip-address.
 *      to          --  dest. ip address in structure.
 *      hto         --  destination address length.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      <0  -- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *          ENOBUFS -- no buffer
 *      others -- Real transmitted packet size.
 *
 * NOTES:
 *      1. Error code please ref. socket define. (sendto)
 *      2. Currently, in switch mode, use managed vlan only. All convertion will not used
 *         when we change to router mode.
 */
ssize_t DHCP_TXRX_SendPktThruIML(struct interface_info *interface,
                     struct packet *packet,
                     struct dhcp_packet *raw,
                     UI32_T len,
                     struct in_addr from,
                     struct sockaddr_in *to,
                     struct hardware *hto)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    L_MM_Mref_Handle_T       *mref_handle_p;
    UI32_T                   packet_length;
    UI8_T                    cpu_mac[SYS_ADPT_MAC_ADDR_LEN]; 
    UI8_T                    dst_mac[SYS_ADPT_MAC_ADDR_LEN]; 
    UI8_T                    *buf;
    UI32_T                   pdu_len;
    DHCP_TYPE_IpHeader_T     *ip_hdr;
    DHCP_TYPE_UdpHeader_T    *udp_hdr;
    DHCP_TYPE_SYSTEM_STATE_T system_mode;
    UI32_T                   address_mode;
    UI32_T                   om_mode;
#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    UI32_T                   vid_Ifindex;
#endif
    UI32_T                   calculate_length =0;
    UI32_T                   udp_len = 0;
    const UI16_T  ether_type = 0x0800;      /* ipv4 ether type*/

    /* BODY */

    /* 2002/2/9 Penny: Check if dhcp OM == Dynamic address mode, but Netcfg OM == Static,
     * Don't bind DHCP lease to system
     */
    if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(interface->vid_ifIndex, &address_mode))
    {
        return 0;
    }

    if (!DHCP_OM_GetIfMode(interface->vid_ifIndex, &om_mode))
    {
        return 0;
    }

    /* We detect that system just change from dynamic to static address mode, but
     * user not key in "ip dhcp restart", drop the packet we wanna send
     */
    else if ( (address_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) &&
              (address_mode != om_mode) )
    {
        return 0;
    }
    /* 2002-3-23:
    ** Check if current default mgmt vlan has change or not
    ** If changed, return immediately, don't send any packet*/

#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    {
        UI32_T    vid_tmp;

        /* get default mgmt vlan */
        VLAN_POM_GetManagementVlan(&vid_tmp);
        VLAN_OM_ConvertToIfindex(vid_tmp, &vid_Ifindex);
        if (vid_Ifindex != interface->vid_ifIndex)
            return 0;
    }
#endif
    /*  For Layer 2 (switch mode), processing steps :
     *  if (interface == NULL)
     *      if_ptr = DHCP_OM_GetNextInterface(NULL);
     *  else
     *      if_ptr = interface;
     *  Get vid_ifIndex from interface.
     *      vid_ifIndex = if_ptr->vid_ifIndex;
     *  1. Find out :
     *      a. vlan_ifIndex -- management-vlan from NETCFG
     *      b. cpu-mac as src_mac from NETCFG
     *      c. dst_mac from hardware.
     *  2. Construct L_MM_Mref object for packet out.
     */
    /*
     *  Prepare Broadcast parameters
     *  1. source mac uses manage-vlan''s MAC
     *  2. dest. mac :
     *     (in DHCP/IAD code, hto always is NULL.)
     *     check the remote site ip (to).
     *     if it is 0xffffffff,
     *        dest. mac should be 0xffffffffffff.
     *     else
     *        find the ip associated with mac. from tmp. cache table.
     *
     *
     */
    /*  Penny 2002-1-30
     *  Check system mode here. If system mode != DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE
     *      , drop packet
     */
    system_mode = (DHCP_TYPE_SYSTEM_STATE_T)DHCP_MGR_CheckSystemState();
    if (system_mode != DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE)
        return 0;

#if (SYS_CPNT_ROUTING != TRUE && SYS_CPNT_MULTIPLE_MGMT_IP != TRUE)
    VLAN_PMGR_GetVlanMac(vid_Ifindex, cpu_mac);
#else
    memcpy(cpu_mac,interface->hw_address.haddr, sizeof(cpu_mac));
#endif

    /* if we can't find mac address of unicast destination mac address,
     * use broadcast DA(FF-FF-FF-FF-FF-FF),broadcast destination ip(255.255.255.255)
     */
    {
        const UI8_T bcast_mac[SYS_ADPT_MAC_ADDR_LEN]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        if(!DHCP_TXRX_FindDestination(to->sin_addr.s_addr, dst_mac))
        {
            to->sin_addr.s_addr = DHCP_TXRX_BROADCAST_IP;
            memcpy(dst_mac, bcast_mac, SYS_ADPT_MAC_ADDR_LEN);
        }
    }

    mref_handle_p = L_MM_AllocateTxBufferFromDedicatedBufPool(
                        L_MM_TX_BUF_POOL_ID_DHCP_TX,
                        L_MM_USER_ID2(SYS_MODULE_DHCP,
                                      DHCP_TXRX_TRACE_ID_SENDPKTTHRUIML));
    if (mref_handle_p == NULL)
    {
        return  ENOBUFS;
    }

    /*  prepare DHCP packet
     */
    buf = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    memcpy((buf+DHCP_TXRX_TYPE_IP_HEADER_LEN+DHCP_TXRX_TYPE_UDP_HEADER_LEN), (char*)raw, len);

    /*  encaptulate IP header   */
    ip_hdr = (DHCP_TYPE_IpHeader_T*) (buf);
    ip_hdr->ip_ver_hlen = 0x45;
    ip_hdr->tos         = 0;
    ip_hdr->tlen        = L_STDLIB_Hton16((short)(DHCP_TXRX_TYPE_IP_HEADER_LEN+DHCP_TXRX_TYPE_UDP_HEADER_LEN+len));
    ip_hdr->id          = 0;
    ip_hdr->offset      = 0;
    ip_hdr->ttl         = 60;
    ip_hdr->proto       = IP_UDP;
    ip_hdr->csum        = 0;
    ip_hdr->sip         = from.s_addr;
    ip_hdr->dip         = to->sin_addr.s_addr;
    ip_hdr->csum        = ipchksum((unsigned short *)ip_hdr, sizeof(DHCP_TYPE_IpHeader_T));

    udp_hdr = (DHCP_TYPE_UdpHeader_T*)((UI8_T*)ip_hdr + sizeof(DHCP_TYPE_IpHeader_T));
    udp_hdr->dst_port = to->sin_port;
    udp_hdr->src_port = L_STDLIB_Hton16(DHCP_TXRX_DEFAULT_CLIENT_PORT); /* use 68   */
    udp_len = DHCP_TXRX_TYPE_UDP_HEADER_LEN + len;
    udp_hdr->udp_header_len = L_STDLIB_Hton16((short) udp_len);
    udp_hdr->chksum = 0;
    
    /* caculate udp checksum */
    calculate_length = DHCP_UDP_PSEUDO_HEADER_LENGTH + udp_len;
    CALCULATE_LENGTH_NO_TRUNCATE(calculate_length);

    udp_hdr->chksum = DHCP_ALGO_UdpChkSumCalculate(ip_hdr, calculate_length);

    /*  calculate packet length */
    packet_length = len + DHCP_TXRX_TYPE_IP_HEADER_LEN + DHCP_TXRX_TYPE_UDP_HEADER_LEN;
    /*  send packet to IML  */
    IML_PMGR_SendPkt(mref_handle_p,
                     interface->vid_ifIndex,
                     packet_length,
                     dst_mac,
                     cpu_mac,
                     ether_type,
                     FALSE);
    return  len;

}   /*  end of DHCP_TXRX_SendPktThruIML */

/* FUNCTION NAME : DHCP_TXRX_SendPktThruIMLToClientPort
 * PURPOSE:
 *      Send out one packet from specified interface thru IML to Client port.
 *
 * INPUT:
 *      interface   --  interface send to.
 *      packet      --  keep dhcp packet and related options.
 *      raw         --  raw packet contains dhcp packet
 *      len         --  packet length
 *      from        --  src ip-address.
 *      to          --  dest. ip address in structure.
 *      hto         --  destination address length.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      <0  -- error occurs (EPIPE, ENOTCONN, EDESTADDRREQ, EMSGSIZE...)
 *          ENOBUFS -- no buffer
 *      others -- Real transmitted packet size.
 *
 * NOTES:
 *      1. Error code please ref. socket define. (sendto)
 *      2. Currently, in switch mode, use managed vlan only. All convertion will not used
 *         when we change to router mode.
 */
ssize_t DHCP_TXRX_SendPktThruIMLToClientPort(struct interface_info *interface,
                     struct packet *packet,
                     struct dhcp_packet *raw,
                     UI32_T len,
                     struct in_addr from,
                     struct sockaddr_in *to,
                     struct hardware *hto)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /*struct interface_info *if_ptr;*/
    L_MM_Mref_Handle_T *mref_handle_p;

    UI32_T      packet_length;
    UI8_T       cpu_mac[6]; /* Penny changed from 16 to 6 */
    UI8_T       dst_mac[6]; /* Penny changed from 16 to 6 2002-1-9 */
    UI8_T       *buf;
    UI32_T      pdu_len;
    DHCP_TYPE_IpHeader_T    *ip_hdr;
    DHCP_TYPE_UdpHeader_T   *udp_hdr;
    unsigned char   broadcast[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    DHCP_TYPE_SYSTEM_STATE_T system_mode;
    UI32_T vid, vid_ifindex;
    UI32_T calculate_length=0;
    UI32_T udp_len = 0;
    const UI16_T  ether_type = 0x0800;      /* ipv4 ether type*/

    /*
     *  Prepare Broadcast parameters
     *  1. source mac uses manage-vlan''s MAC
     *  2. dest. mac :
     *     (in DHCP/IAD code, hto always is NULL.)
     *     check the remote site ip (to).
     *     if it is 0xffffffff,
     *        dest. mac should be 0xffffffffffff.
     *     else
     *        find the ip associated with mac. from tmp. cache table.
     *
     *
     */
    /*  Penny 2002-1-30
     *  Check system mode here. If system mode != DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE
     *      , drop packet
     */
    system_mode = (DHCP_TYPE_SYSTEM_STATE_T)DHCP_MGR_CheckSystemState();
    if (system_mode != DHCP_TYPE_SYSTEM_STATE_PROVISION_COMPLETE)
        return 0;

    VLAN_POM_GetManagementVlan(&vid);
    VLAN_OM_ConvertToIfindex(vid, &vid_ifindex);
    VLAN_PMGR_GetVlanMac(vid_ifindex, cpu_mac);
    /* Penny 2002-1-9 suppose dst info is got from "to" not "hto" */

    memcpy(dst_mac, broadcast, 6);

    mref_handle_p = L_MM_AllocateTxBufferFromDedicatedBufPool(
                        L_MM_TX_BUF_POOL_ID_DHCP_TX,
                        L_MM_USER_ID2(SYS_MODULE_DHCP,
                                      DHCP_TXRX_TRACE_ID_SENDPKTTHRUIMLTOCLIENTPORT));
    if (mref_handle_p == NULL)
    {
        /*  Log message to system : Can't create mref   */
        return  ENOBUFS;
    }

    /*  prepare DHCP packet */
    buf = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    memcpy ((buf+DHCP_TXRX_TYPE_IP_HEADER_LEN+DHCP_TXRX_TYPE_UDP_HEADER_LEN), (char*)raw, len);

    /*  encaptulate IP header   */
    ip_hdr = (DHCP_TYPE_IpHeader_T*) (buf);
    ip_hdr->ip_ver_hlen = 0x45;
    ip_hdr->tos         = 0;
    ip_hdr->tlen        = L_STDLIB_Hton16((short)DHCP_TXRX_TYPE_IP_HEADER_LEN+DHCP_TXRX_TYPE_UDP_HEADER_LEN+len);
    ip_hdr->id          = 0;
    ip_hdr->offset      = 0;
    ip_hdr->ttl         = 60;
    ip_hdr->proto       = IP_UDP;
    ip_hdr->csum        = 0;
    ip_hdr->sip         = from.s_addr;
    ip_hdr->dip         = to->sin_addr.s_addr;
    ip_hdr->csum        = ipchksum((unsigned short *)ip_hdr, sizeof(DHCP_TYPE_IpHeader_T));

    udp_hdr = (DHCP_TYPE_UdpHeader_T*)((UI8_T*)ip_hdr + sizeof(DHCP_TYPE_IpHeader_T));
    udp_hdr->dst_port = to->sin_port;
    udp_hdr->src_port = L_STDLIB_Hton16(DHCP_TXRX_DEFAULT_SERVER_PORT); /* use 67   */
    udp_len = DHCP_TXRX_TYPE_UDP_HEADER_LEN + len;
    udp_hdr->udp_header_len = L_STDLIB_Hton16((short) udp_len);
    udp_hdr->chksum = 0;

    /* caculate udp checksum */
    calculate_length = DHCP_UDP_PSEUDO_HEADER_LENGTH + udp_len;
    CALCULATE_LENGTH_NO_TRUNCATE(calculate_length);

    udp_hdr->chksum = DHCP_ALGO_UdpChkSumCalculate(ip_hdr, calculate_length);

    /*  calculate packet length */
    packet_length = len + DHCP_TXRX_TYPE_IP_HEADER_LEN + DHCP_TXRX_TYPE_UDP_HEADER_LEN;
    /*  send packet to IML  */
    IML_PMGR_SendPkt(mref_handle_p,
                     interface->vid_ifIndex,
                     packet_length,
                     dst_mac,
                     cpu_mac,
                     ether_type,
                     FALSE);
    return  len;
}   /*  end of DHCP_TXRX_SendPktThruIMLToClientPort */

/* FUNCTION NAME : receive_packet
 * PURPOSE:
 *      Retrieve one packet from specified interface.
 *
 * INPUT:
 *      sockid      --  the socket we will receive from.
 *      buf         --  buffer keeping received packet.
 *      len         --  buffer size.
 *      from        --  pointer point to source address
 *      hfrom       --  source hardware address of this packet.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      0       -- end of file, remote is closed.
 *      < 0     -- error occurs. (EWOULDBLOCK, ENOTCONN)
 *      others  -- length of received data on success
 *
 *
 * NOTES:
 *      Error code please ref. socket define. (recvfrom)
 */
ssize_t receive_packet (int sockid,
                        unsigned char *buf,
                        UI32_T len,
                        struct sockaddr_in2 *from,
                        struct hardware *hfrom)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    socklen_t flen = sizeof *from;
    int result;

    /* BODY */
    result = recvfrom (sockid, (char *)buf, len, 0,
                   (struct sockaddr *)from, &flen);

    return result;
}   /*  end of receive_packet   */


/* FUNCTION NAME : DHCP_TXRX_SaveIpMac
 * PURPOSE:
 *      Save (ip, mac) pair in local table for future reference.
 *
 * INPUT:
 *      ip      --  the ip associated with mac. (key)
 &      rif     --  routing interface.
 *      mac     --  the source mac which bring this ip packet from.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE    --  OK.
 *      FLASE   --  Table full.
 *
 *
 * NOTES:
 *      Error code please ref. socket define. (recvfrom)
 */
BOOL_T DHCP_TXRX_SaveIpMac(UI32_T ip, int ifindex, UI8_T *mac)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     index;
    int     i;
//Timon    UI32_T  pre_mask;
    /* BODY */
    index = DHCP_TXRX_FindEntryByIp(ip);
    if (-1 == index)
    {
        for (i=0; i<DHCP_TXRX_MAX_LEARN_IP; i++)
        {
            if (ip_mac_table[i].ip == 0)
            {
//Timon                pre_mask = SYSFUN_InterruptLock();
                ip_mac_table[i].ip = ip;
                ip_mac_table_entry_used++;
//Timon                SYSFUN_InterruptUnlock(pre_mask);
                index = i;
                break;
            }
        }
        if (i>= DHCP_TXRX_MAX_LEARN_IP)
            return  FALSE;
    }
    /* Layer 2 use: set rif to 1 */
#if SYS_CPNT_ROUTING
    ip_mac_table[index].ifindex = ifindex;
#else
    ip_mac_table[index].ifindex = 1;
#endif
    memcpy (ip_mac_table[index].mac, mac, 6);
    return  TRUE;
}   /*  end of DHCP_TXRX_SaveIpMac  */



/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */

/* FUNCTION NAME : DHCP_TXRX_GetMacByIp
 * PURPOSE:
 *      Retrieve (ip, mac) pair from local table.
 *
 * INPUT:
 *      ip      --  the ip associated with mac. (key)
 *
 * OUTPUT:
 *      rif     --  associated rif.
 *      mac     --  the source mac which bring this ip packet from.
 *
 * RETURN:
 *      TRUE    --  OK.
 *      FLASE   --  Not found.
 *
 *
 * NOTES:
 *      Error code please ref. socket define. (recvfrom)
 */
static BOOL_T DHCP_TXRX_GetMacByIp(UI32_T ip, int *ifindex, UI8_T *mac)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int     index;
    int     i;
    UI8_T   *ptr;
    /* BODY */
    index = DHCP_TXRX_FindEntryByIp(ip);
    if (-1 == index)
    {
        ptr = mac;
        for (i=0; i<6; i++, ptr++)
            *ptr = 0xff;
        return  FALSE;
    }
    memcpy (mac, ip_mac_table[index].mac, 6);
    *ifindex = ip_mac_table[index].ifindex;
    return  TRUE;
}   /*  end of DHCP_TXRX_GetMacByIp */


/* FUNCTION NAME : DHCP_TXRX_FindEntryByIp
 * PURPOSE:
 *      Find the index of entry, same ip with input.
 *
 * INPUT:
 *      ip      --  the ip associated with mac. (key)
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      -1  --  not found
 *      n   --  the index.
 *
 *
 * NOTES:
 *      Error code please ref. socket define. (recvfrom)
 */
static int DHCP_TXRX_FindEntryByIp(UI32_T ip)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    int i;
    int total_count = ip_mac_table_entry_used;
    /* BODY */
    for (i=0; i<DHCP_TXRX_MAX_LEARN_IP && total_count>0; i++)
    {
        if (ip_mac_table[i].ip == 0)
            continue;
        if (ip_mac_table[i].ip == ip)
            return  i;
        else
            total_count--;
    }
    return  -1;
}   /*  end of DHCP_TXRX_FindEntryByIp  */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - DHCP_TXRX_FindDestination
 * ---------------------------------------------------------------------
 * PURPOSE  : Find destination mac address according to destination ip address
 * INPUT    : dst_ip        --  destination ip address
 * OUTPUT   : dst_mac       --  destination mac address
 * RETURN   : TRUE/FALSE
 * NOTES    : if mac cache has record with destination ip address, use corresponding mac address;
 *            if no record with desitnation ip address, check if gateway is existed. 
 *            if yes, check mac cache and use corresponding mac address; otherwise, use broadcast mac address
 * ---------------------------------------------------------------------
 */
static BOOL_T DHCP_TXRX_FindDestination(
    UI32_T dst_ip,
    UI8_T  dst_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    int ifindex=0;

    if(NULL == dst_mac)
        return FALSE;
    
    if(dst_ip == DHCP_TXRX_BROADCAST_IP)
    {   
        return FALSE;
    }

    if(DHCP_TXRX_GetMacByIp(dst_ip, &ifindex, dst_mac))
    {
        if(ifindex == 0)
        {
            NETCFG_TYPE_InetRifConfig_T rif_config;
            memset(&rif_config, 0, sizeof(rif_config));
            memcpy(rif_config.addr.addr, &dst_ip, SYS_ADPT_IPV4_ADDR_LEN);
            rif_config.addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            rif_config.addr.type = L_INET_ADDR_TYPE_IPV4;
    
            if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetRifFromIp(&rif_config))
            {	
                /* if ifindex is invalid, we must get if from NETCFG_OM_IP and save it in mac cache
                 */
                DHCP_TXRX_SaveIpMac(dst_ip, rif_config.ifindex, dst_mac);		
            }		
        }
        
        return TRUE;
    }

    return FALSE;
}



/**************************************************************************
IPCHKSUM - Checksum IP Header
**************************************************************************/
static unsigned short ipchksum(unsigned short *ip, int len)
{
    unsigned long sum = 0;
    len >>= 1;
    while (len--) {
        sum += *(ip++);
        if (sum > 0xFFFF)
            sum -= 0xFFFF;
    }
    return(((unsigned short)(~sum) & 0x0000FFFF));
}

