/* Module Name: STKTPLG_OM.H
 *
 * Purpose:
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *
 * Copyright(C)      Accton Corporation, 2002 - 2005
 */

#ifndef  STKTPLG_OM_H
#define  STKTPLG_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"

#include "dev_swdrv.h"
#include "stktplg_type.h"

/* NAME CONSTANT DECLARATIONS
 */

/* definitions of ipc commands in STKTPLG_OM which will be used in POM operation
 */
enum
{
    STKTPLG_OM_IPC_CMD_GET_NUMBER_OF_UNIT=1,
    STKTPLG_OM_IPC_CMD_GET_LOCAL_MAX_PORT_CAPABILITY,
    STKTPLG_OM_IPC_CMD_GET_MAX_PORT_CAPABILITY,
    STKTPLG_OM_IPC_CMD_GET_MAX_PORT_NUMBER_ON_BOARD,
    STKTPLG_OM_IPC_CMD_GET_PORT_TYPE,
    STKTPLG_OM_IPC_CMD_UNIT_EXIST,
    STKTPLG_OM_IPC_CMD_GET_MY_RUNTIME_FIRMWARE_VER,
    STKTPLG_OM_IPC_CMD_GET_MY_UNIT_ID,
    STKTPLG_OM_IPC_CMD_GET_LOCAL_UNIT_BASE_MAC,
    STKTPLG_OM_IPC_CMD_GET_LOCAL_UNIT_BASE_MAC_FOR_MGMT_PORT,
    STKTPLG_OM_IPC_CMD_GET_UNIT_BASE_MAC,
    STKTPLG_OM_IPC_CMD_GET_UNIT_BOARD_ID,
    STKTPLG_OM_IPC_CMD_PORT_EXIST,
    STKTPLG_OM_IPC_CMD_GET_BOARD_MODULE_TYPE_REG,
    STKTPLG_OM_IPC_CMD_GET_MODULEA_TYPE,
    STKTPLG_OM_IPC_CMD_GET_MODULEB_TYPE,
    STKTPLG_OM_IPC_CMD_GET_OLD_MODULE_TYPE,
    STKTPLG_OM_IPC_CMD_GET_MODULEA_OLD_TYPE,
    STKTPLG_OM_IPC_CMD_GET_MODULEB_OLD_TYPE,
    STKTPLG_OM_IPC_CMD_GET_SYS_INFO,
    STKTPLG_OM_IPC_CMD_GET_DEVICE_INFO,
    STKTPLG_OM_IPC_CMD_GET_MODULE_INFO,
    STKTPLG_OM_IPC_CMD_SET_MODULE_INFO,
    STKTPLG_OM_IPC_CMD_SLAVE_IS_READY,
    STKTPLG_OM_IPC_CMD_GET_SWITCH_OPER_STATE,
    STKTPLG_OM_IPC_CMD_GET_MASTER_UNIT_ID,
    STKTPLG_OM_IPC_CMD_GET_SIMPLEX_STACKING_PORT,
    STKTPLG_OM_IPC_CMD_GET_UNIT_BOOT_REASON,
    STKTPLG_OM_IPC_CMD_GET_JACK_TYPE,
    STKTPLG_OM_IPC_CMD_GET_MAU_MEDIA_TYPE,
    STKTPLG_OM_IPC_CMD_GET_PORT_MEDIA_CAPABILITY,
    STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT,
    STKTPLG_OM_IPC_CMD_GET_PORT_MAPPING,
    STKTPLG_OM_IPC_CMD_IS_TRANSITION,
    STKTPLG_OM_IPC_CMD_IS_PROVISION_COMPLETED,
    STKTPLG_OM_IPC_CMD_GET_STACKING_DBENTRY,
    STKTPLG_OM_IPC_CMD_GET_STACKING_DBENTRY_BY_MAC,
    STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_UP,
    STKTPLG_OM_IPC_CMD_GET_UNITS_REL_POSITION,
    STKTPLG_OM_IPC_CMD_EXP_MODULE_IS_INSERT,
    STKTPLG_OM_IPC_CMD_GET_HI_GI_PORT_NUM,
    STKTPLG_OM_IPC_CMD_OPTION_MODULE_IS_EXIST,
    STKTPLG_OM_IPC_CMD_IS_LOCAL_UNIT,
    STKTPLG_OM_IPC_CMD_GET_ALL_UNITS_PORT_MAPPING,
    STKTPLG_OM_IPC_CMD_GET_MAX_PORT_NUMBER_OF_MODULE_ON_BOARD,
    STKTPLG_OM_IPC_CMD_GET_MODULE_HI_GI_PORT_NUM,
    STKTPLG_OM_IPC_CMD_GET_UNIT_MODULE_PORTS,
    STKTPLG_OM_IPC_CMD_GET_UNIT_MAINBOARD_PORTS,
    STKTPLG_OM_IPC_CMD_IS_OPTION_MODULE,
    STKTPLG_OM_IPC_CMD_GET_STACK_STATUS,
    STKTPLG_OM_IPC_CMD_GET_UNIT_STATUS,
    STKTPLG_OM_IPC_CMD_GET_MODULE_STATUS,
    STKTPLG_OM_IPC_CMD_GET_MODULE_TYPE,
    STKTPLG_OM_IPC_CMD_IS_VALID_DRIVER_UNIT,
    STKTPLG_OM_IPC_CMD_GET_MY_DRIVER_UNIT,
    STKTPLG_OM_IPC_CMD_DRIVER_UNIT_EXIST,
    STKTPLG_OM_IPC_CMD_GET_NEXT_DRIVER_UNIT,
    STKTPLG_OM_IPC_CMD_GET_CURRENT_STACKING_DB,
    STKTPLG_OM_IPC_CMD_COMBO_SFP_INDEX_TO_USER_PORT,
    STKTPLG_OM_IPC_CMD_USER_PORT_TO_COMBO_SFP_INDEX,
    STKTPLG_OM_IPC_CMD_GET_MAIN_BOARD_PORT_NUM,
    STKTPLG_OM_IPC_CMD_GET_MY_MODULE_ID,
    STKTPLG_OM_IPC_CMD_GET_LOCAL_MODULE_PORT_NUMBER,
    STKTPLG_OM_IPC_CMD_UNIT_MODULE_TO_EXT_UNIT,
    STKTPLG_OM_IPC_CMD_IS_MODULE_PORT,
    STKTPLG_OM_IPC_CMD_PORT_LIST_2_DRIVER_UNIT_LIST,
    STKTPLG_OM_IPC_CMD_GET_MASTER_BUTTON_STATUS,
    STKTPLG_OM_IPC_CMD_IS_HBT_TOO_OLD,
    STKTPLG_OM_IPC_CMD_IS_ALL_SLAVES_AND_MODULES_WITH_MC,
    STKTPLG_OM_IPC_CMD_LOGICAL_2_PHY_DEVICE_PORT_ID,
    STKTPLG_OM_IPC_CMD_GET_MAX_CHIP_NUM,
    STKTPLG_OM_IPC_CMD_GET_NEIGHBOR_STACKING_PORT,
    STKTPLG_OM_IPC_CMD_GET_NEIGHBOR_STACKING_PORT_BY_CHIP_VIEW,
    STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_BOARD_ID,
    STKTPLG_OM_IPC_CMD_GET_UNIT_FAMILY_ID,
    STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_FAMILY_ID,
    STKTPLG_OM_IPC_CMD_GET_UNIT_PROJECT_ID,
    STKTPLG_OM_IPC_CMD_GET_NEXT_UNIT_PROJECT_ID,
    STKTPLG_OM_IPC_CMD_GET_STACKING_STATE,
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_IPC_CMD_IS_ANY_UNIT_NEED_TO_PROCESS,
#endif
    STKTPLG_OM_IPC_CMD_IS_STKTPLG_STATE_CHANGED,
    STKTPLG_OM_IPC_CMD_GET_STACKINGPORT_INFO
};

typedef enum STKTPLG_OM_Simple_State_E
{
    STKTPLG_OM_SIMPLE_STATE_ARBITRATION = 0,
    STKTPLG_OM_SIMPLE_STATE_MASTER,
    STKTPLG_OM_SIMPLE_STATE_SLAVE
} STKTPLG_OM_Simple_State_T;

/* STKTPLG_OM debug mode flag definitions
 */
#define STKTPLG_OM_DEBUG_MODE_FLAG_DEBUG_MSG         BIT_0

#define STKTPLG_OM_MSGBUF_TYPE_SIZE ((UI32_T)(&(((STKTPLG_OM_IPCMsg_T*)0)->data)))

/* currently, the maximum number of entry will be used is 8
 * it might be better to define as a sys_adpt constant
 */
#define STKTPLG_OM_NUM_OF_ENTRY_FOR_HI_GI_PORT 8

/*bitrate for identify sfp type,add by wx*/
#define STKTPLG_OM_BITRATE_100FX               0x01
#define STKTPLG_OM_BITRATE_1000SFP             0x0a
#define STKTPLG_OM_BITRATE_10GSFP              0x64

#define UNITS_FOR_PORTMAPPING   ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK+1)/2)
#define UNKNOWN_MODULE_ID      0xff
#define UNKNOWN_DEVICE_ID      0xff
#define UNKNOWN_DEVICE_PORT_ID 0xff
#define UNKNOWN_PHY_ID         0xff
#define STKTPLG_HBT1_TIMEOUT_MAX_COUNT  1

/* system reboot reason
 */

#define STKTPLG_OM_REBOOT_FROM_COLDSTART          1
#define STKTPLG_OM_REBOOT_FROM_WARMSTART          2
#define STKTPLG_OM_REBOOT_FROM_RUNTIME_PROVISION  3

#define STKTPLG_BOOT_REASON_MAGIC_WORD            0xd4c3b2a1
#define STKTPLG_BOOT_REASON_COLD_START_MAGIC_WORD 0x0

/* length of MAC address
 */

#define STKTPLG_MAC_ADDR_LEN                             SYS_ADPT_MAC_ADDR_LEN
#define STKTPLG_OM_MAX_MODULE_NBR                        1

/* define topology event and event message
 */

#define STKTPLG_NO_ACTION                                0x00
#define STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE     0x01
#define STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE         0x02
#define STKTPLG_MASTER_LOSE_MSG                          0x03
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#define STKTPLG_UNIT_HOT_INSERT_REMOVE                   0x04
#endif


/* define HBT type
 */

#define STKTPLG_HBT_TYPE_0                               0
#define STKTPLG_HBT_TYPE_1                               1
#define STKTPLG_HBT_TYPE_2                               2
#define STKTPLG_TCN                                      3
#define STKTPLG_HELLO_TYPE_0                             4
#define STKTPLG_HELLO_TYPE_1_ENQ                         5 /* obsolete */
#define STKTPLG_HELLO_TYPE_1_RDY                         6 /* obsolete */
#define STKTPLG_HBT_TYPE_0_REBOUND                       7
#define STKTPLG_HBT_TYPE_1_REBOUND                       8
#define STKTPLG_EXP_INFO_TO_SLAVE                        9 /* obsolete */
#define STKTPLG_EXP_INFO_TO_MASTER                      10 /* obsolete */
#define STKTPLG_CLOSED_LOOP_TCN                         11
#define STKTPLG_HBT_TYPE_0_UP                           12 /* obsolete */
#define STKTPLG_HBT_TYPE_0_DOWN                         13
#define STKTPLG_HBT_TYPE_1_DOWN                         14
#define STKTPLG_HBT_TYPE_2_DOWN                         15
#define STKTPLG_HBT_TYPE_0_ACK                          16 /* obsolete */
#define STKTPLG_HBT_TYPE_HALT                           17
#define STKTPLG_EXP_NOTIFY                              18 /* obsolete */
#define STKTPLG_TPLG_SYNC                               19
#define STKTPLG_TCN_TYPE_1                              20
#define STKTPLG_HBT_TYPE_MAX                            21

/* define timer constant for stack management algorithm
 */

#define STKTPLG_TIMER_HBT0_TIMEOUT              1000  /* 0.5 seconds */
                                                      /* 1 ticks is 10 ms
                                                       * in fact, we should define this
                                                       * constant with ticks per second
                                                      */
#define STKTPLG_TIMER_HBT1_TIMEOUT_MASTER       4500
#define STKTPLG_TIMER_HBT1_TIMEOUT              2000  /* master Wait for HBT1 return */
#define STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE     ((STKTPLG_TIMER_HBT1_TIMEOUT + STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER)*STKTPLG_HBT1_TIMEOUT_MAX_COUNT)
#define STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER    500  /* Period of sending HBT1 */
#define STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT     2000
#define STKTPLG_TIMER_HELLO_TIMEOUT             2000
#define STKTPLG_TIMER_HELLO_1_TIMEOUT           2000
#define STKTPLG_TIMER_HBT2_TIMEOUT              2000
#define STKTPLG_TIMER_PREEMPTED_TIMEOUT         2000
#define STKTPLG_TIMER_TPLG_SYNC_TIMEOUT         2000
#define STKTPLG_TIMER_SEND_HELLO_TIMEOUT        150


#define STKTPLG_TIMER_HELLO_0_UP                0x00
#define STKTPLG_TIMER_HELLO_0_DOWN              0x01
#define STKTPLG_TIMER_HBT0_UP                   0x02
#define STKTPLG_TIMER_HBT0_DOWN                 0x03
#define STKTPLG_TIMER_HBT1_UP                   0x04
#define STKTPLG_TIMER_HBT1_DOWN                 0x05
#define STKTPLG_TIMER_HBT1_M                    0x06
#define STKTPLG_TIMER_HBT2                      0x07
#define STKTPLG_TIMER_GET_TPLG_INFO_UP          0x08
#define STKTPLG_TIMER_HELLO_1                   0x09
#define STKTPLG_TIMER_PREEMPTED_MASTER          0x0a
#define STKTPLG_TIMER_GET_TPLG_INFO_DOWN        0x0b
#define STKTPLG_TIMER_HBT1_M_DOWN               0x0c
#define STKTPLG_TIMER_TPLG_SYNC                 0x0d
#define STKTPLG_TIMER_SEND_HELLO                0x0e
#define STKTPLG_TIMER_MAX                       0x0f

#define STKTPLG_OM_MAX_NUM_OF_COMPONENTS 32
#define STKTPLG_OM_MIB_STRING_LENGTH     32

/* NOTE! String length(not including '\0') of following constants should be less
 * than or equal to STKTPLG_MGR_MIB_STRING_LENGTH
 */
#define MIB2_STACK_ENT_DESC_STR           " Stack "
#define MIB2_UNIT_ENT_DESC_STR            " Unit %ld "
#define MIB2_STACK_MGT_ENT_DESC_STR       " Primary Stack Management Card "
#define MIB2_BACK_MGT_NET_DESC_STR        " Backup Stack Manager %ld "
#define MIB2_STACK_CARD_DESC_STR          " Stacking Card Unit %ld "
#define MIB2_PORT_ENT_DESC_STR            " Port %ld Unit %ld "
#define MIB2_PORT_IF_DESC_STR             "RMON Port %ld on Unit %ld"

/* NAMING CONSTANT DECLARARTIONS
 */
#define  MIB2ENT_NULL_ENTINDEX        0xFFFFFFFF

/* range of Entindex 
 */
#define STACK_ENT_START             1
#define STACK_ENT_END               4
#define UNIT_ENT_START	            101
#define UNIT_ENT_END                110
#define MANAGEMENT_ENT_START	    201
#define MANAGEMENT_ENT_END	        210
#define STACK_MODUL_ENT_START       401
#define STACK_MODUL_ENT_END         410
#define PORT_ENT_START              1001
#define PORT_ENT_END                9999

#define INDEX_NUM_PER_UNIT          1000 /*depend on PORT_ENT_START */

/* type of interfaceTYPE DECLARATIONS
 */
enum MIB2ENT_TYPE_E
{
   ENT_STACK_NAME = 1,
   ENT_UNIT_NUM,
   ENT_MANAGEMENT_PORT,
   ENT_STACK_CARD,
   ENT_PHYSICAL_PORT
};

enum MIB2ENT_CLASS_E
{
   MIB2ENT_CLASS_OTHER = 1,
   MIB2ENT_CLASS_ENT_CLASS_UNKNOWN,
   MIB2ENT_CLASS_CHASSIS,
   MIB2ENT_CLASS_BACKPLANE,
   MIB2ENT_CLASS_CONTAINER,
   MIB2ENT_CLASS_POWERSUPPLY,
   MIB2ENT_CLASS_FAN,
   MIB2ENT_CLASS_SENSOR,
   MIB2ENT_CLASS_MODULE,
   MIB2ENT_CLASS_PORTS,
   MIB2ENT_CLASS_STACK
};

enum MIB2ENT_FRU_TYPE_E
{
   MIB2ENT_FRU_TRUE = 1,
   MIB2ENT_FRU_FALSE
};
/* MACRO FUNCTION DECLARATIONS
 */
/* define the macros about the index translation
 */
#define STKTPLG_OM_UPORT_TO_IFINDEX(unit, port)     ( ((unit)-1) * SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + (port) )
#define STKTPLG_OM_IFINDEX_TO_UNIT(ifindex)         ( ((UI32_T)(((ifindex)-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))+1 )
#define STKTPLG_OM_IFINDEX_TO_PORT(ifindex)         ( (ifindex) - (STKTPLG_OM_IFINDEX_TO_UNIT(ifindex)-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT )
#define STKTPLG_OM_IFINDEX_TO_DRIVERUNIT(ifindex)   ( STKTPLG_OM_IsModulePort(STKTPLG_OM_IFINDEX_TO_UNIT(ifindex),STKTPLG_OM_IFINDEX_TO_PORT(ifindex))?STKTPLG_OM_IFINDEX_TO_UNIT(ifindex)+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK:STKTPLG_OM_IFINDEX_TO_UNIT(ifindex))


#define STKTPLG_OM_TRUNKID_TO_IFINDEX(trunk_id)     ( (trunk_id) + SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1 )
#define STKTPLG_OM_IFINDEX_TO_TRUNKID(ifindex)      ( (ifindex) - SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + 1 )

#define STKTPLG_OM_IS_USER_PORT(ifindex)    ( ifindex >= SYS_ADPT_ETHER_1_IF_INDEX_NUMBER &&  \
                                          ifindex <= (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK) * (SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT) )

/* Macro function for calculation of ipc msg_buf size based on structure name
 * used in STKTPLG_OM_IPCMsg_T.data
 */
#define STKTPLG_OM_GET_MSGBUFSIZE(msg_data_type) \
        (STKTPLG_OM_MSGBUF_TYPE_SIZE + sizeof(msg_data_type))

/* DATA TYPE DECLARATIONS
 */
typedef enum STKTPLG_OM_GBIC_Connector_Value_E
{
    GBIC_CONNECTOR_VLAUE_UNKNOWN                            = 0,
    GBIC_CONNECTOR_VLAUE_SC                                 = 1,
    GBIC_CONNECTOR_VLAUE_FIBRE_STYLE_1                      = 2,
    GBIC_CONNECTOR_VLAUE_FIBRE_STYLE_2                      = 3,
    GBIC_CONNECTOR_VLAUE_BNT_TNC                            = 4,
    GBIC_CONNECTOR_VLAUE_FIBRE_COAXIAL                      = 5,
    GBIC_CONNECTOR_VLAUE_FIBRE_JACK                         = 6,
    GBIC_CONNECTOR_VLAUE_LC                                 = 7,
    GBIC_CONNECTOR_VLAUE_MTRJ                               = 8,
    GBIC_CONNECTOR_VLAUE_MU                                 = 9,
    GBIC_CONNECTOR_VLAUE_SG                                 = 0x0a,
    GBIC_CONNECTOR_VLAUE_OPTICAL_PIGTAIL                    = 0x0b,
    GBIC_CONNECTOR_VLAUE_MPO                                = 0x0c,
    GBIC_CONNECTOR_VLAUE_RESERVED1_FIRST                    = 0x0d,
    GBIC_CONNECTOR_VLAUE_RESERVED1_LAST                     = 0x1f,
    GBIC_CONNECTOR_VLAUE_HSSDC_2                            = 0x20,
    GBIC_CONNECTOR_VLAUE_COPPER_PIGTAIL                     = 0x21,
    GBIC_CONNECTOR_VLAUE_RJ45                               = 0x22,
    GBIC_CONNECTOR_VLAUE_NO_SEPARABLE_CONNECTOR             = 0x23,
    GBIC_CONNECTOR_VLAUE_RESERVED2_FIRST                    = 0x24,
    GBIC_CONNECTOR_VLAUE_RESERVED2_LAST                     = 0x7e,
    GBIC_CONNECTOR_VLAUE_CONNECTOR_NAME_IN_BYTES_128_143    = 0x7f,
    GBIC_CONNECTOR_VLAUE_VENDOR_SPECIFIC_FIRST              = 0x80,
    GBIC_CONNECTOR_VLAUE_VENDOR_SPECIFIC_LAST               = 0xff,

} STKTPLG_OM_GBIC_Connector_Value_T;

typedef enum STKTPLG_OM_GBIC_Gigabit_Ethernet_Compliance_Codes_Value_E
{
    GIGABIT_CODES_VLAUE_1000BASE_SX = (1<<0),
    GIGABIT_CODES_VLAUE_1000BASE_LX = (1<<1),
    GIGABIT_CODES_VLAUE_1000BASE_CX = (1<<2),
    GIGABIT_CODES_VLAUE_1000BASE_T  = (1<<3),

} STKTPLG_OM_GBIC_Gigabit_Ethernet_Compliance_Codes_Value_T;

/* define state for stack topology
 */

typedef enum STKTPLG_OM_State_E
{
    STKTPLG_STATE_INIT = 0,
    STKTPLG_STATE_ARBITRATION,
    STKTPLG_STATE_STANDALONE,
    STKTPLG_STATE_MASTER_SYNC,
    STKTPLG_STATE_GET_TOPOLOGY_INFO,
    STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT,
    STKTPLG_STATE_MASTER,
    STKTPLG_STATE_SLAVE,
    STKTPLG_STATE_HALT,
    STKTPLG_STATE_PRE_STANDALONE,
    STKTPLG_STATE_MAX

} STKTPLG_OM_State_T;


typedef struct
{
    UI32_T  pkt_type;
    UI32_T  pkt_len;
    UI8_T   module_id ;
    UI8_T   module_type;
    UI8_T   module_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; /*Charles change this name*/
}__attribute__((packed, aligned(1))) Module2Main_Packet_T;



typedef struct
{
    UI32_T  pkt_type;
    UI32_T  unit_id;
    UI8_T   module_id ;
    UI8_T   module_type;
    BOOL_T  is_master_ready;
    UI32_T  master_unit_id;
    UI32_T  mainbrd_port_num;
    UI32_T  start_unit_forportmapping;
    DEV_SWDRV_Device_Port_Mapping_T  port_mapping[UNITS_FOR_PORTMAPPING][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    Stacking_Info_T                  stack_info;
}__attribute__((packed, aligned(1)))Main2Module_Packet_T;

typedef struct
{
    UI32_T  pkt_type;
}__attribute__((packed, aligned(1)))Main2Module_ResetPacket_T;


typedef struct
{
    UI8_T   mod_dev_id;              /* module ID : 5-bit, device ID : 3-bit */
    UI8_T   device_port_phy_id;      /* physical port ID : 4-bit, PHY ID of a given physical port : 4-bit */
    UI8_T   port_type;               /* The type/function of given port      */
} STKTPLG_Device_Port_Mapping_T;

/* data type for timer
 */

typedef struct STKTPLG_OM_Timer_S
{
    UI32_T ticks_for_timeout;
    BOOL_T active;
    UI8_T  reserved[3];

} STKTPLG_OM_Timer_T;


typedef struct STKTPLG_OM_Info_S
{
    UI32_T project_id;                                          /* family ID + board id; 2003/12/5  */
    UI8_T  mac_addr[SYS_ADPT_MAC_ADDR_LEN];                     /* MAC address                      */
    UI8_T  serial_no[SYS_ADPT_SERIAL_NO_STR_LEN + 1];           /* serial number                    */
    UI8_T  manufacture_date[SYS_ADPT_MANUFACTURE_DATE_LEN + 1]; /* Product date, option to key in. 2001-10-31  */
    UI8_T  mainboard_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];       /* main board hardware version      */
    UI8_T  agent_hw_ver[SYS_ADPT_HW_VER_STR_LEN + 1];           /* agent board hardware version     */
    UI8_T  loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];             /* Loader software version          */
    UI8_T  loader_customized_ver[SYS_ADPT_LOADER_CUSTOMIZED_VER_STR_LEN + 1]; /* Loader customized software version */
    UI8_T  post_ver[SYS_ADPT_FW_VER_STR_LEN + 1];               /* POST software version            */
    UI8_T  post_customized_ver[SYS_ADPT_POST_CUSTOMIZED_VER_STR_LEN + 1]; /* POST customzied software version */
    UI8_T  runtime_sw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];         /* runtime version                  */
    UI8_T  runtime_customized_sw_ver[SYS_ADPT_FW_CUSTOMIZED_VER_STR_LEN + 1]; /* runtime customized version */
    UI8_T  kernel_ver[SYS_ADPT_KERNEL_VER_STR_LEN + 1];         /* kernel version                   */
    UI8_T  epld_ver[SYS_ADPT_EPLD_VER_STR_LEN+1];               /* EPLD version*/
    UI8_T  model_number[SYS_ADPT_MODEL_NUMBER_LEN + 1];         /* module number                    */
    UI8_T  board_id;                                            /* model number string              */

    /* This records software build time as Unix seconds,
     * i.e. number of seconds since 1970.01.01 00:00 UTC.
     * Use SYS_TIME_ConvertTime for display to UI.
     */
    UI32_T  software_build_time;
} STKTPLG_OM_Info_T;

typedef struct STK_UNIT_CFG_S
{
    STKTPLG_OM_Info_T       board_info;
    UI8_T                   sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI8_T                   sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI32_T                  sw_identifier;
    UI8_T                   module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T                   module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T                   exp_module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T                   exp_module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT];
    UI8_T                   nport;       /* total installed ports of the unit */
    UI8_T                   num_chips;   /* Chip number of the device */
    UI32_T                  boot_reason;
  /*UI8_T                   module_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; Charles chenge this name*/
    UI8_T                   module_expected_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    UI8_T                   cfg_hw_port_mode[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
    UI8_T                   oper_hw_port_mode[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
#endif
} STK_UNIT_CFG_T;


typedef struct STK_UNIT_CFG_S2
{
    STKTPLG_OM_Info_T       board_info;
    UI8_T                   sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI8_T                   sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI32_T                  sw_identifier;
    UI8_T                   module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT+1];
    UI8_T                   module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT+1];
    UI8_T                   exp_module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT+1];
    UI8_T                   exp_module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT+1];
    UI8_T                   nport;     /* total installed ports of the unit */
    UI8_T                   num_chips; /* Chip number of the device */
    UI32_T                  boot_reason;
    UI8_T                   module_expected_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
} STK_UNIT_CFG_T2;

typedef struct STKTPLG_OM_switchModuleInfoEntry_S
{
    UI32_T  unit_index;
    UI32_T  module_index;
    UI8_T   hardware_ver[SYS_ADPT_HW_VER_STR_LEN + 1];
    UI8_T   microcode_ver[SYS_ADPT_HW_VER_STR_LEN + 1];
    UI8_T   loader_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   boot_rom_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   op_code_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    UI8_T   port_number;
    UI8_T   serial_number[SYS_ADPT_SERIAL_NO_STR_LEN + 1];
    UI8_T   module_type;
    UI8_T   model_number[SYS_ADPT_MODEL_NUMBER_LEN + 1];
    UI8_T   epld_ver[SYS_ADPT_EPLD_VER_STR_LEN+1];
    UI8_T   desc[SYS_ADPT_MODULE_DESC_STR_LEN + 1];
    UI8_T   module_validation;
    UI32_T  xenpak_status;
} STKTPLG_OM_switchModuleInfoEntry_T;

typedef struct STKTPLG_OM_StackingDB_S
{
    UI32_T unit_id;
    UI8_T mac_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI32_T device_type;
}STKTPLG_OM_StackingDB_T;

/* data type for HBT (Heart Beat Train) header
 */
typedef struct STKTPLG_OM_HBT_Header_S
{
    /* keep what version
     */
    UI8_T  version;
    /* type of this HBT
     *   we have three types:
     *     type 0: do msater election
     *     type 1: keep alive
     *     type 2: get topology information
     *     type 3: TCN
     *     type 4: Hello
     */
    UI8_T  type;
    /* for receiving,
     *    indicate what unit id should be for receiving unit
     * for transmit,
     *    indicate what unit id should be for next unit
     */
    UI8_T  next_unit;
    /* sequence number of this packet.
     */
    UI8_T  seq_no;
    /* indicate master units in the whole stacking
     *    LSB will be unit 1
     */
    UI16_T masters_location;
    /* length of payload, do not inclue length of this header
     */
    UI16_T length;
    /* checksum for HBT
     */
    UI32_T checksum;
}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_Header_T;


typedef struct STKTPLG_OM_HBT_0_1_Payload_S
{
    /* mac address of this unit
     */
    UI8_T  mac_addr[STKTPLG_MAC_ADDR_LEN];

    /* bit-wised fields
     */
    UI8_T expansion_module_exist:1;  /* set by slaves */
    UI8_T expansion_module_ready:1;
    /* if this unit is slave, this variable indicates if this unit enters slave
     * and become stable
     */
    UI8_T slave_ready:1;
    /* boolean variable to keep if button is pressed or not
     */
    UI8_T button_pressed:1;
    UI8_T provision_completed_state:1;
    UI8_T preempted_master:1;
    UI8_T is_ring:1;
    UI8_T preempted:1;
    UI8_T other_expansion_module_dirty:1;  /* set by master to notify slaves */
    UI8_T master_provision_completed:1;

    /* image type of this unit
     */
    UI16_T image_type;
    /* how many switch chips are installed for this unit
     */
    UI8_T  chip_nums;
    /* for receiving,
     *    indicate what unit id should be for receiving unit
     * for transmit,
     *    indicate what unit id should be for next unit
     */
    UI8_T  unit_id;
    /* starting module ID assigned from master for this unit
     */
    UI8_T  start_module_id;
    /* board ID of this unit
     */
    UI8_T  board_id;

    UI16_T expansion_module_type;

    UI8_T  expansion_module_id;

    /*1231 */
    UI8_T  module_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; /*Charles change this name*/

    /* stacking ports link status
     */
    UI8_T  stacking_ports_link_status;

  /*UI8_T  phy_unit_id;*/

    /* variable to keep how many units are stacking together
     */
    UI8_T  total_units;

    UI8_T  next_stacking_unit;

    /* variable to keep how many units are stacking together
     */
    UI8_T  total_units_up;
    /* variable to keep how many units are stacking together
     */
    UI8_T  total_units_down;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    UI8_T  past_master_mac[STKTPLG_MAC_ADDR_LEN];
#else

    STKTPLG_Device_Port_Mapping_T port_mapping[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];
#endif
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
    UI8_T teng_module_id[SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT];
#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    STKTPLG_TYPE_Stacking_Port_Option_T stacking_port_option;
#endif
}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_0_1_Payload_T;


typedef struct STKTPLG_OM_TPLG_SYNC_Payload_S
{
    /* Information from which unit want to sync with you.
     */
    UI8_T           src_unit_id;
    /* If you got the information, please set this formation to your database,
     * and turn off the bit belong to you to indicate you got this information.
     */
    UI8_T           unit_bmp_to_get;
    /* This if my unit information. Thanks.
     */
    STK_UNIT_CFG_T  unit_cfg;
}__attribute__((packed, aligned(1))) STKTPLG_OM_TPLG_SYNC_Payload_T;


typedef struct STKTPLG_OM_HELLO_0_S
{
    STKTPLG_OM_HBT_Header_T header;

    UI8_T                   mac_addr[STKTPLG_MAC_ADDR_LEN];

    /* Packet issued from which port, LAN_TYPE_TX_UP_LINK or LAN_TYPE_TX_DOWN_LINK.  
     */
    UI8_T                   tx_up_dw_port;
}__attribute__((packed, aligned(1))) STKTPLG_OM_HELLO_0_T;

typedef struct STKTPLG_OM_HBT_2_Payload_S
{
    UI8_T  unit_id;

    /* unit configuration/information
     */
    STK_UNIT_CFG_T  unit_cfg;

}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_2_Payload_T;

typedef struct STKTPLG_OM_HBT_22_Payload_S
{
    UI8_T  unit_id;

    /* unit configuration/information
     */
    STK_UNIT_CFG_T2  unit_cfg;

}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_22_Payload_T;

typedef struct STKTPLG_OM_HBT_0_1_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T       header;
    /* originator_runtime_fw_ver: slave units will retrieve the runtime firmware
     *                            version from this field so that whether
     *                            ISC status is normal or abnormal can be known.
     */
    UI8_T                         originator_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1];
    /* payload
     */
    STKTPLG_OM_HBT_0_1_Payload_T  payload[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_0_1_T;


typedef struct STKTPLG_OM_HBT_2_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T       header;
    /* payload
     */
    STKTPLG_OM_HBT_2_Payload_T    payload;

}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_2_T;

typedef struct STKTPLG_OM_HBT_22_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T       header;
    /* payload
     */
    STKTPLG_OM_HBT_22_Payload_T    payload;

}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_22_T;
typedef struct STKTPLG_OM_HBT_3_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T       header;
    /* payload
     */
    STKTPLG_OM_HBT_0_1_Payload_T  payload;

}__attribute__((packed, aligned(1))) STKTPLG_OM_HBT_3_T;

typedef struct STKTPLG_OM_TPLG_SYNC_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T         header;
    /* payload
     */
    STKTPLG_OM_TPLG_SYNC_Payload_T  payload;

}__attribute__((packed, aligned(1))) STKTPLG_OM_TPLG_SYNC_T;

typedef struct STKTPLG_OM_TCN_TYPE_1_S
{
    /* header
     */
    STKTPLG_OM_HBT_Header_T     header;
    /* payload
     */
    /* exist_units: bitmap of existing units in the stack
     *              if the slave unit that received this
     *              packet not in the bitmap, that slave
     *              unit will notify stkctrl to
     *              enter transition mode
     *              Refer to EPR ES4626F-SW-FLF-38-01081.
     */
    UI16_T                      exist_units;
   /* ttl: time to live. when ttl is 0, the received packet
    *      should be dropped. This field is added in version2.
    *      Refer to EPR ES4626F-SW-FLF-38-01270.
    */
    UI8_T                       ttl; 

}__attribute__((packed, aligned(1))) STKTPLG_OM_TCN_TYPE_1_T;

/* data type for control information of stack topology
 */

typedef struct STKTPLG_OM_Ctrl_Info_S
{
    /* state for stack topology
     */
    STKTPLG_OM_State_T    state;
    /* timer for stack management algorithm
     *
     *  0: timer for HBT 0,   1: timer for HBT 1
     *  2: timer for receiving packets from up-link stack port
     *  3: timer for get topology information from slave unit
     */
    STKTPLG_OM_Timer_T    timer[STKTPLG_TIMER_MAX];
    /* variable to keep unit id of this machine
     */
    UI8_T                 my_unit_id;
    /* variable to keep how many units are stacking together
     */
    UI8_T                 total_units;
    /* boolean variable to keep if buttone is pressed or not
     */
    BOOL_T                button_pressed;

    /* boolean variable to keep if button for master election is pressed or
     * not when we enter arbitration state
     * we should clear this variable before we enter arbitration state.
     */
    BOOL_T                button_pressed_arbitration;
    /* boolean variable to keep if we receive packets from up link stack
     * port within a certain period of time
     */
    BOOL_T                receive_packets;

    /* image type for this machine
     */
    UI16_T                image_type;
    /* how many switch chips installed in this machine
     */
    UI8_T                 chip_nums;
    /* board ID of this machine, with board ID, we can
     * know H/W configuration of this machine
     */
    UI8_T                 board_id;
    /* keep hbt information for sending hbt type 1 and
     * use this information to check if there is any topology change
     */
    STKTPLG_OM_HBT_0_1_T  stable_hbt_up;

    STKTPLG_OM_HBT_0_1_T  stable_hbt_down;

    /* keep unit id of master unit
     */
    UI8_T                 master_unit_id;
    /* keep which unit are we queried for topology information
     */
    UI8_T                 query_unit_up;

    UI8_T                 query_unit_down;
    /* keep sequence number of every kind of HBT packet
     */
    UI8_T                 seq_no[STKTPLG_HBT_TYPE_MAX];
    /* MAC address of this unit
     */
    UI8_T                 my_mac[STKTPLG_MAC_ADDR_LEN];
    /* MAC address of master unit
     */
    UI8_T                 master_mac[STKTPLG_MAC_ADDR_LEN];

    /* stacking ports logic link status - 
     * When received Hello on the port, the logic link status is on.
     * When Hello packet which is sent from the link partner is not received in
     * a period of time, the logical link status is changed to down.    
     * Contrast to up_phy_status and down_phy_status, a stacking port can be phy up, 
     * but logically down when 1. the other side of the cable is not a working stacking port.
     *                         2. DAC cable can make phy up with merely plugged 1 of its 2 sides.
     */
    UI8_T                 stacking_ports_logical_link_status;

    /* Master preempted
     */
    BOOL_T                preempted;

    UI8_T                 bounce_msg;


    /* variable to keep unit id of this machine
     */
    UI8_T                 my_logical_unit_id;

    UI8_T                 my_phy_unit_id_up;

    UI8_T                 my_phy_unit_id_down
    ;
    /* variable to keep how many units are stacking together
     */
    UI8_T                 total_units_up;
    /* variable to keep how many units are stacking together
     */
    UI8_T                 total_units_down;

    UI8_T                 last_module_id;

    UI8_T                 next_stacking_unit;

    UI8_T                 start_module_id;

    UI16_T                expansion_module_type;

    UI8_T                 expansion_module_id;

    UI8_T                 exp_module_id[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    BOOL_T                expansion_module_exist;

    BOOL_T                expansion_module_ready;

    BOOL_T                is_ring;

    BOOL_T                reset_state;

    UI32_T                stacking_ports_link_checked;

    UI8_T                 up_phy_status;

    UI8_T                 down_phy_status;

    UI8_T                 stacking_dev_id;

    STKTPLG_OM_HBT_0_1_T  prev_type_0;

    BOOL_T                stack_maintenance;

    UI8_T                 standalone_hello;

    BOOL_T                preempted_master;

    BOOL_T                provision_completed_state;

    /*1231 */
    UI8_T                 module_runtime_fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]; /*Charles change this name*/

    /* These 3 fields are for hot-swap module.
     */
    BOOL_T                  exp_module_state_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    UI32_T                  expected_module_types[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    UI32_T                  synced_module_types[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];

    /* For TPLG sync: indicate which unit need to sync my local unit databace
     */
    UI8_T                  stk_unit_cfg_dirty_sync_bmp;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
 /* For Unit Hot Insert/Remove. Stores whether is unit was MASTER or SLAVE*/
    UI8_T                   past_role;

    /* For Unit Hot Insert/Remove. Stores the MAC address of the last Master*/
    UI8_T                   past_master_mac[STKTPLG_MAC_ADDR_LEN];

    /* For Unit Hot Insert/Remove. Stable, if the database is ready to be copied*/
    BOOL_T                  stable_flag;
 #endif
 #if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    BOOL_T                 stacking_button_state;
 #endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    STKTPLG_TYPE_Stacking_Port_Option_T stacking_port_option;
#endif

} STKTPLG_OM_Ctrl_Info_T;


typedef struct STKTPLG_DataBase_Info_S
{
    /* mac address of this unit
     */
    UI8_T mac_addr[STKTPLG_MAC_ADDR_LEN];
    /* variable to keep board id of this machine
     */
    UI8_T device_type;
    UI8_T id_flag; /* 0: empty 1: used before 3:used now */

} STKTPLG_DataBase_Info_T;

typedef struct STKTPLG_OM_Hello_T1_S
{
    STKTPLG_OM_HBT_Header_T header;

    UI16_T expansion_module_type;

} STKTPLG_OM_Hello_T1_T;

typedef struct STKTPLG_OM_To_Module_S
{
    /* PDU type
    */
    UI8_T  type;    /* ADD_MODULE, ASSIGN_ID */
    /* Expansion module type
    */
    UI8_T  expansion_module_id;
    /* checksum for hello type 1
    */
    UI32_T checksum;

} STKTPLG_OM_To_Module_T;

typedef struct STKTPLG_OM_To_Master_S
{
    STKTPLG_OM_HBT_Header_T header;
    /* PDU type
    */
    UI8_T  type;
    /* IS Expansion module exist ?
    */
    BOOL_T expansion_module_exist;
    /* Expansion module type
    */
    UI16_T expansion_module_type;
    /* Expansion module ID
    */
    UI8_T  expansion_module_id;
    /* Unit id
    */
    UI8_T  unit_id;
    /* checksum for hello type 1
    */
    UI32_T checksum;

} STKTPLG_OM_To_Expansion_Module_T;

/********************************
 ** data type for ipc messages **
 ********************************
 */

/* data type for structures used in STKTPLG_OM_IPCMsg_T.data
 */
typedef struct STKTPLG_OM_OneUI32Data_S
{
    UI32_T value;
} STKTPLG_OM_OneUI32Data_T;

typedef struct STKTPLG_OM_TwoUI32Data_S
{
    UI32_T value[2];
} STKTPLG_OM_TwoUI32Data_T;

typedef struct STKTPLG_OM_ThreeUI32Data_S
{
    UI32_T value[3];
} STKTPLG_OM_ThreeUI32Data_T;

typedef struct STKTPLG_OM_FourUI32Data_S
{
    UI32_T value[4];
} STKTPLG_OM_FourUI32Data_T;

typedef struct STKTPLG_OM_RuntimeFwVer_S
{
    UI8_T value[SYS_ADPT_FW_VER_STR_LEN+1];
} STKTPLG_OM_RuntimeFwVer_T;

typedef struct STKTPLG_OM_MacAddr_S
{
    UI8_T value[STKTPLG_MAC_ADDR_LEN];
} STKTPLG_OM_MacAddr_T;

typedef struct STKTPLG_OM_DevicePortMapping_S
{
    DEV_SWDRV_Device_Port_Mapping_T    value[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
} STKTPLG_OM_DevicePortMapping_T;

typedef struct STKTPLG_OM_AllDevicePortMapping_S
{
    DEV_SWDRV_Device_Port_Mapping_T    value[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
}STKTPLG_OM_AllDevicePortMapping_T;

typedef struct STKTPLG_OM_GetStackingDBEntryByMac_S
{
    UI8_T mac[STKTPLG_MAC_ADDR_LEN];
    UI8_T device_type;
} STKTPLG_OM_GetStackingDBEntryByMac_T;

typedef struct STKTPLG_OM_GetHiGiPortNum_S
{
    UI32_T port_num[STKTPLG_OM_NUM_OF_ENTRY_FOR_HI_GI_PORT];
} STKTPLG_OM_GetHiGiPortNum_T;

typedef struct STKTPLG_OM_GetCurrentStackingDB_S
{
    STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
} STKTPLG_OM_GetCurrentStackingDB_T;

typedef struct STKTPLG_OM_PortList2DriverUnitList_S
{
    UI8_T  port_list[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
} STKTPLG_OM_PortList2DriverUnitList_T;

typedef struct STKTPLG_OM_SetModuleInfo_S
{
    UI32_T unit;
    UI32_T module_index;
    STKTPLG_OM_switchModuleInfoEntry_T module_info_entry;
} STKTPLG_OM_SetModuleInfo_T;

/* structure for the request/response ipc message in stktplg pom and om
 */
typedef struct
{
    union STKTPLG_OM_IPCMsg_Type_U
    {
        UI32_T cmd;    /* for sending IPC request. STKTPLG_OM_IPC_CMD_XXX */
        UI32_T result; /* for response */
    } type;

    union
    {
        STKTPLG_OM_OneUI32Data_T   one_ui32;
        STKTPLG_OM_TwoUI32Data_T   two_ui32;
        STKTPLG_OM_ThreeUI32Data_T three_ui32;
        STKTPLG_OM_FourUI32Data_T  four_ui32;
        STKTPLG_OM_RuntimeFwVer_T  runtime_fw_ver;
        STKTPLG_OM_MacAddr_T       mac_addr;
        STKTPLG_OM_Info_T          info;
        STK_UNIT_CFG_T             stk_unit_cfg;
        STKTPLG_OM_switchModuleInfoEntry_T switch_module_info_entry;
        STKTPLG_OM_SetModuleInfo_T         set_module_info;
        STKTPLG_OM_DevicePortMapping_T     device_port_mapping;
        STKTPLG_OM_AllDevicePortMapping_T  all_device_port_mapping;
        STKTPLG_DataBase_Info_T    database_info;
        STKTPLG_OM_GetStackingDBEntryByMac_T get_stacking_dbentry_by_mac;
        STKTPLG_OM_GetHiGiPortNum_T          get_hi_gi_port_num;
        STKTPLG_OM_GetCurrentStackingDB_T    get_current_stacking_db;
        STKTPLG_OM_PortList2DriverUnitList_T port_list_2_driver_unit_list;
    } data; /* contains the supplemntal data for the corresponding cmd */
} STKTPLG_OM_IPCMsg_T;

typedef struct STKTPLG_OM_OID_S
{
    I32_T  	num_components;		/* original:int */
    /* type of component_list MUST be consistent with
     * type of oid defined in SNMP
     */
    UI32_T  component_list[STKTPLG_OM_MAX_NUM_OF_COMPONENTS];	/* original:WORD */
} STKTPLG_OM_OID_T;

/* value that can be  R/W and need to keep in om */
typedef struct STKTPLG_OM_EntPhysicalEntryRw_S 
{
    UI8_T   ent_physical_serial_num[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_alias[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_asset_id[STKTPLG_OM_MIB_STRING_LENGTH + 1];
} STKTPLG_OM_EntPhysicalEntryRw_T;

typedef struct STKTPLG_OM_EntPhysicalEntry_S 
{
    STKTPLG_OM_OID_T  ent_physical_vendor_type;    /* object component list */
    I32_T   ent_physical_index;
    I32_T   ent_physical_contained_in;
    I32_T   ent_physical_class; 
    /* Note: valueas following
     *           -- other(1),
     *           -- unknown(2),
     *           -- chassis(3),
     *           -- backplane(4),
     *           -- container(5),     -- e.g., chassis slot or daughter-card holder
     *           -- powerSupply(6),
     *           -- fan(7),
     *           -- sensor(8),
     *           -- module(9),        -- e.g., plug-in card or daughter-card
     *           -- port(10),
     *           --stack(11)         -- e.g., stack of multiple chassis entities
     */
    I32_T   ent_physical_parent_rel_pos;
    UI8_T   ent_physical_descr[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_name[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_hardware_rev[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_firmware_rev[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_software_rev[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_mfg_name[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_model_name[STKTPLG_OM_MIB_STRING_LENGTH + 1];
    UI8_T   ent_physical_is_fru; /* 1:true, 2:false */
    STKTPLG_OM_EntPhysicalEntryRw_T ent_physical_entry_rw;
} STKTPLG_OM_EntPhysicalEntry_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
BOOL_T STKTPLG_OM_AttachProcessResources(void);

BOOL_T STKTPLG_OM_SetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info);


/* FUNCTION NAME : STKTPLG_OM_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG OM which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InitiateProcessResources(void);

/* FUNCTION NAME : STKTPLG_OM_InitiateSystemResources
 * PURPOSE: This function initializes STKTPLG OM system resources
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_InitiateSystemResources(void);

/* FUNCTION NAME : STKTPLG_OM_GetNumberOfUnit
 * PURPOSE: To get the number of units existing in the stack.
 * INPUT:   None.
 * OUTPUT:  num_of_unit  -- the number of units existing in the stack.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetNumberOfUnit(UI32_T *num_of_unit);


/* FUNCTION NAME : STKTPLG_OM_GetLocalMaxPortCapability
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalMaxPortCapability(UI32_T *max_port_number);


/* FUNCTION NAME : STKTPLG_OM_GetMaxPortCapability
 * PURPOSE: To get the maximum port number of the target unit.
 * INPUT:   unit               -- unit id
 * OUTPUT:  *max_port_number_p -- maximum port number
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetMaxPortCapability(UI8_T unit, UI32_T *max_port_number_p);

/* FUNCTION NAME : STKTPLG_OM_SetMaxPortCapability
 * PURPOSE: To set the maximum port number of the target unit.
 * INPUT:   unit             -- unit id
 * OUTPUT:  max_port_number -- maximum port number
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_SetMaxPortCapability(UI8_T unit, UI32_T max_port_number);

/* FUNCTION NAME : STKTPLG_OM_GetMaxPortNumberOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_OM_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number);


/* FUNCTION NAME : STKTPLG_OM_GetPortType
 * PURPOSE: To get the type of the specified port of the specified unit.
 * INPUT:   unit_id    -- unit id.
 *          port       -- phyical(user) port id
 * OUTPUT:  port_type    -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Reference leaf_es3526a.mib
 *          -- #define VAL_portType_other               1L
 *          -- #define VAL_portType_hundredBaseTX       2L
 *          -- #define VAL_portType_hundredBaseFX       3L
 *          -- #define VAL_portType_thousandBaseSX      4L
 *          -- #define VAL_portType_thousandBaseLX      5L
 *          -- #define VAL_portType_thousandBaseT       6L
 *          -- #define VAL_portType_thousandBaseGBIC    7L
 *          -- #define VAL_portType_thousandBaseMiniGBIC        8L
 *          -- #define VAL_portType_hundredBaseFxScSingleMode   9L
 *          -- #define VAL_portType_hundredBaseFxScMultiMode    10L
 *
 */
BOOL_T STKTPLG_OM_GetPortType(UI32_T unit, UI32_T port, UI32_T *port_type);

/* FUNCTION NAME : STKTPLG_OM_GetSfpPortType
 * PURPOSE: To get the type of fiber medium of the specified port of the specified unit.
 * INPUT:   unit_id  	 -- unit id.
 *          sfp_index
 * OUTPUT:  port_type_p  -- port type of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSfpPortType(UI32_T unit, UI32_T sfp_index, UI32_T *port_type_p);

/* FUNCTION NAME : STKTPLG_OM_GetModuleID
 * PURPOSE: To get the module id of the specified port of the specified unit.
 * INPUT:   unit_id      -- unit id.
 *          u_port       -- phyical(user) port id
 * OUTPUT:  module_id    -- module id of the specified port of the specified unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   Here are two defined return module id
 *          SYS_HWCFG_MODULE_ID_10G_COPPER
 *          SYS_HWCFG_MODULE_ID_XFP
 *          SYS_HWCFG_MODULE_ID_10G_SFP
 *
 */
BOOL_T STKTPLG_OM_GetModuleID(UI32_T unit, UI32_T u_port, UI8_T *module_id);

/* FUNCTION NAME: STKTPLG_OM_UnitExist
 * PURPOSE: This function is used to check if the specified unit is
 *          existing or not.
 * INPUT:   unit_id  -- unit id
 * OUTPUT:  None.
 * RETUEN:  TRUE   -- exist
 *          FALSE  -- not exist
 * NOTES:   Use got mac address of each unit to
 *          know if unit exist or not.
 *
 */
BOOL_T STKTPLG_OM_UnitExist(UI32_T unit_id);

/* FUNCTION NAME : STKTPLG_OM_GetMyRuntimeFirmwareVer
 * PURPOSE: Get the runtime firmware version of the local unit.
 * INPUT:   None
 * OUTPUT:  runtime_fw_ver_ar - Runtime firmware version will be put to this array.
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_OM_GetMyRuntimeFirmwareVer(UI8_T runtime_fw_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1]);

/* FUNCTION NAME : STKTPLG_OM_GetMyUnitID
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMyUnitID(UI32_T *my_unit_id);


/* FUNCTION NAME : STKTPLG_OM_GetLocalUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalUnitBaseMac(UI8_T *base_mac_addr);
#if (SYS_CPNT_MGMT_PORT == TRUE)
BOOL_T STKTPLG_OM_GetLocalUnitBaseMac_ForMgmtPort(UI8_T *base_mac_addr);
#endif

/* FUNCTION NAME : STKTPLG_OM_GetUnitBaseMac
 * PURPOSE: To get the base address of the local unit.
 * INPUT:   unit_id         -- unit id.
 * OUTPUT:  base_mac_addr   -- base mac address (6 bytes) of this unit.
 * RETUEN:  TRUE            -- successful.
 *          FALSE           -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetUnitBaseMac(UI8_T unit, UI8_T *base_mac_addr);

/* FUNCTION NAME: STKTPLG_OM_GetUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitBoardID(UI32_T unit, UI32_T *board_id);

/* FUNCTION NAME : STKTPLG_OM_PortExist
 * PURPOSE: This function is used to check if the specified port is
 *          existing or not.
 * INPUT:   logical_unit_id -- Logical unit ID
 *          logical_port_id -- logical port ID
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_PortExist(UI32_T unit_id, UI32_T port_id);

/* FUNCTION NAME: STKTPLG_OM_GetBoardModuleTypeReg
 * PURPOSE: This function is used to get module type from board register.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetBoardModuleTypeReg(UI8_T module_index, UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetModuleAType
 * PURPOSE: This function is used to get module 1 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleAType(UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetModuleBType
 * PURPOSE: This function is used to get module 2 type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleBType(UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetOldModuleType
 * PURPOSE: This function is used to get old module type.
 * INPUT:   module_index    -- module index.
 *          *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetOldModuleType(UI8_T module_index, UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetModuleAOldType
 * PURPOSE: This function is used to get module 1 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleAOldType(UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetModuleBOldType
 * PURPOSE: This function is used to get module 2 old type.
 * INPUT:   *module_type    -- output buffer of module type value.
 * OUTPUT:  *module_type    -- module type value.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleBOldType(UI8_T *module_type);


/* FUNCTION NAME: STKTPLG_OM_GetSwServiceTag
 * PURPOSE: This function is used to get this the sw service tag
 * INPUT:   unit_id            -- unit id
 * OUTPUT:  sw_service_tag     -- sw service tag
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwServiceTag(UI32_T unit, UI8_T sw_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1]);

/* FUNCTION NAME: STKTPLG_OM_GetSwChassisServiceTag
 * PURPOSE: This function is used to get this the sw chassis service tag
 * INPUT:   unit_id            -- unit id
 * OUTPUT:  sw_service_tag     -- sw chassis service tag
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwChassisServiceTag(UI32_T unit, UI8_T sw_chassis_service_tag[SYS_ADPT_SERIAL_NO_STR_LEN + 1]);

/* FUNCTION NAME: STKTPLG_OM_GetSwIdentifier
 * PURPOSE: This function is used to get this the sw identifier
 * INPUT:   unit_id             -- unit id
 * OUTPUT:  sw_identifier_p     -- identifier
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetSwIdentifier(UI32_T unit, UI32_T *sw_identifier_p);

/* FUNCTION NAME: STKTPLG_OM_GetDeviceModulePresented
 * PURPOSE: This function is used to get this unit's module present
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  module_type     -- module presented
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetDeviceModulePresented(UI32_T unit, UI8_T module_presented[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT]);


/* FUNCTION NAME: STKTPLG_OM_GetDeviceModuleType
 * PURPOSE: This function is used to get this unit's module type
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  module_type     -- module type
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetDeviceModuleType(UI32_T unit, UI8_T module_type[SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT]);


/* FUNCTION NAME: STKTPLG_OM_GetModuleExpRuntimeFwVer
 * PURPOSE: This function is used to get module expected runtime firmware version
 * INPUT:   unit_id     -- unit id
 * OUTPUT:  fw_ver      -- firmware version string
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   None
 */
BOOL_T STKTPLG_OM_GetModuleExpRuntimeFwVer(UI32_T unit, UI8_T fw_ver[SYS_ADPT_FW_VER_STR_LEN + 1]);


/* FUNCTION NAME: STKTPLG_OM_GetSysInfo
 * PURPOSE: This function is used to get this system information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *sys_info       -- sys info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This funxtion is used by sysmgmt to get system information.
 *
 */
BOOL_T STKTPLG_OM_GetSysInfo(UI32_T unit, STKTPLG_OM_Info_T *sys_info);


/* FUNCTION NAME: STKTPLG_OM_GetDeviceInfo
 * PURPOSE: This function is used to get this device information.
 * INPUT:   unit_id         -- unit id
 * OUTPUT:  *device_info    -- device info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info);

/* FUNCTION NAME: STKTPLG_OM_GetModuleInfo
 * PURPOSE: This function is used to get this module's information.
 * INPUT:   unit         -- main board unit id
 *          module_index -- module index
 *          *module_info -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetModuleInfo(UI32_T unit, UI32_T module_index, STKTPLG_OM_switchModuleInfoEntry_T *module_info);

/* FUNCTION NAME: STKTPLG_OM_SlaveIsReady
 * PURPOSE: This utiity function is to report is slave mode is initialized completely
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE: Yes.
 *          FALSE: No.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_SlaveIsReady(void);

/* FUNCTION NAME: STKTPLG_OM_GetSwitchOperState
 * PURPOSE: This function is used to get the whole system oper state.
 * INPUT:   *switch_oper_state -- buffer of oper state.
 * OUTPUT:  *switch_oper_state -- oper state.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   VAL_switchOperState_other           1
 *          VAL_switchOperState_unknown         2
 *          VAL_switchOperState_ok              3
 *          VAL_switchOperState_noncritical     4
 *          VAL_switchOperState_critical        5
 *          VAL_switchOperState_nonrecoverable  6
 */
BOOL_T STKTPLG_OM_GetSwitchOperState(UI8_T *switch_oper_state);

/* FUNCTION NAME: STKTPLG_OM_GetMasterUnitId
 * PURPOSE: This routine is used to get master unit id.
 * INPUT:   *master_unit_id  -- master unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- successful
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMasterUnitId(UI8_T *master_unit_id);


/* FUNCTION NAME: STKTPLG_OM_GetStackingPort
 * PURPOSE: This function is used to get stacking port for a specified unit.
 * INPUT:   src_unit -- unit number
 * OUTPUT:  stack_port -- stacking port
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetSimplexStackingPort(UI32_T src_unit, UI32_T *stack_port);

/* FUNCTION NAME: STKTPLG_OM_GetUnitBootReason
 * PURPOSE: This function is used to get boot reason of some unit.
 * INPUT:   unit -- unit.
 * OUTPUT:  *boot_reason -- boot reason.
 * RETUEN:  TRUE :  successful
 *          FALSE : failed
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART
 */
BOOL_T STKTPLG_OM_GetUnitBootReason(UI32_T unit, UI32_T *boot_reason);

/* FUNCTION NAME: STKTPLG_OM_SetUnitBootReason
 * PURPOSE: This function is used to set boot reason of some unit.
 * INPUT:   unit -- unit.
 * OUTPUT:  boot_reason -- boot reason.
 * OUTPUT:  None
 * RETUEN:  TRUE  : successful
 *          FALSE : failed
 * NOTES:   STKTPLG_OM_REBOOT_FROM_COLDSTART
 *          STKTPLG_OM_REBOOT_FROM_WARMSTART
 */
BOOL_T STKTPLG_OM_SetUnitBootReason(UI32_T unit, UI32_T boot_reason);

#if (SYS_CPNT_MAU_MIB == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetJackType
 * -------------------------------------------------------------------------
 * FUNCTION: Get jack type of some user port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 *           jack_index -- Which jack.
 * OUTPUT  : jack_type  -- VAL_ifJackType_other
 *                         VAL_ifJackType_rj45
 *                         VAL_ifJackType_rj45S
 *                         VAL_ifJackType_db9
 *                         VAL_ifJackType_bnc
 *                         VAL_ifJackType_fAUI
 *                         VAL_ifJackType_mAUI
 *                         VAL_ifJackType_fiberSC
 *                         VAL_ifJackType_fiberMIC
 *                         VAL_ifJackType_fiberST
 *                         VAL_ifJackType_telco
 *                         VAL_ifJackType_mtrj
 *                         VAL_ifJackType_hssdc
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetJackType (UI32_T unit,
                                UI32_T port,
                                UI32_T mau_index,
                                UI32_T jack_index,
                                UI32_T *jack_type);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetMauMediaType
 * -------------------------------------------------------------------------
 * FUNCTION: Get media type of some MAU.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port.
 *           mau_index  -- Which MAU.
 * OUTPUT  : media_type -- STKTPLG_MGR_MEDIA_TYPE_E.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetMauMediaType (UI32_T unit, UI32_T port, UI32_T mau_index, UI32_T *media_type);
#endif /* (SYS_CPNT_MAU_MIB == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetPortMediaCapability
 * -------------------------------------------------------------------------
 * FUNCTION: Get media capability of port.
 * INPUT   : unit       -- Which unit.
 *           port       -- Which port. *
 * OUTPUT  : media_cap  -- STKTPLG_TYPE_PORT_MEDIA_CAP_E
 * RETURN  : TRUE/FALSE
 * NOTE    : media_cap is a bitmap of possible media capability. It's
 *           useful to identify the combo port
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetPortMediaCapability(UI32_T unit, UI32_T port, UI32_T* media_cap);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnit
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnit(UI32_T *unit_id);

/* FUNCTION NAME : STKTPLG_OM_GetPortMapping
 * PURPOSE: To get the maximum port number of the local unit.
 * INPUT:   None.
 * OUTPUT:  *max_port_number -- maximum port number of the local unit.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetPortMapping(DEV_SWDRV_Device_Port_Mapping_T *mapping, UI32_T unit);

/* FUNCTION NAME : STKTPLG_OM_IsTransition
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_IsTransition(void);

/* FUNCTION NAME : STKTPLG_OM_IsProvisionCompleted
 * PURPOSE  : To check if the system provision complete or not.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE.
 * Notes    : None.
 */
BOOL_T STKTPLG_OM_IsProvisionCompleted(void);

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBEntry
 * PURPOSE: To get the unit id of myself.
 * INPUT:   None.
 * OUTPUT:  my_unit_id   -- the unit id of myself.
 * RETUEN:  TRUE         -- master/slave.
 *          FALSE        -- transition state.
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetStackingDBEntry(STKTPLG_DataBase_Info_T *db, UI32_T entry);

/* FUNCTION NAME : STKTPLG_OM_GetStackingDBEntryByMac
 * PURPOSE: This api will return the unit id according to the given mac and set
 *          the specified device_type to the entry of the given mac.
 *
 * INPUT:   mac          -- the mac which is used to find the db entry
 *          device_type  -- board id of the found db entry. this id will be set
 *                          to the entry if it is found.
 * OUTPUT:  None.
 * RETUEN:  Non-Zero: unit id of the db entry with the given mac
 *          Zero: not found
 * NOTES:
 *
 */
UI32_T STKTPLG_OM_GetStackingDBEntryByMac(UI8_T mac[STKTPLG_MAC_ADDR_LEN],UI8_T device_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnitUp
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID Up
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnitUp(UI32_T *unit_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextUnitDown
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_id       -- Which Unit
 * OUTPUT  : unit_id       -- Next Unit ID down
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetNextUnitDown(UI32_T *unit_id);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitsRelPosition
 * -------------------------------------------------------------------------
 * FUNCTION: Get Next Unit ID.
 * INPUT   : unit_a       -- Which Unit
 * INPUT   : unit_b       -- Which Unit
 * OUTPUT  : position     -- Relative position from unit_a,
 *                           high-bit set if unit_b is in the Down direction
 *                           of unit_a.
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_GetUnitsRelPosition(UI32_T unit_a, UI32_T unit_b, UI32_T *position);

void STKTPLG_OM_ShowCFG(void);

/* FUNCTION NAME: STKTPLG_OM_ExpModuleIsInsert
 * PURPOSE: This function is used to check  slot-in module. insert or not
 * INPUT:   unit            -- unit id.
 * OUTPUT:  None.
 * RETUEN:  TRUE  -- insert and ready
 *          FALSE -- non-insert ,or un-ready
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_ExpModuleIsInsert(UI32_T unit_id);

void STKTPLG_OM_GetHiGiPortNum(UI32_T *port_num);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_OptionModuleIsExist
 *---------------------------------------------------------------------------
 * PURPOSE:  to  check the unit  insered the option module  or not
 * INPUT:    unit :  unit
 * OUTPUT:   ext_unit_id :option module  unit id
 * RETURN:   TRUE   : exist
 *           FALSE  : non-exist
 * NOTE:     1. this function is for a specify unit only  or  for each unit existing in the stacking
 *              it should check the unit insered the option module  or not
 *              if it has option module, the isc_remote_call or isc_send  also need send to the option module
 *          example:
 *                   isc_remote_call(unit, );
 *                   if(STKTPLG_OM_OptionModule_Exist(unit,&ext_unit_id)
 *                   {
 *                       isc_remote_call(unit+SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK,);
 *                   }
 *           2. now is only one option module
 *           STKTPLG_OM_OptionModuleIsExist(UI32_T unit, UI32_T  option_nums) int the feature
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_OptionModuleIsExist(UI32_T unit,UI32_T *ext_unit_id);

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_POM_IsLocalUnit
 *---------------------------------------------------------------------------
 * PURPOSE:  to  the port of  the specify unit  is belong to local or remote unit
 * INPUT:    unit :  destination unit
 *           port :
 * OUTPUT:   ext_unit_id  :  unit id
 * RETURN:   TRUE   : is local
 *           FALSE   : is remote
 * NOTE:     this function is for (unit,port)
 *           x*n+y :
 *           its meaning the option module x  of unit #y
 *           n: SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK
 *           when x=0 , y  is mainborad

 *
 *----------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_IsLocalUnit(UI32_T unit,UI32_T port,UI32_T *ext_unit_id);

/* FUNCTION NAME :
 * PURPOSE  :
 * INPUT    :
 * OUTPUT   :
 * RETUEN   :
 * Notes    :
 */
BOOL_T STKTPLG_OM_GetAllUnitsPortMapping(DEV_SWDRV_Device_Port_Mapping_T mapping[][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);


/* FUNCTION NAME : STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard
 * PURPOSE: To get the maximum port number on board .
 * INPUT:   unit             -- unit id.
 * OUTPUT:  *max_port_number -- maximum port number on board.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   0           -- get local device.
 *          otherwise   -- get this device.
 */
BOOL_T STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(UI8_T unit, UI32_T *max_option_port_number);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetModuleHiGiPortNum
 *---------------------------------------------------------------------------
 * PURPOSE:  To provide dev_nic HiGi port number for different modules
 * INPUT:    IUC_unit_id        ---IUC_unit_id
 * OUTPUT:   port_num           ---HiGi port number
 * RETURN:   None
 * NOTE:     Hard coded to retrieve module type for the first module
 *           in the array. Also assuming that every mainboard only
 *           have one module only.
 *----------------------------------------------------------------
 */
UI8_T STKTPLG_OM_GetModuleHiGiPortNum(UI16_T IUC_unit_id);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitModulePorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in module.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit or Module not present.
 *           FALSE --- Got information.
 * NOTE:
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetUnitModulePorts(UI32_T unit_id, UI32_T *start_port_id, UI32_T *nbr_of_ports);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitMainboardPorts
 *---------------------------------------------------------------------------
 * PURPOSE:  To get ports in mainboard.
 * INPUT:    Unit -- Which unit to get.
 * OUTPUT:   *start_port_id  -- starting port.
 *           *nbr_of_ports   -- port number.
 * RETURN:   TRUE  --- Unit not present.
 *           FALSE --- Got information.
 * NOTE:     Base on current synced module database.
 *------------------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetUnitMainboardPorts(UI32_T unit, UI32_T *start_port_id, UI32_T *nbr_of_ports);


/* FUNCTION NAME : STKTPLG_OM_IsOptionModule
 * PURPOSE  : STKTPLG to check if I am a module.
 * INPUT    : None.
 * OUTPUT   : None.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_IsOptionModule(void);

/* FUNCTION NAME : STKTPLG_OM_GetStackStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetStackStatus(BOOL_T *is_stack_status_normal);

/* FUNCTION NAME : STKTPLG_OM_GetUnitStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_stack_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetUnitStatus(UI32_T unit, BOOL_T *is_unit_status_normal);

/* FUNCTION NAME : STKTPLG_OM_GetModuleStatus
 * PURPOSE  : STKTPLG to notify tplg state for image version validation.
 * INPUT    : None.
 * OUTPUT     is_module_status_normal ---
 *              TRUE: Normal
 *              FALSE: Abnormal
 * RETUEN   : TRUE/FALSE
 * Notes    : 1) By spec, set abnormal state if version of "main boards" are different.
 *               else set as normal state.
 *            2) Base on 1), call this API only when topology is firmed.
 */
BOOL_T STKTPLG_OM_GetModuleStatus(UI32_T unit, UI32_T module, BOOL_T *is_module_status_normal);

/* FUNCTION NAME : STKTPLG_OM_GetModuleType
 * PURPOSE  : STKTPLG to get module type.
 * INPUT    : unit -- Which unit.
 *            module -- which module.
 * OUTPUT     module_type -- Type of module.
 * RETUEN   : TRUE/FALSE
 * Notes    : None.
 */
BOOL_T STKTPLG_OM_GetModuleType(UI32_T unit, UI32_T module, UI32_T *module_type);

/* FUNCTION NAME : STKTPLG_OM_IsValidDriverUnit
 * PURPOSE  : To know the driver is valid or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- Version is valid.
 *            FALSE -- For main board: Version is different from master main board.
 *                     For module: Version is dofferent from expected module version.
 */
BOOL_T STKTPLG_OM_IsValidDriverUnit(UI32_T driver_unit);

/* FUNCTION NAME : STKTPLG_OM_GetMyDriverUnit
 * PURPOSE  : To get my driver unit ID.
 * INPUT    : None.
 * OUTPUT   : *my_driver_unit -- my driver ID.
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_OM_GetMyDriverUnit(UI32_T *my_driver_unit);

/* FUNCTION NAME : STKTPLG_OM_DriverUnitExist
 * PURPOSE  : To know the driver exist or not.
 * INPUT    : driver_unit -- which driver want to know.
 * OUTPUT   : None
 * RETUEN   : TRUE  -- exist.
 *            FALSE -- not exist.
 */
BOOL_T STKTPLG_OM_DriverUnitExist(UI32_T driver_unit);

/* FUNCTION NAME : STKTPLG_OM_GetNextDriverUnit
 * PURPOSE  : To get next driver unit.
 * INPUT    : Next to which driver unit.
 * OUTPUT   : Next driver is which.
 * RETUEN   : TRUE/FALSE
 */
BOOL_T STKTPLG_OM_GetNextDriverUnit(UI32_T *driver_unit);

/* FUNCTION NAME: STKTPLG_OM_GetCurrentStackingDB
 * PURPOSE: This API is used to get the mac and unit id and device type
 * in the stacking
 * INPUT:   None.
 * OUTPUT:  stacking_db : array of structure.
 * RETUEN:  0  -- failure
 *          otherwise -- success. the returned value is the number of entries
 * NOTES:   None.
 */

UI8_T STKTPLG_OM_GetCurrentStackingDB(STKTPLG_OM_StackingDB_T stacking_db[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK]);

/* FUNCTION NAME: STKTPLG_OM_SfpIndexToUserPort
 * PURPOSE: This function is translate from SFP index to user port.
 * INPUT:   unit      -- which unit.
 *          sfp_index -- which SFP.
 * OUTPUT:  port -- which user port
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_SfpIndexToUserPort(UI32_T unit, UI32_T sfp_index, UI32_T *port);

/* FUNCTION NAME: STKTPLG_OM_UserPortToSfpIndexAndType
 * PURPOSE: This function is translate from user port to SFP index.
 *          and return sfp_type
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 *          sfp_type_p -- which SFP type.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UserPortToSfpIndexAndType(UI32_T unit, UI32_T port, UI32_T *sfp_index, UI8_T *sfp_type_p, BOOL_T *is_break_out);

/* FUNCTION NAME: STKTPLG_OM_UserPortToSfpIndex
 * PURPOSE: This function is translate from user port to SFP index.
 * INPUT:   unit -- which unit.
 *          port -- which user port
 * OUTPUT:  sfp_index -- which SFP.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKTPLG_OM_UserPortToSfpIndex(UI32_T unit, UI32_T port, UI32_T *sfp_index);

/* FUNCTION NAME : STKTPLG_OM_GetMainBoardPortNum
 * PURPOSE: To get the max port number on main board .(24/48)
 * INPUT:   NONE.
 * OUTPUT:  portnum -- max port number on main board
 * RETUEN:  NONE
 * NOTES:  This API is useless for mainboard.
 *
 */
void  STKTPLG_OM_GetMainBoardPortNum(UI32_T  *portnum);


/* FUNCTION NAME : STKTPLG_OM_GetMyModuleID
 * PURPOSE: To get my module type and assigned module ID.
 * INPUT:   NONE.
 * OUTPUT:  mymodid -- my assigned module ID
 *          mymodtype -- my module type
 * RETUEN:  NONE
 * NOTES:   This API is useless for mainboard.
 *
 */
void STKTPLG_OM_GetMyModuleID(UI8_T *mymodid,UI8_T *mymodtype);

/* FUNCTION NAME : STKTPLG_OM_GetLocalModulePortNumber
 * PURPOSE: To get the max port number of module conncted to me .
 * INPUT:   NONE.
 * OUTPUT:  portnum -- max port number of module
 * RETUEN:  FALSE : no module inserted
            TRUE  : otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetLocalModulePortNumber(UI32_T *portnum);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_UnitModuleToExtUnit
 *---------------------------------------------------------------------------
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTE:
 *----------------------------------------------------------------
 */
void STKTPLG_OM_UnitModuleToExtUnit( UI32_T unit,UI32_T module_x,UI32_T *ext_unit_id);

/* FUNCTION NAME : STKTPLG_OM_IsModulePort
 * PURPOSE: This function check if a port is a module port
 * INPUT:   unit:
 *          port:
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- is a module port
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_IsModulePort(UI32_T unit, UI32_T port);

/* FUNCTION NAME : STKTPLG_OM_PortList2DriverUnitList
 * PURPOSE: This function is to convert the port bitmap into driver unit bitmap
 * INPUT:   port_list: the port bitmap, size: SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * OUTPUT:  driver_unit_list: the driver unit bitmap
 * RETUEN:  NONE
 * NOTES:
 *
 */
void STKTPLG_OM_PortList2DriverUnitList(UI8_T *port_list, UI32_T *driver_unit_list);


#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)

/* FUNCTION NAME : STKTPLG_OM_GetMasterButtonStatus
 * PURPOSE: This function is to get the master button status setting for a specified unit
 * INPUT:   unit_id
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- master button status is enabled
 *          FALSE        -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetMasterButtonStatus(UI32_T unit_id);

#endif

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/* FUNCTION NAME : STKTPLG_OM_GetStackingButtonStatus
 * PURPOSE: This function is to get the stacking button status setting
 * INPUT:   None.
 * OUTPUT:  isPressed : stacking button pressed status
 *          isActived : stacking button active status
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKTPLG_OM_GetStackingButtonStatus(BOOL_T *isPressed, BOOL_T *isActived);
#endif

/* FUNCTION NAME : STKTPLG_OM_IsHBTTooOld
 * PURPOSE  : check if runime version of slave is too old to have different HBT2 format.
 * INPUT    : *runtime_sw_ver: runtime version
 * OUTPUT   : None
 * RETUEN   : TRUE: Old format of HBT2
 *            FALSE: Otherwise
 */
BOOL_T STKTPLG_OM_IsHBTTooOld(UI8_T *runtime_sw_vesion);

/* ken_chen: SCL_ENABLE_GBICx move to SYS_HWCFG_SCL_ENABLE_GBICx
 */

/* FUNCTION NAME : STKTPLG_OM_IsAllSlavesAndModlesWithMC
 * PURPOSE  : check if any version of slave or optional module is too old to have MC mechanism.
 * INPUT    : None.
 * OUTPUT   : None
 * RETUEN   : TRUE: Normal
 *            FALSE: Some slave or module is too old
 */
BOOL_T STKTPLG_OM_IsAllSlavesAndModulesWithMC(void);

/* FUNCTION NAME: STKTPLG_OM_Logical2PhyDevicePortID
 * PURPOSE: This function is used to convert logical address mode to
 *          physical address mode.
 * INPUT:   unit_id -- unit id of logical view
 *          port_id -- port id of logical view
 * OUTPUT:  phy_device_id   -- device id of physical view
 *          phy_port_id     -- port id of physical view
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_Logical2PhyDevicePortID(UI32_T unit_id,
                                          UI32_T port_id,
                                          UI32_T *phy_device_id,
                                          UI32_T *phy_port_id);

/* FUNCTION NAME: STKTPLG_OM_GetSwitchModuleInfoEntry
 * PURPOSE: This function is used to get this module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_OM_GetSwitchModuleInfoEntry(STKTPLG_OM_switchModuleInfoEntry_T *entry);

/* FUNCTION NAME: STKTPLG_OM_GetNextSwitchModuleInfoEntry
 * PURPOSE: This function is used to get_next module's information for SNMP.
 * INPUT:   entry->unit_index       -- unit id.
 * OUTPUT:  entry                   -- module info.
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   Give the unit = 0 to get the first unit information.
 *          This function is used by SNMP ModuleInfoTable.
 *
 */
BOOL_T STKTPLG_OM_GetNextSwitchModuleInfoEntry(STKTPLG_OM_switchModuleInfoEntry_T *entry);

/* FUNCTION NAME: STKTPLG_OM_GetMaxChipNum
 * PURPOSE: Get the switch chip number in this unit.
 * INPUT:   unit   -- unit number.
 * OUTPUT:  *max_chip_nmber  -- maximum chip number.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_OM_GetMaxChipNum(UI32_T unit, UI32_T *max_chip_nmber);

/* FUNCTION NAME: STKTPLG_OM_GetNeighborStackingPort
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(broadcom view)-- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by AMTR module.
 *
 */
BOOL_T STKTPLG_OM_GetNeighborStackingPort(UI32_T src_unit, UI32_T src_chip_id,
                                          UI32_T specific_unit, UI32_T specific_chip_id,
                                          UI32_T *port);

/* FUNCTION NAME: STKTPLG_OM_GetNeighborStackingPortByChipView
 * PURPOSE: Get the stacking port of this specific chip which close to the source chip device.
 * INPUT:   src_unit            -- source unit number.
 *          src_chip_id         -- source chip id.
 *          specific_unit       -- specific unit number.
 *          specific_chip_id    -- specific chip id.
 * OUTPUT:  *port(chip view)    -- stacking port which close to source unit/src_chip_id
 *                                 in specific_unit/specific_chip_id SOC chip
 *                                 (return chip physical port).
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   This function is used by HR module.
 *
 */
BOOL_T STKTPLG_OM_GetNeighborStackingPortByChipView(UI32_T src_unit, UI32_T src_chip_id,
                                                    UI32_T specific_unit, UI32_T specific_chip_id,
                                                    UI32_T *port);

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitBoardID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  board_id -- This is board ID, and used to be family serial number,
 *                      that is 5-bit field in project ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitBoardID(UI32_T *unit, UI32_T *board_id);

/* FUNCTION NAME: STKTPLG_OM_GetUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitFamilyID(UI32_T unit, UI32_T *family_id);

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitFamilyID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  family_id -- This is the family ID. and is 11-bit field in project
 *                       ID.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitFamilyID(UI32_T *unit, UI32_T *family_id);

/* FUNCTION NAME: STKTPLG_OM_GetUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- unit number, SYS_VAL_LOCAL_UNIT_ID for local unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:
 */
BOOL_T STKTPLG_OM_GetUnitProjectID(UI32_T unit, UI32_T *project_id);

/* FUNCTION NAME: STKTPLG_OM_GetNextUnitProjectID
 * PURPOSE: This function is used to get board ID
 * INPUT:   unit     -- Next to which unit.
 * OUTPUT:  project_id -- This is the project ID, that is 16-bit.
 * RETUEN:  TRUE/FALSE
 * NOTES:   SYS_VAL_LOCAL_UNIT_ID is not supported.
 */
BOOL_T STKTPLG_OM_GetNextUnitProjectID(UI32_T *unit, UI32_T *project_id);

/* FUNCTION NAME : STKTPLG_OM_GetStackingState
 * PURPOSE: To get the stacking state
 * INPUT:   None.
 * OUTPUT:  state        -- The stacking state
 * RETUEN:  TRUE         -- Sucess
 *          FALSE        -- Fail
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_GetStackingState(UI32_T *state);

/* FUNCTION NAME: STKTPLG_OM_IsStkTplgStateChanged
 * PURPOSE: This function is used to let other cpnt to know whether the state is changed in real time
 * INPUT:   stk_state == stktplg state
 *          STKTPLG_OM_SIMPLE_STATE_ARBITRATION = 0,
 *          STKTPLG_OM_SIMPLE_STATE_MASTER,
 *          STKTPLG_OM_SIMPLE_STATE_SLAVE
 * OUTPUT:  none
 * RETUEN:  TRUE :  changed
 *          FALSE : not changed
 * NOTES:
 *
 */
BOOL_T STKTPLG_OM_IsStkTplgStateChanged(STKTPLG_OM_Simple_State_T stk_state);

/* FUNCTION NAME : STKTPLG_OM_HandleIPCReqMsg
 * PURPOSE : This function is used to handle ipc request for stktplg om.
 * INPUT   : msg_p(acctually SYSFUN_Msg_T*)  --  the ipc request for stktplg_om
 * OUTPUT  : msg_p(acctually SYSFUN_Msg_T*)  --  the ipc response to send when return value is TRUE
 * RETUEN  : TRUE  --  A response is requred to send
 *           FALSE --  Need not to send response.
 * NOTES   :
 *          Type of msg_p is defined as void* intentionally.
 *          Reason:
 *          stktplg_tx_type.h is included by k_l_mm.c for dedicated buffer pool
 *          and stktplg_om.h is included by stktplg_tx_type.h for the data type definition
 *          k_l_mm.c is in kernel space so it need to include k_sysfun.h instead of sysfun.h
 *          stktplg_om.h will be included by k_l_mm.c and if stktplg_om.h include sysfun.h
 *          will lead to trouble. To avoid this issue, we define type of msg_p as void*
 *
 */
BOOL_T STKTPLG_OM_HandleIPCReqMsg(void *msg_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetNextSfpPortByUnitPort
 * -------------------------------------------------------------------------
 * FUNCTION: Get the unit/port of a SFP port which is next to the given
 *           unit/port
 * INPUT   : unit_p        -- Which unit
 *           port_p        -- Which port
 * OUTPUT  : unit_p        -- Corresponding unit of next SFP index
 *           port_p        -- Corresponding port of next SFP index
 * RETURN  : TRUE/FALSE
 * NOTE    :
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetNextSfpPortByUnitPort(UI32_T *unit_p, UI32_T *port_p);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
#define UNIT_PRESENT  1
#define UNIT_ABSENT   0

#define RENUMBER_PDU        0x03

/* FUNCTION NAME : STKTPLG_OM_TopologyIsStable
 * PURPOSE  : Check topo is stable.
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None.
 */
BOOL_T STKTPLG_OM_TopologyIsStable(void);

/* FUNCTION NAME: STKTPLG_OM_ProcessUnitInsertRemove
 * PURPOSE: Process one unit insertion/removal
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE  - No more units to be processed
 *          FASLE - not done;
 * NOTES:
 */
BOOL_T STKTPLG_OM_ProcessUnitInsertRemove(BOOL_T *is_insertion,
                                     UI32_T *starting_port_ifindex,
                                     UI32_T *number_of_ports,
                                     UI8_T * unit_id,
                                     BOOL_T *use_default);

/* FUNCTION NAME: STKTPLG_OM_IsAnyUnitNeedToProcess
 * PURPOSE: To check if any more units need to be processed
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:
 */
BOOL_T STKTPLG_OM_IsAnyUnitNeedToProcess(void);

/* FUNCTION NAME: STKTPLG_OM_SlaveSetUnitIsValidBits
 * PURPOSE: To set unit_is_valid bits according to current topology
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:
 */
void STKTPLG_OM_SlaveSetUnitIsValidBits(void);

void STKTPLG_OM_ClearSnapShotUnitEntry(UI32_T unit);

void STKTPLG_OM_CopyDatabaseToSnapShot(void);

BOOL_T *STKTPLG_OM_GetUnitIsValidArray(void);

BOOL_T *STKTPLG_OM_GetProvisionLostArray(void);

STKTPLG_DataBase_Info_T *STKTPLG_OM_GetPastStackingDB(void);

BOOL_T STKTPLG_OM_ENG_ProvisionCompletedOnce(void);

STKTPLG_OM_Ctrl_Info_T *STKTPLG_OM_ENG_GetCtrlInfo(void);

BOOL_T STKTPLG_OM_ENG_GetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info);

BOOL_T STKTPLG_OM_ENG_UnitExist(UI32_T unit_id);

BOOL_T STKTPLG_OM_ENG_SetMaxPortCapability(UI8_T unit, UI32_T max_port_number);

BOOL_T STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(UI8_T unit, UI32_T *max_port_number);

BOOL_T STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(UI8_T unit, UI32_T *max_option_port_number);

BOOL_T STKTPLG_OM_ENG_SetDeviceInfo(UI32_T unit, STK_UNIT_CFG_T *device_info);

void STKTPLG_OM_ENG_GetMyRuntimeFirmwareVer(UI8_T runtime_fw_ver_ar[SYS_ADPT_FW_VER_STR_LEN+1]);

UI8_T *STKTPLG_OM_ENG_GetUnitCfg(void);

BOOL_T STKTPLG_OM_ENG_IsValidDriverUnit(UI32_T driver_unit);
void print_GetUnitCfg(void);

/* FUNCTION NAME : STKTPLG_OM_DumpStackingDB
 * PURPOSE: Dump stackingdb and current_temp_stackingdb for information
 * INPUT:   None
 * OUTPUT:  None.
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_OM_DumpStackingDB(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsTempStackingDBEmpty
 * -------------------------------------------------------------------------
 * PURPOSE :  check if current_temp_stackingdb is empty
 * INPUT   :  None     
 * OUTPUT  :  None
 * RETURN  :  TRUE:  current_temp_stackingdb is empty
 *            FALSE: current_temp_stackingdb is NOT empty
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsTempStackingDBEmpty(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetUnitIdFromTempStackingDB
 * -------------------------------------------------------------------------
 * PURPOSE :  Get unit id from temp stacking db.
 * INPUT   :  mac          - the mac of theunit
              device_type  - the device type of the unit       
 * OUTPUT  :  None
 * RETURN  :  Non-Zero: An available unit id
 *            Zero: No availabe unit id
 * NOTE    :
 * -------------------------------------------------------------------------*/
UI32_T STKTPLG_OM_GetUnitIdFromTempStackingDB(
        UI8_T mac[STKTPLG_MAC_ADDR_LEN], UI8_T device_type);
        
void STKTPLG_OM_SetCurrentTempStackingDB(void);

#endif

/* FUNCTION NAME: STKTPLG_OM_GetDebugMode
 * PURPOSE: To get debug mode value from STKTPLG_OM
 * INPUT:
 * OUTPUT:  debug_mode  -  debug mode value
 * RETUEN:
 * NOTES:   BIT 0 -> 1: Show debug message 0: Do not show debug message
 */
void STKTPLG_OM_GetDebugMode(UI8_T *debug_mode);

/* FUNCTION NAME: STKTPLG_OM_SetDebugMode
 * PURPOSE: To set debug mode value in STKTPLG_OM
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES:   BIT 0 -> 1: Show debug message 0: Do not show debug message
 */
void STKTPLG_OM_SetDebugMode(UI8_T debug_mode);

void STKTPLG_OM_SlaveSetPastMasterMac(void);

BOOL_T STKTPLG_OM_GetTenGModuleState(UI32_T *is_exist);

BOOL_T STKTPLG_OM_SetMyUnitID(UI32_T my_unit_id);

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
BOOL_T STKTPLG_OM_GetStackingButtonState(BOOL_T *state);
BOOL_T STKTPLG_OM_IsStackingButtonChanged(void);
BOOL_T STKTPLG_OM_GetStackingPortInfo(UI32_T *is_stacking_enable,UI32_T *up_link_port,UI32_T *down_link_port  );

#endif

#if (SYS_CPNT_POE==TRUE)
/* FUNCTION: STKTPLG_OM_IsPoeDevice
 * PURPOSE:  For POE_MGR to check if some unit is POE device or not.
 * INPUT:    unit_id -- The unit that caller want to check.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- This unit is a POE device.
 *           FALSE -- This unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsPoeDevice(UI32_T unit_id);

/* FUNCTION: STKTPLG_OM_IsLocalPoeDevice
 * PURPOSE:  For POE_MGR to check if local unit is POE device or not.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- Local unit is a POE device.
 *           FALSE -- Local unit is not a POE device.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsLocalPoeDevice(void);
#endif

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT > 1)
/* FUNCTION: STKTPLG_OM_IsSupportRPU
 * PURPOSE:  For SYS_MGR to check if some unit has Redundant Power Unit.
 * INPUT:    unit_id -- The unit that caller want to check.
 * OUTPUT:   None.
 * RETURN:   TRUE  -- This unit supports RPU.
 *           FALSE -- This unit does not support RPU.
 * NOTES:    None.
 */
BOOL_T STKTPLG_OM_IsSupportRPU(UI32_T unit_id);
#endif

#ifdef ASF4526B_FLF_P5
BOOL_T STKTPLG_OM_DetectSfpInstall(UI32_T unit, UI32_T port, BOOL_T *is_present);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsComboPort
 * -------------------------------------------------------------------------
 * FUNCTION: To know whether the given port is combo port or not.
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port is combo port, return code is TRUE. Or return code is FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsComboPort(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_HasSfpPort
 * -------------------------------------------------------------------------
 * FUNCTION: This function returns TRUE if the given port is a sfp port
 *           or is a combo port which contains a sfp port.
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port is combo port, return code is TRUE.
 *           If this port is a sfp port, return code is TRUE.
 *           Other cases will return FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_HasSfpPort(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_IsPoEPDPort
 * -------------------------------------------------------------------------
 * FUNCTION: To know whether the given port supports PoE PD
 * INPUT   : unit -- which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : If this port supports PoE PD, return code is TRUE. Or return code is FALSE.
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T STKTPLG_OM_IsPoEPDPort(UI32_T unit, UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - STKTPLG_OM_GetModulePortListByModuleSlotIndex
 * -------------------------------------------------------------------------
 * FUNCTION: To get port list of the specified module slot
 * INPUT   : module_slot_index
 *           module_id
 * OUTPUT  : portlist - module port list
 * RETURN  : number of module ports
 * NOTE    : None
 * -------------------------------------------------------------------------*/
UI32_T STKTPLG_OM_GetModulePortListByModuleSlotIndex(UI8_T module_slot_index, UI8_T module_id, UI32_T *portlist);

/* FUNCTION NAME: STKTPLG_OM_GetEntPhysicalEntry
 * PURPOSE: Get the entPhysicalTable information.
 * INPUT:  *ent_physical_entry.ent_physical_index   -- entity physical index.
 * OUTPUT: *ent_physical_entry  -- information structure of this index.
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:  This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_OM_GetEntPhysicalEntry(STKTPLG_OM_EntPhysicalEntry_T *ent_physical_entry);

/* FUNCTION NAME: STKTPLG_OM_GetNextEntPhysicalEntry
 * PURPOSE: Get next the entPhysicalTable information.
 * INPUT:  *ent_physical_entry.ent_physical_index   -- entity physical index.
 * OUTPUT: *ent_physical_entry  -- information structure of this next index.
 * RETURN: TRUE  -- success
 *         FALSE -- failure
 * NOTES:  This function is used by SNMP.
 *
 */
BOOL_T STKTPLG_OM_GetNextEntPhysicalEntry(STKTPLG_OM_EntPhysicalEntry_T *ent_physical_entry);

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalAlias
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_alias_p -- entity physical index.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalAlias(I32_T ent_physical_index, UI8_T *ent_physical_alias_p);

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalSeriaNum
 * PURPOSE: Set the EntPhysicalSeriaNum field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_seriaNum_p -- entity physical serial num.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalSeriaNum(I32_T ent_physical_index, UI8_T *ent_physical_seriaNum_p);

/* FUNCTION NAME: STKTPLG_OM_SetEntPhysicalAssetId
 * PURPOSE: Set the entPhysicalAlias field.
 * INPUT:   ent_physical_index  -- entity physical index.
 *          *ent_physical_asset_id_p -- entity physical asset id.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_SetEntPhysicalAssetId(I32_T ent_physical_index, UI8_T *ent_physical_asset_id_p);

/* FUNCTION NAME: STKTPLG_OM_GetPortFigureType
 * PURPOSE: Get the port figure type of the specified unit and port.
 * INPUT:   unit -- unit id
 *          port -- port id
 * OUTPUT:  port_figure_type_p -- port figure type
 * RETURN:  TRUE  -- success
 *          FALSE -- failure
 */
BOOL_T STKTPLG_OM_GetPortFigureType(UI32_T unit, UI32_T port, STKTPLG_TYPE_Port_Figure_Type_T *port_figure_type_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetStackingPortPhyDevPortId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id and port id of the specified stacking port
 *          type.
 * INPUT:   port_type   - up link or down link stacking port
 * OUTPUT:  device_id_p - physical device id of the specified port
 *          phy_port_p  - physical port id of the specified port
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_TYPE_T port_type, UI32_T *device_id_p, UI32_T *phy_port_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetLocalCpuEnabledDevId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id of the local unit that CPU enabled.
 * INPUT:   None
 * OUTPUT:  device_id_p - physical device id of the local unit that CPU enabled.
 * RETUEN:  None
 * NOTES:   This function will not be used in Broadcom chip project.
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetLocalCpuEnabledDevId(UI8_T *device_id_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetCpuEnabledDevId
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the physical device id of the specified board id that 
 *          CPU enabled.
 * INPUT:   board_id    - board ID for this unit, it is project independent.
 * OUTPUT:  device_id_p - physical device id of the specified board 
                          id that CPU enabled
 * RETUEN:  None
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetCpuEnabledDevId(UI8_T board_id, UI8_T *device_id_p);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_SetStackingPortOption
 *---------------------------------------------------------------------------------
 * PURPOSE: Set the stacking port of user configuration to the specified option.
 * INPUT:   option - stacking port option 1, 2 or off.
 * OUTPUT:  None
 * RETUEN:  TRUE   - success.
 *          FALSE  - error
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetStackingPortOption(STKTPLG_TYPE_Stacking_Port_Option_T option);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: STKTPLG_OM_GetStackMaxUnitID
 *---------------------------------------------------------------------------------
 * PURPOSE: Get the max number of unit id existing in the stack.
 * INPUT:   None
 * OUTPUT:  max_unit_id_p - the max number of unit id
 * RETUEN:  TRUE   - success.
 *          FALSE  - error
 * NOTES:
 *---------------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetStackMaxUnitID(UI32_T *max_unit_id_p);

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_HwPortMode_UpdatePortMapping
 * -------------------------------------------------------------------------
 * PURPOSE : To sync port mapping from board info
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_HwPortMode_UpdatePortMapping(void);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetAllOperHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set all HW port mode oper status
 * INPUT   : unit
 *           oper_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetAllOperHwPortMode(UI32_T unit, UI8_T oper_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetAllCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To get all HW port mode config
 * INPUT   : unit
 * OUTPUT  : cfg_hw_port_mode_ar
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetAllCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetAllCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set all HW port mode config
 * INPUT   : unit
 *           cfg_hw_port_mode_ar
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetAllCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode config
 * INPUT   : unit
 *           port
 * OUTPUT  : hw_port_mode_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetCfgHwPortMode(UI32_T unit, UI32_T port, STKTPLG_TYPE_HwPortMode_T *hw_port_mode_p);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_SetCfgHwPortMode
 * -------------------------------------------------------------------------
 * PURPOSE : To set HW port mode config
 * INPUT   : unit
 *           port
 *           hw_port_mode
 * OUTPUT  : None
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_SetCfgHwPortMode(UI32_T unit, UI32_T port, STKTPLG_TYPE_HwPortMode_T hw_port_mode);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_OM_GetHwNextPortModeEntry
 * -------------------------------------------------------------------------
 * PURPOSE : To get HW port mode entry for UI
 * INPUT   : hw_port_mode_p->unit
 *           hw_port_mode_p->port_info
 * OUTPUT  : hw_port_mode_p
 * RETURN  : TRUE / FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T STKTPLG_OM_GetHwNextPortModeEntry(STKTPLG_TYPE_HwPortModeEntry_T *hw_port_mode_entry_p);
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

/* ken_chen: SCL_ENABLE_GBICx move to SYS_HWCFG_SCL_ENABLE_GBICx
 */

/* from other files, not mbus.h, from borad.h
 */
#define SCL_ENABLE_GBIC 0x0001
#define EUMBBAR_VAL     0x80500000
#define I2CADR_REG      0x3000

/* UP_PORT, DOWN_PORT and BOTH_PORTS are defined in lan_type.h now
 * #define UP_PORT             0x01
 * #define DOWN_PORT           0x02
 * #define BOTH_PORTS          0x03
 * #define EXPANSION_MODULE    0x04
 */

#define FIRST_EDGES_CHANGE  0x08

#define NORMAL_PDU          0x00
#define BOUNCE_PDU          0x01

#endif   /* STKTPLG_OM_H */
