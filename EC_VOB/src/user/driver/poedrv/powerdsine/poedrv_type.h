/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file declares the APIs for POEDRV OM to read/write the database.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created	
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2009
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

/* The Key field is used to define the type of message to be sent or 
 * the message received.
 */
#define POEDRV_TYPE_KEY_COMMAND                        0x00
#define POEDRV_TYPE_KEY_PROGRAM                        0x01
#define POEDRV_TYPE_KEY_REQUEST                        0x02
#define POEDRV_TYPE_KEY_TELEMETRY                      0x03
#define POEDRV_TYPE_KEY_REPORT                         0x52          /* ASCII 'R' */
#define POEDRV_TYPE_KEY_SOFTWARE_DOWNLOAD_TELEMETRY   0x54          /* ASCII 'T' */

/* The Main Subject field is used to define the type of data required by the host.
 */
#define POEDRV_TYPE_MAINSUBJECT_CHANNEL                0x05
#define POEDRV_TYPE_MAINSUBJECT_E2                     0x06
#define POEDRV_TYPE_MAINSUBJECT_GLOBAL                 0x07
#define POEDRV_TYPE_MAINSUBJECT_RESTORE_FACT           0x2D
#define POEDRV_TYPE_MAINSUBJECT_FLASH                  0xFF  

/* The Subject1 field is used together with Main Subject field to define the type
 * of data required by the host.
 */
#define POEDRV_TYPE_SUBJECT1_DETECT_TEST               0x3C
#define POEDRV_TYPE_SUBJECT1_LATCH                     0x3A
#define POEDRV_TYPE_SUBJECT1_MASKZ                     0x2B
#define POEDRV_TYPE_SUBJECT1_ON_OFF                    0x0C
#define POEDRV_TYPE_SUBJECT1_PARAMZ                    0x25
#define POEDRV_TYPE_SUBJECT1_PORTS_STATUS1             0x31
#define POEDRV_TYPE_SUBJECT1_PORTS_STATUS2             0x32
#define POEDRV_TYPE_SUBJECT1_PORTS_STATUS3             0x33
#define POEDRV_TYPE_SUBJECT1_PORTS_STATUS4             			0x47
#define POEDRV_TYPE_SUBJECT1_PORTS_STATUS5             			0x48
#define POEDRV_TYPE_SUBJECT1_PORT_STATUS               0x0E
#define POEDRV_TYPE_SUBJECT1_PRIORITY                  0x0A
#define POEDRV_TYPE_SUBJECT1_PRODUCTION_INFOZ          0x13
#define POEDRV_TYPE_SUBJECT1_RESET                     0x55
#define POEDRV_TYPE_SUBJECT1_SAVE_CONFIG               0x0F
#define POEDRV_TYPE_SUBJECT1_SUPPLY                    0x0B
#define POEDRV_TYPE_SUBJECT1_TEMP_SUPPLY               0xA2
#define POEDRV_TYPE_SUBJECT1_SYSTEM_STATUS             0x3D
#define POEDRV_TYPE_SUBJECT1_USER_BYTE                 0x41
#define POEDRV_TYPE_SUBJECT1_TEMPORARY                 0x42
#define POEDRV_TYPE_SUBJECT1_TEMPORARY_CHANNEL_MATRIX  0x43
#define POEDRV_TYPE_SUBJECT1_ACTIVE_CHANNEL_MATRIX     0x44
#define POEDRV_TYPE_SUBJECT1_PARAMETER                 0x4A
#define POEDRV_TYPE_SUBJECT1_VERSIONZ                  0x1E
#define POEDRV_TYPE_SUBJECT1_TMPset                    0x62

/* The Subject2 field is used together with Main Subject and Subject1 fields to 
 * define the type of data required by the host.
 */
#define POEDRV_TYPE_SUBJECT2_SOFTWARE_VERSION          0x21
#define POEDRV_TYPE_SUBJECT2_MAIN                      0x17
#define POEDRV_TYPE_SUBJECT2_MEASUREMENTZ              0x1A
#define POEDRV_TYPE_SUBJECT2_SUPPLY1                   0x15
#define POEDRV_TYPE_SUBJECT2_SUPPLY2                   0x16
#define POEDRV_TYPE_SUBJECT2_VERSION_30K               0x1F
#define POEDRV_TYPE_SUBJECT2_VERSION_30K_DATE          0x22
#define POEDRV_TYPE_SUBJECT2_ResEnable                 0x04
#define POEDRV_TYPE_SUBJECT2_CapEnable                 0x06
/* Space
 */
#define POEDRV_TYPE_SPACE                              0x4E          /* ASCII 'N' */
/*FOR new messages in Serial Communication Protocol*/
#define POEDRV_TYPE_ALL_CHANNEL						0x80
#define POEDRV_TYPE_SUBJECT2_BAT						0x16
#define POEDRV_TYPE_SUBJECT1_FORCE_POWER			0x51
#define POEDRV_TYPE_INDIVIDUAL_MASK					0x56
#define POEDRV_TYPE_PORT_FULL_INIT					0x4A
#define POEDRV_TYPE_SUBJECT1_PORTS_POWER1			0x4B
#define POEDRV_TYPE_SUBJECT1_PORTS_POWER2			0x4C
#define POEDRV_TYPE_SUBJECT1_PORTS_POWER3			0x4D
#define POEDRV_TYPE_SUBJECT1_PORTS_POWER4			0x4F
#define POEDRV_TYPE_SUBJECT1_PORTS_POWER5			0x50
#define POEDRV_TYPE_POWER_BUDGET						0x57
#define POEDRV_TYPE_POWER_LOSS						0x58
#define POEDRV_TYPE_POWER_MANAGEMODE                0x5F

/* Definitions of PSE port power classifications in milliwatts
 */
#define POEDRV_TYPE_MAX_POWER_CLASS_0                  21000       /* default:21 Watts for Ericsson */
#define POEDRV_TYPE_MAX_POWER_CLASS_1                  4000        /* 4.0 Watts                     */
#define POEDRV_TYPE_MAX_POWER_CLASS_2                  7000        /* 7.0 Watts                     */
#define POEDRV_TYPE_MAX_POWER_CLASS_3                  15400       /* 15.4 Watts                    */
#define POEDRV_TYPE_MAX_POWER_CLASS_4                  21000       /* reserved for future use       */

/*Default Power Bank in RON is 7*/
#define POEDRV_TYPE_POWER_BANK                         0x7
/*Delta Value is used in set power bank and set power source*/
#define POEDRV_TYPE_DELTA_VALUE                        0x13
/* Default Definitions of minimum and maximum shutdown voltage,
 * these are the voltages in decivolts allowed for normal operation.
 * Max voltage: 60V, Min voltage: 40.5V for Ericsson
 */
/*In RON Build Version above 0x0d, we use power bank to set mainpower.
  However, there are minimum value for max_volt(57) and min_volt(44) 
 */
#define POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_H             0x02         /* high-byte voltage */
#define POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_L             0x3A         /* low-byte voltage  */
#define POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_H             0x01         /* high-byte voltage */
#define POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_L             0xB8         /* low-byte voltage  */

/* Definitions of PSE port status
 */
#define POEDRV_TYPE_PORT_POWERED_CAP_DETECT                 0x0
#define POEDRV_TYPE_PORT_POWERED_RES_DETECT                 0x1
#define POEDRV_TYPE_PORT_OFF_VOLT_TOO_HIGH                  0x6
#define POEDRV_TYPE_PORT_OFF_VOLT_TOO_LOW                   0x7
#define POEDRV_TYPE_PORT_OFF_TEMPORARY_SHUT_DOWN            0x8
#define POEDRV_TYPE_PORT_OFF_PORT_NOT_ACTIVE                0xC
#define POEDRV_TYPE_PORT_OFF_POWERUP_NOT_COMPLETED          0x11
#define POEDRV_TYPE_PORT_ASIC_HARDWARE_FAULT                0x12
#define POEDRV_TYPE_PORT_OFF_USER_SETTING                   0x1A
#define POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED               0x1B
#define POEDRV_TYPE_PORT_OFF_NOT_PD                         0x1C
#define POEDRV_TYPE_PORT_OFF_OVL_UDL                        0x1D
#define POEDRV_TYPE_PORT_UNDERLOAD                          0x1E
#define POEDRV_TYPE_PORT_OVERLOAD                           0x1F
#define POEDRV_TYPE_PORT_OFF_POWER_MANAGEMENT               0x20
#define POEDRV_TYPE_PORT_HARDWARE_FAULT                     0x21
#define POEDRV_TYPE_PORT_OFF_VOLT_FEED                      0x24
#define POEDRV_TYPE_PORT_OFF_IMPROPER_CAP_DETECTION         0x25
#define POEDRV_TYPE_PORT_OFF_DISCHARGE_LOAD                 0x26
#define POEDRV_TYPE_PORT_OFF_FORCE_ON                       0x2B
#define POEDRV_TYPE_RESERVED_FOR_FUTURE_USE                 0x2C
#define POEDRV_TYPE_FORCEON_HIGH_VOLT                       0x2D
#define POEDRV_TYPE_FORCEON_LOW_VOLT                        0x2E
#define POEDRV_TYPE_FORCEON_DISABLE_PDU                     0x2F
#define POEDRV_TYPE_FORCEONOFF_STATUS_CHANGE                0x30
#define POEDRV_TYPE_PORT_OFF_FORCEON_OVERLOAD               0x31
#define POEDRV_TYPE_PORT_OFF_FORCEON_POWER_MANAGEMENT       0x32
#define POEDRV_TYPE_FORCEON_ASIC_COMMUNICATION_ERROR 	    0x33

/* Definitions of strings on software download to PoE controller
 */
#define POEDRV_TYPE_SW_ENTER                           "ENTR"
#define POEDRV_TYPE_SW_BOOT_RESPONSE                   "TPE\r\n"
#define POEDRV_TYPE_SW_ERASE                           "E"
#define POEDRV_TYPE_SW_ERASE_RESPONSE                  "TOE\r\n"
#define POEDRV_TYPE_SW_ERASE_COMPLETED                 "TE\r\nTPE\r\n"
#define POEDRV_TYPE_SW_ERASE_COMPLETED_2               "TPE\r\n"
#define POEDRV_TYPE_SW_PROGRAM                         "P"
#define POEDRV_TYPE_SW_PROGRAM_RESPONSE                "TOP\r\n"
#define POEDRV_TYPE_SW_SEND_LINE_RESPONSE              "T*\r\n"
#define POEDRV_TYPE_SW_END_OF_FILE                     "TP\r\n"
#define POEDRV_TYPE_SW_SEND_RESET                      "RST"

/* Maximum size of firmware for PoE controller
 */
#define POEDRV_TYPE_SIZE_OF_POE_SOFTWARE               (200 * SYS_TYPE_1K)

#define POEDRV_TYPE_PSE_MIN_PORT_NUMBER               SYS_ADPT_POE_PSE_MIN_PORT_NUMBER
#define POEDRV_TYPE_PSE_MAX_PORT_NUMBER               SYS_ADPT_POE_PSE_MAX_PORT_NUMBER
#define POEDRV_NO_OF_POE_PORTS                        SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT
#define POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_RPS      SYS_HWCFG_MAX_POWER_ALLOCATION_RPS
#define POEDRV_TYPE_PSE_POWER_MAX_ALLOCATION_LOCAL    SYS_HWCFG_MAX_POWER_ALLOCATION_LOCAL
#define POEDRV_TYPE_PSE_POWER_MIN_ALLOCATION          SYS_HWCFG_MIN_POWER_ALLOCATION
#define POEDRV_TYPE_PSE_POWER_GUARD_BAND              SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION

#define POEDRV_TYPE_SYSTEM_NORMAL                      1
#define POEDRV_TYPE_SYSTEM_OVERLOAD                    2
#define POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD    95 /* percentage */

/* State definitions for PoE operation
 */
enum POEDRV_OPERATION_STATE_E
{
    POEDRV_NO_OPERATION_STATE = 0,                     /* no operation state     */
    POEDRV_NORMAL_OPERATION_STATE,                     /* normal operation state */
    POEDRV_FAULT_RECOVERY_STATE,                       /* fault recovery state   */
    POEDRV_SYSTEM_BOOTING_STATE                        /* system booting state   */
};

/*
 */
enum POEDRV_REPORT_STATUS_E
{
    POEDRV_REPORT_UNKNOWN_MESSAGE = 0,          /* unknown report message                      */
    POEDRV_REPORT_CORRECTLY_EXECUTED,           /* command received/correctly executed         */
    POEDRV_REPORT_WRONG_CHECKSUM,               /* command received/wrong checksum             */
    POEDRV_REPORT_CONFLICT_IN_SUBJECT_BYTES,    /* failed execution/conflict in subject bytes  */
    POEDRV_REPORT_WRONG_DATA_BYTE_VALUE,        /* failed execution/wrong data byte value      */
    POEDRV_REPORT_UNDEFINED_KEY_VALUE           /* failed execution/undefined key value        */

};

enum POEDRV_PORT_STATUS_E
{
    POEDRV_PORT_IS_OFF = 0,                     /* initial status                              */
    POEDRV_PORT_IS_ON,                          /* port is powered normally                    */
    POEDRV_PORT_IS_OFF_OVERLOAD,                /* port is off due to overload                 */
    POEDRV_PORT_IS_OFF_UNDERLOAD,               /* port is off due to underload                */
    POEDRV_PORT_IS_OFF_NO_CONNECT,              /* port is off due to no connection            */
    POEDRV_PORT_IS_OFF_NON_PD,                  /* port is off due to connection to non-PD     */
};

/* DATA TYPE DECLARATIONS
 */



/* Definitions of 15-byte packet message structure for communications between 
 * Host and PoE controller. Communication parameters are transferred in big-endian 
 * format.
 */
typedef struct POEDRV_TYPE_PktBuf_S   
{
    UI8_T      key;                               /* operation code              */
    UI8_T      echo_no;                           /* echo(sequence) number       */
    UI8_T      main_subject;                      /* main subject: type of data  */
    UI8_T      subject1;                          /* subject 1: type of data     */
    UI8_T      subject2;                          /* subject 2 or DATA5 field    */
    UI8_T      data6;                             /* DATA6 field                 */
    UI8_T      data7;                             /* DATA7 field                 */
    UI8_T      data8;                             /* DATA8 field                 */
    UI8_T      data9;                             /* DATA9 field                 */
    UI8_T      data10;                            /* DATA10 field                */
    UI8_T      data11;                            /* DATA11 field                */
    UI8_T      data12;                            /* DATA12 field                */
    UI8_T      data13;                            /* DATA13 field                */
    UI8_T      checksum_H;                        /* checksum: high byte         */
    UI8_T      checksum_L;                        /* checksum: low byte          */

}__attribute__((packed, aligned(1))) POEDRV_TYPE_PktBuf_T; 

typedef struct POEDRV_Mainpower_Info_S
{
    UI8_T     unit_id;                 /* Unit ID                     */
    UI32_T    board_id;                /* Board ID                    */
    UI8_T     main_pse_oper_status;    /* main PSE operational status */
    UI32_T    main_pse_power;          /* Main PSE power              */
    UI32_T    main_pse_consumption;    /* main PSE consumption        */
    UI8_T     legacy_detection_enable; /* Legacy_Detection*/

}__attribute__((packed, aligned(1))) POEDRV_Mainpower_Info_T;

/* Database for power status by PSE port
 */
typedef struct POEDRV_Port_Info_S
{
    UI8_T      detection_status;     /* port detection status     */
    UI8_T      admin_status;                    /* port admin status         */
    UI8_T      poe_linkup;                              /* PD is detected            */
    UI8_T      poe_active;                      /* PD is drawing power       */
    UI8_T      actual_status;        /* actual port status        */
    UI8_T      power_class;          /* port power classification */
    BOOL_T     is_overload;          /* port overload status      */
    UI32_T     power_consumption;    /* port power consumption    */
    BOOL_T     is_port_failure;      /* port failure status      */

}__attribute__((packed, aligned(1))) POEDRV_Port_Info_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM

    POEDRV_Mainpower_Info_T     poedrv_mainpower_info;
    POEDRV_Port_Info_T          poedrv_port_info[POEDRV_NO_OF_POE_PORTS];
    UI8_T                       poedrv_port_state[POEDRV_NO_OF_POE_PORTS];

    UI32_T                      poedrv_my_unit_id;        /* Local unit ID              */
    UI32_T                      poedrv_num_of_units;      /* numer of POE  switch       */
    BOOL_T                      is_stop_polling;          /* Stop polling controller    */
/* Get board id from stktplg and initializes POE database by POEDRV_InitDataBase()
   the following static variables have the same definition to POE database*/
/* Logical port to physical port mapping matrix,Logical ports are count from 1
 */
    UI32_T  poedrv_port_L2P_matrix[SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];
/* Physical port to logical port mapping matrix,Logical ports are count from 0
 */
    UI32_T  poedrv_port_P2L_matrix[SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT];
/* Max power allocation of logical ports
 */
    UI32_T  per_port_power_max_allocation[POEDRV_NO_OF_POE_PORTS];

/* Max power allocation of POE system
*/
    UI32_T  main_pse_power_max_allocation;
    UI32_T  poedrv_min_port_number;
    UI32_T  poedrv_max_port_number;
    UI8_T   poedrv_poe_image_version_1;
    UI8_T   poedrv_poe_image_version_2;
    UI8_T   poedrv_poe_image_build;

    BOOL_T poedrv_provision_complete;
    BOOL_T  poedrv_hw_enable;

}POEDRV_TYPE_ShmemData_T;


#endif  /* POEDRV_TYPE_H */

