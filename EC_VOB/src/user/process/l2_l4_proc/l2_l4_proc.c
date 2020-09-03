/* MODULE NAME: L2_L4_PROC.C
 *
 * PURPOSE:
 *    This module implements the functionality of L2_L4 process.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/05/03     --- Timon, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <stdio.h>
#include "sys_bld.h"
#include "sys_module.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#if (SYS_CPNT_NICDRV == TRUE)
#include "dev_nicdrv_pmgr.h"
#endif
#include "gvrp_group.h"
#include "l_cmnlib_init.h"
#include "l2_l4_proc_comm.h"
#include "l2mux_group.h"
#include "l4_group.h"
#include "netaccess_group.h"
#include "dev_amtrdrv_pmgr.h"
#include "dev_nmtrdrv_pmgr.h"
#include "dev_rm_pmgr.h"
#include "dev_swdrv_pmgr.h"
#include "dev_swdrvl3_pmgr.h"
#include "dev_swdrvl4_pmgr.h"
#if (SYS_CPNT_SNMP == TRUE)
#include "snmp_pmgr.h"
#endif
#include "cmgr_group.h"
#include "swctrl_group.h"
#if (SYS_CPNT_SYSLOG == TRUE)
#include "syslog_pmgr.h"
#include "syslog_pom.h"
#endif
#include "sysrsc_mgr.h"

#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_om.h"
#include "1x_pom.h"
#endif

#if (SYS_CPNT_L2MCAST == TRUE)
#include "l2mcast_group.h"
#endif

#if (SYS_CPNT_LACP == TRUE)
#include "lacp_group.h"
#include "lacp_om.h"
#endif

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_om.h"
#include "lldp_pmgr.h"
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
#include "swctrl.h"
#include "swctrl_pmgr.h"
#include "swctrl_pom.h"
#endif

#if (SYS_CPNT_VLAN == TRUE)
#include "vlan_om.h"
#include "vlan_pmgr.h"
#endif

#if (SYS_CPNT_BRIDGE == TRUE)
#include "sta_group.h"
#include "xstp_om.h"
#include "xstp_pmgr.h"
#include "xstp_pom.h"
#endif
#include "extbrg_pmgr.h"

#if (SYS_CPNT_AMTR == TRUE)
#include "amtr_om.h"
#include "amtr_pmgr.h"
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
#include "amtrl3_group.h"
#include "amtrl3_om.h"
#include "amtrl3_pmgr.h"
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
#include "udphelper_group.h"
#include "udphelper_om.h"
#include "udphelper_pmgr.h"
#endif

#if (SYS_CPNT_DAI == TRUE)
#include "dai_group.h"
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
#include "oam_group.h"
#include "oam_om.h"
#include "oam_pmgr.h"
#endif

#include "netcfg_pmgr_main.h"
#include "netcfg_pmgr_route.h"
#include "netcfg_pmgr_ip.h"
#include "netcfg_pom_main.h"

#if ((SYS_CPNT_DHCPSNP == TRUE) || (SYS_CPNT_DHCPV6SNP == TRUE))
#include "dhcpsnp_group.h"
#endif

#if (SYS_CPNT_DHCPSNP == TRUE)
#include "dhcpsnp_pmgr.h"
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
#include "dhcpv6snp_pmgr.h"
#include "dhcpv6snp_pom.h"
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#include "ipsg_pmgr.h"
#include "ipsg_pom.h"
#endif

#if(SYS_CPNT_CFM == TRUE)
#include "cfm_group.h"
#include "cfm_mgr.h"
#endif

#if (SYS_CPNT_POE == TRUE)
#include "poe_pom.h"
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
#include "dhcp_pom.h"
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
#include "pppoe_ia_group.h"
#endif

#include "l_mm_backdoor.h"
#if (SYS_CPNT_NDSNP == TRUE)
#include "ndsnp_pmgr.h"
#include "ndsnp_pom.h"
#include "ndsnp_om.h"
#endif

#include "uc_mgr.h"

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
#include "ip6sg_om.h"
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

#include "dcb_group.h"

#if (SYS_CPNT_RADIUS == TRUE)
#include "radius_pmgr.h"
#include "radius_pom.h"
#endif

#if (SYS_CPNT_MLAG == TRUE)
#include "mlag_group.h"
#include "mlag_om.h"
#include "mlag_pom.h"
#endif

#if (SYS_CPNT_IPAL == TRUE)
#include "ipal_kernel.h"
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_pmgr.h"
#include "vxlan_group.h"
#include "vxlan_om.h"
#endif

#if (SYS_CPNT_LACP == TRUE)
#include "lacp_pmgr.h"
#include "lacp_pom.h"
#endif

#if (SYS_CPNT_NETACCESS == TRUE)
#include "netaccess_pmgr.h"
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
#include "psec_pom.h"
#include "psec_pmgr.h"
#include "psec_om.h"
#endif

#if(SYS_CPNT_MLDSNP == TRUE)
#include "mldsnp_pmgr.h"
#include "mldsnp_pom.h"
#endif

#if (SYS_CPNT_DHCPV6==TRUE)
#include "dhcpv6_pmgr.h"
#include "dhcpv6_pom.h"
#endif

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_pmgr.h"
#endif

#if (SYS_CPNT_CFM == TRUE)
#include "cfm_pmgr.h"
#endif

#include "if_pmgr.h"
#include "iml_pmgr.h"
#include "l2mux_pmgr.h"
#include "l4_pmgr.h"
#include "mib2_pmgr.h"
#include "mib2_pom.h"
#include "nmtr_pmgr.h"
#include "pri_pmgr.h"
#include "radius_pmgr.h"
#include "radius_pom.h"
#include "stkctrl_pmgr.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"
#include "telnet_pmgr.h"
#include "trk_pmgr.h"

#if (SYS_CPNT_NSM == TRUE)
#include "nsm_pmgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* the size of the ipc msg buf for om thread to receive and response POM ipc
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 * (this size does not include IPC msg header)
 */
#define POM_MSGBUF_MAX_REQUIRED_SIZE sizeof(L2_L4_PROC_OmMsg_T)


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */

typedef union
{
#if (SYS_CPNT_LACP == TRUE)
    LACP_OM_IpcMsg_T lacp_om_ipcmsg;
#endif

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_OM_IpcMsg_T lldp_om_ipcmsg;
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
    SWCTRL_OM_IPCMsg_T swctrl_om_ipcmsg;
#endif

#if (SYS_CPNT_VLAN == TRUE)
    VLAN_OM_IpcMsg_T vlan_om_ipcmsg;
#endif

#if (SYS_CPNT_BRIDGE == TRUE)
    XSTP_OM_IpcMsg_T xstp_om_ipcmsg;
#endif

#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_OM_IPCMsg_T dot1x_om_ipcmsg;
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_OM_IPCMsg_T amtrl3_om_ipcmsg;
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    EFM_OAM_OM_IPCMsg_T efm_oam_om_ipcmsg;
#endif

#if (SYS_CPNT_CFM == TRUE)
    CFM_MGR_IPCMsg_T cfm_mgr_ipcmsg;
#endif

#if (SYS_CPNT_AMTR == TRUE)
    AMTR_OM_IpcMsg_T amtr_om_ipcmsg;
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    IPSG_OM_IPCMsg_T ipsg_om_ipcmsg;
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_OM_IpcMsg_T  vxlan_om_ipcmsg;
#endif

    L_MM_BACKDOOR_IpcMsg_T l_mm_backdoor_ipcmsg;
} L2_L4_PROC_OmMsg_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static BOOL_T L2_L4_PROC_Init(void);
static BOOL_T L2_L4_PROC_InitiateProcessResources(void);
static void   L2_L4_PROC_Create_InterCSC_Relation(void);
static void   L2_L4_PROC_Daemonize_Entry(void* arg);
static void   L2_L4_PROC_Create_All_Threads(void);
static void   L2_L4_PROC_Main_Thread_Function_Entry(void);


/* STATIC VARIABLE DEFINITIONS
 */

/* the buffer for retrieving ipc request for om thread
 * The size of this buffer should pick the maximum of size required for POM ipc
 * request and response in all CSCs handled in this process.
 */
static UI8_T omtd_ipc_buf[SYSFUN_SIZE_OF_MSG(POM_MSGBUF_MAX_REQUIRED_SIZE)];


/* EXPORTED SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - main
 *-----------------------------------------------------------------------------
 * PURPOSE : The main entry for L2_L4 process
 *
 * INPUT   : argc --  the size of the argv array
 *           argv --  the array of arguments
 *
 * OUTPUT  : None.
 *
 * RETURN  : 0  -- Success
 *           -1 -- Error
 * NOTES   : This function is the entry point for L2_L4 process.
 *-----------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    /* Initialize the resource of L2_L4 process
     */
    if (L2_L4_PROC_Init() == FALSE)
    {
        printf("\nL2_L4_PROC_Process_Init fail.\n");
        return -1;
    }

    /* Spawn the L2_L4 process
     */
    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_L2_L4_PROCESS_NAME,
                            L2_L4_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("\nSYSFUN_SpawnProcess(L2_L4 process) error.\n");
        return -1;
    }

    return 0;
}


/* LOCAL SUBPROGRAM BODIES
 */

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_Init
 *-----------------------------------------------------------------------------
 * PURPOSE : Do initialization procedures for L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T L2_L4_PROC_Init(void)
{
    /* In L2_L4_PROC_Init(), following operations shall be
     * performed if necessary.
     * (a) attach system resource through SYSRSC
     * (b) process common resource init
     * (c) OM init
     * (d) process specific resource init (Initiate IPC facility)
     */

    L_CMNLIB_INIT_InitiateProcessResources();

    if (SYSRSC_MGR_AttachSystemResources() == FALSE)
    {
        printf("\n%s: SYSRSC_MGR_AttachSystemResource fail\n", __FUNCTION__);
        return FALSE;
    }

    if (L2_L4_PROC_InitiateProcessResources() == FALSE)
    {
        return FALSE;
    }

    /* Register callback functions
     */
    L2_L4_PROC_Create_InterCSC_Relation();

    /* Init PMGR
     */
#if (SYS_CPNT_NSM == TRUE)
    NSM_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_NICDRV == TRUE)
    DEV_NICDRV_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_SNMP == TRUE)
    SNMP_PMGR_Init();
#endif

#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_AAA == TRUE)
    AAA_PMGR_Init();
    AAA_POM_Init();
#endif
    AMTR_PMGR_InitiateProcessResource();
    //CLI_PMGR_InitiateProcessResource();
    //CLI_POM_InitiateProcessResource();
    //CLUSTER_PMGR_Init();
    //CLUSTER_POM_Init();
    //DBSYNC_TXT_PMGR_InitiateProcessResource();
    DEV_AMTRDRV_PMGR_InitiateProcessResource();
    DEV_NMTRDRV_PMGR_InitiateProcessResource();
    DEV_SWDRV_PMGR_InitiateProcessResource();
    DEV_SWDRVL3_PMGR_InitiateProcessResource();
    DEV_SWDRVL4_PMGR_InitiateProcessResource();
    DEVRM_PMGR_InitiateProcessResource();

    //DOT1X_PMGR_Init();
    /*EPR:NULL
     *Problem: When send LLDP packet to DUT,DUT will printf lots of dot1x information.
     *Solution: LLDP mgr call dot1x pom function but have not get the msg queue,so cause problem.When init LLDP ,get the msg queue handle
     *Fixed by:DanXie
     */
#if (SYS_CPNT_DOT1X == TRUE)
    DOT1X_POM_Init();
#endif
    EXTBRG_PMGR_InitiateProcessResource();
    IF_PMGR_Init();
    IML_PMGR_InitiateProcessResource();
    L2MUX_PMGR_InitiateProcessResource();
    L4_PMGR_Init();
#if (SYS_CPNT_LACP == TRUE)
    LACP_PMGR_InitiateProcessResource();
    LACP_POM_InitiateProcessResource();
#endif
#if (SYS_CPNT_LLDP == TRUE)
    LLDP_PMGR_InitiateProcessResources();
#endif
#if (SYS_CPNT_DCBX == TRUE)
    DCBX_PMGR_InitiateProcessResources();
#endif
#if (SYS_CPNT_PFC == TRUE)
    PFC_PMGR_InitiateProcessResource();
#endif
#if (SYS_CPNT_ETS == TRUE)
    ETS_PMGR_InitiateProcessResource();
#endif
    //MGMT_IP_FLT_PMGR_Init();
    MIB2_PMGR_Init();
    MIB2_POM_Init();
#if (SYS_CPNT_NETACCESS == TRUE)
    NETACCESS_PMGR_InitiateProcessResources();
#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
    NETCFG_PMGR_MAIN_InitiateProcessResource();
    NETCFG_POM_MAIN_InitiateProcessResource();
    NMTR_PMGR_InitiateProcessResource();
    //NSM_PMGR_InitiateProcessResource();
#if (SYS_CPNT_POE == TRUE)
    POE_PMGR_InitiateProcessResources();
    POE_POM_InitiateProcessResources();
#endif
    PRI_PMGR_InitiateProcessResources();
#if (SYS_CPNT_PORT_SECURITY == TRUE)
    PSEC_POM_InitiateProcessResource();
    PSEC_PMGR_InitiateProcessResources();
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */
    RADIUS_PMGR_InitiateProcessResources();
    RADIUS_POM_InitiateProcessResources();
    STKCTRL_PMGR_InitiateProcessResources();
    STKTPLG_PMGR_InitiateProcessResources();
    STKTPLG_POM_InitiateProcessResources();
    SWCTRL_POM_Init();
    SWCTRL_PMGR_Init();
    //SWCTRL_POM_Init();
    SYSLOG_POM_InitiateProcessResource();
    //TACACS_PMGR_Init();
    //TACACS_POM_Init();
    TELNET_PMGR_InitiateProcessResource();
    //TELNET_POM_InitiateProcessResource();
    TRK_PMGR_InitiateProcessResource();
    //USERAUTH_PMGR_Init();
    VLAN_PMGR_InitiateProcessResource();
    XSTP_PMGR_InitiateProcessResource();
    XSTP_POM_InitiateProcessResource();

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_PMGR_InitiateProcessResource();
#endif

#if(SYS_CPNT_MLDSNP == TRUE)
    MLDSNP_PMGR_InitiateProcessResources();
    MLDSNP_POM_InitiateProcessResources();
#endif
#if (SYS_CPNT_DHCPSNP == TRUE)
    DHCPSNP_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
    IPSG_PMGR_InitiateProcessResource();
    IPSG_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
    DHCPV6SNP_PMGR_InitiateProcessResource();
    DHCPV6SNP_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCPV6 == TRUE)
    DHCPv6_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_DHCP_RELAY_OPTION82 == TRUE)
    DHCP_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_RSPAN == TRUE)
    RSPAN_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_DHCPV6_RELAY == TRUE)
    DHCPv6_POM_InitiateProcessResource();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    EFM_OAM_PMGR_InitiateProcessResource();
#endif

#if (SYS_CPNT_NDSNP == TRUE)
    NDSNP_POM_InitiateProcessResource();
#endif

#if(SYS_CPNT_CFM == TRUE)
    CFM_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_CN == TRUE)
    CN_PMGR_InitiateProcessResources();
#endif

#if (SYS_CPNT_MLAG == TRUE)
    MLAG_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_IPAL == TRUE)
    IPAL_Kernel_Init();
#endif /* #if (SYS_CPNT_IPAL == TRUE) */

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_PMGR_InitiateProcessResources();
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_InitiateProcessResources
 *-----------------------------------------------------------------------------
 * PURPOSE : Initialize the resource used in L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : TRUE  -- Success
 *           FALSE -- Fail
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static BOOL_T L2_L4_PROC_InitiateProcessResources(void)
{
    /* Init common resource
     */
    if (L2_L4_PROC_COMM_InitiateProcessResources() == FALSE)
    {
        return FALSE;
    }

    /* Init process resources for each member CSC group
     */
    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
       return FALSE;
    }

    CMGR_GROUP_InitiateProcessResources();

    GVRP_GROUP_InitiateProcessResources();

#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_GROUP_InitiateProcessResource();
#endif

    L2MUX_GROUP_InitiateProcessResource();

#if (SYS_CPNT_LACP == TRUE)
    LACP_GROUP_InitiateProcessResources();
#endif

    NETACCESS_GROUP_InitiateProcessResources();

#if (SYS_CPNT_BRIDGE == TRUE)
    STA_GROUP_InitiateProcessResources();
#endif

    SWCTRL_GROUP_InitiateProcessResource();

#if (SYS_CPNT_L4 == TRUE)
    L4_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
    UDPHELPER_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_DAI == TRUE)
    DAI_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    OAM_GROUP_InitiateProcessResources();
#endif


#if ((SYS_CPNT_DHCPSNP == TRUE) || (SYS_CPNT_DHCPV6SNP == TRUE))
    DHCPSNP_GROUP_InitiateProcessResources();
#endif
#if (SYS_CPNT_CFM == TRUE)
   CFM_GROUP_InitiateProcessResource();
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
    PPPOE_IA_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
    DCB_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_RADIUS == TRUE )
    RADIUS_PMGR_InitiateProcessResources();
    RADIUS_POM_InitiateProcessResources();
#endif

#if (SYS_CPNT_MLAG == TRUE)
    MLAG_GROUP_InitiateProcessResources();
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_GROUP_InitiateProcessResources();
#endif

    return TRUE;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE : Relationships between CSCs are created through function
 *           pointer registrations and callback functions.
 *           At this init phase, all operations related to function pointer
 *           registrations will be processed.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void L2_L4_PROC_Create_InterCSC_Relation(void)
{
    L_CMNLIB_INIT_Create_InterCSC_Relation();

    CMGR_GROUP_Create_InterCSC_Relation();

    GVRP_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_GROUP_Create_InterCSC_Relation();
#endif

    L2MUX_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_LACP == TRUE)
    LACP_GROUP_Create_InterCSC_Relation();
#endif

    NETACCESS_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_BRIDGE == TRUE)
    STA_GROUP_Create_InterCSC_Relation();
#endif

    SWCTRL_GROUP_Create_InterCSC_Relation();

#if (SYS_CPNT_L4 == TRUE)
    L4_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
    UDPHELPER_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_DAI == TRUE)
    DAI_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    OAM_GROUP_Create_InterCSC_Relation();
#endif


#if ((SYS_CPNT_DHCPSNP == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE))
    DHCPSNP_GROUP_Create_InterCSC_Relation();
#endif
#if (SYS_CPNT_CFM == TRUE)
    CFM_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
    PPPOE_IA_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
    DCB_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_MLAG == TRUE)
    MLAG_GROUP_Create_InterCSC_Relation();
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_GROUP_Create_InterCSC_Relation();
#endif

    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE : After the process has been daemonized, the main thread of the
 *           process will call this function to start the main thread.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *------------------------------------------------------------------------------
 */
static void L2_L4_PROC_Daemonize_Entry(void* arg)
{
    /* create all threads in this process
     * The thread group membership of each thread shall be set in the
     * beginning of the function entry of each thread.
     */
    L2_L4_PROC_Create_All_Threads();

    /* Enter main thread function entry.
     * In main thread function entry, it shall take the responsibility
     * of handling OM IPC messages for this process
     */
    L2_L4_PROC_Main_Thread_Function_Entry();

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_Create_All_Threads
 *-----------------------------------------------------------------------------
 * PURPOSE : Spawn all threads in L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
static void L2_L4_PROC_Create_All_Threads(void)
{
    CMGR_GROUP_Create_All_Threads();

    L2MUX_GROUP_Create_All_Threads();

    SWCTRL_GROUP_Create_All_Threads();

#if (SYS_CPNT_LACP == TRUE)
    LACP_GROUP_Create_All_Threads();
#endif

    GVRP_GROUP_Create_All_Threads();

    NETACCESS_GROUP_Create_All_Threads();

#if (SYS_CPNT_BRIDGE == TRUE)
    STA_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_L4 == TRUE)
    L4_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_L2MCAST == TRUE)
    L2MCAST_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
    AMTRL3_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_UDP_HELPER == TRUE)
    UDPHELPER_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_DAI == TRUE)
    DAI_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
    OAM_GROUP_Create_All_Threads();
#endif

#if ((SYS_CPNT_DHCPSNP == TRUE)||(SYS_CPNT_DHCPV6SNP == TRUE)||(SYS_CPNT_NDSNP == TRUE))
    DHCPSNP_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_CFM == TRUE)
   CFM_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_PPPOE_IA == TRUE)
    PPPOE_IA_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_DCB_GROUP == TRUE)
    DCB_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_MLAG == TRUE)
    MLAG_GROUP_Create_All_Threads();
#endif

#if (SYS_CPNT_VXLAN == TRUE)
    VXLAN_GROUP_Create_All_Threads();
#endif

    return;
}

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - L2_L4_PROC_Main_Thread_Function_Entry
 *-----------------------------------------------------------------------------
 * PURPOSE : The entry function of the main thread for L2_L4 process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : 1. The main thread of the process shall never be terminated before
 *              any other threads spawned in this process. If the main thread
 *              terminates, other threads in the same process will also be
 *              terminated.
 *           2. The main thread is responsible for handling OM IPC request for
 *              read operations of All OMs in this process.
 *-----------------------------------------------------------------------------
 */
static void L2_L4_PROC_Main_Thread_Function_Entry(void)
{
    SYSFUN_MsgQ_T ipc_msgq_handle;
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)omtd_ipc_buf;
    BOOL_T       need_resp;

    /* Main thread will handle OM IPC request
     * raise main thread priority to the priority of the raising high priority
     * setting in SYSFUN_OM_ENTER_CRITICAL_SECTION()
     */
    if (SYSFUN_SetTaskPriority(SYS_BLD_OM_THREAD_SCHED_POLICY,
            SYSFUN_TaskIdSelf(), SYS_BLD_OM_THREAD_PRIORITY) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);
        return;
    }

    /* create om ipc message queue handle
     */
    if (SYSFUN_CreateMsgQ(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,
            SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_msgq_handle) != SYSFUN_OK)
    {
        printf("\n%s: SYSFUN_CreateMsgQ fail.\n", __FUNCTION__);
        return;
    }

    while (1)
    {
        /* wait for the request ipc message
         */
        if (SYSFUN_ReceiveMsg(ipc_msgq_handle, 0, SYSFUN_TIMEOUT_WAIT_FOREVER,
                POM_MSGBUF_MAX_REQUIRED_SIZE, msgbuf_p) == SYSFUN_OK)
        {
            /* handle request message based on cmd
             */
            switch (msgbuf_p->cmd)
            {
#if (SYS_CPNT_LACP == TRUE)
                case SYS_MODULE_LACP:
                    need_resp = LACP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_LLDP == TRUE)
                case SYS_MODULE_LLDP:
                    need_resp = LLDP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_SWCTRL == TRUE)
                case SYS_MODULE_SWCTRL:
                    need_resp = SWCTRL_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_VLAN == TRUE)
                case SYS_MODULE_VLAN:
                    need_resp = VLAN_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_BRIDGE == TRUE)
                case SYS_MODULE_XSTP:
                    need_resp = XSTP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_DOT1X == TRUE)
                case SYS_MODULE_DOT1X:
                    need_resp = DOT1X_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_AMTRL3 == TRUE)
                case SYS_MODULE_AMTRL3:
                    need_resp = AMTRL3_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_MLDSNP == TRUE)
                case SYS_MODULE_MLDSNP:
                    need_resp = MLDSNP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_EFM_OAM == TRUE)
                case SYS_MODULE_EFM_OAM:
                    need_resp = EFM_OAM_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
#if (SYS_CPNT_CFM == TRUE)
                case SYS_MODULE_CFM:
                    /*here won't be called now, because cfm still not provide pom*/
                    //need_resp = CFM_POM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif
#if (SYS_CPNT_AMTR == TRUE)
                case SYS_MODULE_AMTR:
                    need_resp = AMTR_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_DHCPV6SNP == TRUE)
                case SYS_MODULE_DHCPV6SNP:
                    need_resp = DHCPV6SNP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_NDSNP == TRUE)
                case SYS_MODULE_NDSNP:
                    need_resp = NDSNP_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_IPV6_SOURCE_GUARD == TRUE)
                case SYS_MODULE_IP6SG:
                    need_resp = IP6SG_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
                case SYS_MODULE_IPSG:
                    need_resp = IPSG_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

                case SYS_MODULE_CMNLIB:
                    need_resp = L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    break;

#if (SYS_CPNT_MLAG == TRUE)
                case SYS_MODULE_MLAG:
                    need_resp = MLAG_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif

#if (SYS_CPNT_PORT_SECURITY == TRUE)
                case SYS_MODULE_PSEC:
                    need_resp = PSEC_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif /* #if (SYS_CPNT_PORT_SECURITY == TRUE) */

#if (SYS_CPNT_VXLAN == TRUE)
                case SYS_MODULE_VXLAN:
                    need_resp = VXLAN_OM_HandleIPCReqMsg(msgbuf_p);
                    break;
#endif


                default:
                    printf("\n%s: Invalid IPC req cmd:%d.\n", __FUNCTION__, msgbuf_p->cmd);
                    /* Unknown command. There is no way to identify whether this
                     * ipc message need or not need a response. If we response to
                     * a asynchronous msg, then all following synchronous msg will
                     * get wrong responses and that might not easy to debug.
                     * If we do not response to a synchronous msg, the requester
                     * will be blocked forever. It should be easy to debug that
                     * error.
                     */
                    need_resp = FALSE;
            }

            if ((need_resp == TRUE) && (SYSFUN_SendResponseMsg(ipc_msgq_handle,
                    msgbuf_p) != SYSFUN_OK))
            {
                printf("\n%s: SYSFUN_SendResponseMsg fail.\n", __FUNCTION__);
            }
        }
        else
        {
            SYSFUN_Debug_Printf("\n%s: SYSFUN_ReceiveMsg fail.\n",
                __FUNCTION__);
        }
    }
}
