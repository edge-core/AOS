/* ------------------------------------------------------------------------
 *  FILE NAME  -  SYS_ENV_VM.C
 * ------------------------------------------------------------------------
 * PURPOSE: 1. This package provide a set of database and machine to store the
 *             environment status and send trap when necessary.
 *          2. The environment status means fan status, power status, thermal,
 *             and etc.
 *          3. The naming constant defined in this package shall be reused by
 *             all the BNBU L2/L3 switch projects.
 *          4. This package shall be reusable for all all the BNBU L2/L3 switch
 *             projects.
 *
 *  History:
 *       Charles Cheng   3/12/2003      new created for machanism of fan detection
 *       Charles Cheng   3/21/2003      machanism of power detection
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2003
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "sys_env_vm.h"
#include "leaf_es3626a.h"

 #include "trap_event.h"

#include "l_cvrt.h"
#include "sys_dflt.h"
#include "stktplg_pom.h"
#include "snmp_pmgr.h"
#include "sysdrv.h"
#include "backdoor_mgr.h"
#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
#include "stktplg_board.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
typedef enum ThermalStatus_E
{
    THERMAL_STATUS_FALLING,
    THERMAL_STATUS_RISING    
} ThermalStatus_T;

/* LOCAL TYPE DECLARATION
 */

/* LOCAL SUBPROGRAM DECLARATION
 */
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static int SYS_ENV_VM_ThermalActionSortedListCmpFunc(void * inlist_element, void * input_element);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T sys_env_vm_power_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static UI32_T sys_env_vm_power_type[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
static UI32_T sys_env_vm_alarm_input_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];/* bit mapped value which stores 4 status */
static char   sys_env_vm_alarm_input_name[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT][MAXSIZE_swAlarmInputName+1];
#endif
#if (SYS_CPNT_ALARM_DETECT == TRUE)
static UI32_T sys_env_vm_major_alarm_output_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
static UI32_T sys_env_vm_minor_alarm_output_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
#endif


#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
static UI32_T sys_env_vm_fan_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
static UI32_T sys_env_vm_fan_oper_speed[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
static UI32_T sys_env_vm_fan_admin_speed[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
static UI32_T sys_env_vm_fan_fail_counter[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
static BOOL_T sys_env_vm_fan_speed_force_full;
#endif
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static I32_T sys_env_vm_thermal_temperature[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
static SYS_ENV_VM_SwitchThermalActionTable_T sys_env_vm_thermal_action_table =
{
    /* sorted_list
     */
    {0, /* nbr_of_element */
     0, /* size_of_element */
     0,
     NULL, /* head */
     NULL, /* compare */
    },
    FALSE /* is_init */
};

static ThermalStatus_T sys_env_vm_thermal_status[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

/* MACRO FUNCTIONS DECLARACTION
 */
#define SYSMGMT_BACKDOOR_Printf(fmt_p, ...) BACKDOOR_MGR_Printf((fmt_p), ##__VA_ARGS__)
#define SYS_ENV_VM_POWER_TYPE_IS_DC(power_type) ({ \
    BOOL_T __ret; \
    if(((power_type)!=VAL_swIndivPowerType_AC) && \
       ((power_type)!=VAL_swIndivPowerType_none) && \
       ((power_type)!=VAL_swIndivPowerType_AC_Wrong)) \
       __ret=TRUE; \
    else \
       __ret=FALSE; \
    __ret;})

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static void SYS_ENV_VM_ThermalTakeAction(UI32_T unit_index, UI32_T thermal_index,
    I32_T thermal_temperature_old, I32_T thermal_temperature_new,
    BOOL_T *abnormal_status_changed_p, BOOL_T *is_abnormal_p);
#endif

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static void SYS_ENV_VM_GetPowerModuleSetStatus(UI32_T unit_id, BOOL_T *is_status_good_p);
#endif

static BOOL_T sys_env_debug_mode = FALSE;

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetDatabaseToDefault
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to init the database without sending trap.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetDatabaseToDefault(void)
{
    UI32_T unit;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    UI32_T fan;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    UI32_T thermal;
#endif
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    UI32_T alarm_input_mask;
    UI8_T  alarm_input_index;
    char alarm_input_buf[MAXSIZE_swAlarmInputName+1];
#endif
#if (SYS_CPNT_POWER_DETECT == TRUE) && (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    SYS_HWCFG_PowerRegInfo_T power_reg_info;
    UI32_T board_id=0;
    UI8_T pwr_idx;
    BOOL_T unit_exist;
#endif


    for(unit = 1; unit <= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        /* main power
         */
        #if (SYS_CPNT_POWER_DETECT == TRUE)
            #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == FALSE)
            /* when SYS_HWCFG_PWR_PRES_MASK and SYS_HWCFG_PWR_PRES_OK are both
             * zeros, it means the main power is a built-in power and always present
             */
            #if (SYS_HWCFG_PWR_PRES_MASK==0) && (SYS_HWCFG_PWR_PRES_OK==0)
        sys_env_vm_power_status[unit-1][0] = VAL_swIndivPowerStatus_red;/*main power: power down */
            #else
        sys_env_vm_power_status[unit-1][0] = VAL_swIndivPowerStatus_notPresent;/*main power: not present */
            #endif
            #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2) /*s'pose no trird power*/
        /* redundant power
         */
        /* when SYS_HWCFG_RPS_PRES_MASK and SYS_HWCFG_RPS_PRES_OK are both
         * zeros, it means the redundant power is a built-in power and always present
         */
            #if (SYS_HWCFG_RPS_PRES_MASK==0) && (SYS_HWCFG_RPS_PRES_OK==0)
        sys_env_vm_power_status[unit-1][1] = VAL_swIndivPowerStatus_red;/*main power: power down */
            #else
        sys_env_vm_power_status[unit-1][1] = VAL_swIndivPowerStatus_notPresent;
            #endif

            #endif /* end of #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2) */
            #else /* #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == FALSE) */
            if (STKTPLG_OM_UnitExist(unit)==TRUE)
            {
                unit_exist=TRUE;
                if (STKTPLG_OM_GetUnitBoardID(unit, &board_id)==FALSE)
                {
                    /* treat it as unit not exists
                     */
                    unit_exist=FALSE;
                }
            }
            else
            {
                unit_exist=FALSE;
            }

            for(pwr_idx=1; pwr_idx<=SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; pwr_idx++)
            {
                BOOL_T get_info_ok;

                if (unit_exist==TRUE)
                    get_info_ok=STKTPLG_BOARD_GetPowerStatusInfoByBoardId(board_id, pwr_idx, &power_reg_info);
                else
                    get_info_ok=STKTPLG_BOARD_GetPowerStatusInfo(pwr_idx, &power_reg_info);

                if (get_info_ok == TRUE)
                {
                    /* when power_present_mask and power_is_present_val are both
                     * zeros, it means the redundant power is a built-in power and always present
                     */
                    if ((power_reg_info.power_present_mask==0) &&
                        (power_reg_info.power_is_present_val==0))
                    {
                        sys_env_vm_power_status[unit-1][pwr_idx-1] = VAL_swIndivPowerStatus_red;/* power down */
                    }
                    else
                    {
                        sys_env_vm_power_status[unit-1][pwr_idx-1] = VAL_swIndivPowerStatus_notPresent;
                    }
                }
                else
                {
                    /* some board id might have only 1 PSU
                     * If STKTPLG_BOARD_GetPowerStatusInfo returns FALSE, treat
                     * it as not present.
                     */
                    sys_env_vm_power_status[unit-1][pwr_idx-1] = VAL_swIndivPowerStatus_notPresent;
                }
            }
            #endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == FALSE) */
        #else /* #if (SYS_CPNT_POWER_DETECT == TRUE) */
        sys_env_vm_power_status[unit-1][0] = VAL_swIndivPowerStatus_green;/*main power: present */
        #endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        {
            UI32_T power_index;

            /* power type
             */
            for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
                sys_env_vm_power_type[unit-1][power_index]=VAL_swIndivPowerType_none;
        }
#endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
        /* alarm input */
        alarm_input_mask = (SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK);
        sys_env_vm_alarm_input_status[unit-1] = ((SYS_HWCFG_SYSTEM_ALARM_INPUT_ASSERTED<<SYS_HWCFG_SYSTEM_ALARM_INPUT_SHIFT)&alarm_input_mask)?0:alarm_input_mask; /* alarm input: not asserted */
        for(alarm_input_index=1; alarm_input_index  <= SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT; alarm_input_index++)
        {
            sprintf(alarm_input_buf, "ALARM_IN%d", alarm_input_index);
            strcpy(sys_env_vm_alarm_input_name[unit-1][alarm_input_index-1], alarm_input_buf);
        }
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
        sys_env_vm_major_alarm_output_status[unit-1] = 0;
        sys_env_vm_minor_alarm_output_status[unit-1] = 0;
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        for(fan = 1; fan <= SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; fan++)
        {
            sys_env_vm_fan_status[unit-1][fan-1] = VAL_switchFanStatus_ok;
            sys_env_vm_fan_fail_counter[unit-1][fan-1] = 0;
            sys_env_vm_fan_oper_speed[unit-1][fan-1] = 0;
            sys_env_vm_fan_admin_speed[unit-1][fan-1] = 0;
        }
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
        sys_env_vm_fan_speed_force_full = SYS_DFLT_SYSMGMT_FAN_SPEED_FORCE_FULL;
#endif
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        for(thermal = 1; thermal <= SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; thermal++)
        {
            sys_env_vm_thermal_temperature[unit-1][thermal-1] = 0;
            sys_env_vm_thermal_status[unit-1][thermal-1] = THERMAL_STATUS_FALLING;
        }
#endif
    }

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    SYS_ENV_VM_InitThermalActionTable();
#endif
    return TRUE;
}

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetAlarmInputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get status of alarm input.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status_p         ---  status of four alarm input(bit mapped)
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_GetAlarmInputStatus(UI32_T unit, UI32_T *status_p)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (status_p == NULL)
    {
        return FALSE;
    }

    /* action
     */
    *status_p = sys_env_vm_alarm_input_status[unit-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetAlarmInputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set status of alarm input.
 * INPUT   : unit             --- Which unit.
 *           status           ---  status of four alarm input(bit mapped)
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_SetAlarmInputStatus(UI32_T unit, UI32_T status)
{
    TRAP_EVENT_TrapData_T       trap_data={0};
    UI32_T diff_status;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if ( status & ~(SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK))
    {
        printf(" status=0x%lx is not subset of 0x%lx \n", (unsigned long)status, (unsigned long)(SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK) );
        return FALSE;
    }

    /* action
     */
    if (sys_env_vm_alarm_input_status[unit-1] == status)
    {
        return TRUE;
    }

    /* use xor to evaulate the difference bits
     */
    diff_status = sys_env_vm_alarm_input_status[unit-1] ^ status;
    sys_env_vm_alarm_input_status[unit-1] = status;

    /* send trap
     */
    if (diff_status & SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK)
    {
        if(SYSDRV_ALARM_INPUT_IS_ASSERTED(status, SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK))
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_1;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputType_alarmInputType_1;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_1-1]);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT_RECOVER;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_1;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputRecoverType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputRecoverType_alarmInputRecoverType_1;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_1-1]);
        }
        trap_data.community_specified = FALSE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }

    if (diff_status & SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK)
    {
        if(SYSDRV_ALARM_INPUT_IS_ASSERTED(status, SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK))
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_2;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputType_alarmInputType_2;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_2-1]);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT_RECOVER;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_2;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputRecoverType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputRecoverType_alarmInputRecoverType_2;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_2-1]);
        }
        trap_data.community_specified = FALSE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }

    if (diff_status & SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK)
    {
        if(SYSDRV_ALARM_INPUT_IS_ASSERTED(status, SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK))
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_3;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputType_alarmInputType_3;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_3-1]);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT_RECOVER;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_3;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputRecoverType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputRecoverType_alarmInputRecoverType_3;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_3-1]);
        }
        trap_data.community_specified = FALSE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }

    if (diff_status & SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK)
    {
        if(SYSDRV_ALARM_INPUT_IS_ASSERTED(status, SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK))
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_4;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputType_alarmInputType_4;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_4-1]);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_SW_ALARM_INPUT_RECOVER;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[0] = unit;
            trap_data.u.alarmMgt.instance_sw_alarm_unit_index_alarm_input_index[1] = VAL_alarmInputType_alarmInputType_4;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmInputRecoverType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmInputRecoverType_alarmInputRecoverType_4;
            strcpy(trap_data.u.alarmMgt.sw_alarm_input_name, sys_env_vm_alarm_input_name[unit-1][VAL_alarmInputType_alarmInputType_4-1]);
        }
        trap_data.community_specified = FALSE;
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetAlarmInputName
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get name of alarm input.
 * INPUT   : unit             --- Which unit.
 *           index            --- Which index.
 * OUTPUT  : name_p           --- description of alarm input
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------
 */
BOOL_T SYS_ENV_VM_GetAlarmInputName(UI32_T unit, UI32_T index, char *name_p)
{
    UI8_T len;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if(index == 0 || index > SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
    {
        return FALSE;
    }

    if (name_p == NULL)
    {
        return FALSE;
    }

    /* action
     */
    len = strlen(sys_env_vm_alarm_input_name[unit-1][index-1])+1;
    strncpy(name_p, sys_env_vm_alarm_input_name[unit-1][index-1],
            len<= MAXSIZE_swAlarmInputName?len:(MAXSIZE_swAlarmInputName+1));

    return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetAlarmInputName
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set name of alarm input.
 * INPUT   : unit             --- Which unit.
 *           index            --- Which index.
 *           name_p           --- description of alarm input
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetAlarmInputName(UI32_T unit, UI32_T index, char *name_p)
{
    UI8_T len;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (index == 0 || index > SYS_ADPT_MAX_NBR_OF_ALARM_INPUT_PER_UNIT)
    {
        return FALSE;
    }

    if (name_p == NULL)
    {
        return FALSE;
    }

    /* action
     */
    len = strlen(name_p)<=MAXSIZE_swAlarmInputName?strlen(name_p):MAXSIZE_swAlarmInputName;
    strncpy(sys_env_vm_alarm_input_name[unit-1][index-1], name_p, len);
    sys_env_vm_alarm_input_name[unit-1][index-1][len]='\0';

    return TRUE;
}
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetMajorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get major alarm output status.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status           --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * RETURN  : TRUE if get successfully
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetMajorAlarmOutputStatus(UI32_T unit, UI32_T *status)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (status == NULL)
    {
        return FALSE;
    }

    /* action
     */
    *status = sys_env_vm_major_alarm_output_status[unit-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetMajorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set major alarm output status.
 * INPUT   : unit             --- Which unit.
 *           status           --- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : None.
 * RETURN  : TRUE if set sucessfully.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetMajorAlarmOutputStatus(UI32_T unit, UI32_T status)
{
    TRAP_EVENT_TrapData_T       trap_data={0};
    UI32_T                      diff_status;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if ( status & ~(SYSDRV_ALARMMAJORSTATUS_MASK_ALL))
    {
        printf("status=0x%lx is not subset of 0x%lx \n", (unsigned long)status, (unsigned long)(SYSDRV_ALARMMAJORSTATUS_MASK_ALL));
        return FALSE;
    }

    /* action
     */
    if (sys_env_vm_major_alarm_output_status[unit-1] == status)
    {
        return TRUE;
    }

    /* use xor to evaluate the difference bit
     */
    diff_status=sys_env_vm_major_alarm_output_status[unit-1] ^ status;

    /* send trap
     */
    if(diff_status & SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
    {
        if(status & SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorType_powerModuleFailure;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM_RECOVERY;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorRecoveryType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorRecoveryType_powerModuleRecovery;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
    }

    if(diff_status & SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK)
    {
        if(status & SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK)
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorType_allFanFailure;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM_RECOVERY;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorRecoveryType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorRecoveryType_fanRecovery;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
    }

    if(diff_status & SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK)
    {
        if(status & SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK)
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorType_wrongPowerModuleSet;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
        else
        {
            trap_data.trap_type = TRAP_EVENT_MAJOR_ALARM_RECOVERY;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMajorRecoveryType;            
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMajorRecoveryStatus_wrongPowerModuleSetRecovery;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
    }

    sys_env_vm_major_alarm_output_status[unit-1]=status;

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetMinorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get minor alarm output status.
 * INPUT   : unit             --- Which unit.
 * OUTPUT  : status           --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * RETURN  : TRUE if get successfully
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetMinorAlarmOutputStatus(UI32_T unit, UI32_T *status)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (status == NULL)
    {
        return FALSE;
    }

    /* action
     */
    *status = sys_env_vm_minor_alarm_output_status[unit-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetMinorAlarmOutputStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set minor alarm output status.
 * INPUT   : unit             --- Which unit.
 *           status           --- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : None.
 * RETURN  : TRUE if set sucessfully.
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetMinorAlarmOutputStatus(UI32_T unit, UI32_T status)
{
    TRAP_EVENT_TrapData_T       trap_data={0};
    UI32_T                      diff_status;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if ( status & ~(SYSDRV_ALARMMINORSTATUS_MASK_ALL))
    {
        printf("status=0x%lx is not subset of 0x%lx \n", (unsigned long)status, (unsigned long)(SYSDRV_ALARMMINORSTATUS_MASK_ALL));
        return FALSE;
    }

    /* action
     */
    if (sys_env_vm_minor_alarm_output_status[unit-1] == status)
    {
        return TRUE;
    }

    /* use xor to evaluate the difference bit
     */
    diff_status=sys_env_vm_minor_alarm_output_status[unit-1] ^ status;

    /* send trap if required
     */
    if(diff_status & SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
    {
        if(status & SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
        {
            trap_data.trap_type = TRAP_EVENT_MINOR_ALARM;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMinorType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMinorType_thermalDetectorOverHeating;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
    }

    if(diff_status & SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK)
    {
        if(status & SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK)
        {
            trap_data.trap_type = TRAP_EVENT_MINOR_ALARM;
            trap_data.u.alarmMgt.alarmType = LEAF_alarmMinorType;
            trap_data.u.alarmMgt.alarmObjectType = VAL_alarmMinorType_fanFailure;
            trap_data.community_specified = FALSE;
            SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
        }
    }

    sys_env_vm_minor_alarm_output_status[unit-1]=status;

    return TRUE;
}

#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetPowerStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get power status.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power.
 * OUTPUT  : status --- VAL_swIndivPowerStatus_notPresent
 *                      VAL_swIndivPowerStatus_green
 *                      VAL_swIndivPowerStatus_red
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetPowerStatus(UI32_T unit, UI32_T power, UI32_T *status)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return FALSE;
    }

    if (status == 0)
    {
        return FALSE;
    }

    /* action
     */
    *status = sys_env_vm_power_status[unit-1][power-1];

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetPowerStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set power status.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power.
 *           status --- VAL_swIndivPowerStatus_notPresent
 *                      VAL_swIndivPowerStatus_green
 *                      VAL_swIndivPowerStatus_red
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetPowerStatus(UI32_T unit, UI32_T power, UI32_T status)
{
    TRAP_EVENT_TrapData_T       trap_data;
#ifdef ECS4810_12MV2
    UI32_T power_switch = power;
#endif

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return FALSE;
    }

    if (status != VAL_swIndivPowerStatus_notPresent &&
        status != VAL_swIndivPowerStatus_green      &&
        status != VAL_swIndivPowerStatus_red        )
    {
        return FALSE;
    }

    /* action
     */
    if (sys_env_vm_power_status[unit-1][power-1] == status)
    {
        return TRUE;
    }

    sys_env_vm_power_status[unit-1][power-1] = status;

    /* send trap
     */
#if 1 /*Build2 no */     
    trap_data.trap_type                    = TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP;
    trap_data.community_specified          = FALSE;

    trap_data.u.sw_power_status_change_trap.sw_indiv_power_unit_index               = unit;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_unit_index[0]   = unit;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_unit_index[1]   = power;

    trap_data.u.sw_power_status_change_trap.sw_indiv_power_index                    = power;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_index[0]        = unit;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_index[1]        = power;

    trap_data.u.sw_power_status_change_trap.sw_indiv_power_status                   = status;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_status[0]       = unit;
    trap_data.u.sw_power_status_change_trap.instance_sw_indiv_power_status[1]       = power;

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif

#ifdef ECS4810_12MV2
    /* One of the power recovers:
     *     For ECS4810_12MV2, Main power will
     *     supplies the power if its status is ok.
     * ToDo: replace project_name with
     * XX_active_power_main_preferred/XX_active_power_redundant_preferred
     */
    if(status == VAL_swIndivPowerStatus_green)
    {
        if(power == 1)
            power_switch  = 1;
        else
            return TRUE;
    }
    /* One of the power fails:
     *     switch to another power 
     */
    else
    {
        if(power == 1)
            power_switch = 2;
        else if(power == 2)
            return TRUE;
    }


    trap_data.trap_type                    = TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP;
    trap_data.community_specified          = FALSE;

    trap_data.u.sw_active_power_change_trap.sw_indiv_power_unit_index               = unit;
    trap_data.u.sw_active_power_change_trap.instance_sw_indiv_power_unit_index[0]   = unit;
    trap_data.u.sw_active_power_change_trap.instance_sw_indiv_power_unit_index[1]   = power_switch;

    trap_data.u.sw_active_power_change_trap.sw_indiv_power_index                    = power_switch;
    trap_data.u.sw_active_power_change_trap.instance_sw_indiv_power_index[0]        = unit;
    trap_data.u.sw_active_power_change_trap.instance_sw_indiv_power_index[1]        = power_switch;

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
#endif

#endif
    return TRUE;
}

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetPowerModuleSetStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power module set status.
 * INPUT  :  unit_id             -- Which unit
 * OUTPUT :  is_status_good_p    -- TRUE: status good. FALSE: status bad
 * RETURN :  None
 * NOTE   :  if DC power module is used on the device, the DC power type
 *           must be the same. Failed to follow this will damage the device.
 *---------------------------------------------------------------------------
 */
static void SYS_ENV_VM_GetPowerModuleSetStatus(UI32_T unit_id, BOOL_T *is_status_good_p)
{
    UI32_T power_index;
    UI32_T found_dc_power_type=SYS_HWCFG_COMMON_POWER_UNKNOWN_MODULE_TYPE;
    BOOL_T dc_power_exists=FALSE;

    /* assume status is good at first
     */
    *is_status_good_p=TRUE;

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("Dump old power status:[index]:power_type\r\n");
        for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
        {
            SYSMGMT_BACKDOOR_Printf("[%lu]%lu ", (unsigned long)power_index, (unsigned long)sys_env_vm_power_type[unit_id-1][power_index]);
        }
        SYSMGMT_BACKDOOR_Printf("\r\n");
    }

    /* get module type
     */
    for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
    {
        /* Do not need to check power type if power not present
         */
        if(sys_env_vm_power_status[unit_id-1][power_index]!=VAL_swIndivPowerStatus_notPresent)
        {
            /* check any dc power exist
             * keep dc power type if found
             */
            if(dc_power_exists==FALSE && SYS_ENV_VM_POWER_TYPE_IS_DC(sys_env_vm_power_type[unit_id-1][power_index])==TRUE)
            {
                dc_power_exists=TRUE;
                found_dc_power_type=sys_env_vm_power_type[unit_id-1][power_index];
            }
            else if(SYS_ENV_VM_POWER_TYPE_IS_DC(sys_env_vm_power_type[unit_id-1][power_index])==TRUE)
            {
                /* if other dc power exist and its type is different from
                 * the first found dc power, module set status is not ok
                 */
                if(sys_env_vm_power_type[unit_id-1][power_index]!=found_dc_power_type)
                {
                    *is_status_good_p=FALSE;
                    break;
                }
            }
        }
    }

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("Dump new power status:[index]:power_type is_status_good=%u\r\n", *is_status_good_p);
        for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
        {
            SYSMGMT_BACKDOOR_Printf("[%lu]%lu ", (unsigned long)power_index, (unsigned long)sys_env_vm_power_type[unit_id-1][power_index]);
        }
        SYSMGMT_BACKDOOR_Printf("\r\n");
    }

    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetPowerType
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get power type.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power index(1 based).
 * OUTPUT  : type   --- VAL_swIndivPowerType_DC_N48
 *                      VAL_swIndivPowerType_DC_P24
 *                      VAL_swIndivPowerType_AC
 *                      VAL_swIndivPowerType_DC_N48_Wrong
 *                      VAL_swIndivPowerType_DC_P24_Wrong
 *                      VAL_swIndivPowerType_none
 *                      VAL_swIndivPowerType_AC_Wrong
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetPowerType(UI32_T unit, UI32_T power, UI32_T *type)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return FALSE;
    }

    if (type == 0)
    {
        return FALSE;
    }

    /* action
     */
    *type = sys_env_vm_power_type[unit-1][power-1];

    return TRUE;

}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetPowerType
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set power type.
 * INPUT   : unit   --- Which unit.
 *           power  --- Which power index(1 based).
 *           type   --- VAL_swIndivPowerType_DC_N48
 *                      VAL_swIndivPowerType_DC_P24
 *                      VAL_swIndivPowerType_AC
 *                      VAL_swIndivPowerType_none
 * OUTPUT  :
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetPowerType(UI32_T unit, UI32_T power, UI32_T type)
{
    UI32_T power_index;
    BOOL_T power_module_set_status_good;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        return FALSE;
    }

    if (type != VAL_swIndivPowerType_DC_N48      &&
        type != VAL_swIndivPowerType_DC_P24      &&
        type != VAL_swIndivPowerType_AC          &&
        type != VAL_swIndivPowerType_none
       )
    {
        return FALSE;
    }

    /* nothing to do if the power type to be set is the
     * same with the current type
     */
    if (sys_env_vm_power_type[unit-1][power-1] == type)
    {
        return TRUE;
    }

    /* update sys_env_vm_power_type
     */
    sys_env_vm_power_type[unit-1][power-1]=type;

    /* get power module set status
     */
    SYS_ENV_VM_GetPowerModuleSetStatus(unit, &power_module_set_status_good);

    if(power_module_set_status_good==TRUE)
    {
        for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
        {
            switch(sys_env_vm_power_type[unit-1][power-1])
            {
                case VAL_swIndivPowerType_DC_N48_Wrong:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_DC_N48;
                    break;
                case VAL_swIndivPowerType_DC_P24_Wrong:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_DC_P24;
                    break;
                case VAL_swIndivPowerType_AC_Wrong:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_AC;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
        {
            switch(sys_env_vm_power_type[unit-1][power-1])
            {
                case VAL_swIndivPowerType_DC_N48:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_DC_N48_Wrong;
                    break;
                case VAL_swIndivPowerType_DC_P24:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_DC_P24_Wrong;
                    break;
                case VAL_swIndivPowerType_AC_Wrong:
                    sys_env_vm_power_type[unit-1][power-1]=VAL_swIndivPowerType_AC_Wrong;
                    break;
                default:
                    break;
            }
        }
    }

    sys_env_vm_power_type[unit-1][power-1] = type;
    return TRUE;

}
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan status.
 * INPUT   : unit   --- Which unit.
 *           fan   ---  Which fan.
 * OUTPUT  : status --- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanStatus(UI32_T unit, UI32_T fan, UI32_T *status)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    if (status == 0)
    {
        return FALSE;
    }

    /* action
     */
    *status = sys_env_vm_fan_status[unit-1][fan-1];

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("\r\n SYS_ENV_VM_GetFanStatus : unit %lu,fan %lu,status %lu", (unsigned long)unit, (unsigned long)fan, (unsigned long)*status);
    }

    return TRUE;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetFanStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set fan status and send trap if iy is
 *           necessary.
 * INPUT   : unit   --- Which unit.
 *           port   --- Which port.
 *           status --- VAL_switchFanStatus_ok/VAL_switchFanStatus_failure
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetFanStatus(UI32_T unit, UI32_T fan, UI32_T status)
{
    TRAP_EVENT_TrapData_T       trap_data;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    if (status != VAL_switchFanStatus_ok && status != VAL_switchFanStatus_failure)
    {
        return FALSE;
    }

    /* action
     */
    sys_env_vm_fan_status[unit-1][fan-1] = status;

    /* incement fan fail counter */
    if(status == VAL_switchFanStatus_failure)
    {
        sys_env_vm_fan_fail_counter[unit-1][fan-1]++;
    }
    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("\r\n SYS_ENV_VM_SetFanStatus : unit %lu,fan %lu,status %lu",(unsigned long)unit,(unsigned long)fan,(unsigned long)status);
    }

    /* send trap
     */

    trap_data.community_specified = FALSE;

    if (status == VAL_switchFanStatus_ok)
    {
        /* fail to ok
         */
        trap_data.trap_type = TRAP_EVENT_FAN_RECOVER;
    }
    else
    {
        /* ok to fail
         */
        trap_data.trap_type = TRAP_EVENT_FAN_FAILURE;
     }

    trap_data.u.fan_trap.instance_trap_unit_index[0] = unit;
    trap_data.u.fan_trap.instance_trap_unit_index[1] = fan;
    trap_data.u.fan_trap.trap_unit_index = unit;

    trap_data.u.fan_trap.instance_trap_fan_index[0] = unit;
    trap_data.u.fan_trap.instance_trap_fan_index[1] = fan;
    trap_data.u.fan_trap.trap_fan_index  = fan;

#if (SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
    SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanFailCounter
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan fail counter.
 * INPUT   : unit   --- Which unit.
 *           fan   --- Which fan.
 *           *counter   --- buffer for fan fail counter
 * OUTPUT  : *counter -- fan fail counter
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanFailCounter(UI32_T unit, UI32_T fan, UI32_T *fan_fail_counter)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    if (fan_fail_counter == 0)
{
        return FALSE;
    }

    /* action
     */
    *fan_fail_counter = sys_env_vm_fan_fail_counter[unit-1][fan-1];
    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("\r\n SYS_ENV_VM_GetFanFailCounter : unit %lu,fan %lu,fail %lu",(unsigned long)unit,(unsigned long)fan,(unsigned long)sys_env_vm_fan_fail_counter[unit-1][fan-1]);
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetFanSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get fan speed.
 * INPUT   : unit      --- Which unit.
 *           fan       --- Which fan.
 * OUTPUT  : fan_speed_p --- Fan speed.
 * RETURN  : TRUE   -  Success
 *           FALSE  -  Fail
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetFanSpeed(UI32_T unit, UI32_T fan, UI32_T *fan_speed_p)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    if (fan_speed_p == NULL)
    {
        return FALSE;
    }

    /* action
     */
    *fan_speed_p = sys_env_vm_fan_oper_speed[unit-1][fan-1];

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("SYS_ENV_VM_GetFanSpeed : unit %lu,fan %lu,speed %lu\r\n",(unsigned long)unit,(unsigned long)fan,(unsigned long)*fan_speed_p);
    }

    return TRUE;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetFanSpeed
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set fan speed
 * INPUT   : unit      --- Which unit.
 *           fan       --- Which fan.
 *           fan_speed --- Fan speed.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetFanSpeed(UI32_T unit, UI32_T fan, UI32_T fan_speed)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (fan == 0 || fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        return FALSE;
    }

    /* action
     */
    sys_env_vm_fan_oper_speed[unit-1][fan-1] = fan_speed;

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("SYS_ENV_VM_SetFanSpeed : unit %lu,fan %lu,speed %lu\r\n",(unsigned long)unit,(unsigned long)fan,(unsigned long)fan_speed);
    }

    return TRUE;
}

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYS_ENV_VM_SetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to Set fan speed full database
 * INPUT:   mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYS_ENV_VM_SetFanSpeedForceFull(BOOL_T mode)
{
    /* semantic check
     */
    if (sys_env_vm_fan_speed_force_full == mode)
    {
        return TRUE;
    }

    sys_env_vm_fan_speed_force_full = mode;

    return TRUE;
}

/*------------------------------------------------------------------------ 
 * FUNCTION NAME: SYS_ENV_VM_GetFanSpeedForceFull
 *------------------------------------------------------------------------ 
 * PURPOSE: This routine is used to Get fan speed full database
 * INPUT:   None.
 * OUTPUT:  mode:  TRUE  -- enabled.
 *                 FALSE -- disabled.
 * RETURN:  TRUE  : success
 *          FALSE : fail
 * NOTES:   None.
 *------------------------------------------------------------------------ 
 */
BOOL_T SYS_ENV_VM_GetFanSpeedForceFull(BOOL_T *mode)
{
    /* semantic check
     */
    if (mode == 0)
    {
        return FALSE;
    }

    /* action
     */
    *mode = sys_env_vm_fan_speed_force_full;

    return TRUE;
}
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetThermalStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get thermal status of the specified
 *           thermal sensor.
 * INPUT   : unit           -- Which unit.
 *           thermal_idx    -- Which thermal sensor. (starts from 1)
 * OUTPUT  : temperature_p  -- Thermal temperature
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. No critical section protection, because semaphore exists in SYS_MGR.
 *           2. Thermal status is a generic name for all of the information
 *              which might have in a thermal sensor. For now, thermal status
 *              only contains temperature data.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetThermalStatus(UI32_T unit, UI32_T thermal_idx, I32_T *temperature_p)
{
    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (thermal_idx == 0 || thermal_idx > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if (temperature_p == NULL)
    {
        return FALSE;
    }

    /* action
     */
    *temperature_p = sys_env_vm_thermal_temperature[unit-1][thermal_idx-1];

    if(sys_env_debug_mode == TRUE)
    {
        SYSMGMT_BACKDOOR_Printf("\r\n SYS_ENV_VM_GetThermalStatus : unit %lu,thermal_idx %lu,temp %ld",(unsigned long)unit,(unsigned long)thermal_idx,(long)*temperature_p);
    }

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetThermalStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set thermal status and send trap if it is
 *           necessary.
 * INPUT   : unit        -- Which unit.
 *           thermal_idx -- Which thermal sensor. (starts from 1)
 *           temperature -- Thermal temperature
 * OUTPUT  : abnormal_status_changed_p
 *                       -- TRUE if the abnormal status is changed
 *                          FALSE if the abnormal status is not changed
 *           is_abnormal_p
 *                       -- TRUE if the temperature got from the specified
 *                          thermal sensor falls in the abnormal region
 *                          (overheating or undercooling)
 *                          FALSE if the temperature got from the specified
 *                          thermal sensor falls in the normal region
 * RETURN  : TRUE/FALSE
 * NOTE    : 1. No critical section protection, because semaphore exists in SYS_MGR.
 *           2. Thermal status is a generic name for all of the information
 *              which might have in a thermal sensor. For now, thermal status
 *              only contains temperature data.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetThermalStatus(UI32_T unit, UI32_T thermal_idx, I32_T temperature,
    BOOL_T *abnormal_status_changed_p, BOOL_T *is_abnormal_p)
{
    I32_T temperature_old;

    /* semantic check
     */
    if (unit == 0 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }

    if (thermal_idx == 0 || thermal_idx > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
    {
        return FALSE;
    }

    if (abnormal_status_changed_p==NULL || is_abnormal_p==NULL)
    {
        return FALSE;
    }

    /* action
     */
    temperature_old = sys_env_vm_thermal_temperature[unit-1][thermal_idx-1];
    sys_env_vm_thermal_temperature[unit-1][thermal_idx-1] = temperature;

    /* send trap
     */
    SYS_ENV_VM_ThermalTakeAction(unit, thermal_idx, temperature_old, temperature,
        abnormal_status_changed_p, is_abnormal_p);
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_InitThermalActionTable
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to initial thermal action table.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_InitThermalActionTable(void)
{
    UI32_T unit_index;
    UI32_T thermal_index;

    if(FALSE==sys_env_vm_thermal_action_table.is_init)
    {
        L_SORT_LST_Create(&(sys_env_vm_thermal_action_table.sorted_list),
                          SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT,
                          sizeof(SYS_ENV_VM_SwitchThermalActionEntry_T),
                          SYS_ENV_VM_ThermalActionSortedListCmpFunc);
        sys_env_vm_thermal_action_table.is_init=TRUE;
    }
    else
    {
        L_SORT_LST_Delete_All(&(sys_env_vm_thermal_action_table.sorted_list));
    }

    /* initialize thermal upper and lower threshold entries
     */
    {
        I8_T thermal_threshold_up_array[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT]={0};
        I8_T thermal_threshold_down_array[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT]={0};

        if (SYSDRV_GetThermalThresholds(thermal_threshold_down_array, thermal_threshold_up_array)==TRUE)
        {

            for(unit_index = 0; STKTPLG_POM_GetNextUnit(&unit_index) == TRUE;)
            {
                for(thermal_index = 1; thermal_index <= SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; thermal_index++)
                {
                    SYS_ENV_VM_SwitchThermalActionEntry_T new_entry;


                    new_entry.rising_threshold = thermal_threshold_up_array[thermal_index-1];
                    new_entry.falling_threshold = thermal_threshold_down_array[thermal_index-1];

                    new_entry.unit_index = unit_index;
                    new_entry.thermal_index = thermal_index;
                    new_entry.action_index = 1;

                    //new_entry.action = L_CVRT_SNMP_BIT_VALUE_32(SYS_DFLT_THERMAL_ACTION);
                    new_entry.action = SYS_DFLT_THERMAL_ACTION;
                    new_entry.status = VAL_switchThermalActionStatus_valid;
                    new_entry.is_default_entry = TRUE;

                    if(FALSE==L_SORT_LST_Set(&(sys_env_vm_thermal_action_table.sorted_list), &new_entry))
                    {
                        /* critical error, use printf to show error message
                         */
                        printf("%s(): L_SORT_LST_Set fail\r\n", __FUNCTION__);
                    }
                }
            }
        }
    }

    return;
}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get switch thermal action entry.
 * INPUT   : *action_entry -- output buffer of action entry
 * OUTPUT  : *action_entry -- action entry
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 *           Entry key are as follow:
 *           1.action_entry->unit_index
 *           2.action_entry->thermal_index
 *           3.action_entry->action_index
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry)
{
    if(action_entry == NULL)
    {
        return FALSE;
    }

    return L_SORT_LST_Get(&(sys_env_vm_thermal_action_table.sorted_list), action_entry);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetNextSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get next switch thermal action entry.
 * INPUT   : *action_entry -- output buffer of next action entry with key
 * OUTPUT  : *action_entry -- next action entry
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 *           Entry key are as follow:
 *           1.action_entry->unit_index
 *           2.action_entry->thermal_index
 *           3.action_entry->action_index
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_GetNextSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry)
{
    if(action_entry == NULL)
    {
        return FALSE;
    }
    /* Get first entry if unit_index, thermal_index and action_index are all zeros.
     */
    if((action_entry->unit_index == 0) &&
       (action_entry->thermal_index == 0) &&
       (action_entry->action_index == 0))
    {
        return L_SORT_LST_Get_1st(&(sys_env_vm_thermal_action_table.sorted_list), action_entry);
    }

    return L_SORT_LST_Get_Next(&(sys_env_vm_thermal_action_table.sorted_list), action_entry);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action entry.
 * INPUT   : *action_entry -- pointer of action entry
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionEntry(SYS_ENV_VM_SwitchThermalActionEntry_T *action_entry)
{
    return L_SORT_LST_Set(&(sys_env_vm_thermal_action_table.sorted_list), action_entry);
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionRisingThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action rising threshold.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           rising_threshold -- rising threshold
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionRisingThreshold(UI32_T unit_index, UI32_T thermal_index,
                                                        UI32_T action_index, UI32_T rising_threshold)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T entry;

    entry.unit_index = unit_index;
    entry.thermal_index = thermal_index;
    entry.action_index = action_index;

    if(TRUE==L_SORT_LST_Get(&(sys_env_vm_thermal_action_table.sorted_list), &entry))
    {
        if(entry.is_default_entry == TRUE)
        {
            return FALSE;
        }

        entry.rising_threshold = rising_threshold;
        return L_SORT_LST_Set(&(sys_env_vm_thermal_action_table.sorted_list), &entry);
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionFallingThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action falling threshold.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           falling_threshold -- falling threshold
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionFallingThreshold(UI32_T unit_index, UI32_T thermal_index,
                                                         UI32_T action_index, UI32_T falling_threshold)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T entry;

    entry.unit_index = unit_index;
    entry.thermal_index = thermal_index;
    entry.action_index = action_index;

    if(TRUE==L_SORT_LST_Get(&(sys_env_vm_thermal_action_table.sorted_list), &entry))
    {
        if(entry.is_default_entry == TRUE)
        {
            return FALSE;
        }

        entry.falling_threshold = falling_threshold;
        return L_SORT_LST_Set(&(sys_env_vm_thermal_action_table.sorted_list), &entry);
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionAction
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action action.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           action -- action
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionAction(UI32_T unit_index, UI32_T thermal_index,
                                               UI32_T action_index, UI32_T action)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T entry;

    entry.unit_index = unit_index;
    entry.thermal_index = thermal_index;
    entry.action_index = action_index;

    if(TRUE==L_SORT_LST_Get(&(sys_env_vm_thermal_action_table.sorted_list), &entry))
    {
        entry.action = action;
        return L_SORT_LST_Set(&(sys_env_vm_thermal_action_table.sorted_list), &entry);
    }

    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetSwitchThermalActionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set switch thermal action status.
 * INPUT   : unit_index -- unit index
 *           thermal_index -- thermal index
 *           action_index -- action index
 *           status -- status
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : No critical section protection, because semaphore exists in SYS_MGR.
 * -------------------------------------------------------------------------*/
BOOL_T SYS_ENV_VM_SetSwitchThermalActionStatus(UI32_T unit_index, UI32_T thermal_index,
                                               UI32_T action_index, UI32_T status)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T entry;

    if(status == VAL_switchThermalActionStatus_invalid)
    {
        entry.unit_index = unit_index;
        entry.thermal_index = thermal_index;
        entry.action_index = action_index;

        return L_SORT_LST_Delete(&(sys_env_vm_thermal_action_table.sorted_list), &entry);
    }
    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_ThermalTakeAction
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to check if need to take action,if yes, take it
 * INPUT   : unit_index              -- unit index
 *           thermal_index           -- thermal sensor index
 *           thermal_temperature_old -- previous temperature got from the given
 *                                      thermal sensor
 *           thermal_temperature_new -- current temperature got from the given
 *                                      thermal sensor
 * OUTPUT  : abnormal_status_changed_p
 *                                   -- TRUE if the abnormal status is changed
 *                                      FALSE if the abnormal status is not changed
 *           is_abnormal_p
 *                                   -- TRUE if the temperature got from the specified
 *                                      thermal sensor falls in the abnormal region
 *                                      (overheating or undercooling)
 *                                      FALSE if the temperature got from the specified
 *                                      falls in the normal region.
 * RETURN  : None
 * NOTE    : This function will not validate the input argument, the sanity check
 *           for the input arguments must be done by the caller.
 * -------------------------------------------------------------------------*/
static void SYS_ENV_VM_ThermalTakeAction(UI32_T unit_index, UI32_T thermal_index,
    I32_T thermal_temperature_old, I32_T thermal_temperature_new,
    BOOL_T *abnormal_status_changed_p, BOOL_T *is_abnormal_p)
{
    SYS_ENV_VM_SwitchThermalActionEntry_T entry;
    UI32_T                                trap_style;
    ThermalStatus_T                       old_thermal_status=sys_env_vm_thermal_status[unit_index-1][thermal_index-1];
    ThermalStatus_T                       new_thermal_status;

    *abnormal_status_changed_p=FALSE;

    /* below setting is for get the first entry with
     * the specified unit_index and thermal_index.
     * (action_index can't be zero)
     */
    entry.unit_index = unit_index;
    entry.thermal_index = thermal_index;
    entry.action_index = 0;

    if(FALSE==L_SORT_LST_Get_Next(&(sys_env_vm_thermal_action_table.sorted_list), &entry))
    {
        return;
    }

    if( entry.action == VAL_switchThermalActionAction_trap)
    {
        trap_style = TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP;
    }
    else
    {
        trap_style = TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY;
    }

    /* evaluate new thermal status
     */
    new_thermal_status = old_thermal_status;
    if ((thermal_temperature_old <= entry.rising_threshold) &&
        (thermal_temperature_new >  entry.rising_threshold))
    {
        new_thermal_status = THERMAL_STATUS_RISING;
        *is_abnormal_p=TRUE;
    }
    else if ((thermal_temperature_old >= entry.falling_threshold) &&
             (thermal_temperature_new <  entry.falling_threshold))
    {
        new_thermal_status = THERMAL_STATUS_FALLING;
        *is_abnormal_p=FALSE;
    }

    sys_env_vm_thermal_status[unit_index-1][thermal_index-1]=new_thermal_status;

    /* We'll send thermal action trap if thermal status changes
     */
    if (new_thermal_status != old_thermal_status)
    {
        *abnormal_status_changed_p=TRUE;
        while(1)
        {
            if (new_thermal_status == THERMAL_STATUS_RISING)
            {
                /* send rising trap */
                TRAP_EVENT_TrapData_T data;

                data.community_specified = FALSE;
                data.trap_type = TRAP_EVENT_THERMAL_RISING;
                data.u.thermal_rising.instance_switchThermalTempValue[0] = unit_index;
                data.u.thermal_rising.instance_switchThermalTempValue[1] = thermal_index;
                data.u.thermal_rising.switchThermalTempValue = thermal_temperature_new;
                data.u.thermal_rising.instance_switchThermalActionRisingThreshold[0] = unit_index;
                data.u.thermal_rising.instance_switchThermalActionRisingThreshold[1] = thermal_index;
                data.u.thermal_rising.instance_switchThermalActionRisingThreshold[2] = entry.action_index;
                data.u.thermal_rising.switchThermalActionRisingThreshold = entry.rising_threshold;

#if (SYS_CPNT_TRAPMGMT == TRUE)
                TRAP_MGR_ReqSendTrapOptional(&data, trap_style);
#else
                SNMP_PMGR_ReqSendTrapOptional(&data, trap_style);
#endif

            }
            else if (new_thermal_status == THERMAL_STATUS_FALLING)
            {
                /* send falling trap */

                TRAP_EVENT_TrapData_T data;

                data.community_specified = FALSE;
                data.trap_type = TRAP_EVENT_THERMAL_FALLING;
                data.u.thermal_falling.instance_switchThermalTempValue[0] = unit_index;
                data.u.thermal_falling.instance_switchThermalTempValue[1] = thermal_index;
                data.u.thermal_falling.switchThermalTempValue = thermal_temperature_new;
                data.u.thermal_falling.instance_switchThermalActionFallingThreshold[0] = unit_index;
                data.u.thermal_falling.instance_switchThermalActionFallingThreshold[1] = thermal_index;
                data.u.thermal_falling.instance_switchThermalActionFallingThreshold[2] = entry.action_index;
                data.u.thermal_falling.switchThermalActionFallingThreshold = entry.falling_threshold;

#if (SYS_CPNT_TRAPMGMT == TRUE)
                TRAP_MGR_ReqSendTrapOptional(&data, trap_style);
#else
                SNMP_PMGR_ReqSendTrapOptional(&data, trap_style);
#endif
            }

            /* try to get next entry with the specified unit_index and thermal_index
             */
            if(FALSE==L_SORT_LST_Get_Next(&(sys_env_vm_thermal_action_table.sorted_list), &entry))
            {
                break;
            }
            else
            {
               if((entry.unit_index =! unit_index) || (entry.thermal_index != thermal_index))
                   break;
            }

        } /* end of while */
    } /* end of if (new_thermal_status != old_thermal_status) */
}

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_GetDebugMode
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to get debug mode for sys_env_vm
 * INPUT   : *mode -- output buffer of debug mode
 * OUTPUT  : *mode -- debug mode
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_GetDebugMode(BOOL_T *mode)
{
    *mode = sys_env_debug_mode;
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SYS_ENV_VM_SetDebugMode
 * -------------------------------------------------------------------------
 * FUNCTION: This routine is used to set debug mode for sys_env_vm
 * INPUT   : mode -- debug mode
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SYS_ENV_VM_SetDebugMode(BOOL_T mode)
{
    sys_env_debug_mode = mode;
    return;
}

/* LOCAL SUBPROGRAM BODIES
 */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - ThermalActionSortedListCmpFunc
 * -------------------------------------------------------------------------
 * FUNCTION: Compare function for sys_env_vm_thermal_action_table.sorted_list
 * INPUT   : inlist_element -- the element in the list
 *           input_element  -- the given element to be compared with
                               inlist_element
 * OUTPUT  :
 * RETURN  :
 *           =0 : equal
 *           >0 : inlist_element > input_element
 *           <0 : inlist_element < input_element
 * NOTE    :
 *           Compare field priority:
 *           unit_index > thermal_index > action_index
 * -------------------------------------------------------------------------*/
static int SYS_ENV_VM_ThermalActionSortedListCmpFunc(void * inlist_element, void * input_element)
{
    int                                   ret;
    SYS_ENV_VM_SwitchThermalActionEntry_T *elm1_p, *elm2_p;

    elm1_p = (SYS_ENV_VM_SwitchThermalActionEntry_T*)inlist_element;
    elm2_p = (SYS_ENV_VM_SwitchThermalActionEntry_T*)input_element;
    if(0 != (ret = (elm1_p->unit_index) - (elm2_p->unit_index)))
    {
        return ret;
    }

    if(0 != (ret = (elm1_p->thermal_index) - (elm2_p->thermal_index)))
    {
        return ret;
    }

    if(0 != (ret=((elm1_p->action_index) - (elm2_p->action_index))))
    {
        return ret;
    }

    return 0;
}
#endif

