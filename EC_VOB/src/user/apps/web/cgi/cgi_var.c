/*--------------------------------------------------------------------------+
 * FILE NAME - cgi_var.c                                                    +
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
 * Copyright(C)                            Edgecore Technology Corp., 2012  +
 * -------------------------------------------------------------------------*/

/*---------------------------
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include "cgi.h"

#include "cgi_var.h"
#include "cgi_lib.h"
#include "cgi_coretype.h"
#include "cgi_util.h"

#include "sys_cpnt.h"

#include "swdrv_type.h"
#include "swdrv_lib.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "swctrl_pmgr.h"
#include "vlan_pom.h"
#include "vlan_pmgr.h"
#include "vlan_mgr.h"
#include "vlan_lib.h"
#include "trk_mgr.h"
#include "trk_pmgr.h"
#include "lacp_mgr.h"
#include "lacp_pmgr.h"
#include "netcfg_pom_arp.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_arp.h"
#include "sys_hwcfg.h"
#include "if_pmgr.h"
#include "ip_lib.h"

#if (SYS_CPNT_IPV6 == TRUE)
#include "netcfg_pom_nd.h"
#include "netcfg_pmgr_nd.h"
#endif

#if (SYS_CPNT_QOS_V2 == TRUE)
#include "l4_mgr.h"
#include "l4_pmgr.h"
#else
#include "l4_cos_mgr.h"
#include "l4_cos_pmgr.h"
#include "l4_ds_mgr.h"
#include "l4_ds_pmgr.h"
#endif /* #if (SYS_CPNT_QOS_V2 == TRUE) */

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_mgr.h"
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#include "lldp_type.h"
#include "leaf_ansi_tia_1057.h"
#endif /* #if (SYS_CPNT_LLDP == TRUE) */

#if (SYS_CPNT_SNMP == TRUE)
#if (SYS_CPNT_SNMP_VERSION == 3)
#include "snmp_mgr.h"
#include "snmp_pmgr.h"
#endif /* #if (SYS_CPNT_SNMP_VERSION == 3) */
#endif /* #if (SYS_CPNT_SNMP == TRUE) */

#if (SYS_CPNT_DHCP_INFORM == TRUE)
#include "netcfg_pmgr_ip.h"
#endif /* #if (SYS_CPNT_DHCP_INFORM == TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
#include "vrrp_type.h"
#include "vrrp_pmgr.h"
#endif /* #if (SYS_CPNT_VRRP == TRUE) */

#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pom.h"
#include "mldsnp_type.h"
#endif /* #if (SYS_CPNT_MLDSNP == TRUE) */

#if (SYS_CPNT_ROUTING == TRUE)
#include "netcfg_type.h"
#if (SYS_CPNT_OSPF == TRUE)
#include "ospf_pmgr.h"
#include "ospf_type.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_POE == TRUE)
#include "poe_pom.h"
#include "stktplg_om.h"
#if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE)
#include "stktplg_board.h"
#endif /* #if (SYS_CPNT_POE_PORT_MAX_ALLOC_DIFF == TRUE) */
#endif /* #if (SYS_CPNT_POE == TRUE) */

#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#include "dos_om.h"
#endif  /* #if (SYS_CPNT_DOS == TRUE) */

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif /* SYS_CPNT_DHCPSNP */

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pom.h"
#include "ipsg_pmgr.h"
#endif

#if (SYS_CPNT_DAI == TRUE)
#include "dai_pmgr.h"
#endif

#include "sys_time.h"

/*----------------------------
 * EXPORTED SUBPROGRAM BODIES
 *----------------------------*/
#ifndef MAX
#define MAX(x1,x2) ((x1) >= (x2) ? (x1) : (x2))
#endif /* #ifndef MAX */

#ifndef STR_BUFF_UNUSED
#define STR_BUFF_UNUSED(ary) (sizeof(ary) - strlen(ary) - 1)
#endif /* #ifndef STR_BUFF_UNUSED */

