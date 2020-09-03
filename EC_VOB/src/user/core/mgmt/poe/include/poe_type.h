/*-----------------------------------------------------------------------------
 * FILE NAME: poe_type.h
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The definitions of common data structure for PoE driver, including 
 *    command types, report types, and packet format. 
 *
 * NOTES:
 *    None.
 *
 * History:                                                               
 *    03/31/2003 - Benson Hsu, Created	
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */


#ifndef POE_TYPE_H
#define POE_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "leaf_3621.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define POE_TYPE_MANAGED_OBJECT_CLASS                              SYS_ADPT_SYS_DESC_STR

/* Perceived Severity
 */
#define POE_TYPE_PERCEIVED_SEVERITY_CLEARED                        1
#define POE_TYPE_PERCEIVED_SEVERITY_INDETERMINATE                  2
#define POE_TYPE_PERCEIVED_SEVERITY_CRITICAL                       3
#define POE_TYPE_PERCEIVED_SEVERITY_MAJOR                          4
#define POE_TYPE_PERCEIVED_SEVERITY_MINOR                          5
#define POE_TYPE_PERCEIVED_SEVERITY_WARNING                        6

/* POE gerneral return value */
#define POE_TYPE_RETURN_ERROR                              0
#define POE_TYPE_RETURN_OK                                 1
#define POE_TYPE_RETURN_MASTER_MODE_ERROR                  2
#define POE_TYPE_RETURN_TTL_EXCEED                         3
#define POE_TYPE_TOO_MANY_NEIGHBOR                         4
#define POE_TYPE_RETURN_TX_DELAY_ERROR                     5

/* Event Type
 */
#define POE_TYPE_EVENT_TYPE_OTHER                                  1
#define POE_TYPE_EVENT_TYPE_COMMUNICATIONS                         2
#define POE_TYPE_EVENT_TYPE_QUALITY_OF_SERVICE                     3
#define POE_TYPE_EVENT_TYPE_PROCESSING_ERROR                       4
#define POE_TYPE_EVENT_TYPE_EQUIPMENT                              5
#define POE_TYPE_EVENT_TYPE_ENVIRONMENT                            6
#define POE_TYPE_EVENT_TYPE_INTEGRITY_VIOLATION                    7
#define POE_TYPE_EVENT_TYPE_PHYSICAL_VIOLATION                     8
#define POE_TYPE_EVENT_TYPE_OPERATIONAL_VIOLATION                  9
#define POE_TYPE_EVENT_TYPE_SECURITY_VIOLATION                     10
#define POE_TYPE_EVENT_TYPE_TIME_DOMAIN_VIOLATION                  11

/* Probable Cause
 */
#define POE_TYPE_PROBABLE_CAUSE_OTHER                              255
#define POE_TYPE_PROBABLE_CAUSE_COOLING_FAN_FAILURE                107
#define POE_TYPE_PROBABLE_CAUSE_TEMPERATURE_UNACCEPTABLE           284
#define POE_TYPE_PROBABLE_CAUSE_THRESHOLD_CROSSED                  285

/* Define for time range poe
 */
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
#define POE_TYPE_TIMERANGE_STATUS_NONE                             0
#define POE_TYPE_TIMERANGE_STATUS_ACTIVE                           1
#define POE_TYPE_TIMERANGE_STATUS_INACTIVE                         2
#endif

/* Daniel Chen 2007/12/14
 */
#define POE_TYPE_EVENT_NONE                                 0x0
#define POE_TYPE_EVENT_ENTER_TRANSITION                     0x1
#define POE_TYPE_EVENT_TIMER                                0x2
#define POE_TYPE_EVENT_PORT_POE_STATE_CHANGE                0x4
#define POE_TYPE_EVENT_LLDP_RECVD                           0x8
#define POE_TYPE_EVENT_FORCE_HIGH_POWER                     0x10


#define POE_TYPE_EVENT_ALL                                  0x3F

/* define */
#define     PSE_PORT_ADMIN_DEF                          SYS_DFLT_PSE_PORT_ADMIN
#define     PSE_PORT_POWER_PAIRS_CONTROL_ABILITY_DEF    SYS_DFLT_PSE_PORT_POWER_PAIRS_CONTROL_ABILITY
#define     PSE_PORT_POWER_PAIRS_DEF                    SYS_DFLT_PSE_PORT_POWER_PAIRS
#if 0
#define     PSE_PORT_POWER_DETECTION_CONTROL_DEF        SYS_DFLT_PSE_PORT_POWER_DETECTION_CONTROL
#endif 
#define     PSE_PORT_DETECTION_STATUS_DEF               SYS_DFLT_PSE_PORT_DETECTION_STATUS
#define     PSE_PORT_POWER_PRIORITY_DEF                 SYS_DFLT_PSE_PORT_POWER_PRIORITY
#if 0
#define     PSE_PORT_POWER_MAINTENANCE_STATUS_DEF       SYS_DFLT_PSE_PORT_POWER_MAINTENANCE_STATUS
#endif
#define     PSE_PORT_MPSABSENT_COUNT_DEF                SYS_DFLT_PSE_PORT_MPSABSENT_COUNT
#define     PSE_PORT_OVERCURRENT_COUNT_DEF              SYS_DFLT_PSE_PORT_OVERCURRENT_COUNT
#define     PSE_PORT_TYPE_DEF                           SYS_DFLT_PSE_PORT_TYPE
#define     PSE_PORT_POWER_CLASSIFICATION_DEF           SYS_DFLT_PSE_PORT_POWER_CLASSIFICATION
#define     PSE_PORT_POWER_MAX_ALLOCATION               SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION
#ifdef SYS_CPNT_POE_PSE_DOT3AT
#define     PSE_PORT_POWER_HIGHPOWER_MODE_DEF           SYS_DFLT_PSE_PORT_POWER_HIGHPOWER_MODE
#endif

#define     MAIN_PSE_POWER_DEF                          SYS_DFLT_MAIN_PSE_POWER
#define     MAIN_PSE_OPER_STATUS_DEF                    SYS_DFLT_MAIN_PSE_OPER_STATUS        /* off */
#define     MAIN_PSE_CONSUMPTION_POWER_DEF              SYS_DFLT_MAIN_PSE_CONSUMPTION_POWER
#define     MAIN_PSE_USAGE_THRESHOLD_DEF                SYS_DFLT_MAIN_PSE_USAGE_THRESHOLD
#define     MAIN_PSE_POWER_MAX_ALLOCATION               SYS_DFLT_MAIN_PSE_POWER_MAX_ALLOCATION

#define     NOTIFICATION_CONTROL_DEF                    SYS_DFLT_NOTIFICATION_CONTROL 

/* DATA TYPE DECLARATIONS
 */


/* Data structure on Event Time
 */
typedef struct POE_TYPE_Time_S
{
    UI16_T    year;        /* 0~65536 */
    UI8_T     month;       /* 1~12    */
    UI8_T     day;         /* 1~31    */
    UI8_T     hour;        /* 0~23    */
    UI8_T     minute;      /* 0~59    */
    UI8_T     second;      /* 0~59    */
    UI8_T     deci_second; /* 0~9     */
}__attribute__((packed, aligned(1))) POE_TYPE_Time_T;

typedef struct POE_OM_PsePort
{
   UI32_T   pse_port_admin_enable;
   UI32_T   pse_port_power_pairs_ctrl_ability;
   UI32_T   pse_port_power_pairs;
   UI32_T   pse_port_power_detection_ctrl;
   UI32_T   pse_port_detection_status;
   UI32_T   pse_port_power_priority;
   UI32_T   pse_port_power_maintenance_status;
   UI32_T   pse_port_mpsabsent_counter;
   UI32_T   pse_port_over_curr_counter;
   UI8_T    pse_port_type[MAXSIZE_pethPsePortType+1];
   UI32_T   pse_port_power_classifications;
   UI32_T   pse_port_power_max_allocation;
   UI32_T   pse_port_consumption_power;
#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
   UI32_T   pse_port_invalid_signature_counter;
   UI32_T   pse_port_power_denied_counter;
   UI32_T   pse_port_overload_counter;
   UI32_T   pse_port_short_counter;
#endif
#ifdef SYS_CPNT_POE_PSE_DOT3AT
    UI32_T   pse_port_force_power_mode; /* 1: force high power, 0: normal mode */
#endif
#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
    UI8_T    pse_port_time_range_name[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH+1];
    UI32_T   pse_port_time_range_index;  /* time range callback use index as its key */
    UI32_T   pse_port_time_range_status;
#endif
}POE_OM_PsePort_T;

typedef struct POE_OM_MainPse
{
   UI32_T   main_pse_power;
   UI32_T   main_pse_oper_status;
   UI32_T   main_pse_consumption_power;
   UI32_T   main_pse_usage_threshold;
   UI32_T   main_pse_power_max_allocation;
   UI32_T   main_pse_Legacy_Detection;
   UI8_T    max_pse_port_number;
}POE_OM_MainPse_T;

#ifdef SYS_CPNT_POE_PSE_DOT3AT

#if 0
typedef struct
{
    UI16_T status_powerValue;
    UI16_T state_control_portValue;
    UI16_T ttl;
    UI8_T status_powerType;
    UI8_T status_powerSource;
    UI8_T status_powerPriority;
    UI8_T state_control_powerType;
    UI8_T state_control_powerSource;
    UI8_T state_control_powerPriority;
    UI8_T state_control_acknowledge;
}POE_MGR_DOT3at_PowerMode_T;
#endif

typedef struct
{
    UI8_T   power_type;
    UI8_T   power_source;
    UI8_T   power_priority;

#if (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_1_0)
    UI16_T  power_value;
    UI8_T   requested_power_type;
    UI8_T   requested_power_source;
    UI8_T   requested_power_priority;
    UI16_T  requested_power_value;
    UI8_T   acknowledge;
#elif (SYS_CPNT_POE_PSE_DOT3AT == SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2)
    UI16_T  pd_requested_power;
    UI16_T  pse_allocated_power;
#endif /* #if (SYS_CPNT_POE_PSE_DOT3AT_DRAFT_3_2 == TRUE) */

} POE_TYPE_Dot3atPowerInfo_T;

#endif

#endif  /* POEDRV_TYPE_H */
