/* MODULE NAME:  WEB_proc.c
 * PURPOSE:
 *    for WEB process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    7/6/2007 - Rich Lee, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "l_threadgrp.h"
#include "web_proc_comm.h"
#include "web_group.h"
#include "sys_cpnt.h"
#include "backdoor_mgr.h"
#include "sysrsc_mgr.h"
#include "l_cmnlib_init.h"
#include "uc_mgr.h"
#include "sys_pmgr.h"
#include "cli_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "l_mm_backdoor.h"
#include "http_init.h"
#include "cgi_init.h"

/*
#if (SYS_CPNT_HTTP == TRUE)
#include "HTTP_init.h"
#endif
*/
#if (SYS_CPNT_TELNET == TRUE)
#include "telnet_mgr.h"
#include "telnet_om.h"
#include "telnet_pmgr.h"
#include "telnet_pom.h"
#include "telnet_init.h"
#endif

#if (SYS_CPNT_SFLOW == TRUE)
#include "sflow_pmgr.h"
#endif /* #if (SYS_CPNT_SFLOW == TRUE) */

#if (SYS_CPNT_VLAN == TRUE)
#include "vlan_pmgr.h"
#endif

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_STA)
#include "sta_pmgr.h"
#endif

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#endif

#if (SYS_CPNT_AMTR == TRUE)
#include "amtr_pmgr.h"
#include "amtr_pom.h"
#endif

#if (SYS_CPNT_NMTR == TRUE)
#include "nmtr_pmgr.h"
#endif

#if (SYS_CPNT_SYSLOG == TRUE )
#include "syslog_pmgr.h"
#include "syslog_pom.h"
#endif

#if (SYS_CPNT_VRRP  == TRUE )
#include "vrrp_pmgr.h"
#include "vrrp_pom.h"
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#include "trk_pmgr.h"
#endif

#if (SYS_CPNT_STKMGMT==TRUE)
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"
#include "stkctrl_pmgr.h"
#endif

#if (SYS_CPNT_LACP == TRUE)
#include "lacp_pmgr.h"
#include "lacp_pom.h"
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_pmgr.h"
#endif

#include "extbrg_pmgr.h"

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pmgr.h"
#include "lldp_pom.h"
#endif

#if (SYS_CPNT_MLAG == TRUE)
#include "mlag_pmgr.h"
#include "mlag_pom.h"
#endif

#if (SYS_CPNT_SSH2 == TRUE)
    #include "sshd_init.h"
    #include "sshd_pmgr.h"
#endif

#if (SYS_CPNT_KEYGEN == TRUE)
    #include "keygen_pmgr.h"
#endif

#if (SYS_CPNT_QOS == SYS_CPNT_QOS_DIFFSERV)
    #include "l4_pmgr.h"
#endif

#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_pmgr.h"
#include "radius_pom.h"
#endif

#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_pmgr.h"
#include "tacacs_pom.h"
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_pmgr.h"
#endif

#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_pmgr.h"
#include "1x_pom.h"
#endif

#if(SYS_CPNT_MIB2MGMT==TRUE)
#include "if_pmgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#endif

#if(SYS_CPNT_XFER == TRUE)
#include "xfer_pmgr.h"
#endif

#if(SYS_CPNT_CLUSTER == TRUE)
#include "cluster_pmgr.h"
#include "cluster_pom.h"
#endif

#if(SYS_CPNT_SNMP == TRUE)
#include "snmp_pmgr.h"
#endif

#if (SYS_CPNT_DHCP==TRUE)
#include "dhcp_pmgr.h"
#endif

#include "userauth_pmgr.h"

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_mgr.h"
#include "aaa_pmgr.h"
#include "aaa_pom.h"
#endif

#include "netcfg_type.h"
#include "netcfg_pmgr_main.h"
#include "netcfg_pom_main.h"
#include "pri_pmgr.h"
#include "iml_pmgr.h"


#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_pmgr.h"
#endif

#if(SYS_CPNT_MGMT_IP_FLT==TRUE)
#include "mgmt_ip_flt.h"
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pmgr.h"
#include "ipsg_pom.h"
#endif

#if (SYS_CPNT_SNTP == TRUE)
#include "sntp_pmgr.h"
#endif

#if (SYS_CPNT_NTP == TRUE)
#include "ntp_pmgr.h"
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
#include "oam_pmgr.h"
#include "oam_pom.h"
#endif

#include "ping_pmgr.h"
#include "ping_pom.h"

#if(SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pmgr.h"
#include "mldsnp_pom.h"
#endif

#if(SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif

#if (SYS_CPNT_L2MCAST == TRUE)
#include "igv3snp_pmgr.h"
/*delete igmpsnp_pom.h xinhai.yuan*/
#endif

#if (SYS_CPNT_DHCP == TRUE)
#include "dhcp_pmgr.h"
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
#include "dhcpv6_pmgr.h"
#endif

#if (SYS_CPNT_ADD == TRUE)
#include "add_pmgr.h"
#endif

#if (SYS_CPNT_IPV6 == TRUE)
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_ip.h"
#include "netcfg_pmgr_nd.h"
#include "netcfg_pom_nd.h"
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_TRACEROUTE == TRUE)
#include "traceroute_pmgr.h"
#include "traceroute_pom.h"
#endif

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
#include "ospf_pmgr.h"
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_OSPF6 == TRUE)
#include "ospf6_pmgr.h"
#endif

#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#include "nsm_mgr.h"
#endif

#if (SYS_CPNT_DNS == TRUE)
#include "dns_pmgr.h"
#include "dns_pom.h"
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#endif /* #if (SYS_CPNT_UDP_HELPER == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
#include "dai_pmgr.h"
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_mgr.h"
#include "webauth_init.h"
#include "webauth_pmgr.h"
#endif

#if (SYS_CPNT_POE == TRUE )
#include "poe_pmgr.h"
#include "poe_pom.h"
#endif

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#endif

#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#endif

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
#include "ip6sg_pmgr.h"
#include "ip6sg_pom.h"
#endif /* #if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
#include "ipal_kernel.h"
#endif

#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_pmgr.h"
#include "vxlan_pom.h"
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pom.h"
#endif

#if (SYS_CPNT_PFC == TRUE)
#include "pfc_pmgr.h"
#endif

#if (SYS_CPNT_BGP == TRUE)
#include "bgp_pmgr.h"
#include "bgp_pom.h"
#endif

#if (SYS_CPNT_PBR == TRUE)
#include "netcfg_pom_pbr.h"
#include "netcfg_pmgr_pbr.h"
#endif

#if (SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
#include "led_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(WEB_PROC_OM_MSG_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union WEB_PROC_OM_MSG_U
{/*
#if (SYS_CPNT_HTTP == TRUE)
    HTTP_OM_IPCMsg_T http_om_ipcmsg;
#endif
*/
#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_MGR_IPCMsg_Data_T webauth_mgr_ipcmsg;
#endif
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} WEB_PROC_OM_MSG_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T WEB_PROC_Init(void);
static BOOL_T WEB_PROC_InitiateProcessResource(void);
static void   WEB_PROC_Create_InterCSC_Relation(void);
static void   WEB_PROC_Create_All_Threads(void);
static void   WEB_PROC_Daemonize_Entry(void* arg);

/* STATIC VARIABLE DECLARATIONS
 */

/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for WEB process
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for WEB process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(WEB_PROC_Init()==FALSE)
    {
        printf("WEB_Process_Init fail.\n");
        return -1;
    }
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_WEB_PROCESS_NAME,
                            WEB_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("WEB_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for WEB process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T WEB_PROC_Init(void)
{
    /* In WEB_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */
    L_CMNLIB_INIT_InitiateProcessResources();

    if(FALSE==SYSRSC_MGR_AttachSystemResources())
    {
        printf("%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if(WEB_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    /* Register callback fun
     */
    WEB_PROC_Create_InterCSC_Relation();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in WEB process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T WEB_PROC_InitiateProcessResource(void)
{
    if(WEB_PROC_COMM_InitiateProcessResource()==FALSE)
        return FALSE;

    /* self init
     */
    HTTP_INIT_InitiateProcessResource();
    CGI_INIT_InitiateSystemResources();

    if(CLI_PMGR_InitiateProcessResource() == FALSE)
    {
       printf(" CLI_PMGR_InitiateProcessResource fail\n");
       return FALSE;
    }

    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf(" UC_MGR_InitiateProcessResources fail\n");
       return FALSE;
    }

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_INIT_Initiate_System_Resources();
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    if(SYS_PMGR_InitiateProcessResource()== FALSE)
    {
       printf(" SYS_PMGR_InitiateProcessResource fail\n");
       return FALSE;
    }

 /*
  * system init
  */
    if(DEV_NICDRV_PMGR_InitiateProcessResource()==FALSE)
    {
        printf(" DEV_NICDRV_PMGR_InitiateProcessResource fail\n");
        return FALSE;
    }

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();

    if(TRK_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

#endif

#if (SYS_CPNT_STKMGMT==TRUE)
    if(STKTPLG_POM_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_POM_InitiateProcessResources fail\n");
       return FALSE;
    }

    if(STKTPLG_PMGR_InitiateProcessResources()== FALSE)
    {
       printf(" STKTPLG_PMGR_InitiateProcessResources fail\n");
       return FALSE;
    }

    if(STKCTRL_PMGR_InitiateProcessResources()==FALSE)
    {
       printf(" STKCTRL_PMGR_InitiateProcessResources fail\n");
       return FALSE;
    }
#endif

#if (SYS_CPNT_SYSLOG == TRUE )

    if(SYSLOG_PMGR_InitiateProcessResource()==FALSE)
    {
        printf(" SYSLOG_PMGR_InitiateProcessResource fail\n");
        return FALSE;
    }
    if(SYSLOG_POM_InitiateProcessResource() == FALSE)
    {
         printf(" SYSLOG_POM_InitiateProcessResource fail\n");
       return FALSE;
    }

#endif

 /*
  * l2 init
  */
#if (SYS_CPNT_VLAN == TRUE)
    if(VLAN_PMGR_InitiateProcessResource()==FALSE)
    {
        printf(" VLAN_PMGR_InitiateProcessResource fail\n");
        return FALSE;
    }
#endif

#if (SYS_CPNT_STP == SYS_CPNT_STP_TYPE_RSTP || SYS_CPNT_STP == SYS_CPNT_STP_TYPE_MSTP)
    if(XSTP_PMGR_InitiateProcessResource()==FALSE)
    {
        printf(" XSTP_PMGR_InitiateProcessResource fail\n");
        return FALSE;
    }
    if(XSTP_POM_InitiateProcessResource()==FALSE)
    {
        printf(" XSTP_POM_InitiateProcessResource fail\n");
        return FALSE;
    }
#endif

#if (SYS_CPNT_AMTR == TRUE)
    if(AMTR_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(AMTR_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_NMTR == TRUE)
    NMTR_PMGR_InitiateProcessResource();
#endif

    if(PRI_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;

#if (SYS_CPNT_LACP == TRUE)
    if(LACP_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
    if(LACP_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

    if(EXTBRG_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

#if (SYS_CPNT_LLDP == TRUE)
    if(LLDP_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;
    if(LLDP_POM_InitiateProcessResources()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_L2MCAST == TRUE )
    if(IGMPSNP_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_PMGR_InitiateProcessResources();
    MLDSNP_POM_InitiateProcessResources();
#endif

#endif

#if (SYS_CPNT_DHCP == TRUE)
    if(DHCP_PMGR_InitiateProcessResources() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
    if(DHCPv6_PMGR_InitiateProcessResources() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    if(DHCP_POM_InitiateProcessResource() == FALSE)
    {
       printf(" DHCP_POM_InitiateProcessResource fail\n");
       return FALSE;
    }
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
    if(DHCPSNP_PMGR_InitiateProcessResource() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    if(IPSG_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(IPSG_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    if(RSPAN_PMGR_InitiateProcessResource() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
    if(PPPOE_IA_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_MLAG == TRUE)
    if (MLAG_PMGR_InitiateProcessResources() == FALSE)
    {
    	return FALSE;
    }
    if (MLAG_POM_InitiateProcessResources() == FALSE)
    {
    	return FALSE;
    }
#endif

    /* L3 CSC*/
    if(IML_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(NETCFG_PMGR_MAIN_InitiateProcessResource()==FALSE)
        return FALSE;

    if(NETCFG_POM_MAIN_InitiateProcessResource()==FALSE)
        return FALSE;

#if (SYS_CPNT_IPV6 == TRUE)
    if(NETCFG_POM_IP_InitiateProcessResource()==FALSE)
    {
        return FALSE;
    }

    if(NETCFG_PMGR_IP_InitiateProcessResource()==FALSE)
    {
        return FALSE;
    }

    if(NETCFG_PMGR_ND_InitiateProcessResource()==FALSE)
    {
        return FALSE;
    }

    if(NETCFG_POM_ND_InitiateProcessResource()==FALSE)
    {
        return FALSE;
    }
#endif/* #if (SYS_CPNT_IPV6 == TRUE) */

#if (SYS_CPNT_DHCP==TRUE)
    if(DHCP_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;
#endif

    if(PING_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;

    if(PING_POM_InitiateProcessResources()==FALSE)
        return FALSE;

#if (SYS_CPNT_NSM == TRUE)
    if(NSM_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_CFM == TRUE)
    if(CFM_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
    if(UDPHELPER_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_VRRP == TRUE)
    if(VRRP_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;
    if(VRRP_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    if (VXLAN_PMGR_InitiateProcessResources() == FALSE)
    {
    	return FALSE;
    }
    if (VXLAN_POM_InitiateProcessResources() == FALSE)
    {
    	return FALSE;
    }
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
    if (AMTRL3_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

/*
 * Security init
 */
#if (SYS_CPNT_PORT_SECURITY == TRUE)
    if (PSEC_PMGR_InitiateProcessResources()==FALSE)
    {
        printf("%s: PSEC_PMGR_InitiateProcessResources failed\n", __FUNCTION__);
        return FALSE;
    }
#endif

#if (SYS_CPNT_SSH2 == TRUE)
{
    if (SSHD_PMGR_InitiateProcessResources()==FALSE)
    {
        printf("%s: SSHD_PMGR_InitiateProcessResources failed\n", __FUNCTION__);
        return FALSE;
    }
}
#endif

#if (SYS_CPNT_KEYGEN== TRUE)
{
    if (KEYGEN_PMGR_InitiateProcessResources()==FALSE)
    {
        printf("%s: KEYGEN_PMGR_InitiateProcessResources failed\n", __FUNCTION__);
        return FALSE;
    }
}
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_DAI == TRUE)
    if (DAI_PMGR_InitiateProcessResource() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_SFLOW == TRUE)
    if (SFLOW_PMGR_InitiateProcessResource() == FALSE)
        return FALSE;
#endif

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
    if (IP6SG_PMGR_InitiateProcessResource() == FALSE)
    {
        return FALSE;
    }

    if (IP6SG_POM_InitiateProcessResource() == FALSE)
    {
        return FALSE;
    }
#endif /* #if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE) */

/*
 * Utility init
 */
#if(SYS_CPNT_TELNET == TRUE)
    if( TELNET_PMGR_InitiateProcessResource() == FALSE)
    {
        printf("%s: TELNET_PMGR_InitiateProcessResource failed\n", __FUNCTION__);
        return FALSE;
    }

    if( TELNET_POM_InitiateProcessResource() == FALSE)
    {
        printf("%s: TELNET_POM_InitiateProcessResource failed\n", __FUNCTION__);
        return FALSE;
    }


#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
    IML_PMGR_InitiateProcessResource();
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    XFER_PMGR_InitiateProcessResource();

#if(SYS_CPNT_CLUSTER==TRUE)
    CLUSTER_PMGR_Init();
    CLUSTER_POM_Init();
#endif

#if(SYS_CPNT_MIB2MGMT==TRUE)
    IF_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
#endif

/*
 * UI
 */

/*
#if (SYS_CPNT_CLI == TRUE)
    if(CLI_INIT_InitiateProcessResources()==FALSE)
        return FALSE;
#endif
*/
#if (SYS_CPNT_SNMP == TRUE)
     SNMP_PMGR_Init();
#endif

/*
 * L4
 */
	L4_PMGR_Init();

#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_POM_Init();
#endif

#if (SYS_CPNT_AAA == TRUE)
    AAA_PMGR_Init();
    AAA_POM_Init();
#endif

#if (SYS_CPNT_DOS == TRUE)
    DOS_PMGR_InitiateProcessResources();
#endif

    USERAUTH_PMGR_Init();

#if(SYS_CPNT_MGMT_IP_FLT==TRUE)
    MGMT_IP_FLT_Init();
#endif

#if (SYS_CPNT_TACACS == TRUE )
     TACACS_PMGR_Init();
     TACACS_POM_Init();
#endif

#if (SYS_CPNT_RADIUS == TRUE )
     RADIUS_PMGR_InitiateProcessResources();
     RADIUS_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PMGR_InitiateProcessResources();
#endif

/*
#if (SYS_CPNT_HTTP == TRUE)
    if(HTTP_INIT_InitiateProcessResource()==FALSE)
        return FALSE;
#endif
*/
#if (SYS_CPNT_WEBAUTH==TRUE)
    WEBAUTH_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_SNTP == TRUE)
    SNTP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE )
    EFM_OAM_PMGR_InitiateProcessResource();
    EFM_OAM_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_POE==TRUE)
    if(POE_PMGR_InitiateProcessResources()==FALSE)
        return FALSE;
    if(POE_POM_InitiateProcessResources()==FALSE)
        return FALSE;
#endif

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_PMGR_InitiateProcessResources();
    MLDSNP_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_DNS == TRUE)
    DNS_PMGR_InitiateProcessResource();
    DNS_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_PMGR_InitiateProcessResources();
    TRACEROUTE_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_ROUTING == TRUE)
#if (SYS_CPNT_OSPF == TRUE)
    OSPF_PMGR_InitiateProcessResource();
#endif /* #if (SYS_CPNT_OSPF == TRUE) */
#endif /* #if (SYS_CPNT_ROUTING == TRUE) */

#if (SYS_CPNT_OSPF6 == TRUE)
    OSPF6_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_IPAL == TRUE)
    IPAL_Kernel_Init();
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
    PFC_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_BGP == TRUE)
    BGP_PMGR_InitiateProcessResource();
    BGP_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_PBR == TRUE)
    NETCFG_PMGR_PBR_InitiateProcessResources();
    NETCFG_POM_PBR_InitiateProcessResources();
#endif

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Relashionships between CSCs are created through function
 *    pointer registrations and callback functions.
 *    At this init phase, all operations related to function pointer
 *    registrations will be processed.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void WEB_PROC_Create_InterCSC_Relation(void)
{
    WEB_GROUP_Create_InterCSC_Relation();
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in WEB process will be spawned in this function.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void WEB_PROC_Create_All_Threads(void)
{
    WEBGROUP_Create_All_Threads();

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for WEB process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    The main thread of the process shall never be terminated before any other
 *    threads spawned in this process. If the main thread terminates, other
 *    threads in the same process will also be terminated.
 *
 *    The main thread is responsible for handling OM IPC request for read
 *    operations of All OMs in this process.
 *------------------------------------------------------------------------------
 */
static void WEB_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp=0;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_WEB_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* wait for the request ipc message
         */
        if(SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER, POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p)==SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch(msgbuf_p->cmd)
            {
#if (SYS_CPNT_HTTP == TRUE)
                case SYS_MODULE_HTTP:
/*                     need_resp = HTTP_OM_HandleIPCReqMsg(msgbuf_p);*/
                    break;
#endif
#if (SYS_CPNT_WEBAUTH == TRUE)
                case SYS_MODULE_WEBAUTH:
/*                    need_resp = WEBAUTH_OM_HandleIPCReqMsg(msgbuf_p);*/
                    break;
#endif
		case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("%s: Invalid IPC req cmd.\n", __FUNCTION__);
                    /* Unknow command. There is no way to idntify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp=FALSE;
            }

            if((need_resp==TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle, msgbuf_p)!=SYSFUN_OK))
                printf("%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\n", __FUNCTION__);
        }
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : WEB_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    After the process has been daemonized, the main thread of the process
 *    will call this function to start the main thread.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
static void WEB_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    WEB_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    WEB_PROC_Main_Thread_Function_Entry();

}

