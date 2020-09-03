/* MODULE NAME:  snmp_proc.c
 * PURPOSE:
 *    This is snmp main process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    5/2/2007 - Eli Lin, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "sysrsc_mgr.h"
#include "l_threadgrp.h"
#include "backdoor_mgr.h"
#include "snmp_proc_comm.h"
#include "l_cmnlib_init.h"
#include "snmp_group.h"

#include "l_mm_backdoor.h"

#include "uc_mgr.h"

#if (SYS_CPNT_SNMP == TRUE)
#include "snmp_init.h"
#include "snmp_task.h"
#include "snmp_mgr.h"
#endif

#include "1x_pmgr.h"

#if (SYS_CPNT_AAA == TRUE)
#include "aaa_pmgr.h"
#include "aaa_pom.h"
#endif

//#include "bdgext_pmgr.h"
#include "cli_pmgr.h"
#if (SYS_CPNT_CLUSTER==TRUE)
#include "cluster_pmgr.h"
#include "cluster_pom.h"
#endif
//#include "dbsync_txt_pmgr.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_nicdrv_pmgr.h"
#include "dev_nmtrdrv_pmgr.h"
//#include "dev_rm_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "dev_swdrvl4_pmgr.h"
#include "dev_swdrv_pmgr.h"
#include "dhcp_pmgr.h"
#include "dhcpsnp_pmgr.h"
#include "extbrg_pmgr.h"
#include "if_pmgr.h"
#include "iml_pmgr.h"
#include "l2mux_pmgr.h"
#include "l4_pmgr.h"
#include "lacp_pmgr.h"
#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#endif
//#include "led_pmgr.h"
#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pmgr.h"
#endif
#include "mgmt_ip_flt.h"
#include "mib2_pmgr.h"
#include "nmtr_pmgr.h"
#include "pri_pmgr.h"
#include "psec_pmgr.h"
#include "radius_pmgr.h"
#include "snmp_pmgr.h"
#include "stkctrl_pmgr.h"
#include "stktplg_pmgr.h"
#include "swctrl_pmgr.h"
#include "syslog_pmgr.h"
//#include "sys_bnr_pmgr.h"
#include "sys_pmgr.h"
#include "tacacs_pmgr.h"
#include "telnet_pmgr.h"
//#include "test.txt"
#include "trk_pmgr.h"
#include "userauth_pmgr.h"
#include "vlan_pmgr.h"
#include "xstp_pmgr.h"
#include "1x_pom.h"
#include "amtr_pmgr.h"
#include "amtr_pom.h"
//#include "backdoor_pom.h"
//#include "bdgext_pom.h"
#include "cli_pom.h"
//#include "dbsync_txt_pom.h"
//#include "dev_amtrdrv_pom.h"
//#include "dev_nicdrv_pom.h"
//#include "dev_nmtrdrv_pom.h"
//#include "dev_rm_pom.h"
//#include "dev_swdrvl3_pom.h"
//#include "dev_swdrvl4_pom.h"
//#include "dev_swdrv_pom.h"
//#include "extbrg_pom.h"
//#include "if_pom.h"
#include "lacp_pom.h"
#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_pom.h"
#endif /* #if (SYS_CPNT_LLDP == TRUE) */
#include "mib2_pom.h"
#include "netcfg_type.h"
//#include "pri_pom.h"
//#include "psec_pom.h"
#include "radius_pom.h"
//#include "snmp_pom.h"
//#include "stkctrl_pom.h"
#include "stktplg_pom.h"
#include "swctrl_pom.h"
#include "syslog_pom.h"
//#include "sys_bnr_pom.h"
//#include "sys_pom.h"
#include "tacacs_pom.h"
#include "telnet_pom.h"
//#include "trk_pom.h"
//#include "userauth_pom.h"
#include "vlan_pom.h"
#include "xstp_pom.h"
#if (SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pom.h"
#include "mldsnp_pmgr.h"
#endif
#include "dns_pom.h"
#include "dns_pmgr.h"

#include "sshd_pmgr.h"
#include "keygen_pmgr.h"
#if (SYS_CPNT_SMTP == TRUE)
#include "smtp_pmgr.h"
#endif
#include "sntp_pmgr.h"
#if (SYS_CPNT_NTP == TRUE)
#include "ntp_pmgr.h"
#endif
#include "netaccess_pmgr.h"
#include "ping_pom.h"
#include "ping_pmgr.h"
#include "traceroute_pom.h"
#include "traceroute_pmgr.h"
#if(SYS_CPNT_RIP == TRUE)
#include "rip_pmgr.h"
#endif
#if(SYS_CPNT_OSPF == TRUE)
#include "ospf_pmgr.h"
#endif
#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#endif
#if (SYS_CPNT_ADD == TRUE)
#include "add_pmgr.h"
#endif

#if(SYS_CPNT_CFM==TRUE)
#include "cfm_pmgr.h"
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
#include "vrrp_pmgr.h"
#include "vrrp_pom.h"
#endif /* SYS_CPNT_VRRP */

#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_pmgr.h"
#endif

#if (SYS_CPNT_WEBAUTH == TRUE)
#include "webauth_pmgr.h"
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
#include "ipal_kernel.h"
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
#include "dai_pmgr.h"
#endif

#if ((SYS_CPNT_HTTP == TRUE) || (SYS_CPNT_HTTPS == TRUE))
#include "http_pmgr.h"
#endif

#if (SYS_CPNT_POE == TRUE)
#include "poe_pmgr.h"
#include "poe_pom.h"
#endif

#if (SYS_CPNT_SFLOW == TRUE)
#include "sflow_pmgr.h"
#endif

#if (SYS_CPNT_DYING_GASP == TRUE)
#include "dying_gasp.h"
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_pmgr.h"
#endif

#if (SYS_CPNT_DOS == TRUE)
#include "dos_pmgr.h"
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
#include "dhcpv6snp_pmgr.h"
#include "dhcpv6snp_pom.h"
#endif

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
#include "ip6sg_pmgr.h"
#include "ip6sg_pom.h"
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "dhcp_pom.h"
#endif

#if (SYS_CPNT_BGP == TRUE)
#include "bgp_pmgr.h"
#endif

#if(SYS_CPNT_SYNCE == TRUE)
#include "sync_e_pmgr.h"
#endif

#if (SYS_CPNT_DCBX == TRUE)
#include "dcbx_pmgr.h"
#endif

#if (SYS_CPNT_CN == TRUE)
#include "cn_pmgr.h"
#endif

#if (SYS_CPNT_ETS == TRUE)
#include "ets_pmgr.h"
#endif

#if (SYS_CPNT_PFC == TRUE)
#include "pfc_pmgr.h"
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
#include "af_pmgr.h"
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE) */

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pmgr.h"
#include "ipsg_pom.h"
#endif

#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_pmgr.h"
#include "vxlan_pom.h"
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_pom.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(SNMP_PROC_OM_Msg_T)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
 /* union all data type used for OM IPC message to get the maximum required
 * ipc message buffer
 */
typedef union SNMP_PROC_OM_Msg_U
{
    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} SNMP_PROC_OM_Msg_T;

/* STATIC VARIABLE DECLARATIONS
 */
/* the buffer for retrieving ipc request for main thread
 * main thread will be responsible for handling all ipc
 * request to all OMs in this process.
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SNMP_PROC_Init(void);
static BOOL_T SNMP_PROC_InitiateProcessResource(void);
static void SNMP_PROC_Create_InterCSC_Relation(void);
static void SNMP_PROC_Create_All_Threads(void);
static void SNMP_PROC_Daemonize_Entry(void* arg);
static void SNMP_PROC_Main_Thread_Function_Entry(void);

/* EXPORTED SUBPROGRAM BODIES
 */
/* XXX steven.jiang for warnings */
BOOL_T CLI_POM_InitiateProcessResource(void);
BOOL_T NETCFG_PMGR_MAIN_InitiateProcessResource(void);
BOOL_T NETCFG_POM_MAIN_InitiateProcessResource(void);
void   TACACS_POM_Init(void);
void   USERAUTH_PMGR_Init(void);
BOOL_T EFM_OAM_PMGR_InitiateProcessResource(void);
BOOL_T EFM_OAM_POM_InitiateProcessResource(void);
void   AAA_POM_Init(void);
void   AAA_PMGR_Init(void);
void   XFER_PMGR_InitiateProcessResource(void);

/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for snmp process
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
 *    This function is the entry point for SNMP process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(SNMP_PROC_Init()==FALSE)
    {
        SYSFUN_Debug_Printf("SNMP_Process_Init fail.\n");
        return -1;
    }

    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_SNMP_PROCESS_NAME,
                            SNMP_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("SNMP_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for SNMP process.
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
static BOOL_T SNMP_PROC_Init(void)
{
    /* In SNMP_PROC_Init(), following operations shall be
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

    if(SNMP_PROC_InitiateProcessResource()==FALSE)
        return FALSE;

    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
       return FALSE;
    }

#if (SYS_CPNT_AMTR == TRUE)
    if(AMTR_PMGR_InitiateProcessResource()==FALSE)
        return FALSE;

    if(AMTR_POM_InitiateProcessResource()==FALSE)
        return FALSE;
#endif

    CLI_PMGR_InitiateProcessResource();
    CLI_POM_InitiateProcessResource();

#if (SYS_CPNT_CLUSTER==TRUE)
    CLUSTER_PMGR_Init();
    CLUSTER_POM_Init();
#endif

    DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NICDRV_PMGR_InitiateProcessResource();
    DEV_NMTRDRV_PMGR_InitiateProcessResource();
    DEV_SWDRV_PMGR_InitiateProcessResource();
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
    DEV_SWDRVL4_PMGR_InitiateProcessResource();
    DHCP_PMGR_InitiateProcessResources();

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCP_POM_InitiateProcessResource();
#endif

#if ((SYS_CPNT_DHCPSNP == TRUE) || (SYS_CPNT_IP_SOURCE_GUARD == TRUE))
    DHCPSNP_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    IPSG_PMGR_InitiateProcessResource();
    IPSG_POM_InitiateProcessResource();
#endif

    EXTBRG_PMGR_InitiateProcessResource();
    IF_PMGR_Init();
    IML_PMGR_InitiateProcessResource();
    L4_PMGR_Init();

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_PMGR_InitiateProcessResources();
    LLDP_POM_InitiateProcessResources();
#endif  /* #if (SYS_CPNT_LLDP == TRUE) */

    //MGMT_IP_FLT_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
    NETCFG_PMGR_MAIN_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();
#if(SYS_CPNT_RIP == TRUE)
    RIP_PMGR_InitiateProcessResource();/*Donny.li for rip 2*/
#endif
#if(SYS_CPNT_OSPF == TRUE)
    OSPF_PMGR_InitiateProcessResource(); /*Donny.li for ospf*/
#endif
#if (SYS_CPNT_NSM == TRUE)
    NSM_PMGR_InitiateProcessResource();
#endif
    NMTR_PMGR_InitiateProcessResource();
    PRI_PMGR_InitiateProcessResources();
    PSEC_PMGR_InitiateProcessResources();
    RADIUS_PMGR_InitiateProcessResources();
    RADIUS_POM_InitiateProcessResources();
    SNMP_PMGR_Init();
    STKCTRL_PMGR_InitiateProcessResources();
    STKTPLG_PMGR_InitiateProcessResources();
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_PMGR_Init();
    SWCTRL_POM_Init();
    SYS_PMGR_InitiateProcessResource();
    SYSLOG_PMGR_InitiateProcessResource();
    SYSLOG_POM_InitiateProcessResource();
    TACACS_PMGR_Init();
    TACACS_POM_Init();
    TELNET_PMGR_InitiateProcessResource();
    TELNET_POM_InitiateProcessResource();
    TRK_PMGR_InitiateProcessResource();
    USERAUTH_PMGR_Init();
    VLAN_PMGR_InitiateProcessResource();
    VLAN_POM_InitiateProcessResource();
    XSTP_PMGR_InitiateProcessResource();
    XSTP_POM_InitiateProcessResource();
/*
  *EPR:ES4827G-FLF-ZZ-00456
  *PROBLEM:cli exception when dot1qconstraintTypeDefault do get next operation
  *RootCouse:because we didn't do init process resource operation in mib process
  *Solution:add such init process resource under snmp_proc.
  *approved by :Hard Sun
  *fixed by:Jinhua Wei
  */

#if (SYS_CPNT_EFM_OAM == TRUE)
    EFM_OAM_PMGR_InitiateProcessResource();
    EFM_OAM_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_POM_Init();
#endif

#if (SYS_CPNT_LACP == TRUE)
    LACP_PMGR_InitiateProcessResource();
    LACP_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    //MGMT_IP_FLT_PMGR_Init();
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_POM_InitiateProcessResources();
    MLDSNP_PMGR_InitiateProcessResources();
#endif /* end of #if (SYS_CPNT_MLDSNP == TRUE) */

#if (SYS_CPNT_DNS == TRUE)
    DNS_PMGR_InitiateProcessResource();
    DNS_POM_InitiateProcessResource();
#endif /* end of #if (SYS_CPNT_DNS == TRUE) */

#if (SYS_CPNT_ACCOUNTING == TRUE)
    AAA_POM_Init();
    AAA_PMGR_Init();
#endif /* #if (SYS_CPNT_ACCOUNTING == TRUE) */

#if (SYS_CPNT_DOS == TRUE)
    DOS_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_SSHD == TRUE || SYS_CPNT_SSH2 == TRUE)
    SSHD_PMGR_InitiateProcessResources();
#if (SYS_CPNT_SSH2 == TRUE)
    KEYGEN_PMGR_InitiateProcessResources();
#endif
#endif

#if (SYS_CPNT_SMTP == TRUE)
    SMTP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_SNTP == TRUE)
    SNTP_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_NTP == TRUE)
    NTP_PMGR_InitiateProcessResources();
#endif

#if(SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_PING == TRUE)
    PING_POM_InitiateProcessResources();
    PING_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_TRACEROUTE == TRUE)
    TRACEROUTE_PMGR_InitiateProcessResources();
    TRACEROUTE_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_ADD == TRUE)
    ADD_PMGR_InitiateProcessResources();
#endif

#if(SYS_CPNT_CFM==TRUE)
    CFM_PMGR_InitiateProcessResources();
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

#if (SYS_CPNT_VRRP == TRUE)
    VRRP_PMGR_InitiateProcessResource();
    VRRP_POM_InitiateProcessResource();
#endif /* SYS_CPNT_VRRP */

#if (SYS_CPNT_UDP_HELPER == TRUE)
    UDPHELPER_PMGR_InitiateProcessResource();
#endif /* #if (SYS_CPNT_UDP_HELPER == TRUE) */

#if (SYS_CPNT_WEBAUTH == TRUE)
    WEBAUTH_PMGR_InitiateProcessResources();
#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

#if (SYS_CPNT_IPAL == TRUE)
    IPAL_Kernel_Init();
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#if (SYS_CPNT_DAI == TRUE)
    DAI_PMGR_InitiateProcessResource();
#endif

#if ((SYS_CPNT_HTTP == TRUE) || (SYS_CPNT_HTTPS == TRUE))
    HTTP_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_POE == TRUE)
    POE_PMGR_InitiateProcessResources();
    POE_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_SFLOW == TRUE)
    SFLOW_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE )
    PPPOE_IA_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_DYING_GASP == TRUE)
    DYING_GASP_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
    DHCPV6SNP_PMGR_InitiateProcessResource();
    DHCPV6SNP_POM_InitiateProcessResource();
#endif
#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
    IP6SG_PMGR_InitiateProcessResource();
    IP6SG_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_BGP == TRUE)
    BGP_PMGR_InitiateProcessResource();
#endif

#if(SYS_CPNT_SYNCE == TRUE)
    SYNC_E_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_DCBX == TRUE)
    if (DCBX_PMGR_InitiateProcessResources() == FALSE)
    {
        return FALSE;
    }
#endif

#if (SYS_CPNT_CN == TRUE)
    if (CN_PMGR_InitiateProcessResources() == FALSE)
    {
    	return FALSE;
    }
#endif

#if (SYS_CPNT_ETS == TRUE)
    ETS_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_PFC == TRUE)
    PFC_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_APP_FILTER == TRUE)
    AF_PMGR_InitiateProcessResources();
#endif /* #if (SYS_CPNT_APP_FILTER == TRUE) */

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_PMGR_InitiateProcessResources();
    VXLAN_POM_InitiateProcessResources();
#endif

    /* Register callback fun
     */
    SNMP_PROC_Create_InterCSC_Relation();
    XFER_PMGR_InitiateProcessResource();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_InitiateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in mgmt process.
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
static BOOL_T SNMP_PROC_InitiateProcessResource(void)
{
    if(SNMP_PROC_COMM_InitiateProcessResource()==FALSE)
        return FALSE;

#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_Initiate_System_Resources();
#endif
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Relashionships between SNMPs are created through function
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
static void SNMP_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

#if (SYS_CPNT_SNMP == TRUE)
    SNMP_INIT_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_DYING_GASP == TRUE)
    DYING_GASP_Create_InterCSC_Relation();
#endif
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_Create_All_Threads
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    All of the threads in SNMP process will be spawned in this function.
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
static void SNMP_PROC_Create_All_Threads(void)
{
#if (SYS_CPNT_DYING_GASP == TRUE)
    DYING_GASP_CreateTask();
#endif

#if (SYS_CPNT_SNMP == TRUE)
    SNMP_GROUP_Create_All_Threads();
#endif
    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    This is the entry function of the main thread for SNMP process.
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
static void SNMP_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p=(SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY, SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY))
        printf("%s: SYSFUN_SetTaskPriority fail.\r\n", __FUNCTION__);

    /* create om ipc message queue handle
     */
    if(SYSFUN_CreateMsgQ(SYS_BLD_SNMP_PROC_OM_IPCMSGQ_KEY, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle)!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_CreateMsgQ fail.\r\n", __FUNCTION__);
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
                case SYS_MODULE_CMNLIB:
		            need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

                default:
                    printf("%s: Invalid IPC req cmd.\r\n", __FUNCTION__);
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
                printf("%s: SYSFUN_SendResponseMsg fail.\r\n", __FUNCTION__);

        }
        else
        {
            SYSFUN_Debug_Printf("%s: SYSFUN_ReceiveMsg fail.\r\n", __FUNCTION__);
        }
    }

}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SNMP_PROC_Daemonize_Entry
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
static void SNMP_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    SNMP_PROC_Create_All_Threads();
    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    SNMP_PROC_Main_Thread_Function_Entry();

}

