/*--------------------------------------------------------------------------+
 * FILE NAME - cgi_lib.c                                                    +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file                                                      +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *                                   creation                               +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * -------------------------------------------------------------------------*/

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "sys_cpnt.h"

//#include "c_lib.h"
#include "cgi_lib.h"
#include "cgi_coretype.h"
#include "cgi_util.h"
#include "cgi_real.h"
#include "cgi.h"
#include "cgi_debug.h"

#include <libxml/parser.h>
#include "http_envcfg.h"

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
#include "ospf_pmgr.h"
#include "ospf_type.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#include "uc_mgr.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "sys_mgr.h"
#include "sys_pmgr.h"
#include "sys_time.h"
#include "sys_hwcfg.h"
#include "swdrv_type.h"
#include "swdrv_lib.h"
#include "mib2_mgr.h"
#include "mib2_pom.h"
#include "iml_mgr.h"
#include "l_inet.h"
//#include "netcfg_mgr.h"
#include "stktplg_type.h"
#include "stktplg_mgr.h"
#include "stktplg_pmgr.h"
#include "stkctrl_pmgr.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "vlan_mgr.h"
#include "vlan_pmgr.h"
#include "vlan_lib.h"
#include "userauth.h"
#include "userauth_pmgr.h"
#include "leaf_sys.h"
#include "leaf_vbridge.h"
//#include "bdg_ext.h"
#include "nmtr_mgr.h"
#include "if_mgr.h"
#include "if_pmgr.h"
#include "telnet_mgr.h"
#include "telnet_pmgr.h"
#include "fs.h"
#include "fs_type.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "amtr_pmgr.h"
#include "http_mgr.h"
#include "trk_mgr.h"
#include "lacp_mgr.h"
#include "lacp_pmgr.h"
#include "lacp_pom.h"
#include "xfer_pmgr.h"
#include "l_cvrt.h"
#include "sys_dflt_xor.h"
#include "trk_pmgr.h"
#include "ip_lib.h"
#include "extbrg_pmgr.h"

#if (SYS_CPNT_XFER_FTP == TRUE)
#include "xfer_mgr.h"
#endif /* #if (SYS_CPNT_XFER_FTP == TRUE) */

#include "netcfg_pom_arp.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_arp.h"

#if (SYS_CPNT_ROUTING == TRUE)
#include "netcfg_type.h"
#include "netcfg_pmgr_route.h"
#include "nsm_type.h"
#include "nsm_pmgr.h"
#if (SYS_CPNT_RIP == TRUE)
#include "netcfg_pmgr_rip.h"
#endif /* #if (SYS_CPNT_RIP == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP))
#include "xstp_type.h"
#include "xstp_mgr.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#endif /* #if ((SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP) || (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)) */

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
#include "sys_reload_mgr.h"
#endif /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */

#if (SYS_CPNT_COS == TRUE)
#include "pri_pmgr.h"
#include "cos_vm.h"
#endif /* #if (SYS_CPNT_COS == TRUE) */

#if (SYS_CPNT_QOS_V2 == TRUE)
#include "l4_mgr.h"
#include "l4_pmgr.h"
#else
#include "l4_cos_mgr.h"
#include "l4_cos_pmgr.h"
#include "l4_ds_mgr.h"
#include "l4_ds_pmgr.h"
#endif /* #if (SYS_CPNT_QOS_V2 == TRUE) */

#if (SYS_CPNT_ADD == TRUE)
#include "add_mgr.h"
#include "add_pmgr.h"
#endif /* #if (SYS_CPNT_ADD == TRUE) */

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#endif /* #if (SYS_CPNT_AAA == TRUE) */

#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_mgr.h"
#include "radius_pom.h"
#endif /* #if (SYS_CPNT_RADIUS == TRUE) */

#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_mgr.h"
#include "tacacs_pom.h"
#endif /* #if (SYS_CPNT_TACACS == TRUE) */

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_pmgr.h"
#include "webauth_type.h"
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
#include "dai_pmgr.h"
#include "dai_type.h"
#endif  /* #if (SYS_CPNT_DAI == TRUE) */

#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_pom.h"
#include "1x_mgr.h"
#include "1x_pmgr.h"
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_type.h"
#include "netaccess_mgr.h"
#include "netaccess_pmgr.h"
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */

#if (SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
#include "sshd_mgr.h"
#include "sshd_pmgr.h"
#include "keygen_mgr.h"
#include "keygen_type.h"
#include "keygen_pmgr.h"
#endif /* #if (SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE) */

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
#include "mgmt_ip_flt.h"
#endif /* #if (SYS_CPNT_MGMT_IP_FLT == TRUE) */

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_mgr.h"
#include "psec_pmgr.h"
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

#if ((SYS_CPNT_SYSLOG == TRUE) || (SYS_CPNT_REMOTELOG == TRUE))
#include "syslog_mgr.h"
#endif /* #if ((SYS_CPNT_SYSLOG == TRUE) || (SYS_CPNT_REMOTELOG == TRUE)) */

#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_mgr.h"
#endif /* #if (SYS_CPNT_SMTP == TRUE) */

#if (SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pom.h"
#include "cluster_type.h"
#endif /* #if (SYS_CPNT_CLUSTER == TRUE) */

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#include "cfm_type.h"
#endif  /* #if (SYS_CPNT_CFM == TRUE) */

#if (SYS_CPNT_SNTP == TRUE)
#include "sntp_mgr.h"
#include "sntp_pmgr.h"
#endif /* #if (SYS_CPNT_SNTP == TRUE) */

#if (SYS_CPNT_NTP == TRUE)
#include "ntp_pmgr.h"
#endif /* #if (SYS_CPNT_NTP == TRUE) */

#if (SYS_CPNT_EFM_OAM ==TRUE)
#include "oam_type.h"
#include "oam_mgr.h"
#include "leaf_4878.h"
#endif /* #if (SYS_CPNT_EFM_OAM ==TRUE) */

#if (SYS_CPNT_POE == TRUE)
#include "poe_pom.h"
#include "stktplg_om.h"
#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
#include "stktplg_board.h"
#endif /* #if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE) */
#endif /* #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_SNMP == TRUE)
#include "trap_mgr.h"
#include "leaf_1213.h"
#include "leaf_2863.h"
#include "leaf_3413n.h"
#if (SYS_CPNT_SNMP_VERSION == 3)
#include "snmp_mgr.h"
#include "snmp_pmgr.h"
#include "leaf_3415.h"
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
#endif /* #if (SYS_CPNT_SNMP == TRUE) */

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_mgr.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "lldp_type.h"
#include "leaf_ansi_tia_1057.h"
#endif /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_type.h"
#include "dhcp_pmgr.h"
#endif /* #if (SYS_CPNT_DHCP == TRUE) */

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "dhcp_mgr.h"
#include "dhcp_pom.h"
#include "dhcp_om.h"
#endif /* #if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif /* #if (SYS_CPNT_DHCPSNP == TRUE) */

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pom.h"
#endif

#include "swctrl_pom.h"
#include "swctrl_pmgr.h"
#include "vlan_pom.h"

#if (SYS_CPNT_IPV6 == TRUE)
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_nd.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_ip.h"
#include "ipal_if.h"
#include "ipal_types.h"
#endif  /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#include "dns_type.h"
#endif /* #if (SYS_CPNT_DNS == TRUE) */

#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#include "udphelper_type.h"
#endif /* #if (SYS_CPNT_UDP_HELPER == TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
#include "vrrp_type.h"
#include "vrrp_pmgr.h"
#endif /* #if (SYS_CPNT_VRRP == TRUE) */

#include "leaf_es3626a.h"

#if (SYS_CPNT_PING == TRUE)
#include "leaf_2925p.h" /* ping MIB */
#include "ping_pmgr.h"
#include "ping_pom.h"
#endif  /* #if (SYS_CPNT_PING == TRUE) */

#if (SYS_CPNT_TRACEROUTE == TRUE)
#include "traceroute_pmgr.h"
#include "traceroute_pom.h"
#if (SYS_CPNT_ROUTING == TRUE)
#include "ipal_types.h"
#include "ipal_route.h"
#endif
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "dhcpsnp_pmgr.h"
#endif /* #if (SYS_CPNT_IP_SOURCE_GUARD == TRUE) */

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
#include "ip6sg_pmgr.h"
#include "ip6sg_pom.h"
#include "ip6sg_type.h"
#endif /* #if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE) */

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#include "rspan_om.h"
#endif /* #if (SYS_CPNT_RSPAN == TRUE) */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
#include "netcfg_pmgr_ip.h"
#endif

#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#include "dos_om.h"
#endif  /* #if (SYS_CPNT_DOS == TRUE) */

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#endif /* #if (SYS_CPNT_PPPOE_IA == TRUE) */

#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pom.h"
#include "mldsnp_type.h"
#endif /* #if (SYS_CPNT_MLDSNP == TRUE) */

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#include "nmtr_pmgr.h"
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_pmgr.h"
#endif

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pom.h"
#endif

/*----------------------------
 * EXPORTED SUBPROGRAM BODIES
 *----------------------------*/


#if 1


#define IS_THIS_DEBUG_ON()  (CGI_DEBUG_GetFlag() & CGI_DEBUG_LIBRARY)

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_RIP == TRUE)
#define PFXLEN2MASK(len) ~((1<<(32-len)) - 1)
#endif /* #if (SYS_CPNT_RIP == TRUE) */

#if (SYS_CPNT_OSPF == TRUE)

/* OSPF LSA Type definition. */
#define OSPF_UNKNOWN_LSA                0
#define OSPF_ROUTER_LSA                 1
#define OSPF_NETWORK_LSA                2
#define OSPF_SUMMARY_LSA                3
#define OSPF_SUMMARY_LSA_ASBR           4
#define OSPF_AS_EXTERNAL_LSA            5
#define OSPF_GROUP_MEMBER_LSA           6  /* Not supported. */
#define OSPF_AS_NSSA_LSA                7
#define OSPF_EXTERNAL_ATTRIBUTES_LSA    8  /* Not supported. */
#define OSPF_LINK_OPAQUE_LSA            9
#define OSPF_AREA_OPAQUE_LSA            10
#define OSPF_AS_OPAQUE_LSA              11
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#ifndef MAX
#define MAX(x1,x2)  ((x1) >= (x2) ? (x1) : (x2))
#endif

#define STR_BUFF_UNUSED(ary) sizeof(ary)-strlen(ary)-1

#include "cgi_lib_json.c"

#endif

