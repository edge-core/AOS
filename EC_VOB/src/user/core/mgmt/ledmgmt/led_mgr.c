/* Module Name: LED_MGR.H
 * Purpose: This moudle is responsible for the control of LED activity in
 *          response to system changes such as port link up or down.
 * Notes:
 * History:
 *      23/Sep/2002     -- add enter transition mode MACRO.
 *      11/Sep/2001     -- master mode will spawn a task to handle status
 *                         change
 *      21/June/2001    -- reconciled with other modules to confirm the
 *                         interface
 *      12/June/2001    -- speed and duplex functions combined for extra
 *                         readability; 'unit' parameter added to accomondate
 *                         master/slave control machanism
 *      07/June/2001    -- First Draft created by Jimmy Pai
 *      26/Apr/2004     -- Modify function by Ted Lin for fault LED setting(fan fail or Thermal)
 *
 * Copyright(C)      Accton Corporation, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdio.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sysfun.h"
#include "leddrv.h"
#include "swctrl.h"
#include "leaf_es3626a.h"
#include "led_mgr.h"
#include "stktplg_pmgr.h"
#include "stktplg_pom.h"
#include "sys_pmgr.h"

#include "eh_type.h"
#include "eh_mgr.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "phyaddr_access.h"

#if (SYS_CPNT_POE == TRUE) || (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
#include "stktplg_board.h"
#endif

#if (SYS_CPNT_POE == TRUE)
#include "poedrv_type.h"
#include "poedrv.h"
#include "uc_mgr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define LED_MGR_FUNCTION_NUMBER         0xffffffff
#define LED_MGR_TIMER_EVENT             0x00000001   /* timer event */
#define LED_MGR_TASK_0_5_SEC_TICKS      500

#define LED_MGR_BYTE_IN_BITMAP(INDEX)   ((int)(((INDEX)-1)/8))
#define LED_MGR_BIT_IN_BITMAP(INDEX)    (1 << (7 - (((INDEX)-1) - LED_MGR_BYTE_IN_BITMAP((INDEX))*8)))


/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T                  number_of_ports;
    LEDDRV_System_Status_T  sys_status;
    LEDDRV_Port_Status_T    port_status[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
} LED_MGR_Unit_T;

/* stack information */
typedef struct
{
    LED_MGR_Unit_T  unit_info[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
} LED_MGR_Stack_T;

/* MACRO FUNCTION DECLARATIONS
 */
#define LED_MGR_GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(led_mgr_gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define LED_MGR_GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(led_mgr_gpio_sem_id)

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if (SYS_CPNT_POE == TRUE) || (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE)
static void LED_MGR_Task(void);
#endif

BOOL_T LED_MGR_InitStackStatusBuffer(void);
void LED_MGR_RegisterSWCtrlCallback(void);

#if (SYS_CPNT_POE == TRUE)
void LED_MGR_SetPOELed_callback(UI32_T unit, UI32_T port, UI32_T value);
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
static void LED_MGR_PowerStatusChanged(UI32_T unit, UI32_T power, UI32_T status);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
static void LED_MGR_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status);
#endif

#if (SYS_CPNT_ALARM_DETECT==TRUE) && (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV==TRUE)
static void LED_MGR_SetMajorAlarmOutputLed(UI32_T unit, UI32_T status);
static void LED_MGR_SetMinorAlarmOutputLed(UI32_T unit, UI32_T status);
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
static void LED_MGR_ThermalStatusChanged(UI32_T unit, UI32_T thermal, BOOL_T is_abnormal);
#endif

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
void LED_MGR_LoopbackTestFail(void);
#endif
void LED_MGR_XFPModuleStatusChanged(UI32_T unit, UI32_T port, BOOL_T status);

#if (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE)
static void LED_MGR_CheckCableInsert(void);
#endif

/* STATIC VARIABLE DEFINITIONS
 */
/* stack information */
static LED_MGR_Stack_T led_mgr_status;

/* ledmgmt task_id */
/*static UI32_T led_mgr_task_id;*/ /*unused*/

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
static UI8_T LED_MGR_failed_pbmp[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
#endif

#if (SYS_CPNT_POE == TRUE)
static UI8_T led_poe_mode = LEDDRV_LED_MODE_NORMAL;
#endif

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC;

static UI32_T led_mgr_gpio_sem_id;


/* EXPORTED SUBPROGRAM BODIES
 */
void LED_MGR_Init(void)
{
    UI32_T ret;

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &led_mgr_gpio_sem_id);
    if (ret != SYSFUN_OK)
    {
        printf("%s:%d: SYSFUN_GetSem fails. (%lu)\n", __FUNCTION__, __LINE__, (unsigned long)ret);
        return;
    }

    return;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LED_MGR_Create_InterCSC_Relation(void)
{

#if 0 /*comment by gordon temporarily*/
#if (SYS_CPNT_POE == TRUE)
    SYS_MGR_Register_PowerStatusChanged_CallBack(LED_MGR_PowerStatusChanged);
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    SYS_MGR_Register_FanStatusChanged_CallBack( LED_MGR_FanStatusChanged );
#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    SYS_MGR_Register_ThermalStatusChanged_CallBack(LED_MGR_ThermalStatusChanged);
#endif

    SYS_MGR_Register_XFPModuleStatusChanged_Callback(LED_MGR_XFPModuleStatusChanged);

#if (SYS_CPNT_POE == TRUE)
    POEDRV_Register_PortStatusChange_CallBack(LED_MGR_SetPOELed_callback);
#endif

#endif
} /* end of LED_MGR_Create_InterCSC_Relation */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LED_MGR_Display(void)
{
    LEDDRV_Display();
}
#if (SYS_CPNT_POWER_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_MGR_PowerStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Registered callback function, when power status is changed
 *            the registered function will be called.
 * INPUT    : unit          - unit number
 *            power         - the power number
 *            status        - status of the power
 *                      VAL_swIndivPowerStatus_notPresent
 *                      VAL_swIndivPowerStatus_green
 *                      VAL_swIndivPowerStatus_red
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static void LED_MGR_PowerStatusChanged(UI32_T unit, UI32_T power, UI32_T status)
{
    if ( power == 0 || power > SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT )
    {
        return;
    }

    LEDDRV_SetPowerStatus(unit, power, status);

    return;
}
#endif

#if (SYS_CPNT_POE == TRUE)
static void LED_MGR_Task(void)
{
#if 0
    /* To support changing poe led of remote unit(stacking):
     * LEDDRV_LightFrontPortLed() must be called by LEDDRV_SetPortStatus()
     * As for query poe_button repeatedly, LED_MGR_Display() can be
     * used. Directly return here for nothing need to be done.
     */

    void *leddrv_timer_id = (void *) 0;
    UI32_T pending_events = 0;
    UI32_T unit = 1, i = 0;
    UI8_T led_poe_mode = LEDDRV_LED_MODE_NORMAL;
    static BOOL_T poe_act[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT] = {{LEDDRV_POE_PORT_INACTIVE}};
    STKTPLG_BOARD_BoardInfo_T board_info;
    UC_MGR_Sys_Info_T uc_sys_info;


    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
       return;
    }

    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");

        /* severe problem, while loop here
         */
        while (TRUE);
    }
    STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, &board_info);

    if (board_info.is_poe_device == TRUE)
    {
        leddrv_timer_id = SYSFUN_PeriodicTimer_Create();
        led_poe_mode = LEDDRV_LED_MODE_POE;
        SYSFUN_PeriodicTimer_Start(leddrv_timer_id, SYS_BLD_IDLE_TASK_SLEEP_PERIOD, LED_MGR_TIMER_EVENT);
        while (TRUE)
        {
            SYSFUN_ReceiveEvent(LED_MGR_TIMER_EVENT, SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER, &pending_events);
            STKTPLG_OM_GetMyUnitID(&unit);
            for (i = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER; i <= board_info.max_pse_port_number; i++)
            {
                if (poe_act[unit - 1][i - 1] != led_mgr_status.unit_info[unit - 1].port_status[i - 1].poe_active)
                {
                    poe_act[unit - 1][i - 1] = led_mgr_status.unit_info[unit - 1].port_status[i - 1].poe_active;
                    LEDDRV_LightFrontPortLed(unit, i, led_poe_mode, poe_act[unit - 1][i - 1]); /* according to current poe status to light port LED */
                }
            }
        }
    } /* end of if (board_info.is_poe_device == TRUE) */

#endif
    return;
}/* end of LED_MGR_Task */

void LED_MGR_SetPOELed_callback(UI32_T unit, UI32_T port, UI32_T value)
{
    if(TRUE==STKTPLG_POM_IsPoeDevice(unit))
    {
        if (SWCTRL_UserPortExisting(unit, port)) //exist ?
        {
            if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
            {
                EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
                return;
            }
            else
            {
                if (unit < 1 || unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
                    return;
                }
                if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
                {
                    EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
                    return;
                }

#if (SYS_CPNT_POE_ASIC == SYS_CPNT_POE_ASIC_POWERDSINE)
                switch(value)
                {
                    case POEDRV_TYPE_PORT_POWERED_CAP_DETECT :
                    case POEDRV_TYPE_PORT_POWERED_RES_DETECT :
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_UP;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_ACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    case POEDRV_TYPE_PORT_ASIC_HARDWARE_FAULT :
                    case POEDRV_TYPE_PORT_HARDWARE_FAULT :
                    case POEDRV_TYPE_PORT_OFF_OVL_UDL :
                    case POEDRV_TYPE_PORT_UNDERLOAD :
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    case POEDRV_TYPE_PORT_OVERLOAD :
                        if (led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link == LEDDRV_POE_PORT_LINK_UP)
                        {
                            led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_OVERLOAD;
                        }
                        else
                        {
                            led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        }
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    case POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED :
                        if (  led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active == LEDDRV_POE_PORT_ACTIVE ||
                                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload == LEDDRV_POE_PORT_OVERLOAD ||
                                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin == LEDDRV_POE_ADMIN_DISABLE ||
                                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link == LEDDRV_POE_PORT_LINK_UP )
                        {
                            led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
                            led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                            led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        }
                        else
                        {
                        }
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    case POEDRV_TYPE_PORT_OFF_POWER_MANAGEMENT :
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_UP;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        //led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    case POEDRV_TYPE_PORT_OFF_USER_SETTING :
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_DISABLE;
                        break;
                    case POEDRV_TYPE_PORT_OFF_VOLT_TOO_HIGH :
                    case POEDRV_TYPE_PORT_OFF_VOLT_TOO_LOW :
                    case POEDRV_TYPE_PORT_OFF_TEMPORARY_SHUT_DOWN :
                    case POEDRV_TYPE_PORT_OFF_PORT_NOT_ACTIVE :
                    case POEDRV_TYPE_PORT_OFF_POWERUP_NOT_COMPLETED:
                    case POEDRV_TYPE_PORT_OFF_NOT_PD :
                    case POEDRV_TYPE_PORT_OFF_VOLT_FEED :
                    case POEDRV_TYPE_PORT_OFF_IMPROPER_CAP_DETECTION:
                    case POEDRV_TYPE_PORT_OFF_DISCHARGE_LOAD :
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link= LEDDRV_POE_PORT_LINK_DOWN;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        break;
                    default :
                        //printf("\n Actual Status Value from POEDRV is out of range(%lu)\n", value);
                        break;
                }
#else /* Broadcom series */
                switch (value)
                {
                    case POEDRV_TYPE_PORT_LINKOFF:
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_DISABLE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link= LEDDRV_POE_PORT_LINK_DOWN;
                        break;
                    case POEDRV_TYPE_PORT_LINKON:
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_ACTIVE;
                        led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_UP;
                        break;
                    default:
                        printf("\n Actual Status Value from POEDRV is out of range(%lu)\n", value);
                        return;
                }
#endif
                LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
            }
            return;
        }/* end of if (SWCTRL_UserPortExisting(unit, port)) */
    }/* end of if(TRUE==STKTPLG_POM_IsPoeDevice(unit))  */
    return;
}

void LED_MGR_CreateTask(void)
{
    UI32_T  led_mgr_thread_id;
    UI32_T  unit;
    /* Create a thread for LED management
     */
    STKTPLG_POM_GetMyUnitID(&unit);
/*
    if(TRUE==STKTPLG_POM_IsPoeDevice(unit))
*/
    {
        if(SYSFUN_SpawnThread(SYS_BLD_LED_MGR_THREAD_PRIORITY,
                    SYS_BLD_LED_MGR_SCHED_POLICY,
                    (char*)SYS_BLD_LED_CSC_THREAD_NAME,
                    SYS_BLD_TASK_COMMON_STACK_SIZE,
                    SYSFUN_TASK_NO_FP,
                    LED_MGR_Task,
                    NULL, &led_mgr_thread_id)!=SYSFUN_OK)
        {
            printf("%s:Spawn LED MGR thread fail.\r\n", __FUNCTION__);
        }
    }
    return;
}
#else /* else of #if (SYS_CPNT_POE == TRUE) */
#if (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE)
static void   LED_MGR_Task(void)
{
    UI32_T leddrv_timer_id;
    UI32_T pending_events;

    if (SYSFUN_StartPeriodicTimerEvent(SYS_BLD_IDLE_TASK_SLEEP_PERIOD, LED_MGR_TIMER_EVENT, 0, &leddrv_timer_id)
        != SYSFUN_OK)
    {
        printf("LEDMGR Start Timer Failed");
        while(TRUE);
    }

    while(TRUE)
    {
        SYSFUN_ReceiveEvent(LED_MGR_TIMER_EVENT, SYSFUN_EVENT_WAIT_ANY,
                            SYSFUN_TIMEOUT_WAIT_FOREVER, &pending_events);

        LED_MGR_CheckCableInsert();
    }
}
#endif /* end of #if (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE) */

void LED_MGR_CreateTask(void)
{
#if (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE)
    UI32_T  led_mgr_task_id;
    /* Create a task for LED management
     */
    SYSFUN_SpawnTask (SYS_BLD_LED_MGR_TASK,
                      SYS_BLD_LED_MGR_TASK_PRIORITY,
                      SYS_BLD_TASK_COMMON_STACK_SIZE,
                      SYSFUN_TASK_NO_FP,
                      LED_MGR_Task,
                      (void *)(NULL),
                      &led_mgr_task_id);

#endif
    return;
}
#endif /* end of #if (SYS_CPNT_POE == TRUE) */


#if (SYS_CPNT_3COM_DISABLE_LED_SPEC == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_CheckCableInsert
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will check out the cable is insert or not when
 *            the port admin is disabled. and give different LED pattern to leddrv.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Aaron add here for 3COM specific request.
 * ------------------------------------------------------------------------
 */
static void LED_MGR_CheckCableInsert(void)
{
    UI32_T  unit, port;
    UI32_T  copper_energy_detect;

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }

    /* The is 3COM specific LED specification
     * If the port_admin is disabled and cable is insert --> Green/Amber interchange
     * If the port_admin is disabled and cable is remove --> OFF
     */
    for (unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        for(port=1; port<=SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            if (led_mgr_status.unit_info[unit-1].port_status[port-1].admin == LEDDRV_ADMIN_DISABLE &&
                SWCTRL_UIUserPortExisting(unit, port) == TRUE)
            {
                LEDDRV_Port_Status_T    port_status;

                copper_energy_detect = FALSE;
                SWDRV_GetCopperEnergyDetect(unit, port, &copper_energy_detect);
                memcpy (&port_status, &led_mgr_status.unit_info[unit-1].port_status[port-1], sizeof(LEDDRV_Port_Status_T));
                if (copper_energy_detect == TRUE)
                    port_status.link = LEDDRV_LINK_UP;
                else
                    port_status.link = LEDDRV_LINK_DOWN;
                LEDDRV_SetPortStatus(unit, port, FALSE, &port_status);
            }
        }
    }
    return;
}
#endif

BOOL_T LED_MGR_EnterMasterMode(void)
{
    UI32_T  unit=0, port, port_type;
#if (SYS_CPNT_POE == TRUE)
    UI32_T board_id = 0;
    UI8_T  max_pse_port = SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;
    BOOL_T is_poe_device = FALSE;
    STKTPLG_BOARD_BoardInfo_T board_info;
#endif

    /* allocate space for stack status buffer and set to default
     */
    if (LED_MGR_InitStackStatusBuffer() == FALSE)
    {
        return FALSE;
    }

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
    LED_MGR_LoopbackTestFail();
#endif

    while (STKTPLG_POM_GetNextUnit(&unit))
    {
#if (SYS_CPNT_POE == TRUE)
        if(TRUE==STKTPLG_POM_IsPoeDevice(unit))
        {
            is_poe_device = TRUE;
            STKTPLG_OM_GetUnitBoardID(unit, &board_id);
            memset(&board_info, 0, sizeof(STKTPLG_BOARD_BoardInfo_T));
            STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
            max_pse_port = board_info.max_pse_port_number;
        }
        else
        {
            is_poe_device = FALSE;
        }
#endif
        for (port=1; port<=led_mgr_status.unit_info[unit-1].number_of_ports; port++)
        {
            led_mgr_status.unit_info[unit-1].port_status[port-1].link = LEDDRV_LINK_DOWN;
            led_mgr_status.unit_info[unit-1].port_status[port-1].admin = LEDDRV_ADMIN_ENABLE;
            led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_1000M;
            led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_FULL;

#if (SYS_CPNT_POE == TRUE)
            if(TRUE==is_poe_device)
            {
                if(port >= SYS_ADPT_POE_PSE_MIN_PORT_NUMBER && port <= max_pse_port)
                {
                    led_mgr_status.unit_info[unit-1].port_status[port-1].led_mode = LEDDRV_LED_MODE_POE;
                }
                else
                {
                    led_mgr_status.unit_info[unit-1].port_status[port-1].led_mode = LEDDRV_LED_MODE_NORMAL;
                }
                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
                led_mgr_status.unit_info[unit-1].port_status[port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
            }
#endif
            /*add by wx to support 100FX SFP*/
            STKTPLG_POM_GetPortType(unit, port, &port_type);
            if (port_type == VAL_portType_hundredBaseFX || port_type == VAL_portType_hundredBaseTX)
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_100M;
            /* 2004/11/2
             * Setting LED of all exist port
             * to link down is not necessary as default value is dimmed
             * and it cause timing issue for module's LED
             */
            /*
               if (STKTPLG_MGR_PortExist(unit, port+1) == TRUE)
               LEDDRV_SetPortStatus(unit, port+1, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
             */
        }
    }

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();

    return TRUE;
}

BOOL_T LED_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();

    return TRUE;
}

void LED_MGR_Set_TransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();
}

BOOL_T LED_MGR_EnterTransitionMode(void)
{
    /* UI32_T  unit, port, port_type; */

    SYSFUN_ENTER_TRANSITION_MODE();

    /* 2004/11/2
     * STKTPLG_MGR_GetNextUnit(&unit) could cause return incorrect value
     * during transition mode
     * LED_DRV disables all LED during LEDDRV_EnterTransitionMode
     * making this part extra effort
     */
#if 0
    #if 1 /* michael */

        unit = 0;

        while (STKTPLG_MGR_GetNextUnit(&unit))
        {
            for (port=0; port<led_mgr_status.unit_info[unit-1].number_of_ports; port++)
            {
                led_mgr_status.unit_info[unit-1].port_status[port].link = LEDDRV_LINK_DOWN;
                led_mgr_status.unit_info[unit-1].port_status[port].admin = LEDDRV_ADMIN_ENABLE;
                led_mgr_status.unit_info[unit-1].port_status[port].speed  = LEDDRV_SPEED_1000M;
                led_mgr_status.unit_info[unit-1].port_status[port].duplex = LEDDRV_DUPLEX_HALF;
#if (SYS_CPNT_POE == TRUE)
                led_mgr_status.unit_info[unit-1].port_status[port].led_mode= LEDDRV_LED_MODE_NORMAL;
                led_mgr_status.unit_info[unit-1].port_status[port].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
                led_mgr_status.unit_info[unit-1].port_status[port].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
                led_mgr_status.unit_info[unit-1].port_status[port].poe_active =   LEDDRV_POE_PORT_INACTIVE;
                led_mgr_status.unit_info[unit-1].port_status[port].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
#endif
                /*add by wx to support 100FX SFP*/
                STKTPLG_OM_GetPortType(unit, port+1, &port_type);
                if (port_type == VAL_portType_hundredBaseFX || port_type == VAL_portType_hundredBaseTX)
                    led_mgr_status.unit_info[unit-1].port_status[port].speed  = LEDDRV_SPEED_100M;

                if (STKTPLG_MGR_PortExist(unit, port+1) == TRUE)
                {
                    LEDDRV_SetPortStatus(unit, port+1, &led_mgr_status.unit_info[unit-1].port_status[port]);
                }
            }
        }

    #else
        /* All light off in transitione mode */
        for (unit=0; unit<led_mgr_status.number_of_units; unit++)
        {
            for (port=0; port<led_mgr_status.unit_info[unit].number_of_ports; port++)
            {
                led_mgr_status.unit_info[unit].port_status[port].link = LEDDRV_LINK_DOWN;
                led_mgr_status.unit_info[unit].port_status[port].admin = LEDDRV_ADMIN_ENABLE;
                led_mgr_status.unit_info[unit].port_status[port].speed  = LEDDRV_SPEED_1000M;
                led_mgr_status.unit_info[unit].port_status[port].duplex = LEDDRV_DUPLEX_HALF;

                if (STKTPLG_MGR_PortExist(unit, port+1) == TRUE)
                {
                    LEDDRV_SetPortStatus(unit+1, port+1, &led_mgr_status.unit_info[unit].port_status[port]);
                }
            }
        }
    #endif
#endif
    return TRUE;
}

#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
/* FUNCTION NAME: LED_MGR_LoopbackTestFail
 * PURPOSE: check loopback test failure and call to LEDDRV to set Pattern
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE -
 *          FALSE -
 * NOTES:
 */
void LED_MGR_LoopbackTestFail(void)
{
    UI32_T      unit=0, port;
    UI32_T      ifindex;

    if (FALSE == SWCTRL_GetSwitchLoopbackTestFailurePorts(LED_MGR_failed_pbmp))
    {
        return;
    }

    while (STKTPLG_POM_GetNextUnit(&unit))
    {
        for (port=1; port<=led_mgr_status.unit_info[unit-1].number_of_ports; port++)
        {
            if (SWCTRL_LPORT_UNKNOWN_PORT != SWCTRL_UserPortToIfindex(unit, port, &ifindex))
            {
                if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
                {
                    LEDDRV_SetLoopbackFailStatus(unit, port, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
                }
            }
        }
    }
}
#endif

/* FUNCTION NAME: LED_MGR_Provision_Complete
 * PURPOSE: Lights up diag LED after provision has completed
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Provision_Complete(void)
{
    UI32_T unit;

    for(unit = 0; STKTPLG_POM_GetNextUnit(&unit); )
    {
        LEDDRV_ProvisionComplete(unit, led_mgr_status.unit_info[unit-1].sys_status);
    }
    return;
}

/* FUNCTION NAME: LED_MGR_InitStackStatusBuffer
 * PURPOSE: Initiate local static variable 'status' to store the stack info
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - initialization successful
 *          FALSE - initialization failed:most likely memory allocation problem
 * NOTES:
 *
 */
BOOL_T LED_MGR_InitStackStatusBuffer(void)
{
    UI32_T unit = 0;

    while (STKTPLG_POM_GetNextUnit(&unit))
    {
        led_mgr_status.unit_info[unit-1].number_of_ports = SWCTRL_GetUnitPortNumber(unit);
    }

    return TRUE;
}


/* FUNCTION NAME: LED_MGR_RegisterSWCtrlCallback
 * PURPOSE: register call back function at SWCTRL for:
 *          port link up/down, enable/disable, speed&duplex change
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - regitered successfully
 *          FALSE - registry failed
 * NOTES:
 *
 */
void LED_MGR_RegisterSWCtrlCallback(void)
{
#if 0
    SWCTRL_Register_UPortLinkUp_CallBack( LED_MGR_Linkup );
    SWCTRL_Register_UPortLinkDown_CallBack( LED_MGR_Linkdown );

    SWCTRL_Register_UPortAdminEnable_CallBack( LED_MGR_AdminEnable );
    SWCTRL_Register_UPortAdminDisable_CallBack( LED_MGR_AdminDisable );

    SWCTRL_Register_UPortSpeedDuplex_CallBack( LED_MGR_SpeedDuplexChange );

    SWCTRL_Register_UPortTypeChanged_CallBack( LED_MGR_PortTypeChanged );
#endif
}


/* FUNCTION NAME: LED_MGR_AdminEnable
 * PURPOSE: call back function for port enable
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *          Message is sent to LED_MGR task for processing
 */
void LED_MGR_AdminEnable (UI32_T unit, UI32_T port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }
    else
    {
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        UI32_T  ifindex;
        /* Check whether the port failed in loopback test */
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        {
            return;
        }

        /* If the port failed in the loopback test, return without change the LED status */
        if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
        {
            return;
        }
#endif
        if (STKTPLG_POM_UnitExist(unit) == FALSE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
            return;
        }
        if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
            return;
        }

        led_mgr_status.unit_info[unit-1].port_status[port-1].admin = LEDDRV_ADMIN_ENABLE;
        LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
    }
    return;
}


/* FUNCTION NAME: LED_MGR_AdminDisable
 * PURPOSE: call back function for port disable
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - regitered successfully
 *          FALSE - registry failed
 * NOTES:
 *          Message is sent to LED_MGR task for processing
 */
void LED_MGR_AdminDisable (UI32_T unit, UI32_T port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }
    else
    {
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        UI32_T  ifindex;
        /* Check whether the port failed in loopback test */
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        {
            return;
        }

        /* If the port failed in the loopback test, return without change the LED status */
        if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
        {
            return;
        }
#endif
        if (STKTPLG_POM_UnitExist(unit) == FALSE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
            return;
        }

        if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
            return;
        }

        led_mgr_status.unit_info[unit-1].port_status[port-1].admin = LEDDRV_ADMIN_DISABLE;
        LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
    }
    return;
}


/* FUNCTION NAME: LED_MGR_Linkup
 * PURPOSE: call back function for port link up
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *          Message is sent to LED_MGR task for processing
 */
void LED_MGR_Linkup (UI32_T unit, UI32_T port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }
    else
    {
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        UI32_T  ifindex;
        /* Check whether the port failed in loopback test */
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        {
            return;
        }

        /* If the port failed in the loopback test, return without change the LED status */
        if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
        {
            return;
        }
#endif
        if (STKTPLG_POM_UnitExist(unit) == FALSE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
            return;
        }

        if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
            return;
        }

        led_mgr_status.unit_info[unit-1].port_status[port-1].link = LEDDRV_LINK_UP;
        LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
    }
    return;
}


/* FUNCTION NAME: LED_MGR_Linkdown
 * PURPOSE: call back function for port link down
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *          Message is sent to LED_MGR task for processing
 */
void LED_MGR_Linkdown (UI32_T unit, UI32_T port)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }
    else
    {
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        UI32_T  ifindex;
        /* Check whether the port failed in loopback test */
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        {
            return;
        }

        /* If the port failed in the loopback test, return without change the LED status */
        if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
        {
            return;
        }
#endif
        if (STKTPLG_POM_UnitExist(unit) == FALSE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
            return;
        }

        if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
            return;
        }

        led_mgr_status.unit_info[unit-1].port_status[port-1].link = LEDDRV_LINK_DOWN;
        LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
    }
    return;
}


/* FUNCTION NAME: LED_MGR_SpeedDuplexChange
 * PURPOSE: call back function  for speed&duplex change
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *          Message is sent to LED_MGR task for processing
 */
void LED_MGR_SpeedDuplexChange(UI32_T unit, UI32_T port, UI32_T speed_duplex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_LEDMGMT, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_NOT_IN_MASTER_MODE, EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR);
        return;
    }
    else
    {
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        UI32_T  ifindex;
        /* Check whether the port failed in loopback test */
        if (SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        {
            return;
        }

        /* If the port failed in the loopback test, return without change the LED status */
        if (LED_MGR_failed_pbmp[LED_MGR_BYTE_IN_BITMAP(ifindex)] & LED_MGR_BIT_IN_BITMAP(ifindex))
        {
            return;
        }
#endif
        if (STKTPLG_POM_UnitExist(unit) == FALSE)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Unit number");
            return;
        }

        if (port < 1 || port > led_mgr_status.unit_info[unit-1].number_of_ports)
        {
            EH_MGR_Handle_Exception1(SYS_MODULE_LED, LED_MGR_FUNCTION_NUMBER, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "Port number");
            return;
        }

        switch(speed_duplex)
        {
            case VAL_portSpeedDpxStatus_halfDuplex10:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_10M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_HALF;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex10:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_10M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_FULL;
                break;
            case VAL_portSpeedDpxStatus_halfDuplex100:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_100M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_HALF;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex100:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_100M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_FULL;
                break;
            case VAL_portSpeedDpxStatus_halfDuplex1000:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_1000M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_HALF;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex1000:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_1000M;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_FULL;
                break;
            case VAL_portSpeedDpxStatus_halfDuplex10g:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_10G;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_HALF;
                break;
            case VAL_portSpeedDpxStatus_fullDuplex10g:
                led_mgr_status.unit_info[unit-1].port_status[port-1].speed  = LEDDRV_SPEED_10G;
                led_mgr_status.unit_info[unit-1].port_status[port-1].duplex = LEDDRV_DUPLEX_FULL;
                break;
            default:
                break;
        };
        LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);
    }
    return;
}

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: LED_MGR_FanStatusChanged
 * PURPOSE: Light the LED according to the given fan status
 * INPUT  : unit   -- in which unit
 *          fan    -- which fan id(Starts from 1)
 *          status -- fan status(VAL_switchFanStatus_ok/VAL_switchFanStatus_failure)
 * OUTPUT : none
 * RETUEN : none
 * NOTES  : none
 */
static void LED_MGR_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status)
{
    BOOL_T is_fault;
    UI8_T nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;

    nbr_of_fan = STKTPLG_BOARD_GetFanNumber();

    if (fan == 0 || fan > nbr_of_fan)
    {
        return;
    }

    if (status != VAL_switchFanStatus_ok)
    {
        is_fault=TRUE;  /* fan not O.K */
    }
    else
    {
        is_fault=FALSE; /* fan O.K */
    }

    LEDDRV_SetFaultStatus(unit, LEDDRV_SYSTEM_FAULT_TYPE_FAN, fan, is_fault);

    return;
}
#endif

#if (SYS_CPNT_ALARM_DETECT==TRUE) && (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV==TRUE)
/* FUNCTION NAME: LED_MGR_SetMajorAlarmOutputLed
 * FUNCTION: This function will set the Major Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMAJORSTATUS_XXX_MASK
 * OUTPUT  : none
 * RETUEN  : none
 * NOTES   : none
 */
static void LED_MGR_SetMajorAlarmOutputLed(UI32_T unit, UI32_T status)
{
    LEDDRV_SetMajorAlarmOutputLed(unit, status);
}

/* FUNCTION NAME: LED_MGR_SetMinorAlarmOutputLed
 * FUNCTION: This function will set the Minor Alarm output led
 * INPUT   : unit   -- in which unit
 *           status -- SYSDRV_ALARMMINORSTATUS_XXX_MASK
 * OUTPUT  : none
 * RETUEN  : none
 * NOTES   : none
 */
static void LED_MGR_SetMinorAlarmOutputLed(UI32_T unit, UI32_T status)
{
    LEDDRV_SetMinorAlarmOutputLed(unit, status);
}

#endif

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_MGR_ThermalStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Light the LED according to the given thermal sensor status
 * INPUT    : unit          - unit number
 *            thermal       - thermal sensor number (Starts from 1)
 *            is_abnormal   - TRUE  - the temperature of the given thermal sensor falls in the
 *                                    abnormal region (overheating or undercooling)
 *                            FALSE - the temperature of the given thermal sensor falls in the
 *                                    normal region
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
static void LED_MGR_ThermalStatusChanged(UI32_T unit, UI32_T thermal, BOOL_T is_abnormal)
{
    LEDDRV_SetFaultStatus(unit, LEDDRV_SYSTEM_FAULT_TYPE_THERMAL, thermal, is_abnormal);
    return;
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_MGR_PortTypeChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Registered callback function so that this function will be
 *            invoked when a u_port changes its type. Light SFP in use
 *            LED when according to the given port type.
 * INPUT    : unit          - unit number
 *            port          - port number
 *            port_type     - one of the port type values defined in MIB
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : The activity LED is controled by LEDuP now. The active
 *            field in structure in LEDDRV_Port_Status_T is no longer
 *            used. This bit can be used to determined wether or not the
 *            SFP is in use.
 * ---------------------------------------------------------------------
 */
void LED_MGR_PortTypeChanged(UI32_T unit, UI32_T port, UI32_T port_type)
{
    if (port_type == VAL_portType_thousandBaseSfp)
    {
        led_mgr_status.unit_info[unit-1].port_status[port-1].sfp_in_use = TRUE;
    }
    else
    {
        led_mgr_status.unit_info[unit-1].port_status[port-1].sfp_in_use = FALSE;
    }

/* Support 100Base-FX to set intial speed */
    if (port_type == VAL_portType_hundredBaseFX)
        led_mgr_status.unit_info[unit-1].port_status[port-1].speed = LEDDRV_SPEED_100M;
    if (port_type == VAL_portType_thousandBaseSfp)
        led_mgr_status.unit_info[unit-1].port_status[port-1].speed = LEDDRV_SPEED_1000M;

    LEDDRV_SetPortStatus(unit, port,FALSE, &(led_mgr_status.unit_info[unit-1].port_status[port-1]));
}

/* FUNCTION NAME: LED_MGR_Start_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED blink green
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Start_Xfer(UI32_T unit)
{

    LEDDRV_System_Status_T status;

    memcpy(&status, &(led_mgr_status.unit_info[unit-1].sys_status), sizeof(LEDDRV_System_Status_T));
    status.reserve = LEDDRV_SYSTEM_XFER;
    LEDDRV_SetSystemXferStatus(unit, status);
    return;

}

/* FUNCTION NAME: LED_MGR_Stop_Xfer(UI32_T unit)
 * PURPOSE: Make Diag LED back to normal
 * INPUT:   unit:unit num
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void LED_MGR_Stop_Xfer(UI32_T unit)
{

    LEDDRV_System_Status_T status;

    status.reserve = LEDDRV_SYSTEM_NORMAL;
    LEDDRV_SetSystemXferStatus(unit, status);
    return;

}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void LED_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    UI32_T unit, port, port_type, trunk_id, i;
    BOOL_T is_module;
#if (SYS_CPNT_POE == TRUE)
    UI32_T board_id = 0;
    UI8_T  max_pse_port = SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;
    BOOL_T is_poe_device = FALSE;
    STKTPLG_BOARD_BoardInfo_T board_info;
#endif

    if (SWCTRL_LogicalPortToUserPort(starting_port_ifindex, &unit, &port, &trunk_id) != TRUE)
    {
        printf(" LED_MGR_HandleHotInsertion: SWCTRL_LogicalPortToUserPort return FALSE\n\r");
        return;
    }

    is_module = STKTPLG_POM_IsModulePort(unit, port);

    if(is_module == TRUE)
    {
        /* This is to light the module insert LED
         */
        LEDDRV_SetHotSwapInsertion(unit);
    }
    else
    {
        led_mgr_status.unit_info[unit-1].number_of_ports = number_of_port;
    }

#if (SYS_CPNT_POE == TRUE)
    if(TRUE==STKTPLG_POM_IsPoeDevice(unit))
    {
        is_poe_device = TRUE;
        STKTPLG_OM_GetUnitBoardID(unit, &board_id);
        memset(&board_info, 0, sizeof(STKTPLG_BOARD_BoardInfo_T));
        STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
        max_pse_port = board_info.max_pse_port_number;
    }
#endif

    /* Set Module port default setting for LED
     */
    for (i=0; i<number_of_port; i++)
    {
        if(STKTPLG_POM_GetPortType(unit, i+port, &port_type) == FALSE)
        {
            printf("\r\nLED_MGR:Module get port type error!!");
            return;
        }
        led_mgr_status.unit_info[unit-1].port_status[i+port-1].link = LEDDRV_LINK_DOWN;
        led_mgr_status.unit_info[unit-1].port_status[i+port-1].admin = LEDDRV_ADMIN_ENABLE;

        /* Check if port is 10G
         */
        if(port_type == VAL_portType_tenG ||
           port_type == VAL_portType_tenGBaseT ||
           port_type == VAL_portType_tenGBaseXFP ||
           port_type == VAL_portType_tenGBaseSFP)
        {
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].speed  = LEDDRV_SPEED_10G;
        }
        /* else use default 1000M
         */
        else
        {
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].speed  = LEDDRV_SPEED_1000M;
        }
        led_mgr_status.unit_info[unit-1].port_status[i+port-1].duplex = LEDDRV_DUPLEX_FULL;

#if (SYS_CPNT_POE == TRUE)
        if(TRUE==is_poe_device)
        {
            if((i+port) >= SYS_ADPT_POE_PSE_MIN_PORT_NUMBER && (i+port) <= max_pse_port)
            {
                led_mgr_status.unit_info[unit-1].port_status[i+port-1].led_mode = LEDDRV_LED_MODE_POE;
            }
            else
            {
                led_mgr_status.unit_info[unit-1].port_status[i+port-1].led_mode = LEDDRV_LED_MODE_NORMAL;
            }
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].poe_admin = LEDDRV_POE_ADMIN_ENABLE;
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].poe_link = LEDDRV_POE_PORT_LINK_DOWN;
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].poe_active = LEDDRV_POE_PORT_INACTIVE;
            led_mgr_status.unit_info[unit-1].port_status[i+port-1].poe_overload = LEDDRV_POE_PORT_NOT_OVERLOAD;
        }
#endif
        /* LEDDRV_SetPortStatus(unit, i+port, &led_mgr_status.unit_info[unit-1].port_status[i+port-1]); */
    }

    return;
}/* End of LED_MGR_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void LED_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T unit;
    UI32_T port;
    BOOL_T is_module;

    unit = ((starting_port_ifindex-1)/SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)+1;
    port = (starting_port_ifindex - (unit-1)*SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);

    is_module = STKTPLG_POM_IsModulePort(unit, port);

    if(is_module == TRUE)
    {
        LEDDRV_SetHotSwapRemoval(unit);
    }

    return;
} /* End of LED_MGR_HandleHotRemoval */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetModuleLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the module led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            color   --     LEDDRV_COLOR_OFF              off
 *                           LEDDRV_COLOR_GREEN            green
 *                           LEDDRV_COLOR_AMBER            amber
 *                           LEDDRV_COLOR_GREENFLASH       green flash
 *                           LEDDRV_COLOR_AMBERFLASH       amber flash
 *                           LEDDRV_COLOR_GREENAMBER_FLASH green amber flash
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
void LED_MGR_SetModuleLED(UI32_T unit_id, UI8_T color)
{
    LEDDRV_SetModuleLed(unit_id, color);
    return;
}

#if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetLocationLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the location led status
 * INPUT    : unit_id   -- the unit desired to set
 *            led_is_on -- TRUE : Turn on Location LED
 *                         FALSE: Turn off Location LED
 * OUTPUT   : None
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_MGR_SetLocationLED(UI32_T unit_id, BOOL_T led_is_on)
{
    return LEDDRV_SetLocationLED(unit_id, led_is_on);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_GetLocationLEDStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will get the location led status
 * INPUT    : unit_id     -- the unit desired to set
 * OUTPUT   : led_is_on_p -- TRUE : Location LED is on
 *                           FALSE: Location LED is off
 * RETURN   : LEDDRV_TYPE_RET_OK              - Successfully
 *            LEDDRV_TYPE_RET_ERR_HW_FAIL     - Operation failed due to hardware problem
 *            LEDDRV_TYPE_RET_ERR_NOT_SUPPORT - Operation failed because the device does not support
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
LEDDRV_TYPE_RET_T LED_MGR_GetLocationLEDStatus(UI32_T unit_id, BOOL_T *led_is_on_p)
{
    return LEDDRV_GetLocationLEDStatus(unit_id, led_is_on_p);
}
#endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

#if (SYS_CPNT_POE == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - LED_MGR_SetPoeLED
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will set the PoE led for the speicific unit
 * INPUT    : unit_id -- the unit desired to set
 *            status  -- POEDRV_TYPE_SYSTEM_OVERLOAD, the mainpower overload of PoE system
                         POEDRV_TYPE_SYSTEM_NORMAL, the mainpower normal of PoE system
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None.
 * ------------------------------------------------------------------------
 */
void LED_MGR_SetPoeLED(UI32_T unit_id, UI8_T status)
{

#if (SYS_CPNT_LEDMGMT_POE_LED == TRUE)
    if (STKTPLG_OM_IsPoeDevice(unit_id) == TRUE)
    {
        led_mgr_status.unit_info[unit_id - 1].sys_status.reserve = status;
        LEDDRV_SetPoeLed(unit_id, status);
    }
#endif

}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - LED_MGR_XFPModuleStatusChanged
 * ---------------------------------------------------------------------
 * PURPOSE  : Registered callback function, when thermal status is changed
 *            the registered function will be called.
 * INPUT    : unit          - unit number
 *            thermal       - thermal number
 *            status        - status of the XFPModule.
 * OUTPUT   : None.
 * RETURN   : None
 * NOTES    : None
 * ---------------------------------------------------------------------
 */
void LED_MGR_XFPModuleStatusChanged(UI32_T unit, UI32_T port, BOOL_T status)
{
    if(status == TRUE)
    {
        led_mgr_status.unit_info[unit-1].port_status[port-1].xfpmoduleinsert=TRUE;
    }
    else if(status == FALSE)
    {
        led_mgr_status.unit_info[unit-1].port_status[port-1].xfpmoduleinsert=FALSE;
    }

    LEDDRV_SetPortStatus(unit, port, FALSE, &led_mgr_status.unit_info[unit-1].port_status[port-1]);

    return;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : LED_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for dev_swdrvl4.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T LED_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    if(ipcmsg_p==NULL)
        return FALSE;

    ipcmsg_p->msg_size = LED_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();

    switch(LED_MGR_MSG_CMD(ipcmsg_p))
    {
        case LED_MGR_IPC_CMD_START_XFER:
            LED_MGR_Start_Xfer((*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id);
            break;
        case LED_MGR_IPC_CMD_STOP_XFER:
            LED_MGR_Stop_Xfer((*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id);
            break;
        case LED_MGR_IPC_CMD_SET_MODULE_LED:
            LED_MGR_SetModuleLED((*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                 (*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).color);
            break;
        #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE)
        case LED_MGR_IPC_CMD_SET_LOCATION_LED:
        {
            LED_MGR_IPCMsg_SetLocationLED_T *data_p;

            data_p = (LED_MGR_IPCMsg_SetLocationLED_T*)LED_MGR_MSG_DATA(ipcmsg_p);
            LED_MGR_MSG_RETVAL(ipcmsg_p) = (UI32_T)LED_MGR_SetLocationLED(data_p->unit_id, data_p->is_led_on);
        }
            break;

        case LED_MGR_IPC_CMD_GET_LOCATION_LED:
        {
            LED_MGR_IPCMsg_GetLocationLED_T *data_p;

            data_p = (LED_MGR_IPCMsg_GetLocationLED_T*)LED_MGR_MSG_DATA(ipcmsg_p);
            LED_MGR_MSG_RETVAL(ipcmsg_p) = (UI32_T)LED_MGR_GetLocationLEDStatus(data_p->unit_id, (BOOL_T*)data_p);
            ipcmsg_p->msg_size = LED_MGR_GET_MSGBUFSIZE(BOOL_T);
        }
            break;
        #endif /* end of #if(SYS_CPNT_LEDMGMT_LOCATION_LED == TRUE) */

#if(SYS_CPNT_POE==TRUE)
        case LED_MGR_IPC_CMD_SET_POE_LED:
            LED_MGR_SetPoeLED((*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                 (UI8_T)(*(LED_MGR_IPCMsg_SetUnitColor_T *)LED_MGR_MSG_DATA(ipcmsg_p)).color);
            break;

        case LED_MGR_IPC_CMD_SET_POE_LED_CALLBACK:
            LED_MGR_SetPOELed_callback((*(LED_MGR_IPCMsg_SetPoELED_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                 (*(LED_MGR_IPCMsg_SetPoELED_T *)LED_MGR_MSG_DATA(ipcmsg_p)).port_id,
                                 (*(LED_MGR_IPCMsg_SetPoELED_T *)LED_MGR_MSG_DATA(ipcmsg_p)).status);
            break;

#endif
#if(SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case LED_MGR_IPC_CMD_SET_FAN_FAIL_LED:
            LED_MGR_FanStatusChanged((*(LED_MGR_IPCMsg_SetUnitFanstatus_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                     (*(LED_MGR_IPCMsg_SetUnitFanstatus_T *)LED_MGR_MSG_DATA(ipcmsg_p)).fan_id,
                                     (*(LED_MGR_IPCMsg_SetUnitFanstatus_T *)LED_MGR_MSG_DATA(ipcmsg_p)).status);
            break;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case LED_MGR_IPC_CMD_THERMAL_STATUS_CHANGED:
            LED_MGR_ThermalStatusChanged((*(LED_MGR_IPCMsg_ThermalStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                         (*(LED_MGR_IPCMsg_ThermalStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).thermal_id,
                                         (*(LED_MGR_IPCMsg_ThermalStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).is_abnormal);
            break;
#endif
#if (SYS_CPNT_POWER_DETECT == TRUE)
        case LED_MGR_IPC_CMD_POWER_STATUS_CHANGED:
            LED_MGR_PowerStatusChanged((*(LED_MGR_IPCMsg_PowerStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                         (*(LED_MGR_IPCMsg_PowerStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).power_id,
                                         (*(LED_MGR_IPCMsg_PowerStatusChanged_T *)LED_MGR_MSG_DATA(ipcmsg_p)).status);
            break;
#endif

#if (SYS_CPNT_ALARM_DETECT==TRUE) && (SYS_CPNT_ALARM_SET_ALARM_LED_BY_LEDDRV==TRUE)
        case LED_MGR_IPC_CMD_SET_MAJOR_ALARM_OUTPUT_LED:
            LED_MGR_SetMajorAlarmOutputLed((*(LED_MGR_IPCMsg_SetAlarmOutputLed_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                           (*(LED_MGR_IPCMsg_SetAlarmOutputLed_T *)LED_MGR_MSG_DATA(ipcmsg_p)).status);
            break;
        case LED_MGR_IPC_CMD_SET_MINOR_ALARM_OUTPUT_LED:
            LED_MGR_SetMinorAlarmOutputLed((*(LED_MGR_IPCMsg_SetAlarmOutputLed_T *)LED_MGR_MSG_DATA(ipcmsg_p)).unit_id,
                                           (*(LED_MGR_IPCMsg_SetAlarmOutputLed_T *)LED_MGR_MSG_DATA(ipcmsg_p)).status);
            break;
#endif
        default:
            SYSFUN_Debug_Printf("%s(): Invalid cmd(%d).\n", __FUNCTION__, (int)(ipcmsg_p->cmd));
            LED_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            break;
    }
    return TRUE;

}
