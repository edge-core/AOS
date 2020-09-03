/* MODULE NAME: MAC_SYNC_PROC.C
 *
 * PURPOSE:
 *    This module implements the functionality of syncing EVPN MACs to AMTR.
 *
 * NOTES:
 *    1. monitor netlink RTM_NEWNEIGH/RTM_DELNEIGH messages from kernel.
 *    2. decode messages and send to AMTR.
 *
 * Copyright(C)      Accton Corporation, 2020
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/if_arp.h>
#include <argp.h>
#include <libgen.h>
#include <signal.h>

#include "libnetlink.h"

#include "l_inet.h"
#include "amtr_pmgr.h"
#include "amtrl3_pom.h"
#include "vxlan_pom.h"
#include "vxlan_pmgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MAC_SYNC_DBG_NONE           0
#define MAC_SYNC_DBG_ERR            1
#define MAC_SYNC_DBG_INF            2
#define MAC_SYNC_DBG_TRC            3
#define MAC_SYNC_DBG_MIN            MAC_SYNC_DBG_NONE
#define MAC_SYNC_DBG_MAX            MAC_SYNC_DBG_TRC

#define MAC_SYNC_VTEP_NAME_PFX      "vtep"

/* MACRO FUNCTION DECLARATIONS
 */
#define xstr(s) str(s)
#define str(s)  #s
#define dbglvl_help   "The verbosity of debug messages (" xstr(MAC_SYNC_DBG_MIN) \
                          "~" xstr(MAC_SYNC_DBG_MAX) ")."

#define MAC_SYNC_DBG_PRINTF  printf

#define MAC_SYNC_DBG_IS_DBG_ON(flag)                            \
            (MAC_SYNC_PROC_IsDbgOn (MAC_SYNC_ ## flag))

#define MAC_SYNC_DBG_MSG(flag, fmt, ...)                        \
        if (MAC_SYNC_DBG_IS_DBG_ON(flag))          \
            MAC_SYNC_DBG_PRINTF("%s:(%4d):%s " fmt "\r\n",      \
            #flag, __LINE__, basename(__FILE__), ##__VA_ARGS__ );


#define MAC_SYNC_DBG_MSG_DW(...)                                \
                do { MAC_SYNC_DBG_MSG(__VA_ARGS__) } while (0)

#define MAC_SYNC_DBG_MSG_SHORT(flag, fmt, ...)                  \
        if (MAC_SYNC_DBG_IS_DBG_ON(flag))                       \
            MAC_SYNC_DBG_PRINTF(fmt, ##__VA_ARGS__ );

#define MAC_SYNC_BIT_NLGRP(grp)          (1<<(grp-1))

#define MAC_SYNC_DUMP_NSFLAG(flag, name)    \
    if (flag & NUD_ ## name) {              \
        flag &= ~NUD_ ## name;              \
        MAC_SYNC_DBG_PRINTF(#name " ");     \
    }

/* DATA TYPE DECLARATIONS
 */
typedef struct {
    int     debuglvl;
} arguments_t;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DEFINITIONS
 */
static arguments_t mac_sync_arguments = {
    .debuglvl = MAC_SYNC_DBG_NONE,
};

static struct argp_option mac_sync_options[] =
{
    { "debuglvl", 'd', "DEBUGLVL", 0, dbglvl_help, 0 },
    { 0 }
};

static struct rtnl_handle mac_sync_rth = { .fd = -1 };
static UI8_T mac_sync_all_zero_mac[SYS_ADPT_MAC_ADDR_LEN] = {0};

/* EXPORTED SUBPROGRAM BODIES
 */

/* LOCAL SUBPROGRAM BODIES
 */
static void MAC_SYNC_PROC_SignalHandler(int sig)
{
    if (sig == SIGHUP)
    {
        mac_sync_arguments.debuglvl =
            (mac_sync_arguments.debuglvl) ? MAC_SYNC_DBG_NONE : MAC_SYNC_DBG_TRC;
    }
}

static void MAC_SYNC_PROC_InitSignals(void)
{
    struct sigaction sigact;

    sigact.sa_handler = MAC_SYNC_PROC_SignalHandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    sigaction(SIGHUP, &sigact, (struct sigaction *)NULL);
}

/* Parse a single option. */
static error_t
MAC_SYNC_PROC_ParseOpt(int key, char *arg, struct argp_state *state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    arguments_t *arguments_p = state->input;

    switch (key)
    {
    case 'd':
        errno = 0;

        arguments_p->debuglvl = strtoul(arg, NULL, 0);
        if (errno != 0)
        {
            argp_error(state, "Invalid debuglvl \"%s\"", arg);
            return errno;
        }
        break;

    case ARGP_KEY_ARG:
        /* Too many arguments. */
        if (state->arg_num >= 2)
            argp_usage (state);

        break;

    default:
        return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

/* return 1 if dbg flag is on
 */
static int MAC_SYNC_PROC_IsDbgOn(int flag)
{
    return (mac_sync_arguments.debuglvl >= flag);
}

const char *MAC_SYNC_PROC_GetHostname(
    int af, int len, const void *addr, char *buf, int buflen)
{
    switch (af) {
    case AF_INET:
    case AF_INET6:
        return inet_ntop(af, addr, buf, buflen);
    default:
        return "???";
    }
}

const char *MAC_SYNC_PROC_LladdrNtoa(const unsigned char *addr, int alen, char *buf, int blen)
{
    int i;
    int l;

    if (alen == 4) {
        return inet_ntop(AF_INET, addr, buf, blen);
    }
    if (alen == 16) {
        return inet_ntop(AF_INET6, addr, buf, blen);
    }
    snprintf(buf, blen, "%02x", addr[0]);
    for (i = 1, l = 2; i < alen && l < blen; i++, l += 3)
        snprintf(buf + l, blen - l, ":%02x", addr[i]);
    return buf;
}

static void MAC_SYNC_PROC_DumpNeighState(unsigned int ns_flag)
{
    if (ns_flag && (MAC_SYNC_DBG_IS_DBG_ON(DBG_INF)))
    {
        MAC_SYNC_DUMP_NSFLAG(ns_flag, INCOMPLETE);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, REACHABLE);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, STALE);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, DELAY);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, PROBE);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, FAILED);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, NOARP);
        MAC_SYNC_DUMP_NSFLAG(ns_flag, PERMANENT);
    }
}

static void MAC_SYNC_PROC_DumpNdmFlags(unsigned int ndm_flags)
{
    if (MAC_SYNC_DBG_IS_DBG_ON(DBG_INF))
    {
        if (ndm_flags & NTF_ROUTER)
            MAC_SYNC_DBG_PRINTF("router ");

        if (ndm_flags & NTF_PROXY)
            MAC_SYNC_DBG_PRINTF("proxy ");

        if (ndm_flags & NTF_EXT_LEARNED)
            MAC_SYNC_DBG_PRINTF("extern_learn ");
    }
}

static BOOL_T MAC_SYNC_PROC_ProcessVtep(BOOL_T is_add, UI32_T vnid, L_INET_AddrIp_T *host_ip_p)
{
    UI32_T  rc;

    if (FALSE == is_add)
        rc = VXLAN_PMGR_DelFloodRVtep(vnid, host_ip_p);
    else
        rc = VXLAN_PMGR_AddFloodRVtep(vnid, host_ip_p);

    if (VXLAN_TYPE_RETURN_OK != rc)
        MAC_SYNC_DBG_MSG_DW(DBG_ERR, "failed to set evpn vtep");
    else
        MAC_SYNC_DBG_MSG_DW(DBG_INF, "ok to set evpn vtep");

    return (VXLAN_TYPE_RETURN_OK == rc);
}

static int MAC_SYNC_PROC_PorcessNeighMsg(struct nlmsghdr *n)
{
    struct ndmsg *r = NLMSG_DATA(n);
    struct rtattr *tb[NDA_MAX+1];
    const char *dst = NULL;
    unsigned char *lladdr_p = NULL;

    int     len = n->nlmsg_len;
    int     lladdr_len = 0;
    UI32_T  vnid = 0;
    BOOL_T  is_add = FALSE;
    char    lladdr_buff[100];
    char    dst_buff[256];

    MAC_SYNC_DBG_MSG(DBG_INF, "len/type/flags: %08x %08x %08x",
        n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);

    len -= NLMSG_LENGTH(sizeof(*r));
    if (len < 0) {
        MAC_SYNC_DBG_MSG(DBG_ERR, "BUG: wrong nlmsg len %d", len);
        return -1;
    }

    switch (n->nlmsg_type)
    {
    case RTM_DELNEIGH:
        MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "Del ");
        break;
    case RTM_NEWNEIGH:
        MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "Add ");
        is_add = TRUE;
        break;
    default:
        return -1;
    }

    parse_rtattr(tb, NDA_MAX, NDA_RTA(r), n->nlmsg_len - NLMSG_LENGTH(sizeof(*r)));

    MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "fami %d ", r->ndm_family);

    if (tb[NDA_DST]) {
        int family = r->ndm_family;

        if (family == AF_BRIDGE) {
            if (RTA_PAYLOAD(tb[NDA_DST]) == sizeof(struct in6_addr))
                family = AF_INET6;
            else
                family = AF_INET;
        }

        dst = MAC_SYNC_PROC_GetHostname(
                family, RTA_PAYLOAD(tb[NDA_DST]), RTA_DATA(tb[NDA_DST]),
                dst_buff, sizeof(dst_buff));
        if (NULL != dst)
            MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "dst %s ", dst);
    }

    {
        char ifname_buff[IFNAMSIZ] = {0};
        extern char *if_indextoname(unsigned int __ifindex, char *__ifname);

        if (NULL != if_indextoname(r->ndm_ifindex, ifname_buff))
            MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "dev %s ", ifname_buff);

        if (0 == memcmp(ifname_buff, MAC_SYNC_VTEP_NAME_PFX, sizeof(MAC_SYNC_VTEP_NAME_PFX)-1))
        {
            vnid = strtoul(&ifname_buff[sizeof(MAC_SYNC_VTEP_NAME_PFX)-1], NULL, 0);
        }
    }

    if (tb[NDA_LLADDR]) {
        const char *lladdr_str = NULL;
        lladdr_p   = RTA_DATA(tb[NDA_LLADDR]);
        lladdr_len = RTA_PAYLOAD(tb[NDA_LLADDR]);
        lladdr_str = MAC_SYNC_PROC_LladdrNtoa(lladdr_p,
                     lladdr_len,
                     lladdr_buff, sizeof(lladdr_buff));

        if (NULL != lladdr_str)
            MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "lladdr %s ", lladdr_str);
    }

    MAC_SYNC_PROC_DumpNdmFlags(r->ndm_flags);

    MAC_SYNC_PROC_DumpNeighState(r->ndm_state);

    MAC_SYNC_DBG_MSG_SHORT(DBG_INF, "\n");

    /* for EVPN type 2 route, ex:
     *   Add fami 7 dst 100.100.100.101 dev vtep100 lladdr 00:03:00:06:02:8a extern_learn REACHABLE
     *
     * for EVPN type 3 route, ex:
     *   Add fami 7 dst 100.100.100.101 dev vtep10200 lladdr 00:00:00:00:00:00 NOARP PERMANENT
     *
     * TODO:
     *   only use this kind of message to add/del evpn mac/vtep.
     */
    if (0 != vnid && NULL != dst && NULL != lladdr_p)
    {
        L_INET_AddrIp_T                 host_ip;
        AMTRL3_OM_VxlanTunnelEntry_T    vxlan_tunnel = {0};
        AMTR_TYPE_AddrEntry_T           addr_entry = {0};
        VXLAN_OM_VNI_T                  vni_entry = {0};
        VXLAN_OM_RVtep_T                rvtep_entry = {0};
        UI16_T                          l_vxlan_port;

        if (lladdr_len != SYS_ADPT_MAC_ADDR_LEN)
        {
            MAC_SYNC_DBG_MSG(DBG_ERR, "unknown lladdr");
            return -1;
        }

        vni_entry.vni = vnid;
        if (VXLAN_TYPE_RETURN_OK != VXLAN_POM_GetVniEntry(&vni_entry))
        {
            MAC_SYNC_DBG_MSG(DBG_ERR, "vnid not exist");
            return -1;
        }

        if (L_INET_RETURN_SUCCESS != L_INET_StringToInaddr(L_INET_FORMAT_IP_ADDR,
                                                           dst,
                                                           (L_INET_Addr_T *) &host_ip,
                                                           sizeof(host_ip)))
        {
            MAC_SYNC_DBG_MSG(DBG_ERR, "invalid hostip");
            return -1;
        }

        if (0 == memcmp(mac_sync_all_zero_mac, lladdr_p, SYS_ADPT_MAC_ADDR_LEN))
        {
            /* for EVPN type 3 */
            MAC_SYNC_PROC_ProcessVtep(is_add, vnid, &host_ip);
        }
        else
        {
            BOOL_T  rc;

            /* for EVPN type 2 */
            addr_entry.vid       = vni_entry.vfi;
            addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
            addr_entry.source    = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
            addr_entry.action    = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;

            memcpy(addr_entry.mac, lladdr_p, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(addr_entry.r_vtep_ip, host_ip.addr, SYS_ADPT_IPV4_ADDR_LEN);

            if (VXLAN_POM_GetNextFloodRVtep(&rvtep_entry) != VXLAN_TYPE_RETURN_OK)
            {
                MAC_SYNC_DBG_MSG(DBG_ERR, "rvtep not exist");

                if (FALSE == is_add)
                    return 0;

                /* sometimes type 2 msg come before type 3 */
                if (FALSE == MAC_SYNC_PROC_ProcessVtep(is_add, vnid, &host_ip))
                {
                    MAC_SYNC_DBG_MSG(DBG_ERR, "failed to create rvtep");
                    return -1;
                }

                if (VXLAN_POM_GetNextFloodRVtep(&rvtep_entry) != VXLAN_TYPE_RETURN_OK)
                {
                    MAC_SYNC_DBG_MSG(DBG_ERR, "rvtep not exist");
                    return -1;
                }
            }

            vxlan_tunnel.local_vtep  = rvtep_entry.s_ip;
            vxlan_tunnel.vfi_id      = vni_entry.vfi;
            vxlan_tunnel.remote_vtep = host_ip;

            {
                UI32_T err_cnt;

                /* TODO: try to wait vxlan to create the vxlan tunnel */
                for (err_cnt =0; err_cnt < 10; err_cnt++)
                {
                    if (TRUE == AMTRL3_POM_GetVxlanTunnelEntry(SYS_ADPT_DEFAULT_FIB, &vxlan_tunnel))
                    {
                        break;
                    }

                    if (err_cnt < 10)
                    {
                        SYSFUN_Sleep(100);
                    }
                    else
                    {
                        MAC_SYNC_DBG_MSG(DBG_ERR, "vxlan tunnel not ready");
                        return -1;
                    }
                }
            }

            VXLAN_TYPE_R_PORT_CONVERTTO_L_PORT(vxlan_tunnel.uc_vxlan_port, l_vxlan_port);

            if (l_vxlan_port == 0)
                l_vxlan_port = SYS_ADPT_VXLAN_MAX_LOGICAL_PORT_ID - 1;
            addr_entry.ifindex = l_vxlan_port;

            MAC_SYNC_DBG_MSG_SHORT(DBG_INF,
                "lport/vid/rip/mac-%ld/%d/%d.%d.%d.%d/%02X%02X%02X:%02X%02X%02X\n",
                addr_entry.ifindex,
                addr_entry.vid,
                addr_entry.r_vtep_ip[0], addr_entry.r_vtep_ip[1],
                addr_entry.r_vtep_ip[2], addr_entry.r_vtep_ip[3],
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]
                );

            if (FALSE == is_add)
                rc = AMTR_PMGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
            else
                rc = AMTR_PMGR_SetAddrEntry(&addr_entry);

            if (FALSE == rc)
                MAC_SYNC_DBG_MSG_DW(DBG_ERR, "failed to set evpn mac");
            else
                MAC_SYNC_DBG_MSG_DW(DBG_INF, "ok to set evpn mac");
        }
    }

    return 0;
}

static int MAC_SYNC_PROC_RecvMsg(struct rtnl_ctrl_data *ctrl, struct nlmsghdr *n, void *arg)
{
    switch (n->nlmsg_type) {
    case RTM_NEWNEIGH:
    case RTM_DELNEIGH:
        MAC_SYNC_PROC_PorcessNeighMsg(n);
        return 0;

    default:
        fprintf(stderr,
            "Unknown message: type=0x%08x flags=0x%08x len=0x%08x\n",
            n->nlmsg_type, n->nlmsg_flags, n->nlmsg_len);
    }

    return 0;
}

#define MAC_SYNC_INIT_PROC_RESOURCE(api)           \
    if (api() == FALSE) {                          \
        MAC_SYNC_DBG_MSG(DBG_ERR, #api " Fail");   \
        return -1;                                 \
    }

int main(int argc, char **argv)
{
    /* Our argp parser. */
    static struct argp argp = { mac_sync_options, MAC_SYNC_PROC_ParseOpt, NULL, NULL };

    MAC_SYNC_PROC_InitSignals();

    /* Parse our arguments; every option seen by `parse_opt' will be
     * reflected in arguments.
     */
    argp_parse(&argp, argc, argv, 0, 0, &mac_sync_arguments);

    MAC_SYNC_DBG_MSG(DBG_INF, "debuglvl - %d", mac_sync_arguments.debuglvl);

    MAC_SYNC_INIT_PROC_RESOURCE(AMTR_PMGR_InitiateProcessResource);
    MAC_SYNC_INIT_PROC_RESOURCE(AMTRL3_POM_InitiateProcessResource);
    MAC_SYNC_INIT_PROC_RESOURCE(VXLAN_POM_InitiateProcessResources);
    MAC_SYNC_INIT_PROC_RESOURCE(VXLAN_PMGR_InitiateProcessResources);

    if (rtnl_open(&mac_sync_rth, MAC_SYNC_BIT_NLGRP(RTNLGRP_NEIGH)) < 0)
        exit(1);

    if (rtnl_listen(&mac_sync_rth, MAC_SYNC_PROC_RecvMsg, stdout) < 0)
        exit(2);

    return 0;
}

