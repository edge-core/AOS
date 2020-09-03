/*
 *   File Name: ipal_rt_netlink.c
 *   Purpose:   ipal netlink socket implementation in user space
 *   Note:
 *   Create:    Basen LV     2008.04.06
 *
 *   Histrory:
 *              Modify         Date      Reason
 *
 *
 *   Copyright(C)  Accton Corporation 2007~2009
 */

/*
 * INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_module.h"

#include "l_sort_lst.h"
#include "l_prefix.h"
#include "l_mm.h"
#include "netinet/in.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_addr.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/neighbour.h>
#include <linux/if_link.h>

#include "ipal_types.h"
#include "ipal_debug.h"
#include "ipal_reflect.h"

#include "vlan_lib.h"

/*
 * NAMING CONST DECLARATIONS
 */
#define NUD_VALID   (NUD_PERMANENT|NUD_NOARP|NUD_REACHABLE|NUD_PROBE|NUD_STALE|NUD_DELAY)
#define IPAL_BUFSIZ 4096

#define IPAL_UNSPEC             0x00
#define IPAL_ARP                0x01
#define IPAL_IF                 0x02
#define IPAL_ROUTE_IPV4         0x03
#define IPAL_ROUTE_IPV6         0x04
#define IPAL_NEIGH_IPV4         0x05
#define IPAL_NEIGH_IPV6         0x06
#define IPAL_ADDR_IPV4          0x07
#define IPAL_ADDR_IPV6          0x08
#define IPAL_ROUTE_CACHE_IPV6   0x09
#define IPAL_ROUTE_LOOKUP       0x0A


/*
 * MACRO FUNCTION DECLARATIONS
 */
#define NDA_RTA(r)      ((struct rtattr*)(((char*)(r)) + NLMSG_ALIGN(sizeof(struct ndmsg))))
#define NDA_PAYLOAD(n)  NLMSG_PAYLOAD(n,sizeof(struct ndmsg))

/*
 * DATA TYPE DECLARATIONS
 */

/* Message structure
 */
typedef struct RT_MSG_S
{
    UI32_T key;
    UI8_T *str;
}RT_MSG_T;

/* Socket interface to kernel
 */
typedef struct NLSOCK_S
{
    int sock;
    int seq;
    struct sockaddr_nl snl;
    UI8_T *name;
}NLSOCK_T;

/*
 * STATIC VARIABLE DECLARATIONS
 */

/* Command channel
 */
NLSOCK_T ipal_netlink_cmd  = { -1, 0, {0,}, (UI8_T *)"netlink-cmd" };
NLSOCK_T ipal_netlink_listen = { -1, 0, {0}, (UI8_T *)"netlink-listen" };

RT_MSG_T nlmsg_rt_str[] =
{
    {RTM_NEWROUTE, (UI8_T *)"RTM_NEWROUTE"},
    {RTM_DELROUTE, (UI8_T *)"RTM_DELROUTE"},
    {RTM_GETROUTE, (UI8_T *)"RTM_GETROUTE"},
    {RTM_NEWLINK,  (UI8_T *)"RTM_NEWLINK"},
    {RTM_DELLINK,  (UI8_T *)"RTM_DELLINK"},
    {RTM_GETLINK,  (UI8_T *)"RTM_GETLINK"},
    {RTM_NEWADDR,  (UI8_T *)"RTM_NEWADDR"},
    {RTM_DELADDR,  (UI8_T *)"RTM_DELADDR"},
    {RTM_GETADDR,  (UI8_T *)"RTM_GETADDR"},
    {0,            NULL}
};

/*
 * LOCAL SUBPROGRAM DECLARATIONS
 */
#if defined (IPAL_DEBUG)
static char *IPAL_Netlink_Msg_Lookup (RT_MSG_T *mes, UI32_T key);
#endif

static UI32_T IPAL_Netlink_Talk_Filter (struct sockaddr_nl *snl, struct nlmsghdr *h);
static UI32_T IPAL_Netlink_Metrics(struct rtattr *rta, int len, int *mtu);
static UI32_T IPAL_Netlink_Parse_Info (NLSOCK_T *nl, void *arg1, void *arg2);
static UI32_T IPAL_Netlink_Talk (struct nlmsghdr *n, NLSOCK_T *nl);
static UI32_T IPAL_Addattr_l (struct nlmsghdr *n, int maxlen, int type, void *data, int alen);
static UI32_T IPAL_Addattr32 (struct nlmsghdr *n, int maxlen, int type, int data);
static void   IPAL_Parse_Rtattr (struct rtattr **tb, int max, struct rtattr *rta, int len);
//static UI32_T IPAL_Netlink_Arp (int cmd, int family, int flags, const IPAL_IPv4_ARP_T *arp);
static UI32_T IPAL_Netlink_Neigh (int cmd, int family, int flags, const IPAL_NeighborEntry_T *neigh);
static UI32_T IPAL_Netlink_Route (int cmd, int family, int flags, int table, const L_INET_AddrIp_T *p,
                                    const void *nhs);
static UI32_T IPAL_Netlink_Address (int cmd, int family, int flags, UI32_T ifindex, const L_INET_AddrIp_T *p);
static UI32_T IPAL_Netlink_AddressAlias (int cmd, int family, int flags, UI32_T ifindex, const L_INET_AddrIp_T *p, UI32_T alias_id);
static UI32_T IPAL_Netlink_Socket (NLSOCK_T *nl, UI32_T groups, UI8_T non_block);
static UI32_T IPAL_Netlink_Request (int family, int type, NLSOCK_T *nl);
static UI32_T IPAL_Netlink_RequestRoute (int family, L_INET_AddrIp_T *ipaddr_p, NLSOCK_T *nl);

#if 0
static UI32_T IPAL_Rt_Netlink_Arp_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_IPv4_ARP_T *arp_entry);
#endif

static UI32_T IPAL_Rt_Netlink_Neigh_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_NeighborEntryIpv4_T *neigh_entry);
static UI32_T IPAL_Rt_Netlink_Addr_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv4AddressEntry_T *addr_entry);
static UI32_T IPAL_Rt_Netlink_Route_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv4UcRouteEntry_T *route_entry);

#if (SYS_CPNT_IPV6 == TRUE)
static UI32_T IPAL_Rt_Netlink_Neigh_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_NeighborEntryIpv6_T *neigh_entry);
static UI32_T IPAL_Rt_Netlink_Addr_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv6AddressEntry_T *addr_entry);
static UI32_T IPAL_Rt_Netlink_Route_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv6UcRouteEntry_T *route_entry, BOOL_T is_cache);
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

static UI32_T IPAL_Rt_Netlink_IF_Info (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_IfInfo_T *if_info_entry);
static UI32_T IPAL_Rt_Netlink_RouteIf_Info (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_RouteIfInfo_T *route_if_p);

static BOOL_T IPAL_Rt_Netlink_IsInterfaceAliasAddress(struct ifaddrmsg *ifa, struct rtattr *rta);

/* Message lookup function
 */
#if defined (IPAL_DEBUG)
static char *IPAL_Netlink_Msg_Lookup (RT_MSG_T *mes, UI32_T key)
{
    RT_MSG_T *pnt;
    for (pnt = mes; pnt->key != 0; pnt++)
    {
        if (pnt->key == key)
        {
            return (char *)pnt->str;
        }
    }
    return "";
}
#endif

static UI32_T IPAL_Netlink_Talk_Filter (struct sockaddr_nl *snl, struct nlmsghdr *h)
{
    IPAL_DEBUG_PRINT ("netlink_talk: ignoring message type 0x%04x",
                        h->nlmsg_type);
    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Netlink_Metrics(struct rtattr *rta, int len, int *mtu)
{
    while(RTA_OK(rta, len))
    {
        if(rta->rta_type == RTAX_MTU)
        {
            *mtu = *(unsigned *)RTA_DATA(rta);
            return IPAL_RESULT_OK;
        }

        rta = RTA_NEXT(rta, len);
    }

    return IPAL_RESULT_FAIL;
}


/* Receive message from netlink interface and pass those information
   to the given function. */
static UI32_T IPAL_Netlink_Parse_Info (NLSOCK_T *nl, void *arg1, void *arg2)
{
    int status;
    UI32_T ret   = IPAL_RESULT_OK;
    UI32_T error = IPAL_RESULT_OK;
    UI32_T doit;
    /* IPAL_IPv4_ARP_LIST_T    *arp_list_p = NULL; */
    L_SORT_LST_List_T       *neigh_ipv4_list_p = NULL;
    L_SORT_LST_List_T       *neigh_ipv6_list_p = NULL;
    IPAL_Ipv4AddressList_T  **ipv4_addr_list_pp = NULL, *ipv4_addr_list_p = NULL, *tmp_ipv4_addr_list_p = NULL;
    IPAL_Ipv6AddressList_T  **ipv6_addr_list_pp = NULL, *ipv6_addr_list_p = NULL, *tmp_ipv6_addr_list_p = NULL;;
    IPAL_IfInfoList_T       **if_list_pp = NULL, *if_list_p = NULL, *tmp_if_list_p = NULL;
    IPAL_Ipv4UcRouteList_T  **ipv4_route_list_pp = NULL, *ipv4_route_list_p = NULL, *tmp_ipv4_route_list_p = NULL;
    IPAL_Ipv6UcRouteList_T  **ipv6_route_list_pp = NULL, *ipv6_route_list_p = NULL, *tmp_ipv6_route_list_p = NULL;
    IPAL_RouteIfInfo_T      *route_if_p = NULL;
    char *buf = NULL;

    doit = *((UI32_T *) arg2);

    switch (doit)
    {
#if 0
        case IPAL_ARP:
            arp_list_p = (IPAL_IPv4_ARP_LIST_T *) arg1;
            break;
#endif
        case IPAL_NEIGH_IPV4:
            neigh_ipv4_list_p = (L_SORT_LST_List_T *) arg1;
            break;
        case IPAL_NEIGH_IPV6:
            neigh_ipv6_list_p = (L_SORT_LST_List_T *) arg1;
            break;
        case IPAL_ADDR_IPV4:
            ipv4_addr_list_pp = (IPAL_Ipv4AddressList_T **) arg1;
            break;
        case IPAL_ADDR_IPV6:
            ipv6_addr_list_pp = (IPAL_Ipv6AddressList_T **) arg1;
            break;
        case IPAL_IF:
            if_list_pp = (IPAL_IfInfoList_T **)arg1;
            break;
        case IPAL_ROUTE_IPV4:
            ipv4_route_list_pp = (IPAL_Ipv4UcRouteList_T **)arg1;
            break;
        case IPAL_ROUTE_IPV6:
        case IPAL_ROUTE_CACHE_IPV6:
            ipv6_route_list_pp = (IPAL_Ipv6UcRouteList_T **)arg1;
            break;
        case IPAL_ROUTE_LOOKUP:
            route_if_p = (IPAL_RouteIfInfo_T *) arg1;
            break;
        case IPAL_UNSPEC:
            break;
        default:
            return IPAL_RESULT_FAIL;
    }

    buf = (char *)L_MM_Malloc(IPAL_BUFSIZ, L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO));
    if(buf == NULL)
    {
        IPAL_DEBUG_PRINT ( "Failed to allocate 4096 bypte buf");
        return IPAL_RESULT_FAIL;
    }

    while (1)
    {
        /* this variable is too big to exhaust stack, we use malloc instead of local variable */
        /* char buf[4096]; */

        struct iovec iov = { buf, IPAL_BUFSIZ};
        struct sockaddr_nl snl = {0};
        struct msghdr msg = { (void*)&snl, sizeof(snl), &iov, 1, NULL, 0, 0};
        struct nlmsghdr *h;

        status = recvmsg (nl->sock, &msg, 0);

        if (status < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EWOULDBLOCK)
                break;

            IPAL_DEBUG_PRINT ("%s recvmsg overrun: %s", nl->name,
                                strerror (errno));
            continue;
        }

        if (snl.nl_pid != 0)
        {
            IPAL_DEBUG_PRINT ("Ignoring non kernel message from pid %u",
                                snl.nl_pid);
            continue;
        }

        if (status == 0)
        {
            IPAL_DEBUG_PRINT ("%s EOF", nl->name);
            ret = IPAL_RESULT_FAIL;
            goto release_and_return;
        }

        if (msg.msg_namelen != sizeof snl)
        {
            IPAL_DEBUG_PRINT ( "%s sender address length error: length %d",
                                    nl->name, msg.msg_namelen);
            ret = IPAL_RESULT_FAIL;
            goto release_and_return;
        }

        for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, status);
                    h = NLMSG_NEXT (h, status))
        {
            /* Finish of reading. */
            if (h->nlmsg_type == NLMSG_DONE)
            {
                goto release_and_return;
            }

            /* Error handling. */
            if (h->nlmsg_type == NLMSG_ERROR)
            {
                struct nlmsgerr *err = (struct nlmsgerr *) NLMSG_DATA (h);

                /* Sometimes the nlmsg_type is NLMSG_ERROR but the err->error
                 * is 0. This is a success.
                 */
                if (err->error == 0)
                {
                    IPAL_DEBUG_PRINT ("%s ACK: type=%s(%u), seq=%u, pid=%d", nl->name,
                                        IPAL_Netlink_Msg_Lookup (nlmsg_rt_str, err->msg.nlmsg_type),
                                        err->msg.nlmsg_type, err->msg.nlmsg_seq,
                                        err->msg.nlmsg_pid);

                    /* return if not a multipart message, otherwise continue */
                    if (!(h->nlmsg_flags & NLM_F_MULTI))
                    {
                        ret = IPAL_RESULT_OK;
                        goto release_and_return;
                    }
                    continue;
                }

                if (h->nlmsg_len < NLMSG_LENGTH (sizeof (struct nlmsgerr)))
                {
                    IPAL_DEBUG_PRINT ("%s error: message truncated", nl->name);
                    ret = IPAL_RESULT_FAIL;
                    goto release_and_return;
                }

                IPAL_DEBUG_PRINT ("%s error: %s, type=%s(%u), seq=%u, pid=%d",
                                    nl->name, strerror (-err->error),
                                    IPAL_Netlink_Msg_Lookup (nlmsg_rt_str, err->msg.nlmsg_type),
                                    err->msg.nlmsg_type, err->msg.nlmsg_seq,
                                    err->msg.nlmsg_pid);

                if (err->error == -EEXIST)
                    ret = IPAL_RESULT_ENTRY_EXIST;
                else
                    ret = IPAL_RESULT_FAIL;

                goto release_and_return;
            }

            /* OK we got netlink message. */
            IPAL_DEBUG_PRINT ("IPAL_Netlink_Parse_Info: %s type %s(%u), seq=%u, pid=%d",
                                nl->name, IPAL_Netlink_Msg_Lookup (nlmsg_rt_str, h->nlmsg_type),
                                h->nlmsg_type, h->nlmsg_seq, h->nlmsg_pid);

            /* Skip unsolicited messages originating from command socket. */
            if (nl != &ipal_netlink_cmd && h->nlmsg_pid == ipal_netlink_cmd.snl.nl_pid)
            {
                IPAL_DEBUG_PRINT ("IPAL_Netlink_Parse_Info: %s packet comes from %s",
                                    nl->name, ipal_netlink_cmd.name);
                continue;
            }

            switch (doit)
            {
#if 0
                case IPAL_ARP:
                {
                    arp_list_p ->arp_entry = (IPAL_IPv4_ARP_T *) malloc(sizeof (IPAL_IPv4_ARP_T) );
                    error = IPAL_Rt_Netlink_Arp_Table (&snl, h, arp_list_p ->arp_entry);
                    if (error == IPAL_RESULT_OK)
                    {
                        arp_list_p ->arp_list_next = (IPAL_IPv4_ARP_LIST_T *) malloc(sizeof (IPAL_IPv4_ARP_LIST_T) );
                        arp_list_p = arp_list_p ->arp_list_next;
                        memset (arp_list_p, 0, sizeof(IPAL_IPv4_ARP_LIST_T));
                    }
                }
                break;
#endif
                case IPAL_NEIGH_IPV4:
                {
                    IPAL_NeighborEntryIpv4_T  neigh_entry;

                    memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv4_T));
                    error = IPAL_Rt_Netlink_Neigh_Ipv4_Table(&snl, h, &neigh_entry);
                    if (error == IPAL_RESULT_OK)
                    {
                        L_SORT_LST_Set(neigh_ipv4_list_p, (void *)&neigh_entry);
                    }
                }
                break;

                case IPAL_ADDR_IPV4:
                {
                    if (NULL == (tmp_ipv4_addr_list_p = (IPAL_Ipv4AddressList_T *) L_MM_Malloc(sizeof(IPAL_Ipv4AddressList_T), L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO))))
                    {
                        ipv4_addr_list_p = *ipv4_addr_list_pp;
                        while (ipv4_addr_list_p)
                        {
                            tmp_ipv4_addr_list_p = ipv4_addr_list_p;
                            ipv4_addr_list_p = ipv4_addr_list_p->next_p;
                            L_MM_Free(tmp_ipv4_addr_list_p);
                        }
                        *ipv4_addr_list_pp = NULL;
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                    memset(tmp_ipv4_addr_list_p, 0x0, sizeof(IPAL_Ipv4AddressList_T));

                    if (IPAL_RESULT_OK == (error = IPAL_Rt_Netlink_Addr_Ipv4_Table (&snl, h, &(tmp_ipv4_addr_list_p->addr_entry))))
                    {
                        if (!ipv4_addr_list_p)
                            *ipv4_addr_list_pp = tmp_ipv4_addr_list_p;
                        else
                            ipv4_addr_list_p->next_p = tmp_ipv4_addr_list_p;

                        ipv4_addr_list_p = tmp_ipv4_addr_list_p;
                    }
                    else
                    {
                        L_MM_Free(tmp_ipv4_addr_list_p);
                    }
                }
                break;

                case IPAL_ROUTE_IPV4:
                {
                    if (NULL == (tmp_ipv4_route_list_p = (IPAL_Ipv4UcRouteList_T *) L_MM_Malloc(sizeof(IPAL_Ipv4UcRouteList_T), L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO))))
                    {
                        ipv4_route_list_p = *ipv4_route_list_pp;
                        while (ipv4_route_list_p)
                        {
                            tmp_ipv4_route_list_p = ipv4_route_list_p;
                            ipv4_route_list_p = ipv4_route_list_p->next_p;
                            L_MM_Free(tmp_ipv4_route_list_p);
                        }
                        *ipv4_route_list_pp = NULL;
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                    memset(tmp_ipv4_route_list_p, 0x0, sizeof(IPAL_Ipv4UcRouteList_T));

                    if (IPAL_RESULT_OK == (error = IPAL_Rt_Netlink_Route_Ipv4_Table (&snl, h, &(tmp_ipv4_route_list_p->route_entry))))
                    {
                        if (!ipv4_route_list_p)
                            *ipv4_route_list_pp = tmp_ipv4_route_list_p;
                        else
                            ipv4_route_list_p->next_p = tmp_ipv4_route_list_p;

                        ipv4_route_list_p = tmp_ipv4_route_list_p;
                    }
                    else
                    {
                        L_MM_Free(tmp_ipv4_route_list_p);
                    }
                }
                break;

#if (SYS_CPNT_IPV6 == TRUE)
                case IPAL_NEIGH_IPV6:
                {
                    IPAL_NeighborEntryIpv6_T  neigh_entry;

                    memset(&neigh_entry, 0x0, sizeof(IPAL_NeighborEntryIpv6_T));
                    error = IPAL_Rt_Netlink_Neigh_Ipv6_Table(&snl, h, &neigh_entry);
                    if (error == IPAL_RESULT_OK)
                    {
                        L_SORT_LST_Set(neigh_ipv6_list_p, (void *)&neigh_entry);
                    }
                }
                break;

                case IPAL_ADDR_IPV6:
                {
                    if (NULL == (tmp_ipv6_addr_list_p = (IPAL_Ipv6AddressList_T *) L_MM_Malloc(sizeof(IPAL_Ipv6AddressList_T), L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO))))
                    {
                        ipv6_addr_list_p = *ipv6_addr_list_pp;
                        while (ipv6_addr_list_p)
                        {
                            tmp_ipv6_addr_list_p = ipv6_addr_list_p;
                            ipv6_addr_list_p = ipv6_addr_list_p->next_p;
                            L_MM_Free(tmp_ipv6_addr_list_p);
                        }
                        *ipv6_addr_list_pp = NULL;
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                    memset(tmp_ipv6_addr_list_p, 0x0, sizeof(IPAL_Ipv6AddressList_T));

                    if (IPAL_RESULT_OK == (error = IPAL_Rt_Netlink_Addr_Ipv6_Table (&snl, h, &(tmp_ipv6_addr_list_p->addr_entry))))
                    {
                        if (!ipv6_addr_list_p)
                            *ipv6_addr_list_pp = tmp_ipv6_addr_list_p;
                        else
                            ipv6_addr_list_p->next_p = tmp_ipv6_addr_list_p;

                        ipv6_addr_list_p = tmp_ipv6_addr_list_p;
                    }
                    else
                    {
                        L_MM_Free(tmp_ipv6_addr_list_p);
                    }
                }
                break;

                case IPAL_ROUTE_IPV6:
                case IPAL_ROUTE_CACHE_IPV6:
                {
                    if (NULL == (tmp_ipv6_route_list_p = (IPAL_Ipv6UcRouteList_T *) L_MM_Malloc(sizeof(IPAL_Ipv6UcRouteList_T), L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO))))
                    {
                        ipv6_route_list_p = *ipv6_route_list_pp;
                        while (ipv6_route_list_p)
                        {
                            tmp_ipv6_route_list_p = ipv6_route_list_p;
                            ipv6_route_list_p = ipv6_route_list_p->next_p;
                            L_MM_Free(tmp_ipv6_route_list_p);
                        }
                        *ipv6_route_list_pp = NULL;
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                    memset(tmp_ipv6_route_list_p, 0x0, sizeof(IPAL_Ipv6UcRouteList_T));

                    if (IPAL_RESULT_OK == (error = IPAL_Rt_Netlink_Route_Ipv6_Table(&snl, h, &(tmp_ipv6_route_list_p->route_entry), (IPAL_ROUTE_CACHE_IPV6 == doit))))
                    {
                        if (!ipv6_route_list_p)
                            *ipv6_route_list_pp = tmp_ipv6_route_list_p;
                        else
                            ipv6_route_list_p->next_p = tmp_ipv6_route_list_p;

                        ipv6_route_list_p = tmp_ipv6_route_list_p;
                    }
                    else
                    {
                        L_MM_Free(tmp_ipv6_route_list_p);
                    }
                }
                break;

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
                case IPAL_IF:
                {
                    if (NULL == (tmp_if_list_p = (IPAL_IfInfoList_T *) L_MM_Malloc(sizeof(IPAL_IfInfoList_T), L_MM_USER_ID2(SYS_MODULE_IPAL, IPAL_TYPE_TRACE_ID_IPAL_RT_NETLINK_PARSE_INFO))))
                    {
                        if_list_p = *if_list_pp;
                        while (if_list_p)
                        {
                            tmp_if_list_p = if_list_p;
                            if_list_p = if_list_p->next_p;
                            L_MM_Free(tmp_if_list_p);
                        }
                        *if_list_pp = NULL;
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                    memset(tmp_if_list_p, 0x0, sizeof(IPAL_IfInfoList_T));

                    if (IPAL_RESULT_OK == (error = IPAL_Rt_Netlink_IF_Info (&snl, h, &(tmp_if_list_p->if_entry))))
                    {
                        if (!if_list_p)
                            *if_list_pp = tmp_if_list_p;
                        else
                            if_list_p->next_p = tmp_if_list_p;

                        if_list_p = tmp_if_list_p;
                    }
                    else
                    {
                        L_MM_Free(tmp_if_list_p);
                    }
                }
                break;

                case IPAL_ROUTE_LOOKUP:
                {
                    if (IPAL_RESULT_OK != (error = IPAL_Rt_Netlink_RouteIf_Info (&snl, h, route_if_p)))
                    {
                        ret = IPAL_RESULT_FAIL;
                        goto release_and_return;
                    }
                }
                break;

                case IPAL_UNSPEC:
                {
                    error = IPAL_Netlink_Talk_Filter (&snl, h);
                }
                break;

                default:
                {
                    error = IPAL_RESULT_FAIL;
                }
                break;
            }

            if (error != IPAL_RESULT_OK)
            {
                IPAL_DEBUG_PRINT ("%s filter function error", nl->name);
                ret = error;
            }
        }

        /* After error care. */
        if (msg.msg_flags & MSG_TRUNC)
        {
            IPAL_DEBUG_PRINT ("%s error: message truncated", nl->name);
            continue;
        }

        if (status)
        {
            IPAL_DEBUG_PRINT ("%s error: data remnant size %d",
                             nl->name, status);
            ret = IPAL_RESULT_FAIL;
            goto release_and_return;
        }
    }

release_and_return:
    if (buf)
        L_MM_Free(buf);

    return ret;
}

/* sendmsg() to netlink socket then recvmsg(). */
static UI32_T IPAL_Netlink_Talk (struct nlmsghdr *n, NLSOCK_T *nl)
{
    UI32_T status;
    int req_mode;
    struct sockaddr_nl snl;
    struct iovec iov = { (void*) n, n->nlmsg_len };
    struct msghdr msg = {(void*) &snl, sizeof snl, &iov, 1, NULL, 0, 0};
    int flags = 0;

    memset (&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;

    n->nlmsg_seq = ++ipal_netlink_cmd.seq;

    /* Request an acknowledgement by setting NLM_F_ACK */
    n->nlmsg_flags |= NLM_F_ACK;

    IPAL_DEBUG_PRINT ("\r\nIPAL_Netlink_Talk: %s type %s(%u), seq=%u",
       ipal_netlink_cmd.name, IPAL_Netlink_Msg_Lookup (nlmsg_rt_str, n->nlmsg_type),
       n->nlmsg_type, n->nlmsg_seq);

    /* Send message to netlink interface. */
    if (sendmsg(nl->sock, &msg, 0) < 0)
    {
        IPAL_DEBUG_PRINT ("IPAL_Netlink_Talk sendmsg() error: %s",
                            strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    /*
    *    * Change socket flags for blocking I/O.
    *    * This ensures we wait for a reply in IPAL_Netlink_Parse_Info().
    *
    */
    if ((flags = fcntl (nl->sock, F_GETFL, 0)) < 0)
        IPAL_DEBUG_PRINT ("F_GETFL error: %s", strerror (errno));

    flags &= ~O_NONBLOCK;
    if (fcntl (nl->sock, F_SETFL, flags) < 0)
        IPAL_DEBUG_PRINT ("F_SETFL error: %s", strerror (errno));

    req_mode = IPAL_UNSPEC;
    status = IPAL_Netlink_Parse_Info (nl, NULL, &req_mode);

    /* Restore socket flags for nonblocking I/O */
    flags |= O_NONBLOCK;
    if (fcntl (nl->sock, F_SETFL, flags) < 0)
        IPAL_DEBUG_PRINT ("F_SETFL error: %s", strerror (errno));

    return status;
}

static UI32_T IPAL_Addattr_l (struct nlmsghdr *n, int maxlen, int type, void *data, int alen)
{
    int len;
    struct rtattr *rta;

    len = RTA_LENGTH(alen);

    if (NLMSG_ALIGN(n->nlmsg_len) + len > maxlen)
        return IPAL_RESULT_FAIL;

    rta = (struct rtattr*) (((char*)n) + NLMSG_ALIGN (n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy (RTA_DATA(rta), data, alen);
    n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Addattr32 (struct nlmsghdr *n, int maxlen, int type, int data)
{
    int len;
    struct rtattr *rta;

    len = RTA_LENGTH(4);

    if (NLMSG_ALIGN (n->nlmsg_len) + len > maxlen)
        return IPAL_RESULT_FAIL;

    rta = (struct rtattr*) (((char*)n) + NLMSG_ALIGN (n->nlmsg_len));
    rta->rta_type = type;
    rta->rta_len = len;
    memcpy (RTA_DATA(rta), &data, 4);
    n->nlmsg_len = NLMSG_ALIGN (n->nlmsg_len) + len;

    return IPAL_RESULT_OK;
}

/* Utility function for parse rtattr. */
static void IPAL_Parse_Rtattr (struct rtattr **tb, int max, struct rtattr *rta, int len)
{
    while (RTA_OK(rta, len))
    {
        if (rta->rta_type <= max)
            tb[rta->rta_type] = rta;
        rta = RTA_NEXT(rta,len);
    }
}

#if 0
/* Arp table change via netlink interface. */
static UI32_T IPAL_Netlink_Arp (int cmd, int family, int flags, const IPAL_IPv4_ARP_T *arp)
{
    int bytelen;
    struct
    {
        struct nlmsghdr n;
        struct ndmsg ndm;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof req);

    bytelen = (family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof(struct ndmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | flags;
    req.n.nlmsg_type = cmd;

    req.ndm.ndm_family = family;
    req.ndm.ndm_ifindex = arp->ifindex;
    req.ndm.ndm_state = arp->state;

    IPAL_Addattr_l (&req.n, sizeof req, NDA_DST, (void *) &(arp->ip_addr), bytelen);
    IPAL_Addattr_l (&req.n, sizeof req, NDA_LLADDR, (void *) arp->phy_address, arp->phy_address_len);

    /* Talk to netlink socket. */
    return IPAL_Netlink_Talk (&req.n, &ipal_netlink_cmd);
}
#endif

/* Neighbor table change via netlink interface. */
static UI32_T IPAL_Netlink_Neigh (int cmd, int family, int flags, const IPAL_NeighborEntry_T *neigh)
{
    int bytelen;
    struct
    {
        struct nlmsghdr n;
        struct ndmsg ndm;
        char buf[1024];
    } req;

    memset(&req, 0, sizeof req);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof(struct ndmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | flags;
    req.n.nlmsg_type = cmd;

    req.ndm.ndm_family = family;
    req.ndm.ndm_ifindex = neigh->ifindex;

    if (family == AF_BRIDGE)
    {
        req.ndm.ndm_flags |= NTF_MASTER;

        req.ndm.ndm_state = NUD_REACHABLE;

        /* set fdb entry static */
        if (neigh->state == NUD_PERMANENT)
            req.ndm.ndm_state |= NUD_NOARP;
    }
    else
    {
        bytelen = (family == AF_INET ? 4 : 16);
        req.ndm.ndm_state = neigh->state;
        IPAL_Addattr_l (&req.n, sizeof req, NDA_DST, (void *) &(neigh->ip_addr.addr), bytelen);
    }

    IPAL_Addattr_l (&req.n, sizeof req, NDA_LLADDR, (void *) neigh->phy_address, neigh->phy_address_len);

    /* Talk to netlink socket. */
    return IPAL_Netlink_Talk (&req.n, &ipal_netlink_cmd);
}


/* Routing table change via netlink interface. */
static UI32_T IPAL_Netlink_Route (int cmd, int family, int flags, int table, const L_INET_AddrIp_T *p,
                                    const void *nhs)
{
    int bytelen;
    int discard;
    char zero_addr[SYS_ADPT_IPV6_ADDR_LEN] = {0};
    IPAL_Ipv4UcNextHop_T *nhs4 = (IPAL_Ipv4UcNextHop_T *)nhs;
#if (SYS_CPNT_IPV6 == TRUE)
    IPAL_Ipv6UcNextHop_T *nhs6 = (IPAL_Ipv6UcNextHop_T *)nhs;
#endif

    struct
    {
        struct nlmsghdr n;
        struct rtmsg r;
        char buf[1024];
    } req;

    memset (&req, 0, sizeof req);

    bytelen = (family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof (struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | flags;
    req.n.nlmsg_type = cmd;

    req.r.rtm_family = family;
    req.r.rtm_table = table;
    req.r.rtm_dst_len = p->preflen;

    if (family == AF_INET && nhs4->flags & IPAL_RIB_FLAG_BLACKHOLE)
        discard = 1;
#if (SYS_CPNT_IPV6 == TRUE)
    else if (family == AF_INET6 && nhs6->flags & IPAL_RIB_FLAG_BLACKHOLE)
        discard = 1;
#endif
    else
        discard = 0;

    if (cmd == RTM_NEWROUTE)
    {
        req.r.rtm_protocol = RTPROT_STATIC; // or RTPROT_BOOT ?
        req.r.rtm_scope = RT_SCOPE_UNIVERSE;

        if (discard)
            req.r.rtm_type = RTN_BLACKHOLE;
        else
            req.r.rtm_type = RTN_UNICAST;
    }

    IPAL_Addattr_l (&req.n, sizeof req, RTA_DST, (void *)(p->addr), bytelen);
    if (discard == 0)
    {
        if (family == AF_INET)
        {
            if (nhs4->nexthopIP.s_addr > 0)
                IPAL_Addattr_l (&req.n, sizeof req, RTA_GATEWAY, (void *) &(nhs4->nexthopIP.s_addr), bytelen);
            if (nhs4->egressIfindex > 0)
                IPAL_Addattr32 (&req.n, sizeof req, RTA_OIF, nhs4->egressIfindex);
        }
#if (SYS_CPNT_IPV6 == TRUE)
        else if (family == AF_INET6)
        {
            if (0 != memcmp(nhs6->nexthopIP.s6_addr, zero_addr, SYS_ADPT_IPV6_ADDR_LEN))
                IPAL_Addattr_l (&req.n, sizeof req, RTA_GATEWAY, (void *) &(nhs6->nexthopIP.s6_addr), bytelen);
            if (nhs6->egressIfindex > 0)
                IPAL_Addattr32 (&req.n, sizeof req, RTA_OIF, nhs6->egressIfindex);
        }
#endif
    }

    /* Talk to netlink socket. */
    return IPAL_Netlink_Talk (&req.n, &ipal_netlink_cmd);
}

/* Interface address modification. */
static UI32_T IPAL_Netlink_Address (int cmd, int family, int flags, UI32_T ifindex, const L_INET_AddrIp_T *p)
{
    int bytelen;

    struct pal_in4_addr netmask;
    struct
    {
        struct nlmsghdr n;
        struct ifaddrmsg ifa;
        char buf[1024];
    } req;

    memset (&req, 0, sizeof req);

    bytelen = (family == AF_INET ? 4 : 16);

    req.n.nlmsg_len = NLMSG_LENGTH (sizeof(struct ifaddrmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | flags;
    req.n.nlmsg_type = cmd;

    req.ifa.ifa_family = family;
    req.ifa.ifa_index = ifindex;
    req.ifa.ifa_prefixlen = p->preflen;

    IPAL_Addattr_l (&req.n, sizeof req, IFA_LOCAL, (void *) &(p->addr), bytelen);

    if(family == AF_INET && cmd == RTM_NEWADDR)
    {
        L_PREFIX_MaskLen2IPv4(p->preflen, &netmask);
        IPAL_Addattr_l (&req.n, sizeof req, IFA_BROADCAST, (void *) &(netmask.s_addr), bytelen);
    }

    return IPAL_Netlink_Talk (&req.n, &ipal_netlink_cmd);
}

/* Interface address alias modification. */
static UI32_T IPAL_Netlink_AddressAlias (int cmd, int family, int flags, UI32_T ifindex, const L_INET_AddrIp_T *p, UI32_T alias_id)
{
	int bytelen;

	struct pal_in4_addr netmask;
	struct
	{
		struct nlmsghdr n;
		struct ifaddrmsg ifa;
		char buf[1024];
	} req;

	memset (&req, 0, sizeof req);

	bytelen = (family == AF_INET ? 4 : 16);

	req.n.nlmsg_len = NLMSG_LENGTH (sizeof(struct ifaddrmsg));
	req.n.nlmsg_flags = NLM_F_REQUEST | flags;
	req.n.nlmsg_type = cmd;

	req.ifa.ifa_family = family;
	req.ifa.ifa_index = ifindex;
	req.ifa.ifa_prefixlen = p->preflen;

	IPAL_Addattr_l (&req.n, sizeof req, IFA_LOCAL, (void *) &(p->addr), bytelen);

    if(family == AF_INET && cmd == RTM_NEWADDR)
	{
	    L_PREFIX_MaskLen2IPv4(p->preflen, &netmask);
		IPAL_Addattr_l (&req.n, sizeof req, IFA_BROADCAST, (void *) &(netmask.s_addr), bytelen);
	}

    if (IS_VLAN_IFINDEX_VAILD(ifindex))
    {
        char ifname[21];
        UI32_T vid;
        VLAN_IFINDEX_CONVERTTO_VID(ifindex,vid);
        sprintf(ifname,"VLAN%lu:%lu",(unsigned long)vid,(unsigned long)alias_id);
        IPAL_Addattr_l (&req.n, sizeof req, IFA_LABEL, (void *) ifname, strlen(ifname)+1);
    }
	return IPAL_Netlink_Talk (&req.n, &ipal_netlink_cmd);
}


/* Make socket for Linux netlink interface. */
static UI32_T IPAL_Netlink_Socket (NLSOCK_T *nl, UI32_T groups, UI8_T non_block)
{
    int ret = 0;
    int sock = 0;
    struct sockaddr_nl snl;
    UI32_T namelen;

    sock = socket (AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (sock < 0)
    {
        IPAL_DEBUG_PRINT ("Can't open %s socket: %s", nl->name,
                            strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    /* Enlarge socket's receive buffer size, default is 163840(160K) bytes
     */
    {
        int result;
        int buffsize = 524288; /* 512KB */

        result = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffsize, sizeof(buffsize));
        if (result != 0)
            IPAL_DEBUG_PRINT(" %s:%d setsockopt failed!", __FUNCTION__, __LINE__);
    }

    if (non_block)
    {
        ret = fcntl (sock, F_SETFL, O_NONBLOCK);
        if (ret < 0)
        {
            IPAL_DEBUG_PRINT ("Can't set %s socket flags: %s", nl->name,
                                strerror (errno));
            goto errout;
        }
    }

    memset (&snl, 0, sizeof snl);
    snl.nl_family = AF_NETLINK;
    snl.nl_groups = groups;

    /* Bind the socket to the netlink structure for anything. */
    ret = bind (sock, (struct sockaddr *) &snl, sizeof snl);
    if (ret < 0)
    {
        IPAL_DEBUG_PRINT ("Can't bind %s socket to group 0x%x: %s", nl->name,
                            snl.nl_groups, strerror (errno));
        goto errout;
    }

    /* multiple netlink sockets will have different nl_pid */
    namelen = sizeof(snl);
    ret = getsockname (sock, (struct sockaddr *) &snl, (socklen_t *)&namelen);
    if (ret < 0 || namelen != sizeof(snl))
    {
        IPAL_DEBUG_PRINT ("Can't get %s socket name: %s", nl->name,
                            strerror (errno));
        goto errout;
    }

    if (nl->sock > 0)
        close(nl->sock);

    nl->snl = snl;
    nl->sock = sock;

    return IPAL_RESULT_OK;

errout:
    close (sock);

    return IPAL_RESULT_FAIL;
}

/* Get type specified information from netlink. */
static UI32_T IPAL_Netlink_Request (int family, int type, NLSOCK_T *nl)
{
    int ret;
    struct sockaddr_nl snl;

    struct
    {
        struct nlmsghdr nlh;
        struct rtgenmsg g;
    } req;

    /* Check netlink socket. */
    if (nl->sock < 0)
    {
        IPAL_DEBUG_PRINT ("%s socket isn't active.", nl->name);
        return IPAL_RESULT_FAIL;
    }
    memset (&snl, 0, sizeof(snl));
    snl.nl_family = AF_NETLINK;

    req.nlh.nlmsg_len = sizeof req;
    req.nlh.nlmsg_type = type;
    req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
    req.nlh.nlmsg_pid = 0;
    req.nlh.nlmsg_seq = ++nl->seq;
    req.g.rtgen_family = family;

    ret = sendto (nl->sock, (void*) &req, sizeof(req), 0,
                    (struct sockaddr*) &snl, sizeof(snl) );
    if (ret < 0)
    {
        IPAL_DEBUG_PRINT ("%s sendto failed: %s", nl->name, strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}

/* Get route for a specified destination address */
static UI32_T IPAL_Netlink_RequestRoute (int family, L_INET_AddrIp_T *ipaddr_p, NLSOCK_T *nl)
{
    int ret;
    struct sockaddr_nl snl;
    UI32_T ifindex;

	struct {
		struct nlmsghdr n;
		struct rtmsg    r;
		char            buf[1024];
	} req;

    /* Check netlink socket. */
    if (nl->sock < 0)
    {
        IPAL_DEBUG_PRINT ("%s socket isn't active.", nl->name);
        return IPAL_RESULT_FAIL;
    }

    memset (&snl, 0, sizeof(snl));
    snl.nl_family = AF_NETLINK;

    memset(&req, 0, sizeof(req));
    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    req.n.nlmsg_flags = NLM_F_REQUEST;
    req.n.nlmsg_type = RTM_GETROUTE;
    req.n.nlmsg_pid = 0;
    req.n.nlmsg_seq = ++nl->seq;
    req.r.rtm_family = family;
    req.r.rtm_table = 0;
    req.r.rtm_protocol = 0;
    req.r.rtm_scope = 0;
    req.r.rtm_type = 0;
    req.r.rtm_src_len = 0;
    req.r.rtm_dst_len = 0;
    req.r.rtm_tos = 0;

    /* Fill destination ip address
     */
    IPAL_Addattr_l(&req.n, sizeof(req), RTA_DST, ipaddr_p->addr, ipaddr_p->addrlen);
    req.r.rtm_dst_len = ipaddr_p->addrlen * 8;

    /* Fill output interface if it is link-local address
     */
    if (L_INET_IS_LINK_LOCAL_ADDR_TYPE(ipaddr_p->type) && ipaddr_p->zoneid != 0)
    {
        ifindex = ipaddr_p->zoneid + SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;
        IPAL_Addattr_l(&req.n, sizeof(req), RTA_OIF, &ifindex, sizeof(ifindex));
    }

    ret = sendto(nl->sock, (void*) &req, sizeof(req), 0,
                    (struct sockaddr*) &snl, sizeof(snl) );
    if (ret < 0)
    {
        IPAL_DEBUG_PRINT ("%s sendto failed: %s", nl->name, strerror (errno));
        return IPAL_RESULT_FAIL;
    }

    return IPAL_RESULT_OK;
}


/*
 *  IPAL ROUTE NETLINK Arp Scan
 */
#if 0
static UI32_T IPAL_Rt_Netlink_Arp_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_IPv4_ARP_T *arp_entry)
{
    int len;
    struct ndmsg *ndm;
    struct rtattr *tb [RTA_MAX + 1];

    ndm = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWNEIGH)
        return IPAL_RESULT_FAIL;

    if ( !(ndm->ndm_state & NUD_VALID))
        return IPAL_RESULT_FAIL;

    if (ndm->ndm_flags & NTF_PROXY)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ndm));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, NDA_RTA (ndm), len);

    arp_entry->ifindex = ndm->ndm_ifindex;
    arp_entry->state = ndm->ndm_state;

    if (tb[NDA_DST])
        arp_entry->ip_addr = *(UI32_T *) RTA_DATA (tb[NDA_DST]);

    if (tb[NDA_LLADDR])
    {
        memcpy (arp_entry->phy_address, RTA_DATA (tb[NDA_LLADDR]), SYS_ADPT_MAC_ADDR_LEN);
    }

    return IPAL_RESULT_OK;
}
#endif

/*
 *  IPAL ROUTE NETLINK IPV4 neighbor Scan
 */
static UI32_T IPAL_Rt_Netlink_Neigh_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_NeighborEntryIpv4_T *neigh_entry)
{
    int len;
    struct ndmsg *ndm;
    struct rtattr *tb [RTA_MAX + 1];
    struct nda_cacheinfo *ci = NULL;


    ndm = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWNEIGH)
        return IPAL_RESULT_FAIL;

    if ( !(ndm->ndm_state & NUD_VALID))
        return IPAL_RESULT_FAIL;

    if (ndm->ndm_flags & NTF_PROXY)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ndm));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, NDA_RTA (ndm), len);

    neigh_entry->ifindex = ndm->ndm_ifindex;
    neigh_entry->state = ndm->ndm_state;

    if (tb[NDA_DST])
    {
        memcpy(neigh_entry->ip_addr,(UI8_T *) RTA_DATA (tb[NDA_DST]), SYS_ADPT_IPV4_ADDR_LEN);
    }

    if (tb[NDA_LLADDR])
    {
        neigh_entry->phy_address_len = SYS_ADPT_MAC_ADDR_LEN;
        memcpy (neigh_entry->phy_address, RTA_DATA (tb[NDA_LLADDR]), SYS_ADPT_MAC_ADDR_LEN);
    }

    if (tb[NDA_CACHEINFO])
    {
        ci = RTA_DATA(tb[NDA_CACHEINFO]);
        neigh_entry->last_update = ci->ndm_updated;
    }

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Rt_Netlink_Addr_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv4AddressEntry_T *addr_entry)
{
    int len;
    struct ifaddrmsg *ifa;
    struct rtattr *rta_tb[RTA_MAX + 1];

    ifa = NLMSG_DATA (h);

    if (ifa->ifa_family != AF_INET)
        return IPAL_RESULT_FAIL;

    if (h->nlmsg_type != RTM_NEWADDR)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ifa));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset(addr_entry, 0x0, sizeof(IPAL_Ipv4AddressEntry_T));

    memset(rta_tb, 0x0, sizeof(rta_tb));
    IPAL_Parse_Rtattr(rta_tb, IFA_MAX, IFA_RTA (ifa), len);

    addr_entry->ifindex     = ifa->ifa_index;
    addr_entry->ifa_flags   = ifa->ifa_flags;
    addr_entry->prefixlen   = ifa->ifa_prefixlen;
    addr_entry->scope       = ifa->ifa_scope;

    if (rta_tb[IFA_ADDRESS] == NULL)
        rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];

    if (rta_tb[IFA_ADDRESS])
        memcpy(addr_entry->addr, RTA_DATA(rta_tb[IFA_ADDRESS]), SYS_ADPT_IPV4_ADDR_LEN);
    else memset(addr_entry->addr, 0x0, SYS_ADPT_IPV4_ADDR_LEN);

    return IPAL_RESULT_OK;
}

/*
 *  IPAL ROUTE NETLINK interface  Scan
 */
static UI32_T IPAL_Rt_Netlink_IF_Info (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_IfInfo_T *if_info_entry)
{
    int len;
    struct ifinfomsg *ifm;
    struct rtattr *tb [RTA_MAX + 1];

    ifm = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWLINK)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ifm));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, IFLA_RTA (ifm), len);

    if_info_entry->ifindex = ifm->ifi_index;

    if (tb[IFLA_ADDRESS])
    {
        memcpy (if_info_entry->hw_addr, RTA_DATA (tb[IFLA_ADDRESS]), SYS_ADPT_MAC_ADDR_LEN);
    }

    if(tb[IFLA_MTU])
    {
        if_info_entry->mtu = *(UI32_T*)RTA_DATA(tb[IFLA_MTU]);
    }

    if (tb[IFLA_IFNAME])
    {
        memcpy (if_info_entry->ifname, RTA_DATA (tb[IFLA_IFNAME]), IFNAMSIZ);
    }

    if (tb[IFLA_PROTINFO])
    {
        void *rtadata;
        struct rtattr *rta, *rta1;
        //int rtasize,
        int rtasize1;

        rta = tb[IFLA_PROTINFO];
        rtadata = RTA_DATA(rta);

        rtasize1 = rta->rta_len;
        for (rta1 = (struct rtattr *) rtadata; RTA_OK(rta1, rtasize1);
             rta1 = RTA_NEXT(rta1, rtasize1))
        {
            void *rtadata1 = RTA_DATA(rta1);

            switch (rta1->rta_type)
            {
                case IFLA_INET6_CACHEINFO:
                    break;
                case IFLA_INET6_FLAGS:
                    if_info_entry->inet6_flags = *((u_int32_t *) rtadata1);
                    break;
                default:
                    break;
            }
        }
    }

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Rt_Netlink_RouteIf_Info (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_RouteIfInfo_T *route_if_p)
{
    int len;
    struct rtmsg *r;
    struct rtattr *tb [RTA_MAX + 1];

    memset(route_if_p, 0, sizeof(IPAL_RouteIfInfo_T));

    r = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWROUTE)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*r));
    if (len < 0)
        return IPAL_RESULT_FAIL;
    if (r->rtm_type != RTN_UNICAST)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, RTM_RTA (r), len);

    if (tb[RTA_OIF])
    {
        route_if_p->ifindex = *(UI32_T *) RTA_DATA (tb[RTA_OIF]);
    }
    if (tb[RTA_GATEWAY])
    {
        memset(&route_if_p->nexthop, 0, sizeof(route_if_p->nexthop));
        memcpy(route_if_p->nexthop.addr, (UI8_T*)RTA_DATA(tb[RTA_GATEWAY]), RTA_PAYLOAD(tb[RTA_GATEWAY]));
        if (r->rtm_family == AF_INET)
        {
            if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(route_if_p->nexthop.addr))
                route_if_p->nexthop.type = L_INET_ADDR_TYPE_IPV4Z;
            else
                route_if_p->nexthop.type = L_INET_ADDR_TYPE_IPV4;
            route_if_p->nexthop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        }
        else
        {
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(route_if_p->nexthop.addr))
                route_if_p->nexthop.type = L_INET_ADDR_TYPE_IPV6Z;
            else
                route_if_p->nexthop.type = L_INET_ADDR_TYPE_IPV6;
            route_if_p->nexthop.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        }
    }
    if (tb[RTA_PREFSRC])
    {
        memset(&route_if_p->src, 0, sizeof(route_if_p->src));
        memcpy(route_if_p->src.addr, (UI8_T*)RTA_DATA(tb[RTA_PREFSRC]), RTA_PAYLOAD(tb[RTA_PREFSRC]));
        if (r->rtm_family == AF_INET)
        {
            if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(route_if_p->src.addr))
                route_if_p->src.type = L_INET_ADDR_TYPE_IPV4Z;
            else
                route_if_p->src.type = L_INET_ADDR_TYPE_IPV4;
            route_if_p->src.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        }
        else
        {
            if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(route_if_p->src.addr))
                route_if_p->src.type = L_INET_ADDR_TYPE_IPV6Z;
            else
                route_if_p->src.type = L_INET_ADDR_TYPE_IPV6;
            route_if_p->src.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
        }
    }

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Rt_Netlink_Route_Ipv4_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv4UcRouteEntry_T *route_entry)
{
    int len;
    struct rtmsg *r;
    struct rtattr *tb [RTA_MAX + 1];

    r = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWROUTE)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*r));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    if (r->rtm_type != RTN_UNICAST &&
        r->rtm_type != RTN_BLACKHOLE)
        return IPAL_RESULT_FAIL;

    if (r->rtm_flags & RTM_F_CLONED)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, RTM_RTA (r), len);

    route_entry->table_id = r->rtm_table;
    route_entry->dst_p.prefixlen = r->rtm_dst_len;

    if(tb[RTA_DST])
    {
        route_entry->dst_p.u.prefix4.s_addr = *(UI32_T*)RTA_DATA(tb[RTA_DST]);
    }
    if (tb[RTA_OIF])
    {
        route_entry->nh.u.ipv4_nh.egressIfindex = *(UI32_T *) RTA_DATA (tb[RTA_OIF]);
    }
    if (tb[RTA_GATEWAY])
    {
        route_entry->nh.u.ipv4_nh.nexthopIP.s_addr = *(UI32_T *)RTA_DATA (tb[RTA_GATEWAY]);
    }

    if (tb[RTA_METRICS])
    {
        int mtu = 0;
        if(IPAL_Netlink_Metrics(RTA_DATA(tb[RTA_METRICS]),
           RTA_PAYLOAD(tb[RTA_METRICS]), &mtu) == IPAL_RESULT_OK)
        {
            route_entry->nh.u.ipv4_nh.mtu = mtu;
        }
    }

    if (tb[RTA_CACHEINFO])
    {
        struct rta_cacheinfo *cacheinfo_p;

        cacheinfo_p = (struct rta_cacheinfo *)RTA_DATA(tb[RTA_CACHEINFO]);
        route_entry->nh.u.ipv4_nh.expires = cacheinfo_p->rta_expires;
    }

    return IPAL_RESULT_OK;
}


/*
 *  IPAL ROUTE NETLINK Fetch All IPv4 Arp
 */
#if 0
UI32_T IPAL_Rt_Netlink_FetchAllIPv4Arp (IPAL_IPv4_ARP_LIST_T *arp_list)
{
    int req_mode = IPAL_ARP;

    IPAL_Netlink_Request (AF_INET, RTM_GETNEIGH, &ipal_netlink_cmd);

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, arp_list, &req_mode);
}
#endif

/*
 *  IPAL NETLINK Fetch All IPv4 Neighbor
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Neighbor (L_SORT_LST_List_T *neigh_list_p)
{
    int req_mode = IPAL_NEIGH_IPV4;

    if (0 != IPAL_Netlink_Request (AF_INET, RTM_GETNEIGH, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, neigh_list_p, &req_mode);
}


/*
 * Fetch all ipv4 interface.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Interface (IPAL_IfInfoList_T **if_list_pp)
{
    int req_mode = IPAL_IF;

    if (0 != IPAL_Netlink_Request (AF_INET, RTM_GETLINK, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, if_list_pp, &req_mode);
}


/*
 *Fetch all ipv4 address.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Addr (IPAL_Ipv4AddressList_T **addr_list_pp)
{
    int req_mode = IPAL_ADDR_IPV4;

    if (0 != IPAL_Netlink_Request (AF_INET, RTM_GETADDR, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, addr_list_pp, &req_mode);
}

/*
 *Fetch all route.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv4Route (IPAL_Ipv4UcRouteList_T **route_list_pp)
{
    int req_mode = IPAL_ROUTE_IPV4;

    if (IPAL_RESULT_OK != IPAL_Netlink_Request (AF_INET, RTM_GETROUTE, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, route_list_pp, &req_mode);
}

UI32_T IPAL_Rt_Netlink_RouteLookup(L_INET_AddrIp_T *dest_ip_p, L_INET_AddrIp_T *src_ip_p,
                                   L_INET_AddrIp_T *nexthop_ip_p, UI32_T *nexthop_if_p)
{
    int req_mode = IPAL_ROUTE_LOOKUP;
    IPAL_RouteIfInfo_T best_route;
    UI32_T ret;
    int family;

    family = L_INET_IS_IPV4_ADDR_TYPE(dest_ip_p->type)? AF_INET: AF_INET6;
    if (IPAL_RESULT_OK != IPAL_Netlink_RequestRoute (family, dest_ip_p, &ipal_netlink_cmd))
    {
        IPAL_DEBUG_PRINT("IPAL_Netlink_RequestRoute() fail\n");
        return IPAL_RESULT_FAIL;
    }

    if (IPAL_RESULT_OK == (ret = IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, &best_route, &req_mode)))
    {
        memcpy(src_ip_p, &best_route.src, sizeof(L_INET_AddrIp_T));
        if (best_route.nexthop.type != L_INET_ADDR_TYPE_UNKNOWN)
            memcpy(nexthop_ip_p, &best_route.nexthop, sizeof(L_INET_AddrIp_T));
        else
            memcpy(nexthop_ip_p, dest_ip_p, sizeof(L_INET_AddrIp_T)); /* if no gateway, means IP is in local subnet */
        *nexthop_if_p = best_route.ifindex;
    }
    return ret;
}


#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  IPAL ROUTE NETLINK IPV6 neighbor Scan
 */
static UI32_T IPAL_Rt_Netlink_Neigh_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_NeighborEntryIpv6_T *neigh_entry)
{
    int len;
    struct ndmsg *ndm;
    struct rtattr *tb [RTA_MAX + 1];
    struct nda_cacheinfo *ci = NULL;
    static int user_hz = 0;

    if (!user_hz)
        user_hz = sysconf(_SC_CLK_TCK);

    ndm = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWNEIGH)
        return IPAL_RESULT_FAIL;

    if ( !(ndm->ndm_state & NUD_VALID))
        return IPAL_RESULT_FAIL;

    if (ndm->ndm_flags & NTF_PROXY)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ndm));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, NDA_RTA (ndm), len);

    neigh_entry->ifindex = ndm->ndm_ifindex;
    neigh_entry->state = ndm->ndm_state;

    if (tb[NDA_DST])
    {
        memcpy(neigh_entry->ip_addr,(UI8_T *) RTA_DATA (tb[NDA_DST]), SYS_ADPT_IPV6_ADDR_LEN);
    }

    if (tb[NDA_LLADDR])
    {
        neigh_entry->phy_address_len = SYS_ADPT_MAC_ADDR_LEN;
        memcpy (neigh_entry->phy_address, RTA_DATA (tb[NDA_LLADDR]), SYS_ADPT_MAC_ADDR_LEN);
    }

    if (tb[NDA_CACHEINFO])
    {
        ci = RTA_DATA(tb[NDA_CACHEINFO]);
        neigh_entry->last_update = ci->ndm_updated / user_hz;
    }

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Rt_Netlink_Addr_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv6AddressEntry_T *addr_entry)
{
    int len;
    struct ifaddrmsg *ifa;
    struct rtattr *rta_tb[RTA_MAX + 1];

    ifa = NLMSG_DATA (h);

    if (ifa->ifa_family != AF_INET6)
        return IPAL_RESULT_FAIL;

    if (h->nlmsg_type != RTM_NEWADDR)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ifa));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    memset(addr_entry, 0x0, sizeof(IPAL_Ipv6AddressEntry_T));

    memset(rta_tb, 0x0, sizeof(rta_tb));
    IPAL_Parse_Rtattr(rta_tb, IFA_MAX, IFA_RTA (ifa), len);

    addr_entry->ifindex     = ifa->ifa_index;
    addr_entry->ifa_flags   = ifa->ifa_flags;
    addr_entry->prefixlen   = ifa->ifa_prefixlen;
    addr_entry->scope       = ifa->ifa_scope;

    if (rta_tb[IFA_ADDRESS] == NULL)
        rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];

    if (rta_tb[IFA_ADDRESS])
        memcpy(addr_entry->addr, RTA_DATA(rta_tb[IFA_ADDRESS]), SYS_ADPT_IPV6_ADDR_LEN);
    else memset(addr_entry->addr, 0x0, SYS_ADPT_IPV6_ADDR_LEN);

    if (rta_tb[IFA_CACHEINFO])
    {
        struct ifa_cacheinfo *ci = RTA_DATA(rta_tb[IFA_CACHEINFO]);

        addr_entry->valid_lft       = ci->ifa_valid;
        addr_entry->preferred_lft   = ci->ifa_prefered;
    }

    return IPAL_RESULT_OK;
}

static UI32_T IPAL_Rt_Netlink_Route_Ipv6_Table (struct sockaddr_nl *snl, struct nlmsghdr *h, IPAL_Ipv6UcRouteEntry_T *route_entry, BOOL_T is_cache)
{
    int len;
    struct rtmsg *r;
    struct rtattr *tb [RTA_MAX + 1];

    r = NLMSG_DATA (h);
    if (h->nlmsg_type != RTM_NEWROUTE)
        return IPAL_RESULT_FAIL;

    len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*r));
    if (len < 0)
        return IPAL_RESULT_FAIL;

    if (r->rtm_type != RTN_UNICAST &&
        r->rtm_type != RTN_BLACKHOLE)
        return IPAL_RESULT_FAIL;

    /* RTM_F_CLONED on rtm_flags means RTF_CACHE on rt6i_flags
     * See rt6_fill_node() in net/ipv6/route.c of Linux kernel
     */
    if (((is_cache)&&(!(r->rtm_flags & RTM_F_CLONED))) ||
        ((!is_cache)&&(r->rtm_flags & RTM_F_CLONED)) )
        return IPAL_RESULT_FAIL;

    memset (tb, 0, sizeof(tb));
    IPAL_Parse_Rtattr (tb, RTA_MAX, RTM_RTA (r), len);

    route_entry->table_id = r->rtm_table;
    route_entry->dst_p.prefixlen = r->rtm_dst_len;

    if(tb[RTA_DST])
    {
        memcpy(route_entry->dst_p.u.prefix6.s6_addr, RTA_DATA(tb[RTA_DST]),sizeof(route_entry->dst_p.u.prefix6.s6_addr));
    }
    if (tb[RTA_OIF])
    {
        route_entry->nh.u.ipv6_nh.egressIfindex = *(UI32_T *) RTA_DATA (tb[RTA_OIF]);
    }
    if (tb[RTA_GATEWAY])
    {
        memcpy(route_entry->nh.u.ipv6_nh.nexthopIP.s6_addr, RTA_DATA (tb[RTA_GATEWAY]),sizeof(route_entry->nh.u.ipv6_nh.nexthopIP.s6_addr));
    }

    if (tb[RTA_METRICS])
    {
        int mtu = 0;
        if(IPAL_Netlink_Metrics(RTA_DATA(tb[RTA_METRICS]),
           RTA_PAYLOAD(tb[RTA_METRICS]), &mtu) == IPAL_RESULT_OK)
        {
            route_entry->nh.u.ipv6_nh.mtu = mtu;
        }
    }

    if (tb[RTA_CACHEINFO])
    {
        struct rta_cacheinfo *cacheinfo_p;

        cacheinfo_p = (struct rta_cacheinfo *)RTA_DATA(tb[RTA_CACHEINFO]);
        route_entry->nh.u.ipv6_nh.expires = cacheinfo_p->rta_expires;
    }

    return IPAL_RESULT_OK;
}

/*
 *  IPAL NETLINK Fetch All IPv6 Neighbor
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Neighbor (L_SORT_LST_List_T *neigh_list_p)
{
    int req_mode = IPAL_NEIGH_IPV6;

    if (0 != IPAL_Netlink_Request (AF_INET6, RTM_GETNEIGH, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, neigh_list_p, &req_mode);
}

/*
 * Fetch all ipv6 interface.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Interface (IPAL_IfInfoList_T **if_list_pp)
{
    int req_mode = IPAL_IF;

    if (0 != IPAL_Netlink_Request (AF_INET6, RTM_GETLINK, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, if_list_pp, &req_mode);
}

/*
 *Fetch all ipv6 address.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Addr (IPAL_Ipv6AddressList_T **addr_list_pp)
{
    int req_mode = IPAL_ADDR_IPV6;

    if (0 != IPAL_Netlink_Request (AF_INET6, RTM_GETADDR, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, addr_list_pp, &req_mode);
}

/*
 *Fetch all IPv6 route.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6Route (IPAL_Ipv6UcRouteList_T **route_list_pp)
{
    int req_mode = IPAL_ROUTE_IPV6;

    if (0 != IPAL_Netlink_Request (AF_INET6, RTM_GETROUTE, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, route_list_pp, &req_mode);
}

/*
 * Fetch all IPv6 route cache.
 */
UI32_T IPAL_Rt_Netlink_FetchAllIpv6RouteCache (IPAL_Ipv6UcRouteList_T **route_list_pp)
{
    int req_mode = IPAL_ROUTE_CACHE_IPV6;

    if (0 != IPAL_Netlink_Request (AF_INET6, RTM_GETROUTE, &ipal_netlink_cmd))
        return IPAL_RESULT_FAIL;

    return IPAL_Netlink_Parse_Info (&ipal_netlink_cmd, route_list_pp, &req_mode);
}

#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if 0
/*
 * IPAL ROUTE NETLINK Add IPv4 Arp
 */
UI32_T IPAL_Rt_Netlink_AddIPv4Arp (const IPAL_IPv4_ARP_T *arp)
{
    /*If arp entry exist,we can replace or change it yet,so change NLM_F_CREATE|NLM_F_EXCL to NLM_F_CREATE|NLM_F_REPLACE--xiongyu 20081031*/
    return IPAL_Netlink_Arp (RTM_NEWNEIGH, AF_INET, NLM_F_CREATE|NLM_F_REPLACE, arp);
}

/*
 * IPAL ROUTE NETLINK Delete IPv4 Arp
 */
UI32_T IPAL_Rt_Netlink_DeleteIPv4Arp (const IPAL_IPv4_ARP_T *arp)
{
    return IPAL_Netlink_Arp (RTM_DELNEIGH, AF_INET, 0, arp);
}
#endif

/*
 *  IPAL ROUTE NETLINK Add IPv4/IPv6 neighbor and bridge fdb
 */
UI32_T IPAL_Rt_Netlink_AddNeighbor(const IPAL_NeighborEntry_T *neigh, BOOL_T replace_if_exist)
{
    int flags;

    if (replace_if_exist)
        flags = NLM_F_CREATE|NLM_F_REPLACE;
    else
        flags = NLM_F_CREATE|NLM_F_EXCL;

    switch(neigh->ip_addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            /* If arp entry exist,we can replace or change it yet,
             * so change NLM_F_CREATE|NLM_F_EXCL to NLM_F_CREATE|NLM_F_REPLACE--xiongyu 20081031
             */
            return IPAL_Netlink_Neigh (RTM_NEWNEIGH, AF_INET, flags, neigh);

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            return IPAL_Netlink_Neigh (RTM_NEWNEIGH, AF_INET6, flags, neigh);

        case L_INET_ADDR_TYPE_UNKNOWN:
            /* used for bridge fdb
             */
            return IPAL_Netlink_Neigh (RTM_NEWNEIGH, AF_BRIDGE, flags, neigh);

        default:
            return IPAL_RESULT_FAIL;
    }
}

/*
 *  IPAL ROUTE NETLINK Delete IPv4/IPv6 neighbor and bridge fdb
 */
UI32_T IPAL_Rt_Netlink_DeleteNeighbor(const IPAL_NeighborEntry_T *neigh)
{
    switch(neigh->ip_addr.type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            return IPAL_Netlink_Neigh (RTM_DELNEIGH, AF_INET, 0, neigh);

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            return IPAL_Netlink_Neigh (RTM_DELNEIGH, AF_INET6, 0, neigh);

        case L_INET_ADDR_TYPE_UNKNOWN:
            /* used for bridge fdb
             */
            return IPAL_Netlink_Neigh (RTM_DELNEIGH, AF_BRIDGE, 0, neigh);

        default:
            return IPAL_RESULT_FAIL;
    }
}

/*
 *  IPAL ROUTE NETLINK Add IPv4 Route
 */
UI32_T IPAL_Rt_Netlink_AddIpv4Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv4UcNextHop_T *nhs)
{
    return IPAL_Netlink_Route (RTM_NEWROUTE, AF_INET, NLM_F_CREATE|NLM_F_EXCL, table_id, p, (void*)nhs);
}

/*
 *  IPAL ROUTE NETLINK Delete Ipv4 Route
 */
UI32_T IPAL_Rt_Netlink_DeleteIpv4Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv4UcNextHop_T *nhs)
{
    return IPAL_Netlink_Route (RTM_DELROUTE, AF_INET, 0, table_id, p, (void*)nhs);
}

#if (SYS_CPNT_IPV6 == TRUE)
/*
 *  IPAL ROUTE NETLINK Add IPv6 Route
 */
UI32_T IPAL_Rt_Netlink_AddIpv6Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv6UcNextHop_T *nhs)
{
    return IPAL_Netlink_Route (RTM_NEWROUTE, AF_INET6, NLM_F_CREATE|NLM_F_EXCL, table_id, p, (void*)nhs);
}

/*
 *  IPAL ROUTE NETLINK Delete IPv6 Route
 */
UI32_T IPAL_Rt_Netlink_DeleteIpv6Route (UI32_T table_id, const L_INET_AddrIp_T *p, const IPAL_Ipv6UcNextHop_T *nhs)
{
    return IPAL_Netlink_Route (RTM_DELROUTE, AF_INET6, 0, table_id, p, (void*)nhs);
}
#endif

/*
 *  IPAL ROUTE NETLINK Add Network Interface IP Address
 */
UI32_T IPAL_Rt_Netlink_AddIfIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p)
{
    switch (p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            return IPAL_Netlink_Address (RTM_NEWADDR, AF_INET, NLM_F_CREATE|NLM_F_EXCL, ifindex, p);

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            return IPAL_Netlink_Address (RTM_NEWADDR, AF_INET6, NLM_F_CREATE|NLM_F_EXCL, ifindex, p);

        default:
            return IPAL_RESULT_FAIL;
    }
}

/*
 *  IPAL ROUTE NETLINK Add Network Interface alias IP Address
 */
UI32_T IPAL_Rt_Netlink_AddIfIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p,UI32_T alias_id)
{
    switch (p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
	        return (IPAL_Netlink_AddressAlias (RTM_NEWADDR, AF_INET, NLM_F_CREATE|NLM_F_EXCL, ifindex, p,alias_id)<0)?IPAL_RESULT_FAIL:IPAL_RESULT_OK;
            break;
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
	        return (IPAL_Netlink_AddressAlias (RTM_NEWADDR, AF_INET6, NLM_F_CREATE|NLM_F_EXCL, ifindex, p,alias_id)<0)?IPAL_RESULT_FAIL:IPAL_RESULT_OK;
            break;
        default:
            break;
    }
    return IPAL_RESULT_FAIL;
}

/*
 *  IPAL ROUTE NETLINK Delete Network Interface IP Address
 */
UI32_T IPAL_Rt_Netlink_DeleteIfIpAddress (UI32_T ifindex, const L_INET_AddrIp_T *p)
{
    switch (p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            return IPAL_Netlink_Address (RTM_DELADDR, AF_INET, 0, ifindex, p);

        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            return IPAL_Netlink_Address (RTM_DELADDR, AF_INET6, 0, ifindex, p);

        default:
            return IPAL_RESULT_FAIL;
    }
}

/*
 *  IPAL ROUTE NETLINK Delete Network Interface alias IP Address
 */
UI32_T IPAL_Rt_Netlink_DeleteIfIpAddressAlias (UI32_T ifindex, const L_INET_AddrIp_T *p, UI32_T alias_id)
{
    switch (p->type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
	        return (IPAL_Netlink_AddressAlias (RTM_DELADDR, AF_INET, 0, ifindex, p,alias_id)<0)?IPAL_RESULT_FAIL:IPAL_RESULT_OK;
            break;
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
	        return (IPAL_Netlink_AddressAlias (RTM_DELADDR, AF_INET6, 0, ifindex, p,alias_id)<0)?IPAL_RESULT_FAIL:IPAL_RESULT_OK;
            break;
        default:
            break;
    }
    return IPAL_RESULT_FAIL;
}

/*
 *  IPAL ROUTE NETLINK Initialization
 */
void IPAL_Rt_Netlink_Init ()
{
    unsigned long groups = 0;

    //groups = RTMGRP_LINK|RTMGRP_IPV4_ROUTE|RTMGRP_IPV4_IFADDR|RTMGRP_NEIGH;
    //groups |= RTMGRP_IPV6_ROUTE|RTMGRP_IPV6_IFADDR;

    groups = groups | RTMGRP_IPV4_IFADDR;

#if (SYS_CPNT_IPV6 == TRUE)
    groups = groups | RTMGRP_IPV6_IFADDR;
#endif

    IPAL_Netlink_Socket (&ipal_netlink_listen, groups, 0);
    IPAL_Netlink_Socket (&ipal_netlink_cmd, 0, 1);
}

UI32_T IPAL_Rt_RecvMsg(IPAL_ReflectMesg_T *reflect_msg_p)
{
    int status;
    char buf[4096];
    struct iovec iov = { buf, sizeof(buf) };
    struct sockaddr_nl snl = {0};
    struct msghdr msg = { (void*)&snl, sizeof(snl), &iov, 1, NULL, 0, 0};
    struct nlmsghdr *h;
    struct ifaddrmsg *ifa;
    struct rtattr *rta_tb[RTA_MAX + 1];
    int len;

    while (1)
    {
        status = recvmsg (ipal_netlink_listen.sock, &msg, 0);
        if (status < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno == EWOULDBLOCK)
            {
                break;
            }
            continue;
        }

        if (snl.nl_pid != 0)
        {
            continue;
        }

        if (status == 0)
        {
            break;
        }

        if (msg.msg_namelen != sizeof snl)
        {
            break;
        }

        for (h = (struct nlmsghdr *) buf; NLMSG_OK (h, status);
             h = NLMSG_NEXT (h, status))
        {
            if (h->nlmsg_type == RTM_NEWADDR || h->nlmsg_type == RTM_DELADDR)
            {
                ifa = NLMSG_DATA(h);
                len = h->nlmsg_len - NLMSG_LENGTH(sizeof (*ifa));
                if (len < 0)
                    continue;
                memset(rta_tb, 0x0, sizeof(rta_tb));
                IPAL_Parse_Rtattr(rta_tb, IFA_MAX, IFA_RTA (ifa), len);
                if (rta_tb[IFA_ADDRESS] == NULL)
                    rta_tb[IFA_ADDRESS] = rta_tb[IFA_LOCAL];
                if (rta_tb[IFA_ADDRESS])
                {
                    memset(reflect_msg_p, 0x0, sizeof(IPAL_ReflectMesg_T));
                    if (h->nlmsg_type == RTM_NEWADDR)
                        reflect_msg_p->type = IPAL_REFLECT_TYPE_NEW_ADDR;
                    else
                        reflect_msg_p->type = IPAL_REFLECT_TYPE_DEL_ADDR;

                    if(ifa->ifa_family == AF_INET6)
                    {
                        memcpy(reflect_msg_p->u.addr.ipaddr.addr, RTA_DATA(rta_tb[IFA_ADDRESS]), SYS_ADPT_IPV6_ADDR_LEN);
                        reflect_msg_p->u.addr.flags          = ifa->ifa_flags;
                        reflect_msg_p->u.addr.ipaddr.preflen = ifa->ifa_prefixlen;
                        reflect_msg_p->u.addr.ifindex        = ifa->ifa_index;
                        reflect_msg_p->u.addr.ipaddr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
                        if (L_INET_ADDR_IS_IPV6_LINK_LOCAL(reflect_msg_p->u.addr.ipaddr.addr))
                        {
                             reflect_msg_p->u.addr.ipaddr.type = L_INET_ADDR_TYPE_IPV6Z;
                             VLAN_OM_ConvertFromIfindex(ifa->ifa_index, &(reflect_msg_p->u.addr.ipaddr.zoneid));
                        }
                        else
                            reflect_msg_p->u.addr.ipaddr.type = L_INET_ADDR_TYPE_IPV6;
                    }
                    else if(ifa->ifa_family == AF_INET)
                    {
                        memcpy(reflect_msg_p->u.addr.ipaddr.addr, RTA_DATA(rta_tb[IFA_ADDRESS]), SYS_ADPT_IPV4_ADDR_LEN);
                        reflect_msg_p->u.addr.flags          = ifa->ifa_flags;
                        reflect_msg_p->u.addr.ipaddr.preflen = ifa->ifa_prefixlen;
                        reflect_msg_p->u.addr.ifindex        = ifa->ifa_index;
                        reflect_msg_p->u.addr.ipaddr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

                        if(IPAL_Rt_Netlink_IsInterfaceAliasAddress(ifa,rta_tb[IFA_LABEL]))
							continue;

                        if (L_INET_ADDR_IS_IPV4_LINK_LOCAL(reflect_msg_p->u.addr.ipaddr.addr))
                        {
                             reflect_msg_p->u.addr.ipaddr.type = L_INET_ADDR_TYPE_IPV4Z;
                             VLAN_OM_ConvertFromIfindex(ifa->ifa_index, &(reflect_msg_p->u.addr.ipaddr.zoneid));
                        }
                        else
                            reflect_msg_p->u.addr.ipaddr.type = L_INET_ADDR_TYPE_IPV4;

                    }
                    return IPAL_RESULT_OK;
                }
                else continue;
            }
        }
    }

    return IPAL_RESULT_FAIL;
}

/* Check if interface address is alias address
 */
static BOOL_T IPAL_Rt_Netlink_IsInterfaceAliasAddress(struct ifaddrmsg *ifa, struct rtattr *rta)
{
	char ifname[21]={0};

	if((NULL==ifa)||(NULL==rta))
		return FALSE;

	if(!IS_VLAN_IFINDEX_VAILD(ifa->ifa_index))
		return FALSE;

	memcpy(ifname, RTA_DATA(rta), 20);
	if(strchr(ifname,':'))
		return TRUE;

	return FALSE;
}

