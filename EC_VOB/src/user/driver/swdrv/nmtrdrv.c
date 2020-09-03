/*------------------------------------------------------------------------------
 * Module Name: nmtrdrv.C
 *------------------------------------------------------------------------------
 * Purpose    : This file is the driver of the network monitor,
 *              provides APIs to manipulate the counters of specific device.
 * Notes      :
 *                  07/02/2002 arthur -- remove callback function
 *
 * Copyright(C)      Accton Corporation, 2001
 *-----------------------------------------------------------------------------
 *                                                          2001/07/23 arthur
 *-----------------------------------------------------------------------------*/

/*------------------------ --
 * INCLUDE FILE DECLARATIONS
 *---------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "sysfun.h"
#include "nmtrdrv.h"
#include "nmtrdrv_om.h"
#include "l_stdlib.h"
#include "stktplg_pom.h"
#include "backdoor_mgr.h"
#include "dev_nmtrdrv_pmgr.h"
#include "sys_callback_mgr.h"


#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#include "l_mm.h"
#endif /*SYS_CPNT_STACKING*/

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/*-----------------
 * LOCAL CONSTANTS
 *-----------------*/
#define NMTRDRV_MASTER_UNIT             1
#define NMTRDRV_RETRY_COUNT             1
#define NMTRDRV_TIMEOUT                 500
#define ALL_DEVICE                      -1
#define NMTRDRV_DEBUG_FLAG_DUMP_TXPKT   0x04
#define NMTRDRV_DEBUG_FLAG_DUMP_RXPKT   0x08
#define NMTRDRV_OPTION_MODULE           255
#define NMTRDRV_POOL_ID                 0
#define NMTRDRV_TRACE_ID                0
#define NMTRDRV_MAX_PORT_NUM_OF_IFSTATS_IN_ONE_PACKET           (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_IfTableStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_IFXSTATS_IN_ONE_PACKET          (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_IfXTableStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_RMONSTATS_IN_ONE_PACKET         (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_RmonStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKESTATS_IN_ONE_PACKET    (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_EtherlikeStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKEPAUSE_IN_ONE_PACKET    (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_EtherlikePause_T))
#define NMTRDRV_MAX_PORT_NUM_OF_IFPERQSTATS_IN_ONE_PACKET       (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_IfPerQStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_PFCSTATS_IN_ONE_PACKET          (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_PfcStats_T))
#define NMTRDRV_MAX_PORT_NUM_OF_QCNSTATS_IN_ONE_PACKET          (SYS_ADPT_ISC_MAX_PDU_LEN/sizeof(SWDRV_QcnStats_T))

#define NMTRDRV_UNIT_TO_UNITBMP(unit) ((0x01) << (unit-1))

/*-------------
 * LOCAL TYPES
 *-------------*/

#if (SYS_CPNT_STACKING == TRUE)
typedef struct
{
    UI16_T  service_id;
    UI16_T  unit;
    UI16_T  start_port;
    UI16_T  updated_port_num;
    union
    {
        SWDRV_IfTableStats_T    if_stats[NMTRDRV_MAX_PORT_NUM_OF_IFSTATS_IN_ONE_PACKET];
        SWDRV_IfXTableStats_T   ifx_stats[NMTRDRV_MAX_PORT_NUM_OF_IFXSTATS_IN_ONE_PACKET];
        SWDRV_RmonStats_T       rmon_stats[NMTRDRV_MAX_PORT_NUM_OF_RMONSTATS_IN_ONE_PACKET];
        SWDRV_EtherlikeStats_T  etherlike_stats[NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKESTATS_IN_ONE_PACKET];
        SWDRV_EtherlikePause_T  etherlike_pause[NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKEPAUSE_IN_ONE_PACKET];
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        SWDRV_IfPerQStats_T     ifperq_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_PFC == TRUE)
        SWDRV_PfcStats_T        pfc_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
#if (SYS_CPNT_CN == TRUE)
        SWDRV_QcnStats_T        qcn_stats[SYS_ADPT_TOTAL_NBR_OF_LPORT];
#endif
    }__attribute__((packed, aligned(1)))data;
}__attribute__((packed, aligned(1)))NMTRDRV_Request_Packet_T;

typedef enum
{
    NMTRDRV_CLEAR_PORT_COUNTER      = 0,
    NMTRDRV_CLEAR_ALL_COUNTERS,
    NMTRDRV_UPDATE_PORT_IF_TABLE_STATS,
    NMTRDRV_UPDATE_PORT_IFX_TABLE_STATS,
    NMTRDRV_UPDATE_PORT_RMON_STATS     ,
    NMTRDRV_UPDATE_PORT_ETHERLIKE_STATS,
    NMTRDRV_SET_PROVISION_COMPLETE,
    NMTRDRV_UPDATE_PORT_ETHERLIKE_PAUSE_STATS,
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    NMTRDRV_UPDATE_PORT_IFPERQ_STATS,
#endif
#if (SYS_CPNT_PFC == TRUE)
    NMTRDRV_UPDATE_PORT_PFC_STATS,
#endif
#if (SYS_CPNT_CN == TRUE)
    NMTRDRV_UPDATE_PORT_QCN_STATS,
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
    NMTRDRV_CLEAR_VLAN_COUNTER,
    NMTRDRV_UPDATE_VLAN_IFX_TABLE_STATS,
#endif
    NMTRDRV_MAX_NUM_OF_SERVICE
}NMTRDRV_Services_ID_T;

typedef BOOL_T (*NMTRDRV_Remote_Function_T)();

#endif /* SYS_CPNT_STACKING */

enum NMTRDRV_TASK_EVENT_E
{
    NMTRDRV_TASK_EVENT_NONE                =   0x0000L,
    NMTRDRV_TASK_EVENT_PERIODIC            =   0x0001L,
    NMTRDRV_TASK_EVENT_ENTER_TRANSITION    =   0x0002L,
};


/*-------------------
 * Local SUBROUTINES
 *-------------------*/
static void NMTRDRV_TASK_Main(void);
static BOOL_T NMTRDRV_Notify_300Utilization(void);
static void   NMTRDRV_GetLocalIfTableStatsNUpdateMasterOM(void);
static void   NMTRDRV_GetLocalIfXTableStatsNUpdateMasterOM(void);
static void   NMTRDRV_GetLocalEtherLikeStatsNUpdateMasterOM(void);
static void   NMTRDRV_GetLocalEtherLikePauseNUpdateMasterOM(void);
static void   NMTRDRV_GetLocalRmonStatsNUpdateMasterOM(void);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void   NMTRDRV_GetLocalIfPerQStatsNUpdateMasterOM(void);
#endif
#if (SYS_CPNT_PFC == TRUE)
static void   NMTRDRV_GetLocalPfcStatsNUpdateMasterOM(void);
#endif
#if (SYS_CPNT_CN == TRUE)
static void   NMTRDRV_GetLocalQcnStatsNUpdateMasterOM(void);
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static void   NMTRDRV_GetLocalVlanIfXTableStatsNUpdateMasterOM(void);
#endif
#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T NMTRDRV_Notify_IfTableStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
static BOOL_T NMTRDRV_Notify_IfXTableStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
static BOOL_T NMTRDRV_Notify_RmonStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
static BOOL_T NMTRDRV_Notify_EtherLikeStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
static BOOL_T NMTRDRV_Notify_EtherLikePause(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static BOOL_T NMTRDRV_Notify_IfPerQStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
#endif
#if (SYS_CPNT_PFC == TRUE)
static BOOL_T NMTRDRV_Notify_PfcStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
#endif
#if (SYS_CPNT_CN == TRUE)
static BOOL_T NMTRDRV_Notify_QcnStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num);
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static BOOL_T NMTRDRV_Notify_IfXTableStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T updated_num);
#endif
static void NMTRDRV_SendStates2MasterViaISC(UI32_T service_id,
                                            UI32_T start_port,
                                            UI32_T num_of_port,
                                            UI32_T max_port_num_in_one_isc_packet,
                                            UI32_T one_port_counter_size,
                                            void   *local_table);
/* Function to dispatch packets received from master to correct service routine */
static BOOL_T NMTRDRV_Remote_Service_Demux(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id);

/* Service routines to handle different requests issued from master unit */
static BOOL_T NMTRDRV_Remote_ClearPortCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_ClearAllCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateIfTableStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateIfXTableStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateRmonStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateEtherLikeStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateEtherLikePause(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static BOOL_T NMTRDRV_Remote_UpdateIfPerQStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
#endif
#if (SYS_CPNT_PFC == TRUE)
static BOOL_T NMTRDRV_Remote_UpdatePfcStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
#endif
#if (SYS_CPNT_CN == TRUE)
static BOOL_T NMTRDRV_Remote_UpdateQcnStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static BOOL_T NMTRDRV_Remote_ClearVlanCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
static BOOL_T NMTRDRV_Remote_UpdateIfXTableStatsForVlan(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);
#endif
static BOOL_T NMTRDRV_Remote_SetProvisionComplete(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p);

#endif /*SYS_CPNT_STACKING*/

static void     NMTRDRV_Backdoor_Menu(void);
static BOOL_T   NMTRDRV_GetUnitPort(UI32_T *unit, UI32_T *port);
static BOOL_T   NMTRDRV_GetPort(UI32_T *port);
static void     NMTRDRV_Backdoor_GetIfInfo(SWDRV_IfTableStats_T *stat, UI32_T debug_unit, UI32_T debug_port);
static void     NMTRDRV_Backdoor_GetIfXInfo(SWDRV_IfXTableStats_T *stat);
static void     NMTRDRV_Backdoor_GetRmonInfo(SWDRV_RmonStats_T *stat);
static void     NMTRDRV_Backdoor_GetEtherLikeInfo(SWDRV_EtherlikeStats_T *stat);
static void     NMTRDRV_Backdoor_GetEtherLikePauseInfo(SWDRV_EtherlikePause_T *stat);
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void     NMTRDRV_Backdoor_GetIfPerQInfo(SWDRV_IfPerQStats_T *stat, UI32_T debug_unit, UI32_T debug_port);
#endif
#if (SYS_CPNT_PFC == TRUE)
static void     NMTRDRV_Backdoor_GetPfcInfo(SWDRV_PfcStats_T *stat, UI32_T debug_unit, UI32_T debug_port);
#endif
#if (SYS_CPNT_CN == TRUE)
static void     NMTRDRV_Backdoor_GetQcnInfo(SWDRV_QcnStats_T *stat, UI32_T debug_unit, UI32_T debug_port);
#endif

/*----------------------------
 * STATIC VARIABLES
 *----------------------------*/

#if (SYS_CPNT_STACKING == TRUE)
static NMTRDRV_Remote_Function_T NMTRDRV_remote_service_table[] =
{
    NMTRDRV_Remote_ClearPortCounter,
    NMTRDRV_Remote_ClearAllCounter,
    NMTRDRV_Remote_UpdateIfTableStats,
    NMTRDRV_Remote_UpdateIfXTableStats,
    NMTRDRV_Remote_UpdateRmonStats,
    NMTRDRV_Remote_UpdateEtherLikeStats,
    NMTRDRV_Remote_SetProvisionComplete,
    NMTRDRV_Remote_UpdateEtherLikePause,
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    NMTRDRV_Remote_UpdateIfPerQStats,
#endif
#if (SYS_CPNT_PFC == TRUE)
    NMTRDRV_Remote_UpdatePfcStats,
#endif
#if (SYS_CPNT_CN == TRUE)
    NMTRDRV_Remote_UpdateQcnStats,
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
    NMTRDRV_Remote_ClearVlanCounter,
    NMTRDRV_Remote_UpdateIfXTableStatsForVlan,
#endif
};

#endif /*SYS_CPNT_STACKING*/

//static BOOL_T  nmtrdrv_task_is_transition_done;

/*-----------------
 * MACOR FUNCTIONS
 *-----------------*/

#define NMTRDRV_DEBUG_MSG(x) if (NMTRDRV_OM_GetDebugFlag() == TRUE ) {printf("\r\n %s(): %s", __FUNCTION__, (x));}

/*----------------------------
 * EXPORTED SUBPROGRAM BODIES
 *----------------------------*/

/*------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_TASK_CreateTask
 *------------------------------------------------------------------------
 * FUNCTION: This function will create address management task
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTRDRV_TASK_CreateTask(void)
{
    UI32_T  task_id;

    if(SYSFUN_SpawnThread(SYS_BLD_NMTRDRV_THREAD_PRIORITY,
                          SYS_BLD_NMTRDRV_THREAD_SCHED_POLICY,
                          SYS_BLD_NMTRDRV_THREAD,
                          SYS_BLD_NMTRDRV_TASK_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          NMTRDRV_TASK_Main,
                          NULL,
                          &task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_NMTRDRV, task_id, SYS_ADPT_NMTRDRV_SW_WATCHDOG_TIMER);
#endif

    return;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_InitiateSystemResources
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTRDRV_InitiateSystemResources(void)
{
    //memset(&stack_info, 0, sizeof(stack_info));
    NMTRDRV_OM_InitiateSystemResources();
    return;
}

void NMTRDRV_AttachSystemResources(void)
{
    NMTRDRV_OM_AttachSystemResources();
}

void NMTRDRV_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    NMTRDRV_OM_GetShMemInfo(segid_p, seglen_p);
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_Create_InterCSC_Relation
 *------------------------------------------------------------------------|
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTRDRV_Create_InterCSC_Relation(void)
{
#if (SYS_CPNT_STACKING == TRUE)
    ISC_Register_Service_CallBack(ISC_NMTRDRV_SID, NMTRDRV_Remote_Service_Demux);
#endif /*SYS_CPNT_STACKING*/
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("nmtrdrv",
                                                      SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
                                                      NMTRDRV_Backdoor_Menu);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initialize data variables for transition mode operation
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterTransitionMode(void)
{
    //SYSFUN_TASK_ENTER_TRANSITION_MODE(nmtrdrv_task_is_transition_done);
    NMTRDRV_OM_EnterTransitionMode();
    NMTRDRV_ClearAllCounters();
    return;
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void NMTRDRV_SetTransitionMode(void)
{
    NMTRDRV_OM_SetTransitionMode();
    NMTRDRV_OM_SetProvisionComplete(FALSE);
    //nmtrdrv_task_is_transition_done = FALSE;
	//SYSFUN_PeriodicTimer_Stop(NMTRDRV_OM_GetTimerId());
    //SYSFUN_SendEvent (NMTRDRV_OM_GetTaskId(), NMTRDRV_TASK_EVENT_ENTER_TRANSITION);
}	/*	end of NMTRDRV_SetTransitionMode	*/

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterMasterMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initial data variables for master mode operation
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterMasterMode(void)
{
    //UI32_T timer_ticks;
    //UI32_T units_in_stack;
    UI32_T my_unit_id;

    STKTPLG_POM_GetMyUnitID(&my_unit_id);
    NMTRDRV_OM_SetMyUnitId(my_unit_id);
    //STKTPLG_POM_GetNumberOfUnit(&units_in_stack);
    //timer_ticks = SYS_BLD_NMTR_UPDATE_STATISTICS_TICKS * units_in_stack;
    /* Set operating mode = master mode first, then start periodic timer
      * If NMTRDRV starts timer firest, task will receive timer event,
      * and stop timer event soon because of transition mode.
      */
    //SYSFUN_PeriodicTimer_Start(NMTRDRV_OM_GetTimerId(), timer_ticks, NMTRDRV_TASK_EVENT_PERIODIC);
    NMTRDRV_OM_EnterMasterMode();
	return;
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterSlaveMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initial data variables for slave mode operation
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterSlaveMode(void)
{
    //UI32_T timer_ticks;
    //UI32_T units_in_stack;
    UI32_T my_unit_id;

    /* set mgr in slave mode */
    STKTPLG_POM_GetMyUnitID(&my_unit_id);
    NMTRDRV_OM_SetMyUnitId(my_unit_id);
    //STKTPLG_POM_GetNumberOfUnit(&units_in_stack);
    //timer_ticks = SYS_BLD_NMTR_UPDATE_STATISTICS_TICKS * units_in_stack;
    //SYSFUN_PeriodicTimer_Start(NMTRDRV_OM_GetTimerId(), timer_ticks, NMTRDRV_TASK_EVENT_PERIODIC);
    NMTRDRV_OM_EnterSlaveMode();
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTRDRV_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Net monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTRDRV_ProvisionComplete(void)
{
    UI32_T                      pdu_len;
    /* BODY
     */
    //SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();
    /* Local Operation */
    NMTRDRV_OM_SetProvisionComplete(TRUE);

#if(SYS_CPNT_STACKING == TRUE)
    if(NMTRDRV_OM_GetOperatingMode()== SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI32_T             unit;
        UI16_T             unit_bmp=0;
        UI16_T             return_unit_bmp=0;

        for (unit = 0; STKTPLG_POM_GetNextDriverUnit(&unit); )
        {
            if (unit == NMTRDRV_OM_GetMyUnitId())
            {
                continue;
            }
            unit_bmp |= NMTRDRV_UNIT_TO_UNITBMP(unit);
        }

        if( unit_bmp != 0)
        {
            L_MM_Mref_Handle_T*         mref_handle_p;
            NMTRDRV_Request_Packet_T*   isc_buffer_p;
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
                	return;
            }
            isc_buffer_p->service_id = NMTRDRV_SET_PROVISION_COMPLETE;

            return_unit_bmp = ISC_SendMcastReliable(unit_bmp,ISC_NMTRDRV_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE);

            if(return_unit_bmp !=0)
            {
                NMTRDRV_DEBUG_MSG("Set provision complete to Slave fail.");
            }
        }
    }
#endif /* SYS_CPNT_STACKING */
    //SYSFUN_RELEASE_CSC();
    return;
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearPortCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T port    -   port number
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearPortCounter(UI32_T unit, UI32_T port)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    NMTRDRV_Request_Packet_T*   isc_buffer_p;
    UI32_T                      pdu_len;
    UI16_T  unit_bmp;
    UI16_T  return_unit_bmp;
#endif
    UI32_T  max_port_number, drv_unit;

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    /* Determine the existance of the specified port */
    if(!STKTPLG_POM_PortExist(unit, port))
    {
        NMTRDRV_DEBUG_MSG("port not exist.");
        //SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    /* Remote operation: remote call can only be carried out in master mode */
    if(unit != NMTRDRV_OM_GetMyUnitId())
    {
#if (SYS_CPNT_STACKING == TRUE)
        /* return false in slave mode */
        if(NMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            NMTRDRV_DEBUG_MSG("Not Master.");
        }
        /* send ISC remote request in master mode */
        else
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                              L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
                	return FALSE;
            }
            /* fill in the data needed in the packet header
             */
            isc_buffer_p->service_id   = NMTRDRV_CLEAR_PORT_COUNTER;
            isc_buffer_p->start_port= port;
            unit_bmp = NMTRDRV_UNIT_TO_UNITBMP(unit);
            return_unit_bmp=ISC_SendMcastReliable(unit_bmp,ISC_NMTRDRV_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE);
            if(return_unit_bmp!=0)
            {
                NMTRDRV_DEBUG_MSG("ISC fail.");
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
#else
        NMTRDRV_DEBUG_MSG("remote units not exist.");
#endif /*SYS_CPNT_STACKING*/
    }
    /* Local operation */
    else
    {

        if (STKTPLG_POM_GetMaxPortNumberOnBoard(unit, &max_port_number) == FALSE)
        {
            //SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        if (port > max_port_number)
        {
            if (STKTPLG_POM_OptionModuleIsExist(unit, &drv_unit) == TRUE)
            {
#if (SYS_CPNT_STACKING == TRUE)
                unit_bmp = NMTRDRV_UNIT_TO_UNITBMP(drv_unit);
                if (unit_bmp!=0)
                {
                    mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                                      L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);
                    isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

                    if (isc_buffer_p==NULL)
                    {
                        	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
                        	return FALSE;
                    }
                    /* fill in the data needed in the packet header  */
                    isc_buffer_p->service_id   = NMTRDRV_CLEAR_PORT_COUNTER;
                    isc_buffer_p->start_port= port;

                    return_unit_bmp=ISC_SendMcastReliable(unit_bmp,ISC_NMTRDRV_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE);
                    if(return_unit_bmp!=0)
                    {
                        NMTRDRV_DEBUG_MSG("ISC fail.");
                        //SYSFUN_RELEASE_CSC();
                        return FALSE;
                    }
                }
#endif
            }
        }
        else
        {

            if (FALSE == DEV_NMTRDRV_PMGR_ClearPortCounter(unit, port))
            {
                NMTRDRV_DEBUG_MSG("Set ASIC fail.");
            }
            else
            {
                NMTRDRV_DEBUG_MSG("Set ASIC success.");
                //SYSFUN_RELEASE_CSC();
                return TRUE;
            }
        }
    }
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end NMTRDRV_ClearPortCounter() */


#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearVlanCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the vlan conuter
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T vid     -   vid
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearVlanCounter(UI32_T unit, UI32_T vid)
{
#if (SYS_CPNT_STACKING == TRUE)
    L_MM_Mref_Handle_T*         mref_handle_p;
    NMTRDRV_Request_Packet_T*   isc_buffer_p;
    UI32_T                      pdu_len;
    UI16_T  unit_bmp;
    UI16_T  return_unit_bmp;
#endif

    /* BODY
     */
    //SYSFUN_USE_CSC(FALSE);

    /* Remote operation: remote call can only be carried out in master mode */
    if(unit != NMTRDRV_OM_GetMyUnitId())
    {
#if (SYS_CPNT_STACKING == TRUE)
        /* return false in slave mode */
        if(NMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            NMTRDRV_DEBUG_MSG("Not Master.");
        }
        /* send ISC remote request in master mode */
        else
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                              L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
                	return FALSE;
            }
            /* fill in the data needed in the packet header
             */
            isc_buffer_p->service_id   = NMTRDRV_CLEAR_VLAN_COUNTER;
            isc_buffer_p->start_port = vid;
            unit_bmp = NMTRDRV_UNIT_TO_UNITBMP(unit);
            return_unit_bmp=ISC_SendMcastReliable(unit_bmp,ISC_NMTRDRV_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE);
            if(return_unit_bmp!=0)
            {
                NMTRDRV_DEBUG_MSG("ISC fail.");
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
#else
        NMTRDRV_DEBUG_MSG("remote units not exist.");
#endif /*SYS_CPNT_STACKING*/
    }
    /* Local operation */
    else
    {
        if (FALSE == DEV_NMTRDRV_PMGR_ClearVlanCounter(vid))
        {
            NMTRDRV_DEBUG_MSG("Set ASIC fail.");
        }
        else
        {
            NMTRDRV_DEBUG_MSG("Set ASIC success.");
            //SYSFUN_RELEASE_CSC();
            return TRUE;
        }
    }
    //SYSFUN_RELEASE_CSC();
    return FALSE;
} /* end NMTRDRV_ClearVlanCounter() */
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear all of counters in whole system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearAllCounters(void)
{
    /* BODY  */
    /*SYSFUN_USE_CSC(FALSE);*/

    /* Clear local counters: both in master and slave mode */
    if (FALSE == DEV_NMTRDRV_PMGR_ClearAllCounters())
    {
        NMTRDRV_DEBUG_MSG("Set ASIC fail.");
        /*SYSFUN_RELEASE_CSC();*/
        return FALSE;
    }

#if (SYS_CPNT_STACKING == TRUE)
    if(NMTRDRV_OM_GetOperatingMode() == SYS_TYPE_STACKING_MASTER_MODE)
    {
        UI32_T unit;
        UI16_T all_drvier_unit_bmp = 0;
        UI16_T return_unit_bmp;
        L_MM_Mref_Handle_T*         mref_handle_p;
        NMTRDRV_Request_Packet_T*   isc_buffer_p;
        UI32_T                      pdu_len;

        for (unit = 0; STKTPLG_POM_GetNextDriverUnit(&unit); )
        {
            if (unit == NMTRDRV_OM_GetMyUnitId())
            {
                continue;
            }
            all_drvier_unit_bmp |= NMTRDRV_UNIT_TO_UNITBMP(unit);
        }
        if (all_drvier_unit_bmp!=0)
        {
            mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);
            isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

            if (isc_buffer_p==NULL)
            {
                	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
                	return FALSE;
            }
            /* fill in the data needed in the packet header
             */
            isc_buffer_p->service_id    = NMTRDRV_CLEAR_ALL_COUNTERS;
            isc_buffer_p->start_port          = 0;
            return_unit_bmp=ISC_SendMcastReliable(all_drvier_unit_bmp,ISC_NMTRDRV_SID,
                                                 mref_handle_p,
                                                 SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                                 NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE);
            if (return_unit_bmp!=0)
            {
                NMTRDRV_DEBUG_MSG("ISC fail.");
                //SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    } /* if */
#endif /* SYS_CPNT_STACKING */

    NMTRDRV_DEBUG_MSG("Set success.");
    //SYSFUN_RELEASE_CSC();
    return TRUE;
} /* end NMTRDRV_ClearAllCounters() */

/* Local SubRoutine ***************************************************************/

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTR_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION:  This function will update port statistics periodically
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------*/
static void NMTRDRV_TASK_Main(void)
{
    UI32_T  nmtrdrv_task_event_var;
    UI32_T  nmtrdrv_task_rcv_event;
    UI32_T  count = 0;
    void*   timer_id;
    UI32_T  timer_ticks;
    UI32_T  units_in_stack = 1;

    timer_id = SYSFUN_PeriodicTimer_Create();
     //STKTPLG_POM_GetNumberOfUnit(&units_in_stack);
    timer_ticks = SYS_BLD_NMTR_UPDATE_STATISTICS_TICKS * units_in_stack;
    SYSFUN_PeriodicTimer_Start(timer_id, timer_ticks, NMTRDRV_TASK_EVENT_PERIODIC);

    nmtrdrv_task_event_var = NMTRDRV_TASK_EVENT_NONE;

    while(1)
    {
    	SYSFUN_ReceiveEvent((NMTRDRV_TASK_EVENT_ENTER_TRANSITION|NMTRDRV_TASK_EVENT_PERIODIC
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                            |SYSFUN_SYSTEM_EVENT_SW_WATCHDOG
#endif
                            ),
    	                     SYSFUN_EVENT_WAIT_ANY,
                             (nmtrdrv_task_event_var == NMTRDRV_TASK_EVENT_NONE)?SYSFUN_TIMEOUT_WAIT_FOREVER: SYSFUN_TIMEOUT_NOWAIT,
                             &nmtrdrv_task_rcv_event);

        nmtrdrv_task_event_var |= nmtrdrv_task_rcv_event;

        /* Get operation mode from MGR
         */
        if (SYS_TYPE_STACKING_TRANSITION_MODE == NMTRDRV_OM_GetOperatingMode())
        {
#if 0 //kh_shi
            if (nmtrdrv_task_event_var & NMTRDRV_TASK_EVENT_ENTER_TRANSITION)
            {
                nmtrdrv_task_is_transition_done = TRUE;
            }
#endif
            nmtrdrv_task_event_var = NMTRDRV_TASK_EVENT_NONE;
            continue;
    	 }

        if  (nmtrdrv_task_event_var & NMTRDRV_TASK_EVENT_PERIODIC)
        {
            if ((FALSE == NMTRDRV_OM_GetProvisionComplete())
#if 0
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            ||(FALSE == STKTPLG_POM_ENG_IsProvisionCompleted())
#endif
#endif
            )
            {
        	    /* NMTR task don't do any thing when provision has not been completed
        	     * even this unit in stacking master mode
        	     */
                nmtrdrv_task_event_var &= ~NMTRDRV_TASK_EVENT_PERIODIC;
                continue;
            }

            NMTRDRV_GetLocalRmonStatsNUpdateMasterOM();

#if (SYS_CPNT_ATC_STORM == TRUE || SYS_CPNT_NMTR_HISTORY == TRUE)
            /* for ATC, it must monitor counter every 1sec,
             * so update ifxtable here.
             */
            NMTRDRV_GetLocalIfXTableStatsNUpdateMasterOM();
#endif
#if (SYS_CPNT_NMTR_HISTORY == TRUE)
            NMTRDRV_GetLocalIfTableStatsNUpdateMasterOM();
#endif

            switch(count%3)
            {
                case 0:
                    /* Update ifx table statistics first
                     * Because when update if table statistics, NMTR_MGR will count
                     * nmtr_mgr_utilization_stats.ifInMulticastPkts_utilization and
                     * nmtr_mgr_utilization_stats.ifInBroadcastPkts_utilization.
                     * These two values will be updated when if table update.
                     */
#if (SYS_CPNT_ATC_STORM != TRUE && SYS_CPNT_NMTR_HISTORY != TRUE)
                    NMTRDRV_GetLocalIfXTableStatsNUpdateMasterOM();
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
                    NMTRDRV_GetLocalVlanIfXTableStatsNUpdateMasterOM();
#endif
                    break;

                case 1:
#if (SYS_CPNT_NMTR_HISTORY != TRUE)
                    NMTRDRV_GetLocalIfTableStatsNUpdateMasterOM();
#endif
                    /* After update IF table, nmtrdrv should notify NMTR_MGR to update 300s_utilization counters
                     */
                    NMTRDRV_Notify_300Utilization();

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
                    NMTRDRV_GetLocalIfPerQStatsNUpdateMasterOM();
#endif
#if (SYS_CPNT_PFC == TRUE)
                    NMTRDRV_GetLocalPfcStatsNUpdateMasterOM();
#endif
#if (SYS_CPNT_CN == TRUE)
                    NMTRDRV_GetLocalQcnStatsNUpdateMasterOM();
#endif
                    break;

                case 2:
                    NMTRDRV_GetLocalEtherLikeStatsNUpdateMasterOM();
                    NMTRDRV_GetLocalEtherLikePauseNUpdateMasterOM();
                    break;

                default:
                    break;
            }
            count++;
            nmtrdrv_task_event_var &= ~NMTRDRV_TASK_EVENT_PERIODIC;
        } /* end of if  (nmtrdrv_task_event_var & NMTRDRV_TASK_EVENT_PERIODIC) */

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if(nmtrdrv_task_event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
       	    SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_NMTRDRV);
            nmtrdrv_task_event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

    }
} /* end NMTR_Task_Main */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_IfTableStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify IfTableStats to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 *          static SWDRV_IfTableStats_T if_stats[]
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_IfTableStats_T if_stats[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_IfTableStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_IFTABLE_STATS,unit, start_port, updated_port_num);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_IfXTableStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify IfXTableStats to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 *          static SWDRV_IfXTableStats_T ifX_stats[]
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_IfXTableStats_T ifX_stats[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_IfXTableStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_IFXTABLE_STATS,unit, start_port, updated_port_num);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_RmonStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify RmonStats to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 *          static SWDRV_RmonStats_T rmon_stats[]
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_RmonStats_T rmon_stats[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_RmonStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_RMON_STATS,unit,start_port, updated_port_num);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_EtherLikeStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify RmonStats to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 *          static SWDRV_EtherlikeStats_T ether_like_stats[]
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_EtherlikeStats_T ether_like_stats[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_EtherLikeStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_ETHERLIKE_STATS,unit, start_port, updated_port_num);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_EtherLikePause
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify ether-like pause status to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_EtherlikePause_T ether_like_pause[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_EtherLikePause(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_ETHERLIKE_PAUSE_STATS, unit, start_port, updated_port_num);
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_300Utilization
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify nmtr_mgr to update 300s_utilization counters .
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's NMTR_MGR_Utilization_300_SECS_T	           nmtr_mgr_utilization_300_secs[1~SYS_ADPT_TOTAL_NBR_OF_LPORT][NMTR_MGR_SLICE_NUMBER]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_300Utilization(void)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_300S_UTILIZATION,NMTRDRV_OM_GetMyUnitId(), 0, 0);
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_IfPerQStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify CoS queue statistics to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : None
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_IfPerQStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{
    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_IFPERQ_STATS,unit, start_port, updated_port_num);
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_PfcStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify PFC statistics to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : None
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_PfcStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{
    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_PFC_STATS,unit, start_port, updated_port_num);
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_QcnStats
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify QCN statistics to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_port
 *          UI32_T updated_port_num
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : None
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_QcnStats(UI32_T unit, UI32_T start_port, UI32_T updated_port_num)
{
    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_QCN_STATS,unit, start_port, updated_port_num);
}
#endif /* (SYS_CPNT_CN  == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: NMTRDRV_Notify_IfXTableStatsForVlan
 *------------------------------------------------------------------------------
 * PURPOSE: This function will notify IfXTableStats to the register.
 * INPUT  : UI32_T unit
 *          UI32_T start_vid
 *          UI32_T updated_num
 *          static SWDRV_IfXTableStats_T ifX_stats[]
 * OUTPUT : None
 * RETURN : TRUE  - success
 *          FALSE - fail
 * NOTES  : update unit's SWDRV_IfXTableStats_T ifX_stats[start_port ~ (start_port+updated_port_num-1)]
 *------------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Notify_IfXTableStatsForVlan(UI32_T unit, UI32_T start_vid, UI32_T updated_num)
{

    if(NMTRDRV_OM_GetOperatingMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }
    return SYS_CALLBACK_MGR_UpdateNmtrdrvStats(SYS_MODULE_NMTRDRV, NMTRDRV_UPDATE_IFXTABLE_STATS_FOR_VLAN, unit, start_vid, updated_num);
}
#endif

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalIfTableStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local IfTable statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalIfTableStatsNUpdateMasterOM(void)
{
	UI32_T unit;
    UI32_T port;
    UI32_T num_of_port;
    SWDRV_IfTableStats_T if_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearIfStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetIfTableStats(unit, port, port, &if_stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetIfStats(unit, port, &if_stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_IfTableStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_IF_TABLE_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_IFSTATS_IN_ONE_PACKET,
                                        sizeof(SWDRV_IfTableStats_T),
                                        (void   *)if_stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }

}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalIfXTableStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local IfXTable statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalIfXTableStatsNUpdateMasterOM(void)
{
	UI32_T unit;
	UI32_T port;
    UI32_T num_of_port;
    SWDRV_IfXTableStats_T ifx_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearIfXStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetIfXTableStats(unit, port, port, &ifx_stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetIfXStats(unit,port, &ifx_stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_IfXTableStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_IFX_TABLE_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_IFXSTATS_IN_ONE_PACKET,
                                        sizeof(SWDRV_IfXTableStats_T),
                                        (void   *)ifx_stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalRmonStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local Rmon statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalRmonStatsNUpdateMasterOM(void)
{
	UI32_T unit;
    UI32_T port;
    UI32_T num_of_port;
    SWDRV_RmonStats_T rmon_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearRmonStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetRmonStats(unit, port, port, &rmon_stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetRmonStats(unit,port, &rmon_stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_RmonStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_RMON_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_RMONSTATS_IN_ONE_PACKET,
                                        sizeof(SWDRV_RmonStats_T),
                                        (void   *)rmon_stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalEtherLikeStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local EhterLike statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalEtherLikeStatsNUpdateMasterOM(void)
{
	UI32_T unit;
	UI32_T port;
    UI32_T num_of_port;
    SWDRV_EtherlikeStats_T etherlike_stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearEtherlikeStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetEtherLikeStats(unit, port, port, &etherlike_stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetEtherlikeStats(unit, port, &etherlike_stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_EtherLikeStats(unit , 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_ETHERLIKE_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKESTATS_IN_ONE_PACKET,
                                        sizeof(SWDRV_EtherlikeStats_T),
                                        (void   *)etherlike_stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }
}

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalEtherLikePauseNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local EhterLike pause statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalEtherLikePauseNUpdateMasterOM(void)
{
	UI32_T unit;
	UI32_T port;
    UI32_T num_of_port;
    SWDRV_EtherlikePause_T etherlike_pause[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearEtherlikePause(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetEtherLikePause(unit, port, port, &etherlike_pause[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetEtherlikePause(unit, port, &etherlike_pause[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_EtherLikePause(unit , 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_ETHERLIKE_PAUSE_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKESTATS_IN_ONE_PACKET,
                                        sizeof(SWDRV_EtherlikePause_T),
                                        (void   *)etherlike_pause);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalIfPerQStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local CoS queue statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalIfPerQStatsNUpdateMasterOM(void)
{
	UI32_T unit;
    UI32_T port;
    UI32_T num_of_port;
    SWDRV_IfPerQStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearIfPerQStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetIfPerQStats(unit, port, port, &stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetIfPerQStats(unit, port, &stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_IfPerQStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_IFPERQ_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_IFPERQSTATS_IN_ONE_PACKET,
                                        sizeof(*stats),
                                        (void   *)stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }

}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalPfcStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local PFC statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalPfcStatsNUpdateMasterOM(void)
{
	UI32_T unit;
    UI32_T port;
    UI32_T num_of_port;
    SWDRV_PfcStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearPfcStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetPfcStats(unit, port, port, &stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetPfcStats(unit, port, &stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_PfcStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_PFC_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_PFCSTATS_IN_ONE_PACKET,
                                        sizeof(*stats),
                                        (void   *)stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }

}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalQcnStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local QCN statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalQcnStatsNUpdateMasterOM(void)
{
	UI32_T unit;
    UI32_T port;
    UI32_T num_of_port;
    SWDRV_QcnStats_T stats[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];

	unit = NMTRDRV_OM_GetMyUnitId();
    STKTPLG_POM_GetLocalMaxPortCapability(&num_of_port);
    for ( port = 1; port <= num_of_port; port++)
    {
#if (SYS_CPNT_MGMT_PORT == TRUE)
        if (SYS_ADPT_MGMT_PORT == port)
        {
            continue;
        }
#endif
        if(!STKTPLG_POM_PortExist(unit, port))
        {
            NMTRDRV_OM_ClearQcnStats(unit,port);
            continue;
        }

        if (FALSE == DEV_NMTRDRV_PMGR_GetQcnStats(unit, port, port, &stats[port-1]))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }
        NMTRDRV_OM_SetQcnStats(unit, port, &stats[port-1]);
    } /* for (port = 1; port <= num_of_port; port++) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_QcnStats(unit, 1, num_of_port);
    }
#if (SYS_CPNT_STACKING == TRUE)
    else if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
    {
        NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_PORT_QCN_STATS,
                                        1,
                                        num_of_port,
                                        NMTRDRV_MAX_PORT_NUM_OF_QCNSTATS_IN_ONE_PACKET,
                                        sizeof(*stats),
                                        (void   *)stats);
    }
#endif
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }

}
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_GetLocalVlanIfXTableStatsNUpdateMasterOM
 *------------------------------------------------------------------------|
 * FUNCTION: This function will get the local IfXTable statistics and notify to master's OM.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
static void NMTRDRV_GetLocalVlanIfXTableStatsNUpdateMasterOM(void)
{
    const UI32_T num_of_stats_buffer = SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD;
    UI32_T unit;
    UI32_T vid;
    UI32_T max_vid;
    UI32_T start_vid, end_vid, updated_num;
    SWDRV_IfXTableStats_T ifx_stats[num_of_stats_buffer];

    unit = NMTRDRV_OM_GetMyUnitId();
    max_vid = SYS_ADPT_MAX_VLAN_ID;

    for (vid = 1; vid <= max_vid; vid++)
    {
        start_vid = vid;
        end_vid = vid + num_of_stats_buffer - 1;
        end_vid = end_vid < max_vid ? end_vid : max_vid;
        updated_num = end_vid - start_vid + 1;

        if (FALSE == DEV_NMTRDRV_PMGR_GetIfXTableStatsForVlan(unit, start_vid, end_vid, ifx_stats))
        {
            NMTRDRV_DEBUG_MSG("Get local table fail.");
            return;
        }

        NMTRDRV_OM_SetIfXStatsForVlan(unit, start_vid, updated_num, ifx_stats);

#if (SYS_CPNT_STACKING == TRUE)
        if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_SLAVE_MODE)/*slave mode*/
        {
            NMTRDRV_SendStates2MasterViaISC(NMTRDRV_UPDATE_VLAN_IFX_TABLE_STATS,
                                            start_vid,
                                            updated_num,
                                            NMTRDRV_MAX_PORT_NUM_OF_IFXSTATS_IN_ONE_PACKET,
                                            sizeof(SWDRV_IfXTableStats_T),
                                            (void   *)ifx_stats);
        }
#endif

        vid = end_vid;

    } /* end of for (vid) */

    if (NMTRDRV_OM_GetOperatingMode()==SYS_TYPE_STACKING_MASTER_MODE)/*master mode*/
    {
        /* In Master mainboard, start_port always is 1
         */
        NMTRDRV_Notify_IfXTableStatsForVlan(unit, 1, max_vid);
    }
    else /* transition mode*/
    {
        NMTRDRV_DEBUG_MSG("transition mode.");
        return;
    }
}
#endif

#if (SYS_CPNT_STACKING == TRUE)
/*----------------------------------------------------------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_SendStates2MasterViaISC
 *----------------------------------------------------------------------------------------------------------------------------|
 * FUNCTION: This function will get the local Rmon statistics and notify to master's OM.
 * INPUT   : UI32_T service_id                      - What service(table) will caller send to Master
 *           UI32_T start_port                      - first port counter information to be sent
 *           UI32_T num_of_port                     - NMTRDRV needs to send how many port counter information to Master
 *           UI32_T max_port_num_in_one_isc_packet  - How many port counter information can send in one isc packet.
 *                                                    Max port number will be different in different counter table sending.
 *           UI32_T one_port_counter_size           - size of counter information of one port.
 *           void   *isc_request_payload            - local_table[] will be copy to which table of isc_request.
 *           void   local_table[]                   - NMTRDRV uses this table to save chip's counter information.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1. This function is only called in Slave mode.
 *           2. service_id                      - NMTRDRV_UPDATE_PORT_IF_TABLE_STATS
 *                                                NMTRDRV_UPDATE_PORT_IFX_TABLE_STATS
 *                                                NMTRDRV_UPDATE_PORT_RMON_STATS
 *                                                NMTRDRV_UPDATE_PORT_ETHERLIKE_STATS
 *                                                NMTRDRV_UPDATE_PORT_ETHERLIKE_PAUSE_STATS
 *              num_of_port                     - ES4649 mainboard = 48; module = 1
 *              max_port_num_in_one_isc_packet  - NMTRDRV_MAX_PORT_NUM_OF_IFSTATS_IN_ONE_PACKET
 *                                                NMTRDRV_MAX_PORT_NUM_OF_IFXSTATS_IN_ONE_PACKET
 *                                                NMTRDRV_MAX_PORT_NUM_OF_RMONSTATS_IN_ONE_PACKET
 *                                                NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKESTATS_IN_ONE_PACKET
 *                                                NMTRDRV_MAX_PORT_NUM_OF_ETHERLIKEPAUSE_IN_ONE_PACKET
 *              one_port_counter_size           - sizeof(SWDRV_IfTableStats_T)
 *                                                sizeof(SWDRV_IfXTableStats_T)
 *                                                sizeof(SWDRV_RmonStats_T)
 *                                                sizeof(SWDRV_EtherlikeStats_T)
 *                                                sizeof(SWDRV_EtherlikePause_T)
 *              *isc_request_payload            - nmtrdrv_request.data.if_stats
 *                                                nmtrdrv_request.data.ifx_stats
 *                                                nmtrdrv_request.data.rmon_stats
 *                                                nmtrdrv_request.data.etherlike_stats
 *                                                nmtrdrv_request.data.etherlike_pause
 *              local_table[]                   - nmtrdrv_if_stats[]
 *                                                nmtrdrv_ifx_stats[]
 *                                                nmtrdrv_rmon_stats[]
 *                                                nmtrdrv_etherlike_stats[]
 *                                                nmtrdrv_etherlike_pause[]
 *-----------------------------------------------------------------------------------------------------------------*/
static void NMTRDRV_SendStates2MasterViaISC(UI32_T service_id,
                                            UI32_T start_port,
                                            UI32_T num_of_port,
                                            UI32_T max_port_num_in_one_isc_packet,
                                            UI32_T one_port_counter_size,
                                            void   *local_table)
{
    /* the first isc packet carry port info from loacl_table[0] either mainboard or module.
     * the second isc packet will from loacl_table[(max_port_num_in_one_isc_packet+1)-1].
     */

    L_MM_Mref_Handle_T*         mref_handle_p;
    NMTRDRV_Request_Packet_T*   isc_buffer_p;
    UI32_T                      pdu_len;
    UI32_T                      local_table_start_port_index = start_port;
    UI32_T                      isc_packet_num;
    UI32_T                      isc_packet_index;
    UI32_T                      max_port_num_on_board;
    UI16_T                      master_unit_bmp;
    UI8_T                       master_unit_id;
    BOOL_T                      is_option_module=FALSE;

    if (STKTPLG_POM_IsOptionModule())
    {
        is_option_module = TRUE;
        if (STKTPLG_POM_GetMaxPortNumberOnBoard(NMTRDRV_OM_GetMyUnitId(), &max_port_num_on_board) == FALSE)
        {
            NMTRDRV_DEBUG_MSG("Get max port number on board fail.");
            return ;
        }
    }

    /* Send to Master via ISC
     */
    STKTPLG_POM_GetMasterUnitId(&master_unit_id);
    master_unit_bmp = NMTRDRV_UNIT_TO_UNITBMP(master_unit_id);

    /* nmtrdrv needs how many isc packet to send this local_table.
     */
    isc_packet_num =  num_of_port/max_port_num_in_one_isc_packet;

    if ((num_of_port%max_port_num_in_one_isc_packet)!=0)
    {
        isc_packet_num++;
    }

    for (isc_packet_index=1;isc_packet_index<=isc_packet_num;isc_packet_index++)
    {
        mref_handle_p = L_MM_AllocateTxBuffer(sizeof(NMTRDRV_Request_Packet_T), /* tx_buffer_size */
                                              L_MM_USER_ID(SYS_MODULE_NMTRDRV, NMTRDRV_POOL_ID, NMTRDRV_TRACE_ID) /* user_id */);

        isc_buffer_p = L_MM_Mref_GetPdu (mref_handle_p, &pdu_len);

        if (isc_buffer_p==NULL)
        {
        	NMTRDRV_DEBUG_MSG("Get Pdu fail.");
        	return;
        }

        isc_buffer_p->service_id = service_id;
        isc_buffer_p->unit = NMTRDRV_OM_GetMyUnitId();

        /* Initialize isc_buffer_p->start_port before first isc packet.
         */
        if (is_option_module)
        {
            /* option module's start_port = max_port_num_on_board +1
             */
            isc_buffer_p->start_port = max_port_num_on_board+start_port+(max_port_num_in_one_isc_packet*(isc_packet_index-1));
        }
        else /* mainboard*/
        {
            /* ex. ES4625 send if_table
             * first isc packet:  mainboard, nmtrdrv_request.start_port =  1
             *                    module, nmtrdrv_request.start_port = 1
             * second isc packet: mainboard, nmtrdrv_request.start_port = 1+15 = 16
             *                    module, nmtrdrv_request.start_port = 49 + 15 = 64
             * third isc packet: mainboard, nmtrdrv_request.start_port = 16+15 = 31
             *                    module, nmtrdrv_request.start_port = 64 + 15 = 78
             */
            isc_buffer_p->start_port = start_port+(max_port_num_in_one_isc_packet*(isc_packet_index-1));
        }

        if (isc_packet_index!=isc_packet_num)
        {
            /*isc_packet_index<=isc_packet_num, if isc_packet_index!=isc_packet_num, it means isc_packet_index< isc_packet_num
             * So, this is not last isc packet for this request.
             */
            isc_buffer_p->updated_port_num = max_port_num_in_one_isc_packet;
        }
        else
        {
            isc_buffer_p->updated_port_num = (num_of_port%max_port_num_in_one_isc_packet);
        }

        /* copy loacl table to isc payload.
        */
        /* isc_buffer_p->data.if_stats's address == isc_buffer_p->data.ifx_stats == isc_buffer_p->data.rmon_stats
         *                                       == isc_buffer_p->data.etherlike_stats
         */
        memcpy(isc_buffer_p->data.if_stats,
               ((UI8_T *)local_table)+((local_table_start_port_index-start_port)*one_port_counter_size),
               one_port_counter_size*(isc_buffer_p->updated_port_num));

         if (ISC_SendMcastReliable(master_unit_bmp,ISC_NMTRDRV_SID,
                                             mref_handle_p,
                                             SYS_DFLT_STK_MGMT_PACKET_TO_CPU_PRIORITY,
                                             NMTRDRV_RETRY_COUNT, NMTRDRV_TIMEOUT, FALSE) != 0)
        {
            NMTRDRV_DEBUG_MSG("ISC fail.");
            return;
        }

        /*ex. ES4625 send if_table
         * first isc packet:  local_table_start_port_index = 1
         * second isc packet: local_table_start_port_index = 1+ 15 = 16
         * third isc packet: local_table_start_port_index = 16+ 15 = 31
         */
        local_table_start_port_index += max_port_num_in_one_isc_packet;
    }
}
#endif

#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_ClearPortCounter
 *------------------------------------------------------------------------------
 * Purpose  : Clears the port counter on slave and retrun status back to master
 * INPUT    : ISC_Key_T *key    - key of ISC
 *            UI32_T    port    - port number
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Send status back to master unit instead of returning it to local unit
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_ClearPortCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    if(request_p==NULL)
        return FALSE;

    /* Do the job */
    return NMTRDRV_ClearPortCounter(NMTRDRV_OM_GetMyUnitId(), request_p->start_port);
} /* end of NMTRDRV_Remote_ClearPortCounter() */

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_ClearAllCounter
 *------------------------------------------------------------------------------
 * Purpose  : Clears all the port counter on slave and retrun status back to master
 * INPUT    : ISC_Key_T *key    - key of ISC
 *            UI32_T    port    - port number
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Send status back to master unit instead of returning it to local unit
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_ClearAllCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    return NMTRDRV_ClearAllCounters();
} /* end of NMTRDRV_Remote_ClearAllCounter() */

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateIfTableStats
 *------------------------------------------------------------------------------
 * Purpose  : Send IfTable Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdateIfTableStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
	UI32_T offset;
	
    if (NMTRDRV_OM_GetDebugFlag() == TRUE )
    {
        printf("\r\n --------NMTRDRV_Remote_UpdateIfTableStats()-----------");
        printf("\r\n unit            : %d",request_p->unit);
        printf("\r\n start_port      : %d",request_p->start_port);
        printf("\r\n updated_port_num: %d",request_p->updated_port_num);
        printf("\r\n -----------------------------------------------------");
    }
		
	for ( offset = 0; offset < request_p->updated_port_num; offset ++)
	{
		if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
		{
			NMTRDRV_OM_ClearIfStats(request_p->unit,request_p->start_port + offset);
			continue;
		}
		NMTRDRV_OM_SetIfStats(request_p->unit,request_p->start_port + offset, &request_p->data.if_stats[offset]);
			
	} /* for (port = 1; port <= num_of_port; port++) */
	
    return NMTRDRV_Notify_IfTableStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateIfXTableStats
 *------------------------------------------------------------------------------
 * Purpose  : Send IfXTable Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdateIfXTableStats(ISC_Key_T *key,NMTRDRV_Request_Packet_T *request_p)
{
	UI32_T offset;
	
	 for ( offset = 0; offset < request_p->updated_port_num; offset ++)
	 {
		 if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
		 {
			 NMTRDRV_OM_ClearIfXStats(request_p->unit,request_p->start_port + offset);
			 continue;
		 }
		 NMTRDRV_OM_SetIfXStats(request_p->unit,request_p->start_port + offset, &request_p->data.ifx_stats[offset]);
		 
	 } /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_IfXTableStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateRmonStats
 *------------------------------------------------------------------------------
 * Purpose  : Send Rmon Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T   NMTRDRV_Remote_UpdateRmonStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{    
    UI32_T offset;

	for ( offset = 0; offset < request_p->updated_port_num; offset ++)
	{
		if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
		{
			NMTRDRV_OM_ClearRmonStats(request_p->unit,request_p->start_port + offset);
			continue;
		}
		NMTRDRV_OM_SetRmonStats(request_p->unit,request_p->start_port + offset, &request_p->data.rmon_stats[offset]);
		
	} /* for (port = 1; port <= num_of_port; port++) */
	

    return NMTRDRV_Notify_RmonStats(request_p->unit,request_p->start_port, request_p->updated_port_num);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateEtherLikeStats
 *------------------------------------------------------------------------------
 * Purpose  : Send Ether Like Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T   NMTRDRV_Remote_UpdateEtherLikeStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    UI32_T offset;

	for ( offset = 0; offset < request_p->updated_port_num; offset ++)
	{
		if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
		{
			NMTRDRV_OM_ClearEtherlikeStats(request_p->unit,request_p->start_port + offset);
			continue;
		}
		NMTRDRV_OM_SetEtherlikeStats(request_p->unit,request_p->start_port + offset, &request_p->data.etherlike_stats[offset]);
		
	} /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_EtherLikeStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateEtherLikePause
 *------------------------------------------------------------------------------
 * Purpose  : Send Ether Like Pause Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T   NMTRDRV_Remote_UpdateEtherLikePause(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    UI32_T offset;

	for ( offset = 0; offset < request_p->updated_port_num; offset ++)
	{
		if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
		{
			NMTRDRV_OM_ClearEtherlikePause(request_p->unit,request_p->start_port + offset);
			continue;
		}
		NMTRDRV_OM_SetEtherlikePause(request_p->unit,request_p->start_port + offset, &request_p->data.etherlike_pause[offset]);
		
	} /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_EtherLikePause(request_p->unit, request_p->start_port, request_p->updated_port_num);
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateIfPerQStats
 *------------------------------------------------------------------------------
 * Purpose  : Send CoS queue statistics to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdateIfPerQStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    UI32_T offset;

    if (NMTRDRV_OM_GetDebugFlag() == TRUE )
    {
        printf("\r\n --------NMTRDRV_Remote_UpdateIfPerQStats()-----------");
        printf("\r\n unit            : %d",request_p->unit);
        printf("\r\n start_port      : %d",request_p->start_port);
        printf("\r\n updated_port_num: %d",request_p->updated_port_num);
        printf("\r\n -----------------------------------------------------");
    }

    for ( offset = 0; offset < request_p->updated_port_num; offset ++)
    {
        if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
        {
            NMTRDRV_OM_ClearIfPerQStats(request_p->unit,request_p->start_port + offset);
            continue;
        }
        NMTRDRV_OM_SetIfPerQStats(request_p->unit,request_p->start_port + offset, &request_p->data.ifperq_stats[offset]);

    } /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_IfPerQStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdatePfcStats
 *------------------------------------------------------------------------------
 * Purpose  : Send PFC statistics to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdatePfcStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    UI32_T offset;

    if (NMTRDRV_OM_GetDebugFlag() == TRUE )
    {
        printf("\r\n --------NMTRDRV_Remote_UpdatePfcStats()-----------");
        printf("\r\n unit            : %d",request_p->unit);
        printf("\r\n start_port      : %d",request_p->start_port);
        printf("\r\n updated_port_num: %d",request_p->updated_port_num);
        printf("\r\n -----------------------------------------------------");
    }

    for ( offset = 0; offset < request_p->updated_port_num; offset ++)
    {
        if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
        {
            NMTRDRV_OM_ClearPfcStats(request_p->unit,request_p->start_port + offset);
            continue;
        }
        NMTRDRV_OM_SetPfcStats(request_p->unit,request_p->start_port + offset, &request_p->data.pfc_stats[offset]);

    } /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_PfcStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateQcnStats
 *------------------------------------------------------------------------------
 * Purpose  : Send QCN statistics to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdateQcnStats(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    UI32_T offset;

    if (NMTRDRV_OM_GetDebugFlag() == TRUE )
    {
        printf("\r\n --------NMTRDRV_Remote_UpdateQcnStats()-----------");
        printf("\r\n unit            : %d",request_p->unit);
        printf("\r\n start_port      : %d",request_p->start_port);
        printf("\r\n updated_port_num: %d",request_p->updated_port_num);
        printf("\r\n -----------------------------------------------------");
    }

    for ( offset = 0; offset < request_p->updated_port_num; offset ++)
    {
        if(!STKTPLG_POM_PortExist(request_p->unit, request_p->start_port + offset))
        {
            NMTRDRV_OM_ClearQcnStats(request_p->unit,request_p->start_port + offset);
            continue;
        }
        NMTRDRV_OM_SetQcnStats(request_p->unit,request_p->start_port + offset, &request_p->data.qcn_stats[offset]);

    } /* for (port = 1; port <= num_of_port; port++) */

    return NMTRDRV_Notify_QcnStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}
#endif /* (SYS_CPNT_CN == TRUE) */

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_ClearVlanCounter
 *------------------------------------------------------------------------------
 * Purpose  : Clears the vlan counter on slave and retrun status back to master
 * INPUT    : ISC_Key_T *key    - key of ISC
 *            UI32_T    port    - port number
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Send status back to master unit instead of returning it to local unit
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_ClearVlanCounter(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    return NMTRDRV_ClearVlanCounter(request_p->unit, request_p->start_port);
}

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_UpdateIfXTableStatsForVlan
 *------------------------------------------------------------------------------
 * Purpose  : Send IfXTable Stats to master
 * INPUT    : ISC_Key_T *key                        - key of ISC
 *            NMTRDRV_Request_Packet_T *request_p   - request packet
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_UpdateIfXTableStatsForVlan(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    NMTRDRV_OM_SetIfXStatsForVlan(request_p->unit, request_p->start_port, request_p->updated_port_num, request_p->data.ifx_stats);
    return NMTRDRV_Notify_IfXTableStats(request_p->unit, request_p->start_port, request_p->updated_port_num);
}
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_Remote_SetProvisionComplete
 *------------------------------------------------------------------------------
 * Purpose  : Set provision complete to Slave
 * INPUT    : ISC_Key_T *key    - key of ISC
 *            error             - error code
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-----------------------------------------------------------------------------*/
static BOOL_T NMTRDRV_Remote_SetProvisionComplete(ISC_Key_T *key, NMTRDRV_Request_Packet_T *request_p)
{
    NMTRDRV_OM_SetProvisionComplete(TRUE);
    return TRUE;
}

static BOOL_T NMTRDRV_Remote_Service_Demux(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T svc_id)
{
    NMTRDRV_Request_Packet_T    *request_p;
    UI32_T pdu_len;
    BOOL_T retval;

    request_p = (NMTRDRV_Request_Packet_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);

    /*
     * Check to abort operation if callback service id(opcode) is more then
     * number of callback service on this drive.
     */
    if(request_p->service_id >= NMTRDRV_MAX_NUM_OF_SERVICE || NMTRDRV_remote_service_table[request_p->service_id]==NULL)
    {
        L_MM_Mref_Release(&mref_handle_p);
        printf("\r\nNmtrdrv: Service ID is invalid!\r\n");
        return TRUE;
    }

    retval = NMTRDRV_remote_service_table[request_p->service_id](key, request_p);
    L_MM_Mref_Release(&mref_handle_p);
    return retval;
}
#endif /* SYS_CPNT_STACKING */

static BOOL_T NMTRDRV_GetUnitPort(UI32_T *unit, UI32_T *port)
{
    char buf[3];
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    *unit = atoi(buf);
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    *port = atoi(buf);
    if(!STKTPLG_POM_PortExist(*unit, *port))
    {
        BACKDOOR_MGR_Print("Port does not exist");
        return FALSE;
    }
    return TRUE;
}

static BOOL_T NMTRDRV_GetPort(UI32_T *port)
{
    UI32_T my_driver_unit;
    char buf[3];
    BACKDOOR_MGR_Print("\r\nEnter Port Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 2);
    *port = atoi(buf);
    STKTPLG_POM_GetMyDriverUnit(&my_driver_unit);
    if(!STKTPLG_POM_PortExist(my_driver_unit, *port))
    {
        BACKDOOR_MGR_Print("Port does not exist");
        return FALSE;
    }
    return TRUE;
}

#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
static BOOL_T NMTRDRV_GetUnitVlan(UI32_T *unit, UI32_T *vid)
{
    char buf[5];
    BACKDOOR_MGR_Print("\r\nEnter Unit Number: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 1);
    *unit = atoi(buf);
    BACKDOOR_MGR_Print("\r\nEnter VID: ");
    BACKDOOR_MGR_RequestKeyIn(buf, 4);
    *vid = atoi(buf);
    if (*vid > SYS_ADPT_MAX_VLAN_ID)
    {
        BACKDOOR_MGR_Print("VID is invalid");
        return FALSE;
    }
    return TRUE;
}
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */

static void NMTRDRV_Backdoor_Menu(void)
{
    char    buf[16];
    int   ch;
    BOOL_T  exit;
	UI32_T lunit;
    UI32_T  debug_unit = 0;
    UI32_T  debug_port = 0;

    SWDRV_IfTableStats_T   if_stats;
    SWDRV_IfXTableStats_T  ifx_stats;
    SWDRV_RmonStats_T      rmon_stats;
    SWDRV_EtherlikeStats_T etherlike_stats;
    SWDRV_EtherlikePause_T etherlike_pause;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
    SWDRV_IfPerQStats_T    ifperq_stats;
#endif
#if (SYS_CPNT_PFC == TRUE)
    SWDRV_PfcStats_T       pfc_stats;
#endif
#if (SYS_CPNT_CN == TRUE)
    SWDRV_QcnStats_T       qcn_stats;
#endif

	lunit = NMTRDRV_OM_GetMyUnitId();

    exit = FALSE;
    for(; !exit;)
    {
        BACKDOOR_MGR_Print("\r\n    NMTRDEV BACKDOOR MENU");
        BACKDOOR_MGR_Print("\r\n=============================");
        BACKDOOR_MGR_Print("\r\n0. Exit");
        BACKDOOR_MGR_Print("\r\n1. Clear All Counters");
        BACKDOOR_MGR_Print("\r\n2. Clear Port Counter");
        BACKDOOR_MGR_Print("\r\n3. Show Local If Info");
        BACKDOOR_MGR_Print("\r\n4. Show Local Ifx Info");
        BACKDOOR_MGR_Print("\r\n5. Show Local Rmon Info");
        BACKDOOR_MGR_Print("\r\n6. Show Local Etherlink Info");
        BACKDOOR_MGR_Print("\r\n7. Show Local Etherlink Pause Info");
        BACKDOOR_MGR_Printf("\r\n8. Hide/Show debug message[%s]",NMTRDRV_OM_GetDebugFlag()?"ON":"OFF");
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
        BACKDOOR_MGR_Print("\r\n9. Show Local IfPerQ Info");
#endif
#if (SYS_CPNT_PFC == TRUE)
        BACKDOOR_MGR_Print("\r\n10. Show Local PFC Info");
#endif
#if (SYS_CPNT_CN == TRUE)
        BACKDOOR_MGR_Print("\r\n11. Show Local QCN Info");
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
        BACKDOOR_MGR_Print("\r\n12. Show Local VLAN Ifx Info");
#endif
        BACKDOOR_MGR_Print("\r\nEnter you selection: ");

        BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);
        BACKDOOR_MGR_Print("\r\n");

        if (1 != sscanf(buf, "%d", &ch))
            continue;

        switch(ch)
        {
            case 0:
                exit = TRUE;
                break;
            case 1:
                NMTRDRV_ClearAllCounters();
                break;
            case 2:
                if( !NMTRDRV_GetUnitPort(&debug_unit, &debug_port))
                    break;
                NMTRDRV_ClearPortCounter(debug_unit, debug_port);
                break;
            case 3:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetIfStats(lunit,debug_port, &if_stats);
                NMTRDRV_Backdoor_GetIfInfo(&if_stats, debug_unit, debug_port);
                break;
            case 4:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetIfXStats(lunit,debug_port, &ifx_stats);
                NMTRDRV_Backdoor_GetIfXInfo(&ifx_stats);
                break;
            case 5:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetRmonStats(lunit,debug_port, &rmon_stats);
                NMTRDRV_Backdoor_GetRmonInfo(&rmon_stats);
                break;
            case 6:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetEtherlikeStats(lunit,debug_port, &etherlike_stats);
                NMTRDRV_Backdoor_GetEtherLikeInfo(&etherlike_stats);
                break;
            case 7:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetEtherlikePause(lunit,debug_port, &etherlike_pause);
                NMTRDRV_Backdoor_GetEtherLikePauseInfo(&etherlike_pause);
                break;
            case 8:
                NMTRDRV_OM_SetDebugFlag(!NMTRDRV_OM_GetDebugFlag());
                break;
#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
            case 9:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetIfPerQStats(lunit,debug_port, &ifperq_stats);
                NMTRDRV_Backdoor_GetIfPerQInfo(&ifperq_stats, debug_unit, debug_port);
                break;
#endif
#if (SYS_CPNT_PFC == TRUE)
            case 10:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetPfcStats(lunit,debug_port, &pfc_stats);
                NMTRDRV_Backdoor_GetPfcInfo(&pfc_stats, debug_unit, debug_port);
                break;
#endif
#if (SYS_CPNT_CN == TRUE)
            case 11:
                if( !NMTRDRV_GetPort(&debug_port))
                    break;
                NMTRDRV_OM_GetQcnStats(lunit,debug_port, &qcn_stats);
                NMTRDRV_Backdoor_GetQcnInfo(&qcn_stats, debug_unit, debug_port);
                break;
#endif
#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
            case 12:
                if( !NMTRDRV_GetUnitVlan(&debug_unit, &debug_port))
                    break;
                NMTRDRV_OM_GetIfXStatsForVlan(debug_unit, debug_port, 1, &ifx_stats);
                NMTRDRV_Backdoor_GetIfXInfo(&ifx_stats);
                break;
#endif
            default:
                break;
        }
    }
}

static void NMTRDRV_Backdoor_GetIfInfo(SWDRV_IfTableStats_T *stat, UI32_T debug_unit, UI32_T debug_port)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};

    BACKDOOR_MGR_Printf("\r\nUnit: %ld,    Port: %ld", (long)debug_unit, (long)debug_port);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInOctets),L_STDLIB_UI64_L32(stat->ifInOctets),str);
    BACKDOOR_MGR_Printf("\r\nifInOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInUcastPkts),L_STDLIB_UI64_L32(stat->ifInUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInNUcastPkts),L_STDLIB_UI64_L32(stat->ifInNUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInNUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInDiscards),L_STDLIB_UI64_L32(stat->ifInDiscards),str);
    BACKDOOR_MGR_Printf("\r\nifInDiscards:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInErrors),L_STDLIB_UI64_L32(stat->ifInErrors),str);
    BACKDOOR_MGR_Printf("\r\nifInErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInUnknownProtos),L_STDLIB_UI64_L32(stat->ifInUnknownProtos),str);
    BACKDOOR_MGR_Printf("\r\nifInUnknownProtos:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutOctets),L_STDLIB_UI64_L32(stat->ifOutOctets),str);
    BACKDOOR_MGR_Printf("\r\nifOutOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutUcastPkts),L_STDLIB_UI64_L32(stat->ifOutUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutNUcastPkts),L_STDLIB_UI64_L32(stat->ifOutNUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutNUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutDiscards),L_STDLIB_UI64_L32(stat->ifOutDiscards),str);
    BACKDOOR_MGR_Printf("\r\nifOutDiscards:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutErrors),L_STDLIB_UI64_L32(stat->ifOutErrors),str);
    BACKDOOR_MGR_Printf("\r\nifOutErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutQLen),L_STDLIB_UI64_L32(stat->ifOutQLen),str);
    BACKDOOR_MGR_Printf("\r\nifOutQLen:         %s", str);
    return;
}

static void NMTRDRV_Backdoor_GetIfXInfo(SWDRV_IfXTableStats_T *stat)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInMulticastPkts),L_STDLIB_UI64_L32(stat->ifInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInMulticastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifInBroadcastPkts),L_STDLIB_UI64_L32(stat->ifInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifInBroadcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutMulticastPkts),L_STDLIB_UI64_L32(stat->ifOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutMulticastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifOutBroadcastPkts),L_STDLIB_UI64_L32(stat->ifOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifOutBroadcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCInOctets),L_STDLIB_UI64_L32(stat->ifHCInOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCInOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCInUcastPkts),L_STDLIB_UI64_L32(stat->ifHCInUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCInMulticastPkts),L_STDLIB_UI64_L32(stat->ifHCInMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInMulticastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCInBroadcastPkts),L_STDLIB_UI64_L32(stat->ifHCInBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCInBroadcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCOutOctets),L_STDLIB_UI64_L32(stat->ifHCOutOctets),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCOutUcastPkts),L_STDLIB_UI64_L32(stat->ifHCOutUcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutUcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCOutMulticastPkts),L_STDLIB_UI64_L32(stat->ifHCOutMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutMulticastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ifHCOutBroadcastPkts),L_STDLIB_UI64_L32(stat->ifHCOutBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\nifHCOutBroadcastPkts:         %s", str);
    return;
}

static void NMTRDRV_Backdoor_GetRmonInfo(SWDRV_RmonStats_T *stat)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsDropEvents),L_STDLIB_UI64_L32(stat->etherStatsDropEvents),str);
    BACKDOOR_MGR_Printf("\r\netherStatsDropEvents:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsOctets),L_STDLIB_UI64_L32(stat->etherStatsOctets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsOctets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts),L_STDLIB_UI64_L32(stat->etherStatsPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsBroadcastPkts),L_STDLIB_UI64_L32(stat->etherStatsBroadcastPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsBroadcastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsMulticastPkts),L_STDLIB_UI64_L32(stat->etherStatsMulticastPkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsMulticastPkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsCRCAlignErrors),L_STDLIB_UI64_L32(stat->etherStatsCRCAlignErrors),str);
    BACKDOOR_MGR_Printf("\r\netherStatsCRCAlignErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsUndersizePkts),L_STDLIB_UI64_L32(stat->etherStatsUndersizePkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsUndersizePkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsOversizePkts),L_STDLIB_UI64_L32(stat->etherStatsOversizePkts),str);
    BACKDOOR_MGR_Printf("\r\netherStatsOversizePkts:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsFragments),L_STDLIB_UI64_L32(stat->etherStatsFragments),str);
    BACKDOOR_MGR_Printf("\r\netherStatsFragments:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsJabbers),L_STDLIB_UI64_L32(stat->etherStatsJabbers),str);
    BACKDOOR_MGR_Printf("\r\netherStatsJabbers:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsCollisions),L_STDLIB_UI64_L32(stat->etherStatsCollisions),str);
    BACKDOOR_MGR_Printf("\r\netherStatsCollisions:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts64Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts64Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts64Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts65to127Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts65to127Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts65to127Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts65to127Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts65to127Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts65to127Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts128to255Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts128to255Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts128to255Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts256to511Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts256to511Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts256to511Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts512to1023Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts512to1023Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts512to1023Octets:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->etherStatsPkts1024to1518Octets),L_STDLIB_UI64_L32(stat->etherStatsPkts1024to1518Octets),str);
    BACKDOOR_MGR_Printf("\r\netherStatsPkts1024to1518Octets:         %s", str);
    return;
}
static void NMTRDRV_Backdoor_GetEtherLikeInfo(SWDRV_EtherlikeStats_T *stat)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsAlignmentErrors),L_STDLIB_UI64_L32(stat->dot3StatsAlignmentErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsAlignmentErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsFCSErrors),L_STDLIB_UI64_L32(stat->dot3StatsFCSErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsFCSErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsSingleCollisionFrames),L_STDLIB_UI64_L32(stat->dot3StatsSingleCollisionFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSingleCollisionFrames:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsMultipleCollisionFrames),L_STDLIB_UI64_L32(stat->dot3StatsMultipleCollisionFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsMultipleCollisionFrames:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsSQETestErrors),L_STDLIB_UI64_L32(stat->dot3StatsSQETestErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSQETestErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsDeferredTransmissions),L_STDLIB_UI64_L32(stat->dot3StatsDeferredTransmissions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsDeferredTransmissions:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsLateCollisions),L_STDLIB_UI64_L32(stat->dot3StatsLateCollisions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsLateCollisions:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsExcessiveCollisions),L_STDLIB_UI64_L32(stat->dot3StatsExcessiveCollisions),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsExcessiveCollisions:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsInternalMacTransmitErrors),L_STDLIB_UI64_L32(stat->dot3StatsInternalMacTransmitErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsInternalMacTransmitErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsCarrierSenseErrors),L_STDLIB_UI64_L32(stat->dot3StatsCarrierSenseErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsCarrierSenseErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsFrameTooLongs),L_STDLIB_UI64_L32(stat->dot3StatsFrameTooLongs),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsFrameTooLongs:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsInternalMacReceiveErrors),L_STDLIB_UI64_L32(stat->dot3StatsInternalMacReceiveErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsInternalMacReceiveErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsSymbolErrors),L_STDLIB_UI64_L32(stat->dot3StatsSymbolErrors),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsSymbolErrors:         %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3StatsDuplexStatus),L_STDLIB_UI64_L32(stat->dot3StatsDuplexStatus),str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsDuplexStatus:         %s", str);
    BACKDOOR_MGR_Printf("\r\ndot3StatsRateControlAbility:              %ld", (long)stat->dot3StatsRateControlAbility);
    BACKDOOR_MGR_Printf("\r\ndot3StatsRateControlStatus:              %ld", (long)stat->dot3StatsRateControlStatus);
    return;
}

static void NMTRDRV_Backdoor_GetEtherLikePauseInfo(SWDRV_EtherlikePause_T *stat)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};

    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3InPauseFrames),L_STDLIB_UI64_L32(stat->dot3InPauseFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3InPauseFrames:          %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->dot3OutPauseFrames),L_STDLIB_UI64_L32(stat->dot3OutPauseFrames),str);
    BACKDOOR_MGR_Printf("\r\ndot3OutPauseFrames:         %s", str);
    return;
}

#if (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE)
static void NMTRDRV_Backdoor_GetIfPerQInfo(SWDRV_IfPerQStats_T *stat, UI32_T debug_unit, UI32_T debug_port)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    int i;

    BACKDOOR_MGR_Printf("\r\nUnit: %ld,    Port: %ld", (long)debug_unit, (long)debug_port);

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_PRIORITY_QUEUE; i++)
    {
        BACKDOOR_MGR_Printf("\r\nQueue: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->cosq[i].ifOutOctets),L_STDLIB_UI64_L32(stat->cosq[i].ifOutOctets),str);
        BACKDOOR_MGR_Printf("\r\nifOutOctets:                %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->cosq[i].ifOutPkts),L_STDLIB_UI64_L32(stat->cosq[i].ifOutPkts),str);
        BACKDOOR_MGR_Printf("\r\nifOutPkts:                  %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->cosq[i].ifOutDiscardOctets),L_STDLIB_UI64_L32(stat->cosq[i].ifOutDiscardOctets),str);
        BACKDOOR_MGR_Printf("\r\nifOutDiscardOctets:         %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->cosq[i].ifOutDiscardPkts),L_STDLIB_UI64_L32(stat->cosq[i].ifOutDiscardPkts),str);
        BACKDOOR_MGR_Printf("\r\nifOutDiscardPkts:           %s", str);
    }

    return;
}
#endif /* (SYS_CPNT_NMTR_PERQ_COUNTER == TRUE) */

#if (SYS_CPNT_PFC == TRUE)
static void NMTRDRV_Backdoor_GetPfcInfo(SWDRV_PfcStats_T *stat, UI32_T debug_unit, UI32_T debug_port)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    int i;

    BACKDOOR_MGR_Printf("\r\nUnit: %ld,    Port: %ld", (long)debug_unit, (long)debug_port);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ieee8021PfcRequests),L_STDLIB_UI64_L32(stat->ieee8021PfcRequests),str);
    BACKDOOR_MGR_Printf("\r\nieee8021PfcRequests:        %s", str);
    L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->ieee8021PfcIndications),L_STDLIB_UI64_L32(stat->ieee8021PfcIndications),str);
    BACKDOOR_MGR_Printf("\r\nieee8021PfcIndications:     %s", str);

    for (i = 0; i < 8; i++)
    {
        BACKDOOR_MGR_Printf("\r\nPriority: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->pri[i].ieee8021PfcRequests),L_STDLIB_UI64_L32(stat->pri[i].ieee8021PfcRequests),str);
        BACKDOOR_MGR_Printf("\r\nieee8021PfcRequests:        %s", str);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->pri[i].ieee8021PfcIndications),L_STDLIB_UI64_L32(stat->pri[i].ieee8021PfcIndications),str);
        BACKDOOR_MGR_Printf("\r\nieee8021PfcIndications:     %s", str);
    }

    return;
}
#endif /* (SYS_CPNT_PFC == TRUE) */

#if (SYS_CPNT_CN == TRUE)
static void NMTRDRV_Backdoor_GetQcnInfo(SWDRV_QcnStats_T *stat, UI32_T debug_unit, UI32_T debug_port)
{
    /*1~19: string length of UI64_T, str[20]='0'*/
    char str[21] = {0};
    int i;

    BACKDOOR_MGR_Printf("\r\nUnit: %ld,    Port: %ld", (long)debug_unit, (long)debug_port);

    for (i = 0; i < SYS_ADPT_CN_MAX_NBR_OF_CP_PER_PORT; i++)
    {
        BACKDOOR_MGR_Printf("\r\nCP Queue: %d", i);
        L_STDLIB_UI64toa(L_STDLIB_UI64_H32(stat->cpq[i].qcnStatsOutCnms),L_STDLIB_UI64_L32(stat->cpq[i].qcnStatsOutCnms),str);
        BACKDOOR_MGR_Printf("\r\nqcnStatsOutCnms:            %s", str);
    }

    return;
}
#endif /* (SYS_CPNT_CN == TRUE) */

