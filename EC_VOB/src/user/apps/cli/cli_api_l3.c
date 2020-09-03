#include <string.h>
#include "leaf_2096.h"
#include "cli_api.h"
#include "cli_api_l3.h"
#include "leaf_es3626a.h"
#include "netcfg_type.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_pmgr_rip.h" /*Lin.Li, for RIP porting*/
#include "l_radix.h"
#endif
#include "ip_lib.h"
#include "l_inet.h"
#include "leaf_1850.h"
#include "vlan_pmgr.h"
#include "vlan_lib.h"
#include "l_mm.h"
#include "sys_time.h"

#if (SYS_CPNT_NSM == TRUE)
    #include "nsm_mgr.h"
    #include "nsm_type.h"
    #include "nsm_pmgr.h"
#endif

#include "netcfg_pmgr_route.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pom_route.h"
#include "netcfg_om_ip.h"

#include "ipal_if.h"
#include "ipal_types.h"


/*fuzhimin, 20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <linux/if_ether.h>
#endif
/*fuzhimin,20090106,end*/

#include "dhcp_type.h"

#if(SYS_CPNT_DHCP == TRUE)
#include "dhcp_pmgr.h"
#include "dhcp_pom.h"
#include "dhcp_mgr.h"
#endif
#if (SYS_CPNT_OSPF == TRUE)
#include "netcfg_pmgr_ospf.h"
#include "netcfg_mgr_ospf.h"
#include "ospf_mgr.h"
#include "ospf_pmgr.h"
#include "ospf_type.h"
#endif
#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#include "netcfg_pom_ip.h"
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_type.h"
#include "amtrl3_pom.h"
#endif

#if (SYS_CPNT_BGP == TRUE)
#include "cli_api_bgp.h" /* show_ip_protocols_bgp() */
#endif

#define CHECK_FLAG(V,F)      ((V) & (F))
#define SET_FLAG(V,F)        (V) = (V) | (F)
#define UNSET_FLAG(V,F)      (V) = (V) & ~(F)
#define FLAG_ISSET(V,F)      (((V) & (F)) == (F))
#define SN_LAN                          1

/* wakka: define to avoid compiling wrong
 */
#if (SYS_CPNT_RIP == TRUE) || (SYS_CPNT_OSPF == TRUE)
#define CLI_API_L3_SUPPORT_CONVERT_IFINDEX_AND_NAME     TRUE
#endif
#if (SYS_CPNT_RIP == TRUE)
#define CLI_API_L3_SUPPORT_GET_IP_PREFIX_STR    TRUE
#endif

#if (SYS_CPNT_ARP == TRUE)
static L_INET_AddrIp_T CLI_API_L3_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen);
#endif /* #if (SYS_CPNT_ARP == TRUE) */

/* PIM MACRO */
#define INIT_IPV4_GROUP_PREFIX(inet_addr) do{ \
    memset(&inet_addr, 0, sizeof(inet_addr)); \
    inet_addr.type = L_INET_ADDR_TYPE_IPV4; \
    inet_addr.addrlen = 4; \
}while(0)

#define INIT_IPV4_ALL_GROUP_PREFIX(inet_addr) do{ \
    INIT_IPV4_GROUP_PREFIX(inet_addr); \
    inet_addr.addr[0] = 0xE0; /* 224.0.0.0 */ \
    inet_addr.preflen = 4; \
}while(0)


#if (SYS_CPNT_OSPF == TRUE)
static int CLI_API_L3_Ospf_Str2AreaId (char *str, struct pal_in4_addr *area_id, UI32_T *format);
static BOOL_T show_ospf_virtual_link_all(UI32_T vr_id);
static int CLI_API_L3_Str2Addr(char *str, struct pal_in4_addr *addr);
UI32_T show_ip_protocols_ospf(UI32_T *line_num_p);
#endif /* #if (SYS_CPNT_OSPF == TRUE) */

UI32_T show_ip_protocols_rip(UI32_T *line_num_p);
#if (SYS_CPNT_OSPF == TRUE)
#define CHECK_FLAG(V,F)      ((V) & (F))
/* OSPF LSA Type definition. */
#define OSPF_UNKNOWN_LSA            0
#define OSPF_ROUTER_LSA             1
#define OSPF_NETWORK_LSA            2
#define OSPF_SUMMARY_LSA            3
#define OSPF_SUMMARY_LSA_ASBR           4
#define OSPF_AS_EXTERNAL_LSA            5
#define OSPF_GROUP_MEMBER_LSA           6  /* Not supported. */
#define OSPF_AS_NSSA_LSA                7
#define OSPF_EXTERNAL_ATTRIBUTES_LSA    8  /* Not supported. */
#define OSPF_LINK_OPAQUE_LSA            9
#define OSPF_AREA_OPAQUE_LSA            10
#define OSPF_AS_OPAQUE_LSA              11
#define ROUTER_LSA_BIT_B        (1 << 0)
#define ROUTER_LSA_BIT_E        (1 << 1)
#define ROUTER_LSA_BIT_V        (1 << 2)
#define ROUTER_LSA_BIT_W        (1 << 3)
#define ROUTER_LSA_BIT_NT       (1 << 4)
#define ROUTER_LSA_BIT_S        (1 << 5) /* Shortcut-ABR. */

#define AS_EXTERNAL_LSA_BIT_T       (1 << 0)
#define AS_EXTERNAL_LSA_BIT_F       (1 << 1)
#define AS_EXTERNAL_LSA_BIT_E       (1 << 2)

#define OSPF_AREA_DEFAULT           0
#define OSPF_AREA_STUB              1
#define OSPF_AREA_NSSA              2
#define OSPF_AREA_TYPE_MAX          3

/* OSPF Authentication Type. */
#define OSPF_AUTH_NULL               0
#define OSPF_AUTH_SIMPLE             1
#define OSPF_AUTH_CRYPTOGRAPHIC          2

#define HAVE_NSSA
struct message
{
  s_int32_t key;
  char *str;
};

static char *show_database_desc[] =
{
  "unknown",
  "Router Link States",
  "Net Link States",
  "Summary Link States",
  "ASBR-Summary Link States",
  "AS External Link States",
#if defined (HAVE_NSSA) || defined (HAVE_OPAQUE_LSA)
  "Group Membership LSA",
  "NSSA-external Link States",
#endif /* HAVE_NSSA */
#ifdef HAVE_OPAQUE_LSA
  "Type-8 LSA",
  "Link-Local Opaque-LSA",
  "Area-Local Opaque-LSA",
  "AS-Global Opaque-LSA",
#endif /* HAVE_OPAQUE_LSA */
};

static char *show_database_header[] =
{
  "",
  "Link ID         ADV Router      Age  Seq#       CkSum  Link count",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route",
  "Link ID         ADV Router      Age  Seq#       CkSum",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route                 Tag",
#ifdef HAVE_NSSA
  " --- header for Group Member ----",
  "Link ID         ADV Router      Age  Seq#       CkSum  Route                 Tag",
#endif /* HAVE_NSSA */
#ifdef HAVE_OPAQUE_LSA
#ifndef HAVE_NSSA
  "--- type-6 ---",
  "--- type-7 ---",
#endif /* HAVE_NSSA */
  "--- type-8 ---",
  "Link ID         ADV Router      Age  Seq#       CkSum Opaque ID",
  "Link ID         ADV Router      Age  Seq#       CkSum Opaque ID",
  "Link ID         ADV Router      Age  Seq#       CkSum Opaque ID",
#endif /* HAVE_OPAQUE_LSA */
};

static char *ospf_abr_type_descr_str[] =
{
  "Unknown",
  "Standard (RFC2328)",
  "Alternative Cisco (RFC3509)",
  "Alternative IBM (RFC3509)",
  "Alternative Shortcut"
};

static struct message ospf_area_type_msg[] =
{
  { OSPF_AREA_DEFAULT,  "Default" },
  { OSPF_AREA_STUB,     "Stub" },
  { OSPF_AREA_NSSA,     "NSSA" },
};
static int ospf_area_type_msg_max = OSPF_AREA_TYPE_MAX;

static char *ospf_shortcut_mode_descr_str[] =
{
  "Default",
  "Enabled",
  "Disabled"
};

static struct message ospf_lsa_type_msg[] =
{
  { OSPF_UNKNOWN_LSA,      "unknown" },
  { OSPF_ROUTER_LSA,       "router-LSA" },
  { OSPF_NETWORK_LSA,      "network-LSA" },
  { OSPF_SUMMARY_LSA,      "summary-LSA" },
  { OSPF_SUMMARY_LSA_ASBR, "ASBR-summary-LSA" },
  { OSPF_AS_EXTERNAL_LSA,  "AS-external-LSA" },
  { OSPF_GROUP_MEMBER_LSA, "GROUP_MEMBER_LSA" },
  { OSPF_AS_NSSA_LSA,      "AS-NSSA-LSA" },
  { 8,                     "Type-8 LSA" },
  { OSPF_LINK_OPAQUE_LSA,  "Link-Local Opaque-LSA" },
  { OSPF_AREA_OPAQUE_LSA,  "Area-Local Opaque-LSA" },
  { OSPF_AS_OPAQUE_LSA,    "AS-external Opaque-LSA" }
};

static struct  message ospf_link_state_id_type_msg[] =
{
  { OSPF_UNKNOWN_LSA,      "(unknown)" },
  { OSPF_ROUTER_LSA,       "" },
  { OSPF_NETWORK_LSA,      "(address of Designated Router)" },
  { OSPF_SUMMARY_LSA,      "(summary Network Number)" },
  { OSPF_SUMMARY_LSA_ASBR, "(AS Boundary Router address)" },
  { OSPF_AS_EXTERNAL_LSA,  "(External Network Number)" },
  { OSPF_GROUP_MEMBER_LSA, "(Group membership information)" },
  { OSPF_AS_NSSA_LSA,      "(External Network Number For NSSA)" },
  { 8,                     "(Type-8 LSID)" },
  { OSPF_LINK_OPAQUE_LSA,  "(Link-Local Opaque-Type/ID)" },
  { OSPF_AREA_OPAQUE_LSA,  "(Area-Local Opaque-Type/ID)" },
  { OSPF_AS_OPAQUE_LSA,    "(AS-external Opaque-Type/ID)" }
};
/* OSPF LSA Link Type. */
#define LSA_LINK_TYPE_POINTOPOINT       1
#define LSA_LINK_TYPE_TRANSIT           2
#define LSA_LINK_TYPE_STUB              3
#define LSA_LINK_TYPE_VIRTUALLINK       4
#define IS_ROUTER_LSA_SET(B,F)       (CHECK_FLAG ((B), ROUTER_LSA_ ## F))

static char *link_type_desc[] =
{
  "(null)",
  "another Router (point-to-point)",
  "a Transit Network",
  "Stub Network",
  "a Virtual Link",
};

static char *link_id_desc[] =
{
  "(null)",
  "Neighboring Router ID",
  "Designated Router address",
  "Network/subnet number",
  "Neighboring Router ID",
};

static char *link_data_desc[] =
{
  "(null)",
  "Router Interface address",
  "Router Interface address",
  "Network Mask",
  "Router Interface address",
};
static int ospf_lsa_type_msg_max = 8;
static int ospf_link_state_id_type_msg_max = 8;

static char *
ospf_option_dump (u_char options, char *buf)
{
    /* OSPF options. */
    #define OSPF_OPTION_T           (1 << 0)    /* TOS. */
    #define OSPF_OPTION_E           (1 << 1)
    #define OSPF_OPTION_MC          (1 << 2)
    #define OSPF_OPTION_NP          (1 << 3)
    #define OSPF_OPTION_EA          (1 << 4)
    #define OSPF_OPTION_DC          (1 << 5)
    #define OSPF_OPTION_O           (1 << 6)
    #define OSPF_OPTION_L           OSPF_OPTION_EA  /* LLS. */

    sprintf (buf, "*|%s|%s|%s|%s|%s|%s|%s",
         (options & OSPF_OPTION_O) ? "O" : "-",
         (options & OSPF_OPTION_DC) ? "DC" : "-",
         #ifdef HAVE_RESTART
         (options & OSPF_OPTION_L) ? "L" : "-",
         #else /* HAVE_RESTART */
         (options & OSPF_OPTION_EA) ? "EA" : "-",
         #endif /* HAVE_RESTART */
         (options & OSPF_OPTION_NP) ? "N/P" : "-",
         (options & OSPF_OPTION_MC) ? "MC" : "-",
         (options & OSPF_OPTION_E) ? "E" : "-",
         (options & OSPF_OPTION_T) ? "T" : "-");

    return buf;
}

static char *
mes_lookup (struct message *meslist, s_int32_t max, s_int32_t index)
{
  if (index < 0 || index >= max)
    return "invalid";

  return meslist[index].str;
}
#define LOOKUP(X,Y) mes_lookup(X, X ## _max, Y)

#define SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry) \
do{\
    if (CHECK_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_PROC))\
    {\
        HexAddrToStr(lsa_entry.router_id.s_addr, tmpBuf, FALSE);\
        sprintf(tmpBuf2, "\r\n            OSPF Router with ID (%s) (Process ID %ld)\r\n",\
                         tmpBuf, (long)lsa_entry.proc_id);  \
        PROCESS_MORE(tmpBuf2);          \
        UNSET_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_PROC);\
    }\
    if (CHECK_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_TYPE))\
    {\
        if ( lsa_entry.type == OSPF_AS_EXTERNAL_LSA )\
            sprintf(tmpBuf2, "\r\n                %s \r\n\r\n",\
                             show_database_desc[lsa_entry.type]); \
        else\
        {\
            HexAddrToStr(lsa_entry.area_id.s_addr, tmpBuf, FALSE);\
            if ( lsa_entry.area_type == OSPF_AREA_NSSA )\
                sprintf(tmpBuf2, "\r\n                %s (Area %s [NSSA])\r\n\r\n",\
                                 show_database_desc[lsa_entry.type], tmpBuf); \
            else if ( lsa_entry.area_type == OSPF_AREA_STUB ) \
                sprintf(tmpBuf2, "\r\n                %s (Area %s [Stub])\r\n\r\n",\
                                 show_database_desc[lsa_entry.type], tmpBuf);\
            else\
                sprintf(tmpBuf2, "\r\n                %s (Area %s)\r\n\r\n",\
                                 show_database_desc[lsa_entry.type], tmpBuf);\
        }\
        PROCESS_MORE(tmpBuf2);\
        UNSET_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_TYPE);\
    }\
    sprintf(tmpBuf2, "  LS Age: %d\r\n", lsa_entry.header.ls_age); \
    PROCESS_MORE(tmpBuf2); \
    sprintf(tmpBuf2, "  Options: 0x%x (%s)\r\n", lsa_entry.header.options,\
              ospf_option_dump (lsa_entry.header.options, option_str));\
    PROCESS_MORE(tmpBuf2);\
    if (lsa_entry.header.type == OSPF_ROUTER_LSA)\
    {       \
      sprintf(tmpBuf2, "  Flags: 0x%x" , lsa_entry.bits);\
      PROCESS_MORE(tmpBuf2);\
      if (lsa_entry.bits)\
      {\
          sprintf(tmpBuf2, " :%s%s%s%s%s",\
               IS_ROUTER_LSA_SET (lsa_entry.bits, BIT_B) ? " ABR" : "",\
               IS_ROUTER_LSA_SET (lsa_entry.bits, BIT_E) ? " ASBR" : "",\
               IS_ROUTER_LSA_SET (lsa_entry.bits, BIT_V) ? " VL-endpoint" : "",\
               IS_ROUTER_LSA_SET (lsa_entry.bits, BIT_S) ? " Shortcut" : "",\
               IS_ROUTER_LSA_SET (lsa_entry.bits, BIT_NT) ? " NSSA-Translator" : "");\
          PROCESS_MORE(tmpBuf2);\
      }\
      sprintf(tmpBuf2, "\r\n");\
      PROCESS_MORE(tmpBuf2);\
    }\
    sprintf(tmpBuf2, "  LS Type: %s\r\n", LOOKUP (ospf_lsa_type_msg, lsa_entry.header.type));\
    PROCESS_MORE(tmpBuf2);\
    HexAddrToStr(lsa_entry.header.id.s_addr, tmpBuf, FALSE);\
    sprintf(tmpBuf2, "  Link State ID: %s %s\r\n", tmpBuf, LOOKUP (ospf_link_state_id_type_msg, lsa_entry.header.type));\
    PROCESS_MORE(tmpBuf2);\
    HexAddrToStr(lsa_entry.header.adv_router.s_addr, tmpBuf, FALSE);\
    sprintf(tmpBuf2, "  Advertising Router: %s\r\n", tmpBuf);\
    PROCESS_MORE(tmpBuf2);\
    sprintf(tmpBuf2, "  LS Seq Number: %08lx\r\n", (long)pal_ntoh32 (lsa_entry.header.ls_seqnum));\
    PROCESS_MORE(tmpBuf2);\
    sprintf(tmpBuf2, "  Checksum: 0x%04x\r\n", pal_ntoh16 (lsa_entry.header.checksum));\
    PROCESS_MORE(tmpBuf2);\
    sprintf(tmpBuf2, "  Length: %d\r\n", pal_ntoh16 (lsa_entry.header.length));\
    PROCESS_MORE(tmpBuf2);\
}while(0)

#endif /* #if (SYS_CPNT_OSPF == TRUE) */

/*fuzhimin,20090212*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
static int if_ioctl (int request, caddr_t buffer);
static int setEth0IpAddr(char*devName,int unit,int ipaddr,int netmask);
static int delEth0IpAddr(char*devName,int unit);

/* call ioctl system call */
static int if_ioctl (int request, caddr_t buffer)
{
    int sock;
    int ret = 0;

    sock = socket (AF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        /*printf ("if_ioctl: failed to create a socket\n");*/
        return (-1/*ERROR*/);
    }

    ret = ioctl (sock, request, buffer);
    if (ret < 0)
    {
        /*printf ("if_ioctl: failed to ioctl\n");*/
        close (sock);
        return (-1/*ERROR*/);
    }
    close (sock);
    return 0;
}


/*******************************************************************************
*
*setEth0IpAddr
*/

static int setEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit,           /* unit number */
    int         ipaddr,                   /* target ip address */
    int         netmask               /* subnet mask */
)
{
    char ifname [20];
    int s;
    struct ifreq ifr;
    struct sockaddr_in addrTmp;
    int ret;
    struct sockaddr_in mask;

    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0' )
    {
        /*printf("setEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */

    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    /*set IFADDRESS*/
    addrTmp.sin_addr.s_addr = ipaddr;
    addrTmp.sin_family = AF_INET;
    memcpy (&ifr.ifr_addr, &addrTmp, sizeof (struct sockaddr_in));
    ret = if_ioctl (SIOCSIFADDR, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("setEth0IpAddr: Failed SIOCSIFADDR for %s\n", ifname);*/
        return (-1);
    }

    /*set NETMASK*/
    mask.sin_addr.s_addr = netmask;
    mask.sin_family = AF_INET;
    memcpy (&ifr.ifr_netmask, &mask, sizeof (struct sockaddr_in));
    ret = if_ioctl (SIOCSIFNETMASK, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("setEth0IpAddr: Failed SIOCSIFNETMASK for %s\n", ifname);*/
        return (-1);
    }
#if 0
    /* set subnet mask, if any specified */

    if (netmask &&
    netIoctl (ifname, SIOCSIFNETMASK, htonl(netmask)) != 0)
    {
    printf ("usrNetBootConfig: Failed SIOCSIFNETMASK for %s\n", ifname);
    return (-1);
    }

    /* set inet addr */

    ipaddr = inet_addr(addr);      /* htonl is done inside inet_addr */
    if (netIoctl(ifname, SIOCSIFADDR, ipaddr) != 0)
    {
    printf("usrNetBootConfig: Failed SIOCSIFADDR for %s\n", ifname);
    return (-1);
    }
#endif

    /* Set IFF_UP */
    if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        /*printf ("setEth0IpAddr: failed to create a socket\n");*/
        return (-1/*ERROR*/);
    }

    bzero ((caddr_t)&ifr, sizeof (ifr));
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    if(ioctl(s, (int)SIOCGIFFLAGS, (int)&ifr) != 0)
    {
        /*printf ("setEth0IpAddr: ioctl (SIOCGIFFLAGS)\n");*/
        close (s);
        return (-1);
    }
    ifr.ifr_flags |= IFF_UP;
    if(ioctl (s, (int)SIOCSIFFLAGS, (int)&ifr) != 0)
    {
        /*printf("setEth0IpAddr: ioctl (SIOCSIFFLAGS)\n");*/
        close (s);
        return (-1);
    }
    close (s);

    return (0);
}



/*******************************************************************************
*
*delEth0IpAddr
*/

static int delEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit
)
{
    char ifname [20];
    struct ifreq ifr;
    int ret;

    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0' )
    {
        /*printf("delEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */

    bzero (ifname, sizeof (ifname));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFFLAGS for %s\n", ifname);*/
        return (-1);
    }
    ifr.ifr_flags &= ~IFF_UP;
    ret = if_ioctl (SIOCSIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCSIFFLAGS for %s\n", ifname);*/
        return (-1);
    }

    return (0);
}



/*******************************************************************************
*
*getEth0IpAddr
*/
int getEth0IpAddr
(
    char *      devName,                /* device name e.g. "eth" */
    int         unit,
    int *  ipAddr,
    int *  netmask,
    int*   flag
)
{
    char ifname [20];
    struct ifreq ifr;
    int ret;

    /*
     * Do nothing if another device is already configured or an
     * error was detected in the boot parameters.
     */

    if (devName == NULL || devName[0] == '\0'
         || ipAddr == NULL)
    {
        /*printf("delEth0IpAddr: Invalid Argument\n");*/
        return (-1);
    }

    /* build interface name */
    bzero (ifname, sizeof (ifname));
    bzero ((caddr_t)&ifr, sizeof (ifr));
    sprintf (ifname, "%s%d", devName, unit);
    strncpy (ifr.ifr_name, ifname, IFNAMSIZ);

    ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFADDR for %s\n", ifname);*/
        return (-1);
    }
    *flag = ifr.ifr_flags;

    ret = if_ioctl (SIOCGIFADDR, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFADDR for %s\n", ifname);*/
        return (-1);
    }
    *ipAddr = ((struct sockaddr_in*)(&(ifr.ifr_addr)))->sin_addr.s_addr;

    ret = if_ioctl (SIOCGIFNETMASK, (caddr_t) &ifr);
    if (ret < 0)
    {
        /*printf("delEth0IpAddr: Failed SIOCGIFNETMASK for %s\n", ifname);*/
        return (-1);
    }
    *netmask = ((struct sockaddr_in*)(&(ifr.ifr_netmask)))->sin_addr.s_addr;

    return (0);
}
#endif
/*fuzhimin,20090212,end*/

#if (SYS_CPNT_ROUTING == TRUE)
#if (CLI_API_L3_SUPPORT_CONVERT_IFINDEX_AND_NAME == TRUE)
/*static BOOL_T CLI_API_L3_GetIfindexFromIfname(char *ifname, UI32_T *ifindex);*/
static BOOL_T CLI_API_L3_GetIfnameFromIfindex(UI32_T ifindex, char *ifname);
#endif
#if (CLI_API_L3_SUPPORT_GET_IP_PREFIX_STR == TRUE)
static void CLI_API_L3_GetIpPrefixString(char *ipaddr, char *ipmask, char *str);
#endif

#if (CLI_API_L3_SUPPORT_CONVERT_IFINDEX_AND_NAME == TRUE)
#if 0
static BOOL_T CLI_API_L3_GetIfindexFromIfname(char *ifname, UI32_T *ifindex)
{
    char    *ptr;
    UI32_T  vlan_id = 0;

    if(strncasecmp("vlan",ifname,4)!= 0)
      return FALSE;
    for(ptr = ifname + 4; *ptr != '\0'; ++ptr)
      if ((*ptr -'9' > 0) || (*ptr -'0'< 0))
      {
        return FALSE;
      }
    sscanf(ifname+4, "%ld", &vlan_id);
    if(VLAN_OM_ConvertToIfindex(vlan_id, ifindex) == TRUE)
        return TRUE;
    else
        return FALSE;
}
#endif
static BOOL_T CLI_API_L3_GetIfnameFromIfindex(UI32_T ifindex, char *ifname)
{
    UI32_T vid = 0;

    if(ifname == NULL)
        return FALSE;

    if (FALSE == VLAN_OM_ConvertFromIfindex(ifindex, &vid))
        return FALSE;

    snprintf(ifname, 20, "VLAN%ld", (long)vid);
    return TRUE;
}
#endif

#if (CLI_API_L3_SUPPORT_GET_IP_PREFIX_STR == TRUE)
static void CLI_API_L3_GetIpPrefixString(char *ipaddr, char *ipmask, char *str)
{
    UI32_T plen,count,i;
    UI8_T addr_str[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    char pstr[3] = {0};

    count = 0;
    strcpy(str, ipaddr);
    CLI_LIB_AtoIp(ipmask,addr_str);
    memcpy(&plen, addr_str, sizeof(UI32_T));
    for(i=0;i<32;i++)
    {
        if(plen &(1<<i))
        {
            count++;
        }
    }
    str[strlen(str)] = '/';
    sprintf(pstr,"%ld",(long)count);
    strcat(str,pstr);
}
#endif
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_ARP == TRUE)
static L_INET_AddrIp_T CLI_API_L3_ComposeInetAddr(UI16_T type,UI8_T* addrp,UI16_T prefixLen)
{
    L_INET_AddrIp_T addr;
    memset(&addr,0,sizeof(addr));
    addr.type = type;
    if(L_INET_ADDR_TYPE_IPV4 == type ||L_INET_ADDR_TYPE_IPV4Z==type)
        addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    else if(L_INET_ADDR_TYPE_IPV6 == type||L_INET_ADDR_TYPE_IPV6Z==type)
        addr.addrlen = SYS_ADPT_IPV6_ADDR_LEN;
    else{printf ("Wooop! something wring!\r\n");}
    memcpy(addr.addr,addrp,addr.addrlen);
    addr.preflen = prefixLen;
    return addr;
}
#endif /* #if (SYS_CPNT_ARP == TRUE) */

UI32_T CLI_API_L3_Show_Ip_Rip_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)

    BOOL_T instance_flag = FALSE;/*if the rip instance create*/

    L_INET_AddrIp_T inet_addr;

    NETCFG_TYPE_RIP_If_T entry;
    NETCFG_TYPE_RIP_Instance_T instance_entry;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T vid = 0, entry_vid = 0;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    memset(&instance_entry, 0, sizeof(NETCFG_TYPE_RIP_Instance_T));

    if(NETCFG_PMGR_RIP_GetInstanceEntry(&instance_entry) == NETCFG_TYPE_OK)
    {
        instance_flag = TRUE;
    }

    while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
    {

        if(FALSE == VLAN_IFINDEX_CONVERTTO_VID(entry.ifindex, entry_vid))
            continue;

        /* vlan is specified */
        if(arg[0] && (arg[0][0]=='v')) /* vlan */
        {
            vid = CLI_LIB_AtoUl(arg[1],10);
            if(vid != entry_vid)
                continue;
        }

        sprintf(buff,"Interface: VLAN %lu\r\n", (unsigned long)entry_vid);
        PROCESS_MORE(buff);

        memset(&inet_addr, 0, sizeof(inet_addr));
        inet_addr.type = L_INET_ADDR_TYPE_IPV4;
        /* check if any active rif on vlan */
        if(instance_flag && (NETCFG_TYPE_OK == NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex(entry.ifindex, &inet_addr)))
        {
            sprintf(buff,"  Routing Protocol: RIP\r\n");
            PROCESS_MORE(buff);

            if(entry.recv_packet)
            {
                enum NETCFG_TYPE_RIP_Version_E recv_version;
                /* If there is no version configuration in the interface,
                   use rip's version setting. */
                if (! RIP_IF_PARAM_CHECK (&entry, RECV_VERSION))
                {
                    if (instance_entry.version == NETCFG_TYPE_RIP_GLOBAL_VERSION_BY_INTERFACE)
                        recv_version = SYS_DFLT_RIP_RECEIVE_PACKET_VERSION; /* NETCFG_TYPE_RIP_VERSION_VERSION_1_AND_2 */
                    else
                        recv_version = instance_entry.version;
                }
                /* If interface has RIP version configuration use it. */
                else
                {
                    recv_version = entry.recv_version_type;
                }

                if(recv_version == NETCFG_TYPE_RIP_VERSION_VERSION_1)
                {
                    sprintf(buff,"    Receive RIPv1 packets only\r\n");
                }
                else if(recv_version == NETCFG_TYPE_RIP_VERSION_VERSION_2)
                {
                    sprintf(buff,"    Receive RIPv2 packets only\r\n");
                }
                else if(recv_version == NETCFG_TYPE_RIP_VERSION_VERSION_1_AND_2)
                {
                    sprintf(buff,"    Receive RIPv1 and RIPv2 packets\r\n");
                }
            }
            else
            {
                sprintf(buff,"    Disable to receive RIP packets\r\n");
            }
            PROCESS_MORE(buff);

            if(entry.send_packet)
            {
                enum NETCFG_TYPE_RIP_Version_E send_version;
                /* If there is no version configuration in the interface,
                   use rip's version setting. */
                if (! RIP_IF_PARAM_CHECK (&entry, SEND_VERSION))
                {
                    if (instance_entry.version == NETCFG_TYPE_RIP_GLOBAL_VERSION_BY_INTERFACE)
                        send_version = SYS_DFLT_RIP_SEND_PACKET_VERSION; /* NETCFG_TYPE_RIP_VERSION_VERSION_1_COMPATIBLE */
                    else
                        send_version = instance_entry.version;
                }
                /* If interface has RIP version configuration use it. */
                else
                {
                    send_version = entry.send_version_type;
                }

                if(send_version == NETCFG_TYPE_RIP_VERSION_VERSION_1)
                {
                    sprintf(buff,"    Send RIPv1 packets only\r\n");
                }
                else if(send_version == NETCFG_TYPE_RIP_VERSION_VERSION_2)
                {
                    sprintf(buff,"    Send RIPv2 packets only\r\n");
                }
                else if(send_version == NETCFG_TYPE_RIP_VERSION_VERSION_1_COMPATIBLE)
                {
                    sprintf(buff,"    Send RIPv1 Compatible\r\n");
                }
            }
            else
            {
                sprintf(buff,"    Disable to send RIP packets\r\n");
            }
            PROCESS_MORE(buff);

            sprintf(buff,"    Passive interface: %s\r\n", (entry.pass_if)? "Enabled" : "Disabled");
            PROCESS_MORE(buff);

            if(entry.auth_mode == NETCFG_TYPE_RIP_AUTH_SIMPLE_PASSWORD)
            {
                sprintf(buff,"    Authentication mode: Text\r\n");
            }
            else if(entry.auth_mode == NETCFG_TYPE_RIP_AUTH_MD5)
            {
                sprintf(buff,"    Authentication mode: Md5\r\n");
            }
            else if(entry.auth_mode == NETCFG_TYPE_RIP_NO_AUTH)
            {
                sprintf(buff,"    Authentication mode: (None)\r\n");
            }
            PROCESS_MORE(buff);

            if(*entry.auth_str)
            {
                sprintf(buff,"    Authentication string: %s\r\n", entry.auth_str);
            }
            else
            {
                sprintf(buff,"    Authentication string: (None)\r\n");
            }
            PROCESS_MORE(buff);


            if(entry.split_horizon == NETCFG_TYPE_RIP_SPLIT_HORIZON)
            {
                sprintf(buff,"    Split horizon: Enabled\r\n");
            }
            else if(entry.split_horizon == NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED)
            {
                sprintf(buff,"    Split horizon: Enabled with Poisoned Reverse\r\n");
            }
            else if(entry.split_horizon == NETCFG_TYPE_RIP_SPLIT_HORIZON_NONE)
            {
                sprintf(buff,"    Split horizon: Disabled\r\n");
            }
            PROCESS_MORE(buff);

            memset(&inet_addr, 0, sizeof(inet_addr));
            inet_addr.type = L_INET_ADDR_TYPE_IPV4;
            while(NETCFG_TYPE_OK == NETCFG_PMGR_RIP_GetNextActiveRifByVlanIfIndex(entry.ifindex, &inet_addr))
            {
                char buf[L_INET_MAX_IP4ADDR_STR_LEN+1] = {0};
                L_INET_InaddrToString((L_INET_Addr_T *)&inet_addr, buf, sizeof(buf));
                sprintf(buff,"    IP interface address: %s\r\n", buf);
                PROCESS_MORE(buff);
            }

        }
        else
        {
            sprintf(buff,"  RIP is not enabled on this interface\r\n");
            PROCESS_MORE(buff);
        }
    }
#endif
    return CLI_NO_ERROR;
}
/* show ip protocols [bgp| ospf| rip] */
UI32_T CLI_API_L3_Show_Ip_Protocols(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_BGP == TRUE || SYS_CPNT_OSPF == TRUE || SYS_CPNT_RIP == TRUE)
    UI32_T line_num = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
#endif

    if(arg[0] == NULL)
    {
#if (SYS_CPNT_BGP == TRUE)
        show_ip_protocols_bgp(&line_num);
        PROCESS_MORE("\r\n");
#endif /* #if (SYS_CPNT_BGP == TRUE) */

#if (SYS_CPNT_OSPF == TRUE)
        show_ip_protocols_ospf(&line_num);
        PROCESS_MORE("\r\n");
#endif
#if (SYS_CPNT_RIP == TRUE)
        show_ip_protocols_rip(&line_num);
        PROCESS_MORE("\r\n");
#endif
        return CLI_NO_ERROR;
    }
    else if(!strncmp(arg[0], "bgp", 3))
    {
#if (SYS_CPNT_BGP == TRUE)
        show_ip_protocols_bgp(&line_num);
#endif /* #if (SYS_CPNT_BGP == TRUE) */
    }
    else if(!strncmp(arg[0], "ospf", 4))
    {
#if (SYS_CPNT_OSPF == TRUE)
        show_ip_protocols_ospf(&line_num);
#endif
    }
    else if(!strncmp(arg[0], "rip", 3))
    {
#if (SYS_CPNT_RIP == TRUE)
        show_ip_protocols_rip(&line_num);
#endif
    }
    return CLI_NO_ERROR;

}

UI32_T show_ip_protocols_rip(UI32_T *line_num_p)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T nexttime;
    UI32_T line_num = 0;
    BOOL_T in_flag = FALSE;
    BOOL_T out_flag = FALSE;
    char ifname[10] = {0};
    char recv_version[4];
    char send_version[13];
    UI8_T addr[SYS_ADPT_IPV4_ADDR_LEN];
    char route_addr[20]= {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T buff_len;
    NETCFG_TYPE_RIP_If_T entry;
    NETCFG_TYPE_RIP_Instance_T instance_entry;
    NETCFG_TYPE_RIP_Peer_Entry_T peer_entry;
    NETCFG_TYPE_RIP_Network_T network_entry;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
    memset(&instance_entry, 0, sizeof(NETCFG_TYPE_RIP_Instance_T));
    memset(&peer_entry, 0, sizeof(NETCFG_TYPE_RIP_Peer_Entry_T));
    memset(&network_entry, 0, sizeof(NETCFG_TYPE_RIP_Network_T));

    line_num = *line_num_p;
    if(NETCFG_PMGR_RIP_GetInstanceEntry(&instance_entry) == NETCFG_TYPE_OK)
    {

        sprintf(buff,"Routing Protocol is \"rip\"\r\n");
        PROCESS_MORE(buff);

        buff_len = 0;
        buff_len += sprintf(buff,"  Sending updates every %ld seconds with +/-5 seconds", (long)instance_entry.timer.update);
        if(NETCFG_PMGR_RIP_GetNextThreadTimer(&nexttime))
        {
            buff_len += sprintf(buff + buff_len,", next due in %ld seconds\r\n", (long)nexttime);
        }
        else
        {
            buff_len += sprintf(buff + buff_len,"\r\n");
        }
        PROCESS_MORE(buff);

        sprintf(buff,"  Timeout after %ld seconds, garbage collect after %ld seconds\r\n", (long)instance_entry.timer.timeout,(long)instance_entry.timer.garbage);
        PROCESS_MORE(buff);
        while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
        {
            if((*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN]) ||
               (*entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN]))
            {
                in_flag = TRUE;
            }

            if((*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT]) ||
               (*entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT]))
            {
                out_flag = TRUE;
            }

            if(!in_flag && !out_flag)
            {
                break;
            }
        }

        if(out_flag)
        {
            sprintf(buff,"  Outgoing update filter list for all interface is set\r\n");
        }
        else
        {
            sprintf(buff,"  Outgoing update filter list for all interface is not set\r\n");
        }
        PROCESS_MORE(buff);

        memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
        while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
        {
            if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT] ||
               *entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT])
            {
                buff_len = 0;
                CLI_API_L3_GetIfnameFromIfindex(entry.ifindex, ifname);
                buff_len += sprintf(buff,"  %s filter by ", ifname);
                if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT] &&
                   *entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT])
                {
                    buff_len += sprintf(buff + buff_len,"  %s, (prefix-list) %s",
                                                        entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT],
                                                        entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT]);
                }
                else if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT])
                {
                    buff_len += sprintf(buff + buff_len,"  %s", entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT]);
                }
                else if(*entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT])
                {
                    buff_len += sprintf(buff + buff_len,"  (prefix-list) %s", entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_OUT]);
                }
                buff_len += sprintf(buff + buff_len,"\r\n");
                PROCESS_MORE(buff);
            }
        }

        if(in_flag)
        {
            sprintf(buff,"  Incoming update filter list for all interface is set\r\n");
        }
        else
        {
            sprintf(buff,"  Incoming update filter list for all interface is not set\r\n");
        }
        PROCESS_MORE(buff);
        memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
        while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
        {
            if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN] ||
               *entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN])
            {
                buff_len = 0;
                CLI_API_L3_GetIfnameFromIfindex(entry.ifindex, ifname);
                buff_len += sprintf(buff,"  %s filter by ", ifname);
                if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN] &&
                   *entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN])
                {
                    buff_len += sprintf(buff + buff_len,"  %s, (prefix-list) %s",
                                                        entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN],
                                                        entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN]);
                }
                else if(*entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN])
                {
                    buff_len += sprintf(buff + buff_len,"  %s", entry.distribute_table.acl_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN]);
                }
                else if(*entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN])
                {
                    buff_len += sprintf(buff + buff_len,"  (prefix-list) %s", entry.distribute_table.pre_list[NETCFG_TYPE_RIP_DISTRIBUTE_IN]);
                }
                buff_len += sprintf(buff + buff_len,"\r\n");
                PROCESS_MORE(buff);
            }
        }

        sprintf(buff,"  Default redistribution metric is %ld\r\n", (long)instance_entry.default_metric);
        PROCESS_MORE(buff);
        buff_len = 0;
        buff_len += sprintf(buff,"  Redistributing:");
        if(instance_entry.redistribute[NETCFG_TYPE_RIP_Redistribute_Connected] != NULL)
        {
            buff_len += sprintf(buff + buff_len," Connected ");
        }
        if(instance_entry.redistribute[NETCFG_TYPE_RIP_Redistribute_Static] != NULL)
        {
            buff_len += sprintf(buff + buff_len," Static ");
        }
        if(instance_entry.redistribute[NETCFG_TYPE_RIP_Redistribute_Ospf] != NULL)
        {
            buff_len += sprintf(buff + buff_len," OSPF ");
        }
        if(instance_entry.redistribute[NETCFG_TYPE_RIP_Redistribute_Bgp] != NULL)
        {
            buff_len += sprintf(buff + buff_len," BGP ");
        }
        buff_len += sprintf(buff + buff_len,"\r\n");
        PROCESS_MORE(buff);

        if(instance_entry.version == NETCFG_TYPE_RIP_GLOBAL_VERSION_BY_INTERFACE)
        {
            sprintf(buff,"  Default version control: send version by interface set,receive version by interface set\r\n");
        }
        else
        {
            sprintf(buff,"  Default version control: send version %d,receive version %d \r\n", instance_entry.version, instance_entry.version);
        }
        PROCESS_MORE(buff);

        sprintf(buff,"    Interface  Send         Recv\r\n");
        PROCESS_MORE(buff);
        memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
        while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
        {
            CLI_API_L3_GetIfnameFromIfindex(entry.ifindex, ifname);

            if(entry.send_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_1)
            {
                strcpy(send_version,"1");
            }
            else if(entry.send_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_2)
            {
                strcpy(send_version,"2");
            }
            else if(entry.send_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_1_COMPATIBLE)
            {
                strcpy(send_version,"1-compatible");
            }

            if(entry.recv_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_1)
            {
                strcpy(recv_version,"1");
            }
            else if(entry.recv_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_2)
            {
                strcpy(recv_version,"2");
            }
            else if(entry.recv_version_type == NETCFG_TYPE_RIP_VERSION_VERSION_1_AND_2)
            {
                strcpy(recv_version,"1 2");
            }
            sprintf(buff,"    %-11s%-13s%-4s\r\n", ifname, send_version, recv_version);
            PROCESS_MORE(buff);
        }
        sprintf(buff,"  Routing for Networks:\r\n");
        PROCESS_MORE(buff);
        while(NETCFG_PMGR_RIP_GetNextNetworkTable(&network_entry) == NETCFG_TYPE_OK)
        {
            memcpy(addr, &network_entry.ip_addr, sizeof(UI32_T));
            sprintf(buff,"    %d.%d.%d.%d/%ld\r\n", addr[0],addr[1], addr[2], addr[3],(long)network_entry.pfxlen);
            PROCESS_MORE(buff);
        }
        memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_If_T));
        while(NETCFG_PMGR_RIP_GetNextInterfaceEntry(&entry) == NETCFG_TYPE_OK)
        {
            if(entry.network_if)
            {
                CLI_API_L3_GetIfnameFromIfindex(entry.ifindex, ifname);
                sprintf(buff,"    %s\r\n", ifname);
                PROCESS_MORE(buff);
            }

        }
        memset(&network_entry, 0, sizeof(NETCFG_TYPE_RIP_Network_T));
        while(NETCFG_PMGR_RIP_GetNextNeighborTable(&network_entry) == NETCFG_TYPE_OK)
        {
            memset(addr, 0, sizeof(addr));
            memcpy(addr, &network_entry.ip_addr, sizeof(UI32_T));
            sprintf(buff,"    %d.%d.%d.%d\r\n", addr[0],addr[1], addr[2], addr[3]);
            PROCESS_MORE(buff);
        }

        sprintf(buff,"  Routing Information Sources:\r\n");
        PROCESS_MORE(buff);

        sprintf(buff,"    Gateway         Distance  Last Update  Bad Packets  Bad Routes\r\n");
        PROCESS_MORE(buff);
        while(NETCFG_PMGR_RIP_GetNextPeerEntry(&peer_entry))
        {
            memset(addr, 0, sizeof(addr));
            memcpy(addr, &peer_entry.peer_addr, sizeof(UI32_T));
            sprintf(route_addr,"%d.%d.%d.%d", addr[0],addr[1], addr[2], addr[3]);
            sprintf(buff,"    %-15s %-8ld  %-11s  %-11d  %-10d\r\n", route_addr,(long)instance_entry.distance, peer_entry.timebuf,peer_entry.recv_badpackets, peer_entry.recv_badroutes);
            PROCESS_MORE(buff);
        }

        sprintf(buff,"  The maximum number of RIP routes allowed: %ld\r\n", (long)instance_entry.pmax);
        PROCESS_MORE(buff);

        sprintf(buff,"  Distance: Default is %ld\r\n", (long)instance_entry.distance);
        PROCESS_MORE(buff);
    }

    *line_num_p = line_num;
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Default_Metric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T default_metric;

    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTER_CMD_W1_DEFAULTMETRIC:

         default_metric = atoi(arg[0]);

         if(NETCFG_PMGR_RIP_DefaultMetricSet(default_metric)!= NETCFG_TYPE_OK)
         {
             CLI_LIB_PrintStr("The RIP default metric setting is failed.\r\n");
         }

         break;

         case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_DEFAULTMETRIC:

         if( NETCFG_PMGR_RIP_DefaultMetricUnset()!= NETCFG_TYPE_OK)
         {
             CLI_LIB_PrintStr("The RIP system default metric setting is failed.\r\n");
         }

         break;

         default:
         break;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Distribute_List(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_RIP == TRUE)
      char *list_name = NULL;
      UI32_T type;
      UI32_T list_type;
      char *ifname = NULL;
      if(!strcmp(arg[0], "prefix"))
      {
          list_type = NETCFG_TYPE_RIP_DISTRIBUTE_PREFIX_LIST;
          list_name = arg[1];
          if(!strcmp(arg[2], "in"))
          {
             type = NETCFG_TYPE_RIP_DISTRIBUTE_IN;
          }
          else if(!strcmp(arg[2], "out"))
          {
             type = NETCFG_TYPE_RIP_DISTRIBUTE_OUT;
          }
          ifname = arg[3];
      }
      else
      {
          list_type = NETCFG_TYPE_RIP_DISTRIBUTE_ACCESS_LIST;
          list_name = arg[0];
          if(!strcmp(arg[1], "in"))
          {
             type = NETCFG_TYPE_RIP_DISTRIBUTE_IN;
          }
          else if(!strcmp(arg[1], "out"))
          {
            type = NETCFG_TYPE_RIP_DISTRIBUTE_OUT;
          }
          ifname =arg[2];
      }

      switch(cmd_idx)
      {
      case PRIVILEGE_CFG_ROUTER_CMD_W1_DISTRIBUTELIST:
        if( NETCFG_PMGR_RIP_DistributeListAdd(ifname, list_name,type,list_type)!= NETCFG_TYPE_OK)
        {

          CLI_LIB_PrintStr("The RIP distribute list  is failed.\r\n");
        }
      break;

      case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_DISTRIBUTELIST:
        if( NETCFG_PMGR_RIP_DistributeListDelete(ifname, list_name,type,list_type)!= NETCFG_TYPE_OK)
        {
          CLI_LIB_PrintStr("The RIP delete distribute list  is failed.\r\n");
        }
      break;

      default:
      break;
      }
#endif
      return CLI_NO_ERROR;
#endif /* vai comment-out temporarily for ES3628BT-FLF-ZZ-00634 */
    return CLI_ERR_CMD_NOT_IMPLEMENT;
}

UI32_T CLI_API_L3_Rip_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
      UI32_T metric;

      switch(cmd_idx)
      {
          case PRIVILEGE_CFG_ROUTER_CMD_W1_REDISTRIBUTE:
          if(arg[1] != NULL)
          {
                if(!strncmp("connected", arg[0], strlen(arg[0])) ||
                    !strncmp("ospf", arg[0], strlen(arg[0])) ||
                    !strncmp("static", arg[0], strlen(arg[0])) ||
                    !strncmp("bgp", arg[0], strlen(arg[0])))
                {
                      if(!strncmp("metric", arg[1], strlen(arg[1])))
                      {
                          metric = atoi(arg[2]);
                          if (metric < 0 || metric > NETCFG_TYPE_RIP_INFINITY_METRIC)
                          {
                            CLI_LIB_PrintStr("Invalid metric value\r\n");
                            return CLI_NO_ERROR;
                          }

                          if(arg[3] != NULL)
                          {
                            if (NULL == arg[4])
                            {
                                CLI_LIB_PrintStr("Invalid route-map string\r\n");
                                return CLI_NO_ERROR;
                            }
                            return CLI_ERR_CMD_NOT_IMPLEMENT;
#if 0
                               if(!strncmp("route-map", arg[3], strlen(arg[3]))
                               {
                                 if (NULL != arg[4])
                                    rmap = arg[4];
                                 NETCFG_PMGR_RIP_RedistributeAllSet(arg[0],metric, rmap);
                               }
#endif /* vai comment-out temporarily */
                          }
                          else
                          {
                            NETCFG_PMGR_RIP_RedistributeMetricSet(arg[0], metric);
                          }

                      }
                      else if(!strncmp("route-map", arg[1], strlen(arg[1])))
                      {
                        if (NULL == arg[2])
                        {
                            CLI_LIB_PrintStr("Invalid route-map string\r\n");
                            return CLI_NO_ERROR;
                        }
                        return CLI_ERR_CMD_NOT_IMPLEMENT;
#if 0
                         NETCFG_PMGR_RIP_RedistributeRmapSet(arg[0], arg[2]);
#endif /* vai comment-out temporarily */
                      }


                }
          }
          else
          {
            NETCFG_PMGR_RIP_RedistributeSet(arg[0]);
          }

          break;

      case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_REDISTRIBUTE:
        if( NETCFG_PMGR_RIP_RedistributeUnset(arg[0])!= NETCFG_TYPE_OK)
        {
          CLI_LIB_PrintStr("The RIP default redistribute list is failed.\r\n");
        }

      break;

        default:
        return CLI_NO_ERROR;
      }
#endif
      return CLI_NO_ERROR;

}


UI32_T CLI_API_L3_Rip_Maximum_Prefix(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T pmax = 0;
    if(arg[0] != NULL)
    {
        pmax = atoi(arg[0]);
    }
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_ROUTER_CMD_W1_MAXIMUMPREFIX:
          if(NETCFG_PMGR_RIP_MaxPrefixSet(pmax)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP maximum prefix setting is failed.\r\n");
          }
        break;

        case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_MAXIMUMPREFIX:
          if( NETCFG_PMGR_RIP_MaxPrefixUnset()!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP default maximum prefix setting is failed.\r\n");
          }
        break;

        default:
        return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Timers(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T update;
    UI32_T timeout;
    UI32_T garbage;
    NETCFG_TYPE_RIP_Timer_T timer;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_ROUTER_CMD_W2_TIMERS_BASIC:
        update = atoi(arg[0]);
        timeout = atoi(arg[1]);
        garbage = atoi(arg[2]);
        timer.timeout = timeout;
        timer.update = update;
        timer.garbage = garbage;

        if( NETCFG_PMGR_RIP_TimerSet(&timer)!= NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("The RIP timer setting is failed.\r\n");
        }
        break;

        case PRIVILEGE_CFG_ROUTER_CMD_W3_NO_TIMERS_BASIC:

        if( NETCFG_PMGR_RIP_TimerUnset()!= NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("The RIP timer setting is failed.\r\n");
        }
        break;

        default:
        return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T version;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_ROUTER_CMD_W1_VERSION:
          version = atoi(arg[0]);

          if(NETCFG_PMGR_RIP_VersionSet(version)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP version setting is failed.\r\n");
          }
        break;

        case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_VERSION:
          if(  NETCFG_PMGR_RIP_VersionUnset()!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP default version setting is failed.\r\n");
          }
        break;

        default:
        return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Authentication_Mode(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T mode = NETCFG_TYPE_RIP_AUTH_MD5;
    if(arg[0] != NULL )
    {

        if( (arg[0][0] == 'm') || (arg[0][0] == 'M'))
        {
            mode = NETCFG_TYPE_RIP_AUTH_MD5;
        }
        else if( (arg[0][0] == 't') || (arg[0][0] == 'T'))
        {
            mode = NETCFG_TYPE_RIP_AUTH_SIMPLE_PASSWORD;
        }
    }
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_RIP_AUTHENTICATION_MODE:



          if(NETCFG_PMGR_RIP_AuthModeSet(ctrl_P->CMenu.vlan_ifindex, mode)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP authentication mode MD5 is failed.\r\n");
          }


        break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_RIP_AUTHENTICATION_MODE:


          if( NETCFG_PMGR_RIP_AuthModeUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP default authentication mode MD5 is failed.\r\n");
          }

        break;

        default:
        return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Rip_Authentication_String(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_RIP_AUTHENTICATION_STRING:
          if (strlen(arg[0]) > NETCFG_TYPE_RIP_AUTH_STRING_LENGTH)
          {
            CLI_LIB_PrintStr("Invalid authentication string length.\r\n");
            return CLI_NO_ERROR;
          }

          if(NETCFG_PMGR_RIP_AuthStringSet(ctrl_P->CMenu.vlan_ifindex, arg[0])!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP authentication string  is failed.\r\n");
          }


        break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_RIP_AUTHENTICATION_STRING:
          if( NETCFG_PMGR_RIP_AuthStringUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP default authentication string is failed.\r\n");
          }

        break;

        default:
        return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Rip_Receive_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    #if (SYS_CPNT_RIP == TRUE)
     UI32_T type ;

     switch(cmd_idx)
     {
     case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_RIP_RECEIVE_VERSION:
         if(arg[1] != NULL)
         {
            type = NETCFG_TYPE_RIP_VERSION_VERSION_1_AND_2;
         }
         else if(arg[0] != NULL)
         {
             if(!strncmp("1", arg[0], strlen(arg[0])))
             {
                type = NETCFG_TYPE_RIP_VERSION_VERSION_1;
             }
             else if(!strncmp("2", arg[0], strlen(arg[0])))
             {
                type = NETCFG_TYPE_RIP_VERSION_VERSION_2;
             }
             else
             {
                 CLI_LIB_PrintStr("The RIP receive version parameter setting  is error.\r\n");
                  return CLI_ERR_CMD_INVALID;
             }
         }
         else
         {
             CLI_LIB_PrintStr("The RIP receive version parameter setting  is error.\r\n");
              return CLI_ERR_CMD_INVALID;
         }

       if(NETCFG_PMGR_RIP_RecvVersionTypeSet(ctrl_P->CMenu.vlan_ifindex,  type)!= NETCFG_TYPE_OK)
       {
           CLI_LIB_PrintStr("The RIP receive version setting  is failed.\r\n");
       }
     break;

     case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_RIP_RECEIVE_VERSION:
       if(NETCFG_PMGR_RIP_RecvVersionUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
       {
           CLI_LIB_PrintStr("The RIP disable receive version setting is failed.\r\n");
       }
     break;

     default:
     break;
     }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Rip_Receive_Packet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_RIP_RECEIVEPACKET:
          if(NETCFG_PMGR_RIP_RecvPacketSet(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP receive packet  is failed.\r\n");
          }
        break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_RIP_RECEIVEPACKET:
          if(NETCFG_PMGR_RIP_RecvPacketUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
          {
              CLI_LIB_PrintStr("The RIP disable receive packet is failed.\r\n");
          }
        break;

        default:
        return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_RipSend_Version(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T type = NETCFG_TYPE_RIP_VERSION_UNSPEC;
    if(arg[0] != NULL)
    {
        if(!strncmp("1", arg[0], strlen(arg[0])))
        {
            type = NETCFG_TYPE_RIP_VERSION_VERSION_1;
        }
        else if(!strncmp("2", arg[0], strlen(arg[0])))
        {
            type = NETCFG_TYPE_RIP_VERSION_VERSION_2;
        }
        else if(!strncmp("1-compatible", arg[0], strlen(arg[0])))
        {
            type = NETCFG_TYPE_RIP_VERSION_VERSION_1_COMPATIBLE;
        }
        else
        {
             CLI_LIB_PrintStr("The RIP send version parameter setting  is error.\r\n");
             return CLI_ERR_CMD_INVALID;
        }
    }
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_RIP_SEND_VERSION:
      if(NETCFG_PMGR_RIP_SendVersionTypeSet(ctrl_P->CMenu.vlan_ifindex,  type)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP send version setting  is failed.\r\n");
      }


    break;

    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_RIP_SEND_VERSION:


      if(NETCFG_PMGR_RIP_SendVersionUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP disable send version setting is failed.\r\n");
      }

    break;

    default:
    return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Send_Packet(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_RIP_SENDPACKET:



      if(NETCFG_PMGR_RIP_SendPacketSet(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP receive packet  is failed.\r\n");
      }


    break;

    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_RIP_SENDPACKET:
      if(NETCFG_PMGR_RIP_SendPacketUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP disable receive packet is failed.\r\n");
      }

    break;

    default:
    return CLI_ERR_CMD_INVALID;
    }
#endif
    return CLI_NO_ERROR;

}
UI32_T CLI_API_L3_Rip_Split_Horizon(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T type;
    type = NETCFG_TYPE_RIP_SPLIT_HORIZON;

    if(arg[0] != NULL)
    {
        if( (arg[0][0] == 'p') || (arg[0][0] == 'P'))
        {
            type = NETCFG_TYPE_RIP_SPLIT_HORIZON_POISONED;
        }
    }
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_IP_RIP_SPLITHORIZON:


      if(NETCFG_PMGR_RIP_SplitHorizonSet(ctrl_P->CMenu.vlan_ifindex,  type)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP split horizon set is failed.\r\n");
      }


    break;

    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_NO_IP_RIP_SPLITHORIZON:


      if(NETCFG_PMGR_RIP_SplitHorizonUnset(ctrl_P->CMenu.vlan_ifindex)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP default split horizon is failed.\r\n");
      }


    break;

    default:
    return CLI_ERR_CMD_INVALID;
    }


#endif
    return CLI_NO_ERROR;

}


UI32_T CLI_API_L3_Rip_Passive_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T  vid=0;
    vid = CLI_LIB_AtoUl(arg[1],10);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_ROUTER_CMD_W1_PASSIVEINTERFACE:

      if(NETCFG_PMGR_RIP_PassiveIfAdd(vid)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP Passive If Address setting is failed.\r\n");
      }


    break;

    case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_PASSIVEINTERFACE:


      if( NETCFG_PMGR_RIP_PassiveIfDelete(vid)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP delete passive If Address setting is failed.\r\n");
      }

    break;

    default:
    return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Rip_Neighbor(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T  neighbor = 0;
    UI8_T addr[SYS_ADPT_IPV4_ADDR_LEN]= {0};

    CLI_LIB_AtoIp(arg[0], addr);
    memcpy(&neighbor, addr, sizeof(addr));

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_ROUTER_CMD_W1_NEIGHBOR:

      if( NETCFG_PMGR_RIP_NeighborSet(neighbor)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP  Neighbor Ip Address setting is failed.\r\n");
      }


    break;

    case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_NEIGHBOR:


      if(NETCFG_PMGR_RIP_NeighborUnset(neighbor)!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The RIP delete  Neighbor Ip Address setting is failed.\r\n");
      }

    break;

    default:
    return CLI_ERR_CMD_INVALID;
    }

#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Rip_Global_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_TYPE_RIP_Packet_Debug_Type_T type;
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_DEBUG_RIP:

       if(!strcmp(arg[0],"all"))
       {
            if( NETCFG_PMGR_RIP_ConfigDebug()!= TRUE)
            {
                CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
            }
       }
       if(!strcmp(arg[0],"events"))
       {

           if( NETCFG_PMGR_RIP_ConfigDebugEvent()!= TRUE)
           {
                CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
           }
       }

       else if(!strcmp(arg[0],"packet"))
       {

            if(arg[1]== NULL)
            {
                type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE;
            }
            else
            {
                if(!strcmp(arg[1],"detail"))
                {
                    type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL;
                }
                else if(!strcmp(arg[1],"recv"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE;
                    }
                }
                else if(!strcmp(arg[1],"send"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND;
                    }
                }
            }

            if( NETCFG_PMGR_RIP_ConfigDebugPacket(type)!= TRUE)
            {
                CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
            }
       }
       else if(!strcmp(arg[0],"route"))
       {
           if( NETCFG_PMGR_RIP_ConfigDebugNsm( )!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
           }
       }
    break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_DEBUG_RIP:

       if(!strcmp(arg[0],"all"))
       {
           if( NETCFG_PMGR_RIP_ConfigUnDebug()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
           }
       }
       if(!strcmp(arg[0],"events"))
       {

           if( NETCFG_PMGR_RIP_ConfigUnDebugEvent()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
           }
       }

       else if(!strcmp(arg[0],"packet"))
       {

            if(arg[1]== NULL)
            {
                type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE;
            }
            else
            {
                if(!strcmp(arg[1],"detail"))
                {
                    type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL;
                }
                else if(!strcmp(arg[1],"recv"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE;
                    }
                }
                else if(!strcmp(arg[1],"send"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND;
                    }
                }
            }

            if( NETCFG_PMGR_RIP_ConfigUnDebugPacket(type)!= TRUE)
            {
                CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
            }
       }
       else if(!strcmp(arg[0],"route"))
       {
           if( NETCFG_PMGR_RIP_ConfigUnDebugNsm( )!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP debugging fail.\r\n");
           }
       }
    break;
    default:
    return CLI_ERR_CMD_INVALID;
    }

#endif
#endif
  return CLI_NO_ERROR;
}
UI32_T CLI_API_L3_Rip_Exec_Debug(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
# if 0
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_TYPE_RIP_Packet_Debug_Type_T type;

    switch(cmd_idx)
    {
    case PRIVILEGE_EXEC_CMD_W2_DEBUG_RIP:

       if(!strcmp(arg[0],"all"))
       {
           if( NETCFG_PMGR_RIP_Debug()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }
       if(!strcmp(arg[0],"events"))
       {

           if( NETCFG_PMGR_RIP_DebugEvent()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }

       else if(!strcmp(arg[0],"packet"))
       {

            if(arg[1]== NULL)
            {
                type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE;
            }
            else
            {
                if(!strcmp(arg[1],"detail"))
                {
                    type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL;
                }
                else if(!strcmp(arg[1],"recv"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE;
                    }
                }
                else if(!strcmp(arg[1],"send"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND;
                    }
                }
            }

            if( NETCFG_PMGR_RIP_DebugPacket(type)!= TRUE)
            {
                CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
            }
       }
       else if(!strcmp(arg[0],"route"))
       {
           if( NETCFG_PMGR_RIP_DebugNsm( )!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }
    break;

    case PRIVILEGE_EXEC_CMD_W3_NO_DEBUG_RIP:

       if(!strcmp(arg[0],"all"))
       {
           if( NETCFG_PMGR_RIP_UnDebug()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }
       if(!strcmp(arg[0],"events"))
       {

           if( NETCFG_PMGR_RIP_UnDebugEvent()!= TRUE)
           {
           CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }

       else if(!strcmp(arg[0],"packet"))
       {

            if(arg[1]== NULL)
            {
                type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE;
            }
            else
            {
                if(!strcmp(arg[1],"detail"))
                {
                    type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_DETAIL;
                }
                else if(!strcmp(arg[1],"recv"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVEANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_RECEIVE;
                    }
                }
                else if(!strcmp(arg[1],"send"))
                {
                    if(arg[2]!= NULL)
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SENDANDDETAIL;
                    }
                    else
                    {
                        type = NETCFG_TYPE_RIP_PACKET_DEBUG_TYPE_SEND;
                    }
                }
            }

            if( NETCFG_PMGR_RIP_UnDebugPacket(type)!= TRUE)
            {
                CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
            }
       }
       else if(!strcmp(arg[0],"route"))
       {
           if( NETCFG_PMGR_RIP_UnDebugNsm( )!= TRUE)
           {
                CLI_LIB_PrintStr("The RIP  Exec debugging fail.\r\n");
           }
       }
    break;
    default:
    return CLI_ERR_CMD_INVALID;
    }

#endif
#endif
  return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Clear_Ip_Rip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    char str[24] = {0};

    if(arg[1] != NULL)
    {
        CLI_API_L3_GetIpPrefixString(arg[0], arg[1], str);
    }
    else
    {
        strcpy(str, arg[0]);
    }

    if(NETCFG_PMGR_RIP_ClearRoute(str) != NETCFG_TYPE_OK)
    {
        CLI_LIB_PrintStr("Clear RIP route failed.\r\n");
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Rip_Distance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    UI32_T distance,plen,count,i;
    UI8_T addr_str[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    UI32_T addr;
    if(arg[1] != NULL)
    {
        count = 0;
        distance = atoi(arg[0]);
        CLI_LIB_AtoIp(arg[1],addr_str);
        memcpy(&addr, addr_str, sizeof(UI32_T));
        memset(addr_str, 0, sizeof(addr_str));
        CLI_LIB_AtoIp(arg[2],addr_str);
        memcpy(&plen, addr_str, sizeof(UI32_T));
        for(i=0;i<32;i++)
        {
            if(plen &(1<<i))
            {
                count++;
            }
        }

        switch(cmd_idx)
        {
       case PRIVILEGE_CFG_ROUTER_CMD_W1_DISTANCE:
            if(NETCFG_PMGR_RIP_DistanceSet(distance, addr, count, arg[3])!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Rip distance Setting is failed.\r\n");
            }

            break;

            case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_DISTANCE:
            if( NETCFG_PMGR_RIP_DistanceUnset(addr, count)!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Rip default distance Setting is failed.\r\n");
            }
            break;

            default:
            return CLI_NO_ERROR;
        }
    }
    else
    {
        distance = atoi(arg[0]);
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_ROUTER_CMD_W1_DISTANCE:
            if(NETCFG_PMGR_RIP_DistanceDefaultSet(distance)!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Rip distance Setting is failed.\r\n");
            }

            break;

            case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_DISTANCE:
            if( NETCFG_PMGR_RIP_DistanceDefaultUnset()!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Rip default distance Setting is failed.\r\n");
            }
            break;

            default:
            return CLI_NO_ERROR;
        }
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Rip_Originate(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_ROUTER_CMD_W2_DEFAULTINFORMATION_ORIGINATE:

      if(NETCFG_PMGR_RIP_DefaultAdd()!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The Router Rip originate Setting is failed.\r\n");
      }

    break;

    case PRIVILEGE_CFG_ROUTER_CMD_W3_NO_DEFAULTINFORMATION_ORIGINATE:


      if( NETCFG_PMGR_RIP_DefaultDelete()!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The Router Rip Setting is failed.\r\n");
      }

    break;

    default:
    return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;

}

UI32_T CLI_API_L3_Show_Rip_Debugging(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_RIP == TRUE)
    NETCFG_TYPE_RIP_Debug_Status_T status;
    memset(&status, 0, sizeof(status));

    NETCFG_PMGR_RIP_GetDebugStatus(&status);
    CLI_LIB_PrintStr("RIP debugging status:\n");

    if(status.event_all_status)
    {
        CLI_LIB_PrintStr("  RIP event debugging is on\n");
    }

    if(status.packet_all_status)
    {
        if((status.packet_send_status) && (status.packet_recv_status))
        {
            if(status.packet_detail_status)
            {
                CLI_LIB_PrintStr("  RIP packet detail debugging is on\n");
            }
            else
            {
                CLI_LIB_PrintStr("  RIP packet debugging is on\n");
            }
        }
        else if(status.packet_send_status)
        {
            if(status.packet_detail_status)
            {
                CLI_LIB_PrintStr("  RIP packet send detail debugging is on\n");
            }
            else
            {
                CLI_LIB_PrintStr("  RIP packet send debugging is on\n");
            }
        }
        else if(status.packet_recv_status)
        {
            if(status.packet_detail_status)
            {
                CLI_LIB_PrintStr("  RIP packet receive detail debugging is on\n");
            }
            else
            {
                CLI_LIB_PrintStr("  RIP packet receive debugging is on\n");
            }
        }
    }

    if(status.nsm_all_status)
    {
        CLI_LIB_PrintStr("  RIP route debugging is on\n");
    }
    CLI_LIB_PrintStr("\n");
#endif
#endif
    return CLI_NO_ERROR;
}

/*Lin.Li, for porting RIP end*/

/*Lin.Li, for ARP porting, modify start*/
/* command: show arp */

UI32_T CLI_API_L3_Show_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;

   NETCFG_TYPE_IpNetToMediaEntry_T  entry;

   char  type[30] = {0};
   UI32_T vid = 0;
   UI32_T time_out_value = 0;
   UI8_T ip_str[18] = {0};
   UI32_T count = 0;

   memset(&entry, 0, sizeof(NETCFG_TYPE_IpNetToMediaEntry_T));

   if(NETCFG_POM_ND_GetIpNetToMediaTimeout(&time_out_value) != FALSE )
   {
      CLI_LIB_PrintStr_1("ARP Cache Timeout: %lu (seconds)\r\n", (unsigned long)time_out_value);
   }
   sprintf(buff,"\r\nIP Address      MAC Address       Type      Interface\r\n");
   PROCESS_MORE(buff);
   sprintf(buff,"--------------- ----------------- --------- -----------\r\n");
   PROCESS_MORE(buff);


    while (NETCFG_PMGR_ND_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
   //while (NETCFG_PMGR_ARP_GetNextIpNetToMediaEntry(&entry) == NETCFG_TYPE_OK)
   {

      L_INET_Ntoa(entry.ip_net_to_media_net_address,ip_str);
      VLAN_OM_ConvertFromIfindex(entry.ip_net_to_media_if_index, &vid);

      switch(entry.ip_net_to_media_type)
      {
         case VAL_ipNetToMediaType_other:
         strcpy(type,"other");
         break;

         case VAL_ipNetToMediaType_invalid:
         strcpy(type,"invalid");
         break;

         case VAL_ipNetToMediaType_dynamic:
         strcpy(type,"dynamic");
         break;

         case VAL_ipNetToMediaType_static:
         strcpy(type,"static");
         break;

         default:
         strcpy(type,"other");
         break;
      }

      sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-9s VLAN%-4lu\r\n",
              ip_str,
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[0],
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[1],
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[2],
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[3],
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[4],
              entry.ip_net_to_media_phys_address.phy_address_cctet_string[5],
              type,(unsigned long)vid);
      PROCESS_MORE(buff);
      count++;
   }

   sprintf(buff, "\r\nTotal entry : %lu\r\n",(unsigned long)count);
   PROCESS_MORE(buff);
   return CLI_NO_ERROR;

}

/* command: clear arp-cache */

UI32_T CLI_API_L3_Clear_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   char choice[2] = {0};

   CLI_LIB_PrintStr("This operation will delete all the dynamic entries in ARP Cache.\r\n");
   CLI_LIB_PrintStr("Do you want to continue this operation (y/n)?");
   CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
   CLI_LIB_PrintNullStr(1);

   if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
      return CLI_NO_ERROR;

   if ( NETCFG_PMGR_ND_DeleteAllDynamicIpv4NetToMediaEntry()!= NETCFG_TYPE_OK)
   {
      CLI_LIB_PrintStr("Failed flush the dynamic ARP Cache.\r\n");
   }
   return CLI_NO_ERROR;
}
/*Lin.Li, for ARP porting, modify end*/

UI32_T CLI_API_L3_Show_Ip_Host_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_IPV4_ROUTING == TRUE)
    /* UI32_T total_ip_host_route; */
    AMTRL3_TYPE_InetHostRouteEntry_T ip_host_route_entry;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    char  ip_str[L_INET_MAX_IPADDR_STR_LEN+1] = {0};
    UI32_T line_num = 0;
    UI32_T vid;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;
    UI32_T port_type = SWCTRL_LPORT_UNKNOWN_PORT;

#if 0
    total_ip_host_route = AMTRL3_POM_GetTotalHostRouteNumber(AMTRL3_TYPE_FLAGS_IPV4, SYS_ADPT_DEFAULT_FIB);
    sprintf((char*)buff, " Total count: %lu\r\n", total_ip_host_route);
    PROCESS_MORE(buff);
#endif

    sprintf((char*)buff, " IP Address      MAC Address       VLAN Port\r\n");
    PROCESS_MORE(buff);

    sprintf((char*)buff, " --------------- ----------------- ---- -------\r\n");
    PROCESS_MORE(buff);

    memset (&ip_host_route_entry, 0, sizeof(AMTRL3_TYPE_InetHostRouteEntry_T));
    ip_host_route_entry.dst_inet_addr.type    = L_INET_ADDR_TYPE_IPV4;
    ip_host_route_entry.dst_inet_addr.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
    while( AMTRL3_POM_GetNextInetHostRouteEntry(AMTRL3_TYPE_FLAGS_IPV4,
                                                SYS_ADPT_DEFAULT_FIB,
                                                &ip_host_route_entry) )
    {
        if (FALSE == VLAN_OM_ConvertFromIfindex(ip_host_route_entry.dst_vid_ifindex, &vid))
            continue;

        L_INET_InaddrToString((L_INET_Addr_T*)&(ip_host_route_entry.dst_inet_addr), ip_str, sizeof(ip_str));

        if (ip_host_route_entry.lport != 0)
        {
            port_type = SWCTRL_POM_LogicalPortToUserPort(ip_host_route_entry.lport,&unit,&port,&trunk_id);
            if(SWCTRL_LPORT_TRUNK_PORT == port_type)
            {
                sprintf((char*)buff, " %-15s %02X-%02X-%02X-%02X-%02X-%02X %4lu Trunk%lu\r\n",
                        ip_str,
                        ip_host_route_entry.dst_mac[0],ip_host_route_entry.dst_mac[1],ip_host_route_entry.dst_mac[2],ip_host_route_entry.dst_mac[3],ip_host_route_entry.dst_mac[4],ip_host_route_entry.dst_mac[5],
                        (unsigned long)vid,(unsigned long)trunk_id);
                PROCESS_MORE(buff);
            }
            else if (SWCTRL_LPORT_NORMAL_PORT == port_type)
            {
                sprintf((char*)buff, " %-15s %02X-%02X-%02X-%02X-%02X-%02X %4lu %lu/%lu\r\n",
                        ip_str,
                        ip_host_route_entry.dst_mac[0],ip_host_route_entry.dst_mac[1],ip_host_route_entry.dst_mac[2],ip_host_route_entry.dst_mac[3],ip_host_route_entry.dst_mac[4],ip_host_route_entry.dst_mac[5],
                        (unsigned long)vid,(unsigned long)unit,(unsigned long)port);
                PROCESS_MORE(buff);
            }
        }
#if 0
        else
        {
            sprintf((char*)buff, " %-15s %02X-%02X-%02X-%02X-%02X-%02X %4lu Local\r\n",
                    ip_str,
                    ip_host_route_entry.dst_mac[0],ip_host_route_entry.dst_mac[1],ip_host_route_entry.dst_mac[2],ip_host_route_entry.dst_mac[3],ip_host_route_entry.dst_mac[4],ip_host_route_entry.dst_mac[5],
                    vid);
            PROCESS_MORE(buff);
        }
#endif
    }
    CLI_LIB_PrintStr("\r\n");

#endif /* (SYS_CPNT_IPV4_ROUTING == TRUE) */

    return CLI_NO_ERROR;
}

/* command: clear ip route */
UI32_T CLI_API_L3_Clear_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0//rich
   UI32_T ip_addr = 0;
   UI32_T netmask = 0;

   if(arg[0][0] == '*')
   {
      UI8_T  choice[2] = {0};

      CLI_LIB_PrintStr("This operation will delete all the dynamic entries in routing table.\r\n");
      CLI_LIB_PrintStr("Are you sure to continue this operation (y/n)?");
      CLI_PARS_ReadLine( choice, sizeof(choice), TRUE, FALSE);
      CLI_LIB_PrintNullStr(1);

      if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
         return CLI_NO_ERROR;

      if(NETCFG_PMGR_DeleteAllDynamicRoutes()!=NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to clear all the dynamic entries in routing table.\r\n");
      }
   }
   else
   {
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);

      if(arg[1]==0)
      {
         if(NETCFG_PMGR_DeleteDynamicRoute(ip_addr,0)!=NETCFG_MGR_OK)
         {
            CLI_LIB_PrintStr("Failed to clear the dynamic entries.\r\n");
         }
      }
      else
      {
         CLI_LIB_AtoIp(arg[1], (UI8_T*)&netmask);
         if(NETCFG_PMGR_DeleteDynamicRoute(ip_addr,netmask)!=NETCFG_MGR_OK)
         {
            CLI_LIB_PrintStr("Failed to clear the dynamic entries.\r\n");
         }
      }
   }
#endif
   return CLI_NO_ERROR;
}


#if (SYS_CPNT_NSM == TRUE)
/* command: show ip route */
static const UI8_T *CLI_API_IP_RouteTypeStr(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV4_LOCAL:
            return (UI8_T *)"Connected";
        case NSM_MGR_ROUTE_TYPE_IPV4_STATIC:
            return (UI8_T *)"Static";
        case NSM_MGR_ROUTE_TYPE_IPV4_RIP:
            return (UI8_T *)"Rip";
        case NSM_MGR_ROUTE_TYPE_IPV4_OSPF:
            return (UI8_T *)"Ospf";
        case NSM_MGR_ROUTE_TYPE_IPV4_BGP:
            return (UI8_T *)"Bgp";
        case NSM_MGR_ROUTE_TYPE_IPV4_ISIS:
            return (UI8_T *)"Isis";
        default:
            return (UI8_T *)"Unknown";
    }

    return (UI8_T *)"Unknown";
}
#endif

#if 0
static const UI8_T *CLI_API_IP_RouteSubTypeStr(NSM_MGR_RouteSubType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_IA:
            return (UI8_T *)"IA";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_1:
            return (UI8_T *)"N1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_NSSA_2:
            return (UI8_T *)"N2";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_1:
            return (UI8_T *)"E1";
        case NSM_MGR_ROUTE_SUBTYPE_OSPF_EXTERNAL_2:
            return (UI8_T *)"E2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L1:
            return (UI8_T *)"L1";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_L2:
            return (UI8_T *)"L2";
        case NSM_MGR_ROUTE_SUBTYPE_ISIS_IA:
            return (UI8_T *)"ia";
        case NSM_MGR_ROUTE_SUBTYPE_BGP_MPLS:
        default:
            return (UI8_T *)"  ";
    }

    return (UI8_T *)"  ";
}


static const UI8_T *CLI_API_IP_RouteTypeChar(NSM_MGR_RouteType_T type)
{
    switch (type)
    {
        case NSM_MGR_ROUTE_TYPE_IPV4_LOCAL:
            return (UI8_T *)"C";
        case NSM_MGR_ROUTE_TYPE_IPV4_STATIC:
            return (UI8_T *)"S";
        case NSM_MGR_ROUTE_TYPE_IPV4_RIP:
            return (UI8_T *)"R";
        case NSM_MGR_ROUTE_TYPE_IPV4_OSPF:
            return (UI8_T *)"O";
        case NSM_MGR_ROUTE_TYPE_IPV4_BGP:
            return (UI8_T *)"B";
        case NSM_MGR_ROUTE_TYPE_IPV4_ISIS:
            return (UI8_T *)"i";
        default:
            return (UI8_T *)"?";
    }

    return (UI8_T *)"?";
}
#endif

#if (SYS_CPNT_NSM == TRUE)
static void CLI_API_Show_Ip_Route_Summary(void)
{
    NSM_MGR_RouteType_T type;
    UI32_T total = 0;
    UI32_T number;
    UI32_T fib_number = 0;
    UI8_T  multipath_num = 0;

    if (NSM_TYPE_RESULT_OK == NSM_PMGR_GetRouteNumber(NSM_MGR_ROUTE_TYPE_IPV4_ALL, &total, &fib_number, &multipath_num))
    {
        CLI_LIB_PrintStr("IP routing table name is Default-IP-Routing-Table(0)\r\n");
        CLI_LIB_PrintStr_1("IP routing table maximum-paths is %d\r\n", multipath_num);

        for (type = NSM_MGR_ROUTE_TYPE_IPV4_LOCAL; type <= NSM_MGR_ROUTE_TYPE_IPV4_ISIS; type++)
        {
            number = 0;
            NSM_PMGR_GetRouteNumber(type, &number, &fib_number, &multipath_num);
            if (number > 0)
            {
                CLI_LIB_PrintStr_2("%-15s %ld\r\n", CLI_API_IP_RouteTypeStr(type), (long)number);
            }
        }

        CLI_LIB_PrintStr_2("%-15s %ld\r\n", "Total",(long)total);
    }

    return;
}
#endif

#if 0
static void CLI_API_Show_Ip_Route_Detail(NSM_MGR_RouteType_T type, BOOL_T database)
{
    NSM_MGR_GetNextRouteEntry_T entry;
    char tmp_str[CLI_DEF_MAX_BUFSIZE];
    UI8_T zero_bytes[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    int len;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    BOOL_T first;
    BOOL_T candidate_default;
    UI32_T ret;

    memset(&entry, 0, sizeof(NSM_MGR_GetNextRouteEntry_T));

    for(ret=NSM_PMGR_GetNextIpv4Route(&entry);TRUE;ret=NSM_PMGR_GetNextIpv4Route(&entry))
    {
        UI8_T idx;

        /* Filter type */
        if (type != NSM_MGR_ROUTE_TYPE_IPV4_ALL)
        {
            if (entry.data.ip_route_type != type)
            {
                if (ret == NSM_TYPE_RESULT_OK)
                    continue;
                else if (ret == NSM_TYPE_RESULT_EOF)
                    break;

                break;
            }
        }

        if (ret == NSM_TYPE_RESULT_OK || ret == NSM_TYPE_RESULT_EOF)
        {
            first = TRUE;
            candidate_default = FALSE;

            for(idx=0; idx<entry.data.num_of_next_hop; idx++)
            {
                /* Display forwarding table information only unless database
                 * option is specified.
                 */
                if ((database == FALSE) &&
                    (!(entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)))
                {
                    continue;
                }

                if (TRUE == first)
                {
                    if ((memcmp(entry.data.ip_route_dest.addr, zero_bytes, SYS_ADPT_IPV4_ADDR_LEN)==0) &&
                            (entry.data.ip_route_dest_prefix_len == 0) &&
                            (database == FALSE))
                    {
                        candidate_default = TRUE;
                    }

                    CLI_LIB_PrintStr_3("%s%c%s ",
                                    CLI_API_IP_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IP_RouteSubTypeStr(entry.data.ip_route_subtype));
                    len = SYSFUN_Sprintf((char *)tmp_str, "%s%c%s ",
                                    CLI_API_IP_RouteTypeChar(entry.data.ip_route_type),
                                    candidate_default ? '*' : ' ',
                                    CLI_API_IP_RouteSubTypeStr(entry.data.ip_route_subtype));

                    if (database == TRUE)
                    {
                        CLI_LIB_PrintStr_3("%c%c%c",
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                        len += SYSFUN_Sprintf((char *)tmp_str, "%c%c%c",
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                    }
                    else
                    {
                        CLI_LIB_PrintStr("   ");
                        len += 3;
                    }
                    CLI_LIB_PrintStr_4("%d.%d.%d.%d/",
                                entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1],
                                entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3]);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%d.%d.%d.%d/",
                                entry.data.ip_route_dest.addr[0], entry.data.ip_route_dest.addr[1],
                                entry.data.ip_route_dest.addr[2], entry.data.ip_route_dest.addr[3]);

                    CLI_LIB_PrintStr_1("%d", entry.data.ip_route_dest_prefix_len);
                    len += SYSFUN_Sprintf((char *)tmp_str, "%d", entry.data.ip_route_dest_prefix_len);

                    first = FALSE;
                }
                else
                {
                    if (database == TRUE)
                    {
                        CLI_LIB_PrintStr_3("     %c%c%c",
                            (entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_FIB)
                            ? '*' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_SELECTED)
                            ? '>' : ' ',
                            (entry.data.flags & NSM_TYPE_ROUTE_STALE)
                            ? 'p' : ' ');
                    }
                    else
                    {
                        CLI_LIB_PrintStr("        ");
                    }

                    SYSFUN_Sprintf((char *)tmp_str, "%*c", len - 8, ' ');
                    CLI_LIB_PrintStr(tmp_str);
                }

                if (entry.data.ip_route_type != NSM_MGR_ROUTE_TYPE_IPV4_LOCAL)
                    CLI_LIB_PrintStr_2("[%d/%ld]", entry.data.distance, entry.data.metric);

                switch (entry.data.next_hop_type[idx])
                {
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4_IFNAME:
                    case NSM_TYPE_NEXTHOP_TYPE_IPV4_IFINDEX:
                        CLI_LIB_PrintStr_4(" via %d.%d.%d.%d",
                                entry.data.ip_next_hop[idx].addr[0], entry.data.ip_next_hop[idx].addr[1],
                                entry.data.ip_next_hop[idx].addr[2], entry.data.ip_next_hop[idx].addr[3]);

                        if (entry.data.flags & NSM_TYPE_ROUTE_BLACKHOLE)
                            CLI_LIB_PrintStr(", Null0");
                        else if (entry.data.next_hop_ifname[idx][0] != '\0')
                        {
                            CLI_LIB_PrintStr_1(", %s", entry.data.next_hop_ifname[idx]);
                            entry.data.next_hop_ifname[idx][0] = '\0';
                        }
                        else if (entry.data.next_hop_ifindex[idx])
                        {
                            CLI_LIB_PrintStr_1(", %s", entry.data.next_hop_ifname[idx]);
                        }

                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFINDEX:
                        CLI_LIB_PrintStr_1(" is directly connected, %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    case NSM_TYPE_NEXTHOP_TYPE_IFNAME:
                        CLI_LIB_PrintStr_1(" is directly connected, %s", entry.data.next_hop_ifname[idx]);
                        entry.data.next_hop_ifname[idx][0] = '\0';
                        break;

                    default:
                        break;
                }

                if ((entry.data.next_hop_flags[idx] & NSM_TYPE_NEXTHOP_FLAG_ACTIVE) !=
                                        NSM_TYPE_NEXTHOP_FLAG_ACTIVE)
                {
                    CLI_LIB_PrintStr(" inactive");
                }

                CLI_LIB_PrintStr("\n");
            }
        }

        if(ret != NSM_TYPE_RESULT_OK)
            break;
        PROCESS_MORE(buff);
    }

    return;
}
#endif

#if (SYS_CPNT_NSM == TRUE)
static void CLI_API_L3_Show_Ip_Route_Buffer(NSM_MGR_RouteType_T type, BOOL_T database)
{
    UI32_T line_num = 0;
    UI32_T start_line = 0;
    UI32_T max_fetch_line;
    UI32_T fetched_line_count = 0;
    UI8_T buff[NSM_MGR_CLI_GET_NEXT_N_IPV4_ROUTE_BUFFER_SIZE] = {0};
    UI32_T ret;

    if (NSM_PMGR_PrefetchIpv4Route(type, database) == FALSE)
        return;

    /* if the return value is NSM_TYPE_RESULT_INSSUFICIENT_BUFFER_SIZE, means
     * NSM_MGR_CLI_GET_NEXT_N_IPV4_ROUTE_BUFFER_SIZE is not enough to store max_fetch_line routes,
     * but we can still display the first fetched_line_count routes and then fetchs the remains
     */
    max_fetch_line = CLI_LIB_GetMaxTerminalLineNum()-1;
    while((NSM_TYPE_RESULT_OK == (ret = NSM_PMGR_CLIGetNextNIpv4Route(start_line, buff, max_fetch_line, &fetched_line_count)))
          ||(NSM_TYPE_RESULT_INSSUFICIENT_BUFFER_SIZE == ret))
    {
        if (fetched_line_count == 0)
            break;

        CLI_LIB_PrintLongStr((char *)buff);

        start_line += fetched_line_count;

        /* Aux. PROCESS_MORE */
        line_num += fetched_line_count;
        if (CLI_API_Get_Print_Interactive_Mode() == TRUE)
        {
            if (line_num >= CLI_LIB_GetMaxTerminalLineNum()-1)
            {
                switch(CLI_LIB_ProcMoreFeature())
                {
                    case 1:
                        line_num = 0;
                        break;
                    case 2:
                        line_num--;
                        break;
                    case 4:
                        CLI_API_Set_Print_Interactive_Mode(FALSE);
                        line_num = 0;
                        break;
                    default:
                        goto clean_up;
                }
            }

            max_fetch_line = CLI_LIB_GetMaxTerminalLineNum() - line_num - 1;
        }
        else
        {
            max_fetch_line = CLI_LIB_GetMaxTerminalLineNum() - 1;
        }

        buff[0] = '\0';
        fetched_line_count = 0;
    }

clean_up:
    NSM_PMGR_PostfetchIpv4Route();

    return;
}
#endif

UI32_T CLI_API_L3_Show_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_NSM == TRUE)
    char show_header[] =
        "Codes: C - connected, S - static, R - RIP, B - BGP\r\n"
        "       O - OSPF, IA - OSPF inter area\r\n"
        "       N1 - OSPF NSSA external type 1, N2 - OSPF NSSA external type 2\r\n"
        "       E1 - OSPF external type 1, E2 - OSPF external type 2\r\n"
        "       i - IS-IS, L1 - IS-IS level-1, L2 - IS-IS level-2,"
        " ia - IS-IS inter area\r\n";
    /* Normal case.  */
    char header_normal[] =
        "       * - candidate default\r\n\r\n";

    /* Database case.  */
    char header_database[] =
        "       > - selected route, * - FIB route, p - stale info\r\n\r\n";

    BOOL_T database = FALSE;
    NSM_MGR_RouteType_T type = NSM_MGR_ROUTE_TYPE_IPV4_ALL;
    BOOL_T summary = FALSE;

    if (arg[0] != NULL)
    {
        switch(*arg[0])
        {
            case 'd':
            case 'D':
                database = TRUE;
                break;
            case 'b':
            case 'B':
                type = NSM_MGR_ROUTE_TYPE_IPV4_BGP;
                break;
            case 'c':
            case 'C':
                type = NSM_MGR_ROUTE_TYPE_IPV4_LOCAL;
                break;

            case 'r':
            case 'R':
                type = NSM_MGR_ROUTE_TYPE_IPV4_RIP;
                break;

            case 'o':
            case 'O':
                type = NSM_MGR_ROUTE_TYPE_IPV4_OSPF;
                break;

            case 's':
            case 'S':
                if ((arg[0][1] == 't') || (arg[0][1] == 'T'))
                    type = NSM_MGR_ROUTE_TYPE_IPV4_STATIC;
                else if ((arg[0][1] == 'u') || (arg[0][1] == 'U'))
                    summary = TRUE;
                else
                {
                    return CLI_ERR_INTERNAL;
                }
                break;

            default:
                break;
        }
    }

    if (summary == TRUE)
    {
        CLI_API_Show_Ip_Route_Summary();
        return CLI_NO_ERROR;
    }

    CLI_LIB_PrintStr(show_header);
    if (database == TRUE)
        CLI_LIB_PrintStr(header_database);
    else
        CLI_LIB_PrintStr(header_normal);

    CLI_API_L3_Show_Ip_Route_Buffer(type, database);
#endif

    return CLI_NO_ERROR;
}

/*Lin.Li, for ARP porting, modify start*/
/* command : arp timeout */
UI32_T CLI_API_L3_Arp_Timeout(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_ARP_SUPPORT_TIMEOUT == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_ARP_TIMEOUT:
        if(NETCFG_PMGR_ND_SetIpNetToMediaTimeout(atoi((char *)arg[0]))!=NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to configure the timeout.\r\n");
            return CLI_NO_ERROR;
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ARP_TIMEOUT:

        if(NETCFG_PMGR_ND_SetIpNetToMediaTimeout(SYS_DFLT_IP_NET_TO_MEDIA_ENTRY_TIMEOUT)!=NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to set the timeout to default.\r\n");
            return CLI_NO_ERROR;
        }
        break;
    }
#endif
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */
    return CLI_NO_ERROR;
}
/*Lin.Li, for ARP porting, modify end*/

/* command : ip routing */
#if 0
UI32_T CLI_API_L3_Ip_Routing(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    //UI32_T vlan_status;
    UI32_T ret = 0;
    UI32_T vr_id  = 0;
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_ROUTING:
/* peter_yu, for compile
#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
            if( SWCTRL_PMGR_GetRunningPrivateVlanStatus(&vlan_status) != SYS_TYPE_GET_RUNNING_CFG_FAIL)
            {
                if(vlan_status==TRUE)
                {
                    CLI_LIB_PrintStr("Pvlan is enabled. Please disable pvlan first.\r\n");
                    return CLI_NO_ERROR;
                }
            }
#endif
*/
            ret = NETCFG_PMGR_ROUTE_EnableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV4);
            switch(ret)
            {
                case NETCFG_TYPE_OK:
                case NETCFG_TYPE_NO_CHANGE:
                    break;
                default:
                    CLI_LIB_PrintStr("Failed to enable the IP routing process.\r\n");
                    break;
            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_ROUTING:
            ret = NETCFG_PMGR_ROUTE_DisableIpForwarding(vr_id, L_INET_ADDR_TYPE_IPV4);

            switch(ret)
            {
                case NETCFG_TYPE_OK:
                case NETCFG_TYPE_NO_CHANGE:
                    break;
                default:
                    CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
                    break;
            }

            break;

        default:
            break;
        }
    return CLI_NO_ERROR;
}

/* command: ip route */
#endif

/* command: ip sw-route*/
UI32_T CLI_API_L3_Ip_SW_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)
    UI32_T ret = 0;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_SWROUTE:
            ret = NETCFG_PMGR_ROUTE_EnableSWRoute(TRUE);
         break;

       case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_SWROUTE:
            ret = NETCFG_PMGR_ROUTE_EnableSWRoute(FALSE);
         break;
    }

    if (ret == NETCFG_TYPE_SW_ROUTE_NOT_NEED)
        CLI_LIB_PrintStr("The model doesn't need this command\r\n");
#endif /*#if (SYS_CPNT_STATIC_ROUTE_AND_METER_WORKAROUND == TRUE)*/

    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ip_Route(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0 /* Through FRRouting to set route.*/
#if (SYS_CPNT_IPV4_ROUTING == TRUE)
    UI32_T              action_flags;
    UI32_T              distance    = SYS_ADPT_MIN_ROUTE_DISTANCE;
    UI32_T              return_type = 0;
    UI8_T               dst_ipAddress[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T               netmask_Address[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    UI8_T               gateway_Address[SYS_ADPT_IPV4_ADDR_LEN] = {0};
    char                choice[2] = {0};
    BOOL_T              all_network = TRUE;
    L_INET_AddrIp_T     dest, next_hop;
    UI32_T ret = 0;

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_ROUTE:
        ret = CLI_LIB_AtoIp(arg[0], dst_ipAddress);
        if(!ret)
        {
            CLI_LIB_PrintStr("Invalid destination address.\r\n");
            return CLI_NO_ERROR;
        }
        ret = CLI_LIB_AtoIp(arg[1], netmask_Address);
        if(!ret)
        {
            CLI_LIB_PrintStr("Invalid netmask.\r\n");
            return CLI_NO_ERROR;
        }
        ret = CLI_LIB_AtoIp(arg[2], gateway_Address);
        if(!ret)
        {
            CLI_LIB_PrintStr("Invalid gateway address.\r\n");
            return CLI_NO_ERROR;
        }
        memset(&dest, 0, sizeof(L_INET_AddrIp_T));
        memset(&next_hop, 0, sizeof(L_INET_AddrIp_T));

        dest.type = L_INET_ADDR_TYPE_IPV4;
        dest.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
        next_hop.type = L_INET_ADDR_TYPE_IPV4;
        next_hop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

        memcpy(dest.addr, dst_ipAddress, SYS_ADPT_IPV4_ADDR_LEN);
        dest.preflen = IP_LIB_MaskToCidr(netmask_Address);
        memcpy(next_hop.addr, gateway_Address, SYS_ADPT_IPV4_ADDR_LEN);

        if(((dest.preflen > 0)&&(IP_LIB_IsZeroNetwork(dest.addr) == TRUE))
        || (IP_LIB_IsIpInClassD(dest.addr) == TRUE)
        || (IP_LIB_IsIpInClassE(dest.addr) == TRUE)
        || (IP_LIB_IsLoopBackIp(dest.addr) == TRUE)
        || (IP_LIB_IsBroadcastIp(dest.addr) == TRUE)
        || (IP_LIB_IsMulticastIp(dest.addr) == TRUE)
        || (IP_LIB_IsTestingIp(dest.addr) == TRUE)
        )
        {
            CLI_LIB_PrintStr_1("Invalid Destination IP address - %s\r\n", arg[0]);
            return CLI_NO_ERROR;
        }

        if (IP_LIB_IsValidNetworkMask(netmask_Address) == FALSE)
        {
            CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
            return CLI_NO_ERROR;
        }

        if(IP_LIB_OK != IP_LIB_IsValidForRemoteIp(next_hop.addr))
        {
            CLI_LIB_PrintStr_1("Invalid Nexthop IP address - %s\r\n", arg[2]);
            return CLI_NO_ERROR;
        }

        if (arg[3] != NULL)
        {
            distance = CLI_LIB_AtoUl(arg[3], 10);
        }
        ret = NETCFG_PMGR_ROUTE_AddStaticRoute(&dest, &next_hop, distance);

        switch(ret)
        {
            case NETCFG_TYPE_OK:
                /* do nothing */
                break;
            case NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT:
                CLI_LIB_PrintStr("Failed. Due to the nexthop conflicts on normal interface and craft interface.\r\n");
                break;
            case NETCFG_TYPE_ERR_ROUTE_TWO_NEXTHOP_ON_CRAFT_INT_IS_NOT_ALLOWED:
                CLI_LIB_PrintStr("Failed. To have two nexthops on craft interface is not allowed.\r\n");
                break;
            case NETCFG_TYPE_FAIL:
            default:
                CLI_LIB_PrintStr("Failed to set static route.\r\n");
                break;

        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_ROUTE:

        if(arg[0][0] == '*')
        {
            CLI_LIB_PrintStr("This operation will delete all static entries in routing table.\r\n");
            CLI_LIB_PrintStr("Are you sure to continue this operation (y/n)?");
            CLI_PARS_ReadLine(choice, sizeof(choice), TRUE, FALSE);
            CLI_LIB_PrintNullStr(1);

            if( choice[0] == 0 || (choice[0] != 'y' && choice[0] != 'Y') )
                return CLI_NO_ERROR;

            //return_type = NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes();

            action_flags = L_INET_ADDR_TYPE_IPV4;
            return_type = NETCFG_PMGR_ROUTE_DeleteAllStaticIpCidrRoutes(action_flags);

            if ( return_type == NETCFG_TYPE_OK )
            {
                CLI_LIB_PrintStr("The all static entries have been cleared.\r\n");
            }
            else
            {
                CLI_LIB_PrintStr("Failed to clear all static entries.\r\n");
            }
        }
        else
        {
            ret = CLI_LIB_AtoIp(arg[0], dst_ipAddress);
            if(!ret)
            {
                CLI_LIB_PrintStr("Invalid destination address.\r\n");
                return CLI_NO_ERROR;
            }

            ret = CLI_LIB_AtoIp(arg[1], netmask_Address);
            if(!ret)
            {
                CLI_LIB_PrintStr("Invalid netmask.\r\n");
                return CLI_NO_ERROR;
            }
            if (arg[2] != NULL)
            {
                all_network = FALSE;
                ret = CLI_LIB_AtoIp(arg[2], gateway_Address);
                if(!ret)
                {
                    CLI_LIB_PrintStr("Invalid gateway address.\r\n");
                    return CLI_NO_ERROR;
                }
            }

            if (all_network == FALSE)
            {
                memset(&next_hop, 0, sizeof(L_INET_AddrIp_T));/* it must reset to zero, or it will has garbage data*/
                dest.type = L_INET_ADDR_TYPE_IPV4;
                dest.addrlen = next_hop.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
                next_hop.type = L_INET_ADDR_TYPE_IPV4;
                memcpy(dest.addr, dst_ipAddress, SYS_ADPT_IPV4_ADDR_LEN);
                dest.preflen = IP_LIB_MaskToCidr(netmask_Address);
                memcpy(next_hop.addr, gateway_Address, SYS_ADPT_IPV4_ADDR_LEN);


                if (NETCFG_PMGR_ROUTE_DeleteStaticRoute(&dest, &next_hop)!=NETCFG_TYPE_OK)
                {
                    CLI_LIB_PrintStr("Failed to clear the entry.\r\n");
                }
            }
            else if(NETCFG_PMGR_ROUTE_DeleteStaticRouteByNetwork(dst_ipAddress,netmask_Address)!=NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr( "Failed to clear the entries.\r\n");
                return CLI_NO_ERROR;
            }
        }
        break;

    default:
        return CLI_NO_ERROR;
    }
#endif /* #if (SYS_CPNT_IPV4_ROUTING == TRUE) */
#endif
    return CLI_NO_ERROR;
}


/*Lin.Li, for ARP porting, modify start*/
/* command: arp */

UI32_T CLI_API_L3_Arp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ARP == TRUE)
    UI32_T                      ipAddress = 0;
    UI32_T                      return_type = 0;
    L_INET_AddrIp_T             inetAddress;
    NETCFG_TYPE_PhysAddress_T   physAddress;

    CLI_LIB_AtoIp(arg[0], (UI8_T*)&ipAddress);
    inetAddress = CLI_API_L3_ComposeInetAddr(L_INET_ADDR_TYPE_IPV4,(UI8_T*)&ipAddress,0);

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W1_ARP:
        physAddress.phy_address_type = SN_LAN;
        physAddress.phy_address_len =  SYS_ADPT_MAC_ADDR_LEN;
        CLI_LIB_ValsInMac(arg[1], physAddress.phy_address_cctet_string);
        return_type = NETCFG_PMGR_ND_AddStaticIpNetToPhysicalEntry(0, &inetAddress, &physAddress);

        if(return_type == NETCFG_TYPE_OK)
            return CLI_NO_ERROR;
        else
        {
            switch(return_type)
            {
            case NETCFG_TYPE_CAN_NOT_ADD_LOCAL_IP:
                CLI_LIB_PrintStr("The added IP address is the same as local IP address; the add action is aborted.\r\n");
                return CLI_NO_ERROR;
                break;

            case NETCFG_TYPE_TABLE_FULL:
                CLI_LIB_PrintStr("The static ARP Cache is full; the add action is aborted.\r\n");
                return CLI_NO_ERROR;
                break;

            case NETCFG_TYPE_ENTRY_EXIST:
                CLI_LIB_PrintStr("The added IP address exists; the add action is aborted.\r\n");
                return CLI_NO_ERROR;
                break;

             case NETCFG_TYPE_INVALID_ARG:
                 CLI_LIB_PrintStr("The added IP address or physical address is invalid; the add action is aborted.\r\n");
                 return CLI_NO_ERROR;
                break;

            default:
                CLI_LIB_PrintStr("Failed to set the static ARP cache entry.\r\n");
                return CLI_NO_ERROR;
                break;
            }
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_ARP:
        return_type = NETCFG_PMGR_ND_DeleteStaticIpNetToPhysicalEntry(0, &inetAddress);

        if (return_type != NETCFG_TYPE_OK)
        {
            if (return_type == NETCFG_TYPE_ENTRY_NOT_EXIST)
            {
                CLI_LIB_PrintStr("The deleted static IP address does not exist; the delete action is aborted.\r\n");
                return CLI_NO_ERROR;
            }
            else if(return_type == NETCFG_TYPE_INVALID_ARG)
            {
                CLI_LIB_PrintStr("The deleted static IP address is invalid; the delete action is aborted.\r\n");
                return CLI_NO_ERROR;
            }
            else
            {
                CLI_LIB_PrintStr("Failed to delete this entry.\r\n");
                return CLI_NO_ERROR;
            }
        }
        break;

    default:
        break;
    }
#endif /* #if (SYS_CPNT_ARP == TRUE) */

    return CLI_NO_ERROR;
}
/*Lin.Li, for ARP porting, modify end*/


/* command: show ip rip */

UI32_T CLI_API_L3_Show_Ip_Rip(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_RIP == TRUE)
    NETCFG_TYPE_RIP_Route_T entry;
    char ifname[10];
    UI8_T addr[SYS_ADPT_IPV4_ADDR_LEN];
    char route_addr[20]= {0};
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    UI32_T buff_len = 0;

    memset(&entry, 0, sizeof(NETCFG_TYPE_RIP_Route_T));
    PROCESS_MORE("\r\n");
    sprintf(buff,"Codes: R - RIP, Rc - RIP connected, Rs - RIP static,\r\n");
    PROCESS_MORE(buff);

    sprintf(buff,"       C - Connected, S - Static, O - OSPF\r\n");
    PROCESS_MORE(buff);
    PROCESS_MORE("\r\n");

    sprintf(buff,"   Network            Next Hop        Metric From            Interface Time\r\n");
    PROCESS_MORE(buff);

    while (NETCFG_PMGR_RIP_GetNextRouteEntry(&entry) == NETCFG_TYPE_OK)
    {
        buff_len = 0;
        memset(addr, 0, sizeof(addr));
        memcpy(addr, &entry.dest_addr, sizeof(UI32_T));
        sprintf(route_addr,"%d.%d.%d.%d/%ld", addr[0],addr[1], addr[2], addr[3],(long)entry.dest_pfxlen);
        buff_len += sprintf(buff,"%-2s %-18s ", entry.type_str,route_addr);
        if(entry.nexthop_addr)
        {
            memset(addr, 0, sizeof(addr));
            memcpy(addr, &entry.nexthop_addr, sizeof(UI32_T));
            memset(route_addr, 0, sizeof(route_addr));
            sprintf(route_addr,"%d.%d.%d.%d", addr[0],addr[1], addr[2], addr[3]);
            buff_len += sprintf(buff + buff_len,"%-15s ", route_addr);
        }
        else
        {
            buff_len += sprintf(buff + buff_len,"%-15s ", "");
        }
        buff_len += sprintf(buff + buff_len,"%-6ld ", (long)entry.metric);

        if(entry.from_addr)
        {
            memset(addr, 0, sizeof(addr));
            memcpy(addr, &entry.from_addr, sizeof(UI32_T));
            memset(route_addr, 0, sizeof(route_addr));
            sprintf(route_addr,"%d.%d.%d.%d", addr[0],addr[1], addr[2], addr[3]);
            buff_len += sprintf(buff + buff_len,"%-15s ", route_addr);
        }
        else
        {
            buff_len += sprintf(buff + buff_len,"%-15s ", "");
        }

        CLI_API_L3_GetIfnameFromIfindex(entry.ifindex, ifname);
        buff_len += sprintf(buff + buff_len,"%-9s ", ifname);

        if(*entry.timebuf)
        {
            buff_len += sprintf(buff + buff_len,"%s\r\n", entry.timebuf);
        }
        else
        {
            buff_len += sprintf(buff + buff_len,"%s\r\n", "--");
        }
        PROCESS_MORE(buff);
    }

#endif
   return CLI_NO_ERROR;
}


/* command: router rip
enter into router rip configuration mode */
UI32_T CLI_API_L3_Router_Rip(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_RIP == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_ROUTER_RIP:

      if(NETCFG_PMGR_RIP_RouterRipSet()!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The Router Rip Setting is failed.\r\n");
      }
      else
      {
         ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_ROUTER_MODE;
      }
    break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ROUTER_RIP:


      if( NETCFG_PMGR_RIP_RouterRipUnset()!= NETCFG_TYPE_OK)
      {
          CLI_LIB_PrintStr("The Router Rip Setting is failed.\r\n");
      }

    break;

    default:
        return CLI_ERR_CMD_INVALID;
    }
    return CLI_NO_ERROR;

#endif
   return CLI_NO_ERROR;

}

/* command: router ospf
enter into router ospf configuration mode */
UI32_T CLI_API_L3_Router_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF== TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;

    if(arg[0] != NULL)
    {
        proc_id = atoi(arg[0]);

    }
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_ROUTER_OSPF:
            if(NETCFG_PMGR_OSPF_RouterOspfSet(vr_id, vrf_id, proc_id)!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Ospf Setting is failed.\r\n");
            }
            else
            {
                ctrl_P->CMenu.process_id = proc_id;
                ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_ROUTEROSPF_MODE;
            }
        break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ROUTER_OSPF:
            if( NETCFG_PMGR_OSPF_RouterOspfUnset(vr_id, vrf_id, proc_id)!= NETCFG_TYPE_OK)
            {
                CLI_LIB_PrintStr("The Router Ospf Setting is failed.\r\n");
            }

        break;

        default:
            return CLI_ERR_CMD_INVALID;
    }

#endif /* #if (SYS_CPNT_OSPF== TRUE) */
    return CLI_NO_ERROR;

}

/* command: ip proxy-arp */
UI32_T CLI_API_L3_Ip_Proxy(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_PROXY_ARP == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IP_PROXYARP:

        if (NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(ctrl_P->CMenu.vlan_ifindex, TRUE)!= NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to enable proxy ARP.\r\n");
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IP_PROXYARP:

        if (NETCFG_PMGR_IP_SetIpNetToMediaProxyStatus(ctrl_P->CMenu.vlan_ifindex, FALSE)!= NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
        }
        break;

    default:
        break;
    }
#endif /* #if (SYS_CPNT_PROXY_ARP == TRUE) */

    return CLI_NO_ERROR;
}

/* command: network */
UI32_T CLI_API_L3_Rip_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_RIP == TRUE)
    char str[24] = {0};

    UI32_T vid=0;
    L_PREFIX_T  network_address;

    if((arg[0][0] == 'v') || (arg[0][0] == 'V'))
    {
        /* network vlan x*/
        vid = CLI_LIB_AtoUl(arg[1],10);
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_ROUTER_CMD_W1_NETWORK:
                if( NETCFG_PMGR_RIP_NetworkSetByVid(vid)!= NETCFG_TYPE_OK)
                {
                  CLI_LIB_PrintStr("Failed to add the RIP network address.\r\n");
                }
            break;
            case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_NETWORK:

                if(NETCFG_PMGR_RIP_NetworkUnsetByVid(vid)!= NETCFG_TYPE_OK)
                {
                  CLI_LIB_PrintStr("Failed to delete the RIP network address.\r\n");
                }
            break;
            default:
                return CLI_NO_ERROR;
        }
    }
    else
    {
        /* network A.B.C.D A.B.C.D*/
        memset(&network_address, 0, sizeof(L_PREFIX_T));
        CLI_API_L3_GetIpPrefixString(arg[0],arg[1],str);
        L_PREFIX_Str2Prefix(str,&network_address);
        switch(cmd_idx)
        {
            case PRIVILEGE_CFG_ROUTER_CMD_W1_NETWORK:
                if( NETCFG_PMGR_RIP_NetworkSetByAddress(network_address)!= NETCFG_TYPE_OK)
                {
                  CLI_LIB_PrintStr("Failed to add the RIP network address.\r\n");
                }
            break;
            case PRIVILEGE_CFG_ROUTER_CMD_W2_NO_NETWORK:
                if(NETCFG_PMGR_RIP_NetworkUnsetByAddress(network_address)!= NETCFG_TYPE_OK)
                {
                  CLI_LIB_PrintStr("Failed to delete the RIP network address.\r\n");
                }
            break;
            default:
                return CLI_NO_ERROR;
        }
    }

#endif
   return CLI_NO_ERROR;
}

/* command: dhcp relay server */
UI32_T CLI_API_L3_DHCP_RELAY_SERVER(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_DHCP_RELAY == TRUE)
   UI32_T   ip_Address[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER]={0};
   UI32_T   number_of_relay_server=0;
   UI32_T   count;
   UI32_T   ret=0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W4_IP_DHCP_RELAY_SERVER:
      for (count=0;count<SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER;count++)
      {
          if(arg[count]!=NULL)
          {
              CLI_LIB_AtoIp(arg[count], (UI8_T*)&ip_Address[count]);
              number_of_relay_server++;
          }
          else
          {
              break;
          }
      }

      ret = DHCP_PMGR_AddInterfaceRelayServerAddress(ctrl_P->CMenu.vlan_ifindex,number_of_relay_server,ip_Address);

      switch(ret)
      {
        case DHCP_MGR_OK:
            break;
        case DHCP_MGR_NO_IP:
            CLI_LIB_PrintStr("The interface has not yet configured IP.\r\n");
            return CLI_NO_ERROR;
        case DHCP_MGR_DYNAMIC_IP:
            CLI_LIB_PrintStr("The interface has not yet configured IP.\r\n");
            return CLI_NO_ERROR;
        case DHCP_MGR_SERVER_ON:
            CLI_LIB_PrintStr("DHCP Server is running.\r\n");
            return CLI_NO_ERROR;
        default:
            CLI_LIB_PrintStr("Failed to set up IP operation for relay server.\r\n");
            return CLI_NO_ERROR;
      }

      DHCP_PMGR_Restart3(DHCP_TYPE_RESTART_RELAY);

      break;

   case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W5_NO_IP_DHCP_RELAY_SERVER:

      number_of_relay_server = 0;
      for (count=0;count<SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER;count++)
      {
          if(arg[count]!=NULL)
          {
              CLI_LIB_AtoIp(arg[count], (UI8_T*)&ip_Address[count]);
              number_of_relay_server++;
          }
          else
          {
              break;
          }
      }

      if (number_of_relay_server==0)
      {
          if (DHCP_PMGR_DeleteAllRelayServerAddress(ctrl_P->CMenu.vlan_ifindex)!=DHCP_PMGR_OK)
          {
              CLI_LIB_PrintStr("Failed to delete the DHCP Relay Server.\r\n");
          }
      }
      else
      {
          if( DHCP_PMGR_DeleteInterfaceRelayServerAddress(ctrl_P->CMenu.vlan_ifindex,number_of_relay_server,ip_Address)!=DHCP_PMGR_OK)
          {
              CLI_LIB_PrintStr("Failed to delete the DHCP Relay Server.\r\n");
          }
      }

      DHCP_PMGR_Restart3(DHCP_TYPE_RESTART_RELAY);
#if 0
      DHCP_PMGR_SetRestartObject(DHCP_MGR_RESTART_RELAY);
#endif
      break;

   }
#endif
   return CLI_NO_ERROR;
}


/* L2 ---------------------> L3 */

UI32_T CLI_API_IP_Dhcp_Restart(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   switch (cmd_idx)
   {
#if (SYS_CPNT_DHCP_CLIENT == TRUE)
   case PRIVILEGE_EXEC_CMD_W4_IP_DHCP_RESTART_CLIENT:
      DHCP_PMGR_Restart3(DHCP_MGR_RESTART_CLIENT);
      break;
#endif
#if (SYS_CPNT_DHCP_RELAY == TRUE)
   case PRIVILEGE_EXEC_CMD_W4_IP_DHCP_RESTART_RELAY:
      DHCP_PMGR_Restart3(DHCP_MGR_RESTART_RELAY);
      break;
#endif
   default:
      break;
   }

   return CLI_NO_ERROR;
}


/* command : ip address */
UI32_T CLI_API_Ip_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T ret;
    UI32_T vid;
    UI32_T uint_ipMask    = 0;
    UI32_T mode = 0;
    NETCFG_TYPE_InetRifConfig_T rif_config, old_rif;
    UI8_T  byte_mask[SYS_ADPT_IPV4_ADDR_LEN] = {};
    L_INET_AddrIp_T default_gateway;
    UI32_T old_addr_mode;
    BOOL_T has_default_gateway = FALSE;
    BOOL_T ip_addr_add_success = FALSE;
    BOOL_T has_old_rif         = FALSE;
    BOOL_T is_secondary = FALSE;
    BOOL_T is_virtual = FALSE;
    int i;

/*if support cfgdb ip address can not set when provision*/
#if (SYS_CPNT_CFGDB == TRUE)
#if (SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB == TRUE)
    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
    {
        return CLI_NO_ERROR;
    }
#endif
#endif

    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));

    VLAN_OM_ConvertFromIfindex(ctrl_P->CMenu.vlan_ifindex, &vid);

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IP_ADDRESS:

            if(arg[0]==NULL)
                return CLI_ERR_INTERNAL;

            switch(*arg[0])
            {
#if (SYS_CPNT_DHCP_CLIENT == TRUE)
                case 'd':
                case 'D':
                    if(NETCFG_PMGR_IP_SetIpAddressMode(ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_DHCP) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as DHCP mode on VLAN %lu\r\n", vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as DHCP mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    else
                    {

                        DHCP_PMGR_Restart();


                    }
                    break;
#endif

#if (SYS_CPNT_BOOTP == TRUE)
                case 'b':
                case 'B':
                    if(NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as BOOTP mode on VLAN %lu\r\n", vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as BOOTP mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    break;
#endif

                default:
                    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                    rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                    rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
                    rif_config.row_status = VAL_netConfigStatus_2_createAndGo;

                    for (i = 1; arg[i];)
                    {
                        if (strncmp(arg[i], "secondary", 3) == 0)
                        {
                            is_secondary = TRUE;
                        }
                        if (strncmp(arg[i], "virtual", 3) == 0)
                        {
                            is_virtual = TRUE;
                        }
                            i++;
                    }

                    if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
                    {
                        /* format: 192.168.1.1 255.255.255.0 */

                        /* convert to byte array format */
                        IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                        if(!IP_LIB_IsValidNetworkMask(byte_mask))
                        {
                            CLI_LIB_PrintStr_1("Invalid Netmask address - %s\r\n", arg[1]);
                            return CLI_NO_ERROR;
                        }

                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }

                        rif_config.addr.preflen = IP_LIB_MaskToCidr(byte_mask);

                    }
                    else
                    {
                        /* format: 192.168.1.1/24 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }

                        IP_LIB_CidrToMask(rif_config.addr.preflen, byte_mask);

                    }

                    if(is_secondary) /* secondary */
                    {
                        /* if primary rif is got by DHCP, it shouldn't set static secondary rif */
                        if(NETCFG_POM_IP_GetIpAddressMode(ctrl_P->CMenu.vlan_ifindex,&mode)!= NETCFG_TYPE_OK)
                        {
                           CLI_LIB_PrintStr_1("Failed to get the IP address mode on VLAN %lu\r\n", (unsigned long)vid);
                           return CLI_NO_ERROR;
                        }

                        if((mode != NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE))
                        {
                           CLI_LIB_PrintStr("Current IP address mode is DHCP\r\n");
                           return CLI_NO_ERROR;
                        }
                    }

                    ret = IP_LIB_IsValidForIpConfig(rif_config.addr.addr, byte_mask);
                    switch(ret)
                    {
                        case IP_LIB_INVALID_IP:
                        case IP_LIB_INVALID_IP_LOOPBACK_IP:
                        case IP_LIB_INVALID_IP_ZERO_NETWORK:
                        case IP_LIB_INVALID_IP_BROADCAST_IP:
                        case IP_LIB_INVALID_IP_IN_CLASS_D:
                        case IP_LIB_INVALID_IP_IN_CLASS_E:
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                            break;
                       case IP_LIB_INVALID_IP_SUBNET_NETWORK_ID:
                            CLI_LIB_PrintStr("Invalid IP address. Can't be network ID.\r\n");
                            return CLI_NO_ERROR;
                            break;
                        case IP_LIB_INVALID_IP_SUBNET_BROADCAST_ADDR:

                            CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast IP.\r\n");
                            return CLI_NO_ERROR;

                        default:
                            break;
                    }

                    for(i=1;arg[i];)
                    {
                        if (strncmp(arg[i], "default-gateway", 3) == 0)
                        {
                            if (arg[i+1] != NULL)
                            {
                                if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                                   arg[i+1],
                                                                                   (L_INET_Addr_T *)&default_gateway,
                                                                                   sizeof(default_gateway)))
                                {
                                    CLI_LIB_PrintStr_1("Invalid default gateway address - %s\r\n", arg[i+1]);
                                    return CLI_NO_ERROR;
                                }

                                if (!IP_LIB_IsIpBelongToSubnet(rif_config.addr.addr, rif_config.addr.preflen,
                                        default_gateway.addr))
                                {
                                    CLI_LIB_PrintStr("Default-gateway is not in subnet.\r\n");
                                    return CLI_NO_ERROR;
                                }

                                has_default_gateway = TRUE;

                                /* store old address mode and rif config in order to rollback when fail
                                 */
                                if (NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ctrl_P->CMenu.vlan_ifindex, &old_addr_mode))
                                {
                                    CLI_LIB_PrintStr_1("Failed to get the IP address mode on VLAN %lu\r\n", (unsigned long)vid);
                                    return CLI_NO_ERROR;
                                }
                                old_rif.ifindex = ctrl_P->CMenu.vlan_ifindex;
                                if (NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&old_rif))
                                    has_old_rif = TRUE;
                            }
                            i+=2;
                        }
                        else
                            i++;
                    }

                    if( NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", vid);
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid);
                    }
                    else
                    {
                        if (is_secondary)
                            rif_config.ipv4_role = NETCFG_TYPE_MODE_SECONDARY;

                        if (is_virtual)
                            rif_config.ipv4_role = NETCFG_TYPE_MODE_VIRTUAL;

                        ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                        switch(ret)
                        {
                            case NETCFG_TYPE_OK:
                                ip_addr_add_success = TRUE;
                                break;
                            case NETCFG_TYPE_MORE_THAN_TWO_OVERLAPPED:
                                CLI_LIB_PrintStr_1("Failed to set IP address on VLAN %lu. Overlapped with existing address.\r\n", (unsigned long)vid);
                                break;
                            default:
                                CLI_LIB_PrintStr_1("Failed to set IP address on VLAN %lu\r\n", (unsigned long)vid);
                                break;
                        }
                    }

                    if (ip_addr_add_success && has_default_gateway)
                    {
                        ret = NETCFG_PMGR_ROUTE_AddDefaultGateway(&default_gateway, SYS_DFLT_DEFAULT_GATEWAY_METRIC);
                        switch(ret)
                        {
                            case NETCFG_TYPE_OK:
                                /* do nothing */
                                break;
                            case NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT:
                                CLI_LIB_PrintStr("Failed to set default gateway. Nexthop conflicts on normal interface and craft interface.\r\n");
                                break;
                            case NETCFG_TYPE_ERR_ROUTE_TWO_NEXTHOP_ON_CRAFT_INT_IS_NOT_ALLOWED:
                                CLI_LIB_PrintStr("Failed to set default gateway. Two nexthops on craft interface is not allowed.\r\n");
                                break;
                            case NETCFG_TYPE_FAIL:
                            default:
                                CLI_LIB_PrintStr("Failed to set default gateway\r\n");
                                break;
                        }

                        /* Rollback old rif config when fail
                         */
                        if (ret != NETCFG_TYPE_OK)
                        {
                            NETCFG_PMGR_IP_SetIpAddressMode(ctrl_P->CMenu.vlan_ifindex, old_addr_mode);
                            if (old_addr_mode == NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE)
                            {
                                if(has_old_rif)
                                {
                                    old_rif.row_status = VAL_netConfigStatus_2_createAndGo;
                                    NETCFG_PMGR_IP_SetInetRif(&old_rif, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                                }
                                else
                                {
                                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
                                    NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);
                                }
                            }
                        }
                    }

                    break;
            }
            break;

        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IP_ADDRESS:

            if(arg[0] == NULL)                               /* no ip address */
            {

                memset(&rif_config, 0, sizeof(rif_config));
                rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                /* we must destroy secondary rif first */
                while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config))
                {

                    rif_config.row_status = VAL_netConfigStatus_2_destroy;

                    if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
                    {
                        CLI_LIB_PrintStr_1("Failed to destory IP address on VLAN %lu\r\n", (unsigned long)vid);
                    }



                 }

                 /* Destroy primary rif at last */
                 memset(&rif_config, 0, sizeof(rif_config));
                 rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                 if(NETCFG_TYPE_OK == NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config))
                 {

                    rif_config.row_status = VAL_netConfigStatus_2_destroy;
                    if(NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetInetRif(&rif_config,NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB))
                    {
                        CLI_LIB_PrintStr_1("Failed to destory IP address on VLAN %lu\r\n", (unsigned long)vid);
                    }

                 }

              }
              else
              {

                 if((arg[0][0] == 'D') || (arg[0][0] == 'd'))  /* no ip address dhcp */
                 {

                    /* if command is "no ip address dhcp, need to clear client lease and send DHCPREALEASE additionally "*/
                    if(DHCP_MGR_OK != DHCP_PMGR_ReleaseClientLease(ctrl_P->CMenu.vlan_ifindex))
                    {
                         CLI_LIB_PrintStr("Failed to release dhcp client lease\r\n");
                    }

                    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetIpAddressMode(ctrl_P->CMenu.vlan_ifindex,&mode))
                    {
                        CLI_LIB_PrintStr_1("Failed to get the IP address mode on VLAN %lu\r\n", (unsigned long)vid);
                        return CLI_NO_ERROR;
                    }

                    if(mode != NETCFG_TYPE_IP_ADDRESS_MODE_DHCP)
                    {
                        CLI_LIB_PrintStr("Current IP address mode is not DHCP\r\n");
                        return CLI_NO_ERROR;
                    }


                    if( NETCFG_TYPE_OK != NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE))
                    {
                         CLI_LIB_PrintStr_1("Failed to set the IP address mode as user defined mode on VLAN %lu\r\n", (unsigned long)vid)
                    }



                 }
                else
                {
                    /* get vlan ip first*/
                    memset(&rif_config, 0, sizeof(NETCFG_TYPE_InetRifConfig_T));
                    rif_config.ifindex = ctrl_P->CMenu.vlan_ifindex;
                    for (i = 1; arg[i];)
                    {
                        if (strncmp(arg[i], "secondary", 3) == 0)
                        {
                            is_secondary = TRUE;
                        }
                        if (strncmp(arg[i], "virtual", 3) == 0)
                        {
                            is_virtual = TRUE;
                        }
                            i++;
                    }

                    if(is_secondary)
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_SECONDARY;
                    else if (is_virtual)
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_VIRTUAL;
                    else
                        rif_config.ipv4_role = NETCFG_TYPE_MODE_PRIMARY; /* default */
                    if((arg[1]!=NULL) && CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask))
                    {
                        /* format: 192.168.1.1 255.255.255.0 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_ADDR,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }

                        /* convert to byte array format */
                        IP_LIB_UI32toArray(uint_ipMask, byte_mask);

                        rif_config.addr.preflen = IP_LIB_MaskToCidr(byte_mask);

                    }
                    else
                    {
                        /* format: 192.168.1.1/24 */
                        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IPV4_PREFIX,
                                                                           arg[0],
                                                                           (L_INET_Addr_T *) &rif_config.addr,
                                                                           sizeof(rif_config.addr)))
                        {
                            CLI_LIB_PrintStr_1("Invalid IP address - %s\r\n", arg[0]);
                            return CLI_NO_ERROR;
                        }
                    }


                    if(NETCFG_TYPE_OK != NETCFG_POM_IP_GetRifFromInterface(&rif_config))
                    {
                        /* There is no matched address on this VLAN */
                        CLI_LIB_PrintStr_1("Can't find address on VLAN %lu\r\n", (unsigned long)vid);
                        break;
                    }
                    rif_config.row_status = VAL_netConfigStatus_2_destroy;

                    if( NETCFG_PMGR_IP_SetIpAddressMode (ctrl_P->CMenu.vlan_ifindex, NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE) != NETCFG_TYPE_OK)
                    {
#if 0
#if (SYS_CPNT_EH == TRUE)
                        CLI_API_Show_Exception_Handeler_Msg();
#else
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", vid)
#endif
#endif
                        CLI_LIB_PrintStr_1("Failed to set the IP address mode as user define mode on VLAN %lu\r\n", (unsigned long)vid)
                    }
                    else
                    {
                        ret = NETCFG_PMGR_IP_SetInetRif(&rif_config, NETCFG_TYPE_IP_CONFIGURATION_TYPE_CLI_WEB);

                        if(ret != NETCFG_TYPE_OK)
                        {
#if 0
#if (SYS_CPNT_EH == TRUE)
                            CLI_API_Show_Exception_Handeler_Msg();
#else
                            CLI_LIB_PrintStr_1("Failed to remove IP address on VLAN %lu\r\n", vid);
#endif
#endif
                            CLI_LIB_PrintStr_1("Failed to remove IP address on VLAN %lu\r\n", (unsigned long)vid);
                        }
                    }
                 }
            }
            break;

        default:
            break;
    }
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Multipath_Number(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_DFLT_ALLOWED_ECMP_NBR_PER_ROUTE > SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE)
#error (SYS_DFLT_ALLOWED_ECMP_NBR_PER_ROUTE > SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE) !!
#endif
#if 0 /*temply mark out for compile pass*/
#if (SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE > 1)
    UI32_T multipath = 0;

    switch (cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W1_MAXIMUMPATHS:
            multipath = CLI_LIB_AtoUl(arg[0], 10);
            if (multipath < 1 || multipath > SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE)
            {
                CLI_LIB_PrintStr("Invalid multipath number value.\r\n");
                return CLI_NO_ERROR;
            }

            if (NSM_PMGR_SetMultipathNumber((UI8_T)multipath) != NSM_TYPE_RESULT_OK)
            {
                CLI_LIB_PrintStr("Failed to set multipath number value.\r\n");
                return CLI_NO_ERROR;
            }

            break;
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_NO_MAXIMUMPATHS:
            multipath = SYS_DFLT_ALLOWED_ECMP_NBR_PER_ROUTE;

            if (NSM_PMGR_SetMultipathNumber((UI8_T)multipath) != NSM_TYPE_RESULT_OK)
            {
                CLI_LIB_PrintStr("Failed to set multipath number value.\r\n");
                return CLI_NO_ERROR;
            }

            break;
        default:
            break;
    }
#endif /* #if (SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE > 1) */
#endif
    return CLI_NO_ERROR;
}

#if 0
/* local function ------------ no ip address with no argument, call by dhcp and bootp */

static BOOL_T API_Noipaddress_With_Noargument(UI32_T Ifindex)
{
#if 0 /* comment-out by vai */
//#if (SYS_CPNT_ROUTING == TRUE)
    NETCFG_MGR_RifConfig_T  rif_config;
    UI32_T  primary_ip = 0, secondary_ip = 0;
    UI32_T  primary_mask = 0, secondary_mask = 0;
    rif_config.ip_address = primary_ip = 0;
    rif_config.ip_mask = primary_mask = 0;

    while (NETCFG_PMGR_GetNextRif(&rif_config)==NETCFG_MGR_OK)
    {
       if (rif_config.vid_ifindex == Ifindex)
       {
          if (rif_config.ipv4_role == NETIF_ACTIVE_MODE_PRIMARY)
          {
             primary_ip = rif_config.ip_address;
             primary_mask = rif_config.ip_mask;
          }
          else
          {
             secondary_ip = rif_config.ip_address;
             secondary_mask = rif_config.ip_mask;

             if (NETCFG_PMGR_SetRifRowStatus (secondary_ip, secondary_mask, VAL_netConfigStatus_2_destroy)!=NETCFG_MGR_OK)
             {
                return FALSE;
             }
          }
       }
   }
   if (primary_ip != 0)
   {
      if (NETCFG_PMGR_SetRifRowStatus (primary_ip, primary_mask, VAL_netConfigStatus_2_destroy)!=NETCFG_MGR_OK)
      {
         return FALSE;
      }
   }
#endif
   return TRUE;
}
#endif

/* command : ip default-gateway */
UI32_T CLI_API_Ip_Defaultgateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
   UI32_T ui32_ip;
   UI8_T byte_ip[SYS_ADPT_IPV4_ADDR_LEN];
   L_INET_AddrIp_T default_gateway;
   UI32_T ret = 0;

    memset(&default_gateway, 0, sizeof(default_gateway));
/*if support cfgdb ip address can not set when provision*/
#if (SYS_CPNT_CFGDB == TRUE)
#if (SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB == TRUE)
    if (ctrl_P->sess_type == CLI_TYPE_PROVISION)
    {
        return CLI_NO_ERROR;
    }
#endif
#endif
    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_DEFAULTGATEWAY:
            ret = CLI_LIB_AtoIp(arg[0], (UI8_T *)&ui32_ip);
            if(!ret)
            {
                CLI_LIB_PrintStr("Invalid address.\r\n");
                return CLI_NO_ERROR;
            }
            /* convert to byte array format */
            IP_LIB_UI32toArray(ui32_ip, byte_ip);
            default_gateway.type = L_INET_ADDR_TYPE_IPV4;
            memcpy(default_gateway.addr, byte_ip, SYS_ADPT_IPV4_ADDR_LEN);
            default_gateway.addrlen = SYS_ADPT_IPV4_ADDR_LEN;

            ret = NETCFG_PMGR_ROUTE_AddDefaultGateway(&default_gateway, SYS_DFLT_DEFAULT_GATEWAY_METRIC);
            switch(ret)
            {
                case NETCFG_TYPE_OK:
                    /* do nothing */
                    break;
                case NETCFG_TYPE_ERR_ROUTE_NEXTHOP_CONFLICT_ON_NORMAL_INT_AND_CRAFT_INT:
                    CLI_LIB_PrintStr("Failed. Due to the nexthop conflicts on normal interface and craft interface.\r\n");
                    break;
                case NETCFG_TYPE_ERR_ROUTE_TWO_NEXTHOP_ON_CRAFT_INT_IS_NOT_ALLOWED:
                CLI_LIB_PrintStr("Failed. To have two nexthops on craft interface is not allowed.\r\n");
                    break;
                case NETCFG_TYPE_FAIL:
                default:
                    CLI_LIB_PrintStr("Failed to set default gateway\r\n");
                    break;

            }
            break;

        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_DEFAULTGATEWAY:
        default_gateway.type = L_INET_ADDR_TYPE_IPV4;
        default_gateway.addrlen = SYS_ADPT_IPV4_ADDR_LEN;
            NETCFG_PMGR_ROUTE_GetDefaultGateway(&default_gateway);
        if(NETCFG_PMGR_ROUTE_DeleteDefaultGateway(&default_gateway)!=NETCFG_TYPE_OK)
            {
#if 0
#if (SYS_CPNT_EH == TRUE)
                CLI_API_Show_Exception_Handeler_Msg();
#else
                CLI_LIB_PrintStr("Failed to disable the default gateway.\r\n");
#endif
#endif
                CLI_LIB_PrintStr("Failed to disable the default gateway.\r\n");
            }
            break;

        default:
            return CLI_NO_ERROR;
    }
    return CLI_NO_ERROR;
}

/* command: show ip interface */
UI32_T CLI_API_Show_Ip_Interfaces(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T rif_vid = 0, target_id = 0;

    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    NETCFG_TYPE_L3_Interface_T ip_interface;
    NETCFG_TYPE_InetRifConfig_T rif_config;
    UI8_T mask[SYS_ADPT_IPV4_ADDR_LEN];

    memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));

    while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
    {
        /* vlan is specified */
        if(arg[0] && (arg[0][0]=='v')) /* vlan */
        {
            target_id = atoi(arg[1]);
            if(FALSE == VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &rif_vid))
                continue;

            if(rif_vid != target_id)
                continue;
        }
        else if(arg[0] && (arg[0][0]=='l')) /* loopback int. is specified */
        {
            if (FALSE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
                continue;

        }

        if (TRUE == IP_LIB_IsLoopbackInterface(ip_interface.u.physical_intf.if_flags))
        {
#if (SYS_CPNT_LOOPBACK_IF_VISIBLE == TRUE)
            if (FALSE == IP_LIB_ConvertLoopbackIfindexToId(ip_interface.ifindex, &rif_vid))
                continue;

            CLI_LIB_PrintStr_1("Loopback %lu is ", (unsigned long)rif_vid);
#else
            /* skip loopback interface */
            continue;
#endif
        }
#if (SYS_CPNT_IP_TUNNEL == TRUE)
        else if(VLAN_L3_IP_TUNNELTYPE == ip_interface.iftype)
        {
            /*current , we do not support IPv4 tunnel*/
            continue;
            IP_LIB_ConvertTunnelIdFromIfindex(ip_interface.ifindex, &rif_vid);
            CLI_LIB_PrintStr_1("Tunnel %lu is ", (unsigned long)rif_vid);
        }
#endif /*SYS_CPNT_IP_TUNNEL*/
        else
        {
            VLAN_OM_ConvertFromIfindex(ip_interface.ifindex, &rif_vid);
            CLI_LIB_PrintStr_1("VLAN %lu is ", (unsigned long)rif_vid);
        }
        if (IP_LIB_IsIpInterfaceUp(ip_interface.u.physical_intf.if_flags) == TRUE)
            CLI_LIB_PrintStr("Administrative Up");
        else
            CLI_LIB_PrintStr("Administrative Down");

        if (IP_LIB_IsIpInterfaceRunning(ip_interface.u.physical_intf.if_flags) == TRUE)
            CLI_LIB_PrintStr(" - Link Up\r\n");
        else
            CLI_LIB_PrintStr(" - Link Down\r\n");
        PROCESS_MORE(buff);

        SYSFUN_Sprintf((char *)buff, "  Address is %02X-%02X-%02X-%02X-%02X-%02X\r\n", ip_interface.u.physical_intf.logical_mac[0], ip_interface.u.physical_intf.logical_mac[1],
                        ip_interface.u.physical_intf.logical_mac[2], ip_interface.u.physical_intf.logical_mac[3], ip_interface.u.physical_intf.logical_mac[4],
                        ip_interface.u.physical_intf.logical_mac[5]);
        PROCESS_MORE(buff);

        CLI_LIB_PrintStr_2("  Index: %lu, MTU: %lu\r\n",
                        (unsigned long)ip_interface.ifindex, (unsigned long)ip_interface.u.physical_intf.mtu);
        PROCESS_MORE(buff);

        switch(ip_interface.u.physical_intf.ipv4_address_mode)
        {
            case NETCFG_TYPE_IP_ADDRESS_MODE_USER_DEFINE:
                CLI_LIB_PrintStr("  Address Mode is User specified\r\n");
                break;

#if (SYS_CPNT_DHCP_CLIENT == TRUE)
            case NETCFG_TYPE_IP_ADDRESS_MODE_DHCP:
                CLI_LIB_PrintStr("  Address Mode is DHCP\r\n");
                break;
#endif
#if (SYS_CPNT_BOOTP == TRUE)
            case NETCFG_TYPE_IP_ADDRESS_MODE_BOOTP:
                CLI_LIB_PrintStr("  Address Mode is BOOTP\r\n");
                break;
#endif

            default:
                CLI_LIB_PrintStr("  Address Mode is not specified\r\n");
                break;
        }
        PROCESS_MORE(buff);

        memset(&rif_config, 0, sizeof(rif_config));
     /* get primary rif first */
        rif_config.ifindex = ip_interface.ifindex;
        if(NETCFG_POM_IP_GetPrimaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            if(rif_config.row_status != NETCFG_TYPE_StaticIpCidrEntryRowStatus_active)
                continue;

            if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV4)
                && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV4Z))
                break;

            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                            rif_config.addr.addr[0], rif_config.addr.addr[1],
                            rif_config.addr.addr[2], rif_config.addr.addr[3]);
            IP_LIB_CidrToMask(rif_config.addr.preflen, mask);
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d", mask[0], mask[1], mask[2], mask[3]);
            CLI_LIB_PrintStr("\r\n");
            PROCESS_MORE(buff);
        }

        /* get secondary rif */
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ip_interface.ifindex;
        while(NETCFG_POM_IP_GetNextSecondaryRifFromInterface(&rif_config) == NETCFG_TYPE_OK)
        {
            if(rif_config.row_status != NETCFG_TYPE_StaticIpCidrEntryRowStatus_active)
                continue;

            if((rif_config.addr.type != L_INET_ADDR_TYPE_IPV4)
                && (rif_config.addr.type != L_INET_ADDR_TYPE_IPV4Z))
                break;

            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                            rif_config.addr.addr[0], rif_config.addr.addr[1],
                            rif_config.addr.addr[2], rif_config.addr.addr[3]);
            IP_LIB_CidrToMask(rif_config.addr.preflen, mask);
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d", mask[0], mask[1], mask[2], mask[3]);
                CLI_LIB_PrintStr(" Secondary\r\n");
            PROCESS_MORE(buff);
        }
#if (SYS_CPNT_VIRTUAL_IP == TRUE)
        /* get virtual rif */
        memset(&rif_config, 0, sizeof(rif_config));
        rif_config.ifindex = ip_interface.ifindex;

        while(NETCFG_TYPE_OK == NETCFG_POM_IP_GetNextVirtualRifByIfindex(&rif_config))
        {
            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                            rif_config.addr.addr[0], rif_config.addr.addr[1],
                            rif_config.addr.addr[2], rif_config.addr.addr[3]);
            IP_LIB_CidrToMask(rif_config.addr.preflen, mask);
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d", mask[0], mask[1], mask[2], mask[3]);
                CLI_LIB_PrintStr(" Virtual\r\n");
            PROCESS_MORE(buff);
        }
#endif

        if (ip_interface.u.physical_intf.proxy_arp_enable == TRUE)
            SYSFUN_Sprintf((char *)buff, "  Proxy ARP is enabled\r\n");
        else
            SYSFUN_Sprintf((char *)buff, "  Proxy ARP is disabled\r\n");
        PROCESS_MORE(buff);
#if (SYS_CPNT_DHCP_CLIENT == TRUE)
        {
            DHCP_MGR_Vendor_T vendor_classid;

            memset(&vendor_classid, 0, sizeof(vendor_classid));
            if (DHCP_MGR_OK == DHCP_PMGR_C_GetVendorClassId(ip_interface.ifindex, &vendor_classid))
            {
                CLI_LIB_PrintStr("  DHCP Vendor Class-ID: ");
                if (vendor_classid.vendor_mode == DHCP_MGR_CLASSID_HEX)
                {
                    UI8_T index=0;
                    CLI_LIB_PrintStr("0x");
                    for (index=0; index<vendor_classid.vendor_len;index++)
                    {
                        CLI_LIB_PrintStr_1("%02x",vendor_classid.vendor_buf[index]);
                    }
                }
                else if (vendor_classid.vendor_mode == DHCP_MGR_CLASSID_TEXT)
                {
                    CLI_LIB_PrintStr_1("%s", vendor_classid.vendor_buf);
                }

                CLI_LIB_Printf("\r\n");
            }
        }
#endif

#if (SYS_CPNT_DHCP_INFORM == TRUE)
        if (TRUE == ip_interface.dhcp_inform)
            CLI_LIB_PrintStr("  DHCP Inform is enabled\r\n");
        else
            CLI_LIB_PrintStr("  DHCP Inform is disabled\r\n");
#endif

#if (SYS_CPNT_DHCP_RELAY == TRUE)
        {
            UI32_T relay_server[SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER] = {0};
            UI8_T  idx=0;

            sprintf(buff,"  DHCP relay server: ");

            if(DHCP_OM_OK == DHCP_POM_GetIfRelayServerAddress(ip_interface.ifindex, relay_server))
            {
                for(idx=0;idx<SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER;idx++)
                {
                    if(relay_server[idx])
                    {
                        if(idx!=0)
                            sprintf(buff+strlen(buff),",");

                        sprintf(buff+strlen(buff),"%u.%u.%u.%u",
                            ((UI8_T *)(&(relay_server[idx])))[0],
                            ((UI8_T *)(&(relay_server[idx])))[1],
                            ((UI8_T *)(&(relay_server[idx])))[2],
                            ((UI8_T *)(&(relay_server[idx])))[3]);
                    }
                }

                sprintf(buff+strlen(buff),"\r\n");
            }
            else
            {
                sprintf(buff+strlen(buff),"0.0.0.0\r\n");
            }

            PROCESS_MORE(buff);
        }
#endif
    } /* while */


/*fuzhimin,20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
    /*show ethernet0*/
    {
        struct ifreq ifr;
        int ret;
        int     ipaddr;
        int     netmask;


        bzero ((caddr_t)&ifr, sizeof (ifr));
        strcpy (ifr.ifr_name, "eth0");
        ret = if_ioctl (SIOCGIFFLAGS, (caddr_t) &ifr);
        if (ret < 0)
        {
            /*printf(" Failed SIOCGIFFLAGS for eth0\n");*/
            return (CLI_NO_ERROR);
        }
        if(ifr.ifr_flags & IFF_UP)
        {
            CLI_LIB_PrintStr((UI8_T *)"Ethernet0 is Administrative Up\r\n");
            ret = if_ioctl (SIOCGIFHWADDR, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFHWADDR for eth0\n");*/
                return (CLI_NO_ERROR);
            }

            memset(buff,0,CLI_DEF_MAX_BUFSIZE);
            SYSFUN_Sprintf((char *)buff, "  Address is %02X-%02X-%02X-%02X-%02X-%02X\r\n", ifr.ifr_hwaddr.sa_data[0], ifr.ifr_hwaddr.sa_data[1],
                            ifr.ifr_hwaddr.sa_data[2], ifr.ifr_hwaddr.sa_data[3], ifr.ifr_hwaddr.sa_data[4],
                            ifr.ifr_hwaddr.sa_data[5]);

            CLI_LIB_PrintStr(buff);

            ret = if_ioctl (SIOCGIFMTU, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFMTU for eth0\n");*/
                return (CLI_NO_ERROR);
            }

            CLI_LIB_PrintStr_1("  MTU: %d\r\n",ifr.ifr_mtu);

            ret = if_ioctl (SIOCGIFADDR, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFADDR for eth0\n");*/
                return (CLI_NO_ERROR);
            }
            ipaddr = ((struct sockaddr_in*)(&(ifr.ifr_addr)))->sin_addr.s_addr;
            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                                (ipaddr&0xff000000)>>24, (ipaddr&0xff0000)>>16,
                                (ipaddr&0xff00)>>8, (ipaddr&0xff));

            ret = if_ioctl (SIOCGIFNETMASK, (caddr_t) &ifr);
            if (ret < 0)
            {
                /*printf(" Failed SIOCGIFNETMASK for eth0\n");*/
                return (CLI_NO_ERROR);
            }
            netmask = ((struct sockaddr_in*)(&(ifr.ifr_netmask)))->sin_addr.s_addr;
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d",
                                (netmask&0xff000000)>>24, (netmask&0xff0000)>>16,
                                (netmask&0xff00)>>8, (netmask&0xff));
            CLI_LIB_PrintStr((UI8_T *)"\r\n");
        }
#if 0
{
    struct phy_device *phydev;
    phydev = get_fcc_phy_device();

     CLI_LIB_PrintStr_2("PHY: %s - Link is %s", phydev->dev.bus_id,
            phydev->link ? "Up" : "Down");
    if (phydev->link)
        CLI_LIB_PrintStr_2(" - %d/%s", phydev->speed,
                0x0/*DUPLEX_FULL*/ == phydev->duplex ?
                "Full" : "Half");

    CLI_LIB_PrintStr("(UI8_T *)\n");

}
#endif
    }
#endif
/*fuzhimin,20090106,end*/

#if (SYS_CPNT_CRAFT_PORT == TRUE)
    /*show craft interface */
    {
        NETCFG_TYPE_CraftInetAddress_T craft_addr;
        memset(&craft_addr, 0, sizeof(craft_addr));
        if(NETCFG_POM_IP_GetCraftInterfaceInetAddress(&craft_addr) == NETCFG_TYPE_OK)
        {
            CLI_LIB_PrintStr("Craft interface is Administrative Up\r\n");
            CLI_LIB_PrintStr_4("  IP Address: %d.%d.%d.%d",
                            craft_addr.addr.addr[0], craft_addr.addr.addr[1],
                            craft_addr.addr.addr[2], craft_addr.addr.addr[3]);
            IP_LIB_CidrToMask(craft_addr.addr.preflen, mask);
            CLI_LIB_PrintStr_4(" Mask: %d.%d.%d.%d\r\n", mask[0], mask[1], mask[2], mask[3]);
            PROCESS_MORE(buff);

        }
    }
#endif /* SYS_CPNT_CRAFT_PORT */

    return CLI_NO_ERROR;
}


/*-----------------------------------------ospf---------------------------------*/

/* command:area area_id default-cost
           area area_id nssa
           area area_id range
           area area_id stub */

UI32_T CLI_API_L3_ROUTEROSPF_AREA(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
#if 0 /*These functions will be implemented in future -- xiongyu 20090112*/

   UI32_T ip_address = {0}, mask = {0}, area_id=0, router_id=0;

   UI8_T  auth_type[20] = {0} , message_type[20] = {0};
   UI32_T parse_count5 = 0,parse_count7 = 0,parse_count9 = 0,parse_count11 = 0,
          parse_count6 = 0,parse_count8 = 0,parse_count10 = 0,parse_count12 = 0;

   BOOL_T authentication=FALSE, hello_interval=FALSE, retransmit_interval=FALSE,
          transmit_delay=FALSE, dead_interval=FALSE, authentication_key=FALSE, message_digest_key=FALSE;

   UI32_T now_status=VAL_ospfVirtIfStatus_createAndGo,parse_true=FALSE;

   OSPF_TYPE_OspfNssaAreaEntry_T nssa_area_record;
   OSPF_TYPE_OspfAreaAggregateEntry_T area_aggregate_entry;
   OSPF_TYPE_OspfStubAreaEntry_T stub_area_entry;
   OSPF_TYPE_OspfStubAreaSummaryEntry_T stub_area_summary_entry;
   OSPF_TYPE_OspfVirtIfEntry_T virt_if_entry;

   memset(&nssa_area_record, 0, sizeof(OSPF_TYPE_OspfNssaAreaEntry_T));
   memset(&area_aggregate_entry, 0, sizeof(OSPF_TYPE_OspfAreaAggregateEntry_T));
   memset(&stub_area_summary_entry, 0, sizeof(OSPF_TYPE_OspfStubAreaSummaryEntry_T));
   memset(&stub_area_entry, 0, sizeof(OSPF_TYPE_OspfStubAreaEntry_T));
   memset(&virt_if_entry, 0, sizeof(OSPF_TYPE_OspfVirtIfEntry_T));

   CLI_LIB_AtoIp(arg[0], (UI8_T*)&area_id);

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_AREA:

      /* default cost */
      if(arg[1][0]=='d' || arg[1][0]=='D')
      {
         stub_area_entry.ospf_stub_area_id = area_id;
         if (NETCFG_PMGR_GetOspfStubEntry(&stub_area_entry)!= NETCFG_MGR_OK)
         {
            stub_area_entry.ospf_stub_metric= atoi(arg[2]);
            stub_area_entry.ospf_stub_status= VAL_ospfStubStatus_createAndGo;

            if(NETCFG_PMGR_SetOspfStubEntry(&stub_area_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set default cost.\r\n");
            }
         }
         else
         {
            stub_area_entry.ospf_stub_metric= atoi(arg[2]);
            stub_area_entry.ospf_stub_status= L_RSTATUS_SET_OTHER_COLUMN;

            if(NETCFG_PMGR_SetOspfStubEntry(&stub_area_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set default cost.\r\n");
            }
         }
         return CLI_NO_ERROR;
      }
      /* nssa */
      else if(arg[1][0]=='n' || arg[1][0]=='N')
      {
         nssa_area_record.ospf_nssa_area_id = area_id;
         if (NETCFG_PMGR_GetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
         {
            if(NETCFG_PMGR_GetDefaultOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to get nssa default value\r\n");
            }
            else
            {
               nssa_area_record.ospf_nssa_area_id= area_id;
               if (arg[2][0]=='n' || arg[2][0]=='N')
               {
                  nssa_area_record.ospf_nssa_redistribute_status= VAL_ospfNssaRedistributeStatus_disabled;
               }
               if(arg[2][0]=='d' || arg[2][0]=='D' || arg[3][0]=='d' || arg[3][0]=='D')
               {
                  nssa_area_record.ospf_nssa_default_information_status= VAL_ospfNssaOriginateDefaultInfoStatus_enabled;
               }
               nssa_area_record.ospf_nssa_status= VAL_ospfNssaStatus_createAndGo;
               if(NETCFG_PMGR_SetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to set nssa area entry\r\n");
               }
            }
         }
         else
         {
            if (arg[2][0]=='n' || arg[2][0]=='N')
            {
               nssa_area_record.ospf_nssa_redistribute_status = VAL_ospfNssaRedistributeStatus_disabled;
            }
            if(arg[2][0]=='d' || arg[2][0]=='D' || arg[3][0]=='d' || arg[3][0]=='D')
            {
               nssa_area_record.ospf_nssa_default_information_status= VAL_ospfNssaOriginateDefaultInfoStatus_enabled;
            }
            nssa_area_record.ospf_nssa_status= L_RSTATUS_SET_OTHER_COLUMN;
            if(NETCFG_PMGR_SetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set nssa area entry\r\n");
            }
         }
         return CLI_NO_ERROR;
      }
      /* range */
      else if(arg[1][0]=='r' || arg[1][0]=='R')
      {
         CLI_LIB_AtoIp(arg[2], (UI8_T*)&ip_address);
         CLI_LIB_AtoIp(arg[3], (UI8_T*)&mask);

         area_aggregate_entry.ospf_area_aggregate_area_id = area_id;
         if (NETCFG_PMGR_GetDefaultOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
         {
            if(NETCFG_PMGR_GetDefaultOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to get default range value\r\n");
            }
            else
            {
               area_aggregate_entry.ospf_area_aggregate_area_id= area_id;
               area_aggregate_entry.ospf_area_aggregate_net= ip_address;
               area_aggregate_entry.ospf_area_aggregate_mask= mask;
               /*if (arg[4][0]=='a' || arg[4][0]=='A')
               {
                  area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_advertiseMatching;
               }
               else*/
               if (arg[4][0]=='n' || arg[4][0]=='N')
               {
                  area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_doNotAdvertiseMatching;
               }
               else
               {
                  area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_advertiseMatching;
               }

               area_aggregate_entry.ospf_area_aggregate_status= L_RSTATUS_SET_OTHER_COLUMN;
               if(NETCFG_PMGR_SetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to set area range entry\r\n");
               }
            }
         }
         else
         {
            area_aggregate_entry.ospf_area_aggregate_area_id= area_id;
            area_aggregate_entry.ospf_area_aggregate_net= ip_address;
            area_aggregate_entry.ospf_area_aggregate_mask= mask;
            /*
              06/04/2004 05:03 Pttch modify because argument can have null, so need check no argument state
            */

/*
            if (arg[4][0]=='a' || arg[4][0]=='A')
            {
               area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_advertiseMatching;
            }
            else if (arg[4][0]=='n' || arg[4][0]=='N')
*/
            if (arg[4][0]=='n' || arg[4][0]=='N')
            {
               area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_doNotAdvertiseMatching;
            }
            else
            {
               area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_advertiseMatching;
            }

            area_aggregate_entry.ospf_area_aggregate_status= VAL_ospfAreaAggregateStatus_createAndGo;
            if(NETCFG_PMGR_SetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set area range entry\r\n");
            }
         }
         return CLI_NO_ERROR;
      }
      /* stub */
      else if(arg[1][0]=='s' || arg[1][0]=='S')
      {
         stub_area_summary_entry.stub_area_summary.ospf_stub_area_id = area_id;
         if(NETCFG_PMGR_GetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
         {
            if(NETCFG_PMGR_GetDefaultOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to get default stub area summary entry value\r\n");
            }
            else
            {
               stub_area_summary_entry.stub_area_summary.ospf_stub_area_id = area_id;
               /*
                  2004/7/9 11:59 pttch modify, need add summary seclection for it.
               */

               switch(arg[2][0])
               {
               case 'n':
               case 'N':
                  stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_noAreaSummary;
                  break;

               case 's':
               case 'S':
                  stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_sendAreaSummary;
                  break;

               default:
                  stub_area_summary_entry.ospf_area_summary = SYS_DFLT_OSPF_AREA_SUMMARY;
                  break;
               }

               stub_area_summary_entry.stub_area_summary.ospf_stub_status = VAL_ospfStubStatus_createAndGo;
               if(NETCFG_PMGR_SetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to set stub area summary entry\r\n");
               }
            }
         }
         else
         {
            stub_area_summary_entry.stub_area_summary.ospf_stub_area_id = area_id;
            /*
               2004/7/9 11:59 pttch modify, need add summary seclection for it.
            */
            switch(arg[2][0])
            {
            case 'n':
            case 'N':
               stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_noAreaSummary;
               break;

            case 's':
            case 'S':
               stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_sendAreaSummary;
               break;

            default:
               stub_area_summary_entry.ospf_area_summary = SYS_DFLT_OSPF_AREA_SUMMARY;
               break;
            }

            stub_area_summary_entry.stub_area_summary.ospf_stub_status = L_RSTATUS_SET_OTHER_COLUMN;
            if(NETCFG_PMGR_SetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set stub area summary entry\r\n");
            }
         }
         return CLI_NO_ERROR;
      }
      /* virtual link */
      if(arg[1][0]=='v' || arg[1][0]=='V')
      {
         CLI_LIB_AtoIp(arg[2], (UI8_T*)&router_id);
         virt_if_entry.ospf_virt_if_area_id= area_id;
         virt_if_entry.ospf_virt_if_neighbor= router_id;
         virt_if_entry.ospf_virt_if_auth_type = OSPF_TYPE_AUTH_TYPE_MD5;

         if (NETCFG_PMGR_GetOspfVirtIfEntry(&virt_if_entry)!= NETCFG_MGR_OK)
     {
        if(NETCFG_PMGR_GetDefaultOspfVirtIfEntry(&virt_if_entry)!= NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to get default virtual-link value\r\n");
        }
        else
        {
           now_status=VAL_ospfVirtIfStatus_createAndGo;
               goto PARSE_CURRNET_VALUE;
        }
     }
     else
     {
        now_status=L_RSTATUS_SET_OTHER_COLUMN;
        goto PARSE_CURRNET_VALUE;
         }
      }

      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_AREA:

      /* no area area-id */
      {
         OSPF_TYPE_OspfAreaEntry_T ospf_area_entry;
         if(arg[1]==0)
         {
            ospf_area_entry.ospf_area_id = area_id;
            if (NETCFG_PMGR_GetOspfAreaEntry(&ospf_area_entry)!= NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Can't find ospf area entry\r\n");
        }
        else
        {
           ospf_area_entry.ospf_area_status= VAL_ospfAreaStatus_destroy;
           if(NETCFG_PMGR_SetOspfAreaEntry(&ospf_area_entry)!= NETCFG_MGR_OK)
           {
              CLI_LIB_PrintStr("Failed to destory ospf area entry\r\n");
           }
        }
        return CLI_NO_ERROR;
         }
      }

      /* default cost */
      if(arg[1][0]=='d' || arg[1][0]=='D')
      {
         stub_area_entry.ospf_stub_area_id = area_id;
         if (NETCFG_PMGR_GetOspfStubEntry(&stub_area_entry)!= NETCFG_MGR_OK)
     {
        CLI_LIB_PrintStr("Can't find stub entry\r\n");
     }
     else
     {
        if(NETCFG_PMGR_ClearOspfStubMetric(area_id,0)!= NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to destory default cost\r\n");
        }
     }
     return CLI_NO_ERROR;
      }
      /* nssa */
      else if(arg[1][0]=='n' || arg[1][0]=='N')
      {
         nssa_area_record.ospf_nssa_area_id = area_id;

         if (NETCFG_PMGR_GetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
     {
        CLI_LIB_PrintStr("Can't find nssa entry\r\n");
     }
     else
     {
        if (arg[2][0]=='n' || arg[2][0]=='N')
        {
           nssa_area_record.ospf_nssa_redistribute_status= VAL_ospfNssaRedistributeStatus_enabled;
           if(NETCFG_PMGR_SetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
            {
                  CLI_LIB_PrintStr("Failed to set default redistribute status of nssa entry\r\n");
            }
        }
        else if(arg[2][0]=='d' || arg[2][0]=='D' || arg[3][0]=='d' || arg[3][0]=='D')
        {
           nssa_area_record.ospf_nssa_default_information_status= VAL_ospfNssaOriginateDefaultInfoStatus_disabled;
           if(NETCFG_PMGR_SetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
            {
               CLI_LIB_PrintStr("Failed to set default information originate status of nssa entry\r\n");
            }
        }
        else
        {
        nssa_area_record.ospf_nssa_status= VAL_ospfNssaStatus_destroy;
        if(NETCFG_PMGR_SetOspfNssaEntry(&nssa_area_record)!= NETCFG_MGR_OK)
        {
               CLI_LIB_PrintStr("Failed to destory nssa entry\r\n");
            }
         }

         }
         return CLI_NO_ERROR;
      }
      /* range */
      else if(arg[1][0]=='r' || arg[1][0]=='R')
      {
         CLI_LIB_AtoIp(arg[2], (UI8_T*)&ip_address);
         CLI_LIB_AtoIp(arg[3], (UI8_T*)&mask);
         area_aggregate_entry.ospf_area_aggregate_net= ip_address;
     area_aggregate_entry.ospf_area_aggregate_mask= mask;
         area_aggregate_entry.ospf_area_aggregate_area_id = area_id;
         area_aggregate_entry.ospf_area_aggregate_lsdb_type = VAL_ospfAreaAggregateLsdbType_summaryLink;

         if (NETCFG_PMGR_GetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
     {
        CLI_LIB_PrintStr("Failed to get range entry\r\n");
     }
     else
     {
        if (arg[4][0]=='a' || arg[4][0]=='A')
        {
           area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_doNotAdvertiseMatching;
           if(NETCFG_PMGR_SetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
           {
               CLI_LIB_PrintStr("Failed to disable advertise attribution of area range entry\r\n");
           }
        }
        else if (arg[4][0]=='n' || arg[4][0]=='N')
        {
           area_aggregate_entry.ospf_area_aggregate_effect= VAL_ospfAreaAggregateEffect_advertiseMatching;
           if(NETCFG_PMGR_SetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
           {
               CLI_LIB_PrintStr("Failed to disable no advertise attribution of area range entry\r\n");
           }
        }
        else
        {
        area_aggregate_entry.ospf_area_aggregate_status= VAL_ospfAreaAggregateStatus_destroy;
        if(NETCFG_PMGR_SetOspfAreaAggregateEntry(&area_aggregate_entry)!= NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to destory area range entry\r\n");
        }
     }

     }
     return CLI_NO_ERROR;
      }
      /* stub */
      else if(arg[1][0]=='s' || arg[1][0]=='S')
      {
         stub_area_summary_entry.stub_area_summary.ospf_stub_area_id = area_id;
         if (NETCFG_PMGR_GetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
         {
            CLI_LIB_PrintStr("Area entry is not existed\r\n");
         }
         else
         {

            switch(arg[2][0])
            {
            case 'n':
            case 'N':
               stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_sendAreaSummary;
               if(NETCFG_PMGR_SetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to disable no summary attribution of area stub summary entry\r\n");
               }
               break;

            case 's':
            case 'S':
               stub_area_summary_entry.ospf_area_summary = VAL_ospfAreaSummary_noAreaSummary;
               if(NETCFG_PMGR_SetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to disable no summary attribution of area stub summary entry\r\n");
               }
               break;

            default:
               stub_area_summary_entry.stub_area_summary.ospf_stub_status = VAL_ospfStubStatus_destroy;
               if(NETCFG_PMGR_SetOspfStubAreaSummaryEntry(&stub_area_summary_entry)!= NETCFG_MGR_OK)
               {
                  CLI_LIB_PrintStr("Failed to destory area stub summary entry\r\n");
               }
               break;
            }
         }
         return CLI_NO_ERROR;
      }
      /* virtual link */
      if(arg[1][0]=='v' || arg[1][0]=='V')
      {
         CLI_LIB_AtoIp(arg[2], (UI8_T*)&router_id);
         virt_if_entry.ospf_virt_if_area_id= area_id;
         virt_if_entry.ospf_virt_if_neighbor= router_id;

         if(arg[3]==0)
         {
            if (NETCFG_PMGR_GetOspfVirtIfEntry(&virt_if_entry)!= NETCFG_MGR_OK)
            {
           CLI_LIB_PrintStr("Failed to get virtual-link entry\r\n");
            }
            else
            {
           virt_if_entry.ospf_virt_if_status = VAL_ospfVirtIfStatus_destroy;
           if(NETCFG_PMGR_SetOspfVirtIfEntry(&virt_if_entry)!= NETCFG_MGR_OK)
           {
              CLI_LIB_PrintStr("Failed to destory virtual-link entry\r\n");
               }
            }
            return CLI_NO_ERROR;
         }
         else
         {
        parse_true=TRUE;
        goto PARSE_TRUE_VALUE;
         }
      }

      break;

   default:
      return CLI_NO_ERROR;
   }

   PARSE_CURRNET_VALUE:
   {

      virt_if_entry.ospf_virt_if_area_id= area_id;
      virt_if_entry.ospf_virt_if_neighbor= router_id;

      if(arg[3]==0)
      {
         goto PARSE_END;
      }

      strcpy(auth_type, arg[3]);

/*pttch 2004/8/31 04:19 seperate type and key setting
      if(strcmp(auth_type,"message-digest-key") == 0)
      {
            virt_if_entry.ospf_virt_if_auth_type=OSPF_TYPE_AUTH_TYPE_MD5;
      }
*/
      if(strcmp(auth_type, "authentication") == 0)
      {
         virt_if_entry.ospf_virt_if_auth_type= OSPF_TYPE_AUTH_TYPE_SIMPLE;
         if(arg[4]!=0)
         {
            strcpy(message_type, arg[4]);

            if(strcmp(message_type, "message-digest") == 0)
            {
               virt_if_entry.ospf_virt_if_auth_type= OSPF_TYPE_AUTH_TYPE_MD5;
               parse_count5 = 4;
               goto PARSE_TREE_H5;
            }
            else if(arg[4][0]=='n' || arg[4][0]=='N')
            {
               virt_if_entry.ospf_virt_if_auth_type= OSPF_TYPE_AUTH_TYPE_NONE;
               parse_count5 = 4;
               goto PARSE_TREE_H5;
            }
            else if(arg[4][0]=='h'||arg[4][0]=='H')
            {
               virt_if_entry.ospf_virt_if_hello_interval= atoi(arg[5]);
               parse_count6 = 3;
               goto PARSE_TREE_R6;
            }
            else if(arg[4][0]=='r'||arg[4][0]=='R')
            {
               virt_if_entry.ospf_virt_if_retrans_interval= atoi(arg[5]);
               parse_count6 = 2;
               goto PARSE_TREE_R6;
            }
            else if(arg[4][0]=='t'||arg[4][0]=='T')
            {
               virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[5]);
               parse_count6 = 1;
               goto PARSE_TREE_R6;
            }
            else if(arg[4][0]=='d'||arg[4][0]=='D')
            {
               virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[5]);
               parse_count6 = 0;
               goto PARSE_TREE_R6;
            }
            else if(arg[4][0]=='a' || arg[4][0]=='A')
            {
               memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[5],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
               goto PARSE_END;
            }
            else if(arg[4][0]=='m'||arg[4][0]=='M')
            {
               virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[5]);
               memcpy(virt_if_entry.ospf_virt_if_md5key,arg[7],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));

               goto PARSE_END;
            }
         }
      }

      else if(arg[3][0]=='h'||arg[3][0]=='H')
      {
         virt_if_entry.ospf_virt_if_hello_interval= atoi(arg[4]);
         parse_count5 = 3;
         goto PARSE_TREE_H5;
      }
      else if(arg[3][0]=='r'||arg[3][0]=='R')
      {
         virt_if_entry.ospf_virt_if_retrans_interval= atoi(arg[4]);
         parse_count5 = 2;
         goto PARSE_TREE_H5;
      }
      else if(arg[3][0]=='t'||arg[3][0]=='T')
      {
         virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[4]);
         parse_count5 = 1;
         goto PARSE_TREE_H5;
      }
      else if(arg[3][0]=='d'||arg[3][0]=='D')
      {
         virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[4]);
         parse_count5 = 0;
         goto PARSE_TREE_H5;
      }
      else if(arg[3][0]=='a' || arg[3][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[4],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[3][0]=='m'||arg[3][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[4]);
        memcpy(virt_if_entry.ospf_virt_if_md5key,arg[6],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TREE_H5:
   {
      if(parse_count5>3)
      {
         if(arg[5][0]=='h'||arg[5][0]=='H')
         {
            virt_if_entry.ospf_virt_if_hello_interval= atoi(arg[6]);
            parse_count7 = 3;
            goto PARSE_TREE_R7;
         }
      }
      if(parse_count5>2)
      {
         if(arg[5][0]=='r'||arg[5][0]=='R')
         {
            virt_if_entry.ospf_virt_if_retrans_interval= atoi(arg[6]);
            parse_count7 = 2;
            goto PARSE_TREE_R7;
         }
      }
      if(parse_count5>1)
      {
         if(arg[5][0]=='t'||arg[5][0]=='T')
         {
            virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[6]);
            parse_count7 = 1;
            goto PARSE_TREE_R7;
         }
      }
      if(parse_count5>0)
      {
         if(arg[5][0]=='d'||arg[5][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[6]);
            parse_count7 = 0;
            goto PARSE_TREE_R7;
         }
      }
      if(arg[5][0]=='a' || arg[5][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[6],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[5][0]=='m'||arg[5][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[6]);
        memcpy(virt_if_entry.ospf_virt_if_md5key,arg[8],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));

         goto PARSE_END;
      }
   }

   PARSE_TREE_R6:
   {
      if(parse_count6>2)
      {
         if(arg[6][0]=='r'||arg[6][0]=='R')
         {
            virt_if_entry.ospf_virt_if_retrans_interval= atoi(arg[7]);
            parse_count8 = 2;
            goto PARSE_TREE_T8;
         }
      }
      if(parse_count6>1)
      {
         if(arg[6][0]=='t'||arg[6][0]=='T')
         {
            virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[7]);
            parse_count8 = 1;
            goto PARSE_TREE_T8;
         }
      }
      if(parse_count6>0)
      {
         if(arg[6][0]=='d'||arg[6][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[7]);
            parse_count8 = 0;
            goto PARSE_TREE_T8;
         }
      }
      if(arg[6][0]=='a' || arg[6][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[7],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[6][0]=='m'||arg[6][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[7]);
      memcpy(virt_if_entry.ospf_virt_if_md5key,arg[9],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TREE_T8:
   {
      if(parse_count8>1)
      {
         if(arg[8][0]=='t'||arg[8][0]=='T')
         {
            virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[9]);
            parse_count10 = 1;
            goto PARSE_TREE_D10;
         }
      }
      if(parse_count8>0)
      {
         if(arg[8][0]=='d'||arg[8][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[9]);
            parse_count10 = 0;
            goto PARSE_TREE_D10;
         }
      }
      if(arg[8][0]=='a' || arg[8][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[9],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[8][0]=='m'||arg[8][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[9]);
        memcpy(virt_if_entry.ospf_virt_if_md5key,arg[11],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TREE_D10:
   {
      if(parse_count10>0)
      {
         if(arg[10][0]=='d'||arg[10][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[11]);
            parse_count12 = 0;
            goto PARSE_TREE_A12;
         }
      }
      if(arg[10][0]=='a' || arg[10][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[11],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[10][0]=='m'||arg[10][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[11]);
        memcpy(virt_if_entry.ospf_virt_if_md5key,arg[13],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));

         goto PARSE_END;
      }
   }

   PARSE_TREE_A12:
   {
      if(arg[12][0]=='a' || arg[12][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[13],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[12][0]=='m'||arg[12][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[13]);
         memcpy(virt_if_entry.ospf_virt_if_md5key,arg[15],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));

         goto PARSE_END;
      }
   }

   PARSE_TREE_R7:
   {
      if(parse_count7>2)
      {
         if(arg[7][0]=='r'||arg[7][0]=='R')
         {
            virt_if_entry.ospf_virt_if_retrans_interval= atoi(arg[8]);
            parse_count9 = 2;
            goto PARSE_TREE_T9;
         }
      }
      if(parse_count7>1)
      {
         if(arg[7][0]=='t'||arg[7][0]=='T')
         {
            virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[8]);
            parse_count9 = 1;
            goto PARSE_TREE_T9;
         }
      }
      if(parse_count7>0)
      {
         if(arg[7][0]=='d'||arg[7][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[8]);
            parse_count9 = 0;
            goto PARSE_TREE_T9;
         }
      }
      if(arg[7][0]=='a' || arg[7][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[8],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[7][0]=='m'||arg[7][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[8]);
         memcpy(virt_if_entry.ospf_virt_if_md5key,arg[10],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TREE_T9:
   {
      if(parse_count9>1)
      {
         if(arg[9][0]=='t'||arg[9][0]=='T')
         {
            virt_if_entry.ospf_virt_if_transit_delay= atoi(arg[10]);
            parse_count11 = 1;
            goto PARSE_TREE_D11;
         }
      }
      if(parse_count9>0)
      {
         if(arg[9][0]=='d'||arg[9][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[10]);
            parse_count11 = 0;
            goto PARSE_TREE_D11;
         }
      }
      if(arg[9][0]=='a' || arg[9][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[10],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[9][0]=='m'||arg[9][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[10]);
         memcpy(virt_if_entry.ospf_virt_if_md5key,arg[12],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }


   PARSE_TREE_D11:
   {
      if(parse_count11>0)
      {
         if(arg[11][0]=='d'||arg[11][0]=='D')
         {
            virt_if_entry.ospf_virt_if_rtr_dead_interval= atoi(arg[12]);
            goto PARSE_TREE_A13;
         }
      }
      if(arg[11][0]=='a' || arg[11][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[12],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[11][0]=='m'||arg[11][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[12]);
         memcpy(virt_if_entry.ospf_virt_if_md5key,arg[14],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TREE_A13:
   {
      if(arg[13][0]=='a' || arg[13][0]=='A')
      {
         memcpy(virt_if_entry.ospf_virt_if_simplekey,arg[14],SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);
         goto PARSE_END;
      }
      else if(arg[13][0]=='m'||arg[13][0]=='M')
      {
         virt_if_entry.ospf_virt_if_md5keyid= atoi(arg[14]);
         memcpy(virt_if_entry.ospf_virt_if_md5key,arg[16],sizeof(UI8_T)*(SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN));
         goto PARSE_END;
      }
   }

   PARSE_TRUE_VALUE:
   {
      strcpy(auth_type, arg[3]);

      if(strcmp(auth_type, "authentication") == 0)
      {
         authentication=TRUE;
         if(arg[4]!=0)
         {
            strcpy(message_type, arg[4]);

            if(strcmp(message_type, "message-digest") == 0)
            {
               parse_count5 = 4;
               goto PARSE_TREE_TRUE_H5;
            }
            else if(arg[4][0]=='n' || arg[4][0]=='N')
            {
               parse_count5 = 4;
               goto PARSE_TREE_TRUE_H5;
            }
            else if(arg[4][0]=='h'||arg[4][0]=='H')
            {
               hello_interval=TRUE;
               parse_count6 = 3;
               goto PARSE_TREE_TRUE_R6;
            }
            else if(arg[4][0]=='r'||arg[4][0]=='R')
            {
               retransmit_interval=TRUE;
               parse_count6 = 2;
               goto PARSE_TREE_TRUE_R6;
            }
            else if(arg[4][0]=='t'||arg[4][0]=='T')
            {
               transmit_delay=TRUE;
               parse_count6 = 1;
               goto PARSE_TREE_TRUE_R6;
            }
            else if(arg[4][0]=='d'||arg[4][0]=='D')
            {
               dead_interval=TRUE;
               parse_count6 = 0;
               goto PARSE_TREE_TRUE_R6;
            }
            else if(arg[4][0]=='a' || arg[4][0]=='A')
            {
               authentication_key=TRUE;
               goto PARSE_END;
            }
            else if(arg[4][0]=='m'||arg[4][0]=='M')
            {
               //md5_keyid= atoi(arg[5]);
               message_digest_key=TRUE;
               goto PARSE_END;
            }
         }
      }

      else if(arg[3][0]=='h'||arg[3][0]=='H')
      {
         hello_interval=TRUE;
         parse_count5 = 3;
         goto PARSE_TREE_TRUE_H5;
      }
      else if(arg[3][0]=='r'||arg[3][0]=='R')
      {
         retransmit_interval=TRUE;
         parse_count5 = 2;
         goto PARSE_TREE_TRUE_H5;
      }
      else if(arg[3][0]=='t'||arg[3][0]=='T')
      {
         transmit_delay=TRUE;
         parse_count5 = 1;
         goto PARSE_TREE_TRUE_H5;
      }
      else if(arg[3][0]=='d'||arg[3][0]=='D')
      {
         dead_interval=TRUE;
         parse_count5 = 0;
         goto PARSE_TREE_TRUE_H5;
      }
      else if(arg[3][0]=='a' || arg[3][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[3][0]=='m'||arg[3][0]=='M')
      {
         //md5_keyid= atoi(arg[4]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_H5:
   {
      if(parse_count5>3)
      {
         if(arg[4][0]=='h'||arg[4][0]=='H')
         {
            hello_interval=TRUE;
            parse_count7 = 3;
            goto PARSE_TREE_TRUE_R7;
         }
      }
      if(parse_count5>2)
      {
         if(arg[4][0]=='r'||arg[4][0]=='R')
         {
            retransmit_interval=TRUE;
            parse_count7 = 2;
            goto PARSE_TREE_TRUE_R7;
         }
      }
      if(parse_count5>1)
      {
         if(arg[4][0]=='t'||arg[4][0]=='T')
         {
            transmit_delay=TRUE;
            parse_count7 = 1;
            goto PARSE_TREE_TRUE_R7;
         }
      }
      if(parse_count5>0)
      {
         if(arg[4][0]=='d'||arg[4][0]=='D')
         {
            dead_interval=TRUE;
            parse_count7 = 0;
            goto PARSE_TREE_TRUE_R7;
         }
      }
      if(arg[4][0]=='a' || arg[4][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[4][0]=='m'||arg[4][0]=='M')
      {
         //md5_keyid= atoi(arg[6]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_R6:
   {
      if(parse_count6>2)
      {
         if(arg[5][0]=='r'||arg[5][0]=='R')
         {
            retransmit_interval=TRUE;
            parse_count8 = 2;
            goto PARSE_TREE_TRUE_T8;
         }
      }
      if(parse_count6>1)
      {
         if(arg[5][0]=='t'||arg[5][0]=='T')
         {
            transmit_delay=TRUE;
            parse_count8 = 1;
            goto PARSE_TREE_TRUE_T8;
         }
      }
      if(parse_count6>0)
      {
         if(arg[5][0]=='d'||arg[5][0]=='D')
         {
            dead_interval=TRUE;
            parse_count8 = 0;
            goto PARSE_TREE_TRUE_T8;
         }
      }
      if(arg[5][0]=='a' || arg[5][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[5][0]=='m'||arg[5][0]=='M')
      {
         //md5_keyid= atoi(arg[7]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_T8:
   {
      if(parse_count8>1)
      {
         if(arg[6][0]=='t'||arg[6][0]=='T')
         {
            transmit_delay=TRUE;
            parse_count10 = 1;
            goto PARSE_TREE_TRUE_D10;
         }
      }
      if(parse_count8>0)
      {
         if(arg[6][0]=='d'||arg[6][0]=='D')
         {
            dead_interval=TRUE;
            parse_count10 = 0;
            goto PARSE_TREE_TRUE_D10;
         }
      }
      if(arg[6][0]=='a' || arg[6][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[6][0]=='m'||arg[6][0]=='M')
      {
         //md5_keyid= atoi(arg[9]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_D10:
   {
      if(parse_count10>0)
      {
         if(arg[7][0]=='d'||arg[7][0]=='D')
         {
            dead_interval=TRUE;
            parse_count12 = 0;
            goto PARSE_TREE_TRUE_A12;
         }
      }
      if(arg[7][0]=='a' || arg[7][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[7][0]=='m'||arg[7][0]=='M')
      {
         //md5_keyid= atoi(arg[11]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_A12:
   {
      if(arg[8][0]=='a' || arg[8][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[8][0]=='m'||arg[8][0]=='M')
      {
         //md5_keyid= atoi(arg[13]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_R7:
   {
      if(parse_count7>2)
      {
         if(arg[5][0]=='r'||arg[5][0]=='R')
         {
            retransmit_interval=TRUE;
            parse_count9 = 2;
            goto PARSE_TREE_TRUE_T9;
         }
      }
      if(parse_count7>1)
      {
         if(arg[5][0]=='t'||arg[5][0]=='T')
         {
            transmit_delay=TRUE;
            parse_count9 = 1;
            goto PARSE_TREE_TRUE_T9;
         }
      }
      if(parse_count7>0)
      {
         if(arg[5][0]=='d'||arg[5][0]=='D')
         {
            dead_interval=TRUE;
            parse_count9 = 0;
            goto PARSE_TREE_TRUE_T9;
         }
      }
      if(arg[5][0]=='a' || arg[5][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[5][0]=='m'||arg[5][0]=='M')
      {
         //md5_keyid= atoi(arg[8]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_T9:
   {
      if(parse_count9>1)
      {
         if(arg[6][0]=='t'||arg[6][0]=='T')
         {
            transmit_delay=TRUE;
            parse_count11 = 1;
            goto PARSE_TREE_TRUE_D11;
         }
      }
      if(parse_count9>0)
      {
         if(arg[6][0]=='d'||arg[6][0]=='D')
         {
            dead_interval=TRUE;
            parse_count11 = 0;
            goto PARSE_TREE_TRUE_D11;
         }
      }
      if(arg[6][0]=='a' || arg[6][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[6][0]=='m'||arg[6][0]=='M')
      {
         //md5_keyid= atoi(arg[10]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }


   PARSE_TREE_TRUE_D11:
   {
      if(parse_count11>0)
      {
         if(arg[7][0]=='d'||arg[7][0]=='D')
         {
            dead_interval=TRUE;
            goto PARSE_TREE_TRUE_A13;
         }
      }
      if(arg[7][0]=='a' || arg[7][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[7][0]=='m'||arg[7][0]=='M')
      {
         //md5_keyid= atoi(arg[12]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_TREE_TRUE_A13:
   {
      if(arg[8][0]=='a' || arg[8][0]=='A')
      {
         authentication_key=TRUE;
         goto PARSE_END;
      }
      else if(arg[8][0]=='m'||arg[8][0]=='M')
      {
         //md5_keyid= atoi(arg[14]);
         message_digest_key=TRUE;
         goto PARSE_END;
      }
   }

   PARSE_END:
   {
      UI32_T neighbor_id;

      CLI_LIB_AtoIp(arg[2], (UI8_T*)&neighbor_id);
      if(parse_true==TRUE)
      {
         if(authentication==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfAuthType(area_id,neighbor_id)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link auth type\r\n");
        }
     }
     if(hello_interval==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfHelloInterval(area_id,neighbor_id)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link hello interval\r\n");
        }
     }
     if(retransmit_interval==TRUE)
     {
            if(NETCFG_PMGR_ClearOspfVirtIfRetransInterval(area_id,neighbor_id)!=NETCFG_MGR_OK)
            {
           CLI_LIB_PrintStr("Failed to clear virtual link retransmit interval\r\n");
        }
     }
     if(transmit_delay==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfTransitDelay(area_id,neighbor_id)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link transmit delay\r\n");
        }
     }
     if(dead_interval==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfRtrDeadInterval(area_id,neighbor_id)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link dead interval\r\n");
        }
     }
     if(authentication_key==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfAuthKey(area_id,neighbor_id)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link authentication key\r\n");
        }
     }
     if(message_digest_key==TRUE)
     {
        if(NETCFG_PMGR_ClearOspfVirtIfAuthMD5Key(area_id,neighbor_id,0)!=NETCFG_MGR_OK)
        {
           CLI_LIB_PrintStr("Failed to clear virtual link message digest key\r\n");
        }
     }
      }
      else
      {
         if(now_status==VAL_ospfVirtIfStatus_createAndGo)
         {
            virt_if_entry.ospf_virt_if_status= VAL_ospfVirtIfStatus_createAndGo;
         }
         else if(now_status==L_RSTATUS_SET_OTHER_COLUMN)
         {
            virt_if_entry.ospf_virt_if_status = L_RSTATUS_SET_OTHER_COLUMN;
         }
         if(NETCFG_PMGR_SetOspfVirtIfEntry(&virt_if_entry)!= NETCFG_MGR_OK)
         {
            CLI_LIB_PrintStr("Failed to set virtual-link\r\n");
         }
      }
   }
#endif
#endif
   return CLI_NO_ERROR;
}

/* command: clear ip ospf */
UI32_T CLI_API_L3_Clear_Ip_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T ret;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id;

    if(!strncmp(arg[0], "process", 7))
    {
        ret = OSPF_PMGR_ClearOspfProcessAll(vr_id);
        if (ret != OSPF_TYPE_RESULT_SUCCESS)
        {
            CLI_LIB_PrintStr("Failed to clear all ospf process.\r\n");
        }
    }
    else
    {
        proc_id = atoi(arg[0]);
        ret = OSPF_PMGR_ClearOspfProcess(vr_id, proc_id);
        if ( ret != OSPF_TYPE_RESULT_SUCCESS )
        {
            CLI_LIB_PrintStr_1("Failed to clear ospf process %s.\r\n", arg[0]);
        }
    }
#endif
   return CLI_NO_ERROR;
}

/* command: compatible rfc1583 */
UI32_T CLI_API_L3_ROUTEROSPF_Compatible(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_OSPF == TRUE)
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_COMPATIBLE_RFC1583:

      if( NETCFG_PMGR_EnableOspfCompatibleRfc1583()!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to enable the OSPF compatible rfc1583.\r\n");
      }
      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_COMPATIBLE_RFC1583:

      if( NETCFG_PMGR_DisableOspfCompatibleRfc1583()!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
#endif
   return CLI_NO_ERROR;
}

/* command : default-information */
UI32_T CLI_API_L3_ROUTEROSPF_Default_information(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
   int i;
   UI32_T ret;
   int mtype;
   int metric;
   UI32_T vr_id = SYS_DFLT_VR_ID;
   UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;

    proc_id = ctrl_P->CMenu.process_id;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_DEFAULTINFORMATION_ORIGINATE:
     if ( OSPF_PMGR_DefaultInfoSet(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS )
       CLI_LIB_PrintStr("The OSPF default-infor set operation is failed.\r\n");
     /* Set other options */
     for (i = 0; arg[i];)
     {
       if (strncmp(arg[i], "metric-", 7) == 0)
       {
         mtype = atoi (arg[i + 1]);
         ret = OSPF_PMGR_DefaultInfoMetricTypeSet (vr_id, proc_id, mtype);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set metric type.\r\n");
         i += 2;
       }
       else if (strncmp (arg[i], "metric", 6) == 0)
       {
         metric = atoi (arg[i + 1]);
         ret = OSPF_PMGR_DefaultInfoMetricSet (vr_id, proc_id, metric);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set metric.\r\n");
         i += 2;
       }
       else if (strncmp (arg[i], "a", 1) == 0)
       {
         ret = OSPF_PMGR_DefaultInfoAlwaysSet (vr_id, proc_id);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set \"always\".\r\n");
         i += 1;
       }
       else if (strncmp (arg[i], "r", 1) == 0)
       {
         CLI_LIB_PrintStr("Not support route map now!\r\n");
         /* Not support filter now 2009.2.10 */
#if 0
         ret = OSPF_PMGR_DefaultInfoRoutemapSet (vr_id, proc_id, arg[i + 1]);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set route map.\r\n");
#endif
         i += 2;
       }
     }

     break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_DEFAULTINFORMATION_ORIGINATE:
       if ( !arg[0] )
           if ( OSPF_PMGR_DefaultInfoUnset(vr_id, proc_id) != OSPF_TYPE_RESULT_SUCCESS )
             CLI_LIB_PrintStr("The OSPF default-infor unset operation is failed.\r\n");
       /* Set other options */
       for (i = 0; arg[i];)
       {
         if (strncmp(arg[i], "metric-", 7) == 0)
         {
           i += 1;
           ret = OSPF_PMGR_DefaultInfoMetricTypeUnset (vr_id, proc_id);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to Unset metric type.\r\n");

         }
         else if (strncmp (arg[i], "metric", 6) == 0)
         {
           i += 1;
           ret = OSPF_PMGR_DefaultInfoMetricUnset (vr_id, proc_id);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to Unset metric.\r\n");

         }
         else if (strncmp (arg[i], "a", 1) == 0)
         {
           i += 1;
           ret = OSPF_PMGR_DefaultInfoAlwaysUnset (vr_id, proc_id);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to Unset \"always\".\r\n");

         }
         else if (strncmp (arg[i], "r", 1) == 0)
         {
           i += 1;
           CLI_LIB_PrintStr("Not support route map now!\r\n");
           /* Not support filter now 2009.2.10 */
#if 0
           ret = OSPF_PMGR_DefaultInfoRoutemapUnset (vr_id, proc_id);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to Unset route map.\r\n");
#endif
         }
       }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
   return CLI_NO_ERROR;
}

/* command: default metric */
UI32_T CLI_API_L3_ROUTEROSPF_Default_Metric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_OSPF == TRUE)
   OSPF_TYPE_OspfSystemGroup_T system_group_entry;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_DEFAULTMETRIC:

     if (NETCFG_PMGR_GetOspfDefaultInformation(&system_group_entry)!= NETCFG_MGR_OK)
     {
        CLI_LIB_PrintStr("Failed to get default value\r\n");
     }
     system_group_entry.ospf_def_met= atoi(arg[0]);
     /*system_group_entry.ospf_def_status=VAL_ospfNssaStatus_createAndGo;*/
     if(NETCFG_PMGR_SetOspfDefaultInformation(&system_group_entry)!= NETCFG_MGR_OK)
     {
        CLI_LIB_PrintStr("Failed to configure the OSPF default metric setting.\r\n");
     }
     break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_DEFAULTMETRIC:

      if( NETCFG_PMGR_ClearOspfDefaultInformationMetric()!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
#endif
   return CLI_NO_ERROR;

}


/* command: network address mask area area-id */
UI32_T CLI_API_L3_ROUTEROSPF_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_OSPF == TRUE)
   UI32_T ip_address = {0}, mask={0}, area_id=0;
   OSPF_TYPE_OspfNetworkAreaAddressEntry_T network_area_address_entry;

   memset(&network_area_address_entry, 0, sizeof(OSPF_TYPE_OspfNetworkAreaAddressEntry_T));

   CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_address);
   CLI_LIB_AtoIp(arg[1], (UI8_T*)&mask);
   CLI_LIB_AtoIp(arg[3], (UI8_T*)&area_id);

   network_area_address_entry.ospf_network_area_address = ip_address;
   network_area_address_entry.ospf_network_area_wildcard_mask = mask;
   network_area_address_entry.ospf_network_area_areaid = area_id;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_NETWORK:

      network_area_address_entry.ospf_network_area_status = VAL_ospfNetworkAreaStatus_createAndGo;

      if(NETCFG_PMGR_SetOspfNetworkAreaAddressEntry(network_area_address_entry)!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to configure the OSPF network function.\r\n");
      }

      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_NETWORK:

      network_area_address_entry.ospf_network_area_status = VAL_ospfNetworkAreaStatus_destroy;

      if(NETCFG_PMGR_SetOspfNetworkAreaAddressEntry(network_area_address_entry)!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
#endif
   return CLI_NO_ERROR;
}

/* command: ospf auto-cost */
UI32_T CLI_API_L3_ROUTEROSPF_Autocost(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
   UI32_T vr_id = SYS_DFLT_VR_ID;
   UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;

    proc_id = ctrl_P->CMenu.process_id;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_AUTOCOST_REFERENCEBANDWIDTH:
      if( OSPF_PMGR_AutoCostSet(vr_id, proc_id, atoi(arg[0]))!= OSPF_TYPE_RESULT_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to set the OSPF auto cost.\r\n");
      }
      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_AUTOCOST_REFERENCEBANDWIDTH:

      if( OSPF_PMGR_AutoCostUnset(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to unset the OSPF auto cost.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
   return CLI_NO_ERROR;
}

/* command: router-id ip-address */
UI32_T CLI_API_L3_ROUTEROSPF_Routerid(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_OSPF == TRUE)
   UI32_T ip_address = {0};

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_ROUTERID:

      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_address);

      if( NETCFG_PMGR_SetOspfRouterId(ip_address)!=NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to enable the OSPF router ID.\r\n");
      }
      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_ROUTERID:

      if( NETCFG_PMGR_ClearOspfRouterId()!=NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
#endif
   return CLI_NO_ERROR;
}
/* command: redistribute protocol */
UI32_T CLI_API_L3_ROUTEROSPF_Redistribute(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
   int i;
   UI32_T ret;
   int mtype;
   int metric;
   u_int32_t tag;
   UI32_T vr_id = SYS_DFLT_VR_ID;
   UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;

    proc_id = ctrl_P->CMenu.process_id;
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_REDISTRIBUTE:
     if ( OSPF_PMGR_RedistributeProtoSet(vr_id, proc_id, arg[0]) != OSPF_TYPE_RESULT_SUCCESS )
       CLI_LIB_PrintStr("The OSPF redistribute set operation is failed.\r\n");
     /* Set other options */
     for (i = 1; arg[i]; i += 2)
     {
       if (strncmp(arg[i], "metric-", 7) == 0)
       {
         mtype = atoi (arg[i + 1]);
         ret = OSPF_PMGR_RedistributeMetricTypeSet (vr_id, proc_id, arg[0], mtype);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set metric type.\r\n");

       }
       else if (strncmp (arg[i], "metric", 6) == 0)
       {
         metric = atoi (arg[i + 1]);
         ret = OSPF_PMGR_RedistributeMetricSet (vr_id, proc_id, arg[0], metric);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set metric.\r\n");

       }
       else if (strncmp (arg[i], "t", 1) == 0)
       {
         tag = strtoul(arg[i + 1], NULL, 0);
         ret = OSPF_PMGR_RedistributeTagSet (vr_id, proc_id, arg[0], tag);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set tag.\r\n");

       }
       else if (strncmp (arg[i], "r", 1) == 0)
       {
         CLI_LIB_PrintStr("Not support route map now!\r\n");
         /* Not support filter now 2009.2.10 */
#if 0

         ret = OSPF_PMGR_RedistributeRoutemapSet (vr_id, proc_id, arg[0], arg[i + 1]);
         if ( ret != OSPF_TYPE_RESULT_SUCCESS )
            CLI_LIB_PrintStr("Failed to set route map.\r\n");
#endif
       }
     }

     break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_REDISTRIBUTE:
       if ( !arg[1] )
         if ( OSPF_PMGR_RedistributeProtoUnset(vr_id, proc_id, arg[0]) != OSPF_TYPE_RESULT_SUCCESS )
           CLI_LIB_PrintStr("The OSPF redistribute unset operation is failed.\r\n");
       /* Set other options */
       for (i = 1; arg[i]; i += 1)
       {
         if (strncmp(arg[i], "metric-", 7) == 0)
         {
           ret = OSPF_PMGR_RedistributeMetricTypeUnset (vr_id, proc_id, arg[0]);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to unset metric type.\r\n");

         }
         else if (strncmp (arg[i], "metric", 6) == 0)
         {
           ret = OSPF_PMGR_RedistributeMetricUnset (vr_id, proc_id, arg[0]);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to unset metric.\r\n");

         }
         else if (strncmp (arg[i], "t", 1) == 0)
         {
           ret = OSPF_PMGR_RedistributeTagUnset (vr_id, proc_id, arg[0]);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to unset tag.\r\n");
         }
         else if (strncmp (arg[i], "r", 1) == 0)
         {
           CLI_LIB_PrintStr("Not support route map now!\r\n");
           /* Not support filter now 2009.2.10 */
#if 0
           ret = OSPF_PMGR_RedistributeRoutemapUnset (vr_id, proc_id, arg[0]);
           if ( ret != OSPF_TYPE_RESULT_SUCCESS )
              CLI_LIB_PrintStr("Failed to unset route map.\r\n");
#endif
         }
       }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
   return CLI_NO_ERROR;
}

/* command: summary-address */
UI32_T CLI_API_L3_ROUTEROSPF_Summary_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
   UI32_T ip_address = 0, mask = 0;
   UI32_T vr_id = SYS_DFLT_VR_ID;
   UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;
   UI8_T  mask_byte[SYS_ADPT_IPV4_ADDR_LEN];
   struct pal_in4_addr ip_addr;

    proc_id = ctrl_P->CMenu.process_id;
    CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_address);
    CLI_LIB_AtoIp(arg[1], (UI8_T*)&mask);
    IP_LIB_UI32toArray(mask, mask_byte);
   ip_addr.s_addr = ip_address;
   mask = IP_LIB_MaskToCidr(mask_byte);
   if ( mask == 0 && ip_address == 0 )
        CLI_LIB_PrintStr("Invalid configuration.\r\n");
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_SUMMARYADDRESS:

      if (OSPF_PMGR_SummaryAddressSet(vr_id, proc_id, ip_address, mask)!= OSPF_TYPE_RESULT_SUCCESS)
      {
        CLI_LIB_PrintStr("Failed to set summary address entry\r\n");
      }
      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_SUMMARYADDRESS:

      if (OSPF_PMGR_SummaryAddressUnset(vr_id, proc_id, ip_address, mask)!= OSPF_TYPE_RESULT_SUCCESS)
      {
         CLI_LIB_PrintStr("Failed to unset summary address entry\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
   return CLI_NO_ERROR;
}
/* command: timers spf */
UI32_T CLI_API_L3_ROUTEROSPF_Timers_Spf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if 0
#if (SYS_CPNT_OSPF == TRUE)
   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_TIMERS_SPF:

      if(NETCFG_PMGR_SetOspfSpfHoldTime(atoi(arg[0]))!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to enable the OSPF timers.\r\n");
      }
      break;

   case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_TIMERS_SPF:

      if(NETCFG_PMGR_ClearOspfSpfHoldTime()!= NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable the operation.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
#endif
   return CLI_NO_ERROR;
}
/* command: ip ospf */
UI32_T CLI_API_L3_Ip_Ospf_Interface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;

    UI32_T ifindex = ctrl_P->CMenu.vlan_ifindex;
    BOOL_T addr_flag = FALSE;
    UI32_T ip_addr = 0;
    struct pal_in4_addr addr;
    UI16_T ptr;

    switch(cmd_idx)
    {
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IP_OSPF:

            if(arg[0] == NULL)
            {
                CLI_LIB_PrintStr("Invalid command.\r\n");
            }
            if(CLI_LIB_CheckIPAddr(arg[0],&ptr) != 0)
            {
                switch(arg[0][0])
                {
                    case 'a':
                    case 'A':
                        if(strncmp(arg[0],"authentication-key",18) == 0)
                        {
                            char auth_key[SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN + 1] = {0};
                            if(strlen(arg[1]) == 0)
                                CLI_LIB_PrintStr("Invalid input value.\r\n");

                            strncpy(auth_key, arg[1], SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);

                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfAuthenticationKeySet(vr_id, ifindex, auth_key, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the OSPF auth key.\r\n");
                            }
                        }
                        else
                        {
                            UI8_T auth_type = NETCFG_TYPE_OSPF_AUTH_SIMPLE;

                            if(arg[1] != NULL)
                            {
                                if(arg[1][0]=='m' || arg[1][0]=='M')
                                {
                                    auth_type = NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC;
                                }
                                else if(arg[1][0]=='n' || arg[1][0]=='N')
                                {
                                    auth_type = NETCFG_TYPE_OSPF_AUTH_NULL;
                                }
                            }

                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfAuthenticationTypeSet(vr_id, ifindex, auth_type, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the OSPF auth type.\r\n");
                            }
                        }
                        break;
                    case 'm':
                    case 'M':
                        {
                            char auth_key[SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN + 1] = {0};
                            UI8_T key_id;
                            UI32_T rc = OSPF_TYPE_RESULT_FAIL;

                            if(strlen(arg[3]) == 0)
                                CLI_LIB_PrintStr("Invalid input value.\r\n");

                            key_id = atoi(arg[1]);
                            strncpy(auth_key, arg[3], SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN);

                            addr.s_addr = 0;
                            addr_flag = FALSE;
                            rc = OSPF_PMGR_IfMessageDigestKeySet(vr_id, ifindex, key_id, auth_key, addr_flag, addr);
                            switch(rc)
                            {
                                case OSPF_TYPE_RESULT_SUCCESS:
                                    //Do nothing
                                    break;
                                case OSPF_TYPE_RESULT_MD5_KEY_EXIST:
                                    CLI_LIB_PrintStr("Failed to set message digest key. The key already exists.\r\n");
                                    break;
                                default:
                                    CLI_LIB_PrintStr("Failed to set message digest key.\r\n");

                            }
                        }
                        break;
                    case 'p':
                    case 'P':
                        {
                            UI8_T priority;

                            priority = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfPrioritySet(vr_id, ifindex, priority, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF priority.\r\n");
                            }
                        }
                        break;
                    case 'c':
                    case 'C':
                        {
                            UI32_T cost;

                            cost = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfCostSet(vr_id, ifindex, cost, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF cost.\r\n");
                            }
                        }
                        break;
                    case 'd':
                    case 'D':
                        {
                            UI32_T interval;

                            interval = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfDeadIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF dead interval.\r\n");
                            }
                        }
                        break;
                    case 'h':
                    case 'H':
                        {
                            UI32_T interval;

                            interval = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfHelloIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF hello interval.\r\n");
                            }
                        }
                        break;
                    case 'r':
                    case 'R':
                        {
                            UI32_T interval;

                            interval = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfRetransmitIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF retransmit interval.\r\n");
                            }
                        }
                        break;
                    case 't':
                    case 'T':
                        {
                            UI32_T delay;

                            delay = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfTransmitDelaySet(vr_id, ifindex, delay, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF transmit delay.\r\n");
                            }
                        }
                        break;
                    default:
                        break;
                }

            }
            else
            {
                switch(arg[1][0])
                {
                    case 'a':
                    case 'A':
                        if(strncmp(arg[1],"authentication-key",18) == 0)
                        {
                            char auth_key[SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN + 1] = {0};
                            if(strlen(arg[2]) == 0)
                                CLI_LIB_PrintStr("Invalid input value.\r\n");

                            strncpy(auth_key, arg[2], SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN);

                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfAuthenticationKeySet(vr_id, ifindex, auth_key, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the OSPF auth key.\r\n");
                            }
                        }
                        else
                        {
                            UI8_T auth_type = NETCFG_TYPE_OSPF_AUTH_SIMPLE;

                            if(arg[2] != NULL)
                            {
                                if(arg[2][0]=='m' || arg[2][0]=='M')
                                {
                                    auth_type = NETCFG_TYPE_OSPF_AUTH_CRYPTOGRAPHIC;
                                }
                                else if(arg[2][0]=='n' || arg[2][0]=='N')
                                {
                                    auth_type = NETCFG_TYPE_OSPF_AUTH_NULL;
                                }
                            }

                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfAuthenticationTypeSet(vr_id, ifindex, auth_type, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the OSPF auth type.\r\n");
                            }
                        }
                        break;
                    case 'm':
                    case 'M':
                        {
                            char auth_key[SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN + 1] = {0};
                            UI8_T key_id;
                            UI32_T rc = OSPF_TYPE_RESULT_FAIL;

                            if(strlen(arg[4]) == 0)
                                CLI_LIB_PrintStr("Invalid input value.\r\n");

                            key_id = atoi(arg[2]);
                            strncpy(auth_key, arg[4], SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN);

                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;
                            rc = OSPF_PMGR_IfMessageDigestKeySet(vr_id, ifindex, key_id, auth_key, addr_flag, addr);
                            switch(rc)
                            {
                                case OSPF_TYPE_RESULT_SUCCESS:
                                    //Do nothing
                                    break;
                                case OSPF_TYPE_RESULT_MD5_KEY_EXIST:
                                    CLI_LIB_PrintStr("Failed to set message digest key. The key already exists.\r\n");
                                    break;
                                default:
                                    CLI_LIB_PrintStr("Failed to set message digest key.\r\n");
                            }
                        }
                        break;
                    case 'p':
                    case 'P':
                        {
                            UI8_T priority;

                            priority = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfPrioritySet(vr_id, ifindex, priority, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF priority.\r\n");
                            }
                        }
                        break;
                    case 'c':
                    case 'C':
                        {
                            UI32_T cost;

                            cost = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfCostSet(vr_id, ifindex, cost, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF cost.\r\n");
                            }
                        }
                        break;
                    case 'd':
                    case 'D':
                        {
                            UI32_T interval;

                            interval = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfDeadIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF dead interval.\r\n");
                            }
                        }
                        break;
                    case 'h':
                    case 'H':
                        {
                            UI32_T interval;

                            interval = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfHelloIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF hello interval.\r\n");
                            }
                        }
                        break;
                    case 'r':
                    case 'R':
                        {
                            UI32_T interval;

                            interval = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfRetransmitIntervalSet(vr_id, ifindex, interval, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF retransmit interval.\r\n");
                            }
                        }
                        break;
                    case 't':
                    case 'T':
                        {
                            UI32_T delay;

                            delay = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfTransmitDelaySet(vr_id, ifindex, delay, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to set the ip OSPF transmit delay.\r\n");
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            break;
        case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IP_OSPF:

            if(arg[0] == NULL)
            {
                CLI_LIB_PrintStr("Invalid command.\r\n");
            }
            if(CLI_LIB_CheckIPAddr(arg[0],&ptr) != 0)
            {
                switch(arg[0][0])
                {
                    case 'a':
                    case 'A':
                        if(strncmp(arg[0],"authentication-key",18) == 0)
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfAuthenticationKeyUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the OSPF auth key.\r\n");
                            }
                        }
                        else
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfAuthenticationTypeUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the OSPF auth type.\r\n");
                            }
                        }
                        break;
                    case 'm':
                    case 'M':
                        {
                            UI8_T key_id;

                            key_id = atoi(arg[1]);
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfMessageDigestKeyUnset(vr_id, ifindex, key_id, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF message digest key.\r\n");
                            }
                        }
                        break;
                    case 'p':
                    case 'P':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfPriorityUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF priority.\r\n");
                            }
                        }
                        break;
                    case 'c':
                    case 'C':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfCostUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF cost.\r\n");
                            }
                        }
                        break;
                    case 'd':
                    case 'D':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfDeadIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF dead interval.\r\n");
                            }
                        }
                        break;
                    case 'h':
                    case 'H':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfHelloIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF hello interval.\r\n");
                            }
                        }
                        break;
                    case 'r':
                    case 'R':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfRetransmitIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF retransmit interval.\r\n");
                            }
                        }
                        break;
                    case 't':
                    case 'T':
                        {
                            addr.s_addr = 0;
                            addr_flag = FALSE;

                            if(OSPF_PMGR_IfTransmitDelayUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF transmit delay.\r\n");
                            }
                        }
                        break;
                    default:
                        break;
                }

            }
            else
            {
                switch(arg[1][0])
                {
                    case 'a':
                    case 'A':
                        if(strncmp(arg[1],"authentication-key",18) == 0)
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfAuthenticationKeyUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the OSPF auth key.\r\n");
                            }
                        }
                        else
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfAuthenticationTypeUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the OSPF auth type.\r\n");
                            }
                        }
                        break;
                    case 'm':
                    case 'M':
                        {
                            UI8_T key_id;

                            key_id = atoi(arg[2]);
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfMessageDigestKeyUnset(vr_id, ifindex, key_id, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF message digest key.\r\n");
                            }
                        }
                        break;
                    case 'p':
                    case 'P':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfPriorityUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF priority.\r\n");
                            }
                        }
                        break;
                    case 'c':
                    case 'C':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfCostUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF cost.\r\n");
                            }
                        }
                        break;
                    case 'd':
                    case 'D':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfDeadIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF dead interval.\r\n");
                            }
                        }
                        break;
                    case 'h':
                    case 'H':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfHelloIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF hello interval.\r\n");
                            }
                        }
                        break;
                    case 'r':
                    case 'R':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfRetransmitIntervalUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF retransmit interval.\r\n");
                            }
                        }
                        break;
                    case 't':
                    case 'T':
                        {
                            CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
                            addr.s_addr = ip_addr;
                            addr_flag = TRUE;

                            if(OSPF_PMGR_IfTransmitDelayUnset(vr_id, ifindex, addr_flag, addr)!= OSPF_TYPE_RESULT_SUCCESS)
                            {
                                CLI_LIB_PrintStr("Failed to clear the ip OSPF transmit delay.\r\n");
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
            break;

        default:
            return CLI_NO_ERROR;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
    return CLI_NO_ERROR;
}

#if (SYS_CPNT_OSPF == TRUE)
/* command: show ip ospf [area-id]         border-routers
                         border-routers    database
                         database          virtual-links
                         interface
                         neighbor
                         summary-address
                         virtual-links                    */

/* command: show ip ospf neighbor */
static UI32_T show_ospf_neighbor(char *arg[])
{
#define IFSM_DROther            5
#define IFSM_Backup             6
#define IFSM_DR                 7
   UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   char  tmpBuf[100] = {0};
   UI32_T line_num = 0;
   UI8_T  ID_address[18] = {0},ip_address[18] = {0};
   char   state[30] = {0};
   OSPF_MGR_NBR_ENTRY_T  ospf_neighbor_entry;
   UI8_T flags = TRUE;
   UI32_T proc_id;

   memset(&ospf_neighbor_entry, 0, sizeof(OSPF_MGR_NBR_ENTRY_T));
   ospf_neighbor_entry.proc_id = 0;
   CLI_LIB_PrintStr("\r\n      ID          Pri        State          Address      Interface\r\n");
   CLI_LIB_PrintStr("--------------- ------ ---------------- --------------- --------------\r\n");

   if (arg[0][0] >= '0' && arg[0][0] <= '9')
   {
       ospf_neighbor_entry.proc_id = atoi(arg[0]);
       flags = FALSE;
   }
   do
   {
       proc_id = ospf_neighbor_entry.proc_id;
       memset(&ospf_neighbor_entry, 0, sizeof(OSPF_MGR_NBR_ENTRY_T));
       ospf_neighbor_entry.proc_id = proc_id;
       while( OSPF_PMGR_GetNextOspfNeighborEntry(&ospf_neighbor_entry) == OSPF_TYPE_RESULT_SUCCESS )
       {

          L_INET_Ntoa(ospf_neighbor_entry.ospf_nbr_rtr_id,ID_address);
          L_INET_Ntoa(ospf_neighbor_entry.ospf_nbr_ip_addr,ip_address);

          switch(ospf_neighbor_entry.ospf_nbr_state)
          {
             case VAL_ospfNbrState_down:
             strcpy(state,"DOWN");
             break;

             case VAL_ospfNbrState_attempt:
             strcpy(state,"ATTEMPT");
             break;

             case VAL_ospfNbrState_init:
             strcpy(state,"INIT");
             break;

             case VAL_ospfNbrState_twoWay:
             strcpy(state,"2WAY");
             break;

             case VAL_ospfNbrState_exchangeStart:
             strcpy(state,"EX START");
             break;

             case VAL_ospfNbrState_exchange:
             strcpy(state,"EXCHANGE");
             break;

             case VAL_ospfNbrState_loading:
             strcpy(state,"LOADING");
             break;

             case VAL_ospfNbrState_full:
             strcpy(state,"FULL");
             break;

             default:
             break;
          }
          /* Virtual neighbor has no DR/BDR */
          if ( ospf_neighbor_entry.virt_nbr_b )
              strcat(state,"/-");
          else
          {
              switch(ospf_neighbor_entry.router_role)
              {
                 case IFSM_DR:
                 strcat(state,"/DR");
                 break;

                 case IFSM_Backup:
                 strcat(state,"/BDR");
                 break;

                 case IFSM_DROther:
                 strcat(state,"/DROTHER");
                 break;

                 default:
                 break;
              }
          }
          sprintf(tmpBuf, "%15s %6d %16s %15s %15s\r\n",ID_address,ospf_neighbor_entry.ospf_nbr_priority,state,ip_address, ospf_neighbor_entry.ifname);
          PROCESS_MORE(tmpBuf);
       }
   }

    while(flags && OSPF_PMGR_GetNextProcessStatus(0, &ospf_neighbor_entry.proc_id) == OSPF_TYPE_RESULT_SUCCESS);
        return CLI_NO_ERROR;
}

static UI32_T show_ip_ospf_select_database(char* arg[])
{
    int loop;
    OSPF_MGR_LSA_ENTRY_T lsa_entry;
    UI32_T ip_address;
    char option_str[30];
    UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    char tmpBuf[20];
    char tmpBuf2[100];
    char tmpBuf3[100];
    int  not_link_id = FALSE;

    memset(&lsa_entry, 0, sizeof(lsa_entry));
    if (arg[0][0] >= '0' && arg[0][0] <= '9')
    {
        lsa_entry.proc_id = atoi(arg[0]);
        SET_FLAG(lsa_entry.flags, OSPF_GET_DATABASE_ID);
        arg++;
    }
    /* Show ip ospf database adv-router ||  Show ip ospf database [] adv-router */
    if( arg[1] && (!strcmp(arg[1], "adv-router") || !strcmp(arg[1], "ADV-ROUTER")) )

    {
        SET_FLAG(lsa_entry.flags, OSPF_SHOW_DATABASE_ADV_ROUTER);
        CLI_LIB_AtoIp(arg[2], (UI8_T*)&ip_address);
        lsa_entry.adv_router.s_addr = ip_address;
        not_link_id = TRUE;
    }
    if( arg[2] && (!strcmp(arg[2], "adv-router") || !strcmp(arg[2], "ADV-ROUTER")) )

    {
        SET_FLAG(lsa_entry.flags, OSPF_SHOW_DATABASE_ADV_ROUTER);
        CLI_LIB_AtoIp(arg[3], (UI8_T*)&ip_address);
        lsa_entry.adv_router.s_addr = ip_address;
        not_link_id = TRUE;
    }
    /* Show ip ospf database self-originate ||  Show ip ospf database [] self-originate */
    if( (arg[1] && (!strcmp(arg[1], "self-originate") || !strcmp(arg[1], "SELF-ORIGINATE")) )
        || (arg[2] && (!strcmp(arg[2], "self-originate") || !strcmp(arg[2], "SELF-ORIGINATE"))) )
    {
        SET_FLAG(lsa_entry.flags, OSPF_SHOW_DATABASE_SELF);
    }
    /* Show ip ospf database [] A.B.C.D */
    if ( (FALSE == not_link_id) && arg[2] && arg[2][0] >= '0' && arg[2][0] <= '9' )
    {
        SET_FLAG(lsa_entry.flags, OSPF_SHOW_DATABASE_ID);
        CLI_LIB_AtoIp(arg[2], (UI8_T*)&ip_address);
        lsa_entry.link_id.s_addr = ip_address;
    }
    /* show ip ospf databas brief */
    if ( arg[1] == NULL
         || (!strcmp(arg[1], "adv-router") || !strcmp(arg[1], "ADV-ROUTER"))
         || (!strcmp(arg[1], "self-originate") || !strcmp(arg[1], "SELF-ORIGINATE")) )
    {
        lsa_entry.show_brief_b = TRUE;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
            if (CHECK_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_PROC))
            {
                HexAddrToStr(lsa_entry.router_id.s_addr, tmpBuf, FALSE);
                sprintf(tmpBuf2, "\r\n            OSPF Router with ID (%s) (Process ID %ld)\r\n",
                                 tmpBuf, (long)lsa_entry.proc_id);
                PROCESS_MORE(tmpBuf2);
                UNSET_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_PROC);
            }

            if (CHECK_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_TYPE))
            {
                HexAddrToStr(lsa_entry.area_id.s_addr, tmpBuf, FALSE);
                if ( lsa_entry.type == OSPF_AS_EXTERNAL_LSA )
                    sprintf(tmpBuf2, "\r\n                %s \r\n\r\n",
                                     show_database_desc[lsa_entry.type]);
                else
                {
                    if ( lsa_entry.area_type == OSPF_AREA_NSSA )
                        sprintf(tmpBuf2, "\r\n                %s (Area %s [NSSA])\r\n\r\n",
                                         show_database_desc[lsa_entry.type], tmpBuf);
                    else if ( lsa_entry.area_type == OSPF_AREA_STUB )
                        sprintf(tmpBuf2, "\r\n                %s (Area %s [Stub])\r\n\r\n",
                                         show_database_desc[lsa_entry.type], tmpBuf);
                    else
                        sprintf(tmpBuf2, "\r\n                %s (Area %s)\r\n\r\n",
                                         show_database_desc[lsa_entry.type], tmpBuf);
                }
                PROCESS_MORE(tmpBuf2);
                UNSET_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_TYPE);
            }
            if (CHECK_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_BRIEF))
            {
                sprintf(tmpBuf2, "%s\r\n", show_database_header[lsa_entry.type]);
                PROCESS_MORE(tmpBuf2);
                UNSET_FLAG (lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_BRIEF);
            }
            /* LSA common part show. */
            HexAddrToStr(lsa_entry.header.id.s_addr, tmpBuf, FALSE);
            sprintf(tmpBuf2, "%-15s ", tmpBuf);
            HexAddrToStr(lsa_entry.header.adv_router.s_addr, tmpBuf, FALSE);
            sprintf(tmpBuf3, "%-15s %4d 0x%08lx 0x%04x",
                   tmpBuf, lsa_entry.header.ls_age,
                   (long)lsa_entry.header.ls_seqnum,
                   lsa_entry.header.checksum);
            strcat(tmpBuf2, tmpBuf3);
            /* LSA specific part show. */
            switch (lsa_entry.header.type)
            {
              case OSPF_ROUTER_LSA:
                sprintf(tmpBuf3, " %-ld", (long)lsa_entry.links);
                strcat(tmpBuf2, tmpBuf3);
                break;
              case OSPF_SUMMARY_LSA:
              {
                UI32_T route = 0;
                /* apply mask */
                IP_LIB_GetPrefixAddr((UI8_T *)&lsa_entry.link_id.s_addr, SYS_ADPT_IPV4_ADDR_LEN, lsa_entry.network_mask_len, (UI8_T *) &route);

                HexAddrToStr(route, tmpBuf, FALSE);
                sprintf(tmpBuf3, " %s/%ld", tmpBuf, (long)lsa_entry.network_mask_len);
                strcat(tmpBuf2, tmpBuf3);
                break;
              }
              case OSPF_AS_EXTERNAL_LSA:
          #ifdef HAVE_NSSA
              case OSPF_AS_NSSA_LSA:
          #endif /* HAVE_NSSA */
                HexAddrToStr(lsa_entry.link_id.s_addr, tmpBuf, FALSE);
                sprintf(tmpBuf3, " %s", lsa_entry.e_bit? "E2" : "E1");
                sprintf(tmpBuf + strlen(tmpBuf), "/%ld", (long)lsa_entry.network_mask_len);
                sprintf(tmpBuf3 + strlen(tmpBuf3), " %-18s", tmpBuf);
                sprintf(tmpBuf3 + strlen(tmpBuf3), " %lu", (unsigned long)lsa_entry.tag);
                strcat(tmpBuf2, tmpBuf3);
                break;
              case OSPF_NETWORK_LSA:
              case OSPF_SUMMARY_LSA_ASBR:
                break;
          #ifdef HAVE_OPAQUE_LSA
              case OSPF_LINK_OPAQUE_LSA:
              case OSPF_AREA_OPAQUE_LSA:
              case OSPF_AS_OPAQUE_LSA:
                sprintf(tmpBuf3, " %u", OPAQUE_ID (lsa->data->id));
                strcat(tmpBuf2, tmpBuf3);
                break;
          #endif /* HAVE_OPAQUE_LSA */
              default:
                break;
            }
            /* sprintf(tmpBuf2, " L=%d", lsa->lock); XXX */
            strcat(tmpBuf2, "\r\n");
            PROCESS_MORE(tmpBuf2);
        }
        return CLI_NO_ERROR;
    }
    /* Show ip ospf database router */
    SET_FLAG(lsa_entry.show_flags, OSPF_SHOW_DATABASE_HEADER_TYPE);
    if ( !strcmp(arg[1], "router") || !strcmp(arg[1], "ROUTER") )
    {
        lsa_entry.type = OSPF_ROUTER_LSA;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
              if ( lsa_entry.first_lsa_entry == TRUE)
              {
                  SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
              }
              for( loop = 0; loop < OSPF_MGR_LSA_LINK_NUMBER; loop ++ )
              {
                  if( 0 == lsa_entry.data.link[loop].id.s_addr )
                      break;
                  sprintf(tmpBuf2, "    Link connected to: %s\r\n",link_type_desc[lsa_entry.data.link[loop].type]);
                  PROCESS_MORE(tmpBuf2);
                  HexAddrToStr(lsa_entry.data.link[loop].id.s_addr, tmpBuf, FALSE);
                  sprintf(tmpBuf2, "     (Link ID) %s: %s\r\n",link_id_desc[lsa_entry.data.link[loop].type],tmpBuf);
                  PROCESS_MORE(tmpBuf2);
                  HexAddrToStr(lsa_entry.data.link[loop].data.s_addr, tmpBuf, FALSE);
                  sprintf(tmpBuf2, "     (Link Data) %s: %s\r\n",link_data_desc[lsa_entry.data.link[loop].type],tmpBuf );
                  PROCESS_MORE(tmpBuf2);
                  sprintf(tmpBuf2, "      Number of TOS metrics: %d\r\n", lsa_entry.data.link[loop].num_tos);
                  PROCESS_MORE(tmpBuf2);
                  sprintf(tmpBuf2, "      TOS 0 Metric: %d\r\n", lsa_entry.data.link[loop].metric);
              }
              strcat(tmpBuf2, "\r\n");
              PROCESS_MORE(tmpBuf2);
        }
    }
    /* Show ip ospf database network */
    if ( !strcmp(arg[1], "network") || !strcmp(arg[1], "NETWORK") )
    {
        lsa_entry.type = OSPF_NETWORK_LSA;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
              if ( lsa_entry.first_lsa_entry == TRUE)
              {
                  SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
                  sprintf (tmpBuf2, "  Network Mask: /%ld\r\n", (long)lsa_entry.network_mask_len);
                  PROCESS_MORE(tmpBuf2);
              }
              for( loop = 0; loop < OSPF_MGR_LSA_LINK_NUMBER; loop ++ )
              {
                  if( 0 == lsa_entry.data.neighbor_id[loop].s_addr )
                      break;
                  HexAddrToStr(lsa_entry.data.neighbor_id[loop].s_addr, tmpBuf, FALSE);
                  sprintf(tmpBuf2, "        Attached Router: %s\r\n", tmpBuf);
                  PROCESS_MORE(tmpBuf2);
              }
              sprintf(tmpBuf2, "\r\n");
              PROCESS_MORE(tmpBuf2);
        }
    }
    /* Show ip ospf database summary */
    if ( !strcmp(arg[1], "summary") || !strcmp(arg[1], "SUMMARY") )
    {
        lsa_entry.type = OSPF_SUMMARY_LSA;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
            SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
            sprintf(tmpBuf2, "  Network Mask: /%ld\r\n", (long)lsa_entry.network_mask_len);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        TOS: 0  Metric: %ld\r\n", (long)lsa_entry.summary_metric);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "\r\n");
            PROCESS_MORE(tmpBuf2);
        }
    }
    /* Show ip ospf database asbr-summary */
    if ( !strcmp(arg[1], "asbr-summary") || !strcmp(arg[1], "ASBR-SUMMARY") )
    {
        lsa_entry.type = OSPF_SUMMARY_LSA_ASBR;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
            SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
            sprintf(tmpBuf2, "  Network Mask: /%ld\r\n", (long)lsa_entry.network_mask_len);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        TOS: 0  Metric: %ld\r\n", (long)lsa_entry.summary_metric);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "\r\n");
            PROCESS_MORE(tmpBuf2);
        }
    }
    /* Show ip ospf database external */
    if ( !strcmp(arg[1], "external") || !strcmp(arg[1], "EXTERNAL") )
    {
        lsa_entry.type = OSPF_AS_EXTERNAL_LSA;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
            SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
            sprintf(tmpBuf2, "  Network Mask: /%ld\r\n", (long)lsa_entry.network_mask_len);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        Metric Type: %s\r\n",
                 lsa_entry.e_bit ?
                 "2 (Larger than any link state path)" : "1");
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        TOS: 0\r\n");
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        Metric: %ld\r\n", (long)lsa_entry.summary_metric);
            PROCESS_MORE(tmpBuf2);
            HexAddrToStr(lsa_entry.next_hop.s_addr, tmpBuf, FALSE);
            sprintf(tmpBuf2, "        Forward Address: %s\r\n", tmpBuf);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        External Route Tag: %lu\r\n\r\n", (unsigned long)lsa_entry.tag);
            PROCESS_MORE(tmpBuf2);
        }
    }
    /* Show ip ospf database nssa-external */
    if ( !strcmp(arg[1], "nssa-external") || !strcmp(arg[1], "NSSA-EXTERNAL") )
    {
        lsa_entry.type = OSPF_AS_NSSA_LSA;
        while( OSPF_PMGR_GetNextOspfLsaEntry(&lsa_entry) == OSPF_TYPE_RESULT_SUCCESS )
        {
            SHOW_IP_OSPF_DATABASE_HEADER(lsa_entry);
            sprintf(tmpBuf2, "  Network Mask: /%ld\r\n", (long)lsa_entry.network_mask_len);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        Metric Type: %s\r\n",
                 lsa_entry.e_bit ?
                 "2 (Larger than any link state path)" : "1");
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        TOS: 0\r\n");
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        Metric: %ld\r\n", (long)lsa_entry.summary_metric);
            PROCESS_MORE(tmpBuf2);
            HexAddrToStr(lsa_entry.next_hop.s_addr, tmpBuf, FALSE);
            sprintf(tmpBuf2, "        Forward Address: %s\r\n", tmpBuf);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "        External Route Tag: %lu\r\n\r\n", (unsigned long)lsa_entry.tag);
            PROCESS_MORE(tmpBuf2);
        }
    }
    return CLI_NO_ERROR;
}

static int show_ip_ospf(char* arg[])
{
    UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    char tmpBuf[20];
    char tmpBuf2[100];
    OSPF_MGR_OSPF_ENTRY_T ospf_entry;

    memset(&ospf_entry, 0, sizeof(OSPF_MGR_OSPF_ENTRY_T));
    if( arg[0] && (arg[0][0] <= '9') && (arg[0][0] >= '0') )
    {
        ospf_entry.proc_id = atoi(arg[0]);
        SET_FLAG(ospf_entry.show_flags, OSPF_GET_OSPF_ONE_PROC);
    }

    SET_FLAG(ospf_entry.show_flags, OSPF_SHOW_OSPF_HEADER_PROC);
    while ( OSPF_PMGR_GetNextOspfEntry(&ospf_entry) == OSPF_TYPE_RESULT_SUCCESS )
    {
        /* Show header */
        if ( CHECK_FLAG(ospf_entry.show_flags, OSPF_SHOW_OSPF_HEADER_PROC) )
        {
            UNSET_FLAG(ospf_entry.show_flags, OSPF_SHOW_OSPF_HEADER_PROC);
            /* Show Router ID. */
            HexAddrToStr(ospf_entry.router_id.s_addr, tmpBuf, FALSE);
            sprintf(tmpBuf2, " Routing Process \"ospf %ld\" with ID %s\r\n",
                    (long)ospf_entry.proc_id, tmpBuf);
            PROCESS_MORE(tmpBuf2);
            /* Show Uptime.  */
            if ( ospf_entry.day == 0 && ospf_entry.hour == 0 && ospf_entry.minute == 0 )
            {
                sprintf(tmpBuf2, " Process is not up\r\n");
            }
            else
            {
                sprintf(tmpBuf2," Process uptime is");
                if (ospf_entry.day != 0)
                {
                    sprintf(tmpBuf," %d day%s", ospf_entry.day, ospf_entry.day <= 1 ? "" : "s");
                    strcat(tmpBuf2, tmpBuf);
                }
                if (ospf_entry.day != 0 || ospf_entry.hour != 0)
                {
                  sprintf(tmpBuf," %d hour%s", ospf_entry.hour, ospf_entry.hour <= 1 ? "" : "s");
                  strcat(tmpBuf2, tmpBuf);
                }
                sprintf(tmpBuf," %d minute%s\r\n", ospf_entry.minute, ospf_entry.minute <= 1 ? "" : "s");
                strcat(tmpBuf2, tmpBuf);
            }
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, " Conforms to RFC2328, and RFC1583Compatibility flag is %s\r\n",
                    ospf_entry.compatible_rfc1583 ? "enabled" : "disabled");
            PROCESS_MORE(tmpBuf2);
            /* Show capability. */
            sprintf(tmpBuf2, " Supports only single TOS(TOS0) routes\r\n");
            PROCESS_MORE(tmpBuf2);
            /* Show ABR/ASBR flags. */
            if (ospf_entry.is_abr)
            {
                sprintf(tmpBuf2, " This router is an ABR, ABR Type is %s\r\n",
                        ospf_abr_type_descr_str[ospf_entry.abr_type]);
                PROCESS_MORE(tmpBuf2);
            }
            if (ospf_entry.is_asbr)
            {
                sprintf(tmpBuf2, " This router is an ASBR "
                         "(injecting external routing information)\r\n");
                PROCESS_MORE(tmpBuf2);
            }
            /* Show SPF timers. */
            sprintf(tmpBuf2, " SPF schedule delay %lu secs, "
                   "Hold time between two SPFs %lu secs\r\n",
                   (unsigned long)ospf_entry.spf_delay, (unsigned long)ospf_entry.spf_holdtime);
            PROCESS_MORE(tmpBuf2);
            /* Show refresh parameters. */
            sprintf(tmpBuf2, " Refresh timer %d secs\r\n", ospf_entry.interval);
            PROCESS_MORE(tmpBuf2);
            /* Show current DD exchange neighbors.  */
            sprintf(tmpBuf2, " Number of incoming current DD exchange neighbors %hu/%hu\r\n",
               ospf_entry.dd_count_in, ospf_entry.max_dd);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, " Number of outgoing current DD exchange neighbors %hu/%hu\r\n",
               ospf_entry.dd_count_out, ospf_entry.max_dd);
            PROCESS_MORE(tmpBuf2);
            /* Show number of AS-external-LSAs. */
            sprintf( tmpBuf2, " Number of external LSA %ld. Checksum 0x%06lX\r\n",
                           (long)ospf_entry.external_lsa_num,
                           (long)ospf_entry.checksum );
            PROCESS_MORE(tmpBuf2);
            /* Show number of AS-opaque-LSAs. */
            sprintf(tmpBuf2, " Number of opaque AS LSA %d. Checksum 0x%06X\r\n", 0, 0);
            PROCESS_MORE(tmpBuf2);
            /* Show lsdb limit if configured. */
            if (ospf_entry.overflow_lsdb_limit_b)
            {
                sprintf(tmpBuf2, " LSDB database overflow limit is %lu\r\n",
                        (unsigned long)ospf_entry.overflow_lsdb_limit);
                PROCESS_MORE(tmpBuf2);
                if (ospf_entry.overflow_lsdb_already)
                {
                   sprintf(tmpBuf2, " LSDB exceed overflow limit.\r\n");
                   PROCESS_MORE(tmpBuf2);
                }
            }
            sprintf(tmpBuf2, " Number of LSA originated %lu\r\n", (unsigned long)ospf_entry.lsa_originate_count);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, " Number of LSA received %lu\r\n", (unsigned long)ospf_entry.rx_lsa_count);
            PROCESS_MORE(tmpBuf2);
            /* Show number of areas attached. */
            sprintf(tmpBuf2, " Number of areas attached to this router: %ld\r\n", (long)ospf_entry.count_area);
            PROCESS_MORE(tmpBuf2);
            sprintf(tmpBuf2, "\r\n");
            PROCESS_MORE(tmpBuf2);
        }

        /**************************************
              ********* Show each area status**********
              ***************************************/

        HexAddrToStr(ospf_entry.area_id.s_addr, tmpBuf, FALSE);
        /* Show Area ID. */
        sprintf(tmpBuf2, "    Area %s", tmpBuf);
        PROCESS_MORE(tmpBuf2);
        /* Show Area type/mode. */
        if (ospf_entry.area.backbone_b)
        {
            sprintf(tmpBuf2, " (BACKBONE)");
            PROCESS_MORE(tmpBuf2);
        }
        else if (!ospf_entry.area.area_default)
        {
          sprintf(tmpBuf2, " (%s%s)",
                  LOOKUP (ospf_area_type_msg, ospf_entry.area.area_type),
                  ospf_entry.area.summary_flag ? ", no-summary" : "");

          PROCESS_MORE(tmpBuf2);
        }
        if (!ospf_entry.area.area_active)
        {
          sprintf(tmpBuf2, " (Inactive)");
          PROCESS_MORE(tmpBuf2);
        }
        sprintf(tmpBuf2, "\r\n");
        PROCESS_MORE(tmpBuf2);
        /* Show number of interfaces. */
        sprintf(tmpBuf2, "        Number of interfaces in this area is %d(%lu)\r\n",
                ospf_entry.area.active_if_count, (unsigned long)ospf_entry.area.if_count);
        PROCESS_MORE(tmpBuf2);
        /* Show number of fully adjacent neighbors. */
        sprintf(tmpBuf2, "        Number of fully adjacent neighbors in this area is "
             "%ld\r\n", (long)ospf_entry.area.full_nbr_count);
        PROCESS_MORE(tmpBuf2);
        /* Show number of fully adjacent virtual neighbors. */

        if (!ospf_entry.area.backbone_b)
        {
            sprintf(tmpBuf2, "        Number of fully adjacent virtual neighbors through "
                   "this area is %ld\r\n", (long)ospf_entry.area.full_virt_nbr_count);
            PROCESS_MORE(tmpBuf2);
        }
        /* Show authentication type. */
        sprintf(tmpBuf2, "        Area has ");
        if (ospf_entry.area.auth_type == OSPF_AUTH_NULL)
          strcat(tmpBuf2, "no authentication\r\n");
        else if (ospf_entry.area.auth_type == OSPF_AUTH_SIMPLE)
          strcat(tmpBuf2, "simple password authentication\r\n");
        else if (ospf_entry.area.auth_type == OSPF_AUTH_CRYPTOGRAPHIC)
          strcat(tmpBuf2, "message digest authentication\r\n");
        PROCESS_MORE(tmpBuf2);
        /* Show SPF calculation times. */
        if (ospf_entry.area.spf_calc_count > 0)
        {
            sprintf(tmpBuf2, "        SPF algorithm last executed "
                    "%02d:%02d:%02d.%03d ago\r\n",
                    ospf_entry.area.h,
                    ospf_entry.area.m,
                    ospf_entry.area.s,
                    ospf_entry.area.us );
            PROCESS_MORE(tmpBuf2);
        }

        sprintf(tmpBuf2, "        SPF algorithm executed %lu times\r\n",
             (unsigned long)ospf_entry.area.spf_calc_count);
        PROCESS_MORE(tmpBuf2);
        /* Show number of LSA. */
        sprintf(tmpBuf2, "        Number of LSA %ld. Checksum 0x%06lx\r\n",
                (long)ospf_entry.area.lsdb_count_all,
                (long)ospf_entry.area.lsdb_checksum_all);
        PROCESS_MORE(tmpBuf2);
        if ( OSPF_AREA_NSSA == ospf_entry.area.area_type )
        {
            sprintf(tmpBuf2, "        NSSA Translator State is ");
            if (ospf_entry.area.translator_state == OSPF_NSSA_TRANSLATOR_DISABLED)
                strcat (tmpBuf2, "disabled");
            else if (ospf_entry.area.translator_state == OSPF_NSSA_TRANSLATOR_ENABLED)
                strcat (tmpBuf2, "enabled");
            else if (ospf_entry.area.translator_state == OSPF_NSSA_TRANSLATOR_ELECTED)
                strcat (tmpBuf2, "elected");
            strcat (tmpBuf2, "\r\n");
            PROCESS_MORE(tmpBuf2);
        }
        /* Show short-cut status. */
        if (!ospf_entry.area.area_default)
        {
          sprintf(tmpBuf2, "        Shortcutting mode: %s, S-bit consensus: %s\r\n",
                  ospf_shortcut_mode_descr_str[ospf_entry.area.mode],
                  ospf_entry.area.shortcut_b? "ok" : "no");
          PROCESS_MORE(tmpBuf2);
        }

    }
    return 0;
}

/* Path Code String.  */
static char *ospf_path_code_str[] =
{
  "*",
  "C",
  "D",
  "O",
  "IA",
  "E1",
  "E2",
  "N1",
  "N2",
};

/* command: show ip ospf [area-id]         border-routers
                         border-routers    database
                         database          virtual-links
                         interface
                         neighbor
                         summary-address
                         virtual-links                    */

UI32_T CLI_API_L3_Show_Ip_Ospf(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}

#endif /* #if (SYS_CPNT_OSPF == TRUE) */

#if 0 /*These functions will be implemented in future -- xiongyu 20090112*/
#if (SYS_CPNT_OSPF == TRUE)

/* find ospf argument type */
static void show_ip_ospf_select_database(UI32_T i,UI32_T area_id,char *arg[])
{

static BOOL_T show_ip_ospf_paring_database(OSPF_TYPE_FilterLsdbBitmap_T *ospf_filter_bitmap,CLI_API_OspfDatabaseType_T database_type,UI32_T area_id,char *arg[],UI32_T i);
static UI32_T show_ospf_database(CLI_API_OspfDatabaseType_T database_type,BOOL_T all_area,UI32_T area_id,OSPF_TYPE_FilterLsdbBitmap_T ospf_filter_bitmap);
   OSPF_TYPE_FilterLsdbBitmap_T ospf_filter_bitmap;
   CLI_API_OspfDatabaseType_T database_type;

   memset(&ospf_filter_bitmap,0,sizeof(OSPF_TYPE_FilterLsdbBitmap_T));

   if(arg[i]==0)
   {
      database_type=DATABASE_TYPE_ALL;
   }
   else if(strncmp(arg[i],"ad",2) == 0)
   {
      database_type=DATABASE_TYPE_ADV_ROUTER;
   }
   else if(strncmp(arg[i],"as",2) == 0)
   {
      database_type=DATABASE_TYPE_ASBR_SUMMARY;
   }
   else if(strncmp(arg[i],"d",1) == 0)
   {
      database_type=DATABASE_TYPE_DATABASE_SUMMARY;
   }
   else if(strncmp(arg[i],"e",1) == 0)
   {
      database_type=DATABASE_TYPE_EXTERNAL;
   }
   else if(strncmp(arg[i],"ne",2) == 0)
   {
      database_type=DATABASE_TYPE_NETWORK;
   }
   else if(strncmp(arg[i],"ns",2) == 0)
   {
      database_type=DATABASE_TYPE_NSSA_EXTERNAL;
   }
   else if(strncmp(arg[i],"r",1) == 0)
   {
      database_type=DATABASE_TYPE_ROUTER;
   }
   else if(strncmp(arg[i],"se",2) == 0)
   {
      database_type=DATABASE_TYPE_SELF_ORIGINATE;
   }
   else if(strncmp(arg[i],"su",2) == 0)
   {
      database_type=DATABASE_TYPE_SUMMARY;
   }

   if(i==1)  /* all_area==TRUE */
   {
      show_ip_ospf_paring_database(&ospf_filter_bitmap,database_type,area_id,arg,i);
      show_ospf_database(database_type,TRUE,area_id,ospf_filter_bitmap);
   }
   else if(i==2) /* all_area==FALSE */
   {
      show_ip_ospf_paring_database(&ospf_filter_bitmap,database_type,area_id,arg,i);
      ospf_filter_bitmap.filter_area_id = 1;
      ospf_filter_bitmap.area_id = area_id;
      show_ospf_database(database_type,FALSE,area_id,ospf_filter_bitmap);
   }
}


static BOOL_T show_ip_ospf_paring_database(OSPF_TYPE_FilterLsdbBitmap_T *ospf_filter_bitmap,CLI_API_OspfDatabaseType_T database_type,UI32_T area_id,char *arg[],UI32_T i)
{
   UI32_T router_id=0;

   NETCFG_PMGR_GetOspfRouterId(&router_id);

   if(database_type==DATABASE_TYPE_ADV_ROUTER)
   {
      CLI_LIB_AtoIp(arg[i+1], (UI8_T*)&ospf_filter_bitmap->adv_router_ip);
      ospf_filter_bitmap->filter_adv_router_ip=1;
   }
   else if(database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      if(arg[i+1]!=0)
      {
     CLI_LIB_AtoIp(arg[i+1], (UI8_T*)&ospf_filter_bitmap->self_originate_link_state_id);
         ospf_filter_bitmap->filter_self_originate_link_state_id =2;
      }
      else
      {
         ospf_filter_bitmap->self_originate_link_state_id = router_id;
         ospf_filter_bitmap->filter_self_originate_link_state_id =1;
      }
   }
   else if(arg[i]==0 || database_type==DATABASE_TYPE_DATABASE_SUMMARY)
   {
      memset(&ospf_filter_bitmap,0,sizeof(OSPF_TYPE_FilterLsdbBitmap_T));
   }
   else
   {
      if(arg[i+1]==0)
      {
         return TRUE;
      }
      else if(arg[i+1][0]=='a' || arg[i+1][0]=='A')
      {
         CLI_LIB_AtoIp(arg[i+2], (UI8_T*)&ospf_filter_bitmap->adv_router_ip);
         ospf_filter_bitmap->filter_adv_router_ip=1;
      }
      else if(arg[i+1][0]=='s' || arg[i+1][0]=='S')
      {
         if(arg[i+2]!=0)
         {
            CLI_LIB_AtoIp(arg[i+2], (UI8_T*)&ospf_filter_bitmap->self_originate_link_state_id);
            ospf_filter_bitmap->filter_self_originate_link_state_id =2;
         }
         else
         {
            ospf_filter_bitmap->self_originate_link_state_id = router_id;
            ospf_filter_bitmap->filter_self_originate_link_state_id =1;
         }
      }
      else
      {
         CLI_LIB_AtoIp(arg[i+1], (UI8_T*)&ospf_filter_bitmap->link_state_id);
         ospf_filter_bitmap->filter_link_state_id =1;
         if(arg[i+2]==0)
         {
            return TRUE;
         }
         else if(arg[i+2][0]=='a' || arg[i+2][0]=='A')
         {
            CLI_LIB_AtoIp(arg[i+3], (UI8_T*)&ospf_filter_bitmap->adv_router_ip);
            ospf_filter_bitmap->filter_adv_router_ip=1;
         }
         else if(arg[i+2][0]=='s' || arg[i+2][0]=='S')
         {
            if(arg[i+3]!=0)
            {
               CLI_LIB_AtoIp(arg[i+3], (UI8_T*)&ospf_filter_bitmap->self_originate_link_state_id);
               ospf_filter_bitmap->filter_self_originate_link_state_id =2;
            }
            else
            {
               ospf_filter_bitmap->self_originate_link_state_id = router_id;
               ospf_filter_bitmap->filter_self_originate_link_state_id =1;
            }
         }
      }
   }
   return TRUE;
}



/* command: show ip ospf */
static UI32_T show_ospf()
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   UI32_T first_falg = 1;

   UI32_T router_id=0,asbdr_status=0,areabdr_status=0,area_no=0,ospf_interface_number_per_area=0;
   UI32_T is_simple=0, is_md5=0;
   UI8_T RedistributeProtocol[10];
   OSPF_TYPE_OspfAreaEntry_T ospf_area_entry;
   OSPF_TYPE_OspfRedistributeEntry_T redistribute_entry;
   OSPF_TYPE_OspfStubAreaEntry_T stub_area_entry;
   OSPF_TYPE_OspfNssaAreaEntry_T nssa_area_record;
   OSPF_TYPE_OspfAreaAggregateEntry_T area_aggregate_entry;

   memset(&ospf_area_entry,0,sizeof(OSPF_TYPE_OspfAreaEntry_T));
   memset(&redistribute_entry,0,sizeof(OSPF_TYPE_OspfRedistributeEntry_T));
   memset(&stub_area_entry, 0, sizeof(OSPF_TYPE_OspfStubAreaEntry_T));
   memset(&nssa_area_record, 0, sizeof(OSPF_TYPE_OspfNssaAreaEntry_T));
   memset(&area_aggregate_entry, 0, sizeof(OSPF_TYPE_OspfAreaAggregateEntry_T));

   if(NETCFG_PMGR_GetOspfRouterId(&router_id)==NETCFG_MGR_OK)
   {
      CLI_LIB_PrintStr_4("Routing Process with ID %d.%d.%d.%d\r\n", ((UI8_T *)(&(router_id)))[0], ((UI8_T *)(&(router_id)))[1], ((UI8_T *)(&(router_id)))[2], ((UI8_T *)(&(router_id)))[3]);
   }

   CLI_LIB_PrintStr("Supports only single TOS(TOS0) route\r\n");

   if((NETCFG_PMGR_GetOspfASBdrRtrStatus(&asbdr_status)==NETCFG_MGR_OK)&&(NETCFG_PMGR_GetOspfAreaBdrRtrStatus(&areabdr_status)==NETCFG_MGR_OK))
   {
      if(asbdr_status==VAL_ospfASBdrRtrStatus_true&&areabdr_status==VAL_ospfAreaBdrRtrStatus_true)
      {
         CLI_LIB_PrintStr("It is an area border and autonomous system boundary router\r\n");
      }
      else if(asbdr_status==VAL_ospfASBdrRtrStatus_false&&areabdr_status==VAL_ospfAreaBdrRtrStatus_true)
      {
         CLI_LIB_PrintStr("It is an area border router\r\n");
      }
      else if(asbdr_status==VAL_ospfASBdrRtrStatus_true&&areabdr_status==VAL_ospfAreaBdrRtrStatus_false)
      {
         CLI_LIB_PrintStr("It is an autonomous system boundary router\r\n");
      }
      else if(asbdr_status==VAL_ospfASBdrRtrStatus_false&&areabdr_status==VAL_ospfAreaBdrRtrStatus_false)
      {
         CLI_LIB_PrintStr("It is an internal router\r\n");
      }
   }

   first_falg = 1;
   memset(&redistribute_entry,0,sizeof(redistribute_entry));
   while(NETCFG_PMGR_GetNextOspfRedistributeEntry(&redistribute_entry)==NETCFG_MGR_OK)
   {
      if(redistribute_entry.ospf_redistribute_status == VAL_ospfRedistributeStatus_active)
      {
         if(first_falg == 1)
         {
         CLI_LIB_PrintStr("Redistributing External Routes from,\r\n");
            first_falg = 0;
         }

         switch(redistribute_entry.ospf_redistribute_protocol)
         {
            case VAL_ospfRedistributeProtocol_rip:
            strcpy(RedistributeProtocol,"rip");
            break;

            case VAL_ospfRedistributeProtocol_static:
            strcpy(RedistributeProtocol,"static");
            break;

            default:
            break;
         }

         CLI_LIB_PrintStr_2("     %s with metric mapped to %lu\r\n",RedistributeProtocol,redistribute_entry.ospf_redistribute_met);
      }
   }

   if(NETCFG_PMGR_GetOspfAreaNumber(&area_no)==NETCFG_MGR_OK)
   {
      CLI_LIB_PrintStr_1("Area number of this router is %lu\r\n",area_no);
   }

   if((NETCFG_PMGR_GetFirstOspfAreaEntry(&ospf_area_entry)==NETCFG_MGR_OK) &&
       ((ospf_area_entry.ospf_area_status == VAL_ospfAreaStatus_active) ||
        (ospf_area_entry.ospf_area_status == VAL_ospfAreaStatus_createAndGo)))
   {
      if(ospf_area_entry.ospf_area_id == 0)
      {
         CLI_LIB_PrintStr_4("Area %d.%d.%d.%d (BACKBONE)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
      }
      else
      {
         stub_area_entry.ospf_stub_area_id = ospf_area_entry.ospf_area_id;
         nssa_area_record.ospf_nssa_area_id = ospf_area_entry.ospf_area_id;
         if((NETCFG_PMGR_GetOspfStubEntry(&stub_area_entry) == NETCFG_MGR_OK) &&
            ((stub_area_entry.ospf_stub_status == VAL_ospfStubStatus_active) ||
            (stub_area_entry.ospf_stub_status == VAL_ospfStubStatus_createAndGo)))
         {
            CLI_LIB_PrintStr_4("Area %d.%d.%d.%d (STUB)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
         else if((NETCFG_PMGR_GetOspfNssaEntry(&nssa_area_record) == NETCFG_MGR_OK) &&
                 ((nssa_area_record.ospf_nssa_status == VAL_ospfNssaStatus_active) ||
                 (nssa_area_record.ospf_nssa_status == VAL_ospfNssaStatus_createAndGo)))
         {
            CLI_LIB_PrintStr_4("Area %d.%d.%d.%d (NSSA)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
         else
         {
            CLI_LIB_PrintStr_4("Area %d.%d.%d.%d\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
      }

      if(NETCFG_PMGR_GetOspfInterfaceNumberPerArea(ospf_area_entry.ospf_area_id,&ospf_interface_number_per_area)==NETCFG_MGR_OK)
      {
         CLI_LIB_PrintStr_1("     Number of interfaces in this area is %lu\r\n",ospf_interface_number_per_area);
      }

      if(NETCFG_PMGR_GetOspfAuthTypePerArea(ospf_area_entry.ospf_area_id,&is_simple,&is_md5)==NETCFG_MGR_OK)
      {
         if(is_simple==OSPF_TYPE_AUTH_TYPE_SIMPLE)
         {
            CLI_LIB_PrintStr("     Area has simple password authentication\r\n");
         }

         if(is_simple==OSPF_TYPE_AUTH_TYPE_MD5)
         {
            CLI_LIB_PrintStr("     Area has md5 authentication\r\n");
         }
      }
      CLI_LIB_PrintStr_1("     SPF algorithm executed %lu times\r\n",ospf_area_entry.ospf_spf_runs);
      /*
         pttch 2004/8/10 01:41 should not check aread_id, Core layer will get correct value
         if(ospf_area_entry.ospf_area_id != 0)
      */
      {
         UI32_T mask_len = 0;
         UI32_T first_flag = 1;

         area_aggregate_entry.ospf_area_aggregate_area_id = 0;
         area_aggregate_entry.ospf_area_aggregate_lsdb_type = 0;
         area_aggregate_entry.ospf_area_aggregate_net = 0;
         area_aggregate_entry.ospf_area_aggregate_mask = 0;
         /*
            pttch 2004/8/10 01:41 should move check area_id condiction to while loop or some entry will not get from getnext API
         */
         while(NETCFG_PMGR_GetNextOspfAreaAggregateEntry(&area_aggregate_entry) == NETCFG_MGR_OK)
         {
            if (area_aggregate_entry.ospf_area_aggregate_area_id != ospf_area_entry.ospf_area_id)
            {
               continue;
            }

            mask_len = IP_LIB_MaskToCidr(area_aggregate_entry.ospf_area_aggregate_mask);
            if((area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_active) ||
               (area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_createAndGo))
            if(first_flag)
            {
                sprintf(buff,"     Area ranges are\r\n");
                PROCESS_MORE(buff);
                first_flag = 0;
            }
            if((area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_active) ||
               (area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_createAndGo))
            {
                sprintf(buff,"        %d.%d.%d.%d/%ld  Active\r\n",((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[0],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[1],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[2],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[3],
                                       mask_len);
            }
            else
            {
                sprintf(buff,"        %d.%d.%d.%d/%ld\r\n",((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[0],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[1],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[2],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[3],
                                       mask_len);
            }

            PROCESS_MORE(buff);
         }
      }
   }

   memset(&ospf_area_entry,0,sizeof(OSPF_TYPE_OspfAreaEntry_T));
   memset(&redistribute_entry,0,sizeof(OSPF_TYPE_OspfRedistributeEntry_T));

   while((NETCFG_PMGR_GetNextOspfAreaEntry(&ospf_area_entry)==NETCFG_MGR_OK) &&
         ((ospf_area_entry.ospf_area_status == VAL_ospfAreaStatus_active) ||
          (ospf_area_entry.ospf_area_status == VAL_ospfAreaStatus_createAndGo)))
   {
      if(ospf_area_entry.ospf_area_id == 0)
      {
         sprintf(buff,"Area %d.%d.%d.%d (BACKBONE)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
      }
      else
      {
         stub_area_entry.ospf_stub_area_id = ospf_area_entry.ospf_area_id;
         nssa_area_record.ospf_nssa_area_id = ospf_area_entry.ospf_area_id;
         if((NETCFG_PMGR_GetOspfStubEntry(&stub_area_entry) == NETCFG_MGR_OK) &&
            ((stub_area_entry.ospf_stub_status == VAL_ospfStubStatus_active) ||
            (stub_area_entry.ospf_stub_status == VAL_ospfStubStatus_createAndGo)))
         {
            sprintf(buff,"Area %d.%d.%d.%d (STUB)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
         else if((NETCFG_PMGR_GetOspfNssaEntry(&nssa_area_record) == NETCFG_MGR_OK) &&
                 ((nssa_area_record.ospf_nssa_status == VAL_ospfNssaStatus_active) ||
                 (nssa_area_record.ospf_nssa_status == VAL_ospfNssaStatus_createAndGo)))
         {
            sprintf(buff,"Area %d.%d.%d.%d (NSSA)\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
         else
   {
      sprintf(buff, "Area %d.%d.%d.%d\r\n",((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[0], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[1], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[2], ((UI8_T *)(&(ospf_area_entry.ospf_area_id)))[3] );
         }
      }
      PROCESS_MORE(buff);

      if(NETCFG_PMGR_GetOspfInterfaceNumberPerArea(ospf_area_entry.ospf_area_id,&ospf_interface_number_per_area)==NETCFG_MGR_OK)
      {
         sprintf(buff, "     Number of interfaces in this area is %lu\r\n",ospf_interface_number_per_area);
         PROCESS_MORE(buff);
      }

      if(NETCFG_PMGR_GetOspfAuthTypePerArea(ospf_area_entry.ospf_area_id,&is_simple,&is_md5)==NETCFG_MGR_OK)
      {
         if(is_simple==OSPF_TYPE_AUTH_TYPE_SIMPLE)
         {
            sprintf(buff, "     Area has simple password authentication\r\n");
            PROCESS_MORE(buff);
         }
         else if(is_simple==OSPF_TYPE_AUTH_TYPE_MD5)
         {
            sprintf(buff, "     Area has md5 authentication\r\n");
            PROCESS_MORE(buff);
         }
      }
      sprintf(buff, "     SPF algorithm executed %lu times\r\n",ospf_area_entry.ospf_spf_runs);
      PROCESS_MORE(buff);
      /*
         pttch 2004/8/10 01:41 should not check aread_id, Core layer will get correct value
         if(ospf_area_entry.ospf_area_id != 0)
      */
      {
         UI32_T mask_len = 0;
         UI32_T first_flag = 1;

         area_aggregate_entry.ospf_area_aggregate_area_id = 0;
         area_aggregate_entry.ospf_area_aggregate_lsdb_type = 0;
         area_aggregate_entry.ospf_area_aggregate_net = 0;
         area_aggregate_entry.ospf_area_aggregate_mask = 0;
         /*
            pttch 2004/8/10 01:41 should move check area_id condiction to while loop or some entry will not get from getnext API
         */
         while(NETCFG_PMGR_GetNextOspfAreaAggregateEntry(&area_aggregate_entry) == NETCFG_MGR_OK)
         {
            if (area_aggregate_entry.ospf_area_aggregate_area_id != ospf_area_entry.ospf_area_id)
            {
               continue;
            }
            mask_len = IP_LIB_MaskToCidr(area_aggregate_entry.ospf_area_aggregate_mask);
            if((area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_active) ||
               (area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_createAndGo))
            if(first_flag)
            {
                sprintf(buff,"     Area ranges are\r\n");
                PROCESS_MORE(buff);
                first_flag = 0;
            }
            if((area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_active) ||
               (area_aggregate_entry.ospf_area_aggregate_status == VAL_ospfAreaAggregateStatus_createAndGo))
            {
                sprintf(buff,"        %d.%d.%d.%d/%ld  Active\r\n",((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[0],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[1],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[2],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[3],
                                       mask_len);
            }
            else
            {
                sprintf(buff,"        %d.%d.%d.%d/%ld\r\n",((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[0],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[1],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[2],
                                       ((UI8_T *)(&(area_aggregate_entry.ospf_area_aggregate_net)))[3],
                                       mask_len);
            }

            PROCESS_MORE(buff);
         }
      }
   }

   return TRUE;
}

/* command: show ip ospf [area-id] border-routers */
static UI32_T show_ospf_border_routers(BOOL_T all_area,UI32_T area_id)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;

   OSPF_TYPE_OspfBorderRouterEntry_T border_router_entry;
   UI8_T ip_str[18] ={0}, next_hop[18] ={0}, area_ip[18] = {0};
   UI8_T bor_type[10] = {0}, rte_type[10] = {0};

   memset(&border_router_entry, 0, sizeof(OSPF_TYPE_OspfBorderRouterEntry_T));


   CLI_LIB_PrintStr("\r\n  Destination      Next Hop      Cost   Type  RteType      Area        SPF No\r\n");
   CLI_LIB_PrintStr("--------------- --------------- ------ ----- -------- --------------- -------\r\n");

   while(NETCFG_PMGR_GetNextBorderRouterEntry(&border_router_entry)==NETCFG_MGR_OK)
   {
      if(all_area==FALSE)
      {
     if(border_router_entry.area_id != area_id)
         {
            continue;
         }
      }
      L_INET_Ntoa(border_router_entry.destination,ip_str);
      L_INET_Ntoa(border_router_entry.next_hop,next_hop);

      switch(border_router_entry.type)
      {
         case OSPF_TYPE_ABR:
         strcpy(bor_type,"ABR");
         break;

         case OSPF_TYPE_ABR_AND_ASBR:
         strcpy(bor_type,"ABR/ASBR");
         break;

         case OSPF_TYPE_ASBR:
         strcpy(bor_type,"ASBR");
         break;

         default:
         break;
      }
      switch(border_router_entry.rte_type)
      {
         case OSPF_TYPE_INTRA:
         strcpy(rte_type,"INTRA");
         break;

         case OSPF_TYPE_INTER:
         strcpy(rte_type,"INTER");
         break;

         default:
         break;
      }
      L_INET_Ntoa(border_router_entry.area_id,area_ip);

      sprintf(buff, "%15s %15s %6lu %5s %8s %15s %7lu\r\n",ip_str,next_hop,border_router_entry.cost,bor_type,rte_type,area_ip,border_router_entry.spf_number);
      PROCESS_MORE(buff);
   }

   return TRUE;
}

/* display ospf database header */
static UI32_T show_ospf_database_header(UI8_T *area_ip,UI32_T line_num,UI32_T type)
{
   switch(type)
   {
     case DATABASE_TYPE_ROUTER:
     CLI_LIB_PrintStr_1("\r\n\r\n     Displaying Router Link States(Area %s)\r\n",area_ip);
     break;

     case DATABASE_TYPE_NETWORK:
     CLI_LIB_PrintStr_1("\r\n\r\n     Displaying Net Link States(Area %s)\r\n",area_ip);
     break;

     case DATABASE_TYPE_SUMMARY:
     CLI_LIB_PrintStr_1("\r\n\r\n     Displaying Summary Net Link States(Area %s)\r\n",area_ip);
     break;

     case DATABASE_TYPE_ASBR_SUMMARY:
     CLI_LIB_PrintStr_1("\r\n\r\n     Displaying Summary ASB Link States(Area %s)\r\n",area_ip);
     break;

     case DATABASE_TYPE_EXTERNAL:
     CLI_LIB_PrintStr("\r\n\r\n     Displaying AS External Link States\r\n");
     break;

     case DATABASE_TYPE_NSSA_EXTERNAL:
     CLI_LIB_PrintStr_1("\r\n\r\n     Displaying Type-7 AS External Link States(Area %s)\r\n",area_ip);
     break;

   }
   CLI_LIB_PrintStr("       Link ID      ADV Router    Age      Seq#     Checksum\r\n");
   CLI_LIB_PrintStr("--------------- --------------- ------ ----------- -----------\r\n");
   line_num=line_num+5;
   return line_num;
}

static void initial_database_content(OSPF_TYPE_OspfLsdbEntry_T *lsdb_entry, OSPF_TYPE_RouterLsaInfo_T *router_lsa,
                                     OSPF_TYPE_NetworkLsaInfo_T *network_lsa,OSPF_TYPE_SummaryLsaInfo_T *summary_lsa,OSPF_TYPE_SummaryLsaInfo_T *as_summary_lsa,OSPF_TYPE_ExternalLsaInfo_T *as_external_lsa,UI32_T *count,UI32_T *previous_area_id)
{
   memset(lsdb_entry, 0, sizeof(OSPF_TYPE_OspfLsdbEntry_T));
   memset(router_lsa, 0, sizeof(OSPF_TYPE_RouterLsaInfo_T));
   memset(network_lsa, 0, sizeof(OSPF_TYPE_NetworkLsaInfo_T));
   memset(summary_lsa, 0, sizeof(OSPF_TYPE_SummaryLsaInfo_T));
   memset(as_summary_lsa, 0, sizeof(OSPF_TYPE_SummaryLsaInfo_T));
   memset(as_external_lsa, 0, sizeof(OSPF_TYPE_ExternalLsaInfo_T));
   *count = 0;
   *previous_area_id = 0;
}

/* command: show ip ospf [area-id] database */
static UI32_T show_ospf_database(CLI_API_OspfDatabaseType_T database_type,BOOL_T all_area,UI32_T area_id,OSPF_TYPE_FilterLsdbBitmap_T par_ospf_filter_bitmap)
{
   static UI32_T show_one_ospf_database_database_summary(OSPF_TYPE_LsaCount_T lsa_count_entry,UI32_T line_num,UI32_T count);
   static UI32_T show_one_ospf_database_external(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_ExternalLsaInfo_T as_external_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id);
   static UI32_T show_one_ospf_database_summary(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_SummaryLsaInfo_T summary_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id);
   static UI32_T show_one_ospf_database_network(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_NetworkLsaInfo_T network_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id,UI32_T router_count);
   static UI32_T show_one_ospf_database_router(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_RouterLsaInfo_T router_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id);
   static UI32_T show_one_ospf_database(OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,BOOL_T all_area,UI32_T line_num,UI32_T count,UI32_T previous_area_id,UI32_T type);

    /* 2006/06/28
     * ES4649-38-00189: free memory before return
     */
   static UI32_T show_one_ospf_database_router_macro(OSPF_TYPE_RouterLsaInfo_T router_lsa, UI32_T line_num, BOOL_T flag);
   static UI32_T show_ospf_database_header(UI8_T *area_ip,UI32_T line_num,UI32_T type);
   static void initial_database_content(OSPF_TYPE_OspfLsdbEntry_T *lsdb_entry, OSPF_TYPE_RouterLsaInfo_T *router_lsa,
                                        OSPF_TYPE_NetworkLsaInfo_T *network_lsa,OSPF_TYPE_SummaryLsaInfo_T *summary_lsa,OSPF_TYPE_SummaryLsaInfo_T *as_summary_lsa,OSPF_TYPE_ExternalLsaInfo_T *as_external_lsa,UI32_T *count,UI32_T *previous_area_id);

   UI32_T line_num = 0;
   OSPF_TYPE_OspfLsdbEntry_T lsdb_entry;
   OSPF_TYPE_RouterLsaInfo_T router_lsa;
   OSPF_TYPE_NetworkLsaInfo_T network_lsa;
   OSPF_TYPE_SummaryLsaInfo_T summary_lsa,as_summary_lsa;
   OSPF_TYPE_ExternalLsaInfo_T as_external_lsa;
   OSPF_TYPE_ExternalLsaInfo_T nssa_external_lsa;
   OSPF_TYPE_LsaCount_T lsa_count_entry;
   UI32_T count=0,router_count=0;
   UI32_T previous_area_id = 0;

   memset(&nssa_external_lsa, 0, sizeof(OSPF_TYPE_ExternalLsaInfo_T));
   memset(&lsa_count_entry, 0, sizeof(OSPF_TYPE_LsaCount_T));
   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_DATABASE_SUMMARY)
   {
      if(NETCFG_PMGR_GetOspfLsaCount(area_id,&lsa_count_entry)==NETCFG_MGR_OK)
      {
         line_num = show_one_ospf_database_database_summary(lsa_count_entry,line_num,count);
         if (line_num == JUMP_OUT_MORE)
         {
            return JUMP_OUT_MORE;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }

      }
      if(all_area==TRUE)
      {
         memset(&lsa_count_entry, 0, sizeof(OSPF_TYPE_LsaCount_T));
         while(NETCFG_PMGR_GetNextOspfLsaCountByAreaID(lsa_count_entry.area_id,&lsa_count_entry)==NETCFG_MGR_OK)
         {
            if((line_num = show_one_ospf_database_database_summary(lsa_count_entry,line_num,count))==JUMP_OUT_MORE)
            {
                return CLI_NO_ERROR;
            }
            else if (line_num == EXIT_SESSION_MORE)
            {
                return CLI_EXIT_SESSION;
            }

            count++;
         }
      }
      return TRUE;
   }

   if(database_type==DATABASE_TYPE_ALL)
   {
      /* NETCFG_PMGR_GetNextFilterRouterLinkStateEntry */

      while(NETCFG_PMGR_GetNextFilterRouterLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&router_lsa)==NETCFG_MGR_OK)
      {
           /* Derek */
           L_MM_Free(router_lsa.rlink);

         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_ROUTER))==JUMP_OUT_MORE)
         {
             return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }

      initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

      /* NETCFG_PMGR_GetNextFilterNetLinkStateEntry */
      while(NETCFG_PMGR_GetNextFilterNetLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&network_lsa,&router_count)==NETCFG_MGR_OK)
      {
          /*Derek */
          L_MM_Free(network_lsa.nlink);

         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_NETWORK))==JUMP_OUT_MORE)
         {
             return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
      initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

      /* NETCFG_PMGR_GetNextFilterNetSumLinkStateEntry */
      while(NETCFG_PMGR_GetNextFilterNetSumLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&summary_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_SUMMARY))==JUMP_OUT_MORE)
         {
             return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
      initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

      /* NETCFG_PMGR_GetNextFilterAsbrSumLinkStateEntry */

      while(NETCFG_PMGR_GetNextFilterAsbrSumLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&as_summary_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_ASBR_SUMMARY))==JUMP_OUT_MORE)
         {
             return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
      initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

      /* NETCFG_PMGR_GetNextFilterExternalLinkStateEntry */

      while(NETCFG_PMGR_GetNextFilterExternalLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&as_external_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_EXTERNAL))==JUMP_OUT_MORE)
         {
             return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }

      initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

      while(NETCFG_PMGR_GetNextFilterNssaExternalLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&nssa_external_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database(lsdb_entry,all_area,line_num,count,previous_area_id,DATABASE_TYPE_NSSA_EXTERNAL))==JUMP_OUT_MORE)
         //if((line_num = show_one_ospf_database_external(DATABASE_TYPE_NSSA_EXTERNAL,lsdb_entry,nssa_external_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
      return TRUE;
   }


   if(database_type==DATABASE_TYPE_ROUTER || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterRouterLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&router_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_router(DATABASE_TYPE_ROUTER,lsdb_entry,router_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }

   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_NETWORK || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterNetLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&network_lsa,&router_count)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_network(DATABASE_TYPE_NETWORK,lsdb_entry,network_lsa,line_num,count,previous_area_id,router_count))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }

   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_SUMMARY || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterNetSumLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&summary_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_summary(DATABASE_TYPE_SUMMARY,lsdb_entry,summary_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }

   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_ASBR_SUMMARY || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterAsbrSumLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&as_summary_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_summary(DATABASE_TYPE_ASBR_SUMMARY,lsdb_entry,as_summary_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }

   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_EXTERNAL || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterExternalLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&as_external_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_external(DATABASE_TYPE_EXTERNAL,lsdb_entry,as_external_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         else if (line_num == EXIT_SESSION_MORE)
         {
             return CLI_EXIT_SESSION;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }

   initial_database_content(&lsdb_entry,&router_lsa,&network_lsa,&summary_lsa,&as_summary_lsa,&as_external_lsa,&count,&previous_area_id);

   if(database_type==DATABASE_TYPE_NSSA_EXTERNAL || database_type==DATABASE_TYPE_ADV_ROUTER || database_type==DATABASE_TYPE_SELF_ORIGINATE)
   {
      while(NETCFG_PMGR_GetNextFilterNssaExternalLinkStateEntry(&par_ospf_filter_bitmap,&lsdb_entry,&nssa_external_lsa)==NETCFG_MGR_OK)
      {
         if((line_num = show_one_ospf_database_external(DATABASE_TYPE_NSSA_EXTERNAL,lsdb_entry,nssa_external_lsa,line_num,count,previous_area_id))==JUMP_OUT_MORE)
         {
            return CLI_NO_ERROR;
         }
         previous_area_id=lsdb_entry.ospf_lsdb_area_id;
         count++;
      }
   }
   return TRUE;
}

static UI32_T show_one_ospf_database(OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,BOOL_T all_area,UI32_T line_num,UI32_T count,UI32_T previous_area_id,UI32_T type)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T link_id[18] = {0},adv_router[18] = {0},sequence[20] = {0},checksum[20] = {0};
   UI8_T area_ip[18] = {0};


   L_INET_Ntoa(lsdb_entry.ospf_lsdb_lsid,link_id);
   L_INET_Ntoa(lsdb_entry.ospf_lsdb_router_id,adv_router);

   L_INET_Ntoa(lsdb_entry.ospf_lsdb_area_id,area_ip);
   if(lsdb_entry.ospf_lsdb_area_id!=previous_area_id || count==0)
   {
      line_num = show_ospf_database_header(area_ip,line_num,type);
   }
   sprintf (sequence,"0X%08X", (int)lsdb_entry.ospf_lsdb_sequence);
   sprintf (checksum,"0X%04X", (int)lsdb_entry.ospf_lsdb_checksum);

   sprintf(buff, "%15s %15s %6lu %11s %11s\r\n",link_id,adv_router,lsdb_entry.ospf_lsdb_age,sequence,checksum);
   PROCESS_MORE_FUNC(buff);

   return line_num;
}

static UI32_T show_one_ospf_database_header(UI32_T count,UI32_T line_num,UI32_T previous_area_id,UI32_T area_id,UI32_T type)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T router_ip[18] = {0},area_ip[18] = {0};
   UI32_T router_id=0;

   NETCFG_PMGR_GetOspfRouterId(&router_id);
   L_INET_Ntoa(router_id,router_ip);
   L_INET_Ntoa(area_id,area_ip);

   if(count==0)
   {
      sprintf(buff, "\r\nOSPF Router with id(%s)\r\n",router_ip);
      PROCESS_MORE_FUNC(buff);
   }

   if(area_id!=previous_area_id || count==0)
   {
      switch(type)
      {
         case DATABASE_TYPE_ROUTER:
         sprintf(buff, "\r\n          Displaying Router Link States(Area %s)\r\n\r\n",area_ip);
         break;

         case DATABASE_TYPE_NETWORK:
         sprintf(buff, "\r\n          Displaying Net Link States(Area %s)\r\n\r\n",area_ip);
         break;

         case DATABASE_TYPE_SUMMARY:
         sprintf(buff, "\r\n          Displaying Summary Net Link States(Area %s)\r\n\r\n",area_ip);
         break;

         case DATABASE_TYPE_ASBR_SUMMARY:
         sprintf(buff, "\r\n          Displaying Summary ASB Link States(Area %s)\r\n\r\n",area_ip);
         break;

         case DATABASE_TYPE_EXTERNAL:
         sprintf(buff, "\r\n          Displaying AS External Link States\r\n\r\n");
         break;

         case DATABASE_TYPE_NSSA_EXTERNAL:
         sprintf(buff, "\r\n          Displaying Type-7 AS External Link States(Area %s)\r\n\r\n",area_ip);
         break;
      }
      PROCESS_MORE_FUNC(buff);
   }
   return line_num;
}

static UI32_T show_one_ospf_detial_entry(UI32_T database_type,OSPF_TYPE_LsaHeader_T entry,BOOL_T flag,UI32_T line_num)
{

   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T  sOption[60] = {0}, preOption[60] = {0};
   UI8_T link_state_ip[18] = {0},adv_router_ip[18] = {0};


   switch(database_type)
   {
      case DATABASE_TYPE_ROUTER:
      sprintf(buff, "\r\nLink State Data Router (Type 1)\r\n");
      break;

      case DATABASE_TYPE_NETWORK:
      sprintf(buff, "\r\nLink State Data Network (Type 2)\r\n");
      break;

      case DATABASE_TYPE_SUMMARY:
      sprintf(buff, "\r\nLink State Data Summary (Type 3)\r\n");
      break;

      case DATABASE_TYPE_ASBR_SUMMARY:
      sprintf(buff, "\r\nLink State Data Summary ASB (Type 4)\r\n");
      break;

      case DATABASE_TYPE_EXTERNAL:
      sprintf(buff, "\r\nLink State Data External (Type 5)\r\n");
      break;

      case DATABASE_TYPE_NSSA_EXTERNAL:
      sprintf(buff, "\r\nLink State Data NSSA External (Type 7)\r\n");
      break;

   }
   PROCESS_MORE_FUNC(buff);
   PROCESS_MORE_FUNC("-------------------------------\r\n");

   sprintf(buff, "\r\nLS age: %u\r\n",entry.ls_age);
   PROCESS_MORE_FUNC(buff);

   if (entry.ls_option & 0x01)
   {
      sprintf(sOption, "TOS routing");
      memcpy(preOption, sOption,sizeof(sOption));
      flag = TRUE;
   }
   if (entry.ls_option & 0x02)
   {
      if (flag)
      {
         sprintf(sOption, "External routing, %s", preOption);
      }
      else
      {
     sprintf(sOption, "External routing");
     flag = TRUE;
      }
      memcpy(preOption, sOption,sizeof(sOption));
   }
   if (entry.ls_option & 0x04)
   {
      if (flag)
      {
         sprintf(sOption, "Multicast, %s", preOption);
      }
      else
      {
         sprintf(sOption, "Multicast");
         flag = TRUE;
      }
      memcpy(preOption, sOption,sizeof(sOption));
   }
   if (entry.ls_option & 0x08)
   {
      if (flag)
      {
         sprintf(sOption, "NSSA, %s", preOption);
      }
      else
      {
         sprintf(sOption, "NSSA");
         flag = TRUE;
      }
      memcpy(preOption, sOption,sizeof(sOption));
   }
   if (flag)
   {
      sprintf(sOption, "Support %s capability", preOption);
   }
   else
   {
      sprintf(sOption, "No capability support");
   }

   switch(database_type)
   {
      case DATABASE_TYPE_ROUTER:
   sprintf(buff, "Options: %s\r\nLS Type: Router Links\r\n",sOption);
      break;

      case DATABASE_TYPE_NETWORK:
      sprintf(buff, "Options: %s\r\nLS Type: Network Links\r\n",sOption);
      break;

      case DATABASE_TYPE_SUMMARY:
      sprintf(buff, "Options: %s\r\nLS Type: Summary Links(Network)\r\n",sOption);
      break;

      case DATABASE_TYPE_ASBR_SUMMARY:
      sprintf(buff, "Options: %s\r\nLS Type: Summary Links(AS Boundary Router)\r\n",sOption);
      break;

      case DATABASE_TYPE_EXTERNAL:
      sprintf(buff, "Options: %s\r\nLS Type: AS External Link\r\n",sOption);
      break;

      case DATABASE_TYPE_NSSA_EXTERNAL:
      sprintf(buff, "Options: %s\r\nLS Type: AS External Link\r\n",sOption);
      break;

   }
   PROCESS_MORE_FUNC(buff);

   L_INET_Ntoa(entry.ls_id,link_state_ip);
   switch(database_type)
   {
      case DATABASE_TYPE_ROUTER:
   sprintf(buff, "Link State ID: %s (Originating Router's Router ID)\r\n",link_state_ip);
      break;

      case DATABASE_TYPE_NETWORK:
      sprintf(buff, "Link State ID: %s (IP interface address of the Designated Router)\r\n",link_state_ip);
      break;

      case DATABASE_TYPE_SUMMARY:
      sprintf(buff, "Link State ID: %s (The destination Summary Network Number)\r\n",link_state_ip);
      break;

      case DATABASE_TYPE_ASBR_SUMMARY:
      sprintf(buff, "Link State ID: %s (AS Boundary Router's Router ID)\r\n",link_state_ip);
      break;

      case DATABASE_TYPE_EXTERNAL:
      sprintf(buff, "Link State ID: %s (External Network Number)\r\n",link_state_ip);
      break;

      case DATABASE_TYPE_NSSA_EXTERNAL:
      sprintf(buff, "Link State ID: %s (External IP Network Number)\r\n",link_state_ip);
      break;

   }
   PROCESS_MORE_FUNC(buff);

   L_INET_Ntoa(entry.ls_artr,adv_router_ip);
   sprintf(buff, "Advertising Router: %s\r\n",adv_router_ip);
   PROCESS_MORE_FUNC(buff);

   sprintf (buff,"LS Sequence Number: %08X\r\n", (int)entry.ls_seq);
   PROCESS_MORE_FUNC(buff);
   sprintf (buff,"LS Checksum: 0x%04X\r\n", (int)entry.ls_csum);
   PROCESS_MORE_FUNC(buff);
   sprintf (buff,"Length: %u\r\n",entry.ls_len);
   PROCESS_MORE_FUNC(buff);

   return line_num;
}

/* 2006/06/28
 * ES4649-38-00189: free memory before return
 */
static UI32_T show_one_ospf_database_router_macro(OSPF_TYPE_RouterLsaInfo_T router_lsa, UI32_T line_num, BOOL_T flag)
{
   UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T link_id_ip[18] = {0}, link_data_ip[18] = {0};
   UI8_T preOption[60] = {0},sLinkFlags[20] = {0};

   UI32_T i=0;

   flag = FALSE;
   memset(&preOption, 0, sizeof(preOption));
   if (router_lsa.flags & AFLAG_V)
   {
      sprintf(sLinkFlags, "Virtual Link");
      strcpy(preOption, sLinkFlags);
      flag = TRUE;
   }
   if (router_lsa.flags & AFLAG_E)
   {
      if (flag)
      {
         sprintf(sLinkFlags, "AS Boundary Router, %s", preOption);
      }
      else
      {
         sprintf(sLinkFlags, "AS Boundary Router");
         flag = TRUE;
      }
      strcpy(preOption, sLinkFlags);
   }
   if (router_lsa.flags & AFLAG_B)
   {
      if (flag)
      {
         sprintf(sLinkFlags, "Area Border Router, %s", preOption);
      }
      else
      {
         sprintf(sLinkFlags, "Area Border Router");
         flag = TRUE;
      }
      strcpy(preOption, sLinkFlags);
   }
   if (!flag)
   {
      sprintf(sLinkFlags, "None");
   }

   sprintf (buff,"Router Role: %s\r\n",sLinkFlags);
   PROCESS_MORE_FUNC(buff);

   sprintf (buff,"Number of Links: %u\r\n",router_lsa.cnt);
   PROCESS_MORE_FUNC(buff);

   for(i=0;i<router_lsa.cnt;i++)
   {
      CLI_LIB_PrintStr("-------------------------------------------------------\r\n");
      L_INET_Ntoa(router_lsa.rlink[i].lid,link_id_ip);

      /*if(router_lsa.rlink[i].type>0&&router_lsa.rlink[i].type<5)
      {
         sprintf (buff,"   Link ID:  %s (%s)\r\n",link_id_ip,link_id_string[router_lsa.rlink[i].type-1]);
         PROCESS_MORE_FUNC(buff);
      }*/

      L_INET_Ntoa(router_lsa.rlink[i].ldata,link_data_ip);

      /*if(router_lsa.rlink[i].type>0&&router_lsa.rlink[i].type<5)
      {
         sprintf (buff,"   Link Data: %s (%s)\r\n",link_data_ip,link_data_string[router_lsa.rlink[i].type-1]);
         PROCESS_MORE_FUNC(buff);
      }*/

      switch(router_lsa.rlink[i].type)
      {
         case 1:
         sprintf (buff,"   Link ID:   %s (Neighboring router's Router ID)\r\n",link_id_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Data: %s (Interface's MIB-II ifIndex)\r\n",link_data_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Type: Point to Point Connection\r\n");
         PROCESS_MORE_FUNC(buff);
         break;

         case 2:
         sprintf (buff,"   Link ID:   %s (IP address of Designated Router)\r\n",link_id_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Data: %s (Router interface's IP address)\r\n",link_data_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Type: Connection to a transit network\r\n");
         PROCESS_MORE_FUNC(buff);
         break;

         case 3:
         sprintf (buff,"   Link ID:   %s (IP Network/Subnet Number)\r\n",link_id_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Data: %s (Network's IP address mask)\r\n",link_data_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Type: Connection to a stub network\r\n");
         PROCESS_MORE_FUNC(buff);
         break;

         case 4:
         sprintf (buff,"   Link ID:   %s (Neighboring router's Router ID)\r\n",link_id_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Data: %s (Router interface's IP address)\r\n",link_data_ip);
         PROCESS_MORE_FUNC(buff);
         sprintf (buff,"   Link Type: Virtual Link\r\n");
         PROCESS_MORE_FUNC(buff);
         break;
      }


      sprintf (buff,"   Number of TOS metrics: %u\r\n",router_lsa.rlink[i].numtos);
      PROCESS_MORE_FUNC(buff);
      sprintf (buff,"   Metrics: %u\r\n\r\n",router_lsa.rlink[i].met);
      PROCESS_MORE_FUNC(buff);
   }

   L_MM_Free(router_lsa.rlink);
   return line_num;
}

static UI32_T show_one_ospf_database_router(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_RouterLsaInfo_T router_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id)
{
    BOOL_T flag=FALSE;

    line_num = show_one_ospf_database_header(count,line_num,previous_area_id,lsdb_entry.ospf_lsdb_area_id,database_type);
    if (line_num == JUMP_OUT_MORE)
    {
      if (router_lsa.rlink)
      {
          L_MM_Free(router_lsa.rlink);
          router_lsa.rlink = NULL;
      }
      return JUMP_OUT_MORE;
    }
    line_num = show_one_ospf_detial_entry(database_type,router_lsa.lsa_header,flag,line_num);

    if (line_num == JUMP_OUT_MORE)
    {
      if (router_lsa.rlink)
      {
          L_MM_Free(router_lsa.rlink);
          router_lsa.rlink = NULL;
      }
      return JUMP_OUT_MORE;
    }

    /* 2006/06/28
     * ES4649-38-00189: free memory before return
     */
    line_num = show_one_ospf_database_router_macro(router_lsa, line_num, flag);

    if (line_num == JUMP_OUT_MORE)
    {
      if (router_lsa.rlink)
      {
          L_MM_Free(router_lsa.rlink);
          router_lsa.rlink = NULL;
      }
      return JUMP_OUT_MORE;
    }
}


static UI32_T show_one_ospf_database_network(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_NetworkLsaInfo_T network_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id,UI32_T router_count)
{
   UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T mask_ip[18] = {0}, attach_router_ip[18] = {0};
   BOOL_T flag=FALSE;

   UI32_T i=0;

   line_num = show_one_ospf_database_header(count,line_num,previous_area_id,lsdb_entry.ospf_lsdb_area_id,database_type);
   if (line_num == JUMP_OUT_MORE)
   {
      if (network_lsa.nlink)
      {
          L_MM_Free(network_lsa.nlink);
          network_lsa.nlink = NULL;
      }
      return JUMP_OUT_MORE;
   }
   line_num = show_one_ospf_detial_entry(database_type,network_lsa.lsa_header,flag,line_num);
   if (line_num == JUMP_OUT_MORE)
   {
      if (network_lsa.nlink)
      {
          L_MM_Free(network_lsa.nlink);
          network_lsa.nlink = NULL;
      }
      return JUMP_OUT_MORE;
   }

   L_INET_Ntoa(network_lsa.mask,mask_ip);
   sprintf (buff,"Network Mask:    %s\r\n",mask_ip);
   PROCESS_MORE_FUNC(buff);

   PROCESS_MORE_FUNC("\r\n");
   for(i=0;i<router_count;i++)
   {
      L_INET_Ntoa(network_lsa.nlink[i].rid,attach_router_ip);
      sprintf (buff,"Attached Router: %s\r\n",attach_router_ip);
      PROCESS_MORE_FUNC(buff);
   }
   L_MM_Free(network_lsa.nlink);

   return line_num;
}

static UI32_T show_one_ospf_database_summary(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_SummaryLsaInfo_T summary_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id)
{
   UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T mask_ip[18] ={0};
   BOOL_T flag=FALSE;

   line_num = show_one_ospf_database_header(count,line_num,previous_area_id,lsdb_entry.ospf_lsdb_area_id,database_type);
   if (line_num == JUMP_OUT_MORE)
   {
      return JUMP_OUT_MORE;
   }
   line_num = show_one_ospf_detial_entry(database_type,summary_lsa.lsa_header,flag,line_num);
   if (line_num == JUMP_OUT_MORE)
   {
      return JUMP_OUT_MORE;
   }

   L_INET_Ntoa(summary_lsa.mask,mask_ip);
   sprintf (buff,"Network Mask: %s\r\n",mask_ip);
   PROCESS_MORE_FUNC(buff);
   sprintf (buff,"Metric:       %lu\r\n\r\n",summary_lsa.met);
   PROCESS_MORE_FUNC(buff);

   return line_num;
}

static UI32_T show_one_ospf_database_external(UI32_T database_type,OSPF_TYPE_OspfLsdbEntry_T lsdb_entry,OSPF_TYPE_ExternalLsaInfo_T as_external_lsa,UI32_T line_num,UI32_T count,UI32_T previous_area_id)
{
   UI8_T buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T mask_ip[18] ={0},forward_ip[18] ={0};
   BOOL_T flag=FALSE;
   UI8_T metric_buff[CLI_DEF_MAX_BUFSIZE] = {0};

   line_num = show_one_ospf_database_header(count,line_num,previous_area_id,lsdb_entry.ospf_lsdb_area_id,database_type);
   if (line_num == JUMP_OUT_MORE)
   {
      return JUMP_OUT_MORE;
   }
   line_num = show_one_ospf_detial_entry(database_type,as_external_lsa.lsa_header,flag,line_num);
   if (line_num == JUMP_OUT_MORE)
   {
      return JUMP_OUT_MORE;
   }

   L_INET_Ntoa(as_external_lsa.mask,mask_ip);
   sprintf (buff,"Network Mask: %s\r\n",mask_ip);
   PROCESS_MORE_FUNC(buff);

   switch(as_external_lsa.ext_type+1)
   {
      case 1:
         strcpy(metric_buff,"(Comparable directly to link state metric)");
         break;
      case 2:
         strcpy(metric_buff,"(Larger than any link state path)");
         break;
      case 0:
      default:
         strcpy(metric_buff,"\0");
         break;
   }

   sprintf (buff,"Metric Type:        %lu%s\r\n",as_external_lsa.ext_type+1,metric_buff);
   PROCESS_MORE_FUNC(buff);
   sprintf (buff,"Metric:             %lu\r\n",as_external_lsa.met);
   PROCESS_MORE_FUNC(buff);
   L_INET_Ntoa(as_external_lsa.fwd,forward_ip);
   sprintf (buff,"Forward Address:    %s\r\n",forward_ip);
   PROCESS_MORE_FUNC(buff);
   sprintf (buff,"External Route Tag: %lu\r\n\r\n",as_external_lsa.tag);
   PROCESS_MORE_FUNC(buff);

   return line_num;
}

static UI32_T show_one_ospf_database_database_summary(OSPF_TYPE_LsaCount_T lsa_count_entry,UI32_T line_num,UI32_T count)
{
   UI8_T router_ip[18] = {0},area_ip[18] = {0};
   UI32_T router_id = 0;
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};

   NETCFG_PMGR_GetOspfRouterId(&router_id);
   L_INET_Ntoa(router_id,router_ip);
   L_INET_Ntoa(lsa_count_entry.area_id,area_ip);
#if 0
   if(count==0)
   {
      sprintf(buff, "\r\n            OSPF Router with id(%s)\r\n",router_ip);
      PROCESS_MORE_FUNC(buff);
   }
#endif
   sprintf(buff, "\r\n\r\nArea ID (%s)\r\n",area_ip);
   PROCESS_MORE_FUNC(buff);

   sprintf(buff, "       Router    Network    Sum-Net    Sum-ASBR    External-AS     External-Nssa\r\n");
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "          %lu         %lu          %lu          %lu             %lu               %lu\r\n",
           lsa_count_entry.router_lsa_count,lsa_count_entry.network_lsa_count,lsa_count_entry.summary_lsa_count,lsa_count_entry.as_summary_lsa_count,lsa_count_entry.as_external_lsa_count,lsa_count_entry.nssa_external_lsa_count);
   PROCESS_MORE_FUNC(buff);
   sprintf(buff, "Total LSA Counts : %lu\r\n",lsa_count_entry.total_lsa_count);
   PROCESS_MORE_FUNC(buff);
   return line_num;
}

/* command: show ip ospf neighbor */
static UI32_T show_ospf_neighbor(BOOL_T all_area,UI32_T area_id)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   UI8_T ID_address[18] = {0},ip_address[18] = {0},state[30] = {0};
   OSPF_TYPE_OspfNbrEntry_T  ospf_neighbor_entry;
   UI32_T router_role=0;

   memset(&ospf_neighbor_entry, 0, sizeof(OSPF_TYPE_OspfNbrEntry_T));

   CLI_LIB_PrintStr("\r\n      ID          Pri        State          Address\r\n");
   CLI_LIB_PrintStr("--------------- ------ ---------------- ---------------\r\n");

   while(NETCFG_PMGR_GetNextOspfNeighborEntry(&ospf_neighbor_entry,&router_role)==NETCFG_MGR_OK)
   {

      L_INET_Ntoa(ospf_neighbor_entry.ospf_nbr_rtr_id,ID_address);
      L_INET_Ntoa(ospf_neighbor_entry.ospf_nbr_ip_addr,ip_address);

      switch(ospf_neighbor_entry.ospf_nbr_state)
      {
         case VAL_ospfNbrState_down:
         strcpy(state,"DOWN");
         break;

         case VAL_ospfNbrState_attempt:
         strcpy(state,"ATTEMPT");
         break;

         case VAL_ospfNbrState_init:
         strcpy(state,"INIT");
         break;

         case VAL_ospfNbrState_twoWay:
         strcpy(state,"2WAY");
         break;

         case VAL_ospfNbrState_exchangeStart:
         strcpy(state,"EX START");
         break;

         case VAL_ospfNbrState_exchange:
         strcpy(state,"EXCHANGE");
         break;

         case VAL_ospfNbrState_loading:
         strcpy(state,"LOADING");
         break;

         case VAL_ospfNbrState_full:
         strcpy(state,"FULL");
         break;

         default:
         break;
      }

      switch(router_role)
      {
         case OSPF_TYPE_DR:
         strcat(state,"/DR");
         break;

         case OSPF_TYPE_BDR:
         strcat(state,"/BDR");
         break;

         case OSPF_TYPE_DROTHER:
         strcat(state,"/DROTHER");
         break;

         default:
         break;
      }

      sprintf(buff, "%15s %6lu %16s %15s\r\n",ID_address,ospf_neighbor_entry.ospf_nbr_priority,state,ip_address);
      PROCESS_MORE(buff);
   }
   return TRUE;
}
/* command: show ip ospf summary-address */
static UI32_T show_ospf_summary_address(BOOL_T all_area,UI32_T area_id)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   OSPF_TYPE_OspfSummaryAddressEntry_T  summary_address_entry;

   memset(&summary_address_entry, 0, sizeof(OSPF_TYPE_OspfSummaryAddressEntry_T));

   while(NETCFG_PMGR_GetNextOspfSummaryAddressEntry(&summary_address_entry)==NETCFG_MGR_OK)
   {
      if(summary_address_entry.ospf_summary_address==0)
      {
         break;
      }
      sprintf(buff, "%d.%d.%d.%d/%d.%d.%d.%d\r\n",((UI8_T *)(&summary_address_entry.ospf_summary_address))[0],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_address))[1],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_address))[2],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_address))[3]
                                                 ,((UI8_T *)(&summary_address_entry.ospf_summary_mask))[0],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_mask))[1],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_mask))[2],
                                                  ((UI8_T *)(&summary_address_entry.ospf_summary_mask))[3]);
      PROCESS_MORE(buff);
   }
   return TRUE;
}
/* command: show ip ospf [area-id] virtual-links */
static UI32_T show_ospf_virtual_links(BOOL_T all_area,UI32_T area_id)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   OSPF_TYPE_OspfVirtIfEntry_T  ospf_virtlink_entry;
   OSPF_TYPE_OspfVirtNbrEntry_T    ospf_virt_nbr_entry;

   memset(&ospf_virtlink_entry, 0, sizeof(OSPF_TYPE_OspfVirtIfEntry_T));
   memset(&ospf_virt_nbr_entry,0,sizeof(OSPF_TYPE_OspfVirtNbrEntry_T));

   while(NETCFG_PMGR_GetNextOspfVirtIfEntry(&ospf_virtlink_entry)==NETCFG_MGR_OK)
   {
      if(all_area==FALSE)
      {
     if(ospf_virtlink_entry.ospf_virt_if_area_id != area_id)
         {
            continue;
         }
      }
      ospf_virt_nbr_entry.ospf_virt_nbr_area = ospf_virtlink_entry.ospf_virt_if_area_id;
      ospf_virt_nbr_entry.ospf_virt_nbr_rtr_id = ospf_virtlink_entry.ospf_virt_if_neighbor;

      if( NETCFG_PMGR_GetOspfVirtNbrEntry(&ospf_virt_nbr_entry) == NETCFG_MGR_OK)
      {
          if(ospf_virt_nbr_entry.ospf_virt_nbr_state == VAL_ospfVirtNbrState_down)
          {

              sprintf(buff, "Virtual Link to router %d.%d.%d.%d is down\r\n",((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[0],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[1],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[2],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[3]);
          }
          else
          {
      sprintf(buff, "Virtual Link to router %d.%d.%d.%d is up\r\n",((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[0],
                                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[1],
                                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[2],
                                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[3]);
          }
      }
      else
      {
           sprintf(buff, "Virtual Link to router %d.%d.%d.%d is down\r\n",((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[0],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[1],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[2],
                                                                           ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_neighbor))[3]);
      }


      PROCESS_MORE(buff);
      sprintf(buff, "Transit area %d.%d.%d.%d\r\n",((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_area_id))[0],
                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_area_id))[1],
                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_area_id))[2],
                                                   ((UI8_T *)(&ospf_virtlink_entry.ospf_virt_if_area_id))[3]);

      PROCESS_MORE(buff);
      sprintf(buff, "Transmit Delay is %lu sec\r\n",ospf_virtlink_entry.ospf_virt_if_transit_delay);
      PROCESS_MORE(buff);
      sprintf(buff, "Timer intervals configured, Hello %lu, Dead %lu, Retransmit %lu\r\n",ospf_virtlink_entry.ospf_virt_if_hello_interval,ospf_virtlink_entry.ospf_virt_if_rtr_dead_interval,ospf_virtlink_entry.ospf_virt_if_retrans_interval);
      PROCESS_MORE(buff);
   }
   return TRUE;
}
/* command: show ip ospf interface */
static UI32_T show_ospf_interface(BOOL_T all_area,UI32_T area_id,UI32_T vid)
{
   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;
   UI32_T vlan_first_entry_flag = 0;
   UI32_T first_flag = 1;

   UI32_T vid_ifindex=0, rif_num=0, router_id=0,ifindex=0;
   UI32_T rif_vid = 0;
   UI8_T ip_str[18] = {0},ip_mask[18] = {0},area_ip[18] = {0},router_ip[18] = {0},
         dr_ip[18] = {0}, bdr_ip[18] = {0}, state_role[10] = {0},
         dr_interface_ip[18] = {0},bdr_interface_ip[18] = {0};


   NETCFG_IP_ADDRESS_MODE_T access_mode;
   NETCFG_MGR_RifConfig_T rif_config;
   NETCFG_PMGR_ipAddrEntry_T  ip_addr_entry;
   OSPF_TYPE_OspfIfEntry_T if_entry_record;
   OSPF_TYPE_OspfIfMetricEntry_T if_metric_entry;

   memset(&rif_config, 0, sizeof(NETCFG_MGR_RifConfig_T));
   memset(&ip_addr_entry, 0, sizeof(NETCFG_PMGR_ipAddrEntry_T));
   memset(&if_entry_record, 0, sizeof(OSPF_TYPE_OspfIfEntry_T));
   memset(&if_metric_entry, 0, sizeof(OSPF_TYPE_OspfIfMetricEntry_T));

   VLAN_PMGR_ConvertToIfindex(vid, &ifindex);

   NETCFG_PMGR_GetOspfRouterId(&router_id);
   L_INET_Ntoa(router_id,router_ip);

   while (NETCFG_PMGR_GetNextIpAddressMode(&vid_ifindex, &access_mode)==NETCFG_MGR_OK)
   {
      if(vid!=0)
      {
         if(vid_ifindex != ifindex)
         {
            continue;
         }
      }

      if(first_flag != 1)
      {
      PROCESS_MORE("\r\n");
      }

      VLAN_PMGR_ConvertFromIfindex(vid_ifindex, &rif_vid);

      {
         UI32_T get_ret=0x0fffffff;
         char   *op_status[3]={"up","down","not existing"};
         int    op_index = 1;
         rif_num = 0;

         if (NETCFG_PMGR_GetNextRifFromInterface(vid_ifindex, &rif_num)==TRUE)
         {
            if ((get_ret=NETCFG_PMGR_GetRifConfig(rif_num, &rif_config))==NETCFG_MGR_OK)
            {
               if (rif_config.if_active_status == NETCFG_PMGR_ACTIVE_STATUS_INTERFACE_UP)
                op_index = 0;
               else if (rif_config.if_active_status == NETCFG_PMGR_ACTIVE_STATUS_INTERFACE_DOWN)
                op_index = 1;
               else
                op_index = 2;
            }
         }
         else
         {
            get_ret = 0x0fffffff;
         }

         vlan_first_entry_flag = 1;

         do
         {
            if (get_ret == NETCFG_MGR_OK)
            {
               L_INET_Ntoa(rif_config.ip_address,ip_str);
               L_INET_Ntoa(rif_config.ip_mask,ip_mask);

               if_entry_record.ospf_if_ip_address=rif_config.ip_address;
               if((NETCFG_PMGR_GetOspfIfEntry(&if_entry_record)==NETCFG_MGR_OK) &&
                  ((if_entry_record.ospf_if_status == VAL_ospfIfStatus_active) ||
                   (if_entry_record.ospf_if_status == VAL_ospfIfStatus_createAndGo)))
               {
                  if(vlan_first_entry_flag)
                  {
                     sprintf(buff,"VLAN %lu is %s\r\n",rif_vid, op_status[op_index]);
                     PROCESS_MORE(buff);
                  }

                  L_INET_Ntoa(if_entry_record.ospf_if_area_id,area_ip);
                  sprintf(buff, "  Interface Address %s, Mask %s, Area %s\r\n",ip_str,ip_mask,area_ip);
                  PROCESS_MORE(buff);

                  if_metric_entry.ospf_if_metric_ip_address=rif_config.ip_address;
                  NETCFG_PMGR_GetOspfIfMetricEntry(&if_metric_entry);

                  sprintf(buff, "  Router ID %s, Network Type BROADCAST, Cost: %lu\r\n",router_ip,if_metric_entry.ospf_if_metric_value);
                  PROCESS_MORE(buff);

                  {
                     UI32_T ospf_router_role=0;
                     memset(state_role, 0, sizeof(state_role));

                     NETCFG_PMGR_GetOspfRoleByIpInterface(rif_config.ip_address,&ospf_router_role);
                     switch(ospf_router_role)
                     {
                        case OSPF_TYPE_DR:
                        strcpy(state_role,"DR");
                        break;

                        case OSPF_TYPE_BDR:
                        strcpy(state_role,"BDR");
                        break;

                        case OSPF_TYPE_DROTHER:
                        strcpy(state_role,"DROTHER");
                        break;

                        default:
                        break;
                     }
                  }

                  sprintf(buff, "  Transmit Delay is %lu sec, State %s, Priority %lu\r\n",if_entry_record.ospf_if_transit_delay,state_role,if_entry_record.ospf_if_rtr_priority);
                  PROCESS_MORE(buff);

                  L_INET_Ntoa(if_entry_record.ospf_if_designated_router,dr_ip);
                  L_INET_Ntoa(if_entry_record.ospf_if_backup_designated_router,bdr_ip);

                  {
                     UI32_T dr_ip_interface=0,bdr_ip_interface=0;
                     NETCFG_PMGR_GetRouterIDByInterface(rif_config.ip_address,&dr_ip_interface,&bdr_ip_interface);
                     L_INET_Ntoa(dr_ip_interface,dr_interface_ip);
                     L_INET_Ntoa(bdr_ip_interface,bdr_interface_ip);
                  }

                  sprintf(buff, "  Designated Router id %s, Interface address %s\r\n",dr_interface_ip,dr_ip);
                  PROCESS_MORE(buff);

                  sprintf(buff, "  Backup Designated router id %s, Interface addr %s\r\n",bdr_interface_ip,bdr_ip);
                  PROCESS_MORE(buff);

                  sprintf(buff, "  Timer intervals configured, Hello %lu, Dead %lu, Retransmit %lu\r\n",if_entry_record.ospf_if_hello_interval,if_entry_record.ospf_if_rtr_dead_interval,if_entry_record.ospf_if_retrans_interval);
                  PROCESS_MORE(buff);

                  PROCESS_MORE("\r\n");
                  vlan_first_entry_flag = 0;
                  first_flag = 0;
               }
            }
            if (NETCFG_PMGR_GetNextRifFromInterface(vid_ifindex, &rif_num)==TRUE)
            {
               get_ret=NETCFG_PMGR_GetRifConfig(rif_num, &rif_config);
            }
            else
            {
               get_ret = 0x0fffffff;
            }
         }while (get_ret == NETCFG_MGR_OK);
      }
   }
   return TRUE;
}

#endif

#endif //#if 0


/*---------------------dhcp server-------------------------*/
/* command: clear ip dhcp binding */
UI32_T CLI_API_L3_Clear_Ip_Dhcp_Binding(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_addr = 0;
   if(arg[0] == NULL)
   {
      if(DHCP_PMGR_ClearAllIpBinding()!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to free all assigned IPs to DHCP server pool.\r\n");
      }
   }
   else
   {
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);
      if(DHCP_PMGR_ClearIpBinding(ip_addr)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to free assigned IPs to DHCP server pool.\r\n");
      }
   }
#endif
   return CLI_NO_ERROR;
}

/* command: ip dhcp excluded-address */
UI32_T CLI_API_L3_Ip_Dhcp_Excluded_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T low_addr=0,high_addr=0;

   CLI_LIB_AtoIp(arg[0], (UI8_T*)&low_addr);
   if(arg[1]!=0)
   {
      CLI_LIB_AtoIp(arg[1], (UI8_T*)&high_addr);
   }

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_DHCP_EXCLUDEDADDRESS:

      if(DHCP_PMGR_SetExcludedIp(low_addr,high_addr)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set excluded address.\r\n");
      }
      else
      {
        /*ret = DHCP_PMGR_SetRestartObject(DHCP_PMGR_RESTART_SERVER);
      if (ret == DHCP_PMGR_FAIL)
         printf("\n Failed to set excluded address. Failed to setting restart object.");
          DHCP_PMGR_Restart3(DHCP_PMGR_RESTART_SERVER);   */
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_DHCP_EXCLUDEDADDRESS:

      if(DHCP_PMGR_DelExcludedIp(low_addr,high_addr)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete excluded address.\r\n");
      }
      else
      {
         /*ret = DHCP_PMGR_SetRestartObject(DHCP_PMGR_RESTART_SERVER);
      if (ret == DHCP_PMGR_FAIL)
         printf("\n Failed to delete excluded address. Failed to setting restart object.");
          DHCP_PMGR_Restart3(DHCP_PMGR_RESTART_SERVER);*/
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: ip dhcp pool */
UI32_T CLI_API_L3_Ip_Dhcp_Pool(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   char Name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1] = {0};

   strcpy(Name, arg[0]);

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_DHCP_POOL:

      if(DHCP_PMGR_EnterPool(Name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set IP dhcp pool name.\r\n");
      }
      else
      {
         strcpy(ctrl_P->CMenu.Pool_name,Name);
         ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_DHCPPOOL_MODE;
      }
      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_DHCP_POOL:

      if(DHCP_PMGR_DeletePool(Name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete IP dhcp pool name.\r\n");
      }
      else
      {
         memset(ctrl_P->CMenu.Pool_name, 0, sizeof(ctrl_P->CMenu.Pool_name));
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#else
   //strcpy(ctrl_P->CMenu.Pool_name,Name);
   //ctrl_P->CMenu.AccMode = PRIVILEGE_CFG_DHCPPOOL_MODE;
   //CLI_LIB_PrintStr_2("pool name: %s, menu pool name: %s\r\n",Name,ctrl_P->CMenu.Pool_name);
#endif
   return CLI_NO_ERROR;
}

/* command: host ip_address [mask] */
UI32_T CLI_API_L3_Dhcp_Pool_Host(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_addr=0,netmask=0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_HOST:
    {
        if(!CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr))
        {
            CLI_LIB_PrintStr("Invalid ip address format.\r\n");
            return CLI_NO_ERROR;
        }

        if(arg[1]!=0)
        {
            if(!CLI_LIB_AtoIp(arg[1], (UI8_T*)&netmask))
            {
                CLI_LIB_PrintStr("Invalid network mask format.\r\n");
                return CLI_NO_ERROR;
            }
        }

        if(DHCP_PMGR_SetHostToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_addr,netmask)!=DHCP_PMGR_OK)
        {
         CLI_LIB_PrintStr("Failed to set host to pool configuration entry.\r\n");
        }
    }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_HOST:

      if(DHCP_PMGR_DelHostFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete host from pool configuration entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: hardware-address [type] */
UI32_T CLI_API_L3_Dhcp_Pool_Hardware_Address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI8_T  phy_ipAddress[SYS_ADPT_MAC_ADDR_LEN]    = {0};
   UI32_T type =DHCP_MGR_HTYPE_ETHER;
   UI32_T return_type;

   switch(cmd_idx)
   {
    case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_HARDWAREADDRESS:
        CLI_LIB_ValsInMac(arg[0], phy_ipAddress);

        if(arg[1][0]=='e' || arg[1][0]=='E')
        {
          type = DHCP_MGR_HTYPE_ETHER;
        }
        else if(arg[1][0]=='i' || arg[1][0]=='I')
        {
          type = DHCP_MGR_HTYPE_IEEE802;
        }

        return_type = DHCP_PMGR_SetMacToPoolConfigEntry(ctrl_P->CMenu.Pool_name,phy_ipAddress);
        if (return_type != DHCP_PMGR_OK)
        {
            switch(return_type)
            {
                case DHCP_MGR_INVALID_MAC_ADDRESS:
                   CLI_LIB_PrintStr("Set invaild hardware-address to pool config entry.\r\n");
                   return CLI_NO_ERROR;
                default:
                   CLI_LIB_PrintStr("Failed to set hardware-address to pool config entry.\r\n");
                   return CLI_NO_ERROR;
            }
        }

        if(DHCP_PMGR_SetMacTypeToPoolConfigEntry(ctrl_P->CMenu.Pool_name, type)!=DHCP_PMGR_OK)
        {
            CLI_LIB_PrintStr("Failed to set hardware-address type to pool config entry.\r\n");
        }
        break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_HARDWAREADDRESS:

      if(DHCP_PMGR_DelMacFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete hardware-address from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: network network-number [mask] */
UI32_T CLI_API_L3_Dhcp_Pool_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_addr=0,netmask=0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_NETWORK:


      if(!CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr))
      {
          CLI_LIB_PrintStr("Invalid ip address format.\r\n");
          return CLI_NO_ERROR;
      }

      if(arg[1]!=0)
      {
          if(!CLI_LIB_AtoIp(arg[1], (UI8_T*)&netmask))
          {
              CLI_LIB_PrintStr("Invalid network mask format.\r\n");
              return CLI_NO_ERROR;
          }
      }

      if(DHCP_PMGR_SetNetworkToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_addr,netmask)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set network to pool config entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_NETWORK:

      if(DHCP_PMGR_DelNetworkFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete network from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: domain-name domain */
UI32_T CLI_API_L3_Dhcp_Pool_Domain_Name(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   char Name[SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN+1] = {0};

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_DOMAINNAME:

      strcpy(Name, arg[0]);

      if(DHCP_PMGR_SetDomainNameToPoolConfigEntry(ctrl_P->CMenu.Pool_name,Name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set domain-name to pool config entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_DOMAINNAME:

      if(DHCP_PMGR_DelDomainNameFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete domain-name from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: dhs-server address [address2] */
UI32_T CLI_API_L3_Dhcp_Pool_Dns_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_array[] = {0,0};
   UI32_T  size=1;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_DNSSERVER:

      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_array[0]);

      if(arg[1]!=0)
      {
         size =2;
         CLI_LIB_AtoIp(arg[1], (UI8_T*)&ip_array[1]);
      }
      if(DHCP_PMGR_SetDnsServerToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_array,size)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set dns-server to pool config entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_DNSSERVER:

      if(DHCP_PMGR_DelDnsServerFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete dns-server from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: lease {infinite | days [hours][minutes]} */
UI32_T CLI_API_L3_Dhcp_Pool_Lease(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T day=0,hour=0,minute=0, lease_time=0;
   UI32_T infinite=0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_LEASE:

    if(arg[0][0]=='i' || arg[0][0]=='I')
    {
      infinite=1;
    }
    else
    {
        day = CLI_LIB_AtoUl(arg[0],10);
        hour = CLI_LIB_AtoUl(arg[1],10);
        minute = CLI_LIB_AtoUl(arg[2],10);

        if((day > 365)||(hour > 23)||(minute > 59))
        {
            CLI_LIB_PrintStr("Invalid lease time.\r\n");
            return CLI_NO_ERROR;
        }

        lease_time = day*DHCP_TYPE_ONE_DAY_SEC +
                     hour*DHCP_TYPE_ONE_HOUR_SEC +
                     minute*DHCP_TYPE_ONE_MIN_SEC;

      }

      if(DHCP_PMGR_SetLeaseTimeToPoolConfigEntry(ctrl_P->CMenu.Pool_name,lease_time,infinite)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set the lease time to pool configuration entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_LEASE:

      if(DHCP_PMGR_DelLeaseTimeFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete lease time from pool configuration entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: default-router address [address2] */
UI32_T CLI_API_L3_Dhcp_Pool_Default_Router(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_array[] = {0,0};
   UI32_T size=1;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_DEFAULTROUTER:
   {
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_array[0]);

      if(arg[1]!=0)
      {
        size =2;
        CLI_LIB_AtoIp(arg[1], (UI8_T*)&ip_array[1]);
      }

      if(DHCP_PMGR_SetDfltRouterToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_array,size)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set default router to pool config entry.\r\n");
      }
    }
    break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_DEFAULTROUTER:

      if(DHCP_PMGR_DelDfltRouterFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete default router from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: bootfile filename */
UI32_T CLI_API_L3_Dhcp_Pool_Boot_File(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   char  Name[SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN+1] = {0};

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_BOOTFILE:

      strcpy(Name, arg[0]);

      if(DHCP_PMGR_SetBootfileToPoolConfigEntry(ctrl_P->CMenu.Pool_name,Name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set boot file to pool config entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_BOOTFILE:

      if(DHCP_PMGR_DelBootfileFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete boot file from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: netbios-name-server address [address2] */
UI32_T CLI_API_L3_Dhcp_Pool_Netbios_Name_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T ip_array[] = {0,0};
   UI32_T size=1;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_NETBIOSNAMESERVER:

    {
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_array[0]);

      if(arg[1]!=0)
      {
         size =2;
         CLI_LIB_AtoIp(arg[1], (UI8_T*)&ip_array[1]);
      }
      if(DHCP_PMGR_SetNetbiosNameServerToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_array,size)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set netbios name server to pool config entry.\r\n");
      }
    }
    break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_NETBIOSNAMESERVER:

      if(DHCP_PMGR_DelNetbiosNameServerFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete netbios name server from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: netbios-node-type type */
UI32_T CLI_API_L3_Dhcp_Pool_Netbios_Node_Type(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T type=0;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_NETBIOSNODETYPE:

    if(arg[0][0]=='b' ||arg[0][0] =='B')
    {
      type=DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE;
    }
    else if(arg[0][0]=='h' ||arg[0][0] =='H')
    {
      type=DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE;
    }
    else if(arg[0][0]=='m' ||arg[0][0] =='M')
    {
      type=DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE;
    }
    else if(arg[0][0]=='p' ||arg[0][0] =='P')
    {
      type=DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE;
    }
    if(DHCP_PMGR_SetNetbiosNodeTypeToPoolConfigEntry(ctrl_P->CMenu.Pool_name,type)!=DHCP_PMGR_OK)
    {
     CLI_LIB_PrintStr("Failed to set netbios node type to pool config entry.\r\n");
    }
    break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_NETBIOSNODETYPE:

      if(DHCP_PMGR_DelNetbiosNodeTypeFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete netbios node type from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: next-server address */
UI32_T CLI_API_L3_Dhcp_Pool_Next_Server(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T  ip_addr;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_NEXTSERVER:

      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);

      if(DHCP_PMGR_SetNextServerToPoolConfigEntry(ctrl_P->CMenu.Pool_name,ip_addr)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set next server to pool config entry.\r\n");
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_NEXTSERVER:

      if(DHCP_PMGR_DelNextServerFromPoolConfigEntry(ctrl_P->CMenu.Pool_name)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to delete next server from pool config entry.\r\n");
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: service dhcp */
UI32_T CLI_API_L3_Service_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
    UI32_T ret;

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_GLOBAL_CMD_W2_SERVICE_DHCP:

      ret = DHCP_PMGR_CheckRestartObj(DHCP_MGR_RESTART_SERVER);

      switch(ret)
      {
        case DHCP_MGR_OK:
            break;
        case DHCP_MGR_DYNAMIC_IP:
            CLI_LIB_PrintStr("The interface has not yet configured IP.\r\n");
            return CLI_NO_ERROR;
        case DHCP_MGR_RELAY_ON:
            CLI_LIB_PrintStr("DHCP Relay is running.\r\n");
            return CLI_NO_ERROR;
        default:
            CLI_LIB_PrintStr("Failed to enable DHCP server.\r\n");
            return CLI_NO_ERROR;
      }

      DHCP_PMGR_Restart3(DHCP_MGR_RESTART_SERVER);

      break;

   case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_SERVICE_DHCP:

      if(DHCP_PMGR_RemoveSystemRole(DHCP_MGR_BIND_SERVER)!=DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to disable DHCP server.\r\n");
      }
      else
      {
         DHCP_PMGR_Restart3(DHCP_MGR_RESTART_CLIENT);
      }
      break;

   default:
      return CLI_NO_ERROR;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: client-identifier {hex number | text string} */
UI32_T CLI_API_L3_Dhcp_ClientIdentifier(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T id_mode;
   UI32_T length;
   char  str[MAXSIZE_dhcpcIfClientId+1 ] = {0};

   switch(cmd_idx)
   {
   case PRIVILEGE_CFG_DHCPPOOL_CMD_W1_CLIENTIDENTIFIER:
      switch(arg[0][0])
      {
      case 'h':
      case 'H':
         id_mode = DHCP_MGR_CID_HEX;
         {
            UI32_T i;
            UI32_T j = 0;
            UI8_T  temp_str[MAXSIZE_dhcpcIfClientId*2+1] = {0};
            char  buff[3] = {0};
            if (strlen(arg[1])%2 != 0)
            {
               length = strlen(arg[1])/2 + 1;
               temp_str[0] = '0';
               memcpy(&(temp_str[1]), arg[1], sizeof(UI8_T)*strlen(arg[1]));
            }
            else
            {
               length = strlen(arg[1])/2;
               memcpy(temp_str, arg[1], sizeof(UI8_T)*strlen(arg[1]));
            }
            for (i = 0; i < MAXSIZE_dhcpcIfClientId - 1; i++)
            {
               buff[0] = temp_str[j];
               buff[1] = temp_str[j+1];
               buff[2] = 0;
               str[i] = (UI8_T)CLI_LIB_AtoUl(buff,16);
               j += 2;
            }
         }
         break;

      case 't':
      case 'T':
         id_mode = DHCP_MGR_CID_TEXT;
         length = strlen(arg[1]);
         memcpy(str, arg[1], sizeof(UI8_T)*strlen(arg[1]));
         break;

      default:
         return CLI_ERR_INTERNAL;
      }

      if (DHCP_PMGR_SetCidToPoolConfigEntry(ctrl_P->CMenu.Pool_name, id_mode, length, str) != DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to set dhcp client identifier\r\n");
         return CLI_NO_ERROR;
      }
      break;

   case PRIVILEGE_CFG_DHCPPOOL_CMD_W2_NO_CLIENTIDENTIFIER:

      if (DHCP_PMGR_DelCidToPoolConfigEntry(ctrl_P->CMenu.Pool_name) != DHCP_PMGR_OK)
      {
         CLI_LIB_PrintStr("Failed to remove dhcp client identifier\r\n");
         return CLI_NO_ERROR;
      }
      break;

   default:
      return CLI_ERR_INTERNAL;
   }
#endif
   return CLI_NO_ERROR;
}

/* command: show ip dhcp */
UI32_T CLI_API_L3_Show_Ip_Dhcp(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_DHCP_SERVER == TRUE)
   UI32_T count = 0, line_num = 0, i = 0, j = 0;
   UI8_T  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI8_T  ip_str[18] = {0}, mask_str[18] = {0}, pool_type[8] = {0};
   UI8_T  low_addr_str[86][18], high_addr_str[86][18];
   UI8_T  strBuf[18] = {0}, last_pool_name[SYS_ADPT_DHCP_MAX_POOL_NAME_LEN+1] = {0};
   DHCP_TYPE_PoolConfigEntry_T pool_config;
   DHCP_MGR_ActivePoolRange_T active_range;

   memset(&pool_config, 0, sizeof(DHCP_TYPE_PoolConfigEntry_T));
   memset(&active_range, 0, sizeof(DHCP_MGR_ActivePoolRange_T));

   CLI_LIB_PrintStr("\r\n  Name   Type   IP Address         Mask                Active Pool\r\n");
   CLI_LIB_PrintStr("-------- ---- --------------- --------------- ---------------------------------\r\n");

   if(arg[0]==0)
   {
      while(DHCP_PMGR_GetNextPoolConfig(&pool_config) == DHCP_MGR_OK)
      {
        memset(&active_range, 0, sizeof(DHCP_MGR_ActivePoolRange_T));
        strcpy((char *)pool_type, "");
        strcpy((char *)ip_str, "");
        strcpy((char *)low_addr_str, "");
        strcpy((char *)high_addr_str, "");
        strcpy((char *)last_pool_name, pool_config.pool_name);

        L_INET_Ntoa(pool_config.sub_netmask, mask_str);

        switch (pool_config.pool_type)
        {
            case DHCP_MGR_POOL_NETWORK:
                strcpy((char *)pool_type, "Net");
                L_INET_Ntoa(pool_config.network_address, ip_str); //Get IP address
                break;
            case DHCP_MGR_POOL_HOST:
                strcpy((char *)pool_type, "Host");
                L_INET_Ntoa(pool_config.host_address, ip_str); //Get IP address
                break;
            default :
                strcpy((char *)ip_str,"");
                strcpy((char *)mask_str, "");
                break;
        }
        i = 0;
        /* get active range */
        if(pool_config.pool_type == DHCP_MGR_POOL_NETWORK)
        {
            while (DHCP_PMGR_GetNextActivePool((char *)last_pool_name, &active_range) == DHCP_MGR_OK)
            {
                /* Search key is pool_name + pool_range.low_address in dhcp_mgr. */
                if (strcmp((char *)last_pool_name, pool_config.pool_name) == 0)
                {
                    L_INET_Ntoa(active_range.low_address, low_addr_str[i]);
                    L_INET_Ntoa(active_range.high_address, high_addr_str[i]);
                    i++;
                }
                else
                {

                    break;
                }
            }

        }


        if (low_addr_str[0][0] == '\0')
        {

            sprintf((char *)buff, "%-8s %-4s %-15s %-15s\r\n", pool_config.pool_name, pool_type, ip_str, mask_str);
            PROCESS_MORE((char *)buff);
        }
        else
        {
            if (i > 1)
            {

                sprintf ((char *)buff, "%-8s %-4s %-15s %-15s %-15s - %-15s\r\n", pool_config.pool_name, pool_type, ip_str, mask_str, low_addr_str[0], high_addr_str[0]);
                PROCESS_MORE((char *)buff);
                j = 1;
                while (j < i)
                {
                    strcat ((char *)buff, "                                              ");
                    sprintf ((char *)strBuf, "%-15s", low_addr_str[j]);
                    strcat  ((char *)buff, (char *)strBuf);
                    strcat  ((char *)buff, " - ");
                    sprintf ((char *)strBuf, "%-15s", high_addr_str[j]);
                    strcat  ((char *)buff, (char *)strBuf);
                    strcat((char *)buff, "\r\n");
                    PROCESS_MORE((char *)buff);
                    j++;
                }

            }
            else
            {

                sprintf ((char *)buff, "%-8s %-4s %-15s %-15s %-15s - %-15s\r\n", pool_config.pool_name, pool_type, ip_str, mask_str, low_addr_str[0], high_addr_str[0]);
                PROCESS_MORE((char*)buff);

            }
        }

        count++;
      }
   }

   sprintf((char *)buff, "\r\nTotal entry : %lu\r\n",count);
   PROCESS_MORE((char *)buff);

#endif
   return CLI_NO_ERROR;
}


/* command: show ip dhcp binding */
UI32_T CLI_API_L3_Show_Ip_Dhcp_Binding(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_DHCP_SERVER == TRUE)

   char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
   UI32_T line_num = 0;

   UI32_T ip_addr = 0;
   UI8_T ip_str[18] = {0};
   UI8_T UTC[22]={0};
   char  UTC_l[22]={0};
   UI32_T count = 0;
   DHCP_MGR_Server_Lease_Config_T ip_binding;
   UI32_T cur_time = 0;
   memset(&ip_binding, 0, sizeof(DHCP_MGR_Server_Lease_Config_T));

   CLI_LIB_PrintStr("\r\n     IP                MAC           Lease Time        Start\r\n");
   CLI_LIB_PrintStr("                                    (dd/hh/mm/ss)             \r\n");
   CLI_LIB_PrintStr("--------------- ----------------- ------------------ ----------------------\r\n");

   /* get system real time */
   SYS_TIME_GetRealTimeBySec(&cur_time);

   if(arg[0]==0)
   {

      while(DHCP_MGR_OK == DHCP_PMGR_GetNextActiveIpBinding(ip_binding.lease_ip,&ip_binding))
      {
         L_INET_Ntoa(ip_binding.lease_ip,ip_str);

     SYS_TIME_ConvertTime(ip_binding.start_time, (char *)UTC);

     if(ip_binding.lease_time==DHCP_MGR_INFINITE_LEASE_TIME)
     {
        sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-18s %11s\r\n",ip_str,ip_binding.hardware_address[0],ip_binding.hardware_address[1],
                                          ip_binding.hardware_address[2],ip_binding.hardware_address[3],ip_binding.hardware_address[4],ip_binding.hardware_address[5],"Infinite",UTC);
     }
     else
     {

        sprintf(UTC_l,"%lu/%lu/%lu/%lu",ip_binding.lease_time/86400,(ip_binding.lease_time% 86400)/3600,((ip_binding.lease_time % 86400) % 3600)/60,((ip_binding.lease_time % 86400) % 3600)%60);
        sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-18s %11s\r\n",ip_str,ip_binding.hardware_address[0],ip_binding.hardware_address[1],
                                          ip_binding.hardware_address[2],ip_binding.hardware_address[3],ip_binding.hardware_address[4],ip_binding.hardware_address[5],UTC_l,UTC);

     }
     PROCESS_MORE(buff);
     count++;
      }
   }
   else
   {
      CLI_LIB_AtoIp(arg[0], (UI8_T*)&ip_addr);

      if(DHCP_PMGR_OK == DHCP_PMGR_GetActiveIpBinding(ip_addr,&ip_binding))
      {
         L_INET_Ntoa(ip_binding.lease_ip,ip_str);

     SYS_TIME_ConvertTime(ip_binding.start_time, (char *)UTC);

     if(ip_binding.lease_time==DHCP_MGR_INFINITE_LEASE_TIME)
     {
        sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-18s %11s\r\n",ip_str,ip_binding.hardware_address[0],ip_binding.hardware_address[1],
                                          ip_binding.hardware_address[2],ip_binding.hardware_address[3],ip_binding.hardware_address[4],ip_binding.hardware_address[5],"Infinite",UTC);
     }
     else
     {

        sprintf(UTC_l,"%lu/%lu/%lu/%lu",ip_binding.lease_time/86400,(ip_binding.lease_time% 86400)/3600,((ip_binding.lease_time % 86400) % 3600)/60,((ip_binding.lease_time % 86400) % 3600)%60);
        sprintf(buff, "%-15s %02X-%02X-%02X-%02X-%02X-%02X %-18s %11s\r\n",ip_str,ip_binding.hardware_address[0],ip_binding.hardware_address[1],
                                          ip_binding.hardware_address[2],ip_binding.hardware_address[3],ip_binding.hardware_address[4],ip_binding.hardware_address[5],UTC_l,UTC);
     }
     PROCESS_MORE(buff);
     count++;
      }
   }

   sprintf(buff, "\r\nTotal entries : %lu\r\n",count);
   PROCESS_MORE(buff);

#endif

   return CLI_NO_ERROR;
}

/* command: show ip dhcp pool */
UI32_T CLI_API_L3_Show_Ip_Dhcp_Pool(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{

#if (SYS_CPNT_DHCP_SERVER == TRUE)
    DHCP_TYPE_PoolConfigEntry_T pool_config;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    char addr_str[L_INET_MAX_IP4ADDR_STR_LEN+1]={0};
    UI8_T display_flag = 0;

#define DHCP_POOL_DISPLAY_NETWORK   1
#define DHCP_POOL_DISPLAY_HOST      1<<1
#define DHCP_POOL_DISPLAY_ALL       DHCP_POOL_DISPLAY_NETWORK | DHCP_POOL_DISPLAY_HOST

    if(arg[0]== NULL)
    {
        display_flag = DHCP_POOL_DISPLAY_ALL;
    }
    else
    {
        if((arg[0][0]== 'n')||(arg[0][0]== 'N'))
        {
            display_flag = DHCP_POOL_DISPLAY_NETWORK;
        }
        else
        {
            display_flag = DHCP_POOL_DISPLAY_HOST;
        }
    }

    memset(&pool_config, 0, sizeof(pool_config));
    while(DHCP_MGR_OK == DHCP_PMGR_GetNextPoolConfig(&pool_config))
    {
        if((pool_config.pool_type == DHCP_MGR_POOL_NETWORK)&&
           (!(display_flag & DHCP_POOL_DISPLAY_NETWORK)))
        {
            continue;
        }

        if((pool_config.pool_type == DHCP_MGR_POOL_HOST)&&
           (!(display_flag & DHCP_POOL_DISPLAY_HOST)))
        {
            continue;
        }

        /* pool name */
        sprintf(buff+strlen(buff),"Pool name : %s\r\n",pool_config.pool_name);
        PROCESS_MORE(buff);
        sprintf(buff+strlen(buff),"Pool type : ");
        switch(pool_config.pool_type)
        {
            case DHCP_MGR_POOL_NETWORK:
                sprintf(buff+strlen(buff),"Network\r\n");
                PROCESS_MORE(buff);
                L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.network_address), addr_str, sizeof(addr_str));
                sprintf(buff+strlen(buff),"    Network address        : %s\r\n",addr_str);
                break;
            case DHCP_MGR_POOL_HOST:
                sprintf(buff+strlen(buff),"Host\r\n");
                PROCESS_MORE(buff);
                L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.host_address), addr_str, sizeof(addr_str));
                sprintf(buff+strlen(buff),"    Host address           : %s\r\n",addr_str);
                break;
            default:
                sprintf(buff+strlen(buff),"None\r\n");
                PROCESS_MORE(buff);
                sprintf(buff+strlen(buff),"\r\n");
                break;
        }
        PROCESS_MORE(buff);
        PROCESS_MORE("\r\n");
        PROCESS_MORE("\r\n");
        /* subnet mask */
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.sub_netmask), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"    Subnet mask            : %s\r\n",addr_str);
        PROCESS_MORE(buff);
        PROCESS_MORE("\r\n");
        sprintf(buff+strlen(buff),"    Boot file              : %s\r\n",pool_config.options.bootfile);
        PROCESS_MORE(buff);

        sprintf(buff+strlen(buff),"    Client identifier mode : ");
        switch(pool_config.options.cid.id_mode)
        {
            case DHCP_MGR_CID_HEX:
            {
                UI8_T index=0;
                sprintf(buff+strlen(buff),"Hex\r\n");
                PROCESS_MORE(buff);
                sprintf(buff+strlen(buff),"    Client identifier      : ");
                for(index=0;index< pool_config.options.cid.id_len;index++)
                    sprintf(buff+strlen(buff),"%02X",pool_config.options.cid.id_buf[index]);
                sprintf(buff+strlen(buff),"\r\n");
            }
                break;
            case DHCP_MGR_CID_TEXT:
                sprintf(buff+strlen(buff),"Text\r\n");
                PROCESS_MORE(buff);
                sprintf(buff+strlen(buff),"    Client identifier      : %s\r\n",pool_config.options.cid.id_buf);
                break;
            default:
                sprintf(buff+strlen(buff),"None\r\n");
                break;
        }
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.default_router[0]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"    Default router         : %s\r\n",addr_str);
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.default_router[1]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"                             %s\r\n",addr_str);
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.dns_server[0]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"    DNS server             : %s\r\n",addr_str);
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.dns_server[1]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"                             %s\r\n",addr_str);
        PROCESS_MORE(buff);
        sprintf(buff+strlen(buff),"    Domain name            : %s\r\n",pool_config.options.domain_name);
        PROCESS_MORE(buff);
        sprintf(buff+strlen(buff),"    Hardware type          : ");
        switch(pool_config.hardware_address.htype)
        {
            case DHCP_MGR_HTYPE_ETHER:
                sprintf(buff+strlen(buff),"Ethernet\r\n");
                break;
            case DHCP_MGR_HTYPE_IEEE802:
                sprintf(buff+strlen(buff),"IEEE802\r\n");
                break;
            default:
                sprintf(buff+strlen(buff),"None\r\n");
                break;
        }
        PROCESS_MORE(buff);
        sprintf(buff+strlen(buff),"    Hardware address       : %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            pool_config.hardware_address.haddr[0],
            pool_config.hardware_address.haddr[1],
            pool_config.hardware_address.haddr[2],
            pool_config.hardware_address.haddr[3],
            pool_config.hardware_address.haddr[4],
            pool_config.hardware_address.haddr[5]);
        PROCESS_MORE(buff);

        /* lease time */
        sprintf(buff+strlen(buff),"    Lease time             : ");
        if(DHCP_MGR_INFINITE_LEASE_TIME == pool_config.options.lease_time)
        {
            sprintf(buff+strlen(buff),"infinite\r\n");
        }
        else
        {
            UI32_T day=0,min=0,hour=0;
            day = pool_config.options.lease_time/86400;
            hour = (pool_config.options.lease_time % 86400)/3600;
            min = ((pool_config.options.lease_time % 86400) % 3600)/60;
            sprintf(buff+strlen(buff),"%lu d/ %lu h/ %lu m\r\n",day,hour,min);
        }
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.netbios_name_server[0]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"    Netbios name server    : %s\r\n",addr_str);
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.netbios_name_server[1]), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"                             %s\r\n",addr_str);
        PROCESS_MORE(buff);
        sprintf(buff+strlen(buff),"    Netbios node type      : ");

        switch(pool_config.options.netbios_node_type)
        {
            case DHCP_MGR_NETBIOS_NODE_TYPE_B_NODE:
                sprintf(buff+strlen(buff),"Broadcast\r\n");
                break;
            case DHCP_MGR_NETBIOS_NODE_TYPE_H_NODE:
                sprintf(buff+strlen(buff),"Hybrid\r\n");
                break;
            case DHCP_MGR_NETBIOS_NODE_TYPE_M_NODE:
                sprintf(buff+strlen(buff),"Mixed\r\n");
                break;
            case DHCP_MGR_NETBIOS_NODE_TYPE_P_NODE:
                sprintf(buff+strlen(buff),"Peer-to-peer\r\n");
                break;
            default:
                sprintf(buff+strlen(buff),"None\r\n");
                break;
        }
        PROCESS_MORE(buff);
        L_INET_Ntop(L_INET_AF_INET, (UI8_T *)&(pool_config.options.next_server), addr_str, sizeof(addr_str));
        sprintf(buff+strlen(buff),"    Next server            : %s\r\n",addr_str);
        PROCESS_MORE(buff);

        PROCESS_MORE("\r\n");

    }
#endif
   return CLI_NO_ERROR;
}


/* command: [no] management vlan */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_MANAGEMENT_Vlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "(config)management vlan"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_MANAGEMENT_Vlan (UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    UI32_T vid, vid_ifidx;

    switch(cmd_idx)
    {
        /* management vlan */
        case PRIVILEGE_CFG_GLOBAL_CMD_W2_MANAGEMENT_VLAN:
        {
            /*convert vid to vid_ifindex*/
            vid = atoi(arg[0]);
            VLAN_PMGR_ConvertToIfindex(vid, &vid_ifidx);

            switch(NETCFG_PMGR_SetManagementVid(vid_ifidx))
            {
                case NETCFG_MGR_OK:
                    CLI_LIB_PrintStr("Management VLAN is set successfully.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_IP_ALREADY_EXIST:
                    CLI_LIB_PrintStr("There exists an IP address in the VLAN. Please remove the IP address first.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_NO_SUCH_INTERFACE:
                    CLI_LIB_PrintStr("The VLAN does not exist.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_FAIL:
                    CLI_LIB_PrintStr("Failed to set management VLAN.\r\n");
                    return CLI_NO_ERROR;

                default:
                    CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
                    return CLI_ERR_INTERNAL;
            }
        }

                /* no management vlan */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_MANAGEMENT_VLAN:
        {
            switch(NETCFG_PMGR_DeleteManagementVid())
            {
                case NETCFG_MGR_OK:
                    CLI_LIB_PrintStr("Management VLAN is removed successfully.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_FAIL:
                    CLI_LIB_PrintStr("Failed to remove management VLAN.\r\n");
                    return CLI_NO_ERROR;

                default:
                    CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
                    return CLI_ERR_INTERNAL;
            }
        }

        default:
            CLI_LIB_PrintStr("Unrecognized command.\r\n");
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE) */

    return CLI_NO_ERROR;

}/*end of CLI_API_MANAGEMENT_Vlan*/


/* command: show management vlan */
/*------------------------------------------------------------------------------
 * FUNCTION NAME - CLI_API_Show_Management_Vlan
 *------------------------------------------------------------------------------
 * PURPOSE  : This is action for the command "show management vlan"
 *            in global configuration mode
 * INPUT    : cmd_idx, *arg[], *ctrl_P
 * OUTPUT   : none
 * RETURN   :
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T CLI_API_Show_Management_Vlan (UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)
    UI32_T vid_ifidx, vid;
    UI8_T default_gateway[4]={0};

    /* management vlan */
    switch(NETCFG_PMGR_GetManagementVid(&vid_ifidx))
    {
        case NETCFG_MGR_OK:
            break;

        case NETCFG_PMGR_FAIL:
            CLI_LIB_PrintStr("Current Management VLAN ID : None\r\n");
            return CLI_NO_ERROR;

        default:
            CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
            return CLI_ERR_INTERNAL;
    }

    if(VLAN_PMGR_ConvertFromIfindex(vid_ifidx, &vid)==FALSE)
    {
        vid = 0;
    }
    if (vid == 0)
    {
        CLI_LIB_PrintStr("Current Management VLAN ID : None\r\n");
    }else{
        CLI_LIB_PrintStr_1("Current Management VLAN ID : %lu\r\n", vid);
    }

    /* management vlan default-gateway */
    switch(NETCFG_PMGR_GetManagementVlanDefaultGateway((UI32_T*) default_gateway))
    {
        case NETCFG_MGR_OK:
            CLI_LIB_PrintStr_4("Management VLAN Default-gateway : %d.%d.%d.%d\r\n", default_gateway[0], default_gateway[1], default_gateway[2], default_gateway[3]);
            return CLI_NO_ERROR;

        case NETCFG_PMGR_ENTRY_NOT_EXIST:
            CLI_LIB_PrintStr("Management VLAN Default-gateway : None\r\n");
            return CLI_NO_ERROR;

        case NETCFG_PMGR_CAN_NOT_GET:
            CLI_LIB_PrintStr("Failed to retrieve management VLAN default-gateway information.\r\n");
            return CLI_NO_ERROR;

        default:
            CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
            return CLI_ERR_INTERNAL;
    }

#endif /*#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)*/
    return CLI_NO_ERROR;
}


/* command: [no] management vlan default-gateway */
UI32_T CLI_API_MANAGEMENT_Vlan_DefaultGateway(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)

    UI32_T default_gateway = 0;

    switch(cmd_idx)
    {
        /* management vlan default gateway */
        case PRIVILEGE_CFG_GLOBAL_CMD_W3_MANAGEMENT_VLAN_DEFAULTGATEWAY:
        {
            CLI_LIB_AtoIp(arg[0], (UI8_T*)&default_gateway);
            switch(NETCFG_PMGR_SetManagementVlanDefaultGateway(default_gateway))
            {
                case NETCFG_MGR_OK:
                    CLI_LIB_PrintStr("Management VLAN default-gateway is set successfully.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_DUPLICATE_GATEWAY_IP:
                    CLI_LIB_PrintStr("Management VLAN default-gateway already exists. Please remove the default-gateway first.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_INVALID_NEXT_HOP:
                    CLI_LIB_PrintStr("This management VLAN default-gateway is invalid.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_CAN_NOT_ADD:
                    CLI_LIB_PrintStr("Failed to set management VLAN default-gateway.\r\n");
                    return CLI_NO_ERROR;

                default:
                    CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
                    return CLI_ERR_INTERNAL;
            }
        }

                /* no management vlan default-gateway */
        case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_MANAGEMENT_VLAN_DEFAULTGATEWAY:
        {
            switch(NETCFG_PMGR_DeleteManagementVlanDefaultGateway())
            {
                case NETCFG_MGR_OK:
                    CLI_LIB_PrintStr("Management VLAN default-gateway is removed successfully.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_INVALID_NEXT_HOP:
                    CLI_LIB_PrintStr("The management VLAN default-gateway you entered does not exist.\r\n");
                    return CLI_NO_ERROR;

                case NETCFG_PMGR_CAN_NOT_DELETE:
                    CLI_LIB_PrintStr("Failed to remove management VLAN.\r\n");
                    return CLI_NO_ERROR;

                default:
                    CLI_LIB_PrintStr("Unrecognized value returned from netcfg.\r\n");
                    return CLI_ERR_INTERNAL;
            }
        }

        default:
            CLI_LIB_PrintStr("Unrecognized command.\r\n");
            return CLI_ERR_INTERNAL;
    }
#endif /*#if (SYS_CPNT_ISOLATED_MGMT_VLAN == TRUE)*/

    return CLI_NO_ERROR;
}

static int CLI_API_L3_Str2Addr(char *str, struct pal_in4_addr *addr)
{
    UI8_T addr_tmp[SYS_ADPT_IPV4_ADDR_LEN]= {0};

    if(0 == CLI_LIB_AtoIp(str, addr_tmp))
        return -1;
    memcpy(&(addr->s_addr), addr_tmp, sizeof(UI32_T));
    return 0;
}

UI32_T CLI_API_L3_Ospf_Network(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_RouterId(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;
    struct pal_in4_addr router_id;

    proc_id = ctrl_P->CMenu.process_id;

    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_ROUTERID:
         CLI_API_L3_Str2Addr(arg[0], &router_id);

         if(OSPF_PMGR_RouterIdSet(vr_id, proc_id, router_id.s_addr)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF Router Id setting is failed.\r\n");
         }

         break;

         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_ROUTERID:

         if(OSPF_PMGR_RouterIdUnset(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF Router Id set to default is failed.\r\n");
         }

         break;

         default:
         break;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_Timer(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;
    UI32_T delay_timer = 0, hold_timer = 0;


    proc_id = ctrl_P->CMenu.process_id;
    if(arg[0])
    {
        delay_timer = atoi(arg[0]);
        hold_timer = atoi(arg[1]);
    }
    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_TIMERS_SPF:

         if(OSPF_PMGR_TimerSet(vr_id, proc_id, delay_timer, hold_timer)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF timer setting is failed.\r\n");
         }

         break;

         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_TIMERS_SPF:

         if( OSPF_PMGR_TimerUnset(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF timer set to default is failed.\r\n");
         }

         break;

         default:
         break;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_DefaultMetric(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;
    UI32_T metric = 0;

    proc_id = ctrl_P->CMenu.process_id;

    if(arg[0])
    {
        metric = atoi(arg[0]);
    }
    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_DEFAULTMETRIC:

         if(OSPF_PMGR_DefaultMetricSet(vr_id, proc_id, metric)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF default metric setting is failed.\r\n");
         }

         break;

         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_DEFAULTMETRIC:

         if(OSPF_PMGR_DefaultMetricUnset(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF default metric set to default is failed.\r\n");
         }

         break;

         default:
         break;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_PassiveInterface(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI8_T  addr_tmp[SYS_ADPT_IPV4_ADDR_LEN]= {0};
    OSPF_TYPE_Passive_If_T passive_entry;
    UI32_T ret;
    UI32_T vid=0;

    memset(&passive_entry, 0, sizeof(OSPF_TYPE_Passive_If_T));
    passive_entry.vr_id = SYS_DFLT_VR_ID;
    passive_entry.proc_id = ctrl_P->CMenu.process_id;

    vid =CLI_LIB_AtoUl(arg[1],10);

    VLAN_VID_CONVERTTO_IFINDEX(vid, passive_entry.ifindex);

    if(arg[2])
    {
        CLI_LIB_AtoIp(arg[2], addr_tmp);
        memcpy(&passive_entry.addr, addr_tmp, sizeof(UI32_T));
    }

    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W1_PASSIVEINTERFACE:
         ret = OSPF_PMGR_PassiveIfSet(&passive_entry);
         if(ret != OSPF_TYPE_RESULT_SUCCESS)
         {
             if(ret == OSPF_TYPE_RESULT_IF_NOT_EXIST)
             {
                 CLI_LIB_PrintStr("Please specify an existing interface.\r\n");
             }
             else
             {
                 CLI_LIB_PrintStr("The OSPF passive interface setting is failed.\r\n");
             }
         }
         break;

         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_NO_PASSIVEINTERFACE:
         ret = OSPF_PMGR_PassiveIfUnset(&passive_entry);
         if(ret != OSPF_TYPE_RESULT_SUCCESS)
         {
             if(ret == OSPF_TYPE_RESULT_IF_NOT_EXIST)
             {
                 CLI_LIB_PrintStr("Please specify an existing interface.\r\n");
             }
             else
             {
                 CLI_LIB_PrintStr("The OSPF delete passive interface setting is failed.\r\n");
             }
         }
         break;

         default:
         break;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_CompatibleRfc1583(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T proc_id = SYS_DFLT_OSPF_PROC_ID;

    proc_id = ctrl_P->CMenu.process_id;

    switch(cmd_idx)
    {
         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W2_COMPATIBLE_RFC1583:

         if(OSPF_PMGR_CompatibleRfc1853Set(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF compatible rfc1583 setting is failed.\r\n");
         }

         break;

         case PRIVILEGE_CFG_ROUTEROSPF_CMD_W3_NO_COMPATIBLE_RFC1583:

         if( OSPF_PMGR_CompatibleRfc1853Unset(vr_id, proc_id)!= OSPF_TYPE_RESULT_SUCCESS)
         {
             CLI_LIB_PrintStr("The OSPF compatible rfc1583 set to default is failed.\r\n");
         }

         break;

         default:
         break;
    }
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ospf_Area(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    return CLI_NO_ERROR;
}

UI32_T show_ip_protocols_ospf(UI32_T *line_num_p)
{
#if (SYS_CPNT_OSPF == TRUE)
    UI32_T proc_id = 0xffffffff;
    UI32_T vr_id = SYS_DFLT_VR_ID;
    UI32_T vrf_id = SYS_DFLT_VRF_ID;
    OSPF_TYPE_Network_Area_T       network_entry;
    OSPF_TYPE_Multi_Proc_Summary_Addr_T summary_addr_entry;
    OSPF_TYPE_Multi_Proc_Redist_T config;
    UI8_T addr[SYS_ADPT_IPV4_ADDR_LEN];
    UI32_T line_num = 0;
    UI32_T buff_len = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    int i = 0;

    line_num = *line_num_p;
    while(OSPF_PMGR_GetNextProcessStatus(vr_id, &proc_id) == OSPF_TYPE_RESULT_SUCCESS)
    {
        sprintf(buff,"Routing Protocol is \"ospf %ld\"\r\n",(long)proc_id);
        PROCESS_MORE(buff);

        buff_len = 0;
        buff_len += sprintf(buff,"  Redistributing: ");

        /* redistribute connected */
        config.vr_id = vr_id;
        config.proc_id = proc_id;
        config.proto = OSPF_TYPE_REDISTRIBUTE_CONNECTED;
        if(OSPF_PMGR_GetMultiProcRedistEntry(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, &config) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if (CHECK_FLAG(config.flags, NETCFG_TYPE_OSPF_REDIST_ENABLE))
            {
                i++;
                buff_len += sprintf(buff + buff_len, "connected");
            }
        }

        /* redistribute static */
        config.vr_id = vr_id;
        config.proc_id = proc_id;
        config.proto = OSPF_TYPE_REDISTRIBUTE_STATIC;
        if(OSPF_PMGR_GetMultiProcRedistEntry(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, &config) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if (CHECK_FLAG(config.flags, NETCFG_TYPE_OSPF_REDIST_ENABLE))
            {
                if(i != 0)
                {
                    buff_len += sprintf(buff + buff_len, ", ");
                }
                i++;
                buff_len += sprintf(buff + buff_len, "static");
            }
        }

        /* redistribute rip */
        config.vr_id = vr_id;
        config.proc_id = proc_id;
        config.proto = OSPF_TYPE_REDISTRIBUTE_RIP;
        if(OSPF_PMGR_GetMultiProcRedistEntry(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, &config) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if (CHECK_FLAG(config.flags, NETCFG_TYPE_OSPF_REDIST_ENABLE))
            {
                if(i != 0)
                {
                    buff_len += sprintf(buff + buff_len, ", ");
                }
                i++;
                buff_len += sprintf(buff + buff_len, "rip");
            }
        }

        /* redistribute bgp */
        config.vr_id = vr_id;
        config.proc_id = proc_id;
        config.proto = OSPF_TYPE_REDISTRIBUTE_BGP;
        if(OSPF_PMGR_GetMultiProcRedistEntry(SYS_DFLT_VR_ID, SYS_DFLT_VRF_ID, &config) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if (CHECK_FLAG(config.flags, NETCFG_TYPE_OSPF_REDIST_ENABLE))
            {
                if(i != 0)
                {
                    buff_len += sprintf(buff + buff_len, ", ");
                }
                i++;
                buff_len += sprintf(buff + buff_len, "bgp");
            }
        }

        buff_len += sprintf(buff + buff_len,"\r\n");
        PROCESS_MORE(buff);

        sprintf(buff,"  Routing for Networks:\r\n");
        PROCESS_MORE(buff);
        memset(&network_entry, 0, sizeof(OSPF_TYPE_Network_Area_T));
        network_entry.vr_id = vr_id;
        network_entry.proc_id =proc_id;
        while(OSPF_PMGR_GetNextNetworkAreaTable(&network_entry) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if((network_entry.proc_id == proc_id) && (network_entry.vr_id == vr_id))
            {
                memcpy(addr, &network_entry.network_addr, sizeof(UI32_T));
                sprintf(buff,"    %d.%d.%d.%d/%ld\r\n", addr[0],addr[1], addr[2], addr[3],(long)network_entry.network_pfx);
                PROCESS_MORE(buff);
            }
            else
            {
                break;
            }
        }

        sprintf(buff,"  Routing for Summary Address:\r\n");
        PROCESS_MORE(buff);
        memset(&summary_addr_entry, 0, sizeof(OSPF_TYPE_Multi_Proc_Summary_Addr_T));
        summary_addr_entry.proc_id =proc_id;
        summary_addr_entry.config_type = OSPF_TYPE_SUMMARY_CONFIG_TYPE_CLI;
        summary_addr_entry.indexlen = 0;
        while(OSPF_PMGR_GetNextMultiProcSummaryAddrEntry(vr_id, vrf_id, &summary_addr_entry) == OSPF_TYPE_RESULT_SUCCESS)
        {
            if(summary_addr_entry.proc_id == proc_id)
            {
                memcpy(addr, &summary_addr_entry.summary_address, sizeof(UI32_T));
                sprintf(buff,"    %d.%d.%d.%d/%ld\r\n", addr[0],addr[1], addr[2], addr[3],(long)summary_addr_entry.summary_pfxlen);
                PROCESS_MORE(buff);
                summary_addr_entry.indexlen = 8;
            }
            else
            {
                break;
            }
        }

        sprintf(buff,"  Distance: (default is 110)\r\n");
        PROCESS_MORE(buff);
    }

    *line_num_p = line_num;
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
    return CLI_NO_ERROR;
}
/*fuzhimin,20090106*/
#if (SYS_CPNT_IP_FOR_ETHERNET0 == TRUE)
UI32_T CLI_API_Ip_Address_Eth0(UI16_T cmd_idx, UI8_T *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
      UI32_T uint_ipAddress=0;
      UI32_T uint_ipMask=0;
      UI8_T   ip_address[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   mask[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   bcast_addr[SYS_ADPT_IPV4_ADDR_LEN];
      UI8_T   network_id[SYS_ADPT_IPV4_ADDR_LEN];

    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W2_IP_ADDRESS:
        CLI_LIB_AtoIp(arg[0], (UI8_T*)&uint_ipAddress);
        CLI_LIB_AtoIp(arg[1], (UI8_T*)&uint_ipMask);

        ip_address[0] = (uint_ipAddress&0xff000000)>>24;
        ip_address[1] = (uint_ipAddress&0xff0000)>>16;
        ip_address[2] = (uint_ipAddress&0xff00)>>8;
        ip_address[3] = (uint_ipAddress&0xff);
        mask[0] = (uint_ipMask&0xff000000)>>24;
        mask[1] = (uint_ipMask&0xff0000)>>16;
        mask[2] = (uint_ipMask&0xff00)>>8;
        mask[3] = (uint_ipMask&0xff);

             /* check ip_address, ip_mask,
             * 1. must be either class A, class B, or class C
             * 2. can't be network b'cast ip or network id
             * 3. can't be loop back ip
             * 4. can't be b'cast ip
             * 5. can't be m'cast ip
             */

             /* class A, class B or class C */
             if((IP_LIB_IsIpInClassA(ip_address) == FALSE) && (IP_LIB_IsIpInClassB(ip_address) == FALSE) &&
                     (IP_LIB_IsIpInClassC(ip_address) == FALSE) &&
                         (((ip_address[0] & mask[0])!= 0) ||
                         ((ip_address[1] & mask[1])!= 0) ||
                         ((ip_address[2] & mask[2])!= 0) ||
                         ((ip_address[3] & mask[3])!= 0)))
             {
                 CLI_LIB_PrintStr("Invalid IP address. Must be either class A, class B, or class C.\r\n");
                 return CLI_NO_ERROR;
             }

             /*  network b'cast ip   */
             if (IP_LIB_GetSubnetBroadcastIp(ip_address, mask, bcast_addr) == IP_LIB_OK)
             {
                 if(memcmp(bcast_addr, ip_address, SYS_ADPT_IPV4_ADDR_LEN) == 0)
                 {
                     CLI_LIB_PrintStr("Invalid IP address. Can't be network b'cast ip.\r\n");
                     return CLI_NO_ERROR;
                 }
             }

              /*  network ID  */
              IP_LIB_GetNetworkID(ip_address, mask, network_id);
              if (memcmp(network_id, ip_address, SYS_ADPT_IPV4_ADDR_LEN) == 0 )
              {
                  CLI_LIB_PrintStr("Invalid IP address. Can't be network id\r\n");
                  return CLI_NO_ERROR;
              }

             /* loopback ip, b'cast ip, or m'cast ip */
             if ((IP_LIB_IsLoopBackIp (ip_address) == TRUE) ||
                 (IP_LIB_IsMulticastIp(ip_address) == TRUE) ||
                 (IP_LIB_IsBroadcastIp(ip_address) == TRUE))
             {
                 CLI_LIB_PrintStr("Invalid IP address. Can't be loopback ip, b'cast ip, or m'cast ip\r\n");
                 return CLI_NO_ERROR;
             }

        /*set ethernet0 ip address*/
        if(setEth0IpAddr("eth",0,uint_ipAddress,uint_ipMask)==-1)
             {
                  CLI_LIB_PrintStr("Failed to add IP address of Ethernet0.\r\n");
             }
        break;

    case PRIVILEGE_CFG_INTERFACE_ETH0_CMD_W3_NO_IP_ADDRESS:
              if(delEth0IpAddr("eth",0)==-1)
              {
                  CLI_LIB_PrintStr("Failed to delete IP address of Ethernet0.\r\n");
              }
            break;

    default:
          return CLI_ERR_INTERNAL;
    }

    return CLI_NO_ERROR;

}
#endif
/*fuzhimin,20090106,end*/

UI32_T CLI_API_Show_Ip_Helper(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_UDP_HELPER == TRUE)
    UI32_T flag;
    L_INET_AddrIp_T helper;
    UI32_T forward_port;
    UI32_T helper_count;
    UI32_T port_count;
    UI32_T ret;
    UI32_T status;
    UI32_T vid = 0;
    char  buff[CLI_DEF_MAX_BUFSIZE] = {0};
    UI32_T line_num = 0;
    NETCFG_TYPE_L3_Interface_T ip_interface;

    /* Global information */
    ret = UDPHELPER_PMGR_GetStatus(&status);
    if ( ret == UDPHELPER_TYPE_RESULT_SUCCESS )
    {
        sprintf(buff,"Helper mechanism is %s\r\n", status ? "enabled" : "disabled");
        PROCESS_MORE(buff);
    }
    /* Forward port information */
    sprintf(buff,"Forward port list (maximum count: %u) \r\n", SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT);
    PROCESS_MORE(buff);
    forward_port = 0;
    port_count = 0;
    while(UDPHELPER_PMGR_GetNextForwardPort(&forward_port) == UDPHELPER_TYPE_RESULT_SUCCESS )
    {
        port_count++;
        sprintf(buff,"  %lu \r\n", forward_port);
        PROCESS_MORE(buff);
    }
    sprintf(buff,"Total port number now is: %lu\r\n", port_count);
    PROCESS_MORE(buff);
    /* Helper address information */
    memset(&ip_interface, 0, sizeof(NETCFG_TYPE_L3_Interface_T));
    helper_count = 0;
    sprintf(buff,"Helper address list (maximum count: %u) \r\n", SYS_ADPT_UDPHELPER_MAX_HELPER);
    PROCESS_MORE(buff);
    while(NETCFG_POM_IP_GetNextL3Interface(&ip_interface) == NETCFG_TYPE_OK)
    {
        flag = TRUE;
        memset(&helper, 0, sizeof(helper));
        while(UDPHELPER_PMGR_GetNextHelper(ip_interface.ifindex, &helper) == UDPHELPER_TYPE_RESULT_SUCCESS )
        {
            if ( flag )
            {
                flag = FALSE;
                VLAN_IFINDEX_CONVERTTO_VID(ip_interface.ifindex, vid);
                sprintf(buff,"Interface VLAN %lu: \r\n", vid);
                PROCESS_MORE(buff);
            }
            helper_count++;
            sprintf(buff, "  %d.%d.%d.%d\r\n",
                    helper.addr[0], helper.addr[1],
                    helper.addr[2],helper.addr[3]);
            PROCESS_MORE(buff);
        }
    }
    sprintf(buff,"Total helper number now is: %lu\r\n", helper_count);
    PROCESS_MORE(buff);
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_helper_status(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_UDP_HELPER == TRUE)
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_IP_HELPER:
        if(UDPHELPER_PMGR_SetStatus(TRUE) != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
           CLI_LIB_PrintStr("Failed to enable UDP helper.\r\n");
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_IP_HELPER:
        if(UDPHELPER_PMGR_SetStatus(FALSE) != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
           CLI_LIB_PrintStr("Failed to disable UDP helper.\r\n");
        }
       break;

    default:
       return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_Ip_helper_address(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_UDP_HELPER == TRUE)
    struct pal_in4_addr addr;
    L_INET_AddrIp_T inet_addr;
    UI32_T ret=UDPHELPER_TYPE_RESULT_FAIL;
    CLI_API_L3_Str2Addr (arg[0], &addr);
    memset(&inet_addr, 0, sizeof(inet_addr));
    inet_addr.type = L_INET_ADDR_TYPE_IPV4;
    inet_addr.preflen = 32;
    inet_addr.addrlen = 4;
    memcpy(inet_addr.addr, &addr, sizeof(addr));
    if (addr.s_addr == 0)
    {
        CLI_LIB_PrintStr("Cannot config zero helper address.\r\n");
        return CLI_NO_ERROR;
    }
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W2_IP_HELPERADDRESS:
        ret = UDPHELPER_PMGR_AddHelperAddress(ctrl_P->CMenu.vlan_ifindex, inet_addr);
        if( ret != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
            if(ret == UDPHELPER_TYPE_RESULT_HELPER_FULL)
                CLI_LIB_PrintStr("Failed to add IP helper address on interface.(Full of IP helper address)\r\n");
            else
                CLI_LIB_PrintStr("Failed to add IP helper address on interface.\r\n");
        }
        break;

    case PRIVILEGE_CFG_INTERFACE_VLAN_CMD_W3_NO_IP_HELPERADDRESS:
        if(UDPHELPER_PMGR_DelHelperAddress(ctrl_P->CMenu.vlan_ifindex, inet_addr) != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
           CLI_LIB_PrintStr("Failed to delete IP helper address on interface.\r\n");
        }
       break;

    default:
       return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

UI32_T CLI_API_L3_Ip_Forward_Protocol(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_UDP_HELPER == TRUE)
    UI32_T ret=UDPHELPER_TYPE_RESULT_FAIL;
    if (strtoul(arg[0], NULL, 0) == 0)
    {
        CLI_LIB_PrintStr("Cannot config zero port.\r\n");
        return CLI_NO_ERROR;
    }
    switch(cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_IP_FORWARDPROTOCOL_UDP:
        ret = UDPHELPER_PMGR_AddForwardUdpPort(strtoul(arg[0], NULL, 0));
        if(ret != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
            if(ret == UDPHELPER_TYPE_RESULT_FORWARD_PORT_FULL)
                CLI_LIB_PrintStr("Failed to add forward port.(Full of forward port)\r\n");
            else
                CLI_LIB_PrintStr("Failed to add forward port.\r\n");
        }
        break;

    case PRIVILEGE_CFG_GLOBAL_CMD_W4_NO_IP_FORWARDPROTOCOL_UDP:
        if(UDPHELPER_PMGR_DelForwardUdpPort(strtoul(arg[0], NULL, 0)) != UDPHELPER_TYPE_RESULT_SUCCESS)
        {
           CLI_LIB_PrintStr("Failed to delete forward port.\r\n");
        }
       break;

    default:
       return CLI_NO_ERROR;
    }
#endif
    return CLI_NO_ERROR;
}

/* command: [no] ecmp load-balance
 *               { dst-ip-l4-port | hash-selection-list hsl-id }
 */
UI32_T CLI_API_Ecmp_Load_Balance(
    UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    UI32_T  mode = SYS_DFLT_ECMP_BALANCE_MODE, hidx =0;

    switch (cmd_idx)
    {
    case PRIVILEGE_CFG_GLOBAL_CMD_W2_ECMP_LOADBALANCE:
        if ((arg[0][0] == 'h') || (arg[0][0] == 'H'))
        {
            mode = NETCFG_TYPE_ECMP_HASH_SELECTION;
            hidx = atoi(arg[1]);
        }
        break;
    case PRIVILEGE_CFG_GLOBAL_CMD_W3_NO_ECMP_LOADBALANCE:
        break;
    default:
        return CLI_NO_ERROR;
    }

    if (NETCFG_TYPE_OK != NETCFG_PMGR_ROUTE_SetEcmpBalanceMode(mode, hidx))
    {
#if (SYS_CPNT_EH == TRUE)
        CLI_API_Show_Exception_Handeler_Msg();
#else
        CLI_LIB_PrintStr("Failed to set ECMP load balance mode.\r\n");
#endif
    }

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

    return CLI_NO_ERROR;
}

/* command: show ecmp load-balance */
UI32_T CLI_API_L3_Show_Ecmp_Load_Balance(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
#if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE)
    char *ecmp_tag = "ECMP Load Balance Mode";

    UI32_T  mode, idx;

    if (NETCFG_TYPE_OK != NETCFG_POM_ROUTE_GetEcmpBalanceMode(&mode, &idx))
    {
        mode = NETCFG_TYPE_ECMP_MAX;
    }

    switch(mode)
    {
    case NETCFG_TYPE_ECMP_HASH_SELECTION:
        CLI_LIB_Printf(" %s : %s (%ld)\r\n",ecmp_tag, "Hash Selection List", idx);
        break;
    case NETCFG_TYPE_ECMP_DIP_L4_PORT:
        CLI_LIB_Printf(" %s : %s\r\n", ecmp_tag, "Destination IP Address And L4 Port");
        break;
#if 0
    case NETCFG_TYPE_ECMP_DST_IP:
        CLI_LIB_Printf(" %s : %s", ecmp_tag, "Destination IP Address");
        break;
#endif
    default:
        CLI_LIB_PrintStr("Failed to display ECMP load balance information.\r\n");
        break;
    }

#endif /* #if (SYS_CPNT_ECMP_BALANCE_MODE == TRUE) */

   return CLI_NO_ERROR;
}

