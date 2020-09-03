/* MODULE NAME:  foundry_init.c
 *
 * PURPOSE: For SNMP to access Brocade's private MIB files:
 *
 * enterprises.foundry(1991).products(1)
 *
 * NOTES:
 *
 * HISTORY (mm/dd/yyyy)
 *    05/09/2011 - Qiyao Zhong, Created
 *
 * Copyright(C)      Accton Corporation, 2011
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "foundry_sn_agent.h"
#include "foundry_sn_switch_group.h"
#include "foundry_sn_arp_group.h"
#include "foundry_sn_mac_authentication.h"
#include "foundry_mac_vlan.h"
#include "foundry_lag.h"
#include "fdry_dai.h"
#include "fdry_dhcp_snooping.h"
#include "fdry_ip_source_guard.h"
#include "fdry_sntp.h"
#include "fdry_radius.h"
#include "fdry_tacacs.h"
#include "fdry_trap.h"
#include "brocade_syslog.h"
#include "foundry_sn_ip.h"

/* ------------------------------------------------------------------------
 * FUNCTION NAME - init_foundry
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initialisation routine for the whole Brocade MIB:
 *
 *              enterprises.foundry(1991).products(1)
 *
 * INPUT    :   None.
 *
 * OUTPUT   :   None.
 *
 * RETURN   :   None.
 *
 * NOTES    :   None.
 * ------------------------------------------------------------------------
 */
void init_foundry(void)
{
    /* MODULE-IDENTITY node:
     * 4 FOUNDRY-SN-AGENT-MIB.snAgent - nothing inside
     *
     * Variables:
     * 1.1.1 FOUNDRY-SN-AGENT-MIB.snChassis.*
     * 1.1.2 FOUNDRY-SN-AGENT-MIB.snAgentSys.*
     */
    init_snAgent();

    /* 1.1.3 FOUNDRY-SN-SWITCH-GROUP-MIB.snSwitch
     */
    init_snSwitch();     

#if (SYS_CPNT_ROUTING == TRUE)
    /* 1.1.3.22 FOUNDRY-SN-ARP-GROUP-MIB.snArpInfo
     */
    init_snArpInfo();
#endif

#if (SYS_CPNT_MAC_VLAN == TRUE)
    /* 1.1.3.32 FOUNDRY-MAC-VLAN-MIB.fdryMacVlanMIB
     */
    init_fdryMacVlanMIB();
#endif

    /* 1.1.3.33 FOUNDRY-LAG-MIB.fdryLinkAggregationGroupMIB
     */
    init_fdryLinkAggregationGroupMIB();     

#if (SYS_CPNT_NETACCESS_MACAUTH == TRUE)
    /* 1.1.3.34 FOUNDRY-SN-MAC-AUTHENTICATION-MIB.snMacAuth
     */
    init_snMacAuth();
#endif

#if (SYS_CPNT_DAI == TRUE)
    /* 1.1.3.35 FDRY-DAI-MIB.fdryDaiMIB
     */
    init_fdryDaiMIB();
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
    /* 1.1.3.36 FDRY-DHCP-SNOOP-MIB.fdryDhcpSnoopMIB
     */
    init_fdryDhcpSnoopMIB();
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    /* 1.1.3.37 FDRY-DHCP-SNOOP-MIB.fdryIpSrcGuardMIB
     */
    init_fdryIpSrcGuardMIB();
#endif

#if (SYS_CPNT_SNTP == TRUE)
    /* 1.1.7.1 FDRY-SNTP-MIB.fdrySntpServerTable
     */
    init_fdrySntpServerTable();
#endif

#if (SYS_CPNT_RADIUS == TRUE)
    /* 1.1.8.1 FDRY-RADIUS-MIB.fdryRadiusServerTable
     */
    init_fdryRadiusServerTable();
#endif

#if (SYS_CPNT_TACACS == TRUE)
    /* 1.1.9.1 FDRY-TACACS-MIB.fdryTacacsServerTable
     */
    init_fdryTacacsServerTable();
#endif

#if (SYS_CPNT_SNMP == TRUE)
    /* 1.1.10.1 FDRY-TRAP-MIB.fdryTrapReceiverTable
     */
    init_fdryTrapReceiverTable();
#endif

#if (SYS_CPNT_SYSLOG == TRUE)
#if (SYS_CPNT_REMOTELOG == TRUE)
    /* 1.1.11.1 BROCADE-SYSLOG-MIB.brocadeSysLogMIB
     */
    init_brcdSysLogServerTable();
#endif  /* (SYS_CPNT_REMOTELOG == TRUE) */
#endif  /*(SYS_CPNT_SYSLOG == TRUE) */

    /* 1.2.2 FOUNDRY-SN-IP-MIB.snIp
     */
    init_snIp();

#if 0  /* not support for now */
    /* 1.2.6 FOUNDRY-SN-IGMP-MIB.snIgmp
     */
    init_snIgmp();
#endif
}
