/*-----------------------------------------------------------------------------
 * Module Name: SYSDRV.C
 *-----------------------------------------------------------------------------
 * PURPOSE: This module provides the functions of reload for system management
 *          to control local and remote unit via ISC.
 *-----------------------------------------------------------------------------
 * NOTES:
 * 1. The main function of this file is to provide the function of
 *    reload for local unit and remote unit.
 * 2. Only one task access OM, protected mechanism is not used for performance consideration
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
 *    11/21/2002 - Benson Hsu,    Created
 *     3/12/2003 - Charles Cheng, New mechanism of fan detection
 *     3/20/2003 - Charles Cheng. New mechanism of power detection
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "sys_time.h"
#include "leaf_sys.h"
#include "sysfun.h"
#include "uc_mgr.h"
#include "l_ipcmem.h"
#include "l_mm.h"
#include "sys_hwcfg.h"
#include "sys_hwcfg_common.h"
#include "sys_bld.h"
#include "leaf_es3626a.h"

#include "backdoor_mgr.h" /* anzhen.zheng, 2/2/2008 */
#include "backdoor_lib.h"
#include "phyaddr_access.h" /* anzhen.zheng, 2/19/2008 */
#include "sysrsc_mgr.h"

#if (SYS_HWCFG_UART_TO_I2C_CHIP_TYPE != SYS_HWCFG_UART_TO_I2C_CHIP_NONE)
#include "uart_to_i2c.h"
#endif

#if (SYS_CPNT_CFGDB == TRUE)
//#include "cfgdb_mgr.h"
#endif

#include "fs.h"
#include "fs_type.h"
#if (SYS_CPNT_ISCDRV == TRUE)
#include "isc.h"
//#include "iuc.h"
#endif
#include "sysdrv.h"
#include "sysdrv_util.h"
#include "sysdrv_private.h"
/*#include "cacheLib.h"*/   /* This is defined in VxWorks   */

#include "swdrv.h"
/*#include "i2c_export.h"*/

#include "i2cdrv.h"

#include "sys_callback_mgr.h"
#include "leddrv.h"

#include "stktplg_pom.h"
#include "i2c.h"

#include "stktplg_board.h"

#if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
#include "cplddrv.h"
#define UPLOAD_CPLD_SCRIPT_FILE "/etc/upload_cpld.sh"
#endif

#if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE)
#include "onlpdrv_sfp.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define BACKDOOR_OPEN

/* older projects uses i2c bus index 0 to access SFP EEPROM
 * if SYS_HWCFG_I2C_SFP_EEPROM_BUS_IDX is not defined
 * define it as 0
 */
#if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API!=TRUE) && !defined(SYS_HWCFG_I2C_SFP_EEPROM_BUS_IDX)
#define SYS_HWCFG_I2C_SFP_EEPROM_BUS_IDX 0
#endif

/* MACRO FUNCTION DECLARATIONS
 */
#if (SYS_HWCFG_SUPPORT_DIFFERENT_THERMAL_NUM == FALSE)
#define STKTPLG_BOARD_GetThermalNumber() ({int __ret=SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; __ret;})
#endif

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

static UI8_T nbr_of_fan = SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT;

#ifndef SYS_HWCFG_DEFAULT_FAN_SPEED_MODE
#define SYS_HWCFG_DEFAULT_FAN_SPEED_MODE SYSDRV_FAN_MID_SPEED
#endif

#if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_WIN83782)
#define SYSDRV_FAN_CHIP_ID  SYS_HWCFG_MONITOR_CHIP_ID
#elif  (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7475)/* #if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_WIN83782) */
#define SYSDRV_FAN_CHIP_ID		0x30
#elif  (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_ADT7470)/*End of SYS_HWCFG_FAN_ADT7475*/
#define SYSDRV_FAN_CHIP_ID    0x40
#else /*End of SYS_HWCFG_FAN_ADT7470*/
#define SYSDRV_FAN_CHIP_ID  0
#endif /* end of #if (SYS_HWCFG_FAN_CONTROLLER_TYPE == SYS_HWCFG_FAN_WIN83782) */

#define SYSDRV_FAN_SPEED_IN_TRANSITION_COUNTER_DEFAULT_RESET_VALUE 3

#endif /*End of SYS_CPNT_STKTPLG_FAN_DETECT*/

#if (SYS_CPNT_POWER_DETECT == TRUE)
    #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD!=TRUE)
    /* assumes that the default power status register access method is
     * SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR
     */
    #ifndef SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD
    #define SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR
    #endif

    /* assumes that all of the power status use the same register address to access
     * by default
     */
    #ifndef SYS_HWCFG_POWER_STATUS_ADDR_ARRAY
    #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT==1)
    #define SYS_HWCFG_POWER_STATUS_ADDR_ARRAY {SYS_HWCFG_POWER_STATUS_ADDR}
    #elif (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT==2)
    #define SYS_HWCFG_POWER_STATUS_ADDR_ARRAY {SYS_HWCFG_POWER_STATUS_ADDR, SYS_HWCFG_POWER_STATUS_ADDR}
    #else
    #error "Inappropriate value of SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT"
    #endif
    #endif
    #endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD!=TRUE) */

    #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
    /* assumes that all of the power type use the same register address to access
     * by default
     */
    #ifndef SYS_HWCFG_POWER_TYPE_ADDR_ARRAY
    #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT==1)
    #define SYS_HWCFG_POWER_TYPE_ADDR_ARRAY {SYS_HWCFG_POWER_TYPE_ADDR}
    #elif (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT==2)
    #define SYS_HWCFG_POWER_TYPE_ADDR_ARRAY {SYS_HWCFG_POWER_TYPE_ADDR, SYS_HWCFG_POWER_TYPE_ADDR}
    #else
    #error "Inappropriate value of SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT"
    #endif
    #endif /* #ifndef SYS_HWCFG_POWER_TYPE_ADDR_ARRAY */

    #endif /* #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */

#endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

/* semaphore for GPIO
 */
static UI32_T sysdrv_sem_id;

#define SYSDRV_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(sysdrv_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SYSDRV_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(sysdrv_sem_id)

#if (SYS_CPNT_STACKING == TRUE)
#define SYSDRV_MASTER_UNIT_ID     1
#define SYSDRV_TIME_OUT           300
#define SYSDRV_TRY_TIMES          3
#define SYSDRV_POOL_ID_ISC_SEND   0
#define SYSDRV_POOL_ID_ISC_REPLY  1

/* Definitions of packet structure for communications between
 * local unit and remote unit
 */
typedef struct SYSDRV_PktHdr_S
{
    UI8_T      ServiceID;     /* service ID */

    union
    {
        struct
        {
            UI32_T unit;
            UI32_T fan;
            UI32_T status;
            UI32_T speed;
        }__attribute__((packed, aligned(1)))fan_status_changed;

        struct
        {
            UI32_T unit;
            UI32_T power;
            UI32_T status;
            UI32_T reversed;
        }__attribute__((packed, aligned(1)))power_status_changed;

        struct
        {
            UI32_T unit;
            UI32_T power;
            UI32_T type;
            UI32_T reversed;
        }__attribute__((packed, aligned(1)))power_type_changed;

        struct
        {
            UI32_T unit;
            UI32_T thermal;
            I32_T status;
            UI32_T reversed;
        }__attribute__((packed, aligned(1)))thermal_status_changed;

        struct
        {
            UI32_T unit;
            UI32_T xenpak_type;
        }__attribute__((packed, aligned(1)))xenpak_status_changed;

        struct
        {
            UI32_T unit;
            UI32_T status;
        }__attribute__((packed, aligned(1)))alarm_input_status_changed;

        struct
        {
            UI32_T unit;
            UI32_T status;
        }__attribute__((packed, aligned(1)))alarm_output_status_changed;

    }__attribute__((packed, aligned(1)))info;

}__attribute__((packed, aligned(1)))SYSDRV_PktHdr_T;

/* Type definition for function table
 */
typedef BOOL_T (*SYSDRV_ServiceFunc_t) (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);

/* Service ID
 */
enum SYSDRV_SERVICE_ID_E
{
    SYSDRV_SERVICE_ID_COLD_START_SYSTEM,         /*master to slave*/    /* restart system to loader                 */
    SYSDRV_SERVICE_ID_RELOAD_SYSTEM,             /*master to slave*/    /* restart system to loader with cold start */
    SYSDRV_SERVICE_ID_WARM_START_SYSTEM,         /*master to slave*/    /* restart system to loader with warm start */
    SYSDRV_SERVICE_ID_PROVISION_COMPLETE,        /*master to slave*/
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    SYSDRV_SERVICE_ID_ENABLEFANSPEEDFORCEFULL,   /*master to slave*/    /*SYSDRV_SlaveSetFanSpeedForceFull*/
    SYSDRV_SERVICE_ID_DISABLEFANSPEEDFORCEFULL,  /*master to slave*/    /*SYSDRV_SlaveSetFanSpeedForceFull*/
#endif
    SYSDRV_SERVICE_ID_NOTIFY_FAN_STATUS_CHANGED, /*slave to master*/
    SYSDRV_SERVICE_ID_NOTIFY_POWER_STATUS_CHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_THERMAL_STATUS_CHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_THERMAL_TEMPERATURE_CHANGED_OBSOLETED,
    SYSDRV_SERVICE_ID_NOTIFY_FAN_SPEED_CHANGED,   /*slave to master*/
    SYSDRV_SERVICE_ID_NOTIFY_XENPAKSTATECHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_ALARM_INPUT_STATUS_CHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_MAJOR_ALARM_OUTPUT_STATUS_CHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_MINOR_ALARM_OUTPUT_STATUS_CHANGED,
    SYSDRV_SERVICE_ID_NOTIFY_POWER_TYPE_CHANGED,
    SYSDRV_SERVICE_ID_MAX_NUM_OF_SERVICE    /*the max number of service id*/
};

static BOOL_T SYSDRV_SendRequestToSlave(UI8_T service_id);

#endif /*(SYS_CPNT_STACKING == TRUE)*/

/* DATA TYPE DECLARATIONS
 */
#if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE)
typedef enum GBIC_INFO_TYPE_E
{
    GBIC_INFO_TYPE_BASIC = ONLPDRV_GBIC_INFO_TYPE_BASIC,
    GBIC_INFO_TYPE_DDM = ONLPDRV_GBIC_INFO_TYPE_DDM,
} GBIC_INFO_TYPE_T;

typedef enum GBIC_ACCESS_TYPE_E
{
    GBIC_ACCESS_TYPE_READ = ONLPDRV_SFP_GBIC_ACCESS_TYPE_READ,
    GBIC_ACCESS_TYPE_WRITE = ONLPDRV_SFP_GBIC_ACCESS_TYPE_WRITE,
} GBIC_ACCESS_TYPE_T;
#else /* #if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE) */
typedef enum GBIC_INFO_TYPE_E
{
    GBIC_INFO_TYPE_BASIC = 0,
    GBIC_INFO_TYPE_DDM
} GBIC_INFO_TYPE_T;

typedef enum GBIC_ACCESS_TYPE_E
{
    GBIC_ACCESS_TYPE_READ = 0,
    GBIC_ACCESS_TYPE_WRITE
} GBIC_ACCESS_TYPE_T;
#endif /* end of #if (SYS_CPNT_SYSDRV_USE_ONLP==TRUE) */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) && (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC != TRUE) && (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_BID != TRUE)
static BOOL_T SYSDRV_DetectFanControllerType(UI32_T *fan_type);
#endif

/* ISC callback service routine, called by SYSDRV_ProtocolReceiver_CallBack()
 */
#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SYSDRV_FromIscColdStartSystem             (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
static BOOL_T SYSDRV_FromIscReloadSystem                (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
static BOOL_T SYSDRV_FromIscWarmStartSystem             (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
static BOOL_T SYSDRV_FromIscProvisionComplete           (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
static BOOL_T SYSDRV_FromIscEnableFanSpeedForceFull     (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
static BOOL_T SYSDRV_FromIscDisableFanSpeedForceFull    (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*master -> slave*/
#endif
static BOOL_T SYSDRV_FromIscNotify_FanStatusChanged     (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_FanSpeedChanged      (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_PowerStatusChanged   (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_ThermalStatusChanged (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p); /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_ThermalTemperatureChanged_Obsoleted(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);
static BOOL_T SYSDRV_FromIscNotify_XenpakStatusChanged  (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);
static BOOL_T SYSDRV_FromIscNotify_AlarmInputStatusChanged      (ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);  /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_MajorAlarmOutputStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);  /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_MinorAlarmOutputStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);  /*slave -> master*/
static BOOL_T SYSDRV_FromIscNotify_PowerTypeChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p);               /*slave -> master*/
#endif

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static BOOL_T SYSDRV_GetPowerPresentStatus(UI32_T power_index, BOOL_T *is_present_p);
#endif

#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SYSDRV_SendRequestToSlave(UI8_T service_id);
#endif

static BOOL_T SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_T gbic_info_type, UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p, GBIC_ACCESS_TYPE_T gbic_access_type);
static void SYSDRV_SetDatabaseToDefault(void);

/* LOCAL VARIABLE DECLARATIONS
 */

static SYSDRV_Shmem_Data_T *sysdrv_shmem_data_p;

#if (SYS_CPNT_STACKING == TRUE)
/* Function table for receiving opcode
 */
static SYSDRV_ServiceFunc_t sysdrv_func_tab[] =
{
    SYSDRV_FromIscColdStartSystem,            /*master to slave*/ /*SYSDRV_SERVICE_ID_COLD_START_SYSTEM            */
    SYSDRV_FromIscReloadSystem,               /*master to slave*/ /*SYSDRV_SERVICE_ID_RELOAD_SYSTEM                */
    SYSDRV_FromIscWarmStartSystem,            /*master to slave*/ /*SYSDRV_SERVICE_ID_WARM_START_SYSTEM            */
    SYSDRV_FromIscProvisionComplete,          /*master to slave*/ /*SYSDRV_SERVICE_ID_PROVISION_COMPLETE           */
#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    SYSDRV_FromIscEnableFanSpeedForceFull,         /*master to slave*/
    SYSDRV_FromIscDisableFanSpeedForceFull,        /*master to slave*/
#endif
    SYSDRV_FromIscNotify_FanStatusChanged,    /*slave to master*/ /*SYSDRV_SERVICE_ID_NOTIFY_FAN_STATUS_CHANGED    */
    SYSDRV_FromIscNotify_PowerStatusChanged,  /*slave to master*/ /*SYSDRV_SERVICE_ID_NOTIFY_POWER_STATUS_CHANGED  */
    SYSDRV_FromIscNotify_ThermalStatusChanged,/*slave to master*/ /*SYSDRV_SERVICE_ID_NOTIFY_THERMAL_STATUS_CHANGED*/
    SYSDRV_FromIscNotify_ThermalTemperatureChanged_Obsoleted,/*slave to master*/ /*SYSDRV_SERVICE_ID_NOTIFY_THERMAL_TEMPERATURE_CHANGED_OBSOLETED*/
    SYSDRV_FromIscNotify_FanSpeedChanged,     /*slave to master*/ /*SYSDRV_SERVICE_ID_NOTIFY_FAN_SPEED_CHANGED     */
    SYSDRV_FromIscNotify_XenpakStatusChanged,                      /*SYSDRV_SERVICE_ID_NOTIFY_XENPAKSTATECHANGED    */
    SYSDRV_FromIscNotify_AlarmInputStatusChanged,                  /*SYSDRV_SERVICE_ID_NOTIFY_ALARM_INPUT_STATUS_CHANGED    */
    SYSDRV_FromIscNotify_MajorAlarmOutputStatusChanged,            /*SYSDRV_SERVICE_ID_NOTIFY_MAJOR_ALARM_OUTPUT_STATUS_CHANGED   */
    SYSDRV_FromIscNotify_MinorAlarmOutputStatusChanged,            /*SYSDRV_SERVICE_ID_NOTIFY_MINOR_ALARM_OUTPUT_STATUS_CHANGED   */
    SYSDRV_FromIscNotify_PowerTypeChanged,    /*slave to master*/  /*SYSDRV_SERVICE_ID_NOTIFY_POWER_TYPE_CHANGED */
};
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
    #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD!=TRUE)
static SYS_TYPE_VAddr_T power_status_addr[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
    #endif

    #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static SYS_TYPE_VAddr_T power_type_addr[SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT];
    #endif
#endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SYSDRV_Initiate_System_Resources
 *-----------------------------------------------------------------------------
 * PURPOSE: initializes all resources for SYSDRV
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T SYSDRV_InitiateSystemResources(void)
{
    UI32_T ret;

    sysdrv_shmem_data_p = (SYSDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYSDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(sysdrv_shmem_data_p);

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSDRV, &sysdrv_sem_id);
    if (ret != SYSFUN_OK)
    {
        printf("%s:%d: SYSFUN_GetSem fails. (%lu)\n", __FUNCTION__, __LINE__, (unsigned long)ret);
        return FALSE;
    }

    sysdrv_shmem_data_p->bd_debug_flag=0;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
        #if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
    if (STKTPLG_BOARD_GetFanSpeedModeSMInfo(&(sysdrv_shmem_data_p->fan_speed_mode_sm_info))==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)STKTPLG_BOARD_GetFanSpeedModeSMInfo failed\r\n", __FUNCTION__, __LINE__);
    }
        #endif
    #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */
#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

    SYSDRV_SetDatabaseToDefault();

#if 0
    /* Move the code snippet to SYSDRV_TASK_TaskMain().
     * because SYSDRV_FAN_CHIP_Init() and SYSDRV_THERMAL_CHIP_Init()
     * might call I2CDRV. However, I2CDRV in MARVELL SDK will use IPC message and handled in
     * driver process. It is unable to use I2CDRV in sysinit_proc.
     */
    #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    if (SYSDRV_FAN_CHIP_Init() != TRUE)
    {
        printf("\r\nFAN initial failed.");
        return FALSE;
    }
    #endif
    #if (SYS_CPNT_THERMAL_DETECT == TRUE)
    if (SYSDRV_THERMAL_CHIP_Init() != TRUE)
    {
        printf("\r\nThermal initial failed.");
        return FALSE;
    }
    #endif
#endif

    I2CDRV_InitiateSystemResources();

#if 0
    /* Move this code snipet to SYSDRV_TASK_TaskMain().
     * SYSDRV_DetectPeripheralInstall() will call to I2CDRV,
     * however, I2CDRV in MARVELL SDK will use IPC message and handled in
     * driver process. It is unable to use I2CDRV in sysinit_proc.
     */
    /*Detect peripheral*/
    SYSDRV_DetectPeripheralInstall();
#endif /* end of #if 0 */

#ifdef BACKDOOR_OPEN
    sysdrv_shmem_data_p->sysdrv_backdoor_i2c_shell_bus_index=0;
#endif

    return TRUE;

}  /* End of SYSDRV_Initiate_System_Resources() */

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
void SYSDRV_AttachSystemResources(void)
{
    UI32_T ret;

    sysdrv_shmem_data_p = (SYSDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYSDRV_SHMEM_SEGID);
    I2CDRV_AttachSystemResources();

    ret = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYSDRV, &sysdrv_sem_id);
    if (ret != SYSFUN_OK)
    {
        printf("%s:%d: SYSFUN_GetSem fails. (%lu)\n", __FUNCTION__, __LINE__, (unsigned long)ret);
        return;
    }

#if (SYS_CPNT_POWER_DETECT == TRUE)

#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD!=TRUE)
    {
        SYS_TYPE_PAddr_T power_status_phy_addrs[] = SYS_HWCFG_POWER_STATUS_ADDR_ARRAY;
        UI32_T index;

        if(SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT!=sizeof(power_status_phy_addrs)/sizeof(power_status_phy_addrs[0]))
        {
            printf("%s(%d)Inconsistent SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT(%d) value and array size(%u). Halt.\r\n",
                __FUNCTION__, __LINE__, SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT, sizeof(power_status_phy_addrs)/sizeof(power_status_phy_addrs[0]));

            while(TRUE);

            return;
        }

    #if (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR)

        #if defined(ECS4810_12MV2)
        {
            UC_MGR_Sys_Info_T sys_info;

            UC_MGR_InitiateProcessResources();

            if(UC_MGR_GetSysInfo(&sys_info)==FALSE)
            {
                printf("%s(%d)Failed to get sysinfo from UC.\r\n", __FUNCTION__, __LINE__);
                return FALSE;
            }
            if(sys_info.board_id!=1)
            {
                power_status_phy_addrs[0] = SYS_HWCFG_GPIO_IN+3;
                power_status_phy_addrs[1] = SYS_HWCFG_GPIO_IN+3;
            }
        }
        #endif /* end of #if defined(ECS4810_12MV2) */

        for(index=0; index<sizeof(power_status_phy_addrs)/sizeof(power_status_phy_addrs[0]); index++)
        {
            if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(power_status_phy_addrs[index], &power_status_addr[index]))
            {
                printf("\r\n%s:Failed to access SYS_HWCFG_POWER_STATUS_ADDR(0x%lX)", __FUNCTION__, (unsigned long)power_status_phy_addrs[index]);
                return;
            }
        }
    #elif (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_I2C)
        for(index=0; index<sizeof(power_status_phy_addrs)/sizeof(power_status_phy_addrs[0]); index++)
        {
            power_status_addr[index] = power_status_phy_addrs[index];
        }
    #else /* #if (SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD==SYS_HWCFG_REG_ACCESS_METHOD_PHYADDR) */
    #error "Unknown SYS_HWCFG_POWER_STATUS_REG_ACCESS_METHOD"
    #endif
    }
#endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD!=TRUE) */

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
    {
        SYS_TYPE_PAddr_T power_type_phy_addrs[] = SYS_HWCFG_POWER_TYPE_ADDR_ARRAY;
        UI32_T index;

        for(index=0; index<sizeof(power_type_phy_addrs)/sizeof(power_type_phy_addrs[0]); index++)
        {
            if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(power_type_phy_addrs[index], &power_type_addr[index]))
            {
                printf("%s:Failed to get virtual addr of physicall address 0x%08lx\r\n", __FUNCTION__, power_type_phy_addrs[index]);
                return;
            }
        }
    }
#endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */

#endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

    return;
}

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
void SYSDRV_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(sysdrv_shmem_data_p);

    sysdrv_shmem_data_p->is_transition_done = FALSE;
    SYSFUN_SendEvent (sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_ENTER_TRANSITION);

} /* End of SYSDRV_SetTransitionMode() */

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
void SYSDRV_EnterTransitionMode(void)
{

    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(sysdrv_shmem_data_p);
    SYSDRV_SetDatabaseToDefault();
} /* End of SYSDRV_EnterTransitionMode() */


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
void SYSDRV_EnterMasterMode(void)
{
    /* sysdrv_my_unit_id is used after provision complete.
     */
    if (FALSE == STKTPLG_POM_GetMyUnitID(&(sysdrv_shmem_data_p->sysdrv_my_unit_id)))
    {
        printf("\r\n[SYSDRV] Failed to get my unit ID\r\n");
    }

    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(sysdrv_shmem_data_p);

} /* End of SYSDRV_EnterMasterMode() */


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
void SYSDRV_EnterSlaveMode(void)
{
    /* sysdrv_my_unit_id is used after provision complete.
     */
    if (FALSE == STKTPLG_POM_GetMyUnitID(&(sysdrv_shmem_data_p->sysdrv_my_unit_id)))
    {
        printf("\r\n[SYSDRV] Failed to get my unit ID\r\n");
    }

    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(sysdrv_shmem_data_p);

} /* End of SYSDRV_EnterSlaveMode() */

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
/* FUNCTION NAME: SYSDRV_SetProvisionCompleteFanSpeed
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is to set fan speed when provision complete
 *-----------------------------------------------------------------------------
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static void SYSDRV_SetProvisionCompleteFanSpeed(void)
{

#if (SYS_CPNT_SYSDRV_USE_ONLP!=TRUE)
    if ( SYSDRV_GetFanSpeedForceFull() == FALSE )
    {
        SYSDRV_SetFanSpeedMode(SYS_HWCFG_DEFAULT_FAN_SPEED_MODE);
    }
#endif

}
#endif /* end of #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

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
void SYSDRV_ProvisionComplete(void)
{
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    SYSDRV_SetProvisionCompleteFanSpeed();
#endif

    sysdrv_shmem_data_p->sysdrv_is_provision_complete = TRUE;

#if (SYS_CPNT_STACKING == TRUE)
    /* In master mode, send provision complete information to all slave units
     */
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if (FALSE == SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_PROVISION_COMPLETE))
         {
              printf("\r\nSYSDRV_ProvisionComplete: unable to send request to slave unit!!");
              return ;
         }
    }
#endif /* SYS_CPNT_STACKING */

    return ;
}

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)

/* FUNCTION NAME: SYSDRV_GetFanNum
 *-----------------------------------------------------------------------------
 * PURPOSE: Get actually FAN number for a specific unit
 *-----------------------------------------------------------------------------
 * INPUT    : unit->unit ID
 * OUTPUT   : fanNum->store actually FAN number
 * RETURN   : TRUE  - At least one fan installed on the unit
 *            FALSE - No fan is installed on the unit
 *-----------------------------------------------------------------------------
 * NOTES:
 */
BOOL_T SYSDRV_GetFanNum(UI32_T unit,UI32_T *fanNum)
{
    *fanNum = STKTPLG_BOARD_GetFanNumber();
    if(*fanNum == 0)
        return FALSE;

    return TRUE;
}

/* FUNCTION NAME: SYSDRV_GetFanFailNum
 * PURPOSE: This function will return the number of failed fan.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  The number of failed fan.
 * NOTES:   None.
 */
UI32_T SYSDRV_GetFanFailNum(void)
{
    UI32_T fail_count;
    UI8_T i;

    for(i=0,fail_count=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
    {
        if(sysdrv_shmem_data_p->sysdrv_fan_status[i]!=VAL_switchFanStatus_ok)
            fail_count++;
    }
    return fail_count;
}

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
UI32_T SYSDRV_GetFanFullSpeedVal(void)
{
    /* When SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE is TRUE,
     * SYS_CPNT_SYSDRV_FAN_MULTIPLE_SPEED_LEVEL must be TRUE. So only
     * check SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE here.
     */
#if (SYS_CPNT_SYSDRV_DYNAMIC_FAN_SPEED_MODE_STATE_MACHINE==TRUE)
    SYSDRV_FanSpeedModeStateMachine_Info_T sm_info;
    if (STKTPLG_BOARD_GetFanSpeedModeSMInfo(&sm_info)==TRUE)
    {
        return sm_info.number_of_speed_level;
    }

    /* Failed to get number of speed level from STKTPLG_BOARD,
     * use 2 as a fail-safe value (Because there is at least 2 speed level, it is safe to use 2 as fan full speed level value)
     */
    BACKDOOR_MGR_Printf("%s:STKTPLG_BOARD_GetFanSpeedModeSMInfo error.\r\n", __FUNCTION__);
    return 2;
#else
    return SYSDRV_FAN_FULL_SPEED;
#endif
}

#endif /*End of SYS_CPNT_STKTPLG_FAN_DETECT*/

/* FUNCTION NAME: SYSDRV_Notify_AlarmInputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when alarm input status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       ---
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_AlarmInputStatusChanged(UI32_T unit, UI32_T status)
{
    UI16_T               ret_val;
    UI8_T                master_unit_id;

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif

        SYS_CALLBACK_MGR_SYSDRV_AlarmInputStatusChanged_Callback(SYS_MODULE_SYSDRV, unit, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*Step 2: If slave mode, call ISC remote call to master unit*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_ALARM_INPUT_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan status changed to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_ALARM_INPUT_STATUS_CHANGED;
        isc_buf_p->info.alarm_input_status_changed.unit  = unit;
        isc_buf_p->info.alarm_input_status_changed.status = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

/* FUNCTION NAME: SYSDRV_Notify_MajorAlarmOutputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when major alarm output status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       --- SYSDRV_ALARMMAJORSTATUS_XXX
 *                             (e.g. SYSDRV_ALARMMAJORSTATUS_POWER_STATUS_CHANGED_MASK)
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_MajorAlarmOutputStatusChanged(UI32_T unit, UI32_T status)
{
    UI16_T               ret_val;
    UI8_T                master_unit_id;

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif
        if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("%s(%d):unit=%lu,status=0x%08lx\r\n", __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)status);

        SYS_CALLBACK_MGR_SYSDRV_MajorAlarmOutputStatusChanged_Callback(SYS_MODULE_SYSDRV, unit, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*Step 2: If slave mode, call ISC remote call to master unit*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_MAJOR_ALARM_OUTPUT_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan status changed to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_MAJOR_ALARM_OUTPUT_STATUS_CHANGED;
        isc_buf_p->info.alarm_output_status_changed.unit  = unit;
        isc_buf_p->info.alarm_output_status_changed.status = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

/* FUNCTION NAME: SYSDRV_Notify_MinorAlarmOutputStatusChanged
 *-----------------------------------------------------------------------------
 * PURPOSE: This function will callback when minor alarm output status is changed
 *-----------------------------------------------------------------------------
 * INPUT    : unit         --- Which unit.
 *            status       --- SYSDRV_ALARMMINORSTATUS_XXX
 *                             (e.g. SYSDRV_ALARMMINORSTATUS_OVERHEAT_MASK)
 * OUTPUT   : none
 * RETURN   : none
 *-----------------------------------------------------------------------------
 * NOTES:
 */
void SYSDRV_Notify_MinorAlarmOutputStatusChanged(UI32_T unit, UI32_T status)
{
    UI16_T               ret_val;
    UI8_T                master_unit_id;

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif
        if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            BACKDOOR_MGR_Printf("%s(%d):unit=%lu,status=0x%08lx\r\n", __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)status);

        SYS_CALLBACK_MGR_SYSDRV_MinorAlarmOutputStatusChanged_Callback(SYS_MODULE_SYSDRV, unit, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*Step 2: If slave mode, call ISC remote call to master unit*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_MINOR_ALARM_OUTPUT_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan status changed to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_MINOR_ALARM_OUTPUT_STATUS_CHANGED;
        isc_buf_p->info.alarm_output_status_changed.unit  = unit;
        isc_buf_p->info.alarm_output_status_changed.status = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

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
void SYSDRV_Notify_PowerStatusChanged(UI32_T unit, UI32_T power, UI32_T status)
{
    UI32_T               my_unit_id;
    UI16_T               ret_val;
    UI8_T                master_unit_id;

    /* Step 1: Call LedDrv to light/off the LED of Power or RPU if unit = my_unit_id*/
    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    if (unit == my_unit_id)
    {
        LEDDRV_SetPowerStatus( unit, power, status);
    }

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif

#if (SYS_CPNT_POWER_DETECT == TRUE)
        SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_Callback(SYS_MODULE_SYSDRV, unit, power, status);
#elif (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
        SYS_CALLBACK_MGR_SYSDRV_PowerStatusChanged_Callback(SYS_MODULE_SYSDRV, unit, power, status);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*Step 2: If slave mode, call ISC remote call to master unit*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_POWER_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan status changed to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_POWER_STATUS_CHANGED;
        isc_buf_p->info.power_status_changed.unit  = unit;
        isc_buf_p->info.power_status_changed.power = power;
        isc_buf_p->info.power_status_changed.status = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
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
void SYSDRV_Notify_PowerTypeChanged(UI32_T unit, UI32_T power, UI32_T type)
{
    UI16_T               ret_val;
    UI8_T                master_unit_id;

    if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
    {
        BACKDOOR_MGR_Printf("%s(%d):unit=%lu, power=%lu, type=0x%08lX\r\n",
            __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)power, (unsigned long)type);
    }
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif

        SYS_CALLBACK_MGR_SYSDRV_PowerTypeChanged_CallBack(SYS_MODULE_SYSDRV, unit, power, type);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*Step 2: If slave mode, call ISC remote call to master unit*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_POWER_TYPE_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
                BACKDOOR_MGR_Printf("%s():L_MM_Mref_GetPdu fails.\r\n", __FUNCTION__);
            return;
        }

        /* notify fan status changed to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_POWER_TYPE_CHANGED;
        isc_buf_p->info.power_type_changed.unit  = unit;
        isc_buf_p->info.power_type_changed.power = power;
        isc_buf_p->info.power_type_changed.type  = type;

        STKTPLG_OM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            if(SYSDRV_SHOW_ERROR_MSG_FLAG()==TRUE)
                BACKDOOR_MGR_Printf("%s():ISC_SendMcastReliable fails.(ret_val=0x%x)\r\n", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}
#endif /* end of #if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE) */

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
void SYSDRV_Notify_FanStatusChanged(UI32_T unit, UI32_T fan, UI32_T status)
{
    UI8_T                master_unit_id;

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif
        SYS_CALLBACK_MGR_SYSDRV_FanStatusChanged_CallBack(SYS_MODULE_SYSDRV, unit, fan, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*slave mode*/
    {
        SYSDRV_PktHdr_T*    isc_buf_p;
        L_MM_Mref_Handle_T* mref_handle_p;
        UI32_T              pdu_len;
        UI16_T              ret_val;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_FAN_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if (isc_buf_p == NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan status change to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_FAN_STATUS_CHANGED;
        isc_buf_p->info.fan_status_changed.unit = unit;
        isc_buf_p->info.fan_status_changed.fan  = fan;
        isc_buf_p->info.fan_status_changed.status = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT, FALSE);

        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

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
void SYSDRV_Notify_FanSpeedChanged(UI32_T unit, UI32_T fan, UI32_T speed)
{
    UI8_T                master_unit_id;

#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif
        SYS_CALLBACK_MGR_SYSDRV_FanSpeedChanged_CallBack(SYS_MODULE_SYSDRV, unit, fan, speed);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*slave mode*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;
        UI16_T              ret_val;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_FAN_SPEED_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        /* notify fan speed change to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_FAN_SPEED_CHANGED;
        isc_buf_p->info.fan_status_changed.unit   = unit;
        isc_buf_p->info.fan_status_changed.fan    = fan;
        isc_buf_p->info.fan_status_changed.speed  = speed;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}
#endif /*End of SYS_CPNT_STKTPLG_FAN_DETECT*/
/*add by fen.wang
  PURPOSE: This function will notify sysdrv to the system will reload after a while
*/

 void SYSDRV_PrepareStartSystem(UI32_T type)
{
    if(type==SYS_VAL_COLD_START_FOR_RELOAD)
     SYSFUN_SendEvent(sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_PREPARE_COLD_START_FOR_RELOAD);
    else if(type==SYS_VAL_WARM_START_FOR_RELOAD)
     SYSFUN_SendEvent(sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_PREPARE_WARM_START_FOR_RELOAD);
    return ;
}

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
void SYSDRV_ColdStartSystem(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if ( SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_COLD_START_SYSTEM) == FALSE )
         {
              BACKDOOR_MGR_Printf("\r\nSYSDRV_ColdStartSystem: unable to send request to slave unit!!\r\n");
              return ;
         }
    }
#endif
    /* The event after SYSDRV_EVENT_PREPARE_COLD_START_FOR_RELOAD
     * should be SYSDRV_EVENT_COLD_START_FOR_RELOAD not SYSDRV_EVENT_COLD_START.
     */
    SYSFUN_SendEvent(sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_COLD_START_FOR_RELOAD);

    return ;
} /* End of SYSDRV_ColdStartSystem() */

void SYSDRV_ColdStartSystemForTimeout(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if ( SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_COLD_START_SYSTEM) == FALSE )
         {
              BACKDOOR_MGR_Printf("\r\nSYSDRV_ColdStartSystem: unable to send request to slave unit!!\r\n");
              return ;
         }
    }
#endif
}

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
void SYSDRV_ReloadSystem(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if ( SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_RELOAD_SYSTEM) == FALSE )
         {
              BACKDOOR_MGR_Printf("\r\nSYSDRV_ColdStartSystem: unable to send request to slave unit!!\r\n");
              return ;
         }
    }
#endif

    SYSFUN_SendEvent(sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_WARM_START_FOR_RELOAD);

    return ;
} /* End of SYSDRV_ReloadSystem() */


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
void SYSDRV_WarmStartSystem(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if ( SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_WARM_START_SYSTEM) == FALSE )
         {
              BACKDOOR_MGR_Printf("\r\nSYSDRV_ColdStartSystem: unable to send request to slave unit!!\r\n");
              return ;
         }
    }
#endif

    SYSFUN_SendEvent(sysdrv_shmem_data_p->sys_drv_task_id, SYSDRV_EVENT_WARM_START_FOR_RELOAD);

    return ;
} /* End of SYSDRV_WarmStartSystem() */

void SYSDRV_WarmStartSystemForTimeout(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if ( SYSDRV_SendRequestToSlave(SYSDRV_SERVICE_ID_WARM_START_SYSTEM) == FALSE )
         {
              BACKDOOR_MGR_Printf("\r\nSYSDRV_ColdStartSystem: unable to send request to slave unit!!\r\n");
              return ;
         }
    }
#endif
}

/* LOCAL SUBPROGRAM BODIES
 */

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
BOOL_T SYSDRV_SetI2CChannel(UI32_T channel_addr, UI8_T channel_regaddr, UI8_T channel_num)
{
    SYSFUN_Sleep(10);
    return I2CDRV_SetChannel( channel_addr, channel_regaddr, channel_num);
}

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
BOOL_T SYSDRV_GetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info)
{
    SYSFUN_Sleep(10);
    return I2CDRV_GetI2CInfo( slave_addr, offset, size, info);
}

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
BOOL_T SYSDRV_SetI2CInfo(UI8_T slave_addr, UI16_T offset, UI8_T size, UI8_T *info)
{
    SYSFUN_Sleep(10);
    return I2CDRV_SetI2CInfo( slave_addr, offset, size, info);
}


#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
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
BOOL_T SYSDRV_SetFanSpeedMode(SYSDRV_FanSpeed_T mode)
{
    if (SYSDRV_TASK_IS_VALID_FAN_SPEED_MODE(mode)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s: Invalid fan speed mode %d\r\n",
            __FUNCTION__, (int)mode);
        return FALSE;
    }

    SYSDRV_ENTER_CRITICAL_SECTION();
    sysdrv_shmem_data_p->sysdrv_speed_setting_mode=mode;
    SYSDRV_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

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
BOOL_T SYSDRV_GetFanSpeedForceFull(void)
{
    return sysdrv_shmem_data_p->sysdrv_fan_speed_force_full;
}

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
BOOL_T SYSDRV_SetFanSpeedForceFull(BOOL_T mode)
{
    UI8_T  service_id;

    SYSDRV_ENTER_CRITICAL_SECTION();
    sysdrv_shmem_data_p->sysdrv_fan_speed_force_full = mode;
    SYSDRV_LEAVE_CRITICAL_SECTION();

#if (SYS_CPNT_STACKING == TRUE)
    if (mode == TRUE)
        service_id = SYSDRV_SERVICE_ID_ENABLEFANSPEEDFORCEFULL;
    else
        service_id = SYSDRV_SERVICE_ID_DISABLEFANSPEEDFORCEFULL;
    /* In master mode, send provision complete information to all slave units
     */
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
         if (FALSE == SYSDRV_SendRequestToSlave(service_id))
         {
              printf("\r\nSYSDRV_SetFanSpeedForceFull: unable to send request to slave unit!!");
              return FALSE;
         }
    }
#endif /* SYS_CPNT_STACKING */

    return TRUE;
}
#endif /* end of #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE) */

#if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC != TRUE) && (SYS_HWCFG_FAN_DETECT_FAN_STATUS_BY_BID != TRUE)

/* FUNCTION NAME: SYSDRV_DetectFanControllerType
 * PURPOSE: This function is used to check fan controller type
 * INPUT:   None.
 * OUTPUT:  fan_type     -- UI32_T
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
static BOOL_T SYSDRV_DetectFanControllerType(UI32_T *fan_type)
{
    SYS_HWCFG_FanRegInfo_T reg_info;
    UI32_T unit_id, board_id;
    UI8_T  i2cbuf[8], i2c_dev_addr;
    BOOL_T status;

    memset(i2cbuf, 0, sizeof(UI8_T) *8);
    if( (STKTPLG_OM_GetMyUnitID(&unit_id)==TRUE) &&
            (STKTPLG_OM_GetUnitBoardID(unit_id, &board_id)==TRUE))
    {
        nbr_of_fan = STKTPLG_BOARD_GetFanNumber();
        if(SYS_HWCFG_GetFanRegInfo(board_id, 0, &reg_info)==FALSE)
        {
            printf("%s():Failed to get thermal register info.\r\n", __FUNCTION__);
            return FALSE;
        }
    }
    else
    {
        printf("%s():Failed to get unit id or board id.\r\n", __FUNCTION__);
        return FALSE;
    }

    if((reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C)&&
            (reg_info.access_method!=SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL))
    {
        printf("%s():Invalid access method: %u\r\n", __FUNCTION__, reg_info.access_method);
        return FALSE;
    }

    /* set and lock i2c channel if required
     */
    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
    {
        if(I2CDRV_SetAndLockMux(reg_info.info.i2c_with_channel.i2c_mux_index,
                    reg_info.info.i2c_with_channel.channel_val)==FALSE)
        {
            SYSFUN_Debug_Printf("%s():Failed to set and lock channel.\r\n", __FUNCTION__);
            return FALSE;
        }
        i2c_dev_addr = reg_info.info.i2c_with_channel.i2c_reg_info.dev_addr;
    }
    else
        i2c_dev_addr = reg_info.info.i2c.dev_addr;

    status = SYSDRV_GetI2CInfo(i2c_dev_addr, SYSDRV_FAN_CHIP_ID, 1, i2cbuf);

    if (status == TRUE)
    {
        *fan_type = SYS_HWCFG_FAN_CONTROLLER_TYPE;
    }
    else
    {
        *fan_type = SYS_HWCFG_FAN_NONE;
    }

    if(reg_info.access_method==SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
        I2CDRV_UnLockMux(reg_info.info.i2c_with_channel.i2c_mux_index);

    return TRUE;
}
#endif /* End of #if (SYS_CPNT_SYSDRV_MIXED_THERMAL_FAN_ASIC != TRUE) */
#endif /*End of SYS_CPNT_STKTPLG_FAN_DETECT*/

#if (SYS_CPNT_STACKING == TRUE)
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
BOOL_T SYSDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    SYSDRV_PktHdr_T  *msg_buf;
    UI32_T            pdu_len;
    BOOL_T            ret;

    msg_buf = (SYSDRV_PktHdr_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(msg_buf==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
        return FALSE;
    }

    /*
     * Check to abort operation if callback service id(opcode) is more then
     * number of callback service on this drive.
     */
    if(msg_buf->ServiceID >= SYSDRV_SERVICE_ID_MAX_NUM_OF_SERVICE || sysdrv_func_tab[msg_buf->ServiceID]==NULL)
    {
        printf("\r\nSYSDRV: Service ID is invalid!\r\n");
        return FALSE;
    }
    else
    {
        /* call related function for this service ID to restart system
         */
        ret=sysdrv_func_tab[msg_buf->ServiceID](key, msg_buf);

    }
    /* release reference
     */
    L_MM_Mref_Release(&mref_handle_p);

    return ret;
} /* End of SYSDRV_ISC_Handler() */


/* FUNCTION NAME: SYSDRV_SendRequestToSlave
 *-----------------------------------------------------------------------------
 * PURPOSE: This function is used by Master to send restart request to Slave.
 *-----------------------------------------------------------------------------
 * INPUT    : service_id : Service ID for slave unit, a type of restart.
 * OUTPUT   : none
 * RETURN   : TRUE : successful
 *            FALSE: failure
 *-----------------------------------------------------------------------------
 * NOTES:
 */
static BOOL_T SYSDRV_SendRequestToSlave(UI8_T service_id)
{
    L_MM_Mref_Handle_T     *mref_handle_p;
    SYSDRV_PktHdr_T        *sysdrv_header;
    UI32_T                 num_of_unit, index, unit_bmp=0, my_unit_id, drv_unit;
    UI32_T                 pdu_len;
    UI8_T                  master_unit_id;

    /* Cannot get number of units in this stack
     */
    if ( STKTPLG_POM_GetNumberOfUnit(&num_of_unit) == FALSE )
    {
         return FALSE;
    }

    /* Check to see if stack topology is in standalone or in stacking
     */
    STKTPLG_POM_GetMyUnitID(&my_unit_id);

    if ((STKTPLG_POM_OptionModuleIsExist(my_unit_id, &drv_unit) == FALSE) && (num_of_unit == 1))
    {
         /* Do nothing if in standalone
          */
         return TRUE;
    }

    /* Send packet to all remote units via ISC (Multicast)
     */
    STKTPLG_POM_GetMasterUnitId(&master_unit_id);

    index=0;
    while(TRUE == STKTPLG_POM_GetNextUnit(&index))
    {
        if (index == master_unit_id)
        {
            /* add Master Option Module */
            if (STKTPLG_POM_OptionModuleIsExist(index, &drv_unit) == TRUE)
            {
                unit_bmp |= BIT_VALUE(drv_unit-1);
            }
            continue;
        }
        unit_bmp |= BIT_VALUE(index-1);
        /* add Slave Option Module */
        if (STKTPLG_POM_OptionModuleIsExist(index, &drv_unit) == TRUE)
        {
            unit_bmp |= BIT_VALUE(drv_unit-1);
        }
    }

    if(unit_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, service_id));

        /* Get pointer of SYSDRV header within ISC buffer
         */
        sysdrv_header = (SYSDRV_PktHdr_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if( sysdrv_header == NULL )
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
            return FALSE;
        }

        sysdrv_header->ServiceID = service_id;

        if(0!=ISC_SendMcastReliable(unit_bmp, ISC_SYSDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT, FALSE))
        {
            return FALSE;
        }
    }

    return TRUE;

} /* End of SYSDRV_SendRequestToSlave() */

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
BOOL_T SYSDRV_SendWarmStartToOptionModule(UI32_T drv_unit)
{
    L_MM_Mref_Handle_T     *mref_handle_p;
    /* charlie_rem UI32_T                 sysdrv_frame_len;*/
    SYSDRV_PktHdr_T        *sysdrv_header;
    /* charlie_rem UI32_T                 num_of_unit, master_unit_id, index, my_unit_id;*/
    UI32_T                 pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                          L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_WARM_START_SYSTEM));

    /* Get pointer of SYSDRV header within ISC buffer
     */
    sysdrv_header = (SYSDRV_PktHdr_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    if( sysdrv_header == NULL )
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
        return FALSE;
    }

    sysdrv_header->ServiceID = SYSDRV_SERVICE_ID_WARM_START_SYSTEM;

    if(0!=ISC_SendMcastReliable(BIT_VALUE(drv_unit-1), ISC_SYSDRV_SID,
                                mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT, FALSE))
    {
        return FALSE;
    }

    return TRUE;

} /* End of SYSDRV_SendWarmStartToOptionModule() */


/* master -> slave */
static BOOL_T SYSDRV_FromIscColdStartSystem(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_ColdStartSystem();
    return TRUE;
}

/* master -> slave */
static BOOL_T SYSDRV_FromIscReloadSystem(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_ColdStartSystem();
    return TRUE;
}

/* master -> slave */
static BOOL_T SYSDRV_FromIscWarmStartSystem(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_WarmStartSystem();
    return TRUE;
}

/* master -> slave */
static BOOL_T SYSDRV_FromIscProvisionComplete(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_ProvisionComplete();
    return TRUE;
}

#if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
static BOOL_T SYSDRV_FromIscEnableFanSpeedForceFull(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    return SYSDRV_SetFanSpeedForceFull(TRUE);
}

static BOOL_T SYSDRV_FromIscDisableFanSpeedForceFull(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    return SYSDRV_SetFanSpeedForceFull(FALSE);
}
#endif

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_FanStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_FanStatusChanged(buf_p->info.fan_status_changed.unit,
                                   buf_p->info.fan_status_changed.fan,
                                   buf_p->info.fan_status_changed.status);
#endif
    return TRUE;
}

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_FanSpeedChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_FanSpeedChanged(buf_p->info.fan_status_changed.unit,
                                  buf_p->info.fan_status_changed.fan,
                                  buf_p->info.fan_status_changed.speed);
#endif

    return TRUE;
}

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_PowerStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_PowerStatusChanged(buf_p->info.power_status_changed.unit,
                                     buf_p->info.power_status_changed.power,
                                     buf_p->info.power_status_changed.status);
    return TRUE;
}

/* slave -> master */
#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
static BOOL_T SYSDRV_FromIscNotify_PowerTypeChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_Notify_PowerTypeChanged(buf_p->info.power_type_changed.unit,
                                   buf_p->info.power_type_changed.power,
                                   buf_p->info.power_type_changed.type);
    return TRUE;
}
#else
static BOOL_T SYSDRV_FromIscNotify_PowerTypeChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    return FALSE;
}
#endif

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_AlarmInputStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_AlarmInputStatusChanged(buf_p->info.alarm_input_status_changed.unit,
                                     buf_p->info.alarm_input_status_changed.status);
    return TRUE;
}

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_MajorAlarmOutputStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_Notify_MajorAlarmOutputStatusChanged(buf_p->info.alarm_output_status_changed.unit,
                                                buf_p->info.alarm_output_status_changed.status);
    return TRUE;
}

/* slave -> master */
static BOOL_T SYSDRV_FromIscNotify_MinorAlarmOutputStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    SYSDRV_Notify_MinorAlarmOutputStatusChanged(buf_p->info.alarm_output_status_changed.unit,
                                                buf_p->info.alarm_output_status_changed.status);
    return TRUE;
}

/* slave -> master */

static BOOL_T SYSDRV_FromIscNotify_ThermalStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_ThermalStatusChanged( buf_p->info.thermal_status_changed.unit,
                                        buf_p->info.thermal_status_changed.thermal,
                                        buf_p->info.thermal_status_changed.status);
#endif
    return TRUE;
}

/* slave -> master */

static BOOL_T SYSDRV_FromIscNotify_ThermalTemperatureChanged_Obsoleted(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    /* This function is obsoleted. This function is not removed because I2C Service ID
     * should be kept for backward compability
     */
#if 0
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    /* This is a temporary fix for ISC exception proposed by Charles */
    SYSDRV_Notify_ThermalTemperatureChanged( buf_p->info.thermal_status_changed.unit,
                                        buf_p->info.thermal_status_changed.thermal,
                                        buf_p->info.thermal_status_changed.status);
#endif
#else
    BACKDOOR_MGR_Printf("%s: Warnning. An obsoleted function is called.\r\n", __FUNCTION__);
#endif
    return TRUE;
}

#endif /* end of SYS_CPNT_STACKING == TRUE */


#if 0 /* SYSDRV_TestI2CChannel is not used now */
/*#if ((SYS_CPNT_THERMAL_DETECT == TRUE) && (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE))*/
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
UI32_T SYSDRV_TestI2CChannel(UI32_T interval)
{
    UI8_T i2cbuf[8];
    UI32_T fail, i;

    fail=0;
    if (interval >0)
    {
        SYSFUN_Sleep(interval);
    }
    #if (SYS_HWCFG_FAN_CONTROLLER_TYPE != SYS_HWCFG_FAN_WIN83782)
    if (!I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_THERMAL_0, 0, 2, i2cbuf))
    #else
    if (!I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_THERMAL_0, SYS_HWCFG_THERMAL_1, 1, i2cbuf))
    #endif
    {
        fail++;
    }
    if (interval >0)
    {
        SYSFUN_Sleep(interval);
    }
    if (!I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_FAN_MONITOR_0, SYSDRV_FAN_MONITOR, 1, i2cbuf))
    {
        fail++;
    }

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
    {
        if (interval >0)
        {
            SYSFUN_Sleep(interval);
        }
#if defined(SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP)
        if((SYS_HWCFG_MAX_FAN_CONTROLLER_VALID_INDEX_BITMAP & (0x1<<(i)))==0)
            continue;
#endif

        if (!I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_FAN_MONITOR_0, FAN_SPEED_ADDR[i], 1, i2cbuf))
        {
            fail++;
        }
    }

    return fail;
}
#endif

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
BOOL_T SYSDRV_GetThermalThresholds(I8_T falling_thresholds[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT], I8_T rising_thresholds[SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT])
{
#ifdef SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY
    const I8_T thermal_threshold_up[][SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT] = SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY;
    const I8_T thermal_threshold_down[][SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT] = SYS_ADPT_THERMAL_THRESHOLD_DOWN_WITH_MULTIPLE_BIDS_ARRAY;
    UC_MGR_Sys_Info_T sys_info;
    UI8_T             i;

    if ( UC_MGR_GetSysInfo(&sys_info) != TRUE )
    {
        BACKDOOR_MGR_Printf("%s:Failed to get system infomation from UC\r\n", __FUNCTION__);
        return FALSE;
    }

    if (sys_info.board_id > (sizeof(thermal_threshold_up)/(sizeof(I8_T)*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT)))
    {
        BACKDOOR_MGR_Printf("%s:Warning! bid= %lu. Inconsistent definition of SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY.\r\n",
            __FUNCTION__, (unsigned long)sys_info.board_id);
        return FALSE;
    }

    for (i=0; i<SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; i++)
    {
        falling_thresholds[i] = thermal_threshold_down[sys_info.board_id][i];
        rising_thresholds[i] = thermal_threshold_up[sys_info.board_id][i];
    }
    return TRUE;

#else /* #ifdef SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY */
        const I8_T thermal_threshold_up[] = SYS_ADPT_THERMAL_THRESHOLD_UP_ARRAY;
        const I8_T thermal_threshold_down[] = SYS_ADPT_THERMAL_THRESHOLD_DOWN_ARRAY;

        if ( (sizeof(thermal_threshold_up)/sizeof(thermal_threshold_up[0])) > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT )
        {
            BACKDOOR_MGR_Printf("%s:Inconsistent definition of SYS_ADPT_THERMAL_THRESHOLD_UP_ARRAY\r\n", __FUNCTION__);
            return FALSE;
        }

        if ( (sizeof(thermal_threshold_down)/sizeof(thermal_threshold_down[0])) > SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT )
        {
            BACKDOOR_MGR_Printf("%s:Inconsistent definition of SYS_ADPT_THERMAL_THRESHOLD_DOWN_ARRAY\r\n", __FUNCTION__);
            return FALSE;
        }

        memcpy(falling_thresholds, thermal_threshold_down, sizeof(I8_T)*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT);
        memcpy(rising_thresholds, thermal_threshold_up, sizeof(I8_T)*SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT);
        return TRUE;
#endif /* end of #ifdef SYS_ADPT_THERMAL_THRESHOLD_UP_WITH_MULTIPLE_BIDS_ARRAY */
}

/* FUNCTION NAME: SYSDRV_GetThermalNum
 *-----------------------------------------------------------------------------
 * PURPOSE: Get actually thermalnumber for a specific unit
 *-----------------------------------------------------------------------------
 * INPUT    : unit->unit ID
 * OUTPUT   : thermal_num->store actually thermal number
 * RETURN   : TRUE  - At least one fan installed on the unit
 *            FALSE - No fan is installed on the unit
 *-----------------------------------------------------------------------------
 * NOTES:
 */

BOOL_T SYSDRV_GetThermalNum(UI32_T unit,UI32_T *thermal_num)
{

    *thermal_num = STKTPLG_BOARD_GetThermalNumber();

    if(*thermal_num == 0)
        return FALSE;

    return TRUE;
}

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
void SYSDRV_Notify_ThermalStatusChanged(UI32_T unit, UI32_T thermal, I32_T status)
{
    UI8_T                master_unit_id;


#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif

    SYS_CALLBACK_MGR_SYSDRV_ThermalStatusChanged_CallBack(SYS_MODULE_SYSDRV, unit, thermal, status);
//    SYS_PMGR_ThermalStatusChanged_CallBack(unit, thermal, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*slave mode*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;
        UI16_T              ret_val;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_THERMAL_STATUS_CHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }


        /* notify thermal temperature change to master
         */
        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_THERMAL_STATUS_CHANGED;
        isc_buf_p->info.thermal_status_changed.unit    = unit;
        isc_buf_p->info.thermal_status_changed.thermal = thermal;
        isc_buf_p->info.thermal_status_changed.status  = status;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /*ErrorMessage
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */


/* -----------------------------------------------------------------------------
 * FUNCTION NAME - SYSDRV_GetPushButtonStatus
 * -----------------------------------------------------------------------------
 * PURPOSE : This function is used to read S/W master button status from FLASH.
 * INPUT   : unit_id - unit id or drive number
 * OUTPUT  : status  - S/W master button status, press(TRUE)/unpress(FALSE)
 * RETURN  : TRUE  - successful
 *           FALSE - otherwise
 * NOTES   : The S/W master button status will be save on an invisible binary file.
 *           File name is ".sw_push_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetPushButtonStatus(UI32_T unit_id, BOOL_T *status)
{
    UI32_T flash_data_count;
    UI32_T read_flash_result;

    read_flash_result = FS_ReadFile(unit_id,
                                    (UI8_T*)".sw_push_btn_status",
                                    (UI8_T *)status,
                                    1,
                                    &flash_data_count);

    if (FS_RETURN_OK == read_flash_result)
    {
        return TRUE;
    }
    else
    {
        /* If status file is not existed, status will be set unpress */
        *status = FALSE;
        return FALSE;
    }
}

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
 *           File name is ".sw_push_btn_status".
 * -----------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetPushButtonStatus(UI32_T unit_id, BOOL_T status)
{
    UI32_T write_flash_result;

    write_flash_result = FS_WriteFile(unit_id,
                                      (UI8_T*)".sw_push_btn_status",
                                      (UI8_T*)"Software Push Button Status",
                                      FS_FILE_TYPE_PRIVATE,
                                      (UI8_T *)&status,
                                      1,
                                      1);

    if (FS_RETURN_OK == write_flash_result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

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
BOOL_T SYSDRV_GetStackingButtonStatus(BOOL_T *status)
{
    UI32_T flash_data_count;
    UI32_T read_flash_result;

    read_flash_result = FS_ReadFile(DUMMY_DRIVE,
                                    (UI8_T*)".sw_stacking_btn_status",
                                    (UI8_T *)status,
                                    1,
                                    &flash_data_count);

    if (FS_RETURN_OK == read_flash_result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

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
BOOL_T SYSDRV_SetStackingButtonStatus(BOOL_T status)
{
    UI32_T write_flash_result;

    write_flash_result = FS_WriteFile(DUMMY_DRIVE,
                                      (UI8_T*)".sw_stacking_btn_status",
                                      (UI8_T*)"Software Stacking Button Status",
                                      FS_FILE_TYPE_PRIVATE,
                                      (UI8_T *)&status,
                                      1,
                                      1);

    if (FS_RETURN_OK == write_flash_result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

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
void SYSDRV_Notify_XenpakStatusChanged(UI32_T unit, UI32_T xenpak_type)
{
#if (SYS_CPNT_STACKING == TRUE)
    if ( SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p) == SYS_TYPE_STACKING_MASTER_MODE )
    {
#endif
    //SYS_CALLBACK_MGR_SYSDRV_XenpakStatusChanged_CallBack(SYS_MODULE_SYSDRV,xenpak_type);
#if (SYS_CPNT_STACKING == TRUE)
    }
    else  /*slave mode*/
    {
        SYSDRV_PktHdr_T    *isc_buf_p;
        L_MM_Mref_Handle_T *mref_handle_p;
        UI32_T              pdu_len;
        UI16_T              ret_val;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(SYSDRV_PktHdr_T),
                                              L_MM_USER_ID(SYS_MODULE_SYSDRV, SYSDRV_POOL_ID_ISC_SEND, SYSDRV_SERVICE_ID_NOTIFY_XENPAKSTATECHANGED));
        isc_buf_p = (SYSDRV_PktHdr_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
        if(isc_buf_p==NULL)
        {
            SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails.", __FUNCTION__);
            return;
        }

        isc_buf_p->ServiceID = SYSDRV_SERVICE_ID_NOTIFY_XENPAKSTATECHANGED;
        isc_buf_p->info.xenpak_status_changed.unit  = unit;
        isc_buf_p->info.xenpak_status_changed.xenpak_type = xenpak_type;

        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        ret_val = ISC_SendMcastReliable(BIT_VALUE(master_unit_id-1), ISC_SYSDRV_SID,
                                        mref_handle_p, SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                        SYSDRV_TRY_TIMES, SYSDRV_TIME_OUT,
                                        FALSE);
        if(ret_val!=0)
        {
            /* Module SYSDRV_Notify_XenpakStatusChanged ISC_SendMcastReliable error
             */
            SYSFUN_Debug_Printf("\r\n%s():ISC_SendMcastReliable fails.(ret_val=0x%x)", __FUNCTION__, ret_val);
        }
    }
#endif

    return;
}

#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SYSDRV_FromIscNotify_XenpakStatusChanged(ISC_Key_T *key, SYSDRV_PktHdr_T *buf_p)
{
    STKTPLG_OM_switchModuleInfoEntry_T module_info;
    UI32_T unit;
    UI32_T module_num = 1;


    SYSDRV_Notify_XenpakStatusChanged(buf_p->info.xenpak_status_changed.unit,
                                      buf_p->info.xenpak_status_changed.xenpak_type);

    unit = buf_p->info.xenpak_status_changed.unit;
    if(TRUE == STKTPLG_POM_GetModuleInfo(unit, module_num, &module_info) )
    {
        if( module_info.xenpak_status != buf_p->info.xenpak_status_changed.xenpak_type)
        {
            module_info.xenpak_status = buf_p->info.xenpak_status_changed.xenpak_type;
            if(FALSE==STKTPLG_POM_SetModuleInfo(unit, module_num, &module_info))
            {
                SYSFUN_Debug_Printf("\r\n%s():STKTPLG_OM_SetModuleInfo fails", __FUNCTION__);
                return FALSE;
            }
        }
        }
    else
    {
        SYSFUN_Debug_Printf("\r\n%s():STKTPLG_OM_GetModuleInfo fails", __FUNCTION__);
    }

    return TRUE;
}
#endif

/*------------------------------------------------------------------------------
 * Function : SYSDRV_SetTaskTransitionDone
 *------------------------------------------------------------------------------
 * Purpose  : System driver task will notify transistion done to sysdrv by this function.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
void SYSDRV_SetTaskTransitionDone(void)
{
    sysdrv_shmem_data_p->is_transition_done = TRUE;
    return;
}

/*------------------------------------------------------------------------------
 * Function : SYSDRV_IsProvisionComplete
 *------------------------------------------------------------------------------
 * Purpose  : Check Whether Provision state is complete or not by this function.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : Completed, False : Not yet
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
BOOL_T SYSDRV_IsProvisionComplete(void)
{
    return sysdrv_shmem_data_p->sysdrv_is_provision_complete;
}

/*------------------------------------------------------------------------------
 * Function : SYSDRV_GetOperatingMode
 *------------------------------------------------------------------------------
 * Purpose  : Get operating mode by this function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : UI32_T SYS_TYPE_Stacking_Mode_T
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
UI32_T SYSDRV_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(sysdrv_shmem_data_p);
}

/*------------------------------------------------------------------------------
 * Function : SYSDRV_SetTaskID
 *------------------------------------------------------------------------------
 * Purpose  : Set sysdrv task ID to database
 * INPUT    : UI32_T task_id  -- system driver task ID
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
void SYSDRV_SetTaskID(UI32_T task_id)
{
    sysdrv_shmem_data_p->sys_drv_task_id = task_id;
    return;
}

/*add by michael.wang,2008-7-1 */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_GetSfpInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  read Sfp Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array,1,2,....
 *           offset:data register offset address
 *           size:size of read data
 * OUTPUT:   info_p:read data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_GetSfpInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p)
{
    return SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_BASIC, dev_index, offset, size, info_p, GBIC_ACCESS_TYPE_READ);
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetSfpInfo
 *---------------------------------------------------------------------------
 * PURPOSE:  write Sfp Info by I2C BUS
 * INPUT:    dev_index:SFP index in define SFP array,1,2,....
 *           offset:data register offset address
 *           size:size of written data
 * OUTPUT:   info_p:writtend data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_SetSfpInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p)
{
    return SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_BASIC, dev_index, offset, size, info_p, GBIC_ACCESS_TYPE_WRITE);
}
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
BOOL_T SYSDRV_GetSfpDdmInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p)
{
    return SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_DDM, dev_index, offset, size, info_p, GBIC_ACCESS_TYPE_READ);
}

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
BOOL_T SYSDRV_SetSfpDdmInfo(UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info_p)
{
    return SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_DDM, dev_index, offset, size, info_p, GBIC_ACCESS_TYPE_WRITE);
}

#if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_AccessGbicInfoInternal
 *---------------------------------------------------------------------------
 * PURPOSE:  A generic function to read/write Sfp Info by I2C BUS according to the
 *           specified Sfp Info Type.
 * INPUT:    gbic_info_type: Valid types are listed below:
 *                           GBIC_INFO_TYPE_BASIC (Read through I2C address 0x50)
 *                           GBIC_INFO_TYPE_DDM   (Read through I2C address 0x51)
 *           dev_index:SFP index in define SFP array,1,2,....
 *           offset:data register offset address
 *           size:size of read/written data
 * OUTPUT:   info:read/written data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_T gbic_info_type, UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info, GBIC_ACCESS_TYPE_T gbic_access_type)
{
    UI32_T unit, port;
    UI16_T retry=5;
    BOOL_T rc=FALSE;

    STKTPLG_POM_GetMyUnitID(&unit);
    if (STKTPLG_POM_SfpIndexToUserPort(unit, dev_index, &port)==FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)STKTPLG_POM_SfpIndexToUserPort error.(unit %lu sfp_index %lu type=%d).\r\n",
            __FUNCTION__, __LINE__, (unsigned long)unit, (unsigned long)dev_index, (int)gbic_info_type);
        return rc;
    }

    while (retry>0)
    {
        rc = ONLPDRV_SFP_AccessGbicInfo(unit, port, (ONLPDRV_GBIC_INFO_TYPE_T)gbic_info_type,
            offset, size, (ONLPDRV_SFP_GBIC_ACCESS_TYPE_T)gbic_access_type, info);

        if (rc==FALSE)
        {
            if(SYSDRV_SHOW_DEBUG_MSG_FLAG()==TRUE)
            {
                BACKDOOR_MGR_Printf("%s():ONLPDRV_AccessGbicInfo error(retry left=%hu)\r\n", __FUNCTION__, retry-1);
            }
        }
        else
        {
            break;
        }
        retry--;
    }

    return rc;
} /* end of static BOOL_T SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_T gbic_info_type, ...) */
#else /* #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_AccessGbicInfoInternal
 *---------------------------------------------------------------------------
 * PURPOSE:  A generic function to read/write Sfp Info by I2C BUS according to the
 *           specified Sfp Info Type.
 * INPUT:    gbic_info_type: Valid types are listed below:
 *                           GBIC_INFO_TYPE_BASIC (Read through I2C address 0x50)
 *                           GBIC_INFO_TYPE_DDM   (Read through I2C address 0x51)
 *           dev_index:SFP index in define SFP array,1,2,....
 *           offset:data register offset address
 *           size:size of read/written data
 * OUTPUT:   info:read/written data info
 * RETURN:   TRUE/FALSE
 * NOTE:
 *---------------------------------------------------------------------------
 */
static BOOL_T SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_T gbic_info_type, UI32_T dev_index, UI16_T offset, UI8_T size, UI8_T *info, GBIC_ACCESS_TYPE_T gbic_access_type)
{
    BOOL_T  ret=TRUE, rc;
    UI8_T   j, len, access_size, i2c_dev_addr;
    UI16_T  offset_tmp;
    UI8_T   *data_in;
    UI8_T   flag = 0;
    UI8_T   i2c_bus_idx;
    UI32_T  sfp_retry_times=5;
    UI32_T  bytes_per_transaction=113;
#if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE)
    UC_MGR_Sys_Info_T uc_sys_info;
    SYS_HWCFG_i2cRegAndChannelInfo_T reg_info;
#else
    UI32_T  devaddr;
    UI8_T   channel;
#endif

    switch (gbic_info_type)
    {
        case GBIC_INFO_TYPE_DDM:
            i2c_dev_addr = SYS_HWCFG_I2C_SLAVE_EEPROM+1;
            break;
        default:
        case GBIC_INFO_TYPE_BASIC:
            i2c_dev_addr = SYS_HWCFG_I2C_SLAVE_EEPROM;
            break;
    }

    len = size;
    offset_tmp = offset;
    data_in = info;

    if(info == NULL)
    {
        BACKDOOR_MGR_Printf("%s(%d)Get GBIC Info Fail(type=%d).\r\n", __FUNCTION__, __LINE__, (int)gbic_info_type);
        return FALSE;
    }

#if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE)
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        BACKDOOR_MGR_Printf("%s(): Get UC System Information Fail.\r\n", __FUNCTION__);
        return FALSE;
    }

    if (SYS_HWCFG_GetGBIC_I2CMuxAndChannelInfo(uc_sys_info.board_id, dev_index, &reg_info) == FALSE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Get GBIC I2C info fail(dev_index=%lu)\r\n", __FUNCTION__, __LINE__, (unsigned long)dev_index);
        return FALSE;
    }
    i2c_bus_idx = reg_info.i2c_reg_info.bus_idx;
#else /* #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */
    if(SYS_HWCFG_GetGBIC_I2CChannelInfo(dev_index, &devaddr, (UI8_T *)&channel) == FALSE)
    {
        BACKDOOR_MGR_Printf("Get GBIC pca9548 fail ! \r\n");
        return FALSE;
    }
    i2c_bus_idx = SYS_HWCFG_I2C_SFP_EEPROM_BUS_IDX;
#endif /* #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */

#if (SYS_CPNT_I2C == TRUE)
#if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE)
    if (I2CDRV_SetAndLockMux(reg_info.i2c_mux_index, reg_info.channel_val)==FALSE)
        return FALSE;
#else /* #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */
#ifdef SYS_HWCFG_PCA9548_DATAADDR  /* Thomas added for 3628bt build error */
    if(SYSDRV_SetI2CChannel(devaddr, SYS_HWCFG_PCA9548_DATAADDR, channel) == FALSE)
#else
    if(SYSDRV_SetI2CChannel(devaddr, 0, channel) == FALSE)
#endif
    {
           return FALSE;
    }
#endif /* end of #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */

    while (len)
    {
        if (len > bytes_per_transaction)
        {
            access_size = bytes_per_transaction;
            len -= bytes_per_transaction;
        }
        else
        {
            access_size = len;
            len = 0;
        }

        switch (gbic_access_type)
        {
            case GBIC_ACCESS_TYPE_READ:
                if (I2CDRV_GetI2CInfoWithBusIdx(i2c_bus_idx, (i2c_dev_addr), offset_tmp, access_size, data_in)== FALSE)
                {
                    for (j=0; j< sfp_retry_times; j++)
                    {
                        SYSFUN_Sleep(10);

                        if (I2CDRV_GetI2CInfoWithBusIdx(i2c_bus_idx, (i2c_dev_addr), offset_tmp, access_size, data_in)==TRUE)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (!flag )
                    {
                        /* failed for retry-times, give up */
                        ret=FALSE;
                        len=0;
                    }
                }

                offset_tmp += bytes_per_transaction;
                data_in += bytes_per_transaction;
                break;

            case GBIC_ACCESS_TYPE_WRITE:
                if (I2CDRV_SetI2CInfoWithBusIdx(i2c_bus_idx, (i2c_dev_addr), offset_tmp, access_size, data_in)== FALSE)
                {
                    for (j=0; j< sfp_retry_times; j++)
                    {
                        SYSFUN_Sleep(10);

                        if (I2CDRV_SetI2CInfoWithBusIdx(i2c_bus_idx, (i2c_dev_addr), offset_tmp, access_size, data_in)==TRUE)
                        {
                            flag = 1;
                            break;
                        }
                    }
                    if (!flag )
                    {
                        /* failed for retry-times, give up */
                        ret=FALSE;
                        len=0;
                    }
                }

                offset_tmp += bytes_per_transaction;
                data_in += bytes_per_transaction;
                break;
            default:
                printf("Invalid GBIC access type ! \n");
                ret=FALSE;
                len=0;
                break;
        }
    }
#if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE)
    rc=I2CDRV_UnLockMux(reg_info.i2c_mux_index);
    if(rc==FALSE)
        ret=FALSE;
#else /* #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */
#ifdef SYS_HWCFG_PCA9548_DATAADDR  /* Thomas added for 3628bt build error */
    rc=SYSDRV_SetI2CChannel(devaddr,SYS_HWCFG_PCA9548_DATAADDR,SYS_HWCFG_PCA9548_DEFAULT_CHANNEL);
    if(rc==FALSE)
        ret=FALSE;
#endif /* #ifdef SYS_HWCFG_PCA9548_DATAADDR */
#endif /* end of #if (SYS_CPNT_I2C_GET_GBIC_INFO_BY_I2CDRV_MUX_API==TRUE) */
#endif /* end of #if (SYS_CPNT_I2C == TRUE) */

    return ret;
} /* end of static BOOL_T SYSDRV_AccessGbicInfoInternal(GBIC_INFO_TYPE_T gbic_info_type, ...) */
#endif /* end of #if (SYS_CPNT_SYSDRV_USE_ONLP == TRUE) */

#if (SYS_HWCFG_CPLD_TYPE != SYS_HWCFG_CPLD_TYPE_NONE)
/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_Upgrade_CPLD
 *---------------------------------------------------------------------------
 * PURPOSE:  Do upgrade CPLD fw
 * INPUT:    buf  : cpld data
 *           bufsize: cpld data of length
 * OUTPUT:   None
 * RETURN:   TRUE - Success/FALSE - Failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
BOOL_T SYSDRV_Upgrade_CPLD(UI8_T *buf, UI32_T bufsize)
{
    BOOL_T siRetCode = FALSE;
    UI32_T sched_policy, priority;

    if (SYSFUN_GetTaskPriority(SYSFUN_TaskIdSelf(), &sched_policy, &priority) != SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("SYSFUN_GetTaskPriority() error\r\n");
        return FALSE;
    }

    if(SYSFUN_SetTaskPriority(SYSFUN_SCHED_RR, SYSFUN_TaskIdSelf(), SYS_BLD_REALTIMEPROCESS_2ND_HIGH_PRIORITY)!=SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("SYSFUN_SetTaskPriority() error\r\n");
        return FALSE;
    }

    siRetCode = CPLDDRV_Upgrade_CPLD(buf, bufsize);

    if(SYSFUN_SetTaskPriority(sched_policy, SYSFUN_TaskIdSelf(), priority)!=SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("SYSFUN_SetTaskPriority() error\r\n");
        return FALSE;
    }

    return siRetCode;
}
#endif

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SYSDRV_SetDatabaseToDefault
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the value in SYSDRV database to default value or state.
 * INPUT:    None
 * OUTPUT:   None
 * RETURN:   None
 * NOTE:     1. An observed value(e.g. temperature, psu present) need to be
 *              reset to the initialized value. Note that the initialized value
 *              must be the same between the SYSDRV(Driver layer) and
 *              SYS_MGMT(Core layer).
 *           2. A configurable value(e.g. force all fans to operate in full speed)
 *              need to be reset to the SYS_DFLT value.
 *           3. Do not reset the value in database that will not be changed.
 *              For example, thermal type does not need to reset.
 *---------------------------------------------------------------------------
 */
static void SYSDRV_SetDatabaseToDefault(void)
{
#if ((SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) || (SYS_CPNT_THERMAL_DETECT == TRUE))
    int i;
#endif
#if (SYS_CPNT_POWER_DETECT == TRUE) && (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    SYS_HWCFG_PowerRegInfo_T power_reg_info;
    UI8_T pwr_idx;
#endif
#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    UI32_T alarm_input_mask;
#endif

    sysdrv_shmem_data_p->sysdrv_is_provision_complete = FALSE;

#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
    sysdrv_shmem_data_p->sysdrv_speed_setting_mode = SYS_HWCFG_DEFAULT_FAN_SPEED_MODE;
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; i++)
    {
        sysdrv_shmem_data_p->sysdrv_fan_install_status[i] = TRUE;
        sysdrv_shmem_data_p->sysdrv_fan_status[i] = VAL_switchFanStatus_ok;
        sysdrv_shmem_data_p->sysdrv_fan_speed[i]  = 0x0;
    }
    #if (SYS_CPNT_SYSMGMT_FAN_SPEED_FORCE_FULL == TRUE)
    sysdrv_shmem_data_p->sysdrv_fan_speed_force_full = SYS_DFLT_SYSMGMT_FAN_SPEED_FORCE_FULL;
    #endif

    #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE)
        #if (SYS_CPNT_SYSMGMT_FAN_SPEED_SELF_ADJUST == TRUE)
    memset(sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter, 0, sizeof(sysdrv_shmem_data_p->fan_speed_calibration_adjust_counter));
        #endif
    sysdrv_shmem_data_p->fan_speed_in_transition_counter = 0;
    sysdrv_shmem_data_p->fan_speed_in_transition_counter_reset_value=SYSDRV_FAN_SPEED_IN_TRANSITION_COUNTER_DEFAULT_RESET_VALUE;
    #endif /* end of #if (SYS_HWCFG_FAN_SPEED_DETECT_SUPPORT==TRUE) */
#endif /* #if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE) */

#if (SYS_CPNT_THERMAL_DETECT == TRUE)
    for (i=0; i<SYS_HWCFG_MAX_NBR_OF_THERMAL_PER_UNIT; i++)
    {
        sysdrv_shmem_data_p->sysdrv_thermal_install_status[i] = TRUE;
        sysdrv_shmem_data_p->sysdrv_thermal_temp[i] = 0;
    }
#endif /* end of #if (SYS_CPNT_THERMAL_DETECT == TRUE) */

#if (SYS_CPNT_POWER_DETECT == TRUE)
#if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE)
    for (pwr_idx=1; pwr_idx<=SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT; pwr_idx++)
    {
        if (STKTPLG_BOARD_GetPowerStatusInfo(pwr_idx, &power_reg_info) == TRUE)
        {
            sysdrv_shmem_data_p->sysdrv_local_unit_power_status[pwr_idx-1] =
                (~(power_reg_info.power_status_ok_val)) & (power_reg_info.power_status_mask); /* PSU status not ok */
            sysdrv_shmem_data_p->sysdrv_local_unit_power_status[pwr_idx-1] |=
                (~(power_reg_info.power_is_present_val)) & (power_reg_info.power_present_mask); /* PSU not present */
            sysdrv_shmem_data_p->sysdrv_local_unit_power_status[pwr_idx-1] |=
                (power_reg_info.power_is_alert_val) & (power_reg_info.power_alert_mask); /* PSU is in alert(failed) state */
        }
        else
        {
            BACKDOOR_MGR_Printf("%s(%d)STKTPLG_BOARD_GetPowerStatusInfo failed.(power_index=%hu)\r\n", __FUNCTION__, __LINE__, pwr_idx);
        }
    }
#else /* #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */

    sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0] =  (~SYS_HWCFG_PWR_STATUS_OK) & SYS_HWCFG_PWR_STATUS_MASK; /* main power:not present */

#ifdef SYS_HWCFG_PWR_PRES_MASK
    sysdrv_shmem_data_p->sysdrv_local_unit_power_status[0] |=  (~SYS_HWCFG_PWR_PRES_OK) & SYS_HWCFG_PWR_PRES_MASK;
#endif
#if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2)
    sysdrv_shmem_data_p->sysdrv_local_unit_power_status[1] |=  (~SYS_HWCFG_RPS_PRES_OK) & SYS_HWCFG_RPS_PRES_MASK; /* redundant power:not present */
#ifdef SYS_HWCFG_RPS_STATUS_MASK
    sysdrv_shmem_data_p->sysdrv_local_unit_power_status[1] |=  (~SYS_HWCFG_RPS_STATUS_OK) & SYS_HWCFG_RPS_STATUS_MASK;
#endif
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_POWER_PER_UNIT == 2) */
#endif /* end of #if (SYS_HWCFG_POWER_DETECT_POWER_STATUS_BY_STKTPLG_BOARD == TRUE) */
#endif /* end of #if (SYS_CPNT_POWER_DETECT == TRUE) */

#if (SYS_CPNT_SUPPORT_POWER_MODULE_TYPE==TRUE)
#define INVALID_POWER_TYPE 0XFF
    {
        UI8_T power_index;

        /* init as an invalid power type, so it will notify sysmgr for the
         * detected power type
         */
        for(power_index=0; power_index<SYS_ADPT_MAX_NBR_OF_FAN_PER_UNIT; power_index++)
            sysdrv_shmem_data_p->sysdrv_local_unit_power_type[power_index] = INVALID_POWER_TYPE;
    }
#endif

#if (SYS_CPNT_ALARM_DETECT == TRUE)
    sysdrv_shmem_data_p->sysdrv_majorAlarmType_bitmap=0;
    sysdrv_shmem_data_p->sysdrv_minorAlarmType_bitmap=0;
#endif

#if (SYS_CPNT_ALARM_INPUT_DETECT == TRUE)
    alarm_input_mask = (SYS_HWCFG_SYSTEM_ALARM_INPUT_1_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_2_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_3_MASK|SYS_HWCFG_SYSTEM_ALARM_INPUT_4_MASK);
    sysdrv_shmem_data_p->sysdrv_AlarmInputType_bitmap = ((SYS_HWCFG_SYSTEM_ALARM_INPUT_ASSERTED<<SYS_HWCFG_SYSTEM_ALARM_INPUT_SHIFT)&alarm_input_mask)?0:alarm_input_mask; /* alarm input: not asserted */
#endif

}

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
BOOL_T SYSDRV_GetCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    UI32_T flash_data_count;
    UI32_T read_flash_result;

    read_flash_result = FS_ReadFile(unit,
                                    (UI8_T*)".hw_profile_port_mode",
                                    cfg_hw_port_mode_ar,
                                    SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT,
                                    &flash_data_count);

    /* file size doesn't match, this file might be generated
     * by different version runtime, so ignore it.
     */
    if (flash_data_count != SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        memset(cfg_hw_port_mode_ar, 0, SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);
    }

    if (FS_RETURN_OK == read_flash_result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

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
BOOL_T SYSDRV_SetCfgHwPortMode(UI32_T unit, UI8_T cfg_hw_port_mode_ar[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT])
{
    UI32_T write_flash_result;

    write_flash_result = FS_WriteFile(unit,
                                      (UI8_T*)".hw_profile_port_mode",
                                      (UI8_T*)"HW profile port mode",
                                      FS_FILE_TYPE_PRIVATE,
                                      cfg_hw_port_mode_ar,
                                      SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT,
                                      SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT);


    if (FS_RETURN_OK == write_flash_result)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif /* (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE) */

