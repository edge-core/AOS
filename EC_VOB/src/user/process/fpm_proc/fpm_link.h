/*

NETLINK headers, utility funcitons added here.
NO need to import/ include anything from Zebra/ FRR code.
Now FPM is independent code, only dependency is with ATAN/ AMTRL3 .

*/

#include <asm/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/if_tun.h>
#include <stdbool.h>

#include "fpm.h"

static int AF_INET_SOCK = -1;

/* for debug messages
 */
#define FPM_DBG_NONE  	0
#define FPM_DBG_ERR   	1
#define FPM_DBG_INF   	2
#define FPM_DBG_TRC	  	3
#define FPM_DBG_MIN		FPM_DBG_NONE
#define FPM_DBG_MAX		FPM_DBG_TRC

#define FPM_DBG_PRINTF  printf

#define FPM_DBG_MSG(flag, fmt, ...)                 \
    if ( fpm_is_dbg_on (FPM_ ## flag))  		    \
        FPM_DBG_PRINTF("%s:(%4d):%s " fmt "\r\n",   \
        #flag, __LINE__, basename(__FILE__), ##__VA_ARGS__ );

/* FPM NetLink Macros */
#define ETH_FORMAT "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x"
#define ETH_ADDR_PRINT(x) x[0],x[1],x[2],x[3],x[4],x[5]
#define IPV4_FORMAT "%u.%u.%u.%u"
#define IPV4_ADDR_PRINT(ip)  (ip & 0xff), ((ip >> 8) & 0xff), ((ip >> 16) & 0xff), ((ip >> 24) & 0xff)

#define IPV4_MAX_PREFIXLEN 32

#define BUFLEN 4096
#define NEXT_HOP_KERNEL INT_MAX

#ifndef MAX_NEXT_HOPS
#define MAX_NEXT_HOPS 256 /* used with pending_next_hop_lookup() */
#endif

#define	TRUE	1
#define	FALSE	0

/*
 * Sizes of outgoing and incoming stream buffers for writing/reading
 * FPM messages.
 */
#define ZFPM_IBUF_SIZE (2 * FPM_MAX_MSG_LEN)
#define ZFPM_OBUF_SIZE (FPM_MAX_MSG_LEN)

/* Buffer to receive the data from Zebra Client */
char recv_netlink_msg[ZFPM_IBUF_SIZE];

/* Buffer for sending data to Zebra FPM Client */
char send_netlink_msg[ZFPM_OBUF_SIZE];

/* Copy the received data and process in other thread */
char zebra_fib_data[ZFPM_IBUF_SIZE];

/* GLobal variable for Client FD , for temp usage*/
int fpm_recv_len;

typedef int l3_next_hop_id_t; /* unique ID for next_hop entries */
typedef int l3_intf_id_t;     /* unique ID for l3 interfaces */


struct ethaddr {
        uint8_t octet[ETH_ALEN];
} __attribute__((packed));

/*
 * Generic IP address - union of IPv4 and IPv6 address.
 */
enum ipaddr_type_t {
        IPADDR_NONE = 0,
        IPADDR_V4 = 1, /* IPv4 */
        IPADDR_V6 = 2, /* IPv6 */
};

struct ipaddr {
        enum ipaddr_type_t ipa_type;
        union {
                uint8_t addr;
                struct in_addr _v4_addr;
                struct in6_addr _v6_addr;
        } ip;
#define ipaddr_v4 ip._v4_addr
#define ipaddr_v6 ip._v6_addr
};


typedef struct esi_t_ {
        uint8_t val[10];
} esi_t;

struct evpn_ead_addr {
        esi_t esi;
        uint32_t eth_tag;
};

struct evpn_macip_addr {
        uint32_t eth_tag;
        uint8_t ip_prefix_length;
        struct ethaddr mac;
        struct ipaddr ip;
};

struct evpn_imet_addr {
        uint32_t eth_tag;
        uint8_t ip_prefix_length;
        struct ipaddr ip;
};

struct evpn_es_addr {
        esi_t esi;
        uint8_t ip_prefix_length;
        struct ipaddr ip;
};

struct evpn_prefix_addr {
        uint32_t eth_tag;
        uint8_t ip_prefix_length;
        struct ipaddr ip;
};

struct evpn_addr {
        uint8_t route_type;
        union {
                struct evpn_ead_addr _ead_addr;
                struct evpn_macip_addr _macip_addr;
                struct evpn_imet_addr _imet_addr;
                struct evpn_es_addr _es_addr;
                struct evpn_prefix_addr _prefix_addr;
        } u;
#define ead_addr u._ead_addr
#define macip_addr u._macip_addr
#define imet_addr u._imet_addr
#define es_addr u._es_addr
#define prefix_addr u._prefix_addr
};

struct flowspec_prefix {
        uint16_t prefixlen; /* length in bytes */
        uintptr_t ptr;
};

/* FRR generic prefix structure. */
struct prefix {
        uint8_t family;
        uint16_t prefixlen;
        union {
                uint8_t prefix;
                struct in_addr prefix4;
                struct in6_addr prefix6;
                struct {
                        struct in_addr id;
                        struct in_addr adv_router;
                } lp;
                struct ethaddr prefix_eth; /* AF_ETHERNET */
                uint8_t val[16];
                uint32_t val32[4];
                uintptr_t ptr;
                struct evpn_addr prefix_evpn; /* AF_EVPN */
                struct flowspec_prefix prefix_flowspec; /* AF_FLOWSPEC */
        } u __attribute__((aligned(8)));
};


/* Enum for ROUTE OPERATIONS */
enum route_operation {
    OP_ADD,
    OP_DEL
};


/* Route Info */
typedef struct fpm_ipv4_route_s {
    char src_route_valid;       /* does ->src* contain a real value */
    char dst_route_valid;       /* does ->dst* contain a real value */
    char gateway_valid;         /* does ->gateway contain a real value */
    unsigned int src_if_index;
    unsigned int src_ip;
    unsigned char src_mask_len;   /* 16 not 255.255.0.0 */
    unsigned int dst_if_index;
    unsigned int dst_ip;
    unsigned char dst_mask_len;   /* 16 not 255.255.0.0 */
    unsigned int gateway;    /* gateway == 0 if no gateway/we are last hop */
} fpm_v4_route_t;

/* Structures for next-hop handling
 * A linked list of next_hop
 */

/* Data structure may need to change like hash/ xxxx tree, based on the requirement */
typedef struct pending_next_hop_s {
    char is_gateway;    // is the pending nh for a gateway or not?
    unsigned int pending_next_hop;
    unsigned int pending_netmask;
    unsigned int route_ip;           // this is only used if a gateway route
    unsigned int route_netmask;      // this is only used if a gateway route
    struct pending_next_hop_s * next;
} pending_next_hop_t;

typedef struct next_hop_entry_s {
    unsigned int next_hop_ip;
    l3_next_hop_id_t next_hop_id;
    struct next_hop_entry_s * next;
} next_hop_entry_t;

typedef struct next_hop_db_s {
    struct pending_next_hop_s * head;
    struct next_hop_entry_s * nh_head;
} next_hop_db;


/* NextHop Info */

int pending_next_hop_del_gateway(next_hop_db *db,
        unsigned int gateway_ip,
        unsigned int route_ip,
        unsigned int route_netmask);

int pending_next_hop_del_direct(next_hop_db *db,
        unsigned int route_ip,
        unsigned int route_netmask);

int pending_next_hop_add_direct(
        next_hop_db * db,
        unsigned int route_ip,
        unsigned int route_netmask);
int pending_next_hop_add_gateway(next_hop_db * db, unsigned int gateway_ip,
                                 unsigned int route_ip, unsigned int route_netmask);
int next_hop_del(next_hop_db * db, unsigned int next_hop_ip);
int next_hop_add(next_hop_db * db, unsigned int next_hop_ip, l3_next_hop_id_t next_hop_id);
int next_hop_lookup(next_hop_db *db, unsigned int next_hop_ip, l3_next_hop_id_t * next_hop_id);
char * interface_index_to_name(int index, char * buf, int buflen);
int next_hop_db_init(next_hop_db ** db);
int pending_next_hop_lookup(
        next_hop_db * db,
        unsigned int neighbor,
        unsigned int * dst_ip,
        unsigned int * netmask,
        int * n_entries);



/* Listening for FPM Connection */
void fpm_init(in_addr_t fpm_server_ip, unsigned short fpm_server_port);

/* Accept Zebra FPM Client Connection */
void fpm_link(int sock_fd, struct sockaddr_in fpm_serv);

/* Read the data from Zebra FPM Client */
void *fpm_read_data (void *zfpm_cli_fd);

/* Process the received data */
void *fpm_process_data(void *arg);

/* Write data to Zebra FPM Client */
void *fpm_write_data (void *zfpm_cli_fd);

/* Process FPM FIB info */
void fpm_route_info_decode(char *fib_buf);

int fpm_amtrl3_ipv4_add (unsigned int fib_table, fpm_v4_route_t *entry);
int fpm_amtrl3_ipv4_delete (unsigned int fib_table, fpm_v4_route_t *entry);
void fpm_handle_v4_route(enum route_operation rt_op, struct nlmsghdr * nl_msg);
int fpm_processv4_route(struct nlmsghdr * nl_msg, fpm_v4_route_t *entry);
void fpm_handle_v4_neighbor(enum route_operation rt_op, struct nlmsghdr * nl_msg);

int next_hop_db_free(next_hop_db * );
int set_socket_non_blocking(int );

int fpm_is_dbg_on(int flag);

/* Utility Functions */
char * interface_index_to_name(int index, char * buf, int buflen)
{
    struct ifreq ifr;
    if (AF_INET_SOCK == -1)
        AF_INET_SOCK = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_ifindex = index;
    int err = ioctl(AF_INET_SOCK, SIOCGIFNAME, &ifr);
    if (err == 0)
    {
        snprintf(buf, buflen, "%s", ifr.ifr_name);
        return buf;
    }
    else
    {
        return NULL;
    }
}

int next_hop_lookup(next_hop_db *db, unsigned int next_hop_ip, l3_next_hop_id_t * next_hop_id)
{
    next_hop_entry_t * curr;
    curr = db->nh_head;
    while (curr != NULL)
    {
        if (curr->next_hop_ip == next_hop_ip)
        {
            *next_hop_id = curr->next_hop_id;
            return 1;
        }
        curr = curr->next;
    }
    return 0;   /* not found */
}

int next_hop_add(next_hop_db * db, unsigned int next_hop_ip, l3_next_hop_id_t next_hop_id)
{
    next_hop_entry_t * curr;
    curr = db->nh_head;
    while (curr != NULL)
    {
        if (curr->next_hop_ip == next_hop_ip)
            return 1;
        curr = curr->next;
    }
    /* curr is currently NULL, just reuse */
    curr = malloc(sizeof(next_hop_entry_t));
    if (curr == NULL)
    {
        return -1;
    }
    curr->next_hop_ip = next_hop_ip;
    curr->next_hop_id = next_hop_id;
    curr->next = db->nh_head;   // add to front of list
    db->nh_head = curr;

    return 0;
}

int next_hop_del(next_hop_db * db, unsigned int next_hop_ip)
{
    next_hop_entry_t * curr, *prev;
    prev = NULL;
    curr = db->nh_head;
    while (curr != NULL)
    {
        if (curr->next_hop_ip == next_hop_ip)
            break;
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL)
        return 1;   /* not found */

    if (prev != NULL)
        prev->next = curr->next;
    else
        db->nh_head = curr->next;
    free(curr);

    return 0;
}

static int pending_next_hop_gateway_entry_init(
        pending_next_hop_t ** entry,
        unsigned int gateway_ip,
        unsigned int ip_dst,
        unsigned int netmask)
{
    (*entry) = malloc(sizeof(**entry));
    if ((*entry) == NULL)
    {
        return -1;
    }
    (*entry)->is_gateway = 1;
    (*entry)->pending_next_hop = gateway_ip;
    (*entry)->pending_netmask = 0xffffffff; /* match all bits on gateway */
    (*entry)->route_ip = ip_dst;
    (*entry)->route_netmask = netmask;

    return 0;
}

static int pending_next_hop_cmp(
        unsigned int ip_a,
        unsigned int netmask_a,
        unsigned int ip_b,
        unsigned int netmask_b)
{
    int a_val = ip_a & netmask_a;
    int b_val = ip_b & netmask_b;
    return a_val - b_val;
}

static int pending_add(next_hop_db * db, pending_next_hop_t * entry)
{
    pending_next_hop_t * curr, *prev = NULL;
    int cmp = 1;

    curr = db->head;

    /** look for place in sorted list **/
    while ((curr != NULL) && (
                (cmp = pending_next_hop_cmp(
                        curr->pending_next_hop,
                        curr->pending_netmask,
                        entry->pending_next_hop,
                        entry->pending_netmask)) < 0))
    {
        prev = curr;
        curr = curr->next;
    }

    if ( cmp == 0)
    {
        return -1;
    }
    entry->next = curr;
    if (prev == NULL)       /* adding to begin of list */
        db->head = entry;
    else                    /* adding to middle or end */
        prev->next = entry;

    return 0;       /* success */
}


int pending_next_hop_add_gateway(next_hop_db * db, unsigned int gateway_ip, unsigned int route_ip, unsigned int route_netmask)
{
    pending_next_hop_t * entry;
    pending_next_hop_gateway_entry_init(
            &entry,
            gateway_ip,
            route_ip,
            route_netmask);

    return pending_add(db, entry);
}

static int pending_next_hop_direct_entry_init(
        pending_next_hop_t ** entry,
        unsigned int ip_dst,
        unsigned int netmask)
{
    (*entry) = malloc(sizeof(**entry));
    if ((*entry) == NULL)
    {
        return -1;
    }
    (*entry)->is_gateway = 0;
    (*entry)->pending_next_hop = ip_dst;
    (*entry)->pending_netmask = netmask;

    return 0;
}

int pending_next_hop_add_direct(
        next_hop_db * db,
        unsigned int route_ip,
        unsigned int route_netmask)
{
    pending_next_hop_t * entry;

    pending_next_hop_direct_entry_init(
            &entry,
            route_ip,
            route_netmask);

    return pending_add(db, entry);
}


static int pending_next_hop_entry_free(pending_next_hop_t * entry)
{
    free(entry);
    return 0;
}

static int pending_next_hop_del(
        next_hop_db *db,
        unsigned int route_ip,
        unsigned int route_netmask)
{
    pending_next_hop_t * prev = NULL, * curr;
    curr = db->head;

    while ( (curr != NULL) && (pending_next_hop_cmp(
                        curr->route_ip,
                        curr->route_netmask,
                        route_ip,
                        route_netmask)!=0))
    {
        prev = curr;
        curr = curr->next;
    }

    if (curr == NULL) {
        return -1;      /* not found */
    }
    if (prev == NULL)
        db->head = curr->next;  /* was begin of list */
    else
        prev->next = curr->next; /* was middle or end */

    pending_next_hop_entry_free(curr);
    return 0;
}

int pending_next_hop_del_gateway(next_hop_db *db,
        unsigned int gateway_ip,
        unsigned int route_ip,
        unsigned int route_netmask)
{
    return pending_next_hop_del(db, route_ip, route_netmask);
}

int pending_next_hop_del_direct(next_hop_db *db,
        unsigned int route_ip,
        unsigned int route_netmask)
{
    return pending_next_hop_del(db, route_ip, route_netmask);
}


int next_hop_db_free(next_hop_db * db)
{
    pending_next_hop_t * curr, * next;
    next_hop_entry_t * nh_curr, * nh_next;
    curr = db->head;
    while (curr != NULL)
    {
        next = curr->next;
        pending_next_hop_del(db, curr->route_ip, curr->route_netmask);
        curr = next;
    }
    nh_curr = db->nh_head;
    while (nh_curr != NULL)
    {
        nh_next = nh_curr->next;
        free(nh_curr);
        nh_curr = nh_next;
    }

    free(db);
    return 0;
}

int pending_next_hop_lookup(
        next_hop_db * db,
        unsigned int neighbor,
        unsigned int * dst_ip,
        unsigned int * netmask,
        int * n_entries)
{
    int max = *n_entries;
    *n_entries = 0;
    pending_next_hop_t *curr = db->head;

    /** look for place in sorted list **/
    while ((curr != NULL) && (
                pending_next_hop_cmp(
                        curr->pending_next_hop,
                        curr->pending_netmask,
                        neighbor,
                        0xffffffff) < 0)
            )
    {
        if ((neighbor & curr->pending_netmask) == curr->pending_next_hop)
        {
            if ( (*n_entries) >= max)
                return -2;      // return array full
            dst_ip[*n_entries] = curr->pending_next_hop;
            netmask[*n_entries] = curr->pending_netmask;
            (*n_entries) ++;
        }
        curr = curr->next;
    }

    return (*n_entries > 0)? 1 : 0;
}

int next_hop_db_init(next_hop_db ** db)
{
    FPM_DBG_MSG(DBG_INF, "Allocating memory for next-hop DS");
    *db = malloc(sizeof(**db));
    if ((*db) == NULL)
    {
        FPM_DBG_MSG(DBG_ERR , "Failed to alloc next_hop_db(): %s",
                strerror(errno));
        return -1;
    }
    (*db)->head = NULL;
    (*db)->nh_head = NULL;
    return 0;
}


int set_socket_non_blocking(int fd)
{
    int flags, s;

    flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        FPM_DBG_MSG(DBG_ERR, "fcntl(): %d : %s", flags, strerror(errno));
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(fd, F_SETFL, flags);
    if (s == -1)
    {
        FPM_DBG_MSG(DBG_ERR, "fcntl(): %d : %s", s, strerror(errno));
        return -1;
    }

    return 0;
}

