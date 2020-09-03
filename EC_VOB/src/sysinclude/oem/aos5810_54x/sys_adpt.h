#ifndef  SYS_ADPT_H
#define  SYS_ADPT_H


/*----------------------------------------------------------------------------
 * Package: SYS_ADPT.H
 * Purpose: This package defines the system adaptation value for the project.
 * Note: The naming constant defined in this package shall be reused
 *        by all the BNBU L2/L3 switch projects .
 *  History
 *
 *   ryan     07/16/2001      new created
 *   jjyoung  10/29/2002      modified for Alps (XGS) project
 *   Jason    01/13/2003      Modify OID for ES4624C-ZZ and Manufacture Nam*   Jason    02/10/2003      Add two constants for MAU MIB supporting
 *   Jason    02/14/2003      Add constants for supporting Protocol based VLAN
 *   Zhong QY 12-08/2003      Synchronised from ES4649-32 (Hagrid)
 *   wuli     2004-06-01      Add defination fr internal management VLAN
 *   wuli     2004-06-15      Add constant for Diffsrv
 *   Zhong QY       04-30/2007
 *      Moved to be based on ACPv3/main/ES4649.
 *
 * ------------------------------------------------------------------------
 * Copyright(C)							  	Accton Technology Corp. , 2007
 * ------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#include "leaf_dvmrp.h"
#include "leaf_1850.h"
#include "leaf_2933.h"
#include "leaf_2934.h"
#include "leaf_3413t.h"
#include "leaf_ieeelldp.h"
#include "leaf_es3626a.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Note that SYS_ADPT_PROJECT_ID must be the same with the definition of
 * "PROJECT_ID" in $ACCPROJ/utils/build_env_init
 */
//#define SYS_ADPT_PROJECT_ID 472

/* Define extra stack size for platform.
 * Currently,
 * PowerPC+Broadcom:     0 KBytes (default platform)
 * MIPS+Broadcom:        0 KBytes
 * Marvell(ARM9):        16 KBytes
 * Intel(X86-64):        8 MBytes
 */
#define SYS_ADPT_EXTRA_TASK_STACK_SIZE      (8 * SYS_TYPE_1M_BYTES)

/* Define the max numner of units can be stacked together, and
 * the max number of port can be installed in a unit.
 * Note: 1. The adaptation value, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK, is limited/determined
 *          by the system resources and customer requirements.
 *       2. The adaptation value, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT, will be 8*N, and
 *          will be determined by
 *          (the port numbers of base unit + port numbers of optioal slot * number of slots) + padding
 *       3. The optional module can be a 1-port or 2-port module. So,
 *          for 24+2 family, the SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT will be (24 + 2 * 2) + 4 = 32.
 *          for 24+2 family, the SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT will be (48 + 2 * 2) + 4 = 56.
 *       4. The port nmber of each optional slot is fixed and predefined. The total number of ports
 *          for each slot will be determined by the optional moudle with max port supported.
 *          For 24+2 model with 1-port or 2-port module supported,
 *              - the port number of 1st optional solt will be 25 and 26, and
 *              - the port number of 1st optional solt will be 27 and 28.
 *          For 48+2 model with 1-port or 2-port module supported,
 *              - the port number of 1st optional solt will be 49 and 50, and
 *              - the port number of 1st optional solt will be 51 and 52.
 *       5. The number of LED should be displayed is predefined by hardware.
 *          For 24+2 model, there will be 26 set of LED needs to be taken care by software
 *       6.  "SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK" defines the maximum number of units can be stacked together.
 *          The so-called "unit" are the physical device(e.g. a L3 switch) recognized by normal end users.
 *       7.  "SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK" defines the maximum number of driver units can be stacked together.
 *          The so-called "driver units" are the entity recognized in the view of a driver layer. For example,
 *          there are two driver units on a unit in BLANC_R2, one is mainboard and the other one is expansion module.
 *          Reference "STKTPLG_OM_GetNextDriverUnit()" for the concept of DRIVER UNIT.
 *
 *          Example:
 *          If we have an 1-port module installed in slot 1, and a two port mdule installed in solt 2,
 *          for 24+2 model, the port number of optional modules will be 25, 27, and 28, where 26 is not used.
 *
 *          if we only have an 1-port module installed in slot 2, again for 24+2 model,
 *          the port number of optional module will be 27, where 25, 26, and 28 are not used.
 */
#define SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK                      1   /* Max number of units can be stacked togather   */
#define SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK               1   /* Max number of driver units */
#define SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT                       160 /* Max number of port can be installed in a unit */

/* If SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD is greater than 49., the size of HBT1 used in stktplg
 * (i.e. sizeof(STKTPLG_OM_HBT_0_1_T)) will execeed the limitation of the maximum size of the payload
 * allowed in ISC. Need to further reduce the size of STKTPLG_OM_HBT_0_1_T
 * under this situation.
 */
#define SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD              160 /* Max number of port can be installed in a unit */

/*!!! to ask Charlie or Echo
 *!!! Gordon's version is (2, 8). <- (0, ) does not work
 *!!!   maybe because of new StkTplg
 *!!! Galaxy ES4626H is (0, 0). <- will be our target
 *!!! server is (4, 0).
 */
#define SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT                     1   /* Max number of module can be installed in a unit */
#define SYS_ADPT_MAX_NBR_OF_PORT_PER_MODULE                     0   /* Max number of port per module */

#define SYS_ADPT_MAX_NBR_OF_10G_MODULE_PORT                     2

/* wuli, 2005-03-21 add */
#define SYS_ADPT_MAX_NBR_OF_STACKING_PORTS                      0
#define SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD                  SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD

/* Define switch clustering parameters
 */
#define SYS_ADPT_CLUSTER_MAX_NBR_OF_MEMBERS                     16  /* limitation of cluster members */
#define SYS_ADPT_CLUSTER_MAX_NBR_OF_CANDIDATES                  100 /* limitation of cluster candidates */

/* Zhong Qiyao, 2004.06.11, changed from 4 to 5, because total includes 5670 */
#define SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT                       1   /* Max number of chip can be installed in a unit */
#define SYS_ADPT_MAX_NBR_OF_SWITCH_PORT_PER_CHIP                134  /* Max number of switch port can be installed in a chip */

#define SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT                       160 /* Number of sfp ports in a unit */
#define SYS_ADPT_1ST_STACKING_PORT_NBR_PER_UNIT                 500 /* First stacking port's user port number in a unit; ES3526V used */
#define SYS_ADPT_1ST_PORT_NBR_OF_1ST_OPTION_SLOT_OF_24_MODEL    0
#define SYS_ADPT_1ST_PORT_NBR_OF_2ND_OPTION_SLOT_OF_24_MODEL    (SYS_ADPT_1ST_PORT_NBR_OF_1ST_OPTION_SLOT_OF_24_MODEL + 2)


#define SYS_ADPT_TOTAL_NBR_OF_LPORT                             (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK *       \
                                                                 SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT  +       \
                                                                 SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
#define SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST           ((SYS_ADPT_TOTAL_NBR_OF_LPORT + 7) / 8)
#define SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST           ((SYS_ADPT_TOTAL_NBR_OF_LPORT * 2 + 7) / 8)

#define SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST                ((SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 7) / 8)

/* wuli, 2005-03-21 add */
#define SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_TOTAL_PORT_LIST           ((SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD+ 7) / 8)

/* Defines the ports allowed to be trunk member.
 *
 * NOTE: this is a uport list with length SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST
 */
#undef SYS_ADPT_ALLOW_TO_BE_TM_PORT_LIST

/* Define watch dog parameters
 */
#define SYS_ADPT_MAX_NBR_OF_WATCH_DOG_LOG_INFO                  32


/* Define the size of L2 MAC address table, and L3 IP-host table.
 * Note: The adaptation value for these two tables is limited by the ASIC chip.
 */
#define SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY           (104 * SYS_TYPE_1K)
#define SYS_ADPT_MAX_NBR_TOTAL_STATIC_MAC               (SYS_TYPE_1K)
#define SYS_ADPT_MAX_NBR_OF_AUTO_LEARN_MAC              1024 /* Max auto learn MAC number */
#define SYS_ADPT_MAX_NBR_OF_STATIC_MAC_PER_PORT         1024 /* Max static MAC per port can set */

#define SYS_ADPT_RESERVED_INFO_LEN                      32


/* Define total number trunks can be created per system, and also
 * define the total number of ports can be aggregated to a trunk.
 * Note: 1. The adaptation value, SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM, is limited/determined by
 *          SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK and ASIC chip.
 *       2. The adaptation value,SYS_ADPT_MAX_NBR_OF_10_100_PORT_PER_TRUNK and
 *          SYS_ADPT_MAX_NBR_OF_GIGA_PORT_PER_TRUNK, is limited by the ASIC chip.
 */
#define SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM             27 /* Max number of trunk port can be setup per stack */
#define SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK               8
#define SYS_ADPT_MAX_NBR_OF_10_100_PORT_PER_TRUNK        SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK
#define SYS_ADPT_MAX_NBR_OF_GIGA_PORT_PER_TRUNK          SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK
#define SYS_ADPT_MAX_NBR_OF_10GIGA_PORT_PER_TRUNK        SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK
#define SYS_ADPT_MAX_NBR_OF_40GIGA_PORT_PER_TRUNK        SYS_ADPT_MAX_NBR_OF_PORT_PER_TRUNK

/*
 *  Format character:
 *  $p  -- port number      $u  -- unit number
 *  $v  -- vlan id          $t  -- trunk id
 *  $i  -- ifindex          $b  -- loopback id
 */
/* If Description */
#define SYS_ADPT_LPORT_IF_DESC_STR                       "Ethernet Port on unit $u%d, port $p%d"
#define SYS_ADPT_TRUNK_MEMBER_IF_DESC_STR                "Trunk Member Port on Trunk ID $t%d"
#define SYS_ADPT_TRUNK_IF_DESC_STR                       "Trunk ID $t%04d"
#define SYS_ADPT_RS232_IF_DESC_STR                       "Console port"
#define SYS_ADPT_MPORT_IF_DESC_STR                       "Management Port on unit $u%d"
#define SYS_ADPT_VLAN_IF_DESC_STR                        "VLAN ID $v%04d"
#define SYS_ADPT_LOOPBACK_IF_DESC_STR                    "Loopback Interface $b%d"

/* If Name */
#define SYS_ADPT_GIGABIT_IFNAME                          "Port$i%d"
#define SYS_ADPT_FASTETH_IFNAME                          "Port$i%d"
#define SYS_ADPT_ETHERNET_IFNAME                         "Port$i%d"
#define SYS_ADPT_TRUNK_IFNAME                            "Trunk$t%d"
#define SYS_ADPT_RS232_IFNAME                            "Console$i%d"
#define SYS_ADPT_LOOPBACK_IFNAME                         "Loopback$b%d"
#define SYS_ADPT_VLAN_IFNAME                             "VLAN$v%d"

/* Define the total number of priority queues that ASIC can support per port.
 */
#define SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE               8

/* Define the total number of priority queues that ASIC can support per cpu port.
 */
#define SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE_FOR_CPU_PORT  8

/* SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN is used when allocating
 * buffer for transmission by L_MM_AllocateTxBuffer(). The API will reserve
 * size SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN on the beginning of the
 * allocated buffer. Cares should be taken when lower level transmission
 * required reserved size is changed!!!
 */
#define SYS_ADPT_TX_BUFFER_MAX_RESERVED_HEADER_LEN       32


/* Define the total number of IEEE 802.1Q VLAN ID can be created/supported by the system.
 * Note: 1. The adaptation value, SYS_ADPT_MAX_NBR_OF_VLAN, is limited by the Switch chip.
 *          This adaptation value may also be limited the system resource.
 *       2. The adaptation value, SYS_ADPT_MAX_VLAN_ID, can be any number in the range 1 ~ 4095
 *          as defined in the IEEE802.1Q.
 *          This adaption value will be determined based on the applications or customer requirements.
 *       3. The trunk name max length is just the same as MAXSIZE_ifAlias in RFC2863.
 *          And also this max lenght is the same as Trunk max name length.
 */
#define SYS_ADPT_MAX_NBR_OF_VLAN                         4094
#define SYS_ADPT_MAX_VLAN_ID                             4094    /* 4094 is reserved  */
#define SYS_ADPT_MAX_VLAN_NAME_LEN                       32      /* 32 chars */

/*Configure the GVRP supported VLAN range.
  Only the VLAN ID in the range will be carried on packet to remote switch
 */
#define SYS_ADPT_FIRST_DYNAMIC_VLAN_ID                   1
#define SYS_ADPT_LAST_DYNAMIC_VLAN_ID                    256

#define SYS_ADPT_MAX_NBR_OF_ISOLATED_VLAN                        1
#define SYS_ADPT_MAX_NBR_OF_UPLINK_IF_FOR_ISOLATED_VLAN          1
/* Max number of MAC-based VLAN entry in the system
 */
#define SYS_ADPT_MAX_NBR_OF_MAC_VLAN_ENTRY                      32 /*bcm 56510 max is 1024*/
#define SYS_ADPT_MAX_MAC_VLAN_ENTRY_PRIORITY                    7
#define SYS_ADPT_MIN_MAC_VLAN_ENTRY_PRIORITY                    0
/* Max number of IP subnet-based VLAN entry in the system
 */
#define SYS_ADPT_MAX_NBR_OF_IP_SUBNET_VLAN_ENTRY                32 /*bcm 56510 max is 255*/
#define SYS_ADPT_MAX_SUBNET_VLAN_ENTRY_PRIORITY                    7
#define SYS_ADPT_MIN_SUBNET_VLAN_ENTRY_PRIORITY                    0

/* Define range of internal management VLAN
 * Note: 1. Internal management VLAN is for special management and is transperant to administrator
 *       2. Internal management VLANs are reserved VLAN can not be used for normal configuration
 */
/* Only projects that support stacking need this.
 * Besides, DEV_SWDRV_ResetOnDemand() is not written well
 * and will get error when soc_L2MCm_field32_set(device_id, &l2mc_entry, PORT_BITMAP_HIf, SOC_PBMP_WORD_GET(pbmp[device_id],1)); is called
*/
#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
#define SYS_ADPT_VLAN_INTERNAL_MANAGEMENT_VLAN                  4094
#endif

/* Define protocol based VLAN relative adapt constants
 */
#define SYS_ADPT_1V_MAX_PROTOCOL_VALUE_BUFFER_LENGTH            5

#define SYS_ADPT_1V_MAX_NBR_OF_FRAME_TYPE                       3       /* chip limitation */
#define SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP                   16      /* chip limitation */

/* Max number of protocol group in the system for the object dot1vProtocolGroupTable
 * defined in the v-bridge Mib
 */
#define SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP_ENTRY             SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_GROUP

/* Max number of protocol port entry per port for the object dot1vProtocolPortTable
 * defined in the v-bridge Mib
 * The totol number of the entry in the system shall be equal to
 * (SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_PORT_ENTRY_PER_PORT * SYS_ADPT_TOTAL_NBR_OF_LPORT)
 */
#define SYS_ADPT_1V_MAX_NBR_OF_PROTOCOL_PORT_ENTRY_PER_PORT     8

#define SYS_ADPT_1V_MAX_PRIORITY_OF_PROTOCOL_PORT       7
#define SYS_ADPT_1V_MIN_PRIORITY_OF_PROTOCOL_PORT       0


/* Define the total number of IP M'cast groups can be handled/supported by the IGMP Snooping function.
 * Note: The adaptation valuse, SYS_ADPT_MAX_NBR_OF_IGMP_SNOOPING_MCAST_GROUPS, is limited/determined
 *       by the system resource and customer requirements.
 */
#define SYS_ADPT_MAX_NBR_OF_IGMP_SNOOPING_MCAST_GROUPS          SYS_TYPE_1K
#define SYS_ADPT_IGMP_MAX_SUPPORT_GROUP                         1024
#define SYS_ADPT_IGMP_MAX_SUPPORT_ROUTER                        32
#define SYS_ADPT_IGMP_MAX_SUPPORT_STATIC_GROUP                  64
#define SYS_ADPT_IGMP_MAX_SUPPORT_STATIC_GROUP_AND_SOURCE       64
#define SYS_ADPT_IGMP_MAX_SUPPORT_STATIC_ROUTER                 32
#define SYS_ADPT_IGMP_PROFILE_MAX_VALUE                         MAX_igmpSnoopProfileNumber /* max allowed profile ID */
#define SYS_ADPT_IGMP_PROFILE_MIN_VALUE                         MIN_igmpSnoopProfileNumber /* min allowed profile ID */
#define SYS_ADPT_IGMP_PROFILE_TOTAL_NBR                         SYS_ADPT_TOTAL_NBR_OF_LPORT
#define SYS_ADPT_IGMP_THROTTLE_TOTAL_NBR_OF_PORT                SYS_ADPT_TOTAL_NBR_OF_LPORT
#define SYS_ADPT_IGMP_THROTTLE_MAX_GROUP                        SYS_ADPT_IGMP_MAX_SUPPORT_GROUP
#define SYS_ADPT_IGMP_THROTTLE_MIN_GROUP                        0
#define SYS_ADPT_IGMP_MAX_VERSION                               MAX_igmpSnoopVersion
#define SYS_ADPT_IGMP_MIN_VERSION                               MIN_igmpSnoopVersion
#define SYS_ADPT_IGMP_SNOOPING_ENTRY_LIMIT_MAX                  1024
#define SYS_ADPT_IGMP_SNOOPING_ENTRY_LIMIT_HALF_MAX             512
/* 2009-04-14 Jimi Chen, for IGMP/IGMP proxy */
#define SYS_ADPT_IGMP_MAX_LAST_MEMBER_QUERY_INTERVAL            MAX_igmpInterfaceLastMembQueryIntvl
#define SYS_ADPT_IGMP_MIN_LAST_MEMBER_QUERY_INTERVAL            1
#define SYS_ADPT_IGMP_MAX_QUERY_MAX_RESPONSE_TIME               MAX_igmpInterfaceQueryMaxResponseTime
#define SYS_ADPT_IGMP_MIN_QUERY_MAX_RESPONSE_TIME               MIN_igmpInterfaceQueryMaxResponseTime
#define SYS_ADPT_IGMP_MAX_QUERY_INTERVAL                        255
#define SYS_ADPT_IGMP_MIN_QUERY_INTERVAL                        1
#define SYS_ADPT_IGMP_MAX_ROBUSTNESS                            MAX_igmpInterfaceRobustness
#define SYS_ADPT_IGMP_MIN_ROBUSTNESS                            MIN_igmpInterfaceRobustness
#define SYS_ADPT_IGMP_MIN_UNSOLICITED_REPORT_INTERVAL           1
#define SYS_ADPT_IGMP_MAX_UNSOLICITED_REPORT_INTERVAL           65535
#define SYS_ADPT_IGMP_MIN_ROBUSTNESSS_VALUE                     MIN_igmpSnoopLastMemberQueryCount
#define SYS_ADPT_IGMP_MAX_ROBUSTNESSS_VALUE                     MAX_igmpSnoopLastMemberQueryCount

#define SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY                  64   /* unknown + static + dynamic */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_ROUTER_PORT                  16   /* max route port -- by vlan */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP                 32
#define SYS_ADPT_MLD_MIN_UNSOLICITED_REPORT_INTERVAL           1
#define SYS_ADPT_MLD_MAX_UNSOLICITED_REPORT_INTERVAL           65535
#define SYS_ADPT_MDLSNP_MLD_REPORT_LIMIT_PER_SECOND_MAX  0xff
#define SYS_ADPT_MLDSNP_MLD_REPORT_LIMIT_PER_SECOND_MIN  5
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_RECORD_HOST_IP        4


#define SYS_ADPT_MLD_PROFILE_MAX_VALUE                         MAX_mldSnoopProfileNumber /* max allowed profile ID */
#define SYS_ADPT_MLD_PROFILE_MIN_VALUE                         MIN_mldSnoopProfileNumber /* min allowed profile ID */
/*define how many profile*/
#define SYS_ADPT_MLD_PROFILE_TOTAL_NBR                         SYS_ADPT_TOTAL_NBR_OF_LPORT
/*define each profile can have how many group range*/
#define SYS_ADPT_MLD_EACH_PROFILE_MAX_SUPPORT_GROUPS           128
/*if you want to have each profile has same group rang's, set SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS == (SYS_ADPT_MLD_EACH_PROFILE_MAX_SUPPORT_GROUPS*SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
  if you want to have a group range pool, and echa profile can allocate from it, set SYS_ADPT_MLD_EACH_PROFILE_MAX_SUPPORT_GROUPS > SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS
*/
#define SYS_ADPT_MLD_TOTAL_PROFILE_GROUPS                      (SYS_ADPT_MLD_EACH_PROFILE_MAX_SUPPORT_GROUPS*SYS_ADPT_MLD_PROFILE_TOTAL_NBR)
#define SYS_ADPT_MLD_THROTTLE_TOTAL_NBR_OF_PORT                SYS_ADPT_TOTAL_NBR_OF_LPORT

#define SYS_ADPT_IGMP_SNOOP_INVALID_FORWARD_PRIORITY                   0xffff
#define SYS_ADPT_IGMP_SNOOP_MAX_FORWARD_PRIORITY                       7   /*depend on chip*/
#define SYS_ADPT_IGMP_SNOOP_MIN_FORWARD_PRIORITY                       0   /*depend on chip*/
#define SYS_ADPT_IGMP_SNOOP_MAX_LAST_MEMBER_QUERY_INTERVAL            MAX_igmpSnoopLastMemberQueryInterval
#define SYS_ADPT_IGMP_SNOOP_MIN_LAST_MEMBER_QUERY_INTERVAL            MIN_igmpSnoopLastMemberQueryInterval
#define SYS_ADPT_IGMP_SNOOP_MAX_QUERY_MAX_RESPONSE_TIME               MAX_igmpSnoopProxyQueryResponseInterval
#define SYS_ADPT_IGMP_SNOOP_MIN_QUERY_MAX_RESPONSE_TIME               MIN_igmpSnoopProxyQueryResponseInterval
#define SYS_ADPT_IGMP_SNOOP_MAX_QUERY_INTERVAL                        MAX_igmpSnoopProxyQueryInterval
#define SYS_ADPT_IGMP_SNOOP_MIN_QUERY_INTERVAL                        MIN_igmpSnoopProxyQueryInterval
#define SYS_ADPT_IGMP_SNOOP_MAX_NBR_OF_RECORD_HOST_IP                 4

#define SYS_ADPT_IGMPSNP_MAX_INSTANCE    1           /*the number of mve domain is base on this, instance id start from 0 to this define*/

#define SYS_ADPT_IGMP_SNOOP_MAX_LAST_MEMBER_QUERY_INTERVAL            MAX_igmpSnoopLastMemberQueryInterval
#define SYS_ADPT_IGMP_SNOOP_MIN_LAST_MEMBER_QUERY_INTERVAL            MIN_igmpSnoopLastMemberQueryInterval


#define SYS_ADPT_IGMP_SNOOP_MIN_ROUTER_EXPIRE_TIME 1
#define SYS_ADPT_IGMP_SNOOP_MAX_ROUTER_EXPIRE_TIME 65535

#define SYS_ADPT_IGMP_SNOOP_MIN_UNSOLICITE_REPORT_INTERVAL 1
#define SYS_ADPT_IGMP_SNOOP_MAX_UNSOLICITE_REPORT_INTERVAL 65535

#define SYS_ADPT_IGMP_SNOOPING_MAX_VERSION 2

#define SYS_ADPT_MVR_MAX_INSTANCE_ID      0
#define SYS_ADPT_MVR_MIN_INSTANCE_ID      0

/*profile name max length
 */
#define SYS_ADPT_MVR_PROFILE_NAME_MAX_LEN 21
#define SYS_ADPT_MVR_PROFILE_TOTAL_NBR  32
#define SYS_ADPT_MVR_PROFILE_GROUP_RANGE_TOTAL_NBR  64 /*each profile can have how many range*/

#define SYS_ADPT_MVR6_MAX_INSTANCE_ID       1
#define SYS_ADPT_MVR6_MIN_INSTANCE_ID       1

#define SYS_ADPT_MVR6_PROFILE_NAME_MAX_LEN 21   /*it better to use odd value for memory alignment*/
#define SYS_ADPT_MVR6_PROFILE_TOTAL_NBR  16
#define SYS_ADPT_MVR6_PROFILE_GROUP_RANGE_TOTAL_NBR 4    /*each profile can have how many range*/

#define SYS_ADPT_IGMP_SNOOP_IGMP_REPORT_LIMIT_PER_SECOND_MAX  0xff
#define SYS_ADPT_IGMP_SNOOP_IGMP_REPORT_LIMIT_PER_SECOND_MIN  5

/* System adapt value for MVR */
#define SYS_ADPT_MVR_MAX_NBR_OF_GROUPS                          1024
#define SYS_ADPT_MVR_INVALID_FORWARD_PRIORITY                   SYS_ADPT_IGMP_SNOOP_INVALID_FORWARD_PRIORITY /*shall be the same as igmpsnooping, because gateway doesn't have seperate API*/
#define SYS_ADPT_MVR_MAX_FORWARD_PRIORITY                       7   /*depend on chip*/
#define SYS_ADPT_MVR_MIN_FORWARD_PRIORITY                       0   /*depend on chip*/
#define SYS_ADPT_MVR_MAX_ROBUSTNESSS                            MAX_igmpSnoopLastMemberQueryCount
#define SYS_ADPT_MVR_MIN_ROBUSTNESSS                            1
#define SYS_ADPT_MVR_MAX_PROXY_QUERY_INTERVAL                   MAX_igmpSnoopProxyQueryInterval
#define SYS_ADPT_MVR_MIN_PROXY_QUERY_INTERVAL                   MIN_igmpSnoopProxyQueryInterval
#define SYS_ADPT_MVR6_ENTRY_LIMIT_MAX     SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY

#define SYS_ADPT_IGMP_PROFILE_MAX_SUPPORT_GROUPS                SYS_ADPT_IGMP_MAX_SUPPORT_GROUP

#define SYS_ADPT_MVR6_MAX_NBR_OF_GROUPS                         SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY /*can't be large than SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY*/
#define SYS_ADPT_MVR6_INVALID_FORWARD_PRIORITY                  SYS_ADPT_IGMP_SNOOP_INVALID_FORWARD_PRIORITY /*shall be the same as igmpsnooping, because gateway doesn't have seperate API*/
#define SYS_ADPT_MVR6_MAX_FORWARD_PRIORITY                      7   /*depend on chip*/
#define SYS_ADPT_MVR6_MIN_FORWARD_PRIORITY                      0   /*depend on chip*/
#define SYS_ADPT_MVR6_MAX_ROBUSTNESSS                            MAX_mldSnoopRobustness
#define SYS_ADPT_MVR6_MIN_ROBUSTNESSS                            1
#define SYS_ADPT_MVR6_MAX_PROXY_QUERY_INTERVAL                   31744
#define SYS_ADPT_MVR6_MIN_PROXY_QUERY_INTERVAL                   2

/* 2003-OCT-09 JJ Young: add naming constant for Biker to merge code from Beagle
 */
#define SYS_ADPT_MIN_STATIC_ROUTE_METRIC                        1
#define SYS_ADPT_MAX_STATIC_ROUTE_METRIC                        16

/* Define the total number of mirror ports can be created/supported by the system.
 * Note: 1. The adaptation value, SYS_ADPT_MAX_NBR_OF_MIRROR_PORT, is limited by the Switch chip.
 *          This adaptation value may also be limited the system resource.
 *       2. This adaption value will be determined based on the applications or customer requirements.
 */
#define SYS_ADPT_MAX_NBR_OF_MIRROR_PORT                         1


/* Define the max length of name of a trunk
 * Note:  The trunk name max length is just the same as MAXSIZE_ifAlias in RFC2863.
 *        And also this max lenght is the same as VLAN max name length.
 */
#define SYS_ADPT_MAX_TRUNK_NAME_LEN                             64

/* Define the file name length for file system
 */
#define SYS_ADPT_FILE_SYSTEM_NAME_LEN                           32

/* Define the string maximum length for MIB2_MGR
 */
#define SYS_ADPT_MAX_SYSTEM_NAME_STR_LEN                        255

/* For DB_Sync and Auto Image Download */
#define SYS_ADPT_MAX_CONFIG_FILE_SIZE                           (1024*1000)

/* Define the if_index value of each ethernet port, trunk port, mirror port, RS232, and VLAN interface.
 * Note: 1. Please refer to RFC2233 for the detailed definition of interfaces.
 *
 *       2. The adaptation value, SYS_ADPT_ETHER_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first ethernet port (port#1, unit#1) in the system.
 *
 *          - The if_index number for each ethernet port is pre-assigned.
 *          - The if_index number of unit_i and port_j will be
 *
 *              SYS_ADPT_ETHER_1_IF_INDEX_NUMBER +
 *              (unit_i - 1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + port_j - 1
 *
 *       3. The adaptation value, SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first trunk port (port trunk#1) in the system.
 *
 *          - The if_index number for each Trunk is pre-assigned.
 *          - The if_index number of trunk_i will be
 *
 *              SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + i - 1
 *
 *       4. The adaptation value, SYS_ADPT_MIRROR_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first mirror port (port mirror#1) in the system.
 *
 *          - The if_index number for each Mirror port is pre-assigned.
 *          - The if_index number of mirror_port_i will be
 *
 *              SYS_ADPT_MIRROR_1_IF_INDEX_NUMBER + i - 1
 *
 *       5. The adaptation value, SYS_ADPT_RS232_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first RS232 UART port (RS232 #1) in the system.
 *
 *          - The if_index number for each RS232 UART port is pre-assigned.
 *          - The if_index number of RS2323 UART port_i will be
 *
 *              SYS_ADPT_RS232_1_IF_INDEX_NUMBER + i - 1
 *
 *       6. The adaptation value, SYS_ADPT_CPU_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first CPU port (CPU #1) in the system.
 *
 *          - The if_index number for each CPU port is pre-assigned.
 *          - The if_index number of CPU port_i will be
 *
 *              SYS_ADPT_CPU_1_IF_INDEX_NUMBER + i - 1
 *
 *       7. The adaptation value, SYS_ADPT_VLAN_1_IF_INDEX_NUMBER, defines the
 *          if_index number of first created VLAN (default VLAN#1) in the system.
 *
 *          - The if_index number for each Trunk is dynamic allocated, but not pre-assigned.
 *          - For each new created VLAN, the next available if_index number will be assigned
 *            to that VLAN.
 *          - The assigned if_index number will be released/free when a VLAN is deleted.
 *          - A VLAN ID will be ono-to-one mapping to a if_index number.
 */
#define SYS_ADPT_ETHER_1_IF_INDEX_NUMBER                        1
#define SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER                        (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK *       \
                                                                 SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT  +       \
                                                                 SYS_ADPT_ETHER_1_IF_INDEX_NUMBER )
#define SYS_ADPT_RS232_1_IF_INDEX_NUMBER                        (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER +         \
                                                                 SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM)
#define SYS_ADPT_CPU_1_IF_INDEX_NUMBER                          (SYS_ADPT_RS232_1_IF_INDEX_NUMBER +         \
                                                                 1)
#define SYS_ADPT_VLAN_1_IF_INDEX_NUMBER                         1001

/* ifindex for craft port, only used when SYS_CPNT_CRAFT_PORT is TRUE
 */
#define SYS_ADPT_CRAFT_INTERFACE_IFINDEX                        (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + 4096)

#define SYS_ADPT_MAX_NUMBER_OF_VRF_IN_SYSTEM                    SYS_ADPT_MAX_NUMBER_OF_FIB
#define SYS_ADPT_LOOPBACK_IF_INDEX_BASE                         (SYS_ADPT_VLAN_1_IF_INDEX_NUMBER -     \
                                                                 SYS_ADPT_MAX_NUMBER_OF_VRF_IN_SYSTEM)

#define SYS_ADPT_MAX_NBR_OF_LOOPBACK_IF                         1
#define SYS_ADPT_LOOPBACK_IF_INDEX_NUMBER                       6000


/* Define the max number of MAU and Jack number for MAU MIB supporting.
 */
#define SYS_ADPT_MAX_NBR_OF_MAU_PER_USER_PORT                   2
#define SYS_ADPT_MAX_NBR_OF_JACK_PER_MAU                        1


#if 0
/* Define the total number of routing interfaces can be created/supported by the system.
 * Note: The adaptation value, SYS_ADPT_MAX_NBR_OF_RIF, will be determined/limited by
 *       the system resource (CPU power, DRAM and Flash memory size).
 */
#define SYS_ADPT_MAX_NBR_OF_IF_ENTRY                            1
#define SYS_ADPT_RIF_1_IF_INDEX_NUMBER                          6001
#endif

/* Define the Diffserv related adaptation values for the system.
 * Note: The adaptation value, SYS_ADPT_MAX_NBR_OF_DIFFSERV_LEVEL and SYS_ADPT_MAX_NBR_OF_DIFFSERV_PROFILE,
 *       are determined/limited by the system resource (DRAM and Flash memory size) and customer requirements.
 */
#define SYS_ADPT_MAX_NBR_OF_DIFFSERV_LEVEL                      64
#define SYS_ADPT_MAX_NBR_OF_DIFFSERV_PROFILE                    32

/* Define the max length of string to contain numerical ID's
 * used in CLI and Web, e.g. "1-2,4-8,11-13".
 */
#define SYS_ADPT_MAX_DIFFSERV_ID_STR_LENGTH                     100

/* Define  QoS Police and Rate
 */

#define SYS_ADPT_QOS_MIN_POLICE_BURST (             1000) /* bytes    */
#define SYS_ADPT_QOS_MAX_POLICE_BURST (128 * 1000 * 1000) /* 128 M bytes */
#define SYS_ADPT_QOS_MIN_POLICE_RATE  (               0)  /* kbps         */
#define SYS_ADPT_QOS_MAX_POLICE_RATE  (40 * 1000 * 1000)  /* kbps, 40 Gbps */

#define SYS_ADPT_ACL_MAX_DST_IPV6_PREFIX_LEN                    128

#define SYS_ADPT_DIFFSERV_MIN_POLICE_BURST  (             1000) /* bytes    */
#define SYS_ADPT_DIFFSERV_MAX_POLICE_BURST  (128 * 1000 * 1000) /* 128 M bytes */

#define SYS_ADPT_DIFFSERV_MIN_POLICE_RATE   (               0)  /* kbps         */
#define SYS_ADPT_DIFFSERV_MAX_POLICE_RATE   (40 * 1000 * 1000)  /* kbps, 40 Gbps */

#define SYS_ADPT_MAC_ADDR_LEN                                   6
#define SYS_ADPT_IPV4_ADDR_LEN                                  4
#define SYS_ADPT_IPV6_ADDR_LEN                                  16
#define SYS_ADPT_SERIAL_NO_STR_LEN                              21  /* max 21 chars, example: ACTyywwnnnn */
#define SYS_ADPT_HW_VER_STR_LEN                                 5   /* max 5 chars, example: R01A         */
#define SYS_ADPT_EPLD_VER_STR_LEN                               8   /* max 8 chars, example: 04/01/01     */
#define SYS_ADPT_FW_VER_STR_LEN                                 23  /* max 23 chars, example: 1234.1234.debg0204.1234 */
#define SYS_ADPT_LOADER_CUSTOMIZED_VER_STR_LEN                  SYS_ADPT_FW_VER_STR_LEN
#define SYS_ADPT_POST_CUSTOMIZED_VER_STR_LEN                    SYS_ADPT_FW_VER_STR_LEN
#define SYS_ADPT_FW_CUSTOMIZED_VER_STR_LEN                      SYS_ADPT_FW_VER_STR_LEN
#define SYS_ADPT_KERNEL_VER_STR_LEN                             19  /* max 19 chars, example: 2.6.19.2-1-1.1 */
#define SYS_ADPT_MANUFACTURE_DATE_LEN                           10  /* eg. 2001-11-09                     */
#define SYS_ADPT_MODEL_NUMBER_LEN                               15  /* max 15 chars, example: ES3508A-B3  */

/*Define license file*/
#define SYS_ADPT_LICENSE_FILE_PATH               "/flash/"
#define SYS_ADPT_LICENSE_FILE_NAME               "license.lic"
#define SYS_ADPT_LICENSE_AC_FILE_NAME            "license.ac"
#define SYS_ADPT_LICENSE_RESULT_FILE_NAME        "license.res"

#define SYS_ADPT_LICENSE_AC_LEN	                 64

/* Define the max size (in bytes) of upload/download file.
 * Note: The adaptation value, SYS_ADPT_MAX_FILE_SIZE, will be used
 *       to allocate the buffer for file upload/download.
 */
#define SYS_ADPT_MAX_FILE_SIZE                                  (256 * SYS_TYPE_1M_BYTES)

    /*===   Systemwide constant Declaration       ===*/
/* define total number of task in system, impact with EH table, TCB, ... */
#define SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM                      130
/* Configure driver, but not know or request for port, use this as symbole */
#define SYS_ADPT_DESTINATION_PORT_UNKNOWN      0xFFFF
/* Configure driver, but not know or request for unit, use this as symbole */
#define SYS_ADPT_DESTINATION_UNIT_UNKNOWN      0xFFFF
/* When configures device, the vlan is unknown or not needed */
#define SYS_ADPT_DESTINATION_VID_UNKNOWN       0xFFFF
/* When configures device, the trunk is unknown or not needed */
#define SYS_ADPT_DESTINATION_TRUNK_ID_UNKNOWN  0xFFFF
/* At L3 configuration, the MAC not got, but need to active something in chip,
 * use this Mac, can't pass out device, if found out side, it MUST BE a bug.
 */
#define SYS_ADPT_DESTINATION_MAC_UNKNOWN       0x0030F1000000


/* This defines for SNMP, the maximum value for the "max-repititions" field of
 * the "get bulk" request.
 * Though the "max-repititions" value depends on the amout of system available memory,
 * this constant defines a safe value that will generate response packets, of which frame size < 1518 bytes.
 */
#define SYS_ADPT_SNMP_MAX_REPETITIONS_OF_GET_BULK           100

/* Define total number of user authentications for CLI, SNMP, and WEB management.
 */
#define SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING           5

/* Defines the maximum number of SNMP trap and notify receivers.
 *
 * IMPORTANT:
 * Because of a limitation in the implementation
 * of inform, we need to pre-open SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER number of ports,
 * starting at 1042, thus:
 * 1042 to (1042 + SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER - 1)
 *
 * Because these ports are actually used by some standard features,
 * we have to make sure our switch does not support these features,
 * and is not using these ports.
 */
#define SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER                   5

#define SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE                     16
#define SYS_ADPT_MAX_LOGIN_PRIVILEGE                        15

/* Define the maximum number of management IP filters.
 */
#define SYS_ADPT_MAX_NBR_OF_MGMT_IP_FLT                     15

/* This is used regardless of protocol, only ip number
 * For 3Com, define max number of mgmt_ip_filter is 15
 * But for Accton, we support 3 protocol * 15 ip for each protocol
 * So for Accton, this value is 45
 */
#define SYS_ADPT_MAX_NBR_OF_IP_ADDR_FOR_MGMT_IP_FLT         45


/* Define the max number of system operations and error events can be logged by the system.
 * Note: The adaptation value, SYS_ADPT_MAX_NBR_OF_AUDIT_LOG_ENTRY, is limited/determined
 *       by the system resources and customer requirements.
 */
#define SYS_ADPT_MAX_NBR_OF_SYSTEM_LOG_ENTRY                256
#define SYS_ADPT_MAX_NBR_OF_ERROR_LOG_ENTRY                 256


/* Loader and diagnostic passwords
 */
#define SYS_ADPT_LOADER_BACKDOOR_PASSWORD                   "mercury"
#define SYS_ADPT_DIAG_BACKDOOR_PASSWORD                     "mercury"
#if 1
#define SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT            "mercury"
#endif

/* Zhong Qiyao, 2004.04.28, moved from "post.h" to "sys_adpt.h"
 * max length of text to describe each POST item
 */
#define SYS_ADPT_POST_DESC_MAX_LENGTH                       32
#define SYS_ADPT_POST_ITEM_DESC_MAX_LENGTH                  128


/* Zhong Qiyao: maybe merged to SwDrv backdoor
 */
#if 0
#define SYS_ADPT_CLI_BACKDOOR_PASSWORD_BROADCOM             "bcm"
#endif


/* Buffer for maximum size of CLI config file.
 *
 * Please evaluate your project basing on:
 * memory size (SDRAM)
 * command collection (e.g. VLAN, ACL, name lengths, port lists)
 * layer 2 versus layer 3 (layer 3 has much more commands)
 * standalone versus stacking (stacking has much more ports)
 */
#define SYS_ADPT_CLI_MAX_CONFIG_SIZE                        (10000 * SYS_TYPE_1K)

/* Maximum size for CLI display buffer.
 *
 * For Accton CLI, the recommended values is 300.
 * For customer CLI, please refer to that customer's projects.
 */
#define SYS_ADPT_CLI_MAX_BUFSIZE                            300


/* Define the constant for snmpv3 component
 */
#define SYS_ADPT_PRIVATEMIB_OID                             1, 3, 6, 1, 4, 1, 259, 12, 1, 5
#define SYS_ADPT_PRIVATEMIB_INDEX_ACCESSIBLE                0

/* This defines the minumum and maximum values of snmpTargetAddrTimeout
 * in RFC3413 in the implementation. The unit is 0.01 s.
 * For a default project, the minumum value is MIN_snmpTargetAddrTimeout,
 * and the maximum value is MAX_snmpTargetAddrTimeout.
 */
#define SYS_ADPT_MIN_SNMP_TARGET_ADDR_TIMEOUT               MIN_snmpTargetAddrTimeout
#define SYS_ADPT_MAX_SNMP_TARGET_ADDR_TIMEOUT               MAX_snmpTargetAddrTimeout


/* Define bucket numbers for RMON probes.
 */
#define SYS_ADPT_MAX_NBR_OF_RMON_ETHER_STATS_ENTRY      (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)

#define SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY  ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) * 2)
#define SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_ENTRY          (SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_DEFAULT_ENTRY + \
                                                        (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
#define SYS_ADPT_MAX_NBR_OF_RMON_HISTORY_LOG_ENTRY      8

#define SYS_ADPT_MAX_NBR_OF_DEFAULT_BCAST_ALARM         (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
#define SYS_ADPT_MAX_NBR_OF_RMON_ALARM_ENTRY            (SYS_ADPT_MAX_NBR_OF_DEFAULT_BCAST_ALARM + \
                                                        (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) * 1)

#define SYS_ADPT_MAX_NBR_OF_RMON_EVENT_ENTRY            (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
#define SYS_ADPT_MAX_NBR_OF_RMON_EVENT_LOG_ENTRY        8

#define SYS_ADPT_MAX_RMON_OWNER_STR_LEN                 32

/* Added by Jason, suggested by James
 */
#define SYS_ADPT_MAX_NBR_OF_SAS_ALARM                   (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
#define SYS_ADPT_MAX_OWNER_LEN                          19
#define SYS_ADPT_MAX_OBJ_LEN                            32
#define SYS_ADPT_MAX_EVENT_DESCR_LEN                    127
#define SYS_ADPT_MAX_USER_COMM_LEN                      35
#define SYS_ADPT_SNMP_ADMIN_STRING_LENGTH               32


/* Added by Jason, suggested by SPK, JJYoung and Zhong for RADIUS setting
 */
/* The RADIUS timeout cannot be too long, because if you
 * change the authentication mode (local vs. RADIUS), or
 * change the user database, the cache can still be used.
 */
#define SYS_ADPT_HTTP_RADIUS_TIMEOUT                    10  /* was 300 */
#define SYS_ADPT_HTTP_RADIUS_CACHE_SIZE                 5

/* Define http/https connection timeout value to avoid web server be blocked
   by packet lose or telnet session. The value dependent on web context.
   Maybe range 10 - 40 sec.
*/
#define SYS_ADPT_HTTP_CONNECTION_TIMEOUT                10

/* Define http/https web user connection timeout value.
*/
#define SYS_ADPT_HTTP_WEB_USER_CONNECTION_TIMEOUT       30000  /* 5 min */

/* Define http/https web oem color.
*/
#define SYS_ADPT_HTTP_WEB_OEM_COLOR       "#B52C29"

/* Define http/https web user login page path.
*/
#define SYS_ADPT_HTTP_WEB_LOGIN_PAGE_PATH       "/home/login.htm"

/* Define http/https web user index page path.
*/
#define SYS_ADPT_HTTP_WEB_INDEX_PAGE_PATH       "/index.htm"

/* Define the realm constant for HTTP Basic/Digest authorization */
#define SYS_ADPT_HTTP_AUTHENTICATION_REALM              "Web Management"

/* Define parameters for HTTPS
 */
#define SYS_ADPT_HTTPS_MAX_PRIVATE_KEY_PASSWORD_LEN     32 /* 3Com aggreed */

/* Define web authentication login page path.
*/
#define SYS_ADPT_WEBAUTH_LOGIN_PAGE_PATH       "/webauth/login.htm"

/* Define web authentication login fail page path.
*/
#define SYS_ADPT_WEBAUTH_LOGIN_FAIL_PAGE_PATH       "/webauth/login_fail.htm"

/* Define web authentication login fail held page path.
*/
#define SYS_ADPT_WEBAUTH_LOGIN_FAIL_HELD_PAGE_PATH       "/webauth/login_fail_held.htm"

/* Define web authentication login full page path.
*/
#define SYS_ADPT_WEBAUTH_LOGIN_FULL_PAGE_PATH       "/webauth/login_full.htm"

/* Define web authentication login success page path.
*/
#define SYS_ADPT_WEBAUTH_LOGIN_SUCCESS_PAGE_PATH       "/webauth/login_success.htm"

/* Web Authentication authenticates all the hosts for each port.
 * The maximum number of hosts supported on a port is defined as following.
 * DUT shall only allow the maximum number of this value
 */
#define SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT      8

/* DUT shall be able to specify the maximum times of login attempts.
 *  If a host¡¦s failure counts exceed the maximum, DUT shall not allow
 *  doing authentication for a while.
 */
#define SYS_ADPT_WEBAUTH_MIN_LOGIN_ATTEMPTS             MIN_webAuthLoginAttempts
#define SYS_ADPT_WEBAUTH_MAX_LOGIN_ATTEMPTS             MAX_webAuthLoginAttempts

/* DUT shall be able to specify session timeout period. After the
 *  time has been exceeded, the session is terminated.
 */
#define SYS_ADPT_WEBAUTH_MIN_SESSION_TIMEOUT            MIN_webAuthSessionTimeout
#define SYS_ADPT_WEBAUTH_MAX_SESSION_TIMEOUT            MAX_webAuthSessionTimeout

/* DUT shall be able to specify quiet period. When host login retry
 *  counts reach the maximum of login attempts, DUT shall not allow
 *  any login authorizations. When quiet timer expires, DUT shall send
 *  login page whenever it receives a host HTTP request.
 */
#define SYS_ADPT_WEBAUTH_MIN_QUIET_PERIOD               MIN_webAuthQuietPeriod
#define SYS_ADPT_WEBAUTH_MAX_QUIET_PERIOD               MAX_webAuthQuietPeriod

/* Added by Jason Wang, suggested by Kevin, Radius Client */
#define SYS_ADPT_RADIUS_USE_SERVICE_TYPE_AS_PRIVILEGE   TRUE                /* Use the RADIUS attribute "Service-Type" as privilege */
#define SYS_ADPT_RADIUS_USE_FIELD_ID_AS_PRIVILEGE       FALSE               /* Use the RADIUS attribute "Filter-Id" as privilege */
#define SYS_ADPT_RADIUS_FIELD_ID_FOR_ADMIN_PRIVILEGE    "rw"    /* Mapping this Filter-Id to administrator */
#define SYS_ADPT_RADIUS_FIELD_ID_FOR_SUPPER_PRIVILEGE   "su"    /* Mapping this Filter-Id to supper user */
#define SYS_ADPT_RADIUS_FIELD_ID_FOR_GUEST_PRIVILEGE    "ro"    /* Mapping this Filter-Id to guest */
#define SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS              5
#define SYS_ADPT_RADIUS_SECRET_KEY_MIN_LENGTH           0
#define SYS_ADPT_RADIUS_SECRET_KEY_MAX_LENGTH           (3 * 16) /* FOR RADIUS/TACACS secret key that MUST be multiple of 16 */
#define SYS_ADPT_RADIUS_MIN_TIMEOUT                     1
#define SYS_ADPT_RADIUS_MAX_TIMEOUT                     60
#define SYS_ADPT_RADIUS_MIN_RETRANSMIT                  1
#define SYS_ADPT_RADIUS_MAX_RETRANSMIT                  5

#define SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP            5 /* the max number of radius server group */
#define SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP       5 /* the max number of tacacs server group */
#define SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME       MAXSIZE_aaaAccountMethodGroupName  /* the max length of server group name */

#define SYS_ADPT_MAX_NBR_OF_ACCOUNTING_LIST             5 /* The max number of accounting method-list */
#define SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME        MAXSIZE_aaaAccountMethodName /* the max length of list name */

#define SYS_ADPT_MIN_ACCOUNTING_UPDATE_INTERVAL         MIN_aaaUpdate  /* minute */
#define SYS_ADPT_MAX_ACCOUNTING_UPDATE_INTERVAL         MAX_aaaUpdate  /* minute */

#define SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS            SYS_ADPT_TOTAL_NBR_OF_LPORT /* The max number of RADIUS accounting users */

#define SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS              1
#define SYS_ADPT_TACACS_MIN_TIMEOUT                     1
#define SYS_ADPT_TACACS_MAX_TIMEOUT                     60
#define SYS_ADPT_TACACS_MIN_RETRANSMIT                  1
#define SYS_ADPT_TACACS_MAX_RETRANSMIT                  5

/* Max sessions for accounting TACACS+ user.
 * It should be Console + max sessions of Telnet/SSH
 */
#define SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS       (1 + SYS_ADPT_MAX_TELNET_NUM)

#define SYS_ADPT_MAX_NBR_OF_AUTHORIZATION_LIST          5 /* The max number of authorization method-list */
#define SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME     MAXSIZE_aaaAuthMethodName /* the max length of list name */

/* netaccess
 */
#define SYS_ADPT_NETACCESS_TOTAL_NBR_OF_SECURE_ADDRESSES_PER_SYSTEM    2048
#define SYS_ADPT_NETACCESS_MAX_NBR_OF_SECURE_ADDRESSES_PER_PORT        1024
#define SYS_ADPT_NETACCESS_MIN_NBR_OF_SECURE_ADDRESSES_PER_PORT        1
#define SYS_ADPT_NETACCESS_MAX_LEN_OF_VLAN_LIST                        128/* character */
#define SYS_ADPT_NETACCESS_MAX_LEN_OF_QOS_PROFILE                      256/* character,use Qos definition */
#define SYS_ADPT_NETACCESS_MAX_REAUTH_TIME                             1000000L/* seconds */
#define SYS_ADPT_NETACCESS_MIN_REAUTH_TIME                             120L/* seconds */
#define SYS_ADPT_NETACCESS_MAX_FILTER_ID                               64
#define SYS_ADPT_NETACCESS_MIN_FILTER_ID                               1
#define SYS_ADPT_NETACCESS_TOTAL_NBR_OF_MAC_FILTER_ENTRY_PER_SYSTEM    200

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
/* SYS_DFLT_NETACCESS_MACAUTH_SECURE_ADDRESSES_PER_PORT <= SYS_ADPT_NETACCESS_MACAUTH_MAX_NBR_OF_SECURE_ADDRESSES_PER_PORT
 */
#define SYS_ADPT_NETACCESS_MACAUTH_MAX_NBR_OF_SECURE_ADDRESSES_PER_PORT    1024

/* Define the max frame size that can be handled by the system.
 * Note: 1. For tagged frame, the max frame size will be 1522 (1518+4) bytes for EthernetII and 802.3 frame.
 *       2. For the switch/router which can support Jumbo frame, this adaptation value need to be redefined
 *      based on the ASIC spec.
 *     3. This adaptation value shall be used by any software components which need the max frame size for
 *          processing. The following two cases are example to use this adaptation value:
 *
 *      RFC1493 - "dot1dTpPortMaxInfo" object
 *      RFC2233 - "ifMtu" object
 */
#define SYS_ADPT_MAX_FRAME_SIZE                         1522       /* 9216 Jumbo frame length supported by XGS */

/* Define the max untagged frame size that can be handled by the system
 * when jumbo frame is enable.
 * Note: 1. For tagged frame, the max frams size will depend on chip capability.
 *       2. If it is allowed to configure MTU by user,
 *          this definition shall be the same with MAX_portMtu
 */
#if (SYS_CPNT_SWCTRL_MTU_CONFIG_MODE == SYS_CPNT_SWCTRL_MTU_NOT_CONFIGURABLE)
#define SYS_ADPT_MAX_JUMBO_MTU                          MAX_portMtu
#else
#define SYS_ADPT_MAX_JUMBO_MTU                          9408
#endif

/* different interface has different ifMtu.
 * SYS_ADPT_IF_MTU is the minmum mtu value of all interfaces.
 * Note: the mtu do not include the frame header, CRC, tag.
 */
#define SYS_ADPT_IF_MTU                                 1500    /* ethernet ifmtu */

#define SYS_ADPT_NIC_RECEIVE_RATE_INTERNAL              30000   /* micro-second */

/* This is defined for Stacking product.
 * If there is a stacking port which the inband packet will receive from
 * this port.  Since no matter Lport or Uport in core layer does not
 * have this port, we have defined a specific port for inband mac cache.
 */
#define SYS_ADPT_STACKING_PORT                          500


/* This is for OEM Adapt
 */
#define BLANC
#define XGS_SWITCH

#define SYS_ADPT_PACKETCELL_RESOLUTION                  64

#ifndef ECS5610_52S
#define ECS5610_52S
#endif

/*#define SYS_ADPT_SUPPORT_RTC*/

/* "system-wide, universal sysOID" means that: (1) In the SNMP MIB,
 * this is a system-wide variable, not per unit of stack.
 * (2) For Accton projects with universal sysOID (all models of universal
 * code uses the same sysOID), this constant is the same for all board ID's.
 *
 * The whole stack has this sysOID, not different models have different
 * sysOID's. Because there is only one sysOID, there is also only one
 * sysDescription, etc.
 * Board ID dependence is for some customers who have different sysOID's.
 */
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID0     "AOS5810-54X"
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID1     "AOS5810-54T"
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID2     "AOS6810-32X"
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID3     ""
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID4     ""
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID5     ""
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID6     ""
#define SYS_ADPT_SYS_DESC_STR_FOR_BOARD_ID7     ""

#define SYS_ADPT_MAX_OID_STRING_LEN             128 /* characters */
#define SYS_ADPT_MAX_OID_COUNT                  128 /* including index */

/* "system-wide, universal sysOID" means that: (1) In the SNMP MIB,
 * this is a system-wide variable, not per unit of stack.
 * (2) For Accton projects with universal sysOID (all models of universal
 * code uses the same sysOID), this constant is the same for all board ID's.
 *
 * The whole stack has this sysOID, not different models have different
 * sysOID's. Because there is only one sysOID, there is also only one
 * sysDescription, etc.
 * Board ID dependence is for some customers who have different sysOID's.
 */
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID0      "1.3.6.1.4.1.259.12.1.5.101"
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID1      "1.3.6.1.4.1.259.12.1.5.102"
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID2      "1.3.6.1.4.1.259.12.1.5.103"
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID3      ""
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID4      ""
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID5      ""
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID6      ""
#define SYS_ADPT_SYS_OID_STR_FOR_BOARD_ID7      ""


/*
 * For those projects supporting L2 only:
 * #define SYS_ADPT_SYS_SERVICES    (SYS_VAL_sysServices_physical | SYS_VAL_sysServices_datalink_or_subnetwork)
 *
 * #define SYS_ADPT_LLDP_SYSTEM_CAPABILITIES        (1 << VAL_lldpLocSysCapSupported_bridge)
 *
 * For those projects supporting L2 & L3:
 * #define SYS_ADPT_SYS_SERVICES    ( SYS_VAL_sysServices_physical   \
 *                                 | SYS_VAL_sysServices_datalink_or_subnetwork   \
 *                                 | SYS_VAL_sysServices_internet)
 *
 * #define SYS_ADPT_LLDP_SYSTEM_CAPABILITIES        (1 << VAL_lldpLocSysCapSupported_bridge | \
 *                                                   1 << VAL_lldpLocSysCapSupported_router)
 */

#define SYS_ADPT_SYS_SERVICES    ( SYS_VAL_sysServices_physical \
                                 | SYS_VAL_sysServices_datalink_or_subnetwork \
                                 | SYS_VAL_sysServices_internet )

#define SYS_ADPT_LLDP_SYSTEM_CAPABILITIES        (1 << VAL_lldpLocSysCapSupported_bridge | \
                                                  1 << VAL_lldpLocSysCapSupported_router)

#define SYS_ADPT_LLDP_MAX_CHASSIS_ID_LENGTH 32
#define SYS_ADPT_LLDP_MAX_PORT_ID_LENGTH 32

/*below define the bit position
#define VAL_lldpXMedPortCapSupported_capabilities	0L
#define VAL_lldpXMedPortCapSupported_networkPolicy	1L
#define VAL_lldpXMedPortCapSupported_location	2L
#define VAL_lldpXMedPortCapSupported_extendedPSE	3L
#define VAL_lldpXMedPortCapSupported_extendedPD	4L
#define VAL_lldpXMedPortCapSupported_inventory	5L
*/
#define SYS_ADPT_LLDP_MED_CAPABILITY ((1<<VAL_lldpXMedPortCapSupported_capabilities)\
                                      |(1<<VAL_lldpXMedPortCapSupported_networkPolicy) \
                                      |(1<<VAL_lldpXMedPortCapSupported_location) \
                                      |(1<<VAL_lldpXMedPortCapSupported_inventory) \
                                     )

#define SYS_ADPT_BRIDGE_PORT_CAPABILITIES   (   SYS_VAL_dot1dPortCapabilities_dot1qDot1qTagging \
                                              | SYS_VAL_dot1dPortCapabilities_dot1qConfigurableAcceptableFrameTypes \
                                              | SYS_VAL_dot1dPortCapabilities_dot1qIngressFiltering    )

/* Define QinQ tpid max and min value.
 */
#define SYS_ADPT_DOT1Q_TUNNEL_TPID_MIN            0x8000
#define SYS_ADPT_DOT1Q_TUNNEL_TPID_MAX            0xFFFF
/* Project dependent max number of remote data per port
 */
#define SYS_ADPT_LLDP_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT             4

/* Define private MIB root for trap manager to use.
 * SNMP private MIB cannot use this #define, and must be generated.
 */
/* "system-wide, universal sysOID" means that: (1) In the SNMP MIB,
 * this is a system-wide variable, not per unit of stack.
 * (2) For Accton projects with universal sysOID (all models of universal
 * code uses the same sysOID), this constant is the same for all board ID's.
 *
 * The whole stack has this sysOID, not different models have different
 * sysOID's. Because there is only one sysOID, there is also only one
 * sysDescription, etc.
 * Board ID dependence is for some customers who have different sysOID's.
 */
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID0         "1.3.6.1.4.1.259.12.1.5"
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID1         "1.3.6.1.4.1.259.12.1.5"
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID2         "1.3.6.1.4.1.259.12.1.5"
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID3         ""
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID4         ""
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID5         ""
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID6         ""
#define SYS_ADPT_PRIVATE_MIB_ROOT_FOR_BOARD_ID7         ""

/* String length (not including '\0') of SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID0-7
 * must be less than or equal to STKTPLG_MGR_MIB_STRING_LENGTH
 */
/* "system-wide, universal sysOID" means that: (1) In the SNMP MIB,
 * this is a system-wide variable, not per unit of stack.
 * (2) For Accton projects with universal sysOID (all models of universal
 * code uses the same sysOID), this constant is the same for all board ID's.
 *
 * The whole stack has this sysOID, not different models have different
 * sysOID's. Because there is only one sysOID, there is also only one
 * sysDescription, etc.
 * Board ID dependence is for some customers who have different sysOID's.
 */
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID0             "AOS5810-54X"
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID1             "AOS5810-54T"
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID2             "AOS6810-32X"
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID3             ""
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID4             ""
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID5             ""
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID6             ""
#define SYS_ADPT_PRODUCT_NAME_FOR_BOARD_ID7             ""

/* 2004/12/29 thomas add
EPR ES4649-ZZ-00890:Add new constant SYS_ADPT_MODEL_NAME_FOR_BOARD_ID0 to show customer model name for WEB
1. sys_adpt.h: add new constants SYS_ADPT_MODEL_NAME_FOR_BOARD_ID0 and SYS_ADPT_MAX_MODEL_NAME_SIZE
2. sys_mgr.c: add new api SYS_MGR_GetModelName to get model name read from sys_adpt.h base on board id
3. WEB/CGI: call sys_mgr.c to get model name instead of hard code*/
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID0             "AOS5810-54X"
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID1             "AOS5810-54T"
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID2             "AOS6810-32X"
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID3             ""
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID4             ""
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID5             ""
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID6             ""
#define SYS_ADPT_MODEL_NAME_FOR_BOARD_ID7             ""

#define SYS_ADPT_MAX_MODEL_NAME_SIZE                  16
/*end of 2004/12/15 phoebe add*/

#define SYS_ADPT_DEVICE_NAME_STRING_LEN                 128 /* characters */

/* Device Name is only developed by 3Com Request.
 * When user types system/inventory, CLI will print device name as follows.
 * If user types system/summary, CLI will print Product Name as above.
 * Jason, 9/9/2003
 */
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID0              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID1              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID2              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID3              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID4              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID5              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID6              ""
#define SYS_ADPT_DEVICE_NAME_FOR_BOARD_ID7              ""

/* String length(not includeing '\0') of SYS_ADPT_PRODUCT_MANUFACTURER must be
 * less than or equal to STKTPLG_MGR_MIB_STRING_LENGTH
 */
#define SYS_ADPT_PRODUCT_MANUFACTURER                   "Edgecore Networks, Inc."
#define SYS_ADPT_PRODUCT_DESCRIPTION                    "48-port 10G SFP+ and 4-port 40G QSFP+ uplink port managed switch"


/* Define the maximum string length and text for all types of
 * expansion modules.
 */
#define SYS_ADPT_MODULE_DESC_STR_LEN                   32

#define SYS_ADPT_MODULE_DESC_STR_1                     ""
#define SYS_ADPT_MODULE_DESC_STR_2                     ""
#define SYS_ADPT_MODULE_DESC_STR_3                     ""
#define SYS_ADPT_MODULE_DESC_STR_4                     ""
#define SYS_ADPT_MODULE_DESC_STR_5                     ""
#define SYS_ADPT_MODULE_DESC_STR_6                     ""
#define SYS_ADPT_MODULE_DESC_STR_7                     ""
#define SYS_ADPT_MODULE_DESC_STR_8                     ""
#define SYS_ADPT_MODULE_DESC_STR_9                     ""
#define SYS_ADPT_MODULE_DESC_STR_10                    ""
#define SYS_ADPT_MODULE_DESC_STR_11                    ""
#define SYS_ADPT_MODULE_DESC_STR_12                    ""
#define SYS_ADPT_MODULE_DESC_STR_13                    ""
#define SYS_ADPT_MODULE_DESC_STR_14                    ""
#define SYS_ADPT_MODULE_DESC_STR_15                    ""
#define SYS_ADPT_MODULE_DESC_STR_16                    ""
#define SYS_ADPT_MODULE_DESC_STR_17                    ""
#define SYS_ADPT_MODULE_DESC_STR_18                    ""
#define SYS_ADPT_MODULE_DESC_STR_19                    ""

#define SYS_ADPT_MAX_COMM_STR_NAME_LEN                 32

#define SYS_ADPT_MAX_NBR_OF_LOGIN_USER                 16
#define SYS_ADPT_MAX_USER_NAME_LEN                     32
#define SYS_ADPT_MAX_PASSWORD_LEN                      32
#define SYS_ADPT_MAX_ENCRYPTED_PASSWORD_LEN            16


/* Define the logfile size in file system (flash), the symbol is one logfile's length
 * There are 2 logfile exist in file system, so the total size is 2 times.
 */
#define SYS_ADPT_LOGFILE_SIZE_IN_FLASH                  (256 * SYS_TYPE_1K_BYTES)

/*  Define Maximun telnet session number.
 */
#define SYS_ADPT_MAX_TELNET_NUM                         8

/*  Define Ping related constant.
 *      SYS_ADPT_MAX_PING_NUM : Maximum allowing coexisted ping session request.
 *      SYS_ADPT_MIN_PING_SIZE: Minimum allowed size of ping data.
 *      SYS_ADPT_MAX_PING_SIZE: Maximum allowed size of ping data.
 *      SYS_ADPT_PING_MAX_NAME_SIZE: Maximum name size of owner_index or test_name in SNMP ping ctl entry.
 *      SYS_ADPT_PING_MAX_DATA_FILL_SIZE:
 *      SYS_ADPT_PING_MAX_MISC_OPTIONS_SIZE:
 *      SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY: Maximum prob history for a single ping session.
 */
#define SYS_ADPT_MAX_PING_NUM                           16
#define SYS_ADPT_MIN_PING_SIZE                          32
#define SYS_ADPT_MAX_PING_SIZE                          512 /* for CLI only, SNMP is 0..65507 */
#define SYS_ADPT_PING_MAX_NAME_SIZE                     32
#define SYS_ADPT_PING_MAX_DATA_FILL_SIZE                10
#define SYS_ADPT_PING_MAX_MISC_OPTIONS_SIZE             10
#define SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY     20

/* IPv6 ping6 */
#define SYS_ADPT_MAX_PING6_NUM                           16
#define SYS_ADPT_MIN_PING6_SIZE                          0
#define SYS_ADPT_MAX_PING6_SIZE                          1500 /* for CLI only, SNMP is 0..65507 */


/* UC buffer pointer maxinum bumber, the value mean maxinum available buffer number
 * if no free pointer buffer, even have free uc memory, you wouldn't get the free uc memory for using
 */
#define SYS_ADPT_MAX_UC_BUFFER_POINT_INDEX_NUM          64

#define SYS_ADPT_GARP_MAX_JOIN_TIME                     1000
#define SYS_ADPT_GARP_MIN_JOIN_TIME                     20
#define SYS_ADPT_GARP_MAX_LEAVE_TIME                    3000
#define SYS_ADPT_GARP_MIN_LEAVE_TIME                    60
#define SYS_ADPT_GARP_MAX_LEAVEALL_TIME                 18000
#define SYS_ADPT_GARP_MIN_LEAVEALL_TIME                 500

/* Naming constant to indicate the min and max aging time of the ASIC
 */
#define SYS_ADPT_MAX_DOT1D_TP_AGING_TIME                1000000
#define SYS_ADPT_MIN_DOT1D_TP_AGING_TIME                10

/* Define LAN Broadcast Storm threshold and time
 *      SYS_ADPT_LAN_BSTORM_THRESHOLD:          How many packets received in one second and to be thought BStorm is happened
 *      SYS_ADPT_NO_BSTORM_TIME:                How many seconds to count if there is no continuous BSTORM
 *      SYS_ADPT_DEFAULT_DELAY_TIME_OF_BSTORM:  If the case is not continuous BSTORM, we will just reset the MIN
 *                                              delay time to this value
 *      SYS_ADPT_MIN_DELAY_TIME_OF_BSTORM:      The min delay time for disabling receive Broadcast packet when the
 *                                              BSTORM is happened
 *      SYS_ADPT_MAX_DELAY_TIME_OF_BSTORM:      The max delay time for disabling receive Broadcast packet when the
 *                                              BSTORM is happened
 */
#define SYS_ADPT_LAN_BSTORM_THRESHOLD                   80
#define SYS_ADPT_NO_BSTORM_TIME                         5
#define SYS_ADPT_DEFAULT_DELAY_TIME_OF_BSTORM           5
#define SYS_ADPT_MIN_DELAY_TIME_OF_BSTORM               40
#define SYS_ADPT_MAX_DELAY_TIME_OF_BSTORM               50

/* These constants specify the DEV_NICDRV/LAN Buffer parameters

 */

#define SYS_ADPT_MAX_LAN_RX_BUF_SIZE_PER_PACKET         1600
#define SYS_ADPT_MAX_NBR_OF_LAN_PACKET_RX_BUFFER          512
#define SYS_ADPT_NIC_MAX_RX_BUF_SIZE_PER_PACKET             1600

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_NIC_PACKET_BUFFER_ADDRESS_OFFSET           34   /* will substract this value when packet buffer memory free */
#define SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_PER_PRIORITY      300   /* queue size per prioity queue for front port packet */
#define SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC           1200    /* queue size of prioity queue for ISC packet except STKTPLG */
#define SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_REPLY     1200    /* message queue size reserved for ISC reply packet */
#define SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_STKTPLG   300 /* reserved for ISC stktplg packet */
#define SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_1_ASIC_DMA    16    /* reserve buffer number per ASIC chip */
#define SYS_ADPT_NIC_TOTAL_NBR_OF_PACKET_BUFFER             ((SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE* \
                                                             SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_PER_PRIORITY)+ \
                                                             SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC + \
                                                             SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_REPLY + \
                                                             SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_ISC_STKTPLG + \
                                                             (SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT* \
                                                              SYS_ADPT_NIC_NBR_OF_PACKET_BUFFER_FOR_1_ASIC_DMA))
#define SYS_ADPT_NIC_TOTAL_RX_BUFFER_SIZE                   (SYS_ADPT_NIC_TOTAL_NBR_OF_PACKET_BUFFER* \
                                                             SYS_ADPT_NIC_MAX_RX_BUF_SIZE_PER_PACKET)

#define SYS_ADPT_TOTOAL_DMA_BUFFER_SIZE (12*SYS_TYPE_1M_BYTES)
/* The following are for ISC
 * there are level 1 ~ level SYS_ADPT_ISC_MAX_SPEEDLEVEL supported , those are classified by processing speed.
 */
#define SYS_ADPT_ISC_MAX_SPEEDLEVEL             4

#define SYS_ADPT_ISC_SPEEDLEVEL1_RETRANSMIT_DELAY_TICK     8
#define SYS_ADPT_ISC_SPEEDLEVEL2_RETRANSMIT_DELAY_TICK     6
#define SYS_ADPT_ISC_SPEEDLEVEL3_RETRANSMIT_DELAY_TICK     4
#define SYS_ADPT_ISC_SPEEDLEVEL4_RETRANSMIT_DELAY_TICK     2

#define SYS_ADPT_ISC_INTERNAL_SPEEDLEVEL            4
#define SYS_ADPT_ISC_LAN_DIRECTCALL_SPEEDLEVEL      4  /* assume will enqueue immediately */
#define SYS_ADPT_ISC_LAN_CALLBYAGENT_SPEEDLEVEL     3
#define SYS_ADPT_ISC_AMTRDRV_DIRECTCALL_SPEEDLEVEL  3
#define SYS_ADPT_ISC_AMTRDRV_CALLBYAGENT_SPEEDLEVEL 3
#define SYS_ADPT_ISC_NMTRDRV_SPEEDLEVEL             2
#define SYS_ADPT_ISC_SWDRV_SPEEDLEVEL               3
#define SYS_ADPT_ISC_LEDDRV_SPEEDLEVEL              3
#define SYS_ADPT_ISC_FS_SPEEDLEVEL                  1
#define SYS_ADPT_ISC_SWDRVL4_SPEEDLEVEL             3
#define SYS_ADPT_ISC_SYSDRV_SPEEDLEVEL              3
#define SYS_ADPT_ISC_SWDRVL4_G_SPEEDLEVEL           3
#define SYS_ADPT_ISC_HRDRV_SPEEDLEVEL               3
#define SYS_ADPT_ISC_SWDRVL3_SPEEDLEVEL             3
#define SYS_ADPT_ISC_SWDRV_CACHE_SPEEDLEVEL         3
#define SYS_ADPT_ISC_CFGDB_SPEEDLEVEL               2
#define SYS_ADPT_ISC_POEDRV_SPEEDLEVEL              3
#define SYS_ADPT_ISC_STK_TPLG_SPEEDLEVEL            4  /* direct call, STK has msg queue */
#define SYS_ADPT_ISC_RULE_CTRL_SPEEDLEVEL           3
#define SYS_ADPT_ISC_MSL_SPEEDLEVEL                 3

#define SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN         32 /* depent on header size defined in iuc.c and isc.c */

#define SYS_ADPT_ISC_MAX_FRAGMENT_BUFFER            16 /* maximal number of fragment buffer support */
/* NAMING CONSTANT DECLARATIONS
 */
#define SYS_ADPT_ISC_MAX_PDU_LEN (SYS_ADPT_MAX_FRAME_SIZE - SYS_ADPT_ISC_LOWER_LAYER_HEADER_LEN - 4/*CRC*/)


/* Naming constant to indicate the maximum number of file on Flash.
 * This section may be modified with the requirement of project.
 */
#define SYS_ADPT_MAX_NUM_OF_FILE                        32
#define SYS_ADPT_MAX_NUM_OF_FILE_SUBFILE                0
#define SYS_ADPT_MAX_NUM_OF_FILE_KERNEL                 2
#define SYS_ADPT_MAX_NUM_OF_FILE_DIAG                   2
#define SYS_ADPT_MAX_NUM_OF_FILE_RUNTIME                1
#define SYS_ADPT_MAX_NUM_OF_FILE_SYSLOG                 SYS_ADPT_MAX_NUM_OF_FILE
#define SYS_ADPT_MAX_NUM_OF_FILE_CMDLOG                 SYS_ADPT_MAX_NUM_OF_FILE
#define SYS_ADPT_MAX_NUM_OF_FILE_CONFIG                 16
#define SYS_ADPT_MAX_NUM_OF_FILE_POSTLOG                SYS_ADPT_MAX_NUM_OF_FILE
#define SYS_ADPT_MAX_NUM_OF_FILE_PRIVATE                SYS_ADPT_MAX_NUM_OF_FILE
#define SYS_ADPT_MAX_NUM_OF_FILE_CERTIFICATE            1
#define SYS_ADPT_MAX_NUM_OF_FILE_ARCHIVE                2
#define SYS_ADPT_MAX_NUM_OF_FILE_BINARY_CONFIG          2
#define SYS_ADPT_MAX_NUM_OF_FILE_PUBLIC                 SYS_ADPT_MAX_NUM_OF_FILE
#define SYS_ADPT_MAX_NUM_OF_FILE_LICENSE                1
#define SYS_ADPT_MAX_NUM_OF_FILE_NOS_INSTALLER          2
#define SYS_ADPT_MAX_NUM_OF_FILE_TOTAL                  0

    /*..    CFGDB component based adaptives        ..*/
/* These constants here are for CFGDB
 */
#define SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION     1024
#define SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION            1
#define SYS_ADPT_BINARY_CFG_FILE_MAX_SECTION            (SYS_ADPT_BINARY_CFG_FILE_MIN_SECTION + \
                                                         SYS_ADPT_MAX_NBR_OF_BINARY_CFG_FILE_SECTION - 1)

#define SYS_ADPT_BINARY_CFG_FILE_SIZE                   (256 * SYS_TYPE_1K_BYTES)/*128000*/   /*in byte*//*fuzhimin,20090507*/
#define SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_HOLD_TIME     500      /*in tick; Magma-1291, 5 sec ref. auto-save*/
#define SYS_ADPT_BINARY_CFG_FILE_AUTOSAVE_MAX_HOLD_TIME 2000     /*in tick*/

/* Definition maximum syslog remote server number and
 * maximum message length of remote log packet
 */
#define SYS_ADPT_MAX_LENGTH_OF_REMOTELOG_MESSAGE        1024
#define SYS_ADPT_MAX_NUM_OF_REMOTELOG_SERVER            5

/*define maximun ssh session number */
#define SYS_ADPT_MAX_SSH_NUMBER                         SYS_ADPT_MAX_TELNET_NUM
#define SYS_ADPT_MAX_SSH_SERVER_KEY_SIZE                896
#define SYS_ADPT_MIN_SSH_SERVER_KEY_SIZE                512
#define SYS_ADPT_MAX_SSH_CIPHER_STRING_LEN              64
#define SYS_ADPT_SSH_MAX_RANDOM_CHARACTERS_LEN          32   /* 3Com common spec. */

/* Defines the SNTP polling interval.
 * A default project should use MAX_sntpPollInterval as maximum value.
 * and use MIN_sntpPollInterval as minimum value.
 */
#define SYS_ADPT_SNTP_MAX_POLLING_INTERVAL 		        MAX_sntpPollInterval
#define SYS_ADPT_SNTP_MIN_POLLING_INTERVAL 		        MIN_sntpPollInterval

/* Definition of AMTR H/W address table sync argument
 */
#define SYS_ADPT_AMTR_NBR_OF_ITERATION_TO_START_HW_ARL_SYNC  300
#define SYS_ADPT_AMTR_NBR_OF_ITERATION_FOR_SYNC_INTERVAL     1
#define SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN        512
#define SYS_ADPT_AMTR_NBR_OF_ADDR_TO_ANNOUNCE_IN_ONE_SCAN    SYS_ADPT_AMTR_NBR_OF_ADDR_TO_SYNC_IN_ONE_SCAN
#define SYS_ADPT_AMTR_TYPE_SYNC2HISAM_NUM                    1024

/* This symbol is for MSTP using */
#define SYS_ADPT_MAX_NBR_OF_MST_INSTANCE                    33     /* 32(MSTIs) + 1(CIST), Max is 65 */
#define SYS_ADPT_XSTP_REGION_NAME                       "XSTP REGION 0"

#define SYS_ADPT_MAX_BSTORM_RATE_LIMIT                 59520000 /* pps */
#define SYS_ADPT_MIN_BSTORM_RATE_LIMIT                 500

#define SYS_ADPT_MAX_MSTORM_RATE_LIMIT                 59520000 /* pps */
#define SYS_ADPT_MIN_MSTORM_RATE_LIMIT                 500

#define SYS_ADPT_MAX_UNKNOWN_USTORM_RATE_LIMIT         59520000 /* pps */
#define SYS_ADPT_MIN_UNKNOWN_USTORM_RATE_LIMIT         500

#define SYS_ADPT_ATC_STORM_CONTROL_UNIT                     1000
/* This symbol is used for ATC Broadcast Storm.
*/
#define SYS_ADPT_ATC_BSTORM_MAX_RATE_LIMIT                  (255 * SYS_ADPT_ATC_STORM_CONTROL_UNIT) /* 255K = 255000 = 0x3E418 */
#define SYS_ADPT_ATC_BSTORM_MIN_RATE_LIMIT                  (1 * SYS_ADPT_ATC_STORM_CONTROL_UNIT) /* 1K = 1000 = 0x3E8 */
#define SYS_ADPT_ATC_BSTORM_MAX_TC_ON_TIMER                 300 /* MAX Traffic Control On Timer: 300(second) */
#define SYS_ADPT_ATC_BSTORM_MIN_TC_ON_TIMER                 5/* MIN Traffic Control On Timer: 5(second) */
#define SYS_ADPT_ATC_BSTORM_MAX_TC_RELEASE_TIMER            900 /* MAX Traffic Control Release Timer: 900(second) */
#define SYS_ADPT_ATC_BSTORM_MIN_TC_RELEASE_TIMER            5 /* MIN Traffic Control Release Timer: 5(second) */

/* This symbol is used for ATC Multicast Storm.
*/
#define SYS_ADPT_ATC_MSTORM_MAX_RATE_LIMIT                  (255 * SYS_ADPT_ATC_STORM_CONTROL_UNIT) /* 255K = 255000 = 0x3E418 */
#define SYS_ADPT_ATC_MSTORM_MIN_RATE_LIMIT                  (1 * SYS_ADPT_ATC_STORM_CONTROL_UNIT) /* 1K = 1000 = 0x3E8 */
#define SYS_ADPT_ATC_MSTORM_MAX_TC_ON_TIMER                 300 /* MAX Traffic Control On Timer: 300(second) */
#define SYS_ADPT_ATC_MSTORM_MIN_TC_ON_TIMER                 5 /* MIN Traffic Control On Timer: 5(second) */
#define SYS_ADPT_ATC_MSTORM_MAX_TC_RELEASE_TIMER            900 /* MAX Traffic Control Release Timer: 900(second) */
#define SYS_ADPT_ATC_MSTORM_MIN_TC_RELEASE_TIMER            5 /* MIN Traffic Control Release Timer: 5(second) */


/* The SYS_ADPT_MAX_RATE_LIMIT value is different in different project
 * 5508 is 10000, blanc/blanc-08/Hagrid is 1000
 */
#define SYS_ADPT_MAX_RATE_LIMIT                 40000000
#define SYS_ADPT_MIN_RATE_LIMIT                 64


/* This value is for adjust rate value from UI to core layer.
 * because core layer always use kbits, if the unit used by UI is Mbps
 * SYS_ADPT_UI_RATE_LIMIT_FACTOR must set to 1000
 */
#define SYS_ADPT_UI_RATE_LIMIT_FACTOR           1

/* rate limit on cpu port and queue, the max size of PKTMAXBUCKET and PKTPORTMAXBUCKET is 1024. anzhen.zheng, 8/26/2008 */
#define SYS_ADPT_CPU_PORT_PPS                  2048
#define SYS_ADPT_CPU_PORT_BURST                1024

#define SYS_ADPT_CPU_COS0_PPS                  800
#define SYS_ADPT_CPU_COS0_BURST                1024
#define SYS_ADPT_CPU_COS1_PPS                  500
#define SYS_ADPT_CPU_COS1_BURST                1000
#define SYS_ADPT_CPU_COS2_PPS                  400
#define SYS_ADPT_CPU_COS2_BURST                800
#define SYS_ADPT_CPU_COS3_PPS                  400
#define SYS_ADPT_CPU_COS3_BURST                800
#define SYS_ADPT_CPU_COS4_PPS                  2048
#define SYS_ADPT_CPU_COS4_BURST                1024
#define SYS_ADPT_CPU_COS5_PPS                  2048
#define SYS_ADPT_CPU_COS5_BURST                1024
#define SYS_ADPT_CPU_COS6_PPS                  200
#define SYS_ADPT_CPU_COS6_BURST                400
#define SYS_ADPT_CPU_COS7_PPS                  200
#define SYS_ADPT_CPU_COS7_BURST                400
/* cos queues of cpu port are scheduled with (wrr + strict priority), weight 0 stand for strict queue.
 * chip limiation:
 *   for WRR, range of weight is 0~15
 */
#define SYS_ADPT_CPU_COS0_WEIGHT      1
#define SYS_ADPT_CPU_COS1_WEIGHT      2
#define SYS_ADPT_CPU_COS2_WEIGHT      4
#define SYS_ADPT_CPU_COS3_WEIGHT      6
#define SYS_ADPT_CPU_COS4_WEIGHT      6
#define SYS_ADPT_CPU_COS5_WEIGHT      12
#define SYS_ADPT_CPU_COS6_WEIGHT      14
#define SYS_ADPT_CPU_COS7_WEIGHT      0

/* Default data for configuring RX system
 * 4 channels total, channel 0 for tx, channel 1,2,3 for rx.
 * channel 1 include COS5,COS6 and COS7, for most significant packets like stacking;
 * channel 2 include COS2, COS3, COS4.
 * channel 3 include COS0 and COS1, for line speed packet, mac learning, unknown ipmc packet, etc.;
 * anzhen.zheng, 2008-11-14.
 */
#define SYS_ADPT_RX_CHANNEL1_COS_BITMAP     0xe0    /* COS bitmap channel to receive, COS5, cos6 and cos7. */
#define SYS_ADPT_RX_CHANNEL2_COS_BITMAP     0x1c    /* COS bitmap channel to receive ,COS2, COS3 and COS4*/
#define SYS_ADPT_RX_CHANNEL3_COS_BITMAP     0x03    /* COS bitmap channel to receive, COS0 and COS1 */

/* packet to cpu queue*/
#define SYS_ADPT_CPU_QUEUE_STKTPLG		7

#define SYS_ADPT_CPU_QUEUE_BPDU			6

#define SYS_ADPT_CPU_QUEUE_STKMGMT       6
#define SYS_ADPT_CPU_QUEUE_IUC           5
#define SYS_ADPT_CPU_QUEUE_L2CP          5
#define SYS_ADPT_CPU_QUEUE_LACP          5
#define SYS_ADPT_CPU_QUEUE_GVRP          5
#define SYS_ADPT_CPU_QUEUE_DOT1X         5
#define SYS_ADPT_CPU_QUEUE_LLDP          5
#define SYS_ADPT_CPU_QUEUE_EAPS          5
#define SYS_ADPT_CPU_QUEUE_RIP            5
#define SYS_ADPT_CPU_QUEUE_OSPF_HELLO     5
#define SYS_ADPT_CPU_QUEUE_PIM_HELLO      5
#define SYS_ADPT_CPU_QUEUE_PIM_BOOT       5
#define SYS_ADPT_CPU_QUEUE_PIM_CANDIDATE  5
#define SYS_ADPT_CPU_QUEUE_ARP_REPLY      5
#define SYS_ADPT_CPU_QUEUE_MLD       5
#define SYS_ADPT_CPU_QUEUE_UDLD           5

#define SYS_ADPT_CPU_QUEUE_OSPF           5
#define SYS_ADPT_CPU_QUEUE_IGMP           3
#define SYS_ADPT_CPU_QUEUE_PIM            5
#define SYS_ADPT_CPU_QUEUE_DVMRP          5
#define SYS_ADPT_CPU_QUEUE_VRRP           5
#define SYS_ADPT_CPU_QUEUE_OAM            5

#define SYS_ADPT_CPU_QUEUE_DHCP           3
#define SYS_ADPT_CPU_QUEUE_DHCP6          3

#define SYS_ADPT_CPU_QUEUE_ALL_HOST       3
#define SYS_ADPT_CPU_QUEUE_ALL_ROUTER     3

#define SYS_ADPT_CPU_QUEUE_MYMAC_MYIP		4
#define SYS_ADPT_CPU_QUEUE_ARP_REQUEST		3

#define SYS_ADPT_CPU_QUEUE_ICMP_REDIRECT 	1
#define SYS_ADPT_CPU_QUEUE_MYMAC_NOT_MYIP 	1
#define SYS_ADPT_CPU_QUEUE_TTL0 	        1
#define SYS_ADPT_CPU_QUEUE_TTL1 	        1
#define SYS_ADPT_CPU_QUEUE_IP_OPTION        1
#define SYS_ADPT_CPU_QUEUE_UNKNOW_IPMC	 	1
#define SYS_ADPT_CPU_QUEUE_IP_BCAST		    1

#define SYS_ADPT_CPU_QUEUE_L2_SLF		0

#define SYS_ADPT_CPU_QUEUE_PTP			6

#define SYS_ADPT_CPU_QUEUE_ORG_SPECIFIC3    6

#if 0
/* mmu setting */
#define SYS_ADPT_FE_LWM_COS_CELL_LIMIT      16
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT      16
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT      16
#define SYS_ADPT_CPU_LWM_COS_CELL_LIMIT      108
#define SYS_ADPT_FE_HOL_COS_PKT_LIMIT      128
#define SYS_ADPT_GE_HOL_COS_PKT_LIMIT      192
#define SYS_ADPT_HG_HOL_COS_PKT_LIMIT      192
#define SYS_ADPT_CPU_HOL_COS_PKT_LIMIT      35

#define SYS_ADPT_TOTAL_DYN_CELL_LIMIT       3744
#define SYS_ADPT_FE_DYN_CELL_LIMIT       3744
#define SYS_ADPT_GE_DYN_CELL_LIMIT       3744
#define SYS_ADPT_HG_DYN_CELL_LIMIT       3744
#define SYS_ADPT_CPU_DYN_CELL_LIMIT       0

#define SYS_ADPT_FE_IBP_PKT_LIMIT       36
#define SYS_ADPT_GE_IBP_PKT_LIMIT       36
#define SYS_ADPT_HG_IBP_PKT_LIMIT       36
#define SYS_ADPT_CPU_IBP_PKT_LIMIT       36
#define SYS_ADPT_FE_IBP_CELL_LIMIT       864
#define SYS_ADPT_GE_IBP_CELL_LIMIT       864
#define SYS_ADPT_HG_IBP_CELL_LIMIT       864
#define SYS_ADPT_CPU_IBP_CELL_LIMIT       864
#define SYS_ADPT_IBP_DISCARD_LIMIT      8192
#else
/*  2009-01-19, Jinfeng Chen:
    Change mmu setting to support GE port/stacking port's drr scheduling mode,
    others is not supported.
 */
#define SYS_ADPT_FE_LWM_COS_CELL_LIMIT      12
#define SYS_ADPT_FE_DYN_CELL_LIMIT          2208
#define SYS_ADPT_FE_HOL_COS_PKT_LIMIT       128
#define SYS_ADPT_FE_IBP_CELL_LIMIT          716
#define SYS_ADPT_FE_IBP_PKT_LIMIT           40

#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_0    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_1    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_2    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_3    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_4    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_5    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_6    32
#define SYS_ADPT_GE_LWM_COS_CELL_LIMIT_7    32
#define SYS_ADPT_GE_DYN_CELL_LIMIT          8064
#define SYS_ADPT_GE_HOL_COS_PKT_LIMIT       256
#define SYS_ADPT_GE_IBP_CELL_LIMIT          1152
#define SYS_ADPT_GE_IBP_PKT_LIMIT           78

#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_0    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_1    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_2    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_3    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_4    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_5    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_6    32
#define SYS_ADPT_HG_LWM_COS_CELL_LIMIT_7    32
#define SYS_ADPT_HG_DYN_CELL_LIMIT          8064
#define SYS_ADPT_HG_HOL_COS_PKT_LIMIT       256
#define SYS_ADPT_HG_IBP_CELL_LIMIT          1152
#define SYS_ADPT_HG_IBP_PKT_LIMIT           78

#define SYS_ADPT_CPU_LWM_COS_CELL_LIMIT     144
#define SYS_ADPT_CPU_DYN_CELL_LIMIT         0
#define SYS_ADPT_CPU_HOL_COS_PKT_LIMIT      77/* packet to CPU, CPU HOL happens before ingress port's IBP, avoid packet loss by ingress port's IBP. */
#define SYS_ADPT_CPU_IBP_CELL_LIMIT         1152/* tropicana has 8192 cells(2M bytes) packet buffer, CPU port should not entering IBP in any condition. */
#define SYS_ADPT_CPU_IBP_PKT_LIMIT          78/* max XQ for FE and GE is 1536, CPU port should not entering IBP in any condition. */

#define SYS_ADPT_TOTAL_DYN_CELL_LIMIT    8064   /*4096, To pass through put test for different size packet, modify total dynamic cell to be half of total buffer(8192 cells). */
#define SYS_ADPT_IBP_DISCARD_LIMIT          16384
#endif

/* Definition of maximums and minimums of line (console/Telnet) configuration parameters
 */
#define SYS_ADPT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT_MAX              300
#define SYS_ADPT_SYSMGR_CONSOLE_LOGIN_RESPONSE_TIMEOUT_MIN              10

#define SYS_ADPT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD_MAX                  120
#define SYS_ADPT_SYSMGR_CONSOLE_PASSWORD_THRESHOLD_MIN                  1

#define SYS_ADPT_SYSMGR_CONSOLE_EXEC_TIMEOUT_MAX                        65535
#define SYS_ADPT_SYSMGR_CONSOLE_EXEC_TIMEOUT_MIN                        60

#define SYS_ADPT_SYSMGR_CONSOLE_SILENT_TIME_MAX                         65535
#define SYS_ADPT_SYSMGR_CONSOLE_SILENT_TIME_MIN                         1

#define SYS_ADPT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT_MAX               300
#define SYS_ADPT_SYSMGR_TELNET_LOGIN_RESPONSE_TIMEOUT_MIN               10

#define SYS_ADPT_SYSMGR_TELNET_PASSWORD_THRESHOLD_MAX                   120
#define SYS_ADPT_SYSMGR_TELNET_PASSWORD_THRESHOLD_MIN                   1

#define SYS_ADPT_SYSMGR_TELNET_EXEC_TIMEOUT_MAX                         65535
#define SYS_ADPT_SYSMGR_TELNET_EXEC_TIMEOUT_MIN                         60

#define SYS_ADPT_SYSMGR_TELNET_SILENT_TIME_MAX                          65535
#define SYS_ADPT_SYSMGR_TELNET_SILENT_TIME_MIN                          1

#define SYS_ADPT_MAX_PROMPT_STRING_LEN                  32

#define SYS_ADPT_MIN_UART_BAUDRATE                      9600
#define SYS_ADPT_MAX_UART_BAUDRATE                      115200

/* For SNMP restartControl (2: warmBoot), (3: coldBoot), the time to delay to make sure
 * SNMP agnet can reply SNMP client success before take the action to restart the
 * system.
 * Note that this constant here may be CPU speed dependent, if CPU is fast, then the
 * constant here could be smaller.
 */
#define SYS_ADPT_SYSTEM_RESTART_DELAY_TIME              200    /*in tick*/

/* Definition for max number of power
 */
#define SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT              2

/* Definition for max number of fan in user specification
 * On AOS7712-32X, there are 6 fans in the device.
 */
#define SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT                6

#define SYS_ADPT_FAN_SPEED_FULL                         {100}
#define SYS_ADPT_FAN_SPEED_MID                          {75}

/* SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL
 *     Number of fan speed level. This constant must be defined when
 *     SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL is defined as TRUE.
 *     The fan speed level starts from 1. The higher value gets higer
 *     fan speed.
 *     Note: 1. If the number of fan speed level is different among different
 *              board ids , this constant must be defined as the maximum
 *              number among all board ids.
 */
#define SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL        4

/* SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_UP_TEMPERATURE_THRESHOLD_ARRAY
 *     To be used for the array to get the up temperature threshold
 *     of each level. The meaning of the array is shown below:
 *     fan_speed_level_to_up_temperature_threshold_ar[fan_speed_level-1] = up_temp_threshold
 *         fan_speed_level = the fan speed level to be looked up
 *         up_temp_threshold = the up temperature threshold.
 *     Note: 1. when the current temperature is larger than up threshold, the speed
 *              level will be increased by 1.
 *           2. the last element of the array must be 0
 */
#define SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_UP_TEMPERATURE_THRESHOLD_ARRAY   {49, 53, 57, 0 }

/* SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_DOWN_TEMPERATURE_THRESHOLD_ARRAY
 *     To be used for the array to get the down temperature threshold
 *     of each level. The meaning of the array is shown below:
 *     fan_speed_level_to_down_temperature_threshold_ar[fan_speed_level-1] = down_temp_threshold
 *         fan_speed_level = the fan speed level to be looked up
 *         down_temp_threshold = the down temperature threshold.
 *     Note: 1. when the current temperature is less than down threshold, the speed
 *              level will be decreased by 1.
 *           2. the last element of the array must be 0
 */

#define SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_DOWN_TEMPERATURE_THRESHOLD_ARRAY { 0, 42, 47, 52}

/* SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR
 *     Defines the number of thermal sensor to be checked.
 *     This constant is referenced when SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE
 *     is defined as SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP.
 *     Note: 1. If the number of thermal sensor to be checked is different among
 *              different board ids, this constant must be defined as the maximum
 *              number among all board ids.
 */
#define SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR 2

/* SYS_ADPT_SYSDRV_BID_0_FAN_SPEED_MODE_SM_CHECK_THERMAL_INDEX_ARRAY
 *     Defines the thermal sensor indices to be checked for evaluation of
 *     fan speed level.
 *     This constant is referenced when SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE
 *     is defined as SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP.
 */
#define SYS_ADPT_SYSDRV_BID_0_FAN_SPEED_MODE_SM_CHECK_THERMAL_INDEX_ARRAY      {1, 2}

/* Definition for max number of thermal
 */
/* kinghong modify: move to SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT */
/* #define SYS_ADPT_MAX_NBR_OF_THERMAL_PER_UNIT           2 */
#define SYS_ADPT_THERMAL_SENSOR_LM75                    0
#define SYS_ADPT_THERMAL_SENSOR_LM77                    1
#define SYS_ADPT_THERMAL_SENSOR_TYPE                    SYS_ADPT_THERMAL_SENSOR_LM75

#define SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY                 \
    {                                                                          \
        {50, 50, 50, 50, 0},                      /* for board id 0 - AS5812-54X */   \
        {50, 50, 50, 50, 0},                      /* for board id 1 - AS5812-54T */   \
        {50, 50, 50, 50, 50},                    /* for board id 2 - AS6812-32X */   \
    }

#define SYS_ADPT_THERMAL_THRESHOLD_DOWN_WITH_MULTIPLE_BIDS_ARRAY               \
    {                                                                          \
        {38, 38, 38, 38, 0},                      /* for board id 0 - AS5812-54X */   \
        {38, 38, 38, 38, 0},                      /* for board id 1 - AS5812-54T */   \
        {38, 38, 38, 38, 38},                    /* for board id 2 - AS6812-32X */   \
    }

#if 	(SYS_ADPT_THERMAL_SENSOR_TYPE == SYS_ADPT_THERMAL_SENSOR_LM77)
#define SYS_ADPT_THERMAL_0_THRESHOLD_UP                 80
#define SYS_ADPT_THERMAL_0_THRESHOLD_DOWN               70
#define SYS_ADPT_THERMAL_1_THRESHOLD_UP                 80
#define SYS_ADPT_THERMAL_1_THRESHOLD_DOWN               70
#define SYS_ADPT_THERMAL_THRESHOLD_NORMAL               75
#endif

#if 	(SYS_ADPT_THERMAL_SENSOR_TYPE == SYS_ADPT_THERMAL_SENSOR_LM75)
#define SYS_ADPT_THERMAL_0_THRESHOLD_UP                 65
#define SYS_ADPT_THERMAL_0_THRESHOLD_DOWN               10
#define SYS_ADPT_THERMAL_1_THRESHOLD_UP                 65
#define SYS_ADPT_THERMAL_1_THRESHOLD_DOWN               10
#define SYS_ADPT_THERMAL_THRESHOLD_NORMAL_HIGH          45
#define SYS_ADPT_THERMAL_THRESHOLD_NORMAL_LOW           10
#endif

#define SYS_ADPT_ES4626H_THERMAL_0_THRESHOLD_UP                78
#define SYS_ADPT_ES4626H_THERMAL_0_THRESHOLD_DOWN              65
#define SYS_ADPT_ES4626H_THERMAL_1_THRESHOLD_UP                78
#define SYS_ADPT_ES4626H_THERMAL_1_THRESHOLD_DOWN              65

#define SYS_ADPT_ES4650H_THERMAL_0_THRESHOLD_UP                83
#define SYS_ADPT_ES4650H_THERMAL_0_THRESHOLD_DOWN              75
#define SYS_ADPT_ES4650H_THERMAL_1_THRESHOLD_UP                83
#define SYS_ADPT_ES4650H_THERMAL_1_THRESHOLD_DOWN              75

#define SYS_ADPT_MAX_NBR_OF_ACTION_PER_THERMAL_PER_UNIT  1
#define SYS_ADPT_MIN_THERMAL_ACTION_RISING_THRESHOLD     60
#define SYS_ADPT_MAX_THERMAL_ACTION_RISING_THRESHOLD     100
#define SYS_ADPT_MIN_THERMAL_ACTION_FALLING_THRESHOLD    60
#define SYS_ADPT_MAX_THERMAL_ACTION_FALLING_THRESHOLD    100

/* for ACL
 */

#define SYS_ADPT_MAX_NBRS_OF_ACL                        1024
#define SYS_ADPT_ACL_MAX_NAME_LEN                       MAXSIZE_diffServAclName

/* User view, how many ACE per ACL.
 *
 * When SYS_CPNT_ACL_AUTO_COMPRESS_ACE is TRUE. The the value may be larger
 * then
 * MAX(SYS_ADPT_MAX_NBRS_OF_MAC_ACE, SYS_ADPT_MAX_NBRS_OF_IP_ACE,
 *     SYS_ADPT_MAX_NBRS_OF_IPV6_ACE)
 *
 * When SYS_CPNT_ACL_AUTO_COMPRESS_ACE is FALSE. The the value should be
 * MAX(SYS_ADPT_MAX_NBRS_OF_MAC_ACE, SYS_ADPT_MAX_NBRS_OF_IP_ACE,
 *     SYS_ADPT_MAX_NBRS_OF_IPV6_ACE)
 *
 */
#define SYS_ADPT_MAX_NBRS_OF_ACE                        100

/* User view, how many ACE in the system
 */
#define SYS_ADPT_MAX_NBRS_OF_ACE_OF_SYSTEM              (SYS_ADPT_MAX_NBRS_OF_ACE*SYS_ADPT_MAX_NBRS_OF_ACL)

/* Hardware view, how many rule per MAC/IP/IPv6 ACL.
 * These constants are used to define how many rule instance per class instance.
 *
 * The number of rule instance per class instance SHOULD be
 * MAX(SYS_ADPT_MAX_NBRS_OF_MAC_ACE, SYS_ADPT_MAX_NBRS_OF_IP_ACE,
 *     SYS_ADPT_MAX_NBRS_OF_IPV6_ACE)
 *
 * This value SHOULD NOT larger than number rule per selector/group on Broadcom
 * chip.
 */
#define SYS_ADPT_MAX_NBRS_OF_MAC_ACE                    1024
#define SYS_ADPT_MAX_NBRS_OF_IP_ACE                     1024
#define SYS_ADPT_MAX_NBRS_OF_IPV6_STD_ACE               1024
#define SYS_ADPT_MAX_NBRS_OF_IPV6_EXT_ACE               1024
#define SYS_ADPT_MAX_NBRS_OF_MAC_QOS                    1024
#define SYS_ADPT_MAX_NBRS_OF_IP_QOS                     1024
#define SYS_ADPT_MAX_NBRS_OF_IPV6_STD_QOS               1024
#define SYS_ADPT_MAX_NBRS_OF_IPV6_EXT_QOS               1024

#define SYS_ADPT_MAX_NBRS_OF_TCPUDP_ACE                 96
#define SYS_ADPT_ACL_DEFINE_MASK                        FALSE

/* TIME BASED ACL
 */
#define SYS_ADPT_MAX_NBRS_OF_TIME_ACL_TABLE             16
#define SYS_ADPT_MAX_LENGTH_OF_TIME_ACL_TABLE_NAME      16
#define SYS_ADPT_MAX_NBRS_OF_TIME_ACL_PERIODIC          7


#define SYS_ADPT_ADD_DEFAULT_INGRESS_IP_MASK    FALSE
#define SYS_ADPT_ADD_DEFAULT_EGRESS_IP_MASK             FALSE
#define SYS_ADPT_ADD_DEFAULT_INGRESS_MAC_MASK   FALSE
#define SYS_ADPT_ADD_DEFAULT_EGRESS_MAC_MASK            FALSE


#define SYS_ADPT_ACL_SUPPORT_L4_SPORT_RANGE             FALSE
#define SYS_ADPT_ACL_SUPPORT_L4_DPORT_RANGE             FALSE

#define SYS_ADPT_ACL_SUPPORT_VID_RANGE                  FALSE
#define SYS_ADPT_ACL_SUPPORT_ETHERTYPE_RANGE            FALSE

#define SYS_ADPT_SUPPORT_IP_STD_ACL                     TRUE
#define SYS_ADPT_SUPPORT_IP_EXT_ACL                     TRUE
#define SYS_ADPT_SUPPORT_MAC_ACL                        TRUE

#define SYS_ADPT_SUPPORT_INGRESS_IP_ACL                 TRUE
#define SYS_ADPT_SUPPORT_EGRESS_IP_ACL                  FALSE
#define SYS_ADPT_SUPPORT_INGRESS_MAC_ACL                TRUE
#define SYS_ADPT_SUPPORT_EGRESS_MAC_ACL                 FALSE

#define SYS_ADTP_ACE_SUPPORT_PKTFORAMT                  TRUE
#define SYS_ADPT_COS_CONTAIN_CLI_MAP_IP_PRECEDENCE      TRUE
#define SYS_ADPT_COS_CONTAIN_CLI_MAP_IP_DSCP            TRUE
#define SYS_ADPT_COS_CONTAIN_CLI_MAP_IP_PORT            TRUE
#define SYS_ADPT_COS_CONTAIN_CLI_MAP_ACCESS_LIST        TRUE



#define SYS_ADPT_MAX_NBRS_OF_ACL_OF_MARKER_PER_INTERFACE    5
#define SYS_ADPT_MAX_NBRS_OF_ACL_OF_COS_PER_INTERFACE       5


/* Define the Diffserv related adaptation values for the system.
 * Note: The adaptation value, SYS_ADPT_MAX_NBR_OF_DIFFSERV_LEVEL and SYS_ADPT_MAX_NBR_OF_DIFFSERV_PROFILE,
 *       are determined/limited by the system resource (DRAM and Flash memory size) and customer requirements.
 */
#define  SYS_ADPT_DIFFSERV_MAX_NAME_LENGTH              SYS_ADPT_ACL_MAX_NAME_LEN

/* for CLI show only, CLI need this information, this length does not count end of string char */
#define  SYS_ADPT_DIFFSERV_MAX_DESCRIPTION_LENGTH       64

/* for each port, we can configure a policy */
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_DATAPATH      (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)

/* support both ingress and egress */
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_DIRECTION         1

#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_POLICY_MAP        32  /* table size of clfr */

/* table size of clfrElement, POLICY_MAP to Class is 1 to many */
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_CLASS             128

/* CLASS MAP to RULE is 1 to many */
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_CLASS_MAP         32
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_MAC_RULE          96
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_IP_RULE           96
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_IPV6_RULE         96
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_RULE              288 /* table size of MF */

#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_METER             288
#define  SYS_ADPT_DIFFSERV_MAX_NBR_OF_ACTIONS           288

#define SYS_ADPT_DIFFSERV_MAX_CLASS_NBR_OF_CLASS_MAP            16
#define SYS_ADPT_DIFFSERV_MAX_CLASS_MAP_NBR_OF_POLICY_MAP       16

/* Max number of rules to trap IPv6 link addresses to CPU.
 */
#define SYS_ADPT_MAX_NBR_OF_LINK_LOCAL_TO_CPU           SYS_ADPT_MAX_NBR_OF_L3_INTERFACE

/* for WRR range
 */
#define SYS_ADPT_MAX_RATIO_OF_WRR                       15
#define SYS_ADPT_MIN_RATIO_OF_WRR                       1

/* 2008-06-02, Jinfeng.Chen: for DRR range */
#define SYS_ADPT_MAX_RATIO_OF_DRR						15
#define SYS_ADPT_MIN_RATIO_OF_DRR						1

/* This defines excluded characters used in ASCII string check
 * in function "L_STDLIB_StrIsAsciiPrint".
 * It can be an empty string or one or more characters.
 * If this is not defined, "" is assumed.
 */
#define SYS_ADPT_CMNLIB_ASCII_PRINT_EXCEPT_CHAR        "?"

#if 0 /* Yongxin.Zhao added, 2008-06-04, 11:38:30 */
/* to adjust the lan.c receive buffer */
#define SYS_ADPT_MAX_NBR_OF_MREF_DESC_BLOCK            4000
#endif

/* total mref block in this system */
#define SYS_ADPT_MAX_NBR_OF_MREF_BLOCK                 12000  /* 576 */

/*===  Layer 3 system adaptor definition, not including Layer 2 already defined adaptor ===*/
/* Define maximum number of Primary RIF, limited by H/W resource.
 * This limitation is for StrataSwitch, if chip no limitation, may use available vlan entry.
 */
#define SYS_ADPT_MAX_NBR_OF_PRIMARY_SUBNET             256

/*================================================================*/
/*---   Layer3 system wise adaptives           ---*/
/* Define the total number of routing interfaces can be created/supported by the system.
 * Note: The adaptation value, SYS_ADPT_MAX_NBR_OF_RIF, will be determined/limited by
 *       the system resource (CPU power, DRAM and Flash memory size).
 */
#define SYS_ADPT_1ST_AVAILABLE_RIF_NBR                  1       /* 1st can be allocated rif# */

/* For Beagle chip limitation, only 5 mask reserve for network,
 *     one for default gateway and 4 for net config.
 * For Hagrid, this value is 0, not need to check limitation.
 */
#define SYS_ADPT_NETWORK_MAX_NBR_OF_SUBNET_MASK         0


/* define VRRP, RFC 2338, willy
 */
#define SYS_ADPT_MAX_NBR_OF_VRRP_GROUP                   16
#define SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP                1

/*The range of VRRP ID defined by RFC 2338 P.14
 */
#define SYS_ADPT_MAX_VRRP_ID                              MAX_vrrpOperVrId
#define SYS_ADPT_MIN_VRRP_ID                              MIN_vrrpOperVrId

#define SYS_ADPT_VRRP_AUTHENTICATION_KEY_LEN              8

#define SYS_ADPT_VRRP_DEFL_PRIORITY                       100
#define SYS_ADPT_VRRP_DEFL_DELAY                          0
#define SYS_ADPT_VRRP_DEFL_ADVER_INTERVAL                 1


/*Range of priority defined by RFC 2338 P.11, VRRP owner priority 255
 */
#define SYS_ADPT_MAX_VRRP_PRIORITY                        254
#define SYS_ADPT_MIN_VRRP_PRIORITY                        1

/* Adevertisement Interval , which is 8 bits width in VRRP packet, so the range can only be 1~255
 */
#define SYS_ADPT_MAX_VRRP_ADVER_INTERVAL                  MAX_vrrpOperAdvertisementInterval
#define SYS_ADPT_MIN_VRRP_ADVER_INTERVAL                  MIN_vrrpOperAdvertisementInterval

/* Range of Preempt Delay time
 */
#define SYS_ADPT_MAX_VRRP_PREEMPT_DELAY                   120
#define SYS_ADPT_MIN_VRRP_PREEMPT_DELAY                   0

/* Operation Protocol range
 */
#define SYS_ADPT_MAX_VRRP_OPER_PROTOCOL                   VAL_vrrpOperProtocol_other
#define SYS_ADPT_MIN_VRRP_OPER_PROTOCOL                   VAL_vrrpOperProtocol_ip

/* Properties of virtual router that XGSIII support
 */
#define SYS_ADPT_MAX_NUMBER_OF_VIRTUAL_ROUTER             254
#define SYS_ADPT_VIRTUAL_ROUTER_NONE                      (SYS_ADPT_MAX_NUMBER_OF_VIRTUAL_ROUTER + 1)

/* FIB */
#define SYS_ADPT_MAX_NUMBER_OF_FIB                        255
#define SYS_ADPT_DEFAULT_FIB                              254

/* End of define VRRP */

/* HSRP reference RFC 2281
 * Willy
 */
#define SYS_ADPT_MAX_NBR_OF_HSRP_GROUP                    0
#define SYS_ADPT_MAX_NBR_OF_HSRP_GROUP_PER_SYSTEM         0

/*The range of the number of HSRP interface tracking in a group
 */
#define SYS_ADPT_MAX_NBR_OF_TRACKED_IF_PER_GROUP          0

/*The range of HSRP group number
 */
#define SYS_ADPT_MAX_HSRP_GROUP_NUMBER                    MAX_cHsrpGrpNumber
#define SYS_ADPT_MIN_HSRP_GROUP_NUMBER                    MIN_cHsrpGrpNumber

/*The range of HSRP priority
 */
#define SYS_ADPT_MAX_HSRP_GROUP_PRIORITY                  MAX_cHsrpGrpPriority
#define SYS_ADPT_MIN_HSRP_GROUP_PRIORITY                  1

/*The range of HSRP hello time
 */
#define SYS_ADPT_MAX_HSRP_GROUP_HELLO_TIME                85
#define SYS_ADPT_MIN_HSRP_GROUP_HELLO_TIME                1

/*The range of HSRP hold time
 */
#define SYS_ADPT_MAX_HSRP_GROUP_HOLD_TIME                 255
#define SYS_ADPT_MIN_HSRP_GROUP_HOLD_TIME                 3

/*The range of HSRP preempt delay time
 */
#define SYS_ADPT_MAX_HSRP_GROUP_PREEMPT_DELAY             MAX_cHsrpGrpPreemptDelay
#define SYS_ADPT_MIN_HSRP_GROUP_PREEMPT_DELAY             MIN_cHsrpGrpPreemptDelay

/*The range of HSRP config timeout
 */
#define SYS_ADPT_MAX_HSRP_GROUP_CONFIG_TIMEOUT            MAX_cHsrpConfigTimeout
#define SYS_ADPT_MIN_HSRP_GROUP_CONFIG_TIMEOUT            MIN_cHsrpConfigTimeout

/*The range of interface tracking priority
 */
#define SYS_ADPT_MAX_HSRP_GROUP_IFTRACKED_PRIORITY        MAX_cHsrpExtIfTrackedPriority
#define SYS_ADPT_MIN_HSRP_GROUP_IFTRACKED_PRIORITY        MIN_cHsrpExtIfTrackedPriority

/*The range of the number of HSRP second address setted
 */
#define SYS_ADPT_MAX_HSRP_GROUP_SEC_ADDR                  5
/* End of define HSRP */

/*---ARP---
 * Define the size of ARP Cache table.
 * Note: The ARP Cache contains static configured, dynamic learned, local IP interface defined
 *       VRRP interface and HSRP interface
 *       In Phase2, each rif ocupies 3 arp entry, it is take cared by ipport.h just for phase2
 */
#define SYS_ADPT_MIN_NBR_OF_RIF                          1
#define SYS_ADPT_MAX_NBR_OF_RIF                          1024

/* Total host entry in chip is 8K, multicast routing share 4K. */
#define SYS_ADPT_MAX_ARP_ENTRY                            (64 * SYS_TYPE_1K_BYTES )
#define SYS_ADPT_MAX_NBR_OF_TOTAL_ARP_CACHE_ENTRY         SYS_ADPT_MAX_ARP_ENTRY
#define SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY        256
#define SYS_ADPT_MAX_NBR_OF_STATIC_NEIGHBOR_CACHE_ENTRY   256
#define SYS_ADPT_MAX_NBR_OF_DYNAMIC_ARP_CACHE_ENTRY       (SYS_ADPT_MAX_NBR_OF_TOTAL_ARP_CACHE_ENTRY - \
                                                           SYS_ADPT_MAX_NBR_OF_RIF - \
                                                           SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY)
/*
 *  Define ARP entry age-out time. (ref. leaf_ES3626A.h)
 *      MAX_arpCacheTimeout (86400)
 *      MIN_arpCacheTimeout (300)
 */
#define SYS_ADPT_MAX_ARP_CACHE_TIMEOUT                      MAX_arpCacheTimeout
#define SYS_ADPT_MIN_ARP_CACHE_TIMEOUT                      MIN_arpCacheTimeout
#define SYS_ADPT_MAX_ND_CACHE_TIMEOUT                      MAX_arpCacheTimeout
#define SYS_ADPT_MIN_ND_CACHE_TIMEOUT                      MIN_arpCacheTimeout

/* djd: temp, need re-define */
#define SYS_ADPT_MAX_NBR_OF_TOTAL_IPNET2PHYSICAL_IPV4_CACHE_ENTRY     SYS_ADPT_MAX_ARP_ENTRY
#define SYS_ADPT_MAX_NBR_OF_TOTAL_IPNET2PHYSICAL_IPV6_CACHE_ENTRY     SYS_ADPT_MAX_ARP_ENTRY
#define SYS_ADPT_MAX_NBR_OF_STATIC_IPNET2PHYSICAL_IPV4_CACHE_ENTRY    SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY
#define SYS_ADPT_MAX_NBR_OF_STATIC_IPNET2PHYSICAL_IPV6_CACHE_ENTRY    256
#define SYS_ADPT_MAX_NBR_OF_DYNAMIC_IPNET2PHYSICAL_IPV4_CACHE_ENTRY   SYS_ADPT_MAX_NBR_OF_DYNAMIC_ARP_CACHE_ENTRY
#define SYS_ADPT_MAX_NBR_OF_DYNAMIC_IPNET2PHYSICAL_IPV6_CACHE_ENTRY    (SYS_ADPT_MAX_NBR_OF_TOTAL_ARP_CACHE_ENTRY - \
                                                           SYS_ADPT_MAX_NBR_OF_RIF - \
                                                           SYS_ADPT_MAX_NBR_OF_STATIC_ARP_CACHE_ENTRY)
/* The old configuration in NETCFG_OM was #define NETCFG_OM_IP_MAX_INTERFACE_NBR      (SYS_ADPT_MAX_NBR_OF_VLAN + SYS_ADPT_MAX_TUNNEL_ID+SYS_ADPT_MAX_NUMBER_OF_VRF_IN_SYSTEM)
   But there is a chip limitation in BCM56846(Trident+) that the l3 interface is restricted by MY_STATION_TCAM (512)
 */
#define SYS_ADPT_MAX_NBR_OF_L3_INTERFACE                   (512 + 1 )/* MY_STATION_TCAM(512) + loopback interface(1) */

#define SYS_ADPT_MAX_NBR_OF_IP_MULTICAST_VIFS     513
#define SYS_ADPT_MAX_NBR_OF_IPV6_MULTICAST_MIFS     SYS_ADPT_MAX_NBR_OF_IP_MULTICAST_VIFS


/*..    IP     component based adaptives        ..*/
/*
 *  For 3Com, rif-1 is for management purpose, can't remove, just modify.
 */
#define SYS_ADPT_OEM_MANAGEMENT_RIF                        1

#if(SYS_CPNT_VIRTUAL_IP == TRUE)
/* format is 00:12:CF:00:00:XX, byte5 is the last byte of virtual ip_addr */
#define VIRTUAL_MAC_BYTE0   0x00
#define VIRTUAL_MAC_BYTE1   0x12
#define VIRTUAL_MAC_BYTE2   0xCF
#define VIRTUAL_MAC_BYTE3   0x00
#define VIRTUAL_MAC_BYTE4   0x00
#endif
/*---ICMP---
 * Define the maximum size of ICMP Redirect Display Entries.
 */
#define SYS_ADPT_MAX_IP_ICMP_REDIRECT_DISPLAY_ENTRY        20

/*..    ROUTE  component based adaptives        ..*/
/*     static max allowed value -- 16 (base on 3Com spec.)
 *            min allowed value -- 1
 */
#define SYS_ADPT_MIN_STATIC_ROUTE_METRIC                   1
#define SYS_ADPT_MAX_STATIC_ROUTE_METRIC                   16

/* Administrative Distance */
/* By default
 * Connected Route                  0
 * Static Route                     1
 * BGP                              20
 * OSPF                             110
 * ISIS                             115
 * RIP                              120
 * Unkown                           255
 *
 * If the administrative distance is 255, the Router does not
 * believe the source of that route and does not install the
 * route in the routing table.
 */
#define SYS_ADPT_MIN_ROUTE_DISTANCE                         1
#define SYS_ADPT_MAX_ROUTE_DISTANCE                         255

/*---ROUTE---
 *  Define routing table size.
 *  Total routing entries :
 *     local routing entry : not write to chip. (same as rif number)
 *     static routing entry
 *     dynamic routing entry.
 */
/* Total route entry in chip is 12K, but due to cup performance issue, we expect 8K */
#define SYS_ADPT_MAX_NBR_OF_TOTAL_ROUTE_ENTRY               (16*SYS_TYPE_1K)
#define	SYS_ADPT_MAX_NBR_OF_STATIC_ROUTE_ENTRY				512
#define SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY             (SYS_ADPT_MAX_NBR_OF_TOTAL_ROUTE_ENTRY - \
                                                             SYS_ADPT_MAX_NBR_OF_RIF - \
                                                             SYS_ADPT_MAX_NBR_OF_STATIC_ROUTE_ENTRY)


/*---RIP---
 *  Define RIP used timer range. (ref. leaf_ES3626A.h)
 *      UpdateTime :
 *          MAX_ripUpdateTime (60)
 *          MIN_ripUpdateTime (15)
 *      Timeout
 *          MAX_ripTimeoutTime (360)
 *          MIN_ripTimeoutTime (90)
 *      Garbage collection
 *          MAX_ripGarbageCollectionTime (240)
 *          MIN_ripGarbageCollectionTime (60)
 */
#define SYS_ADPT_MAX_RIP_UPDATE_TIME                        MAX_ripUpdateTime
#define SYS_ADPT_MIN_RIP_UPDATE_TIME                        MIN_ripUpdateTime
#define SYS_ADPT_MAX_RIP_TIMEOUT_TIME                       MAX_ripTimeoutTime
#define SYS_ADPT_MIN_RIP_TIMEOUT_TIME                       MIN_ripTimeoutTime
#define SYS_ADPT_MAX_RIP_GARBAGE_COLLECTION_TIME            MAX_ripGarbageCollectionTime
#define SYS_ADPT_MIN_RIP_GARBAGE_COLLECTION_TIME            MIN_ripGarbageCollectionTime
#define SYS_ADPT_MIN_RIP_METRIC                             1
#define SYS_ADPT_MAX_RIP_METRIC                             15
#define SYS_ADPT_MAX_NBR_OF_RIP_ROUTE_ENTRY                 SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY/*Lin.Li, for rip support*/


/* -----  OSPF -----
 *  Define OSPF used constant, for database capacity reservation
 *      Area Table : It is depended on memory space
 *      Stub Area  : would be 8*Area-table size; but currently TOS=0. It seems that we don't implement TOS yet
 *      IF Table   : subnet-interface table; size same as RIF number.
 *      IF Metric Table : based on TOS, but currently TOS=0, so size is same as IF Table.
 *      VIRTual IF Table :
 *      AREA AFGGREGATE Table :
 *      Summary Address Table :
 *      NSSA Table :
 *      Network Area Table :
 */

#define SYS_ADPT_MAX_NBR_OF_OSPF_AREA                       5
#define SYS_ADPT_MAX_NBR_OF_OSPF_TOS                        1

/* The maximum of supporing stub area entry.
 * The corresponding command is "area X.X.X.X stub".
 * This value should be equal to the supporing area number.
 * (in case all existing area are stub or nssa area)
 * Generally speaking, this entry should be support TOS, but we do not support yet.
 * therefore, it is equal to the maximum of supporting area.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_STUB_AREA                (SYS_ADPT_MAX_NBR_OF_OSPF_AREA * SYS_ADPT_MAX_NBR_OF_OSPF_TOS)

/* The OSPF routing interface entry. This value should be equal to the maximum support RIF.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_IF_ENTRY                 SYS_ADPT_MAX_NBR_OF_RIF

/* Each Interface can has its own metric. The same IF can set different metric on
 * different TOS.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_IF_METRIC_ENTRY          (SYS_ADPT_MAX_NBR_OF_OSPF_IF_ENTRY * SYS_ADPT_MAX_NBR_OF_OSPF_TOS)

/* This is used to stored the virtual link entry.
 * The corresponding command is "area x.x.x.x virtual-link".
 * This vlaue depends on what proper value we want to support.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_VIRT_IF_ENTRY            5

/* The summary of routes on a specific area.
 * The corresponding command is "area X.X.X.X range".
 * Fore every entry, it stores the summary range on a specific area.
 * In current state, this function will only summary Type-3 routes.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_AREA_AGGREGATE_ENTRY     20

/* For AS-external LSA, we can use "(Router-OSPF)summary-address X.X.X.X X.X.X.X"
 * to summarize the matching existing AS-External LSAs to be a single one AS-Ex LSA.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_SUMMARY_ADDRESS          20

/* It stores for a specific nssa area, what is its redistribution policy and default
 * information injection policy. This entry should also support TOS.
 * But in current state, we don't support TOS yet.
 * This value should be treated as SYS_ADPT_MAX_NBR_OF_OSPF_STUB_AREA.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_NSSA_ENTRY               SYS_ADPT_MAX_NBR_OF_OSPF_STUB_AREA

/* The number of networks can be added to an area.
 * the corresponding command is "(Router-OSPF)network X.X.X.X X.X.X.X area X.X.X.X".
 * This value should not be greater than MAX RIF number.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_NETWORK_AREA_ENTRY       16

/* This entry stores the protocols we support to redistribute their route into OSPF.
 * In current state, we only support static and rip.
 * Therefore, this value should be 2.
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_REDISTRIBUTE_ENTRY        2


/* --OSPF--
 *  Define OSPF MIB range(ref. Leaf_1850.h)
 *      External Lsdb Limit Range:
 *          MIN_ospfExtLsdbLimit (-1)
 *          MAX_ospfExtLsdbLimit (2147483647)
 *      Exit Over Flow Interval Range:
 *          MIN_ospfExitOverflowInterval (0)
 *          MAX_ospfExitOverflowInterval (2147483647)
 *      Stub TOS Range:
 *          MIN_ospfStubTOS (0)
 *          MAX_ospfStubTOS (30)
 *      Stub Metric Range:
 *          MIN_ospfStubMetric (0)
 *          MAX_ospfStubMetric (16777215)
 *      Link State Database Advertisement Range:
 *          MINSIZE_ospfLsdbAdvertisement (1)
 *          MAXSIZE_ospfLsdbAdvertisement (65535)
 *      Host TOS Range:
 *          MIN_ospfHostTOS (0)
 *          MAX_ospfHostTOS (30)
 *      Host Metric Range:
 *          MIN_ospfHostMetric (0)
 *          MAX_ospfHostMetric (65535)
 *      If Entry Router Priority Range:
 *          MIN_ospfIfRtrPriority (0)
 *          MAX_ospfIfRtrPriority (255)
 *      If Entry Transit Delay Range:
 *          MIN_ospfIfTransitDelay (0)
 *          MAX_ospfIfTransitDelay (3600)
 *      If Entry Retrans Delay Range:
 *          MIN_ospfIfRetransInterval (0)
 *          MAX_ospfIfRetransInterval (3600)
 *      If Entry Hello Interval Range:
 *          MIN_ospfIfHelloInterval (1)
 *          MAX_ospfIfHelloInterval (65535)
 *      If Entry Rtr Dead Interval Range:
 *          MIN_ospfIfRtrDeadInterval (1)
 *          MAX_ospfIfRtrDeadInterval (2147483647)
 *      If Entry Poll Interval Range:
 *          MIN_ospfIfPollInterval (0)
 *          MAX_ospfIfPollInterval (2147483647)
 *      Interface Entry Authentication Key Size Range:
 *          MINSIZE_ospfIfAuthKey (0)
 *          MAXSIZE_ospfIfAuthKey (256)
 *      Interface Entry Authentication Key Type Range:
 *          MIN_ospfIfAuthType (0)
 *          MAX_ospfIfAuthType (255)
 *      If Entry Metric TOS Range:
 *          MIN_ospfIfMetricTOS (0)
 *          MAX_ospfIfMetricTOS (30)
 *      If Entry Metric Value Range:
 *          MIN_ospfIfMetricValue (0)
 *          MAX_ospfIfMetricValue (65535)
 *      Virtual Interface Entry Transit Delay Range:
 *          MIN_ospfVirtIfTransitDelay (0)
 *          MAX_ospfVirtIfTransitDelay (3600)
 *      Virtual Interface Entry Retrans Interval Range:
 *          MIN_ospfVirtIfRetransInterval (0)
 *          MAX_ospfVirtIfRetransInterval (3600)
 *      Virtual Interface Entry Hello Interval Range:
 *          MIN_ospfVirtIfHelloInterval (1)
 *          MAX_ospfVirtIfHelloInterval (65535)
 *      Virtual Interface Entry Rtr Dead Interval Range:
 *          MIN_ospfVirtIfRtrDeadInterval (0)
 *          MAX_ospfVirtIfRtrDeadInterval (2147483647)
 *      Virtual Interface Entry Authentication Key SizeRange:
 *          MINSIZE_ospfVirtIfAuthKey (0)
 *          MAXSIZE_ospfVirtIfAuthKey (256)
 *      Virtual Interface Entry Authentication Key Type Range:
 *          MIN_ospfVirtIfAuthType (0)
 *          MAX_ospfVirtIfAuthType (255)
 *      Nbr Entry Priority Range:
 *          MIN_ospfNbrPriority (0)
 *          MAX_ospfNbrPriority (255)
 *      External Link State Database Advertisement Size:
 *          SIZE_ospfExtLsdbAdvertisement (36)
 *      External Link State Database Advertisement Size:
 *          SIZE_ospfExtLsdbAdvertisement (36)
 *
 */
#define SYS_ADPT_MIN_OSPF_PROCESS_ID                      1
#define SYS_ADPT_MAX_OSPF_PROCESS_ID                      65535
#define SYS_ADPT_MAX_OSPF_PROCESS_NBR                     10
#define SYS_ADPT_MIN_OSPF_EXT_LSDB_LIMIT                  MIN_ospfExtLsdbLimit
#define SYS_ADPT_MAX_OSPF_EXT_LSDB_LIMIT                  MAX_ospfExtLsdbLimit
#define SYS_ADPT_MIN_OSPF_EXIT_OVERFLOW_INTERVAL          MIN_ospfExitOverflowInterval
#define SYS_ADPT_MAX_OSPF_EXIT_OVERFLOW_INTERVAL          MAX_ospfExitOverflowInterval
#define SYS_ADPT_MIN_OSPF_STUB_TOS                        MIN_ospfStubTOS
#define SYS_ADPT_MAX_OSPF_STUB_TOS                        MAX_ospfStubTOS
#define SYS_ADPT_MIN_OSPF_STUB_METRIC                     MIN_ospfStubMetric
#define SYS_ADPT_MAX_OSPF_STUB_METRIC                     MAX_ospfStubMetric
#define SYS_ADPT_MINSIZE_OSPF_LSDB_ADVERTISEMENT          MINSIZE_ospfLsdbAdvertisement
#define SYS_ADPT_MAXSIZE_OSPF_LSDB_ADVERTISEMENT          MAXSIZE_ospfLsdbAdvertisement
#define SYS_ADPT_MIN_OSPF_HOST_TOS                        MIN_ospfHostTOS
#define SYS_ADPT_MAX_OSPF_HOST_TOS                        MAX_ospfHostTOS
#define SYS_ADPT_MIN_OSPF_HOST_METRIC                     MIN_ospfHostMetric
#define SYS_ADPT_MAX_OSPF_HOST_METRIC                     MAX_ospfHostMetric
#define SYS_ADPT_MIN_OSPF_IF_RTR_PRIORITY                 MIN_ospfIfRtrPriority
#define SYS_ADPT_MAX_OSPF_IF_RTR_PRIORITY                 MAX_ospfIfRtrPriority
#define SYS_ADPT_MIN_OSPF_IF_TRANSIT_DELAY                MIN_ospfIfTransitDelay
#define SYS_ADPT_MAX_OSPF_IF_TRANSIT_DELAY                MAX_ospfIfTransitDelay
#define SYS_ADPT_MIN_OSPF_IF_RETRANS_DELAY                MIN_ospfIfRetransInterval
#define SYS_ADPT_MAX_OSPF_IF_RETRANS_DELAY                MAX_ospfIfRetransInterval
#define SYS_ADPT_MIN_OSPF_IF_HELLO_INTERVAL               MIN_ospfIfHelloInterval
#define SYS_ADPT_MAX_OSPF_IF_HELLO_INTERVAL               MAX_ospfIfHelloInterval
#define SYS_ADPT_MIN_OSPF_IF_RTR_DEAD_INTERVAL            MIN_ospfIfRtrDeadInterval

#define SYS_ADPT_MAX_OSPF_IF_RTR_DEAD_INTERVAL            65535
#define SYS_ADPT_MIN_OSPF_IF_POLL_INTERVAL                MIN_ospfIfPollInterval
#define SYS_ADPT_MAX_OSPF_IF_POLL_INTERVAL                MAX_ospfIfPollInterval
#define SYS_ADPT_MINSIZE_OSPF_IF_AUTH_KEY                 MINSIZE_ospfIfAuthKey
#define SYS_ADPT_MAXSIZE_OSPF_IF_AUTH_KEY                 17
#define SYS_ADPT_MIN_OSPF_IF_AUTH_TYPE                    MIN_ospfIfAuthType
#define SYS_ADPT_MAX_OSPF_IF_AUTH_TYPE                    MAX_ospfIfAuthType
#define SYS_ADPT_MIN_OSPF_IF_METRIC_TOS                   MIN_ospfIfMetricTOS
#define SYS_ADPT_MAX_OSPF_IF_METRIC_TOS                   MAX_ospfIfMetricTOS
#define SYS_ADPT_MIN_OSPF_IF_METRIC_VALUE                 MIN_ospfIfMetricValue
#define SYS_ADPT_MAX_OSPF_IF_METRIC_VALUE                 MAX_ospfIfMetricValue
#define SYS_ADPT_MIN_OSPF_VIRT_IF_TRANSIT_DELAY           MIN_ospfVirtIfTransitDelay
#define SYS_ADPT_MAX_OSPF_VIRT_IF_TRANSIT_DELAY           MAX_ospfVirtIfTransitDelay
#define SYS_ADPT_MIN_OSPF_VIRT_IF_RETRANS_INTERVAL        MIN_ospfVirtIfRetransInterval
#define SYS_ADPT_MAX_OSPF_VIRT_IF_RETRANS_INTERVAL        MAX_ospfVirtIfRetransInterval
#define SYS_ADPT_MIN_OSPF_VIRT_IF_HELLO_INTERVAL          MIN_ospfVirtIfHelloInterval
#define SYS_ADPT_MAX_OSPF_VIRT_IF_HELLO_INTERVAL          MAX_ospfVirtIfHelloInterval
#define SYS_ADPT_MIN_OSPF_VIRT_IF_RTR_DEAD_INTERVAL       MIN_ospfVirtIfRtrDeadInterval
/*#define SYS_ADPT_MAX_OSPF_VIRT_IF_RTR_DEAD_INTERVAL       MAX_ospfVirtIfRtrDeadInterval */
#define SYS_ADPT_MAX_OSPF_VIRT_IF_RTR_DEAD_INTERVAL       65535
#define SYS_ADPT_MINSIZE_OSPF_VIRT_IF_AUTH_KEY            MINSIZE_ospfVirtIfAuthKey
#define SYS_ADPT_MAXSIZE_OSPF_VIRT_IF_AUTH_KEY            17
#define SYS_ADPT_MIN_OSPF_VIRT_IF_AUTH_KEY_TYPE           MIN_ospfVirtIfAuthType
#define SYS_ADPT_MAX_OSPF_VIRT_IF_AUTH_KEY_TYPE           MAX_ospfVirtIfAuthType
#define SYS_ADPT_MIN_OSPF_NBR_PRIORITY                    MIN_ospfNbrPriority
#define SYS_ADPT_MAX_OSPF_NBR_PRIORITY                    MAX_ospfNbrPriority
#define SYS_ADPT_SIZE_OSPF_EXT_LSDB_ADVERTISEMENT         SIZE_ospfExtLsdbAdvertisement
#define SYS_ADPT_OSPF_AUTH_TYPE_NONE             0
#define SYS_ADPT_OSPF_AUTH_TYPE_SIMPLE_PASSWORD  1
#define SYS_ADPT_OSPF_AUTH_TYPE_MD5              2
/* It will be used , but not completed yet
 */
#define SYS_ADPT_MAX_NBR_OF_OSPF_AUTHENCATION_KEY_LEN     8
#define SYS_ADPT_MAX_NBR_OF_OSPF_AUTHENCATION_MD5KEY_LEN  16

/* rfc1850: simple password key length must be 8
 * MD5 key length must be 16
 */
#define SYS_ADPT_OSPF_SIMPLE_PASSWORD_KEY_LEN             8
#define SYS_ADPT_OSPF_MD5_AUTHENCATION_KEY_LEN            16





/*
 *   Define OSPF private MIB range (ref. leaf_ES3626A.h)
 *      Default External Mertic :
 *          MAX_ospfDefaultExternalMetric (16777214)
 *          MIN_ospfDefaultExternalMetric (0)
 *      Redistribute Mertic :
 *          MAX_ospfRedistributeMetric (16777214)
 *          MIN_ospfRedistributeMetric (0)
 *      SPF Hold Time :
 *          MAX_ospfSpfHoldTime        (2147483647)
 *          MIN_ospfSpfHoldTime        (0)
 *      SPF Delay Time :
 *          MAX_ospfSpfDelayTime       (2147483647)
 *          MIN_ospfSpfDelayTime       (0)
 *
 *
 */
/* CLI: default-information originate metric */
#define SYS_ADPT_MAX_OSPF_DEFAULT_EXTERNAL_METRIC         MAX_ospfMultiProcessDefaultExternalMetric
#define SYS_ADPT_MIN_OSPF_DEFAULT_EXTERNAL_METRIC         0 /* Note: MIN_ospfMultiProcessDefaultExternalMetric (-1) is for unspecified */
/* CLI: default-metric */
#define SYS_ADPT_MAX_OSPF_DEFAULT_METRIC                  MAX_ospfMultiProcessDefaultMetric
#define SYS_ADPT_MIN_OSPF_DEFAULT_METRIC                  0 /* Note: MIN_ospfMultiProcessDefaultMetric (-1) is for unspecified */
#define SYS_ADPT_MAX_OSPF_REDISTRIBUTE_METRIC             MAX_ospfRedistributeMetric
#define SYS_ADPT_MIN_OSPF_REDISTRIBUTE_METRIC             MIN_ospfRedistributeMetric
#define SYS_ADPT_MAX_OSPF_SPF_HOLD_TIME                   MAX_ospfMultiProcessSpfHoldTime
#define SYS_ADPT_MIN_OSPF_SPF_HOLD_TIME                   MIN_ospfMultiProcessSpfHoldTime
#define SYS_ADPT_MAX_OSPF_SPF_DELAY_TIME                  MAX_ospfMultiProcessSpfDelayTime
#define SYS_ADPT_MIN_OSPF_SPF_DELAY_TIME                  MIN_ospfMultiProcessSpfDelayTime
#define SYS_ADPT_MIN_OSPF_NSSA_DEFAULT_INFORMATION_METRIC              0
#define SYS_ADPT_MAX_OSPF_NSSA_DEFAULT_INFORMATION_METRIC              16777214
/* ----- End of OSPF -----  */

/*
 *  OSPFv3 settings
 */
#define SYS_ADPT_OSPF6_MAX_NBR_OF_INSTANCE      10
#define SYS_ADPT_OSPF6_MAX_NBR_OF_PROCESS       10
#define SYS_ADPT_OSPF6_MAX_NBR_OF_AREA_IN_PROC  10
#define SYS_ADPT_OSPF6_MAX_NBR_OF_AREARANGE     10
#define SYS_ADPT_OSPF6_MAX_NBR_OF_VLINK         10
#define SYS_ADPT_OSPF6_MAX_NEXTHOP              8

/* IPv6 Neighbor Discovery (ND) */
#define SYS_ADPT_ND_MAX_RETRANS_INTERVAL    0xffffffff /* 4294967295, milliseconds */
#define SYS_ADPT_ND_MIN_RETRANS_INTERVAL    1000
#define SYS_ADPT_ND_MAX_REACHABLE_TIME      3600000     /* milliseconds */
#define SYS_ADPT_ND_MIN_REACHABLE_TIME      1000

#define SYS_ADPT_ND_MAX_VALID_LIFETIME      0xffffffff  /* seconds */
#define SYS_ADPT_ND_MIN_VALID_LIFETIME      0

#define SYS_ADPT_ND_MAX_PREFERRED_LIFETIME  0xffffffff  /* seconds */
#define SYS_ADPT_ND_MIN_PREFERRED_LIFETIME  0


/*
 * For BGP
 */
#define SYS_ADPT_BGP_MAX_NBR_OF_INSTANCE                    1
#define SYS_ADPT_BGP_MAX_NBR_OF_ROUTE_PER_INSTANCE          (2*SYS_ADPT_MAX_NBR_OF_DYNAMIC_ROUTE_ENTRY) /* accumulated of all neighbor learned routes */
#define SYS_ADPT_BGP_MAX_NETWORK_PER_INSTANCE               1024
#define SYS_ADPT_BGP_MAX_NEIGHBOR_PER_INSTANCE              512
#define SYS_ADPT_BGP_MAX_CONFEDERATION_PEER_PER_INSATNCE    10
#define SYS_ADPT_BGP_MAX_DISTANCE_PER_INSATNCE              64
#define SYS_ADPT_BGP_MAX_PEER_GROUP_PER_INSTANCE            64
#define SYS_ADPT_BGP_MAX_PEER_PER_PEER_GROUP                64
#define SYS_ADPT_BGP_MAX_AGGREGATE_ADDR_PER_INSTANCE        64
#define SYS_ADPT_BGP_MIN_AS4_NUMBER                         1
#define SYS_ADPT_BGP_MAX_AS4_NUMBER                         0xffffffff  /* 4294967295 */
#define SYS_ADPT_BGP_MIN_CLUSTER_ID                         1
#define SYS_ADPT_BGP_MAX_CLUSTER_ID                         0xffffffff  /* 4294967295 */
#define SYS_ADPT_BGP_MIN_CONFEDERATION_ID                   1
#define SYS_ADPT_BGP_MAX_CONFEDERATION_ID                   0xffffffff  /* 4294967295 */
#define SYS_ADPT_BGP_MIN_DAMPENING_HALF_LIFE_TIME           1           /* minutes */
#define SYS_ADPT_BGP_MAX_DAMPENING_HALF_LIFE_TIME           45
#define SYS_ADPT_BGP_MIN_DAMPENING_REUSE_LIMIT              1
#define SYS_ADPT_BGP_MAX_DAMPENING_REUSE_LIMIT              20000
#define SYS_ADPT_BGP_MIN_DAMPENING_SUPPRESS_LIMIT           1
#define SYS_ADPT_BGP_MAX_DAMPENING_SUPPRESS_LIMIT           20000
#define SYS_ADPT_BGP_MIN_DAMPENING_MAX_SUPRESS_TIME         1           /* minutes */
#define SYS_ADPT_BGP_MAX_DAMPENING_MAX_SUPRESS_TIME         255
#define SYS_ADPT_BGP_MIN_LOCAL_PREFERENCE                   1
#define SYS_ADPT_BGP_MAX_LOCAL_PREFERENCE                   0xffffffff  /* 4294967295 */
#define SYS_ADPT_BGP_MIN_SCAN_TIME                          5           /* seconds */
#define SYS_ADPT_BGP_MAX_SCAN_TIME                          60          /* seconds */
#define SYS_ADPT_BGP_MIN_ADMINISTRATIVE_DISTANCE            1
#define SYS_ADPT_BGP_MAX_ADMINISTRATIVE_DISTANCE            255
#define SYS_ADPT_BGP_MIN_PATHLIMIT_TTL                      0
#define SYS_ADPT_BGP_MAX_PATHLIMIT_TTL                      255
#define SYS_ADPT_BGP_NEIGHBOR_MAXIMUM_PREFIX                0xffffffff
#define SYS_ADPT_BGP_MIN_REDISTRIBUTE_METRIC                0
#define SYS_ADPT_BGP_MAX_REDISTRIBUTE_METRIC                0xffffffff
#define SYS_ADPT_BGP_MIN_KEEP_ALIVE_INTERVAL                0
#define SYS_ADPT_BGP_MAX_KEEP_ALIVE_INTERVAL                65535
#define SYS_ADPT_BGP_MIN_HOLD_TIME                          0
#define SYS_ADPT_BGP_MAX_HOLD_TIME                          65535
#define SYS_ADPT_BGP_MIN_NEIGHBOR_CONNECT_RETRY_INTERVAL    0
#define SYS_ADPT_BGP_MAX_NEIGHBOR_CONNECT_RETRY_INTERVAL    65535
#define SYS_ADPT_BGP_MIN_NEIGHBOR_ADVERTISEMENT_INTERVAL    0
#define SYS_ADPT_BGP_MAX_NEIGHBOR_ADVERTISEMENT_INTERVAL    600
#define SYS_ADPT_BGP_MIN_NEIGHBOR_ALLOW_AS_IN               1
#define SYS_ADPT_BGP_MAX_NEIGHBOR_ALLOW_AS_IN               10
#define SYS_ADPT_BGP_MIN_NEIGHBOR_WEIGHT                    0
#define SYS_ADPT_BGP_MAX_NEIGHBOR_WEIGHT                    65535
#define SYS_ADPT_BGP_MAX_PEER_GROUP_NAME_LENGTH             80
#define SYS_ADPT_BGP_MAX_PEER_STRING_LENGTH                 80  /* store ip addr str or peer group name str */
#define SYS_ADPT_BGP_MAX_CLEAR_BGP_ARGUMENT_STRING_LENGTH   256
#define SYS_ADPT_BGP_MAX_NEIGHBOR_DESCRIPTION_LENGTH        80
#define SYS_ADPT_BGP_MD5_AUTHENCATION_KEY_LEN               25


/* policy related:  as path access list, community, ext-community, ip prefix list, route map */
#define SYS_ADPT_STANDARD_COMMUNITY_MIN_NUMBER          1
#define SYS_ADPT_STANDARD_COMMUNITY_MAX_NUMBER          99
#define SYS_ADPT_EXPANDED_COMMUNITY_MIN_NUMBER          100
#define SYS_ADPT_EXPANDED_COMMUNITY_MAX_NUMBER          500
#define SYS_ADPT_MIN_IP_PREFIX_LENGTH                   0
#define SYS_ADPT_MAX_IP_PREFIX_LENGTH                   32
#define SYS_ADPT_MIN_IP_PREFIX_LIST_SEQUENCE_NUMBER     1
#define SYS_ADPT_MAX_IP_PREFIX_LIST_SEQUENCE_NUMBER     0xffffffff
#define SYS_ADPT_MAX_NBR_OF_ROUTE_MAP                   512
#define SYS_ADPT_MIN_ROUTE_MAP_SEQUENCE_NUMBER          1
#define SYS_ADPT_MAX_ROUTE_MAP_SEQUENCE_NUMBER          65535
#define SYS_ADPT_MAX_SEQUENCE_PER_ROUTE_MAP             128
#define SYS_ADPT_MIN_ROUTE_MAP_METRIC                   0
#define SYS_ADPT_MAX_ROUTE_MAP_METRIC                   0xffffffff
#define SYS_ADPT_MIN_ROUTE_MAP_WEIGHT                   0
#define SYS_ADPT_MAX_ROUTE_MAP_WEIGHT                   0xffffffff
#define SYS_ADPT_MAX_ACCESS_LIST_NAME_LENGTH            32
#define SYS_ADPT_MAX_ROUTE_MAP_NAME_LENGTH              32
#define SYS_ADPT_MAX_PREFIX_LIST_NAME_LENGTH            32
#define SYS_ADPT_MAX_AS_PATH_ACCESS_LIST_NAME_LENGTH    32
#define SYS_ADPT_MAX_COMMUNITY_NAME_LENGTH              32
#define SYS_ADPT_MAX_ROUTE_MAP_COMMAND_LENGTH           32
#define SYS_ADPT_MAX_ROUTE_MAP_ARGUMENT_LENGTH          200 //16 items of 4294967295 and 15 spaces = 175 // old is 80
#define SYS_ADPT_MAX_REGULAR_EXPRESSION_LENGTH          200 // 16 items of 65535:65535 and 15 spaces = 191. //80

/* --IGMP--
 *  Define IGMP MIB range(ref. Leaf_2933.h)
 *      Interface index range:
 *          MIN_igmpInterfaceIfIndex (1)
 *          MAX_igmpInterfaceIfIndex (2147483647)
 *      Interface query max response time range:
 *          MIN_igmpInterfaceQueryMaxResponseTime (0)
 *          MAX_igmpInterfaceQueryMaxResponseTime (255)
 *      Proxy interface index range:
 *          MIN_igmpInterfaceProxyIfIndex (1)
 *          MAX_igmpInterfaceProxyIfIndex (2147483647)
 *      Interface robustness variable
 *          MIN_igmpInterfaceRobustness (1)
 *          MAX_igmpInterfaceRobustness (255)
 *      Interface last member query interval
 *          MIN_igmpInterfaceLastMembQueryIntvl (0)
 *          MAX_igmpInterfaceLastMembQueryIntvl (255)
 *      Interface ifindex of igmp cache
 *          MIN_igmpCacheIfIndex (1)
 *          MAX_igmpCacheIfIndex (2147483647)
 *
 */
#define SYS_ADPT_MIN_IGMP_INTERFACE_IFINDEX                     MIN_igmpInterfaceIfIndex
#define SYS_ADPT_MAX_IGMP_INTERFACE_IFINDEX                     MAX_igmpInterfaceIfIndex
#define SYS_ADPT_MIN_IGMP_INTERFACE_QUERY_MAX_RESPONSE_TIME     MIN_igmpInterfaceQueryMaxResponseTime
#define SYS_ADPT_MAX_IGMP_INTERFACE_QUERY_MAX_RESPONSE_TIME     MAX_igmpInterfaceQueryMaxResponseTime
#define SYS_ADPT_MIN_IGMP_INTERFACE_PROXY_IFINDEX               MIN_igmpInterfaceProxyIfIndex
#define SYS_ADPT_MAX_IGMP_INTERFACE_PROXY_IFINDEX               MAX_igmpInterfaceProxyIfIndex
#define SYS_ADPT_MIN_IGMP_INTERFACE_ROBUSTNESS                  MIN_igmpInterfaceRobustness
#define SYS_ADPT_MAX_IGMP_INTERFACE_ROBUSTNESS                  MAX_igmpInterfaceRobustness
#define SYS_ADPT_MIN_IGMP_INTERFACE_LAST_MEMB_QUERY_INTERVAL    MIN_igmpInterfaceLastMembQueryIntvl
#define SYS_ADPT_MAX_IGMP_INTERFACE_LAST_MEMB_QUERY_INTERVAL    MAX_igmpInterfaceLastMembQueryIntvl
#define SYS_ADPT_MIN_IGMP_CACHE_IFINDEX                         MIN_igmpCacheIfIndex
#define SYS_ADPT_MAX_IGMP_CACHE_IFINDEX                         MAX_igmpCacheIfIndex


/* --DVMRP--
 *  Define DVMRP MIB range(ref. Leaf_dvmrp.h)
 *      Dvmrp version string size range:
 *          MINSIZE_dvmrpVersionString (0)
 *          MAXSIZE_dvmrpVersionString (255)
 *      Dvmrp interface index range:
 *          MIN_dvmrpInterfaceIndex (1)
 *          MAX_dvmrpInterfaceIndex (2147483647)
 *      Dvmrp interface metric range:
 *          MIN_dvmrpInterfaceMetric (1)
 *          MAX_dvmrpInterfaceMetric (31)
 *      Dvmrp interface authentication key size range:
 *          MINSIZE_dvmrpInterfaceKey (0)
 *          MAXSIZE_dvmrpInterfaceKey (255)
 *      Dvmrp neighrob interface index range:
 *          MIN_dvmrpNeighborIfIndex (1)
 *          MAX_dvmrpNeighborIfIndex (2147483647)
 *      Dvmrp neighbor major version range:
 *          MIN_dvmrpNeighborMajorVersion (0)
 *          MAX_dvmrpNeighborMajorVersion (255)
 *      Dvmrp neighbor minor version range:
 *          MIN_dvmrpNeighborMinorVersion (0)
 *          MAX_dvmrpNeighborMinorVersion (255)
 *      Dvmrp route interface index range:
 *          MIN_dvmrpRouteIfIndex (0)
 *          MAX_dvmrpRouteIfIndex (2147483647)
 *      Dvmrp route metric range:
 *          MIN_dvmrpRouteMetric (0)
 *          MAX_dvmrpRouteMetric (32)
 *      Dvmrp route nexthop interface index range:
 *          MIN_dvmrpRouteNextHopIfIndex (1)
 *          MAX_dvmrpRouteNextHopIfIndex (2147483647)
 *
 */
#define SYS_ADPT_MINSIZE_DVMRP_VERSION_STRING                   MINSIZE_dvmrpVersionString
#define SYS_ADPT_MAXSIZE_DVMRP_VERSION_STRING                   MAXSIZE_dvmrpVersionString
#define SYS_ADPT_MIN_DVMRP_INTERFACE_INDEX                      MIN_dvmrpInterfaceIndex
#define SYS_ADPT_MAX_DVMRP_INTERFACE_INDEX                      MAX_dvmrpInterfaceIndex
#define SYS_ADPT_MIN_DMVRP_INTERFACE_METRIC                     MIN_dvmrpInterfaceMetric
#define SYS_ADPT_MAX_DMVRP_INTERFACE_METRIC                     MAX_dvmrpInterfaceMetric
#define SYS_ADPT_MINSIZE_DVMRP_INTERFACE_KEY                    MINSIZE_dvmrpInterfaceKey
#define SYS_ADPT_MAXSIZE_DVMRP_INTERFACE_KEY                    MAXSIZE_dvmrpInterfaceKey
#define SYS_ADPT_MIN_DVMRP_NEIGHBOR_IFINDEX                     MIN_dvmrpNeighborIfIndex
#define SYS_ADPT_MAX_DVMRP_NEIGHBOR_IFINDEX                     MAX_dvmrpNeighborIfIndex
#define SYS_ADPT_MIN_DVMRP_NEIGHBOR_MAJOR_VERSION               MIN_dvmrpNeighborMajorVersion
#define SYS_ADPT_MAX_DVMRP_NEIGHBOR_MAJOR_VERSION               MAX_dvmrpNeighborMajorVersion
#define SYS_ADPT_MIN_DVMRP_NEIGHBOR_MINOR_VERSION               MIN_dvmrpNeighborMinorVersion
#define SYS_ADPT_MAX_DVMRP_NEIGHBOR_MINOR_VERSION               MAX_dvmrpNeighborMinorVersion
#define SYS_ADPT_MIN_DVMRP_ROUTE_IFINDEX                        MIN_dvmrpRouteIfIndex
#define SYS_ADPT_MAX_DVMRP_ROUTE_IFINDEX                        MAX_dvmrpRouteIfIndex
#define SYS_ADPT_MIN_DVMRP_ROUTE_METRIC                         MIN_dvmrpRouteMetric
#define SYS_ADPT_MAX_DVMRP_ROUTE_METRIC                         MAX_dvmrpRouteMetric
#define SYS_ADPT_MIN_DVMRP_ROUTE_NEXTHOP_IFINDEX                MIN_dvmrpRouteNextHopIfIndex
#define SYS_ADPT_MAX_DVMRP_ROUTE_NEXTHOP_IFINDEX                MAX_dvmrpRouteNextHopIfIndex

#define SYS_ADPT_MAX_NBR_OF_TOTAL_DVMRP_ROUTE_ENTRY             512
#define SYS_ADPT_MAX_NBR_OF_STATIC_DVMRP_ROUTE_ENTRY            0
#define SYS_ADPT_MAX_NBR_OF_DYNAMIC_DVMRP_ROUTE_ENTRY           (SYS_ADPT_MAX_NBR_OF_TOTAL_DVMRP_ROUTE_ENTRY - \
                                                                 SYS_ADPT_MAX_NBR_OF_STATIC_DVMRP_ROUTE_ENTRY - \
                                                                SYS_ADPT_MAX_NBR_OF_RIF)

/* Multicast routing entry */
#if (SYS_CPNT_MULTICAST_USING_IPMC != TRUE)
#define SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY                 1023
#else
#define SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY                 1024
#endif

#define SYS_ADPT_MULTICAST_FORWARDING_PRIORITY                  SYS_ADPT_CPU_QUEUE_UNKNOW_IPMC

/*  2004.09.24, ruliang, add for IPMFS  */
#define SYS_ADPT_IPMFS_MAX_REPORT_ENTRIES                 16
#define SYS_ADPT_IPMFS_FLAG_NORMAL 0
#define SYS_ADPT_IPMFS_FLAG_PRUNE   1
#define SYS_ADPT_IPMFS_FLAG_ORIGINATOR 2
#define SYS_ADPT_IPMFS_FLAG_DATA_RATE_METER 3

#define SYS_ADPT_IPMFS_DRM_INTV 1   /* every 1 seconds */
#define SYS_ADPT_IPMFS_PRU_INTV  5  /* every 5 seconds */
#define SYS_ADPT_IPMFS_DEF_INTV 60 /* every 60 seconds */
#define SYS_ADPT_IPMFS_DEF_REP_INTV 180 /* every 180 seconds */
/*  end of  2004.09.24, ruliang  */

/* Multicast route entry ageout time (Seconds(currently, 10 minutes)) */
#define SYS_ADPT_MAX_MULTICAST_ENTRY_AGE_TIME                   600

/*
 *  For each chip, must have limitation about H/W forwarding, for different chipset,
 *  use SYS_ADPT_MAX_NBR_OF_VLANS_PER_PORT_FOR_MULTICAST_GROUP to define the depth of m'cast group
 *  on a port. Ie. how many vlans can be forwarded by chip on a port ? eg. BCM 5615 can forward
 *  one vlan for a group per port.
 *  More explaination :
 *  Port-6 joins vlan 1,2,3 and subnet N1 on vlan-1, N2 on vlan-2, N3 on vlan-3; if a m'cast client
 *  register m'cast group at vlan2, and another m'cast client register at vlan3, the secondary client
 *  will reject vlan3; because BCM 5615 only support 1 (SYS_ADPT_MAX_NBR_OF_VLANS_PER_PORT_FOR_MULTICAST_GROUP).
 */
#define SYS_ADPT_MAX_NBR_OF_VLANS_PER_PORT_FOR_MULTICAST_GROUP  1

/* --PIM--
 *  Define PIM MIB range(ref. Leaf_2934.h)
 *      Pim interface index range:
 *          MIN_pimInterfaceIfIndex     (1)
 *          MAX_pimInterfaceIfIndex     (2147483647)
 *      Pim interface hello interval range:
 *          MIN_pimInterfaceHelloInterval   (1)
 *          MAX_pimInterfaceHelloInterval   (65535)
 *      Pim interface trigger hello interval range:
 *          MIN_pimInterfaceTrigHelloInterval   (0)
 *          MAX_pimInterfaceTrigHelloInterval   (65535)
 *      Pim interface hello holdtime range:
 *          MIN_pimInterfaceHelloHoldtime   (0)
 *          MAX_pimInterfaceHelloHoldtime   (65535)
 *      Pim interface join/prune holdtime range:
 *          MIN_pimInterfaceJoinPruneHoldtime   (0)
 *          MAX_pimInterfaceJoinPruneHoldtime   (65535)
 *      Pim interface graft retry interval range:
 *          MIN_pimInterfaceGraftRetryInterval  (0)
 *          MAX_pimInterfaceGraftRetryInterval  (65535)
 *      Pim interface max graft retries range:
 *          MIN_pimInterfaceMaxGraftRetries     (0)
 *          MAX_pimInterfaceMaxGraftRetries     (65535)
 *
 */
#define SYS_ADPT_PIM_MAX_NUM_OF_STATIC_RP                       128
#define SYS_ADPT_PIM_MAX_NUM_OF_RP_CANDIDATE_GROUP              128
#define SYS_ADPT_PIM_MAX_NUM_OF_NEIGHBOR                        128
#define SYS_ADPT_MIN_PIM_INTERFACE_INDEX                        MIN_pimInterfaceIfIndex
#define SYS_ADPT_MAX_PIM_INTERFACE_INDEX                        MAX_pimInterfaceIfIndex
#if 0
#define SYS_ADPT_MIN_PIM_INTERFACE_HELLO_INTERVAL               1
#define SYS_ADPT_MAX_PIM_INTERFACE_HELLO_INTERVAL               65535
#define SYS_ADPT_MIN_PIM_INTERFACE_HELLO_HOLDTIME               1
#define SYS_ADPT_MAX_PIM_INTERFACE_HELLO_HOLDTIME               65535
#define SYS_ADPT_MIN_PIM_INTERFACE_TRIGGER_HELLO_INTERVAL       0
#define SYS_ADPT_MAX_PIM_INTERFACE_TRIGGER_HELLO_INTERVAL       65535
#define SYS_ADPT_MIN_PIM_INTERFACE_JOIN_PRUNE_HOLDTIME          0
#define SYS_ADPT_MAX_PIM_INTERFACE_JOIN_PRUNE_HOLDTIME          65535
#define SYS_ADPT_MIN_PIM_INTERFACE_GRAFT_RETRY_INTERVAL         0
#define SYS_ADPT_MAX_PIM_INTERFACE_GRAFT_RETRY_INTERVAL         65535
#define SYS_ADPT_MIN_PIM_INTERFACE_MAX_GRAFT_RETRIES            0
#define SYS_ADPT_MAX_PIM_INTERFACE_MAX_GRAFT_RETRIES            65535
#endif
#define SYS_ADPT_MIN_PIM_HELLO_INTERVAL             1   /* seconds */
#define SYS_ADPT_MIN_PIM_TRIGGERED_HELLO_DELAY      0   /* seconds */
#define SYS_ADPT_MIN_PIM_HELLO_HOLD_TIME            1   /* seconds */
#define SYS_ADPT_MIN_PIM_OVERRIDE_INTERVAL          500
#define SYS_ADPT_MIN_PIM_PROPAGATION_DELAY          100
#define SYS_ADPT_MIN_PIM_JOIN_PRUNE_INTERVAL        1   /* seconds */
#define SYS_ADPT_MIN_PIM_JOIN_PRUNE_HOLD_TIME       1   /* seconds */
#define SYS_ADPT_MIN_PIM_GRAFT_RETRY_INTERVAL       1   /* seconds */
#define SYS_ADPT_MIN_PIM_GRAFT_RETRY_COUNT          1
#define SYS_ADPT_MIN_PIM_STATE_REFRESH_INTERVAL     1   /* seconds, referred from ZebOS */
#define SYS_ADPT_MIN_PIM_REGISTER_RATE_LIMIT        1
#define SYS_ADPT_MIN_PIM_SWITCHOVER_THRESHOLD       0
#define SYS_ADPT_MIN_PIM_DESIGNATED_ROUTER_PRIORITY 0   /* referred from ZebOS */
#define SYS_ADPT_MIN_PIM_BSR_PRIORITY               0
#define SYS_ADPT_MIN_PIM_BSR_HASH_MASK_LEN          0
#define SYS_ADPT_MIN_PIM6_BSR_HASH_MASK_LEN          0
#define SYS_ADPT_MIN_PIM_RP_CANDIDATE_INTERVAL      60  /* referred from RFC5059 */
#define SYS_ADPT_MIN_PIM_RP_CANDIDATE_PRIORITY      0   /* referred from cisco */

#define SYS_ADPT_MAX_PIM_HELLO_INTERVAL             65535 /* seconds */
#define SYS_ADPT_MAX_PIM_TRIGGERED_HELLO_DELAY      5     /* seconds */
#define SYS_ADPT_MAX_PIM_HELLO_HOLD_TIME            65535 /* seconds */
#define SYS_ADPT_MAX_PIM_OVERRIDE_INTERVAL          6000
#define SYS_ADPT_MAX_PIM_PROPAGATION_DELAY          5000
#define SYS_ADPT_MAX_PIM_JOIN_PRUNE_INTERVAL        65535 /* seconds */
#define SYS_ADPT_MAX_PIM_JOIN_PRUNE_HOLD_TIME       65535 /* seconds */
#define SYS_ADPT_MAX_PIM_GRAFT_RETRY_INTERVAL       10    /* seconds */
#define SYS_ADPT_MAX_PIM_GRAFT_RETRY_COUNT          10
#define SYS_ADPT_MAX_PIM_STATE_REFRESH_INTERVAL     100   /* seconds, referred from ZebOS */
#define SYS_ADPT_MAX_PIM_REGISTER_RATE_LIMIT        65535
#define SYS_ADPT_MAX_PIM_SWITCHOVER_THRESHOLD       4294967
#define SYS_ADPT_MAX_PIM_DESIGNATED_ROUTER_PRIORITY 0xFFFFFFFE /* = 4294967294, referred from ZebOS */
#define SYS_ADPT_MAX_PIM_BSR_PRIORITY               255
#define SYS_ADPT_MAX_PIM_BSR_HASH_MASK_LEN          32      /* ipv4 group address max length */
#define SYS_ADPT_MAX_PIM6_BSR_HASH_MASK_LEN         128      /* ipv4 group address max length */
#define SYS_ADPT_MAX_PIM_RP_CANDIDATE_INTERVAL      16383   /* seconds, referred from cisco */
#define SYS_ADPT_MAX_PIM_RP_CANDIDATE_PRIORITY      255     /* referred from cisco */

/*..    Trace Route component  adaptives        ..*/
#define SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE              10  /* total number of current trace route */
#define SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROBE_PER_HOP            3
#define SYS_ADPT_TRACEROUTE_MAX_NBR_OF_HOP                      30
#define SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY       (SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROBE_PER_HOP * SYS_ADPT_TRACEROUTE_MAX_NBR_OF_HOP)
#define SYS_ADPT_TRACEROUTE_TOTAL_NBR_OF_FAIL_PROB              3
#define SYS_ADPT_TRACEROUTE_MAX_WAITTIME                        MAX_traceRouteCtlTimeOut
#define SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE                       16  /* Task Name and Test Name */
#define SYS_ADPT_TRACEROUTE_MAX_IP_ADDRESS_STRING_SIZE          18  /* MIB is 128 */
#define SYS_ADPT_TRACEROUTE_MAX_MISC_OPTIONS_SIZE               16  /* MIB is 32 */

/*
 *
 * L3 Driver related definition
 * All of them are to define the driver cache buffer size in slave unit.
 *
 */
#define SYS_ADPT_MAX_NBR_OF_HOST_ROUTE              SYS_ADPT_MAX_ARP_ENTRY
#define SYS_ADPT_MAX_NBR_OF_NET_ROUTE               SYS_ADPT_MAX_NBR_OF_TOTAL_ROUTE_ENTRY  /* not used? */

/* This constant currently is used by swdrvl3.h only for a age function
 * It can be removed but need to change swdrvl3.h as well,
 * if chip does not support auto-age the IP host entries,
 * then software need to handle the entry aging issue, currently is handled by AMTRL3
 */
#define SYS_ADPT_SUPPORT_AUTO_AGE_OUT_IP_HOST_TABLE             0

#define SYS_ADPT_NUMBER_OF_INET_CIDR_ROUTE_POLICY_SUBIDENTIFIER   2

#define SYS_ADPT_MAX_NBR_OF_ECMP_ENTRY_PER_ROUTE    32

/* if chip does not support the software need to handle load balancing
 * Currently, it is handled by AMTRL3
 */
/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_CHIP_SUPPORT_L3_LOAD_BALANCE                   TRUE

#define SYS_ADPT_CHIP_SUPPORT_L3_TRUNK_LOAD_BALANCE             TRUE
#define SYS_ADPT_CHIP_SUPPORT_EGRESS_OBJECT                     TRUE
#define SYS_ADPT_CHIP_SUPPORT_SAME_ECMP_EGRESS                  TRUE
#define SYS_ADPT_CHIP_SUPPORT_LOCAL_HOSE                        TRUE
#define SYS_ADPT_CHIP_USE_DEFAULT_ROUTE_TRAP_TO_CPU             FALSE
#define SYS_ADPT_CHIP_USE_TCAM_FOR_ROUTE_TABLE                  TRUE
#define SYS_ADPT_CHIP_KEEP_TRUNK_ID_IN_HOST_ROUTE               FALSE

/* This constant is used by AMTRL3, AMTRL3 need to complete the whole IP host table
 * scan within SYS_ADPT_MAX_SCAN_TIME scan
 * number of times scan process shall consume to completely scan host route table.
 */
#define SYS_ADPT_MAX_SCAN_TIME                                  64

/*
 *  DHCP server configuration parameters
 *       1. capacity specification :
 *          Relay Server number
 *          Default Router number
 *          DNS server number
 *          NetBios name server number
 *          pool number
 *          total ip number in pools.
 *       2. buffer spec.
 *          pool-name length
 *          domain name length
 *          boot-file name length
 *          client hostname length
 *
 */
#define SYS_ADPT_MAX_NBR_OF_DHCP_RELAY_SERVER                   5
#define SYS_ADPT_MAX_NBR_OF_DHCP_DEFAULT_ROUTER                 2
#define SYS_ADPT_MAX_NBR_OF_DHCP_DNS_SERVER                     2
#define SYS_ADPT_MAX_NBR_OF_DHCP_NETBIOS_NAME_SERVER            2
#define SYS_ADPT_MAX_NBR_OF_DHCP_CONFIGURATION_POOL             8
#define SYS_ADPT_MAX_NBR_OF_DHCP_IP_IN_POOL                     512
#define SYS_ADPT_DHCP_MAX_POOL_NAME_LEN                         32
#define SYS_ADPT_DHCP_MAX_DOMAIN_NAME_LEN                       128
#define SYS_ADPT_DHCP_MAX_BOOTFILE_NAME_LEN                     128

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_DHCP_MAX_TFTP_SERVERS                          5       /* decide the number of tftp servers can be supported for option#150   */
#define SYS_ADPT_DHCP_MAX_TFTP_SERVERS_LIST                     SYS_ADPT_DHCP_MAX_TFTP_SERVERS * SYS_ADPT_IPV4_ADDR_LEN

#define SYS_ADPT_DHCP_MAX_CLIENT_HOSTNAME_LEN                   128
#define SYS_ADPT_DHCP_MAX_IP_EXCLUDED_ELEMENTS                  5
#define SYS_ADPT_DHCP_MAX_RANGE_SET_ELEMENTS                    100
#define SYS_ADPT_MAX_NBR_OF_DHCP_NETWORK_POOL                   8
#define SYS_ADPT_MAX_NBR_OF_DHCP_HOST_POOL                      32
#define SYS_ADPT_MAX_NBR_OF_DHCP_POOL                          (SYS_ADPT_MAX_NBR_OF_DHCP_NETWORK_POOL \
                                                                + SYS_ADPT_MAX_NBR_OF_DHCP_HOST_POOL)
#define SYS_ADPT_MAX_NBR_OF_DHCPV6_RELAY_ADDRESS                5    /* Jimi add */

#define SYS_ADPT_MAX_LENGTH_OF_RID                              32
#define SYS_ADPT_MAX_LENGTH_OF_CID                              32

/* dhcpsnp */
#define SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT             5
#define SYS_ADPT_DHCPSNP_MAX_NBR_OF_BINDING_ENTRY               SYS_ADPT_TOTAL_NBR_OF_LPORT * SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT
#define SYS_ADPT_DHCPSNP_MAX_NBR_OF_CLIENT_PER_PORT_CONFIGURABLE TRUE
#define SYS_ADPT_DHCPSNP_MIN_SYSTEM_RATELIMIT                   1
#define SYS_ADPT_DHCPSNP_MAX_SYSTEM_RATELIMIT                   MAX_dhcpSnoopLimitRate


/* ip source guard */
#define SYS_ADPT_IPSG_MAX_NBR_OF_ACL_CLIENT_PER_PORT            10
#define SYS_ADPT_IPSG_MAX_NBR_OF_ACL_BINDING_ENTRY              (SYS_ADPT_TOTAL_NBR_OF_LPORT * SYS_ADPT_IPSG_MAX_NBR_OF_ACL_CLIENT_PER_PORT)
#define SYS_ADPT_IPSG_MAX_NBR_OF_MAC_ENTRY_PER_PORT             1024
#define SYS_ADPT_IPSG_MAX_NBR_OF_STATIC_MAC_BINDING_ENTRY       100
#define SYS_ADPT_IPSG_MAX_NBR_OF_BLOCKED_RECORD                 512

/* dhcpv6snp */
#define SYS_ADPT_DHCPV6SNP_MAX_NBR_OF_CLIENT_PER_PORT           5
#define SYS_ADPT_DHCPV6SNP_MAX_NBR_OF_BINDING_ENTRY             SYS_ADPT_TOTAL_NBR_OF_LPORT * SYS_ADPT_DHCPV6SNP_MAX_NBR_OF_CLIENT_PER_PORT


/* IP6SG */
#define SYS_ADPT_IP6SG_MAX_NBR_OF_CLIENT_PER_PORT               5
#define SYS_ADPT_IP6SG_MIN_PORT_BINDING                         1
#define SYS_ADPT_IP6SG_MAX_PORT_BINDING                         5
#define SYS_ADPT_IP6SG_MAX_NBR_OF_BINDING_ENTRY                 SYS_ADPT_TOTAL_NBR_OF_LPORT * SYS_ADPT_IP6SG_MAX_NBR_OF_CLIENT_PER_PORT

#define SYS_ADPT_MAX_NUM_OF_SMTP_SERVER                         3
#define SYS_ADPT_MAX_LENGTH_OF_SMTP_EMAIL_ADDRESS               41
#define SYS_ADPT_MAX_NUM_OF_SMTP_DESTINATION_EMAIL_ADDRESS      5
#define SYS_ADPT_MAX_LENGTH_OF_SMTPLOG_MESSAGE                  600


#define SYS_ADPT_DNS_MAX_TIME_OUT                               15
#define SYS_ADPT_DNS_MIN_TIME_OUT                               1
#define SYS_ADPT_DNS_MAX_NBR_OF_LOCAL_REQUEST                   20 /*maggie liu, NSLOOKUP*/
#define SYS_ADPT_DNS_MIN_NBR_OF_LOCAL_REQUEST                   1
#define SYS_ADPT_DNS_MAX_NBR_OF_SERVER_REQUEST                  20
#define SYS_ADPT_DNS_MIN_NBR_OF_SERVER_REQUEST                  1
#define SYS_ADPT_DNS_MAX_NBR_OF_CACHE_SIZE                      6400
#define SYS_ADPT_DNS_MIN_NBR_OF_CACHE_SIZE                      1280

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_DNS_MAX_CACHE_SIZE                             6400
#define SYS_ADPT_DNS_MIN_CACHE_SIZE                             1280

#define SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP                         8
#define SYS_ADPT_MAX_NBR_OF_DNS_HOST_IP_PER_HOST_NAME           1 /*maggie liu, ES3628BT-FLF-ZZ-00147*/
#define SYS_ADPT_MAX_TTL_FOR_RRS_IN_CACHE                       876000
#define SYS_ADPT_DNS_MAX_NBR_OF_HOST_TABLE_SIZE                 16
#define SYS_ADPT_DNS_MAX_NBR_OF_NAME_SERVER_TABLE_SIZE          6
#define SYS_ADPT_DNS_MAX_NBR_OF_DOMAIN_NAME_LIST                3
#define SYS_ADPT_DNS_MAX_NAME_LEGTH                             100
/* River@May 7, 2008, add nslookup mib */
#define SYS_ADPT_DNS_MAX_NSLOOKUP_PTIME                         86400
#define SYS_ADPT_DNS_MIN_NSLOOKUP_PTIME                         0

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_DNS_MAX_NBR_OF_NAME_SERVERS                    6
#define SYS_ADPT_DNS_MAX_NBR_OF_DOMAIN_NAMES                    3
#define SYS_ADPT_DNS_MAX_NAME_LENGTH                            127

/*
this is because the rules can be adjusted
*/
#if(SYS_CPNT_COS == TRUE)
    #if(SYS_CPNT_COS_ING_IP_PRECEDENCE_TO_COS == TRUE)
        #define SYS_ADPT_MAX_NBR_OF_COS_PRECEDENCE_RULE                 8
    #else
        #define SYS_ADPT_MAX_NBR_OF_COS_PRECEDENCE_RULE                 0
    #endif

    #if(SYS_CPNT_COS_ING_DSCP_TO_COS == TRUE)
        #define SYS_ADPT_MAX_NBR_OF_DSCP_RULE                           64
    #else
        #define SYS_ADPT_MAX_NBR_OF_DSCP_RULE                           0
    #endif

    #if(SYS_CPNT_COS_ING_IP_PORT_TO_COS == TRUE)
        #define SYS_ADPT_MAX_NBR_OF_COS_TCP_UDP_PORT_RULE_PER_FFP_PIC   8
    #else
        #define SYS_ADPT_MAX_NBR_OF_COS_TCP_UDP_PORT_RULE_PER_FFP_PIC   0
    #endif



#else
    #define SYS_ADPT_MAX_NBR_OF_COS_PRECEDENCE_RULE                     0
    #define SYS_ADPT_MAX_NBR_OF_DSCP_RULE                               0
    #define SYS_ADPT_MAX_NBR_OF_COS_TCP_UDP_PORT_RULE_PER_FFP_PIC       0

#endif

/* For Per Port Per CoS Rate Limit */
#define SYS_ADPT_MAX_PER_FE_PORT_PER_PRIORITY_RATE_LIMIIT        100
#define SYS_ADPT_MAX_PER_GE_PORT_PER_PRIORITY_RATE_LIMIT         1000
#define SYS_ADPT_MIN_PER_PORT_PER_PRIORITY_RATE_LIMIT            1
#define SYS_ADPT_NUMBER_OF_COS_PER_PORT_PER_PRIORITY_RATE_LIMIT  4
/**/
#define SYS_ADPT_PRI_0_MAP_TO_QUEUE                              1
#define SYS_ADPT_PRI_1_MAP_TO_QUEUE                              0
#define SYS_ADPT_PRI_2_MAP_TO_QUEUE                              0
#define SYS_ADPT_PRI_3_MAP_TO_QUEUE                              1
#define SYS_ADPT_PRI_4_MAP_TO_QUEUE                              2
#define SYS_ADPT_PRI_5_MAP_TO_QUEUE                              2
#define SYS_ADPT_PRI_6_MAP_TO_QUEUE                              3
#define SYS_ADPT_PRI_7_MAP_TO_QUEUE                              3
/**/
#define SYS_ADPT_PRI_0_THE_SAME_QUEUE_PRI                        3
#define SYS_ADPT_PRI_1_THE_SAME_QUEUE_PRI                        2
#define SYS_ADPT_PRI_2_THE_SAME_QUEUE_PRI                        1
#define SYS_ADPT_PRI_3_THE_SAME_QUEUE_PRI                        0
#define SYS_ADPT_PRI_4_THE_SAME_QUEUE_PRI                        5
#define SYS_ADPT_PRI_5_THE_SAME_QUEUE_PRI                        4
#define SYS_ADPT_PRI_6_THE_SAME_QUEUE_PRI                        7
#define SYS_ADPT_PRI_7_THE_SAME_QUEUE_PRI                        6

#if(SYS_CPNT_VLAN_BASED_PRIORITY == TRUE)
#define SYS_ADPT_MAX_NUMBER_OF_VLAN_BASED_PRIORITY_REMARKING	4
#endif

#if(SYS_CPNT_ROUTING == TRUE)
    #define SYS_ADPT_MAX_NBR_OF_DFLT_ROUTE_MASK         1
    #define SYS_ADPT_MAX_NBR_OF_DFLT_ROUTE_RULE         1
    #define SYS_ADPT_MAX_NBR_OF_NET_ROUTE_MASK          4
    #define SYS_ADPT_MAX_NBR_OF_NET_ROUTE_RULE          32
#else
    #define SYS_ADPT_MAX_NBR_OF_DFLT_ROUTE_MASK         0
    #define SYS_ADPT_MAX_NBR_OF_DFLT_ROUTE_RULE         0
    #define SYS_ADPT_MAX_NBR_OF_NET_ROUTE_MASK          0
    #define SYS_ADPT_MAX_NBR_OF_NET_ROUTE_RULE          0
#endif

/* Buffer Number definition */
/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_64       512
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_128      512
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_256      256
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_512      256
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1024     0
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1536     0
#define SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1800     250

/* Yongxin.Zhao added, for merge bcm-5.5.2, temp use, need check with taiwan, 2008-05-22, 14:47:32 */
/* MREF descriptors needed =
 * number of TX buffer + 8 (NIC announcement) + 2*10 (NIC 10 packets/sec after 8 announcements) + max_unit# if stacking for HBT)
 */
#define SYS_ADPT_MAX_NBR_OF_MREF_DESC_BLOCK            (SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_64 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_128 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_256 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_512 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1024 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1536 + \
							SYS_ADPT_L_IPCMEM_NUMBER_OF_BUF_1800 + \
							8 /* NIC announcements */ + \
							2*10 /* NIC 10 packets/sec after 8 announcements */ + \
							8 /* 8 for HBT if max unit#=8 when stacking */)

/* Compatibily issues
 */

/* The first version start to use multicast ISC
 * each version number in hexadecimal occupy 1 byte
 * total of 4 bytes
 */
#define SYS_ADPT_FIRST_MAINBOARD_VERSION_USING_MC      0x00000000
#define SYS_ADPT_FIRST_MODULE_VERSION_USING_MC         0x00000000

#define SYS_ADPT_FIRST_RUNTIME_VERSION_USING_NEW_HBT2  0x00000000

#define SYS_ADPT_MAX_NBR_OF_IML_TX_IP_PKT 30


#define SYS_ADPT_LLDP_MAX_REM_DATA_PER_PORT             8


#define SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI          16
#define SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN        MAXSIZE_voiceVlanOuiDescription

#if (SYS_CPNT_LLDP == TRUE)
    /* Ref lldp_type.h
     * #define LLDP_TYPE_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT        32
     */
    #define SYS_ADPT_MAX_NBR_OF_REM_DATA_ENTRY_PER_PORT        8
    #define SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT (SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI + \
                                                    SYS_ADPT_LLDP_MAX_REM_DATA_PER_PORT)
#else
    #define SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT (SYS_ADPT_ADD_VOICE_VLAN_MAX_NBR_OF_OUI)
#endif

#if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE)
    #define    SYS_ADPT_SYSMGMT_DEFERRED_RELOAD_MAX_MINUTES     (24*24*60)
#endif  /* #if (SYS_CPNT_SYSMGMT_DEFERRED_RELOAD == TRUE) */

#if (SYS_CPNT_EFM_OAM == TRUE)
#define SYS_ADPT_OUI_ADDR_LEN                          3
#define SYS_ADPT_OAM_MIN_REMOTE_PORT                   1
#define SYS_ADPT_OAM_MAX_REMOTE_PORT                   16
#define SYS_ADPT_OAM_MIN_LOOPBACK_FRAME_NUMBER         1
#define SYS_ADPT_OAM_MAX_LOOPBACK_FRAME_NUMBER         99999999
#define SYS_ADPT_OAM_MIN_LOOPBACK_FRAME_SIZE           64
#define SYS_ADPT_OAM_MAX_LOOPBACK_FRAME_SIZE           1518
#define SYS_ADPT_OAM_MIN_ERROR_FRAME_WINDOW            10
#define SYS_ADPT_OAM_MAX_ERROR_FRAME_WINDOW            65535
#define SYS_ADPT_OAM_MIN_ERROR_FRAME_THRESHOLD         1
#define SYS_ADPT_OAM_MAX_ERROR_FRAME_THRESHOLD         65535
#define SYS_ADPT_OAM_VENDOR_INFO                       0
#define SYS_ADPT_OAM_CPE_FIRMWARE_FILE_NAME            56

#define SYS_ADPT_OAM_HW_SUPPORT_LOOPBACK               TRUE

/* If TRUE, work as CO, default mode is active
 * else, work as CPE, default mode is passive
 */
#define SYS_ADPT_OAM_CO                                TRUE

#define SYS_ADPT_OAM_OUI_BYTE1                         0x00
#define SYS_ADPT_OAM_OUI_BYTE2                         0x12
#define SYS_ADPT_OAM_OUI_BYTE3                         0xCF

#endif /* End of SYS_CPNT_EFM_OAM */

/* RESETMGMT component deals with the H/W reset button
 * if the time that the reset button is being pressed/held is T seconds,
 * T < SYS_ADPT_RESETMGMT_RESET_TIME --> no reset
 * T >= SYS_ADPT_RESETMGMT_RESET_TIME && T < SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME --> reset the system
 * T >= SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME --> reset with factory defaults
 */
#if (SYS_CPNT_RESETMGMT == TRUE)
#define SYS_ADPT_RESETMGMT_RESET_TO_FACTORY_DEFAULT_TIME  10 /*sec*/
#define SYS_ADPT_RESETMGMT_RESET_TIME                     5  /*sec*/
#endif

/* Yongxin.Zhao added for merge bcm-5.5.2, 2008.05.22 */
#if defined(SYS_CPNT_CFM) && (SYS_CPNT_CFM == TRUE)
#define SYS_ADPT_CFM_SUPPORTED_VERSION  0
#define SYS_ADPT_CFM_MAX_NBR_OF_MD                    8
#define SYS_ADPT_CFM_MAX_NBR_OF_MA                   32
#define SYS_ADPT_CFM_MAX_NBR_OF_MEP                 200
#define SYS_ADPT_CFM_MAX_NBR_OF_MIP                 (  SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT \
                                                     * SYS_ADPT_CFM_MAX_NBR_OF_MA)
#define SYS_ADPT_CFM_MAX_NBR_OF_REMOTE_MEP          200
#define SYS_ADPT_CFM_MAX_NBR_OF_LTR                4095
#define SYS_ADPT_CFM_MAX_NBR_OF_ERROR                10
#define SYS_ADPT_CFM_MAX_NBR_OF_TIMER SYS_ADPT_CFM_MAX_NBR_OF_MA \
                                                          + SYS_ADPT_CFM_MAX_NBR_OF_REMOTE_MEP*2 \
                                                          + SYS_ADPT_CFM_MAX_NBR_OF_MEP*5 \
                                                          + SYS_ADPT_CFM_MAX_NBR_OF_LTR             \
                                                          + 3    /* clear pending ltr, bgdmm, bgdmr */


#define SYS_ADPT_CFM_MAX_MEP_ID                      MAX_dot1agCfmMepIdentifier
#define SYS_ADPT_CFM_MIN_MEP_ID                      MIN_dot1agCfmMepIdentifier

#define SYS_ADPT_CFM_MAX_MA_INDEX                    MAX_dot1agCfmMaIndex
#define SYS_ADPT_CFM_MIN_MA_INDEX                    MIN_dot1agCfmMaIndex

#define SYS_ADPT_CFM_MAX_MD_INDEX                    65535     /*MAX_dot1agCfmMdIndex*/
#define SYS_ADPT_CFM_MIN_MD_INDEX                    MIN_dot1agCfmMdIndex

#define SYS_ADPT_CFM_MAX_LINKTRACE_TTL               MAX_dot1agCfmMepTransmitLtmTtl
#define SYS_ADPT_CFM_MIN_LINKTRACE_TTL               MIN_dot1agCfmMepTransmitLtmTtl

#define SYS_ADPT_MAX_ERROR_NUM          100
#endif


/* jerry.du copy from ECN430_FB2 sys_adpt.h, just for compile now, Must be fix 20080722 */
#define SYS_ADPT_DEV_SWDRV_XGSIII_50PORT_BOARD_INTER_HG_PORT1   24
#define SYS_ADPT_DEV_SWDRV_XGSIII_50PORT_BOARD_INTER_HG_PORT2   25
#define SYS_ADPT_UPLINK_STACKINGPORT                            26
#define SYS_ADPT_DOWNLINK_STACKINGPORT                          27
#define SYS_ADPT_MAINBRD_To_EXPSION_STACKINGPORT                0
#define SYS_ADPT_EXPSION_To_MAINBRD_STACKINGPORT                0
#define SYS_ADPT_BOTH_STACKINGPORT                              0xff
#define SYS_ADPT_DEV_UART_NAME_0                                "/dev/ttyS1"
#define SYS_ADPT_DEV_UART_NAME_1                                "/dev/ttyS0"
#define SYS_ADPT_RULE_CTRL_PORT_BITMAP_TO_CPU_ONLY              0x0000000000000001LL
#define SYS_ADPT_RULE_CTRL_PORT_BITMAP_TO_ALL_PORTS             0x001fffffffffffffLL
#define SYS_ADPT_RULE_CTRL_PORT_BITMAP_EXCLUDE_CPU              0x001ffffffffffffeLL
#define SYS_ADPT_RULE_CTRL_PHY_NUM_OF_CPU_PORT                  0x0

/* anzhen.zheng, 2008-9-3 */
#define SYS_ADPT_FP_SUPPORT_EFP_AND_VFP             FALSE
#define SYS_ADPT_FP_MAX_NBR_OF_GROUP_PER_CHIP       (12+4)  /* ingress + egress */
#define SYS_ADPT_FP_MAX_NBR_OF_RULE_PER_SELECTOR    1024
#define SYS_ADPT_FP_TOTAL_NBR_OF_TCAM_ENTRY         5120     /* 4*512 + 12*256 */
#define SYS_ADPT_FP_BASIC_NBR_OF_RULE_PER_VIRTUAL_GROUP     128

/* If TRUE, use default segmentation fault handler to dump call stack.
 * If default handler can't work, set FALSE, and then sysfun will handle
 * segmentation fault.
 */
#define SYS_ADPT_USE_DEFAULT_SIGSEGV_HANDLER     FALSE

/* anzhen.zheng, 6/10/2008 */
/* EAPS VLAN MODE */
#define SYS_ADPT_EAPS_TRANSITION_MODE	1
#define SYS_ADPT_EAPS_MASTER_MODE		2

/* hongliang.yang, 4/8/2009 */
/* UDP HELPER */
#define SYS_ADPT_UDPHELPER_MAX_HELPER         1024
#define SYS_ADPT_UDPHELPER_MAX_FORWARD_PORT   100
#define SYS_ADPT_MLD_MAX_SUPPORT_GROUP                      SYS_ADPT_MAX_MULTICAST_FORWARDING_ENTRY
#define SYS_ADPT_MLD_MAX_QUERY_INTERVAL                     255
#define SYS_ADPT_MLD_MIN_QUERY_INTERVAL                     1
#define SYS_ADPT_MLD_MAX_ROBUSTNESS                         MAX_mldInterfaceRobustness
#define SYS_ADPT_MLD_MIN_ROBUSTNESS                         MIN_mldInterfaceRobustness

#define SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY              64  /* unknown + static + dynamic */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_ROUTER_PORT              16  /* max route port -- by vlan */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP             32

#define SYS_ADPT_MLDSNP_MAX_NBR_OF_GROUP_ENTRY              64  /* unknown + static + dynamic */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_ROUTER_PORT              16  /* max route port -- by vlan */
#define SYS_ADPT_MLDSNP_MAX_NBR_OF_STATIC_GROUP             32

#define SYS_ADPT_MLD_MAX_QUERY_MAX_RESPONSE_TIME            MAX_mldInterfaceQueryMaxResponseTime
#define SYS_ADPT_MLD_MIN_QUERY_MAX_RESPONSE_TIME            MIN_mldInterfaceQueryMaxResponseTime
#define SYS_ADPT_MLD_MAX_LAST_MEMBER_QUERY_INTERVAL         MAX_mldInterfaceLastMembQueryIntvl
#define SYS_ADPT_MLD_MIN_LAST_MEMBER_QUERY_INTERVAL         1
#define SYS_ADPT_MLD_MIN_UNSOLICITED_REPORT_INTERVAL        1
#define SYS_ADPT_MLD_MAX_UNSOLICITED_REPORT_INTERVAL        65535

#define SYS_ADPT_MLD_MAX_SUPPORT_STATIC_GROUP               64
#define SYS_ADPT_MLD_MAX_SUPPORT_STATIC_GROUP_AND_SOURCE    64

/* for sFlow */
#define SYS_ADPT_SFLOW_MAX_NUMBER_OF_RECEIVER_ENTRY           SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * SYS_ADPT_TOTAL_PORTS_PER_UNIT_ON_BOARD
#define SYS_ADPT_SFLOW_MAX_RECEIVER_OWNER_STR_LEN             30
#define SYS_ADPT_SFLOW_MIN_RECEIVER_TIMEOUT                   30
#define SYS_ADPT_SFLOW_MAX_RECEIVER_TIMEOUT                   10000000
#define SYS_ADPT_SFLOW_MIN_RECEIVER_DATAGRAM_SIZE             200
#define SYS_ADPT_SFLOW_MAX_RECEIVER_DATAGRAM_SIZE             1500

#define SYS_ADPT_SFLOW_MAX_INSTANCE_OF_DATASOURCE             1
#define SYS_ADPT_SFLOW_MIN_SAMPLING_RATE                      256         /* BCM chip limitation */
#define SYS_ADPT_SFLOW_MAX_SAMPLING_RATE                      0xFFFFFF    /* BCM chip limitation */
#define SYS_ADPT_SFLOW_MIN_SAMPLING_HEADER_SIZE               64
#define SYS_ADPT_SFLOW_MAX_SAMPLING_HEADER_SIZE               256
#define SYS_ADPT_SFLOW_MIN_POLLING_INTERVAL                   1
#define SYS_ADPT_SFLOW_MAX_POLLING_INTERVAL                   10000000

/*Simon Shih : add tunnel interface*/
#define SYS_ADPT_TUNNEL_1_IF_INDEX_NUMBER   8001
#define SYS_ADPT_MAX_TUNNEL_ID              16
#define SYS_ADPT_TUNNEL_6to4_PREFIX_LEN     48

/* vxlan */
#define SYS_ADPT_VXLAN_FIRST_IF_INDEX_NUMBER   10001
#define SYS_ADPT_MAX_VFI_NBR                   0x1000
#define SYS_ADPT_MIN_VXLAN_VFI_ID              0x7000
#define SYS_ADPT_MAX_VXLAN_VFI_ID              (0x7000+SYS_ADPT_MAX_VFI_NBR-1)
#define SYS_ADPT_VXLAN_MAX_NBR_VNI                SYS_ADPT_MAX_VFI_NBR/*SYS_ADPT_MAX_NBR_OF_L3_INTERFACE*/
#define SYS_ADPT_VXLAN_MAX_NBR_OF_UC_FLOOD_ENTRY  100
#define SYS_ADPT_VXLAN_MAX_NBR_OF_MC_FLOOD_ENTRY  SYS_ADPT_VXLAN_MAX_NBR_VNI   /* a VNI only have a multicast group */

/* because apollo chip support 1024 entry in EGR_IP_TUNNEL, we reserved 16 for static tunnel */
#define SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_HOST_CACHE_ENTRY     (1024 - SYS_ADPT_MAX_TUNNEL_ID)
#define SYS_ADPT_MAX_NBR_OF_TUNNEL_DYNAMIC_6to4_NET_CACHE_ENTRY      (1024 - SYS_ADPT_MAX_TUNNEL_ID)

/*Pttch: Add for login prompt, if customer want to modify this, PL can use this to change*/
#define SYS_ADPT_LOGIN_PROMPT_STRING "User Access Verification"

/*for 10G module used */
#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
#define SYS_ADPT_26PORT_10G_MODULE_DEVID     0
#define SYS_ADPT_50PORT_10G_MODULE_DEVID     1
#define SYS_ADPT_26PORT_10G_MODULE1_PHYADDR  (25 + 0x40) /*bcm sdk mdio address starts from 0x40 */
#define SYS_ADPT_26PORT_10G_MODULE2_PHYADDR  (26 + 0x40)
#define SYS_ADPT_50PORT_10G_MODULE1_PHYADDR  (27 + 0x40)
#define SYS_ADPT_50PORT_10G_MODULE2_PHYADDR  (28 + 0x40)

#define SYS_ADPT_26PORT_10G_XE1_PORT  25
#define SYS_ADPT_26PORT_10G_XE0_PORT  24

#define SYS_ADPT_50PORT_10G_XE1_PORT  27
#define SYS_ADPT_50PORT_10G_XE0_PORT  26
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

#if (SYS_CPNT_MAC_BASED_MIRROR == TRUE)
#define SYS_ADPT_MAX_NBR_OF_MAC_BASED_MIRROR_ENTRY 10
#endif

/* The default opcode file name be searched for auto upgrade on
 * file server.
 */
#define SYS_ADPT_XFER_AUTO_UPGRADE_OPCODE_SEARCH_FILENAME  "aos7710-32x.swi"

/* Definition the number of traffic segmentation session can support
 *
 * Traffic seg. is implement by source-based filtering,
 * so the number of sessions is unlimited.
 */
#define SYS_ADPT_PORT_TRAFFIC_SEGMENTATION_MAX_NBR_OF_SESSIONS 4

/* For selective QinQ,
 * specifies how many services a port can subscribe
 */
#define SYS_ADPT_MAX_NBR_OF_QINQ_SERVICE_SUBSCRIPTION_PER_PORT      24

/* specifies supported/required action for QinQ service
 */
#define SYS_ADPT_QINQ_SERVICE_TAG_INFO_SRV_ACTION \
            ( SYS_VAL_vlanDot1qTunnelSrvAction_assignSvid \
            | SYS_VAL_vlanDot1qTunnelSrvAction_removeCtag )
#define SYS_ADPT_QINQ_SERVICE_TAG_INFO_SRV_REQUIRED_ACTION \
            ( SYS_VAL_vlanDot1qTunnelSrvAction_assignSvid )
#define SYS_ADPT_QINQ_SERVICE_DFLT_SRV_ACTION \
            ( SYS_VAL_vlanDot1qTunnelSrvAction_discard \
            | SYS_VAL_vlanDot1qTunnelSrvAction_removeCtag )

/* For L2PT
 */
#define SYS_ADPT_QINQ_L2PT_MAX_NBR_OF_CUSTOM_PDU_CONFIG         16
#define SYS_ADPT_QINQ_L2PT_MAX_NBR_OF_CUSTOM_PDU_PER_PORT       1

/* For PowerSave
 *
 * Some phy of link partner will re-autonego when power saving is enabled
 * It will lead to infinite looping of link-up-and-down.
 * So our solution is that after link-down, don't disable chip-level power saving right now.
 * After delay SYS_ADPT_POWER_SAVE_PHY_REAUTONEGO_TIME ticks(autonego done),
 * if the interface can link up with power saving, don't disable it then.
 */
#define SYS_ADPT_POWER_SAVE_PHY_REAUTONEGO_TIME     500

/* for G.8032 (ERPS)
 */
#define SYS_ADPT_ERPS_MAX_NBR_OF_DOMAIN         (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD/2)
#define SYS_ADPT_ERPS_MAX_LEN_OF_DOMAIN_NAME    MAXSIZE_erpsDomainName
#define SYS_ADPT_ERPS_MIN_LEN_OF_DOMAIN_NAME    MINSIZE_erpsDomainName
#define SYS_ADPT_ERPS_MAX_TIMER_WTR             MAX_erpsDomainWtrTimer      /* min  */
#define SYS_ADPT_ERPS_MIN_TIMER_WTR             MIN_erpsDomainWtrTimer      /* min  */
#define SYS_ADPT_ERPS_MAX_TIMER_GUARD           MAX_erpsDomainGuardTimer    /* ms   */
#define SYS_ADPT_ERPS_MIN_TIMER_GUARD           MIN_erpsDomainGuardTimer    /* ms   */
#define SYS_ADPT_ERPS_MAX_TIMER_HOLDOFF         MAX_erpsDomainHoldoffTimer  /* ms   */
#define SYS_ADPT_ERPS_MIN_TIMER_HOLDOFF         MIN_erpsDomainHoldoffTimer  /* ms   */
#define SYS_ADPT_ERPS_MAX_MEG_LEVEL             MAX_erpsDomainMegLevel
#define SYS_ADPT_ERPS_MIN_MEG_LEVEL             MIN_erpsDomainMegLevel
#define SYS_ADPT_ERPS_MAX_ID                    255
#define SYS_ADPT_ERPS_MIN_ID                    1

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
/* Defined constants for software watchdog timer -- START
 */
#define SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT         10   /* 10 mins */
/*CLI_PROC*/
#define SYS_ADPT_CLI_SW_WATCHDOG_TIMER                            SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT   /* 0: means don't monitor this thread */
#define SYS_ADPT_CLI_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_TELNET_SERVER_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_TELNET_PARENT_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_TELNET_GROUP_SW_WATCHDOG_TIMER                   SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SSH_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SSH_PARENT_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*SNMP_PROC*/
#define SYS_ADPT_SNMP_GROUP_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SNMP_SW_WATCHDOG_TIMER                           SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*IP_SERVICE_PROC*/
#define SYS_ADPT_IP_SERVICE_GROUP_SW_WATCHDOG_TIMER               SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*AUTH_PROTOCOL_PROC*/
#define SYS_ADPT_RADIUS_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_TACACS_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_AUTH_PROTOCOL_GROUP_SW_WATCHDOG_TIMER            SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*XFER_PROC*/
#define SYS_ADPT_XFER_SW_WATCHDOG_TIMER                           SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_XFER_GROUP_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*STKCTRL_PROC*/
#define SYS_ADPT_STKCTRL_GROUP_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*STKTPLG_PROC*/
#define SYS_ADPT_STKTPLG_GROUP_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*SYS_MGMT_PROC*/
#define SYS_ADPT_SYS_MGMT_GROUP_SW_WATCHDOG_TIMER                 SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*CORE_UTIL_PROC*/
#define SYS_ADPT_SYSLOG_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_UTILITY_GROUP_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_CFGDB_GROUP_SW_WATCHDOG_TIMER                    SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_MEMCHK_GROUP_SW_WATCHDOG_TIMER                   SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*L2_L4_PROC*/
#define SYS_ADPT_L2MUX_GROUP_SW_WATCHDOG_TIMER                    SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_IML_RX_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SWCTRL_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SWCTRL_GROUP_SW_WATCHDOG_TIMER                   SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_LACP_GROUP_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_NETACCESS_GROUP_SW_WATCHDOG_TIMER                SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_NETACCESS_NMTR_SW_WATCHDOG_TIMER                 SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_NETACCESS_NMTR_HASH2HISAM_SW_WATCHDOG_TIMER      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_NETACCESS_PSEC_SW_WATCHDOG_TIMER                 SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_STA_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_L4_GROUP_SW_WATCHDOG_TIMER                       SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_L2MCAST_GROUP_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_CFM_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_CMGR_GROUP_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_DHCPSNP_GROUP_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_DHCPSNP_CSC_SW_WATCHDOG_TIMER                    SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_DHCPV6SNP_CSC_SW_WATCHDOG_TIMER                  SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_DCB_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_MLAG_GROUP_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_VXLAN_GROUP_SW_WATCHDOG_TIMER                    SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT

/*APP_PROTOCOL_PROC*/
#define SYS_ADPT_APP_PROTOCOL_SNTP_SW_WATCHDOG_TIMER              SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_APP_PROTOCOL_GROUP_SW_WATCHDOG_TIMER             SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_PING_SW_WATCHDOG_TIMER                           SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*NETCFG_PROC*/
#define SYS_ADPT_NETCFG_GROUP_SW_WATCHDOG_TIMER                   SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*NSM_PROC*/
#define SYS_ADPT_NSM_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*DRIVER_PROC*/
#define SYS_ADPT_DEV_NICDRV_SW_WATCHDOG_TIMER                     SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SWDRV_SW_WATCHDOG_TIMER                          SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_AMTRDRV_SW_WATCHDOG_TIMER                        SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SYSDRV_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_NMTRDRV_SW_WATCHDOG_TIMER                        SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_FLASHDRV_SW_WATCHDOG_TIMER                       SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_LEDDRV_SW_WATCHDOG_TIMER                         SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_DRIVER_GROUP_SW_WATCHDOG_TIMER                   SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_SYS_TIME_SW_WATCHDOG_TIMER                       SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*WEB_PROC*/
#define SYS_ADPT_WEB_GROUP_SW_WATCHDOG_TIMER                      SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
#define SYS_ADPT_HTTP_SW_WATCHDOG_TIMER                           SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/*SYS_CALLBACK_PROC*/
#define SYS_ADPT_SYS_CALLBACK_GROUP_SW_WATCHDOG_TIMER             SYS_ADPT_SW_WATCHDOG_TIMER_DEFAULT
/* Defined constants for software watchdog timer -- END
 */
#endif /* end of #if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE) */

/* For counter history
 */
#define SYS_ADPT_NMTR_HIST_MAX_NBR_OF_CTRL_ENTRY_PER_PORT   3
#define SYS_ADPT_NMTR_HIST_MAX_CTRL_NAME_LEN                31
#define SYS_ADPT_NMTR_HIST_MIN_CTRL_INTERVAL                1       /* sec */
#define SYS_ADPT_NMTR_HIST_MAX_CTRL_INTERVAL                86400   /* sec. 86400 s = 1 day */
#define SYS_ADPT_NMTR_HIST_MAX_CTRL_BUCKETS                 96

#define SYS_ADPT_SYNC_E_MAX_SUPPORT_IFINDEX 28
#define SYS_ADPT_SYNC_E_MIN_SUPPORT_IFINDEX 25

#define SYS_ADPT_SYNC_E_MAX_SSM_PRIORITY   2
#define SYS_ADPT_SYNC_E_MIN_SSM_PRIORITY   0

/* for DOS protection
 *
 * NOTE
 *   - rate is in kbps
 */
#define SYS_ADPT_DOS_MIN_RATELIMIT              64
#define SYS_ADPT_DOS_MAX_RATELIMIT              2000

#define SYS_ADPT_SNAP_OUI_BYTE1     0x00
#define SYS_ADPT_SNAP_OUI_BYTE2     0x12
#define SYS_ADPT_SNAP_OUI_BYTE3     0xCF

#define SYS_ADPT_RSPAN_MAX_NBR_OF_SESSION   2

#define SYS_ADPT_PTP_MIN_LOG_ANNOUNCE_INTERVAL     0    /*annex J.3.2*/
#define SYS_ADPT_PTP_MAX_LOG_ANNOUNCE_INTERVAL     4    /*annex J.3.2*/
#define SYS_ADPT_PTP_MIN_ANNOUNCE_RECEIPT_TIMEOUT  2    /*annex J.3.2*/
#define SYS_ADPT_PTP_MAX_ANNOUNCE_RECEIPT_TIMEOUT  10   /*annex J.3.2*/
#define SYS_ADPT_PTP_MIN_LOG_SYNC_INTERVAL         -1   /*annex J.3.2*/
#define SYS_ADPT_PTP_MAX_LOG_SYNC_INTERVAL         1    /*annex J.3.2*/
#define SYS_ADPT_PTP_MIN_LOG_PEER_DEALY_REQUEST_INTERVAL   0 /*annex J.4.2*/
#define SYS_ADPT_PTP_MAX_LOG_PEER_DELAY_REQUEST_INTERVAL   5 /*annex J.4.2*/
#define SYS_ADPT_PTP_MIN_LOG_DELAY_REQUEST_INTERVAL        0 /*annex J.3.2*/
#define SYS_ADPT_PTP_MAX_LOG_DELAY_REQUEST_INTERVAL        5 /*annex J.3.2*/
#define SYS_ADPT_PTP_MIN_PRIORITY1          0
#define SYS_ADPT_PTP_MAX_PRIORITY1          255
#define SYS_ADPT_PTP_MIN_PRIORITY2          0
#define SYS_ADPT_PTP_MAX_PRIORITY2          255
#define SYS_ADPT_PTP_MIN_INGRESS_LATENCY    0
#define SYS_ADPT_PTP_MAX_INGRESS_LATENCY    1000000
#define SYS_ADPT_PTP_MIN_EGRESS_LATENCY     0
#define SYS_ADPT_PTP_MAX_EGRESS_LATENCY     1000000
#define SYS_ADPT_PTP_MIN_DOMAIN_NUMBER      0
#define SYS_ADPT_PTP_MAX_DOMAIN_NUMBER      255
#define SYS_ADPT_PTP_MAX_FOREIGN_RECORDS    5
#define SYS_ADPT_PTP_TIME_STAMP_FLAG        0xffffffff


/* for NDSNP */
#define SYS_ADPT_NDSNP_MIN_PREFIX_TIMEOUT        3
#define SYS_ADPT_NDSNP_MAX_PREFIX_TIMEOUT     1800
#define SYS_ADPT_NDSNP_MIN_RETRANSMIT_COUNT      1
#define SYS_ADPT_NDSNP_MAX_RETRANSMIT_COUNT      5
#define SYS_ADPT_NDSNP_MIN_RETRANSMIT_INTERVAL   1
#define SYS_ADPT_NDSNP_MAX_RETRANSMIT_INTERVAL   10
#define SYS_ADPT_NDSNP_MIN_NBR_OF_BINDING_PER_PORT 1
#define SYS_ADPT_NDSNP_MAX_NBR_OF_BINDING_PER_PORT 5
#define SYS_ADPT_NDSNP_MAX_NBR_OF_BINDING_ENTRY   SYS_ADPT_TOTAL_NBR_OF_LPORT * SYS_ADPT_NDSNP_MAX_NBR_OF_BINDING_PER_PORT
#define SYS_ADPT_NDSNP_MAX_NBR_OF_PREFIX_ENTRY    100

#if 0 /* obsoleted by SYS_CPNT_FS_GET_MTD_ID_FROM_SCRIPT==TRUE */
/* for customized MTD ID (referenced by fs_type.h)
 */
#define SYS_ADPT_MTD_ID_PART_HW_INFO     1
#define SYS_ADPT_MTD_ID_PART_UB_ENV      6
#define SYS_ADPT_MTD_ID_PART_UBOOT       7
#define SYS_ADPT_MTD_ID_PART_MFG_RUNTIME 2 /* mtdblock2 is ONIE runtime */
#else
/* SYS_ADPT_FS_UBOOT_MTD_NAME
 *     The mtd name of the partition used to put uboot(loader).
 */
#define SYS_ADPT_FS_UBOOT_MTD_NAME "uboot"

/* SYS_ADPT_FS_UBOOT_ENV_MTD_NAME
 *     The mtd name of the partition used to put uboot environment variables
 */
#define SYS_ADPT_FS_UBOOT_ENV_MTD_NAME "uboot-env"

/* SYS_ADPT_FS_HWINFO_MTD_NAME
 *     The mtd name of the partition used to put hardware information.
 */
#define SYS_ADPT_FS_HWINFO_MTD_NAME "hw-info"

#endif

#define SYS_ADPT_FS_DATA_STORAGE_MTD_NAME "open" /* use the mtd with name "open" in ONIE environment to keep data storage data */
#define SYS_ADPT_FS_DATA_STORAGE_SIZE_ALU (64*SYS_TYPE_1K)

#define SYS_ADPT_HW_WATCHDOG_PERIODIC_TIMER_TICKS 500 /* 500 ticks = 5 sec */

/* for Congestion Notification (CN) */
#define SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT  7

/* for ETS */
#define SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS   3
#define SYS_ADPT_ETS_MAX_NBR_OF_TC_WEIGHT   100
#define SYS_ADPT_MAX_NBR_OF_PRIO_GROUP      SYS_ADPT_ETS_MAX_NBR_OF_TRAFFIC_CLASS   /*tmp work-around for swdrv*/

/* for mac notification trap */
/* (SYS_BLD_AMTR_MGR_SYNC_HASH2HISAM_TICKS/SYS_BLD_TICKS_PER_SECOND) = 1 */
#define SYS_ADPT_MIN_AMTR_MAC_NOTIFY_INTERVAL   1    /* in seconds */
#define SYS_ADPT_MAX_AMTR_MAC_NOTIFY_INTERVAL   3600

#if (SYS_CPNT_SYSMGMT_CPU_GUARD == TRUE)
#define SYS_ADPT_CPU_UTILIZATION_LIMIT              100  /* percentage, highest CPU utilization allowed, 100 means no limit */
/* The following 4 parameters are the low/high bounds */
#define SYS_ADPT_CPU_UTILIZATION_WATERMARK_HIGH 100  /* percentage */
#define SYS_ADPT_CPU_UTILIZATION_WATERMARK_LOW  20   /* percentage */
#define SYS_ADPT_CPU_GUARD_THRESHOLD_MAX        2400 /* packets per second */
#define SYS_ADPT_CPU_GUARD_THRESHOLD_MIN        50   /* packets per second */
#endif

#define SYS_ADPT_LBD_MAX_TAG_VLAN_DETECTED_PER_PORT     16

#define SYS_ADPT_MAX_NBR_OF_PBR_RULE        512

#endif

/* openflow table size
 */
#define SYS_ADPT_OF_VLAN_FLOW_TABLE_SIZE  (54*4094)  /* max entries of VLAN flow */
#define SYS_ADPT_OF_L2LEARN_FLOW_TABLE_SIZE  11264    /* max entries of L2 learning flow */
#define SYS_ADPT_OF_L3FWD_FLOW_TABLE_SIZE  11264
#define SYS_ADPT_OF_L2FWD_FLOW_TABLE_SIZE  SYS_ADPT_OF_L2LEARN_FLOW_TABLE_SIZE
#define SYS_ADPT_OPENFLOW_SWITCH_OPMODE_LEGACY  1
#define SYS_ADPT_OPENFLOW_SWITCH_OPMODE_OPENFLOW 2

/* CMNLIB data structure
 */
#define SYS_ADPT_L_DLST_INDEX_T     UI32_T
#define SYS_ADPT_L_HISAM_RIDX_T     UI32_T

#define SYS_ADPT_MAX_HASH_SELECTION_BLOCK_SIZE   4
#define SYS_ADPT_HW_HASH_SELECTION_BLOCK_SIZE    2

#define SYS_ADPT_L_MSG_MESSAGE_SIZE_IN_UI32 6

#define SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN  SYS_ADPT_TOTAL_NBR_OF_LPORT+SYS_ADPT_VXLAN_MAX_NBR_PORT
#define SYS_ADPT_VXLAN_MAX_NBR_PORT      1024
#define SYS_ADPT_VXLAN_LOGICAL_PORT_BASE (SYS_ADPT_TOTAL_NBR_OF_LPORT)+1
#define SYS_ADPT_VXLAN_MIN_REAL_PORT_ID   0X80000001
#define SYS_ADPT_VXLAN_MAX_REAL_PORT_ID (SYS_ADPT_VXLAN_MIN_REAL_PORT_ID + SYS_ADPT_VXLAN_MAX_NBR_PORT)
#define SYS_ADPT_VXLAN_MAX_LOGICAL_PORT_ID (SYS_ADPT_VXLAN_LOGICAL_PORT_BASE + SYS_ADPT_VXLAN_MAX_NBR_PORT)
/* End of SYS_ADPT.H */
