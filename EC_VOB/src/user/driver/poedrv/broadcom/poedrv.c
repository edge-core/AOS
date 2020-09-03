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
 *    06/29/2007 - Daniel Chen, Porting Broadcom series PoE ASIC
 *    12/03/2008 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
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
//#include "leaf_es3626a.h"
#include "leaf_3621.h"
#include "backdoor_mgr.h"
#include "poedrv_type.h"
#include "poedrv.h"
#include "poedrv_backdoor.h"
#include "poedrv_om.h"
#include "fs.h"
#include "fs_type.h"
#include "l_stdlib.h"
//#include "stktplg_board.h"
#include "poedrv_control.h"
#include "poe_image.h"
#include "uc_mgr.h"
#include "phyaddr_access.h"
#include "time.h"

/* NAMING CONSTANT DECLARATIONS
 */

#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"

#define POEDRV_TIME_OUT                  300            /* time to wait for ISC reply */
#define POEDRV_RETRY_TIMES               2
#define POEDRV_ALL_UNIT                  255            /* all unit number  */
#define MASTER_UNIT                      1
#endif /*SYS_CPNT_STACKING*/

//#define POEDRV_ENTER_CRITICAL_SECTION        SYSFUN_ENTER_CRITICAL_SECTION(poedrv_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
//#define POEDRV_LEAVE_CRITICAL_SECTION        SYSFUN_LEAVE_CRITICAL_SECTION(poedrv_sem_id);

#define POEDRV_ENTER_CRITICAL_SECTION 
#define POEDRV_LEAVE_CRITICAL_SECTION

#define POEDRV_DEFAULT_GUARDBAND 0 // SYS_ADPT_PSE_GUARD_BAND

//#define Workaround_Required    /* Workaround for PoE controller to set configuration back to default */
#define ALARM_THRESHOLD 95 /* percentage */
#define POEDRV_UNIT_TO_UNITBMP(unit_id)      ((0x01) << (unit_id-1))
#if 0
#define DBG_PRINT(format,...) printf("%s(%d): "format"\r\n",__FUNCTION__,__LINE__,##__VA_ARGS__); fflush(stdout);
#else
#define DBG_PRINT(format,...)
#endif

//static POEDRV_PINGPONG_DETECTION_T poedrv_pingpong_info[POEDRV_NO_OF_POE_PORTS];

/* DATA TYPE DECLARATIONS
 */
#if (SYS_CPNT_STACKING == TRUE)

/* service ID list
 */
typedef enum
{
    POEDRV_Set_Port_Admin_Status= 0,
    /*POEDRV_Set_Port_Power_Detection_Control,*/
    POEDRV_Set_Port_Power_Priority,
    POEDRV_Set_Port_Power_Pairs,
    POEDRV_Set_Port_Power_Maximum_Allocation,
    POEDRV_Set_Main_power_Maximum_Allocation,
    POEDRV_Software_Download,
    POEDRV_Get_Poe_Software_Version,

    POEDRV_Get_Port_Invalid_Sign_Counter,
    POEDRV_Get_Port_Power_Denied_Counter,
    POEDRV_Get_Port_Overload_Counter,
    POEDRV_Get_Port_Short_Counter,
    POEDRV_Get_Port_MPS_Absent_Counter,
    POEDRV_Get_Poe_Port_Temperature,
    POEDRV_Get_Poe_Port_Voltage,
    POEDRV_Get_Poe_Port_Current,

    POEDRV_User_Port_Existing,
    POEDRV_Reset_Port,
    POEDRV_Set_Port_Dot3at_High_Power_Mode,
    POEDRV_Set_Port_Force_High_Power_Mode,

    POEDRV_Port_Detection,
    POEDRV_Port_Status,
    POEDRV_Is_Main_Power_Reach_Maximun,
    POEDRV_Port_Overload,
    POEDRV_Port_Failure,
    POEDRV_Port_Power_Consumption,
    POEDRV_Port_Power_Classification,
    POEDRV_PSE_Power_consumption,
    POEDRV_PSE_Oper_Status,
    POEDRV_Provision_Complete,
    POEDRV_Set_Capacitor_Detection_Control,
    POEDRV_PSE_Power_Denied_Occur_Frequently,
    POEDRV_NBR_OF_SERVICE_ID /*Don't hard code, count on compilier*/
} POEDRV_ServicesID_T;


/* define PoE ISC Payload type
 */
typedef struct
{
    UI32_T   serviceID;      /* Service ID  */
    UI32_T   unit;           /* stack id (unit number) */
    UI32_T   port;           /* port number */
    union
    {
        UI32_T admin_status;
        UI32_T mode;
        UI32_T priority;
        UI32_T power_pairs;
        UI32_T port_Power_milliwatts;
        UI32_T main_power_watts;
        UI32_T fource_high_power_mode;
        UI32_T dot3at_high_power_mode;
        BOOL_T is_power_enabled;
        BOOL_T is_ClassMode;
        UI8_T  legacy_detection_enable;
        UI8_T  filename[SYS_ADPT_FILE_SYSTEM_NAME_LEN];
        struct
        {
            UI8_T version1;
            UI8_T version2;
            UI8_T build;
        } __attribute__((packed, aligned(1)))Poe_Software_Version;
        struct
        {   /* Event Notification Information */
            UI8_T port_detection_status;                  /* slave change Port status */
            UI8_T actual_status;                                   /*slave actual port status */
            BOOL_T is_port_overload;                       /* slave change PortOverloadStatus */
            UI32_T power_consumption;                       /* slave change PortPowerClassification */
            UI32_T power_classification;                   /* slave change PortPowerClassification */
            UI32_T pse_consumption;	                   /* slave change MainPseConsumption */
            UI32_T pse_oper_status;                        /* slave change PseOperStatus */
        } __attribute__((packed, aligned(1)))notify;
    } __attribute__((packed, aligned(1)))info;
}__attribute__((packed, aligned(1))) POEDRV_Rx_IscBuf_T;

typedef BOOL_T (*POEDRV_ServiceFunc_t) (ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);

#endif

/* Database for mainpower status on PoE system
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
// Eugene do it in POEDRV_OM_Reset, static void   POEDRV_InitDataBase(void);
static void   POEDRV_TaskMain(void);
static void   POEDRV_MonitorPoeMainpowerStatus(void);
static void   POEDRV_MonitorPoeOperationStatus(void);
static void   POEDRV_MonitorPoePortStatus(void);
static void   POEDRV_MonitorPoePortStatistic(void);
// static BOOL_T POEDRV_LocalGetPortPowerConsumption(UI32_T unit, UI32_T port, UI32_T *milliwatts);
// static BOOL_T POEDRV_LocalGetPortPowerClassification(UI32_T unit, UI32_T port, UI32_T *power_class);
static void   POEDRV_Notify_PortDetectionStatusChange(UI32_T unit, UI32_T port, UI8_T admin_status);
static void   POEDRV_Notify_PortStatusChange(UI32_T unit, UI32_T port, UI8_T actual_port_status);
static void   POEDRV_Notify_IsMainPowerReachMaximun(UI32_T unit, UI32_T status);
static void   POEDRV_Notify_PortOverloadStatusChange(UI32_T unit, UI32_T port, BOOL_T is_overload);
static void   POEDRV_Notify_PortPowerClassificationChange(UI32_T unit, UI32_T port, UI32_T power_class);
static void   POEDRV_Notify_PortPowerConsumptionChange(UI32_T unit, UI32_T port, UI32_T power_consumption);
static void   POEDRV_Notify_MainPseConsumptionChange(UI32_T unit, UI32_T power_consumption);
static void   POEDRV_Notify_PseOperStatusChange(UI32_T unit, UI32_T oper_status);
static void   POEDRV_Notify_PowerDeniedOccurFrequently(UI32_T unit, UI32_T port);
static void   POEDRV_UpdateStackingInfo(void);


#if 0
static void   POEDRV_Notify_LegacyDetectionStatusChange(UI32_T unit, UI8_T oper_status);
#endif
#if 0 // daniel
static BOOL_T POEDRV_SendSoftwareDownload(void);
#endif
//#ifdef Workaround_Required
static BOOL_T POEDRV_SetDefaultValue(void);
//#endif

/* Local Subprogram Declaration for PoE stacking management
 */
#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SlaveSetPortAdminStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;
#if 0
static void SlaveSetPortPowerDetectionControl(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref) ;
#endif
static BOOL_T SlaveSetPortPowerPriority(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;
static BOOL_T SlaveSetPortPowerPairs(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;
static BOOL_T SlaveSetPortPowerMaximumAllocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;
static BOOL_T SlaveSetMainpowerMaximum_Allocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;

static BOOL_T SlaveSoftwareDownload(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;
static BOOL_T SlaveGetPoeSoftwareVersion(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf) ;

static BOOL_T SlaveSetProvisionComplete(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveSetCapacitorDetectionControl(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);

static BOOL_T SlaveGetPortInvalidSignCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPortPowerDeniedCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPortOverloadCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPortShortCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPortMPSAbsentCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPoePortTemperature(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPoePortVoltage(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveGetPoePortCurrent(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveUserPortExisting(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveResetPort(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveSetPortDot3atHighPowerMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T SlaveSetPortForceHighPowerMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);

static BOOL_T CallbackMasterPortDetectionStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterPortStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);/*poe led*/
static BOOL_T CallbackMasterPortOverloadStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterPortPowerConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterPortPowerClassification(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterMainPseConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterPseOperStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
static BOOL_T CallbackMasterPowerDeniedOccurFrequently(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf);
#endif

static BOOL_T POEDRV_LocalSetPortAdminStatus(UI32_T port, UI32_T admin_status);
static BOOL_T POEDRV_LocalSetPortPowerPriority(UI32_T port, UI32_T priority);
static BOOL_T POEDRV_LocalSetPortPowerPairs(UI32_T port, UI32_T power_pairs);
static BOOL_T POEDRV_LocalSetPortPowerMaximumAllocation(UI32_T port, UI32_T milliwatts);
static BOOL_T POEDRV_LocalSetMainpowerMaximumAllocation(UI32_T watts);
static BOOL_T POEDRV_LocalGetPoeSoftwareVersion(UI8_T *version1, UI8_T *version2, UI8_T *build);
static BOOL_T POEDRV_LocalSoftwareDownload(UI8_T *filename);

//static BOOL_T POEDRV_LocalSetClassMode(BOOL_T ClassMode);

static BOOL_T POEDRV_LocalSetCapacitorDetectionControl(UI8_T value);

static BOOL_T POEDRV_LocalGetPortInvalidSignCounter(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPortPowerDeniedCounter(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPortOverloadCounter(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPortShortCounter(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPortMPSAbsentCounter(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPoePortTemperature(UI32_T port, I32_T *counters);
static BOOL_T POEDRV_LocalGetPoePortVoltage(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPoePortCurrent(UI32_T port, UI32_T *counters);
static BOOL_T POEDRV_LocalGetPoePortVoltage(UI32_T port, UI32_T *counters);
static void POEDRV_localProvisionComplete(void);
static BOOL_T POEDRV_LocalResetPort(UI32_T port);
static BOOL_T POEDRV_LocalSetPortForceHighPowerMode(UI32_T port, UI32_T mode);
static BOOL_T POEDRV_LocalSetPortDot3atHighPowerMode(UI32_T port, UI32_T mode);




#if 0
static BOOL_T POEDRV_LocalSetPortPowerDetectionControl(UI32_T port,UI32_T mode);
#endif /*Disable*/



/* STATIC VARIABLE DECLARATIONS
 */
//static POEDRV_Mainpower_Info_T poedrv_mainpower_info;
//static POEDRV_Port_Info_T poedrv_port_info[POEDRV_NO_OF_POE_PORTS];
//static UI32_T poedrv_port_counter[POEDRV_NO_OF_POE_PORTS][POEDRV_MAX_COUNTER_TYPE];
//static UI8_T                          poedrv_echo_number;     /* Echo number(0x0~0xFE)      */
//static UI32_T                         poedrv_task_id;         /* PoE driver task ID         */
//static UI32_T                         poedrv_sem_id;          /* PoE driver semaphore ID    */
//static UI32_T                         poedrv_my_unit_id;      /* Local unit ID              */
//static UI32_T                         poedrv_num_of_units;    /* numer of POE  switch       */
//static BOOL_T                         is_enter_program_mode;  /* Enter program mode         */
//static BOOL_T                         is_stop_polling = TRUE;        /* Stop polling controller    */
//#ifdef Workaround_Required
//static BOOL_T                         is_init_flag   = TRUE;
//#endif

//static UI32_T main_pse_power_max_allocation; /* Max power allocation of POE system */
//static UI32_T poedrv_min_port_number;
//static UI32_T poedrv_max_port_number;
//static UI8_T  poedrv_poe_image_version = 0xFF;

//Eugene temp, static UI8_T  poedrv_support_74hc;
//Eugene temp, static BOOL_T  poedrv_support_poe;

static POEDRV_CONTROL_T *dynamic_hooked_poe = NULL;

/* Definition for state transition on a port
 */
// static UI8_T                          poedrv_port_state[POEDRV_NO_OF_POE_PORTS+1];
/* Definitions of callback function list
 */
static SYS_TYPE_CallBack_T *PortDetectionStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PortPowerConsumptionChange_callbacklist;
static SYS_TYPE_CallBack_T *PortPowerClassificationChange_callbacklist;
static SYS_TYPE_CallBack_T *MainPseConsumptionChange_callbacklist;
static SYS_TYPE_CallBack_T *PseOperStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PortOverloadStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *PowerDeniedOccurFrequently_callbacklist; /* for ping-port issue */

/*for POE Led*/
static SYS_TYPE_CallBack_T *PortStatusChange_callbacklist;
static SYS_TYPE_CallBack_T *MainPseConsumptionReachMaximun_callbacklist;

#if (SYS_CPNT_STACKING == TRUE)

/* Callback function table for PoE stacking management
 */
static POEDRV_ServiceFunc_t POEDRV_func_tab[] =
{
    SlaveSetPortAdminStatus,                         /*POEDRV_Set_Port_Admin_Status                */
    /*SlaveSetPortPowerDetectionControl,*/              /*POEDRV_Set_Port_Power_Detection_Control     */
    SlaveSetPortPowerPriority,                       /*POEDRV_Set_Port_Power_Priority              */
    SlaveSetPortPowerPairs,                          /*POEDRV_Set_Port_Power_Pairs                 */
    SlaveSetPortPowerMaximumAllocation,              /*POEDRV_Set_Port_Power_Maximum_Allocation    */
    SlaveSetMainpowerMaximum_Allocation,             /*POEDRV_Set_Main_power_Maximum_Allocation    */
    SlaveSoftwareDownload,                           /*POEDRV_Software_Download                    */
    SlaveGetPoeSoftwareVersion,                      /*POEDRV_Get_Poe_Software_Version             */

    SlaveGetPortInvalidSignCounter,                     /*POEDRV_Get_Port_Invalid_Sign_Counter*/
    SlaveGetPortPowerDeniedCounter,                     /*POEDRV_Get_Port_Power_Denied_Counter*/
    SlaveGetPortOverloadCounter,                        /*POEDRV_Get_Port_Overload_Counter*/
    SlaveGetPortShortCounter,                           /*POEDRV_Get_Port_Short_Counter*/
    SlaveGetPortMPSAbsentCounter,                       /*POEDRV_Get_Port_MPS_Absent_Counter*/
    SlaveGetPoePortTemperature,                         /*POEDRV_Get_Poe_Port_Temperature*/
    SlaveGetPoePortVoltage,                             /*POEDRV_Get_Poe_Port_Voltage*/
    SlaveGetPoePortCurrent,                             /*POEDRV_Get_Poe_Port_Current*/

    SlaveUserPortExisting,                              /*POEDRV_Logical_2_Phy_Device_Port_ID*/
    SlaveResetPort,                                     /*POEDRV_Reset_Port*/
    SlaveSetPortDot3atHighPowerMode,                    /*POEDRV_Set_Port_Dot3at_High_Power_Mode*/
    SlaveSetPortForceHighPowerMode,                     /*POEDRV_Set_Port_Force_High_Power_Mode*/

    CallbackMasterPortDetectionStatus,               /*POEDRV_Port_Detection                       */
    CallbackMasterPortStatus,					      /*poe led		*/
    CallbackMasterPortOverloadStatus,                /*POEDRV_Port_Overload                        */
    CallbackMasterPortPowerConsumption,              /*POEDRV_Port_Power_consumption              */
    CallbackMasterPortPowerClassification,           /*POEDRV_Port_Power_Classification            */
    CallbackMasterMainPseConsumption,                /*POEDRV_Pse_Power_consumption               */
    CallbackMasterPseOperStatus,                     /*POEDRV_Pse_Oper_Status                      */
    SlaveSetProvisionComplete,                          /*POEDRV_ProvisionComplete                    */
    SlaveSetCapacitorDetectionControl,
    CallbackMasterPowerDeniedOccurFrequently,        /*POEDRV_PSE_Power_Denied_Occur_Frequently*/
    NULL
};

#endif


/* MACRO FUNCTIONS DECLARACTION
 */

/* The macro of declared variables, used for stacking mode
 */
//SYSFUN_DECLARE_CSC


#if (SYS_CPNT_STACKING == TRUE)
//BOOL_T POEDRV_Service_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref);
#endif

/* ES3526B2-PoE-FLF-32-00165, Daniel Chen, 2008/3/12
 * Change natual port priority from low number port to high number
 */
/* Daniel Chen, 2007/12/19
 * Used to Port mapping. And this mapping will influence port priority
 */
#if 1 /* Eugene temp */
static UI32_T poedrv_port_mapping[POEDRV_NO_OF_POE_PORTS] =
//{8,7,10,9,12,11,14,13,16,15,18,17,20,19,22,21,24,23,26,25,28,27,30,29};
//{3,2,1,0,7,6,5,4,11,10,9,8,15,14,13,12,19,18,17,16,23,22,21,20};
{2,3,0,1,6,7,4,5,10,11,8,9,14,15,12,13,18,19,16,17,22,23,20,21};
#endif


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : POEDRV_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("poedrv",
    SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY, POEDRV_BACKDOOR_Main);
}


/* FUNCTION NAME : POEDRV_Initiate_System_Resources
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

    /* Initialize pointers of all callback funtions
     */
    PortDetectionStatusChange_callbacklist     = 0;
    PortPowerConsumptionChange_callbacklist    = 0;
    PortPowerClassificationChange_callbacklist = 0;
    MainPseConsumptionChange_callbacklist      = 0;
    PseOperStatusChange_callbacklist           = 0;
    PortOverloadStatusChange_callbacklist      = 0;
    PowerDeniedOccurFrequently_callbacklist     = 0;

    /*For POE Led*/
    PortStatusChange_callbacklist = 0;
    MainPseConsumptionReachMaximun_callbacklist = 0;

    /* Initialize echo sequence number, starting from 0
     */
//    poedrv_echo_number = 0;

    /* Initialize database on PoE status
     */
    POEDRV_OM_Init();
//    memset(&poedrv_mainpower_info, 0, sizeof(POEDRV_Mainpower_Info_T));
//    memset(poedrv_port_info, 0, sizeof(POEDRV_Port_Info_T)*(POEDRV_NO_OF_POE_PORTS));
//    memset(poedrv_port_counter, 0, sizeof(poedrv_port_counter));
//    memset(poedrv_pingpong_info, 0, sizeof(poedrv_pingpong_info));

#if 0 /* Eugene marked for not using universal image */
    if (!STKTPLG_BOARD_GetHWInfoSupport74HC(&poedrv_support_74hc))
    {
        printf("\r\nSTKTPLG_BOARD_GetHWInfoSupport74HC() Error!!");
    }

    if (!STKTPLG_BOARD_GetHWInfoSupportPoE(&poedrv_support_poe))
    {
        printf("\r\nSTKTPLG_BOARD_GetHWInfoSupportPoE() Error!!");
    }
#endif

#if 0 /* Use semaphore in BCM59101 */
    /* Create semaphore
     */
    if (SYSFUN_CreateSem (1/* SEM_FULL */, SYSFUN_SEM_FIFO, &poedrv_sem_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\n\rCreate poedrv_sem_id failed.");
        return FALSE;
    }
#endif


#if (SYS_CPNT_STACKING == TRUE)

//    ISC_Register_Service_CallBack(ISC_POEDRV_SID, POEDRV_Service_Callback);
#endif

#ifndef INCLUDE_DIAG
    /* Register a backdoor debug function
     */
//    POEDRV_Create_InterCSC_Relation();

//Eugene mark,    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("poedrv", xxxx, POEDRV_BACKDOOR_Main);

#endif
    /* After running provision, we should do hardware reset again
     * So Move POEDRV_HardwareReset() to EnterTransitionMode
     */
    /* Release hardware reset to PoE controller
     */
#if 0
    POEDRV_HardwareReset();
#endif

#if 0 /* Eugene do it in POEDRV_OM_AttachSystemResources */
    if (POEDRV_Control_Hook(&dynamic_hooked_poe) == TRUE)
    {
        /* printf("\n%s\n",dynamic_hooked_poe->drv_name);*/
    }

    /* Create semaphore
     */
    POEDRV_EXEC(dynamic_hooked_poe->poedrv_init, ret);
#endif

    return (TRUE);
} /* End of POEDRV_Initiate_System_Resources() */

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
    BOOL_T ret;

    if (POEDRV_Control_Hook(&dynamic_hooked_poe) == TRUE)
    {
        /* printf("\n%s\n",dynamic_hooked_poe->drv_name);*/
    }

    /* Create semaphore
     */
    POEDRV_EXEC(dynamic_hooked_poe->poedrv_init, ret);

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_open_uart, ret);

    POEDRV_OM_AttachSystemResources();

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
    UC_MGR_Sys_Info_T sys_info;
    BOOL_T ret;
    UI8_T version=0, v2, build;
#if 0 /* Eugene marked for not using universal image */
    UI32_T unit=1;
#endif
DBG_PRINT();

    POEDRV_OM_SetProvisionComplete(FALSE);

#if 0 /* Eugene marked, do init database in POEDRV_InitiateSystemResources */
    /* Initialize database on PoE status
     */
    memset(&poedrv_mainpower_info, 0, sizeof(POEDRV_Mainpower_Info_T));
    memset(poedrv_port_info, 0, sizeof(POEDRV_Port_Info_T)*(POEDRV_NO_OF_POE_PORTS));
#endif

POEDRV_UpdateStackingInfo();

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
        printf("\r\nUnable to get sysinfo from UC Manager.");
        return;
    }
    /* Move from POEDRV_Initiate_System_Resources()
     */

    printf("\r\nPoE Image checking...");
    POEDRV_LocalGetPoeSoftwareVersion(&version, &v2, &build);

    if(version == poe_image_version)
    {
        /* do nothing if code version is the same with EEPROM
         */
        printf("\r\nPoE Image version: Ver %d.%d",version>>4, version&0xF);
        /* Disable ports powering on PoE controller
         */
        POEDRV_SetModuleStatus(FALSE);
    }
    else
    {
        /* if code image version is different from EEPROM image version, update it
         */
        printf("\r\nPoE Image updating: Ver %d.%d to Ver %d.%d", version>>4, version&0xF, poe_image_version>>4, poe_image_version&0xF);

        POEDRV_EXEC(dynamic_hooked_poe->poedrv_upgrade_image_command, 
                      ret, POEDRV_TYPE_SUBCOM_CLEAR_IMAGE);

        POEDRV_HardwareReset();

        printf("\r\nPoE Image Loading...");
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_upgrade_image, ret,
                      poe_image, POE_IMAGE_SIZE);

        SYSFUN_Sleep(1200);  /* wait for 12 seconds to boot up PoE subsystem */

#if 0
        {   /* waiting for Bootup */
            UI8_T j,wait=30

            for(j=0;j<=wait;j++)
            {
                version=0;
                POEDRV_LocalGetPoeSoftwareVersion(&version, &v2, &build);

				if (version ==poe_image_version)
				{
                    printf("SUCCESS!!");
                    break;
				}
                else if (j==wait)
                {
                    DBG_PRINT("waiting Bootup for %dsec but still failed",wait);
                    printf("FAIL!!");
                }

                SYSFUN_Sleep(100); /* 1 sec */
            }
            DBG_PRINT("waiting Bootup for %dsec!!!",j);
        }
#else
        version=0;
        POEDRV_LocalGetPoeSoftwareVersion(&version, &v2, &build);

		if (version ==poe_image_version)
            printf("SUCCESS!!");
		else
            printf("FAILURE!!");

#endif

        /* need clear configuration in EEPROM before save image
         */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_upgrade_image_command, 
                      ret, POEDRV_TYPE_SUBCOM_CLEAR_CONFIGURATION);

        /* save image to EEPROM 
         */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_upgrade_image_command, 
                      ret, POEDRV_TYPE_SUBCOM_SAVE_IMAGE);

        /* Disable ports powering on PoE controller
         */
        POEDRV_SetModuleStatus(FALSE);
    }

    POEDRV_OM_EnterTransitionMode();
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
    UI8_T version=0, v2, build;
    UI8_T set_default_count;
    BOOL_T ret;

    /* Initialize database with default value
     */
//Eugene do it in POEDRV_OM_Init,    POEDRV_InitDataBase();
DBG_PRINT();

    ret = POEDRV_LocalGetPoeSoftwareVersion(&version, &v2, &build);
    if ( ret == FALSE)
    {
        SYSFUN_Debug_Printf("\n\r  Get Version Failed !!!");
    }
    else
    {
        SYSFUN_Debug_Printf("\n\rcurrent PoE Image version: Ver %d.%d", version>>4, version&0xF);

        /* Set Default-Value to micro-p
         */
        for (set_default_count=1;set_default_count<=5;set_default_count++)
        {
            if ( POEDRV_SetDefaultValue() == TRUE )
            {
                /* Enable ports powering on PoE controller
                 */
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
    }

    if (ret == TRUE)
        POEDRV_OM_SetStopMonitorFlag(FALSE); /* Enable Polling function */
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
    UI8_T version, v2, build;
    /* Initialize database with default value
     */
//Eugene do it in POEDRV_OM_Init,    POEDRV_InitDataBase();

    POEDRV_OM_EnterSlaveMode();
    /*Get Version and Build to use in set main power allocation*/
    if (POEDRV_LocalGetPoeSoftwareVersion(&version, &v2, &build) == FALSE)
    {
        SYSFUN_Debug_Printf("\n\r  Get Version Failed !!!");
    }
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
	
    if ( SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            SYS_BLD_POEDRV_TASK,
                            SYS_BLD_TASK_COMMON_STACK_SIZE,
                            SYSFUN_TASK_NO_FP,
                            POEDRV_TaskMain,
                            NULL,
                            &thread_id) != SYSFUN_OK )
    {
        printf("\n%s: Spawn POEDRV thread fail.\n", __FUNCTION__);

        return FALSE;
    }

	POEDRV_OM_SetThreadId(thread_id);

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

void POEDRV_Register_IsMainPowerReachMaximun_CallBack(void (*fun)(UI32_T unit, UI32_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(MainPseConsumptionReachMaximun_callbacklist);
}


/* FUNCTION NAME : POEDRV_Register_PortPowerConsumptionChange_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If status of a port has been changed, the registered function
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T detection_status)
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


/* FUNCTION NAME : POEDRV_Register_PowerDeniedOccurFrequently_CallBack
 * PURPOSE: This function is used to register the callback function.
 *          If power denied status of a port changed many times in short period, 
 *          the registered function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port)
 *          unit -- unit ID
 *          port -- port ID
 */
void POEDRV_Register_PowerDeniedOccurFrequently_CallBack(void (*fun)(UI32_T unit, UI32_T port))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(PowerDeniedOccurFrequently_callbacklist);

} /* End of POEDRV_Register_PortOverloadStatusChange_CallBack() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port of poe device is existing
 * INPUT   : unit -- unit ID, port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POEDRV_UserPortExisting(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    UI32_T               phy_port;
    BOOL_T               remote_value;

DBG_PRINT("u%lu,p%lu",unit, port);
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)
DBG_PRINT("u%lu,p%lu",unit, port);

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);
DBG_PRINT("my_unit=%d, unit=%lu, port=%lu",my_unit,unit,port);

        if ( my_unit != unit )
        {
DBG_PRINT("u%lu,p%lu",unit, port);
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
DBG_PRINT("u%lu,p%lu",unit, port);
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_User_Port_Existing)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_User_Port_Existing;
		        isc_buf_p->unit = unit;
		        isc_buf_p->port = port;
DBG_PRINT("u%lu,p%lu",unit, port);

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
DBG_PRINT("u%lu,p%lu",unit, port);
                    return TRUE;
                }
                else
                {
                    /* EH not implemented yet
                     */
DBG_PRINT("u%lu,p%lu",unit, port);
                    return FALSE;
                }
			}
			else
			{
DBG_PRINT("u%lu,p%lu",unit, port);
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /* SYS_CPNT_STACKING */
    	    /* if local unit or standalone
    	     */
	        if (!(remote_value = POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port)))
	        {
    	        /* EH not implemented yet
    	         */
DBG_PRINT("u%lu,p%lu",unit, port);
	            return FALSE;
	        }
DBG_PRINT("u%lu,p%lu",unit, port);
	        return remote_value;
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
DBG_PRINT("u%lu,p%lu",unit, port);
    return FALSE;
}


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
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
DBG_PRINT();

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Admin_Status)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Admin_Status;
                isc_buf_p->port = port;
                isc_buf_p->info.admin_status = admin_status;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /* SYS_CPNT_STACKING */
    	    /* if local unit or standalone
    	     */
	        if (!POEDRV_LocalSetPortAdminStatus(port,admin_status))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
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
    UI32_T phy_port;  /* Physical port ID      */
    BOOL_T ret = FALSE;
DBG_PRINT();

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_admin_status, ret,
        phy_port, admin_status);

    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;
} /* End of POEDRV_LocalSetPortAdminStatus() */

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
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Power_Priority)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Power_Priority;
                isc_buf_p->port = port;
                isc_buf_p->info.priority = priority;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                            POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */

	        if (!POEDRV_LocalSetPortPowerPriority(port,priority))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
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
    UI32_T phy_port;       /* Physical port ID      */
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) == FALSE)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_priority, ret,
        phy_port, priority);

    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;
} /* End of POEDRV_LocalSetPortPowerPriority() */

/* FUNCTION NAME: POEDRV_SetPortPowerPairs
 * PURPOSE: This function is used to set a specified port the power pairs
 *
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          power_pairs -- VAL_pethPsePortPowerPairs_signal (Alternative A)
 *                         VAL_pethPsePortPowerPairs_spare (Alternative B)
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SetPortPowerPairs(UI32_T unit, UI32_T port, UI32_T power_pairs)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Power_Pairs)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Power_Pairs;
                isc_buf_p->port = port;
                isc_buf_p->info.power_pairs = power_pairs;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                            SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                            POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */


	        if (!POEDRV_LocalSetPortPowerPairs(port,power_pairs))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return TRUE;
} /* End of POEDRV_SetPortPowerPairs() */

/* FUNCTION NAME: POEDRV_LocalSetPortPowerPairs
 * PURPOSE: This function is used to set a specified port the power pairs
 *
 * INPUT:   port -- port ID
 *          power_pairs -- VAL_pethPsePortPowerPairs_signal (Alternative A)
 *                         VAL_pethPsePortPowerPairs_spare (Alternative B)
 *
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalSetPortPowerPairs(UI32_T port, UI32_T power_pairs)
{
    UI32_T phy_port;       /* Physical port ID      */
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) == FALSE)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_power_pairs, ret,
        phy_port, power_pairs);

    POEDRV_LEAVE_CRITICAL_SECTION;


    return ret;

} /* End of POEDRV_LocalSetPortPowerPairs() */



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
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Power_Maximum_Allocation)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Power_Maximum_Allocation;
                isc_buf_p->port = port;
                isc_buf_p->info.port_Power_milliwatts = milliwatts;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */


	        if (!POEDRV_LocalSetPortPowerMaximumAllocation(port,milliwatts))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
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
 * NOTES:
 */
static BOOL_T POEDRV_LocalSetPortPowerMaximumAllocation(UI32_T port, UI32_T milliwatts)
{
    UI32_T phy_port;       /* Physical port ID      */
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) == FALSE)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_power_limit, ret,
        phy_port, milliwatts);

    POEDRV_LEAVE_CRITICAL_SECTION;


    return ret;

} /* End of POEDRV_LocalSetPortPowerMaximumAllocation() */

/* FUNCTION NAME: POEDRV_SetMainpowerMaximumAllocation
 * PURPOSE: This function is used to set the power, available for Power
 *          Management on PoE.
 * INPUT:   unit -- unit ID
 *          watts -- power available on PoE system
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SetMainpowerMaximumAllocation(UI32_T unit, UI32_T watts)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Main_power_Maximum_Allocation)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Main_power_Maximum_Allocation;
                isc_buf_p->info.main_power_watts = watts;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
			    printf("\r\nUnit %lu don't support PoE!!", unit);
DBG_PRINT();
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */


	        if (!POEDRV_LocalSetMainpowerMaximumAllocation(watts))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return TRUE;

} /* End of POEDRV_SetMainpowerMaximumAllocation() */
/* FUNCTION NAME: POEDRV_LocalSetMainpowerMaximumAllocation
 * PURPOSE: This function is used to set the power, available for Power
 *          Management on PoE.
 * INPUT:   watts -- power available on PoE system
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalSetMainpowerMaximumAllocation(UI32_T watts)
{
    BOOL_T ret = FALSE;
	UI32_T value;

    POEDRV_OM_GetMainPowerMaxAllocation(&value);
	if (watts > value) /* can't over the limitation of system */
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_power_source_control, ret,
        watts, POEDRV_DEFAULT_GUARDBAND);

    if (ret == TRUE)
    {
        POEDRV_OM_SetMainPowerInfoMainPower(watts);
    }
    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;

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
    UI32_T phy_port;

    /* Check operating mode
     */

    /* Check illegal port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) == FALSE)
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
    UI32_T phy_port;

    /* Check operating mode
     */
    if ( POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE )
    {
    	 return FALSE;
    }

    /* Check illegal port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port,&phy_port) == FALSE)
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
 * OUTPUT:  watts -- power consumption of PoE in watts
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


/* FUNCTION NAME: POEDRV_GetPortInvalidSignCounter
 * PURPOSE: This function is used to get the PoE invalid signature counter for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  counter
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortInvalidSignCounter(UI32_T unit, UI32_T port, UI32_T *counters)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Invalid_Sign_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Port_Invalid_Sign_Counter;
		    	isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        		{
                    *counters = remote_value;
	    			return TRUE;
        	    }
    			else
            	{
                    /* EH not implemented yet
    		         */
                    return FALSE; /* can't config remote */
    			}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPortInvalidSignCounter(port, counters))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPortInvalidSignCounter(UI32_T port, UI32_T *counters)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
	POEDRV_OM_GetPortCounter(port, POEDRV_INVALID_SIGNATURE_COUNTER, counters);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

/* FUNCTION NAME: POEDRV_GetPowerDeniedCounter
 * PURPOSE: This function is used to get the PoE power denied counter for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  counter
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortPowerDeniedCounter(UI32_T unit, UI32_T port, UI32_T *counters)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Power_Denied_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Port_Power_Denied_Counter;
		        isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
                    *counters = remote_value;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPortPowerDeniedCounter(port, counters))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPortPowerDeniedCounter(UI32_T port, UI32_T *counters)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
	POEDRV_OM_GetPortCounter(port, POEDRV_POWER_DENIED_COUNTER, counters);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

/* FUNCTION NAME: POEDRV_GetOverloadCounter
 * PURPOSE: This function is used to get the PoE overload counter for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  counter
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortOverloadCounter(UI32_T unit, UI32_T port, UI32_T *counters)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Overload_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Port_Overload_Counter;
                isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
                    *counters = remote_value;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPortOverloadCounter(port, counters))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPortOverloadCounter(UI32_T port, UI32_T *counters)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
	POEDRV_OM_GetPortCounter(port, POEDRV_OVERLOAD_COUNTER, counters);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

/* FUNCTION NAME: POEDRV_GetPortShortCounter
 * PURPOSE: This function is used to get the PoE short counter for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  counter
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortShortCounter(UI32_T unit, UI32_T port, UI32_T *counters)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Short_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Port_Short_Counter;
                isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
                    *counters = remote_value;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPortShortCounter(port, counters))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPortShortCounter(UI32_T port, UI32_T *counters)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
	POEDRV_OM_GetPortCounter(port, POEDRV_SHORT_COUNTER, counters);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

/* FUNCTION NAME: POEDRV_GetPortMPSAbsentCounter
 * PURPOSE: This function is used to get the PoE MPS absent counter for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  counter
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPortMPSAbsentCounter(UI32_T unit, UI32_T port, UI32_T *counters)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_MPS_Absent_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Port_MPS_Absent_Counter;
		        isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
                    *counters = remote_value;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPortMPSAbsentCounter(port, counters))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPortMPSAbsentCounter(UI32_T port, UI32_T *counters)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
	POEDRV_OM_GetPortCounter(port, POEDRV_MPSABSENT_COUNTER, counters);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;
}

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
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI32_T               remote_value;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Software_Version)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Poe_Software_Version;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
    		    {
                    *version1 = remote_value/256/256;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
    	    POEDRV_OM_GetImageVersion(version1);
    	    if (*version1 == 0xFF)
    	    {
    	        if (POEDRV_LocalGetPoeSoftwareVersion(version1, version2, build) == FALSE)
    	        {
                    /* EH not implemented yet
                     */
    	            return FALSE;
    	        }
    	        POEDRV_OM_SetImageVersion(*version1);
    	    }
	        return TRUE;
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

} /* End of POEDRV_GetPoeSoftwareVersion() */

/* FUNCTION NAME : POEDRV_GetPoeSoftwareVersion
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : unit -- unit ID
 * OUTPUT  : version -- version number
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalGetPoeSoftwareVersion(UI8_T *version1, UI8_T *version2, UI8_T *build)
{
    BOOL_T ret=FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_get_soft_ver, ret,
        version1);

    POEDRV_LEAVE_CRITICAL_SECTION;
    version2 = build = 0;

    return ret;

} /* End of POEDRV_LocalGetPoeSoftwareVersion() */

/* FUNCTION NAME : POEDRV_GetPoePortTemperature
 * PURPOSE: This function is used to get port temperature
 * INPUT   : unit -- unit ID
 *           port -- port number
 * OUTPUT  : temp -- temperature
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPoePortTemperature(UI32_T unit, UI32_T port, I32_T *temp)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    I32_T               remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_MPS_Absent_Counter)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Poe_Port_Temperature;
		        isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
                {
                    *temp = remote_value;
                    return TRUE;
                }
				else
                {
                    /* EH not implemented yet
        		     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPoePortTemperature(port, temp))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPoePortTemperature(UI32_T port, I32_T *temp)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_OM_GetPortInfoTemperature(port, temp);

    return TRUE;
} /* End of POEDRV_GetPoePortTemperature() */

/* FUNCTION NAME : POEDRV_GetPoePortVoltage
 * PURPOSE: This function is used to get port voltage
 * INPUT   : unit -- unit ID
 *           port -- port number
 * OUTPUT  : volt -- voltage (V)
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPoePortVoltage(UI32_T unit, UI32_T port, UI32_T *volt)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Port_Voltage)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Poe_Port_Voltage;
			    isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        		{
                    *volt = remote_value;
		    		return TRUE;
        	    }
				else
            	{
                    /* EH not implemented yet
    	    	     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPoePortVoltage(port, volt))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPoePortVoltage(UI32_T port, UI32_T *volt)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_OM_GetPortInfoVoltage(port, volt);

    return TRUE;
} /* End of POEDRV_GetPoePortVoltage() */

/* FUNCTION NAME : POEDRV_GetPoePortCurrent
 * PURPOSE: This function is used to get port current
 * INPUT   : unit -- unit ID
 *           port -- port number
 * OUTPUT  : cur  -- current (mA)
 *
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_GetPoePortCurrent(UI32_T unit, UI32_T port, UI32_T *cur)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T* mref_handle_p;
	POEDRV_Rx_IscBuf_T* isc_buf_p;
    UI32_T              pdu_len;
    UI32_T              remote_value;
    UI8_T               my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Port_Current)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Get_Poe_Port_Current;
		    	isc_buf_p->port = port;

                if (ISC_RemoteCall((UI8_T)unit, ISC_POEDRV_SID, mref_handle_p,
                                    SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                    sizeof(remote_value), (UI8_T *)&remote_value,
                                    POEDRV_RETRY_TIMES, POEDRV_TIME_OUT))
        		{
                    *cur = remote_value;
		    		return TRUE;
        	    }
				else
            	{
                    /* EH not implemented yet
    	    	     */
                    return FALSE; /* can't config remote */
				}
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */
            if (POEDRV_LocalGetPoePortCurrent(port, cur))
	        {
    	        /* EH not implemented yet
    	         */
	            return TRUE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return FALSE;

}

static BOOL_T POEDRV_LocalGetPoePortCurrent(UI32_T port, UI32_T *cur)
{
    UI32_T phy_port;

    /* Check operating mode
     */

    if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_OM_GetPortInfoCurrent(port, cur);

    return TRUE;
}/* End of POEDRV_GetPoePortCurrent() */



/* FUNCTION NAME : POEDRV_SendRawPacket
 * PURPOSE: This function is used to send a raw packet from engineering backdoor
 * INPUT:   transmit: data pointer of packet to be transmitted (length depend on ASIC)
 * OUTPUT:  receive : data pointer of receiving packet
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
BOOL_T POEDRV_SendRawPacket(UI8_T *transmit, UI8_T *receive)
{
    BOOL_T ret = FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;
    POEDRV_EXEC(dynamic_hooked_poe->poedrv_send_raw_packet, ret,
        transmit, receive);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;
}


/* FUNCTION NAME : POEDRV_HardwareReset
 * PURPOSE: This function is used to issue a hardware reset to PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:  ES4526R-PoE-FLF doesn't support HW reset, we use Software reset
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
//#define GPIO_SWDELAY();   for (j=0;j<=10;j++){}
    UC_MGR_Sys_Info_T sys_info;
    SYS_TYPE_VAddr_T virt_addr;
    UI8_T reset_value = 0;
#if 0 //Eugene temp
    UI32_T       pattern, i;
#endif
    

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
        printf("\r\nUnable to get sysinfo from UC Manager.");
        return;
    }

POEDRV_OM_SetStopMonitorFlag(TRUE);
#if 0 
    while(TRUE)
    {
        if (SUPPORT_74HC_NONE == poedrv_support_74hc) /*PolarIII*/
        {
            reset_value=(SYS_HWCFG_POE_SOFTWARE_RESET);
            *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = reset_value;
    
            SYSFUN_Sleep(30);
            reset_value=(SYS_HWCFG_POE_SOFTWARE_RESET_MASK | SYS_HWCFG_POE_SOFTWARE_RESET);
            *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = reset_value;
            SYSFUN_Sleep(100); // 1 sec
        }
        else if (SUPPORT_74HC_16BITS == poedrv_support_74hc) /*IronBridge*/
        {
            pattern = 0x5; /* Yellow On, PoE reset HIGH, PoE disable High */
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            for (i=0;i<=15;i++)
            {
                if (pattern%2 == 1)
                {
                    *((UI8_T *) 0xb8000067) = 0x2;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x6;
                }
                else
                {
                    *((UI8_T *) 0xb8000067) = 0x0;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x4;
                }
                GPIO_SWDELAY();
                pattern = pattern/2;
            }
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            *((UI8_T *) 0xb8000067) = 0x0;
            GPIO_SWDELAY();

            SYSFUN_Sleep(100);
            pattern = 0x1; /* Yellow On, PoE reset LOW, PoE disable High */
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            for (i=0;i<=15;i++)
            {
                if (pattern%2 == 1)
                {
                    *((UI8_T *) 0xb8000067) = 0x2;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x6;
                }
                else
                {
                    *((UI8_T *) 0xb8000067) = 0x0;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x4;
                }
                GPIO_SWDELAY();
                pattern = pattern/2;
            }
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            *((UI8_T *) 0xb8000067) = 0x0;
            GPIO_SWDELAY();

            SYSFUN_Sleep(100);
            pattern = 0x5; /* Yellow On, PoE reset HIGH, PoE disable High */
            if (sys_info.post_pass == TRUE)
                pattern |= 0x40; /* Set power LED to green if POST successed. */
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            for (i=0;i<=15;i++)
            {
                if (pattern%2 == 1)
                {
                    *((UI8_T *) 0xb8000067) = 0x2;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x6;
                }
                else
                {
                    *((UI8_T *) 0xb8000067) = 0x0;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x4;
                }
                GPIO_SWDELAY();
                pattern = pattern/2;
            }
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            *((UI8_T *) 0xb8000067) = 0x0;
            GPIO_SWDELAY();
        }
        
        break;
    }
#else
DBG_PRINT();

    /* write CPLD to reset PoE module */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SYSTEM_RESET_ADDR, &virt_addr);
    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &reset_value);
    reset_value&=(~SYS_HWCFG_SYSTEM_RESET_POE_SOFTWARE_RESET);
    PHYADDR_ACCESS_Write(virt_addr, 1, 1, &reset_value);
    SYSFUN_Sleep(30);

    PHYADDR_ACCESS_Read(virt_addr, 1, 1, &reset_value);
    reset_value|=(SYS_HWCFG_SYSTEM_RESET_POE_SOFTWARE_RESET);
    PHYADDR_ACCESS_Write(virt_addr, 1, 1, &reset_value);

SYSFUN_Sleep(100); /* 1 sec */

#endif

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
    printf("%s\n", __FUNCTION__);
#if 0
    UI8_T         reset_value = 0;

    if (is_enable)
    {
        reset_value = (SYS_HWCFG_POE_SOFTWARE_RESET_MASK | SYS_HWCFG_POE_SOFTWARE_RESET);
    }
    else
    {
        reset_value = (SYS_HWCFG_POE_SOFTWARE_RESET);
    }

#ifndef INCLUDE_DIAG
    { /* 0: GBIC normal
       * 1: GBIC reset
       * => keeping 0 for moudle reset fileds if the module type is GBIC.
       */
        UI8_T module_type;
        #define MODULE_A_RESET_MASK 0xEF
        #define MODULE_B_RESET_MASK 0xF7

        STKTPLG_MGR_GetModuleAType(&module_type);
        if (SYS_HWCFG_MODULE_ID_1000BASE_X_GBIC == (module_type & SYS_HWCFG_MODULE_ID_MASK))
        {
            reset_value &= MODULE_A_RESET_MASK;
        }
        STKTPLG_MGR_GetModuleBType(&module_type);
        if (SYS_HWCFG_MODULE_ID_1000BASE_X_GBIC == (module_type & SYS_HWCFG_MODULE_ID_MASK))
        {
            reset_value &= MODULE_B_RESET_MASK;
        }
    }
#endif

    *((UI8_T *)SYS_HWCFG_CPLD_HW_RESET_ADDR) = reset_value;


    SYSFUN_Sleep(1);
#endif
} /* End of POEDRV_ReleaseSoftwareReset() */

/* FUNCTION NAME : POEDRV_SetModuleStatus
 * PURPOSE: This function is used to enable/disable PoE function via CPLD
 * INPUT:   status: TRUE - enable, FALSE - disable
 *          
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_SetModuleStatus(BOOL_T status)
{
    SYS_TYPE_VAddr_T virt_addr;
    UI8_T value;
//#define GPIO_SWDELAY();   for (j=0;j<=10;j++){}
    UC_MGR_Sys_Info_T sys_info;
#if 0//Eugene temp
    UI32_T       pattern, i;
#endif
DBG_PRINT("%s", status? "TRUE":"FALSE");

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
        printf("\r\nUnable to get sysinfo from UC Manager.");
        return;
    }
#if 0 /* Eugene marked for not using universal image */
    if ((SUPPORT_POE == poedrv_support_poe) && (SUPPORT_74HC_NONE == poedrv_support_74hc)) /*PolarIII*/
    {
        if (TRUE == status)
        {
            *((volatile UI8_T *)SYS_HWCFG_POE_CONTROL_ADDR) = SYS_HWCFG_POE_ENABLE; 
        }
        else
        {
            *((volatile UI8_T *)SYS_HWCFG_POE_CONTROL_ADDR) = 0; /* bit0 = 0 */
        }
    }
    else if ((SUPPORT_POE == poedrv_support_poe) && (SUPPORT_74HC_16BITS == poedrv_support_74hc)) /*IronBridge*/
    {
        if (TRUE == status)
        {
            pattern = 0x5; /* PoE reset HIGH, PoE disable High */
            if (sys_info.post_pass == TRUE)
                pattern |= 0x40; /* Set power LED to green if POST successed. */
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            for (i=0;i<=15;i++)
            {
                if (pattern%2 == 1)
                {
                    *((UI8_T *) 0xb8000067) = 0x2;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x6;
                }
                else
                {
                    *((UI8_T *) 0xb8000067) = 0x0;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x4;
                }
                GPIO_SWDELAY();
                pattern = pattern/2;
            }
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            *((UI8_T *) 0xb8000067) = 0x0;
            GPIO_SWDELAY();
        }
        else
        {
            pattern = 0x4; /* PoE reset HIGH, PoE disable LOW */
            if (sys_info.post_pass == TRUE)
                pattern |= 0x40; /* Set power LED to green if POST successed. */
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            for (i=0;i<=15;i++)
            {
                if (pattern%2 == 1)
                {
                    *((UI8_T *) 0xb8000067) = 0x2;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x6;
                }
                else
                {
                    *((UI8_T *) 0xb8000067) = 0x0;
                    GPIO_SWDELAY();
                    *((UI8_T *) 0xb8000067) = 0x4;
                }
                GPIO_SWDELAY();
                pattern = pattern/2;
            }
            *((UI8_T *) 0xb8000067) = 0x8;
            GPIO_SWDELAY();
            *((UI8_T *) 0xb8000067) = 0x0;
            GPIO_SWDELAY();
        }
    }
#else

    /* write CPLD to enable/disable PoE module */
    PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_SYSTEM_RESET_ADDR, &virt_addr);
	PHYADDR_ACCESS_Read(virt_addr, 1, 1, &value);
    if (TRUE == status)
        value|=SYS_HWCFG_SYSTEM_RESET_POE_ENABLE;
    else
        value&=~(SYS_HWCFG_SYSTEM_RESET_POE_ENABLE);

    PHYADDR_ACCESS_Write(virt_addr, 1, 1, &value);

#endif
}


/* FUNCTION NAME : POEDRV_SoftwareDownload
 * PURPOSE: This function is used to upgrade software version of PoE controller
 * INPUT:   unit -- unit ID
 *          filename -- filename to be downloaded
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
BOOL_T POEDRV_SoftwareDownload(UI32_T unit, UI8_T *filename)
{
#if 0 /* Eugene marked */
#if (SYS_CPNT_STACKING == TRUE)
	POEDRV_Rx_IscBuf_T   req_buf;
    UI8_T  my_unit;
#if 0 /* Eugene temp */
	UI8_T   rep_buf;
#endif
#endif

    if((unit < 1) || (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK))
    {
    	 return FALSE;
    }

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            /* set remote unit port Power Detection Controlt
             */
            req_buf.serviceID = POEDRV_Software_Download;
            memcpy(req_buf.info.filename,filename,SYS_ADPT_FILE_SYSTEM_NAME_LEN);

#if 0 /* Eugene temp */
        	if (ISC_Remote_Call(unit, ISC_POEDRV_SID, sizeof(req_buf), (UI8_T *)&req_buf,
    						    sizeof(rep_buf), (UI8_T *)&rep_buf, POEDRV_RETRY_TIMES , POEDRV_TIME_OUT))
    		{
        		if (rep_buf==TRUE)
        		{
        			return TRUE;
        		}
        		/* EH not implemented yet
        		 */
        		return FALSE;
        	}
    		/* EH not implemented yet
    		 */
    	return FALSE; /* can't config remote */
#endif
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */

	        if (POEDRV_LocalSoftwareDownload(filename) == FALSE)
	        {
                /* EH not implemented yet
                 */
	            return FALSE;
	        }
	        return TRUE;
          }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
#endif
   return FALSE;

} /* End of POEDRV_SoftwareDownload() */

/* FUNCTION NAME : POEDRV_LocalSoftwareDownload
 * PURPOSE: This function is used to upgrade software version of PoE controller
 * INPUT:   filename -- filename to be downloaded
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static  BOOL_T POEDRV_LocalSoftwareDownload(UI8_T *filename)
{
    printf("%s\n", __FUNCTION__);
    BOOL_T ret=FALSE;
#if 0

    /* Step 1 Stop the periodic polling task
     */

    if ( is_enter_program_mode == FALSE )
    {
         /* Set flag to notify POEDRV task the PoE controller is in program mode.
          */
        is_enter_program_mode = TRUE;
        while ( POEDRV_OM_IsStopMonitorFlagOn() == FALSE ){SYSFUN_Sleep(10);};
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if(dynamic_hooked_poe->poedrv_software_download_toggle != NULL)
        ret = POE_SOFTWARE_DOWNLOAD_TOGGLE(dynamic_hooked_poe);
    POEDRV_LEAVE_CRITICAL_SECTION;
     SYSFUN_Sleep(10);

     /* PoE controller */
    if (ret)
    {
        is_enter_program_mode = FALSE;
        POEDRV_HardwareReset();
#ifndef INCLUDE_DIAG
        if ( POEDRV_BACKDOOR_IsDisplayDebugFlagOn() )
            printf("\n\rError on sending Software download packet");
#endif
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if(dynamic_hooked_poe->poedrv_software_download_process != NULL)
        ret = POE_SOFTWARE_DOWNLOAD_PROCESS(dynamic_hooked_poe,filename,poedrv_my_unit_id);
    else
        ret=FALSE;
    POEDRV_LEAVE_CRITICAL_SECTION;
    if(ret) is_enter_program_mode = FALSE;
#endif

    return ret;
} /* End of POEDRV_LocalSoftwareDownload() */

/* LOCAL SUBPROGRAM BODIES
 */

#if 0 /* Eugene do it in POEDRV_OM_Reset */
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
    UI32_T port;
#if 0 /* Eugene marked for not using universal image */
    UI32_T unit=1;
    UI32_T board_id;
    STKTPLG_BOARD_BoardInfo_T  *board_info_p;
#endif

    for (port=0;port<POEDRV_NO_OF_POE_PORTS;port++)
    {
        poedrv_port_L2P_matrix[port] = 0xFF;
        poedrv_port_P2L_matrix[port] = 0xFF;
        per_port_power_max_allocation[port] = 15400;
    }
    main_pse_power_max_allocation = 180;

    /* Get my unit ID
     */
#ifdef INCLUDE_DIAG
    poedrv_my_unit_id   = 1;
    poedrv_num_of_units = 1;
#else
    STKTPLG_POM_GetMyUnitID(&poedrv_my_unit_id);
    STKTPLG_POM_GetNumberOfUnit(&poedrv_num_of_units);
#endif

#if 0 /* Eugene marked for not using universal image */
    /* Get board_id */
    if ( STKTPLG_MGR_GetUnitBoardID(unit, &board_id) )
    {
        /* Get POEDRV board_info and set relative vlaue */
        if ( STKTPLG_BOARD_GetBoardInformation( board_id, &board_info_p))
        {
            main_pse_power_max_allocation = board_info_p->main_pse_power_max_allocation;

            poedrv_min_port_number = board_info_p->min_poe_port_num;
            poedrv_max_port_number = board_info_p->max_poe_port_num;

            memcpy(poedrv_port_L2P_matrix,&board_info_p->Logical2PhysicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(poedrv_port_P2L_matrix,&board_info_p->Physical2LogicalPort,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

            memcpy(per_port_power_max_allocation,&board_info_p->per_port_power_max_allocation,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
        }
        else
            SYSFUN_Debug_Printf("\n\r*** Can not get related board information.***");
    }
    else
        SYSFUN_Debug_Printf("\n\r*** Fail in getting board id!***");
#else
    main_pse_power_max_allocation = POE_PSE_POWER_MAX_ALLOCATION;

    poedrv_min_port_number = POE_MIN_PORT_NUMBER;
    poedrv_max_port_number = POE_MAX_PORT_NUMBER;

    memcpy(poedrv_port_L2P_matrix,POE_PORT_L2P_MATRIX,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

    memcpy(poedrv_port_P2L_matrix,POE_PORT_P2L_MATRIX,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);

    memcpy(per_port_power_max_allocation,POE_PER_PORT_POWER_MAX_ALLOCATION,sizeof(UI32_T)*POEDRV_NO_OF_POE_PORTS);
#endif


    /* Initialize Main PSE with default value
     */
    poedrv_mainpower_info.unit_id              = poedrv_my_unit_id;
    poedrv_mainpower_info.main_pse_power       = MIN_pethMainPsePower;
    poedrv_mainpower_info.main_pse_oper_status = VAL_pethMainPseOperStatus_faulty;
    poedrv_mainpower_info.main_pse_consumption = 0;
    poedrv_mainpower_info.legacy_detection_enable = 0;
    /* Initialize state transition on all ports
     */
    // memset(poedrv_port_state, POEDRV_PORT_IS_OFF, sizeof(UI8_T)*(POEDRV_NO_OF_POE_PORTS+1));


    /* Initialize port PSE with default value
     */
    for ( port=0; port<POEDRV_NO_OF_POE_PORTS; port++)   /* And  */
    {
        poedrv_port_info[port].detection_status  = VAL_pethPsePortDetectionStatus_disabled;
        poedrv_port_info[port].power_class       = VAL_pethPsePortPowerClassifications_class0;
        poedrv_port_info[port].power_consumption = 0;
        poedrv_port_info[port].is_overload       = FALSE;
    }

    /* Initialize flag for program mode to download software to PoE controller
     */
    is_enter_program_mode = FALSE;

    return;

} /* End of POEDRV_InitDataBase() */
#endif /* Eugene do it in POEDRV_OM_Reset */

/* FUNCTION NAME : POEDRV_TaskMain
 * PURPOSE: The main body of poedrv task
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:
 */
static void POEDRV_TaskMain(void)
{
    UI32_T events;
	void*  timer_id;
	BOOL_T status;

#ifndef INCLUDE_DIAG
#if 0 /* Eugene marked for not using universal image */
    /*if not a poe device, suspend this task itself*/
    if( !STKTPLG_OM_IsPoeDevice(1) ) /*unit = 1 means local unit */
    {
        SYSFUN_SuspendTask (poedrv_task_id);
    }
#endif
#endif

    timer_id = SYSFUN_PeriodicTimer_Create();
    SYSFUN_PeriodicTimer_Start(timer_id, SYS_BLD_POEDRV_UPDATE_POE_STATS_TICKS, 0x1);

    /* main loop of task
     */
    while (TRUE)
    {
        // SYSFUN_Sleep(SYS_BLD_POEDRV_UPDATE_POE_STATS_TICKS);
        // SYSFUN_Sleep(10);
        SYSFUN_ReceiveEvent(0x1,
                            SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER, &events);

        if (POEDRV_BACKDOOR_IsDisplayDebugFlagOn())
        {
            printf("current tick: %lu\n", SYSFUN_GetSysTick());
        }

        /* Polling PoE status only in Master or Slave mode
         */
        POEDRV_OM_GetProvisionComplete(&status);
        if ( ((POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE) && (status)) ||
             ((POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE) && (status)))
        {
            #if 0
             /* Do nothing, if PoE controller is in program mode for software download
              */
             if ( is_enter_program_mode == TRUE )
             {
                  POEDRV_OM_SetStopMonitorFlag(TRUE);
                  is_init_flag    = TRUE;
                  continue;
             }

             POEDRV_OM_SetStopMonitorFlag(FALSE);
             #endif

            /* Polling the PoE controller in order to update database
             */
            if ((POEDRV_OM_IsStopMonitorFlagOn()==FALSE))
            {
                POEDRV_MonitorPoePortStatus();
                POEDRV_MonitorPoeMainpowerStatus();
                POEDRV_MonitorPoeOperationStatus();
                POEDRV_MonitorPoePortStatistic();
            }
        }
    } /* End of while (TRUE) */
} /* End of POEDRV_TaskMain() */


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
    UC_MGR_Sys_Info_T sys_info;
    UI32_T port, phy_port, value, port_min=0, port_max=0;
    BOOL_T ret=FALSE;
DBG_PRINT();

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
        printf("\r\nUnable to get sysinfo from UC Manager.");
        return FALSE;
    }
#if 0 /* Eugene marked for not using universal image */
    if ((SUPPORT_POE == poedrv_support_poe) && (SUPPORT_74HC_16BITS != poedrv_support_74hc)) /* Not Ironbridge */
#endif
    /* Enable Port mapping */
    {
#if 0
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_disable_logical_port_map, ret);
#else
        POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_enable_logical_port_map, ret,
            poedrv_port_mapping, port_max);

        if (FALSE == ret)
        {
            // printf("(%s) viloation type error,  port %d\n", __FUNCTION__, port);
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }
#endif
    }

    /* Set Maximun power */
	POEDRV_OM_GetMainPowerMaxAllocation(&value);
    if (POEDRV_LocalSetMainpowerMaximumAllocation(value) == FALSE)
    {
        // printf("(%s) System MAX allocation Fail\n", __FUNCTION__);
        return FALSE;
    }

    /* Enable Capacitor Detection */
    if (POEDRV_LocalSetCapacitorDetectionControl(TRUE) == FALSE)
    {
        // printf("(%s) Detection Type Error\n", __FUNCTION__);
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    for (port = port_min ; port <= port_max ; port ++)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }

        /* viloation type (Threshold user define) */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_power_threshold_type, ret,
            phy_port, POEDRV_PORT_POWER_THRESHOLD_USER_DEFINE);

        if (FALSE == ret)
        {
            // printf("(%s) viloation type error,  port %d\n", __FUNCTION__, port);
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }

/* Use the default configuration */
#if 0
        /* Normal-Power Mode  */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_force_high_power_mode, ret,
            phy_port, 0);
        if (FALSE == ret)
        {
            // printf("(%s) high power mode error,  port %d\n", __FUNCTION__, port);
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }
#endif

        /* Default Port power allocation */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_power_limit, ret,
            phy_port, SYS_HWCFG_MAX_POWER_INLINE_ALLOCATION);
        if (FALSE == ret)
        {
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }


/* Daniel Chen, 2008/1/5
 * bcm5910x will reset configuration after nReset.
 * (after image V3.0)
 */
#if 1
        /* Daniel Chen, 2007/10/12,
         * Due to the bcm59101 can't reset configuration after nReset,
         * We set the default value manually.
         */    
        /* Default Admin mode */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_admin_status, ret,
            phy_port, SYS_DFLT_PSE_PORT_ADMIN);
        if (FALSE == ret)
        {
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }


        /* Default Priority */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_priority, ret,
            phy_port, SYS_DFLT_PSE_PORT_POWER_PRIORITY);
        if (FALSE == ret)
        {
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }

        /* Default Port power pairs */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_power_pairs, ret,
            phy_port, SYS_DFLT_PSE_PORT_POWER_PAIRS);
        if (FALSE == ret)
        {
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }
#endif

        /* This function can't work on PoE image Ver 2.1 
         * (informed by Broadcom FAE - Finite)
         */
#if 0
        /* Auto Power Up mode */
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_auto_mode, ret,
            phy_port, 1);
        if (FALSE == ret)
        {
            // printf("(%s) port auto mode error,  port %d\n", __FUNCTION__, port);
            POEDRV_LEAVE_CRITICAL_SECTION;
            return FALSE;
        }
#endif        
    }

    /* power management mode */
    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_power_management_mode, ret,
        POEDRV_POWER_MANAGE_DYNAMIC);
    if (FALSE == ret)
    {
        // printf("(%s) power management mode error\n", __FUNCTION__);
        POEDRV_LEAVE_CRITICAL_SECTION;
        return FALSE;
    }

    POEDRV_LEAVE_CRITICAL_SECTION;

    return TRUE;

} /* End of POEDRV_SetDefaultValue() */

#if 0
/* FUNCTION NAME: POEDRV_LocalGetPortPowerConsumption
 * PURPOSE: This function is used to get power consumption of a port.
 * INPUT:   port -- port ID
 *
 * OUTPUT:  milliwatts -- power consumption of a port in milliwatts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:
 */
static BOOL_T POEDRV_LocalGetPortPowerConsumption(UI32_T port, UI32_T *milliwatts)
{
    UI32_T phy_port;       /* Physical port ID      */
    BOOL_T ret=FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_get_port_power_consumption, ret,
            port, milliwatts);

    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;
} /* End of POEDRV_LocalGetPortPowerConsumption() */
#endif

#if 0
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
    UI32_T phy_port;
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;
    if(dynamic_hooked_poe->poedrv_get_port_power_class != NULL)
        ret = POE_GET_PORT_POWER_CLASS(dynamic_hooked_poe, phy_port, power_class);
    POEDRV_LEAVE_CRITICAL_SECTION;

    return ret;

} /* End of POEDRV_LocalGetPortPowerClassification() */
#endif


/* FUNCTION NAME : POEDRV_MonitorPoePortStatus
 * PURPOSE: This function is used to periodically query PoE port statistic and
 *          update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static void POEDRV_MonitorPoePortStatistic(void)
{
    UI32_T port, phy_port;
    UI32_T i, port_min, port_max;
    UI32_T current_ticks, ticks, value;
    UI8_T  ctr[POEDRV_MAX_COUNTER_TYPE] = {0};
    BOOL_T ret;
//DBG_PRINT();
    POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
    for (port=port_min; port<=port_max; port++)
    {
        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            continue;
        }

        /* intitial the local variable */
        memset(ctr, 0, sizeof(UI8_T)*POEDRV_MAX_COUNTER_TYPE);

        POEDRV_ENTER_CRITICAL_SECTION;
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_get_port_all_counter, ret,
                phy_port, ctr);
        if (ret==TRUE)
        {
            POEDRV_EXEC(dynamic_hooked_poe->poedrv_reset_port_statistic, ret,
                    phy_port);
        }
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (ret == TRUE) /* update database */
        {
            for (i=0;i<POEDRV_MAX_COUNTER_TYPE;i++)
            {
                if (ctr[i] > 0)
                {
                    POEDRV_OM_AddPortCounter(port, i, (UI32_T)ctr[i]);
                }
            }

            /* ping-pong port issue 
             * if denied counter increased, we store the time_ticks(first time) and counters
             */
            {
                if (ctr[POEDRV_POWER_DENIED_COUNTER] > 0 )
                {
                    POEDRV_OM_GetPingPongInfoStartTicks(port, &ticks);
                    if (ticks == 0)
                    {
                        ticks = SYSFUN_GetSysTick();
                        POEDRV_OM_SetPingPongInfoStartTicks(port, ticks);
                    }

                    POEDRV_OM_AddPingPongInfoTimesOfPowerDenied(port, (UI32_T)ctr[POEDRV_POWER_DENIED_COUNTER]);
                }
            }

        }
        else
        {
            /* printf("(%s) dynamic_hooked_poe->poedrv_get_port_all_counter Error, Port (%d) \n", __FUNCTION__, port);*/
        }
    }
    /* ping-pong port issue 
     * count the denied counters
     */
    current_ticks = SYS_TIME_GetSystemTicksBy10ms();
    for (port=port_min; port<=port_max; port++)
    {
        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            continue;
        }

        POEDRV_OM_GetPingPongInfoStartTicks(port, &ticks);
        if (ticks > 0)
        {
            /* 20 seconds, 
             * power denied over 3 times , We disable the port
             * one time disable one port
             */
            POEDRV_OM_GetPingPongInfoTimesOfPowerDenied(port, &value);
            if (value >= 3)
            {
                /* clear all port data, re-calculate all port status
                 */
                POEDRV_OM_ResetAllPingPongInfo();

                /* printf("Ping-Pong Port Detected (pno: %d)!\n", port); */
                POEDRV_Notify_PowerDeniedOccurFrequently(1, port);
                break;
            }
            
            if ((current_ticks - ticks) > 2000) /* 20 secs */
            {
                /* over 20 secs and counter of power denied < 3
                 * we reset the data.
                 */
                POEDRV_OM_ResetOnePingPongInfo(port);
            }
        }
    }
}


/* FUNCTION NAME : POEDRV_MonitorPoePortStatus
 * PURPOSE: This function is used to periodically query PoE port status and
 *          update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static void POEDRV_MonitorPoePortStatus(void)
{
    POEDRV_Port_Info_T portInfo;
    UI32_T port, phy_port;
    UI32_T my_unit, port_min, port_max, value;
    UI32_T volt, cur, classification, power;
    I32_T  value1;
    UI8_T  st1, st2, status;
    BOOL_T ret, is_overload;
//DBG_PRINT();

    POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
    for (port=port_min; port<=port_max; port++)
    {

        if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        {
            continue;
        }

        /* intitial the local variable */
        portInfo.detection_status = VAL_pethPsePortDetectionStatus_disabled;
        portInfo.is_overload = FALSE;
        portInfo.power_class = VAL_pethPsePortPowerClassifications_class0;
        portInfo.power_consumption = 0;
        portInfo.led_status = POEDRV_TYPE_PORT_LINKOFF;

        POEDRV_ENTER_CRITICAL_SECTION;
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_get_port_status2, ret,
                phy_port, &st1, &st2);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (TRUE == ret)
        {
            switch (st1)
            {
                case POEDRV_PORT_STATUS1_DISABLE:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_disabled;
                    break;
                case POEDRV_PORT_STATUS1_SEARCHING:
                case POEDRV_PORT_STATUS1_REQUEST_POWER:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_searching;
                    break;
                case POEDRV_PORT_STATUS1_DELIVER_POWER:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_deliveringPower;
                    portInfo.power_class = st2;  /* value in ASIC start from 0 */
                    portInfo.led_status = POEDRV_TYPE_PORT_LINKON;
                    break;
                case POEDRV_PORT_STATUS1_TEST:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_test;
                    portInfo.led_status = POEDRV_TYPE_PORT_LINKON;
                    break;
                case POEDRV_PORT_STATUS1_FAULT:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_fault;
                    if (st2 == POEDRV_PORT_STATUS2_ERROR_OVERLOAD)
                        portInfo.is_overload = TRUE;
                    break;
                case POEDRV_PORT_STATUS1_OTHER_FAULT:
                    portInfo.detection_status = VAL_pethPsePortDetectionStatus_otherFault;
                    if (st2 == POEDRV_PORT_STATUS2_ERROR_OVERLOAD)
                        portInfo.is_overload = TRUE;
                    break;
            }

            POEDRV_OM_GetMyUnitID(&my_unit);
	
            /* LED status */
            POEDRV_OM_GetPortInfoLedStatus(port, &status);
            if (portInfo.led_status != status)
            {               
                POEDRV_OM_SetPortInfoLedStatus(port, portInfo.led_status);
                POEDRV_Notify_PortStatusChange((UI32_T)my_unit, port, portInfo.led_status);                
            }

            /* detection status */
			POEDRV_OM_GetPortInfoDetectionStatus(port, &status);
            if (portInfo.detection_status != status)
            {               
                POEDRV_OM_SetPortInfoDetectionStatus(port, portInfo.detection_status);
                POEDRV_Notify_PortDetectionStatusChange((UI32_T)my_unit, port, portInfo.detection_status);                
            }

            /* power class */
			POEDRV_OM_GetPortInfoPowerClass(port, &classification);
            if (portInfo.power_class != classification)
            {
                POEDRV_OM_SetPortInfoPowerClass(port, portInfo.power_class);
                POEDRV_Notify_PortPowerClassificationChange((UI32_T)my_unit, port, portInfo.power_class);
            }

            /* overload status */
			POEDRV_OM_GetPortInfoIsOverload(port, &is_overload);
            if (portInfo.is_overload != is_overload)
            {
                POEDRV_OM_SetPortInfoIsOverload(port, portInfo.is_overload);
                if (portInfo.is_overload == TRUE)
                {
                    POEDRV_Notify_PortOverloadStatusChange((UI32_T)my_unit, port, portInfo.is_overload);
                }
            }
        } /* if (TRUE == ret) */
        else
        {
            //printf("(%s) dynamic_hooked_poe->poedrv_get_port_status2 Error, Port (%d) \n", __FUNCTION__, port);
        }

        POEDRV_ENTER_CRITICAL_SECTION;
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_get_port_measurement, ret,
            phy_port, &value, &value1, &volt, &cur);
        POEDRV_LEAVE_CRITICAL_SECTION;

        if (TRUE == ret)
        {
            portInfo.power_consumption = value;
			POEDRV_OM_GetPortInfoPowerConsumption(port, &power);
            if (portInfo.power_consumption != power)
            {
                POEDRV_OM_SetPortInfoPowerConsumption(port, portInfo.power_consumption);
                POEDRV_Notify_PortPowerConsumptionChange((UI32_T)my_unit, port, portInfo.power_consumption);
            }
            /* portInfo.temperature = value1; */
			POEDRV_OM_SetPortInfoTemperature(port, value1);
			POEDRV_OM_SetPortInfoVoltage(port, volt);
			POEDRV_OM_SetPortInfoCurrent(port, cur);
        }
        else
        {
            //printf("(%s) dynamic_hooked_poe->poedrv_get_port_power_consumption Error , Port (%d)\n", __FUNCTION__, port);
        }
    }

} /* End of POEDRV_MonitorPoePortStatus() */

/* FUNCTION NAME : POEDRV_MonitorPoeMainpowerStatus
 * PURPOSE: This function is used to periodically query PoE mainpower status and
 *          update database, if necessary.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
static void POEDRV_MonitorPoeMainpowerStatus(void)
{
    UI32_T power_consumption = 0;
    UI32_T port = 0, my_unit;
    UI32_T used, total, port_min, port_max, power;
    static UI32_T sys_pwr = POEDRV_TYPE_SYSTEM_NORMAL;
//DBG_PRINT();

    POEDRV_OM_GetMyUnitID(&my_unit);

    POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
    for(port=port_min;port<=port_max;port++)
    {
        POEDRV_OM_GetPortInfoPowerConsumption(port, &power);
        power_consumption += power;
    }

    power_consumption /= 1000; /* Unit: mW/LSB -> W/LSB */
	POEDRV_OM_GetMainPowerInfoMainConsumption(&power);
    if ( power != power_consumption )
    {
        POEDRV_OM_SetMainPowerInfoMainConsumption(power_consumption);
        POEDRV_Notify_MainPseConsumptionChange(my_unit, power_consumption);
    }

    /* Daniel Chen, 2007/10/11, According ES4526R-PoE Spec.
     * If the main power usage over 90 % and PoE button pressed,
     * we will amber blinking the PoE LED.
     */
	POEDRV_OM_GetMainPowerInfoMainConsumption(&used);
	POEDRV_OM_GetMainPowerInfoMainPower(&total);
    if (( used * 100 / total >= ALARM_THRESHOLD) && sys_pwr == POEDRV_TYPE_SYSTEM_NORMAL)
    {
        sys_pwr = POEDRV_TYPE_SYSTEM_OVEROAD;
        POEDRV_Notify_IsMainPowerReachMaximun(1, sys_pwr);
    }
    else if (( used * 100 / total < ALARM_THRESHOLD) && sys_pwr == POEDRV_TYPE_SYSTEM_OVEROAD)
    {
        sys_pwr = POEDRV_TYPE_SYSTEM_NORMAL;
        POEDRV_Notify_IsMainPowerReachMaximun(1, sys_pwr);
    }
    
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
static void POEDRV_MonitorPoeOperationStatus(void)
{
    UI32_T my_unit;
    UI8_T  oper_status, value;
//DBG_PRINT();

    oper_status = VAL_pethMainPseOperStatus_on;

    POEDRV_OM_GetMyUnitID(&my_unit);
    POEDRV_OM_GetMainPowerInfoMainOperStatus(&value);
    if ( value != oper_status )
    {
        POEDRV_OM_SetMainPowerInfoMainOperStatus(oper_status);
        POEDRV_Notify_PseOperStatusChange(my_unit, (UI32_T)oper_status);
    }
} /* End of POEDRV_MonitorPoeOperationStatus() */

#if 0 // daniel
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

    return ret;

} /* End of POEDRV_SendSoftwareDownload() */
#endif

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
static void POEDRV_Notify_PortDetectionStatusChange(UI32_T unit, UI32_T port, UI8_T detection_status)
{
DBG_PRINT();
#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Port_Detection) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Port_Detection;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        isc_buffer_p->info.notify.port_detection_status = detection_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(SYS_MODULE_POEDRV, unit, port, detection_status);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortDetectionStatusChange() */

/*for POE Led*/
static void POEDRV_Notify_PortStatusChange(UI32_T unit, UI32_T port, UI8_T actual_port_status)
{
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit,port = %ld,%2ld][status = %d][Notify_PortStatusChange]",unit, port, actual_port_status);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Port_Status) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Port_Status;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        isc_buffer_p->info.notify.actual_status = actual_port_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PortStatusChange(SYS_MODULE_POEDRV, unit, port, actual_port_status);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortStatusChange() */

static void POEDRV_Notify_IsMainPowerReachMaximun(UI32_T unit, UI32_T status)
{
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit = %ld][status = %ld][%s]",unit, status, __FUNCTION__);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Is_Main_Power_Reach_Maximun) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Is_Main_Power_Reach_Maximun;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->info.notify.actual_status = status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_IsMainPowerReachMaximun(SYS_MODULE_POEDRV, unit, status);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_IsMainPowerReachMaximun() */


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
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit,port = %ld,%2ld][status = %d][Notify_PortOverloadStatusChange]",unit, port, is_overload);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Port_Overload) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Port_Overload;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        isc_buffer_p->info.notify.is_port_overload = is_overload;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(SYS_MODULE_POEDRV, unit, port, is_overload);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortOverloadStatusChange() */



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
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit,port = %ld,%2ld][Power Consumption = %ld][Notify_PortPowerConsumptionChange]",unit, port, power_consumption);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Port_Power_Consumption) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Port_Power_Consumption;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        isc_buffer_p->info.notify.power_consumption = power_consumption;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(SYS_MODULE_POEDRV, unit, port, power_consumption);

#if (SYS_CPNT_STACKING == TRUE)
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
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit,port = %ld,%2ld][Power class = %ld][Notify_PortPowerClassificationChange]",unit, port, power_class);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Port_Power_Classification) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_Port_Power_Classification;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        isc_buffer_p->info.notify.power_classification = power_class;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(SYS_MODULE_POEDRV, unit, port, power_class);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_PortPowerClassificationChange() */


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
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit = %ld][Main PSE Consumption = %ld][Notify_MainPseConsumptionChange]",unit, power_consumption);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PSE_Power_consumption) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_PSE_Power_consumption;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->info.notify.pse_consumption = power_consumption;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(SYS_MODULE_POEDRV, unit, power_consumption);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/

} /* End of POEDRV_Notify_MainPseConsumptionChange() */


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
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit = %ld][Operational status = %ld][Notify_PseOperStatusChange]",unit, oper_status);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PSE_Oper_Status) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_PSE_Oper_Status;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->info.notify.pse_oper_status = oper_status;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE);

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(SYS_MODULE_POEDRV, unit, oper_status);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/
} /* End of POEDRV_Notify_PseOperStatusChange() */


/* FUNCTION NAME : POEDRV_Notify_PowerDeniedOccurFrequently
 * PURPOSE: This function is used to notify the callback function that
 *          the power denied occurs many times in short period
 * INPUT:   unit -- unit ID
 *          port -- port number
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   None
 */
static void POEDRV_Notify_PowerDeniedOccurFrequently(UI32_T unit, UI32_T port)
{
DBG_PRINT();
#ifndef INCLUDE_DIAG
    if ( POEDRV_BACKDOOR_IsDisplayNotifyFlagOn() )
         printf("\n\r[unit = %ld][port = %ld][Notify_PowerDeniedOccurFrequently]",unit, port);
#endif

#if (SYS_CPNT_STACKING == TRUE)
    if (POEDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        L_MM_Mref_Handle_T* mref_handle_p;
        POEDRV_Rx_IscBuf_T* isc_buffer_p;
        UI32_T              pdu_len;
        UI16_T              unit_bmp=0;
        UI8_T               master_unit_id;

        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                  L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_PSE_Power_Denied_Occur_Frequently) /* user_id */);
        isc_buffer_p =  L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
            return;
        }

        isc_buffer_p->serviceID     = POEDRV_PSE_Power_Denied_Occur_Frequently;
        isc_buffer_p->unit          = unit;
        isc_buffer_p->port          = port;
        STKTPLG_POM_GetMasterUnitId(&master_unit_id);
        unit_bmp = POEDRV_UNIT_TO_UNITBMP(master_unit_id);
        if (TRUE==ISC_SendMcastReliable(unit_bmp,ISC_POEDRV_SID,mref_handle_p,
                              SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                              POEDRV_RETRY_TIMES, POEDRV_TIME_OUT, FALSE));

        return ;
    }
    else
    {
#endif

    SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently(SYS_MODULE_POEDRV, unit, port);

#if (SYS_CPNT_STACKING == TRUE)
    }
#endif /*SYS_CPNT_STACKING*/
} /* End of POEDRV_Notify_PowerDeniedOccurFrequently() */

BOOL_T POEDRV_SetCapacitorDetectionControl(UI32_T unit, UI8_T value)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Capacitor_Detection_Control)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Capacitor_Detection_Control;
                isc_buf_p->info.legacy_detection_enable = value;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /*SYS_CPNT_STACKING*/
    	    /* if local unit or standalone
    	     */


	        if (!POEDRV_LocalSetCapacitorDetectionControl(value))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
    }
    return TRUE;

}

static BOOL_T POEDRV_LocalSetCapacitorDetectionControl( UI8_T value)
{
    UI32_T port, phy_port;
    UI32_T type, port_min, port_max;
	UI8_T  temp;
    BOOL_T ret;

    if (value != TRUE && value != FALSE) /* range check */
    {
        return FALSE;
    }

    POEDRV_OM_GetMainPowerInfoLegacyDectionEnable(&temp);
    if (value == temp) /*No change*/
    {
        return TRUE;
    }

    if (value == TRUE)
    {
        type = POEDRV_PORT_DETECTION_DOT3AF_4POINT_FOLLOWED_BY_LEGACY;
    }
    else
    {
        type = POEDRV_PORT_DETECTION_DOT3AF_4POINT;
    }

    POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
    for (port=port_min;port<=port_max;port++)
    {
        if (POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port)==FALSE)
            continue;
        POEDRV_ENTER_CRITICAL_SECTION;
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_detection_type, ret,
            phy_port, type);
        POEDRV_LEAVE_CRITICAL_SECTION;
        if(FALSE == ret)
            return FALSE;
    }
	POEDRV_OM_SetMainPowerInfoLegacyDectionEnable(value);

    return TRUE;
    //finish
} /* End of POEDRV_LocalSetCapacitorDetectionControl */

#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T SlaveSetPortAdminStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
	return POEDRV_LocalSetPortAdminStatus(buf->port,buf->info.admin_status);
}

#if 0
static void SlaveSetPortPowerDetectionControl(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
	UI8_T status = FALSE,*buf = L_MM_Mref_GetPdu(mem_ref);

    if(POEDRV_LocalSetPortPowerDetectionControl(((POEDRV_Rx_IscBuf_T *)buf)->port,((POEDRV_Rx_IscBuf_T *)buf)->info.mode))
	{
		status = TRUE;
	}
//Eugene tempe,    ISC_Remote_Reply(key,sizeof(status),&status);
	return ;
}
#endif

static BOOL_T SlaveSetPortPowerPriority(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSetPortPowerPriority(buf->port,buf->info.priority);
}

static BOOL_T SlaveSetPortPowerPairs(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSetPortPowerPairs(buf->port,buf->info.power_pairs);
}

static BOOL_T SlaveSetPortPowerMaximumAllocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSetPortPowerMaximumAllocation(buf->port,buf->info.port_Power_milliwatts);
}

static BOOL_T SlaveSetMainpowerMaximum_Allocation(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSetMainpowerMaximumAllocation(buf->info.main_power_watts);
}
/*
static void SlaveGetPortPowerConsumption(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{

}

static void SlaveHardwareReset(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
}
static void SlaveReleaseSoftwareReset(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
}
static void SlaveGetPortPowerConsumption(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref) ;
{
}
*/

static BOOL_T SlaveSoftwareDownload(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSoftwareDownload(buf->info.filename);
}
static BOOL_T SlaveGetPoeSoftwareVersion(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;
    UI8_T                version1,version2,build;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(isc_pdu), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Software_Version) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPoeSoftwareVersion(&version1, &version2, &build))
    {
        *isc_pdu = ((UI32_T)version1)*256*256 + ((UI32_T)version2)*256 + (UI32_T)build;
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}
static BOOL_T SlaveSetProvisionComplete(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    /*Only Polar3 (bid =0) or ironbridge (bid=3) need to do this PoE process, otherwise 
      it will happen exception error
    */
#if 0 /* Eugene marked for not using universal image */
    if (SUPPORT_POE == poedrv_support_poe)
#endif
    {
        POEDRV_localProvisionComplete();
    }

    return TRUE;
}

static BOOL_T SlaveSetCapacitorDetectionControl(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    return POEDRV_LocalSetCapacitorDetectionControl((UI8_T)buf->info.legacy_detection_enable);
}


static BOOL_T SlaveGetPortInvalidSignCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Invalid_Sign_Counter) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPortInvalidSignCounter(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPortPowerDeniedCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Power_Denied_Counter) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPortPowerDeniedCounter(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPortOverloadCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Overload_Counter) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPortOverloadCounter(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPortShortCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_Short_Counter) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPortShortCounter(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPortMPSAbsentCounter(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Port_MPS_Absent_Counter) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPortMPSAbsentCounter(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPoePortTemperature(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    I32_T *              isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(I32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Port_Temperature) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPoePortTemperature(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPoePortVoltage(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Port_Voltage) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPoePortVoltage(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveGetPoePortCurrent(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Get_Poe_Port_Current) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }

    if (POEDRV_LocalGetPoePortCurrent(buf->port, isc_pdu))
    {
        if (ISC_RemoteReply( mref_handle_p, key))
        {
            return TRUE;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
    }

	return FALSE;

}

static BOOL_T SlaveUserPortExisting(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
    L_MM_Mref_Handle_T * mref_handle_p;
    UI32_T *             isc_pdu;
    UI32_T               pdu_len;
DBG_PRINT("p%lu", buf->port);

    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(UI32_T), /* tx_buffer_size */
                                          L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_User_Port_Existing) /* user_id */);
    isc_pdu = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);
DBG_PRINT("p%lu", buf->port);

    if (isc_pdu==NULL)
    {
        return FALSE;
    }
DBG_PRINT("p%lu", buf->port);

    if (POEDRV_OM_Logical2PhyDevicePortID(buf->port, isc_pdu))
    {
DBG_PRINT("p%lu", buf->port);
        if (ISC_RemoteReply( mref_handle_p, key))
        {
DBG_PRINT("p%lu", buf->port);
            return TRUE;
        }
    }
    else
    {
DBG_PRINT("p%lu", buf->port);
        L_MM_Mref_Release(&mref_handle_p);
    }
DBG_PRINT("p%lu", buf->port);

	return FALSE;

}


static BOOL_T SlaveResetPort(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
	return POEDRV_LocalResetPort(buf->port);
}
	
static BOOL_T SlaveSetPortDot3atHighPowerMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
	return POEDRV_LocalSetPortDot3atHighPowerMode(buf->port,buf->info.dot3at_high_power_mode);
}
	
static BOOL_T SlaveSetPortForceHighPowerMode(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
	return POEDRV_LocalSetPortForceHighPowerMode(buf->port,buf->info.fource_high_power_mode);
}

static BOOL_T CallbackMasterPortDetectionStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PortDetectionStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit,((POEDRV_Rx_IscBuf_T *)buf)->port, ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.port_detection_status);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PortDetectionStatusChange(SYS_MODULE_POEDRV, buf->unit, buf->port, buf->info.notify.port_detection_status);

#endif
}
static BOOL_T CallbackMasterPortStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PortStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit,((POEDRV_Rx_IscBuf_T *)buf)->port, ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.actual_status);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PortStatusChange(SYS_MODULE_POEDRV, buf->unit, buf->port, buf->info.notify.actual_status);

#endif
}
static BOOL_T CallbackMasterPortOverloadStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PortOverloadStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->port,((POEDRV_Rx_IscBuf_T *)buf)->info.notify.is_port_overload);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PortOverloadStatusChange(SYS_MODULE_POEDRV, buf->unit, buf->port, buf->info.notify.is_port_overload);

#endif
}

static BOOL_T CallbackMasterPortPowerConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PortPowerConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->port,((POEDRV_Rx_IscBuf_T *)buf)->info.notify.power_consumption);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PortPowerConsumptionChange(SYS_MODULE_POEDRV, buf->unit, buf->port, buf->info.notify.power_consumption);

#endif
}

static BOOL_T CallbackMasterPortPowerClassification(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PortPowerClassificationChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.power_classification);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PortPowerClassificationChange(SYS_MODULE_POEDRV, buf->unit, buf->port, buf->info.notify.power_classification);

#endif
}
static BOOL_T CallbackMasterMainPseConsumption(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=MainPseConsumptionChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.pse_consumption);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_MainPseConsumptionChange(SYS_MODULE_POEDRV, buf->unit, buf->info.notify.pse_consumption);

#endif
}
static BOOL_T CallbackMasterPseOperStatus(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PseOperStatusChange_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->info.notify.pse_oper_status);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PseOperStatusChange(SYS_MODULE_POEDRV, buf->unit, buf->info.notify.pse_oper_status);

#endif
}
static BOOL_T CallbackMasterPowerDeniedOccurFrequently(ISC_Key_T *key, POEDRV_Rx_IscBuf_T *buf)
{
#if 0 /* Eugene temp */
	SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

	for(fun_list=PowerDeniedOccurFrequently_callbacklist; fun_list; fun_list=fun_list->next)
	    fun_list->func(((POEDRV_Rx_IscBuf_T *)buf)->unit, ((POEDRV_Rx_IscBuf_T *)buf)->port);

	return;
#else
    if(buf==NULL)
        return FALSE;

    /* Do the job */
    return SYS_CALLBACK_MGR_POEDRV_PowerDeniedOccurFrequently(SYS_MODULE_POEDRV, buf->unit, buf->port);

#endif
}

#if 0
static void CallbackMasterCapacitorDetectionControl(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
    SYS_TYPE_CallBack_T  *fun_list;
    UI32_T pdu_len;
	UI8_T *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

//Eugene tempe,	ISC_Remote_Reply(key,0,NULL);

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
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len, num_of_unit, unit;
    UI8_T                my_unit;
#endif

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        /* EH not implemented yet
         */
        return;
    }
    else
    {

        /* if local unit or standalone
         */
        POEDRV_localProvisionComplete();

#if (SYS_CPNT_STACKING == TRUE)
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                              L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Provision_Complete)/* user_id */);
        isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

        /* set remote unit port Power Detection Controlt
         */
        isc_buf_p->serviceID = POEDRV_Provision_Complete;

        POEDRV_OM_GetNumOfUnits(&num_of_unit);
        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        for(unit=1; unit<=num_of_unit; unit++)
        {
            if ( my_unit == unit )/*remote unit */
                continue;

            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* UIMSG_MGR_SetErrorCode() */
                    return;
                }
            }
        }
#endif
    }
    return;

} /* POEDRV_ProvisionComplete() */

static void POEDRV_localProvisionComplete(void)
{
    UI32_T port_min, port_max;
    UI8_T  status;
    BOOL_T ret;

    UC_MGR_Sys_Info_T sys_info;

    status = TRUE;

    POEDRV_SetModuleStatus(TRUE);

    /* Daniel Chen, for ping-pong issue
     * clear all counter before we handle ping-pong issue
     */

    if(UC_MGR_GetSysInfo(&sys_info) != TRUE)
    {
        printf("\r\nUnable to get sysinfo from UC Manager.");
        return;
    }
    
    /*Only Polar3 (bid =0) or ironbridge (bid=3) need to do this PoE process, otherwise 
      it will happen exception error
    */
#if 0 /* Eugene marked for not using universal image */
    if (SUPPORT_POE == poedrv_support_poe)
#endif
    {
        POEDRV_OM_GetPOEPortNumber(&port_min, &port_max);
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_reset_all_port_statistic,ret,
            port_max);
    }

    POEDRV_OM_SetProvisionComplete(TRUE);

    return ;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POEDRV_ResetPort
 *-------------------------------------------------------------------------
 * PURPOSE  : reset the a poe port in ASIC
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POEDRV_ResetPort(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Force_High_Power_Mode)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Force_High_Power_Mode;
                isc_buf_p->port = port;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /* SYS_CPNT_STACKING */
    	    /* if local unit or standalone
    	     */
	        if (!POEDRV_LocalResetPort(port))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
	}
    return TRUE;

} /* End of POEDRV_SetPortAdminStatus() */

static BOOL_T POEDRV_LocalResetPort(UI32_T port)
{
    UI32_T phy_port; 
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
        return FALSE;

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_reset_port, ret,
        phy_port);

    POEDRV_LEAVE_CRITICAL_SECTION;


    return ret;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POEDRV_SetPortForceHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (For Force)
 * INPUT    : lport
 *            mode : 1 - high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : exclusive with POEDRV_SetPortDot3atHighPowerMode()
 *-------------------------------------------------------------------------
 */
BOOL_T POEDRV_SetPortForceHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif
DBG_PRINT();
    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Force_High_Power_Mode)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Force_High_Power_Mode;
                isc_buf_p->port = port;
                isc_buf_p->info.fource_high_power_mode = mode;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                           SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                           POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /* SYS_CPNT_STACKING */
    	    /* if local unit or standalone
    	     */
	        if (!POEDRV_LocalSetPortForceHighPowerMode(port, mode))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
	}
    return TRUE;

} /* End of POEDRV_SetPortAdminStatus() */

static BOOL_T POEDRV_LocalSetPortForceHighPowerMode(UI32_T port, UI32_T mode)
{
    UI32_T phy_port; 
    BOOL_T ret = FALSE;
    UI32_T classification_type;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    /* Force High-Power should bypass classification
     * (suggested from Broadcom)
     */
    if (mode == 0)
    {
        classification_type = POEDRV_PORT_CLASSIFICATION_DOT3AF;
    }
    else if (mode == 1)
    {
        classification_type = POEDRV_PORT_CLASSIFICATION_NONE;
    }
    else
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_force_high_power_mode, ret,
        phy_port, mode);

    if (ret == TRUE)
    {
        POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_classification_type, ret,
            phy_port, classification_type);
    }

    POEDRV_LEAVE_CRITICAL_SECTION;


    return ret;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME - POEDRV_SetPortDot3atHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (For DLL)
 * INPUT    : lport
 *            mode : 1 - Switch the Port from 802.3af to High Power Mode
 *                   0 - Ignore the Port switch request
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : exclusive with POEDRV_SetPortForceHighPowerMode()
 *-------------------------------------------------------------------------
 */
BOOL_T POEDRV_SetPortDot3atHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*  mref_handle_p;
	POEDRV_Rx_IscBuf_T*  isc_buf_p;
    UI32_T               pdu_len;
    UI8_T                my_unit;
#endif

    if (POEDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
    /* EH not implemented yet
     */
        return FALSE;
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)

        POEDRV_OM_GetMainPowerInfoUnitID(&my_unit);

        if ( my_unit != unit )
        {
            if (STKTPLG_POM_IsPoeDevice(unit))
            {
                mref_handle_p = L_MM_AllocateTxBuffer(sizeof(POEDRV_Rx_IscBuf_T), /* tx_buffer_size */
                                                      L_MM_USER_ID2(SYS_MODULE_POEDRV, POEDRV_Set_Port_Dot3at_High_Power_Mode)/* user_id */);
                isc_buf_p = (POEDRV_Rx_IscBuf_T*)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

                /* set remote unit port Power Detection Controlt
                 */
                isc_buf_p->serviceID = POEDRV_Set_Port_Dot3at_High_Power_Mode;
                isc_buf_p->port = port;
                isc_buf_p->info.dot3at_high_power_mode = mode;

                if (!ISC_SendMcastReliable(unit, ISC_POEDRV_SID, mref_handle_p,
                                      SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                      POEDRV_RETRY_TIMES , POEDRV_TIME_OUT, FALSE))
                {
                    /* EH not implemented yet
                     */
                    return FALSE; /* can't config remote */
                }
			}
			else
			{
DBG_PRINT();
			    printf("\r\nUnit %lu don't support PoE!!", unit);
                return FALSE;
			}
       	}
        else
        {
#endif /* SYS_CPNT_STACKING */
    	    /* if local unit or standalone
    	     */
	        if (!POEDRV_LocalSetPortDot3atHighPowerMode(port, mode))
	        {
    	        /* EH not implemented yet
    	         */
	            return FALSE;
	        }
#if (SYS_CPNT_STACKING == TRUE)
        }
#endif /*SYS_CPNT_STACKING*/
	}
    return TRUE;

} /* End of POEDRV_SetPortAdminStatus() */

static BOOL_T POEDRV_LocalSetPortDot3atHighPowerMode(UI32_T port, UI32_T mode)
{
    UI32_T phy_port; 
    BOOL_T ret = FALSE;

    /* Mapping logical port ID to physical port ID
     */
    if ( POEDRV_OM_Logical2PhyDevicePortID(port, &phy_port) == FALSE)
    {
        return FALSE;
    }

    if (mode !=0 && mode!=1)
    {
        return FALSE;
    }

    POEDRV_ENTER_CRITICAL_SECTION;

    POEDRV_EXEC(dynamic_hooked_poe->poedrv_set_port_dot3at_high_power_mode, ret,
        phy_port, mode);

    POEDRV_LEAVE_CRITICAL_SECTION;


    return ret;

}

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
#if (SYS_CPNT_STACKING == TRUE)
#if 0

/* -------------------------------------------------------------------------
 * Function : POEDRV_Service_Callback
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulte all of POEDRV via ISC
 * INPUT    : *key      -- key of ISC
 *            *mem_ref  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : callbacked by ISC
 * -------------------------------------------------------------------------
 */
BOOL_T POEDRV_Service_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref)
{
    UI32_T	service;
    UI32_T  pdu_len;
	UI8_T  *buf = L_MM_Mref_GetPdu(mem_ref,&pdu_len);

    service = ((POEDRV_Rx_IscBuf_T *)buf)->serviceID;

    POEDRV_func_tab[service](key,mem_ref);

    L_MM_Mref_Release(&mem_ref);

    return TRUE;
}
#endif

BOOL_T POEDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p)
{
    POEDRV_Rx_IscBuf_T   *buf;
    UI32_T  service_id, pdu_len, ret;

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
        ret = POEDRV_func_tab[service_id](key,buf);
    L_MM_Mref_Release(&mref_handle_p);
DBG_PRINT();

    return TRUE;
} /* POE_ISC_Handler() */
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

void POEDRV_HotSwapInsert(void)
{
    POEDRV_UpdateStackingInfo();
}

void POEDRV_HotSwapremove(void)
{
    POEDRV_UpdateStackingInfo(); 
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
DBG_PRINT("My Unit: %lu",value);
    POEDRV_OM_SetMyUnitID(value);
	POEDRV_OM_SetMainPowerInfoUnitID(((UI8_T) value));
    STKTPLG_POM_GetNumberOfUnit(&value);
DBG_PRINT("Total %lu Units", value);
    POEDRV_OM_SetNumOfUnits(value);
#endif

}


