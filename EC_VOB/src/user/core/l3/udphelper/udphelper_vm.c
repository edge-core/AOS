/* MODULE NAME:  udphelper_mgr.c
 * PURPOSE:
 *     This module provides APIs for UDPHELPER CSC to use.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    03/31/2009 - Lin.Li, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#if (SYS_CPNT_UDP_HELPER == TRUE)
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "dhcp_type.h"
#include "udphelper_om.h"
#include "udphelper_type.h"
#include "amtr_pmgr.h"
#include "amtrl3_pmgr.h"
#include "amtrl3_om.h"
#include "vlan_lib.h"
#include "leaf_es3626a.h"
#include "leaf_4001.h"
#include <sys/socket.h>
/* do not define errno shown below, it will lead to link error shown below on
 * some toolchains:
 * errno: TLS definition in /mnt/disk2/powerpc-none-linux-gnuspe/bin/../powerpc-none-linux-gnuspe/libc/lib/libc.so.6
 * section .tbss mismatches non-TLS reference in ../../core/l3/udphelper/libudphelper_private.a(libudphelper_private_a-udphelper_vm.o)
 */
#if 0
extern int errno;
#else
#include <errno.h>
#endif

static UI32_T udphelper_debug_vm_static = FALSE;

#define UDPHELPER_VM_DebugPrint(ARGS...)                \
    do{                                                 \
        if (udphelper_debug_vm_static)                  \
            printf(ARGS);                               \
    }while(0)

void UDPHELPER_VM_SetDebugStatus(UI32_T status)
{
    udphelper_debug_vm_static = status;
    return;
}
/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_VM_BOOTPRelay
 *------------------------------------------------------------------------
 * FUNCTION: Handle BOOTP packets.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------
 */  
void UDPHELPER_VM_BOOTPRelay(
  UI32_T ifindex,
  DHCP_TYPE_IpHeader_T* ip_pkt,
  DHCP_TYPE_UdpHeader_T* udp_header,
  struct dhcp_packet* bootp_packet)
{
    UI32_T ret;
	int on =1;    
    L_INET_AddrIp_T helper_addr;
    UI32_T output;  
    UI32_T vid;
    int sockfd = 0;
    /*int state;*/
    int result;    
    struct msghdr msg;
    /*struct cmsghdr *cmsg;*/
    struct sockaddr_in from;    
    struct sockaddr_in to;    
    struct iovec iov[1];
    /*struct in_pktinfo *pkt;*/
    UI32_T dip;
    AMTR_TYPE_AddrEntry_T addr_entry;
    AMTRL3_TYPE_InetHostRouteEntry_T host_entry;
    BOOL_T      ret_bool = FALSE;

    UDPHELPER_VM_DebugPrint("UDPHELPER_VM_BOOTPRelay\r\n");
    /* Create socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
        UDPHELPER_VM_DebugPrint("Failed to create UDP socket.\r\n");
	    return;
	}
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(void *)&on, sizeof(on)) < 0)
    {
        close(sockfd);
        UDPHELPER_VM_DebugPrint("Failed to set socket option SO_REUSEADDR.\r\n");
        return;
    }   
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,(void *)&on, sizeof(on)) < 0)
    {
        close(sockfd);
        UDPHELPER_VM_DebugPrint("Failed to set socket option SO_BROADCAST.\r\n");
        return;
    }            
    
    /* set zero of msg hdr */
    memset (&msg, 0, sizeof (struct msghdr));
    memset (&from, 0, sizeof(from));
    memset (&to, 0, sizeof(to));
    ret = UDPHELPER_OM_CheckHelper(ifindex, ntohl(ip_pkt->dip), ntohs(udp_header->dst_port), &output);
    if ( ret == UDPHELPER_TYPE_RESULT_BROADCAST )
    {
#if 0
        from.sin_family = AF_INET;
        /* Set  port. */
        from.sin_port = htons(UDPHELPER_TYPE_BOOTP_CLIENT_PORT); 
        /* Set the ip address with the address of my interface from which the 
                  broadcast packet was received */
        from.sin_addr.s_addr = htons(output);
        /* Address shoud be any address. */        
        ret = bind (sockfd, (struct sockaddr *) &from, 
                     sizeof (struct sockaddr_in));
        if (ret < 0)
        {
            UDPHELPER_VM_DebugPrint("Failed to bind dhcp client port.\r\n");
            close(sockfd);
        }

        /* set the source ip address */
        msg.msg_control = malloc(sizeof(struct cmsghdr) + sizeof(struct in_pktinfo));
        msg.msg_controllen = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo);
        cmsg = CMSG_FIRSTHDR (&msg);
        cmsg->cmsg_len = CMSG_LEN (sizeof (struct in_pktinfo));
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        pkt = (struct in_pktinfo *)CMSG_DATA (cmsg);
        pkt->ipi_spec_dst.s_addr = htons(output);
#endif
        /* The source IP must be ZERO */
        if ( ntohl(ip_pkt->sip) != 0 )
        {
            UDPHELPER_VM_DebugPrint("Source IP is invalid %lx.\r\n", ntohl(ip_pkt->sip));
            close(sockfd);
            return;
        }
        /* The op code  must be 0x01: bootp request message */
        if ( bootp_packet->op != 0x01 )
        {
            UDPHELPER_VM_DebugPrint("DHCP opcode is invalid(should be 0x01) %x.\r\n", bootp_packet->op);
            close(sockfd);
            return;
        }            
        memset(&helper_addr, 0, sizeof(helper_addr));
        while ( UDPHELPER_OM_GetNextHelper(ifindex, &helper_addr)
                     == UDPHELPER_TYPE_RESULT_SUCCESS )
        {            
            to.sin_family = AF_INET;  
            to.sin_port = htons(UDPHELPER_TYPE_BOOTP_SERVER_PORT);            
            memcpy(&to.sin_addr.s_addr, helper_addr.addr, sizeof(helper_addr.addr));
            to.sin_addr.s_addr = htonl(to.sin_addr.s_addr);
            /* 2009-10-22 Jimi, Send UDP packet doesn't need to connect first */
#if 0            
            ret = connect(sockfd, (struct sockaddr *)&to, sizeof (struct sockaddr_in));  
            if ( ret < 0 )
            {
                UDPHELPER_VM_DebugPrint("Failed to connect to dhcp server port.\r\n");
                close(sockfd);
                return;
            }
#endif            
            /* Modify the relevant fields in the dhcp packet */

            /* RFC 1542 4.1.1 
             * The relay agent MUST silently discard BOOTREQUEST messages whose
    ’h       * 'hops' field exceeds the value 16
             */
        	if (bootp_packet->hops > 16)
        	{
                UDPHELPER_VM_DebugPrint("Discarding packet with invalid hops(%u)\r\n", bootp_packet->hops);
                close(sockfd);
        		return;
            }
      
            bootp_packet->hops += 1;
            memcpy(&bootp_packet->giaddr, &output, 4);
            /* Prepare to send message to dhcp server */
            iov[0].iov_base = (void*)bootp_packet;
            iov[0].iov_len = ntohs(ip_pkt->tlen) - (ip_pkt->ip_ver_hlen & 0x0F)*4 - sizeof(DHCP_TYPE_UdpHeader_T);            
            msg.msg_name = &to;
            msg.msg_namelen = sizeof (struct sockaddr_in);
            msg.msg_iov = iov;
            msg.msg_iovlen = 1;
            result = sendmsg ( sockfd, &msg, 0 );
            if ( result < 0 )
            {
                UDPHELPER_VM_DebugPrint("Failed to send msg to dhcp server: %x, ret = %d.\r\n", to.sin_addr.s_addr, result);
                /* Jimi, for EPR ES4626F-SW-FLF-38-00213 
                 * if helper-address is the same as the ip address in the DUT,
                * sendmsg will return result < 0 and close socket.
                * this will cause other helper-address won't be forward. */
                /*close(sockfd);*/
                /*return;*/
            }
        }
    }
    else if ( ret == UDPHELPER_TYPE_RESULT_DHCP_RELAY_BACK )
    {
        /* Check the TTL */
        ip_pkt->ttl--;
        if ( !ip_pkt->ttl )
        {
            UDPHELPER_VM_DebugPrint("dhcp reply packet's ttl is zero.\r\n");
            close(sockfd);
            return;                            
        }
        /* The op code  must be 0x01: bootp reply message */
        if ( bootp_packet->op != 0x02 )
        {
            UDPHELPER_VM_DebugPrint("DHCP opcode is invalid(should be 0x02) %x.\r\n", bootp_packet->op);
            close(sockfd);
            return;
        }                    
        /* Bind myself. */
        memset (&from, 0, sizeof (struct sockaddr_in));
        /* Set  port. */
        from.sin_port = htons(UDPHELPER_TYPE_BOOTP_SERVER_PORT);  
        /* Address shoud be one of my interface's address. */
        from.sin_addr.s_addr = ip_pkt->dip;
        from.sin_family = AF_INET;        
        ret = bind (sockfd, (struct sockaddr *) &from, 
                     sizeof (struct sockaddr_in));
        if (ret < 0)
        {
            UDPHELPER_VM_DebugPrint("Failed to bind dhcp server port.\r\n");
            close(sockfd);
            return;
        }
        if ( bootp_packet->flags & UDPHELPER_TYPE_BOOTP_FLAGS_BROADCAST )
        {
            dip = 0xffffffff; /* broadcast address */
        }
        else
        {
            dip = bootp_packet->yiaddr;
            /* Get the port information according to the MAC address */
            VLAN_OM_ConvertFromIfindex(output, &vid);
            addr_entry.vid = vid;
            memcpy(addr_entry.mac, bootp_packet->chaddr, SYS_ADPT_MAC_ADDR_LEN);
            ret = AMTR_PMGR_GetExactAddrEntry(&addr_entry);
            if ( !ret )
            {
                UDPHELPER_VM_DebugPrint("Didn't learned this mac address on vlan %lu, mac: %d.%d.%d.%d.%d.%d.\r\n", 
                                         output, addr_entry.mac[0],
                                         addr_entry.mac[1], addr_entry.mac[2],
                                         addr_entry.mac[3],addr_entry.mac[4],
                                         addr_entry.mac[5]);
                close(sockfd); 
                return;
            }            
            memset(&host_entry,0,sizeof(host_entry));
            host_entry.dst_vid_ifindex = output;
            memcpy(host_entry.dst_mac, bootp_packet->chaddr, SYS_ADPT_MAC_ADDR_LEN);
            host_entry.dst_inet_addr.type = L_INET_ADDR_TYPE_IPV4;
            host_entry.dst_inet_addr.addrlen = 4;
            memcpy(host_entry.dst_inet_addr.addr, &dip, 4);
            host_entry.lport = addr_entry.ifindex;

            
            ret_bool = AMTRL3_OM_GetInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4, 0, &host_entry);
            
            if ( !ret_bool )
            {
                UDPHELPER_VM_DebugPrint("No host entry existed\r\n");
                /* Create dynamic arp entry */

                ret = AMTRL3_PMGR_SetHostRoute(AMTRL3_TYPE_FLAGS_IPV4, 0, &host_entry, 
                               VAL_ipNetToPhysicalExtType_dynamic);

                if ( !ret )
                {
                    UDPHELPER_VM_DebugPrint("Failed to create arp entry.\r\n");
                    close(sockfd);
                    return;
                }  
            }
        }
        /* Modify fields of dhcp packet */
        bootp_packet->hops--;
        bootp_packet->giaddr = 0;
        memset (&to, 0, sizeof(to));
        to.sin_family = AF_INET;  
        to.sin_port = htons(UDPHELPER_TYPE_BOOTP_CLIENT_PORT);
        to.sin_addr.s_addr = htonl(dip);
        ret = connect(sockfd, (struct sockaddr *)&to, sizeof (struct sockaddr_in));  
        if ( ret < 0 )
        {
            UDPHELPER_VM_DebugPrint("Failed to connect to dhcp client port.\r\n");
            close(sockfd);
            return;
        }
#if 0
        /* set the source ip address */
        msg.msg_control = malloc(sizeof(struct cmsghdr) + sizeof(struct in_pktinfo));
        msg.msg_controllen = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo);
        cmsg = CMSG_FIRSTHDR (&msg);
        cmsg->cmsg_len = CMSG_LEN (sizeof (struct in_pktinfo));
        cmsg->cmsg_level = IPPROTO_IP;
        cmsg->cmsg_type = IP_PKTINFO;
        pkt = (struct in_pktinfo *)CMSG_DATA (cmsg);
        pkt->ipi_spec_dst.s_addr = ip_pkt->dip;
#endif
        /* prepare to send message */
        iov[0].iov_base = (void*)bootp_packet;
        iov[0].iov_len = ntohs(ip_pkt->tlen) - ( (ip_pkt->ip_ver_hlen & 0x0F)*4 ) - sizeof(DHCP_TYPE_UdpHeader_T);            
        msg.msg_name = &to;
        msg.msg_namelen = sizeof (struct sockaddr_in);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        result = sendmsg ( sockfd, &msg, 0 );
        if ( result < 0 )
        {
            UDPHELPER_VM_DebugPrint("bootp msg send error 2: %d.\r\n", result);
            close(sockfd);
            return;                            
        }
    }
    close(sockfd);
    return;
}
/*------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_VM_UDPRelay
 *------------------------------------------------------------------------
 * FUNCTION: Handle common UDP packets.
 *
 * INPUT   : 
 *
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 
 *------------------------------------------------------------------------
 */  
void UDPHELPER_VM_UDPRelay(
  UI32_T ifindex,
  DHCP_TYPE_IpHeader_T* ip_pkt,
  DHCP_TYPE_UdpHeader_T* udp_header)
{
    UI32_T ret;
    UI32_T output;
    int sockfd = 0;  
    int result;
    struct msghdr msg;
    /*struct sockaddr_in from;*/
    struct sockaddr_in to;    
    struct iovec iov[1];
    /*struct in_pktinfo *pkt;*/
    L_INET_AddrIp_T helper_addr;
    int    on = 1;
    
    UDPHELPER_VM_DebugPrint("UDPHELPER_VM_UDPRelay\r\n");
    /* Check again */
    ret = UDPHELPER_OM_CheckHelper(ifindex, ntohl(ip_pkt->dip), ntohs(udp_header->dst_port), &output);
    if ( ret != UDPHELPER_TYPE_RESULT_BROADCAST )
        return;
    /* Create socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd < 0)
	{
        UDPHELPER_VM_DebugPrint("Failed to create UDP socket for normal application.\r\n");
	    return;
	}
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(void *)&on, sizeof(on)) < 0)
    {
        close(sockfd);
        UDPHELPER_VM_DebugPrint("Failed to set socket option SO_REUSEADDR  for normal application.\r\n");
        return;
    }    
#if 0	
    from.sin_family = AF_INET;
    /* Set  port. */
    from.sin_port = udp_header->src_port;
    /* Set the ip address with the address of my interface from which the 
              broadcast packet was received */
    from.sin_addr.s_addr = ip_pkt->sip;
    ret = bind (sockfd, (struct sockaddr *) &from, 
                 sizeof (struct sockaddr_in));
    if (ret < 0)
    {
        UDPHELPER_VM_DebugPrint("Failed to bind port for normal application.\r\n");
        close(sockfd);
    }
    /* set the source ip address */
    msg.msg_control = malloc(sizeof(struct cmsghdr) + sizeof(struct in_pktinfo));
    msg.msg_controllen = sizeof(struct cmsghdr) + sizeof(struct in_pktinfo);
    cmsg = CMSG_FIRSTHDR (&msg);
    cmsg->cmsg_len = CMSG_LEN (sizeof (struct in_pktinfo));
    cmsg->cmsg_level = IPPROTO_IP;
    cmsg->cmsg_type = IP_PKTINFO;
    pkt = (struct in_pktinfo *)CMSG_DATA (cmsg);
    pkt->ipi_spec_dst.s_addr = ip_pkt->sip;
#endif	
	memset(&msg, 0, sizeof(msg));
    memset(&helper_addr, 0, sizeof(helper_addr));
    while ( UDPHELPER_OM_GetNextHelper(ifindex, &helper_addr)
                 == UDPHELPER_TYPE_RESULT_SUCCESS )
    {            
        UDPHELPER_VM_DebugPrint("%s[%d],helper addr = %d.%d.%d.%d.\r\n",__FUNCTION__,__LINE__,
            helper_addr.addr[0],helper_addr.addr[1],helper_addr.addr[2],helper_addr.addr[3]);
        to.sin_family = AF_INET;  
        to.sin_port = udp_header->dst_port;
        memcpy(&to.sin_addr.s_addr, helper_addr.addr, sizeof(helper_addr.addr));
        to.sin_addr.s_addr = htonl(to.sin_addr.s_addr);        
#if 0
        ret = connect(sockfd, (struct sockaddr *)&to, sizeof (struct sockaddr_in));  
        if ( ret < 0 )
        {
            UDPHELPER_VM_DebugPrint("Failed to connect to port for normal application.\r\n");
            close(sockfd);
            return;
        }
#endif        
        /* Prepare to send message to dhcp server */
        iov[0].iov_base = (void*)((char*)udp_header + sizeof(DHCP_TYPE_UdpHeader_T));
        iov[0].iov_len = ntohs(ip_pkt->tlen) - ( (ip_pkt->ip_ver_hlen & 0x0F)*4 ) - sizeof(DHCP_TYPE_UdpHeader_T);            
        msg.msg_name = &to;
        msg.msg_namelen = sizeof (struct sockaddr_in);
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        result = sendmsg ( sockfd, &msg, 0 );
        if ( result < 0 )
        {
            UDPHELPER_VM_DebugPrint("Failed to send msg to udp helper for normal application: %x(%s).\r\n", to.sin_addr.s_addr,strerror(errno));
            /* Jimi, for EPR ES4626F-SW-FLF-38-00213 
             * if helper-address is the same as the ip address in the DUT,
             * sendmsg will return result < 0 and close socket.
             * this will cause other helper-address won't be forward. */
            /*close(sockfd);*/
            /*return;*/
        }
        
    }    
    close(sockfd);
    return;                
}

#endif
