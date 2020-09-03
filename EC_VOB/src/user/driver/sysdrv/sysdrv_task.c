/* MODULE NAME:  sysdrv_task.c
 * PURPOSE:
 *     implementation of sysdrv task
 *
 * NOTES:
 *
 * HISTORY
 *    9/13/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "uc_mgr.h"
#include "stktplg_pom.h"
#include "stktplg_board_util.h"
#include "sysrsc_mgr.h"
#include "leaf_sys.h"
#include "phyaddr_access.h"

#include "fs.h"
#include "leddrv.h"
#include "swdrv.h"

#include "syslog_pmgr.h"
#include "sys_module.h"
#include "trap_event.h"

#include "sysdrv.h"
#include "sysdrv_private.h"
#include "sysdrv_task.h"
#include "sysdrv_util.h"
#include "sys_time.h"
#ifdef EIF8X10G
#include "adm_nic.h"
#include <shared/types.h>
#endif

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
#include "dev_swdrv_pmgr.h"
#endif

#include "i2cdrv.h"
#include "i2c.h"
#include "backdoor_mgr.h"
#include "stktplg_board.h"

#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
#include "onlpdrv_fan.h"
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
#include "onlpdrv_psu.h"
#endif

#endif /* #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSDRV_PERIODIC_POLLING_TICKS               250

/* There are many ways to measure the speed of fan, this controller uses the internal clk.
 * the internal clk(48MHz) is fixed once set by the register, it counts how many clk ticks has passed in one revolution
 * therefore, the faster the fan goes, the lesser clk tick recorded in the counter.
 * fan speed can be evaluated :
 * Fan speed = (1.35*10^6)/(clock counter*Divisor)
 * The duty cycle value which relative with voltage can set Fan speed is ranged from 0x00~0xFF.
 * 0x00 is the lowest value , duty cycle is zero; Write 0xFF, duty cycle is 100%.
 * - Ithink_Chen, 07_05_2005.
 */
#define FAN_DIVISOR    2
#define FAN_RPM_FACTOR 1350000

/* SYSDRV_FAN_STATUS_NOT_ACCESSIBLE_REACTION_TIME
 *   in ticks, to indicates how long SYSDRV can determine
 *   fan status is not accessible.
 */
#define SYSDRV_FAN_STATUS_NOT_ACCESSIBLE_REACTION_TIME  (10 * SYS_BLD_TICKS_PER_SECOND)
#define SYSDRV_DFLT_GET_FAN_SPEED_MAX_FAIL_COUNT \
            ((SYSDRV_FAN_STATUS_NOT_ACCESSIBLE_REACTION_TIME / SYSDRV_PERIODIC_POLLING_TICKS) + 1)

/* SYSDRV_FAN_CALIBRATION_MAX_ADJUST_COUNT
 *   The maximum number of count to adjust the duty cycle of a fan in order to
 *   let the fan rotate in the pre-defined range of fan speed.
 */
#define SYSDRV_FAN_CALIBRATION_MAX_ADJUST_COUNT 10

/* SYSDRV_ALARM_OUTPUT_XXX are logical definitions
 * which are irrelevant to hardware settings
 * if the bit value is 1 means alarm output enable
 */
#define SYSDRV_ALARM_OUTPUT_MAJOR_MASK 0x80
#define SYSDRV_ALARM_OUTPUT_MINOR_MASK 0x40
#define SYSDRV_ALARM_OUTPUT_MAJOR SYSDRV_ALARM_OUTPUT_MAJOR_MASK
#define SYSDRV_ALARM_OUTPUT_MINOR SYSDRV_ALARM_OUTPUT_MINOR_MASK

#if (SYS_CPNT_STKTPLG_FAN_DETECT==TRUE) && \
    ((SYS_HWCFG_FAN_CONTROLLER_TYPE!=SYS_HWCFG_FAN_NONE) || \
     (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP!=0))
#define SYSDRV_TASK_HAS_FAN_CONTROLLER TRUE
#else
#define SYSDRV_TASK_HAS_FAN_CONTROLLER FALSE
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#define DBG_MSG printf
#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == FALSE)
#define STKTPLG_BOARD_GetThermalNumber() ({int __ret=SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; __ret;})
#endif

/* semaphore for GPIO
 */
static UI32_T sysdrv_task_sem_id;

#define SYSDRV_TASK_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(sysdrv_task_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYSDRV_TASK_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(sysdrv_task_sem_id)

#define SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(shmem_data_p) ((shmem_data_p)->fan_speed_mode_sm_info.info.avg_tmp)
#define SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(shmem_data_p) ((shmem_data_p)->fan_speed_mode_sm_info.info.fru_cfg)

/* DATA TYPE DECLARATIONS
 */
typedef const UI32_T (*SYSDRV_TASK_FanSpeedLevelToSettingForOneLevelArrayPtr_T)[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT];

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYSDRV_TASK_TaskMain(void);

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
static UI32_T SYSDRV_TASK_GetEffectiveFanSpeedMode(void);
#endif

static void SYSDRV_TASK_GetFanStatus(UI32_T fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]);
static BOOL_T SYSDRV_TASK_DetectFanStatus(void);
static BOOL_T SYSDRV_TASK_FanStateMachine(UI32_T current, UI32_T new);

#if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
static BOOL_T SYSDRV_TASK_GetFanSpeedThresholds(UI32_T fan_speed_mode, UI32_T *upper_limit_p, UI32_T *lower_limit_p);
static void SYSDRV_TASK_FanSpeedCalibration(void);
#endif /* end of (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
#endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */

#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */


#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static BOOL_T SYSDRV_TASK_DetectThermalStatus(void);
#if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE)
static void SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(UI32_T *fan_speed_mode_p);
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine(UI32_T *fan_speed_setting_mode_p);
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine_AvgTmp(UI32_T *fan_speed_setting_mode_p);
#endif
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine_FruCfg(UI32_T *fan_speed_setting_mode_p);
#endif
#endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE) */

#if (SYS_HWCFG_SUPPORT_THERMAL_SHUTDOWN!=FALSE)
static BOOL_T SYSDRV_TASK_SetThermalShutdownThreshold(void);
#endif

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */


#if 0
static void SYSDRV_TASK_DetectXenpakStatus(void);
#endif

static void SYSDRV_TASK_ReStartSystem(UI32_T type);

#if (SYS_CPNT_POWER_DETECT == TRUE)
static void SYSDRV_TASK_DetectPowerStatus(void);
static BOOL_T SYSDRV_TASK_PowerStateMachine(UI32_T pwr_id, UI8_T pre_val, UI8_T now_val, UI32_T *new_status);
#endif

static void SYSDRV_TASK_ClearNMI(void);

#if (SYS_CPNT_ALARM_DETECT == TRUE)
static void SYSDRV_TASK_DetectAlarmStatus(void);
static BOOL_T SYSDRV_TASK_GetMajorAlarmStatus(UI8_T *current_status);
static BOOL_T SYSDRV_TASK_GetMinorAlarmStatus(UI8_T *current_status);
static BOOL_T SYSDRV_TASK_SetAlarmOutputStatusByRegInfo(SYS_HWCFG_AlarmOutputRegInfo_T *reg_info_p, BOOL_T set_alarm);
#if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV!=TRUE)
static BOOL_T SYSDRV_TASK_SetAlarmOutputLed(UI8_T status);
static BOOL_T SYSDRV_TASK_SetAlarmOutputLedByRegInfo(SYS_HWCFG_AlarmOutputLedRegInfo_T *reg_info_p, BOOL_T set_led_on);
#else /* #if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV!=TRUE) */
#define SYSDRV_TASK_SetAlarmOutputLed(status) ({BOOL_T __ret=TRUE; __ret;})
#define SYSDRV_TASK_SetAlarmOutputLedByRegInfo(reg_info_p, set_led_on) ({BOOL_T __ret=TRUE; __ret;})
#endif /* end of #if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV!=TRUE) */
static BOOL_T SYSDRV_TASK_SetI2CReg(SYS_HWCFG_i2cRegInfo_T *reg_info_p, UI32_T mask, UI32_T data);
static BOOL_T SYSDRV_TASK_SetPhyAddrReg(SYS_HWCFG_phyAddrRegInfo_T* reg_info_p, UI32_T mask, UI32_T data);
#endif /* end of #if (SYS_CPNT_ALARM_DETECT == TRUE) */
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
static BOOL_T SYSDRV_TASK_GetAlarmInputStatus(UI8_T *status_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE)
#if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL!=TRUE)
#if   (SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT == 1)
static const UI32_T  fan_full_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {SYS_HWCFG_FAN_SPEED_MAX};
static const UI32_T  fan_mid_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]  = {SYS_HWCFG_FAN_SPEED_MID};
#elif (SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT == 2)
static const UI32_T  fan_full_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX};
static const UI32_T  fan_mid_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]  = {SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID};
#elif (SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT == 3)
static const UI32_T  fan_full_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX};
static const UI32_T  fan_mid_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]  = {SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID};
#elif (SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT == 4)
static const UI32_T  fan_full_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX, SYS_HWCFG_FAN_SPEED_MAX};
static const UI32_T  fan_mid_speed_setting[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]  = {SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID, SYS_HWCFG_FAN_SPEED_MID};
#endif
#else
static UI32_T fan_speed_level_to_setting_ar[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL][SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {{0}};
#endif

/* set running_fan_speed_mode as 0(special initial value)
 * and it will be synced to SYSDRV_Shmem_Data_T.sysdrv_speed_setting_mode
 * afterwards
 */
static UI32_T  running_fan_speed_mode = 0;
#endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE) */

static SYSDRV_Shmem_Data_T *sysdrv_shmem_data_p;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    #if defined(SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_CPLD) || defined(SYS_HWCFG_FAN_FAULT_STATUS_ADDR)
static SYS_TYPE_VAddr_T sys_hwcfg_fan_fault_status_addr;
    #endif
static UI8_T nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;
#endif

#if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT == TRUE)
static UI32_T sysdrv_skip_fan_speed_mode_state_machine = FALSE;
#endif
static UI8_T nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;

static I2C_OpersFuncPtrs_T i2c_op_fn_ptrs =
{
    &I2CDRV_TwsiDataReadWithBusIdx,
    &I2CDRV_TwsiDataWriteWithBusIdx,
    &I2CDRV_SetAndLockMux,
    &I2CDRV_UnLockMux
};

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SYSDRV_TASK_CreateTask
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_TASK_CreateTask(void)
{
    UI32_T ret_value;
    UI32_T thread_id;

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSDRV, &sysdrv_task_sem_id)) != SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__, (unsigned long)ret_value);
    }

    if(SYSFUN_SpawnThread(SYS_BLD_SYSDRV_THREAD_PRIORITY,
                          SYS_BLD_SYSDRV_THREAD_SCHED_POLICY,
                          SYS_BLD_SYSDRV_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SYSDRV_TASK_TaskMain,
                          NULL, &thread_id)!=SYSFUN_OK)
    {
        printf("\r\n%s:Spawn SYSDRV thread fail.", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_SYSDRV, thread_id, SYS_ADPT_SYSDRV_SW_WATCHDOG_TIMER);
#endif

}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: SYSDRV_TASK_TaskMain
 *-----------------------------------------------------------------------------
 * PURPOSE: Create task in SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static void SYSDRV_TASK_TaskMain(void)
{
    UI32_T sysdrv_rcv_event;
    UI32_T timeout;
    void*  timer_id;
    UI32_T sysdrv_event_var;

    /* Put SYSDRV_FAN_CHIP_Init(), SYSDRV_THERMAL_CHIP_Init(),
     * SYSDRV_DetectPeripheralInstall() here because it cannot be
     * called in SYSDRV_InitiateProcessResources().
     * In these three functions will call I2CDRV APIs
     * which will send IPC request to driver group thread, but driver group
     * thread had not been spawned yet when SYSDRV_InitiateProcessResources()
     * is called.
     */
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    nbr_of_fan = STKTPLG_BOARD_GetFanNumber();
    if (nbr_of_fan != 0)
    {

    #if (SYSDRV_TASK_HAS_FAN_CONTROLLER == TRUE)
        if (SYSDRV_FAN_CHIP_Init() != TRUE)
        {
            printf("%s:FAN initial failed.", __FUNCTION__);
        }
        #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE)
        if (STKTPLG_BOARD_GetFanSpeedLevelSetting(fan_speed_level_to_setting_ar)==FALSE)
        {
            printf("%s(%d):STKTPLG_BOARD_GetFanSpeedLevelSetting error.\r\n", __FUNCTION__, __LINE__);
        }
        #endif /* end of #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE) */

    #endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER == TRUE) */

    }
    if (nbr_of_fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        printf("%s(%d):Critical Error! Invalid nbr_of_fan(%hu), max number of fan=%d\r\nSystem Halt.\r\n",
            __FUNCTION__, __LINE__, nbr_of_fan, SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT);

        while(1){;}
    }

#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC != TRUE) && (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_BID != TRUE)
    SYSDRV_DetectFanControllerType(&(sysdrv_shmem_data_p->fan_type));
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    nbr_of_thermal = STKTPLG_BOARD_GetThermalNumber();
    if ((nbr_of_thermal != 0) && (SYSDRV_THERMAL_CHIP_Init() != TRUE))
    {
        printf("%s:Thermal initial failed.", __FUNCTION__);
    }
#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */
    SYSDRV_DetectPeripheralInstall();

    /* initialize things that will be used in SYSDRV_TASK
     */
    sysdrv_shmem_data_p = (SYSDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYSDRV_SHMEM_SEGID);

    if(sysdrv_shmem_data_p==NULL)
    {
        printf("\r\n%s:Failed to get shared memory ptr", __FUNCTION__);
        return;
    }

    sysdrv_shmem_data_p->sys_drv_task_id = SYSFUN_TaskIdSelf();

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    if(nbr_of_fan != 0)
    {
#if defined(SYS_HWCFG_FAN_FAULT_STATUS_ADDR)
        if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_FAN_FAULT_STATUS_ADDR, &sys_hwcfg_fan_fault_status_addr))
        {
            printf("\r\n%s:Failed to access SYS_HWCFG_FAN_FAULT_STATUS_ADDR", __FUNCTION__);
            return;
        }
#endif /* end of #if defined(SYS_HWCFG_FAN_FAULT_STATUS_ADDR) */
    }
#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if ((SYS_CPNT_POWER_DETECT == TRUE) && (SYS_CPNT_SYSDRV_USE_ONLP == TRUE))
    ONLPDRV_PSU_Init();
#endif

#if (SYS_CPNT_SYSLOG==TRUE)
    SYSLOG_PMGR_InitiateProcessResource();
#endif

    sysdrv_event_var = 0;
    timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(timer_id, SYSDRV_PERIODIC_POLLING_TICKS, SYSDRV_EVENT_TIMER);

#if (SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_HWCFG_SUPPORT_THERMAL_SHUTDOWN!=FALSE)
    if (TRUE != SYSDRV_TASK_SetThermalShutdownThreshold())
        perror("\r\nSYSDRV_TASK_SetThermalShutdownThreshold Error\n");
#endif

    while(TRUE)
    {

        if (sysdrv_event_var != 0)
        {
            timeout = SYSFUN_TIMEOUT_NOWAIT;
        }
        else
        {
            timeout = SYSFUN_TIMEOUT_WAIT_FOREVER;
        }

        SYSFUN_ReceiveEvent (SYSDRV_EVENT_PREPARE_COLD_START_FOR_RELOAD
                             | SYSDRV_EVENT_PREPARE_WARM_START_FOR_RELOAD
                             | SYSDRV_EVENT_TIMER | SYSDRV_EVENT_ENTER_TRANSITION
                             | SYSDRV_EVENT_COLD_START_FOR_RELOAD
                             | SYSDRV_EVENT_WARM_START_FOR_RELOAD
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                             | SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
                             ,SYSFUN_EVENT_WAIT_ANY,
                             timeout,
                             &sysdrv_rcv_event);

        sysdrv_event_var |= sysdrv_rcv_event;

        /* slaves will never recv "prepare" msg
         * so we reload directly
         */
        if (sysdrv_event_var & SYSDRV_EVENT_COLD_START_FOR_RELOAD)
        {
            SYSDRV_TASK_ReStartSystem(SYS_VAL_COLD_START_FOR_RELOAD);
        }

        if (sysdrv_event_var & SYSDRV_EVENT_WARM_START_FOR_RELOAD)
        {
            SYSDRV_TASK_ReStartSystem(SYS_VAL_WARM_START_FOR_RELOAD);
        }

        /* Re-start system: 1st precedence
         */
        if (sysdrv_event_var & (SYSDRV_EVENT_PREPARE_WARM_START_FOR_RELOAD
                                | SYSDRV_EVENT_PREPARE_COLD_START_FOR_RELOAD))
        {
            UI32_T type;
            UI32_T ret;

            /* 20s == 200ms
             * use SYSFUN_MSecondToTick(ms) to convert ms to tick
             */
            timeout = SYSFUN_MSecondToTick(200);

            /* for debug SYSFUN_MSecondToTick
             */
            /*printf("%s:timeout=%dticks\r\n", __FUNCTION__, timeout);*/

            if (sysdrv_event_var & SYSDRV_EVENT_PREPARE_WARM_START_FOR_RELOAD)
                type = SYS_VAL_WARM_START_FOR_RELOAD;
            else
                type = SYS_VAL_COLD_START_FOR_RELOAD;

            ret = SYSFUN_ReceiveEvent (SYSDRV_EVENT_COLD_START_FOR_RELOAD | SYSDRV_EVENT_WARM_START_FOR_RELOAD,
                                       SYSFUN_EVENT_WAIT_ANY,
                                       timeout,
                                       &sysdrv_rcv_event);

#if (SYS_CPNT_STACKING == TRUE)
            if (ret == SYSFUN_RESULT_TIMEOUT)
            {
                //printf("Timeout, send reload to slave\r\n");
                if (type == SYS_VAL_WARM_START_FOR_RELOAD)
                {
                    SYSDRV_WarmStartSystemForTimeout();
                }
                else if (type == SYS_VAL_COLD_START_FOR_RELOAD)
                {
                    SYSDRV_ColdStartSystemForTimeout();
                }
            }
#endif

            //else if (ret == SYSFUN_OK)
            //    printf("%s:Recv \"RELOAD\" event and reload now\r\n", __FUNCTION__);
            //else
            //    printf("%s:Recv \"RELOAD\" event error\r\n", __FUNCTION__);

            SYSDRV_TASK_ReStartSystem(type);
        }

        /* Topology change: 2nd precedence
         */
        if (SYS_TYPE_STACKING_TRANSITION_MODE == SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p))
        {
            if (sysdrv_event_var & SYSDRV_EVENT_ENTER_TRANSITION)
            {
                sysdrv_shmem_data_p->is_transition_done = TRUE;
            }
            sysdrv_event_var = 0;
            continue;
        }

        /* System state: think that if system is not in
         * static state, detection doesn't mean any thing.
         *
         * Another reason that needs to wait for provision complete here:
         *     SYSDRV(Driver layer) and SYS_MGMT(Core layer) will clear the
         *     database when enter transition mode. If SYSDRV sends syscallback
         *     message for the measured value to SYS_MGMT before it enters
         *     operationable state(i.e. after enter master mode), the syscallback
         *     message will not be processed by SYS_MGMT.
         */
        if (FALSE == sysdrv_shmem_data_p->sysdrv_is_provision_complete)
        {
            sysdrv_event_var = 0;
            continue;
        }
        else
        {
            /* Environment detection: 3rd precedence
             */
            if (sysdrv_event_var & SYSDRV_EVENT_TIMER)
            {
#if (SYS_CPNT_POWER_DETECT == TRUE)
                SYSDRV_TASK_DetectPowerStatus();
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
                if (nbr_of_thermal != 0)
                {
                    SYSDRV_TASK_DetectThermalStatus();
                }
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
                if(nbr_of_fan != 0)
                {
                    SYSDRV_TASK_DetectFanStatus();
                }
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
                SYSDRV_TASK_DetectAlarmStatus();
#endif

                sysdrv_event_var ^= SYSDRV_EVENT_TIMER;
            }
        }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(sysdrv_event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SYSDRV);
            sysdrv_event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    } /* end of while(TRUE) */
}

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
/* FUNCTION NAME: SYSDRV_TASK_GetEffectiveFanSpeedMode
 * PURPOSE: Get current effective fan speed mode
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  One of the enum value except SYSDRV_FAN_SPEED_MODE_NBR which are
 *          defined in SYSDRV_FanSpeed_T will be returned.
 * NOTES:   None.
 */
static UI32_T SYSDRV_TASK_GetEffectiveFanSpeedMode(void)
{
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    if (SYSDRV_GetFanSpeedForceFull()==TRUE)
        return SYSDRV_GetFanFullSpeedVal();
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */
    return sysdrv_shmem_data_p->sysdrv_speed_setting_mode;
}
#endif

#if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
/* FUNCTION NAME: SYSDRV_TASK_GetFanSpeed
 * PURPOSE: This routine is used to get fan speed from fan controller.
 * INPUT:   None.
 * OUTPUT:  fan_speed  -  fan speed of each fan
 * RETURN:  TRUE  -  Success
 *          FALSE -  Failed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_GetFanSpeed(UI32_T fan_speed[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT])
{
    static UI16_T sysdrv_get_fan_speed_fail_count=0;

    UI8_T i;

    for (i=0; i<nbr_of_fan; i++)
    {
        if(SYSDRV_FAN_GetSpeedInRPM(i+1, &(fan_speed[i])) != TRUE)
        {
            sysdrv_get_fan_speed_fail_count++;

            /* to allow temporary fan failure
             */
            if (sysdrv_get_fan_speed_fail_count < SYSDRV_DFLT_GET_FAN_SPEED_MAX_FAIL_COUNT)
            {
                /* suppose that fan failure is temporary and is recoverable,
                 * skip running fan speed mode state machine in
                 * SYSDRV_TASK_DetectThermalStatus until fan recover.
                 */
                sysdrv_skip_fan_speed_mode_state_machine = TRUE;
            }
            else
            {
                /* suppose that fan is out of control and is not recoverable,
                 * still try to run fan speed mode state machine
                 */
                sysdrv_skip_fan_speed_mode_state_machine = FALSE;
            }

            if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s:sysdrv_get_fan_speed_fail_count=%hu fan %hu\r\n", __FUNCTION__,
                    sysdrv_get_fan_speed_fail_count, i+1);
            }

            return FALSE;
        }
        else
        {
            /* fan is ok, reset fail count.
             */
            sysdrv_get_fan_speed_fail_count = 0;
            sysdrv_skip_fan_speed_mode_state_machine = FALSE;

            if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s:fan speed=%lu fan %hu\r\n", __FUNCTION__,
                    (unsigned long)fan_speed[i], i+1);
            }


        }
    }


    return TRUE;
}

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_IsFanCanCalibrate
 * PURPOSE: To get the fan speed calibration status.
 * INPUT:   fan_idx  -  which fan (starts from 1)
 * OUTPUT:  None.
 * RETURN:  TRUE  -  The fan speed of the specified fan index is still allowed
 *                   to be calibrated.
 *          FALSE -  The fan speed of the specified fan index is not allowed
 *                   to be calibrated any more.
 * NOTES:   None.
 */
static inline BOOL_T SYSDRV_TASK_IsFanCanCalibrate(UI8_T fan_idx)
{
    BOOL_T ret=FALSE;

    if (sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter[sysdrv_shmem_data_p->sysdrv_speed_setting_mode][fan_idx-1]<SYSDRV_FAN_CALIBRATION_MAX_ADJUST_COUNT)
        ret=TRUE;

    return ret;
}
#else /* #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
/* FUNCTION NAME: SYSDRV_TASK_IsFanCanCalibrate
 * PURPOSE: To get the fan speed calibration status.
 * INPUT:   fan_idx  -  which fan
 * OUTPUT:  None.
 * RETURN:  TRUE  -  The fan speed of the specified fan index is still allowed
 *                   to be calibrated.
 *          FALSE -  The fan speed of the specified fan index is not allowed
 *                   to be calibrated any more.
 * NOTES:   None.
 */
static inline BOOL_T SYSDRV_TASK_IsFanCanCalibrate(UI8_T fan_idx)
{
    return FALSE;
}
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */

#endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)

/* FUNCTION NAME: SYSDRV_TASK_GetFanStatus
 * PURPOSE: This routine is used to get fan status(OK or failed)
 * INPUT:   None.
 * OUTPUT:  fan_status[] - The status of the fans will be put in elements of the
 *                         array. The possible values of the status is shown below:
 *                           VAL_switchFanStatus_ok
 *                           VAL_switchFanStatus_failure
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_GetFanStatus(UI32_T fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT])
{
    UI32_T sysdrv_speed_lower_limit=0, sysdrv_speed_upper_limit=0;
    UI32_T i, speed_setting_mode;

    /* skip checking fan status if the fan speed in transistion counter
     * is not zero
     */
    if (sysdrv_shmem_data_p->fan_speed_in_transition_counter!=0)
    {
        /* assumes all fans are good
         */
        for (i=0; i<nbr_of_fan; i++)
        {
            fan_status[i] = VAL_switchFanStatus_ok;
        }

        return;
    }

    speed_setting_mode = running_fan_speed_mode;

    switch ( speed_setting_mode )
    {
        case SYSDRV_FAN_FULL_SPEED:
        case SYSDRV_FAN_MID_SPEED:
            if (SYSDRV_TASK_GetFanSpeedThresholds(speed_setting_mode,
                &sysdrv_speed_upper_limit, &sysdrv_speed_lower_limit) == FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d):Failed to get upper and lower threshold for fan speed mode %lu\r\n",
                    __FUNCTION__, __LINE__, (unsigned long)speed_setting_mode);

                for (i=0; i<nbr_of_fan; i++)
                {
                    fan_status[i] = VAL_switchFanStatus_failure;
                }
                return;

            }
            break;

        case SYSDRV_FAN_PREPARE_TRANSITION_SPEED:
        {
            /* do not check fan speed in transition speed mode
             * assumes fan status is good in this speed mode
             */
            for (i=0; i<nbr_of_fan; i++)
            {
                fan_status[i] = VAL_switchFanStatus_ok;
            }
            return;
        }

        default: /* should not reach here */
            BACKDOOR_MGR_Printf("%s(%d):Invalid speed setting mode=%lu\r\n",
                __FUNCTION__, __LINE__, (unsigned long)speed_setting_mode);
            break;
    }

    for (i=0; i<nbr_of_fan; i++)
    {
        if ( (sysdrv_shmem_data_p->sysdrv_fan_speed[i] >= sysdrv_speed_lower_limit) &&
             (sysdrv_shmem_data_p->sysdrv_fan_speed[i] <= sysdrv_speed_upper_limit) )
        {
            /* speed within range, it's normal
             */

            fan_status[i] = VAL_switchFanStatus_ok;
        }
        else
        {
           /* speed out of range, abnormal
            * if the fan still can be calibrated, treat it as normal.
            */
            if ((sysdrv_shmem_data_p->sysdrv_fan_speed[i]!=0) && (SYSDRV_TASK_IsFanCanCalibrate(i+1)==TRUE))
            {
                fan_status[i] = VAL_switchFanStatus_ok;

                if ( SYSDRV_SHOW_DEBUG_MSG_FLAG() == TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d): Assumes Fan OK due to fan speed calibration for fan %lu\r\n",
                        __FUNCTION__, __LINE__, (unsigned long)i+1);
                }
            }
            else
            {
                fan_status[i] = VAL_switchFanStatus_failure;
            }

        }
    }

}
#elif (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) /* #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
/* FUNCTION NAME: SYSDRV_TASK_GetFanStatus
 * PURPOSE: This routine is used to get fan status(OK or failed)
 * INPUT:   None.
 * OUTPUT:  fan_status[] - The status of the fans will be put in elements of the
 *                         array. The possible values of the status is shown below:
 *                           VAL_switchFanStatus_ok
 *                           VAL_switchFanStatus_failure
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_GetFanStatus(UI32_T fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT])
{
    UI32_T fan_status_array_sz=SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;
    BOOL_T rc;

    rc=ONLPDRV_FAN_GetAllFanStatus(&fan_status_array_sz, fan_status);
    if ( (rc==FALSE) && (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE) )
    {
        BACKDOOR_MGR_Printf("%s(%d)ONLPDRV_FAN_GetAllFanStatus error.\r\n",
            __FUNCTION__, __LINE__);
    }
}
#else  /* #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
#if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_STKTPLG_BOARD == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_GetFanStatus
 * PURPOSE: This routine is used to get fan status(OK or failed)
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_GetFanStatus(UI32_T fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT])
{
    int    i = 0;
    UI8_T  fan_status_array_index, fan_status_bit_pos;

    /* sanity check
     */
    if (nbr_of_fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s(%d)Error. nbr_of_fan(%hu) is larger than SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT(%d).\r\n",
            __FUNCTION__, __LINE__, nbr_of_fan, SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT);
        return;
    }

    /* get the status from sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg
     * if SYSDRV_BD_DEBUG_FLAG_FAN bit is on
     */
    if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_FAN)
    {
        for (i = 0; i < nbr_of_fan; i++)
        {
            fan_status_array_index=(UI8_T)(i/8);
            fan_status_bit_pos=(UI8_T)(i%8);
            /* Consider it as fan faulty if the bit in
             * sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg is 1
             */
            if ((sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg[fan_status_array_index] & (1<<fan_status_bit_pos))!=0)
            {
                fan_status[i] = VAL_switchFanStatus_failure;
            }
            else
            {
                fan_status[i] = VAL_switchFanStatus_ok;
            }
        }
        return;
    }

    for (i = 0; i < nbr_of_fan; i++)
    {
        SYS_HWCFG_FanStatusInfo_T fan_status_info;
        UI8_T                     reg_val;

        if (STKTPLG_BOARD_GetFanStatusInfo(i+1, &fan_status_info) == FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)STKTPLG_BOARD_GetFanStatusInfo failed.(Fan Idx=%d)(1-based)\r\n", __FUNCTION__, __LINE__, (int)(i+1));
            fan_status[i] = VAL_switchFanStatus_failure;
            goto next_fan_fault_eval;
        }

        if (fan_status_info.fan_status_eval_method_bmp & SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_REG_BIT)
        {
            if (SYSDRV_UTIL_ReadFanRegByRegInfo(&(fan_status_info.fan_fault_reg_info), 1, &reg_val) == FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)SYSDRV_UTIL_ReadFanRegByRegInfo failed.(Fan Idx=%d)(1-based)\r\n", __FUNCTION__, __LINE__, (int)(i+1));
                fan_status[i] = VAL_switchFanStatus_failure;
                goto next_fan_fault_eval;
            }

            if ( (reg_val & fan_status_info.fan_fault_reg_mask) == fan_status_info.fan_fault_reg_val)
            {
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) is faulty.\r\n",
                        __FUNCTION__, __LINE__, i+1);
                }
                fan_status[i] = VAL_switchFanStatus_failure;
            }
            else
            {
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) is OK.\r\n",
                        __FUNCTION__, __LINE__, i+1);
                }
                fan_status[i] = VAL_switchFanStatus_ok;
            }
            if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(%d)reg_val=0x%02hX,mask=0x%02hX,fault_reg_val=0x%02hX\r\n",
                    __FUNCTION__, __LINE__, reg_val, fan_status_info.fan_fault_reg_mask, fan_status_info.fan_fault_reg_val);
            }

        } /* end of if (fan_status_info.fan_status_eval_method_bmp & SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_REG_BIT) */

next_fan_fault_eval:
        if (fan_status_info.fan_status_eval_method_bmp & SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_FAN_SPEED_BIT)
        {
            UI32_T fan_speed;
            static UI32_T fan_speed_fault_detected_count[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT]={0};

            if (SYSDRV_FAN_GetSpeedInRPM(i+1,&fan_speed)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d)SYSDRV_FAN_GetSpeedInRPM failed.(Fan Idx=%d)(1-based)\r\n", __FUNCTION__, __LINE__, (int)(i+1));
                fan_status[i] = VAL_switchFanStatus_failure;
                continue;
            }
            else /* if (SYSDRV_FAN_GetSpeedInRPM(i+1,&fan_speed)==FALSE) */
            {
                if (fan_speed<fan_status_info.fan_fault_speed)
                {
                    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                    {
                        BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) is faulty(fan_speed=%lu, faulty_fan_speed=%lu).\r\n",
                            __FUNCTION__, __LINE__, i+1, (unsigned long)fan_speed, (unsigned long)fan_status_info.fan_fault_speed);
                    }
                    if (fan_speed_fault_detected_count[i] < 2)
                    {
                        fan_speed_fault_detected_count[i]++;
                        fan_status[i] = VAL_switchFanStatus_ok;
                        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                        {
                            BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) speed is faulty(fan_speed=%lu, faulty_fan_speed=%lu).\r\n",
                                __FUNCTION__, __LINE__, i+1, (unsigned long)fan_speed, (unsigned long)fan_status_info.fan_fault_speed);
                        }
                    }
                    else
                    {
                        fan_status[i] = VAL_switchFanStatus_failure;
                        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                        {
                            BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) status is faulty(fan_speed=%lu, faulty_fan_speed=%lu).\r\n",
                                __FUNCTION__, __LINE__, i+1, (unsigned long)fan_speed, (unsigned long)fan_status_info.fan_fault_speed);
                        }
                    }
                }
                else /* if (fan_speed<fan_status_info.fan_fault_speed) */
                {
                    fan_speed_fault_detected_count[i] = 0;
                    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                    {
                        BACKDOOR_MGR_Printf("%s(%d)Fan Idx %d(1-based) is OK(fan_speed=%lu, faulty_fan_speed=%lu).\r\n",
                            __FUNCTION__, __LINE__, i+1, (unsigned long)fan_speed, (unsigned long)fan_status_info.fan_fault_speed);
                    }

                    fan_status[i] = VAL_switchFanStatus_ok;
                } /* end of if (fan_speed<fan_status_info.fan_fault_speed) */
            } /* end of if (SYSDRV_FAN_GetSpeedInRPM(i+1,&fan_speed)==FALSE) */
        } /* end of if (fan_status_info.fan_status_eval_method_bmp & SYS_HWCFG_FAN_STATUS_EVAL_METHOD_FAN_FAULT_FAN_SPEED_BIT) */
    } /* for (i = 0; i < nbr_of_fan; i++) */

}
#else /* #if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_STKTPLG_BOARD == TRUE) */
/* FUNCTION NAME: SYSDRV_TASK_GetFanStatus
 * PURPOSE: This routine is used to get fan status(OK or failed)
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_GetFanStatus(UI32_T fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT])
{
    int    i = 0;
    UI8_T  *temp_local_unit_fan_status_p;
    UI8_T  temp_local_unit_fan_status[SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE] = {0};
    UI8_T  local_unit_fan_fault_mask[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = SYS_HWCFG_FAN_FAULT_MASK_ARRAY;
#if (SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE > 1)
    UI8_T  fan_status_array_index[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = SYS_HWCFG_FAN_STATUS_ARRAY_INDEX_ARRAY;
#endif

    /* sanity check
     */
    if ( (sizeof(local_unit_fan_fault_mask)/sizeof(local_unit_fan_fault_mask[0])) < SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        printf("%s(%d)Warning! Inconsistent definition of SYS_HWCFG_FAN_FAULT_MASK_ARRAY\r\n",
            __FUNCTION__, __LINE__);
        return;
    }

    if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_FAN)
    {
        memcpy(&(temp_local_unit_fan_status[0]), &(sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg[0]), sizeof(sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg));
    }
    else
    {

#if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_CPLD == TRUE)
        if(FALSE==PHYADDR_ACCESS_Read(sys_hwcfg_fan_fault_status_addr, 1, SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE, (UI8_T*)&(temp_local_unit_fan_status[0])))
        {
            BACKDOOR_MGR_Printf("%s: Get Fan Status fail\r\n", __FUNCTION__);
            for (i=0; i<nbr_of_fan; i++)
            {
                fan_status[i] = VAL_switchFanStatus_failure;
            }
            return;
        }
#elif (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_BID == TRUE)
        UI32_T board_id = 0;
        BOOL_T retval = FALSE;


        if (STKTPLG_OM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id) == FALSE)
        {
            printf("%s(%d): Failed to get board id.\r\n", __FUNCTION__, __LINE__);
            return;
        }

        SYSDRV_TASK_ENTER_CRITICAL_SECTION();
        retval = SYS_HWCFG_GetFANStatusByBID(board_id, nbr_of_fan, &(temp_local_unit_fan_status[0]));
        SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

        if (retval == FALSE)
        {
            printf("%s: Get Fan Status fail\r\n", __FUNCTION__);
            for (i=0; i<nbr_of_fan; i++)
            {
                fan_status[i] = VAL_switchFanStatus_failure;
            }

        }
#else /* #if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_CPLD == TRUE) */
        if(FALSE==SYSDRV_FAN_GetStatus(&(temp_local_unit_fan_status[0])))
        {
            BACKDOOR_MGR_Printf("%s: Get Fan Status fail\r\n", __FUNCTION__);
            for (i=0; i<nbr_of_fan; i++)
            {
                fan_status[i] = VAL_switchFanStatus_failure;
            }
        }
#endif /* #if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_CPLD == TRUE) */
    } /* if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_FAN) */

    for (i = 0; i < nbr_of_fan; i++)
    {
#if (SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE > 1)
        temp_local_unit_fan_status_p = &(temp_local_unit_fan_status[ fan_status_array_index[i] ]);
#else
        temp_local_unit_fan_status_p = &(temp_local_unit_fan_status[0]);
#endif

#if (SYS_HWCFG_FAN_DETECT_FAN_STAUS_BY_CPLD_FAULT_REG_BIT_VAL_IS_ZERO == FALSE)
        if (*temp_local_unit_fan_status_p & local_unit_fan_fault_mask[i])
#else
        if (!((*temp_local_unit_fan_status_p) & local_unit_fan_fault_mask[i]))
#endif
        {
            fan_status[i] = VAL_switchFanStatus_failure;
            if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("Fan index %d(1-based) status Failed: \r\n", i+1);
            }
        }
        else
        {
            #if (SYS_CPNT_SYSDRV_FAN_FAULT_NEED_CHECK_FAN_SPEED==TRUE)
            UI32_T speed;
                #if(SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==FALSE)
                    #error "Conflict definition between SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT and SYS_CPNT_SYSDRV_FAN_FAULT_NEED_CHECK_FAN_SPEED"
                #endif
            if (SYSDRV_FAN_GetSpeedInRPM(i+1, &speed)==FALSE)
            {
                BACKDOOR_MGR_Printf("%s(%d): Failed to get fan speed for fan index %d(1-based)\r\n",
                    __FUNCTION__, __LINE__, i+1);
                fan_status[i] = VAL_switchFanStatus_failure;
            }else if(speed>0)
            {
                fan_status[i] = VAL_switchFanStatus_ok;
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("Fan index %d(1-based) status OK: Fan fault not asserted, fan speed=%lu\r\n",
                        i+1, (unsigned long)speed);
                }
            }
            else
            {
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("Fan index %d(1-based) status Failed: Fan fault not asserted, fan speed=%lu\r\n",
                        i+1, (unsigned long)speed);
                }
                fan_status[i] = VAL_switchFanStatus_failure;
            }

            #else /* #if (SYS_CPNT_SYSDRV_FAN_FAULT_NEED_CHECK_FAN_SPEED==TRUE) */
            fan_status[i] = VAL_switchFanStatus_ok;
            #endif /* end of #if (SYS_CPNT_SYSDRV_FAN_FAULT_NEED_CHECK_FAN_SPEED==TRUE) */
        }
    }
}
#endif /* end of #if (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_STKTPLG_BOARD == TRUE) */
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */

/* FUNCTION NAME: SYSDRV_TASK_CheckAndUpdateFanController
 * PURPOSE: This function will compare the current fan speed mode and
 *          the effective fan speed mode got from SYSDRV_TASK_GetEffectiveFanSpeedMode().
 *          If these two fan speed modes are different, the proper fan control
 *          setting will be written to the fan controller according to the
 *          new effective fan speed mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_CheckAndUpdateFanController(void)
{

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) /* Fan speed is controlled by onlp_platmgrd when SYS_CPNT_SYSDRV_USE_ONLP is TRUE */
#if (SYSDRV_TASK_HAS_FAN_CONTROLLER == TRUE)
    SYSDRV_TASK_FanSpeedLevelToSettingForOneLevelArrayPtr_T fan_speed_setting_p=NULL;

    UI32_T effective_fan_speed_mode=SYSDRV_TASK_GetEffectiveFanSpeedMode();
    UI8_T  i;

    if (running_fan_speed_mode != effective_fan_speed_mode )
    {

#if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL!=TRUE)
        switch (effective_fan_speed_mode)
        {
            case SYSDRV_FAN_MID_SPEED:
                fan_speed_setting_p = &fan_mid_speed_setting;
                break;
            case SYSDRV_FAN_FULL_SPEED:
                fan_speed_setting_p = &fan_full_speed_setting;
                break;
            default:
                if (effective_fan_speed_mode==SYSDRV_FAN_PREPARE_TRANSITION_SPEED)
                {
                    running_fan_speed_mode=effective_fan_speed_mode;
                }
                else
                {
                    BACKDOOR_MGR_Printf("%s:Cannot get fan speed setting due to invalid fan speed mode %lu\r\n",
                        __FUNCTION__, (unsigned long)effective_fan_speed_mode);
                }
                return;
        }
#else /* #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL!=TRUE) */
        if (SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(effective_fan_speed_mode)==TRUE)
        {
            fan_speed_setting_p = (SYSDRV_TASK_FanSpeedLevelToSettingForOneLevelArrayPtr_T)&(fan_speed_level_to_setting_ar[effective_fan_speed_mode-1][0]);
        }
        else
        {
            BACKDOOR_MGR_Printf("%s:Cannot get fan speed setting due to invalid fan speed mode %lu\r\n",
                __FUNCTION__, (unsigned long)effective_fan_speed_mode);
            return;
        }
#endif /* end of #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL!=TRUE) */
        /* apply new fan speed setting to the fan controller
         */
        for (i=0; i<nbr_of_fan; i++)
        {
            SYSDRV_FAN_SetSpeed(i+1, (*fan_speed_setting_p)[i]);
            if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("Set duty cycle %d to fan index %d\r\n",
                    (int)((*fan_speed_setting_p)[i]), i+1);
            }
        }

        #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST==TRUE)
        if (running_fan_speed_mode != effective_fan_speed_mode)
        {
            /* Set fan speed in transition counter to predefined value to skip
             * fan speed check and fan speed calibration until fan rotates in
             * stable state.
             */
            sysdrv_shmem_data_p->fan_speed_in_transition_counter = sysdrv_shmem_data_p->fan_speed_in_transition_counter_reset_value;
        }
        #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST==TRUE) */

        running_fan_speed_mode = effective_fan_speed_mode;
    }
#endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER == TRUE) */

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
    if (sysdrv_shmem_data_p->fan_speed_mode_sm_info.type == SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG)
    {
        UI8_T fan_speed_level;

        if(SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).is_in_high_fan_speed_state==TRUE)
        {
            fan_speed_level=1;
        }
        else
        {
            fan_speed_level=0;
        }

        /* Need to consider the case that the psu might be removed and inserted
         * at any time. When the psu is removed and the psu is inserted again,
         * the fan speed level will be reset to the default value. For the
         * ease of implementation, the fan speed level for all of the PSU is
         * always set whenever this function is called.
         */
        if (STKTPLG_BOARD_SetPSUFanSpeedLevel(0, &i2c_op_fn_ptrs,fan_speed_level)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to set fan speed level %hu to PSU\r\n",
                __FUNCTION__, __LINE__, fan_speed_level);
        }

    }
#endif
#endif /* end of #if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE) */
}

/* FUNCTION NAME: SYSDRV_TASK_DetectFanStatus
 * PURPOSE: This routine is used to detect fan status
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE  -  Success
 *          FALSE -  Failed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_DetectFanStatus(void)
{
    int     originalFailNum = 0;
    int     newFailNum = 0;

    UI32_T  new_fan_status[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {0};
    int     i;

    /* sanity check
     */
    if (nbr_of_fan>SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning!Invalid nbr_of_fan %hu(Max value=%d)\r\n",
            __FUNCTION__, __LINE__, nbr_of_fan, SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT);
        return FALSE;
    }

    SYSDRV_TASK_CheckAndUpdateFanController();

#if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
    /* Processing Fan Speed (Fan rotating speed)
     */
    {
        UI32_T  new_fan_speed[SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT] = {0};

        SYSDRV_TASK_GetFanSpeed(new_fan_speed);
        /* compare new fan speed with the previous fan speed
         * and notify SYS MGR if the fan speed is changed
         */
        for (i=0; i<nbr_of_fan; i++)
        {
            if (sysdrv_shmem_data_p->sysdrv_fan_speed[i] != new_fan_speed[i])
            {
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG() == TRUE)
                {
                    BACKDOOR_MGR_Printf("SYSDRV: Fan %d Speed Change from %lu to %lu\r\n",
                        i+1, (unsigned long)sysdrv_shmem_data_p->sysdrv_fan_speed[i], (unsigned long)new_fan_speed[i]);
                }

                SYSDRV_Notify_FanSpeedChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, i+1, new_fan_speed[i]);
                SYSDRV_TASK_ENTER_CRITICAL_SECTION();
                sysdrv_shmem_data_p->sysdrv_fan_speed[i] = new_fan_speed[i];
                SYSDRV_TASK_LEAVE_CRITICAL_SECTION();
            }
        }
    }
#endif

    /* Processing Fan Status (Fan OK or Fan Failed)
     */
    SYSDRV_TASK_GetFanStatus(new_fan_status);

    /* Check Fan Status. If change, notify SYS MGR.
     */
    for (i=0; i<nbr_of_fan; i++)
    {
        /* Calculate the original number of fail fans
         */
        if ( VAL_switchFanStatus_ok != sysdrv_shmem_data_p->sysdrv_fan_status[i] )
            originalFailNum++;

        if (TRUE == SYSDRV_TASK_FanStateMachine(sysdrv_shmem_data_p->sysdrv_fan_status[i], new_fan_status[i]))
        {
            if (SYSDRV_SHOW_DEBUG_MSG_FLAG() == TRUE)
            {
                BACKDOOR_MGR_Printf("SYSDRV: Fan %d Status Change from %lu to %lu\r\n",
                    i+1, (unsigned long)sysdrv_shmem_data_p->sysdrv_fan_status[i], (unsigned long)new_fan_status[i]);
            }

            SYSDRV_Notify_FanStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, i+1, new_fan_status[i]);
            SYSDRV_TASK_ENTER_CRITICAL_SECTION();
            sysdrv_shmem_data_p->sysdrv_fan_status[i] = new_fan_status[i];
            SYSDRV_TASK_LEAVE_CRITICAL_SECTION();
        }

        /* Calculate the number of fail fans after be updated
         */
        if ( VAL_switchFanStatus_ok != sysdrv_shmem_data_p->sysdrv_fan_status[i] )
            newFailNum++;
    }

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
    /* Processing Fan Speed Calibration
     */
    SYSDRV_TASK_FanSpeedCalibration();
#endif

#ifdef HAVE_EFM_OAM /* Tiger Liu, Nov.24, 2006, for EFM-OAM */
    if (newFailNum > originalFailNum)
    {
        AMS_EFM_OAM_Send_CriticalEvent_Pdu();
    }
#endif

   return TRUE;

}

/* FUNCTION NAME: SYSDRV_TASK_FanStateMachine
 * PURPOSE: This routine is used to compare fan state
 * INPUT:   current - current state
 *          new     - new state
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanStateMachine(UI32_T current, UI32_T new)
{
    BOOL_T retval = TRUE;

    if (current == new)
    {
        retval = FALSE;
    }

    return (retval);
}

#if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
/* FUNCTION NAME: SYSDRV_GetFanSpeedThreshold
 * PURPOSE: This routine is used to calibrate fan speed
 * INPUT:   fan_speed_mode  -  this value must be one of the enum value in
 *                             SYSDRV_FanSpeed_T
 *
 * OUTPUT:  upper_limit_p   -  upper limit of fan speed in the specified fan
 *                             speed mode
 *          lower_limit_p   -  lower limit of fan speed in the specified fan
 *                             speed mode
 * RETURN:  TRUE  -  Success
 *          FALSE -  Failed
 * NOTES:   For now, only SYSDRV_FAN_MID_SPEED and SYSDRV_FAN_FULL_SPEED
 *          will have valid upper and lower threshold.
 */
static BOOL_T SYSDRV_TASK_GetFanSpeedThresholds(UI32_T fan_speed_mode, UI32_T *upper_limit_p, UI32_T *lower_limit_p)
{
    BOOL_T ret_val=TRUE;

    switch (fan_speed_mode)
    {
        case SYSDRV_FAN_FULL_SPEED:
            *lower_limit_p = SYSDRV_FAN_FULL_SPEED_LOWER_LIMIT;
            *upper_limit_p = SYSDRV_FAN_FULL_SPEED_UPPER_LIMIT;
            break;

        case SYSDRV_FAN_MID_SPEED:
            *lower_limit_p = SYSDRV_FAN_MID_SPEED_LOWER_LIMIT;
            *upper_limit_p = SYSDRV_FAN_MID_SPEED_UPPER_LIMIT;
            break;

        default:
            ret_val=FALSE;
    }

    return ret_val;
}

/* FUNCTION NAME: SYSDRV_TASK_FanSpeedCalibration
 * PURPOSE: This routine is used to calibrate fan speed
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_FanSpeedCalibration(void)
{
/* for each fan speed mode
 * the fan speed setting can be adjusted up to 20(i.e. SYSDRV_FAN_CALIBRATION_MAX_ADJUST_COUNT*CHANGE_STEP)
 */
#define CHANGE_STEP 2

    const UI32_T *speed_p = &(sysdrv_shmem_data_p->sysdrv_fan_speed[0]);
    UI32_T *fan_speed_setting_p=NULL;
    UI32_T i, sysdrv_speed_lower_limit=0, sysdrv_speed_upper_limit=0;
    UI32_T speed_setting_mode;
    BOOL_T need_adjust;

    /* skip doing fan speed calibration if the fan speed in transistion counter
     * is not zero
     */
    SYSDRV_TASK_ENTER_CRITICAL_SECTION();
    if (sysdrv_shmem_data_p->fan_speed_in_transition_counter!=0)
    {
        sysdrv_shmem_data_p->fan_speed_in_transition_counter--;
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s(%d):fan speed in transition counter=%hu\r\n",
                __FUNCTION__, __LINE__, sysdrv_shmem_data_p->fan_speed_in_transition_counter);
        }

        SYSDRV_TASK_LEAVE_CRITICAL_SECTION();
        return;
    }
    SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

    speed_setting_mode = running_fan_speed_mode;

    if (SYSDRV_TASK_GetFanSpeedThresholds(speed_setting_mode,
        &sysdrv_speed_upper_limit, &sysdrv_speed_lower_limit)==FALSE)
    {
        if (speed_setting_mode!=SYSDRV_FAN_PREPARE_TRANSITION_SPEED)
        {
            /* Do not do fan speed calibration in fan speed setting mode
             * SYSDRV_FAN_PREPARE_TRANSITION_SPEED.
             *
             * Show error message if SYSDRV_TASK_GetFanSpeedThresholds() returns FALSE
             * and current speed setting mode is not SYSDRV_FAN_PREPARE_TRANSITION_SPEED
             */
            BACKDOOR_MGR_Printf("%s:Invalid fan speed mode %lu\r\n",
                __FUNCTION__, (unsigned long)speed_setting_mode);
        }
        return;
    }

    switch (speed_setting_mode)
    {
        case SYSDRV_FAN_MID_SPEED:
            fan_speed_setting_p = &(fan_mid_speed_setting[0]);
            break;
        case SYSDRV_FAN_FULL_SPEED:
            fan_speed_setting_p = &(fan_full_speed_setting[0]);
            break;
        default:
            BACKDOOR_MGR_Printf("%s:Cannot get fan speed setting due to invalid fan speed mode %lu\r\n",
                __FUNCTION__, (unsigned long)speed_setting_mode);
            return;
    }

    for (i=0; i<nbr_of_fan; i++)
    {
        need_adjust=FALSE;
        if( (speed_p[i]!=0) && (SYSDRV_TASK_IsFanCanCalibrate(i+1)==TRUE) )
        {
            if (speed_p[i] < sysdrv_speed_lower_limit)
            {
                need_adjust=TRUE;
                if( (fan_speed_setting_p[i]+CHANGE_STEP)>SYS_HWCFG_FAN_SPEED_MAX )
                    fan_speed_setting_p[i] = SYS_HWCFG_FAN_SPEED_MAX;
                else
                    fan_speed_setting_p[i] += CHANGE_STEP;
            }
            else if (speed_p[i] > sysdrv_speed_upper_limit)
            {
                need_adjust=TRUE;
                if( (fan_speed_setting_p[i]-CHANGE_STEP)>fan_speed_setting_p[i] ) /* the condition means overflow happened */
                    fan_speed_setting_p[i] = 0;
                else
                    fan_speed_setting_p[i] -= CHANGE_STEP;
            }

            if (need_adjust==TRUE)
            {
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG() == TRUE)
                {
                    BACKDOOR_MGR_Printf("Fan %lu speed mode=%lu current speed=%d (low~up:%lu~%lu)\r\n",
                        (unsigned long)i+1, (unsigned long)speed_setting_mode,
                        speed_p[i],
                        (unsigned long)sysdrv_speed_lower_limit, (unsigned long)sysdrv_speed_upper_limit);

                    BACKDOOR_MGR_Printf("adjusted value = %lu , fan_speed_calibration_adjust_counter=%d\r\n",
                        (unsigned long)fan_speed_setting_p[i], sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter[speed_setting_mode][i]);
                }
                SYSDRV_FAN_SetSpeed(i+1, fan_speed_setting_p[i]);
                sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter[speed_setting_mode][i]++;
            }
        }
    }
}
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */

#endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */

#endif /* #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)

/* FUNCTION NAME: SYSDRV_ThermalTemperatureStateMachine
 * PURPOSE: This routine is used to compare previous thermal status with
 *          the current thermal status. If thermal status changes, it will
 *          return TRUE.
 * INPUT:   previous_thermal_temp - previous thermal temperature
 *          new_thermal_temp      - new thermal temperature
 * OUTPUT:  None.
 * RETURN:  TRUE  -  Thermal status is changed.
 *          FALSE -  Thermal status is not chagned.
 * NOTES:   None.
 */
static inline BOOL_T SYSDRV_TASK_ThermalStatusStateMachine(I32_T previous_thermal_temp, I32_T new_thermal_temp)
{
    BOOL_T retval = TRUE;

    if (previous_thermal_temp == new_thermal_temp)
        retval = FALSE;

    return (retval);
}

#if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE)
/* FUNCTION NAME: SYSDRV_TASK_UpdateFanSpeedModeByFanFailState
 * PURPOSE: This routine will check the current fan fail state to determine
 *          the fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode after
 *                                      evaluation of fan fail state
 * RETURN:  None
 * NOTES:   This function will not validate the input argument. Caller
 *          must have been validate the argument.
 */
static void SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(UI32_T *fan_speed_mode_p)
{
#if (SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_INCLUDE_FAN_FAIL_STATE==TRUE)
    UI32_T new_fan_speed_setting_mode = *fan_speed_mode_p;
    UI8_T i;

    /* If SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_INCLUDE_FAN_FAIL_STATE is TRUE,
     * fan speed setting mode must be FULL SPEED when at leaset one fan fail
     * failed. If the current evaluated new fan speed setting mode already
     * equals to FULL SPEED, do not need to check fan status and re-evaluate
     * the new fan speed setting mode again.
     */
    if ( (new_fan_speed_setting_mode!=SYSDRV_FAN_FULL_SPEED) )
    {

        /* set fan speed setting mode to SYSDRV_FAN_FULL_SPEED if at least
         * one fan failed
         */
        for (i=0; i<nbr_of_fan; i++)
        {
            if (sysdrv_shmem_data_p->sysdrv_fan_status[i] != VAL_switchFanStatus_ok)
            {
                new_fan_speed_setting_mode=SYSDRV_FAN_FULL_SPEED;
                if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("Force full fan speed due to at least one fan failed detected.\r\n");
                }
                break;
            }
        }
        *fan_speed_mode_p=new_fan_speed_setting_mode;
    }

#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_SPEED_CONTROL_INCLUDE_FAN_FAIL_STATE==TRUE) */
    return;
}

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine(UI32_T *fan_speed_setting_mode_p)
{
    switch (sysdrv_shmem_data_p->fan_speed_mode_sm_info.type)
    {
        case SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_NONE:
            /* do nothing */
            return FALSE;
            break;
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
        case SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_AVG_TMP:
            return SYSDRV_TASK_FanSpeedModeStateMachine_AvgTmp(fan_speed_setting_mode_p);
            break;
#endif
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
        case SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG:
            return SYSDRV_TASK_FanSpeedModeStateMachine_FruCfg(fan_speed_setting_mode_p);
            break;
#endif
        default:
            BACKDOOR_MGR_Printf("%s(%d)Invalid fan speed mode state machine type(%d).\r\n",
                __FUNCTION__, __LINE__, (int)(sysdrv_shmem_data_p->fan_speed_mode_sm_info.type));
            break;
    }

    return FALSE;
}

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine_AvgTmp
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine_AvgTmp(UI32_T *fan_speed_setting_mode_p)
{
    UI32_T new_fan_speed_setting_mode;
    I32_T avg_temperature;
    UI8_T i, thermal_idx;
    I8_T effective_up_temperature_threshold, effective_down_temperature_threshold;

    /* verify the data in fan speed mode state machine info
     */
    if (sysdrv_shmem_data_p->fan_speed_mode_sm_info.type!=SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_AVG_TMP)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning!Incorrect settings for fan speed mode state machine.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).number_of_thermal_to_check==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)number_of_thermal_to_check should not be 0.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)number_of_speed_level should not be 0.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* validate the input argument
     */
    if (fan_speed_setting_mode_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)fan_speed_setting_mode_p is NULL.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(*fan_speed_setting_mode_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid fan_speed_setting_mode(%lu).\r\n",
            __FUNCTION__, __LINE__, *fan_speed_setting_mode_p);
        /* reset the value to default fan speed mode
         */
        *fan_speed_setting_mode_p=SYS_HWCFG_DEFAULT_FAN_SPEED_MODE;
    }

    /* init new_fan_speed_setting_mode
     */
    new_fan_speed_setting_mode=*fan_speed_setting_mode_p;

    /* calculate the temperature to be used for speed level evaluation
     */
    for (avg_temperature=0,i=0; i<(SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).number_of_thermal_to_check); i++)
    {
        thermal_idx = SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).thermal_idx_to_check_ar[i];
        avg_temperature += sysdrv_shmem_data_p->sysdrv_thermal_temp[thermal_idx-1];
    }
    avg_temperature=avg_temperature/(SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).number_of_thermal_to_check);

    /* check the average temperature against the up and down temperature
     * threshold to get the new speed level(a.k.a. speed mode)
     */
    effective_up_temperature_threshold=SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).speed_level_to_up_temperature_threshold[(*fan_speed_setting_mode_p)-1];
    effective_down_temperature_threshold=SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).speed_level_to_down_temperature_threshold[(*fan_speed_setting_mode_p)-1];

    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        BACKDOOR_MGR_Printf("avg_temp=%ld, up_threshold=%hd, down_threshold=%hd\r\n",
            avg_temperature, effective_up_temperature_threshold, effective_down_temperature_threshold);
    }

    if ((effective_up_temperature_threshold!=0) &&
        (avg_temperature>=effective_up_temperature_threshold))
    {
        /* bump up the speed level by one
         */
        new_fan_speed_setting_mode++;
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Fan speed level up by 1(current=%lu)\r\n", new_fan_speed_setting_mode);
        }
    }
    else if ( (effective_down_temperature_threshold!=0) &&
              (avg_temperature<=effective_down_temperature_threshold))
    {
        /* lower the speed level by one
         */
        new_fan_speed_setting_mode--;
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Fan speed level down by 1(current=%lu)\r\n", new_fan_speed_setting_mode);
        }
    }

    /* validate the value of new_fan_speed_setting_mode
     */
    if (new_fan_speed_setting_mode==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu). Could be caused by the incorrect value in down threshold array\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode);

        if (SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).speed_level_to_down_temperature_threshold[0]!=0)
        {
            BACKDOOR_MGR_Printf("%s(%d)Error! speed_level_to_down_temperature_threshold[0] must be 0.\r\n",
                __FUNCTION__, __LINE__);
        }
        /* reset the value to the lowest speed level
         */
        new_fan_speed_setting_mode=1;
    }

    if (new_fan_speed_setting_mode>sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu).\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode);

        if (SYSDRV_TASK_FAN_SPEED_MODE_SM_AVG_TMP_INFO(sysdrv_shmem_data_p).speed_level_to_up_temperature_threshold[(sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level)-1]!=0)
        {
            BACKDOOR_MGR_Printf("%s(%d)Error! speed_level_to_up_temperature_threshold[%d] must be 0.\r\n",
                __FUNCTION__, __LINE__, sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level-1);
        }

        /* reset the value to the higest fan speed level;
         */
        new_fan_speed_setting_mode=sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level;
    }

    SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(&new_fan_speed_setting_mode);

    /* check whether the evaluated new fan speed settings equals to the current
     * fan speed settings
     */
    if (new_fan_speed_setting_mode != *fan_speed_setting_mode_p)
    {
        *fan_speed_setting_mode_p=new_fan_speed_setting_mode;
        return TRUE;
    }
    return FALSE;
}
#endif /* end of #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_AVG_TMP==TRUE) */

#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine_FruCfg
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine_FruCfg(UI32_T *fan_speed_setting_mode_p)
{
    UI32_T new_fan_speed_setting_mode;
    I8_T   highest_effective_temperature=-128, effective_temperature;
    UI8_T i, thermal_idx, highest_temp_thermal_idx=255;
    BOOL_T is_high_fan_speed_state=FALSE;

    if (STKTPLG_BOARD_UpdateFanSpeedModeSMInfo(&i2c_op_fn_ptrs, &(sysdrv_shmem_data_p->fan_speed_mode_sm_info))==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)STKTPLG_BOARD_UpdateFanSpeedModeSMInfo returns FALSE.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* verify the data in fan speed mode state machine info
     */
    if (sysdrv_shmem_data_p->fan_speed_mode_sm_info.type!=SYSDRV_FANSPEEDMODESTATEMACHINE_TYPE_FRU_CFG)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning!Incorrect settings for fan speed mode state machine.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)number_of_speed_level should not be 0.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* validate the input argument
     */
    if (fan_speed_setting_mode_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)fan_speed_setting_mode_p is NULL.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(*fan_speed_setting_mode_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid fan_speed_setting_mode(%lu).\r\n",
            __FUNCTION__, __LINE__, *fan_speed_setting_mode_p);
        /* reset the value to default fan speed mode
         */
        *fan_speed_setting_mode_p=SYS_HWCFG_DEFAULT_FAN_SPEED_MODE;
    }

    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        BACKDOOR_MGR_Printf("%s(%d)SM Info:{%hd,%hd,%lu,%lu}\r\n",
            __FUNCTION__, __LINE__,
            SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).speed_level_to_up_temperature_threshold,
            SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).speed_level_to_down_temperature_threshold,
            SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).fan_speed_level_ar[0],
            SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).fan_speed_level_ar[1]
            );
    }

    /* init new_fan_speed_setting_mode
     */
    new_fan_speed_setting_mode=*fan_speed_setting_mode_p;

    /* calculate the temperature to be used for speed level evaluation
     */
    for (thermal_idx=1; thermal_idx<=nbr_of_thermal; thermal_idx++)
    {
        effective_temperature=sysdrv_shmem_data_p->sysdrv_thermal_temp[thermal_idx-1]+
            SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).adjusted_temp_offset[thermal_idx-1];
        if (highest_effective_temperature<effective_temperature)
        {
            highest_effective_temperature=effective_temperature;
            highest_temp_thermal_idx=thermal_idx;
        }

    }
    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        if (highest_temp_thermal_idx>nbr_of_thermal)
        {
            BACKDOOR_MGR_Printf("%s(%d)highest_temp_thermal_idx is invalid(%hu). nbr_of_thermal=%hu\r\n",
                __FUNCTION__, __LINE__, highest_temp_thermal_idx, nbr_of_thermal);
        }
        else
        {
            BACKDOOR_MGR_Printf("%s(%d)Highest effective temp=%hd(Primitive temp=%hd, thermal_idx=%hu)\r\n",
                __FUNCTION__, __LINE__, highest_effective_temperature, sysdrv_shmem_data_p->sysdrv_thermal_temp[highest_temp_thermal_idx-1], highest_temp_thermal_idx);
        }
    }

    /* check the highest temperature against the up and down temperature
     * threshold to get the new speed level(a.k.a. speed mode)
     */
    if (highest_effective_temperature>SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).speed_level_to_up_temperature_threshold)
    {
        new_fan_speed_setting_mode=SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).fan_speed_level_ar[1];
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Change to high speed level(orig=%lu,new=%lu)\r\n", *fan_speed_setting_mode_p, new_fan_speed_setting_mode);
        }
        SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).is_in_high_fan_speed_state=TRUE;
    }
    else if (highest_effective_temperature<SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).speed_level_to_down_temperature_threshold)
    {
        new_fan_speed_setting_mode=SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).fan_speed_level_ar[0];
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Change to low speed level(orig=%lu,new=%lu)\r\n", *fan_speed_setting_mode_p, new_fan_speed_setting_mode);
        }
        SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).is_in_high_fan_speed_state=FALSE;
    }
    else
    {
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Speed level not change(%lu)(%s fan speed state\r\n", *fan_speed_setting_mode_p,
                (SYSDRV_TASK_FAN_SPEED_MODE_SM_FRU_CFG_INFO(sysdrv_shmem_data_p).is_in_high_fan_speed_state)?"High":"Low");
        }
    }

    /* validate the value of new_fan_speed_setting_mode
     */
    if (new_fan_speed_setting_mode==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu). Could be caused by the incorrect value in down threshold array\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode);

        /* reset the value to the lowest speed level
         */
        new_fan_speed_setting_mode=1;
    }

    if (new_fan_speed_setting_mode>sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu)(valid max val=%hu).\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode,
            sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level);

        /* reset the value to the higest fan speed level;
         */
        new_fan_speed_setting_mode=sysdrv_shmem_data_p->fan_speed_mode_sm_info.number_of_speed_level;
    }


    /* check whether the evaluated new fan speed settings equals to the current
     * fan speed settings
     */
    if (new_fan_speed_setting_mode != *fan_speed_setting_mode_p)
    {
        *fan_speed_setting_mode_p=new_fan_speed_setting_mode;
        return TRUE;
    }
    return FALSE;
}
#endif /* end of #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE_TYPE_FRU_CFG==TRUE) */

#else /* #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE) */

#if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE)
#if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE == SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine(UI32_T *fan_speed_setting_mode_p)
{
    UI32_T new_fan_speed_setting_mode;
    I32_T avg_temperature;
    UI8_T i;
    I8_T speed_level_to_up_temperature_threshold[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL]  = SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_UP_TEMPERATURE_THRESHOLD_ARRAY;
    I8_T speed_level_to_down_temperature_threshold[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL]= SYS_ADPT_SYSDRV_FAN_SPEED_LEVEL_TO_DOWN_TEMPERATURE_THRESHOLD_ARRAY;
    I8_T effective_up_temperature_threshold, effective_down_temperature_threshold;
    UI8_T thermal_idx_to_check_ar[SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR]=SYS_ADPT_SYSDRV_BID_0_FAN_SPEED_MODE_SM_CHECK_THERMAL_INDEX_ARRAY;

    /* verify the constant definitions
     */
    if (SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR !=
        (sizeof(thermal_idx_to_check_ar)/sizeof(thermal_idx_to_check_ar[0])))
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning!Inconsistent definitions between SYS_ADPT_SYSDRV_FAN_SPEED_MODE_SM_MAX_NUMBER_OF_CHECK_THERMAL_SENSOR and SYS_ADPT_SYSDRV_BID_0_FAN_SPEED_MODE_SM_CHECK_THERMAL_INDEX_ARRAY.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    /* validate the input argument
     */
    if (fan_speed_setting_mode_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)fan_speed_setting_mode_p is NULL.\r\n",
            __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(*fan_speed_setting_mode_p)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid fan_speed_setting_mode(%lu).\r\n",
            __FUNCTION__, __LINE__, *fan_speed_setting_mode_p);
        /* reset the value to default fan speed mode
         */
        *fan_speed_setting_mode_p=SYS_HWCFG_DEFAULT_FAN_SPEED_MODE;
    }

    /* init new_fan_speed_setting_mode
     */
    new_fan_speed_setting_mode=*fan_speed_setting_mode_p;

    /* calculate the temperature to be used for speed level evaluation
     */
    for (avg_temperature=0,i=0; i<(sizeof(thermal_idx_to_check_ar)/sizeof(thermal_idx_to_check_ar[0])); i++)
    {
        avg_temperature += sysdrv_shmem_data_p->sysdrv_thermal_temp[thermal_idx_to_check_ar[i]-1];
    }
    avg_temperature=avg_temperature/(sizeof(thermal_idx_to_check_ar)/sizeof(thermal_idx_to_check_ar[0]));

    /* check the average temperature against the up and down temperature
     * threshold to get the new speed level(a.k.a. speed mode)
     */
    effective_up_temperature_threshold=speed_level_to_up_temperature_threshold[(*fan_speed_setting_mode_p)-1];
    effective_down_temperature_threshold=speed_level_to_down_temperature_threshold[(*fan_speed_setting_mode_p)-1];

    if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        BACKDOOR_MGR_Printf("avg_temp=%ld, up_threshold=%hd, down_threshold=%hd\r\n",
            avg_temperature, effective_up_temperature_threshold, effective_down_temperature_threshold);
    }

    if ((effective_up_temperature_threshold!=0) &&
        (avg_temperature>=effective_up_temperature_threshold))
    {
        /* bump up the speed level by one
         */
        new_fan_speed_setting_mode++;
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Fan speed level up by 1(current=%lu)\r\n", new_fan_speed_setting_mode);
        }
    }
    else if ( (effective_down_temperature_threshold!=0) &&
              (avg_temperature<=effective_down_temperature_threshold))
    {
        /* lower the speed level by one
         */
        new_fan_speed_setting_mode--;
        if (SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("Fan speed level down by 1(current=%lu)\r\n", new_fan_speed_setting_mode);
        }
    }

    /* validate the value of new_fan_speed_setting_mode
     */
    if (new_fan_speed_setting_mode==0)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu). Could be caused by the incorrect value in down threshold array\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode);

        if (speed_level_to_down_temperature_threshold[0]!=0)
        {
            BACKDOOR_MGR_Printf("%s(%d)Error! speed_level_to_down_temperature_threshold[0] must be 0.\r\n",
                __FUNCTION__, __LINE__);
        }
        /* reset the value to the lowest speed level
         */
        new_fan_speed_setting_mode=1;
    }

    if (new_fan_speed_setting_mode>SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning! Invalid new_fan_speed_setting_mode(%lu).\r\n",
            __FUNCTION__, __LINE__, new_fan_speed_setting_mode);

        if (speed_level_to_up_temperature_threshold[SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL-1]!=0)
        {
            BACKDOOR_MGR_Printf("%s(%d)Error! speed_level_to_up_temperature_threshold[%d] must be 0.\r\n",
                __FUNCTION__, __LINE__, SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL-1);
        }

        /* reset the value to the higest fan speed level;
         */
        new_fan_speed_setting_mode=SYS_ADPT_SYSDRV_FAN_MAX_NUMBER_OF_SPEED_LEVEL;
    }

    SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(&new_fan_speed_setting_mode);

    /* check whether the evaluated new fan speed settings equals to the current
     * fan speed settings
     */
    if (new_fan_speed_setting_mode != *fan_speed_setting_mode_p)
    {
        *fan_speed_setting_mode_p=new_fan_speed_setting_mode;
        return TRUE;
    }
    return FALSE;
}
#else /* #if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE == SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP) */
#error "Incorrect definition of SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE"
#endif /* #if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE == SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_AVG_TMP) */

#else /* #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE) */
#if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE==SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine(UI32_T *fan_speed_setting_mode_p)
{
    UI32_T new_fan_speed_setting_mode=*fan_speed_setting_mode_p;
    UI8_T  i;
    UI8_T  nbr_of_fan = STKTPLG_BOARD_GetFanNumber();

    /* do not change current fan speed setting if fan_speed_force_full is enabled
     */
    if (SYSDRV_GetFanSpeedForceFull()==TRUE)
        return FALSE;

    if (nbr_of_fan > SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s(%d)Warning!!! inconsistent definition of SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT.\r\n", __FUNCTION__, __LINE__);
        nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;
    }

    {
        /* check thermal temperatures
         */

        I8_T thermal_threshold_up[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
        I8_T thermal_threshold_down[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
        UI8_T up_count = 0, down_count = 0;

        if (SYSDRV_GetThermalThresholds(thermal_threshold_down, thermal_threshold_up)==FALSE)
        {
            if (SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(%d)SYSDRV_GetThermalThresholds failed.\r\n", __FUNCTION__, __LINE__);
            }
            return FALSE;
        }

        for (i=0; i<nbr_of_thermal; i++)
        {

            if (sysdrv_shmem_data_p->sysdrv_thermal_temp[i] > thermal_threshold_up[i])
                up_count++;

            if (sysdrv_shmem_data_p->sysdrv_thermal_temp[i] < thermal_threshold_down[i])
                down_count++;
        }

        #if (SYS_CPNT_STKTPLG_FAN_SET_FULL_SPEED_WHEN_ONE_THERMAL_OVER_TEMPERATURE == TRUE)
        if (up_count>0)
        #else
        if (up_count==nbr_of_thermal)
        #endif
        {
            new_fan_speed_setting_mode=SYSDRV_FAN_FULL_SPEED;
        }
        else if(down_count == nbr_of_thermal)
        {
            new_fan_speed_setting_mode=SYSDRV_FAN_MID_SPEED;
        }

    }

    SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(&new_fan_speed_setting_mode);

    /* check whether the evaluated new fan speed settings equals to the current
     * fan speed settings
     */
    if (new_fan_speed_setting_mode != *fan_speed_setting_mode_p)
    {
        *fan_speed_setting_mode_p=new_fan_speed_setting_mode;
        return TRUE;
    }
    return FALSE;
}
#elif (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE==SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL_WITH_FAN_TRANSITION_STATE)
/* FUNCTION NAME: SYSDRV_TASK_FanSpeedModeStateMachine
 * PURPOSE: This routine will check the given conditions to determine
 *          current fan speed mode.
 * INPUT:   fan_speed_setting_mode_p -- current fan speed setting mode
 * OUTPUT:  fan_speed_setting_mode_p -- the new fan speed setting mode
 * RETURN:  TRUE  - output fan speed setting mode is changed
 *          FALSE - output fan speed setting mode is not changed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_FanSpeedModeStateMachine(UI32_T *fan_speed_setting_mode_p)
{
    UI32_T new_fan_speed_setting_mode=*fan_speed_setting_mode_p;
    I8_T thermal_raising_threshold[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    I8_T thermal_falling_threshold[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];
    UI8_T i;
    UI8_T up_count = 0, down_count = 0;

    /* do not change current fan speed setting if fan_speed_force_full is enabled
     */
    if (SYSDRV_GetFanSpeedForceFull()==TRUE)
        return FALSE;

    if (SYSDRV_GetThermalThresholds(thermal_falling_threshold, thermal_raising_threshold)==FALSE)
    {
        if (SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s(%d)SYSDRV_GetThermalThresholds failed.\r\n", __FUNCTION__, __LINE__);
        }
        return FALSE;
    }

    for (i=0; i<nbr_of_thermal; i++)
    {
        if (sysdrv_shmem_data_p->sysdrv_thermal_temp[i] > thermal_raising_threshold[i])
            up_count++;

        if (sysdrv_shmem_data_p->sysdrv_thermal_temp[i] < thermal_falling_threshold[i])
            down_count++;
    }

#if (SYS_CPNT_STKTPLG_FAN_SET_FULL_SPEED_WHEN_ONE_THERMAL_OVER_TEMPERATURE == TRUE)
    if (up_count>0)
#else
    if (up_count==nbr_of_thermal)
#endif
    {
        /* at least one temperature of thermal sensors is larger than upper
         * threshold, raise fan speed mode to FULL SPEED.
         */
        switch ( sysdrv_shmem_data_p->sysdrv_speed_setting_mode )
        {
            case SYSDRV_FAN_FULL_SPEED:                 /* already at full speed, do nothing */
                break;

            case SYSDRV_FAN_PREPARE_TRANSITION_SPEED:   /* it has been too hot for the 2nd time in a row, spin up the fan */
                new_fan_speed_setting_mode=SYSDRV_FAN_FULL_SPEED;
                break;

            case SYSDRV_FAN_MID_SPEED:                  /* just detected overheat, set as transition speed mode, keep at current speed */
                new_fan_speed_setting_mode=SYSDRV_FAN_PREPARE_TRANSITION_SPEED;
                break;

            case SYSDRV_FAN_STOP_SPEED:                 /* all other cases, not used, should not happen */
            default:
                if (SYSDRV_SHOW_ERROR_MSG_FLAG() == TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Invalid fan speed mode(%lu)\r\n",
                        __FUNCTION__, __LINE__, sysdrv_shmem_data_p->sysdrv_speed_setting_mode);
                }
                break;
        }
    }
    else if(down_count==nbr_of_thermal)
    {
        switch ( sysdrv_shmem_data_p->sysdrv_speed_setting_mode )
        {
            case SYSDRV_FAN_FULL_SPEED:               /* system is cool enough, reduce current fan speed */
                new_fan_speed_setting_mode=SYSDRV_FAN_MID_SPEED;
                break;

            case SYSDRV_FAN_PREPARE_TRANSITION_SPEED: /* it has been too hot but ok now, set current fan speed mode to middle */
                new_fan_speed_setting_mode=SYSDRV_FAN_MID_SPEED;
                break;

            case SYSDRV_FAN_MID_SPEED:                /* keep current fan speed mode */
                new_fan_speed_setting_mode=SYSDRV_FAN_MID_SPEED;
                break;

            case SYSDRV_FAN_STOP_SPEED:               /* all other cases, not used, should not happen */
            default:
                if (SYSDRV_SHOW_ERROR_MSG_FLAG() == TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d)Invalid fan speed mode(%lu)\r\n",
                        __FUNCTION__, __LINE__, sysdrv_shmem_data_p->sysdrv_speed_setting_mode);
                }
                break;
        }
    }

    SYSDRV_TASK_UpdateFanSpeedModeByFanFailState(&new_fan_speed_setting_mode);

    /* check whether the evaluated new fan speed settings equals to the current
     * fan speed settings
     */
    if (new_fan_speed_setting_mode != *fan_speed_setting_mode_p)
    {
        *fan_speed_setting_mode_p=new_fan_speed_setting_mode;
        return TRUE;
    }
    return FALSE;
}
#else /* #if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE==SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL) */
#error "Incorrect definition of SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE"
#endif /* end of #if (SYS_CPNT_SYSDRV_FAN_SPEED_MODE_STATE_MACHINE_TYPE==SYS_CPNT_SYSDRV_FAN_SPEED_MODE_SM_TYPE_CHECK_COUNT_OF_CRIT_COND_THERMAL) */
#endif /* end of #if (SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL==TRUE) */
#endif /* #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE) */
#endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE) */

/* FUNCTION NAME: SYSDRV_TASK_DetectThermalStatus
 * PURPOSE: This routine is used to detection thermal status
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE  -  Success
 *          FALSE -  Failed
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_DetectThermalStatus(void)
{
    UI8_T nbr_of_thermal;

    nbr_of_thermal = STKTPLG_BOARD_GetThermalNumber();
    /* check temperature from each thermal sensor
     */
    {

        UI8_T i;
        I8_T  new_thermal_recv[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT];

        for (i=0; i<nbr_of_thermal; i++)
        {
            if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_THERMAL)
            {
                new_thermal_recv[i]=(I8_T)(sysdrv_shmem_data_p->sysdrv_thermal_temp_bd_dbg[i]);
            }
            else
            {
                if(SYSDRV_THERMAL_GetTemperature(i+1, &(new_thermal_recv[i])) != TRUE)
                {
                    /* EPR ID: ASF4628BBS5-FLF-EC-00289
                     * Headline: VLAN:Delete vlan 2-4094 via CLI & Web will display error message.
                     * Root cause: when deleting many vlans at a time, it would takes a lot of
                     * time(2-4094 vlan takes 10~12 minutes).
                     * The I2C_Read may sometime return errno with EINTR(4) EIO(5).
                     * So change to call SYSFUN_Debug_Printf to show
                     * the error message
                     */
                    SYSFUN_Debug_Printf("\r\n[SYSDRV] Failed to get temperature %d\n", i+1);
                    return FALSE;
                }
            }

            if (TRUE == SYSDRV_TASK_ThermalStatusStateMachine(sysdrv_shmem_data_p->sysdrv_thermal_temp[i], new_thermal_recv[i]))
            {
                SYSDRV_TASK_ENTER_CRITICAL_SECTION();
                if (FALSE == STKTPLG_POM_GetMyUnitID(&(sysdrv_shmem_data_p->sysdrv_my_unit_id)))
                {
                    printf("\r\n[SYSDRV] Failed to get my unit ID\r\n");
                }
                SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

                /* Send notification for the changed temperature
                 */
                SYSDRV_Notify_ThermalStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, i+1, new_thermal_recv[i]);

                SYSDRV_TASK_ENTER_CRITICAL_SECTION();
                sysdrv_shmem_data_p->sysdrv_thermal_temp[i] = new_thermal_recv[i];
                SYSDRV_TASK_LEAVE_CRITICAL_SECTION();
            }
        }
    }

    /* evaulate fan speed state
     */
#if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE)
    {
        UI32_T fan_speed_setting_mode = sysdrv_shmem_data_p->sysdrv_speed_setting_mode;

        /* evaluate new fan speed state according to the current condtion
         */
        if (
#if (SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT == TRUE)
            (sysdrv_skip_fan_speed_mode_state_machine==FALSE) &&
#endif
            (SYSDRV_TASK_FanSpeedModeStateMachine(&fan_speed_setting_mode)==TRUE)
            )
        {
            if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(%d):Fan Speed Mode change from %lu to %lu\r\n",
                    __FUNCTION__, __LINE__,
                    (unsigned long)sysdrv_shmem_data_p->sysdrv_speed_setting_mode,
                    (unsigned long)fan_speed_setting_mode);
            }

            SYSDRV_SetFanSpeedMode(fan_speed_setting_mode);

        }
    }
#elif ((SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && (SYS_HWCFG_FAN_DETECT_FAN_SPEED_CONTROL_BY_BID == TRUE))
    {
        UI32_T board_id = 0, i = 0;
        BOOL_T retval = FALSE;
        I32_T temperature = 0;
        UI32_T local_unit_fan_speed_mode = SYSDRV_FAN_STOP_SPEED;


        if (STKTPLG_OM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id) == FALSE)
        {
            printf("%s(%d): Failed to get board id.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }

        for (i = 0, temperature = 0; i < nbr_of_thermal; i++)
        {
            temperature += sysdrv_shmem_data_p->sysdrv_thermal_temp[i];
        }
        temperature /= nbr_of_thermal; /* Take care SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT and overflow case. */

        SYSDRV_TASK_ENTER_CRITICAL_SECTION();
        local_unit_fan_speed_mode = sysdrv_shmem_data_p->sysdrv_speed_setting_mode;
        if ((retval = SYS_HWCFG_SetFanSpeedModeByBID(board_id, temperature, &local_unit_fan_speed_mode)) == TRUE)
            sysdrv_shmem_data_p->sysdrv_speed_setting_mode = local_unit_fan_speed_mode;
        SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

        return retval;
    }
#endif /* end of #if (SYSDRV_TASK_HAS_FAN_CONTROLLER==TRUE) */

    return TRUE;
}

#if (SYS_HWCFG_SUPPORT_THERMAL_SHUTDOWN!=FALSE)
static BOOL_T SYSDRV_TASK_SetThermalShutdownThreshold(void)
{
    int index;
    I8_T temperature;
    BOOL_T status = FALSE;

    /* Set critical temperature (110 C) in thermal to enable auto power
    ** shutdown through EPLD
    */
    temperature = 110;
    for (index= 0; index < SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;index++)
        status = SYSDRV_THERMAL_SetThreshold(index+1,   temperature);

    return(status);
}
#endif /* end of #if (SYS_HWCFG_SUPPORT_THERMAL_SHUTDOWN!=FALSE) */

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

#if 0
/* FUNCTION NAME: SYSDRV_TASK_DetectXenpakStatus
 * PURPOSE: This function is used to detect 10G module Xenpak tranceiver status
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 *
 */
static void SYSDRV_TASK_DetectXenpakStatus(void)
{
    UI16_T temp;
    UI32_T medium_flag=0;
    UI8_T tempstr[20]={0};
    UI8_T sout[20]={0};
    int i;
    UI32_T tmp_unit;

    STKTPLG_POM_GetMyUnitID(&tmp_unit);

    /*detect whether Xenpak is present*/
    // soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, 0x8012, &temp);
     soc_miimc45_read(0, 0, 0x01, 0x8012, &temp);

    if( temp != 1 )
    {
        /* Xenpak not present */
        if( sysdrv_shmem_data_p->sysdrv_local_xenpak_status != SYS_DRV_XENPAK_NOT_PRESENT )
        {
            sysdrv_shmem_data_p->sysdrv_local_xenpak_status = SYS_DRV_XENPAK_NOT_PRESENT;
            SYSDRV_Notify_XenpakStatusChanged(tmp_unit, SYS_DRV_XENPAK_NOT_PRESENT);
        }
        return;
    }
    else /* Xenpak present */
    {
        if( (sysdrv_shmem_data_p->sysdrv_local_xenpak_status == SYS_DRV_XENPAK_NOT_PRESENT) ||
            (sysdrv_shmem_data_p->sysdrv_local_xenpak_status == 0) )
        {
            /* Reset Xenpak */
            soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, &temp);
            temp |= 0x8000;
            soc_miimc45_write(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, temp);
            SYSFUN_Sleep(10);
            temp &= 0x7FFF;
            soc_miimc45_write(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, temp);

            /* 2006.01.04 tc
             * XENPAK_MSA_R3.0.pdf states that a 5 seconds initialise time can be allowed.
             * Add maximum delay time (5s) to ensure successful read of eeprom.
             */
            SYSFUN_Sleep(500);

            /* Check PMA/PMD type */
            soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, 0x7, &temp);
            temp &= 0x7;
            if( temp == 0x6 )
            {
                medium_flag = SYS_DRV_XENPAK_UNSUPPORTED_LR;
            }
            else if( temp == 0x5 )
            {
                medium_flag = SYS_DRV_XENPAK_UNSUPPORTED_ER;
            }
            else if( temp == 0x0 )
            {
                medium_flag = SYS_DRV_XENPAK_UNSUPPORTED_CX4;
            }
            else
            {
                medium_flag = SYS_DRV_XENPAK_UNKNOWNTYPE;
            }

            /* Check for 3Com part number */
            for(i=0; i<10; i++)
            {
                soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, (0x80B2 + i), &temp);
                sprintf((char*)tempstr, "%c", temp);
                strcat((char*)sout, (char*)tempstr);
            }

            if( strcmp((char*)sout, SYS_DFLT_XENPAK_PARTNUMBER_LR) == 0 )
            {
                sysdrv_shmem_data_p->sysdrv_local_xenpak_status = SYS_DRV_XENPAK_SUPPORTED_LR;
            }
            else if( strcmp((char*)sout, SYS_DFLT_XENPAK_PARTNUMBER_ER) == 0 )
            {
                sysdrv_shmem_data_p->sysdrv_local_xenpak_status = SYS_DRV_XENPAK_SUPPORTED_ER;
            }
            else if( strcmp((char*)sout, SYS_DFLT_XENPAK_PARTNUMBER_CX4) == 0 )
            {
                sysdrv_shmem_data_p->sysdrv_local_xenpak_status = SYS_DRV_XENPAK_SUPPORTED_CX4;
                /* ES4649-32-00676 - Emcore CX4 requires a soft-reset
                 */
                /* Reset all transcivers therefore no longer require this part
                 */
                /*
                memset(sout, 0, 20);
                for(i=0; i<6; i++)
                {
                    soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, (0x803A + i), &temp);
                    sprintf(tempstr, "%c", temp);
                    strcat(sout, tempstr);
                }

                if( strcmp(sout, "EMCORE") == 0 )
                {
                    soc_miimc45_read(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, &temp);
                    temp |= 0x8000;
                    soc_miimc45_write(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, temp);
                    SYSFUN_Sleep(10);
                    temp &= 0x7FFF;
                    soc_miimc45_write(0, 0, PHY_C45_DEV_PMA_PMD, 0x00, temp);
                }
                */
            }
            else /* Unsupported Xenpak */
            {
                sysdrv_shmem_data_p->sysdrv_local_xenpak_status = medium_flag;
            }

            SYSDRV_Notify_XenpakStatusChanged(tmp_unit, sysdrv_shmem_data_p->sysdrv_local_xenpak_status);
        }
        return;
    }
}
#endif

/* FUNCTION NAME: SYSDRV_TASK_ReStartSystem
 * PURPOSE: This routine is used re-start the system
 * INPUT:   type --- SYS_VAL_COLD_START_FOR_RELOAD
 *                   SYS_VAL_WARM_START_FOR_RELOAD
 *                   SYS_VAL_COLD_START
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_ReStartSystem(UI32_T type)
{
    UI32_T *uc_boot_reason_check_ptr;
    UI8_T  command[32];

    //UI32_T          int_mask;
    UC_MGR_Sys_Info_T sys_info;
    //void            (*start_addr)(int);

    if (UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
         printf("\r\nFailed to get system infomation from UC\r\n");
         return;
    }

    FS_Shutdown();

    /*2004/10/20 Caesar Leong
      Direct raise the task priority at SYSDRV_ReloadSystem & SYSDRV_ColdStartSystem
      to prevent heavy loading make sysdrv task cannot up*/
    /*SYSFUN_SetTaskPriority(SYSFUN_TaskIdSelf(), SYS_BLD_RAISE_TO_HIGH_PRIORITY);*/

    SYSFUN_Sleep(SYS_ADPT_SYSTEM_RESTART_DELAY_TIME);

    uc_boot_reason_check_ptr = (UI32_T *)UC_MGR_Allocate(UC_MGR_BOOT_REASON_CHECKING_INDEX, sizeof(UI32_T), 4);

    if(uc_boot_reason_check_ptr==NULL)
    {
        printf("\r\n%s:UC_MGR_Allocate fail. Reload failed.", __FUNCTION__);
        return;
    }

    switch (type)
    {
        /* EPR ID: ES3510MA-FLF-38-00713
         * Headline: SNMP: OID warmBoot(2) and coldBoot(3)
         *           always show warm start
         * Description: The implementation of uboot for
         *              type=2 is warm reboot. Change
         *              COLD START type as 1 so uboot
         *              will treat it as cold start boot
         */
        #if 0
        case SYS_VAL_COLD_START_FOR_RELOAD:
            sprintf((char *)command, "type=2,startaddr=0x%lx,bootreasonaddr=0x%lx", (UI32_T)sys_info.warm_start_entry, (UI32_T)sys_info.boot_reason_addr);
            break;
        #endif

        case SYS_VAL_WARM_START_FOR_RELOAD:
            sprintf((char *)command, "type=3,startaddr=%p,bootreasonaddr=%p", sys_info.warm_start_entry, sys_info.boot_reason_addr);
            break;

        case SYS_VAL_COLD_START_FOR_RELOAD:
        case SYS_VAL_COLD_START:
            sprintf((char *)command, "type=1,startaddr=%p,bootreasonaddr=%p", sys_info.warm_start_entry, sys_info.boot_reason_addr);
            break;

        default:
            printf("Wrong system re-start type\r\n");
            return;
    }

    //SYSFUN_NonPreempty();

    //int_mask = SYSFUN_InterruptLock();

    /* Disable watchdog */
    SYS_TIME_DisableWatchDogTimer();

    /* Clear pending NMI for watchdog */
    SYSDRV_TASK_ClearNMI();

    LEDDRV_PortShutDown();

#if (SYS_CPNT_MGMT_PORT == TRUE)
    ADM_NIC_Shutdown();
#endif
#if (SYS_CPNT_SWDRV == TRUE)
    SWDRV_ShutdownSwitch();
#endif

    //SYSFUN_DisableInstructionCache();
    //SYSFUN_DisableDataCache();

    //int_mask = SYSFUN_InterruptLock();

    //SYSFUN_SetMasterStatusRegister(0);

    //start_addr(type); to do: remove

    /*
        * jerry.du 20080916, add for enganced reload command
        * type--reload type,
        * 1--cold restart,
        * 2--warm restart from reload
        * 3--cold restart from reload
        * startaddr is system restart reload address
        * if type == 1, startaddr is useless
        * this is temp use, must be modified!!!!
        */
#if 0
#if defined(ES3628BT_FLF_ZZ)
    //command = "type=1";
    command = "type=2,startaddr=0xfff00110";
    //command = "type=3,startaddr=0xfff00110";
#elif defined(ECN430_FB2)
    //command = "type=1";
    //command = "type=2,startaddr=0xfffff008";
    command = "type=3,startaddr=0xfffff008";
#endif
#endif
#if (SYS_CPNT_ONIE_SUPPORT==TRUE)
    /* On projects that support ONIE, the system is started and stopped by
     * a set of rc scripts. Execute "/sbin/reboot" will trigger the system
     * to execute the scripts to do required operations(e.g. unmount all mounted
     * file systems) before rebooting the system.
     */
    if(SYSFUN_ExecuteSystemShell("/sbin/reboot")==SYSFUN_OK)
    {
        SYSFUN_Sleep(60*SYS_BLD_TICKS_PER_SECOND); /* if the system cannot reboot in 60 seconds, continue to call SYSFUN_Reboot() */
    }
#endif

    SYSFUN_Reboot((char *)command);


    return;
}

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_SetAlarmOutputStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set the current alarm output status.
 *-----------------------------------------------------------------------------
 * INPUT    : none.
 * OUTPUT   : none.
 * RETURN   : TRUE if set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_TASK_SetAlarmOutputStatus(UI8_T status)
{
    BOOL_T ret_val=TRUE;
    UI32_T board_id;
    SYS_HWCFG_AlarmOutputRegInfo_T reg_info;

    if(STKTPLG_POM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id)==FALSE)
    {
        printf("%s: Failed to get unit board id\r\n", __FUNCTION__);
        return FALSE;
    }

    /* process major alarm output
     */
    if(SYS_HWCFG_GetMajorAlarmOutputRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s: Failed to get major alarm output reg info.\r\n", __FUNCTION__);
        ret_val=FALSE;
    }
    else
    {
        SYSDRV_TASK_SetAlarmOutputStatusByRegInfo(&reg_info, (status & SYSDRV_ALARM_OUTPUT_MAJOR_MASK)?TRUE:FALSE);
    }

    /* process minor alarm output
     */
    if(SYS_HWCFG_GetMinorAlarmOutputRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s: Failed to get minor alarm output reg info.\r\n", __FUNCTION__);
        ret_val=FALSE;
    }
    else
    {
        SYSDRV_TASK_SetAlarmOutputStatusByRegInfo(&reg_info, (status & SYSDRV_ALARM_OUTPUT_MINOR_MASK)?TRUE:FALSE);
    }

    return ret_val;
}

/* FUNCTION NAME: SYSDRV_TASK_SetAlarmOutputStatusByRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set the alarm output status
 *-----------------------------------------------------------------------------
 * INPUT    : reg_info_p - register information for controlling alarm output
 *            set_alarm  - TRUE if alarm output will be enabled.
 * OUTPUT   : none.
 * RETURN   : TRUE if alarm output status is set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_TASK_SetAlarmOutputStatusByRegInfo(SYS_HWCFG_AlarmOutputRegInfo_T *reg_info_p, BOOL_T set_alarm)
{
    switch(reg_info_p->access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            if(set_alarm)
                return SYSDRV_TASK_SetI2CReg(&(reg_info_p->info.i2c), reg_info_p->mask, reg_info_p->output_enable_val);

            return SYSDRV_TASK_SetI2CReg(&(reg_info_p->info.i2c), reg_info_p->mask, ~(reg_info_p->output_enable_val));
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR:
            if(set_alarm)
                return SYSDRV_TASK_SetPhyAddrReg(&(reg_info_p->info.phy_addr), reg_info_p->mask, reg_info_p->output_enable_val);

            return SYSDRV_TASK_SetPhyAddrReg(&(reg_info_p->info.phy_addr), reg_info_p->mask, ~(reg_info_p->output_enable_val));
        default:
            printf("%s: unknown access method(%hu)\r\n", __FUNCTION__, reg_info_p->access_method);
            break;
    }
    return FALSE;
}

#if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV!=TRUE)
/* FUNCTION NAME: SYSDRV_TASK_SetAlarmOutputLed
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set the current alarm output led.
 *-----------------------------------------------------------------------------
 * INPUT    : status -- (SYSDRV_ALARM_OUTPUT_MAJOR|SYSDRV_ALARM_OUTPUT_MINOR)
 * OUTPUT   : none.
 * RETURN   : TRUE if set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_TASK_SetAlarmOutputLed(UI8_T status)
{
    BOOL_T ret_val=TRUE;
    UI32_T board_id;
    SYS_HWCFG_AlarmOutputLedRegInfo_T reg_info;

    if(STKTPLG_POM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id)==FALSE)
    {
        printf("%s: Failed to get unit board id\r\n", __FUNCTION__);
        return FALSE;
    }

    /* process major alarm led
     */
    if(SYS_HWCFG_GetMajorAlarmOutputLedRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s: Failed to get major alarm output led reg info.\r\n", __FUNCTION__);
        ret_val=FALSE;
    }
    else
    {
        SYSDRV_TASK_SetAlarmOutputLedByRegInfo(&reg_info, (status & SYSDRV_ALARM_OUTPUT_MAJOR_MASK)?TRUE:FALSE);
    }

    /* process minor alarm led
     */
    if(SYS_HWCFG_GetMinorAlarmOutputLedRegInfo(board_id, &reg_info)==FALSE)
    {
        printf("%s: Failed to get minor alarm output led reg info.\r\n", __FUNCTION__);
        ret_val=FALSE;
    }
    else
    {
        SYSDRV_TASK_SetAlarmOutputLedByRegInfo(&reg_info, (status & SYSDRV_ALARM_OUTPUT_MINOR_MASK)?TRUE:FALSE);
    }

    return ret_val;
}

/* FUNCTION NAME: SYSDRV_TASK_SetAlarmOutputLedByRegInfo
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set the alarm output led
 *-----------------------------------------------------------------------------
 * INPUT    : reg_info_p - register information for controlling alarm output led
 *            set_led_on  - TRUE if alarm output led is on
 * OUTPUT   : none.
 * RETURN   : TRUE if alarm output led is set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_TASK_SetAlarmOutputLedByRegInfo(SYS_HWCFG_AlarmOutputLedRegInfo_T *reg_info_p, BOOL_T set_led_on)
{
    switch(reg_info_p->access_method)
    {
        case SYS_HWCFG_REG_ACCESS_METHOD_I2C:
            if(set_led_on)
                return SYSDRV_TASK_SetI2CReg(&(reg_info_p->info.i2c), reg_info_p->mask, reg_info_p->led_on_val);

            return SYSDRV_TASK_SetI2CReg(&(reg_info_p->info.i2c), reg_info_p->mask, ~(reg_info_p->led_on_val));
            break;
        case SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR:
            if(set_led_on)
                return SYSDRV_TASK_SetPhyAddrReg(&(reg_info_p->info.phy_addr), reg_info_p->mask, reg_info_p->led_on_val);

            return SYSDRV_TASK_SetPhyAddrReg(&(reg_info_p->info.phy_addr), reg_info_p->mask, ~(reg_info_p->led_on_val));
            break;
        default:
            printf("%s: unknown access method(%hu)\r\n", __FUNCTION__, reg_info_p->access_method);
            break;
    }
    return FALSE;
}
#endif /* end of #if (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV!=TRUE) */

/* FUNCTION NAME: SYSDRV_TASK_SetI2CReg
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set specified value to the register through i2c
 *-----------------------------------------------------------------------------
 * INPUT    : reg_info_p  -  information of access the register
 *            mask        -  The mask of modifying register value. Only those
 *                           bits with value 1 will be changed.
 *            data        -  The data to be set to the register
 * OUTPUT   : none.
 * RETURN   : TRUE if set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: The expression of the data to be written to the register is shown below:
 *        (original_value & ~(mask)) | (data&mask)
 */
static BOOL_T SYSDRV_TASK_SetI2CReg(SYS_HWCFG_i2cRegInfo_T *reg_info_p, UI32_T mask, UI32_T data)
{
    union
    {
        UI8_T  u8;
        UI16_T u16;
        UI32_T u32;
    } local_data;

    /* only support data_len=1 for now,
     */
    if(reg_info_p->data_len>1)
    {
        printf("%s: not support i2c data len greaten than 1\r\n", __FUNCTION__);
        return FALSE;
    }


    /* read original value
     */
    if (!I2CDRV_TwsiDataRead(reg_info_p->dev_addr, reg_info_p->op_type,
        reg_info_p->validOffset, reg_info_p->offset, reg_info_p->moreThen256,
        reg_info_p->data_len, &(local_data.u8)))
    {
        printf("%s: DEV_SWDRV_PMGR_TwsiDataRead to offset=0x%x in slave addr=0x%x failed\r\n", __FUNCTION__, reg_info_p->offset, reg_info_p->dev_addr);
        return FALSE;
    }

    /* evaluate value to be set
     */
    local_data.u8 &= (UI8_T)(~mask);
    local_data.u8 |= (UI8_T)(data & mask);


    /* set value to the register
     */
    if (!I2CDRV_TwsiDataWrite(reg_info_p->dev_addr, reg_info_p->op_type,
        reg_info_p->validOffset, reg_info_p->offset, reg_info_p->moreThen256,
        &(local_data.u8), reg_info_p->data_len))
    {
        printf("%s: DEV_SWDRV_PMGR_TwsiDataWrite data=0x%x to offset=0x%x in slave addr=0x%x failed\r\n", __FUNCTION__, local_data.u8, reg_info_p->offset, reg_info_p->dev_addr);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME: SYSDRV_TASK_SetPhyAddrReg
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will set specified value to the register through phy
 *          addr access API.
 *-----------------------------------------------------------------------------
 * INPUT    : reg_info_p  -  information of access the register
 *            mask        -  The mask of modifying register value. Only those
 *                           bits with value 1 will be changed.
 *            data        -  The data to be set to the register
 * OUTPUT   : none.
 * RETURN   : TRUE if set sucessfully.
 *-----------------------------------------------------------------------------
 * NOTES: The expression of the data to be written to the register is shown below:
 *        (original_value & ~(mask)) | (data&mask)
 */
static BOOL_T SYSDRV_TASK_SetPhyAddrReg(SYS_HWCFG_phyAddrRegInfo_T* reg_info_p, UI32_T mask, UI32_T data)
{
    SYS_TYPE_VAddr_T alarm_led_addr;
    union
    {
        UI8_T  u8;
        UI16_T u16;
        UI32_T u32;
    } local_data;
    UI8_T  data_type_byte_len;

    local_data.u32=0;
    switch(reg_info_p->data_type_bit_len)
    {
        case 8:
            data_type_byte_len=1;
            break;
        case 16:
            data_type_byte_len=2;
            break;
        case 32:
            data_type_byte_len=4;
            break;
        default:
            printf("%s(%d)Invalid data type bit length=%hu\r\n", __FUNCTION__, __LINE__, reg_info_p->data_type_bit_len);
            return FALSE;
            break;
    }

    if((reg_info_p->data_len/data_type_byte_len)>1)
    {
        printf("%s(%d)Not support more than 1 data element.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(reg_info_p->physical_address, &alarm_led_addr))
    {
        printf("%s:Failed to access physicall address 0x%08lx\r\n", __FUNCTION__, reg_info_p->physical_address);
        return FALSE;
    }

    if(FALSE==PHYADDR_ACCESS_Read(alarm_led_addr, data_type_byte_len, 1, &(local_data.u8)))
    {
        printf("%s: read addr=0x%lx fail\r\n", __FUNCTION__, alarm_led_addr);
        return FALSE;
    }

    switch(reg_info_p->data_type_bit_len)
    {
        case 8:
            local_data.u8 = (UI8_T)((local_data.u8&(~mask)) | data);
            break;
        case 16:
            local_data.u16 = (UI16_T)((local_data.u16&(~mask)) | data);
            break;
        case 32:
            local_data.u32 = (UI32_T)((local_data.u32&(~mask)) | data);
            break;
        default:
            printf("%s(%d)Invalid data type bit length=%hu\r\n", __FUNCTION__, __LINE__, reg_info_p->data_type_bit_len);
            return FALSE;
            break;
    }


    if(FALSE==PHYADDR_ACCESS_Write(alarm_led_addr, data_type_byte_len, 1, &(local_data.u8)))
    {
        printf("%s: write data=0x%lx to addr=0x%lx fail\r\n", __FUNCTION__, local_data.u32, alarm_led_addr);
        return FALSE;
    }

    return TRUE;
}

#endif /* #if (SYS_CPNT_ALARM_DETECT == TRUE) */

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_GetAlarmInputStatus
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will get the current alarm input status.
 *-----------------------------------------------------------------------------
 * INPUT    : none.
 * OUTPUT   : status_p - alarm input status
 * RETURN   : TRUE/FALSE
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_TASK_GetAlarmInputStatus(UI8_T *status_p)
{

#if (SYS_HWCFG_SYSTEM_ALARM_INPUT_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    if(!I2CDRV_GetI2CInfo(SYS_HWCFG_SYSTEM_ALARM_INPUT_ADDR, SYS_HWCFG_SYSTEM_ALARM_INPUT_OFFSET, 1, status_p))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s: Failed to access salve address 0x%08X offset 0x%08X\r\n", __FUNCTION__, SYS_HWCFG_SYSTEM_ALARM_INPUT_ADDR, SYS_HWCFG_SYSTEM_ALARM_INPUT_OFFSET);
        }
        return FALSE;
    }
#elif (SYS_HWCFG_SYSTEM_ALARM_INPUT_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_SYS_HWCFG_API)
    UI32_T board_id;

    STKTPLG_POM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id);
    if(!SYS_HWCFG_GetAlarmInputStatus(board_id, status_p))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s: SYS_HWCFG_GetAlarmInputStatus() failed \r\n", __FUNCTION__);
        }
        return FALSE;
    }
#elif (SYS_HWCFG_SYSTEM_ALARM_INPUT_REG_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR)
    if(PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_SYSTEM_ALARM_INPUT_ADDR, 1, 1, status_p)==FALSE)
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s: Failed to access physical address 0x%08X\r\n", __FUNCTION__, SYS_HWCFG_SYSTEM_ALARM_INPUT_ADDR);
        }
        return FALSE;
    }
#endif /* end of #if defined(ASF4512MP) */

    if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        BACKDOOR_MGR_Printf("%s: alarm input status=0x%02X\r\n", __FUNCTION__, *status_p);
    }

    return TRUE;
}
#endif /* #if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE) */

#if (SYS_CPNT_ALARM_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_GetMajorAlarmStatus
 * PURPOSE: This routine is used to get major alarm status
 * INPUT:   None.
 * OUTPUT:  current_status  -  current major alarm status
 * RETURN:  TRUE  - Major alarm status is on
 *          FALSE - Major alarm status is off
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_GetMajorAlarmStatus(UI8_T *current_status)
{
#if (SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER == TRUE)
    {
#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
        SYS_HWCFG_PowerRegInfo_T power_reg_info;
#endif
        BOOL_T dual_power_status_ok=TRUE;
        UI8_T status_mask_pwr;
        UI8_T status_mask_rps;
        UI8_T status_ok_pwr;
        UI8_T status_ok_rps;
        UI8_T present_mask_pwr;
        UI8_T present_mask_rps;
        UI8_T is_present_val_pwr;
        UI8_T is_present_val_rps;

#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    if (STKTPLG_BOARD_GetPowerStatusInfo(1, &power_reg_info) == TRUE)
    {
        status_mask_pwr=power_reg_info.power_status_mask;
        status_ok_pwr=power_reg_info.power_status_ok_val;
        present_mask_pwr=power_reg_info.power_present_mask;
        is_present_val_pwr=power_reg_info.power_is_present_val;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s:STKTPLG_BOARD_GetPowerStatusInfo failed.(power_index=1)\r\n", __FUNCTION__);
        return FALSE;
    }
    if (STKTPLG_BOARD_GetPowerStatusInfo(2, &power_reg_info) == TRUE)
    {
        status_mask_rps=power_reg_info.power_status_mask;
        status_ok_rps=power_reg_info.power_status_ok_val;
        present_mask_rps=power_reg_info.power_present_mask;
        is_present_val_rps=power_reg_info.power_is_present_val;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s:STKTPLG_BOARD_GetPowerStatusInfo failed.(power_index=1)\r\n", __FUNCTION__);
        return FALSE;
    }
#else /* #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */
        status_mask_pwr=SYS_HWCFG_PWR_STATUS_MASK;
        status_mask_rps=SYS_HWCFG_RPS_STATUS_MASK;
        status_ok_pwr=SYS_HWCFG_PWR_STATUS_OK;
        status_ok_rps=SYS_HWCFG_RPS_STATUS_OK;
        present_mask_pwr=SYS_HWCFG_PWR_PRES_MASK;
        present_mask_rps=SYS_HWCFG_RPS_PRES_MASK;
        is_present_val_pwr=SYS_HWCFG_PWR_PRES_OK;
        is_present_val_rps=SYS_HWCFG_RPS_PRES_OK;
#endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */
        #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT!=2)
        #error "Conflict value of SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT and SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER."
        #endif
        /* check power present
         */
        if(((sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0] & present_mask_pwr) != is_present_val_pwr) ||
           ((sysdrv_shmem_data_p->sysdrv_local_unit_power_status[1] & present_mask_rps) != is_present_val_rps)
          )
        {
            dual_power_status_ok=FALSE;
        }

        /* check power status
         */
        #if defined(ECS4810_12MV2)
        {
            UC_MGR_Sys_Info_T sys_info;

            if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
            {
                printf("%s(%d)Failed to get sysinfo from UC.\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            if(sys_info.board_id!=1)
            {
                status_mask_pwr  = SYS_HWCFG_PWR_STATUS_MASK_BID0;
                status_ok_pwr    = SYS_HWCFG_PWR_STATUS_OK_BID0;
                status_mask_rps  = SYS_HWCFG_RPS_STATUS_MASK_BID0;
                status_ok_rps    = SYS_HWCFG_RPS_STATUS_OK_BID0;
            }
        }
        #endif /* end of #if defined(ECS4810_12MV2) */

        if(((sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0] & status_mask_pwr) != status_ok_pwr) ||
           ((sysdrv_shmem_data_p->sysdrv_local_unit_power_status[1] & status_mask_rps) != status_ok_rps)
          )
        {
            dual_power_status_ok=FALSE;
        }

        if(dual_power_status_ok==FALSE)
        {
            *current_status |= SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK;
        }
        else
        {
           *current_status &= ~(SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK);
        }
    }
#else /* #if (SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER == TRUE) */
    if((sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0]&SYS_HWCFG_PWR_STATUS_MASK)!=SYS_HWCFG_PWR_STATUS_OK)
    {
        *current_status |= SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK;
    }
    else
    {
        *current_status &= ~(SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK);
    }

#endif /* end of #if (SYS_CPNT_ALARM_OUTPUT_MAJOR_DUAL_POWER == TRUE) */

#if (SYS_CPNT_ALARM_OUTPUT_FAN_ALARM == TRUE)
    if(SYSDRV_GetFanFailNum()==nbr_of_fan)
    {
        *current_status |= SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK;
    }
    else
    {
        *current_status &= ~SYSDRV_ALARMMAJORSTATUS_ALL_FAN_FAILURE_MASK;
    }

#endif /* end of #if (SYS_CPNT_ALARM_OUTPUT_FAN_ALARM == TRUE) */

#if (SYS_CPNT_ALARM_OUTPUT_MAJOR_POWER_MODULE_SET_STATUS==TRUE)
    #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE!=TRUE)
    #error "SYS_CPNT_SUPPORT_POWER_MODULE_TYPE should be TRUE"
    #endif
    {
        BOOL_T is_pwr_module_set_status_good;

        if(SYSDRV_GetPowerModuleSetStatus(&is_pwr_module_set_status_good)==TRUE)
        {
            if(is_pwr_module_set_status_good==FALSE)
                *current_status |= SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK;
            else
                *current_status &= ~SYSDRV_ALARMMAJORSTATUS_POWER_MODULE_SET_WRONG_MASK;
        }
        else
        {
            if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(%d):SYSDRV_GetPowerModuleSetStatus failed.\r\n",
                    __FUNCTION__, __LINE__);
            }
        }
    }
#endif
    return (*current_status)?TRUE:FALSE;
}

/* FUNCTION NAME: SYSDRV_TASK_GetMinorAlarmStatus
 * PURPOSE: This routine is used to get minor alarm status
 * INPUT:   None.
 * OUTPUT:  current_status  -  current minor alarm status
 * RETURN:  TRUE  - Minor alarm status is on
 *          FALSE - Minor alarm status is off
 * NOTES:   None.
 */
static BOOL_T SYSDRV_TASK_GetMinorAlarmStatus(UI8_T *current_status)
{
#if (SYS_CPNT_ALARM_OUTPUT_MINOR_OVERHEAT == TRUE && SYS_CPNT_THERMAL_DETECT==TRUE)
    {
        I32_T thermal_threshold[] = SYS_ADPT_THERMAL_THRESHOLD_UP_ARRAY;
        UI8_T thermal_idx;
        UI8_T overheat_count;

        /* check temperature
         */
        for(thermal_idx=0,overheat_count=0; thermal_idx<SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; thermal_idx++)
        {
            if(sysdrv_shmem_data_p->sysdrv_thermal_temp[thermal_idx]>thermal_threshold[thermal_idx])
                overheat_count++;
        }

        if(overheat_count==SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
        {
            *current_status |= SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK;
        }
        else
        {
            *current_status &= ~SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK;
        }
    }
#endif

#if (SYS_CPNT_ALARM_OUTPUT_FAN_ALARM == TRUE)
    {
        UI32_T fail_count = SYSDRV_GetFanFailNum();

        if(fail_count>0 && fail_count<nbr_of_fan)
        {
            *current_status |= SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK;
        }
        else
        {
            *current_status &= ~SYSDRV_ALARMMINORSTATUS_PARTIAL_FAN_FAILURE_MASK;
        }
    }
#endif /* end of #if (SYS_CPNT_ALARM_OUTPUT_FAN_ALARM == TRUE) */

    return (*current_status)?TRUE:FALSE;
}

/* FUNCTION NAME: SYSDRV_TASK_DetectAlarmStatus
 * PURPOSE: This routine is used to detection alarm status
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
static void SYSDRV_TASK_DetectAlarmStatus(void)
{
    UI8_T current_major_alarm_status=0;
    UI8_T current_minor_alarm_status=0;
    UI8_T System_Major_alarm_flag;
    UI8_T System_Minor_alarm_flag;
    static UI8_T alarm_output_status;
    BOOL_T alarm_output_status_changed=FALSE;
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    UI8_T  current_alarm_input_status;
    BOOL_T ret=FALSE;
#endif

    System_Major_alarm_flag = SYSDRV_TASK_GetMajorAlarmStatus(&current_major_alarm_status);
    System_Minor_alarm_flag = SYSDRV_TASK_GetMinorAlarmStatus(&current_minor_alarm_status);

    if (current_major_alarm_status != sysdrv_shmem_data_p->sysdrv_majorAlarmType_bitmap)
    {
        alarm_output_status_changed=TRUE;
        if(System_Major_alarm_flag==TRUE)
        {
            alarm_output_status |= SYSDRV_ALARM_OUTPUT_MAJOR;
        }
        else
        {
            alarm_output_status &= ~(SYSDRV_ALARM_OUTPUT_MAJOR_MASK);
        }
        SYSDRV_Notify_MajorAlarmOutputStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, current_major_alarm_status);
    }

    if (current_minor_alarm_status != sysdrv_shmem_data_p->sysdrv_minorAlarmType_bitmap)
    {
        alarm_output_status_changed=TRUE;
        if(System_Minor_alarm_flag==TRUE)
        {
            alarm_output_status |= SYSDRV_ALARM_OUTPUT_MINOR;
        }
        else
        {
            alarm_output_status &= ~(SYSDRV_ALARM_OUTPUT_MINOR_MASK);
        }
        SYSDRV_Notify_MinorAlarmOutputStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, current_minor_alarm_status);
    }

    if(alarm_output_status_changed)
    {
        SYSDRV_TASK_SetAlarmOutputStatus(alarm_output_status);
        SYSDRV_TASK_SetAlarmOutputLed(alarm_output_status);

        SYSDRV_TASK_ENTER_CRITICAL_SECTION();
        sysdrv_shmem_data_p->sysdrv_majorAlarmType_bitmap = current_major_alarm_status;
        sysdrv_shmem_data_p->sysdrv_minorAlarmType_bitmap = current_minor_alarm_status;
        SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

    }

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    ret = SYSDRV_TASK_GetAlarmInputStatus(&current_alarm_input_status);
    if(ret == TRUE)
    {
        current_alarm_input_status &= (SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK);

        if (current_alarm_input_status != sysdrv_shmem_data_p->sysdrv_AlarmInputType_bitmap)
        {
            SYSDRV_Notify_AlarmInputStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id, current_alarm_input_status);

            SYSDRV_TASK_ENTER_CRITICAL_SECTION();
            sysdrv_shmem_data_p->sysdrv_AlarmInputType_bitmap = current_alarm_input_status;
            SYSDRV_TASK_LEAVE_CRITICAL_SECTION();
        }
    }

#endif
    return;
}
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_DetectPowerStatus
 * PURPOSE: This routine is used to detection power status
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
static void SYSDRV_TASK_DetectPowerStatus(void)
{
    UI8_T  temp_local_unit_pwr_status[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
    UI32_T new_status, i;

#if defined(ASF4512MP)
    UI32_T board_id;

#define PCA8574A_BIT_ACDC 0x3f

    STKTPLG_POM_GetUnitBoardID(sysdrv_shmem_data_p->sysdrv_my_unit_id, &board_id);

    if(board_id==1) /* ase3526 */
    {
#ifdef SYS_HWCFG_I2C_SLAVE_PCA9538PWR
        if (!DEV_SWDRV_PMGR_TwsiDataRead(SYS_HWCFG_I2C_SLAVE_PCA9538PWR, 0, 1, 0, 0, 1, &temp_local_unit_pwr_status[0]))
        {
            printf("%s: DEV_SWDRV_PMGR_TwsiDataRead offset=0x%x from slave addr=0x%x failed\r\n", __FUNCTION__, 0, SYS_HWCFG_I2C_SLAVE_PCA9538PWR);
        }
#else
        if (!DEV_SWDRV_PMGR_TwsiDataRead(SYS_HWCFG_I2C_SLAVE_PCA8574A, 0, 1, PCA8574A_BIT_ACDC, 0, 1, &temp_local_unit_pwr_status[0]))
        {
            printf("%s: DEV_SWDRV_PMGR_TwsiDataRead offset=0x%x from slave addr=0x%x failed\r\n", __FUNCTION__, PCA8574A_BIT_ACDC, SYS_HWCFG_I2C_SLAVE_PCA8574A);
        }
#endif
        temp_local_unit_pwr_status[0] = (temp_local_unit_pwr_status[0]<<4);
        temp_local_unit_pwr_status[1] = temp_local_unit_pwr_status[0];
    }
    else
#endif
    {
        if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_POWER)
        {
            memcpy(temp_local_unit_pwr_status, sysdrv_shmem_data_p->sysdrv_local_unit_power_status_bd_dbg, sizeof(temp_local_unit_pwr_status));
        }
        else
        {
            for(i=0; i<STKTPLG_BOARD_UTIL_GetPowerNumber(); i++)
            {
                if(FALSE==SYSDRV_GetPowerStatusFromASIC(i+1, &temp_local_unit_pwr_status[i]))
                {
                    if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
                        BACKDOOR_MGR_Printf("%s: SYSDRV_GetPowerStatusFromASIC error(i=%lu) fail", __FUNCTION__, (unsigned long)i);
                    return;
                }
            }
        }
    }

    /* main power
     */
    if (TRUE == SYSDRV_TASK_PowerStateMachine(VAL_swIndivPowerIndex_internalPower,
                                         sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0],
                                         temp_local_unit_pwr_status[0],
                                         &new_status))
    {
        SYSDRV_Notify_PowerStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id,
                                         VAL_swIndivPowerIndex_internalPower, new_status);

        #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        if(new_status!=VAL_swIndivPowerStatus_notPresent)
        {
            UI32_T power_type;

            if(SYSDRV_GetPowerModuleType(VAL_swIndivPowerIndex_internalPower, &power_type)==FALSE)
            {
                if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
                {
                    BACKDOOR_MGR_Printf("%s(%d):SYSDRV_GetPowerModuleType failed.\r\n",
                        __FUNCTION__, __LINE__);
                }
            }
            else
            {
                if(power_type!=sysdrv_shmem_data_p->sysdrv_local_unit_power_type[0])
                {
                    SYSDRV_Notify_PowerTypeChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id,
                        VAL_swIndivPowerIndex_internalPower, power_type);
                    sysdrv_shmem_data_p->sysdrv_local_unit_power_type[0]=power_type;
                }
            }
        }
        #endif
    }

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT >= 2) /*s'pose no third power*/
#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT > 2)
#warning "Not handle power index larger than 2"
#endif

    /* redundant power
     */
    if (TRUE == SYSDRV_TASK_PowerStateMachine(VAL_swIndivPowerIndex_externalPower,
                                         sysdrv_shmem_data_p->sysdrv_local_unit_power_status[1],
                                         temp_local_unit_pwr_status[1],
                                         &new_status))
    {
        SYSDRV_Notify_PowerStatusChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id,
                                         VAL_swIndivPowerIndex_externalPower, new_status);

        #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        if(new_status!=VAL_swIndivPowerStatus_notPresent)
        {
            UI32_T power_type;

            if(SYSDRV_GetPowerModuleType(VAL_swIndivPowerIndex_externalPower, &power_type)==FALSE)
            {
                    BACKDOOR_MGR_Printf("%s(%d):SYSDRV_GetPowerModuleType failed.\r\n",
                        __FUNCTION__, __LINE__);
            }
            else
            {
                if(power_type!=sysdrv_shmem_data_p->sysdrv_local_unit_power_type[1])
                {
                    SYSDRV_Notify_PowerTypeChanged(sysdrv_shmem_data_p->sysdrv_my_unit_id,
                        VAL_swIndivPowerIndex_externalPower, power_type);
                    sysdrv_shmem_data_p->sysdrv_local_unit_power_type[1]=power_type;
                }
            }
        }
        #endif

    }
#endif

    SYSDRV_TASK_ENTER_CRITICAL_SECTION();
    memcpy(sysdrv_shmem_data_p->sysdrv_local_unit_power_status,
        temp_local_unit_pwr_status,
        sizeof(UI8_T)*SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT);
    SYSDRV_TASK_LEAVE_CRITICAL_SECTION();

    return;
}
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_TASK_PowerStateMachine
 * PURPOSE: This routine is used to run power state machine
 * INPUT:   pwr_id     - Power index, valid values are started from 1 to SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT.
 *          pre_val    - the power state value of the previous time
 *          now_val    - the power state value of the current time
 * OUTPUT:  new_status - the evaluation result of power status
 * RETURN:  TRUE --- status changed.
 *          FALSE --- status not changed.
 * NOTES:   none */
static BOOL_T SYSDRV_TASK_PowerStateMachine(UI32_T pwr_id, UI8_T pre_val,
                                            UI8_T  now_val, UI32_T *new_status)
{
#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)

    if (pre_val != now_val)
    {
        /* check present status change?
         */
        if ((pre_val & ONLPDRV_PSU_STATUS_PRESENT) != (now_val & ONLPDRV_PSU_STATUS_PRESENT))
        {
            if ((now_val & ONLPDRV_PSU_STATUS_PRESENT) == ONLPDRV_PSU_STATUS_PRESENT)
            {
                /* "not present" => "present"
                 */
                if ((now_val & ONLPDRV_PSU_STATUS_UNPLUGGED) != ONLPDRV_PSU_STATUS_UNPLUGGED)
                {
                    /* AC on
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                else
                {
                    /* AC off
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }

                if ((*new_status == VAL_swIndivPowerStatus_green) &&
                    ((now_val & ONLPDRV_PSU_STATUS_FAILED) == ONLPDRV_PSU_STATUS_FAILED)
                   )
                {
                    /* alert is asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }
            }
            else /* if ((now_val & ONLPDRV_PSU_STATUS_PRESENT) == ONLPDRV_PSU_STATUS_PRESENT) */
            {
                /* "present" => "not present"
                 */
                *new_status = VAL_swIndivPowerStatus_notPresent;
            }
        }
        else /* if ((pre_val & ONLPDRV_PSU_STATUS_PRESENT) != (now_val & ONLPDRV_PSU_STATUS_PRESENT)) */
        {
            if ((now_val & ONLPDRV_PSU_STATUS_PRESENT) == ONLPDRV_PSU_STATUS_PRESENT)
            {
                /* present state is the same => only level state change => must be present
                 */
                if ((now_val & ONLPDRV_PSU_STATUS_UNPLUGGED) != ONLPDRV_PSU_STATUS_UNPLUGGED)
                {
                    /* AC off => AC on
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                else
                {
                    /* AC on => AC off
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }

                if ((*new_status == VAL_swIndivPowerStatus_green) &&
                    ((now_val & ONLPDRV_PSU_STATUS_FAILED) == ONLPDRV_PSU_STATUS_FAILED)
                   )
                {
                    /* alert is asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }
            }
        }
        return TRUE;
    } /* end of if (pre_val != now_val) */

    return FALSE;
#else /* #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */
/* Old projects does not have definition of SYS_CPNT_SYSDRV_POWER_SM_TYPE,
 * assumes that these projects would use SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1
 * as its power state machine type
 */
#if (!defined(SYS_CPNT_SYSDRV_POWER_SM_TYPE)) || (SYS_CPNT_SYSDRV_POWER_SM_TYPE == SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1)
/*Below is the old method
  Suppose H/W EPLD register generic design as:
 ------------------------------------------------------------------------------------------
 |     7     |     6    |    5     |     4    |    3     |     2    |    1     |     0    |
 |--------------------------------------------|----------|----------|----------|----------|
 |                    Reseved                 | RPS_PRES | PWR_PRES |RPS_STATUS|PWR_STATUS|
 ------------------------------------------------------------------------------------------
 */
#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    SYS_HWCFG_PowerRegInfo_T power_reg_info;
#endif

    UI8_T present_mask;
    UI8_T status_mask;

    UI8_T present_ok;
    UI8_T status_ok;

    BOOL_T retval;

#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    if (STKTPLG_BOARD_GetPowerStatusInfo(pwr_id, &power_reg_info) == TRUE)
    {
        present_mask = power_reg_info.power_present_mask;
        status_mask  = power_reg_info.power_status_mask;
        present_ok   = power_reg_info.power_is_present_val;
        status_ok    = power_reg_info.power_status_ok_val;
    }
    else
    {
        BACKDOOR_MGR_Printf("%s(%d)STKTPLG_BOARD_GetPowerStatusInfo failed.(power_index=%lu)\r\n", __FUNCTION__, __LINE__, pwr_id);
        return FALSE;
    } /* end of if (STKTPLG_BOARD_GetPowerStatusInfo(pwr_id, &power_reg_info) == TRUE) */
#else
    switch(pwr_id)
    {
        case 1:
            present_mask = SYS_HWCFG_PWR_PRES_MASK;
            status_mask  = SYS_HWCFG_PWR_STATUS_MASK;
            present_ok   = SYS_HWCFG_PWR_PRES_OK;
            status_ok    = SYS_HWCFG_PWR_STATUS_OK;
            break;

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2)
        case 2:
            present_mask = SYS_HWCFG_RPS_PRES_MASK;
            status_mask  = SYS_HWCFG_RPS_STATUS_MASK;
            present_ok   = SYS_HWCFG_RPS_PRES_OK;
            status_ok    = SYS_HWCFG_RPS_STATUS_OK;
            break;
#endif

        default:
            return FALSE;
    }

    #if defined(ECS4810_12MV2)
    {
        UC_MGR_Sys_Info_T sys_info;

        if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
        {
            printf("%s(%d)Failed to get sysinfo from UC.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
        if(sys_info.board_id!=1)
        {
            switch(pwr_id)
            {

                case 1:
                    status_mask  = SYS_HWCFG_PWR_STATUS_MASK_BID0;
                    status_ok    = SYS_HWCFG_PWR_STATUS_OK_BID0;
                    break;
                case 2:
                    status_mask  = SYS_HWCFG_RPS_STATUS_MASK_BID0;
                    status_ok    = SYS_HWCFG_RPS_STATUS_OK_BID0;
                    break;
            }
        }
    }
    #endif /* end of #if defined(ECS4810_12MV2) */
#endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */

    retval = FALSE;

    if ((pre_val & (present_mask | status_mask)) != (now_val & (present_mask | status_mask)) )
    {
        /* check present status change?
         */
        if ((pre_val & present_mask) != (now_val & present_mask))
        {
            if ((now_val & present_mask) == present_ok)
            {
                /* "not present" => "present"
                 */
                if ((now_val & status_mask) == status_ok)
                {
                    /* AC on
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                else
                {
                    /* AC off
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }
            }
            else
            {
                /* "present" => "not present"
                 */
                *new_status = VAL_swIndivPowerStatus_notPresent;
            }

            retval = TRUE;
        }
        else
        {
            /* According to HW, status bit must be checked with RPS presented. */
            /* Otherwise, status bit may be affected by noise                  */
            if ((now_val & present_mask) == present_ok)
            {
                /* present state is the same => only level state change => must be present
                 */
                if ((now_val & status_mask) == status_ok)
                {
                    /* AC off => AC on
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                else
                {
                    /* AC on => AC off
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }

                retval = TRUE;
            }
        }
    }

#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    if(power_reg_info.power_alert_mask!=0)
    {
        UI8_T alert_mask=power_reg_info.power_alert_mask;
        UI8_T alert_asserted_val=power_reg_info.power_is_alert_val;

        /* check alert state only when the power is presented
         */
        if ((now_val & present_mask) == present_ok)
        {
            if ((pre_val & alert_mask) != (now_val & alert_mask))
            {
                if ((now_val & alert_mask)==alert_asserted_val)
                {
                    /* alert is not asserted => alert is asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }
                else
                {
                    /* alert is asserted => alert is not asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                retval = TRUE;
            }
        }

    }
#else /* #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */
#if (SYS_CPNT_SYSDRV_POWER_SM_HAS_ALERT_STATE==TRUE)
    {
        UI8_T alert_mask;
        UI8_T alert_asserted_val;

        switch(pwr_id)
        {
            case 1:
                alert_mask = SYS_HWCFG_PWR_ALERT_MASK;
                alert_asserted_val  = SYS_HWCFG_PWR_ALERT_ASSERTED;
                break;

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2)
            case 2:
                alert_mask = SYS_HWCFG_RPS_ALERT_MASK;
                alert_asserted_val  = SYS_HWCFG_RPS_ALERT_ASSERTED;
                break;
#endif

            default:
                return FALSE;
        }

        /* check alert state only when the power is presented
         */
        if ((now_val & present_mask) == present_ok)
        {
            if ((pre_val & alert_mask) != (now_val & alert_mask))
            {
                if ((now_val & alert_mask)==alert_asserted_val)
                {
                    /* alert is not asserted => alert is asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_red;
                }
                else
                {
                    /* alert is asserted => alert is not asserted
                     */
                    *new_status = VAL_swIndivPowerStatus_green;
                }
                retval = TRUE;
            }
        }
    }
#endif /* #if (SYS_CPNT_SYSDRV_POWER_SM_HAS_ALERT_STATE==TRUE) */
#endif /* #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */
    return (retval);

/* SYS_HWCF_RPS_ADVANCED_CONTROL shall be obsoleted by SYS_CPNT_SYSDRV_POWER_SM_TYPE_RPS_ADVANCED_CONTROL */
#elif (SYS_CPNT_SYSDRV_POWER_SM_TYPE == SYS_CPNT_SYSDRV_POWER_SM_TYPE_RPS_ADVANCED_CONTROL) || (defined(SYS_HWCF_RPS_ADVANCED_CONTROL)) /* #if (!defined(SYS_CPNT_SYSDRV_POWER_SM_TYPE)) || (SYS_CPNT_SYSDRV_POWER_SM_TYPE = SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1) */
/* Suppose H/W EPLD register generic design as:
 ------------------------------------------------------------------------------------------
 |     7     |     6    |    5     |     4    |    3     |     2    |    1     |     0    |
 |--------------------------------------------|----------|----------|----------|----------|
 |             Reserved | STATUS2  | STATUS1  | RPS_PRES | PWR_PRES |Reserved  |PWR_STATUS|
 ------------------------------------------------------------------------------------------
STATUS2: STATUS1: PWR_STATUS
     0       0         0       -- Reserved
     0       0         1       -- Reserved, RPU connected, RPU NOT delivering power
     0       1         0       -- RPU normal, RPU connected, RPU delivering power
     0       1         1       -- RPU fail
     1       0         x       -- RPS Fan fail, RPS Thremal normal
     1       1         x       -- Thermal Fail

 RPS_PRES:
     0: RPS is NOT present
     1: RPS is present
 PWR_PRES: <Note: According to h/w, this bits always 1>
     0: RPS is NOT present
     1: RPS is present
 PWR_STATUS:
    0: Internal power source failed
    1: Internal power source is working
 */
    BOOL_T retval;

    retval = TRUE;
    switch(pwr_id)
    {
        case VAL_swIndivPowerIndex_internalPower:
        /* Step 1: Check if the status have changed.
         *         For internal power generic disign:
         *         we care bit 0(PWR_STATUS) and  bit 2(PWR_PRES)0
         *         For internal power based on ES4626H:
         *         we care bit 0(VCC12_PG)
         *         appendix by Ithink_Chen, 2005_10_04.
         */
    #if (SYS_HWCFG_RPS600W_NEW_DEFINITION == TRUE)
        if ((pre_val & SYS_HWCFG_PWR_STATUS_MASK ) == (now_val & SYS_HWCFG_PWR_STATUS_MASK))
    #else
        if ((pre_val & (SYS_HWCFG_PWR_STATUS_MASK | SYS_HWCFG_PWR_PRES_MASK)) ==
            (now_val & (SYS_HWCFG_PWR_STATUS_MASK | SYS_HWCFG_PWR_PRES_MASK)))
    #endif
        {
            return FALSE;
        }
#if (SYS_CPNT_SYSDRV_POWER_CHECK_MAIN_POWER_PRESENT_BEFORE_CHECK_FAIL!=TRUE)
        /* Step 2: Check if the internal power is presented or not, ie, check bit 0 is 1 or not.
         * <Note: It is because PWR_PRES(bit2) is always 1, we only take care PWR_STATUS(bit 0).
         *
         */
         if ((now_val & SYS_HWCFG_PWR_STATUS_MASK) == SYS_HWCFG_PWR_STATUS_OK)
         {
            *new_status = VAL_swIndivPowerStatus_green;
         }
         else
         {
            *new_status = VAL_swIndivPowerStatus_red;   /*changed by charles.chen */
         }
#else /* #if (SYS_CPNT_SYSDRV_POWER_CHECK_MAIN_POWER_PRESENT_BEFORE_CHECK_FAIL!=TRUE) */
          /* Step 2: Check present state */
         if ((now_val & SYS_HWCFG_PWR_PRES_MASK) == SYS_HWCFG_PWR_PRES_OK)
         {
             #ifdef SYS_HWCFG_PWR_PRES_OK
             if( (now_val & SYS_HWCFG_PWR_STATUS_MASK) != SYS_HWCFG_PWR_STATUS_OK)
             #elif defined(SYS_HWCFG_PWR_FAIL)
             if( (now_val & SYS_HWCFG_PWR_STATUS_MASK) == SYS_HWCFG_PWR_FAIL)
             #else
             #error "Neither SYS_HWCFG_PWR_PRES_OK nor SYS_HWCFG_PWR_FAIL is defined"
             #endif
             {
                 *new_status = VAL_swIndivPowerStatus_red;
             }
             else
             {
                 *new_status = VAL_swIndivPowerStatus_green;
             }
        }
        else
        {
             *new_status = VAL_swIndivPowerStatus_notPresent;
        }
#endif /* end of #if (SYS_CPNT_SYSDRV_POWER_CHECK_MAIN_POWER_PRESENT_BEFORE_CHECK_FAIL!=TRUE) */
        break;

#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2)
         case VAL_swIndivPowerIndex_externalPower:
        /* Step 1: Check if the status have changed.
         *         For redundant power:
         *         we care bit 3(RPS_PRES), bit 4(STATUS1) and bit 5(STATUS2)
         */
        if ((pre_val & (SYS_HWCFG_RPS_PRES_MASK | SYS_HWCFG_POWER_DETECT_STATUS1_MASK | SYS_HWCFG_POWER_DETECT_STATUS2_MASK)) ==
             (now_val & (SYS_HWCFG_RPS_PRES_MASK | SYS_HWCFG_POWER_DETECT_STATUS1_MASK | SYS_HWCFG_POWER_DETECT_STATUS2_MASK)))
        {
            return FALSE;
        }

        /* Step 2: Check present state */
        if ((now_val & SYS_HWCFG_RPS_PRES_MASK) == SYS_HWCFG_RPS_PRES_OK)
        {
#ifndef ASF4526B_FLF_P5
            /* Check Fault state
             * Two state implies RPS Fault
             * 1. STATUS2 (bit 5)  equal to 1
             * 2. STATUS1 (bit 4): PWR_STATUS(bit 0)  equal to  1:1
             */
#if 1
            if ((now_val & (SYS_HWCFG_POWER_DETECT_STATUS1_MASK | SYS_HWCFG_POWER_DETECT_STATUS2_MASK)) != SYS_HWCFG_RPS_POWERING)
#else
            if (((now_val & SYS_HWCFG_POWER_DETECT_STATUS2_MASK) == SYS_HWCFG_RPS_FAN_OR_THERMAL_FAIL ) ||
                ((now_val & (SYS_HWCFG_POWER_DETECT_STATUS1_MASK | SYS_HWCFG_PWR_STATUS_MASK)) == SYS_HWCFG_RPS_FAIL))
#endif
#else
            if( (now_val & SYS_HWCFG_POWER_DETECT_STATUS1_MASK) == SYS_HWCFG_RPS_FAIL)
#endif
            {
                *new_status = VAL_swIndivPowerStatus_red;
            }
            else
            {
                *new_status = VAL_swIndivPowerStatus_green;
            }
        }

         /* This implied redundant power not present*/
         else
         {
            *new_status = VAL_swIndivPowerStatus_notPresent;
         }

        break;
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2) */

        default:
             return FALSE;
    }

    return (retval);
#else
#error "Incorrect definition of SYS_CPNT_SYSDRV_POWER_SM_TYPE"
#endif /* end of #if (!defined(SYS_CPNT_SYSDRV_POWER_SM_TYPE)) || (SYS_CPNT_SYSDRV_POWER_SM_TYPE = SYS_CPNT_SYSDRV_POWER_SM_TYPE_GENERIC_TYPE1) */
#endif /* end of #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */

}

#endif

static void SYSDRV_TASK_ClearNMI(void)
{
#if (SYS_CPNT_WATCHDOG_TIMER == TRUE)

    UI8_T NMI_status;
    UI8_T current;
#if (SYS_HWCFG_WATCHDOG_TYPE == SYS_HWCFG_WATCHDOG_ECS4910_28F)
   if(FALSE==PHYSICAL_ADDR_ACCESS_Read(SYS_HWCFG_NMI_CONTROL_ADDRESS, 1, 1, &NMI_status))
    {
        return ;
    }
#endif
#if 0 /* michael.wang 2008-8-19 ,use new EPLD interface to clear NMI */
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_WATCH_DOG_CONTROL_ADDRESS, &sys_hwcfg_watch_dog_control_address))
    {
        printf("\r\n%s:Failed to access SYS_HWCFG_WATCH_DOG_CONTROL_ADDRESS", __FUNCTION__);
        return;
    }

    PHYADDR_ACCESS_Read(sys_hwcfg_watch_dog_control_address, 1, 1, &NMI_status);

    //NMI_status = ((volatile UI8_T *) SYS_HWCFG_WATCH_DOG_CONTROL_ADDRESS);
#endif
    current = NMI_status;

#endif
    return;
}

