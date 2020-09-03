/*-----------------------------------------------------------------------------
 * Module Name: SYSDRV.H  											 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of sysdrv.c                                  	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 * 1. The main function of this file is to provide the function of 
 *    reload for local unit and remote unit.
 * 
 *
 *-----------------------------------------------------------------------------
 *
 *         [ Master ]                                  [ Slave ]
 *
 *      +-------------+      
 *      |   SYSMGMT   |   
 *      +-------------+                                            
 *             |                             
 *             | (reload)                              
 *             |       
 *      +-------------+                             +-------------+    
 *      |    SYSDRV   |                             |    SYSDRV   |   
 *      +-------------+                             +-------------+                           
 *       /           \                               /           \
 *      /             \                             /             \
 * (to local unit)    +-----+                 +-----+         (to remote unit)
 *                    | ISC | ------------->> | ISC |
 *                    +-----+                 +-----+
 * 
 *
 *
 *-----------------------------------------------------------------------------
 * HISTORY:																	   
 *    11/21/2002 - Benson Hsu, Created	
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002 		   
 *-----------------------------------------------------------------------------
 */

#ifndef	SYSDRV_H
#define	SYSDRV_H


#include "sys_hwcfg.h"

/* quick workaround for projects that does not define
 * SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE. It is assumed that
 * the array size for the projects that do not define
 * SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE is 1.
 */
#ifndef SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE
#define SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE 1
#endif

#ifndef INCLUDE_DIAG
/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "string.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"
#include "uc_mgr.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "l_mm.h"
#include "isc.h"
#endif

#if (SYS_CPNT_ISCDRV == TRUE)
#include "isc.h"
//#include "iuc.h"
#endif /* end of SYS_CPNT_ISCDRV == TRUE */

#else/* #ifndef INCLUDE_DIAG */
#include "sys_type.h"

#endif /* end of INCLUDE_DIAG */
#include "i2cdrv.h"

#if (SYS_CPNT_ALARM_INPUT_DETECT==TRUE)
#if (SYS_HWCFG_SYSTEM_ALARM_INPUT_ASSERTED == 0)
#define SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, mask) \
    (((alarm_input_status)&(mask))==0)
#else
#define SYSDRV_ALARM_INPUT_IS_ASSERTED(alarm_input_status, mask) \
    (((alarm_input_status)&(mask))!=0)
#endif
#endif /* #if (SYS_CPNT_ALARM_INPUT_DETECT==TRUE) */


#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

#if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE)
#define SYSDRV_FAN_FULL_SPEED (SYSDRV_GetFanFullSpeedVal())
/* simply defines SYSDRV_FanSpeed_T as UI32_T when SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL
 * is TRUE. note that the valid range of fan speed level is from 1 to
 * the value returned by SYSDRV_GetFanFullSpeedVal().
 */
typedef UI32_T SYSDRV_FanSpeed_T;

#else
typedef enum SYSDRV_FanSpeed_E
{
    SYSDRV_FAN_STOP_SPEED = 0,           /* reserved, not using it at the moment*/
    SYSDRV_FAN_LOW_SPEED,
    SYSDRV_FAN_MID_SPEED,
    SYSDRV_FAN_FULL_SPEED,
    SYSDRV_FAN_PREPARE_TRANSITION_SPEED, /* a state to indicate that the thermal sensor has detected an overheat for the 1st consecutive time. it is checked for finding out the 2nd consecutive time.*/
    SYSDRV_FAN_SPEED_MODE_NBR            /* Number of fan speed mode enum, this constant must be put in the last entry. */
} SYSDRV_FanSpeed_T;
#endif

/* workaround for avoid compile error due to undefined SYS_HWCFG constants -- START */
#ifndef SYS_HWCFG_FAN_LOW_SPEED_LOWER_LIMIT
#define SYS_HWCFG_FAN_LOW_SPEED_LOWER_LIMIT 0
#endif
#ifndef SYS_HWCFG_FAN_LOW_SPEED_UPPER_LIMIT
#define SYS_HWCFG_FAN_LOW_SPEED_UPPER_LIMIT 0
#endif
#ifndef SYS_HWCFG_FAN_MID_SPEED_LOWER_LIMIT
#define SYS_HWCFG_FAN_MID_SPEED_LOWER_LIMIT 0
#endif
#ifndef SYS_HWCFG_FAN_MID_SPEED_UPPER_LIMIT
#define SYS_HWCFG_FAN_MID_SPEED_UPPER_LIMIT 0
#endif
#ifndef SYS_HWCFG_FAN_FULL_SPEED_LOWER_LIMIT
#define SYS_HWCFG_FAN_FULL_SPEED_LOWER_LIMIT 0
#endif
#ifndef SYS_HWCFG_FAN_FULL_SPEED_UPPER_LIMIT
#define SYS_HWCFG_FAN_FULL_SPEED_UPPER_LIMIT 0
#endif
/* workaround for avoid compile error due to undefined SYS_HWCFG constants -- END   */

#define SYSDRV_FAN_LOW_SPEED_LOWER_LIMIT    SYS_HWCFG_FAN_LOW_SPEED_LOWER_LIMIT
#define SYSDRV_FAN_LOW_SPEED_UPPER_LIMIT    SYS_HWCFG_FAN_LOW_SPEED_UPPER_LIMIT
#define SYSDRV_FAN_MID_SPEED_LOWER_LIMIT    SYS_HWCFG_FAN_MID_SPEED_LOWER_LIMIT
#define SYSDRV_FAN_MID_SPEED_UPPER_LIMIT    SYS_HWCFG_FAN_MID_SPEED_UPPER_LIMIT
#define SYSDRV_FAN_FULL_SPEED_LOWER_LIMIT   SYS_HWCFG_FAN_FULL_SPEED_LOWER_LIMIT
#define SYSDRV_FAN_FULL_SPEED_UPPER_LIMIT   SYS_HWCFG_FAN_FULL_SPEED_UPPER_LIMIT

#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#define SYSDRV_EVENT_COLD_START_FOR_RELOAD          BIT_1
#define SYSDRV_EVENT_WARM_START_FOR_RELOAD          BIT_2
#define SYSDRV_EVENT_COLD_START                     BIT_3
#define SYSDRV_EVENT_TIMER                          BIT_4
#define SYSDRV_EVENT_ENTER_TRANSITION               BIT_5
#define SYSDRV_EVENT_PREPARE_COLD_START_FOR_RELOAD          BIT_6
#define SYSDRV_EVENT_PREPARE_WARM_START_FOR_RELOAD          BIT_7


#define SYSDRV_SENSORS1_HIGH_THRESHOLD  71
#define SYSDRV_SENSORS2_HIGH_THRESHOLD  51
#define SYSDRV_SENSORS3_HIGH_THRESHOLD  66
#define SYSDRV_SENSORS1_NORMAL_THRESHOLD  66
#define SYSDRV_SENSORS3_NORMAL_THRESHOLD  61

/* Major Alarm type
 */
#define SYSDRV_ALARMMINORSTATUS_ALL_FE_LINK_DOWN_MASK       0x01 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_ALL_GE_LINK_DOWN_MASK       0x02 /*Not used*/
#define SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK        0x04 /*SYS_CPNT_ALARM_OUTPUT_FAN_ALARM*/
#define SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK   0x08 /*SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER*/
#define SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK 0x10 /*SYS_CPNT_ALARM_OUTPUT_MAJOR_POWER_MODULE_SET_STATUS*/
#define SYSDRV_ALARMMAJORSTATUS_OVERHEAT_MASK               0x20 /*Not used*/
#define SYSDRV_ALARMMAJORSTATUS_FAN_FAILURE_MASK            0x40 /*Not used*/
#define SYSDRV_ALARMMAJORSTATUS_MASK_ALL                    0x7F

/* Minor Alarm type
 */
#define SYSDRV_ALARMMINORSTATUS_FE_LINK_DOWN_MASK           0x01 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_GE_LINK_DOWN_MASK           0x02 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK    0x04 /*SYS_CPNT_ALARM_OUTPUT_FAN_ALARM*/
#define SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK               0x08 /*SYS_CPNT_ALARM_OUTPUT_MINOR_OVERHEAT*/
#define SYSDRV_ALARMMINORSTATUS_OVERCOOL_MASK               0x10 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_POWER_SUPPLY_A_MISSING_MASK 0x20 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_POWER_SUPPLY_B_MISSING_MASK 0x40 /*Not used*/
#define SYSDRV_ALARMMINORSTATUS_MASK_ALL                    0x7F

/* constants for bd_debug_flag
 */
#define SYSDRV_BD_DEBUG_FLAG_POWER          BIT_0
#define SYSDRV_BD_DEBUG_FLAG_FAN            BIT_1
#define SYSDRV_BD_DEBUG_FLAG_THERMAL        BIT_2
#define SYSDRV_BD_DEBUG_FLAG_POWER_TYPE     BIT_3
#define SYSDRV_BD_DEBUG_FLAG_SHOW_ERROR_MSG BIT_4
#define SYSDRV_BD_DEBUG_FLAG_SHOW_DEBUG_MSG BIT_5

/* Macros for SYSDRV used internally
 */
#define SYSDRV_SHOW_ERROR_MSG_FLAG() ({\
    BOOL_T __ret; \
    if(sysdrv_shmem_data_p->bd_debug_flag&SYSDRV_BD_DEBUG_FLAG_SHOW_ERROR_MSG) \
        __ret=TRUE;\
    else \
        __ret=FALSE;\
    __ret;})

#define SYSDRV_SHOW_DEBUG_MSG_FLAG() ({\
    BOOL_T __ret; \
    if(sysdrv_shmem_data_p->bd_debug_flag&SYSDRV_BD_DEBUG_FLAG_SHOW_DEBUG_MSG) \
        __ret=TRUE;\
    else \
        __ret=FALSE;\
    __ret;})

#if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE)
#define SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(fan_speed_mode)                                        \
({                                                                                                 \
    BOOL_T __is_valid_fan_speed_mode=TRUE;                                                         \
    if (((fan_speed_mode)==0) || ((fan_speed_mode)>SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL)||\
        ((fan_speed_mode)>SYSDRV_GetFanFullSpeedVal()))                                           \
        __is_valid_fan_speed_mode=FALSE;                                                           \
    __is_valid_fan_speed_mode;                                                                     \
})
#else
#define SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(fan_speed_mode) \
({                                                          \
    BOOL_T __is_valid_fan_speed_mode=TRUE;                  \
    if ((fan_speed_mode)>= SYSDRV_FAN_SPEED_MODE_NBR)       \
        __is_valid_fan_speed_mode=FALSE;                    \
    __is_valid_fan_speed_mode;                              \
})
#endif

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
/* SYSDRV_FanSpeedModeStateMachine_Type_E
 *     The definition for each type of fan speed mode state machine.
 *     SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_NONE is a special type that
 *     indicate a dummy state machine.
 */
typedef enum SYSDRV_FanSpeedModeStateMachine_Type_E
{
    SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_NONE=0, /* a dummy fan speed mode state machine */
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
    SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_AVG_TMP,
#endif
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
    SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG,
#endif
}SYSDRV_FanSpeedModeStateMachine_Type_T;

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
/* SYSDRV_FanSpeedModeStateMachine_AvgTmp_Info_T
 *     The data that is required for fan speed mode state machine type
 *     SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_AVG_TMP.
 */
typedef struct SYSDRV_FanSpeedModeStateMachine_AvgTmp_Info_S
{
    /* thermal_idx_to_check_ar
     *     The thermal sensor indices to be checked for evaluation of
     *     fan speed level.
     */
    UI8_T thermal_idx_to_check_ar[SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR];

    /* number_of_thermal_to_check
     *     The number of thermal sensor to be checked. The number of element in
     *     thermal_idx_to_check_ar must equal to this number.
     */
    UI8_T number_of_thermal_to_check;

    /* speed_level_to_up_temperature_threshold
     *     The array to get the up temperature threshold
     *     of each level. The meaning of the array is shown below:
     *     speed_level_to_up_temperature_threshold[fan_speed_level-1] = up_temp_threshold
     *         fan_speed_level = the fan speed level to be looked up
     *         up_temp_threshold = the up temperature threshold.
     *     Note: 1. when the current temperature is larger than up threshold, the speed
     *              level will be increased by 1.
     *           2. the last element of the array must be 0
     */
    I8_T  speed_level_to_up_temperature_threshold[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL];

    /* speed_level_to_down_temperature_threshold
     *     The array to get the down temperature threshold
     *     of each level. The meaning of the array is shown below:
     *     speed_level_to_down_temperature_threshold[fan_speed_level-1] = down_temp_threshold
     *         fan_speed_level = the fan speed level to be looked up
     *         down_temp_threshold = the down temperature threshold.
     *     Note: 1. when the current temperature is less than down threshold, the speed
     *              level will be decreased by 1.
     *           2. the first element of the array must be 0
     */
    I8_T  speed_level_to_down_temperature_threshold[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL];
} SYSDRV_FanSpeedModeStateMachine_AvgTmp_Info_T;
#endif /* end of #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE) */

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
/* SYSDRV_FanSpeedModeStateMachine_FruCfg_Info_T
 *     The data that is required for fan speed mode state machine type
 *     SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG.
 */
typedef struct SYSDRV_FanSpeedModeStateMachine_FruCfg_Info_S
{
    /* adjusted temperature offset for each thermal sensor
     */
    I8_T  adjusted_temp_offset[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    /* speed_level_to_up_temperature_threshold
     *     the up temperature threshold
     */
    I8_T  speed_level_to_up_temperature_threshold;

    /* speed_level_to_down_temperature_threshold
     *     the down temperature threshold
     */
    I8_T  speed_level_to_down_temperature_threshold;

    /* is_in_high_fan_speed_state
     *     TRUE  -  fan is operated in high speed state
     *     FALSE -  fan is operated in low speed state
     */
    BOOL_T is_in_high_fan_speed_state;
    /* fan_speed_level_ar[2]
     *     Array idx 0 = fan speed level for low temperature
     *     Array idx 1 = fan speed level for high temperature
     */
    UI32_T fan_speed_level_ar[2];
}SYSDRV_FanSpeedModeStateMachine_FruCfg_Info_T;
#endif

/* SYSDRV_FanSpeedModeStateMachine_Info_T
 *     A common type for fan speed mode state machine related information
 *     The real info kept in the structure depends on the value of type.
 */
typedef struct SYSDRV_FanSpeedModeStateMachine_Info_S
{
    union
    {
        UI8_T dummy;
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
        SYSDRV_FanSpeedModeStateMachine_AvgTmp_Info_T avg_tmp; /* for SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_AVG_TMP */
#endif
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
        SYSDRV_FanSpeedModeStateMachine_FruCfg_Info_T fru_cfg; /* for SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG */
#endif
    }info;

    /* number_of_speed_level
     *     Number of fan speed level. The fan speed level starts from 1.
     *     The higher value gets higer fan speed.
     *     Note: 1. If the number of fan speed level is different among different
     *              board ids , this constant must be defined as the maximum
     *              number among all board ids.
     */
    UI8_T number_of_speed_level;

    SYSDRV_FanSpeedModeStateMachine_Type_T type;
} SYSDRV_FanSpeedModeStateMachine_Info_T;
#endif /* end of #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE) */

#ifndef INCLUDE_DIAG
typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    UI32_T sys_drv_task_id;
    UI32_T sysdrv_my_unit_id;
    BOOL_T sysdrv_is_provision_complete;
    BOOL_T is_transition_done;
#if (SYS_CPNT_POWER_DETECT == TRUE)
    UI8_T  sysdrv_local_unit_power_status[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
    UI8_T  sysdrv_local_unit_power_status_bd_dbg[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
#endif
    UI32_T bd_debug_flag;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    #if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC != TRUE)
    UI32_T fan_type;
    #endif
    UI32_T sysdrv_fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
    UI32_T sysdrv_fan_speed[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
    BOOL_T sysdrv_fan_install_status[SYS_HWCFG_MAX_NBR_OF_FAN_PER_UNIT];
    UI32_T sysdrv_speed_setting_mode; /* enum value defined in SYSDRV_FanSpeed_T */
    UI8_T  sysdrv_local_unit_fan_status_bd_dbg[SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE];
    #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
        #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
    I8_T   fan_speed_calibration_adjust_counter[SYSDRV_FAN_SPEED_MODE_NBR][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
        #endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
    /* When fan speed mode is changed and set new duty cycle value to the fan
     * controller, it takes time for fan to rotate in the predefined range of
     * fan speed. Thus when this counter is not zero, the fan speed check will
     * be skipped in order to wait for fan rotating become stable.
     */
    UI8_T  fan_speed_in_transition_counter;
    UI8_T  fan_speed_in_transition_counter_reset_value;
    #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */
    #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    BOOL_T sysdrv_fan_speed_force_full;
    #endif
    #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
    SYSDRV_FanSpeedModeStateMachine_Info_T fan_speed_mode_sm_info;
    #endif

#endif /*End of SYS_CPNT_STKTPLG_FAN_DETECT*/

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    I32_T  sysdrv_thermal_temp[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    BOOL_T sysdrv_thermal_install_status[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    UI8_T  sysdrv_thermal_temp_bd_dbg[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
    UI8_T sysdrv_majorAlarmType_bitmap;
    UI8_T sysdrv_minorAlarmType_bitmap;
#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    UI8_T sysdrv_AlarmInputType_bitmap;
#endif
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
    UI8_T sysdrv_local_unit_power_type[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
    UI8_T sysdrv_local_unit_power_type_bd_dbg[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
#endif
    UI8_T sysdrv_backdoor_i2c_shell_bus_index;
} SYSDRV_Shmem_Data_T;



/* STATIC VARIABLE DECLARATIONS 
 */


/* EXPORTED SUBPROGRAM BODIES
 */
typedef enum 
{
    SYS_DRV_XENPAK_NOT_PRESENT      = 1,
    SYS_DRV_XENPAK_UNKNOWNTYPE      = 2,
    SYS_DRV_XENPAK_UNSUPPORTED_LR   = 3,
    SYS_DRV_XENPAK_UNSUPPORTED_ER   = 4,
    SYS_DRV_XENPAK_UNSUPPORTED_CX4  = 5,
    SYS_DRV_XENPAK_SUPPORTED_LR     = 6,
    SYS_DRV_XENPAK_SUPPORTED_ER     = 7,
    SYS_DRV_XENPAK_SUPPORTED_CX4    = 8
} SYS_DRV_XENPAK_STATUS_T;


/* FUNCTION NAME: SYSDRV_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  : successful
 *            FALSE : failure
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
BOOL_T SYSDRV_InitiateSystemResources(void);

/* FUNCTION NAME: SYSDRV_Create_InterCSC_Relation
 *-----------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Create_InterCSC_Relation(void);

/* FUNCTION NAME: SYSDRV_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_CreateTask(void);


/* FUNCTION NAME: SYSDRV_SetTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set SYSDRV to transition mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_SetTransitionMode(void);


/* FUNCTION NAME: SYSDRV_EnterTransitionMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter transition mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_EnterTransitionMode(void);


/* FUNCTION NAME: SYSDRV_EnterMasterMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter master mode	
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_EnterMasterMode(void);


/* FUNCTION NAME: SYSDRV_EnterSlaveMode
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will force SYSDRV to enter slave mode
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_EnterSlaveMode(void);



/* FUNCTION NAME: SYSDRV_ProvisionComplete
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is called by STKCTRL to notify provision complete
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
void SYSDRV_ProvisionComplete(void);

/* FUNCTION NAME: SYSDRV_ColdStartSystem
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will restart system to Loader with cold start
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   None
 */
void SYSDRV_ColdStartSystem(void);

void SYSDRV_PrepareStartSystem(UI32_T type);


/* FUNCTION NAME: SYSDRV_ReloadSystem
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will restart system to Loader with cold_start parameter.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   None
 */
void SYSDRV_ReloadSystem(void);


/* FUNCTION NAME: SYSDRV_WarmStartSystem
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will restart system to Loader with warm_start parameter.
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   None
 */
void SYSDRV_WarmStartSystem(void);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

/* FUNCTION NAME: SYSDRV_GetFanNum
 *-----------------------------------------------------------------------------
 * PURPOSE: Get actually FAN number for a specific unit
 *-----------------------------------------------------------------------------
 * INPUT    : unit->unit ID
 * OUTPUT   : fanNum->store actually FAN number
 * RETURN   : BOOL_T
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
BOOL_T SYSDRV_GetFanNum(UI32_T unit,UI32_T *fanNum);

/* FUNCTION NAME: SYSDRV_GetFanFailNum
 * PURPOSE: This function will return the number of failed fan.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  The number of failed fan.
 * NOTES:   None.
 */
UI32_T SYSDRV_GetFanFailNum(void);

/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYSDRV_GetFanFullSpeedVal
 *------------------------------------------------------------------------ 
 * PURPOSE: Get the value for the fan full speed.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 *-----------------------------------------------------------------------------
 * NOTES:   None.
 */
UI32_T SYSDRV_GetFanFullSpeedVal(void);

/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYSDRV_SetFanSpeedMode
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to set fan speed mode
 * INPUT:   mode  -- one of the enum value defined in SYSDRV_FanSpeed_T
 *                   except SYSDRV_FAN_SPEED_MODE_NBR.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- Success.
 *          FALSE -- Failed.
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYSDRV_SetFanSpeedMode(SYSDRV_FanSpeed_T mode);

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYSDRV_GetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to get status of force fan speed full
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- enabled.
 *          FALSE -- disabled.
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYSDRV_GetFanSpeedForceFull(void);

/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYSDRV_SetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to force fan speed full
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYSDRV_SetFanSpeedForceFull(BOOL_T mode);
#else /* #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */
/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYSDRV_GetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to get status of force fan speed full
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE  -- enabled.
 *          FALSE -- disabled.
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
static inline BOOL_T SYSDRV_GetFanSpeedForceFull(void) { return FALSE;}
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */
#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)

/* FUNCTION NAME: SYSDRV_GetThermalThresholds
 * PURPOSE: This routine will output the temperature falling thresholds
 *          and temperature rising thresholds of all thermal sensors.
 * INPUT:   None.
 * OUTPUT:  falling_thresholds  -  temperature falling thresholds of all thermal
 *                                 sensors.
 *          rising_thresholds   -  temperature rising threshold of all thermal
 *                                 sensors.
 * RETURN:  TRUE  - Success
 *          FALSE - Failed
 * NOTES:   None.
 */
BOOL_T SYSDRV_GetThermalThresholds(I8_T falling_thresholds[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT], I8_T rising_thresholds[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT]);

/* FUNCTION NAME: SYSDRV_GetThermalNum
 *-----------------------------------------------------------------------------
 * PURPOSE: Get actually thermalnumber for a specific unit
 *-----------------------------------------------------------------------------
 * INPUT    : unit->unit ID
 * OUTPUT   : thermal_num->store actually thermal number
 * RETURN   : BOOL_T
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
BOOL_T SYSDRV_GetThermalNum(UI32_T unit,UI32_T *thermal_num);

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

/* FUNCTION NAME: SYSDRV_SendWarmStartToOptionModule
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used by Master to send restart request to Option Module.
 *-----------------------------------------------------------------------------
 * INPUT    : drv unit
 * OUTPUT   : none
 * RETURN   : TRUE : successful
 *            FALSE: failure
 *-----------------------------------------------------------------------------
 * NOTES:   
 */
BOOL_T SYSDRV_SendWarmStartToOptionModule(UI32_T drv_unit);

#if ((SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE))
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSDRV_TestI2CChannel
 * ---------------------------------------------------------------------
 * PURPOSE: This function will test ISC Channel (fan/thermal) for ES4649/ES4625
 * 																		
 * INPUT: interval -- the polling interval after each ISC channel access in msec.                                     				
 * OUTPUT: None                          					
 * RETURN: fail -- the fail time of accessing ISC channel.                                             		
 * NOTES: None  
 * ---------------------------------------------------------------------
 */
UI32_T SYSDRV_TestI2CChannel(UI32_T interval);
#endif
#endif

/* -----------------------------------------------------------------------------
 * FUNCTION NAME - SYSDRV_GetPushButtonStatus
 * -----------------------------------------------------------------------------
 * PURPOSE : This function is used to read S/W master button status from FLASH.
 * INPUT   : unit_id - unit id or drive number
 * OUTPUT  : status  - S/W master button status, press(TRUE)/unpress(FALSE)
 * RETURN  : TRUE  - successful
 *           FALSE - otherwise
 * NOTES   : The S/W master button status will be save on an invisible binary file.
 *           File name is "$sw_push_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetPushButtonStatus(UI32_T unit_id, BOOL_T *status);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME - SYSDRV_SetPushButtonStatus
 * -----------------------------------------------------------------------------
 * PURPOSE : This function is used to write S/W master button status to FLASH.
 * INPUT   : unit_id - unit id or drive number
 *           status  - S/W master button status, press(TRUE)/unpress(FALSE)
 * OUTPUT  : None.
 * RETURN  : TRUE  - successful
 *           FALSE - otherwise
 * NOTES   : The S/W master button status will be save on an invisible binary file.
 *           File name is "$sw_push_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetPushButtonStatus(UI32_T unit_id, BOOL_T status);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME - SYSDRV_GetStackingButtonStatus
 * -----------------------------------------------------------------------------
 * PURPOSE : This function is used to read S/W stacking button status from FLASH.
 * INPUT   : None.
 * OUTPUT  : status  - S/W stacking button status, press(TRUE)/unpress(FALSE)
 * RETURN  : TRUE  - successful
 *           FALSE - otherwise
 * NOTES   : The S/W stacking button status will be save on an invisible binary file.
 *           File name is ".sw_stacking_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetStackingButtonStatus(BOOL_T *status);

/* -----------------------------------------------------------------------------
 * FUNCTION NAME - SYSDRV_SetStackingButtonStatus
 * -----------------------------------------------------------------------------
 * PURPOSE : This function is used to write S/W stacking button status to FLASH.
 * INPUT   : status  - S/W stacking button status, press(TRUE)/unpress(FALSE)
 * OUTPUT  : None.
 * RETURN  : TRUE  - successful
 *           FALSE - otherwise
 * NOTES   : The S/W stacking button status will be save on an invisible binary file.
 *           File name is ".sw_stacking_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetStackingButtonStatus(BOOL_T status);

#if 0 /* to do: remove */
/* FUNCTION NAME: SYSDRV_Register_XFPModuleStatusChanged_CallBack
 *----------------------------------------------------------------------------------
 * PURPOSE: This function is used to register the callback when XFP module status is changed
 *----------------------------------------------------------------------------------
 * INPUT:   fun -- call back function pointer
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   None
 */
void SYSDRV_Register_XFPModuleStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T status));
#endif

#if 0 /* to do: remove */
/* FUNCTION NAME: SYSDRV_Register_XenpakStatusChanged_CallBack
 *----------------------------------------------------------------------------------
 * PURPOSE: This function is used to register the callback when xenpak status is changed
 *----------------------------------------------------------------------------------
 * INPUT:   fun -- call back function pointer
 * OUTPUT:  None
 * RETURN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   None
 */
void SYSDRV_Register_XenpakStatusChanged_CallBack(void (*fun)(UI32_T unit, UI32_T xenpak_status));
#endif

#if (SYS_CPNT_STACKING == TRUE) && !defined(INCLUDE_DIAG)
/* FUNCTION NAME: SYSDRV_ISC_Handler
 * PURPOSE: This function is called by ISC_AGENT to handle messages from ISC and
 *          dispatch it to related function.
 * INPUT:   key   -- identification information from ISC
 *          mref  -- memory reference for ISC message buffer.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:   This is used for Slave ONLY to receive a service ID from Master.
 *
 */
BOOL_T SYSDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYSDRV_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for SYSDRV in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SYSDRV_AttachSystemResources(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetI2CInfo
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to write information to I2C module with sleep(10).
 * INPUT   : slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be writen
 *           info        : the data, written to I2C module
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : This function just redirect to I2C DRV with sleep(10).
 *------------------------------------------------------------------------*/
BOOL_T SYSDRV_GetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetI2CInfo
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to write information to I2C module with sleep(10)
 * INPUT   : slave_addr  : slave address
 *           offset      : offset on I2C address mapping
 *           size        : the number of bytes to be writen
 *           info        : the data, written to I2C module
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : This function just redirect to I2C DRV with sleep(10).
 *------------------------------------------------------------------------*/
BOOL_T SYSDRV_SetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info);

#if ((SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_WIN83782))
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SYSDRV_GetI2CThermal2Info
 * ---------------------------------------------------------------------
 * PURPOSE: This function will get temperature data from thermal 2 for WinBond W83782D fan.
 *
 * INPUT:   slave_addr -- the i2c address of thermal detector
 * OUTPUT:  *info      -- the temperature data from thermal 2 register.
 * RETURN:  TRUE   --  Get temperature data successfully.
 *          FALSE  --  Failed to get data.
 * NOTES:   None
 * ---------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetI2CThermal2Info(UI8_T slave_addr, UI8_T *info);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_Notify_FanStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when fan status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit   --- Which unit.
 *            fan    --- Which fan.
 *            status --- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is only called by sysdrv.c and sysdrv_task.c.
 */
void SYSDRV_Notify_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status);

/* FUNCTION NAME: SYSDRV_Notify_FanSpeedChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when fan speed is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit   --- Which unit.
 *            fan    --- Which fan.
 *            speed  ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is only called by sysdrv.c and sysdrv_task.c.
 */
void SYSDRV_Notify_FanSpeedChanged(UI32_T unit, UI32_T fan, UI32_T speed);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_Notify_ThermalStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when thermal status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit    --- Which unit.
 *            thermal ---
 *            status  ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is only called by sysdrv.c and sysdrv_task.c.
 */
void SYSDRV_Notify_ThermalStatusChanged(UI32_T unit, UI32_T thermal, I32_T status);
#endif

/* FUNCTION NAME: SYSDRV_Notify_XenpakStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *-----------------------------------------------------------------------------
 * INPUT    : unit   --- Which unit.
 *            power  --- Which power.
 *            status ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES: This function is only called by sysdrv.c and sysdrv_task.c.
 */
void SYSDRV_Notify_XenpakStatusChanged(UI32_T unit, UI32_T xenpak_type);

/* FUNCTION NAME: SYSDRV_Notify_PowerStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when power status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit   --- Which unit.
 *            power  --- Which power.
 *            status ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_PowerStatusChanged(UI32_T unit, UI32_T power, UI32_T status);

/* FUNCTION NAME: SYSDRV_Notify_PowerTypeChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when power type is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit   --- Which unit.
 *            power  --- Which power.
 *            type   --- Which power type
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_PowerTypeChanged(UI32_T unit, UI32_T power, UI32_T type);

/*------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetI2CChannel
 *------------------------------------------------------------------------
 * FUNCTION: This function is used to write information to I2C module with sleep(10).
 * INPUT   : channel_addr  : slave address
 *           channel_regaddr      : offset on I2C address mapping
 *           channel_num        : the number of bytes to be writen
 * OUTPUT  : None
 * RETURN  : TRUE  : success
 *           FALSE : fail
 * NOTE    : This function just redirect to I2C DRV with sleep(10).
 *------------------------------------------------------------------------*/
BOOL_T SYSDRV_SetI2CChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num);

/*add by michael.wang,2008-7-1 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetSfpInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  read Sfp Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array
 *               offset:data register offset address
 *               size:size of read data
 * OUTPUT: info_p:read data
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetSfpInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetSfpInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  write Sfp Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array
 *               offset:data register offset address
 *               size:size of written data
 * OUTPUT: info_p:written data
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetSfpInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetSfpDdmInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  read Sfp DDM Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array,1,2,....
 *               offset:data register offset address
 *               size:size of read data
 * OUTPUT: info_p:read data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetSfpDdmInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetSfpDdmInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  write Sfp DDM Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array,1,2,....
 *               offset:data register offset address
 *               size:size of written data
 * OUTPUT: info_p:written data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetSfpDdmInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_RTC_SetDateTime
 *---------------------------------------------------------------------------
 * PURPOSE:  set RTC time
 * INPUT: year:  year value.  
 *            month:month value.
 *            day:day value
 *           hour:hour value
 *           minute:minute value
 *           second:second value
 * OUTPUT:   
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SYSDRV_RTC_SetDateTime(int year,    /* 2001-2099 */
                       int month,   /* 01-12 */
                       int day,     /* 01-31 */
                       int hour,    /* 00-23 */
                       int minute,  /* 00-59 */
                       int second) ; /* 00-59 */


/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_RTC_GetDateTime
 *---------------------------------------------------------------------------
 * PURPOSE:  GET RTC Time
 * INPUT   : read_fn_p  -  Function pointer for doing I2C read operation. Use
 *                         default I2C read function if this argument is NULL.
 *           write_fn_p -  Function pointer for doing I2C write operation. Use
 *                         default I2C write function if this argument is NULL.
 * OUTPUT: year_p  : year value
 *         month_p : month value
 *         day_p   : day value
 *         hour_p  : hour value
 *         minute_p: minute value
 *         second_p: second value
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T  SYSDRV_RTC_GetDateTime(
                        I2CDRV_TwsiDataRead_Func_T read_fn_p,
                        I2CDRV_TwsiDataWrite_Func_T write_fn_p,
                        int *year_p,      /* 2001-2099 */
                        int *month_p,     /* 01-12 */
                        int *day_p,       /* 01-31 */
                        int *hour_p,      /* 00-23 */
                        int *minute_p,    /* 00-59 */
                        int *second_p);   /* 00-59 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init chip
 * INPUT:          
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_CHIP_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  init chip
 * INPUT:          
 * OUTPUT: 
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_CHIP_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetTemperature
 *---------------------------------------------------------------------------
 * PURPOSE:  get cuurent temperature from Thermal
 * INPUT:      index: Thermal index 1,2,3.             
 * OUTPUT:   temperature:Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_GetTemperature(UI8_T index, I8_T* temperature);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_SetThreshold
 *---------------------------------------------------------------------------
 * PURPOSE:  set Thermal trap value
 * INPUT:      index: Thermal index.             
 * OUTPUT:   temperature:set Thermal sensor temperature
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_SetThreshold(UI8_T index, I8_T  temperature); 

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInRPM
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInRPM(UI8_T index, UI32_T* speed_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan speed value
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInDutyCycle(UI8_T index, UI32_T* speed_p);

/* FUNCTION NAME: SYSDRV_DetectPeripheralInstall
 * PURPOSE: This routine will detect if there is any peripheral installed,
 *          if the peripherals is installed, we will detect it peroidically in
 *          the sysdrv task, otherwise, we will not detect it.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   Currently this function will detect the below Peripheral.
 *          1: Thermal
 *          2: Fan Controller
 *          051025 tc Changed printf to SYSFUN_Debug_Printf, These information are for debug purpose
 */
void SYSDRV_DetectPeripheralInstall(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_SetSpeed
 *---------------------------------------------------------------------------
 * PURPOSE:  set fan speed value
 * INPUT:    index -- fan index. (start from 1)
 * OUTPUT:   speed -- fan speed.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_SetSpeed(UI8_T index, UI32_T speed);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  get Fan status
 * INPUT:    None
 * OUTPUT:   status_p -- fan status
 *           bit 0~7 represent the fan fail status of fan 1~8
 *           if fan fails, set the corresponding bit to 1.
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetStatus(UI8_T* status_p);

void SYSDRV_ColdStartSystemForTimeout(void);
void SYSDRV_WarmStartSystemForTimeout(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetPowerStatusFromASIC
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power status raw data from ASIC.
 * INPUT  :  power_index          -- Which power(1 based)
 * OUTPUT :  power_status_p         -- raw data of power status value got from ASIC
 * RETURN :  TRUE - success FALSE - failed
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetPowerStatusFromASIC(UI32_T power_index, UI8_T *power_status_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetPowerModuleType
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power module type from the register.
 * INPUT  :  power_index         -- Which power(1 based)
 * OUTPUT :  power_module_type_p -- power module type (SYS_HWCFG_COMMON_POWER_XXX_MODULE_TYPE)
 * RETURN :  TRUE - success FALSE - failed
 * NOTE   :  None
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetPowerModuleType(UI32_T power_index, UI32_T *power_module_type_p);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetPowerModuleSetStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power module set status.
 * INPUT  :  None
 * OUTPUT :  is_status_good_p    -- TRUE: status good. FALSE: status bad
 * RETURN :  TRUE: success FALSE: failed
 * NOTE   :  if DC power module is used on the device, the DC power type
 *           must be the same. Failed to follow this will damage the device.
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetPowerModuleSetStatus(BOOL_T *is_status_good_p);

#if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_Upgrade_CPLD
 *---------------------------------------------------------------------------
 * PURPOSE:  Do upgrade CPLD fw
 * INPUT:    buf  : cpld data
             bufsize: cpld data of length
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
*/
BOOL_T SYSDRV_Upgrade_CPLD(UI8_T *buf, UI32_T bufsize);
#endif /*SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE */

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetCfgHwPortMode
 *---------------------------------------------------------------------------
 * PURPOSE:  To load HW port mode setting
 * INPUT:    unit
 * OUTPUT:   cfg_hw_port_mode_ar
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetCfgHwPortMode
 *---------------------------------------------------------------------------
 * PURPOSE:  To save HW port mode setting
 * INPUT:    unit
 *           cfg_hw_port_mode_ar
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT]);
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

#endif /* SYSDRV_H */

