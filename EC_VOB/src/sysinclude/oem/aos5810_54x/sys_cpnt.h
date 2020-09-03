#ifndef  SYS_CPNT_H
#define  SYS_CPNT_H


/*----------------------------------------------------------------------------
 * Package: SYS_CPNT.H
 * Purpose: This package defines the support components/Features for the project.
 * Note: The file should only be used by SYS_CPNT_SUPPORT_.c and stkctrl_task.c.
 *  History
 *
 *   Jason Hsue     07/19/2002      new created
 *   Jason Hsue     10/31/2002      1st revised
 *      Modified 802.1X constant definition
 *   Jason Hsue     11/18/2002      2nd revised
 *      1. Add PVLAN relative constants
 *      2. Add traffic segmentation constant
 *      3. Add Web Compression constant
 *      4. Add watch dog timer and Bstorm lport support constants
 *      5. Add MAU MIB and Protocol VLAN constants
 *   Zhong QY       12-08/2003
 *      1. Synchronisation from ES4649-32 (Hagrid)
 *   wuli           06-15/2004
 *      #define SYS_CPNT_DIFFSERV                   TRUE
 *   wuli    07/30/2004
 *     Diffserv and marker is conflict, change old constant to following
 *     to avoid both set to true
 *      #define SYS_CPNT_QOS_NONE                       0
 *      #define SYS_CPNT_QOS_MARKER                     1
 *      #define SYS_CPNT_QOS_DIFFSERV                   2
 *   Zhong QY       04-30/2007
 *      Moved to be based on ACPv3/main/ES4649.
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             Accton Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* Defined as TRUE for supporting ONIE(Open Network Install Environment).
 */
#define SYS_CPNT_ONIE_SUPPORT TRUE
#define SYS_CPNT_ONIE_BOOT_MODE_SET FALSE

/* define runtime openD_CACHE or not?
 * If runtime not support D_CACHE, we will disable D_CACHE after CLI provision complete.
 * By default, BSP will be always enabled D cache, but after provision complete, project leader
 * can make the decision if he wants to enable or disable D cache during run time based on
 * the fact that if D Cache enable will cause system running abnormal
 */
#define SYS_CPNT_RUNTIME_D_CACHE            TRUE

/* Definition watch dog timer is supported
 */
#define SYS_CPNT_WATCHDOG_TIMER             FALSE

/* Definition for software watchdog timer is supported
 */
#define SYS_CPNT_SW_WATCHDOG_TIMER          TRUE

/* Define main components this project supports
 * Note: Project Leader needs to review these components supported or not
 *       based on product spec.
 */

/* This defines whether "SYSFUN_Debug_Printf" calls "printf" do display
   debug messages in the local console.
 */
#define SYS_CPNT_PRINTF                     FALSE


/* no need this cpnt
 *#define SYS_CPNT_TWOCHIPSTACKINGINONEBOARD                          TRUE
 */
#define SYS_CPNT_TWOCHIPSTACKINGINONEBOARD_E2EFLOWCONTROL           FALSE

/* Turn into debug mode, will show debug message in console
 * must set to FALSE when release runtime image
 */
#define SYS_CPNT_DEBUG                      TRUE
#define SYS_CPNT_DEBUG_XSTP                 TRUE
#define SYS_CPNT_DEBUG_LACP                 TRUE
#define SYS_CPNT_DEBUG_ARP                  TRUE
#define SYS_CPNT_DEBUG_DHCP                 TRUE
#define SYS_CPNT_DEBUG_DHCPSNP              FALSE

/* Definition stackable or not */
#define SYS_CPNT_STACKING                   TRUE

#define SYS_CPNT_BCM_LINKSCAN_ENABLE        TRUE
#define SYS_CPNT_BCM_LINKSCAN_UPDATE_TIME          250000

#define SYS_CPNT_REFINE_ISC_MSG             FALSE

/* Added by fen.wang,2008-8-18,when sender get callback so quickly that receiver
 *  cannot process the msg in time,it will cause the msg queue full,and lost msg
 *  so we need to refine it
 */
#define SYS_CPNT_REFINE_IPC_MSG             FALSE

#define SYS_CPNT_STACKING_BUTTON            FALSE /*eric*/
#define SYS_CPNT_STACKING_BUTTON_SOFTWARE   FALSE
/* TRUE:  Support module hot swap feature
 * FALSE: 1. If module is hot swap inserted, it is ignored and not enabled.
 *        2. If it is hot swap removed, the master console will print a message
 *            and ask user to reboot device. Otherwise it will lead to unexpected behavior.
 */
#define SYS_CPNT_EXPANSION_MODULE_HOT_SWAP  FALSE

/* TRUE:  Support Unit Hot Swap - Resilient Stacking
 * FALSE: Adding or removing units will result in restacking and disturbance in traffic
 */
#define SYS_CPNT_UNIT_HOT_SWAP              TRUE
#define SYS_CPNT_DEBUG_STK_PERFORMANCE             TRUE

#define SYS_CPNT_STKTPLG_SHMEM              TRUE

/* Driver layer */
#define SYS_CPNT_SWDRV                      TRUE
#define SYS_CPNT_NICDRV                     TRUE
#define SYS_CPNT_MGMT_PORT                  FALSE
#define SYS_CPNT_FLASHDRV                   TRUE
#define SYS_CPNT_FLASH_AUTO_DETECTION       TRUE

/* If a craft port (a.k.a. management port) exists in the device,
 * SYS_CPNT_CRAFT_PORT shall be defined as TRUE.
 * And if the mac of the craft port is indepenedt of the front port mac
 * (such as using built-in mac in CPU ASIC), SYS_CPNT_CRAFT_PORT_MODE
 * shall be defined as SYS_CPNT_CRAFT_PORT_MODE_INDEPENDENT_CRAFT_PORT
 * If one of the front port is used as a craft port(that is, front ports
 * and crafts ports uses the same mac), SYS_CPNT_CRAFT_PORT_MODE
 * shall be defined as SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT
 */
#define SYS_CPNT_CRAFT_PORT_MODE_INDEPENDENT_CRAFT_PORT 1
#define SYS_CPNT_CRAFT_PORT_MODE_FRONT_PORT_CRAFT_PORT  2
#define SYS_CPNT_CRAFT_PORT                 TRUE
#define SYS_CPNT_CRAFT_PORT_MODE            SYS_CPNT_CRAFT_PORT_MODE_INDEPENDENT_CRAFT_PORT
#define SYS_CPNT_CRAFT_DHCLIENT             TRUE    /* craft port supports DHCP using 3rd party dhclient process or not */

#if (SYS_CPNT_MGMT_PORT == TRUE)
#define SYS_CPNT_MGMT_PORT_SHOW_IN_WEB      TRUE
#endif

/* Definition for the FLASH memory protection mechanism using the chipset lockbit.
 * SYS_CPNT_FLASH_LOCKBIT_PROTECTION_NONE       : no lockbit protection.
 * SYS_CPNT_FLASH_LOCKBIT_PROTECTION_SYSDATA    : using lockbit to protect the system data (H/W information and loader)
 * SYS_CPNT_FLASH_LOCKBIT_PROTECTION_WHOLE      : using lockbit to protect the whole flash chip.
 */
#define SYS_CPNT_FLASH_LOCKBIT_PROTECTION_NONE          0
#define SYS_CPNT_FLASH_LOCKBIT_PROTECTION_SYSDATA       1
#define SYS_CPNT_FLASH_LOCKBIT_PROTECTION_WHOLE         2

#define SYS_CPNT_FLASH_LOCKBIT_PROTECTION               SYS_CPNT_FLASH_LOCKBIT_PROTECTION_WHOLE

/* For the backward compatibility: if the next-boot runtime image does not support
 * the lockbit protection (i.e., its version is older than the first version defined)
 * then unlock all the lockbit. Zero is assigned to the 1st version if we never roll
 * back by clearing the lockbit for compatibility.
 */
#define SYS_CPNT_FLASH_LOCKBIT_PROTECTION_1ST_VERSION   0x00000000 /* ex.0x02030306 => version no = 2.3.3.6 */


#define SYS_CPNT_LEDDRV                     TRUE
#define SYS_CPNT_VDSLDRV                    FALSE
#define SYS_CPNT_ISCDRV                     TRUE
#define SYS_CPNT_SYSDRV                     TRUE
#if (SYS_CPNT_SYSDRV == TRUE)
#define SYS_CPNT_SYSDRV_USE_ONLP TRUE
#define SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC TRUE
#define SYS_CPNT_SYSDRV_FAN_FAULT_NEED_CHECK_FAN_SPEED TRUE
#define SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL FALSE

#if 0 /* obsoleted by SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE as TRUE */
#define SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL_WITH_FAN_TRANSITION_STATE 1
#define SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL                           2
#define SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP                                                    3
#define SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP
#else

/* SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE
 *     TRUE : The fan speed mode state machine can be set according to different
 *            board ids.
 *     FALSE: Only allow to use one type of fan speed mode state machine for all
 *            board ids.
 */
#define SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE FALSE
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
#define SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP     TRUE
#define SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG     TRUE
#endif

#endif /* end of #if 0 */

#define SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1        1
#define SYS_CPNT_SYSDRV_POWER_SM_TYPE_RPS_ADVANCED_CONTROL 2
#define SYS_CPNT_SYSDRV_POWER_SM_TYPE SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1
#define SYS_CPNT_SYSDRV_POWER_SM_HAS_ALERT_STATE TRUE

#define SYS_CPNT_SYSDRV_POWER_CHECK_MAIN_POWER_PRESENT_BEFORE_CHECK_FAIL TRUE
#endif
#define SYS_CPNT_I2CDRV                     TRUE
#if (SYS_CPNT_I2CDRV == TRUE)
/* SYS_CPNT_I2CDRV_IOCTL_USE_I2C_SLAVE_FORCE
 *     In newer version of linux kernel, it needs to use I2C_SLAVE_FORCE ioctl
 *     command to access the I2C device that had registered a corresponding
 *     i2c device driver in linux kernel.
 *     TRUE  - Use I2C_SLAVE_FORCE ioctl command to access I2C device
 *     FALSE - Use I2C_SLAVE ioctl command to access I2C device
 */
#define SYS_CPNT_I2CDRV_IOCTL_USE_I2C_SLAVE_FORCE TRUE
#endif


/* Apparently that a module with CPU or without CPU will have different design.
 * This constant will differentiate the module with or without CPU.
 */
#define SYS_CPNT_MODULE_WITH_CPU            FALSE

/* Layer 3 driver layer features */
#define SYS_CPNT_SWDRVL3                    TRUE

#define SYS_CPNT_SWDRVL4                TRUE

/* Core layer */
#define SYS_CPNT_CONFDB                     FALSE
#define SYS_CPNT_SYSCALLBACK                FALSE /* currently, SYSCALLBACK only used in AMTR refinement */
#define SYS_CPNT_STKMGMT                    TRUE
#define SYS_CPNT_CLUSTER                    FALSE
#define AUTO_CREATE_CLUSTER_VLAN_AND_JOIN_ALL_PORTS TRUE
#define SYS_CPNT_CFGDB                      TRUE  /*Eli 2007.08.02: will be TRUE   */
#define SYS_CPNT_SWCTRL                     TRUE
#define SYS_CPNT_BRIDGE                     TRUE
#define SYS_CPNT_VLAN                       TRUE

/* TRUE if support MAC-based VLAN; FALSE if not. */
#define SYS_CPNT_MAC_VLAN                   FALSE
/* TRUE if MAC-based VLAN can also assign priority */
#define SYS_CPNT_MAC_VLAN_WITH_PRIORITY     TRUE

/* if true, one and only one untagged VLAN; else, multiple untagged VLANs
 */
#define SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_ACCESS_LINK        TRUE
#define SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_HYBRID_LINK        FALSE

/* if true, one untagged VLAN for PVID; else, all tagged VLANs
 */
#define SYS_CPNT_VLAN_SINGLE_UNTAGGED_ON_TRUNK_LINK         FALSE

/* if true, the pvid is fixed on default VLAN when the port mode is trunk mode.
 */
#define SYS_CPNT_VLAN_DEFAULT_PVID_FIXED_ON_TRUNK_LINK      FALSE

/* if true, when adding a port to a nonexistent VLAN statically, the VLAN will
 * be created automatically.
 */
#define SYS_CPNT_VLAN_AUTO_CREATE_STATIC_VLAN               TRUE

/* if true, when changing PVID, auto join the PVID VLAN as tagged member for
 * trunk mode, untagged member for access and hybrid modes
 */
#define SYS_CPNT_VLAN_AUTO_JOIN_VLAN_FOR_PVID               TRUE

/* if true, it is allowable to delete GVRP dynamic member statically.
 */
#define SYS_CPNT_VLAN_STATICALLY_DELETE_DYNAMIC_MEMBER_ALLOWED      TRUE

/* SYS_CPNT_ISOLATED_MGMT_VLAN define the include/exclude for MGMT-VLAN function
 * When the value is FALSE, no management packet filtering mechanism will not be included.
 * When the value is TRUE, management packet will be checked and decide to drop the packet
 * or not. In current stage, only ES4612-20 and ECN330 should set the value to be TRUE
 */
#define SYS_CPNT_ISOLATED_MGMT_VLAN         FALSE

/* Definition for the behavior to process the GVRP PDU when GVRP is disabled.
 * SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_NO_FLOOD     : discard the received GVRP PDU.
 * SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_FLOOD        : flood the GVRP PDU to all the other ports.
 * SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_FLOOD_VLAN   : flood the GVRP PDU to the assigned vlan according to the ingress rule.
 */
#define SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_NO_FLOOD    0
#define SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_FLOOD       1
#define SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_FLOOD_VLAN  2
#define SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED             SYS_CPNT_GVRP_ACTION_WHEN_GVRP_DISABLED_NO_FLOOD

/* Keep combo port cfg */
#define SYS_CPNT_SWCTRL_COMBO_PORT_CONFIG_KEEP TRUE

#define SYS_CPNT_L2MCAST                    FALSE

#define SYS_CPNT_SECURITY                   TRUE
#define SYS_CPNT_PORT_SECURITY              TRUE
#define SYS_CPNT_PORT_SECURITY_TRUNK        FALSE

/* for DOS protection
 */
#define SYS_CPNT_DOS                            FALSE

#if (SYS_CPNT_DOS == TRUE)
#define SYS_CPNT_DOS_ECHO_CHARGEN               FALSE /* supported by rule, not implemented yet */
#define SYS_CPNT_DOS_LAND                       TRUE
#define SYS_CPNT_DOS_SMURF                      FALSE /* supported by rule, not implemented yet */
#define SYS_CPNT_DOS_TCP_FLOODING               FALSE /* supported by rule, not implemented yet */
#define SYS_CPNT_DOS_TCP_NULL_SCAN              TRUE
#define SYS_CPNT_DOS_TCP_SYN_FIN_SCAN           TRUE
#define SYS_CPNT_DOS_TCP_UDP_PORT_ZERO          FALSE
#define SYS_CPNT_DOS_TCP_XMAS_SCAN              TRUE
#define SYS_CPNT_DOS_UDP_FLOODING               FALSE /* supported by rule, not implemented yet */
#define SYS_CPNT_DOS_WIN_NUKE                   FALSE /* supported by rule, not implemented yet */

/* For some Broadcom chips, ex.
 *   Firebolt
 *   Helix
 * there is only one switch to enable/disable TCP flags check,
 * that is, following features will be enabled/disabled at the same time.
 *   TCP_NULL_SCAN
 *   TCP_SYN_FIN_SCAN
 *   TCP_XMAS_SCAN
 *
 * Here is integrated features for it.
 */
#define SYS_CPNT_DOS_TCP_SCAN                   FALSE
#if (SYS_CPNT_DOS_TCP_SCAN == TRUE)
#undef SYS_CPNT_DOS_TCP_NULL_SCAN
#undef SYS_CPNT_DOS_TCP_SYN_FIN_SCAN
#undef SYS_CPNT_DOS_TCP_XMAS_SCAN
#endif

#if (SYS_CPNT_DOS_SMURF == TRUE)
#define SYS_CPNT_DOS_SMURF_IMPLEMENTED_BY_RULE  TRUE
#endif
#endif /* (SYS_CPNT_DOS == TRUE) */

#if (SYS_CPNT_SECURITY == TRUE)
#define SYS_CPNT_MGMT_IP_FLT                TRUE

#define SYS_CPNT_MGMT_IP_FLT_ENABLE_CHECK_FILTER    FALSE

/* MODE_SUBNET:    configure mgmt ip filter with (ip address, subnet mask)
 * MODE_RANGE:     configure mgmt ip filter with (start ip address, end ip address)
 */
#define SYS_CPNT_MGMT_IP_FLT_MODE_SUBNET    1
#define SYS_CPNT_MGMT_IP_FLT_MODE_RANGE     2
#define SYS_CPNT_MGMT_IP_FLT_MODE           SYS_CPNT_MGMT_IP_FLT_MODE_RANGE

/* This defines whether Management Filters for SSH and HTTPS
 * should be separate from Telnet and HTTP.
 *
 * For a default Accton-brand or platform project, it should be FALSE.
 * For some customers (e.g. 3Com), they are separate and this is TRUE.
 * But they have not considered whether SNMPv3 should be separate from SNMP.
 */
#define SYS_CPNT_MGMT_IP_FLT_SSH_HTTPS      FALSE

#define SYS_CPNT_INTRUSION_MSG_TRAP         TRUE
#endif

#define SYS_CPNT_NETWORK                    TRUE
#define SYS_CPNT_MIB2MGMT                   TRUE
#define SYS_CPNT_SYSMGMT                    TRUE
#define SYS_CPNT_SYSLOG                     TRUE
#define SYS_CPNT_UCMGMT                     TRUE
#define SYS_CPNT_COS                        TRUE
#define SYS_CPNT_DIFFSERV                   FALSE
#define SYS_CPNT_L2MUX                      TRUE
#define SYS_CPNT_QOS_V2                     TRUE

/* UC: use shared memory instead of himem
 */
#if (SYS_CPNT_UCMGMT == TRUE)
#define SYS_CPNT_UCMGMT_SHMEM               TRUE
#endif

/* Some ASIC (e.g. BCM XGSIII) do not allow class map have
 * different class-type (MAC, IP, IPv4) because hardware limitation.
 * But some are ok (e.g. Marvell DX106/107)
 * For Marvell DX106/107, MUST set this naming constant TRUE.
 */
#define SYS_CPNT_QOS_V2_CLASS_MAP_MIXED_MODE        FALSE

/* Temporary constant. This constant is need when some platform does not
 * implement match-all type for class-map.
 * If all platform have implement match-all type for class-map, we can REMOVE
 * this constant.
 *
 * Support match-all type for class-map
 */
#define SYS_CPNT_QOS_V2_CLASS_MAP_MATCH_ALL         FALSE

#define SYS_CPNT_QOS_V2_CLASS_MAP_MATCH_COS         TRUE
#define SYS_CPNT_QOS_V2_CLASS_MAP_MATCH_SOURCE_PORT (FALSE && SYS_CPNT_QOS_V2_EGRESS_PORT)

/* Bind rule(ACL, policy-map) on egress port (not include trunk port now).
 * If this constant is FALSE, then only support bind rule on ingress port.
 */
#define SYS_CPNT_QOS_V2_EGRESS_PORT                 TRUE

#define SYS_CPNT_QOS_V2_CPU_PORT                      TRUE

/*Auto enable counter for control-plane*/
#define SYS_CPNT_QOS_V2_COUNTER                       TRUE

/* Bind rule(ACL, policy-map) on global interface
 * Note: Only support ingress direction (No special reason).
 */
#define SYS_CPNT_QOS_V2_ALL_PORTS                     TRUE

#define SYS_CPNT_QOS_V2_SOFTWARE_BUNDLE_RATE_CONTROL  FALSE

/* Diffserv and marker is conflict, change old constant to following
 * to avoid both set to true
 */
#define SYS_CPNT_QOS_NONE                                       0
#define SYS_CPNT_QOS_MARKER                                     1
#define SYS_CPNT_QOS_DIFFSERV                                   2

#define SYS_CPNT_QOS                                            SYS_CPNT_QOS_DIFFSERV

/* FALSE: shared meters (default)
 *  TRUE: separate (not shared) meters (special chips)
 */
#define SYS_CPNT_DIFFSERV_SEPARATE_METERS                       FALSE

/* Gordon: For Marvell xCat chipsets, the CONFORM ACTION to modify DSCP
 *         is not supported.
 */
#define SYS_CPNT_QOS_DIFFSERV_CONFORM_ACTION_MODIFY_DSCP        TRUE

/* This definition is used for the set action of policy-map's class
 */
#define SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_COS                1   /* BIT-0 */
#define SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_PHB                2   /* BIT-1 */
#define SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_IP_DSCP            4   /* BIT-2 */
#define SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_IP_PRECEDENCE      8   /* BIT-3 */

#define SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_MODE    (SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_COS     | \
                                                        SYS_CPNT_QOS_DIFFSERV_CLASS_SET_ACTION_PHB)

/* inter DSCP = inter priority + color (drop precedence)
 */
#define SYS_CPNT_COS_INTER_DSCP                                 TRUE

#define SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS                   (FALSE && !SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS_PER_PORT          TRUE /* Now only support per-port */

#define SYS_CPNT_COS_ING_DSCP_TO_COS                            (TRUE  && !SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_DSCP_TO_COS_PER_PORT                   TRUE /* Now only support per-port */

#define SYS_CPNT_COS_ING_IP_PORT_TO_COS                         (FALSE && !SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_IP_PORT_TO_COS_PER_PORT                TRUE /* Now only support per-port */

#define SYS_CPNT_COS_ING_COS_TO_QUEUE                           (TRUE  && !SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_COS_TO_QUEUE_PER_PORT                  TRUE /* Now only support per-port */


/* Ingress CoS map to internal DSCP
 */
#define SYS_CPNT_COS_ING_COS_TO_INTER_DSCP                      (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_COS_TO_INTER_DSCP_PER_PORT             TRUE /* TRUE: per-port, FALSE: global */

/* Ingress IP precedence to internal DSCP
 */
#define SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_INTER_DSCP            (TRUE && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_INTER_DSCP_PER_PORT   TRUE /* Now only support per-port */

/* Ingress DSCP map to internal DSCP
 */
#define SYS_CPNT_COS_ING_DSCP_TO_INTER_DSCP                     (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_DSCP_TO_INTER_DSCP_PER_PORT            TRUE /* TRUE: per-port, FALSE: global */

/* Ingress TCP/UDP port to internal DSCP
 */
#define SYS_CPNT_COS_ING_IP_PORT_TO_INTER_DSCP                  (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_IP_PORT_TO_INTER_DSCP_PER_PORT         TRUE /* Now only support per-port */

/* Internal DSCP map to queue
 */
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_QUEUE                    (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_QUEUE_PER_PORT           FALSE /* TRUE: per-port, FALSE: global */

/* Internal DSCP map to cos */
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_COS                      (TRUE && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_COS_PER_PORT             TRUE /* Now only support per-port */

/* Internal DSCP map to color */
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_COLOR                    (TRUE && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_ING_INTER_DSCP_TO_COLOR_PER_PORT           TRUE /* Now only support per-port */

/* cos - trust mode */
#define SYS_CPNT_COS_TRUST_MODE_SUPPORT_COS                     (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_TRUST_MODE_SUPPORT_DSCP                    (TRUE  && SYS_CPNT_COS_INTER_DSCP)
#define SYS_CPNT_COS_TRUST_MODE_SUPPORT_PRECEDENCE              (TRUE  && SYS_CPNT_COS_INTER_DSCP)

/* support Per Port Per CoS Rate Limit
 */
#define SYS_CPNT_QOS_PER_PORT_PER_DOT1P_PRIORITY_RATE_LIMIT TRUE
#define SYS_CPNT_VLAN_BASED_PRIORITY        FALSE

/* Defines whether the user can configure the 802.1p mapping.
 * This is related to the design of the CPU interface.
 * For XGS, it should be FALSE.
 */
#define SYS_CPNT_1P_USER_CONFIGUREABLE             TRUE


#if (SYS_CPNT_SYSMGMT == TRUE)
#define SYS_CPNT_SYS_MGR                    TRUE
#define SYS_CPNT_SYS_TIME                   TRUE
#define SYS_CPNT_SYS_TIME_SUMMERTIME_ZONENAME      TRUE
#define SYS_CPNT_SYS_TIME_SUMMERTIME               TRUE
#define SYS_CPNT_SYS_TIME_ACCUMULATED_SYSTEM_UP_TIME FALSE

#define SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU             TRUE
#define SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION      TRUE

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
#define SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU_PER_TASK    TRUE
#endif

#if (SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU == TRUE)
#define SYS_CPNT_SYSMGMT_CPU_GUARD              TRUE
#endif

/* for sysResourceMgt MIB
 * need set SYS_CPNT_SYSMGMT_MONITORING_PROCESS_CPU and
 *          SYS_CPNT_SYSMGMT_MONITORING_MEMORY_UTILIZATION to TRUE
 */
#define SYS_CPNT_SYSMGMT_RESMIB                             TRUE

#define SYS_CPNT_SYSMGMT_DEFERRED_RELOAD    TRUE

#endif

/* This constant is used trun on the other mechanisms when our system know a plugged
 * in SPF transceiver is invalid or faulty:
 * 1) Shutdown this port
 * 2) Do report link up down of this port
 */
#define SYS_CPNT_INVALID_SFP_SHUTDOWN                      FALSE /* Useless */

/* If value is TRUE, indicate to use strong encryption algorithm for SSH and HTTPS.
 * SSH will use DES(56), 3DES(168), Blowfish(128), and AES(128,192,256) algorithms to encrypt/decrypt data.
 * HTTPS will use DES(56), 3DES(168), and RC4(40,56,128) algorithm to encrypt/decrypt data.
 * Otherwise, if value is FALSE, indicate to use weak encryption algorithm for SSH and HTTPS.
 * SSH will use DES(56) algorithms to encrypt/decrypt data.
 * HTTPS will use DES(56) and RC4(40) algorithm to encrypt/decrypt data.
*/
#define SYS_CPNT_SSL_WITH_STRONG_ENCRYPTION TRUE  /* for 128 bits encryption */

#define SYS_CPNT_SSHD                       FALSE
#define SYS_CPNT_SSH2                       TRUE

/* If this is TRUE, TFTP information and relateds will be stored
 * into the config DB, not CLI config file.
 */
#if (SYS_CPNT_CFGDB == TRUE)
#define SYS_CPNT_SSH2_TFTP_INFO_IN_CFGDB    FALSE
#endif /* SYS_CPNT_CFGDB == TRUE */

#define SYS_CPNT_CPU_PORT_RATE_LIMIT        FALSE /*anzhen.zheng, 8/19/2008*/

#define SYS_CPNT_SOFTBRIDGE                 FALSE
#define SYS_CPNT_SNTP                       TRUE
#define SYS_CPNT_NTP                        TRUE
#define SYS_CPNT_INGRESS_RATE_LIMIT         TRUE
#define SYS_CPNT_EGRESS_RATE_LIMIT          TRUE
#define SYS_CPNT_PORT_TRAFFIC_SEGMENTATION  TRUE    /* H/W support, somebody calls it private vlan  */

#if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE)
#define SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_SINGLE_SESSION        1
#define SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION      2
#define SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE     SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_MODE_MULTIPLE_SESSION
#endif /* #if (SYS_CPNT_PORT_TRAFFIC_SEGMENTATION == TRUE) */

#define SYS_CPNT_SFLOW  TRUE        /*sFlow*/
/* when SYS_CPNT_FE_PORT_GROUP is TRUE
 * it means that all ports of the same chip need to join together.
 * some ASICs, such as marvell DX107 and 88E6095, need to set it to TRUE
 */
#define SYS_CPNT_FE_PORT_GROUP  FALSE


#define SYS_CPNT_STP_TYPE_STA               1
#define SYS_CPNT_STP_TYPE_RSTP              2
#define SYS_CPNT_STP_TYPE_MSTP              3
#define SYS_CPNT_STP                        SYS_CPNT_STP_TYPE_MSTP

#define SYS_CPNT_STP_AUTO_EDGE_PORT         TRUE
#define SYS_CPNT_STP_ROOT_GUARD             TRUE
#define SYS_CPNT_STP_BPDU_GUARD             TRUE
#define SYS_CPNT_STP_BPDU_FILTER            TRUE

/* FALSE: Trunk will NOT inherit the 1st member STP attribute and member port will NOT
 *        inherit the trunk STP attribute too after leave from trunk.
 *        The STP attribute include path cost, fast start state and per port STP state, etc.
 */
#define SYS_CPNT_STP_INHERIT_TRUNK_STP_ATTR TRUE    /* if not defined, assumed TRUE */

#define SYS_CPNT_MAU_MIB                    TRUE

/* FALSE: Follow 802.1s standard and not change MTSI Message Format in MSTP BPDU. (16 octets)
          SYS_ADPT_MAX_NBR_OF_MST_INSTANCE shall be less than or egual 65.
   TRUE: Change MTSI Message Format in MSTP BPDU to solve MSTI lopping issue.(24 octets)
         Modify msti_bridge_priority to msti_bridge_id. (add 7 octets)
         Modify msti_port_priority to msti_port_id. (add 1 octest)
         SYS_ADPT_MAX_NBR_OF_MST_INSTANCE shall be less than or egual 58.
*/
#define SYS_CPNT_XSTP_PATCH_BPDU_FORMAT     FALSE

/* Definition for indicating the MST members from the assigned vlan members */
#define SYS_CPNT_MST_VLAN_EGRESS_TOPOLOGY                0

/* Definition for indicating the MST members from all the existing ports */
#define SYS_CPNT_MST_FULL_MEMBER_TOPOLOGY                1

/* Definition for indicating the MST members */
#define SYS_CPNT_MST_TOPOLOGY                            SYS_CPNT_MST_VLAN_EGRESS_TOPOLOGY

/*Define the MSTP only have delete by port or port+vid when do flush MAC.
   It FALSE, it may cause performance problem, because it may delete MAX vlan no. times.
   */
#define SYS_CPNT_XSTP_MSTP_DELETE_MAC_ONLY_BY_PORT TRUE

/*define the tc won't be prop through configured port*/
#define SYS_CPNT_XSTP_TC_PROP_STOP  TRUE

/* If this is TRUE, default users cannot be deleted.
 */
#define SYS_CPNT_USERAUTH_PROTECT_DEFAULT_USERS FALSE

/* If this is TRUE, default users privilege cannot be modified.
 * The constant shall be FALSE when SYS_CPNT_USERAUTH_PROTECT_DEFAULT_USERS
 * be defined as FALSE.
 */
#define SYS_CPNT_USERAUTH_PROTECT_DEFAULT_USERS_PRIVILEGE       FALSE


/* If this is TRUE, user names and passwords will be stored
 * into the config DB, not CLI config file.
 */
#if (SYS_CPNT_CFGDB == TRUE)
#define SYS_CPNT_USERAUTH_USER_IN_CFGDB     FALSE
#endif /* SYS_CPNT_CFGDB == TRUE */

/* This defines whether a customer-specific implementation of the "manager" (8)
 * user level between "admin" (15) and "guest" (0) is supported.
 * It involves UserAuth, RADIUS, CLI, WEB, SNMP, and maybe others.
 *
 * This must be added to all projects. Otherwise, missing these definitions
 * would result in #if 0 == 0 during comparison, which means always TRUE.
 */
#define SYS_CPNT_USERAUTH_MANAGER_STYLE_NONE  1
#define SYS_CPNT_USERAUTH_MANAGER_STYLE_3COM  2
#define SYS_CPNT_USERAUTH_MANAGER_STYLE     SYS_CPNT_USERAUTH_MANAGER_STYLE_NONE

/* This symbol will be defined to TRUE when HTTPS or SSH support */
#define SYS_CPNT_KEYGEN                     TRUE

#define SYS_CPNT_DNS                        TRUE

/* If this is TRUE, the IP address and relateds will be stored
 * into the config DB, not CLI config file.
 */
#if (SYS_CPNT_CFGDB == TRUE)
#define SYS_CPNT_NETCFG_IP_ADDRESS_IN_CFGDB FALSE
#define SYS_CPNT_CLI_RUNCFG_APPEND_CFGDB_IP FALSE
#endif /* SYS_CPNT_CFGDB == TRUE */


/* Layer 3 Core layer features */
#define SYS_CPNT_AMTRL3                     TRUE
#define SYS_CPNT_ROUTING                    TRUE  /* To distinguish L2 product from L3 product. */
#define SYS_CPNT_MULTIPLE_MGMT_IP           FALSE  /* Multiple management IP for L2 product */
#define SYS_CPNT_IPV6                       TRUE
#define SYS_CPNT_INET6                      TRUE

#define SYS_CPNT_LOOPBACK_IF_VISIBLE        TRUE  /* true to display loopback interface and route */
#define SYS_CPNT_IPV4_ROUTING               TRUE  /* Support IPV4 routing or not for L3 product. */
#if (SYS_CPNT_IPV6 == TRUE)
#define SYS_CPNT_IPV6_ROUTING               FALSE  /* Support IPV6 routing or not for L3 product. */
#endif
#define SYS_CPNT_IP_TUNNEL                  FALSE
#define SYS_CPNT_IPAL                   TRUE /*temp add by simon*/

/* application */
#define SYS_CPNT_TRACEROUTE                 TRUE
#define SYS_CPNT_PING                       TRUE
#define SYS_CPNT_NSLOOKUP                   SYS_CPNT_DNS

/* IML */
#define SYS_CPNT_IML                        TRUE
#define SYS_CPNT_IML_PRIORITY_Q_ENABLED     FALSE
#define SYS_CPNT_ND                         TRUE
#define SYS_CPNT_ARP_SUPPORT_TIMEOUT        TRUE

#if (SYS_CPNT_ROUTING == TRUE)
#define SYS_CPNT_NSM                        FALSE
#define SYS_CPNT_NSM_POLICY                 TRUE
#define SYS_CPNT_ARP                        TRUE
#define SYS_CPNT_PROXY_ARP                  FALSE
#define SYS_CPNT_ICMP                       TRUE
#define SYS_CPNT_RIP                        FALSE

#if (SYS_CPNT_ND == TRUE)
#define SYS_CPNT_RAADV                      FALSE
#endif /*SYS_CPNT_ND*/

/* This constant define the method to determine the metric of a static route
 * that redistributing to RIP. If you can make sure what you should use.
 * Please check with Layer3 team.
 * SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_WITH_ORIG_METRIC:
 *     metric = metric of static route + Interface metric
 *     Currently, 3com's product use this spec.
 * SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_WITH_REDIST_METRIC:
 *     metric = redist metric that configure by user. No matter what metric of static
 *     route is configured, all static route will be redistributed with this metric.
 *     All other product use this spec.
 *
 */
enum
{
    SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_WITH_STATIC_ROUTE_METRIC_PLUS_INTF_METRIC=0,
    SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_WITH_REDIST_METRIC
};
#define SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_METRIC_METHOD SYS_CPNT_REDIST_STATIC_ROUTE_TO_RIP_WITH_REDIST_METRIC

#define SYS_CPNT_RIP_ENABLE_RIF_BASE                TRUE
#define SYS_CPNT_OSPF                               FALSE
#define SYS_CPNT_OSPF6                              FALSE
#define SYS_CPNT_BGP                                FALSE
#define SYS_CPNT_ROUTE_OPTIMIZE_NET_ROUTE           TRUE
#define SYS_CPNT_SUPPORT_HW_NETROUTE                TRUE

/* support BGP rest API
 */
#define SYS_CPNT_BGP_RESTAPI                        TRUE

/* Only for Layer3 product.
For some Broadcom's ASIC, routing will work incorrectly when MSTP is enable.
If enable this feature, user can't enable routing and MSTP at same time for accton product.
For 3com product line, when user enable MSTP, routing will be disabled automatically.

Following Broadcom ASIC have this problem.
- 5690/5691 (all revisions)
- 5695/5696 (revision A, fixed in revision B)
                 note: revision B has a different spanning tree errata as documented
- 565x/566x (revision A, fixed in revision B)
- 5673 (all revisions)
                 note: does not apply to 5674*/
#define SYS_CPNT_ROUTING_MUTUAL_EXCLUSIVE_WITH_MSTP     FALSE


#define SYS_CPNT_HSRP                       FALSE
#define SYS_CPNT_VRRP                       TRUE
#define SYS_CPNT_VRRP_PING                  TRUE

#define SYS_CPNT_VIRTUAL_IP                 TRUE
#endif /* (SYS_CPNT_ROUTING == TRUE) */

/* Application layer */
#define SYS_CPNT_LEDMGMT                    TRUE
#if (SYS_CPNT_LEDMGMT == TRUE)
/* define SYS_CPNT_LEDMGMT_LOCATION_LED as TRUE if this project supports
 * Location LED. Users can issue "location-led on" to turn on Location LED,
 * and issue "location-led off" to turn off Location LED.
 */
#define SYS_CPNT_LEDMGMT_LOCATION_LED       TRUE
#endif /* end of #if (SYS_CPNT_LEDMGMT == TRUE) */

/* define the SNMP component,RMON component and TRAP compoment:
 *
 * Note that in here, the RMON/TRAP component represents the Epliogue Elite RMON
 * packet and TRAP packet, so this packet is now auto enable/disable by the SNMP version.
 * We only need to take care if we want to enable the SNMP component and the
 * SNMP version, if the SNMP version equal to 1 or 2, it will auto enable the
 * RMON/TRAP component(Epliogue Elite) and if the SNMP version is equal to 3.
 * The SNMPV3 (Net-Snmp) component have included RMON (Net-snmp)/TRAP inside and
 * thus we will auto disable the RMON/TRAP component (Epligogue Elite).
 */
#define SYS_CPNT_SNMP                                   TRUE

#if (SYS_CPNT_SNMP == TRUE)
#define SYS_CPNT_SNMP_VERSION  3

/* To be compatible with current naming convention -
 * SNMP and RMON/TRAP enabled/disabled at the same time.
 */
#if ((SYS_CPNT_SNMP_VERSION == 1) || (SYS_CPNT_SNMP_VERSION == 2))
#define SYS_CPNT_RMON                                   TRUE
#define SYS_CPNT_TRAPMGMT                               TRUE

/* DO NOT mean RMON disabled, but
   leave RMON/TRAP auto enabled at the SNMPv3 component (net-snmp) */
#elif (SYS_CPNT_SNMP_VERSION == 3)
#define SYS_CPNT_RMON                                   FALSE
#define SYS_CPNT_TRAPMGMT                               FALSE

/* Should not happen
 */
#else
#define SYS_CPNT_RMON                                   FALSE
#define SYS_CPNT_TRAPMGMT                               FALSE
#endif

#else
#define SYS_CPNT_RMON                                   FALSE
#define SYS_CPNT_TRAPMGMT                               FALSE
#endif

/* If this is TRUE, SNMP communities will be stored
 * into the config DB, not CLI config file.
 */
#if (SYS_CPNT_CFGDB == TRUE)
#define SYS_CPNT_SNMP_COMMUNITY_IN_CFGDB    FALSE
#endif /* SYS_CPNT_CFGDB == TRUE */

/* This defines whether an SNMP variable of type BITS
 * has the bits counting from the left (highest bit of first
 * byte). According to RFC1906, this should be TRUE. But old
 * products have it reversed (like some big companies). So,
 * for backward compatibility, we use this SYS_CPNT.
 */
#define SYS_CPNT_SNMP_BITS_FROM_LEFT        TRUE

/* This defines the style of the private MIB.
 *
 * This must be added to all projects. Otherwise, missing these definitions
 * would result in #if 0 == 0 during comparison, which means always TRUE.
 *
 * This is called "08" instead of the customer's name, in order to hide the
 * customer's name as much as possible (e.g. P5).  But actual MIB files and source
 * code are based on the MIB modules and nodes, and contain the customer's name.
 *
 * SYS_CPNT_SNMP_MIB_STYLE_ACCTON:  private MIB (including traps) from Accton
 * SYS_CPNT_SNMP_MIB_STYLE_08:      private MIB (including traps) from 08
 * SYS_CPNT_SNMP_MIB_STYLP_32:      private MIB (including traps) from
 */
#define SYS_CPNT_SNMP_MIB_STYLE_ACCTON      1
#define SYS_CPNT_SNMP_MIB_STYLE_08          2
#define SYS_CPNT_SNMP_MIB_STYLE_32          3
#define SYS_CPNT_SNMP_MIB_STYLE             SYS_CPNT_SNMP_MIB_STYLE_ACCTON

/* The following defines whether function "SNMP_MGR_SetTrapReceiverStatusByIndex"
 * keeps the resulting sequence of existing indexes always contiguous,
 * e.g. from "1, 2, 3, 4", destroying "3" will resulting in "4" becoming "3",
 * and from "1, 2", creating "4" will also result in "4" becoming "3".
 *
 * For an Accton or platform project, this shall be FALSE.
 * Some customers, e.g. 08, require this to be TRUE.
 */
#define SYS_CPNT_SNMP_CONTIGUOUS_TRAP_DEST_TABLE FALSE

/* This defines whether default groups and users are created in SNMPv3.
 */
#define SYS_CPNT_SNMPV3_DEFAULT_GROUP       FALSE
#define SYS_CPNT_SNMPV3_DEFAULT_USER        FALSE

/* This defines the engine ID style.
 *
 * This must be added to all projects. Otherwise, missing these definitions
 * would result in #if 0 == 0 during comparison, which means always TRUE.
 *
 * SYS_CPNT_SNMPV3_ENGINE_ID_STYLE_ACCTON: Accton and default customers
 * SYS_CPNT_SNMPV3_ENGINE_ID_STYLE_3COM: 3Com only
 */
#define SYS_CPNT_SNMPV3_ENGINE_ID_STYLE_ACCTON 1
#define SYS_CPNT_SNMPV3_ENGINE_ID_STYLE_3COM   2
#define SYS_CPNT_SNMPV3_ENGINE_ID_STYLE SYS_CPNT_SNMPV3_ENGINE_ID_STYLE_ACCTON

/* This defines the style of the target name for SNMPv3.
 *
 * This must be added to all projects. Otherwise, missing these definitions
 * would result in #if 0 == 0 during comparison, which means always TRUE.
 *
 * SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_ACCTON: fixed text.IP address.user name
 * SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM: target name from user
 */
#define SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_ACCTON 1
#define SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_3COM   2
#define SYS_CPNT_SNMPV3_TARGET_NAME_STYLE SYS_CPNT_SNMPV3_TARGET_NAME_STYLE_ACCTON

/* If this is TRUE, default RMON alarm entries will be created.
 */
#define SYS_CPNT_RMON_ALARM_DEFAULT         TRUE /* if not defined, assumed TRUE */

#define SYS_CPNT_WEB                        FALSE

/* Define WEB format is which style
 */
#define SYS_CPNT_WEB_STYLE_ACCTON           1
#define SYS_CPNT_WEB_STYLE_DELL             2
#define SYS_CPNT_WEB_STYLE_3COM             3

/* Define WEB format is Accton style
 */
#define SYS_CPNT_WEB_STYLE                  SYS_CPNT_WEB_STYLE_ACCTON

/* Define if WEB Compression is implemented
 */
#define SYS_CPNT_WEB_COMPRESSION            FALSE

#define SYS_CPNT_WEB_OEM_LOGIN_SCREEN       TRUE

#define SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_DHCP  TRUE

/* use busybox's udhcpc for dynamic provision on craft port
 */
#define SYS_CPNT_CLI_DYNAMIC_PROVISION_VIA_UDHCPC_CRAFT_PORT          TRUE

#define SYS_CPNT_TELNET                     TRUE

#define SYS_CPNT_XFER                       TRUE
/* This constant defines to support FTP download or not
 */
#define SYS_CPNT_XFER_FTP                   TRUE

/* This constant defines to support loader download or not
 */
#define SYS_CPNT_XFER_DOWNLOAD_LOADER       FALSE

#define SYS_CPNT_DHCP                       TRUE
#define SYS_CPNT_BOOTP                      TRUE
#define SYS_CPNT_DHCP_INFORM                FALSE
#define SYS_CPNT_DHCPSNP                    FALSE
#if (SYS_CPNT_DHCPSNP == TRUE)
#define SYS_CPNT_DHCPSNP_INFORMATION_OPTION TRUE
#define SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_CID TRUE
#define SYS_CPNT_DHCPSNP_INFORMATION_OPTION_CONFIGURABLE_RID TRUE
#define SYS_CPNT_DHCPSNP_SYSTEM_RATELIMIT   TRUE
#endif
#define SYS_CPNT_IP_SOURCE_GUARD            FALSE
#if (SYS_CPNT_IP_SOURCE_GUARD == TRUE)
#define SYS_CPNT_IPSG_MAC_MODE              TRUE
#define SYS_CPNT_IPSG_MAC_MODE_BLOCK_RECORD TRUE
#else /* (SYS_CPNT_IP_SOURCE_GUARD == TRUE) */
#define SYS_CPNT_IPSG_MAC_MODE              FALSE
#define SYS_CPNT_IPSG_MAC_MODE_BLOCK_RECORD FALSE
#endif /* (SYS_CPNT_IP_SOURCE_GUARD == TRUE) */

#define SYS_CPNT_NDSNP                      FALSE

#define SYS_CPNT_CLI                        TRUE
#define SYS_CPNT_CLI_BANNER                 TRUE
#define SYS_CPNT_HTTP                       TRUE
#define SYS_CPNT_HTTP_DIGEST_AUTHENTICATION TRUE
#define SYS_CPNT_HTTPS                      TRUE

/* This is define WEB Authentication feature. If this is TRUE,
 * webauth can function after enabling it. All hosts in a webauth
 * enabled port will need authentication if they want use internet resource.
 */
#define SYS_CPNT_WEBAUTH                    TRUE

#define SYS_CPNT_TACACS                     TRUE
#define SYS_CPNT_RADIUS                     TRUE
#define SYS_CPNT_DHCPV6                     FALSE
#define SYS_CPNT_DHCPV6SNP                  FALSE
#define SYS_CPNT_DHCPV6SNP_REMOTE_ID_OPTION TRUE   /* dhcpv6 option37 */
#define SYS_CPNT_IPV6_SOURCE_GUARD          FALSE

/* Follow cisco's operation of filtering display in terminal and
   some setting in terminal, such as length, width, terminal type,
   history size and escape character.
 */
#define SYS_CPNT_CLI_TERMINAL                   FALSE

/* Naming constant to indicate if switch supports
 * adding all commands in a configuration file to the current running configuration.
 * Value TRUE  -- support adding extra running configuration
 *       FALSE -- not support
 */
#define SYS_CPNT_CLI_ADD_TO_RUNNING_CONFIG              TRUE

#define SYS_CPNT_AAA                                    FALSE
#define SYS_CPNT_RADIUS_AUTHENTICATION                  TRUE
#define SYS_CPNT_RADIUS_ACCOUNTING                      FALSE
#define SYS_CPNT_RADIUS_ACCOUNTING_COMMAND              (FALSE && SYS_CPNT_RADIUS_ACCOUNTING)

#define SYS_CPNT_TACACS_PLUS_AUTHENTICATION             TRUE
#define SYS_CPNT_TACACS_PLUS_ACCOUNTING                 FALSE
#define SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND         (FALSE && SYS_CPNT_TACACS_PLUS_ACCOUNTING)
#define SYS_CPNT_AUTHENTICATION                         (SYS_CPNT_RADIUS_AUTHENTICATION || SYS_CPNT_TACACS_PLUS_AUTHENTICATION)
#define SYS_CPNT_ACCOUNTING                             (SYS_CPNT_RADIUS_ACCOUNTING || SYS_CPNT_TACACS_PLUS_ACCOUNTING)
#define SYS_CPNT_ACCOUNTING_COMMAND                     (SYS_CPNT_RADIUS_ACCOUNTING_COMMAND || SYS_CPNT_TACACS_PLUS_ACCOUNTING_COMMAND)

/*maggie liu for authorization*/
#define SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_ENABLE          1
#define SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION   2
#define SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_METHOD             SYS_CPNT_TACACS_PLUS_GRANT_ADMIN_BY_AUTHORIZATION

/* If this is TRUE, the TCP socket of TACACS will support retry and timeout
 * feature.
 * If this is FALSE, the TCP socket of TACACS will use default timeout
 * value and not retry when transmit ion fail.
 */
#define SYS_CPNT_TACACS_PLUS_RETRY_TIMEOUT              TRUE

#define SYS_CPNT_TACACS_PLUS_AUTHORIZATION              FALSE
#define SYS_CPNT_AUTHORIZATION                          SYS_CPNT_TACACS_PLUS_AUTHORIZATION
#define SYS_CPNT_TACACS_PLUS_MULTIPLE_SERVER            TRUE

/* If this is TRUE, the RADIUS secret key will be stored
 * into the config DB, not CLI config file.
 */
#if (SYS_CPNT_CFGDB == TRUE)
#define SYS_CPNT_RADIUS_SECRET_KEY_IN_CFGDB FALSE
#endif /* SYS_CPNT_CFGDB == TRUE */

/* These three constants are defined for 802.1X,
 * SYS_CPNT_DOT1X               -- if the project want to include 802.1X
 * SYS_CPNT_DOT1X_AUTHENTICATOR -- 802.1X authenticator is supported or not
 * SYS_CPNT_DOT1X_TRANSPARENT   -- transparent 802.1X is supported or not
 * SYS_CPNT_DOT1X_OPERATION_MODE-- Identify that 802.1X is supported with transparent or
 *                                  authenticator mode.
 */
#define SYS_CPNT_DOT1X_AUTHENTICATED        1
#define SYS_CPNT_DOT1X_TRANSPARENT          2

#define SYS_CPNT_DOT1X                      TRUE
#define SYS_CPNT_DOT1X_OPERATION_MODE       SYS_CPNT_DOT1X_AUTHENTICATED

/* This defines whether in 802.1X, three encryption methods
 * TTLS, TLS, and PEAP are supports. Old products and products
 * running on smaller memory size may not support these, but
 * the ACP_V3 platform products and products with 128MB of SDRAM are
 * recommended to support these.
 */
#define SYS_CPNT_DOT1X_TTLS_TLS_PEAP        TRUE


/* If true, 802.1X will support MAC-based authentication.
 * Else, 802.1X doesn't support MAC-based authentication.
 */
#define SYS_CPNT_DOT1X_MACBASED_AUTH        TRUE

/* This defines whether support EAPOL frames pass-through.
 * If this is TRUE, 802.1X support EAPOL frames pass-through by
 * user configration.
 * Else, does not support EAPOL frames pass-through.
 */
#define SYS_CPNT_DOT1X_EAPOL_PASS_THROUGH   TRUE

#define SYS_CPNT_NETACCESS                  TRUE
#define SYS_CPNT_NETACCESS_GUEST_VLAN       TRUE
#define SYS_CPNT_NETACCESS_MACAUTH          TRUE
#define SYS_CPNT_NETACCESS_DYNAMIC_VLAN     TRUE
#define SYS_CPNT_NETACCESS_DYNAMIC_QOS      TRUE
#define SYS_CPNT_NETACCESS_MAC_FILTER_TABLE TRUE
#define SYS_CPNT_NETACCESS_LINK_DETECTION   TRUE

/* This defines the aging attribute of the secure MAC address.
 * SYS_CPNT_NETACCESS_AGING_MODE_STATIC, the secure MAC address shall not be aged out.
 * SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC, the secure MAC address shall be aged out.
 * SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE, decide by user.
 *
 * This must be added to all projects. Otherwise, missing these definitions
 * would result in #if 0 == 0 during comparison, which means always TRUE.
 */
#define SYS_CPNT_NETACCESS_AGING_MODE_STATIC            1
#define SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC           2
#define SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE      3  /* user can decide static or dynamic via UI */

#define SYS_CPNT_NETACCESS_AGING_MODE                   SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE   /* one of the above three MAC MODE */

/* Define support igmp authentication or not */
#define SYS_CPNT_IGMPAUTH                   FALSE

/* Define if this is a VDSL product
 */
#define SYS_CPNT_VDSL                       FALSE

/* Special component for root.c to call */
#define SYS_CPNT_PFU                        FALSE       /* Packet Filtering Unit */


/* Define sub components this project supports
 * Note: Project Leader needs to review these components supported or not
 *       based on product spec.
 */
#if (SYS_CPNT_SWCTRL == TRUE)
#define SYS_CPNT_AMTR                       TRUE
#define SYS_CPNT_NMTR                       TRUE
#define SYS_CPNT_LACP                       TRUE
#define SYS_CPNT_LACP_STATIC_JOIN_TRUNK     (TRUE && SYS_CPNT_LACP)
#endif

#define SYS_CPNT_SWCTRL_CABLE_DIAG            (FALSE && SYS_CPNT_SWCTRL)    /* TRUE to support cable diagnostics */
#define SYS_CPNT_SWCTRL_CABLE_DIAG_MARVELL    (SYS_CPNT_SWCTRL_CABLE_DIAG && 1)
#define SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM   (SYS_CPNT_SWCTRL_CABLE_DIAG && 2)
#define SYS_CPNT_SWCTRL_CABLE_DIAG_CHIP       (SYS_CPNT_SWCTRL_CABLE_DIAG && SYS_CPNT_SWCTRL_CABLE_DIAG_BROADCOM)

/* Define if port MAC learning is configurable or not.
 */
#if (SYS_CPNT_AMTR == TRUE)
#define SYS_CPNT_AMTR_PORT_MAC_LEARNING     TRUE
#endif

/* Supports counter history
 */
#define SYS_CPNT_NMTR_HISTORY               (TRUE && SYS_CPNT_NMTR)

/* Define if VLAN counter is supported or not.
 */
#define SYS_CPNT_NMTR_VLAN_COUNTER          (FALSE && SYS_CPNT_NMTR)

/* Set TRUE to sync nmtr statistics to net device.
 */
#define SYS_CPNT_NMTR_SYNC_NDEV             (TRUE && SYS_CPNT_NMTR)


/* Define the LLDP component
 */
#define SYS_CPNT_LLDP                       TRUE /* temp FALSE for ASF4506B */

#if (SYS_CPNT_LLDP == TRUE)
#define SYS_CPNT_LLDP_EXT                   TRUE
#define SYS_CPNT_LLDP_MED                   TRUE
/* DCBX should support only when system support LLDP */
#define SYS_CPNT_DCBX                       TRUE
#endif


#if (SYS_CPNT_SWDRV == TRUE)
#define SYS_CPNT_AMTRDRV                    TRUE
#define SYS_CPNT_NMTRDRV                    TRUE
#define SYS_CPNT_SWDRV_CACHE                FALSE /*eli mark,will be TRUE 20070723 */
#define SYS_CPNT_SWDRVL3_CACHE              FALSE
#endif
/* SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT:
 * If set TRUE, SYS_HWCFG_SUPPORT_PD must also be TRUE
 * If set TRUE, ports that support PD only allow to link up
 * when link partner is PSE port.
 */
#define SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT (FALSE && SYS_CPNT_SWDRV)

/* PORT Shutdown
 * The function is to do port shutdown which it is shutdown of mac or mac-and-phy.
 * SYS_DFLT_DEVSWDRV_DISABLE_PHY_WHEN_PORT_ADMIN_DISABLE : It is defined in sys_dflt.h
 *     TRUE -> The shutdown is shutdown of mac and phy.
 *     FALSE -> The shutdown is only shutdown of mac.
 * SYS_CPNT_SHUTDOWN_ALLOW_IN_TURNK_MEMBER :
 *     TRUE -> Allow the port independently do shutdown when the port is a member of trunk.
 *     FALSE -> Do not allow the port independently do shutdown when the port is a member of trunk.
 */
#define SYS_CPNT_SHUTDOWN_ALLOW_IN_TURNK_MEMBER     FALSE


#if (SYS_CPNT_SYSLOG == TRUE)
#define SYS_CPNT_SYSLOG_BACKUP              TRUE
#define SYS_CPNT_REMOTELOG                  TRUE
#define SYS_CPNT_SMTP                       FALSE /* temp FALSE for ASF4506B */
#endif

/* When this define is set to true, only the specified protocol is allowed to access the switch.
 * eg: management telnet 1.1.1.1
       then user 1.1.1.1 can access the switch by telnet only,
       not by web or snmp.
   when this define is set to false, user (1.1.1.1) can still get access to switch via web/snmp.
   but on telnet, only this user (1.1.1.1) gets access, no one else is allowed to.
 */
#define SYS_CPNT_MANAGEMENT_IP_FILTER_DEFAULT_DENY  FALSE

#define SYS_CPNT_PORT_TRAFFIC_SEGMENTATION_WITHOUT_FFP  TRUE

#if (SYS_CPNT_DHCP == TRUE)
#define SYS_CPNT_DHCP_CLIENT                TRUE
#define SYS_CPNT_DHCP_RELAY                 TRUE
#define SYS_CPNT_DHCP_SERVER                FALSE
#endif

#define SYS_CPNT_DHCP_RELAY_OPTION82        (FALSE && SYS_CPNT_DHCP)

#if (SYS_CPNT_DHCP_CLIENT == TRUE)
#define SYS_CPNT_DHCP_CLIENT_CLASSID        TRUE      /* for option 60 */
#endif


#if (SYS_CPNT_DHCPV6 == TRUE)
#define SYS_CPNT_DHCPV6_RELAY               TRUE
#endif

/* Following is for feature definition of project
 */


/* Naming constant to indicate if ASIC doesn't support per port configuration.
 * Value TRUE  -- support port based priority management
 *       FALSE -- not support
 * 802.1p will not support per port configuration if ASIC not support
 */
#define SYS_CPNT_PRI_MGMT_PORT_BASE         FALSE


/* For COS setting
 */
#define SYS_CPNT_STACK_COS_BY_DEFAULT       TRUE
#define SYS_CPNT_COS_PER_PORT               FALSE

/* For Priority Management
 * Naming constant to indicate if ASIC doesn't support per port configuration.
 * 802.1p will not support per port configuration if ASIC not support
 */
#define SYS_CPNT_PRI_MGMT_PER_PORT          FALSE


/* Define CLI displaying features
 * Note: The following is for CLI to decide what information to display for command:
 *       show version
 */
#if (SYS_CPNT_SYSMGMT == TRUE)
#define SYS_CPNT_SYSMGMT_SHOW_SERVICE_TAG       FALSE
#define SYS_CPNT_SYSMGMT_SHOW_MICROCODE_VER     FALSE
#define SYS_CPNT_SYSMGMT_SHOW_EXPAN_MODULE      FALSE
#define SYS_CPNT_SYSMGMT_SHOW_BACKUP_POWER      TRUE
#define SYS_CPNT_SYSMGMT_SHOW_FAN_STATUS        TRUE
#define SYS_CPNT_SYSMGMT_SHOW_POWER             TRUE
#define SYS_CPNT_CLI_SHOW_ALARM_INPUT           FALSE
#endif

/* Define this constant TRUE to support "show tech-support" on the CLI.
 */
#define SYS_CPNT_CLI_SHOW_TECH_SUPPORT      TRUE

/* Nelson, 11-02-2009, for showing tech support.
 *
 * Moved on 2011.08.16 from SYS_ADPT to SYS_CPNT,
 * because SYS_CPNT is for functions, and SYS_ADPT is for values.
 * These values must be the same as whether these commands exist on the CLI,
 * so that "show tech-support" can display their output.
 *
 * show arp
 * show interfaces
 * show interfaces brief
 */
#define SYS_CPNT_CLI_SHOW_ARP               TRUE
#define SYS_CPNT_CLI_SHOW_INTERFACES        TRUE
#define SYS_CPNT_CLI_SHOW_INTERFACES_BRIEF  TRUE


#define SYS_CPNT_MULTI_BRIDGE_NONE          1
#define SYS_CPNT_MULTI_BRIDGE_AGENT         2
#define SYS_CPNT_MULTI_BRIDGE_BLADE         3
#define SYS_CPNT_MULTI_BRIDGE_BOTH          4


/* This definition is used for project FTTH.  At FTTH project, there would
 * be agent or bridge for rsapi.
 */
#define SYS_CPNT_MULTI_BRIDGE_TYPE          SYS_CPNT_MULTI_BRIDGE_NONE

/* add compile options for storm  sample type
*/
#define SYS_CPNT_STORM_SAMPLE_TYPE_OCTET     FALSE
#define SYS_CPNT_STORM_SAMPLE_TYPE_PACKET    TRUE
#define SYS_CPNT_STORM_SAMPLE_TYPE_PERCENT   FALSE
#define SYS_CPNT_STORM_SAMPLE_TYPE_KILO_BITS FALSE

#define SYS_CPNT_BSTORM_SUPPORT_LPORT       TRUE
#define SYS_CPNT_BSTORM_RATE_PER_PORT_CTRL  TRUE
#define SYS_CPNT_BSTORM_ENABLE_PER_PORT     TRUE
#define SYS_CPNT_MSTORM_SUPPORT_LPORT       TRUE
#define SYS_CPNT_MSTORM_RATE_PER_PORT_CTRL  TRUE
#define SYS_CPNT_MSTORM_ENABLE_PER_PORT     TRUE
#define SYS_CPNT_MULTI_MIRRORED_PORT        TRUE
#define SYS_CPNT_MULTI_MIRROR_TO_PORT       TRUE

#define SYS_CPNT_VLAN_MIRROR                FALSE
#define SYS_CPNT_MAC_BASED_MIRROR           FALSE

/* Unknown Unicast Storm (DLF)
 */
#define SYS_CPNT_UNKNOWN_USTORM             TRUE /* Should change to TRUE later */

#if (SYS_CPNT_UNKNOWN_USTORM == TRUE)
#define SYS_CPNT_UNKNOWN_USTORM_SUPPORT_LPORT       TRUE
#define SYS_CPNT_UNKNOWN_USTORM_RATE_PER_PORT_CTRL  TRUE
#define SYS_CPNT_UNKNOWN_USTORM_ENABLE_PER_PORT     TRUE
#endif

/*  This feature is only one rate limit share to BC/MC/UUC storm control
 */
#define SYS_CPNT_RATE_BASED_STORM_CONTROL           FALSE


#if (SYS_CPNT_MULTI_MIRROR_TO_PORT == FALSE)
/* this feature make sense only when there is only one mirroring destination port
 */
#define SYS_CPNT_ALLOW_DUMMY_MIRRORING_DEST     FALSE
#endif

/* This definition is used to indicate if WRR Q Mode and Q Weight is supported by per port based
 */
#define SYS_CPNT_WRR_Q_MODE_PER_PORT_CTRL   TRUE    /* Jinfeng.chen in 2008-05-30 , old: FALSE*/
#define SYS_CPNT_WRR_Q_WEIGHT_PER_PORT_CTRL TRUE

#define SYS_CPNT_SWCTRL_Q_MODE_DRR   FALSE

/* These define whether flow control and forced-mode speed-dupex
 * are supported. For some chips, this is not supported,
 * and is not available from CLI, SNMP, or Web.
 */
/* TRUE to include flow control config menu
 */
#define SYS_CPNT_FLOW_CONTROL                           TRUE
#define SYS_CPNT_FLOW_CONTROL_NOT_SUPPORT_ON_10G        FALSE /* Useless */
#define SYS_CPNT_RATE_LIMIT_NOT_SUPPORT_ON_10G          TRUE /* Useless */

/* TRUE to include speed duplex config menu
 */
#define SYS_CPNT_SPEED_DUPLEX                           FALSE
#define SYS_CPNT_NEGOTIATION                            TRUE

/* ACPv4 AMTR has the option of using sofware learn or hardware learn on a standalone switch.
 * The following definiation indicates whether the project is to run software learn or hardware learn on a standalone switch.
 * Note: only software learn is supported for stacked switches
 */
#define SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE    TRUE

/* to show all mac address on chip at CLI*/
#define SYS_CPNT_SWCTRL_SHOW_ADDRESS_TABLE_TOTAL_ENTRY   TRUE

/* These three constants define for vlan,
 * They are telling if this project supports vlan hybrid mode, q_trunk mode, access mode
 * separately.
 */
#define SYS_CPNT_VLAN_HYBRID                TRUE
#define SYS_CPNT_VLAN_QTRUNK                TRUE
#define SYS_CPNT_VLAN_ACCESS                TRUE

#define SYS_CPNT_VLAN_NO_CHECK_VLAN_FOR_PVID    TRUE
#define SYS_CPNT_VLAN_AUTO_CREATE_VLAN_FOR_PVID TRUE


/* This content define vlan behavior for crossbar chipset (5670/5671)
 * TRUE  : crossbar ports will join all vlans
 * FALSE : crossbar ports will not join all vlans (depend on its front panel chipset vlan set)
 *
 * Note : Per Project Definition as following
 * TRUE  : all projects except 5508
 * FALSE : 5508
 */
#define SYS_CPNT_CROSSBAR_JOIN_ALL_VLANS    TRUE

/* A null trunk without any member is available
 * for the LACP operation and the manual configuration
 * to add the member in when this definition is TRUE.
 * Otherwise, the null trunk accepts only the static
 * members if it is created by the manual configuration,
 * and vice versa.
 */
#define SYS_CPNT_SUPPORT_NULL_TRUNK         TRUE

/* The static trunk configuration is allowed on the port with the LACP enabled */
#define SYS_CPNT_STATIC_TRUNK_CONFIG_ALLOWED_ON_LACP_PORT   TRUE

/* The old spec was to allow the trunk members to join the same trunk if they had the same attribute,
 * else stop the configuring and returned false.
 * And the new spec is to apply the trunk attribute to the trunk members and let them join the trunk.
 */
#define SYS_CPNT_TRUNK_MEMBER_AUTO_ATTRIBUTE                TRUE

/* To allow user to config trunk load balance mode
 */
#define SYS_CPNT_TRUNK_BALANCE_MODE         TRUE

/* This constant defines fan, thermal and power status detection
 */
#define SYS_CPNT_ALARM_GROUP_MODE_TER       0
#define SYS_CPNT_ALARM_GROUP_MODE_P5        1
#define SYS_CPNT_ALARM_GROUP_MODE           SYS_CPNT_ALARM_GROUP_MODE_TER

#define SYS_CPNT_POWER_DETECT               TRUE
#define SYS_CPNT_STKTPLG_FAN_DETECT         TRUE
/* SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_STATE_MACHINE_HAVE_TRANSITION_STATE
 * TRUE  - The fan speed control state machine has a transition state when fan
 *         speed mode is changed from middle to full or from full to middle.
 * FALSE - No transition state exists when fan speed mode is changed from middle
 *         to full or from full to middle.
 */
#define SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_STATE_MACHINE_HAVE_TRANSITION_STATE FALSE

#if (SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_STATE_MACHINE_HAVE_TRANSITION_STATE==FALSE)
/* Add the fan fail state into consideration when evaluating the
 * fan speed settings to be set to the fan controller
 */
#define SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_INCLUDE_FAN_FAIL_STATE TRUE
/* SYS_CPNT_STKTPLG_FAN_SET_FULL_SPEED_WHEN_ONE_THERMAL_OVER_TEMPERATURE:
 * TRUE  - Set fan speed as FULL when at least one thermal is larger than critical temperature
 * FALSE - Set fan speed as FULL when all of the thermal are larger than critical temperature
 */
#define SYS_CPNT_STKTPLG_FAN_SET_FULL_SPEED_WHEN_ONE_THERMAL_OVER_TEMPERATURE TRUE
#endif
#define SYS_CPNT_THERMAL_DETECT             TRUE

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
#define SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL    FALSE
#define SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST   FALSE
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
#define SYS_CPNT_THERMAL_DETECT_SHOW_TEMPERATURE TRUE
#endif
#define SYS_CPNT_FAN_FAULT_STATE_DETECT     FALSE
#define SYS_CPNT_SEVEN_LED_DETECT           FALSE
#define SYS_CPNT_SUPPORT_ALARM_GROUP        FALSE
#if (SYS_CPNT_POWER_DETECT==TRUE)
#define SYS_CPNT_SUPPORT_POWER_MODULE_TYPE  FALSE
#endif
#define SYS_CPNT_POWER_LED_DISPLAY_BY_SW    FALSE
#define SYS_CPNT_ALARM_DETECT               FALSE /* detect major/minor alarm and also detect alarm input if SYS_CPNT_ALARM_INPUT_DETECT is TRUE */

#if (SYS_CPNT_ALARM_DETECT==TRUE)
#define SYS_CPNT_ALARM_INPUT_DETECT         TRUE /* only detect alarm input */

/* SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV:
 * If set as TRUE, ALARM LED will be controlled by LEDDRV task
 * If set as FALSE, ALARM LED will be controlled by SYSDRV task
 * Need to set as TRUE when FAN fail LED and Major/Minor ALARM LEDs
 * are controlled by the same register
 * If there is time for refinement, ALARM LED shall be controlled by LEDDRV task on all projects
 */
#define SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV TRUE

/***********************************************
 * Major Alarm output type related definitions *
 ***********************************************
 */
/* SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER:
 * If defined as TRUE:
 *     There are two powers. If any one of the power status is failed,
 *     major alarm will be enabled.
 * If defined as FALSE:
 *     Set major alarm if main power status is not ok
 */
#define SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER FALSE
/* SYS_CPNT_ALARM_OUTPUT_MAJOR_POWER_MODULE_SET_STATUS:
 * If defined as TRUE:
 *     Major alarm will be asserted if power module set status is wrong.
 *     (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE must be TRUE in this case)
 * If defined as FALSE:
 *     Do nothing if power module set is wrong.
 */
#define SYS_CPNT_ALARM_OUTPUT_MAJOR_POWER_MODULE_SET_STATUS TRUE

/***********************************************
 * Minor Alarm output type related definitions *
 ***********************************************
 */
/* SYS_CPNT_ALARM_OUTPUT_MINOR_OVERHEAT:
 * Minor alarm will be set when all of the thermal temperature are larger
 * than the threshold.
 */
#define SYS_CPNT_ALARM_OUTPUT_MINOR_OVERHEAT TRUE

/* Fan Alarm: Major Alarm -> All of the fans are failed
 *            Minor Alarm -> At least one of the fans is failed
 */
#define SYS_CPNT_ALARM_OUTPUT_FAN_ALARM      TRUE
#endif
/* This constant defines if the ASIC support NET route
 */
#define SYS_CPNT_NET_ROUTE                  TRUE
#define SYS_CPNT_CHECK_HOST_TABLE_HIT_BIT   TRUE


/* This constant defines if the ASIC supports jumbo frame
 */
#define SYS_CPNT_JUMBO_FRAMES               TRUE

#define SYS_CPNT_ACL                        TRUE
#define SYS_CPNT_ACL_IPV6                   TRUE

#define SYS_CPNT_ACL_IP_EXT_TOS             (TRUE  && (SYS_CPNT_ACL == TRUE))
#define SYS_CPNT_ACL_IP_EXT_DSCP            (TRUE  && (SYS_CPNT_ACL == TRUE))
#define SYS_CPNT_ACL_IP_EXT_PREC            (TRUE  && (SYS_CPNT_ACL == TRUE))
#define SYS_CPNT_ACL_IP_EXT_ICMP            (TRUE  && (SYS_CPNT_ACL == TRUE))

#define SYS_CPNT_ACL_IPV6_EXT_SRC_IP_ADDR   (FALSE && (SYS_CPNT_ACL_IPV6 == TRUE))
#define SYS_CPNT_ACL_IPV6_EXT_DST_IP_ADDR   (TRUE  && (SYS_CPNT_ACL_IPV6 == TRUE))
#define SYS_CPNT_ACL_IPV6_EXT_TRAFFIC_CLASS (FALSE && (SYS_CPNT_ACL_IPV6 == TRUE))
#define SYS_CPNT_ACL_IPV6_EXT_FLOW_LABEL    (FALSE && (SYS_CPNT_ACL_IPV6 == TRUE))
#define SYS_CPNT_ACL_IPV6_EXT_NEXT_HEADER   (TRUE && (SYS_CPNT_ACL_IPV6 == TRUE))

/* IPv4 add to check VID field
 */
#define SYS_CPNT_ACL_IP_PLUS_VID             TRUE

/* time based acl */
#define SYS_CPNT_TIME_BASED_ACL             FALSE

/* Auto aggregate ACE before writting to chip.
 */
#define SYS_CPNT_ACL_AUTO_COMPRESS_ACE      TRUE

#define SYS_CPNT_ACL_COUNTER                TRUE

#define SYS_CPNT_ACL_MIRROR                 TRUE
#define SYS_CPNT_ACL_MIRROR_EGRESS          TRUE

#define SYS_CPNT_HRDRV                      FALSE
#define SYS_CPNT_L4                         TRUE
#define SYS_CPNT_MARKER                     TRUE
#define SYS_CPNT_MARKER_CONTAIN_CLI_SET_DOT1P_PRIORITY             TRUE
#define SYS_CPNT_MARKER_CONTAIN_CLI_SET_IP_DSCP                    TRUE
#define SYS_CPNT_MARKER_CONTAIN_CLI_SET_IP_PRECEDENCE              TRUE

#define SYS_CPNT_INGRESS_RATE_LIMIT_VIA_FILTER_ENGINE              FALSE



/* Define the auto baudrate support or not */
#define SYS_CPNT_AUTOBAUDRATE               FALSE

/* Michael 2004-04-08
 * The detection is for SDRAM size and type (number of banks) as outlined
 * in ES4625 H/W spec.  The purpose of it is to determine the max memory
 * size in order to calculate the location of UC.
 */
/* Define the auto DRAM detect support or not - Michael */
#define SYS_CPNT_SDRAM_AUTO_DETECTION       FALSE

/* Define the combo force mode support or not */
#define SYS_CPNT_COMBO_PORT_FORCE_MODE      TRUE

#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
#define SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED      TRUE
#endif

/* Define STKTPLG detect transceiver type according to EEPROM info.
 */
#define SYS_CPNT_STKTPLG_DETECT_SFP_TYPE    TRUE

/* This constant defines if "restore config from TFTP" should restart
 */
#define SYS_CPNT_XFER_RESTORE_CFG_RESTART   FALSE

/* This constant defines to support auto upgrade
 */
#define SYS_CPNT_XFER_AUTO_UPGRADE    TRUE

/* This constant defines Database Synchronisation (text form)
 */
#define SYS_CPNT_DBSYNC_TXT                 FALSE

/*  If it is TRUE, IGIF will be dynamically creatd by related evnets triiger,
 *    insteado of being created by vlan member add/delete
 */
#define SYS_CPNT_IGMPSNP_MVR_DYNAMIC_IGIF FALSE

/* IGMPSNP full querier
 * Note: 1) In original design, device will be Non-Querier when receive a query.
 *       2) In new design, device will check source ip of query. If source ip of
 *          query is lower than ours, device will be Non-Querier.
 *          Otherwise, device is still Querier.
 * Suggestion: Currently, 3Com and Enterasys request this feature.
 *             Other vendor don't have request. So please set to TRUE in project
 *             of 3Com and Enterasys. About other project, depend on your requirement.
 */
#define SYS_CPNT_IGMPSNP_QUERIER_FULL_SUPPORT       FALSE

/* IGMPSNP static system router port
 * Note: When user configure router port, they only need to input port number.
 *       And this port will become router port in each vlan.
 * Suggestion: This feature is requested by 3Com. And it need to modify APP.
 *             Except to project of 3Com, please set to FALSE unless your
 *             customer request this feature.
 */
#define SYS_CPNT_IGMPSNP_STATIC_SYSTEM_ROUTER_PORT  FALSE

/* Unknown multicast
 * Note: This compile option is used to define forwarding behavior of Unknown Mcast data packet.
 *
 *      1. SYS_CPNT_IGMPSNP_SELECT_FLOOD_UNKNOWN_MCAST_DATA_PACKET
 *          -- Unknown mcast data will be flooded to each port in same vlan.
 *
 *      2. SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT
 *          -- Unknown mcast data will be forward to router port in same vlan.
 *             Strata and Alllayer chip can't provide this feature.
 *             Currently, projects of 3com, Enterasys and Foundry require this feature.
 *
 *      3. SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM
 *          -- Unknown mcast data will be forward to router port in system.
 *             Due to chip's limitation, strata can't meet "SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT". So we use other feature of chip to provide similar function. Unknown mcast data packet will be forwarded to system router port.
 *             If your customer request forward unknown mcast data packet to router port and chip is strata, please use this one.
 *
 */
#define SYS_CPNT_IGMPSNP_SELECT_FLOOD_UNKNOWN_MCAST_DATA_PACKET                     1
#define SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT            2
#define SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT_PER_SYSTEM 3
#define SYS_CPNT_IGMPSNP_PROCESS_UNKNOWN_MCAST_DATA_PACKET  SYS_CPNT_IGMPSNP_SELECT_UNKNOWN_MCAST_DATA_PACKET_TO_ROUTER_PORT

/* Note: It is used to define how chip handle reserve multicast address(224.0.0.x).
 *       If it is defined, mean chip will trap data packet with reserve multicast address to CPU
 *       and won't forward to other port. In current project, no chip have this behavior.
 *       It is defined for future use. Please don't define it now.
 */
#define SYS_CPNT_IGMPSNP_RESERVE_ADDRESS_PACKET_CHIP_TRAP_TO_CPU_BUT_NOT_FORWARD     FALSE

/* Define chip's capability of Layer2 multicast
 */
/* Trunk reload balance
 * Note: This compile option is used to define how to do load balance in trunk port.
 *
 *      1. SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ONE_TRUNK_MEMBER
 *          -- MFLT will choose one trunk member and set it as Mcast forward port in Mcast Entry.
 *             Currently, strata use this one. Other chip may use this one.
 *             But please don't use this one in AllLayer.
 *
 *      2. SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ALL_TRUNK_MEMBER
 *          -- MFLT will choose set all trunk members in Mcast Entry. It is XGS's and AllLayer's behavior.
 *
 */
#define SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ONE_TRUNK_MEMBER   1
#define SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ALL_TRUNK_MEMBER   2
#define SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM   SYS_CPNT_L2MCAST_TRUNK_BALANCE_MECHANISM_SELECT_ALL_TRUNK_MEMBER

/* Unknown mcast forward
 * Note: Define whether chip support "Unknown Mcast Forward Port" feature. Currently, XGS and Strata support this feature.
 */
#define SYS_CPNT_L2MCAST_SUPPORT_SYSTEM_UNKNOWN_MCAST_FORWARD_PORT                         TRUE

/*
 * Define the MLDSNP support or not.
 */
#define SYS_CPNT_MLDSNP                       FALSE
#define SYS_CPNT_MLDSNP_QUERIER               FALSE
#define SYS_CPNT_MLDSNP_SUPPORT_V2            FALSE

/*Define Chip is using which table
*/
#define SYS_CPNT_MULTICAST_USING_IPMC         TRUE

/* Added by Steven.Gao in 2008/5/26 */
/*
 * Porting IGMPSNP from IPI nsm source code. All related macro is defined below.
 */
#define SYS_CPNT_IPI_IGMPSNP                FALSE
/* end of add by Steven.Gao in 2008/5/26 */

/*Define the igmpsnp multicast data forwarding priority
*/
#define SYS_CPNT_IGMPSNP_FORWARD_PRIORITY FALSE


/* Define Default route Action */
#define SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU                                          1
#define SYS_CPNT_DEFAULT_ROUTE_ACTION_ROUTE                                             2
#define SYS_CPNT_DEFAULT_ROUTE_ACTION_DROP                                              3


/*  For unknown destination packet switching action
 *  if there is not defined default gateway.
 *  ref. L3 chip action when use the constant:
 *       1 -- trap to CPU;
 *       2 -- switching to port
 *       3 -- drop the packet
 */
#define SYS_CPNT_L3_SWITCHING_UNKNOW_DST_WITHOUT_DFLT_GW_ACTION SYS_CPNT_DEFAULT_ROUTE_ACTION_TRAP2CPU
#define SYS_CPNT_L3_SWITCHING_NET_ROUTE_ACTION_TO_CPU_NOT_SUPPORT   TRUE


#define SYS_CPNT_RIP_CFG_TABLE_BASE_ON_RIF                      1
#define SYS_CPNT_RIP_CFG_TABLE_BASE_ON_VLAN                     2
#define SYS_CPNT_RIP_CFG_TABLE_BASE             SYS_CPNT_RIP_CFG_TABLE_BASE_ON_VLAN


#define SYS_CPNT_CPU_STORM                      TRUE

/* wuli, 2005-09-05 Add Constant for CPU interface configurations
 */
#define SYS_CPNT_CPU_INTERFACE_CPU_JOIN_VLAN    FALSE

/* wuli 2004-05-21, sync from Hagrid, shall be reviewed */
/* These constant defines 3Com features.
 */
#define SYS_CPNT_3COM_CLI                       FALSE
#define SYS_CPNT_3COM_CLI_ACCTON_CLI_BACKDOOR   FALSE
#define SYS_CPNT_3COM_CLI_PROVISION             FALSE

/* Note: SYS_CPNT_3COM_PASSWORD_RECOVERY is supported only
 * when SYS_CPNT_3COM_CLI_PROVISION is FALSE, *or*
 * SYS_CPNT_CFGDB and SYS_CPNT_USERAUTH_USER_IN_CFGDB is TRUE.
 */
#define SYS_CPNT_3COM_PASSWORD_RECOVERY         FALSE

#define SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB    FALSE

#define SYS_CPNT_3COM_TWO_OPCODES               FALSE

#define SYS_CPNT_3COM_LOOPBACK_TEST             FALSE
#define SYS_CPNT_3COM_COS_DEFAULT               FALSE
#define SYS_CPNT_3COM_BASIC_ACL                 FALSE

/* 3Com, ACE in an ACL must be with the same action,
 * this is different from Accton's ACL.
 */
#define SYS_CPNT_ACL_WITH_THE_SAME_ACTION       FALSE

/* 3Com, ACL mask is predeinfed, and can be configured by user,
 * it is not Accton's ACL.
 */
#define SYS_CPNT_3COM_ACL_MASK_PREDEFINED       FALSE

/* This is different from Accton's ACL.
 * In Accton's ACL mask, mask can be added and deleted,
 * but can't be modified.
 */
#define SYS_CPNT_3COM_ACL_MASK_MODIFICATION     FALSE


/* the following constants are used to define the method to trap packet that DA is
 * CPU MAC address.
 */
#define SYS_CPNT_AMTR_CPU_INTERVENTION_CPU_JOIN_VLAN    1
#define SYS_CPNT_AMTR_CPU_INTERVENTION_FFP_TRAP         2

/* the CSC shall use this constant in the code to specify the method
 */
#define SYS_CPNT_AMTR_CPU_INTERVENTION_METHOD           SYS_CPNT_AMTR_CPU_INTERVENTION_FFP_TRAP


/* Constant for the component BUFFERMGMT to manage the landing buffer.
 * TRUE  if the component BUFFERMGMT is defined, else FALSE.
 * purpose of landing beffer: for driver layer(ex.FS) could access a static allocated buffer
 * instead of dynamically allocate huge memory
 * Ericali 2004-09-01   After BLANC v3.0.0.69
 */
#define SYS_CPNT_BUFFERMGMT                             TRUE

#define SYS_CPNT_MAINBOARD                              TRUE

/* the follwing constants are usd to define which mechanism is used for master button */
#define SYS_CPNT_MASTER_BUTTON_NONE                  0
#define SYS_CPNT_MASTER_BUTTON_SOFTWARE              1
#define SYS_CPNT_MASTER_BUTTON_HARDWARE              2

#define SYS_CPNT_MASTER_BUTTON     SYS_CPNT_MASTER_BUTTON_NONE

/* SYS_CPNT_SWDRV_MMU_CFG_OVERSUBSCRIBE_CONFIG :
 * Under FULL MESH testing, if the packet lost is not allowed, each queue needs more packet buffer to
 * hold packets under burst traffic (one of congestion case) condition.
 * To pass mesh testing, we may need to OVERSUBSCRIBE the packet buffer size for each queue if the
 * total number of packet buffer is too small, such as BCM5690 only have 128K bytes packet per port
 * (for 8 queues).
 * This configuration WILL ALLOW a given queue to use all available packet buffer of a given port.
 * This MAY cause high priority packet get drop when all packet buffer are locked by low priority
 * traffic under heavy congestion environment ...
 * Note: 1) For stacking switch, we need to guarantee that High priority control packet must NOT be
 *          dropped under any traffic storm. So, we shall not OVERSUBSCRIBE the MMU configuration for
 *          stackable product line.
 *       2) For 10G switch, we can OVERSUBSCRIBE the MMU configuration if pass the FULL MESH test is a
 *          major requirement.
 *
 * SYS_CPNT_SWDRV_MMU_CFG_NORMAL_CONFIG :
 * This configuration will limit the total number of packet buffer for each queue.
 * The sum of total buffer of each queue will be equal to total number of packet available for a port.
 * Under this configuration, each queue will get guarantee packet buffer for that priority traffic.
 * However, this also limit the packet buffer can be used per each queue.
 * Under this configuration, each queue will not be able to hold too many packets under burst traffic
 * (one of congestion case) condition. Under FULL MESH test, we will see packet lost understand this
 * configuration.
 * Note: For stacking switch, we need to guarantee that High priority control packet must NOT be
 * dropped under any traffic storm. So, we shall use NORMAL the MMU configuration for stackable
 * product line.
 */
#define SYS_CPNT_SWDRV_MMU_OVERSUBSCRIBE_CONFIG     1
#define SYS_CPNT_SWDRV_MMU_NORMAL_CONFIG            2

#define SYS_CPNT_SWDRV_MMU_CFG_MODE                 SYS_CPNT_SWDRV_MMU_NORMAL_CONFIG

/* wuli, 2004-11-05
 * BCM5690 errata:
 * Workaround:
 * usage:
 * define SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND to TRUE to FFP for Workaround
 * define SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND_MODE to specify L2 product or L3 product
 */
/* Wingson 2004-11-11, add for dev_hrdrv.c to differ L2 from L3 mode
 */
#define SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND       FALSE

#if (SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND == TRUE)
#define SYS_CPNT_FFP_CPU_PRIORITY_L2_MODE           1
#define SYS_CPNT_FFP_CPU_PRIORITY_L3_MODE           2
#define SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND_MODE   SYS_CPNT_FFP_CPU_PRIORITY_L2_MODE
#endif   /* End of SYS_CPNT_FFP_CPU_PRIORITY_WORKAROUND */

#define SYS_CPNT_PACKET_TOCPU_INTERFACE_XGS3        TRUE

/* Automatic Device Detection (ADD)
 */
#define SYS_CPNT_ADD                            FALSE

/* This defines whether Automatic Device Detection (ADD)
 * supports detection via Link Layer Discovery Protocol (LLDP)
 * as defined by IEEE 802.1ab, in the UI and the
 * core layer.
 */
#define SYS_CPNT_ADD_SUPPORT_LLDP               FALSE

/* This defines whether Automatic Device Detection (ADD)
 * supports configurable 802.1p priority in the UI and the
 * core layer.
 */
#define SYS_CPNT_ADD_CONFIGURABLE_PRIORTIY      FALSE

/* This is defined the ADD shall detect IP phone traffic on the native VLAN only or
 * for all VLAN. And auto join the monitored port to voice VLAN when detected a IP
 * phone plug in.
 *
 * If it is TRUE, the ADD shall detect IP phone traffic on native VLAN.
 * Else the ADD shall detect IP phone traffic for all VLAN.
 */
#define SYS_CPNT_ADD_DETECT_NATIVE_VLAN_ONLY    FALSE

/* 565x support DDR ram test
 */
#define SYS_CPNT_DDR_TEST                      TRUE


/*support when this is true it will not run diag after reload
 */
#define SYS_CPNT_SUPPORT_QUICKBOOT                 FALSE

/* Zhong Qiyao 2007.07.19:
 * The following does not look like they belong to SYS_CPNT.
 */
#if 0
/* Constant for the component L3 Driver to manage the packets to CPU.
 * 2005-7-1, vai.wang
 */
#define SYS_CPNT_PKT2CPU_L3SLOWPATH             1   /* L3SlowPathToCPU */
#define SYS_CPNT_PKT2CPU_ICMPREDIRECT           2   /* IcmpRedirectToCPU */
#define SYS_CPNT_PKT2CPU_IPMCTTLERROR           3   /* IpmcTtlErrorToCPU */
#define SYS_CPNT_PKT2CPU_L3UCTTLERROR           4   /* L3UCTtlErrorToCPU */
#define SYS_CPNT_PKT2CPU_DOSATTACK              5   /* DosAttackToCPU */
#define SYS_CPNT_PKT2CPU_TUNNELERROR            6   /* TunnelErrorToCPU */
#define SYS_CPNT_PKT2CPU_MARTIANADDR            7   /* MartianAddrToCPU */
#define SYS_CPNT_PKT2CPU_V6L3ERROR              8   /* V6L3ErrorToCPU */
#define SYS_CPNT_PKT2CPU_SRCROUTE               9   /* SrcRteToCPU */
#define SYS_CPNT_PKT2CPU_IPMCPORTMISS           10  /* IpmcPortMissToCPU */
#define SYS_CPNT_PKT2CPU_V4L3DSTMISS            11  /* V4L3DstMissToCPU */
#define SYS_CPNT_PKT2CPU_V6L3DSTMISS            12  /* V6L3DstMissToCPU */
#define SYS_CPNT_PKT2CPU_UNRESOLVEDL3SRC        13  /* UnresolvedL3SrcToCPU */
#define SYS_CPNT_PKT2CPU_UNKNOWNVLAN            14  /* UnknownVlanToCPU */
#define SYS_CPNT_PKT2CPU_IPMCERROR              15  /* IpmcErrorToCPU */
#define SYS_CPNT_PKT2CPU_V4L3ERROR              16  /* V4L3ErrorToCPU */
#define SYS_CPNT_PKT2CPU_UNKNOWNUCAST           17  /* UnknownUcastToCPU */
#define SYS_CPNT_PKT2CPU_UNKNOWNMCAST           18  /* UnknownMcastToCPU */
#define SYS_CPNT_PKT2CPU_UNKNOWNIPMC            19  /* UnknownIpmcToCPU */
#define SYS_CPNT_PKT2CPU_NONSTATICSTATIONMOVE   20  /* NonStaticStationMoveToCPU */
#define SYS_CPNT_PKT2CPU_STATICSTATIONMOVE      21  /* StaticStationMoveToCPU */
#endif

#define SYS_CPNT_EFM_OAM                                FALSE
#if (SYS_CPNT_EFM_OAM == TRUE)
/* perform remote loopback actively, i.e. permit to send loopback control OAMPDUs
 */
#define SYS_CPNT_EFM_OAM_REMOTE_LB_ACTIVELY             TRUE

/* accept remote loopback passively, i.e. react to loopback control OAMPDUs
 */
#define SYS_CPNT_EFM_OAM_REMOTE_LB_PASSIVELY            FALSE

/* To specify how to trap loopback pdu as CO.
 * TRUE  - by rule
 * FALSE - by chip register
 */
#define SYS_CPNT_EFM_OAM_REMOTE_LB_ACTIVELY_BY_RULE     TRUE

/* To specify how to trap loopback pdu as CPE.
 * TRUE  - by rule
 * FALSE - by chip register
 */
#define SYS_CPNT_EFM_OAM_REMOTE_LB_PASSIVELY_BY_RULE    FALSE

#define SYS_CPNT_EFM_OAM_ORG_SPEC                       FALSE
#endif

/* perform remote loopback actively, i.e. permit to send loopback control OAMPDUs
 */
#define SYS_CPNT_EFM_OAM_REMOTE_LB_ACTIVELY             (FALSE && SYS_CPNT_EFM_OAM)

#if defined(SYS_CPNT_EFM_OAM_ORG_SPEC) && (SYS_CPNT_EFM_OAM_ORG_SPEC == TRUE)
/* OAM organization specific feature
 * Note: OAM org. spec. feature can be extended by vendor.
 *       The org. spec. OAMPDU must be recognized by switch and CPE.
 *       In order to operate with different CPE, switch must be indicated which CPE type used.
 */
#define SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE    1
#define SYS_CPNT_EFM_OAM_ORG_SPEC_CPE_STYLE  SYS_CPNT_EFM_OAM_ORG_SPEC_ACCTON_CPE
#endif

#define SYS_CPNT_INTERNAL_LOOPBACK_TEST     TRUE

/* RESETMGMT component deals with the H/W reset button
 * if the time that the reset button is being pressed/held is T seconds,
 * T < SYS_ADPT_RESETMGMT_RESET_TIME --> no reset
 * T >= SYS_ADPT_RESETMGMT_RESET_TIME && T < SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME --> reset the system
 * T >= SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME --> reset with factory defaults
 */
#define SYS_CPNT_RESETMGMT          FALSE

#define SYS_CPNT_I2C                                 TRUE
#define SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API TRUE
#if (SYS_CPNT_I2C == TRUE)
#define SYS_CPNT_SELECT_CHANNEL_MODE_NOCHANNEL               0
#define SYS_CPNT_SELECT_CHANNEL_MODE__PCA9548                1
#define SYS_CPNT_SELECT_CHANNEL_MODE__EPLD                   2
#define SYS_CPNT_SELECT_CHANNEL_MODE__PCA9544                3
#define SYS_CPNT_SELECT_CHANNEL_MODE__EPLD_VIA_I2C           4
#define SYS_CPNT_SELECT_CHANNEL_MODE__GPIO                   5
#define SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX    6
#define SYS_CPNT_SELECT_CHANNEL_MODE__HWCFG_API              7

#define  SYS_CPNT_SELECT_CHANNEL_MODE  SYS_CPNT_SELECT_CHANNEL_MODE__I2C_MULTI_LEVEL_MUX
#endif

/* ES4626F-38 combo port i2c slave address is defined by HW
 * we define a new cnpt to separate the code
 */
#define SYS_CPNT_COMBO_PORT_I2C_BY_HW        TRUE

#define SYS_CPNT_NOTIFICATIONLOG_MIB  TRUE

/*the 802.1ag protocol*/
#define SYS_CPNT_CFM                                        FALSE
#define SYS_CPNT_CFM_DELAY_MEASUREMENT                      TRUE

/* for AGG test case,
 *   TRUE to forward Y.1731 PDUs which are not for us.
 */
#define SYS_CPNT_CFM_FORWARD_Y1731_PDU_TRANSPARENTLY        TRUE

/* TRUE  - ma name is unique per md
 * FALSE - ma name is unique per system
 */
#define SYS_CPNT_CFM_MA_NAME_UNIQUE_PER_DOMAIN              TRUE

/* TRUE  - UP MEPs use port mac
 * FALSE - UP MEPs use CPU mac
 */
#define SYS_CPNT_CFM_UP_MEP_USE_PORT_MAC                    TRUE

/*end 802.1ag*/

#define SYS_CPNT_POE                FALSE
#define SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0        10        /* implementation is not finished */
#define SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2        32
#define SYS_CPNT_POE_PSE_DOT3AT                  SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2 /*although we does not support poe, we can record it if we have ablilty*/

/* SYS_CPNT_POE_MAX_ALLOC_FIXED defines if the overall
 * max allocation power is fixed. If yes, you can't
 * configure the overall max allocation power.
 */
#define SYS_CPNT_POE_MAX_ALLOC_FIXED             (FALSE && SYS_CPNT_POE)

#define SYS_CPNT_POE_SHOW_POWER_STYLE            (FALSE && SYS_CPNT_POE)

#define SYS_CPNT_LEDDRV_LOAD_CODE_TO_LED_DISPLAY_ASIC FALSE
#if (SYS_CPNT_LEDDRV_LOAD_CODE_TO_LED_DISPLAY_ASIC == TRUE)
/* Define the LED display by ASIC or not?
 */
#define SYS_CPNT_LEDDRV_DISPLAY_BY_ASIC  TRUE
#endif

/* Support IPC msgq flow ctrl
 */
#define SYS_CPNT_MSG_FLOW_CTL       TRUE

/* ATC Storm
 */
#define SYS_CPNT_ATC_STORM                          FALSE
#if (SYS_CPNT_ATC_STORM == TRUE)
#define SYS_CPNT_ATC_BSTORM                         TRUE
#define SYS_CPNT_ATC_MSTORM                         TRUE

/* ATC Broadcast Storm
 */
#if (SYS_CPNT_ATC_BSTORM == TRUE)
#define SYS_CPNT_ATC_BSTORM_SUPPORT_LPORT           FALSE
#define SYS_CPNT_ATC_BSTORM_RATE_PER_PORT_CTRL      TRUE
#define SYS_CPNT_ATC_BSTORM_ENABLE_PER_PORT         TRUE
#endif

/* ATC Multicast Storm
 */
#if (SYS_CPNT_ATC_MSTORM == TRUE)
#define SYS_CPNT_ATC_MSTORM_SUPPORT_LPORT           FALSE
#define SYS_CPNT_ATC_MSTORM_RATE_PER_PORT_CTRL      TRUE
#define SYS_CPNT_ATC_MSTORM_ENABLE_PER_PORT         TRUE
#endif
#endif


#define SYS_CPNT_SYSCTRL_XOR        TRUE

#define SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE  FALSE

/* This definition is used for STORM support mode
 * If no support any storm, please set to SYS_CPNT_STORM_MODE to SYS_CPNT_STORM_NONE,
 * else please combine your support items with '|' in SYS_CPNT_STORM_MODE
 */
#define SYS_CPNT_STORM_NONE               0   /* BIT-0 */
#define SYS_CPNT_STORM_BSTORM             1   /* BIT-1 */
#define SYS_CPNT_STORM_MSTORM             2   /* BIT-2 */
#define SYS_CPNT_STORM_UNKNOWN_USTORM     4   /* BIT-3 */
#define SYS_CPNT_STORM_MODE               (SYS_CPNT_STORM_BSTORM | SYS_CPNT_STORM_MSTORM | SYS_CPNT_STORM_UNKNOWN_USTORM)

/* For Power-save
 */
#define SYS_CPNT_POWER_SAVE                 FALSE

/* Remote Switched Port Analyzer (RSPAN)
 * TRUE, meaning to enable the RSPAN function in the project.
 * This only works for standalone project by now.
 */
#define SYS_CPNT_RSPAN              TRUE
#define SYS_CPNT_RSPAN_FILTER_VLAN  FALSE

/* To avoid making confused, MTU setting is either per system or per port.
 *  if chip can support per port setting, should select SYS_CPNT_SWCTRL_MTU_PER_PORT
 */
#define SYS_CPNT_SWCTRL_MTU_PER_SYSTEM          1
#define SYS_CPNT_SWCTRL_MTU_PER_PORT        2
#define SYS_CPNT_SWCTRL_MTU_NOT_CONFIGURABLE    3
#define SYS_CPNT_SWCTRL_MTU_CONFIG_MODE         SYS_CPNT_SWCTRL_MTU_PER_PORT
#define SYS_CPNT_UDP_HELPER                     FALSE
#define SYS_CPNT_DAI                            FALSE
#define SYS_CPNT_DAI_IP_VALIDATE_ALLOW_ZEROS    TRUE

/* To specify combo port media preferred auto is implemented by
 * hardware or software.
 *
 * TRUE  - Auto media selection is performed by hardware.
 *
 * FALSE - Auto media selection is performed by software.
 *         SWDRV always set combo port to force mode according to
 *         SFP transceiver present state when user config media type of
 *         combo port to auto mode.
 *         It also implies SFP transceiver
 *         present state monitoring need to do.
 */
#define SYS_CPNT_SWDRV_COMBO_PORT_MEDIA_PREFERRED_AUTO_BY_HW    FALSE

/* To specify how to trap packets to CPU.
 *
 * TRUE - trap packtes by rule
 * TRUE - trap packets by chip register.
 */
#define SYS_CPNT_SWDRV_TRAP_IGMP_BY_RULE                    TRUE
#define SYS_CPNT_SWDRV_TRAP_MLD_BY_RULE                     TRUE
#define SYS_CPNT_SWDRV_TRAP_UNKNOWN_IPMC_BY_RULE            TRUE
#define SYS_CPNT_SWDRV_TRAP_UNKNOWN_IPV6MC_BY_RULE          TRUE

/* To specifiy the format of packet delivered between
 * LAN and chip SDK.
 *
 * SYS_CPNT_LAN_FIXED_TPID
 *     TRUE  - LAN always receives/sends packets with TPID 0x8100.
 *     FALSE - LAN receives/sends packets with actual TPID.
 */
#define SYS_CPNT_LAN_FIXED_TPID                     FALSE

/* Workaround if intrusion identification cannot work.
 *     TRUE  - enable workaround, LAN will identify received
 *             packets as an intruder by look up L2 table.
 *     FALSE - no need workaround, LAN will identify
 *             packets as an intruder by reason.
 */
#define SYS_CPNT_LAN_WORKAROUND_FOR_INTRUSION       TRUE


#define SYS_CPNT_MONITORING_PROCESS_CPU             TRUE

/* Support the systemwide counter, NMTR is responsible for clearing counter behavior.
 */
#define SYS_CPNT_SYSTEMWIDE_COUNTER                 TRUE

#define SYS_CPNT_10G_MODULE_SUPPORT                 FALSE     /* for support 10G module used */

#define SYS_CPNT_40G_MODULE_SUPPORT                 TRUE      /* for support 40G module used */

/* Using swdrv to monitor the EEPROM info of SFP instead
 * of monitored by stktplg.
 */
#define SYS_CPNT_SWDRV_MONITOR_SFP_DDM          TRUE

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
#define SYS_CPNT_SFP_DDM_ALARMWARN_TRAP         TRUE
#endif

/* Synchronous Ethernet
 */
#define SYS_CPNT_SYNCE FALSE

#if (SYS_CPNT_SYNCE == TRUE)

#define SYS_CPNT_SYNCE_SUPPORT_MEDIA_1000BASE_T    (0x01<<0)
#define SYS_CPNT_SYNCE_SUPPORT_MEDIA_10G_XFP       (0x01<<1)

/* SYS_CPNT_SYNCE_SUPPORT_MEDIA is a bitmap to indicate the port media types
 * that support synce
 */
#define SYS_CPNT_SYNCE_SUPPORT_MEDIA SYS_CPNT_SYNCE_SUPPORT_MEDIA_10G_XFP
#endif

/* for PPPoE intermediate agent
 */
#define SYS_CPNT_PPPOE_IA                               FALSE
/* TRUE for MO platform,
 *   bcz it has problem to install default rule on trunk now,
 *   change to use global rule temporarily.
 */
#define SYS_CPNT_PPPOE_IA_TRAP_PPPOED_BY_GLOBAL_RULE    FALSE

/* for IPv6 RA-Guard
 *
 * SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_GLOBAL
 *      TRUE  - Packet is trapped by global register/rule
 *      FALSE - Packet is trapped by per port register/rule
 * SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_REG
 *      TRUE  - Packet is trapped by register
 *      FALSE - Packet is trapped by rule
 * SYS_CPNT_IPV6_RA_GUARD_SW_RELAY
 *      TRUE  - Use SW relay to forward packets
 *      FALSE - No SW relay function is needed
 * SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE
 *      TRUE  - Packet is     dropped by per port rule
 *      FALSE - Packet is not dropped by per port rule
 */
#define SYS_CPNT_IPV6_RA_GUARD                      FALSE
#define SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_GLOBAL       FALSE
#define SYS_CPNT_IPV6_RA_GUARD_TRAP_BY_REG          FALSE
#define SYS_CPNT_IPV6_RA_GUARD_SW_RELAY             FALSE
#define SYS_CPNT_IPV6_RA_GUARD_DROP_BY_RULE         FALSE

/* for UDLD (UniDirectional Link Detection)
 *  SYS_CPNT_UDLD_TRAP_UDLD_BY_GLOBAL_RULE
 *      TRUE    - Packet is trapped by global rule
 *      FALSE   - Packet is trapped by per port rule
 *  SYS_CPNT_UDLD_PDU_FORMAT
 *      ACCTON  - should be used as default if this option is not defined.
 *      CISCO   - refer to RFC 5171.
 */
#define SYS_CPNT_UDLD                               FALSE
#define SYS_CPNT_UDLD_TRAP_UDLD_BY_GLOBAL_RULE      TRUE
#define SYS_CPNT_UDLD_PDU_FORMAT_ACCTON             1
#define SYS_CPNT_UDLD_PDU_FORMAT_CISCO              2
#define SYS_CPNT_UDLD_PDU_FORMAT                    SYS_CPNT_UDLD_PDU_FORMAT_ACCTON

/* for ifXEntry.ifAlias
 *
 * SYS_CPNT_SWCTRL_PORT_DESCRIPTION_MIRROR_TO_IF_ALIAS
 *   To mirror portEntry.portName to ifXEntry.ifAlias or not
 *
 * SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS
 *   support CLI command 'alias' or not
 *
 * ACPv4 and former behvaior is
 *   mirror portEntry.portName to ifXEntry.ifAlias and
 *   not support CLI command 'alias'
 *
 * Simba platform from SHLinux vob
 *   not mirror portEntry.portName to ifXEntry.ifAlias and
 *   support CLI command 'alias'
 */
#define SYS_CPNT_SWCTRL_PORT_DESCRIPTION_MIRROR_TO_IF_ALIAS     TRUE
#define SYS_CPNT_SWCTRL_CONTAIN_CLI_ALIAS                       FALSE

/* for MEF9: Discard L2CP
 *   5.1.1 ~ 5.1.3
 *   25.1.1 ~ 25.1.3
 *   5.2.1 ~ 5.2.3
 *   25.2.1 ~ 25.2.3
 */
#define SYS_CPNT_ACL_DENY_DISCARD_ALL_PACKET_FOR_MEF_9_DISCARD_L2CP         FALSE

/* for FS
 */
#define SYS_CPNT_FS_DO_NOT_USE_PART_TABLE_MTD           TRUE
#define SYS_CPNT_FS_SUPPORT_WRITE_RUNTIME_TO_FILESYSTEM TRUE
#define SYS_CPNT_FS_SUPPORT_EXT3                        TRUE
#define SYS_CPNT_FS_USE_SCRIPT_TO_MOUNT_USBDISK         TRUE
/* If SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION is defined as TRUE:
 * runtime cache partition will be used to reduce boot time.
 *
 * It takes much time to load runtime data from a ext3 filesystme in
 * uboot. In order to reduce boot time, an additional
 * partition(i.e. runtime cache partition) is created to keep the startup
 * runtime image.
 * Uboot(loader) will try to load startup runtime image
 * from the runtime cache partition if it is valid.
 * In runtime, FS will copy the startup runtime image to the runtime cache
 * partition when the startup runtime is changed.
 * Note: When SYS_CPNT_ONIE_SUPPORT is defined as TRUE, it means the uboot
 *       to be used is from ONIE. ONIE uboot does not contain the code of the
 *       cache partition design, so SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION
 *       must be FALSE when SYS_CPNT_ONIE_SUPPORT is TRUE.
 */
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
#define SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION     FALSE
#else
#define SYS_CPNT_FS_SUPPORT_RUNTIME_CACHE_PARTITION     TRUE
#endif
/* SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT
 *     TRUE : use script file to get the mtd id by its mtd name
 *     FALSE: use naming constant SYS_ADPT_MTD_ID_XXX to get the mtd id.
 */
#define SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT FALSE

/* SYS_CPNT_FS_SUPPORT_DATA_STORAGE
 * FS data storage service provides non-volatile data storage service for the
 * data that would like to be hidden from the end users on open linux system(i.e. SYS_CPNT_ONIE_SUPPORT is TRUE)
 */
#define SYS_CPNT_FS_SUPPORT_DATA_STORAGE FALSE

#if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE)
/* SYS_CPNT_FS_HWINFO_FROM_ONLP
 * Get the hardware info from onlp library through the utility program onlp_query
 */
#define SYS_CPNT_FS_HWINFO_FROM_ONLP TRUE
#endif

/* for 802.1Qbb, Priority-based Flow Control
 */
#define SYS_CPNT_PFC        TRUE

/* 802.1Qau congestion notification feature */
#define SYS_CPNT_CN         TRUE

/* 802.1Qaz Enhanced Transmission Selection
 */
#define SYS_CPNT_ETS        TRUE

#define SYS_CPNT_DCB_GROUP  ((SYS_CPNT_DCBX == TRUE) || (SYS_CPNT_PFC == TRUE) || \
                             (SYS_CPNT_CN == TRUE) || (SYS_CPNT_ETS == TRUE))

/* Per Queue counters
 */
#define SYS_CPNT_NMTR_PERQ_COUNTER                      TRUE

/* for mac notification trap */
#define SYS_CPNT_AMTR_MAC_NOTIFY                TRUE

/* To allow shutdown trunk member severally
 */
#define SYS_CPNT_SWCTRL_SHUTDOWN_FOR_TRUNK_MEMBER           TRUE

/* Control the specific packet without ACL.
 * For example, control CDP packet to not forward to next switch.
 */
#define SYS_CPNT_APP_FILTER                       FALSE
#if (TRUE == SYS_CPNT_APP_FILTER)
#define SYS_CPNT_APP_FILTER_CDP                   TRUE
#define SYS_CPNT_APP_FILTER_PVST                  TRUE
#endif /* #if (TRUE == SYS_CPNT_APP_FILTER) */

/* SYS_CPNT_IF_LINK_TRAP_PORT_BASE: the constant to indicate the
 * if_mgr provides the port-base configuration for the link up/down
 * trap of the if_mgr.
 * TRUE     -- port-base configuration for the link up/down trap
 * FALSE    -- global setting for the link up/down trap
 */
#define SYS_CPNT_IF_LINK_TRAP_PORT_BASE             TRUE

#define SYS_CPNT_MLAG   TRUE

#define SYS_CPNT_SYS_CALLBACK_SOCKET    TRUE /* sys_callback supports sending msg via Unix Domain Socket */

/* Driver Process to support debug commands from UNIX Domain Socket input in Linux shell mode
 *  * -- without the need of Simba CLI
 *   */
#define SYS_CPNT_DRVP_DBG     TRUE

#define SYS_CPNT_QOS_UI      TRUE
#define SYS_CPNT_AMTR_UI     TRUE
#define SYS_CPNT_ACL_UI      TRUE
#define SYS_CPNT_HTTP_UI     TRUE
#define SYS_CPNT_TRUNK_UI    TRUE
#define SYS_CPNT_FS_UI       TRUE
#define SYS_CPNT_XSTP_UI     TRUE
#define SYS_CPNT_MGMT_IP_FLT_UI TRUE

#define SYS_CPNT_NETCFG_IP_SYNC     FALSE /* whether to sync OM from kernel to AOS */
#define SYS_CPNT_NETCFG_IP_REDIRECT TRUE /* whether to redirect SOCKET IO ctrl on ip and netmask setting to AOS via usermode helper process */

/* QSFP+ DAC breakout cable support
 */
#define SYS_CPNT_HW_PROFILE_PORT_MODE   TRUE

#define SYS_CPNT_NETOPEER    TRUE

#define SYS_CPNT_VXLAN       TRUE
#define SYS_CPNT_VXLAN_SYSTEM_WISE_VLAN_VNI_MAPPING   1
#define SYS_CPNT_VXLAN_PER_PORT_AND_PER_PORT_PER_VLAN 2
#define SYS_CPNT_VXLAN_ACCESS_PORT_MODE SYS_CPNT_VXLAN_PER_PORT_AND_PER_PORT_PER_VLAN

#define SYS_CPNT_PBR         TRUE    /* Policy-based routing */

#define SYS_CPNT_HASH_SELECTION    TRUE
#define SYS_CPNT_ECMP_BALANCE_MODE  TRUE

#define SYS_CPNT_SYS_CALLBACK_ENHANCEMENT   TRUE

#endif  /* SYS_CPNT_H */
/* End of sys_cpnt.h */
