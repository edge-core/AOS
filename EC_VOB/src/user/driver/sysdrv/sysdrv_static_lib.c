/* MODULE NAME:  sysdrv_static_lib.c
 * PURPOSE:
 *     This module contains the implementations of the functions that will be
 *     put in a ".a" archive(i.e. libsysdrv_static.a) to be linked statically
 *     against the binary executable file.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    10/22/2015 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg_common.h"
#include "sys_hwcfg.h"

#if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#endif

#include "sysfun.h"
#include "stktplg_board.h"
#include "uc_mgr.h"
#include "sysrsc_mgr.h"
#include "backdoor_lib.h"
#include "backdoor_mgr.h"
#include "phyaddr_access.h"
#include "i2cdrv.h"

#include "sysdrv_util.h"
#include "sysdrv.h"

#if ((SYS_CPNT_SYSDRV_USE_ONLP == TRUE) && (SYS_CPNT_POWER_DETECT == TRUE))
#include "onlpdrv_psu.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define BACKDOOR_OPEN

/* MACRO FUNCTION DECLARATIONS
 */
#define SYSDRV_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(sysdrv_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYSDRV_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(sysdrv_sem_id)

/* SYSDRV_STATIC_LIB_FUN_PROLOGUE
 *     Macro function used to check whether the static variables defined in this
 *     module had been initialized. This macro should be used in the beginning
 *     of the functions defined in this module except that the function will not
 *     refer to any static variables that are need to be initialized.
 */
#define SYSDRV_STATIC_LIB_FUN_PROLOGUE(...) \
    if(SYSDRV_STATIC_LIB_CheckInit()==FALSE) \
    { \
        printf("%s(%d):SYSDRV_STATIC_LIB_CheckInit error.\r\n", __FUNCTION__, __LINE__); \
        return __VA_ARGS__; \
    }

#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == FALSE)
#define STKTPLG_BOARD_GetThermalNumber() ({int __ret=SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; __ret;})
#endif

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SYSDRV_STATIC_LIB_Init(void);
#if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
/* anzhen.zheng, 2/20/2008 */
static BOOL_T SYSDRV_ReadEpldPage(UI32_T index, UI8_T *data_buffer );
#endif

#ifdef BACKDOOR_OPEN
static void SYSDRV_BackDoor_Menu(void);
static void SYSDRV_I2CBackDoor_Menu(void);
static void SYSDRV_I2CBackdoor_Shell(void);
static int SYSDRV_I2CBackdoorCmd(const BACKDOOR_LIB_SHELL_CMD_T *cmd, int argc, char *argv[]);
static void SYSDRV_BackDoor_ShowlogMenu(void);
static void SYSDRV_BackDoor_GetSfpInfo(void);
    #if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
static void SYSDRV_BackDoor_EpldSubMenu(void);
static BOOL_T SYSDRV_IsEpldAddressValid(UI32_T physical_address);
    #endif
#endif /* end of #ifdef BACKDOOR_OPEN */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T is_init=FALSE;
static SYSDRV_Shmem_Data_T *sysdrv_shmem_data_p=NULL;
static UI32_T sysdrv_sem_id;

#ifdef BACKDOOR_OPEN
enum
{
    SYSDRV_BACKDOOR_I2CSHELL_CMD_R,
    SYSDRV_BACKDOOR_I2CSHELL_CMD_R2,
    SYSDRV_BACKDOOR_I2CSHELL_CMD_W,
    SYSDRV_BACKDOOR_I2CSHELL_CMD_W2,
    SYSDRV_BACKDOOR_I2CSHELL_CMD_BUS
};

static const BACKDOOR_LIB_SHELL_CMD_T sysdrv_backdoor_i2c_shell_cmd[] = {
    /* name, func, usage, help */
    { "r" ,  (void *)SYSDRV_BACKDOOR_I2CSHELL_CMD_R,   SYSDRV_I2CBackdoorCmd, "r <addr> <offset> [len]", "do i2c read with offset" },
    { "r2",  (void *)SYSDRV_BACKDOOR_I2CSHELL_CMD_R2,  SYSDRV_I2CBackdoorCmd, "r2 <addr> [len]", "do i2c read without offset" },
    { "w",   (void *)SYSDRV_BACKDOOR_I2CSHELL_CMD_W,   SYSDRV_I2CBackdoorCmd, "w <addr> <offset> <write_data_hex_list>", "do i2c write with offset" },
    { "w2",  (void *)SYSDRV_BACKDOOR_I2CSHELL_CMD_W2,  SYSDRV_I2CBackdoorCmd, "w2 <addr> <write_data_hex_list>", "do i2c write without offset" },
    { "bus", (void *)SYSDRV_BACKDOOR_I2CSHELL_CMD_BUS, SYSDRV_I2CBackdoorCmd, "bus | bus <bus_index>", "show/set bus index" },
};
#endif

/* LOCAL INLINE FUNCTION
 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_STATIC_LIB_CheckInit
 *---------------------------------------------------------------------------
 * PURPOSE:  Check whether the static variables in this module have been
 *           initialized and do initialization if they have not been initialized.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
static inline BOOL_T SYSDRV_STATIC_LIB_CheckInit(void)
{
    if(is_init==FALSE)
    {
        is_init=TRUE;
        return SYSDRV_STATIC_LIB_Init();
    }
    return TRUE;
}

/* EXPORTED SUBPROGRAM BODIES
 */

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
void SYSDRV_Create_InterCSC_Relation(void)
{
#ifdef  BACKDOOR_OPEN
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sysi2cdrv", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, SYSDRV_I2CBackDoor_Menu);
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("sysdrv", SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, SYSDRV_BackDoor_Menu);
#endif  /* BACKDOOR_OPEN */
}


#if (SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE)

/* Determine which thermal driver ops need to be referenced -- START */
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_W83782))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_w83782;
#endif

#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_NE1617A))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_ne1617a;
#endif

#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_LM75))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_lm75;
#endif

#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_LM77))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_lm77;
#endif

#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_MAX6581))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_max6581;
#endif

#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_ONLP))!=0)
extern SYS_HWCFG_ThermalOps_T thermal_ops_onlp;
#endif

#if (SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP == 0)
#error "No thermal driver is included."
#endif

const static struct
{
    const UI32_T thermal_type;
    const SYS_HWCFG_ThermalOps_T *driver_ops;
} thermal_drivers[] =
    {
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_W83782))!=0)
        {SYS_HWCFG_THERMAL_W83782, &thermal_ops_w83782},
#endif
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_NE1617A))!=0)
        {SYS_HWCFG_THERMAL_NE1617A, &thermal_ops_ne1617a},
#endif
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_LM75))!=0)
        {SYS_HWCFG_THERMAL_LM75, &thermal_ops_lm75},
#endif
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_LM77))!=0)
        {SYS_HWCFG_THERMAL_LM77, &thermal_ops_lm77},
#endif
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_MAX6581))!=0)
        {SYS_HWCFG_THERMAL_MAX6581, &thermal_ops_max6581},
#endif
#if ((SYS_HWCFG_THERMAL_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_THERMAL_ONLP))!=0)
        {SYS_HWCFG_THERMAL_ONLP, &thermal_ops_onlp},
#endif
    };
/* Determine which thermal driver ops need to be referenced -- END */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetThermalControllerOps
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the thermal controller operations data of the given thermal controller
*            type.
 * INPUT:    thermal_ctl_type - Thermal controller type.
 *                              The value must be one of the constant prefixed with
 *                              "SYS_HWCFG_THERMAL_" which is defined in sys_hwcfg_common.h
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
static inline const SYS_HWCFG_ThermalOps_T* SYSDRV_THERMAL_GetThermalControllerOps(UI32_T thermal_ctl_type)
{
    UI32_T i;
    /* need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    for (i=0; i<sizeof(thermal_drivers)/sizeof(thermal_drivers[0]); i++)
    {
        if (thermal_ctl_type==thermal_drivers[i].thermal_type)
        {
            return thermal_drivers[i].driver_ops;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization to the thermal controller chip
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_CHIP_Init(void)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    const SYS_HWCFG_ThermalOps_T *thermal_drv_op_p;
    UI8_T thermal_idx;
    BOOL_T is_init_failed=FALSE;
    UI8_T nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE(FALSE);

    nbr_of_thermal = STKTPLG_BOARD_GetThermalNumber();

    for(thermal_idx=1; thermal_idx<=nbr_of_thermal; thermal_idx++)
    {
        if(STKTPLG_BOARD_GetThermalControllerInfo(thermal_idx, &thermal_ctl_info)==TRUE)
        {
            thermal_drv_op_p = SYSDRV_THERMAL_GetThermalControllerOps(thermal_ctl_info.thermal_type);
            if (thermal_drv_op_p)
            {
                if(thermal_drv_op_p->thermal_chip_init(thermal_idx)==FALSE)
                {
                    BACKDOOR_MGR_Printf("%s(%d):Thermal chip init failed on thermal_idx %hu\r\n",
                        __FUNCTION__, __LINE__, thermal_idx);
                    is_init_failed=TRUE;
                }
            }
            else
            {
                BACKDOOR_MGR_Printf("%s(%d): Failed to find driver op for thermal type %lu(thermal_idx=%hu)\r\n",
                    __FUNCTION__, __LINE__, (unsigned long)thermal_ctl_info.thermal_type, thermal_idx);
                is_init_failed=TRUE;
            }
        }
    }

    return (is_init_failed==FALSE)?TRUE:FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_THERMAL_GetTemperature
 *---------------------------------------------------------------------------
 * PURPOSE:  get current temperature from Thermal
 * INPUT:    index: Thermal index (starts by 1)
 * OUTPUT:   temperature:Thermal sensor temperature (Celsius)
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_THERMAL_GetTemperature(UI8_T index, I8_T* temperature)
{
    SYS_HWCFG_ThermalControlInfo_T thermal_ctl_info;
    const SYS_HWCFG_ThermalOps_T *thermal_drv_op_p;

/*  Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function.
 */

    if(STKTPLG_BOARD_GetThermalControllerInfo(index, &thermal_ctl_info)==FALSE)
        return FALSE;

    thermal_drv_op_p = SYSDRV_THERMAL_GetThermalControllerOps(thermal_ctl_info.thermal_type);
    if(thermal_drv_op_p==NULL)
        return FALSE;

    return thermal_drv_op_p->thermal_chip_get_temperature(index, temperature);
}
#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE)

#if (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP == 0)
#error "No fan controller driver is included"
#endif

/* Determine which fan driver ops need to be referenced -- START */
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_WIN83782))!=0)
extern SYS_HWCFG_FanControllerOps_T fan_controller_ops_win83782;
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_AOS5700_54X))!=0)
extern SYS_HWCFG_FanControllerOps_T fan_controller_ops_aos5700_54x;
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_AOS6700_32X))!=0)
extern SYS_HWCFG_FanControllerOps_T fan_controller_ops_aos6700_32x;
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_ONLP))!=0)
extern SYS_HWCFG_FanControllerOps_T fan_controller_ops_onlp;
#endif

/* Determine which fan driver ops need to be referenced -- END */

const static struct
{
    const UI32_T fan_controller_type;
    const SYS_HWCFG_FanControllerOps_T *driver_ops;
} fan_controller_drivers[] =
    {
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_WIN83782))!=0)
        {SYS_HWCFG_FAN_WIN83782, &fan_controller_ops_win83782},
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_AOS5700_54X))!=0)
        {SYS_HWCFG_FAN_AOS5700_54X, &fan_controller_ops_aos5700_54x},
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_AOS6700_32X))!=0)
        {SYS_HWCFG_FAN_AOS6700_32X, &fan_controller_ops_aos6700_32x},
#endif
#if ((SYS_HWCFG_FAN_SUPPORT_TYPE_BMP & (1UL<<SYS_HWCFG_FAN_ONLP))!=0)
        {SYS_HWCFG_FAN_ONLP, &fan_controller_ops_onlp},
#endif
    };

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetFanControllerOps
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the fan controller operations data of the given fan controller
*            type.
 * INPUT:    fan_ctl_type - Fan controller type.
 *                          The value must be one of the constant prefixed with
 *                          "SYS_HWCFG_FAN_" which is defined in sys_hwcfg_common.h
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
static inline const SYS_HWCFG_FanControllerOps_T* SYSDRV_FAN_GetFanControllerOps(UI32_T fan_ctl_type)
{
    UI32_T i;
    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    for (i=0; i<sizeof(fan_controller_drivers)/sizeof(fan_controller_drivers[0]); i++)
    {
        if (fan_ctl_type==fan_controller_drivers[i].fan_controller_type)
        {
            return fan_controller_drivers[i].driver_ops;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_CHIP_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Do initialization to the fan controller chip
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_CHIP_Init(void)
{
    const SYS_HWCFG_FanControllerOps_T *fan_drv_op_p;
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    UI8_T fan_idx, nbr_of_fan;
    BOOL_T is_init_failed=FALSE;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    nbr_of_fan=STKTPLG_BOARD_GetFanNumber();

    for (fan_idx=1; fan_idx<=nbr_of_fan; fan_idx++)
    {
        if (STKTPLG_BOARD_GetFanControllerInfo(fan_idx, &fan_ctl_info)==TRUE)
        {
            fan_drv_op_p = SYSDRV_FAN_GetFanControllerOps(fan_ctl_info.fan_ctl_type);
            if(fan_drv_op_p)
            {
                if(fan_drv_op_p->fan_chip_init(fan_idx)==FALSE)
                {
                    printf("%s(%d):Fan chip init failed on fan_idx %hu\r\n",
                        __FUNCTION__, __LINE__, fan_idx);
                    is_init_failed=TRUE;
                }
            }
            else
            {
                printf("%s(%d): Failed to find driver op for fan type %lu(fan_idx=%hu)\r\n",
                    __FUNCTION__, __LINE__, (unsigned long)fan_ctl_info.fan_ctl_type, fan_idx);
                is_init_failed=TRUE;
            }
        }
    }

    return (is_init_failed==FALSE)?TRUE:FALSE;
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInRPM
 *---------------------------------------------------------------------------
 * PURPOSE:  get fan speed value in rpm.
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed in rpm
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInRPM(UI8_T index, UI32_T* speed_p)
{
    const SYS_HWCFG_FanControllerOps_T *fan_drv_op_p;
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    if(STKTPLG_BOARD_GetFanControllerInfo(index, &fan_ctl_info)==FALSE)
        return FALSE;

    fan_drv_op_p = SYSDRV_FAN_GetFanControllerOps(fan_ctl_info.fan_ctl_type);

    if (fan_drv_op_p==NULL)
        return FALSE;

    return fan_drv_op_p->fan_chip_get_speed_in_rpm(index, speed_p);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_FAN_GetSpeedInDutyCycle
 *---------------------------------------------------------------------------
 * PURPOSE:  get fan speed value in duty cycle
 * INPUT:    index -- fan index.
 * OUTPUT:   speed_p -- fan speed in duty cycle
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_FAN_GetSpeedInDutyCycle(UI8_T index, UI32_T* speed_p)
{
    const SYS_HWCFG_FanControllerOps_T *fan_drv_op_p;
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;

    if(STKTPLG_BOARD_GetFanControllerInfo(index, &fan_ctl_info)==FALSE)
        return FALSE;

    fan_drv_op_p = SYSDRV_FAN_GetFanControllerOps(fan_ctl_info.fan_ctl_type);

    if (fan_drv_op_p==NULL)
        return FALSE;

    return fan_drv_op_p->fan_chip_get_speed_in_duty_cycle(index, speed_p);
}

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
BOOL_T SYSDRV_FAN_SetSpeed(UI8_T index, UI32_T speed)
{
    const SYS_HWCFG_FanControllerOps_T *fan_drv_op_p;
    SYS_HWCFG_FanControllerInfo_T fan_ctl_info;
    BOOL_T ret;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE(FALSE);

    if(index==0)
    {
        printf("%s(%d):Invalid index\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(STKTPLG_BOARD_GetFanControllerInfo(index, &fan_ctl_info)==FALSE)
        return FALSE;

    fan_drv_op_p = SYSDRV_FAN_GetFanControllerOps(fan_ctl_info.fan_ctl_type);

    if (fan_drv_op_p==NULL)
        return FALSE;

    SYSDRV_ENTER_CRITICAL_SECTION();
    /* set fan speed transaction need to be protected by the critical section
     */
    ret=fan_drv_op_p->fan_chip_set_speed(index, speed);
    SYSDRV_LEAVE_CRITICAL_SECTION();

    return ret;
}
#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) */

#if (SYS_CPNT_POWER_DETECT == TRUE)
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
BOOL_T SYSDRV_GetPowerStatusFromASIC(UI32_T power_index, UI8_T *power_status_p)
{
    SYSDRV_STATIC_LIB_FUN_PROLOGUE(FALSE);

#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)
    return ONLPDRV_PSU_GetPowerStatus(power_index, power_status_p);
#else /* #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */
#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    SYS_HWCFG_PowerRegInfo_T power_reg_info;
#endif

    if(power_index==0 || power_index>SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
    {
        BACKDOOR_MGR_Printf("%s:Invalid power index %lu\r\n", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }

    if(power_status_p==NULL)
    {
        BACKDOOR_MGR_Printf("%s:Invalid power_status_p\r\n", __FUNCTION__);
        return FALSE;
    }

#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    if (STKTPLG_BOARD_GetPowerStatusInfo(power_index, &power_reg_info) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s:STKTPLG_BOARD_GetPowerStatusInfo failed.(power_index=%lu)\r\n", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }
    else if (SYSDRV_UTIL_ReadPowerRegByRegInfo(&power_reg_info, 1, power_status_p) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s:SYSDRV_UTIL_ReadPowerRegByRegInfo failed.(power_index=%lu)\r\n", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }

#elif (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR)
    if(FALSE==PHYADDR_ACCESS_Read(power_status_addr[power_index-1], 1, 1, power_status_p))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("\r\n%s: Access SYS_HWCFG_POWER_STATUS_ADDR(power_index=%lu) fail", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }
#elif (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_I2C)
    #if defined(ES4627MB)
    {
        UC_MGR_Sys_Info_T sys_info;

        if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
        {
            BACKDOOR_MGR_Printf("%s(%d)Failed to get sysinfo from UC.\r\n", __FUNCTION__, __LINE__);
            return FALSE;
        }
        if(sys_info.board_id==2 || sys_info.board_id==6) /* special hw design on ECS4510-28F */
        {
            if(I2CDRV_TwsiDataReadWithBusIdx(1, SYS_HWCFG_POWER_STATUS_I2C_ADDR,
                I2C_7BIT_ACCESS_MODE, TRUE, power_status_addr[power_index-1],
                FALSE, 1, power_status_p)==FALSE)
            {
                if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
                    BACKDOOR_MGR_Printf("\r\n%s: Access SYS_HWCFG_POWER_STATUS_ADDR fail(power_index=%lu)\r\n", __FUNCTION__, (unsigned long)power_index);
                return FALSE;
            }
            return TRUE;
        }
    }
    #endif /* end of #if defined(ES4627MB) */
    if(FALSE==I2CDRV_GetI2CInfo(SYS_HWCFG_POWER_STATUS_I2C_ADDR, power_status_addr[power_index-1], 1, power_status_p))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("\r\n%s: Access SYS_HWCFG_POWER_STATUS_ADDR fail(power_index=%lu)\r\n", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }
#else /* #if (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR) */
#error "Unknown SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD"
#endif/* #if (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD == SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR) */
#endif /* #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */

    return TRUE;
}

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetPowerPresentStatus
 *---------------------------------------------------------------------------
 * PURPOSE:  Get power present status from the register.
 * INPUT  :  power_index          -- Which power(1 based)
 * OUTPUT :  is_present_p         -- TRUE: present, FALSE: not present
 * RETURN :  TRUE - success FALSE - failed
 * NOTE   :  None.
 *---------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_GetPowerPresentStatus(UI32_T power_index, BOOL_T *is_present_p)
{
    UI8_T reg_val;
    UI8_T mask_ar[]= SYS_HWCFG_POWER_STATUS_MASK_ARRAY;
    UI8_T present_ar[]= SYS_HWCFG_POWER_STATUS_PRESENT_VAL_ARRAY;

    /* Need not to use SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */
    if(is_present_p==NULL)
        return FALSE;

    if(power_index<1 && power_index>(sizeof(mask_ar)/sizeof(mask_ar[0])))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("%s:Invalid power_index %lu\r\n", __FUNCTION__, (unsigned long)power_index);
        return FALSE;
    }

    if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_POWER)
    {
        reg_val=sysdrv_shmem_data_p->sysdrv_local_unit_power_status_bd_dbg[power_index-1];
    }
    else
    {
        if(FALSE==SYSDRV_GetPowerStatusFromASIC(power_index, &reg_val))
        {
            return FALSE;
        }
    }

    reg_val &= mask_ar[power_index-1];
    if(reg_val==present_ar[power_index-1])
        *is_present_p=TRUE;
    else
        *is_present_p=FALSE;

    return TRUE;
}

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
BOOL_T SYSDRV_GetPowerModuleType(UI32_T power_index, UI32_T *power_module_type_p)
{
    UI8_T reg_val;
    UI8_T mask[] = SYS_HWCFG_POWER_TYPE_MASK_ARRAY;
    UI8_T bit_shift[] = SYS_HWCFG_POWER_TYPE_BIT_SHIFT_ARRAY;
    UI8_T regval_to_module_type_ar[] =SYS_HWCFG_POWER_TYPE_REG_VAL_TO_POWER_TYPE_ARRAY;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE(FALSE);

    if(power_module_type_p==NULL)
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s(%d): invalid power_module_type_p\r\n", __FUNCTION__, __LINE__);
        }
        return FALSE;
    }

    if(power_index<1 && (power_index>(sizeof(mask)/sizeof(mask[0]))))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
        {
            BACKDOOR_MGR_Printf("%s(%d): invalid power index=%lu\r\n", __FUNCTION__, __LINE__, (unsigned long)power_index);
        }
        return FALSE;
    }

    if(sysdrv_shmem_data_p->bd_debug_flag & SYSDRV_BD_DEBUG_FLAG_POWER_TYPE)
    {
        reg_val=sysdrv_shmem_data_p->sysdrv_local_unit_power_type_bd_dbg[power_index-1];
    }
    else
    {
        if(FALSE==PHYADDR_ACCESS_Read(power_type_addr[power_index-1], 1, 1, &reg_val))
        {
            if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
                BACKDOOR_MGR_Printf("%s: read addr=0x%lx fail(power_index=%lu)\r\n", __FUNCTION__, (unsigned long)power_type_addr[power_index-1], (unsigned long)power_index);
            return FALSE;
        }
    }

    reg_val&=mask[power_index-1];
    reg_val = reg_val >> (bit_shift[power_index-1]);

    if(reg_val>=(sizeof(regval_to_module_type_ar)/sizeof(regval_to_module_type_ar[0])))
    {
        if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("%s: Failed to convert to module type(reg_val=%u)\r\n", __FUNCTION__, reg_val);
        return FALSE;
    }

    *power_module_type_p=regval_to_module_type_ar[reg_val];

    if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
        BACKDOOR_MGR_Printf("power_module_type=%lu\r\n", (unsigned long)*power_module_type_p);

    return TRUE;
}

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
BOOL_T SYSDRV_GetPowerModuleSetStatus(BOOL_T *is_status_good_p)
{
    UI32_T power_index, power_module_type;
    UI32_T found_dc_power_type=SYS_HWCFG_COMMON_POWER_UNKNOWN_MODULE_TYPE;
    BOOL_T dc_power_exists=FALSE;
    BOOL_T is_present=FALSE;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE(FALSE);

    /* assume status is good at first
     */
    *is_status_good_p=TRUE;

    /* get module type
     */
    for(power_index=1; power_index<=SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; power_index++)
    {
        /* Do not need to check power type if power not present
         */
        if(SYSDRV_GetPowerPresentStatus(power_index, &is_present)==FALSE)
        {
            if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(): Failed to get power present status of power %lu\r\n",
                    __FUNCTION__, (unsigned long)power_index);
            }
            return FALSE;
        }

        if(is_present==FALSE)
            continue;

        /* check any dc power exist
         * keep dc power type if found
         */
        if(SYSDRV_GetPowerModuleType(power_index, &power_module_type)==FALSE)
        {
            if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s(): Failed to get power type of power %lu\r\n",
                    __FUNCTION__, (unsigned long)power_index);
            }
            return FALSE;
        }

        if(dc_power_exists==FALSE && SYS_HWCFG_COMMON_POWER_TYPE_IS_DC(power_module_type)==TRUE)
        {
            dc_power_exists=TRUE;
            found_dc_power_type=power_module_type;
        }
        else if(SYS_HWCFG_COMMON_POWER_TYPE_IS_DC(power_module_type)==TRUE)
        {
            /* if other dc power exist and its type is different from
             * the first found dc power, module set status is not ok
             */
            if(power_module_type!=found_dc_power_type)
            {
                *is_status_good_p=FALSE;
                break;
            }
        }

    }

    return TRUE;
}

#endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */
#endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

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
void SYSDRV_DetectPeripheralInstall(void)
{
    UI32_T retrycnt,i;
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    UI32_T speed;
#endif
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    I8_T  temperture;
#endif
    UI8_T nbr_of_thermal = SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE();

    /* detect fan */
    for (i = 0; i< SYS_HWCFG_MAX_NBR_OF_FAN_PER_UNIT; i++)
    {
        for (retrycnt= 1;  retrycnt<= 2; retrycnt++)
        {

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && \
    (((SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) && (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP != 0)) || (SYS_HWCFG_FAN_CONTROLLER_TYPE != SYS_HWCFG_FAN_NONE))
            SYSDRV_ENTER_CRITICAL_SECTION();
    #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT == TRUE)
            sysdrv_shmem_data_p->sysdrv_fan_install_status[i] = SYSDRV_FAN_GetSpeedInRPM(i+1,&speed);
    #else
            sysdrv_shmem_data_p->sysdrv_fan_install_status[i] = SYSDRV_FAN_GetSpeedInDutyCycle(i+1,&speed);
    #endif
            SYSDRV_LEAVE_CRITICAL_SECTION();

            if (TRUE == sysdrv_shmem_data_p->sysdrv_fan_install_status[i])
            {
                SYSFUN_Debug_Printf("Fan %d sensor detect.\n\r", i);
                break;
            }
            else if ((FALSE == sysdrv_shmem_data_p->sysdrv_fan_install_status[i]) && ( 2 == retrycnt))
            {
                SYSFUN_Debug_Printf("SYSDRV_DetectPeripheralInstall: Detect Fan%d fail\n\r",i);
            }
#endif
        }
    }

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    nbr_of_thermal = STKTPLG_BOARD_GetThermalNumber();
#endif
     /*detect Thermal */
    for (i = 0; i< nbr_of_thermal; i++)
    {
             for (retrycnt= 1;  retrycnt<= 2; retrycnt++)
            {
#if (SYS_CPNT_THERMAL_DETECT == TRUE)  /* Thomas added for 3628bt build error */
                SYSDRV_ENTER_CRITICAL_SECTION();
                sysdrv_shmem_data_p->sysdrv_thermal_install_status[i] = SYSDRV_THERMAL_GetTemperature(i+1, &temperture);
                SYSDRV_LEAVE_CRITICAL_SECTION();

                if (TRUE == sysdrv_shmem_data_p->sysdrv_thermal_install_status[i])
                {
                    SYSFUN_Debug_Printf("Thermal %d sensor detect.\n\r", i);
                    break;
                }
                else if ((FALSE == sysdrv_shmem_data_p->sysdrv_thermal_install_status[i]) && ( 2 == retrycnt))
                {
                    SYSFUN_Debug_Printf("SYSDRV_DetectPeripheralInstall: Detect thermal%d fail\n\r",i);
                }
#endif
            }
    }

    return;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_STATIC_LIB_Init
 *---------------------------------------------------------------------------
 * PURPOSE:  Initialize the static variables defined in this module.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   TRUE - Success / FALSE - Failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_STATIC_LIB_Init(void)
{
    UI32_T ret;

    sysdrv_shmem_data_p = (SYSDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYSDRV_SHMEM_SEGID);

    if (sysdrv_shmem_data_p==NULL)
    {
        printf("%s(%d)SYSRSC_MGR_GetShMem fails.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSDRV, &sysdrv_sem_id);
    if (ret != SYSFUN_OK)
    {
        printf("%s(%d)SYSFUN_GetSem fails.(rc=%lu)\n", __FUNCTION__, __LINE__, (unsigned long)ret);
        return FALSE;
    }

    return TRUE;
}

#if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
/* anzhen.zheng */
/* FUNCTION NAME: SYSDRV_ReadEpldPage
 * PURPOSE  : This function is for user mode program to read data from virtual address
 *            which is got from H/W physical address
 * INPUT    : virtaul_address  -- virtual address (got from SYSDRV_EPLD_GetVirtualAddr )
 * OUTPUT   : data_buffer       -- the data buffer to store the read data
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    :
 */
static BOOL_T SYSDRV_ReadEpldPage(UI32_T index, UI8_T *data_buffer )
{
    UI32_T i;
    UI8_T  data[SYS_HWCFG_EPLD_PAGE_SIZE] = {0}, temp[4] = {0};
    UI32_T physical_address;

    if (index < SYS_HWCFG_EPLD_MIN_PAGE || index > SYS_HWCFG_EPLD_MAX_PAGE)
    {
        printf(" Wrong index: %ld\n", (unsigned long)index);
        return FALSE;
    }

    if(NULL == data_buffer)
        return FALSE;

    switch(index)
    {
#ifdef SYS_HWCFG_EPLD_PAGE1_ADDR
        case 1:
            physical_address = SYS_HWCFG_EPLD_PAGE1_ADDR;
            break;
#endif
#ifdef SYS_HWCFG_EPLD_PAGE2_ADDR
        case 2:
            physical_address = SYS_HWCFG_EPLD_PAGE2_ADDR;
            break;
#endif
#ifdef SYS_HWCFG_EPLD_PAGE3_ADDR
        case 3:
            physical_address = SYS_HWCFG_EPLD_PAGE3_ADDR;
            break;
#endif
#ifdef SYS_HWCFG_EPLD_PAGE4_ADDR
        case 4:
            physical_address = SYS_HWCFG_EPLD_PAGE4_ADDR;
            break;
#endif
#ifdef SYS_HWCFG_EPLD_PAGE5_ADDR
        case 5:
            physical_address = SYS_HWCFG_EPLD_PAGE5_ADDR;
            break;
#endif
        default:
            printf(" Wrong index: %ld\n", (long)index);
            return FALSE;
    }

    data_buffer[0] = '\0';
    PHYSICAL_ADDR_ACCESS_Read(physical_address, 1, SYS_HWCFG_EPLD_PAGE_SIZE, (UI8_T *)&data[0]);
    for(i = 0; i < SYS_HWCFG_EPLD_PAGE_SIZE; i++)
    {
        sprintf((char *)(&temp[0]), "%02x ", data[i]);
        strcat((char *)data_buffer, (char *)(&temp[0]));
    }
    data_buffer[3 * SYS_HWCFG_EPLD_PAGE_SIZE] = '\0';
    return TRUE;
}
#endif /* end of #if defined(SYS_HWCFG_EPLD_PAGE_SIZE) */

#ifdef BACKDOOR_OPEN
static void SYSDRV_BackDoor_Menu(void)
{
    UI8_T   ch;
    UI8_T   buf[16];
    BOOL_T  eof=FALSE;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE();

    /*  BODY
     */
    while (! eof)
    {
        BACKDOOR_MGR_Printf("\r\n==========SYSDRV BackDoor Menu================\r\n");
        BACKDOOR_MGR_Printf("\r\n 0. Exit\r\n");
#if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
        BACKDOOR_MGR_Printf(" 1. Read/Write EPLD \r\n");
#endif
        BACKDOOR_MGR_Printf(" 2. showlog\r\n");
        BACKDOOR_MGR_Printf(" 3. GetSfpInfo\r\n");
        BACKDOOR_MGR_Printf(" 4. Set sysdrv backdoor debug flag(0x%08lX)\r\n", (unsigned long)sysdrv_shmem_data_p->bd_debug_flag);
        #if (SYS_CPNT_POWER_DETECT==TRUE)
        BACKDOOR_MGR_Print(" 5. Set debug power status\r\n");
        #endif
        #if (SYS_CPNT_STKTPLG_FAN_DETECT==TRUE)
        BACKDOOR_MGR_Print(" 6. Set debug fan status\r\n");
        #endif
        #if (SYS_CPNT_THERMAL_DETECT==TRUE)
        BACKDOOR_MGR_Print(" 7. Set debug thermal status\r\n");
        #endif
        #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
        BACKDOOR_MGR_Print(" 8. Set power type\r\n");
        #endif
        #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
        BACKDOOR_MGR_Print(" 9. Dump fan speed calibration adjust counter\r\n");
        BACKDOOR_MGR_Print(" a. Reset all fan speed calibration adjust counters\r\n");
        BACKDOOR_MGR_Print(" b. Change fan speed in transition reset counter value\r\n");
        #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
        #if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
        BACKDOOR_MGR_Print(" c. upld EPLD\r\n");
        #endif
        BACKDOOR_MGR_Printf("=================================================\r\n");
        BACKDOOR_MGR_Printf("     select =");
        ch = BACKDOOR_MGR_GetChar();
#if 0
        if ((ch < 0x30)||(ch>0x39))
            continue;
        ch -= 0x30;
#endif

        BACKDOOR_MGR_Printf ("%c\r\n",ch);
        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;
#if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
            case '1':
                SYSDRV_BackDoor_EpldSubMenu();
                break;
#endif
            case '2':
                SYSDRV_BackDoor_ShowlogMenu();
                break;
            case '3':
                SYSDRV_BackDoor_GetSfpInfo();
                break;
            case '4':
                #if (SYS_CPNT_POWER_DETECT==TRUE)
                BACKDOOR_MGR_Printf("Power debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_POWER);
                #endif
                #if (SYS_CPNT_STKTPLG_FAN_DETECT==TRUE)
                BACKDOOR_MGR_Printf("Fan debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_FAN);
                #endif
                #if (SYS_CPNT_THERMAL_DETECT==TRUE)
                BACKDOOR_MGR_Printf("Thermal debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_THERMAL);
                #endif
                #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
                BACKDOOR_MGR_Printf("Power type debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_POWER_TYPE);
                #endif
                BACKDOOR_MGR_Printf("Show error message debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_SHOW_ERROR_MSG);
                BACKDOOR_MGR_Printf("Show debug message debug flag =0x%02X\r\n", SYSDRV_BD_DEBUG_FLAG_SHOW_DEBUG_MSG);
                BACKDOOR_MGR_Print("New debug flag=0x");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Print("\r\n");
                sysdrv_shmem_data_p->bd_debug_flag = strtoul((char*)buf, NULL, 16);
                break;

            #if (SYS_CPNT_POWER_DETECT==TRUE)
            case '5':
            {
                UI8_T i;

                BACKDOOR_MGR_Print("Original debug power status:");
                for(i=0; i<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; i++)
                    BACKDOOR_MGR_Printf("[0x%02x] ", sysdrv_shmem_data_p->sysdrv_local_unit_power_status_bd_dbg[i]);

                BACKDOOR_MGR_Print("\r\n");

                BACKDOOR_MGR_Print("Power index? ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Print("\r\n");
                i = strtoul((char*)buf, NULL, 10);
                if(i<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
                {
                    BACKDOOR_MGR_Print("New setting=0x");
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                    BACKDOOR_MGR_Print("\r\n");
                    sysdrv_shmem_data_p->sysdrv_local_unit_power_status_bd_dbg[i] = strtoul((char*)buf, NULL, 16);
                }
            }
                break;
            #endif /* end of #if (SYS_CPNT_POWER_DETECT==TRUE) */

            #if (SYS_CPNT_STKTPLG_FAN_DETECT==TRUE)
            case '6':
            {
                UI8_T i;

                BACKDOOR_MGR_Printf("Original debug fan status=");
                for (i=0; i<SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE; i++)
                {
                    BACKDOOR_MGR_Printf("0x%02X ", sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg[i]);
                }
                BACKDOOR_MGR_Printf("\r\n");
                for (i=0; i<SYS_HWCFG_FAN_FAULT_STATUS_MAX_ARRAY_SIZE; i++)
                {
                    BACKDOOR_MGR_Printf("New setting[%d]=0x", i);
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                    BACKDOOR_MGR_Printf("\r\n");
                    sysdrv_shmem_data_p->sysdrv_local_unit_fan_status_bd_dbg[i] = strtoul((char*)buf, NULL, 16);
                }
            }
                break;
            #endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT==TRUE) */
            #if (SYS_CPNT_THERMAL_DETECT==TRUE)
            case '7':
            {
                UI8_T i;

                BACKDOOR_MGR_Print("Original debug thermal status:");
                for(i=0; i<SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; i++)
                    BACKDOOR_MGR_Printf("[0x%02x] ", sysdrv_shmem_data_p->sysdrv_thermal_temp_bd_dbg[i]);

                BACKDOOR_MGR_Print("\r\n");

                BACKDOOR_MGR_Print("Thermal index? ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Print("\r\n");
                i = strtoul((char*)buf, NULL, 10);
                if(i<SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)
                {
                    BACKDOOR_MGR_Print("New setting=0x");
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                    BACKDOOR_MGR_Print("\r\n");
                    sysdrv_shmem_data_p->sysdrv_thermal_temp_bd_dbg[i] = strtoul((char*)buf, NULL, 16);
                }
                else
                    BACKDOOR_MGR_Print("Invalid thermal index\r\n");
            }
                break;
            #endif /* end of #if (SYS_CPNT_THERMAL_DETECT==TRUE) */
            #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
            case '8':
            {
                UI8_T i;

                BACKDOOR_MGR_Print("Original power type status:");
                for(i=0; i<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; i++)
                    BACKDOOR_MGR_Printf("[0x%02x] ", sysdrv_shmem_data_p->sysdrv_local_unit_power_type_bd_dbg[i]);

                BACKDOOR_MGR_Print("\r\n");

                BACKDOOR_MGR_Print("Power index? ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Print("\r\n");
                i = strtoul((char*)buf, NULL, 10);
                if(i<SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT)
                {
                    BACKDOOR_MGR_Print("New setting=0x");
                    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                    BACKDOOR_MGR_Print("\r\n");
                    sysdrv_shmem_data_p->sysdrv_local_unit_power_type_bd_dbg[i] = strtoul((char*)buf, NULL, 16);
                }
                else
                    BACKDOOR_MGR_Print("Invalid power index\r\n");
            }
                break;

            #endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */
            #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
            case '9':
            {
                UI8_T i;

                BACKDOOR_MGR_Printf("SYSDRV_FAN_MID_SPEED mode:");
                for (i=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
                {
                    BACKDOOR_MGR_Printf("%d ", sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter[SYSDRV_FAN_MID_SPEED][i]);
                }
                BACKDOOR_MGR_Printf("\r\n");

                BACKDOOR_MGR_Printf("SYSDRV_FAN_FULL_SPEED mode:");
                for (i=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
                {
                    BACKDOOR_MGR_Printf("%d ", sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter[SYSDRV_FAN_FULL_SPEED][i]);
                }
                BACKDOOR_MGR_Printf("\r\n");
            }
                break;
            case 'a':
            {
                memset(sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter, 0,
                    sizeof(sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter));

                BACKDOOR_MGR_Printf("Reset OK.\r\n");
            }
                break;
            case 'b':
            {
                BACKDOOR_MGR_Printf("Current Value=%hu\r\n", sysdrv_shmem_data_p->fan_speed_in_transition_counter_reset_value);
                BACKDOOR_MGR_Printf("New Value=");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                sysdrv_shmem_data_p->fan_speed_in_transition_counter_reset_value=(UI8_T)(atoi((char*)buf));
            }
                break;

            #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) && (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE) */
            #if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
            case 'c':
            {
                FILE *fd;
                struct stat file_stat;
                int    rc;
                char   ip_buffer[28];
                char   sys_buffer[64];
                char   file_buffer[64];
                UI8_T  *f_buf = NULL;
                UI32_T  f_size = 0;

                rc = stat(UPLOAD_CPLD_SCRIPT_FILE, &file_stat);
                if(rc!=0)
                {
                    BACKDOOR_MGR_Printf("Do not support cpld yet.\r\n");
                    break;
                }
                BACKDOOR_MGR_Printf("enter TFTP server ip: ");
                BACKDOOR_MGR_RequestKeyIn(ip_buffer, sizeof(ip_buffer)-1);
                BACKDOOR_MGR_Printf("\r\n");

                BACKDOOR_MGR_Printf("enter cpld finename: ");
                BACKDOOR_MGR_RequestKeyIn(file_buffer, sizeof(file_buffer)-1);
                BACKDOOR_MGR_Printf("\r\n");

                snprintf(sys_buffer, sizeof(sys_buffer), "%s %s %s", UPLOAD_CPLD_SCRIPT_FILE, ip_buffer, file_buffer);
                SYSFUN_ExecuteSystemShell(sys_buffer);

                snprintf(sys_buffer, sizeof(sys_buffer), "/tmp/%s", file_buffer);
                rc = stat(sys_buffer, &file_stat);
                f_size = file_stat.st_size;
                if(rc!=0 || f_size == 0)
                {
                    BACKDOOR_MGR_Printf("stat: tftp CPLD file error.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf("upload file ok, size = %d\r\n", f_size);

                fd = fopen(sys_buffer, "rb");
                if (fd)
                {
                    f_buf = (UI8_T*)malloc(sizeof(UI8_T)*f_size);
                    fread(f_buf, f_size, 1, fd);
                }
                else
                {
                    // error opening file
                    BACKDOOR_MGR_Printf("tftp CPLD file error.\r\n");
                    break;
                }
                fclose(fd);

                if(TRUE == SYSDRV_Upgrade_CPLD(f_buf, f_size))
                        BACKDOOR_MGR_Printf("\r\n upload CPLD successfully.");
                else
                        BACKDOOR_MGR_Printf("\r\n upload CPLD fail.");

                free(f_buf);
                BACKDOOR_MGR_Printf("Done.\r\n");
                break;
            }
            #endif
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */
}   /*  SYSDRV_BackDoor_Menu   */

static void SYSDRV_I2CBackDoor_Menu (void)
{
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
    UI32_T  baudrate;
    char    buf[20] = {0};
    UI8_T   reg_addr, reg_val, idx;
#endif
    int     ch;
    BOOL_T  eof=FALSE;

    SYSDRV_STATIC_LIB_FUN_PROLOGUE();

    /*  BODY
     */
    while (! eof)
    {
        BACKDOOR_MGR_Printf("\r\n==========SYSDRV I2C BackDoor Menu================\r\n");
        BACKDOOR_MGR_Printf("\r\n 0. Exit\r\n");
        BACKDOOR_MGR_Printf(" 1. i2c shell \r\n");
#if (SYS_HWCFG_SUPPORT_RTC == TRUE)  /* Thomas added for 3628bt build error */
        BACKDOOR_MGR_Printf(" 4. I2C Read RTC related \r\n");
        BACKDOOR_MGR_Printf(" 5. I2C Write RTC related \r\n");
#endif /* SYS_HWCFG_SUPPORT_RTC==TRUE */
#if (SYS_CPNT_THERMAL_DETECT == TRUE)  /* Thomas added for 3628bt build error */
        BACKDOOR_MGR_Printf(" 6. I2C Get temperature related \r\n");
#endif /* SYS_CPNT_THERMAL_DETECT == TRUE */
        BACKDOOR_MGR_Printf(" 7. I2C Get EEPROM related \r\n");
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        BACKDOOR_MGR_Printf(" 8. I2C Get fans speed related \r\n");
#endif
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
        BACKDOOR_MGR_Printf(" 9. Read UART TO I2C register \r\n");
        BACKDOOR_MGR_Printf(" a. Write UART TO I2C register \r\n");
        BACKDOOR_MGR_Printf(" b. Change UART2 baudrate\r\n");
        BACKDOOR_MGR_Printf(" c. Change UART TO I2C baudrate\r\n");
        BACKDOOR_MGR_Printf(" d. Read UART2\r\n");
#endif
        BACKDOOR_MGR_Printf("=================================================\n");
        BACKDOOR_MGR_Printf("     select =");

get_input_char:
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c",ch);
        if (ch==EOF)
            goto get_input_char;

        BACKDOOR_MGR_Printf("\r\n");

        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;
            case '1':
                SYSDRV_I2CBackdoor_Shell();
                break;
#if (SYS_HWCFG_SUPPORT_RTC == TRUE)  /* Thomas added for 3628bt build error */
            case '4':
            {
                int yr, mon, day, hr, min, sec;
                if(TRUE == SYSDRV_RTC_GetDateTime(NULL, NULL, &yr, &mon, &day, &hr, &min, &sec))
                {
                    printf("%d/%d %d:%d:%d %d\r\n", mon, day, hr, min, sec, yr);
                }
                else
                    printf("Read RTC fail\r\n");
            }
                break;
            case '5':
                printf("Use CLI command: calendar set\n");
                break;
#endif /* SYS_HWCFG_SUPPORT_RTC==TRUE */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
            case '6':
            {
                UI8_T i;
                I8_T  thermal_recv;
                for (i = 0; i < SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; i++)
                {
                    if(SYSDRV_THERMAL_GetTemperature(i+1, &thermal_recv) != TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\nFailed to get temperature %u\r\n", i + 1);
                    }
                    else
                    {
                        BACKDOOR_MGR_Printf("\r\ntemperature %u is %2d degree celsius\r\n", i + 1, thermal_recv);
                    }
                }
            }
            break;
#endif /* SYS_CPNT_THERMAL_DETECT == TRUE */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && \
    (((SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC == TRUE) && (SYS_HWCFG_FAN_SUPPORT_TYPE_BMP != 0)) || (SYS_HWCFG_FAN_CONTROLLER_TYPE != SYS_HWCFG_FAN_NONE))
            case '8':
            {
                UI32_T i, fan_speed;
                for(i=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
                {
#if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
                    if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(i)))==0)
                    {
                        continue;
                    }
#endif
                    if(SYSDRV_FAN_GetSpeedInDutyCycle(i+1, &fan_speed) != TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\nFailed to get fan %lu speed in duty cycle\r\n", (unsigned long)i + 1);
                    }
                    else
                    {
                        BACKDOOR_MGR_Printf("\r\nFans %lu duty cycle: %lu%%\r\n", (unsigned long)i + 1, (unsigned long)fan_speed);
                    }

                    if(SYSDRV_FAN_GetSpeedInRPM(i+1, &fan_speed) != TRUE)
                    {
                        BACKDOOR_MGR_Printf("\r\nFailed to get fan %lu speed in RPM\r\n", (unsigned long)i + 1);
                    }
                    else
                    {
                        BACKDOOR_MGR_Printf("\r\nFans %lu speed: %lu RPM\r\n", (unsigned long)i + 1, (unsigned long)fan_speed);
                    }
                }
            }
            break;
#endif
#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
            case '9':
                BACKDOOR_MGR_Printf("Reg? 0x");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                reg_addr=(UI8_T)strtoul(buf, NULL, 16);
                if(UART_TO_I2C_ReadRegister(reg_addr, &reg_val)==TRUE)
                    BACKDOOR_MGR_Printf("Val=0x%02x\r\n", reg_val);
                else
                    BACKDOOR_MGR_Printf("Read reg 0x%02x failed\r\n", reg_addr);
                break;
            case 'a':
            case 'A':
                BACKDOOR_MGR_Printf("Reg? 0x");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                reg_addr=(UI8_T)strtoul(buf, NULL, 16);

                BACKDOOR_MGR_Printf("Write val? 0x");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                reg_val=(UI8_T)strtoul(buf, NULL, 16);

                if(UART_TO_I2C_WriteRegister(reg_addr, reg_val)==TRUE)
                    BACKDOOR_MGR_Printf("Write OK\r\n", reg_val);
                else
                    BACKDOOR_MGR_Printf("Write 0x%02x to reg 0x%02x failed\r\n", reg_val, reg_addr);
                break;

                break;
            case 'b':
            case 'B':
                BACKDOOR_MGR_Printf("Baudrate=");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                baudrate=strtoul(buf, NULL, 10);
                UART_TO_I2C_SetUARTBaudRate(baudrate);
                break;
            case 'c':
            case 'C':
                BACKDOOR_MGR_Printf("Baudrate=");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\n");
                baudrate=strtoul(buf, NULL, 10);
                if(UART_TO_I2C_SetBaudrate(baudrate)==TRUE)
                    BACKDOOR_MGR_Printf("Set baudrate ok.\r\n");
                else
                    BACKDOOR_MGR_Printf("Failed to set baudrate.\r\n");
                break;

            case 'd':
            case 'D':
                BACKDOOR_MGR_Printf("Read Len(Max=20)=");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
                BACKDOOR_MGR_Printf("\r\n");
                reg_addr=strtoul(buf, NULL, 10);
                if(reg_addr>20)
                    reg_addr=20;

                if(UART_TO_I2C_ReadFromUART(&reg_addr, (UI8_T*)buf)==TRUE)
                {
                    for(idx=0; idx<reg_addr; idx++)
                        BACKDOOR_MGR_Printf("[0x%2X] ", (UI8_T)(buf[idx]));

                    BACKDOOR_MGR_Printf("\r\n");
                }
                else
                    BACKDOOR_MGR_Printf("Nothing.\r\n");
                break;
#endif /* end of #if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE) */
        }
    }   /*  end of while    */
}   /* SYSDRV_I2CBackDoor_Menu */

static void SYSDRV_I2CBackdoor_Shell(void)
{
    static char buf[80];
    const char *promptstr_p = "i2c>";

    BACKDOOR_LIB_SHELL_T shell;
    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    memset(&shell, 0, sizeof(shell));
    shell.buf = buf;
    shell.buf_size = sizeof(buf);
    shell.prompt = promptstr_p;

    BACKDOOR_LIB_CliShell(&shell,
                          sysdrv_backdoor_i2c_shell_cmd,
                          sizeof(sysdrv_backdoor_i2c_shell_cmd)/sizeof(*sysdrv_backdoor_i2c_shell_cmd));
}

/* return value: 0     -> Success
 *               not 0 -> Error
 */
static int SYSDRV_I2CBackdoorCmd(const BACKDOOR_LIB_SHELL_CMD_T *cmd, int argc, char *argv[])
{
    UI32_T offset;
    UI8_T  data[255];
    UI8_T  i2c_addr, i;
    UI32_T data_len;
    BOOL_T result;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    switch((uintptr_t)(cmd->cookie))
    {
        case SYSDRV_BACKDOOR_I2CSHELL_CMD_R:
            if(argc<2)
                return 1;

            i2c_addr = (UI8_T)  strtoul(argv[0], NULL, 0);
            offset   = (UI32_T) strtoul(argv[1], NULL, 0);
            if(argc>=3)
                data_len = (UI32_T) strtoul(argv[2], NULL, 0);
            else
                data_len = 1;

            if(data_len > ((UI32_T)sizeof(data)))
            {
                BACKDOOR_MGR_Printf("Maximum read length is %lu\r\n", (unsigned long)sizeof(data));
                return 1;
            }

            result=I2CDRV_TwsiDataReadWithBusIdx(sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, I2C_7BIT_ACCESS_MODE, TRUE, offset, FALSE, (UI8_T)data_len, &(data[0]));

            BACKDOOR_MGR_Printf("i2c read: bus %u addr=0x%X offset=0x%lX len=%lu\r\n",
                sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, (unsigned long)offset, (unsigned long)data_len);
            if(result==TRUE)
            {
                BACKDOOR_MGR_DumpHex("", (int)data_len, data);
            }
            else
            {
                BACKDOOR_MGR_Printf("i2c read error.\r\n");
            }
            break;

        case SYSDRV_BACKDOOR_I2CSHELL_CMD_R2:
            if(argc<1)
                return 1;

            i2c_addr = (UI8_T)  strtoul(argv[0], NULL, 0);

            if(argc>=2)
                data_len = (UI32_T) strtoul(argv[1], NULL, 0);
            else
                data_len = 1;


            if(data_len > (UI32_T)(sizeof(data)))
            {
                BACKDOOR_MGR_Printf("maximum read length is %lu\r\n", (unsigned long)sizeof(data));
                return 1;
            }

            result=I2CDRV_TwsiDataReadWithBusIdx(sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, I2C_7BIT_ACCESS_MODE, FALSE, 0, FALSE, (UI8_T)data_len, &(data[0]));

            BACKDOOR_MGR_Printf("i2c read: bus %u addr=0x%X len=%lu\r\n",
                sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, (unsigned long)data_len);
            if(result==TRUE)
            {
                BACKDOOR_MGR_DumpHex("", data_len, data);
            }
            else
            {
                BACKDOOR_MGR_Printf("i2c read error.\r\n");
            }
            break;

        case SYSDRV_BACKDOOR_I2CSHELL_CMD_W:
            if(argc<3)
                return 1;

            i2c_addr = (UI8_T)  strtoul(argv[0], NULL, 0);
            offset   = (UI32_T) strtoul(argv[1], NULL, 0);

            for (i = 2, data_len = 0; i < argc && data_len < sizeof(data); i++)
                data[data_len++] = strtoul(argv[i], NULL, 0);

            BACKDOOR_MGR_Printf("i2c write: bus %u addr=0x%X offset=0x%lX len=%lu\r\n",
                sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, (unsigned long)offset, (unsigned long)data_len);

            result=I2CDRV_TwsiDataWriteWithBusIdx(sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, I2C_7BIT_ACCESS_MODE, TRUE, offset, FALSE, &(data[0]), (UI8_T)data_len);

            if(result==TRUE)
            {
                BACKDOOR_MGR_Printf("I2C write OK.\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("I2C write error.\r\n");
            }
            break;

        case SYSDRV_BACKDOOR_I2CSHELL_CMD_W2:
            if(argc<2)
                return 1;

            i2c_addr = (UI8_T)  strtoul(argv[0], NULL, 0);

            for (i = 1, data_len = 0; i < argc && data_len < ((UI32_T)sizeof(data)); i++)
                data[data_len++] = strtoul(argv[i], NULL, 0);

            BACKDOOR_MGR_Printf("i2c write: bus %u addr=0x%X len=%lu\r\n",
                sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, (unsigned long)data_len);

            result=I2CDRV_TwsiDataWriteWithBusIdx(sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index,
                i2c_addr, I2C_7BIT_ACCESS_MODE, FALSE, 0, FALSE, &(data[0]), (UI8_T)data_len);

            if(result==TRUE)
            {
                BACKDOOR_MGR_Printf("I2C write OK.\r\n");
            }
            else
            {
                BACKDOOR_MGR_Printf("I2C write error.\r\n");
            }
            break;

        case SYSDRV_BACKDOOR_I2CSHELL_CMD_BUS:
            if(argc==0)
            {
                BACKDOOR_MGR_Printf("Current active i2c bus: %u\r\n",
                    sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index);
            }
            else
            {
                sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index=(UI8_T) strtoul(argv[0], NULL, 0);
            }
            break;
        default:
            BACKDOOR_MGR_Printf("Unknown cmd(%lu)\r\n", (unsigned long)(uintptr_t)(cmd->cookie));
            return 1;
            break;
    }

    return 0;
}

extern unsigned long g_dbg_buf;

static void SYSDRV_BackDoor_ShowlogMenu (void)
{
    UC_MGR_KERNEL_DBG_BUF_T *dbg_buf;
    unsigned int i;
    unsigned long start;
    unsigned long end;
    unsigned long size;
    char *buf;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    dbg_buf = (UC_MGR_KERNEL_DBG_BUF_T *)g_dbg_buf;
    start = dbg_buf->start;
    end = dbg_buf->end;
    size = dbg_buf->size;
    buf = (char *)(g_dbg_buf + sizeof(UC_MGR_KERNEL_DBG_BUF_T));

    BACKDOOR_MGR_Printf("----------------------------------------\r\n");

    i = end;
    while (i < size)
    {
        if (buf[i] == '\0')
        {
            i = start;
            continue;
        }

        BACKDOOR_MGR_Printf("%c", buf[i]);
        i++;
        if (i == end)
            break;
    }

    BACKDOOR_MGR_Printf("\r\n----------------------------------------\r\n");

}

static void SYSDRV_BackDoor_GetSfpInfo(void)
{
    char buf[40];
    UI32_T dev_idx;
    UI16_T offset;
    UI8_T  *gbic_info_p;
    UI8_T  size;
    BOOL_T ret;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    BACKDOOR_MGR_Printf("dev index(1 based)=");
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
    dev_idx = strtoul(buf, NULL, 10);

    BACKDOOR_MGR_Printf("\r\noffset=");
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
    offset = strtoul(buf, NULL, 10);

    BACKDOOR_MGR_Printf("\r\nsize=");
    BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
    size = strtoul(buf, NULL, 10);

    gbic_info_p = malloc(size);
    if(gbic_info_p==NULL)
    {
        BACKDOOR_MGR_Printf("malloc failed. operation abort.\r\n");
        return;
    }

    ret = SYSDRV_GetSfpInfo(dev_idx, offset, size, gbic_info_p);

    if(ret==FALSE)
    {
        BACKDOOR_MGR_Printf("SYSDRV_GetSfpInfo failed.\r\n");
        free(gbic_info_p);
        return;
    }

    BACKDOOR_MGR_DumpHex("Sfp Info", size, gbic_info_p);

    free(gbic_info_p);
}

#if defined(SYS_HWCFG_EPLD_PAGE_SIZE)
/* anzhen.zheng, 2/2/2008 */
static void SYSDRV_BackDoor_EpldSubMenu (void)
{
    UI8_T   ch = 0;
	UI8_T	string[20] = {0};
	UI8_T	output[3 * SYS_HWCFG_EPLD_PAGE_SIZE] = {0};
	UI8_T   page = 0, data_type_len;
    union{
        UI8_T  u8;
        UI16_T u16;
        UI32_T u32;
    } data;
	UI32_T 	physical_address;
	UI32_T	value;
    BOOL_T  eof = FALSE;

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    /*  BODY
     */
    while (! eof)
    {
        BACKDOOR_MGR_Printf("\r\n==========SYSDRV BackDoor EPLD Access Sub Menu========\r\n");
        BACKDOOR_MGR_Printf("\r\n 0. Exit\r\n");
        BACKDOOR_MGR_Printf(" 1. EPLD read\r\n");
        BACKDOOR_MGR_Printf(" 2. EPLD write\r\n");
        BACKDOOR_MGR_Printf(" 3. EPLD read page\r\n");
        BACKDOOR_MGR_Printf("=================================================\r\n");
        BACKDOOR_MGR_Printf("     select =");
        ch = BACKDOOR_MGR_GetChar();
#if 0
        if ((ch < 0x30)||(ch>0x39))
            continue;
        ch -= 0x30;
#endif

        BACKDOOR_MGR_Printf ("%c\r\n",ch);
        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;
            case '1':
            {
                BACKDOOR_MGR_Printf(" Data type(1: UI8_T, 2:UI16_T, 4:UI32_T): ");
                memset(string, 0, sizeof(string));
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                BACKDOOR_MGR_Printf("\r\n");
                data_type_len = (UI8_T)(strtoul((const char*)&string[0], NULL, 0));
                if(data_type_len!=1 && data_type_len!=2 && data_type_len!=4)
                {
                    BACKDOOR_MGR_Printf("Invalid data type.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf(" Please input EPLD address:\r\n");
                BACKDOOR_MGR_Printf(" Example:address: 0xea010001\r\n\r\n");
                BACKDOOR_MGR_Printf(" address: ");

                memset(string, 0, sizeof(string));
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                BACKDOOR_MGR_Printf("\r\n");

                physical_address = strtoul((const char *)&string[0], NULL, 0);
                if(!SYSDRV_IsEpldAddressValid(physical_address))
                {
                    BACKDOOR_MGR_Printf(" Invalid EPLD address!\r\n");
                    break;
                }

                switch(data_type_len)
                {
                    case 1:
                    default:
                        if(PHYSICAL_ADDR_ACCESS_Read(physical_address, data_type_len, 1, (UI8_T *)&(data.u8))==TRUE)
                            BACKDOOR_MGR_Printf(" value = 0x%02x\r\n", data.u8);
                        else
                            BACKDOOR_MGR_Printf("Failed to read the physical address.\r\n");
                        break;
                    case 2:
                        if(PHYSICAL_ADDR_ACCESS_Read(physical_address, data_type_len, 1, (UI8_T *)&(data.u16))==TRUE)
                            BACKDOOR_MGR_Printf(" value = 0x%04x\r\n", data.u16);
                        else
                            BACKDOOR_MGR_Printf("Failed to read the physical address.\r\n");
                        break;
                    case 4:
                        if(PHYSICAL_ADDR_ACCESS_Read(physical_address, data_type_len, 1, (UI8_T *)&(data.u32))==TRUE)
                            BACKDOOR_MGR_Printf(" value = 0x%08lx\r\n", data.u32);
                        else
                            BACKDOOR_MGR_Printf("Failed to read the physical address.\r\n");
                        break;
                }

                break;
            }
            case '2':
            {
                BACKDOOR_MGR_Printf(" Data type(1: UI8_T, 2:UI16_T, 4:UI32_T): ");
                memset(string, 0, sizeof(string));
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                BACKDOOR_MGR_Printf("\r\n");
                data_type_len = (UI8_T)(strtoul((const char*)&string[0], NULL, 0));
                if(data_type_len!=1 && data_type_len!=2 && data_type_len!=4)
                {
                    BACKDOOR_MGR_Printf("Invalid data type.\r\n");
                    break;
                }

                BACKDOOR_MGR_Printf(" Please input EPLD address and new value:\r\n");
                BACKDOOR_MGR_Printf(" Example:address value: 0xea010001 0x01\r\n\r\n");
                BACKDOOR_MGR_Printf(" address value: ");

                memset(string, 0, sizeof(string));
                BACKDOOR_MGR_RequestKeyIn(string, sizeof(string)-1);
                BACKDOOR_MGR_Printf("\r\n");

                physical_address = strtoul((const char *)&string[0], NULL, 0);
                if(!SYSDRV_IsEpldAddressValid(physical_address))
                {
                    BACKDOOR_MGR_Printf(" Invalid EPLD address!\r\n");
                    break;
                }

                value = strtoul((const char *)&string[11], NULL, 0);
                switch (data_type_len)
                {
                    case 1:
                    default:
                        data.u8 = (UI8_T)value;
                    if(PHYSICAL_ADDR_ACCESS_Write(physical_address, data_type_len, 1, &(data.u8))==FALSE)
                        BACKDOOR_MGR_Printf("Failed to write the physical address.\r\n");
                        break;
                    case 2:
                        data.u16 = (UI16_T)value;
                        if(PHYSICAL_ADDR_ACCESS_Write(physical_address, data_type_len, 1, (UI8_T*)&(data.u16))==FALSE)
                            BACKDOOR_MGR_Printf("Failed to write the physical address.\r\n");
                        break;
                    case 4:
                        data.u32 = (UI32_T)value;
                        if(PHYSICAL_ADDR_ACCESS_Write(physical_address, data_type_len, 1, (UI8_T*)&(data.u32))==FALSE)
                            BACKDOOR_MGR_Printf("Failed to write the physical address.\r\n");

                        break;
                }

                break;
            }

            case '3':
            {
                if (SYS_HWCFG_EPLD_MAX_PAGE == 5)
                    BACKDOOR_MGR_Printf(" Please input EPLD page(1, 2, 3, 4, 5):\r\n");
                else if (SYS_HWCFG_EPLD_MAX_PAGE == 1)
                    BACKDOOR_MGR_Printf(" Please input EPLD page(1):\r\n");

                BACKDOOR_MGR_Printf(" Example:page: 1\r\n\r\n");
                BACKDOOR_MGR_Printf(" page:");
                ch = BACKDOOR_MGR_GetChar();
                page = ch - '0';
                BACKDOOR_MGR_Printf(" %d\r\n", page);

                if (page < SYS_HWCFG_EPLD_MIN_PAGE || page > SYS_HWCFG_EPLD_MAX_PAGE)
                {
                    BACKDOOR_MGR_Printf(" Invalid page!\r\n");
                    break;
                }

                if (SYSDRV_ReadEpldPage(page, &output[0]))
                {
                    BACKDOOR_MGR_Printf(" value = 0x %s\r\n", output);
                }

                break;
            }
            default :
                break;
        }
    }   /*  end of while    */
}

/* FUNCTION NAME: SYSDRV_IsEpldAddressValid
 * PURPOSE  : This function is used by backdoor, to get a valid EPLD address
 * INPUT    : physical_address  -- physical address of EPLD
 * OUTPUT   :
 * RETURN   : TURE/FALSE        -- success / fail
 * NOTES    :
 */
static BOOL_T SYSDRV_IsEpldAddressValid(UI32_T physical_address)
{
    int i;
    BOOL_T ret = FALSE;
    PHYSICAL_EPLD_ADDR_T epldaddr[SYS_HWCFG_EPLD_MAX_PAGE];

    /* Need not to add SYSDRV_STATIC_LIB_FUN_PROLOGUE in this function
     */

    SYS_HWCFG_Getepldaddr(epldaddr);

    for(i = 0; i < SYS_HWCFG_EPLD_MAX_PAGE; i++)
    {
        if ((physical_address >= epldaddr[i].start_addr) && (physical_address <= epldaddr[i].end_addr))
        {
            ret = TRUE;
            break;
        }
    }

    return ret;
}

#endif /* end of #if defined(SYS_HWCFG_EPLD_PAGE_SIZE) */

#endif /* end of #ifdef BACKDOOR_OPEN */

