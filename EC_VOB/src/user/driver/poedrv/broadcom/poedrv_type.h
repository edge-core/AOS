/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    The definitions of common data structure for PoE driver, including
 *    command types, report types, common structure and packet format.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

 
#ifndef POEDRV_TYPE_H
#define POEDRV_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sysfun.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define POEDRV_TYPE_DATA_ACK                           0x0

#define POEDRV_MPSABSENT_COUNTER 0
#define POEDRV_INVALID_SIGNATURE_COUNTER 1
#define POEDRV_POWER_DENIED_COUNTER 2
#define POEDRV_OVERLOAD_COUNTER 3
#define POEDRV_SHORT_COUNTER 4
#define POEDRV_MAX_COUNTER_TYPE 5


/* For LED */
#define POEDRV_TYPE_PORT_LINKOFF 0 /* PD disconnected */
#define POEDRV_TYPE_PORT_LINKON 1 /* PD connected */
#define POEDRV_TYPE_SYSTEM_NORMAL 1
#define POEDRV_TYPE_SYSTEM_OVEROAD 2

/* BCM59101 Command value
 */
#define POEDRV_TYPE_CMD_PORT_PSE_SWITCH                0x0
#define POEDRV_TYPE_CMD_PORT_POWER_UP_ENABLE           0x1
#define POEDRV_TYPE_CMD_PORT_MAPPING_ENABLE            0x2
#define POEDRV_TYPE_CMD_PORT_RESET                     0x3
#define POEDRV_TYPE_CMD_RAPID_POWER_DOWN               0x4
#define POEDRV_TYPE_CMD_PORT_STATISTIC_RESET           0x5
#define POEDRV_TYPE_CMD_ICUT_IN_HIGH_POWER_MODE_SET    0x7
#define POEDRV_TYPE_CMD_HIGH_POWER_FOR_DLL_SET         0x8
#define POEDRV_TYPE_CMD_PORT_DETECTION_SET             0x10
#define POEDRV_TYPE_CMD_CLASSIFICATION_TYPE_SET        0x11
#define POEDRV_TYPE_CMD_PORT_AUTO_MODE_ENABLE          0x12
#define POEDRV_TYPE_CMD_DISCONNECT_TYPE_SET            0x13
#define POEDRV_TYPE_CMD_INTERRUPT_MASK_SET             0x14 /* For manual mode only */
#define POEDRV_TYPE_CMD_PORT_THRESHOLD_TYPE_SET        0x15
#define POEDRV_TYPE_CMD_PORT_POWER_LIMIT_SET           0x16
#define POEDRV_TYPE_CMD_POWER_MANAGE_MODE_SET          0x17
#define POEDRV_TYPE_CMD_SYSTEM_POWER_SOURCE_SET        0x18
#define POEDRV_TYPE_CMD_PORT_POWER_PAIR_SET            0x19
#define POEDRV_TYPE_CMD_PORT_PRIORITY_SET              0x1A
#define POEDRV_TYPE_CMD_PORT_POWER_MODE_SET            0x1C
#define POEDRV_TYPE_CMD_PORT_MAPPING_SET               0x1D
#define POEDRV_TYPE_CMD_SYSTEM_STATUS_GET              0x20
#define POEDRV_TYPE_CMD_PORT_STATUS_GET                0x21
#define POEDRV_TYPE_CMD_PORT_STATISTIC_GET             0x22
#define POEDRV_TYPE_CMD_TOTAL_POWER_ALLOCATED_GET      0x23
#define POEDRV_TYPE_CMD_INTERRUPT_STATUS_GET           0x24 /* For manual mode only */
#define POEDRV_TYPE_CMD_PORT_CONFIG_GET                0x25
#define POEDRV_TYPE_CMD_PORT_EXT_CONFIG_GET            0x26
#define POEDRV_TYPE_CMD_POWER_MANAGE_MODE_GET          0x27
#define POEDRV_TYPE_CMD_PORT_MEASURE_GET               0x30

#define POEDRV_TYPE_CMD_IMAGE_UPGRADE                  0xE0
#define POEDRV_TYPE_SUBCOM_CHECK_CRC                   0x40
#define POEDRV_TYPE_SUBCOM_DOWNLOAD_IMAGE              0x80
#define POEDRV_TYPE_SUBCOM_CLEAR_IMAGE                 0xC0
#define POEDRV_TYPE_SUBCOM_SAVE_IMAGE                  0xD0
#define POEDRV_TYPE_SUBCOM_CLEAR_CONFIGURATION          0xE0
#define POEDRV_TYPE_SUBCOM_SAVE_CONFIGURATION           0xF0



#define POEDRV_TYPE_PSE_MIN_PORT_NUMBER               SYS_ADPT_POE_PSE_MIN_PORT_NUMBER
#define POEDRV_TYPE_PSE_MAX_PORT_NUMBER               SYS_ADPT_POE_PSE_MAX_PORT_NUMBER
#define POEDRV_NO_OF_POE_PORTS                        SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT
#define POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_RPS      SYS_HWCFG_MAX_POWER_ALLOCATION_RPS
#define POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_LOCAL    SYS_HWCFG_MAX_POWER_ALLOCATION_LOCAL
#define POEDRV_TYPE_PSE_POWER_MIN_ALLOCATION          SYS_HWCFG_MIN_POWER_ALLOCATION


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T admin_status;
    UI32_T power_pair;
    UI32_T power_priority;
    UI32_T power_limit;
    UI32_T detection_type;
    UI32_T power_mode;
}POEDRV_PORT_CONFIG_T;

typedef struct POEDRV_Mainpower_Info_S
{
    UI8_T     unit_id;                 /* Unit ID                     */
    UI8_T     main_pse_oper_status;    /* main PSE operational status */
    UI8_T     legacy_detection_enable; /* Legacy_Detection */
    UI32_T    main_pse_power;          /* Main PSE power              */
    UI32_T    main_pse_consumption;    /* main PSE consumption        */
}__attribute__((packed, aligned(1))) POEDRV_Mainpower_Info_T;

/* Database for power status by PSE port
 */
typedef struct POEDRV_Port_Info_S
{
    UI32_T     power_consumption;    /* port power consumption    */
    I32_T      temperature;          /* port temperature    */
    I32_T      current;              /* port used current    */
    I32_T      voltage;              /* port voltage    */
    UI8_T      detection_status;     /* port detection status     */
    UI8_T      led_status;            /* For LED */
    UI8_T      power_class;          /* port power classification */
    BOOL_T     is_overload;          /* port overload status      */
}__attribute__((packed, aligned(1))) POEDRV_Port_Info_T;

/* ping-pong port issue */
typedef struct 
{
    UI32_T times_of_power_denied;
    UI32_T start_ticks;
}POEDRV_PINGPONG_DETECTION_T;

enum POEDRV_PORT_DETECTION_E
{
    POEDRV_PORT_DETECTION_NONE = 0,
    POEDRV_PORT_DETECTION_LEGACY,
    POEDRV_PORT_DETECTION_DOT3AF_4POINT,
    POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY,
    POEDRV_PORT_DETECTION_DOT3AF_2POINT,
    POEDRV_PORT_DETECTION_DOT3AF_2POINT_FOLLOWED_BY_LEGACY
};

enum POEDRV_PORT_CLASS_TYPE_E
{
    POEDRV_PORT_CLASSIFICATION_NONE = 0,
    POEDRV_PORT_CLASSIFICATION_DOT3AF
};

enum POEDRV_PORT_POWER_THRESHOLD_TYPE_E
{
    POEDRV_PORT_POWER_THRESHOLD_NONE = 0,
    POEDRV_PORT_POWER_THRESHOLD_CLASS_BASE,
    POEDRV_PORT_POWER_THRESHOLD_USER_DEFINE
};

enum POEDRV_POWER_MANAGE_E
{
    POEDRV_POWER_MANAGE_NONE = 0,
    POEDRV_POWER_MANAGE_STATIC,
    POEDRV_POWER_MANAGE_DYNAMIC
};

enum POEDRV_PORT_PRIORITY_E
{
    POEDRV_PORT_PRIORITY_LOW = 0,
    POEDRV_PORT_PRIORITY_MEDIUM,
    POEDRV_PORT_PRIORITY_HIGH,
    POEDRV_PORT_PRIORITY_CRITICAL
};

enum POEDRV_PORT_STATUS1_E
{
    POEDRV_PORT_STATUS1_DISABLE = 0,
    POEDRV_PORT_STATUS1_SEARCHING,
    POEDRV_PORT_STATUS1_DELIVER_POWER,
    POEDRV_PORT_STATUS1_TEST,
    POEDRV_PORT_STATUS1_FAULT,
    POEDRV_PORT_STATUS1_OTHER_FAULT,
    POEDRV_PORT_STATUS1_REQUEST_POWER
};

enum POEDRV_PORT_STATUS2_ERROR_TYPE_E
{
    POEDRV_PORT_STATUS2_ERROR_NONE = 0,
    POEDRV_PORT_STATUS2_ERROR_MPS_ABSENT,
    POEDRV_PORT_STATUS2_ERROR_SHORT,
    POEDRV_PORT_STATUS2_ERROR_OVERLOAD,
    POEDRV_PORT_STATUS2_ERROR_POWER_DENIED
};

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM

    POEDRV_Mainpower_Info_T     poedrv_mainpower_info;
    POEDRV_Port_Info_T          poedrv_port_info[POEDRV_NO_OF_POE_PORTS];
    UI32_T                      poedrv_port_counter[POEDRV_NO_OF_POE_PORTS][POEDRV_MAX_COUNTER_TYPE];
    POEDRV_PINGPONG_DETECTION_T poedrv_pingpong_info[POEDRV_NO_OF_POE_PORTS];

    UI32_T                      poedrv_thread_id;         /* PoE driver task ID         */
    UI32_T                      poedrv_my_unit_id;        /* Local unit ID              */
    UI32_T                      poedrv_num_of_units;      /* numer of POE  switch       */
    BOOL_T                      is_stop_polling;          /* Stop polling controller    */
/* Get board id from stktplg and initializes POE database by POEDRV_InitDataBase()
   the following static variables have the same definition to POE database*/
/* Logical port to physical port mapping matrix,Logical ports are count from 1
 */
    UI32_T  poedrv_port_L2P_matrix[POEDRV_NO_OF_POE_PORTS];

/* Physical port to logical port mapping matrix,Logical ports are count from 0
 */
    UI32_T  poedrv_port_P2L_matrix[POEDRV_NO_OF_POE_PORTS];
/* Max power allocation of logical ports
 */
    UI32_T  per_port_power_max_allocation[POEDRV_NO_OF_POE_PORTS];

    UI32_T main_pse_power_max_allocation; /* Max power allocation of POE system */
    UI32_T poedrv_min_port_number;
    UI32_T poedrv_max_port_number;
    UI8_T  poedrv_poe_image_version;



    BOOL_T poedrv_provision_complete;

}POEDRV_TYPE_ShmemData_T;

/* Definitions of 12-byte packet message structure for communications between
 * Host and PoE controller.
 */
typedef struct POEDRV_BCM_TYPE_PktBuf_S
{
    UI8_T      command;                           /* operation code              */
    UI8_T      seq_no;                            /* sequence number or sub command */
    UI8_T      data1;                             /* DATA1 field                 */
    UI8_T      data2;                             /* DATA2 field                 */
    UI8_T      data3;                             /* DATA3 field                 */
    UI8_T      data4;                             /* DATA4 field                 */
    UI8_T      data5;                             /* DATA5 field                 */
    UI8_T      data6;                             /* DATA6 field                 */
    UI8_T      data7;                             /* DATA7 field                 */
    UI8_T      data8;                             /* DATA8 field                 */
    UI8_T      data9;                             /* DATA9 field                 */
    UI8_T      checksum;                          /* checksum                    */
}__attribute__((packed, aligned(1))) POEDRV_BCM_TYPE_PktBuf_T;

#endif  /* POEDRV_TYPE_H */

