/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv.c
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    This file provides the function to initialize resources, start
 *    timer, create task, dispatch message from message queue, and
 *    the service for upper-layer module to access PoE controller.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created
 *    07/01/2009 - Eugene Yu, Porting to Linux platform (stacking not implement yet)
 *
 * Copyright(C)      Accton Corporation, 2009
 *-----------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sys_module.h"
#include "sys_callback_mgr.h"
#include "sysfun.h"
#include "stktplg_pom.h"
#include "leaf_es3626a.h"
#include "leaf_3621.h"
#include "backdoor_mgr.h"
#include "poedrv_type.h"
#include "poedrv.h"
#include "poedrv_board.h"
#include "poedrv_om.h"
#include "poedrv_backdoor.h"
//#include "uartdrv.h"
#include "fs.h"
#include "fs_type.h"
#include "phyaddr_access.h"

#include "l_stdlib.h"

#include "dev_swdrv_pmgr.h"
#include "dev_swdrv.h"
#include "stktplg_board.h"
#include "uc_mgr.h"

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
#include <cpss/generic/dragonite/cpssGenDragonite.h>
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)
#include "i2cdrv.h"
#endif
#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
#include "poe_pom.h"
#endif
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
#include "isc.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/*  #define POEDRV_DEBUG   TRUE           */

#define LOCAL_HOST                       1

#define POEDRV_HW_REVISION  0

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
#define POEDRV_TIME_OUT                  300            /* time to wait for ISC reply */
#define POEDRV_RETRY_TIMES               2
#define POEDRV_ALL_UNIT                  255            /* all unit number  */
#define MASTER_UNIT                      1
#endif /*SYS_CPNT_STACKING*/

#define POEDRV_ENTER_CRITICAL_SECTION        SYSFUN_TakeSem(poedrv_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
#define POEDRV_LEAVE_CRITICAL_SECTION        SYSFUN_GiveSem(poedrv_sem_id);

#define POEDRV_SIZE_OF_PACKET                          sizeof(POEDRV_TYPE_PktBuf_T)
#define POEDRV_NO_OF_POE_PORTS                         SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT
#define POEDRV_ALL_PORTS                               128   /* Physical port ID to set all ports at once  */
#define POEDRV_CHECKSUM_LENGTH                         2    /* 16-bit checksum                            */
#define POEDRV_NO_OF_BYTES_FOR_CHECKSUM                (POEDRV_SIZE_OF_PACKET-POEDRV_CHECKSUM_LENGTH)
#define LOW_BYTE_MASK                                  0x00FF
#define POEDRV_SYSTEM_RESET_DEFAULT                    0xDF    /* 11011111 */
#define POEDRV_SYSTEM_MASK_BIT1_enable                 0x02    /* "Or"  with it*/
#define POEDRV_SYSTEM_MASK_BIT1_disable                0xfd    /* "And" with it*/

/* Definitions of commands
 */
#define PORT_ON                                        1
#define PORT_OFF                                       0

#define MODE_AUTO                                      0
#define MODE_TEST                                      1

#define POEDRV_Malloc(len)                             malloc(len)
#define POEDRV_Free(ptr)                               free(ptr)

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
#define POEDRV_POLLING_POWER_CLASS FALSE  /* Eugene add, for performance issue, class is useless now */
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)
#define POEDRV_POLLING_POWER_CLASS FALSE  /* Eugene add, for performance issue, class is useless now */
#define POEDRV_POLLING_BY_REGISTER TRUE
#endif /* if (SYS_CPNT_POE_INTERFACE != SYS_CPNT_POE_INTERFACE_DRAGONITE) */

/* MACRO FUNCTION DECLARATIONS
 */

#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

/* The macro of declared variables, used for stacking mode
 */
SYSFUN_DECLARE_CSC

/* Macro function to initialize the transmitting and receving buffer with
 */
#define INITIALIZE_PACKET_BUFFER(X,Y)                            \
    {                                                            \
        memset((X), POEDRV_TYPE_SPACE, POEDRV_SIZE_OF_PACKET);   \
        memset((Y), 0, POEDRV_SIZE_OF_PACKET);                   \
    }

/* Macro function to increment echo number by 1, range: 0x0 ~ 0xFE.
 */
#define INCREMENT_ECHO_NUMBER()              \
    {                                        \
        if ( poedrv_echo_number == 0xFE )    \
             poedrv_echo_number = 0;         \
        else                                 \
             poedrv_echo_number ++;          \
    }

/* Macro function to process delay by loop instruction cycle */
#define POEDRV_PROCESS_DELAY(end_instr)                      \
    {                                                        \
        UI32_T st_instr = 0;                                 \
        for (st_instr = 0; st_instr< end_instr; st_instr++); \
    }


#define POEDRV_GPIO_ENTER_CRITICAL_SECTION() SYSFUN_ENTER_CRITICAL_SECTION(poedrv_gpio_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define POEDRV_GPIO_LEAVE_CRITICAL_SECTION() SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_gpio_sem_id)

/* DATA TYPE DECLARATIONS
 */

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))

/* service ID list
 */
typedef enum
{
    POEDRV_USER_PORT_EXISTING = 0,
    POEDRV_SET_PORT_ADMIN_STATUS,
    /*POEDRV_SET_PORT_POWER_DETECTION_CONTROL,*/
    POEDRV_SET_PORT_POWER_PRIORITY,
    POEDRV_SET_PORT_POWER_MAXIMUM_ALLOCATION,
    POEDRV_SET_MAIN_POWER_MAXIMUM_ALLOCATION,
    POEDRV_SOFTWARE_DOWNLOAD,
    POEDRV_GET_POE_SOFTWARE_VERSION,
    POEDRV_PORT_DETECTION_STATUS,
    POEDRV_PORT_STATUS,
    POEDRV_PORT_OVERLOAD_STATUS,
    POEDRV_PORT_FAILURE_STATUS,
    POEDRV_PORT_POWER_CONSUMPTION,
    POEDRV_PORT_POWER_CLASSIFICATION,
    POEDRV_MAIN_PSE_CONSUMPTION,
    POEDRV_IS_MAIN_POWER_REACH_MAXIMUN,
    POEDRV_PSE_OPER_STATUS,
    POEDRV_PROVISION_COMPLETE,
    POEDRV_SET_CAPACITOR_DETECTION_CONTROL,
    POEDRV_SET_CLASS_MODE,
    POEDRV_NBR_OF_SERVICE_ID /*Don't hard code, count on compilier*/
} POEDRV_ServicesID_T;

/* define PoE ISC Payload type
 */
typedef struct
{
    UI8_T    serviceID;      /* Service ID  */
    UI32_T   unit;           /* stack id (unit number) */
    UI32_T   port;           /* port number */
    union
    {
        UI32_T u32;
        BOOL_T boolean;
        UI8_T  u8;
        UI8_T  name_ar[SYS_ADPT_FILE_SYSTEM_NAME_LEN];
        union
        {   /* Event Notification Information */
            UI32_T u32;                  /* ui32 */
            BOOL_T boolean;              /* bool */
#if 0
            UI8_T  legacy_detection_enable;                /* slave change legacy detection */
#endif
        } __attribute__((packed, aligned(1)))notify;

    } __attribute__((packed, aligned(1)))info;

}__attribute__((packed, aligned(1))) POEDRV_Rx_IscBuf_T;

typedef void (*POEDRV_ServiceFunc_t) (ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);

typedef struct
{
    union
    {
        BOOL_T bool;
    } return_value;

    union
    {
        struct
        {
            UI8_T version[2];
            UI8_T build_number;
        } __attribute__((packed, aligned(1)))poe_software_version;
    } __attribute__((packed, aligned(1))) data;
}__attribute__((packed, aligned(1))) POEDRV_IscReplyBuf_T;

#endif

/* Database for mainpower status on PoE system
 */




/* LOCAL SUBPROGRAM DECLARATIONS
 */

#if 0 /* Eugene marked */
static void   POEDRV_InitDataBase(void);
#endif

static void   POEDRV_TaskMain(void);
static BOOL_T POEDRV_MonitorPoeMainpowerStatus(void);
static BOOL_T POEDRV_MonitorPoeOperationStatus(void);
static BOOL_T POEDRV_MonitorPoePortStatus(void);
static BOOL_T POEDRV_MonitorPoePortClassification(void);

static BOOL_T POEDRV_LocalGetPortPowerConsumption(UI32_T unit, UI32_T port, UI32_T *milliwatts);
static BOOL_T POEDRV_LocalGetPortPowerClassification(UI32_T unit, UI32_T port, UI32_T *power_class);
static void   POEDRV_Notify_PortDetectionStatusChange(UI32_T unit, UI32_T port, UI32_T admin_status);
static void   POEDRV_Notify_PortStatusChange(UI32_T unit, UI32_T port, UI32_T actual_port_status);
static void   POEDRV_Notify_PortOverloadStatusChange(UI32_T unit, UI32_T port, BOOL_T is_overload);
static void   POEDRV_Notify_PortFailureStatusChange(UI32_T unit, UI32_T port, BOOL_T is_port_failure);
static void   POEDRV_Notify_PortPowerClassificationChange(UI32_T unit, UI32_T port, UI32_T power_class);
static void   POEDRV_Notify_PortPowerConsumptionChange(UI32_T unit, UI32_T port, UI32_T power_consumption);

#if (SYS_CPNT_POE_POWER_TYPE != SYS_CPNT_POE_POWER_TYPE_PD)
static void   POEDRV_Notify_MainPseConsumptionChange(UI32_T unit, UI32_T power_consumption);
static void   POEDRV_Notify_IsMainPowerReachMaximun(UI32_T unit, UI32_T status);
#endif

static void   POEDRV_Notify_PseOperStatusChange(UI32_T unit, UI32_T oper_status);
#if 0
static void   POEDRV_Notify_LegacyDetectionStatusChange(UI32_T unit, UI8_T oper_status);
#endif

static BOOL_T POEDRV_SendAndWaitReply(POEDRV_TYPE_PktBuf_T *tx_buf, POEDRV_TYPE_PktBuf_T *rx_buf);

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && !defined(ECS4510_12PD)
static UI8_T  POEDRV_CheckReportMessage(POEDRV_TYPE_PktBuf_T *buf);
#endif

#if 0
static BOOL_T POEDRV_SendPacket(UI8_T *tx_buf);
static BOOL_T POEDRV_ReceivePacket(UI8_T *rx_buf);
#endif
static BOOL_T POEDRV_SendSoftwareDownload(void);

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && !defined(ECS4510_12PD)
static UI16_T POEDRV_Checksum(UI8_T *buffer);
#endif

static UI32_T POEDRV_GetLine(UI8_T *buf, UI8_T *line_buf, UI32_T line_size);

#if (POEDRV_POLLING_BY_REGISTER == TRUE)
static BOOL_T POEDRV_SetDefaultValue(void);
#endif

/* Local Subprogram Declaration for PoE stacking management
 */
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
static void SlaveUserPortExisting(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveSetPortAdminStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
#if 0
static void SlaveSetPortPowerDetectionControl(ISC_Key_T *key, L_MREF_T *mem_ref) ;
#endif
static void SlaveSetPortPowerPriority(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveSetPortPowerMaximumAllocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveSetMainpowerMaximum_Allocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);

static void SlaveSoftwareDownload(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveGetPoeSoftwareVersion(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);

static void SlaveProvisionComplete(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveSetCapacitorDetectionControl(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void SlaveSetClassMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortDetectionStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortOverloadStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortFailureStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortPowerConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPortPowerClassification(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterMainPseConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterIsMainPowerReachMaximum(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static void CallbackMasterPseOperStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p);
static UI16_T POEDRV_GetValidDrvUnitBmp(void);
#endif

static BOOL_T POEDRV_LocalUserPortExisting(UI32_T port);
static BOOL_T POEDRV_LocalSetPortAdminStatus(UI32_T port, UI32_T admin_status);
static BOOL_T POEDRV_LocalSetPortPowerPriority(UI32_T port, UI32_T priority);
static BOOL_T POEDRV_LocalSetPortPowerMaximumAllocation(UI32_T port, UI32_T milliwatts);
static BOOL_T POEDRV_LocalSetMainpowerMaximumAllocation(UI32_T milliwatts);
static BOOL_T POEDRV_LocalGetPoeSoftwareVersion(UI8_T *version1, UI8_T *version2, UI8_T *build);
static BOOL_T POEDRV_LocalSoftwareDownload(UI8_T *filename);
static BOOL_T POEDRV_LocalSetClassMode(BOOL_T ClassMode);
static BOOL_T POEDRV_LocalSetCapacitorDetectionControl(UI8_T value);
static void   POEDRV_UpdateStackingInfo(void);


#if 0
static BOOL_T POEDRV_LocalSetPortPowerDetectionControl(UI32_T port,UI32_T mode);
#endif /*Disable*/

#if ((SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && (SYS_CPNT_POE_POWER_TYPE != SYS_CPNT_POE_POWER_TYPE_PD))
static BOOL_T POEDRV_GetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T data_len, UI8_T *data);
static BOOL_T POEDRV_SetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T *data, UI8_T data_len);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T  poedrv_thread_id;       /* PoE driver task ID         */
static UI32_T  poedrv_sem_id;          /* PoE driver semaphore ID    */
static BOOL_T  is_enter_program_mode;  /* Enter program mode         */
static BOOL_T  is_stop_polling;        /* Stop polling controller    */

/* Definition for state transition on a port
 */
static UI32_T  hw_revision = 0;/*To identify if support Power Classification (RON), IDO don't*/
static UI8_T   build_version = 0;/*To identify if use setting power bank rather than power source*/
/* Definitions of callback function list
 */
static SYS_TYPE_CallBack_T *PortDetectionStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PortPowerConsumptionChange_callbacklist;
static SYS_TYPE_CallBack_T *PortPowerClassificationChange_callbacklist;
static SYS_TYPE_CallBack_T *MainPseConsumptionChange_callbacklist;
static SYS_TYPE_CallBack_T *PseOperStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PortOverloadStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PortFailureStatusChange_callbacklist;

#ifdef BES50
static SYS_TYPE_CallBack_T *PortOverPowerManagementStatusChange_callbacklist;
#endif

#if 0
static SYS_TYPE_CallBack_T *Legacy_Detection_callbacklist;
#endif
/*for POE Led*/
static SYS_TYPE_CallBack_T *PortStatusChange_callbacklist;

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))

/* Callback function table for PoE stacking management
 */
static POEDRV_ServiceFunc_t POEDRV_func_tab[] =
{
    SlaveUserPortExisting,                           /*POEDRV_USER_PORT_EXISTING                   */
    SlaveSetPortAdminStatus,                         /*POEDRV_SET_PORT_ADMIN_STATUS                */
    /*SlaveSetPortPowerDetectionControl,*/           /*POEDRV_SET_PORT_POWER_DETECTION_CONTROL     */
    SlaveSetPortPowerPriority,                       /*POEDRV_SET_PORT_POWER_PRIORITY              */
    SlaveSetPortPowerMaximumAllocation,              /*POEDRV_SET_PORT_POWER_MAXIMUM_ALLOCATION    */
    SlaveSetMainpowerMaximum_Allocation,             /*POEDRV_SET_MAIN_POWER_MAXIMUM_ALLOCATION    */
    SlaveSoftwareDownload,                           /*POEDRV_SOFTWARE_DOWNLOAD                    */
    SlaveGetPoeSoftwareVersion,                      /*POEDRV_GET_POE_SOFTWARE_VERSION             */
    CallbackMasterPortDetectionStatus,               /*POEDRV_PORT_DETECTION_STATUS                */
    CallbackMasterPortStatus,                        /*POEDRV_PORT_STATUS                          */
    CallbackMasterPortOverloadStatus,                /*POEDRV_PORT_OVERLOAD_STATUS                 */
    CallbackMasterPortFailureStatus,                 /*POEDRV_PORT_FAILURE_STATUS                  */
    CallbackMasterPortPowerConsumption,              /*POEDRV_PORT_POWER_CONSUMPTION               */
    CallbackMasterPortPowerClassification,           /*POEDRV_PORT_POWER_CLASSIFICATION            */
    CallbackMasterMainPseConsumption,                /*POEDRV_MAIN_PSE_CONSUMPTION                 */
    CallbackMasterIsMainPowerReachMaximum,           /*POEDRV_IS_MAIN_POWER_REACH_MAXIMUN          */
    CallbackMasterPseOperStatus,                     /*POEDRV_PSE_OPER_STATUS                      */
    SlaveProvisionComplete,                          /*POEDRV_PROVISION_COMPLETE                   */
    SlaveSetCapacitorDetectionControl,               /*POEDRV_SET_CAPACITOR_DETECTION_CONTROL      */
    SlaveSetClassMode,                               /*POEDRV_SET_CLASS_MODE                       */

    NULL                                             /*POEDRV_NBR_OF_SERVICE_ID                    */
};

#endif/* #if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)) */

/* semaphore for GPIO
 */
static UI32_T poedrv_gpio_sem_id;


/* Get board id from stktplg and initializes POE database by POEDRV_InitDataBase()
   the following static variables have the same definition to POE database*/




/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : POEDRV_InitiateSystemResources
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T POEDRV_InitiateSystemResources(void)
{
    UI32_T ret_value;
    DBG_PRINT();

#if 0
    /* Initialize pointers of all callback funtions
     */
    PortDetectionStatusChange_callbacklist     = 0;
    PortPowerConsumptionChange_callbacklist    = 0;
    PortPowerClassificationChange_callbacklist = 0;
    MainPseConsumptionChange_callbacklist      = 0;
    PseOperStatusChange_callbacklist           = 0;
    PortOverloadStatusChange_callbacklist      = 0;
    PortFailureStatusChange_callbacklist       = 0;

#ifdef BES50
    PortOverPowerManagementStatusChange_callbacklist    = 0;
#endif

#if 0
    Legacy_Detection_callbacklist              = 0;
#endif
    /*For POE Led*/
    PortStatusChange_callbacklist = 0;

    /* Initialize operation state
     */
#endif
    /* Initialize echo sequence number, starting from 0
     */
    is_stop_polling       = TRUE;
    is_enter_program_mode = FALSE;

    POEDRV_OM_Init();

    /* Create semaphore
     */
    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

#if 0
#ifndef INCLUDE_DIAG
    /* Register a backdoor debug function
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("poedrv", POEDRV_BACKDOOR_Main);
#endif
    /* After running provision, we should do hardware reset again
     * So Move POEDRV_HardwareReset() to EnterTransitionMode
     */
    /* Release hardware reset to PoE controller
     */
#if 0
    POEDRV_HardwareReset();
#endif
#endif
    return (TRUE);

} /* End of POEDRV_InitiateSystemResources() */

/* FUNCTION NAME : POEDRV_AttachSystemResources
 * PURPOSE: Attach system resource for POEDRV
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_AttachSystemResources(void)
{
    UI32_T ret_value;

    DBG_PRINT();
    POEDRV_OM_AttachSystemResources();

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM, &poedrv_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_GPIO, &poedrv_gpio_sem_id)) != SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__,ret_value);
    }

} /* End of POEDRV_AttachSystemResources() */

/* FUNCTION NAME : POEDRV_SetTransitionMode
 * PURPOSE: This function is used to set POEDRV in transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_SetTransitionMode(void)
{
    DBG_PRINT();

    POEDRV_OM_SetTransitionMode();

} /* End of POEDRV_SetTransitionMode() */


/* FUNCTION NAME : POEDRV_EnterTransitionMode
 * PURPOSE: This function is used to force POEDRV to enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterTransitionMode(void)
{
    DBG_PRINT();

    /* Set provision complete flag to FALSE.
     */
    POEDRV_OM_SetProvisionComplete(FALSE);

    /* Start monitor functions when PoE system is working on "NORMAL" status.
     */
    POEDRV_OM_SetStopMonitorFlag(TRUE);

    /* Initialize database with default value.
     */
    POEDRV_OM_Reset();

    /* Move from POEDRV_Initiate_System_Resources()
     */
#if 0 /* do it in dev_swdrv_init phase */
    POEDRV_ReleaseSoftwareReset(FALSE);
    POEDRV_HardwareReset();

    DEV_SWDRV_PMGR_TwsiInit();
    DEV_SWDRV_PMGR_MSCC_POE_Init();
#endif

    /* Initialize database with board id.
     */
    POEDRV_OM_ResetByBoardID();

    POEDRV_OM_EnterTransitionMode();
    POEDRV_ReleaseSoftwareReset(FALSE);
} /* End of POEDRV_EnterTransitionMode() */


/* FUNCTION NAME : POEDRV_EnterMasterMode
 * PURPOSE: This function is used to force POEDRV to enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterMasterMode(void)
{
    UI8_T version_1 = 0,version_2 = 0, build = 0;

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = NULL;
    UC_MGR_Sys_Info_T sysinfo;
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI8_T max_pse_port = 0;
#endif

#if (POEDRV_POLLING_BY_REGISTER == TRUE)
    UI8_T set_default_count = 0;
#endif

    DBG_PRINT();

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)

    /* Configure maximum pse ports by BID.
     */
    if (UC_MGR_GetSysInfo(&sysinfo) == FALSE)
    {
        printf("%s(%d): UC_MGR_GetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
        while(1);
    }
    memset(&board_info, 0x0, sizeof(STKTPLG_BOARD_BoardInfo_T));
    STKTPLG_BOARD_GetBoardInformation(sysinfo.board_id, &board_info);
    max_pse_port = board_info.max_pse_port_number;

    if(max_pse_port)
    {
        dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));
        if (dragonitep)
        {
            POEDRV_ENTER_CRITICAL_SECTION;
            DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_INIT_SYSTEM, 0, max_pse_port, 0, dragonitep);
            POEDRV_LEAVE_CRITICAL_SECTION;
            free(dragonitep);
        }
    }
#endif

//    SYSFUN_ENTER_MASTER_MODE();
//Eugene temp,    hw_revision = *( (UI8_T *)SYS_HWCFG_HARDWARE_REVISION_ADDR );
    /*Get Version and Build to use in set main power allocation*/
    if (POEDRV_LocalGetPoeSoftwareVersion(&version_1, &version_2, &build) == FALSE)
    {
        SYSFUN_Debug_Printf("\n\r  Get Version Failed !!!");
    }
    else
    {
//printf("r\n\r\nversion:0x%02x, build: 0x%02x\r\n\r\n", version_2, build);
       POEDRV_OM_SetImageVersion(version_1, version_2);
       POEDRV_OM_SetImageBuild(build);
    }

    if (hw_revision ==POEDRV_HW_REVISION) build_version=build;/*Only in RON, we need build*/

#if (POEDRV_POLLING_BY_REGISTER == TRUE)

    /* Set Default-Value to micro-p
     */
    for (set_default_count=1;set_default_count<=5;set_default_count++)
    {
        if ( POEDRV_SetDefaultValue() == TRUE )
        {
            break;
        }
        else
        {
            if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
            {
                printf("\n\rPOEDRV_SetDefaultValue fail,will try again!");
            }
        }
    }
#endif

    POEDRV_OM_EnterMasterMode();
} /* End of POEDRV_EnterMasterMode() */


/* FUNCTION NAME : POEDRV_EnterSlaveMode
 * PURPOSE: This function is used to force POEDRV to enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterSlaveMode(void)
{
    UI8_T version_1,version_2,build;

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = NULL;
    UC_MGR_Sys_Info_T sysinfo;
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI8_T max_pse_port = 0;
#endif

    DBG_PRINT();
    /* Initialize database with default value
     */
    POEDRV_OM_Reset();

    SYSFUN_ENTER_SLAVE_MODE();

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)

    /* Configure maximum pse ports by BID.
     */
    if (UC_MGR_GetSysInfo(&sysinfo) == FALSE)
    {
        printf("%s(%d): UC_MGR_GetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
        while(1);
    }
    memset(&board_info, 0x0, sizeof(STKTPLG_BOARD_BoardInfo_T));
    STKTPLG_BOARD_GetBoardInformation(sysinfo.board_id, &board_info);
    max_pse_port = board_info.max_pse_port_number;

    if(max_pse_port)
    {
        dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));
        if (dragonitep)
        {
            POEDRV_ENTER_CRITICAL_SECTION;
            DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_INIT_SYSTEM, 0, max_pse_port, 0, dragonitep);
            POEDRV_LEAVE_CRITICAL_SECTION;
            free(dragonitep);
        }
    }
#endif

//Eugene temp,    hw_revision = *( (UI8_T *)SYS_HWCFG_HARDWARE_REVISION_ADDR );
    /*Get Version and Build to use in set main power allocation*/
    if (POEDRV_LocalGetPoeSoftwareVersion(&version_1, &version_2, &build) == FALSE)
    {
        SYSFUN_Debug_Printf("\n\r  Get Version Failed !!!");
    }
    else
    {
       POEDRV_OM_SetImageVersion(version_1, version_2);
       POEDRV_OM_SetImageBuild(build);
    }

    if (hw_revision ==POEDRV_HW_REVISION) build_version=build;/*Only in RON, we need build*/

    POEDRV_OM_EnterSlaveMode();
} /* End of POEDRV_EnterSlaveMode() */


/* FUNCTION NAME : POEDRV_CreateTasks
 * PURPOSE: This function is used to create the main task of PoE driver.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T POEDRV_CreateTasks(void)
{
    UI32_T thread_id;

    DBG_PRINT();
    if ( SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_POEDRV_TASK,
                            24 * SYS_TYPE_1K_BYTES,
                            SYSFUN_TASK_NO_FP,
                            POEDRV_TaskMain,
                            NULL,
                            &thread_id) != SYSFUN_OK )
    {
        printf("\n%s: Spawn POEDRV thread fail.\n", __FUNCTION__);

        return FALSE;
    }

    poedrv_thread_id = thread_id;

    return(TRUE);

} /* End of POEDRV_CreateTasks() */


/* FUNCTION NAME : POEDRV_Register_PortDetectionStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If status of a port has been changed, the registered function
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T detection_status)
 *          unit -- unit ID
 *          port -- port ID
 *          detection_status -- detection status of a port
 */
void POEDRV_Register_PortDetectionStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T detection_status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortDetectionStatusChange_callbacklist);

} /* End of POEDRV_Register_PortConsumptionChange_CallBack() */

/*for POE Led*/
void POEDRV_Register_PortStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T actual_port_status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortStatusChange_callbacklist);
}

/* FUNCTION NAME : POEDRV_Register_PortPowerConsumptionChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If status of a port has been changed, the registered function
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T power_consumption)
 *          unit -- unit ID
 *          port -- port ID
 *          power_consumption -- detection status of a port
 */
void POEDRV_Register_PortPowerConsumptionChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T power_consumption))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortPowerConsumptionChange_callbacklist);

} /* End of POEDRV_Register_PortPowerConsumptionChange_CallBack() */

/* FUNCTION NAME : POEDRV_Register_PortOverloadStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If overload status of a port has been changed, the registered
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_overload)
 *          unit -- unit ID
 *          port -- port ID
 *          is_overload -- overload status of a port
 */
void POEDRV_Register_PortOverloadStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_overload))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortOverloadStatusChange_callbacklist);

} /* End of POEDRV_Register_PortOverloadStatusChange_CallBack() */

#ifdef BES50
/* FUNCTION NAME : POEDRV_Register_PortOverPowerManagementStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If overload status of a port has been changed, the registered
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_over_power_budget)
 *          unit -- unit ID
 *          port -- port ID
 *          is_over_power_budget -- over power budget
 */
void POEDRV_Register_PortOverPowerManagementStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_over_power_budget))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortOverPowerManagementStatusChange_callbacklist);

} /* End of POEDRV_Register_PortOverPowerManagementStatusChange_CallBack() */
#endif

/* FUNCTION NAME : POEDRV_Register_PortFailureStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If port failure status has been changed, the registered
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_port_failure)
 *          unit -- unit ID
 *          port -- port ID
 *          is_port_failure -- port failure status
 */
void POEDRV_Register_PortFailureStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_port_failure))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortFailureStatusChange_callbacklist);

} /* End of POEDRV_Register_PortFailureStatusChange_CallBack() */


/* FUNCTION NAME : POEDRV_Register_PortPowerClassificationChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If power class of a port has been changed, the registered function
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T power_class)
 *          unit -- unit ID
 *          port -- port ID
 *          power_class -- power class of a port
 */
void POEDRV_Register_PortPowerClassificationChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T power_class))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PortPowerClassificationChange_callbacklist);

} /* End of POEDRV_Register_PortPowerClassificationChange_CallBack() */


/* FUNCTION NAME : POEDRV_Register_MainPseConsumptionChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If status of main power has been changed, the registered function
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T power_consumption)
 *          unit -- unit ID
 *          power_consumption -- main PSE power consumption
 */
void POEDRV_Register_MainPseConsumptionChange_CallBack(void (*fun)(UI32_T unit, UI32_T power_consumption))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(MainPseConsumptionChange_callbacklist);

} /* End of POEDRV_Register_MainPseConsumptionChange_CallBack() */


/* FUNCTION NAME : POEDRV_Register_PseOperStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If the operational status of the main PSE has been changed,
 *          the registered function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T oper_status)
 *          unit -- unit ID
 *          oper_status -- VAL_pethMainPseOperStatus_on
 *                         VAL_pethMainPseOperStatus_off
 *                         VAL_pethMainPseOperStatus_faulty
 */
void POEDRV_Register_PseOperStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T oper_status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PseOperStatusChange_callbacklist);

} /* End of POEDRV_Register_PseOperStatusChange_CallBack() */


#if 0
/* FUNCTION NAME : POEDRV_Register_Legacy_Detection_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If the operational status of the legacy detection falg has
            been changed,  the registered function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T oper_status)
 *          unit -- unit ID
 *          oper_status -- 1 :enable
 *                         0 :disable
 *
 */
void POEDRV_Register_Legacy_Detection_CallBack(void (*fun)(UI32_T unit, UI8_T oper_status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PseOperStatusChange_callbacklist);

} /* End of POEDRV_Register_PseOperStatusChange_CallBack() */
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port of poe device is existing
 * INPUT   : unit -- unit ID
 *           port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POEDRV_UserPortExisting(UI32_T unit, UI32_T port)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_USER_PORT_EXISTING)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_USER_PORT_EXISTING;
            isc_buf_p->port = port;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */
            if (FALSE == POEDRV_LocalUserPortExisting(port))
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;
}

/* FUNCTION NAME : POEDRV_LocalUserPortExisting
 * PURPOSE: This function will return if this user port of poe device is
 *          existing
 * INPUT:   port       -- port number
 * OUTPUT:  None
 * RETURN:  TRUE  -- Existing
 *          FALSE -- Not Existing
 * NOTES:
 *
 */
static BOOL_T POEDRV_LocalUserPortExisting(UI32_T port)
{
    UI32_T phy_port = 0;

    return POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port);
} /* End of POEDRV_LocalUserPortExisting() */


/* FUNCTION NAME : POEDRV_SetPortAdminStatus
 * PURPOSE: This function is used to enable or disable power and detection mechanism
 *          for a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID, 0 means setting all ports at once.
 *          admin_status -- VAL_pethPsePortAdminEnable_rrue
 *                          VAL_pethPsePortAdminEnable_false
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 *
 */
BOOL_T POEDRV_SetPortAdminStatus(UI32_T unit, UI32_T port, UI32_T admin_status)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_ADMIN_STATUS)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_SET_PORT_ADMIN_STATUS;
            isc_buf_p->port = port;
            isc_buf_p->info.u32 = admin_status;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */
            if (POEDRV_LocalSetPortAdminStatus(port, admin_status) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;
} /* End of POEDRV_SetPortAdminStatus() */

/* FUNCTION NAME : POEDRV_LocalSetPortAdminStatus
 * PURPOSE: This function is used to enable or disable power and detection mechanism
 *          for a port.
 * INPUT:   port -- port ID, 0 means setting all ports at once.
 *          admin_status -- VAL_pethPsePortAdminEnable_true
 *                          VAL_pethPsePortAdminEnable_false
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 *
 */

static BOOL_T POEDRV_LocalSetPortAdminStatus(UI32_T port, UI32_T admin_status)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    /* Check illegal port ID
     */
    if ((port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER))
    {
        free(dragonitep);
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_SET_PORT_ADMIN, 0, port, (UI32_T) ((admin_status == TRUE) ? CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_ENABLED_E : CPSS_GEN_DRAGONITE_PORT_CTRL_STATUS_DISABLED_E), dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    free(dragonitep);

    POEDRV_OM_SetPortInfoAdminStatus(port, admin_status);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)
#ifdef ECS4510_12PD
    UI8_T write_data=0;

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {
         return FALSE;
    }
/* disable PD69101, set bit0 to 1, bit1 to 0
 * enable  PD69101, set bit0 to 0, bit1 to 0
 */
    if(admin_status == VAL_pethPsePortAdminEnable_true)
        write_data = 0x0;
    else// if(admin_status == VAL_pethPsePortAdminEnable_false)
        write_data = 0x1;

    I2CDRV_SetI2CInfo(SYS_HWCFG_I2C_SLAVE_PCA9538PWR, 0x1, 0x1, &write_data);

    POEDRV_OM_SetPortInfoAdminStatus(port, admin_status);
    return TRUE;
#elif defined(ES4552MA_HPOE)
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    UI32_T phy_port = 0;       /* Physical port ID. */
    BOOL_T ret = FALSE;


    /* Check illegal port ID
     */
    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        return FALSE;

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_ON_OFF;
    transmit.subject2     = (UI8_T) (port - 1);

    if ( admin_status == VAL_pethPsePortAdminEnable_true )
        transmit.data6 = PORT_ON;
    else if ( admin_status == VAL_pethPsePortAdminEnable_false )
        transmit.data6 = PORT_OFF;
    else
        return FALSE;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("\n\r***Failed in the POEDRV_LocalSetPortAdminStatus!***");
    }

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

#endif

} /* End of POEDRV_LocalSetPortAdminStatus() */

#if 0   /*Disable*/
/* FUNCTION NAME : POEDRV_SetPortPowerDetectionControl
 * PURPOSE: This function is used to Controls the power detection mechanism
 *          of a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          mode -- VAL_pethPsePortPowerDetectionControl_auto
 *                  VAL_pethPsePortPowerDetectionControl_test
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   auto: enables the power detection mechanism of the port.
 *          test: force continuous discovery without applying power
 *          regardless of whether PD detected.
 */
BOOL_T POEDRV_SetPortPowerDetectionControl(UI32_T unit, UI32_T port, UI32_T mode)
{

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    POEDRV_Rx_IscBuf_T   req_buf;
    UI8_T   rep_buf;
#endif

    /* Check operating mode
     */

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
     /*   EH_MGR_Handle_Exception(SYS_MODULE_SWITCH, POEDRV_ENTER_MASTER_MODE_FUNC_NO,
                EH_TYPE_MSG_NOT_IN_MASTER_MODE, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_DEBUG));
      */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))

        if ( poedrv_mainpower_info.unit_id  != unit )
        {
            /* set remote unit port Power Detection Controlt
             */
            req_buf.serviceID = POEDRV_SET_PORT_POWER_DETECTION_CONTROL;
            req_buf.port = port;
            req_buf.info.mode = mode;

            if (ISC_Remote_Call(unit, ISC_POEDRV_SID, sizeof(req_buf), (UI8_T *)&req_buf,
                                sizeof(rep_buf), (UI8_T *)&rep_buf, POEDRV_RETRY_TIMES , POEDRV_TIME_OUT))
            {

                if (rep_buf==TRUE)
                {
                    return TRUE;
                }
                /* EH not implemented yet
                 */
                /*
                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_SET_PORT_EGRESS_RATE_LIMIT_FUNC_NO,
                    EH_TYPE_MSG_REM_PORT_SET, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                    buff1, buff2);
                */
                return FALSE;
            }

            /* EH not implemented yet
             */

            /*
            EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_SET_PORT_EGRESS_RATE_LIMIT_FUNC_NO,
                EH_TYPE_MSG_REM_PORT_SET, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
            */
            return FALSE; /* can't config remote */
        }
        else
        {
#endif /*SYS_CPNT_STACKING*/
            /* if local unit or standalone
             */


            if (POEDRV_LocalSetPortPowerDetectionControl(port,mode) == FALSE)
            {
            /* EH not implemented yet
             */
            /*
                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_DIS_PORT_ADMIN_FUNC_NO,
                                EH_TYPE_MSG_LOC_DIS, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                buff1, buff2);
            */
                return FALSE;
            }
            return TRUE;
        }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/

    return FALSE;
} /* End of POEDRV_SetPortPowerDetectionControl() */
#endif /*Disable*/


/* FUNCTION NAME: POEDRV_SetPortPowerPriority
 * PURPOSE: This function is used to set a priority to a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          priority -- VAL_pethPsePortPowerPriority_critical
 *                      VAL_pethPsePortPowerPriority_high
 *                      VAL_pethPsePortPowerPriority_low
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   The priority that is set by this variable could be used by a
 *          control mechanism that prevents over current situations by
 *          disconnecting first ports with lower power priority.
 */
BOOL_T POEDRV_SetPortPowerPriority(UI32_T unit, UI32_T port, UI32_T priority)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_POWER_PRIORITY)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_SET_PORT_POWER_PRIORITY;
            isc_buf_p->port = port;
            isc_buf_p->info.u32 = priority;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */

            /* if local unit or standalone
             */
            if (POEDRV_LocalSetPortPowerPriority(port, priority) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;
} /* End of POEDRV_SetPortPowerPriority() */



/* FUNCTION NAME: POEDRV_localSetPortPowerPriority
 * PURPOSE: This function is used to set a priority to a port.
 * INPUT:   port -- port ID
 *          priority -- VAL_pethPsePortPowerPriority_critical
 *                      VAL_pethPsePortPowerPriority_high
 *                      VAL_pethPsePortPowerPriority_low
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   The priority that is set by this variable could be used by a
 *          control mechanism that prevents over current situations by
 *          disconnecting first ports with lower power priority.
 */
static BOOL_T POEDRV_LocalSetPortPowerPriority(UI32_T port, UI32_T priority)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    /* Check illegal port ID
     */
    if ((port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER))
    {
        free(dragonitep);
        return FALSE;
    }

    priority -= 1;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_SET_PORT_PRIORITY, 0, port, priority, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    UI32_T phy_port = 0; /* Physical port ID. */
    BOOL_T ret = FALSE;

    /* Check illegal port ID
     */
    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        return FALSE;

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_PRIORITY;
    transmit.subject2     = (UI8_T) (port - 1);
    transmit.data6        = (UI8_T)priority;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): PORT = %lu, PHY = %lu, PRIORITY = %lu, RET = \"%s\".\r\n", __FUNCTION__, __LINE__, port, phy_port, priority, (ret == TRUE) ? "PASS" : "FAIL");

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

#endif

} /* End of POEDRV_LocalSetPortPowerPriority() */

/* FUNCTION NAME: POEDRV_SetPortPowerMaximumAllocation
 * PURPOSE: This function is used to set a specified port the maximum power
 *          in milliwatts.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          milliwatts -- power limit
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 3000 to 21000 milliwatts.
 */
BOOL_T POEDRV_SetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T milliwatts)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_POWER_MAXIMUM_ALLOCATION)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_SET_PORT_POWER_MAXIMUM_ALLOCATION;
            isc_buf_p->port = port;
            isc_buf_p->info.u32 = milliwatts;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */


            if (POEDRV_LocalSetPortPowerMaximumAllocation(port, milliwatts) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;

} /* End of POEDRV_SetPortPowerMaximumAllocation() */

/* FUNCTION NAME: POEDRV_LocalSetPortPowerMaximumAllocation
 * PURPOSE: This function is used to set a specified port the maximum power
 *          in milliwatts.
 * INPUT:   port -- port ID
 *          milliwatts -- power limit
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 3000 to 21000 milliwatts.
 */
static BOOL_T POEDRV_LocalSetPortPowerMaximumAllocation(UI32_T port, UI32_T milliwatts)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    /* Check illegal port ID
     */
    if ((port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER))
    {
        free(dragonitep);
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_SET_PORT_POWER_MAXIMUM_ALLOCATION, 0, port, milliwatts / 100, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    UI32_T phy_port = 0; /* Physical port ID. */
    UI8_T *power_limit = (UI8_T *) &milliwatts;
    BOOL_T ret = FALSE;


    /* Check illegal port ID
     */
    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
              return FALSE;

#if (SYS_CPNT_POE_ASIC_POWERDSINE_PD69012_PSE_POWER_MAXIMUM_ALLOCATION_WORKAROUND == TRUE)
UI32_T board_id = 0xFF;
UI8_T data[2] = {0xFF, 0xFF};
UI16_T *data_ptr = (UI16_T *) &data[0];
double power_cut_off = 0.0, vmain = 0.0, vscale = 44.25;


    /* Microsemi FAE John:
     * PPL: SW setting per port power limit(W).
     * Pcut: real cut-off power read by SW(W).
     * Vmain: power supply voltage read by SW(V).
     * LGE1: PPL = (Pcut * 44 / Vmain) - 1 or Pcut = Vmain * (PPL + 1) / 44.
     */
    POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
    if ((ret = POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x105C, sizeof(data), data)) == TRUE)
    {
        vmain = (double) *data_ptr * 0.061; /* Resolution = 61mV. */
        power_cut_off = ((double) milliwatts * vscale) / vmain - 1000.0; /* power_cut_off = XXXmW. */
        milliwatts = (UI32_T) power_cut_off;
    }
    else
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
            printf("\n\r***Failed in the POEDRV_LocalSetPortPowerMaximumAllocation!***");
    }
#endif

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = (UI8_T) (port - 1);

#if (SYS_HWCFG_LITTLE_ENDIAN_CPU == TRUE)                /*  Andy */
    transmit.data6        = power_limit[1];
    transmit.data7        = power_limit[0];
#else
    transmit.data6        = power_limit[2];
    transmit.data7        = power_limit[3];
#endif

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("\n\r***Failed in the POEDRV_LocalSetPortPowerMaximumAllocation!***");
    }

    /* Please refer 06-0028-081TN_144_Automode_PM_PD690xx.pdf.
     * After the port successfully passes the startup phase, its maximum power consumption level is limited to the maximum threshold value, as set in the Port Power Limit ¡§TPPL¡¨ register.
     * During on-going port operation, the port power consumption threshold may be modified by changing the TPPL register's value. The TPPL Register's content can be changed via the Host MCU (see Register 134C to 1362 in this document).
     */

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_TEMP_SUPPLY;
    transmit.subject2     = (UI8_T) (port - 1);

#if (SYS_HWCFG_LITTLE_ENDIAN_CPU == TRUE)                /*  Andy */
    transmit.data6        = power_limit[1];
    transmit.data7        = power_limit[0];
#else
    transmit.data6        = power_limit[2];
    transmit.data7        = power_limit[3];
#endif

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
            printf("\n\r***Failed in the POEDRV_LocalSetPortPowerMaximumAllocation!***");
    }

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

#endif

} /* End of POEDRV_LocalSetPortPowerMaximumAllocation() */

/* FUNCTION NAME: POEDRV_SetMainpowerMaximumAllocation
 * PURPOSE: This function is used to set the power, available for Power
 *          Management on PoE.
 * INPUT:   unit -- unit ID
 *          milliwatts -- power available on PoE system
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 36,000 to 800,000 milliwatts.
 */
BOOL_T POEDRV_SetMainpowerMaximumAllocation(UI32_T unit, UI32_T milliwatts)
{
    UI32_T  max_main_power_allocation = 0;

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T board_id;
    BOOL_T local_power = TRUE;
#endif

#if (SYS_CPNT_POE_PSE_RPS_LOCAL_POWER_DIF == TRUE)
    STKTPLG_OM_GetUnitBoardID(unit, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    POE_POM_GetUseLocalPower(unit, &local_power);

    #if (SYS_CPNT_POE_MAX_ALLOC_FIXED == TRUE)
    /* called from POE_MGR_PowerStatusChanged_CallBack() */
    max_main_power_allocation = (local_power? board_info.main_pse_power_max_allocation: board_info.main_pse_power_max_allocation_rps);
    #else
    /* called from POE_MGR_SetMainpowerMaximumAllocation() by cli/web */
    POEDRV_OM_GetMainPowerMaxAllocation(&max_main_power_allocation);
    #endif
#else
    POEDRV_OM_GetMainPowerMaxAllocation(&max_main_power_allocation);
#endif

    if(milliwatts > max_main_power_allocation || milliwatts < SYS_HWCFG_MIN_POWER_ALLOCATION)
        return FALSE;

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_MAIN_POWER_MAXIMUM_ALLOCATION)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_SET_MAIN_POWER_MAXIMUM_ALLOCATION;
            isc_buf_p->info.u32 = milliwatts;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */

            if (POEDRV_LocalSetMainpowerMaximumAllocation(milliwatts) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;

} /* End of POEDRV_SetMainpowerMaximumAllocation() */
/* FUNCTION NAME: POEDRV_LocalSetMainpowerMaximumAllocation
 * PURPOSE: This function is used to set the power, available for Power
 *          Management on PoE.
 * INPUT:   milliwatts -- power available on PoE system
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 36,000 to 800,000 milliwatts.
 */
static BOOL_T POEDRV_LocalSetMainpowerMaximumAllocation(UI32_T milliwatts)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));

    if (!dragonitep)
        return FALSE;

    /* Check operating mode
     */
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        free(dragonitep);
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_SET_SYSTEM_MAINPOWER_MAXIMUM_ALLOCATION, 0, 0, milliwatts / 100/*units: 0.1w*/, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    UI32_T mainpower = L_STDLIB_Hton32(milliwatts / 1000); /* Milliwatts power style. */
    UI8_T *power_limit = (UI8_T *)&mainpower;
    BOOL_T ret = FALSE;
    UI8_T power_bank = 0;


    /* Check operating mode
     */
    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {
         return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_SUBJECT2_MAIN;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
            printf("\n\r***Failed in the POEDRV_LocalSetMainpowerMaximumAllocation!***");
    }

    /* Get current power bank
    */
    power_bank = receive.data10;

    /* Set Power Budget */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
         */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_POWER_BUDGET;
    transmit.data6        = power_bank;
    transmit.data7        = power_limit[2];
    transmit.data8        = power_limit[3];
    transmit.data9        = POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_H;
    transmit.data10       = POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_L;
    transmit.data11       = POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_H;
    transmit.data12       = POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_L;
    transmit.data13       = (UI8_T) POEDRV_TYPE_PSE_POWER_GUARD_BAND;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
            printf("\n\r***Failed in the POEDRV_LocalSetMainpowerMaximumAllocation!***");
    }

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

#endif

} /* End of POEDRV_LocalSetMainpowerMaximumAllocation() */

/* get remote info  are stroed at Master OM
 */

/* FUNCTION NAME: POEDRV_GetPortPowerConsumption
 * PURPOSE: This function is used to get power consumption of a port from
 *          local database.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_consumption -- power consumption of a port in milliwatts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortPowerConsumption(UI32_T unit, UI32_T port, UI32_T *power_consumption)
{
    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {

         return FALSE;
    }

    POEDRV_OM_GetPortInfoPowerConsumption(port, power_consumption);

    return TRUE;

} /* End of POEDRV_GetPortPowerConsumption() */

/* FUNCTION NAME: POEDRV_GetPortPowerClassification
 * PURPOSE: This function is used to get power classfication of a port from
 *          local database.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_class -- power classification of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortPowerClassification(UI32_T unit, UI32_T port, UI32_T *power_class)
{
   /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {

         return FALSE;
    }

    POEDRV_OM_GetPortInfoPowerClass(port, power_class);

    return TRUE;


}/* End of POEDRV_GetPortPowerClassification() */
/* get remote info  are stroed at Master OM
 */

/* FUNCTION NAME: POEDRV_GetMainpowerConsumption
 * PURPOSE: This function is used to get mainpower consumption on PoE.
 * INPUT:   unit -- unit ID
 * OUTPUT:  watts -- power consumption of PoE in milliWatts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetMainpowerConsumption(UI32_T unit, UI32_T *power_consumption)
{
    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    POEDRV_OM_GetMainPowerInfoMainConsumption(power_consumption);

    return TRUE;

} /* End of POEDRV_GetMainpowerConsumption() */


/* FUNCTION NAME: POEDRV_GetPortStatus
 * PURPOSE: This function is used to get status of a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  port_status -- data buffer for status of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortStatus(UI32_T unit, UI32_T port, UI8_T *port_status)
{
    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {

         return FALSE;
    }

    POEDRV_OM_GetPortInfoActualStatus(port, port_status);

    return TRUE;

} /* End of POEDRV_GetPortsStatus() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_GetPortIsSupportPoe
 * -------------------------------------------------------------------------
 * PURPOSE: This function is used to check whether the port support POE
 * INPUT:   port -- port ID
 * OUTPUT:
 * RETURN:  TRUE: support POE,
 *          FALSE: Not support POE
 * NOTES:   2006.11.29 aken, for WEB check port
 * -------------------------------------------------------------------------*/
BOOL_T POEDRV_GetPortIsSupportPoe(UI32_T port)
{
    if(port > 0 && port <= POEDRV_NO_OF_POE_PORTS)
    {
        UI32_T phy_port;

        if(!POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port))
            return TRUE;
        else
            return FALSE;
    }
    else
        return FALSE;

} /* End of POEDRV_GetPortIsSupportPoe() */

/* FUNCTION NAME: POEDRV_GetPortPriority
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortPriority(UI32_T unit, UI32_T port, UI8_T *priority)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    UI32_T                 phy_port;       /* Physical port ID      */
    BOOL_T                 ret;

    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {

         return FALSE;
    }

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {

         return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_PRIORITY;
    transmit.subject2     = (UI8_T)phy_port;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_GetPortPriority!***");
#endif
    }

    if ( ret )
    {
         *priority = receive.main_subject;
    }


    return ret;

} /* End of POEDRV_GetPortPriority() */


/* FUNCTION NAME: POEDRV_GetPortPowerMaximumAllocation
 * PURPOSE: This function is used to get power limit for a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_limit -- port defined power in milliwatts.
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T *power_limit)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    UI32_T                 phy_port;       /* Physical port ID      */
    /*UI8_T                  *value = (UI8_T *)power_limit;*/
    UI32_T               temp_val = 0;
    UI8_T                  *value = (UI8_T *)&temp_val;
    BOOL_T                 ret;

    /* Check operating mode
     */


#ifndef INCLUD_DIAG
    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }
#endif

    //wuli 1222 init power_limit
    memset(value , 0x0, sizeof(UI32_T));    // wuli, add

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {

         return FALSE;
    }

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {

         return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = (UI8_T)phy_port;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_GetPortPowerMaximumAllocation!***");
#endif
    }

    if ( ret )
    {
         value[2] = receive.main_subject;
         value[3] = receive.subject1;
         temp_val = L_STDLIB_Ntoh32(temp_val);
         *power_limit = temp_val;  /* wuli, 1225  ,Andy  */
    }


    return ret;

} /* End of POEDRV_GetPortPowerMaximumAllocation() */


/* FUNCTION NAME: POEDRV_GetMainPowerParameters
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetMainPowerParameters(UI32_T unit, UI32_T *power_available)
{
    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    POEDRV_OM_GetMainPowerInfoMainPower(power_available);

    return TRUE;

} /* End of POEDRV_GetMainPowerParameters() */

/* FUNCTION NAME : POEDRV_SetSystemMask
 * PURPOSE: This function is used to set system mask
 * INPUT:   system_mask
 * OUTPUT:  none
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SetSystemMask(UI8_T system_mask)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }


    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_MASKZ;
    transmit.subject2     = system_mask;


    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_SetSystemMask!***");
#endif
    }


    return ret;

} /* End of POEDRV_GetSystemMask() */
/* FUNCTION NAME : POEDRV_GetSystemMask
 * PURPOSE: This function is used to query system mask
 * INPUT:   None
 * OUTPUT:  system_mask : system mask on PoE controller
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetSystemMask(UI8_T *system_mask)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_MASKZ;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_GetSystemMask!***");
#endif
    }

    if ( ret )
    {
         *system_mask = receive.main_subject;
    }

    return ret;

} /* End of POEDRV_GetSystemMask() */


/* FUNCTION NAME : POEDRV_GetPoeSoftwareVersion
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : unit -- unit ID
 * OUTPUT  : version1 -- version number 1
 *           version2 -- version number 2
 *           build    -- build number
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPoeSoftwareVersion(UI32_T unit, UI8_T *version1, UI8_T *version2, UI8_T *build)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_POE_ASIC_WITHOUT_MCU == TRUE)
        *version1 = 1;
        *version2 = 0;
        *build    = 4;
#else
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_GET_POE_SOFTWARE_VERSION)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_GET_POE_SOFTWARE_VERSION;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;

            #if (SYS_HWCFG_LITTLE_ENDIAN_CPU == TRUE)                               /*  Andy  */
            *version1 = isc_reply.data.poe_software_version.version[1];
            *version2 = isc_reply.data.poe_software_version.version[0];
            #else
            *version1 = isc_reply.data.poe_software_version.version[0];
            *version2 = isc_reply.data.poe_software_version.version[1];
            #endif
            *build = isc_reply.data.poe_software_version.build_number;

            if (*version1!=0 && *version2!=0 && *build!=0)
                return TRUE;
                /* EH not implemented yet */
            else
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */


            POEDRV_OM_GetImageVersion(version1, version2);
            POEDRV_OM_GetImageBuild(build);

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
#endif /*#if (SYS_CPNT_POE_ASIC_WITHOUT_MCU == TRUE)*/
    }

    return TRUE;
} /* End of POEDRV_GetPoeSoftwareVersion() */

/* FUNCTION NAME : POEDRV_GetPoeSoftwareVersion
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : unit -- unit ID
 * OUTPUT  : version1 -- version number 1
 *           version2 -- version number 2
 *           build    -- build number
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalGetPoeSoftwareVersion(UI8_T *version1, UI8_T *version2, UI8_T *build)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_POE_SOFTWARE_VERSION, 0, 0, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    *version1 = (UI8_T) dragonitep->dragoniteData.system.majorVersion;
    if (dragonitep->dragoniteData.system.minorVersion == 4866)
    {
        *version2 = 0x48;
        *build = 0x66;
    }
    else
    {
        *version2 = 0xFF;
        *build = 0xFF;
    }
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T ret = FALSE;


    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller.
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_VERSIONZ;
    transmit.subject2     = POEDRV_TYPE_SUBJECT2_SOFTWARE_VERSION;  /* This naming has been changed to software version. */

    /* Send this request packet to PoE controller and expect to receive a response packet from PoE controller via UART interface.
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
            printf("\n\r***Failed in the POEDRV_LocalGetPoeSoftwareVersion!***");
    }
    else
    {
         *version1 = receive.data6;
         *version2 = receive.data7;
         *build    = receive.data9;
    }

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

#endif

} /* End of POEDRV_LocalGetPoeSoftwareVersion() */

/* FUNCTION NAME : POEDRV_SendRawPacket
 * PURPOSE: This function is used to send a raw packet from engineering backdoor
 * INPUT:   transmit: data pointer of 13-byte packet to be transmitted
 * OUTPUT:  receive : data pointer of receiving packet
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SendRawPacket(UI8_T *transmit, UI8_T *receive)
{
    BOOL_T                 ret;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply((POEDRV_TYPE_PktBuf_T *)transmit, (POEDRV_TYPE_PktBuf_T *)receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_SendRawPacket!***");
#endif
    }

    return ret;
}

/* FUNCTION NAME : POEDRV_HardwareReset
 * PURPOSE: This function is used to issue a hardware reset to PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An hardware reset, bit 6 in system reset register, will be issued
 *          to PoE controller, as shown in following.
 *
 *          /reset _____          ____
 *                      |________|
 *
 *                      |<-10ms->|
 */
void POEDRV_HardwareReset(void)
{
    UC_MGR_Sys_Info_T uc_sys_info;


    if (UC_MGR_GetSysInfo(&uc_sys_info) == FALSE)
    {
        printf("%s(%d): UC_MGR_GetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if (POEDRV_BOARD_HardwareReset(uc_sys_info.board_id) == TRUE)
    {
        /* No need to implement at this moment. */
    }
} /* End of POEDRV_HardwareReset() */

/* FUNCTION NAME : POEDRV_ReleaseSoftwareReset
 * PURPOSE: This function is used to release/hold software reset for PoE controller.
 *          PoE controller will start/stop powering connected PDs.
 * INPUT:   is_enable -- TRUE : port powering enabled
 *                       FALSE: port powering disabled
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An software reset, by setting bit-5 in system reset register, will be issued
 *          to PoE controller, as shown in following.
 *
 *          /reset       _________
 *                 _____|
 *
 *                      |<-10ms->|
 */
void POEDRV_ReleaseSoftwareReset(BOOL_T is_enable)
{
    UC_MGR_Sys_Info_T uc_sys_info;


    if (UC_MGR_GetSysInfo(&uc_sys_info) == FALSE)
    {
        printf("%s(%d): UC_MGR_GetSysInfo failed.\r\n", __FUNCTION__, __LINE__);
        return;
    }

    if (POEDRV_BOARD_ReleaseSoftwareReset(uc_sys_info.board_id, is_enable) == TRUE)
    {
        POEDRV_OM_SetHwEnable(is_enable);
    }
} /* End of POEDRV_ReleaseSoftwareReset() */

/* FUNCTION NAME : POEDRV_SoftwareDownload
 * PURPOSE: This function is used to upgrade software version of PoE controller
 * INPUT:   unit -- unit ID
 *          filename_p -- filename to be downloaded
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
BOOL_T POEDRV_SoftwareDownload(UI32_T unit, UI8_T *filename_p)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SOFTWARE_DOWNLOAD)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Controlt
             */
            isc_buf_p->serviceID = POEDRV_SOFTWARE_DOWNLOAD;
            memcpy(isc_buf_p->info.name_ar, filename_p, SYS_ADPT_FILE_SYSTEM_NAME_LEN);

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;

        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */


            if (POEDRV_LocalSoftwareDownload(filename_p) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }

    return TRUE;

} /* End of POEDRV_SoftwareDownload() */

/* FUNCTION NAME : POEDRV_LocalSoftwareDownload
 * PURPOSE: This function is used to upgrade software version of PoE controller
 * INPUT:   filename_p -- filename to be downloaded
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static  BOOL_T POEDRV_LocalSoftwareDownload(UI8_T *filename_p)
{
    BOOL_T           ret = 0;
    UI8_T            *buf_ptr;
    UI8_T            *file_buf;
    UI8_T            response[10];
    UI8_T            line_buf[255];
    UI32_T           line_size;
    UI32_T           read_count = 0;
    UI32_T           buf_size = 0;
    UI32_T           my_unit = 0;
//Eugene temp,    I32_T            read_bytes = 0;

    /* Memory allocation
     */
    if ( (file_buf = (UI8_T *)POEDRV_Malloc(POEDRV_TYPE_SIZE_OF_POE_SOFTWARE)) == NULL )
         return FALSE;

    POEDRV_OM_GetMyUnitID(&my_unit);

    /* Read file from Flash
     */
    if ( FS_ReadFile(my_unit, filename_p, file_buf, POEDRV_TYPE_SIZE_OF_POE_SOFTWARE, &read_count) != FS_RETURN_OK )
    {
         POEDRV_Free(file_buf);
         return FALSE;
    }

    /* Step 1
     */

    if ( is_enter_program_mode == FALSE )
    {
         /* Set flag to notify POEDRV task the PoE controller is in program mode.
          */
         is_enter_program_mode = TRUE;
         while ( is_stop_polling == FALSE ){SYSFUN_Sleep(10);};

         /* Flush UART buffer
          */
         POEDRV_ENTER_CRITICAL_SECTION;
//Eugene temp,         read_bytes = UARTDRV_GetString(1, response);
         POEDRV_LEAVE_CRITICAL_SECTION;
         SYSFUN_Sleep(10);

         /* Host */
         /* PoE controller */
         if ( POEDRV_SendSoftwareDownload() == FALSE )
         {
              is_enter_program_mode = FALSE;
              POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
              if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                   printf("\n\rError on sending Software download packet");
#endif
              POEDRV_Free(file_buf);
              return FALSE;
         }

         POEDRV_ENTER_CRITICAL_SECTION;
    }
    else
    {
         UI8_T    enter_ptr[10];

         /* Flush UART buffer
          */
         POEDRV_ENTER_CRITICAL_SECTION;
//Eugene temp,         read_bytes = UARTDRV_GetString(1, response);

         /* Host */
         strcpy((char*)enter_ptr, POEDRV_TYPE_SW_ENTER);
//Eugene temp,         ret = UARTDRV_PutStringToUart( UARTDRV_UART2, (UI8_T *)&enter_ptr[0], 1);
         SYSFUN_Sleep(1);

//Eugene temp,         ret = UARTDRV_PutStringToUart( UARTDRV_UART2, (UI8_T *)&enter_ptr[1], 1);
         SYSFUN_Sleep(1);

//Eugene temp,         ret = UARTDRV_PutStringToUart( UARTDRV_UART2, (UI8_T *)&enter_ptr[2], 1);
         SYSFUN_Sleep(1);

//Eugene temp,         ret = UARTDRV_PutStringToUart( UARTDRV_UART2, (UI8_T *)&enter_ptr[3], 1);
    }

    /* Step 2
     */

    /* Host */
    SYSFUN_Sleep(20);

    /* PoE controller */
    memset(response, 0, 10);
//Eugene temp,    read_bytes = UARTDRV_GetString(10, response);

    if ( strcmp(POEDRV_TYPE_SW_BOOT_RESPONSE, (char*)response) != 0 )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on receiving POEDRV_TYPE_SW_BOOT_RESPONSE:%s", response);
#endif
         POEDRV_Free(file_buf);
         return FALSE;
    }

    /* Step 3
     */

    /* Host */
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
         printf("\n\r-Send E: %s len:%u", POEDRV_TYPE_SW_ERASE, strlen(POEDRV_TYPE_SW_ERASE));
#endif
//Eugene temp,    ret = UARTDRV_PutStringToUart( UARTDRV_UART2, POEDRV_TYPE_SW_ERASE, strlen(POEDRV_TYPE_SW_ERASE));

    if ( ret == FALSE )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on transmitting POEDRV_TYPE_SW_ERASE");
#endif
         POEDRV_Free(file_buf);

         return FALSE;
    }
    SYSFUN_Sleep(10);

    /* PoE controller */
    memset(response, 0, 10);
//Eugene temp,    read_bytes = UARTDRV_GetString(10, response);

    if ( strcmp(POEDRV_TYPE_SW_ERASE_RESPONSE, (char*)response) != 0 )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on receiving POEDRV_TYPE_SW_ERASE_RESPONSE:%s", response);
#endif
         POEDRV_Free(file_buf);
         return FALSE;
    }

    /* Step 4
     */
    SYSFUN_Sleep(500);

    /* Step 5
     */

    /* PoE controller */
    memset(response, 0, 10);
//Eugene temp,    read_bytes = UARTDRV_GetString(10, response);

    if ( strcmp(POEDRV_TYPE_SW_ERASE_COMPLETED, (char*)response) != 0 )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on receiving POEDRV_TYPE_SW_ERASE_COMPLETED_1:%s", response);
#endif
         POEDRV_Free(file_buf);
         return FALSE;
    }
    SYSFUN_Sleep(10);

    /* Step 6
     */

    /* Host */
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
         printf("\n\r-Send P: %s len:%u", POEDRV_TYPE_SW_PROGRAM, strlen(POEDRV_TYPE_SW_PROGRAM));
#endif
//Eugene temp,    ret = UARTDRV_PutStringToUart( UARTDRV_UART2, POEDRV_TYPE_SW_PROGRAM, strlen(POEDRV_TYPE_SW_PROGRAM));
    if ( ret == FALSE )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on transmitting POEDRV_TYPE_SW_PROGRAM");
#endif
         POEDRV_Free(file_buf);

         return FALSE;
    }
    SYSFUN_Sleep(10);

    /* PoE controller */
    memset(response, 0, 10);
//Eugene temp,    read_bytes = UARTDRV_GetString(10, response);

    if ( strcmp(POEDRV_TYPE_SW_PROGRAM_RESPONSE, (char*)response) != 0 )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on receiving POEDRV_TYPE_SW_PROGRAM_RESPONSE:%s", response);
#endif
         POEDRV_Free(file_buf);

         return FALSE;
    }

    buf_ptr = file_buf;
    memset(line_buf, 0, 255);
    ret = TRUE;

    while (TRUE)
    {
            memset(line_buf, 0, 255);
            line_size = POEDRV_GetLine(buf_ptr, line_buf, 255);

            if ( line_size < 0 )
                 break;

            if ( buf_size > read_count )
            {
                 ret = FALSE;
                 break;
            }

            /* If starting with "S0", ignore this comment line
             */
            if ( (buf_ptr[0] == 'S') && (buf_ptr[1] == '0') )
            {
                  buf_size += line_size;
                  buf_ptr += line_size;
                  continue;
            }

            /* Step 7
             */

            /* Host */
//Eugene temp,            if ( UARTDRV_PutStringToUart( UARTDRV_UART2, line_buf, line_size-2) == FALSE )
            {
                 ret = FALSE;
                 break;
            }

            buf_size += line_size;
            buf_ptr += line_size;

            SYSFUN_Sleep(10);

            /* PoE controller */
            memset(response, 0, 10);
//Eugene temp,            read_bytes = UARTDRV_GetString(10, response);

            if ( strcmp(POEDRV_TYPE_SW_SEND_LINE_RESPONSE, (char*)response) != 0 )
            {
                 /* Step 8
                  */
                 if ( strcmp(POEDRV_TYPE_SW_END_OF_FILE, (char*)response) == 0 )
                 {
                      ret = TRUE;
                 }
                 else
                 {
#ifndef INCLUDE_DIAG
                      if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                           printf("\n\rError on receiving POEDRV_TYPE_SW_SEND_LINE_RESPONSE:%s", response);
#endif
                      ret = FALSE;
                 }
                 break;
            }

            if ( buf_size == read_count )
                 break;

    } /* End of while() */

    /* Free memory
     */
    POEDRV_Free(file_buf);
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
         printf("\n\r-buf_size:%ld, read_count=%ld",buf_size, read_count);
#endif

    if ( ret == FALSE )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
         return FALSE;
    }

    /* Step 9
     */

    /* Host : Await at least 400 ms*/
    SYSFUN_Sleep(50);

    /* Step 10
     */

    /* Host */
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
         printf("\n\r-Send RST: %s len:%u", POEDRV_TYPE_SW_SEND_RESET, strlen(POEDRV_TYPE_SW_SEND_RESET));
#endif
//Eugene temp,    ret = UARTDRV_PutStringToUart( UARTDRV_UART2, POEDRV_TYPE_SW_SEND_RESET, strlen(POEDRV_TYPE_SW_SEND_RESET));
    if ( ret == FALSE )
    {
         POEDRV_LEAVE_CRITICAL_SECTION;
         POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
         if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
              printf("\n\rError on transmitting POEDRV_TYPE_SW_SEND_RESET");
#endif
         return FALSE;
    }
    SYSFUN_Sleep(10);

    is_enter_program_mode = FALSE;
    POEDRV_LEAVE_CRITICAL_SECTION;
    return TRUE;

} /* End of POEDRV_LocalSoftwareDownload() */

/* LOCAL SUBPROGRAM BODIES
 */

#if 0 /* Eugene marked */
/* FUNCTION NAME : POEDRV_InitDataBase
 * PURPOSE: This function is used to initialize database with default value,
 *          when system enters master mode or slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static void POEDRV_InitDataBase(void)
{
    UI32_T              port;
    UI32_T              unit=1;
    UI32_T              board_id;
    STKTPLG_BOARD_BoardInfo_T  *board_info_p;

    /* Get my unit ID
     */
#ifdef INCLUDE_DIAG
    poedrv_my_unit_id   = 1;
    poedrv_num_of_units = 1;
#else
    STKTPLG_MGR_GetMyUnitID(&poedrv_my_unit_id);
    STKTPLG_MGR_GetNumberOfUnit(&poedrv_num_of_units);
#endif

    /* Get board_id */
    if ( STKTPLG_MGR_GetUnitBoardID(unit, &board_id) )
    {
        /* Get POEDRV board_info and set relative vlaue */
        if ( STKTPLG_BOARD_GetBoardInformation( board_id, &board_info_p))
        {
            main_pse_power_max_allocation =  board_info_p->main_pse_power_max_allocation;

            memcpy(poedrv_port_L2P_matrix,&board_info_p->Logical2PhysicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(poedrv_port_P2L_matrix,&board_info_p->Physical2LogicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(per_port_power_max_allocation,&board_info_p->per_port_power_max_allocation,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

        }
        else
            SYSFUN_Debug_Printf("\n\r*** Can not get related board information.***");
    }
    else
        SYSFUN_Debug_Printf("\n\r*** Fail in getting board id!***");


    /* Initialize Main PSE with default value
     */
    poedrv_mainpower_info.unit_id              = poedrv_my_unit_id;
    poedrv_mainpower_info.main_pse_power       = MIN_pethMainPsePower;
    poedrv_mainpower_info.main_pse_oper_status = VAL_pethMainPseOperStatus_faulty;
    poedrv_mainpower_info.main_pse_consumption = 0;
    poedrv_mainpower_info.legacy_detection_enable=0;
    /* Initialize state transition on all ports
     */
    memset(poedrv_port_state, POEDRV_PORT_IS_OFF, sizeof(UI8_T)*(POEDRV_NO_OF_POE_PORTS+1));


    /* Initialize port PSE with default value
     */
    for ( port=0; port<POEDRV_NO_OF_POE_PORTS; port++)   /* And  */
    {
          poedrv_port_info[port].detection_status   = VAL_pethPsePortDetectionStatus_disabled;
          poedrv_port_info[port].admin_status       = FALSE;
          poedrv_port_info[port].poe_linkup         = FALSE;
          poedrv_port_info[port].poe_active         = FALSE;
          poedrv_port_info[port].power_class        = VAL_pethPsePortPowerClassifications_class0;
          poedrv_port_info[port].power_consumption  = 0;
          /* Aaron 2007-10-05, patch the code for non-802.3af LED incorrect issue, like Cisco 7910, 7920 and 7960
           * Use unused default value to prevent the default value  same as non-802.3af final status
           * Original default value is 0x00 (same as non-802.3af final status and cause PoE LED not correct
           * when connect one non-802.3af device to switch and bootup again.
           */
          poedrv_port_info[port].actual_status      = 0xff;
          poedrv_port_info[port].is_overload        = FALSE;
          poedrv_port_info[port].is_port_failure    = FALSE;

    }

    /* Initialize flag for program mode to download software to PoE controller
     */
    is_enter_program_mode = FALSE;
    is_stop_polling       = FALSE;

    return;

} /* End of POEDRV_InitDataBase() */
#endif

/* FUNCTION NAME : POEDRV_TaskMain
 * PURPOSE: The main body of poedrv task
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 */
static void POEDRV_TaskMain(void)
{
//    UI32_T events;
    BOOL_T  provision_status = FALSE;
    UI8_T operation_status = VAL_pethMainPseOperStatus_off;
//    BOOL_T  hw_enable_status = FALSE;
//    BOOL_T  status = FALSE;
//    void*  timer_id;

    DBG_PRINT();
#if 0
#ifndef INCLUDE_DIAG
    /*if not a poe device, suspend this task itself*/
    if( !STKTPLG_OM_IsPoeDevice(1) ) /*unit = 1 means local unit */
    {
//        SYSFUN_SuspendTask (poedrv_thread_id);
        SYSFUN_SuspendThreadSelf();
    }
#endif
#endif

//    timer_id = SYSFUN_PeriodicTimer_Create();
//    SYSFUN_PeriodicTimer_Start(timer_id, 100, 0x1);

    /* main loop of task
     */
    while (TRUE)
    {
        SYSFUN_Sleep(SYS_BLD_POEDRV_UPDATE_POE_STATS_TICKS);
//        SYSFUN_ReceiveEvent(0x1,
//                            SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER, &events);

        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        {
            printf("%s(%d): Current tick is {%lu, %lu}.\r\n", __FUNCTION__, __LINE__, SYSFUN_GetSysTick(), SYS_TIME_GetSystemTicksBy10ms());
            fflush(stdout);
        }

        /* Polling PoE status only in Master or Slave mode
         */
        POEDRV_OM_GetProvisionComplete(&provision_status);
        if ( ((POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && provision_status) ||
             ((POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE) && provision_status))
        {
             /* Do nothing, if PoE controller is in program mode for software download
              */
             if ( is_enter_program_mode == TRUE )
             {
                  is_stop_polling = TRUE;
                  continue;
             }

             is_stop_polling = FALSE;

            /* Polling the PoE controller when stop polling controller flag is FALSE.
              */
            if (POEDRV_OM_IsStopMonitorFlagOn() == FALSE)
            {
                if (POEDRV_MonitorPoeOperationStatus() == TRUE)
                {

                    /* Start polling functions when operation status of PoE system is "VAL_pethMainPseOperStatus_on".
                     */
                    POEDRV_OM_GetMainPowerInfoMainOperStatus(&operation_status);
                    if (operation_status == VAL_pethMainPseOperStatus_on)
                    {

                        /* Polling the PoE controller in order to update database.
                         */
                        POEDRV_MonitorPoeMainpowerStatus();
                        POEDRV_MonitorPoePortStatus();
                        POEDRV_MonitorPoePortClassification();
                    }
                }
            }
        }
    } /* End of while (TRUE) */
} /* End of POEDRV_TaskMain() */

#if (POEDRV_POLLING_BY_REGISTER == TRUE)
/* FUNCTION NAME: POEDRV_SetDefaultValue
 * PURPOSE: This function is used to set default value for PoE controller
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   None
 */
static BOOL_T POEDRV_SetDefaultValue(void)
{
    POEDRV_TYPE_PktBuf_T    transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T    receive;        /* Receive buffer        */
    /* Andy */
    UI32_T                  port_power_limit = (UI32_T) SYS_DFLT_PSE_PORT_POWER_MAX_ALLOCATION;  /* default power limit: 30000 milliwatts */
    UI8_T                   *max_value = (UI8_T *) &port_power_limit;
    /* Andy */
    UI32_T                  mainpower = 0;
    UI8_T                   *power_limit = (UI8_T *) &mainpower;
    BOOL_T                  ret = FALSE;
    UI8_T                   port = 0;
    UI32_T                  phy_port = 0xFF;
    UI8_T                   power_bank = 0;
    UI32_T                  unit_id = 0, board_id = 0xFF;
    STKTPLG_BOARD_BoardInfo_T board_info;
    UI32_T poedrv_min_port_number = 0, poedrv_max_port_number = 0;


    STKTPLG_OM_GetMyUnitID(&unit_id);
    STKTPLG_OM_GetUnitBoardID(unit_id, &board_id);
    STKTPLG_BOARD_GetBoardInformation(board_id, &board_info);
    mainpower = board_info.main_pse_power_max_allocation / 1000; /* Change milliwatt power style to watt style. */
    POEDRV_OM_GetPOEPortNumber(&poedrv_min_port_number, &poedrv_max_port_number);

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_MASKZ;
    if (SYS_DFLT_CAPACITOR_DETECTION == TRUE)
        transmit.subject2     = POEDRV_TYPE_SUBJECT2_CapEnable; /* 0x06 */  /* enable resistor detection, enable capacitor detection */
    else
        transmit.subject2     = POEDRV_TYPE_SUBJECT2_ResEnable; /* 0x04*/   /* enable resistor detection, disable capacitor detection */

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to set System Mask!***");
#endif
        return FALSE;
    }

/* mapping port */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_TEMPORARY_CHANNEL_MATRIX;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    for ( port=1;port<=POEDRV_NO_OF_POE_PORTS;port++)
    {
        if( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) )
        {
            transmit.subject2     = port-1;
            transmit.data6        = (UI8_T) (board_info.Logical2PhysicalPort[port-1]);
            POEDRV_ENTER_CRITICAL_SECTION;
            ret = POEDRV_SendAndWaitReply(&transmit, &receive);
            POEDRV_LEAVE_CRITICAL_SECTION;
            if(ret == FALSE)
            {
#ifdef POEDRV_DEBUG
                printf("\n\r***Fail in the POEDRV_SetDefaultValue to set channel mapping!***");
#endif
                return FALSE;
            }
        }
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_TEMPORARY_CHANNEL_MATRIX;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {

#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to set System channel mapping!***");
#endif

        return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_PARAMETER;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    for (port = poedrv_min_port_number; port <= poedrv_max_port_number; port++)
    {
        if( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) )
        {
            port_power_limit = board_info.per_port_power_max_allocation[port-1];

#if defined(ES4552MA_HPOE)

#if (SYS_CPNT_POE_ASIC_POWERDSINE_PD69012_PSE_POWER_MAXIMUM_ALLOCATION_WORKAROUND == TRUE)
UI8_T data[2] = {0xFF, 0xFF};
UI16_T *data_ptr = (UI16_T *) &data[0];
double power_cut_off = 0.0, vmain = 0.0, vscale = 44.25;


    /* Microsemi FAE John:
     * PPL: SW setting per port power limit(W).
     * Pcut: real cut-off power read by SW(W).
     * Vmain: power supply voltage read by SW(V).
     * LGE1: PPL = (Pcut * 44 / Vmain) - 1 or Pcut = Vmain * (PPL + 1) / 44.
     */
    if ((ret = POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x105C, sizeof(data), data)) == TRUE)
    {
        vmain = (double) *data_ptr * 0.061; /* Resolution = 61mV. */
        power_cut_off = ((double) port_power_limit * vscale) / vmain - 1000.0; /* power_cut_off = XXXmW. */
        port_power_limit = (UI32_T) power_cut_off;
    }
    else
    {

#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to set channel %u=>%lu Power Limit!", port, phy_port);
#endif

        return FALSE;
    }
#endif /* End of "(SYS_CPNT_POE_ASIC_POWERDSINE_PD69012_PSE_POWER_MAXIMUM_ALLOCATION_WORKAROUND == TRUE)". */

#endif /* End of "defined(ES4552MA_HPOE)". */

            transmit.subject2     = phy_port;
            transmit.data6        = PORT_ON;

#if (SYS_HWCFG_LITTLE_ENDIAN_CPU == TRUE) /* Andy */
            transmit.data7        = max_value[1];
            transmit.data8        = max_value[0];
#else
            transmit.data7        = max_value[2];
            transmit.data8        = max_value[3];
#endif

            transmit.data9        = SYS_DFLT_PSE_PORT_POWER_PRIORITY;
            POEDRV_ENTER_CRITICAL_SECTION;
            ret = POEDRV_SendAndWaitReply(&transmit, &receive);
            POEDRV_LEAVE_CRITICAL_SECTION;
            if(ret == FALSE)
            {
#ifdef POEDRV_DEBUG
                printf("\n\r***Fail in the POEDRV_SetDefaultValue to set channel %u=>%lu Power Limit!",port, phy_port);
#endif
                return FALSE;
            }
        }
    }

#if 0 /* Have done by the above step */
//DBG_PRINT("set ports power priority");
    /* set ports power priority
     */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_PRIORITY;
    transmit.data6        = SYS_DFLT_PSE_PORT_POWER_PRIORITY;  /* priority: low */

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    for ( port=1;port<=POEDRV_NO_OF_POE_PORTS;port++)      /* Andy*/
    {
        if( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) )
        {
        transmit.subject2     = phy_port;          /* 128 for all ports */
//        transmit.subject2     = POEDRV_ALL_PORTS;          /* 128 for all ports */
        POEDRV_ENTER_CRITICAL_SECTION;
        ret = POEDRV_SendAndWaitReply(&transmit, &receive);
        POEDRV_LEAVE_CRITICAL_SECTION;
        if(ret == FALSE)
        {
#ifdef POEDRV_DEBUG
            printf("\n\r***Fail in the POEDRV_SetDefaultValue to set channel priority!***");
#endif
            return FALSE;
        }
        }
    }

//DBG_PRINT("enable all ports");
    /* Enable all ports
     */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_ON_OFF;
    transmit.data6        = PORT_ON;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    for ( port=1;port<=POEDRV_NO_OF_POE_PORTS;port++)      /* Andy*/
    {
        if( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) )
        {
    transmit.subject2     = phy_port;          /* 128 for all ports */
//    transmit.subject2     = POEDRV_ALL_PORTS;          /* 128 for all ports */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to enable port failed!***");
#endif
        return FALSE;
    }
        }
    }

//DBG_PRINT("set ports max allocation power");

    /* set ports max allocation power
     */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    for ( port=SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;port<=SYS_ADPT_POE_PSE_MAX_PORT_NUMBER;port++)      /* Andy*/
    {
        if( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) )
        {
            port_power_limit = board_info.per_port_power_max_allocation[port-1];  /* default power limit: 30000 milliwatts */

            transmit.data6        = max_value[2];
            transmit.data7        = max_value[3];
            transmit.subject2     = (phy_port);
            POEDRV_ENTER_CRITICAL_SECTION;
            ret = POEDRV_SendAndWaitReply(&transmit, &receive);
            POEDRV_LEAVE_CRITICAL_SECTION;
            if(ret == FALSE)
            {
#ifdef POEDRV_DEBUG
                printf("\n\r***Fail in the POEDRV_SetDefaultValue to set channel %u=>%lu Power Limit!",port, phy_port);
#endif
                return FALSE;
            }
        }
    }
#endif

/* Eugene: this step will cause AC disconnect to DC disconnect
 *         and the eeprom init is set to dynamic management already. */
#if 0
//DBG_PRINT("set power manage mode dynamic");
    /* The PM Mode default Value in Revision 5.1 is 0x00,0x02,0x00, but in Revision 4.0
     * it is 0x00,0x00,0x00
     * So we should set Static Mode as default value.
     */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_POWER_MANAGEMODE;
    transmit.data6        = 0x00;
    transmit.data7        = 0x02;
    transmit.data8        = 0x00;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to set dynamic mode!***");
#endif
        return FALSE;
    }
#endif

//DBG_PRINT("get power bank");
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_SUBJECT2_MAIN;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to get Power Bank!***");
#endif
        return FALSE;
    }
    /* Get current power bank
     */
    power_bank = receive.data10;

    /* Set Power Budget */
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_POWER_BUDGET;
    transmit.data6        = power_bank;
    transmit.data7        = power_limit[2];
    transmit.data8        = power_limit[3];
    transmit.data9        = POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_H;
    transmit.data10       = POEDRV_TYPE_MAX_SHUTDOWN_VOLTAGE_L;
    transmit.data11       = POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_H;
    transmit.data12       = POEDRV_TYPE_MIN_SHUTDOWN_VOLTAGE_L;
    transmit.data13       = (UI8_T) POEDRV_TYPE_PSE_POWER_GUARD_BAND;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if (ret != TRUE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r ***Fail in the POEDRV_SetDefaultValue to Set Power Bank Fail,will be tried again!");
#endif
        return FALSE;
    }

    /*After Set Power Bank, we need to Save System Setting to let change occur
     */
#if 0
    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_PROGRAM;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_E2;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SAVE_CONFIG;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Fail in the POEDRV_SetDefaultValue to save config!***");
#endif
//        return FALSE;
    }
#endif
    return TRUE;
} /* End of POEDRV_SetDefaultValue() */
#endif


/* FUNCTION NAME: POEDRV_LocalGetPortPowerConsumption
 * PURPOSE: This function is used to get power consumption of a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  milliwatts -- power consumption of a port in milliwatts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalGetPortPowerConsumption(UI32_T unit, UI32_T port, UI32_T *milliwatts)
{
#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_PORT_MEASURE, unit, port, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    *milliwatts = (UI32_T) dragonitep->dragoniteData.portMeasure[port - 1].powerConsumption * 100;
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    UI32_T phy_port = 0; /* Physical port ID. */
    UI32_T temp_val = 0;
    UI8_T *value = (UI8_T *) &temp_val;
    UI32_T board_id = 0xFF;
    UI8_T data[2] = {0};


    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        return FALSE;

    POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
    if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20) + (phy_port / 12), 0x12B4 + ((phy_port % 12) * 2), sizeof(data), data) == FALSE)
        return FALSE;
    value[2] = data[0];
    value[3] = data[1];
    temp_val = L_STDLIB_Ntoh32(temp_val);
    *milliwatts = temp_val * 100;

    return TRUE;
#else
    UI32_T phy_port = 0; /* Physical port ID. */
    UI32_T temp_val = 0;
    UI8_T *value = (UI8_T *) &temp_val;


    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    BOOL_T ret = FALSE;


        /* Mapping logical port ID to physical port ID
         */
        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return FALSE;
        }

        memset(value , 0x0, sizeof(UI32_T));    // wuli, add

        /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
         */
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        /* Prepare request buffer for sending this command to PoE controller
         */
        transmit.key          = POEDRV_TYPE_KEY_REQUEST;
        transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
        transmit.subject1     = POEDRV_TYPE_SUBJECT1_PARAMZ;
        transmit.subject2     = (UI8_T)phy_port;

        /* Send this request packet to PoE controller and expect to receive
         * a response packet from PoE controller via UART interface
         */
        POEDRV_ENTER_CRITICAL_SECTION;
        ret = POEDRV_SendAndWaitReply(&transmit, &receive);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (ret == FALSE)
        {
            if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("\n\r***Failed in the POEDRV_LocalGetPortPowerConsumption!***");
        }
        else
        {
            value[2] = receive.data7;
            value[3] = receive.data8;
            temp_val = L_STDLIB_Ntoh32(temp_val);
            *milliwatts = temp_val;  //wuli,1225
        }

        return ret;
#endif

#endif

} /* End of POEDRV_LocalGetPortPowerConsumption() */

/* FUNCTION NAME: POEDRV_LocalGetPortPowerClassification
 * PURPOSE: This function is used to get power classification of a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_class -- power class of a port ,range from 0 to 4
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalGetPortPowerClassification(UI32_T unit, UI32_T port, UI32_T *power_class)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_PORT_STATUS, unit, port, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    *power_class = (UI32_T) dragonitep->dragoniteData.portStat[port - 1].portSr.portClass;
    free(dragonitep);

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    UI32_T phy_port = 0xFF; /* Physical port ID. */
    UI32_T temp_val = 0;
    UI8_T *value = ( UI8_T *) &temp_val;
    UI32_T board_id = 0xFF;
    UI8_T data[2] = {0};


        /* Mapping logical port ID to physical port ID
         */
        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
            return FALSE;

    POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
    if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20) + (phy_port / 12), 0x11C2 + ((phy_port % 12) * 2), sizeof(data), data) == FALSE)
        return FALSE;
            value[3] = data[1];
            temp_val = L_STDLIB_Ntoh32(temp_val);
    *power_class = temp_val;

            return TRUE;
#else
    UI32_T                 phy_port;       /* Physical port ID      */
    /*UI8_T                  *value = (UI8_T *)power_classs;*/
    UI32_T                 temp_val = 0;    //wuli,1225
    UI8_T                  *value = ( UI8_T *)&temp_val; //wuli, 1225


    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    BOOL_T ret = FALSE;


        /* Mapping logical port ID to physical port ID
         */
        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            return FALSE;
        }

        memset(value , 0x0, sizeof(UI32_T));    // wuli, add

        /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
         */
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        /* Prepare request buffer for sending this command to PoE controller
         */
        transmit.key          = POEDRV_TYPE_KEY_REQUEST;
        transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
        transmit.subject1     = POEDRV_TYPE_SUBJECT1_PORT_STATUS;
        transmit.subject2     = (UI8_T)phy_port;

        /* Send this request packet to PoE controller and expect to receive
         * a response packet from PoE controller via UART interface
         */
        POEDRV_ENTER_CRITICAL_SECTION;
        ret = POEDRV_SendAndWaitReply(&transmit, &receive);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (ret == FALSE)
        {

#ifdef POEDRV_DEBUG
            printf("\n\r***Failed in the POEDRV_LocalGetPortPowerClassification!***");
#endif

        }
        else
        {
            value[3] = receive.data7;
            temp_val     = L_STDLIB_Ntoh32(temp_val);
            *power_class = temp_val;  //wuli,1225
        }

        return ret;
#endif

#endif

} /* End of POEDRV_LocalGetPortPowerClassification() */

/* FUNCTION NAME : POEDRV_MonitorPoePortStatus
 * PURPOSE: This function is used to periodically query PoE port status and
 *          update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE: Successfully, FALSE: Failed
 * NOTES:   It is required to send three packets to get status of all port.
 */
static BOOL_T POEDRV_MonitorPoePortStatus(void)
{
    UI32_T      my_unit = 0, min_port = 0, max_port = 0, port = 0, phy_port = 0;
    UI8_T       ports_status = 0;
    UI8_T       actual_status, old_detection_status, detection_status;
    UI8_T       port_state;
    BOOL_T      status_change;
    BOOL_T      ports_status_change;/*if ports_status[]  change*/
    BOOL_T      overload, port_failure;
    BOOL_T      is_real_overload_status;
    BOOL_T      is_real_underload_status;


#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_PORT_STATUS, 0, 0, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;

#endif

    POEDRV_OM_GetMyUnitID(&my_unit);
    POEDRV_OM_GetPOEPortNumber(&min_port, &max_port);
    for (port = min_port; port <= max_port; port++)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
             continue;

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
        switch (dragonitep->dragoniteData.portStat[port - 1].portSr.status)
        {
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_PORT_ON_E:
                ports_status = POEDRV_TYPE_PORT_POWERED_RES_DETECT;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SYSTEM_DISABLED_E:
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNKNOWN_E:
                ports_status = POEDRV_TYPE_PORT_ASIC_HARDWARE_FAULT;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_SEARCHING_E:
                ports_status = POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_INVALID_SIGNATURE_E:
                ports_status = POEDRV_TYPE_PORT_OFF_NOT_PD;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_POWER_MANAGE_E:
                ports_status = POEDRV_TYPE_PORT_OFF_POWER_MANAGEMENT;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_OVERLOAD_E:
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_OVERLOAD_E:
                ports_status = POEDRV_TYPE_PORT_OVERLOAD;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_STARTUP_UNDERLOAD_E:
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_UNDERLOAD_E:
                ports_status = POEDRV_TYPE_PORT_UNDERLOAD;
                break;
            case CPSS_GEN_DRAGONITE_PORT_STATUS_SR_DISABLED_E:
                ports_status = POEDRV_TYPE_PORT_OFF_USER_SETTING;
                break;
            default:
                ports_status = 0xFF;  /* unknow now */
                break;
        }

#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
/* PCA9538PWR Address 0x70:
 *  Offset  Bit            NAME            Value(1)          Value(0)
 *       1    0    ENA_PD69101n      Disable PD69101      Enable PD69101
 *       1    1    PSE_Always_output always enable PSE    PSE will be ctrl
 *                                   (where ENA_PD69101n  by PD69101
 *                                    should be disabled)
 *       0    2    PSE_LED0                 see following table
 *       0    3    PSE_LED1                 see following table
 *       3    x    Configuration     bit0,1 are used as output
 *                 Register          bit2,3 are used as input
 *                                   should set as 0xfc
 * -----------------------------------
 *  Status       PSE_LED0     PSE_LED1
 *  AF Mode, On         0            1
 *  AF Mode, Off        1            0
 *  AT Mode, On         0            0 (Not support)
 *  AT Mode, Off        1            1 (Not support)
 *
 */
        UI8_T    read_data=0, ctrl_data;

        I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_PCA9538PWR, 0x0, 0x1, &read_data);
        I2CDRV_GetI2CInfo(SYS_HWCFG_I2C_SLAVE_PCA9538PWR, 0x1, 0x1, &ctrl_data);
        
        if((ctrl_data & 0x3) == 0x0)/* PD69101 is enabled */
        {
            if((read_data & 0xC) == 0x8)
            {
                ports_status = POEDRV_TYPE_PORT_POWERED_RES_DETECT;
            }
            else
            {
                ports_status = POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED;
            }
        }
        else if((ctrl_data & 0x3) == 0x1)/* PD69101 is disabled */
        {
            ports_status = POEDRV_TYPE_PORT_OFF_USER_SETTING;
        }
#elif defined(ES4552MA_HPOE)
        UI32_T board_id = 0xFF;
        UI8_T data[2] = {0};


        POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
        if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20) + (phy_port / 12), 0x11AA + ((phy_port % 12) * 2), sizeof(data), data) == FALSE)
            return FALSE;

            switch (data[1])
            {
                case 0:

#if 1

                    /* Workaround the leakage current issue of PSE port. Refer test result, it shall be 300mA - 700mA.
                     * If the design of PoE board is good, we don't need to process this patch.
                     */
                    if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20) + (phy_port / 12), 0x12B4 + ((phy_port % 12) * 2), sizeof(data), data) == FALSE)
                        return FALSE;
                    if (data[0] == 0 && data[1] < 10)
                    {
                        POEDRV_LocalSetPortAdminStatus(port, VAL_pethPsePortAdminEnable_false);
                        POEDRV_LocalSetPortAdminStatus(port, VAL_pethPsePortAdminEnable_true);
                        ports_status = POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED;
                    }
                    else
                        ports_status = POEDRV_TYPE_PORT_POWERED_RES_DETECT;
#else
                    ports_status = POEDRV_TYPE_PORT_POWERED_RES_DETECT;
#endif

                    break;
                case 19:
                case 20:
                    ports_status = POEDRV_TYPE_PORT_ASIC_HARDWARE_FAULT;
                    break;
                case 4:
                    ports_status = POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED;
                    break;
                case 5:
                    ports_status = POEDRV_TYPE_PORT_OFF_NOT_PD;
                    break;
                case 18:
                    ports_status = POEDRV_TYPE_PORT_OFF_POWER_MANAGEMENT;
                    break;

                /* ES4552MA-HPoE-7LF-LN-01462.
                 * Port [0-11] status of PD690xx.
                 * Microsemi PD690xx, value 10 (dec) and 15 (dec) of register 0x11AA-0x11C0.
                 */
                case 10:
                case 15:
                    ports_status = POEDRV_TYPE_PORT_OVERLOAD;
                    break;

                /* ES4552MA-HPoE-7LF-LN-01462.
                 * Port [0-11] status of PD690xx.
                 * Microsemi PD690xx, value 11 (dec) and 16 (dec) of register 0x11AA-0x11C0.
                 */
                case 11:
                case 16:
                    ports_status = POEDRV_TYPE_PORT_UNDERLOAD;
                    break;
                case 9:
                    ports_status = POEDRV_TYPE_PORT_OFF_USER_SETTING;
                    break;
                default:
                    ports_status = 0xFF;  /* unknow now */
                    break;
            }
#endif

#endif /* End of #if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE) */

        POEDRV_OM_GetPortInfoActualStatus(port, &actual_status);

        detection_status = 0;
        status_change = FALSE;
        ports_status_change = FALSE;
        is_real_overload_status  = FALSE;
        is_real_underload_status = FALSE;
        /*
        Specail Case of Processing POE messages that can reduce frequent notification callback

        POE Port-Stat           Led             Message        [Notify] #Led  #Dectecion
        On->Off                 Green->Off      00->1b,00->1e           Yes   Yes
        On->Non-PD              Green->Off      00->1b,00->1c           Yes   Yes
        Off->Off                Off->Off        1b->1e,1e->1b           No    No
        Non-PD->Non-PD          Off->Off        1b->1c,1c->1b           No    No
        Non-PD->Off             Off->Off        1c->1b,1c->1e           No    No
        Off->Non-PD             Off->Off        1e->1b,1e->1c           No    No


        #Led:notify Led_mgr by POEDRV_Notify_PortStatusChange
        #Detection:notify Poe_mgr by POEDRV_Notify_PortDetectionStatusChange

        Because the status of a overloaded port will change to disable by WAD
        immediately, we don't have to take care of them.
        */

        /* Updated actual status */
        if ( actual_status != ports_status)
        {
            /*If there is no pd connected, we often get message id = 1b or 1e, we treat them in the same way.*/
            /*If there is a copper connected, we often get message id = 1b or 1c, we treat them in the same way.*/
            if (((actual_status == POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED) &&
                (ports_status == POEDRV_TYPE_PORT_UNDERLOAD)) /*1b->1e*/ ||
                ((actual_status == POEDRV_TYPE_PORT_UNDERLOAD) &&
                (ports_status == POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED)) /*1e->1b*/ ||
                ((actual_status == POEDRV_TYPE_PORT_OFF_NOT_PD) &&
                (ports_status == POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED)) /*1c->1b*/ ||
                ((actual_status == POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED) &&
                (ports_status == POEDRV_TYPE_PORT_OFF_NOT_PD)) /*1b->1c*/ ||
                ((actual_status == POEDRV_TYPE_PORT_OFF_NOT_PD) &&
                (ports_status == POEDRV_TYPE_PORT_UNDERLOAD)) /*1c->1e*/ ||
                ((actual_status == POEDRV_TYPE_PORT_UNDERLOAD) &&
                (ports_status == POEDRV_TYPE_PORT_OFF_NOT_PD))/*1e->1c*/)
            {
                ports_status_change = FALSE;
            }
            else
            {
                ports_status_change = TRUE;
            }
            POEDRV_OM_SetPortInfoActualStatus(port, ports_status);
        }

        POEDRV_OM_GetPortInfoIsOverload(port, &overload);
        POEDRV_OM_GetPortInfoDetectionStatus(port, &old_detection_status);
        POEDRV_OM_GetPortInfoIsPortFailure(port, &port_failure);
        /* If status has been changed, update the database and notify callback function
         */
        switch (ports_status)
        {
            case POEDRV_TYPE_PORT_POWERED_CAP_DETECT: /* Port is powered normally */
            case POEDRV_TYPE_PORT_POWERED_RES_DETECT: /* Port is powered normally */
            /* Set state for a port
             */
                POEDRV_OM_SetPortState(port, POEDRV_PORT_IS_ON);
                POEDRV_OM_SetPortInfoLinkUp(port, TRUE);
                POEDRV_OM_SetPortInfoActive(port, TRUE);
                /* Overload status is deactivated
                 */
                if( overload != FALSE )
                {
#ifndef INCLUDE_DIAG
                    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                        printf("\n\r - port %ld status 0x%2x - ",port, ports_status);
#endif
                        POEDRV_OM_SetPortInfoIsOverload(port, FALSE);
                        POEDRV_Notify_PortOverloadStatusChange(my_unit, port, FALSE);
                }

                /* Update detection status
                 */
                if( old_detection_status != VAL_pethPsePortDetectionStatus_deliveringPower )
                {
                    detection_status = VAL_pethPsePortDetectionStatus_deliveringPower;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                /* Port failure status is ceasing
                 */
                if( port_failure != FALSE )
                {
                    POEDRV_OM_SetPortInfoIsPortFailure(port, FALSE);
                    POEDRV_Notify_PortFailureStatusChange(my_unit, port, FALSE);
                    POEDRV_Notify_PortOverloadStatusChange(my_unit, port, FALSE);
                }
                break;

            case POEDRV_TYPE_PORT_ASIC_HARDWARE_FAULT: /* Port is off due to internal hardware fault */
            case POEDRV_TYPE_PORT_HARDWARE_FAULT:      /* Port is off due to internal hardware fault */
                /* Port failure
                 */
                POEDRV_OM_SetPortInfoLinkUp(port, FALSE);
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                if( port_failure != TRUE )
                {
                    POEDRV_OM_SetPortInfoIsPortFailure(port, TRUE);
                    POEDRV_Notify_PortFailureStatusChange(my_unit, port, TRUE);
                }
                break;

            case POEDRV_TYPE_PORT_DETECT_NOT_COMPLETED  :/* due to  software PD detection is not completed*/
            /*This status always exists while port don't supply power, so treat it as a port-off status*/
            case POEDRV_TYPE_PORT_OFF_NOT_PD: /*not a 802.3af compliant PD*/
                if( old_detection_status !=  VAL_pethPsePortDetectionStatus_disabled)
                {
                    detection_status = VAL_pethPsePortDetectionStatus_disabled;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                break;

            case POEDRV_TYPE_PORT_OFF_POWER_MANAGEMENT:/* due to  power budget*/
                /* Nortel project : The connect port causing the main pse cinsumption over
                 *                  the power budget, we will disable the port
                 */
                POEDRV_OM_SetPortInfoLinkUp(port, TRUE);
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                if( old_detection_status != VAL_pethPsePortDetectionStatus_otherFault)
                {
                    detection_status = VAL_pethPsePortDetectionStatus_otherFault;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                break;

            case POEDRV_TYPE_PORT_OVERLOAD:    /* Port is off (overload) */
                /* Set state transition on a port
                 */
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                POEDRV_OM_GetPortState(port, &port_state);
                if ( port_state == POEDRV_PORT_IS_ON )
                {
                    POEDRV_OM_SetPortState(port, POEDRV_PORT_IS_OFF_OVERLOAD);
                    is_real_overload_status  = TRUE;

                    if( port_failure != TRUE )
                    {
                        POEDRV_OM_SetPortInfoIsPortFailure(port, TRUE);
                        POEDRV_Notify_PortFailureStatusChange(my_unit, port, TRUE);
                    }
                }
                else
                {
                    POEDRV_OM_SetPortState(port, POEDRV_PORT_IS_OFF_NON_PD);
                    is_real_overload_status  = FALSE;
                }

                /* Overload status is activated
                 */
                if( overload != TRUE )
                {
#ifndef INCLUDE_DIAG
                    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                        printf("\n\r - port %ld status 0x%2x - ", port, ports_status);
#endif
                    POEDRV_OM_SetPortInfoIsOverload(port, TRUE);
                #if (SYS_HWCFG_POE_CHIP_SUPPORT_OVERLOAD_DETECT == FALSE)
                    /* disable the port when overload occured */
                    if (is_real_overload_status)
                        POEDRV_Notify_PortOverloadStatusChange(my_unit, port, TRUE);
                #endif
                }
                /* Port failure
                 */
                if( port_failure != TRUE )
                {
                    POEDRV_OM_SetPortInfoIsPortFailure(port, TRUE);
                    if (is_real_overload_status)
                        POEDRV_Notify_PortFailureStatusChange(my_unit, port, TRUE);
                }
                /* Update detection status
                 */
                if( (old_detection_status != VAL_pethPsePortDetectionStatus_fault)
                    && is_real_overload_status )
                {
                    detection_status = VAL_pethPsePortDetectionStatus_fault;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                break;

            case POEDRV_TYPE_PORT_UNDERLOAD:         /* Port is off */
                /* Set state transition on a port
                 */
                POEDRV_OM_SetPortInfoLinkUp(port, TRUE);
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                break;

            case POEDRV_TYPE_PORT_OFF_USER_SETTING:  /* Port is off */
                POEDRV_OM_SetPortInfoLinkUp(port, FALSE);
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                //POEDRV_OM_SetPortInfoAdminStatus(port, VAL_pethPsePortAdminEnable_false);
                POEDRV_OM_SetPortInfoIsOverload(port, FALSE);
                if ( old_detection_status != VAL_pethPsePortDetectionStatus_disabled)
                {
                    detection_status = VAL_pethPsePortDetectionStatus_disabled;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                break;

            default:             /* unknown status */
                /* Overload status is deactivated
                 */
                POEDRV_OM_SetPortInfoLinkUp(port, FALSE);
                POEDRV_OM_SetPortInfoActive(port, FALSE);
                if( overload != FALSE )
                {
#ifndef INCLUDE_DIAG
                    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                        printf("\n\r - port %ld status 0x%2x - ",port, ports_status);
#endif
                    POEDRV_OM_SetPortInfoIsOverload(port, FALSE);
                    POEDRV_Notify_PortOverloadStatusChange(my_unit, port, FALSE);
                }

                /* Update detection status
                 */
                if( old_detection_status != VAL_pethPsePortDetectionStatus_disabled )
                {
                    detection_status = VAL_pethPsePortDetectionStatus_disabled;
                    POEDRV_OM_SetPortInfoDetectionStatus(port, detection_status);
                    status_change = TRUE;
                }
                break;
        } /* End of switch (status_ptr[ptr_index]) */

        /* Notify registered callback function if status has been changed
         */
        if ( status_change )
        {
            if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("\r\n %s-%d port %ld detection_status change from 0x%2x to 0x%2x ",
                       __FUNCTION__, __LINE__, port, old_detection_status, detection_status);
            POEDRV_Notify_PortDetectionStatusChange(my_unit, port, detection_status);
        }
        if (ports_status_change == TRUE)
        {
            if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("\r\n %s-%d port %ld status change from 0x%2x to 0x%2x ",
                        __FUNCTION__, __LINE__, port, actual_status, ports_status);
            POEDRV_Notify_PortStatusChange(my_unit, port, ports_status);
        }
    }

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    free(dragonitep);
#endif

    return TRUE;
} /* End of POEDRV_MonitorPoePortStatus() */

/* FUNCTION NAME : POEDRV_MonitorPoePortClassification
 * PURPOSE: This function is used to periodically query PoE power
 *          classification of a port and update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE: Successfully, FALSE: Failed
 * NOTES:
 */
static BOOL_T POEDRV_MonitorPoePortClassification(void)
{
#define POLLING_TIME 4
#define POLLING_PORT_ONE_TIME  ((SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT) / POLLING_TIME)

    UI32_T                 old_power_consumption, power_consumption = 0;
    UI32_T                 my_unit, port;
    UI32_T                 old_power_class, power_class=0;
    UI32_T                 min_poe_port, max_poe_port;
    static UI8_T           count = 0; /*workaround, to reduce cpu utility, only polling 4 ports a time*/
    static BOOL_T          no_need_polling[SYS_ADPT_MAX_NBR_OF_POE_PORT_PER_UNIT]={0};
    UI8_T                   act_status;
    BOOL_T                 ret;

    POEDRV_OM_GetMyUnitID(&my_unit);

    POEDRV_OM_GetPOEPortNumber(&min_poe_port, &max_poe_port);
//    for ( port=min_poe_port; port<=max_poe_port;port++)   /*  andy  */
    for ( port=min_poe_port+(count*POLLING_PORT_ONE_TIME); port<min_poe_port+((count+1)*POLLING_PORT_ONE_TIME);port++)
    {
        /* check if polling power consumption is needed (no powering, no polling) */
        POEDRV_OM_GetPortInfoActualStatus(port, &act_status);

        if (no_need_polling[port-1] == TRUE)
        {
            if ((act_status == POEDRV_TYPE_PORT_POWERED_CAP_DETECT) || (act_status == POEDRV_TYPE_PORT_POWERED_RES_DETECT))
                no_need_polling[port-1] = FALSE;
            else
                continue;
        }
        else
        {
            if ((act_status != POEDRV_TYPE_PORT_POWERED_CAP_DETECT) && (act_status != POEDRV_TYPE_PORT_POWERED_RES_DETECT))
                no_need_polling[port-1] = TRUE;
        }

        /* Get power consumption by port
        */
        ret = POEDRV_LocalGetPortPowerConsumption(my_unit, port, &power_consumption);

        if ( ret == FALSE )
        {
            continue;
        }

        POEDRV_OM_GetPortInfoPowerConsumption(port, &old_power_consumption);
        if ( old_power_consumption != power_consumption )
        {
            POEDRV_OM_SetPortInfoPowerConsumption(port, power_consumption);
            POEDRV_Notify_PortPowerConsumptionChange(my_unit, port, power_consumption);
        }

        if (POEDRV_POLLING_POWER_CLASS)
        {
            if ( hw_revision == POEDRV_HW_REVISION) /*RON*/
            {
                if (POEDRV_LocalGetPortPowerClassification(my_unit, port, &power_class) == TRUE)
                {
                    if ( (power_class <0) || (power_class >4) )
                        continue;
                    else
                    {
                        if (power_class==0)  power_class=VAL_pethPsePortPowerClassifications_class0;
                        else if (power_class==1)  power_class=VAL_pethPsePortPowerClassifications_class1;
                        else if (power_class==2)  power_class=VAL_pethPsePortPowerClassifications_class2;
                        else if (power_class==3)  power_class=VAL_pethPsePortPowerClassifications_class3;
                        else if (power_class==4)  power_class=VAL_pethPsePortPowerClassifications_class4;
                    }
                }
                else
                    continue;
            }
            else
            {
                /* Classify the power consumption by value defined on 802.3af standard
                 */
                if ( power_consumption < POEDRV_TYPE_MAX_POWER_CLASS_1 )            /* max. 4.0 watts  */
                    power_class = VAL_pethPsePortPowerClassifications_class1;

                else if ( power_consumption < POEDRV_TYPE_MAX_POWER_CLASS_2 )       /* max. 7.0 watts  */
                    power_class = VAL_pethPsePortPowerClassifications_class2;

                else if ( power_consumption < POEDRV_TYPE_MAX_POWER_CLASS_3 )       /* max. 15.4 watts */
                    power_class = VAL_pethPsePortPowerClassifications_class3;

                else if ( power_consumption < POEDRV_TYPE_MAX_POWER_CLASS_0 )       /* max. 21 watts   */
                    power_class = VAL_pethPsePortPowerClassifications_class0;

                else
                    continue;
            }/* End of if ( hw_revision == 5) RON */

            POEDRV_OM_GetPortInfoPowerClass(port, &old_power_class);
            if ( old_power_class != power_class )
            {
                POEDRV_OM_SetPortInfoPowerClass(port, power_class);
                POEDRV_Notify_PortPowerClassificationChange(my_unit, port, power_class);
            }
        }

//         SYSFUN_Sleep(2);
    } /* End of for */

    count = (count+1) % POLLING_TIME;

    return TRUE;
} /* End of POEDRV_MonitorPoePortClassification() */


/* FUNCTION NAME : POEDRV_MonitorPoeMainpowerStatus
 * PURPOSE: This function is used to periodically query PoE mainpower status and
 *          update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE: Successfully, FALSE: Failed
 * NOTES:
 */
static BOOL_T POEDRV_MonitorPoeMainpowerStatus(void)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    static UI32_T               system_power_status = POEDRV_TYPE_SYSTEM_NORMAL;
    UI32_T                      old_power_consumption = 0, power_consumption = 0, unit = 0;
    static UI32_T               maximum_power = SYS_DFLT_MAIN_PSE_POWER_MAX_ALLOCATION;
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));

    POEDRV_OM_GetMainPowerMaxAllocation(&maximum_power);

    if (!dragonitep)
        return FALSE;

    POEDRV_OM_SetMainPowerInfoMainPower(maximum_power);
    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_SYSTEM_INFORMATION, 0, 0, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    /* sysTotalRealPowerCons: LSB=0.1 W
     * change unit to milliWatt (should * 100)
     */
    power_consumption = dragonitep->dragoniteData.system.sysTotalRealPowerCons * 100;
    free(dragonitep);
    POEDRV_OM_GetMainPowerInfoMainConsumption(&old_power_consumption);
    if (old_power_consumption != power_consumption)
    {
        POEDRV_OM_SetMainPowerInfoMainConsumption(power_consumption);
        POEDRV_OM_GetMyUnitID(&unit);
        POEDRV_Notify_MainPseConsumptionChange(unit, power_consumption);

        /* *100: calculate percentage
         */
        if ((((power_consumption * 100) / maximum_power) > POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_NORMAL))
        {
            system_power_status = POEDRV_TYPE_SYSTEM_OVERLOAD;
            POEDRV_Notify_IsMainPowerReachMaximun(unit, system_power_status);
            if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("%s(%d): system_power_status = POEDRV_TYPE_SYSTEM_OVERLOAD.\r\n", __FUNCTION__, __LINE__); /* weihsiang_test */
        }
        /* *100: calculate percentage
         */
        else if ((((power_consumption * 100) / maximum_power) < POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_OVERLOAD))
        {
            system_power_status = POEDRV_TYPE_SYSTEM_NORMAL;
            POEDRV_Notify_IsMainPowerReachMaximun(unit, system_power_status);
            if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("%s(%d): system_power_status = POEDRV_TYPE_SYSTEM_NORMAL.\r\n", __FUNCTION__, __LINE__); /* weihsiang_test */
        }
    }

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    return TRUE;
#elif defined(ES4552MA_HPOE)
    static UI32_T system_power_status = POEDRV_TYPE_SYSTEM_NORMAL;
    UI32_T board_id = 0xFF;
    UI32_T power_consumption = 0;
    UI32_T maximum_power = 0;
    UI32_T old_power_consumption = 0, unit_id = 0xFF;
    UI8_T *value = (UI8_T *)&power_consumption;
    UI8_T *max_value = (UI8_T *)&maximum_power;
    UI8_T data[2]={0};

#if (SYS_CPNT_POE_MAX_ALLOC_FIXED == FALSE)
    static UI8_T power_consumption_is_greater_than_maximum_power_counter = 0;
    const UI8_T default_power_consumption_is_greater_than_maximum_power_counter = 5;
#endif


    POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
    if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x138C, sizeof(data), data) == FALSE)
        return FALSE;
    max_value[2] = data[0];
    max_value[3] = data[1];
    maximum_power = L_STDLIB_Ntoh32(maximum_power) * 100; /* Milliwatts power style. */

    data[0] = 0;
    data[1] = 1;
    if (POEDRV_SetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x139C, data, sizeof(data)) == FALSE)
        return FALSE;
    if (POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x12E8, sizeof(data), data) == FALSE)
        return FALSE;
    value[2] = data[0];
    value[3] = data[1];
    power_consumption = L_STDLIB_Ntoh32(power_consumption) * 100; /* Milliwatts power style. */

#if (SYS_CPNT_POE_MAX_ALLOC_FIXED == FALSE)

    /* The value got from the registers in PoE Chip might not reflect the real settings when the configuration to the PoE chip just performed.
     */
    if (power_consumption > maximum_power)
    {
        if (power_consumption_is_greater_than_maximum_power_counter >= default_power_consumption_is_greater_than_maximum_power_counter)
        {
            printf("%s(%d): Total power consumption (%lumW) is greater than mainpower maximum allocation (%lumW).\r\n", __FUNCTION__, __LINE__, power_consumption, maximum_power);
        }
        else
        {
            power_consumption_is_greater_than_maximum_power_counter++;
        }
        return TRUE;    
    }
    else
        power_consumption_is_greater_than_maximum_power_counter = 0;
#endif

    /* Andy  */
    POEDRV_OM_GetMainPowerInfoMainConsumption(&old_power_consumption);
    if (old_power_consumption != power_consumption)
    {
        POEDRV_OM_SetMainPowerInfoMainPower(maximum_power);
        POEDRV_OM_SetMainPowerInfoMainConsumption(power_consumption);
        POEDRV_OM_GetMyUnitID(&unit_id);
        POEDRV_Notify_MainPseConsumptionChange(unit_id, power_consumption);

        if ((((power_consumption * 100) / maximum_power) >= POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_NORMAL))
        {
            system_power_status = POEDRV_TYPE_SYSTEM_OVERLOAD;
            POEDRV_Notify_IsMainPowerReachMaximun(unit_id, system_power_status);
        }
        else if ((((power_consumption * 100) / maximum_power) < POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_OVERLOAD))
        {
            system_power_status = POEDRV_TYPE_SYSTEM_NORMAL;
            POEDRV_Notify_IsMainPowerReachMaximun(unit_id, system_power_status);
        }
    }

    return TRUE;
#else
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    BOOL_T ret = FALSE;


        /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
         */
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        /* Prepare request buffer for sending this command to PoE controller
         */
        transmit.key          = POEDRV_TYPE_KEY_REQUEST;
        transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
        transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
        transmit.subject2     = POEDRV_TYPE_SUBJECT2_MAIN;

        /* Send this request packet to PoE controller and expect to receive
         * a response packet from PoE controller via UART interface
         */
        POEDRV_ENTER_CRITICAL_SECTION;
        ret = POEDRV_SendAndWaitReply(&transmit, &receive);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (ret == FALSE)
        {

#ifdef POEDRV_DEBUG
            printf("\n\r***Failed in the POEDRV_MonitorPoeMainpowerStatus!***");
#endif

        }
        else
        {

        /* Get maximum available power for power management.
             */
            max_value[2] = receive.data11;
            max_value[3] = receive.data12;
            maximum_power= L_STDLIB_Ntoh32(maximum_power);
            POEDRV_OM_SetMainPowerInfoMainPower(maximum_power);

        /* Get power consumption for PoE controller.
             */
            value[2] = receive.main_subject;
            value[3] = receive.subject1;
            power_consumption= L_STDLIB_Ntoh32(power_consumption);

            /* Andy  */
            POEDRV_OM_GetMainPowerInfoMainConsumption(&old_power_consumption);
            if ( old_power_consumption != power_consumption )
            {
                POEDRV_OM_SetMainPowerInfoMainConsumption(power_consumption);
            POEDRV_OM_GetMyUnitID(&unit_id);
            POEDRV_Notify_MainPseConsumptionChange(unit_id, power_consumption);

            if ((((power_consumption * 100) / maximum_power) > POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_NORMAL))
                {
                system_power_status = POEDRV_TYPE_SYSTEM_OVELROAD;
                POEDRV_Notify_IsMainPowerReachMaximun(unit_id, system_power_status);
                }
            else if ((((power_consumption * 100) / maximum_power) < POEDRV_TYPE_SYSTEM_OVERLOAD_ALARM_THRESHOLD) && (system_power_status == POEDRV_TYPE_SYSTEM_OVERLOAD))
                {
                    system_power_status = POEDRV_TYPE_SYSTEM_NORMAL;
                POEDRV_Notify_IsMainPowerReachMaximun(unit_id, system_power_status);
                }
            }
        }

        return TRUE;
#endif

#endif

} /* End of POEDRV_MonitorPoeMainpowerStatus() */


/* FUNCTION NAME : POEDRV_MonitorPoeOperationStatus
 * PURPOSE: This function is used to periodically query main PSE operation status
 *           and update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_MonitorPoeOperationStatus()
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    UI32_T                      unit = 1;
    UI8_T                       old_status = VAL_pethMainPseOperStatus_faulty, new_status = VAL_pethMainPseOperStatus_faulty;
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_GET_CHIPSET_INFORMATION, 0, 0, 0, dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (dragonitep->dragoniteData.chip[0].vmainHighError == GT_FALSE &&\
        dragonitep->dragoniteData.chip[0].overTempError == GT_FALSE &&\
        dragonitep->dragoniteData.chip[0].disablePortsActiveError == GT_FALSE &&\
        dragonitep->dragoniteData.chip[0].vmainLowAfError == GT_FALSE &&\
        dragonitep->dragoniteData.chip[0].vmainLowAtError == GT_FALSE &&\
        dragonitep->dragoniteData.chip[0].tempAlarm == GT_FALSE)
        new_status = VAL_pethMainPseOperStatus_on;
    else if (dragonitep->dragoniteData.chip[0].disablePortsActiveError == GT_TRUE)
        new_status = VAL_pethMainPseOperStatus_off;
    else
        new_status = VAL_pethMainPseOperStatus_faulty;
    free(dragonitep);

    POEDRV_OM_GetMainPowerInfoMainOperStatus(&old_status);

    if (old_status != new_status)
    {
        POEDRV_OM_SetMainPowerInfoMainOperStatus(new_status);
        POEDRV_OM_GetMyUnitID(&unit);
        POEDRV_Notify_PseOperStatusChange(unit, new_status);
    }

    return TRUE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C)

#ifdef ECS4510_12PD
    UI32_T unit = 1, port = SYS_ADPT_POE_PSE_MIN_PORT_NUMBER;
    UI8_T  old_status = VAL_pethMainPseOperStatus_faulty, new_status = VAL_pethMainPseOperStatus_faulty;
    UI8_T  detection_status = VAL_pethPsePortDetectionStatus_fault;

    /* For ECS4510_12PD, there is only one poe port. The MainOperStatus is
     * decided by this port
     */
    POEDRV_OM_GetPortInfoDetectionStatus(port, &detection_status);

    if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("\r\n %s-%d detection_status:%d", __FUNCTION__, __LINE__, detection_status);

    if(detection_status == VAL_pethPsePortDetectionStatus_deliveringPower)
        new_status = VAL_pethMainPseOperStatus_on;
    else
        new_status = VAL_pethMainPseOperStatus_off;

    POEDRV_OM_GetMainPowerInfoMainOperStatus(&old_status);

    if(POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("\r\n %s-%d old_status:%d", __FUNCTION__, __LINE__, old_status);

    if (old_status != new_status)
    {
        POEDRV_OM_SetMainPowerInfoMainOperStatus(new_status);
        POEDRV_OM_GetMyUnitID(&unit);
        POEDRV_Notify_PseOperStatusChange(unit, new_status);
    }
    return TRUE;
#elif defined(ES4552MA_HPOE)
    UI32_T unit_id = 0xFF, board_id = 0xFF;
    UI8_T data[2] = {0xFF, 0xFF};
    UI8_T old_status = VAL_pethMainPseOperStatus_faulty, new_status = VAL_pethMainPseOperStatus_faulty;
    BOOL_T ret = FALSE;


    POEDRV_OM_GetMainPowerInfoBoardID(&board_id);
    if ((ret = POEDRV_GetPoERegister(0, 1, ((board_id == 1) ? 0x30 : 0x20), 0x1314, sizeof(data), data)) == TRUE)
    {
        if (data[0] == 0 && data[1] == 0)
            new_status = VAL_pethMainPseOperStatus_on;
        else if (data[0] == 0 && data[1] == 4)
            new_status = VAL_pethMainPseOperStatus_off;
        else
            new_status = VAL_pethMainPseOperStatus_faulty;

        POEDRV_OM_GetMainPowerInfoMainOperStatus(&old_status);
        if (old_status != new_status)
        {
            POEDRV_OM_SetMainPowerInfoMainOperStatus(new_status);
            POEDRV_OM_GetMyUnitID(&unit_id);
            POEDRV_Notify_PseOperStatusChange(unit_id, new_status);
        }
    }

    return ret;
#else
    UI32_T unit_id = 0xFF;
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    UI8_T old_status = VAL_pethMainPseOperStatus_on, new_status = VAL_pethMainPseOperStatus_on;
    BOOL_T ret = FALSE;


    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller,Get System Status.
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SYSTEM_STATUS;

    /* Send this request packet to PoE controller and expect to receive a response packet from PoE controller via UART interface.
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {

#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_MonitorPoeOperationStatus!***");
#endif

    }
    else
    {
        new_status = (receive.data6 == 0) ? VAL_pethMainPseOperStatus_on : VAL_pethMainPseOperStatus_faulty;
        POEDRV_OM_GetMainPowerInfoMainOperStatus(&old_status);
        if (old_status != new_status)
        {
            POEDRV_OM_SetMainPowerInfoMainOperStatus(new_status);
            POEDRV_OM_GetMyUnitID(&unit_id);
            POEDRV_Notify_PseOperStatusChange(unit_id, new_status);
        }
   }

   return ret;
#endif

#else
    return TRUE;
#endif

} /* End of POEDRV_MonitorPoeOperationStatus() */

#if 0
/* FUNCTION NAME : POEDRV_MonitorSystemMask
 * PURPOSE: This function is used to periodically query system mask
 *           and update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_MonitorSystemMask(UI8_T *oper_status)
{
    if (POEDRV_GetSystemMask(oper_status) != TRUE) return FALSE;
    else
    {
     if (
          poedrv_mainpower_info.legacy_detection_enable !=
          ( ( (*oper_status) & POEDRV_SYSTEM_MASK_BIT1_enable ) >>1 )
        )
        {
            poedrv_mainpower_info.legacy_detection_enable =
            ( ( (*oper_status) & POEDRV_SYSTEM_MASK_BIT1_enable ) >>1 );
            POEDRV_Notify_LegacyDetectionStatusChange(poedrv_my_unit_id, poedrv_mainpower_info.legacy_detection_enable);
        }
     return TRUE;
    }
}/* End of POEDRV_MonitorSystemMask */
#endif

/* FUNCTION NAME : POEDRV_SendSoftwareDownload
 * PURPOSE: This function is used to send a program command for software download
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_SendSoftwareDownload(void)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_PROGRAM;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_FLASH;
    transmit.subject1     = 0x99;
    transmit.subject2     = 0x15;
    transmit.data6        = 0x16;
    transmit.data7        = 0x16;
    transmit.data8        = 0x99;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_SendSoftwareDownload!***");
#endif
    }

    return ret;

} /* End of POEDRV_SendSoftwareDownload() */


/* FUNCTION NAME : POEDRV_Notify_PortDetectionStatusChange
 * PURPOSE: This function is used to notify the callback function that
 *          the status of a port has been changed.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          detection_status -- detection status of a port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PortDetectionStatusChange(UI32_T unit, UI32_T port, UI32_T detection_status)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, port:%2ld, status:%ld\r\n",__FUNCTION__, __LINE__, unit, port, detection_status);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_DETECTION_STATUS)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_DETECTION_STATUS;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.u32 = detection_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(SYS_MODULE_POEDRV, unit, port, detection_status);
//    for(fun_list=PortDetectionStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, detection_status);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortDetectionStatusChange() */

/*for POE Led*/
static void POEDRV_Notify_PortStatusChange(UI32_T unit, UI32_T port, UI32_T actual_port_status)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, port:%2ld, status:%ld\r\n",__FUNCTION__, __LINE__, unit, port, actual_port_status);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_STATUS)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_STATUS;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.u32 = actual_port_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortStatusChange(SYS_MODULE_POEDRV, unit, port, actual_port_status);
//    for(fun_list=PortStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, actual_port_status);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortStatusChange() */


/* FUNCTION NAME : POEDRV_Notify_PortOverloadStatusChange
 * PURPOSE: This function is used to notify the callback function that
 *          overload status of a port has been changed.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          is_overload -- overload status of a port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PortOverloadStatusChange(UI32_T unit, UI32_T port, BOOL_T is_overload)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, port:%2ld, is_overload:%d\r\n",__FUNCTION__, __LINE__, unit, port, is_overload);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_OVERLOAD_STATUS)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_OVERLOAD_STATUS;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.boolean = is_overload;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(SYS_MODULE_POEDRV, unit, port, is_overload);
//    for(fun_list=PortOverloadStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, is_overload);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortOverloadStatusChange() */


/* FUNCTION NAME : POEDRV_Notify_PortFailureStatusChange
 * PURPOSE: This function is used to notify the callback function when
 *          port failure status has been changed.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          is_port_failure -- port failure status
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PortFailureStatusChange(UI32_T unit, UI32_T port, BOOL_T is_port_failure)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, port:%2ld, is_port_failure:%d\r\n",__FUNCTION__, __LINE__, unit, port, is_port_failure);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_FAILURE_STATUS)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_FAILURE_STATUS;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.boolean  = is_port_failure;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange(SYS_MODULE_POEDRV, unit, port, is_port_failure);
//    for(fun_list=PortFailureStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, is_port_failure);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/
} /* End of POEDRV_Notify_PortFailureStatusChange() */

/* FUNCTION NAME : POEDRV_Notify_PortPowerConsumptionChange
 * PURPOSE: This function is used to notify the callback function that
 *          the power class of a port has been changed.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          power_class -- power class of a port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PortPowerConsumptionChange(UI32_T unit, UI32_T port, UI32_T power_consumption)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, port:%2ld, power_consumption:%ld\r\n",__FUNCTION__, __LINE__, unit, port, power_consumption);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_POWER_CONSUMPTION)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_POWER_CONSUMPTION;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.u32 = power_consumption;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(SYS_MODULE_POEDRV, unit, port, power_consumption);
//    for(fun_list=PortPowerConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, power_consumption);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortPowerConsumptionChange() */


/* FUNCTION NAME : POEDRV_Notify_PortPowerClassificationChange
 * PURPOSE: This function is used to notify the callback function that
 *          the power class of a port has been changed.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          power_class -- power class of a port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PortPowerClassificationChange(UI32_T unit, UI32_T port, UI32_T power_class)
{

#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
        printf("%s(%d) unit:%ld, port:%2ld, power_class:%ld\r\n",__FUNCTION__, __LINE__, unit, port, power_class);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PORT_POWER_CLASSIFICATION)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PORT_POWER_CLASSIFICATION;
        isc_buf_p->unit = unit;
        isc_buf_p->port = port;
        isc_buf_p->info.notify.u32 = power_class;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(SYS_MODULE_POEDRV, unit, port, power_class);
//    for(fun_list=PortPowerClassificationChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, port, power_class);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortPowerClassificationChange() */

#if (SYS_CPNT_POE_POWER_TYPE != SYS_CPNT_POE_POWER_TYPE_PD)

/* FUNCTION NAME : POEDRV_Notify_MainPseConsumptionChange
 * PURPOSE: This function is used to notify the callback function that
 *          the status of main power has been changed.
 * INPUT:   unit -- unit ID
 *          power_consumption -- main PSE power consumption
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_MainPseConsumptionChange(UI32_T unit, UI32_T power_consumption)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, main power consumption:%ld mW\r\n",__FUNCTION__, __LINE__, unit, power_consumption);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_MAIN_PSE_CONSUMPTION)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_MAIN_PSE_CONSUMPTION;
        isc_buf_p->unit = unit;
        isc_buf_p->info.notify.u32 = power_consumption;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(SYS_MODULE_POEDRV, unit, power_consumption);
//    for(fun_list=MainPseConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, power_consumption);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_MainPseConsumptionChange() */

static void POEDRV_Notify_IsMainPowerReachMaximun(UI32_T unit, UI32_T status)
{
#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, status:%ld\r\n",__FUNCTION__, __LINE__, unit, status);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_IS_MAIN_POWER_REACH_MAXIMUN)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_IS_MAIN_POWER_REACH_MAXIMUN;
        isc_buf_p->unit = unit;
        isc_buf_p->info.notify.u32 = status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet

                EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
                EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                buff1, buff2);
        */
        return ; /* cannot notify master */

    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun(SYS_MODULE_POEDRV, unit, status);

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_IsMainPowerReachMaximun() */
#endif

/* FUNCTION NAME : POEDRV_Notify_PseOperStatusChange
 * PURPOSE: This function is used to notify the callback function that
 *          the operational status of the main PSE has been changed.
 * INPUT:   unit -- unit ID
 *          oper_status -- operational status on PoE
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PseOperStatusChange(UI32_T unit, UI32_T oper_status)
{

#ifndef INCLUDE_DIAG
    if (POEDRV_BACKDOOR_IsDisplayNotifyFlagOn())
        printf("%s(%d) unit:%ld, oper_status:%ld\r\n",__FUNCTION__, __LINE__, unit, oper_status);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T*  mref_handle_p;
        POEDRV_Rx_IscBuf_T*  isc_buf_p;
        POEDRV_IscReplyBuf_T isc_reply;
        UI32_T pdu_len;
        UI8_T  master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PSE_OPER_STATUS)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        if (isc_buf_p == NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PSE_OPER_STATUS;
        isc_buf_p->unit = unit;
        isc_buf_p->info.notify.u32 = oper_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);

        if(ISC_RemoteCall(master_unit_id, ISC_POEDRV_SID, mref_handle_p,
                          SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                          sizeof(isc_reply), (UI8_T *)&isc_reply,
                          POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet
        EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
        EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
        buff1, buff2);
        */

        return ; /* cannot notify master */
    }
    else
    {
#endif
        SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(SYS_MODULE_POEDRV, unit, oper_status);
//    for(fun_list=PseOperStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
//        fun_list->func(unit, oper_status);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/
} /* End of POEDRV_Notify_PseOperStatusChange() */

#if 0
static void POEDRV_Notify_LegacyDetectionStatusChange(UI32_T unit, UI8_T oper_status)
{
    SYS_TYPE_CallBack_T  *fun_list;

#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit = %ld][Legacy_Detection = %d][Notify_LegacyDetectionStatusChange]",unit, oper_status);
#endif

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    /* if slave unit
     */
    if(POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        POEDRV_Rx_IscBuf_T req_buf;

        req_buf.serviceID = POEDRV_SET_CAPACITOR_DETECTION_CONTROL;
        req_buf.unit = unit;
        req_buf.info.notify.legacy_detection_enable = oper_status;
        if(ISC_Remote_Call(MASTER_UNIT, ISC_POEDRV_SID, sizeof(req_buf), (UI8_T *)&req_buf,  0, NULL, POEDRV_RETRY_TIMES /*retry count*/,
            POEDRV_TIME_OUT))
        {
            return ;
        }
        /*EH not implement yet
        EH_MGR_Handle_Exception2(SYS_MODULE_SWITCH, POEDRV_Notify_MainPseConsumptionChange_FUN_NO,
        EH_TYPE_MSG_SLAVE_UNIT_NOTIFY, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
        buff1, buff2);
        */

        return ; /* cannot notify master */
    }
    else
    {
#endif

    for(fun_list=Legacy_Detection_callbacklist; fun_list; fun_list=fun_list->next)
        fun_list->func(unit, oper_status);
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif/*SYS_CPNT_STACKING*/
} /*End of POEDRV_Notify_LegacyDetectionStatusChange*/
#endif

/* FUNCTION NAME : POEDRV_SendAndWaitReply
 * PURPOSE: This function is used to send a request to PoE controller, and
 *          then wait the reply from PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   This function also provide the service for the calculation
 *          and verification of checksum on all Tx and Rx packet. It also
 *          automatically generate the echo number, incremented by 1.
 */
static BOOL_T POEDRV_SendAndWaitReply(POEDRV_TYPE_PktBuf_T *tx_buf, POEDRV_TYPE_PktBuf_T *rx_buf)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)
    return FALSE;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_UART)
    UI16_T                 checksum = 0;
    UI8_T                  *chksum_ptr;
    BOOL_T                 ret = FALSE;
    BOOL_T                 resend_this_packet = TRUE;
    UI8_T                  retry_times = 0;
    UI8_T                  report_status;
    /* Eugene add for PoE interrupt issue
     */
    POEDRV_TYPE_PktBuf_T   test_buf;
    memset(&test_buf, 0, sizeof(POEDRV_TYPE_PktBuf_T));

    /* Increment the sequence number for this packet
     */
    INCREMENT_ECHO_NUMBER();

    /* Calculate the checksum for this transmitting packet
     */
    tx_buf->echo_no      = poedrv_echo_number;
    checksum             = POEDRV_Checksum((UI8_T *)tx_buf);
    checksum             = L_STDLIB_Hton16(checksum);  /* Andy  */
    chksum_ptr           = (UI8_T *)&checksum;
    tx_buf->checksum_H   = chksum_ptr[0];
    tx_buf->checksum_L   = chksum_ptr[1];

    while ( (resend_this_packet == TRUE) && (retry_times < 2) )
    {
          /* Eugene add for PoE interrupt issue
           */
//Eugene temp,          UARTDRV_FlushBuffer();

          if ( POEDRV_SendPacket((UI8_T *)tx_buf) == TRUE )
          {
               /*  SYSFUN_Sleep(100);   */ /*  Andy */

               if ( POEDRV_ReceivePacket((UI8_T *)rx_buf) == TRUE )
               {
                    /* Calculate the checksum for this receiving packet
                     */
                    checksum   =POEDRV_Checksum((UI8_T *)rx_buf);
                    checksum   = L_STDLIB_Ntoh16(checksum);   /* andy */
                    chksum_ptr = (UI8_T *)&checksum;

                    if ( (rx_buf->checksum_H == chksum_ptr[0]) &&
                         (rx_buf->checksum_L == chksum_ptr[1]) &&
                         (rx_buf->echo_no == tx_buf->echo_no) )
                    {
                         switch (rx_buf->key)
                         {
                            case POEDRV_TYPE_KEY_REPORT:         /* Report message */
                                 report_status = POEDRV_CheckReportMessage(rx_buf);
                                 if ( (report_status == POEDRV_REPORT_CORRECTLY_EXECUTED) )
                                 {
                                      resend_this_packet = FALSE;
                                      ret = TRUE;
                                 }
                                 else
                                 {
#ifndef INCLUDE_DIAG
                                      if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                                           printf("\n\rError on Response Message: report_status %d", report_status);
#endif
                                      resend_this_packet = TRUE;
                                 }
                                 break;

                            case POEDRV_TYPE_KEY_TELEMETRY:      /* telemetry message */
                                 resend_this_packet = FALSE;
                                 ret = TRUE;
                                 break;

                            case POEDRV_TYPE_KEY_SOFTWARE_DOWNLOAD_TELEMETRY: /* software download telemetry */
                                 resend_this_packet = FALSE;
                                 ret = TRUE;
                                 break;

                            default:                             /* unknown response */
                                 break;
                         } /* End of switch (rx_buf->key) */

                    } /* End of if ( (rx_buf->checksum_H ..) */
#ifdef INCLUDE_DIAG
               }
          }
#else
                    else /* checksum error */
                    {
                        if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                              printf(" - Checksum Error - ");

                        SYSFUN_Sleep(1);
                        POEDRV_PROCESS_DELAY(100);
                    }

               } /* End of if ( POEDRV_ReceivePacket..) */
               else /* receiving error */
               {
                    if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                         printf(" - Receiving Error - ");

                    /* sometime received buffers should be returns error
                     * the solution is keep process content switch and push processing delay
                     */
                    SYSFUN_Sleep(1);
                    POEDRV_PROCESS_DELAY(100);
               }
          } /* End of if ( POEDRV_SendPacket..) */
          else if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
          {
               printf(" - Sending Error - ");
          }
#endif
          retry_times ++;

    } /* End of while (resend_this_packet == TRUE) */

    return ret;
#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && !defined(ECS4510_12PD)
	UI16_T				   checksum = 0;
    UI8_T *chksum_ptr = NULL;
	BOOL_T				   ret = TRUE;
    I32_T device_error; /* contain I2C device error number. */
    static UI8_T poedrv_echo_number = 0; /* Echo number(0x0~0xFE). */
    UI32_T unit = 0;


    /* 1. Return "FALSE" directly if the board doesn't support PoE feature.
     * 2. The function has been protected with the semaphore key "SYS_BLD_SYS_SEMAPHORE_KEY_POEDRV_OM".
     */
    STKTPLG_OM_GetMyUnitID(&unit);
    if (STKTPLG_OM_IsPoeDevice(unit) == FALSE)
        return FALSE;

#if defined(ES4552MA_HPOE)
    if (I2CDRV_SetAndLockMux(0, 1) == FALSE)
    {
        printf("%s(%d): Failed to set and lock mux index %u, channel_bmp = 0x%02X.\r\n",  __FUNCTION__, __LINE__, 0, 1);
        return FALSE;
    }
#endif

    /* Increment the sequence number for this packet
     */
    INCREMENT_ECHO_NUMBER();

    /* Calculate the checksum for this transmitting packet
     */
    tx_buf->echo_no 	 = poedrv_echo_number;
    checksum			 = POEDRV_Checksum((UI8_T *)tx_buf);
    chksum_ptr			 = (UI8_T *)&checksum;
    tx_buf->checksum_H	 = chksum_ptr[0];
    tx_buf->checksum_L	 = chksum_ptr[1];
//	while ( (resend_this_packet == TRUE) && (retry_times < 3) )
    {
//		if ( POEDRV_SendPacket((UI8_T *)tx_buf) == TRUE )

//        SYSFUN_Sleep(1);
        if (DEV_SWDRV_PMGR_MSCC_POE_Write((UI8_T *) tx_buf, 15, &device_error))
        {
//    SYSFUN_Sleep(1);
#if 1
            if( tx_buf->main_subject == 0x06 )
                SYSFUN_Sleep(30);
            else if( (tx_buf->subject1== 0x55) && (tx_buf->data6== 0x55) && (tx_buf->data8== 0x55) )
                SYSFUN_Sleep(400);
            else
                SYSFUN_Sleep(5);
#endif
//			if ( POEDRV_ReceivePacket((UI8_T *)rx_buf) == TRUE )
            if (DEV_SWDRV_PMGR_MSCC_POE_Read((UI8_T *) rx_buf, 15, &device_error))
            {
                /* Calculate the checksum for this receiving packet
                 */
                checksum   = POEDRV_Checksum((UI8_T *)rx_buf);
                chksum_ptr = (UI8_T *)&checksum;

                if ((rx_buf->checksum_H == chksum_ptr[0]) &&
                     (rx_buf->checksum_L == chksum_ptr[1]) &&
                     (rx_buf->echo_no == tx_buf->echo_no))
                {
                    switch (rx_buf->key)
                    {
                        case POEDRV_TYPE_KEY_REPORT:            /* Report message */
                            if ( (POEDRV_CheckReportMessage(rx_buf) == POEDRV_REPORT_CORRECTLY_EXECUTED) )
                            {
//                                resend_this_packet = FALSE;
                            }
                            else
                            {
//                                resend_this_packet = TRUE;
                                ret = FALSE;
                            }
                            break;

                        case POEDRV_TYPE_KEY_TELEMETRY:         /* telemetry message */
//                            resend_this_packet = FALSE;
//                            ret = TRUE;
                            break;

                        case POEDRV_TYPE_KEY_SOFTWARE_DOWNLOAD_TELEMETRY: /* software download telemetry */
//                            resend_this_packet = FALSE;
//                            ret = TRUE;
                            break;

                        default:							 /* unknown response */
                            ret = FALSE;
                            break;
                    } /* End of switch (rx_buf->key) */
                } /* End of if ( (rx_buf->checksum_H ..) */
                else
                {
#if 0
                    int i;
                    printf("\nPoE warning checksum wrong %04x != %02x%02x\n",
				         checksum, rx_buf->checksum_H,rx_buf->checksum_L);
                    printf("TX[");
                    for(i=0;i<15;i++)
                        printf("%02x ",((uchar *)tx_buf)[i]);
                    printf("]\n");
                    printf("RX[");
                    for(i=0;i<15;i++)
                        printf("%02x ",((uchar *)rx_buf)[i]);
                    printf("]\n");
//                    SYSFUN_Sleep(10);
#endif
                }
            } /* End of if ( POEDRV_ReceivePacket..) */
            else
            {
                printf("PoE error:MSCC_POE_Read, dev=%ld\r\n", device_error);
            }
//            resend_this_packet = FALSE;

        } /* End of if ( POEDRV_SendPacket..) */
        else
        {
            printf("PoE error:MSCC_POE_Write, dev=%ld\r\n", device_error);
//		retry_times ++;
        }

    } /* End of while (resend_this_packet == TRUE) */

#if defined(ES4552MA_HPOE)
    if (I2CDRV_UnLockMux(0) == FALSE)
    {
        printf("%s(%d): Failed to unlock mux index %u.\r\n",  __FUNCTION__, __LINE__, 0);
        ret = FALSE;
    }
#endif

    return ret;
#else
    return FALSE;
#endif

} /* End of POEDRV_SendAndWaitReply() */

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && !defined(ECS4510_12PD)

/* FUNCTION NAME : POEDRV_CheckReportMessage
 * PURPOSE: This function is used to examine the report message, received from
 *          the PoE controller as a response to a Command or Program or wrong
 *          Request, after a maximum period of 400ms.
 * INPUT:   buf -- data pointer for report message
 * OUTPUT:  None
 * RETURN:  POEDRV_REPORT_UNKNOWN_MESSAGE
 *          POEDRV_REPORT_CORRECTLY_EXECUTED
 *          POEDRV_REPORT_WRONG_CHECKSUM
 *          POEDRV_REPORT_CONFLICT_IN_SUBJECT_BYTES
 *          POEDRV_REPORT_WRONG_DATA_BYTE_VALUE
 *          POEDRV_REPORT_UNDEFINED_KEY_VALUE
 * NOTES:   None
 */
static UI8_T POEDRV_CheckReportMessage(POEDRV_TYPE_PktBuf_T *buf)
{
    /* Command received, and correctly executed
     */
    if ( (buf->main_subject == 0) && (buf->subject1 == 0) )
          return POEDRV_REPORT_CORRECTLY_EXECUTED;

    /* Command received but wrong checksum
     */
    if ( (buf->main_subject == 0xFF) && (buf->subject1 == 0xFF) &&
         (buf->subject2 == 0xFF) && (buf->data6 == 0xFF) )
          return POEDRV_REPORT_WRONG_CHECKSUM;

    /* Failed execution with conflict in subject bytes
     */
    if ( (buf->subject1 >= 0x01) && (buf->main_subject <= 0x7F) )
          return POEDRV_REPORT_CONFLICT_IN_SUBJECT_BYTES;

    /* Failed execution with wrong data byte value
     */
    if ( (buf->main_subject >= 0x80) && (buf->main_subject <= 0x8F) )
          return POEDRV_REPORT_WRONG_DATA_BYTE_VALUE;

    /* Failed execution with undefined key value
     */
    if ( (buf->main_subject == 0xFF) && (buf->subject1 == 0xFF) )
          return POEDRV_REPORT_UNDEFINED_KEY_VALUE;

    /* Unknown report message
     */
    return POEDRV_REPORT_UNKNOWN_MESSAGE;


} /* End of POEDRV_CheckReportMessage() */

/* FUNCTION NAME : POEDRV_Checksum
 * PURPOSE: This function is used to calculate the arithmetic sum of the
 *          first 13 message bytes from the data buffer.
 * INPUT:   buffer -- starting address for calculating checksum
 * OUTPUT:  None
 * RETURN:  checksum -- the 2-byte result of checksum
 * NOTES:   None
 */
static UI16_T POEDRV_Checksum(UI8_T *buffer)
{
     UI8_T              index;
     UI16_T             checksum = 0;

     for (index=0; index<POEDRV_NO_OF_BYTES_FOR_CHECKSUM;index++)
          checksum += buffer[index];

     return checksum;

} /* End of POEDRV_Checksum() */
#endif

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_UART)

/* FUNCTION NAME : POEDRV_SendPacket
 * PURPOSE: This function is used to send a packet to PoE controller
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   None
 */
static BOOL_T POEDRV_SendPacket(UI8_T *tx_buf)
{
    BOOL_T             ret=0;

#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
         POEDRV_BACKDOOR_DisplayPacket(TRUE, tx_buf);
#endif

    POEDRV_PROCESS_DELAY(100);

    //int_mask = SYSFUN_InterruptLock ();             /*  Interrupt Disable   */
//Eugene temp,    ret = UARTDRV_PutStringToUart( UARTDRV_UART2, tx_buf, POEDRV_SIZE_OF_PACKET );
    //SYSFUN_InterruptUnlock (int_mask);              /*  Interrupt Enable    */

    return ret;

} /* End of POEDRV_SendPacket() */

/* FUNCTION NAME : POEDRV_ReceivePacket
 * PURPOSE: This function is used to receive a packet from PoE controller
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   None
 */
static BOOL_T POEDRV_ReceivePacket(UI8_T *rx_buf)
{
    UI32_T             starting_ticks = 0;
    UI32_T             current_ticks = 0;
    I32_T              read_bytes = 0;
    UI32_T             length = 0;
    UI8_T              *buf_ptr = rx_buf;
//Eugene temp,    UI32_T             bytes_to_read = POEDRV_SIZE_OF_PACKET;

    starting_ticks = SYS_TIME_GetSystemTicksBy10ms();

    while ( TRUE )
    {
          /* Read bytes from UART interface
           */
          //int_mask = SYSFUN_InterruptLock ();             /*  Interrupt Disable   */
//Eugene temp,          read_bytes = UARTDRV_GetString(POEDRV_SIZE_OF_PACKET-length, buf_ptr);
          //SYSFUN_InterruptUnlock (int_mask);              /*  Interrupt Enable    */

          /* Check to see if got bytes
           */
          if ( read_bytes > 0)
          {
               length += read_bytes;

               /* Check to see if 15-byte packet is received
                */
               if ( length == POEDRV_SIZE_OF_PACKET )
                    break;

#ifndef INCLUDE_DIAG
               if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                    printf(" - length %ld - ", length);
#endif
                if ( length > POEDRV_SIZE_OF_PACKET )
                {
#ifdef POEDRV_DEBUG
                    printf("\r\nPOEDRV_ReceivePacket receive 0x%08lx bytes",length);
#endif
                    return FALSE;
                }
               buf_ptr += read_bytes;      /*length;*/
          }

          current_ticks = SYS_TIME_GetSystemTicksBy10ms();

          /* Time-out checking
           */
          if ( (current_ticks - starting_ticks) >= 40 )
          {
#ifndef INCLUDE_DIAG
               if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
                    printf("\n\rTime out: Starting ticks=%ld Current ticks=%ld", starting_ticks, current_ticks);

               if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
                    POEDRV_BACKDOOR_DisplayPacket(FALSE, rx_buf);
#endif
               /* Return failed due to timeout
                */
#ifdef POEDRV_DEBUG
                    printf("\r\nPOEDRV_ReceivePacket receive timeout");
#endif
               POEDRV_PROCESS_DELAY(100);
               return FALSE;
          }

          SYSFUN_Sleep(1);

    } /* End of while ( TRUE ) */

#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayPacketFlagOn() )
         POEDRV_BACKDOOR_DisplayPacket(FALSE, rx_buf);
#endif

    return TRUE;

} /* End of POEDRV_ReceivePacket() */
#endif

/* FUNCTION NAME: POEDRV_GetLine
 * PURPOSE: This function is used to get a line from buffer
 * INPUT:   line_buf : buffer to get a line
 *          line_size: maximum size of a line
 * OUTPUT:  None
 * RETURN:  size of a line
 * NOTES:
 *
 */
static UI32_T POEDRV_GetLine(UI8_T *buf, UI8_T *line_buf, UI32_T line_size)
{
    UI8_T        c;
    UI32_T       size;

    for (size=0; size<line_size-1 && (c=buf[size]) != 0 && c!='\n'; ++size)
    {
        line_buf[size] = c;
    }
    if (c == '\n')
    {
        line_buf[size] = c;
        ++size;
    }
    line_buf[size] = '\0';
    return size;

} /* End of POEDRV_GetLine */

#if 0
/* FUNCTION NAME : POEDRV_LocalSetPortPowerDetectionControl
 * PURPOSE: This function is used to Controls the power detection mechanism
 *          of a port.
 * INPUT:   port -- port ID
 *          mode -- VAL_pethPsePortPowerDetectionControl_auto
 *                  VAL_pethPsePortPowerDetectionControl_test
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   auto: enables the power detection mechanism of the port.
 *          test: force continuous discovery without applying power
 *          regardless of whether PD detected.
 */

static BOOL_T POEDRV_LocalSetPortPowerDetectionControl(UI32_T port,UI32_T mode)
{
    UI32_T                 phy_unit;       /* Physical unit ID      */
    UI32_T                 phy_port;       /* Physical port ID      */
    BOOL_T                 ret;

    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */

    /* Check illegal port ID
     */
    if ( (port < POEDRV_TYPE_PSE_MIN_PORT_NUMBER) || (port > POEDRV_TYPE_PSE_MAX_PORT_NUMBER) )
    {
         return FALSE;
    }

    /* Mapping logical port ID to physical port ID
     */
    if ( port == 0 )
    {
         phy_port = POEDRV_ALL_PORTS;
    }
    else if ( BCM_Logical2PhyDevicePortID(0, port, &phy_unit, &phy_port) == FALSE)
         {
              return FALSE;
         }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_CHANNEL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_DETECT_TEST;
    transmit.subject2     = (UI8_T)phy_port;

    if ( mode == VAL_pethPsePortPowerDetectionControl_auto )
    {
            transmit.subject1   =   POEDRV_TYPE_SUBJECT1_DETECT_TEST;
         transmit.data6 = MODE_AUTO;
    }
    else if ( mode == VAL_pethPsePortPowerDetectionControl_test )
    {
            transmit.subject1     =     POEDRV_TYPE_SUBJECT1_DETECT_TEST;
              transmit.data6 = MODE_TEST;
    }
    else
    {
         return FALSE;
    }

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_LocalSetPortPowerDetectionControl!***");
#endif
    }

    return ret;

} /* End of POEDRV_SetPortPowerDetectionControl() */
#endif /*Disable*/

BOOL_T POEDRV_SetCapacitorDetectionControl(UI32_T unit, UI8_T value)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if (my_unit != unit)
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T               pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_CAPACITOR_DETECTION_CONTROL)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            /* set remote unit port Power Detection Control
             */
            isc_buf_p->serviceID = POEDRV_SET_CAPACITOR_DETECTION_CONTROL;
            isc_buf_p->info.u8 = value;

            if (!ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                sizeof(isc_reply), (UI8_T *)&isc_reply,
                                POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /* SYS_CPNT_STACKING */
            /* if local unit or standalone
             */


            if (POEDRV_LocalSetCapacitorDetectionControl(value) == FALSE)
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return TRUE;
}

static BOOL_T POEDRV_LocalSetCapacitorDetectionControl(UI8_T value)
{

#if (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_DRAGONITE)

#if 1
    /* GT_BAD_STATE - Config is not allowed - Application can
     *   send config structure to DRAGONITE controller only once after reset.
     *
     * When we try to config "capDis" bit of CONFIG area,
     *   "cpssDragoniteReadData()" will return the "GT_BAD_STATE" error to us.
     */
    printf("The feature is not supported by MARVELL dragonite SDK.\r\n");
    return TRUE;
#else
    UI8_T old_status;
    CPSS_GEN_DRAGONITE_DATA_STC *dragonitep = (CPSS_GEN_DRAGONITE_DATA_STC *) malloc(sizeof(CPSS_GEN_DRAGONITE_DATA_STC));


    if (!dragonitep)
        return FALSE;

    /* Check operating mode
     */
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        free(dragonitep);
        return FALSE;
    }

    if (value != 1 && value != 0)
    {
        free(dragonitep);
        return FALSE; /*range check*/
    }

    POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(&old_status);
    if (value == old_status)
    {
        free(dragonitep);
        return TRUE; /*No change*/
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if (DEV_SWDRV_PMGR_ProcessDragoniteData((UI32_T) POEDRV_DRAGONITE_SET_CONFIG_CAPACITOR_DETECTION, 0, 0, (UI32_T) ((value == 1) ? GT_TRUE : GT_FALSE), dragonitep) == FALSE)
    {
        POEDRV_LEAVE_CRITICAL_SECTION;
        free(dragonitep);
        return FALSE;
    }
    POEDRV_LEAVE_CRITICAL_SECTION;
    free(dragonitep);
    POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(value);

    return TRUE;
#endif

#elif (SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && !defined(ECS4510_12PD)
    POEDRV_TYPE_PktBuf_T transmit; /* Transmit buffer. */
    POEDRV_TYPE_PktBuf_T receive; /* Receive buffer. */
    BOOL_T ret = FALSE;
    UI8_T old_status = 0;
    UI8_T     system_mask=0;


    /* Check operating mode
     */
    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {
         return FALSE;
    }

    if (value !=1 && value !=0)
    {
        return FALSE; /*range check*/
    }

    POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(&old_status);
    if (value == old_status)
    {
        return TRUE; /*No change*/
    }

    if ( (ret = POEDRV_GetSystemMask(&system_mask)) == TRUE )
    {

        /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
         */
        INITIALIZE_PACKET_BUFFER(&transmit, &receive);

        /* Prepare request buffer for sending this command to PoE controller
         */
        transmit.key          = POEDRV_TYPE_KEY_COMMAND;
        transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
        transmit.subject1     = POEDRV_TYPE_SUBJECT1_MASKZ;
        if(value==1)
            system_mask|=POEDRV_SYSTEM_MASK_BIT1_enable;
        else
            system_mask&=POEDRV_SYSTEM_MASK_BIT1_disable;
        transmit.subject2     = system_mask;

        /* Send this request packet to PoE controller and expect to receive a response packet from PoE controller via UART interface.
         */
        POEDRV_ENTER_CRITICAL_SECTION;
        ret = POEDRV_SendAndWaitReply(&transmit, &receive);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (ret == FALSE)
        {
            if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
                printf("\n\r***Failed in the POEDRV_LocalSetCapacitorDetectionControl!***");
        }
        else
        {
            if(value==1)
                POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(1);
            else
                POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(0);
        }
    }
    else
    {
        if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
            printf("\n\r Failed to get system mask");
    }

    return ret;
#else
    if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        printf("%s(%d): This unit doesn't support the feature.\r\n", __FUNCTION__, __LINE__);
    return TRUE;
#endif

} /* End of POEDRV_LocalSetCapacitorDetectionControl */

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
static void SlaveUserPortExisting(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_USER_PORT_EXISTING) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalUserPortExisting(request_p->port);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveSetPortAdminStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_ADMIN_STATUS) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetPortAdminStatus(request_p->port, request_p->info.u32);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

#if 0
static void SlaveSetPortPowerDetectionControl(ISC_Key_T *key, L_MREF_T *mem_ref)
{
    UI8_T status = FALSE,*buf = L_MREF_GetPdu(mem_ref);

    if(POEDRV_LocalSetPortPowerDetectionControl(((POEDRV_Rx_IscBuf_T *)buf)->port,((POEDRV_Rx_IscBuf_T *)buf)->info.mode))
    {
        status = TRUE;
    }
    ISC_Remote_Reply(key,sizeof(status),&status);
    return ;
}
#endif

static void SlaveSetPortPowerPriority(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_POWER_PRIORITY) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetPortPowerPriority(request_p->port, request_p->info.u32);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveSetPortPowerMaximumAllocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_PORT_POWER_MAXIMUM_ALLOCATION) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetPortPowerMaximumAllocation(request_p->port, request_p->info.u32);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveSetMainpowerMaximum_Allocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_MAIN_POWER_MAXIMUM_ALLOCATION) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetMainpowerMaximumAllocation(request_p->info.u32);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveSoftwareDownload(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SOFTWARE_DOWNLOAD) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSoftwareDownload(request_p->info.name_ar);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveGetPoeSoftwareVersion(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_GET_POE_SOFTWARE_VERSION) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalGetPoeSoftwareVersion(&isc_reply_p->data.poe_software_version.version[0], &isc_reply_p->data.poe_software_version.version[1], &isc_reply_p->data.poe_software_version.build_number);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveProvisionComplete(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;
    UI32_T unit = 0;


    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PROVISION_COMPLETE) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    POEDRV_OM_SetProvisionComplete(TRUE);
    POEDRV_ReleaseSoftwareReset(TRUE);

    /* Stop monitor functions when PoE system is working on "BOOTING" status.
     */
    POEDRV_OM_GetMyUnitID(&unit);
    if (STKTPLG_OM_IsPoeDevice(unit) == TRUE)
        POEDRV_OM_SetStopMonitorFlag(FALSE);

    ISC_RemoteReply(mref_handle_p, key);
    return;
}

static void SlaveSetCapacitorDetectionControl(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_CAPACITOR_DETECTION_CONTROL) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetCapacitorDetectionControl(request_p->info.u8);

    ISC_RemoteReply(mref_handle_p, key);

    return;
}

static void SlaveSetClassMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    UI32_T  pdu_len;
    L_MM_Mref_Handle_T* mref_handle_p;
    POEDRV_IscReplyBuf_T *isc_reply_p;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_IscReplyBuf_T), /* tx_buffer_size */
                        L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_CLASS_MODE) /* user_id */);

    if (NULL == (isc_reply_p = (POEDRV_IscReplyBuf_T*)L_MM_Mref_GetPdu (mref_handle_p, &pdu_len)))
    {
        return;
    }

    isc_reply_p->return_value.bool = POEDRV_LocalSetClassMode(request_p->info.boolean);

    ISC_RemoteReply(mref_handle_p, key);

    return;
}

static void CallbackMasterPortDetectionStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.u32);
    //for(fun_list=PortDetectionStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->port, request_p->info.notify.u32);

    return;
}

static void CallbackMasterPortStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortStatusChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.u32);
    //for(fun_list=PortStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit,request_p->port, request_p->info.notify.u32);

    return;
}

static void CallbackMasterPortOverloadStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.boolean);
    //for(fun_list=PortOverloadStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->port,request_p->info.notify.boolean);

    return;
}

static void CallbackMasterPortFailureStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortFailureStatusChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.boolean);
    //for(fun_list=PortFailureStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->port,request_p->info.notify.boolean);

    return;
}

static void CallbackMasterPortPowerConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.u32);
    //for(fun_list=PortPowerConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->port,request_p->info.notify.u32);

    return;
}

static void CallbackMasterPortPowerClassification(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(SYS_MODULE_POEDRV, request_p->unit, request_p->port, request_p->info.notify.u32);
    //for(fun_list=PortPowerClassificationChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->info.notify.u32);

    return;
}

static void CallbackMasterMainPseConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(SYS_MODULE_POEDRV, request_p->unit, request_p->info.notify.u32);
    //for(fun_list=MainPseConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->info.notify.u32);

    return;
}

static void CallbackMasterIsMainPowerReachMaximum(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun(SYS_MODULE_POEDRV, request_p->unit, request_p->info.notify.u32);
    //for(fun_list=IsMainPowerReachMaximum_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->info.notify.u32);

    return;
}

static void CallbackMasterPseOperStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *request_p)
{
    //SYS_TYPE_CallBack_T  *fun_list;

    SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(SYS_MODULE_POEDRV, request_p->unit, request_p->info.notify.u32);
    //for(fun_list=PseOperStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
    //    fun_list->func(request_p->unit, request_p->info.notify.u32);

    return;
}
#if 0
static void CallbackMasterCapacitorDetectionControl(ISC_Key_T *key, L_MREF_T *mem_ref)
{
    SYS_TYPE_CallBack_T  *fun_list;
    UI8_T                *buf = L_MREF_GetPdu(mem_ref);

    ISC_Remote_Reply(key,0,NULL);

    for(fun_list=Legacy_Detection_callbacklist; fun_list; fun_list=fun_list->next)
        fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, (UI8_T) ( ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.legacy_detection_enable) );

    return ;
}
#endif
#endif /* SYS_CPNT_STACKING */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Switch Driver module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POEDRV_ProvisionComplete(void)
{
    UI32_T unit = 0;


#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    L_MM_Mref_Handle_T*  mref_handle_p;
    POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T  pdu_len;
    UI16_T  unit_bmp=0;

    unit_bmp = POEDRV_GetValidDrvUnitBmp();
    if (unit_bmp!=0)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PROVISION_COMPLETE) /* user_id */);
        isc_buf_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buf_p==NULL)
        {
            return;
        }

        isc_buf_p->serviceID = POEDRV_PROVISION_COMPLETE;

        if (ISC_SendMcastReliable(unit_bmp, ISC_POEDRV_SID,
                                  mref_handle_p,
                                  SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                  POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE) != 0)
        {
            SYSFUN_Debug_Printf("\r\n SWDRV: ISC channel is failed to driver unit");
        }
    }
#endif /* SYS_CPNT_STACKING */

    POEDRV_OM_SetProvisionComplete(TRUE);
    POEDRV_ReleaseSoftwareReset(TRUE);

    /* Start monitor functions when PoE system is working on "NORMAL" status.
     */
    POEDRV_OM_GetMyUnitID(&unit);
    if (STKTPLG_OM_IsPoeDevice(unit) == TRUE)
        POEDRV_OM_SetStopMonitorFlag(FALSE);

    return;
} /* POEDRV_ProvisionComplete() */

/* FUNCTION NAME: POEDRV_SetClassMode
 * PURPOSE: This function is used to set class mode
 *
 * INPUT:   unit -- unit ID
 *          class -- POE mode
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SetClassMode(UI32_T unit, BOOL_T ClassMode)
{
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {

    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
        UI8_T  my_unit;

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            L_MM_Mref_Handle_T*  mref_handle_p;
            POEDRV_Rx_IscBuf_T*  isc_buf_p;
            POEDRV_IscReplyBuf_T isc_reply;
            UI32_T pdu_len;

            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                            L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_SET_CLASS_MODE)/* user_id */);
            isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

            if (isc_buf_p == NULL)
            {
                return FALSE;
            }

            isc_buf_p->serviceID = POEDRV_SET_CLASS_MODE;
            isc_buf_p->unit = unit;
            isc_buf_p->info.boolean = ClassMode;

            if(!ISC_RemoteCall(unit, ISC_POEDRV_SID, mref_handle_p,
                               SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                               sizeof(isc_reply), (UI8_T *)&isc_reply,
                               POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
            {
                /* EH not implemented yet
                 */
                return FALSE;
            }

            if (!isc_reply.return_value.bool)
                return FALSE;
        }
        else
        {
#endif /*SYS_CPNT_STACKING*/
            /* if local unit or standalone
             */


            if (POEDRV_LocalSetClassMode(ClassMode) == FALSE)
            {
            /* EH not implemented yet
             */
                return FALSE;
            }
        }
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
    }
#endif /*SYS_CPNT_STACKING*/

   return TRUE;

} /* End of POEDRV_SetClassMode() */

/* FUNCTION NAME: POEDRV_LocalSetClassMode
 * PURPOSE: This function is used to set class mode
 *
 * INPUT:   ClassMode -- PoE system Mode
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalSetClassMode(BOOL_T ClassMode)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }


    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_COMMAND;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_POWER_MANAGEMODE;
    if (ClassMode)
    {
        transmit.data6        = 0x01;
        transmit.data7        = 0x01;
        transmit.data8        = 0x00;
    }
    else
    {
        transmit.data6        = 0x00;
        transmit.data7        = 0x00;
        transmit.data8        = 0x00;
    }

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_GetClassMode!***");
#endif
    }


    return ret;

} /* End of POEDRV_LocalSetClassMode() */

/* FUNCTION NAME: POEDRV_GetClassMode
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetClassMode(BOOL_T *ClassMode)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_SUPPLY;
    transmit.subject2     = POEDRV_TYPE_POWER_MANAGEMODE;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_LocalSetClassMode!***");
#endif
    }

    if ( ret )
    {
        if ( receive.main_subject==0x01 && receive.subject1==0x01 && receive.subject2==0x00)
        {
            *ClassMode= TRUE;
        }
        else
            *ClassMode= FALSE;
    }


    return ret;

} /* End of POEDRV_GetPortPriority() */

/* FUNCTION NAME: POEDRV_GetPoeDeviceTemperature
 * PURPOSE: This function is used to get the temperature for poe device
 * INPUT:   deviceid -- poe device id
 * OUTPUT:  temperature -- the temperature of the device
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPoeDeviceTemperature(UI32_T unit, UI32_T deviceid, UI32_T *temperature)
{
    POEDRV_TYPE_PktBuf_T   transmit;       /* Transmit buffer       */
    POEDRV_TYPE_PktBuf_T   receive;        /* Receive buffer        */
    BOOL_T                 ret;

    if ( (deviceid<0)||(deviceid>=4) )
        return FALSE;

    /* Check operating mode
     */

    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {

         return FALSE;
    }

    /* Initialize transmitting and receving buffer with space "N" and 0, respectively.
     */
    INITIALIZE_PACKET_BUFFER(&transmit, &receive);

    /* Prepare request buffer for sending this command to PoE controller
     */
    transmit.key          = POEDRV_TYPE_KEY_REQUEST;
    transmit.main_subject = POEDRV_TYPE_MAINSUBJECT_GLOBAL;
    transmit.subject1     = POEDRV_TYPE_SUBJECT1_TMPset;

    /* Send this request packet to PoE controller and expect to receive
     * a response packet from PoE controller via UART interface
     */
    POEDRV_ENTER_CRITICAL_SECTION;
    ret = POEDRV_SendAndWaitReply(&transmit, &receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    if (ret == FALSE)
    {
#ifdef POEDRV_DEBUG
        printf("\n\r***Failed in the POEDRV_GetPoeDeviceTemperature!***");
#endif
    }

    if ( ret )
    {
        switch ( deviceid )
        {
            case 0:
                *temperature = receive.data7;
                break;
            case 1:
                *temperature = receive.data8;
                break;
            case 2:
                *temperature = receive.data9;
                break;
            case 3:
                *temperature = receive.data10;
                break;
            default:
                *temperature =0xFF;
                break;
        }
    }


    return ret;

} /* End of POEDRV_GetPoeDeviceTemperature() */

/* -------------------------------------------------------------------------
 * Function : POEDRV_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulate all of POEDRV via ISC
 * INPUT    : *key      -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_Agent
 * -------------------------------------------------------------------------
 */
#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
BOOL_T POEDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    POEDRV_Rx_IscBuf_T   *buf;
    UI32_T  service_id, pdu_len;

DBG_PRINT();
    buf = L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(buf==NULL)
    {
        SYSFUN_Debug_Printf("\r\n%s():L_MM_Mref_GetPdu fails", __FUNCTION__);
        return FALSE;
    }

DBG_PRINT();
    service_id = buf->serviceID;

    /*
     * Check to abort operation if callback service id(opcode) is more then
     * number of callback service on this drive.
     */
DBG_PRINT();
    if(service_id >= POEDRV_NBR_OF_SERVICE_ID || POEDRV_func_tab[service_id]==NULL)
        printf("POEDRV: Service ID [%lu] is invalid.\r\n", service_id);
    else
        POEDRV_func_tab[service_id](key,buf);
    L_MM_Mref_Release(&mref_handle_p);
DBG_PRINT();

    return TRUE;
} /* POEDRV_ISC_Handler() */
#endif /* #if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)) */

void POEDRV_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("poedrv",
    SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, POEDRV_BACKDOOR_Main);
}

static void POEDRV_UpdateStackingInfo(void)
{
    UI32_T value;

#ifdef INCLUDE_DIAG
DBG_PRINT();
    POEDRV_OM_SetMyUnitID(1);
    POEDRV_OM_SetNumOfUnits(1);
#else
    STKTPLG_POM_GetMyUnitID(&value);
    POEDRV_OM_SetMyUnitID(value);
    POEDRV_OM_SetMainPowerInfoUnitID(((UI8_T) value));
    STKTPLG_POM_GetNumberOfUnit(&value);
    POEDRV_OM_SetNumOfUnits(value);
#endif
}

void POEDRV_HotSwapInsert(void)
{
    POEDRV_UpdateStackingInfo();
}

void POEDRV_HotSwapremove(void)
{
    POEDRV_UpdateStackingInfo();
}

#if ((SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1))
/*------------------------------------------------------------------------------
 * FUNCTION NAME: POEDRV_GetValidDrvUnitBmp
 *------------------------------------------------------------------------------
 * PURPOSE: Getting UnitBmp for all exist unit or module.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : unit_bmp    -- which driver units are exist
 * NOTES  : This function is to search all exist unit id by stktplg
 *          then ISC can forward the information to all exist unit.
 *------------------------------------------------------------------------------
 */
static UI16_T POEDRV_GetValidDrvUnitBmp(void)
{
    UI32_T unit_index;
    UI16_T unit_bmp=0;
    UI8_T  stack_id;

    POEDRV_OM_GetMainPowerInfoUnitID(&stack_id);
    for (unit_index = 0; STKTPLG_POM_GetNextDriverUnit(&unit_index); )
    {
        if (unit_index == stack_id)
        {
            continue;
        }
        unit_bmp |= BIT_VALUE(unit_index-1);
    }
    return unit_bmp;
}
#endif

#if ((SYS_CPNT_POE_INTERFACE == SYS_CPNT_POE_INTERFACE_I2C) && (SYS_CPNT_POE_POWER_TYPE != SYS_CPNT_POE_POWER_TYPE_PD))

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_GetPoERegister
 * -------------------------------------------------------------------------
 * FUNCTION: This function performs preprocessing to write data into register of PSE controller
 * INPUT   : i2c_bus_index: I2C bus index
 *           i2c_mux_channel: I2C mux channel
 *           dev_slv_id: Device slave address
 *           offset: The register offset of PSE controller
 *           data_len: length of data.     
 * OUTPUT  : data: The value to be readed from specified register of PSE controller
 * RETURN  : TRUE: If success
 *           FALSE: If failed
 * NOTE    : Need to refer HW design SPEC firstly!
 * -------------------------------------------------------------------------*/
static BOOL_T POEDRV_GetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T data_len, UI8_T *data)
{
    BOOL_T ret = TRUE;


    if (data == NULL)
        return FALSE;
    memset(data, 0, data_len);

    if (I2CDRV_SetAndLockMux(i2c_bus_index, i2c_mux_channel) == FALSE)
    {
        printf("%s(%d): Failed to set and lock mux index %lu, channel_bmp = 0x%02lX.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index, i2c_mux_channel);
        return FALSE;
    }
    if (I2CDRV_TwsiDataRead(dev_slv_id, 0, 1, offset, (offset < 256) ? 0 : 1, data_len, data) == FALSE)
    {
        printf("%s(%d): Failed to read data from register 0x%04lX.\r\n",  __FUNCTION__, __LINE__, offset);
        ret = FALSE;
    }
    if (I2CDRV_UnLockMux(i2c_bus_index) == FALSE)
    {
        printf("%s(%d): Failed to unlock mux index %lu.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index);
        return FALSE;
    }

    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_SetPoERegister
 * -------------------------------------------------------------------------
 * FUNCTION: This function performs preprocessing to write data into register of PSE controller
 * INPUT   : i2c_bus_index: I2C bus index
 *           i2c_mux_channel: I2C mux channel
 *           dev_slv_id: Device slave address
 *           offset: The register offset of PSE controller
 *           data: The value to be written to specified register of PSE controller
 *           data_len: length of data.
 * OUTPUT  : None
 * RETURN  : TRUE: If success
 *           FALSE: If failed
 * NOTE    : Need to refer HW design SPEC firstly!
 * -------------------------------------------------------------------------*/
static BOOL_T POEDRV_SetPoERegister(UI32_T i2c_bus_index, UI32_T i2c_mux_channel, UI8_T dev_slv_id, UI32_T offset, UI8_T *data, UI8_T data_len)
{
    BOOL_T ret = TRUE;


    if (data == NULL)
        return FALSE;

    if (I2CDRV_SetAndLockMux(i2c_bus_index, i2c_mux_channel) == FALSE)
    {
        printf("%s(%d): Failed to set and lock mux index %lu, channel_bmp = 0x%02lX.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index, i2c_mux_channel);
        return FALSE;
    }
    if (I2CDRV_TwsiDataWrite(dev_slv_id, 0, 1, offset, (offset < 256) ? 0 : 1, data, data_len) == FALSE)
    {
        printf("%s(%d): Failed to write data into register 0x%04lX.\r\n",  __FUNCTION__, __LINE__, offset);
        ret = FALSE;
    }
    if (I2CDRV_UnLockMux(i2c_bus_index) == FALSE)
    {
        printf("%s(%d): Failed to unlock mux index %lu.\r\n",  __FUNCTION__, __LINE__, i2c_bus_index);
        return FALSE;
    }

    return ret;
}
#endif
