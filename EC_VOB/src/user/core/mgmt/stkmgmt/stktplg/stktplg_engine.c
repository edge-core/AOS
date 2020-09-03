/* Module Name: STKTPLG_ENGINE.C
 * Purpose:
 *
 * Notes:
 *
 * History:
 *    10/04/2002       -- David Lin, Create
 *    10/10/2003       -- Vincent Yang
 * Copyright(C)      Accton Corporation, 2002 - 2005
 *
 */

#define STKTPLG_BACKDOOR_OPEN

/* INCLUDE FILE DECLARATIONS
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "stkmgmt_type.h"
#include "stktplg_type.h"
#include "stktplg_task.h"
#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "stktplg_om_private.h"
#include "stktplg_timer.h"
#include "stktplg_tx.h"
#include "stktplg_engine.h"
#include "stktplg_shom.h"
#include "stktplg_board.h"
#include "sysfun.h"
#include "phyaddr_access.h"

#include "snmp_pmgr.h"

#include "lan_type.h"

//#ifdef Use_BcmDrv
//#include "bcmdrv.h"
//#else
#include "dev_swdrv_pmgr.h"
//#endif

#include "leddrv.h"

#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#include "isc_om.h"
#endif

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
#include "sysdrv.h"
#include "leaf_sys.h"
#endif

#ifdef STKTPLG_BACKDOOR_OPEN
#include "stktplg_backdoor.h"
#endif /* STKTPLG_BACKDOOR_OPEN */

/* NAMING CONSTANT DECLARATIONS
 */

#define MAX_MODULE_ID 32
#define     STKTPLG_EXPANSION_ALIVE 19
#define     STKTPLG_EXPANSION_RESET 1
#define     MAX_DEAD_TIMES       40
/* added by Vincent on 3,Aug,04
 * This constant defines how long the so called hot swapp should
 * happen after master state entered
 */
#define     MAX_HOT_SWAP_TIME    20
#define STKTPLG_ENGINE_INVALID_STACKCHIP_DEVICE_ID 0xFF

/* MACRO FUNCTION DECLARATIONS
 */
#define STKTPLG_ENGINE_IS_UPLINK_PORT_UP(LINK_STATUS)   (((LINK_STATUS)&LAN_TYPE_TX_UP_LINK) ?   TRUE : FALSE)
#define STKTPLG_ENGINE_IS_DOWNLINK_PORT_UP(LINK_STATUS) (((LINK_STATUS)&LAN_TYPE_TX_DOWN_LINK) ? TRUE : FALSE)

#define UP_PORT_RX(phy_port_p)                                                          \
({                                                                                      \
    UI32_T dev_id;                                                                      \
    BOOL_T __rc;                                                                        \
    __rc = STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_UP_LINK,   \
                                                   &dev_id, phy_port_p);                \
    __rc;                                                                               \
})

#define DOWN_PORT_RX(phy_port_p)                                                        \
({                                                                                      \
    UI32_T dev_id;                                                                      \
    BOOL_T __rc;                                                                        \
    __rc = STKTPLG_OM_GetStackingPortPhyDevPortId(STKTPLG_TYPE_STACKING_PORT_DOWN_LINK, \
                                                   &dev_id, phy_port_p);                \
    __rc;                                                                               \
})

#if 0
#define DBG printf
#else
#define DBG(...)
#endif

//#define STKTPLG_ENGINE_WORKAROUND

#ifdef STKTPLG_ENGINE_WORKAROUND
#define DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId(device_id, phy_port_id, link_status) ({*link_status=0; TRUE;})

#endif

typedef enum
{
    UNEXIST_ST=0,
    ALIVE_ST,
    RESET_ST,
    ASSIGN_ST,
    READY_ST,
    RESET_IST,
    ASSIGN_IST,
    STATE_MAX
}MainBoard_State;

typedef enum
{
    ID_NEW_EV=0,
    ID_DISAPPEAR_EV,
    ID_SAME_EV,
    ASSIGNID_EV,
    DEAD_EV,
    EVENT_MAX
}MainBoard_Event;

#define xgs_stacking_debug(fmt, arg...)     \
            if (stktplg_engine_debug_mode)  \
                printf(fmt, ##arg)

#define xgs_stacking_test_debug(fmt, arg...)    \
            if (stktplg_engine_test_mode)   \
                printf(fmt, ##arg)

/* DATA TYPE DECLARATIONS
 */

typedef struct STKTPLG_ENGINE_ProcessState_S
{
    UI32_T  state;
    void    (*process_fun)(UI32_T *notify_msg);

} STKTPLG_ENGINE_ProcessState_T;


typedef struct STKTPLG_ENGINE_HandlePkt_S
{
    UI32_T  state;
    void    (*handle_fun)(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);

} STKTPLG_ENGINE_HandlePkt_T;


typedef void (*STKTPLG_ENGINE_TimerEventFunction_T)(UI32_T *notify_msg);


/* LOCAL SUBPROGRAM DECLARATIONS
 */

static void STKTPLG_ENGINE_ProcessTimeEvent(UI32_T *notify_msg);

#if (SYS_CPNT_STACKING == TRUE)
static void STKTPLG_ENGINE_ProcessIncomingPacket(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
#endif


/* functions used to handle periodical time event under different states
 */
static void STKTPLG_ENGINE_ProcessInitState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessArbitrationState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessStandAloneState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessMasterSyncState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessGetTopologyInfoState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessSlaveWaitAssignState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessMasterState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessSlaveState(UI32_T *notify_msg);
static void STKTPLG_ENGINE_ProcessHaltState(UI32_T *notify_msg);


/* functions used to handle incoming packet under different states
 */
#if (SYS_CPNT_STACKING == TRUE)
static void STKTPLG_ENGINE_HandlePktInitState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktArbitrationState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktStandAloneState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktMasterSyncState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktGetTopologyInfoState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktSlaveWaitAssignState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktMasterState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktSlaveState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
static void STKTPLG_ENGINE_HandlePktHaltState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg);
#endif

#if (SYS_CPNT_STACKING == TRUE)
static BOOL_T STKTPLG_ENGINE_SetStackingPortLinkStatus(BOOL_T is_ring, BOOL_T is_rx_at_up_port, STKTPLG_OM_HBT_0_1_T * hbt1_ptr);
static BOOL_T STKTPLG_ENGINE_ButtonPressed(void);
static BOOL_T STKTPLG_ENGINE_PrepareUnitBasicInformation(STKTPLG_OM_HBT_0_1_T *hbt1_ptr, UI8_T *module_id);
static BOOL_T STKTPLG_ENGINE_AllSlaveReady(STKTPLG_OM_HBT_0_1_T *hbt1_ptr);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_ENGINE_MasterStorePastTplgInfo
 * PURPOSE: Master stores stable topology info in preparation for possible Unit Hot Insert
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES: At this stage, all the payload[].unit_id should be valid
 */
static void STKTPLG_ENGINE_MasterStorePastTplgInfo(void);
/* FUNCTION NAME: STKTPLG_ENGINE_CheckAndSetProvisionLost
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES: If provision_lost is TRUE, it is either an Inserted Unit,
 *        or a unit who's provisioning is lost
 */
static void STKTPLG_ENGINE_CheckAndSetProvisionLost(void);

static void STKTPLG_ENGINE_ReturnToArbitrationState(UI8_T rx_port, BOOL_T renumber,BOOL_T stkbntstate_changed);
#else
static void STKTPLG_ENGINE_ReturnToArbitrationState(UI8_T rx_port);
#endif /* end of #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
static BOOL_T STKTPLG_ENGINE_PriorityCompare(STKTPLG_OM_Ctrl_Info_T *self,STKTPLG_OM_HBT_0_1_Payload_T *next);
static void   STKTPLG_ENGINE_CheckHelloTimer0(UI8_T *logic_link_status);
static void   STKTPLG_ENGINE_SendHelloPDU0(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p,UI32_T up_phy_status,UI32_T down_phy_status);
static BOOL_T STKTPLG_ENGINE_TopologyIsAllRightUp(STKTPLG_OM_HBT_0_1_T *hbt1_ptr);
static BOOL_T STKTPLG_ENGINE_TopologyIsAllRightDown(STKTPLG_OM_HBT_0_1_T *hbt1_ptr);

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
static BOOL_T STKTPLG_ENGINE_ButtonStateChanged(void);
#endif

static BOOL_T STKTPLG_ENGINE_TopologyChangedByButton(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p);
static BOOL_T STKTPLG_ENGINE_NextUpFromMaster(void);
static BOOL_T STKTPLG_ENGINE_NextDownFromMaster(void);
static BOOL_T STKTPLG_ENGINE_IsAbleToBeNextMaster(void);
static BOOL_T STKTPLG_ENGINE_CheckAllUnitsButtonStatus(void);
static void   STKTPLG_ENGINE_ClearCtrlInfo(void);
static void   STKTPLG_ENGINE_ResetUnitCfg(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p);
static void   STKTPLG_ENGINE_AssignUnitID(void);
static void   STKTPLG_ENGINE_Check_Stacking_Link_Down(UI8_T logic_link_status, BOOL_T reset_flag, UI32_T *notify_msg);
static BOOL_T STKTPLG_ENGINE_Check_Stacking_Link_Up(STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p,UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T reset_flag,UI32_T *notify_msg);
static BOOL_T STKTPLG_ENGING_Check_Closed_Loop_TCN(STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p,UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p);

static void   STKTPLG_ENGINE_CheckHelloTimer1(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p);
static UI8_T  inc(UI8_T seq_no);
static void STKTPLG_ENGINE_EventHandler(UI32_T event);
static void STKTPLG_ENGINE_PktMasterStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port);
static void STKTPLG_ENGINE_PktMasterSyncStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port);
static void STKTPLG_ENGINE_PktSlaveStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr);
static void STKTPLG_ENGINE_SlaveSendAssignEvToOm(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p);
#if 0 /* JinhuaWei, 03 August, 2008 10:45:21 */
static void STKTPLG_ENGINE_ExpModuleRemove(UI8_T unit_id,UI8_T module_id);
static void STKTPLG_ENGINE_AssignExpModuleID(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port);
#endif /* #if 0 */
static void STKTPLG_ENGINE_CheckAllOmReady(STKTPLG_OM_HBT_0_1_T *hbt_ptr);
static void STKTPLG_ENGINE_CheckTplgSyncStatus();
static void STKTPLG_ENGINE_ProcessTplgSyncPkt(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p);
static void STKTPLG_ENGINE_LogMsg(char *msg_txt);
static void STKTPLG_ENGINE_PrepareStackInfo(Stacking_Info_T *stack_info);
static void STKTPLG_ENGINE_SetPortMappingTable(STKTPLG_OM_HBT_0_1_Payload_T *payload);
static UI8_T STKTPLG_ENGINE_SetPktType(UI8_T type);
static UI8_T STKTPLG_ENGINE_GetPktType();
static void SKTTPLG_ENGINE_SetISCUnitInfo(UI32_T notify_msg);

#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

static BOOL_T STKTPLG_ENGINE_ConfigStackInfoToIUC(void);
static void STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(UI8_T up_down_stacking_port);

/* STATIC VARIABLE DECLARATIONS
 */

static STKTPLG_ENGINE_ProcessState_T process_state[] =
{
    {STKTPLG_STATE_INIT,                  STKTPLG_ENGINE_ProcessInitState},
    {STKTPLG_STATE_ARBITRATION,           STKTPLG_ENGINE_ProcessArbitrationState},
    {STKTPLG_STATE_STANDALONE,            STKTPLG_ENGINE_ProcessStandAloneState},
    {STKTPLG_STATE_MASTER_SYNC,           STKTPLG_ENGINE_ProcessMasterSyncState},
    {STKTPLG_STATE_GET_TOPOLOGY_INFO,     STKTPLG_ENGINE_ProcessGetTopologyInfoState},
    {STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT, STKTPLG_ENGINE_ProcessSlaveWaitAssignState},
    {STKTPLG_STATE_MASTER,                STKTPLG_ENGINE_ProcessMasterState},
    {STKTPLG_STATE_SLAVE,                 STKTPLG_ENGINE_ProcessSlaveState},
    {STKTPLG_STATE_HALT,                  STKTPLG_ENGINE_ProcessHaltState}
};

#if (SYS_CPNT_STACKING == TRUE)
static STKTPLG_ENGINE_HandlePkt_T handle_packet[] =
{
    {STKTPLG_STATE_INIT,                  STKTPLG_ENGINE_HandlePktInitState},
    {STKTPLG_STATE_ARBITRATION,           STKTPLG_ENGINE_HandlePktArbitrationState},
    {STKTPLG_STATE_STANDALONE,            STKTPLG_ENGINE_HandlePktStandAloneState},
    {STKTPLG_STATE_MASTER_SYNC,           STKTPLG_ENGINE_HandlePktMasterSyncState},
    {STKTPLG_STATE_GET_TOPOLOGY_INFO,     STKTPLG_ENGINE_HandlePktGetTopologyInfoState},
    {STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT, STKTPLG_ENGINE_HandlePktSlaveWaitAssignState},
    {STKTPLG_STATE_MASTER,                STKTPLG_ENGINE_HandlePktMasterState},
    {STKTPLG_STATE_SLAVE,                 STKTPLG_ENGINE_HandlePktSlaveState},
    {STKTPLG_STATE_HALT,                  STKTPLG_ENGINE_HandlePktHaltState}
};
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

/* EXPORTED SUBPROGRAM BODIES
 */

static DEV_SWDRV_Device_Port_Mapping_T port_mapping[SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
static STK_UNIT_CFG_T                  device_info;

#if 0 /* JinhuaWei, 03 August, 2008 10:47:58 */
static UI8_T                           recycle_module_id[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
#endif /* #if 0 */
static UI8_T                           recycle_module_id_idx;
static UI32_T   expansion_module_exist_counter=MAX_DEAD_TIMES;
static BOOL_T stktplg_engine_debug_mode = FALSE;
static BOOL_T   stktplg_engine_TCN_mode = TRUE;
#if 0 /* JinhuaWei, 03 August, 2008 10:44:01 */
static BOOL_T stktplg_engine_test_mode = FALSE;
#endif /* #if 0 */
static BOOL_T stktplg_engine_id_table_exist = FALSE ;
static BOOL_T stktplg_stackingdb_updated = TRUE;
static UI32_T   module_info_state;
static BOOL_T   module_swapped=FALSE;
static BOOL_T   not_handled=FALSE;
static BOOL_T   slave_expansion_assignevsend=FALSE;
#if 0 /* JinhuaWei, 03 August, 2008 10:44:04 */
static BOOL_T   om_portmap_change=FALSE;
static BOOL_T   slave_config = FALSE;
#endif /* #if 0 */
UI8_T  pre_unit_id[8]={1,2,3,4,5,6,7,8};

static BOOL_T   expansion_module_ready=FALSE;
static BOOL_T   up_new_link=FALSE, down_new_link=FALSE; /*add by fen.wang,it is useless ,we may delete it*/
static UI8_T    tcnReason=0;
static BOOL_T   notify_stkctrl_transition=TRUE;
static UI32_T   master_state_counter=0;

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
static BOOL_T   first_up_button_status=TRUE;
static BOOL_T   first_down_button_status=TRUE;
#endif

static UI8_T    hbt1_up_timeout_count=0 ;
static UI8_T    hbt1_down_timeout_count=0;

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
/* for hwcfg virtual addresses used in PHYADDR_ACCESS */
static SYS_TYPE_VAddr_T  sys_hwcfg_stack_status_addr;
#endif

static UI8_T   bd_pkt_type=0;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
 static UI32_T process_time[20][20];
#endif

/* FUNCTION NAME : STKTPLG_ENGINE_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG ENGINE which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *
 */
BOOL_T STKTPLG_ENGINE_InitiateProcessResources(void)
{
    BOOL_T ret=TRUE;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
   memset(process_time,0,sizeof(process_time));
   STKTPLG_ENGINE_BD_SetTick(8,3,1);
#endif
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
    if(FALSE==PHYADDR_ACCESS_GetVirtualAddr(SYS_HWCFG_STACK_STATUS_ADDR, &sys_hwcfg_stack_status_addr))
    {
        printf("\r\nFail to get virtual addr of SYS_HWCFG_STACK_STATUS_ADDR");
        sys_hwcfg_stack_status_addr=0;
        ret=FALSE;
    }
#endif

    return ret;
}
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)

void STKTPLG_ENGINE_BD_GetTick()
{
   UI8_T i,j;
   printf("\r\n ========================================");
   for(i=0;i<10;i++)
   {
       for(j=0;j<10;j++)
       {
         if(process_time[i][j]!=0)
          printf("\r\n i %d, j %d,time %ld",i,j,process_time[i][j]);
        }
   }
    printf("\r\n ========================================");
}
void STKTPLG_ENGINE_BD_ClearTick()
{
  memset(process_time,0,sizeof(process_time));
}

void STKTPLG_ENGINE_BD_SetTick(UI8_T type1,UI8_T type2,UI32_T time)
{
   if(process_time[type1][type2]<time)
    process_time[type1][type2]=time;
}
#endif
/* FUNCTION NAME : STKTPLG_ENGINE_StackManagement
 * PURPOSE: This function is to deal tiemr and packet_received event
 * INPUT:
 *          is_timer_event -- TRUE if the function call is triggered by a timer event
 *                            FALSE if the function call is triggered by a incoming packet
 *
 *          rx_port        -- from which port that we receive packet
 *                            Valid values are UP_PORT_RX or DOWN_PORT_RX
 *                            Undefined value when is_timer_event==TRUE
 *
 *          mref_handle_p -- packet that we receive from IUC.
 *                     if NULL, means this function is called by periodical time event.
 * OUTPUT:  notify_msg -- do we need to inform stack control to change system state.
 * RETUEN:  None
 * NOTES:
 *
 */
void STKTPLG_ENGINE_StackManagement(BOOL_T is_timer_event, UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{
    UI32_T start_time,end_time;
    UI8_T state;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
#if (SYS_CPNT_STACKING == TRUE)
    UI8_T type=0;
#endif

    start_time = SYSFUN_GetSysTick();
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    state = ctrl_info_p->state;

    if (is_timer_event == TRUE)
    {
        /* this function should be called for periodical time event
         */
        if (mref_handle_p != NULL)
        {
            /* should not happen, print out error message
             */
            L_MM_Mref_Release(&mref_handle_p);
            perror("\r\nPassing Error Parameters.");
            assert(0);
        }

        /* check what should we do
         */
        STKTPLG_ENGINE_ProcessTimeEvent(notify_msg);
    }
    else
    {
#if (SYS_CPNT_STACKING == TRUE)
        /* handle packet that we receive
         */
#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
/*stacking is not enable,just drop the pkt*/
        if(ctrl_info_p->stacking_button_state == FALSE && mref_handle_p != NULL)
        {
           L_MM_Mref_Release(&mref_handle_p);
           return;
        }
#endif
        STKTPLG_ENGINE_ProcessIncomingPacket(rx_port, mref_handle_p, notify_msg);
        type=STKTPLG_ENGINE_GetPktType();
#endif
    }

/*   EPR:ES3628BT-FLF-ZZ-00742
   Problem: stack:the master unit hung when remve from the stack.
   Rootcause:when the hot removal happend,and other csc is sending pkt to slaves which have been removed
             and the csc is waiting for the slave reply.so the csc will never receive the slave reply.and it is still in the suspend state
   Solution:when the slave removed update isc_om database ,and wakeup the csc which is waiting for the slave reply which is removed
   Files:ISC_OM.C,stktplg_engine.c,stktplg_om.c,stktplg_om.h makefile.am,l_stack.c,l_stack.h,isc_init.c
*/
    //if(*notify_msg == STKTPLG_UNIT_HOT_INSERT_REMOVE )
    {
        SKTTPLG_ENGINE_SetISCUnitInfo(*notify_msg);
    }

    if( *notify_msg == STKTPLG_UNIT_HOT_INSERT_REMOVE )
    {
        UI32_T  drv_unit;
        UI16_T  exist_drv_units = 0;
    
        for (drv_unit=1; drv_unit<=SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK; drv_unit++)
        {
            if (STKTPLG_OM_ENG_UnitExist(drv_unit))
                exist_drv_units |= IUC_STACK_UNIT_BMP(drv_unit);
            else
                continue;
        }

        if (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)
            STKTPLG_TX_SendTCNType1(NULL, LAN_TYPE_TX_UP_LINK, exist_drv_units);
        if (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)
            STKTPLG_TX_SendTCNType1(NULL, LAN_TYPE_TX_DOWN_LINK,exist_drv_units);
    }

    end_time = SYSFUN_GetSysTick()- start_time;
    STKTPLG_BACKDOOR_SetMaxProcessTime(end_time,state,is_timer_event,type);
}

static UI8_T STKTPLG_ENGINE_GetPktType()
{
   return bd_pkt_type;
}

static UI8_T STKTPLG_ENGINE_SetPktType(UI8_T type)
{
  bd_pkt_type = type;
  return TRUE;   /*added by Jinhua Wei ,to remove warning ,becaued control reaches end of non-void function */
}

/* LOCAL SUBPROGRAM BODIES
 */
static void STKTPLG_ENGINE_ProcessTimeEvent(UI32_T *notify_msg)
{
/* If the system has expansion module, SYS_CPNT_STACKING shall be set as TRUE
 */
#if (SYS_CPNT_STACKING == TRUE)
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    expansion_module_exist_counter--;
    if(expansion_module_exist_counter==0 && ctrl_info_p->expansion_module_exist==TRUE)
    {
        ctrl_info_p->expansion_module_exist= FALSE;
        ctrl_info_p->expansion_module_id=UNKNOWN_MODULE_ID;
        ctrl_info_p->expansion_module_ready= FALSE;
        expansion_module_ready=FALSE;
        module_swapped = TRUE;
        STKTPLG_ENGINE_EventHandler(DEAD_EV);
        printf("The optional module has been removed. \r\n");
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == FALSE)
              printf("\r\nPlease reboot your device. Otherwise it will lead to unexpected behavior.\r\n");
#endif
    }

    if ( (ctrl_info_p->state >= STKTPLG_STATE_INIT) && (ctrl_info_p->state < STKTPLG_STATE_MAX) )
    {
        if (process_state[ctrl_info_p->state].process_fun)
            process_state[ctrl_info_p->state].process_fun(notify_msg);

    }
    else
    {
        /* should not happen, print out error message
         */
        perror("\r\nUnknown State.");
        assert(0);
    }
#else //kh_shi add
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();

    if ( (ctrl_info_p->state >= STKTPLG_STATE_INIT) && (ctrl_info_p->state < STKTPLG_STATE_MAX) )
    {
        if (process_state[ctrl_info_p->state].process_fun)
            process_state[ctrl_info_p->state].process_fun(notify_msg);

    }
    else
    {
        /* should not happen, print out error message
         */
        perror("\r\nUnknown State.");
        assert(0);
    }
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

    return;

}

#if (SYS_CPNT_STACKING == TRUE)
static void STKTPLG_ENGINE_ProcessIncomingPacket(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    Module2Main_Packet_T    *hbt_packet;
    UI8_T                   expansion_modue_id;
    UI32_T                  pdu_len;
#ifdef STKTPLG_BACKDOOR_OPEN
    UI8_T                   *hbt1_packet;
    STKTPLG_OM_HBT_Header_T *pHbtPktHeader ;
#endif

    hbt_packet = (Module2Main_Packet_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt_packet==NULL)
        return;

    /* Module sent his first packet
     */
    if(hbt_packet->pkt_type==STKTPLG_EXPANSION_RESET)
    {
        if(ctrl_info_p->expansion_module_exist==TRUE)
        {
            ctrl_info_p->expansion_module_exist= FALSE;
            ctrl_info_p->expansion_module_id=UNKNOWN_MODULE_ID;
            ctrl_info_p->expansion_module_ready= FALSE;
            expansion_module_ready=FALSE;
            module_swapped = TRUE;
            STKTPLG_ENGINE_EventHandler(DEAD_EV);
            printf("The optional module has been removed. \r\n");
            #if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == FALSE)
                printf("\r\nPlease reboot your device. Otherwise it will lead to unexpected behavior.\r\n");
            #endif
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }
    if(hbt_packet->pkt_type==STKTPLG_EXPANSION_ALIVE || hbt_packet->pkt_type==STKTPLG_EXPANSION_RESET)
    {
        ctrl_info_p->expansion_module_type=hbt_packet->module_type;
        memcpy(ctrl_info_p->module_runtime_fw_ver,hbt_packet->module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);

        if ( ctrl_info_p->expansion_module_exist == FALSE)
             module_swapped = TRUE;
        ctrl_info_p->expansion_module_exist = TRUE;
        expansion_module_exist_counter=MAX_DEAD_TIMES;

#if 0
        printf("\r\nRX OM :module_id=%d module_type=%d \r\n",hbt_packet->module_id,hbt_packet->module_type);
#endif

        expansion_modue_id= hbt_packet->module_id;
        if(UNKNOWN_MODULE_ID==expansion_modue_id)
        {
            ctrl_info_p->expansion_module_ready=FALSE;
            STKTPLG_ENGINE_EventHandler(ID_DISAPPEAR_EV);
        }
        else if(ctrl_info_p->expansion_module_id==expansion_modue_id)
        {
            ctrl_info_p->expansion_module_ready=TRUE;
            STKTPLG_ENGINE_EventHandler(ID_SAME_EV);
        }
        else
        {
            STKTPLG_ENGINE_EventHandler(ID_NEW_EV);
        }
        L_MM_Mref_Release(&mref_handle_p);
        return;
    }
#ifdef STKTPLG_BACKDOOR_OPEN
    /* show the pakcet contects with its current state
     */

    hbt1_packet = (UI8_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt1_packet==NULL)
    {
       STKTPLG_BACKDOOR_IncRxCounter(-1);
       return;
    }

    STKTPLG_BACKDOOR_ShowHbtPacket(STKTPLG_BACKDOOR_RXHBT, ctrl_info_p->state, hbt1_packet);
    pHbtPktHeader = (STKTPLG_OM_HBT_Header_T *)hbt1_packet;
    STKTPLG_ENGINE_SetPktType(pHbtPktHeader->type);
#endif /* STKTPLG_BACKDOOR_OPEN */


    /* call corresponding function
     */
    if ( (ctrl_info_p->state >= STKTPLG_STATE_INIT) && (ctrl_info_p->state < STKTPLG_STATE_MAX) )
    {
        if (handle_packet[ctrl_info_p->state].handle_fun)
            handle_packet[ctrl_info_p->state].handle_fun(rx_port, mref_handle_p, notify_msg);
        else
        {
            L_MM_Mref_Release(&mref_handle_p);
            return;
        }
    }
    else
    {
        L_MM_Mref_Release(&mref_handle_p);
        /* should not happen, print out error message
         */
        perror("\r\nUnknown State.");
        assert(0);
    }
    return;
}
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

#if (SYS_CPNT_STACKING == TRUE)
/*=========================================================================================================================
*           |ID_NEW_EV       |ID_DISAPPEAR_EV | ID_SAME_EV         | ASSIGNID_EV |  DEAD_EV
*------------------------------------------------------------------------------------------------------------
*UNEXIST_ST |ALIVE_ST        |ALIVE_ST        |ALIVE_ST            |UNEXIST_ST   | UNEXIST_ST
*ALIVE_ST   |ALIVE_ST        |ALIVE_ST        |ALIVE_ST            |RESET_ST     | UNEXIST_ST
*RESET_ST   |RESET_ST        |ASSIGN_ST       |RESET_ST            |RESET_ST     | UNEXIST_ST
*ASSIGN_ST  |ASSIGN_ST       |ASSIGN_ST       |READY_ST            |ALIVE_ST     | UNEXIST_ST
*READY_ST   |ALIVE_ST        |ALIVE_ST        |READY_ST            |ALIVE_ST     | UNEXIST_ST
*------------------------------------------------------------------------------------------
*/

static void STKTPLG_ENGINE_EventHandler(UI32_T event)
{
    Stacking_Info_T stack_info;
    static const   UI8_T  fsm[][EVENT_MAX]=
    {
        {ALIVE_ST,ALIVE_ST,ALIVE_ST,UNEXIST_ST,UNEXIST_ST},
        {ALIVE_ST,ALIVE_ST,ALIVE_ST,RESET_ST,UNEXIST_ST},
        {RESET_ST,ASSIGN_ST,RESET_ST,RESET_ST,UNEXIST_ST},
        {ASSIGN_ST,ASSIGN_ST,READY_ST,ALIVE_ST,UNEXIST_ST},
        {ALIVE_ST,ALIVE_ST,READY_ST ,ALIVE_ST,UNEXIST_ST}
    };


    switch(module_info_state=fsm[module_info_state][event])
    {

        case    UNEXIST_ST:
                break;
        case    ALIVE_ST:
                STKTPLG_ENGINE_PrepareStackInfo(&stack_info);
                STKTPLG_TX_SendPacketToExp();
                break;
        case    RESET_ST:
                STKTPLG_TX_SendResetPacketToExp();
                break;
        case    ASSIGN_ST:
                STKTPLG_ENGINE_PrepareStackInfo(&stack_info);
                STKTPLG_TX_SendPacketToExp();
                break;
        case    READY_ST:
                STKTPLG_ENGINE_PrepareStackInfo(&stack_info);
                STKTPLG_TX_SendPacketToExp();
                break;
    }

    return;
}
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

/* functions used to handle periodical time event under different states
 */
#if (SYS_CPNT_STACKING == TRUE)
static void STKTPLG_ENGINE_ProcessInitState(UI32_T *notify_msg)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI8_T                     current_logic_link_status;
    UI32_T                    delta,i=STKTPLG_ENGINE_INVALID_STACKCHIP_DEVICE_ID;
    UI32_T                    up_phy_status, down_phy_status;

    xgs_stacking_debug("Process Init state \n");

    STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus(FALSE);

    /* set stacking button info before no shutdown stacking port
     */
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* init share memory database
     */
    if( STKTPLG_OM_IsStackingButtonChanged())
    {
/* EPR:ES3628BT-FLF-ZZ-00081ES3628BT-FLF-ZZ-00104
     Problem:port 27,28 cannot display in vlan information,when thye are not in stacking mode
     Rootcasue: Not init stktplg board info
                default stacking_button_state is set ,and board info is not set,make them the same
     Solution:make the database equal
     Files :stktplg_om.c,stktplg_engine.c
*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /* set the port attribute
         */
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);

    }
#endif
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    if (1 == ctrl_info_p->reset_state)
    {
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
        if (ctrl_info_p->stacking_button_state == FALSE)
        {
            if (STKTPLG_OM_SetStackingPortOption(STKTPLG_TYPE_STACKING_PORT_OPTION_OFF) == FALSE)
            {
                printf("%s(%d): Failed to set stacking port option\r\n", __FUNCTION__, __LINE__);
            }
        }
        else
        {
            if (STKTPLG_OM_SetStackingPortOption(STKTPLG_TYPE_STACKING_PORT_OPTION_ONE) == FALSE)
            {
                printf("%s(%d): Failed to set stacking port option\r\n", __FUNCTION__, __LINE__);
            }
            
        }

        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
#endif
#if 0
        if(FALSE==DEV_SWDRV_PMGR_EnableStackingChipAllPort())
        {
            printf("%s:DEV_SWDRV_PMGR_EnableStackingChipAllPort fail.\r\n", __FUNCTION__);
        }
#else
        #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
        DEV_SWDRV_PMGR_EnableHgPortAdmin();
        #endif
#endif
        // stktplg_engine_debug_mode = TRUE;

        /* get board information first
         */
        if (!STKTPLG_BOARD_GetBoardInformation(ctrl_info_p->board_id, board_info_p))
        {
            perror("\r\nCan not get related board information.");
            assert(0);

            /* severe problem, while loop here
             */
            while (TRUE);
        }

        ctrl_info_p->preempted_master           = FALSE;
        ctrl_info_p->preempted                  = FALSE;

        #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
        if(FALSE==DEV_SWDRV_PMGR_GetStackChipDeviceID(&i))
        {
            #if (SYS_CPNT_STACKING_BUTTON==TRUE) || (SYS_CPNT_STACKING_BUTTON_SOFTWARE==TRUE)
            if (ctrl_info_p->stacking_button_state)
            #endif
            {
                printf("%s:DEV_SWDRV_PMGR_GetStackChipDeviceID fail\r\n", __FUNCTION__);
            }
        }
        #endif

        ctrl_info_p->stacking_dev_id = (UI8_T)i;

        DBG("%s,line %d:Get stack chip dev id=%d\n", __FUNCTION__, __LINE__, i);

        ctrl_info_p->chip_nums = board_info_p->chip_number_of_this_unit;

        ctrl_info_p->last_module_id = ctrl_info_p->chip_nums;

#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:05:50 */
        ctrl_info_p->my_unit_id = 1;
#else /* SYS_CPNT_STKTPLG_SHMEM */
        STKTPLG_OM_SetMyUnitID(1);
#endif

         /*1223*/
        ctrl_info_p->provision_completed_state = FALSE;


        for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
            ctrl_info_p->exp_module_id[i]=UNKNOWN_MODULE_ID;

        STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

        ctrl_info_p->reset_state++;

        /* Send Hello 0 PDU to both UP/DOWN stacking ports and start their timers */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_1);

    }
    else
    {
        /* Reset chip to prepare re-topology
         */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if(ctrl_info_p->past_role == STKTPLG_STATE_INIT)
#endif
            if(FALSE==DEV_SWDRV_PMGR_ResetOnDemand(TRUE))
            {
                printf("%s:DEV_SWDRV_PMGR_ResetOnDemand fail\r\n", __FUNCTION__);
            }

        /* STKTPLG_ENGINE_ClearCtrlInfo clears information that needed to process incoming packet,
         * before ports are disabled by DEV_SWDRV_ResetOnDemand.
         * sequence is swapped to prevent it.
         */
        STKTPLG_ENGINE_ClearCtrlInfo();
    }

    if (notify_stkctrl_transition == TRUE)
    {
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
        xgs_stacking_debug("STKTPLG_ENGINE_ProcessInitState STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");
    }

    notify_stkctrl_transition = FALSE;

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(UP)\r\n", __FUNCTION__);
    }

    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( &down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_ARBITRATION,up_phy_status,down_phy_status);

    /* Update button status */
    STKTPLG_ENGINE_ButtonPressed();

  //  xgs_stacking_debug("stacking dev id - %d\n", ctrl_info_p->stacking_dev_id);



    /* Send Hello and HBT Type 0 PDUs
     */
    ctrl_info_p->button_pressed_arbitration = FALSE;
    ctrl_info_p->state = STKTPLG_STATE_ARBITRATION;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_ARBITRATION);

    ctrl_info_p->stacking_ports_link_checked = 0;

    ctrl_info_p->reset_state = 10;

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    /*if statcking is enabled, it will send pkt and start pkt timeout timer,or do nothing*/
    if(ctrl_info_p->stacking_button_state)
#endif
    {
        STKTPLG_ENGINE_CheckHelloTimer0(&(ctrl_info_p->stacking_ports_logical_link_status));

        if (FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_UP, &delta))
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HELLO_0_UP, STKTPLG_TIMER_HELLO_TIMEOUT);

        if (FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_DOWN, &delta))
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HELLO_0_DOWN, STKTPLG_TIMER_HELLO_TIMEOUT);

        if (FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_SEND_HELLO, &delta))
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_SEND_HELLO, STKTPLG_TIMER_SEND_HELLO_TIMEOUT);

        if (ctrl_info_p->up_phy_status == TRUE)
            STKTPLG_TX_SendHello(NULL, STKTPLG_HELLO_TYPE_0, LAN_TYPE_TX_UP_LINK);
        if (ctrl_info_p->down_phy_status == TRUE)
            STKTPLG_TX_SendHello(NULL, STKTPLG_HELLO_TYPE_0, LAN_TYPE_TX_DOWN_LINK);


        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

        if (TRUE == up_phy_status)
        {
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, 50 /*STKTPLG_TIMER_HBT0_TIMEOUT*/);
        }

        if (TRUE == down_phy_status)
        {
            /* Send HBT0 to DOWN stacking port, and start HBT0 timer */
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_DOWN, 50 /*STKTPLG_TIMER_HBT0_TIMEOUT*/);
        }
    }
#if 0
    recycle_module_id_idx = 0;
#endif
    return;

}
#else /* #if (SYS_CPNT_STACKING == TRUE) */
static void STKTPLG_ENGINE_ProcessInitState(UI32_T *notify_msg)
{
    STKTPLG_OM_Ctrl_Info_T    *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    UI32_T                    delta,i;
    UI32_T                    up_phy_status, down_phy_status;

    xgs_stacking_debug("Process Init state \n");

    STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus(FALSE);

    if (1 == ctrl_info_p->reset_state)
    {
#if 0
       if(FALSE==DEV_SWDRV_PMGR_EnableStackingChipAllPort())
        {
            printf("%s:DEV_SWDRV_PMGR_EnableStackingChipAllPort fail.\r\n", __FUNCTION__);
        }
#else

          DEV_SWDRV_PMGR_EnableHgPortAdmin();
#endif

        /* get board information first
         */
        if (!STKTPLG_BOARD_GetBoardInformation(ctrl_info_p->board_id, board_info_p))
        {
            perror("\r\nCan not get related board information.");
            assert(0);

            /* severe problem, while loop here
             */
            while (TRUE);
        }

        ctrl_info_p->preempted_master           = FALSE;
        ctrl_info_p->preempted                  = FALSE;
        if(FALSE==DEV_SWDRV_PMGR_GetStackChipDeviceID(&i))
        {
            printf("%s:DEV_SWDRV_PMGR_GetStackChipDeviceID fail\r\n", __FUNCTION__);
        }
        ctrl_info_p->stacking_dev_id = (UI8_T)i;

        DBG("%s,line %d:Get stack chip dev id=%d\n", __FUNCTION__, __LINE__, i);

        ctrl_info_p->chip_nums = board_info_p->chip_number_of_this_unit;

        ctrl_info_p->last_module_id = ctrl_info_p->chip_nums;

#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:06:04 */
        ctrl_info_p->my_unit_id = 1;
#else /* SYS_CPNT_STKTPLG_SHMEM */
        STKTPLG_OM_SetMyUnitID(1);
#endif

        ctrl_info_p->provision_completed_state = FALSE;

        STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

        ctrl_info_p->reset_state++;
    }
    else
    {
        /* Reset chip to prepare re-topology
         */
        if(FALSE==DEV_SWDRV_PMGR_ResetOnDemand(TRUE))
        {
            printf("%s:DEV_SWDRV_PMGR_ResetOnDemand fail\r\n", __FUNCTION__);
        }

        /* STKTPLG_ENGINE_ClearCtrlInfo clears information that needed to process incoming packet,
         * before ports are disabled by DEV_SWDRV_ResetOnDemand.
         * sequence is swapped to prevent it.
         */
        STKTPLG_ENGINE_ClearCtrlInfo();
    }

    if (notify_stkctrl_transition == TRUE)
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

    notify_stkctrl_transition = FALSE ;

    ctrl_info_p->button_pressed_arbitration = FALSE;
    ctrl_info_p->state = STKTPLG_STATE_ARBITRATION;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_ARBITRATION);

    ctrl_info_p->reset_state = 10;

    return;
}
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

#if (SYS_CPNT_STACKING == TRUE)
static void STKTPLG_ENGINE_ProcessArbitrationState(UI32_T *notify_msg)
{
    int i;
    UI32_T  max_option_port_number,max_port_number;
#if (SYS_CPNT_UNIT_HOT_SWAP != TRUE)
    int k;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#if 0
    UI8_T* temp_stk_unit =STKTPLG_OM_ENG_GetUnitCfg();
#endif
    /* modified by Jinhua Wei ,to remove warning ,becaued the variable never used */
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)

    UI32_T start_time,end_time;
    start_time = SYSFUN_GetSysTick();
#endif
    UI8_T current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;
    UI32_T up_phy_status, down_phy_status;

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged())
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        /* EPR: N/A
         Problem:Stacking:DUT displayed Error Messages continuously after

         pressed the Unit4's stack_button off and on.
         Rootcause:(1)when 2 dut stacking stable,slave pressed off stacking

         button and pressed stacking button again
                   (2)slave will send renunberPDU to master and master will
         enter transtion again
                   (3)sometimes slave will hang,because ,slave never clear

         the old master-mac-address,it will consider the same master ,but it

         send enter transition mode to stkctrl,make the whole system stay in

         the transition mode
         Solution: send normal tcn pkt to master instead of renumber pdu
                   when the stacking button changed,always clear the old

         master mac-address
         Files:stktplg_engine.c*/

        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif /* end of #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */

        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

        /*enter transition mode,and re-run*/
        return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
        ctrl_info_p->exp_module_id[i]=UNKNOWN_MODULE_ID;

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(UP)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(0,0,end_time);
#endif

    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(0,1,end_time);
#endif

    /* Check if Hello Type 0 UP and DOWN timers timed-out,
     * and update stacking ports link status
     */
#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if(ctrl_info_p->stacking_button_state)/*if stacking is button is not enable,not send pkt*/
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */
    {
        STKTPLG_ENGINE_CheckHelloTimer0(&(ctrl_info_p->stacking_ports_logical_link_status));
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(0,2,end_time);
#endif
        STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(0,3,end_time);
#endif
    /*  STKTPLG_ENGINE_ButtonPressed(); */
    }

    xgs_stacking_debug("---- Arbitration State --- %d + %d: %ld: %d: %ld (%x: %x) (%d) [%d-%d]\r\n",
                       ctrl_info_p->stacking_dev_id,
                       ctrl_info_p->up_phy_status,
                       (UI32_T)up_phy_status,
                       ctrl_info_p->down_phy_status,
                       (UI32_T)down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id,
                       STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT0_UP),
                       STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT0_DOWN)
                       );

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_ARBITRATION,up_phy_status,down_phy_status);

    /* Switch the box into standalone mode if both ends are open
     */
    /* when stacking port is up,but it cannot receive any pkt,it should be standalone
     */
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking port is not active.
     */
     if (( (FALSE == up_phy_status) && (FALSE == down_phy_status))  ||
        ( !STKTPLG_ENGINE_IS_UPLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) &&
        !STKTPLG_ENGINE_IS_DOWNLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) )||
        FALSE == ctrl_info_p->stacking_button_state)
#else
    if (( (FALSE == up_phy_status) && (FALSE == down_phy_status))  ||
       ( !STKTPLG_ENGINE_IS_UPLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) &&
       !STKTPLG_ENGINE_IS_DOWNLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) ))
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */
    {
        /* Restore Port Mapping and Unit Config
         */
        STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

        /* Change to standalone mode if BOTH stacking ports are open
         */
        ctrl_info_p->state = STKTPLG_STATE_STANDALONE;
        STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_STANDALONE);

        ctrl_info_p->total_units = 1;
        ctrl_info_p->total_units_up   = 1;
        ctrl_info_p->total_units_down = 1;

        ctrl_info_p->is_ring = 0;
        ctrl_info_p->stable_hbt_up.payload[0].start_module_id = 0;
        ctrl_info_p->stable_hbt_up.payload[0].chip_nums = ctrl_info_p->chip_nums;
        ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
        ctrl_info_p->stable_hbt_down.payload[0].chip_nums = ctrl_info_p->chip_nums;
        ctrl_info_p->stable_hbt_up.payload[0].board_id = ctrl_info_p->board_id;
        ctrl_info_p->stable_hbt_down.payload[0].board_id = ctrl_info_p->board_id;


        memcpy(ctrl_info_p->stable_hbt_up.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);

        memcpy(ctrl_info_p->stable_hbt_down.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        memcpy(ctrl_info_p->stable_hbt_up.payload[0].past_master_mac ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
        memcpy(ctrl_info_p->stable_hbt_down.payload[0].past_master_mac ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);

        STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
        STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
        STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);

        STKTPLG_ENGINE_AssignUnitID();

        STKTPLG_OM_UpdateExpectedModuleType(); /* Vincent: For standalone */

        ctrl_info_p->stable_hbt_down.payload[0].unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id ;


#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:06:26 */
        ctrl_info_p->my_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
        STKTPLG_OM_SetMyUnitID(ctrl_info_p->stable_hbt_up.payload[0].unit_id);
#endif
        ctrl_info_p->master_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*EPR:ES3628BT-FLF-ZZ-01059
Problem:stack:mac learning error after hot remove unit from ring

stack.
Rootcause:(1)after hotswap,the module id for each dut will

re-assign.And it may changed
          (2)the mac-address learned on old module id will not be

deleted ,and re-learned for the new module id
          (3)so the pkt will forward to error place
Solution:when do hot-swap,keep the module id no change if the

unit-id not changed
Files:stktplg_engine.c*/
        ctrl_info_p->stable_hbt_down.header.next_unit = 2;
        if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_down, &ctrl_info_p->last_module_id))
        {
            ctrl_info_p->state = STKTPLG_STATE_HALT;

            printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");
            return;
        }

        STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[0]));
        STKTPLG_OM_ENG_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
#else /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
        if(TRUE ==ctrl_info_p->expansion_module_exist)
        {
            /*  Store expansion module information. */
            ctrl_info_p->expansion_module_id    = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id       = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist    = TRUE;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id     = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist  = TRUE;
            printf("\r\n -- Main board : Expansion module info - (%d : %d)\r\n",
                                    ctrl_info_p->expansion_module_type,
                                    ctrl_info_p->expansion_module_id);
            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=ctrl_info_p->chip_nums;


        }
        else
        {
            ctrl_info_p->expansion_module_id                            = UNKNOWN_MODULE_ID;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id   = UNKNOWN_MODULE_ID;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist= FALSE;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = FALSE;
            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=UNKNOWN_MODULE_ID;
        }

        STKTPLG_OM_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);

        if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->expansion_module_type,&module_info_p) && (TRUE ==ctrl_info_p->expansion_module_exist))
        {
            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable,sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
            for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
            {
                port_mapping[k].module_id = ctrl_info_p->chip_nums;
            }
        }
        else
        {
            for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
            {
                port_mapping[k].module_id = UNKNOWN_MODULE_ID;
            }
        }
        STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
#endif /* end of #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
        STKTPLG_ENGINE_ConfigStackInfoToIUC();
        module_swapped = FALSE;
        if((TRUE ==ctrl_info_p->expansion_module_exist )&&(ctrl_info_p->expansion_module_id== ctrl_info_p->chip_nums))
        {
            STKTPLG_ENGINE_EventHandler(ASSIGNID_EV);
        }
        /* Restore stacking ports link to send neighbour Hello Type 0
         */
        ctrl_info_p->stacking_ports_logical_link_status = 0; /*(LAN_TYPE_TX_UP_LINK | LAN_TYPE_TX_DOWN_LINK);*/

        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_PREEMPTED_MASTER);

        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_PREEMPTED_MASTER, STKTPLG_TIMER_PREEMPTED_TIMEOUT);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if((ctrl_info_p->past_role != STKTPLG_STATE_MASTER)&&
           (ctrl_info_p->past_role != STKTPLG_STATE_STANDALONE))
        {
            STKTPLG_ENGINE_MasterStorePastTplgInfo();
            /* Take a snap shot
             */
            STKTPLG_OM_CopyDatabaseToSnapShot();
            *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;

            xgs_stacking_debug("ProcessArbitrationState STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE\n");
        }
        else
        {
            STKTPLG_ENGINE_CheckAndSetProvisionLost();
            *notify_msg = STKTPLG_UNIT_HOT_INSERT_REMOVE;
            xgs_stacking_debug("ProcessArbitrationState STKTPLG_UNIT_HOT_INSERT_REMOVE\n");

        }
        ctrl_info_p->past_role=STKTPLG_STATE_STANDALONE;
        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
        ctrl_info_p->stable_flag = TRUE;
#else /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
         /* Change to standalone mode if BOTH stacking ports are open
         */
        *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(0,4,end_time);
#endif
        return;
    }

    /* Send out Next Hello Type 0 PDU
     */

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if(ctrl_info_p->stacking_button_state)/*if stacking button is not enabled,do not send pkt*/
#endif
    {
        if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
        {
            /* Send HBT Type 0 to UP stacking port if we cannot receive any HBT
             */
            if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT0_UP))
            {
                xgs_stacking_debug(" send HBT0 \r\n");
                ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_UP] = 0;
                STKTPLG_TX_SendHBTType0(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,0);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, 50 /*STKTPLG_TIMER_HBT0_TIMEOUT*/);
            }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(0,5,end_time);
#endif
        }
        else
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);

            if (current_logic_link_status != ctrl_info_p->stacking_ports_logical_link_status)
            {
                if (ctrl_info_p->reset_state < 90)
                    ctrl_info_p->reset_state = 95;
            }
    #if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(0,6,end_time);
    #endif
        }

        if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
        {
            /* Send HBT Type 0 to DOWN stacking port if we cannot receive any HBT
             */
            if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT0_DOWN))
            {
                xgs_stacking_debug("Send HBT 0\n");
                ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_DOWN] = 0;
                STKTPLG_TX_SendHBTType0(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,1);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_DOWN, 100 /*STKTPLG_TIMER_HBT0_TIMEOUT*/);
            }
        }
        else
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

            if (current_logic_link_status != ctrl_info_p->stacking_ports_logical_link_status)
            {
                if (ctrl_info_p->reset_state < 90)
                    ctrl_info_p->reset_state = 95;
            }
        }
    #if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(0,7,end_time);
    #endif
    }
    return;

}
#else /* #if (SYS_CPNT_STACKING == TRUE) */
static void STKTPLG_ENGINE_ProcessArbitrationState(UI32_T *notify_msg)
{
    int i,k;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();

    xgs_stacking_debug("---- Arbitration State --- %d\r\n", ctrl_info_p->my_unit_id);

    /* Restore Port Mapping and Unit Config
     */
    STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

    /* Change to standalone mode
     */
    *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
    ctrl_info_p->state = STKTPLG_STATE_STANDALONE;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_STANDALONE);

    ctrl_info_p->total_units = 1;
    ctrl_info_p->total_units_up   = 1;
    ctrl_info_p->total_units_down = 1;

    ctrl_info_p->is_ring = 0;
    ctrl_info_p->stable_hbt_up.payload[0].start_module_id = 0;
    ctrl_info_p->stable_hbt_up.payload[0].chip_nums = ctrl_info_p->chip_nums;
    ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
    ctrl_info_p->stable_hbt_down.payload[0].chip_nums = ctrl_info_p->chip_nums;
    ctrl_info_p->stable_hbt_up.payload[0].board_id = ctrl_info_p->board_id;
    ctrl_info_p->stable_hbt_down.payload[0].board_id = ctrl_info_p->board_id;

    memcpy(ctrl_info_p->stable_hbt_up.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);

    memcpy(ctrl_info_p->stable_hbt_down.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);

    STKTPLG_ENGINE_AssignUnitID();

    ctrl_info_p->stable_hbt_down.payload[0].unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id ;

#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:06:55 */
    ctrl_info_p->my_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
    STKTPLG_OM_SetMyUnitID(ctrl_info_p->stable_hbt_up.payload[0].unit_id);
#endif
    ctrl_info_p->master_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;

    ctrl_info_p->expansion_module_id                            = UNKNOWN_MODULE_ID;
    ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id   = UNKNOWN_MODULE_ID;
    ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist= FALSE;
    ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
    ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = FALSE;
    ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=UNKNOWN_MODULE_ID;

    return;
}
#endif

static void STKTPLG_ENGINE_ProcessStandAloneState(UI32_T *notify_msg)
{
#if (SYS_CPNT_STACKING == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI32_T up_phy_status, down_phy_status;
    UI8_T current_logic_link_status;
    int k;
    UI32_T  max_port_number,max_option_port_number;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
    TRAP_EVENT_TrapData_T trap_data;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    xgs_stacking_debug("Process Standalone state \n");

    /*
     * Get physical link state for both stacking ports
     */

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged())
    {
      /*change the board.c*/
       STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
      /*set the port attribute*/
      DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);

      /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
      STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

      memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
      STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
      *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

       /*enter transition mode,and re-run*/
      return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP)\r\n", __FUNCTION__,__LINE__);
    }
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

 /* Vincent: Because of timing issue the modules
  * might be discovered after master enter master state. Thus we
  * have to wait few more seconds to disable hot swap feature
  */

#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == FALSE)
    if (master_state_counter < MAX_HOT_SWAP_TIME)
#endif
    {
        master_state_counter++;
        if(FALSE ==ctrl_info_p->expansion_module_exist)
        {
            if(TRUE==STKTPLG_OM_ExpModuleIsInsert(ctrl_info_p->my_unit_id))
            {

                STKTPLG_OM_RemoveExpModule(ctrl_info_p->my_unit_id);
            }
        }

        if(TRUE ==ctrl_info_p->expansion_module_exist && module_swapped == TRUE)
        {
            /*  Store expansion module information.
             */
            module_swapped = FALSE;
            ctrl_info_p->expansion_module_id    = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id    = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = TRUE;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id    = ctrl_info_p->chip_nums;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = TRUE;
            STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);

#else
        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
#endif
            if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->expansion_module_type,&module_info_p))
            {
                memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable,sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
                for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
                {
                    port_mapping[k].module_id = ctrl_info_p->chip_nums;
                }
                STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
            }
            /* standalone
             */
            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=ctrl_info_p->chip_nums;
            STKTPLG_ENGINE_ConfigStackInfoToIUC();

        }
        else if(FALSE ==ctrl_info_p->expansion_module_exist && module_swapped == TRUE )
        {
            module_swapped = FALSE;
            ctrl_info_p->expansion_module_id    = 0xff;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id    = 0xff;
            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = FALSE;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id    = 0xff;
            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = FALSE;

            STKTPLG_OM_GetPortMapping(port_mapping,ctrl_info_p->my_unit_id);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
            STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);

#else
            STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
            STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
#endif
            for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
            {
                port_mapping[k].module_id = 0xff;
            }
            STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);

            /*standalone */
            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=0xff;
            STKTPLG_ENGINE_ConfigStackInfoToIUC();
        }
    } /* end of if (master_state_counter < MAX_HOT_SWAP_TIME) */
    if(ctrl_info_p->expansion_module_ready && module_swapped == FALSE)
    {
        if(FALSE==STKTPLG_OM_ExpModuleIsInsert(ctrl_info_p->my_unit_id))
        {
            STKTPLG_OM_InsertExpModule(ctrl_info_p->my_unit_id,ctrl_info_p->expansion_module_type,ctrl_info_p->module_runtime_fw_ver);
        }
    }


    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_OFF,up_phy_status,down_phy_status);

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
     /* if stacking is button is not enable,not send pkt and start the pkt
      * timer
      */
     if(ctrl_info_p->stacking_button_state)
#endif
     {
        /* Check if Hello Type 0 timer time-out
         */
        STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);

        STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);

       /* xgs_stacking_debug("---- Standalone State --- %d + %d: %ld: %d: %ld (%x: %x) (%d)\r\n",
                           ctrl_info_p->stacking_dev_id,
                           ctrl_info_p->up_phy_status,
                           up_phy_status,
                           ctrl_info_p->down_phy_status,
                           down_phy_status,
                           current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                           ctrl_info_p->my_unit_id); */

        /* Check if Hello Type 1 timer time-out
         */
        STKTPLG_ENGINE_CheckHelloTimer1(ctrl_info_p);

        /* phy status change from off to on, will not trigger TCN.
         */  
#if 0          
        if( ((FALSE == ctrl_info_p->up_phy_status  ) && (TRUE == up_phy_status  )) ||
            ((FALSE == ctrl_info_p->down_phy_status) && (TRUE == down_phy_status)) )
        {
            STKTPLG_ENGINE_LogMsg("[TCN] Standalone: Stacking link up\n");
            tcnReason = 3 ;
            #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("ProcessStandAloneState TOPO change\n");
            #else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
            #endif
            return;
        }
#endif  
    }

    if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_PREEMPTED_MASTER))
    {
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_PREEMPTED_MASTER);

        ctrl_info_p->stable_hbt_up.payload[0].preempted = TRUE;

        ctrl_info_p->stable_hbt_down.payload[0].preempted = TRUE;

        ctrl_info_p->preempted = TRUE;

        ctrl_info_p->preempted_master = TRUE;

        printf("---- Master state preemption\r\n");
    }

    if ((tcnReason !=0) && (STKTPLG_OM_IsProvisionCompleted() == TRUE))
    {
        trap_data.trap_type=TRAP_EVENT_TCN;
        trap_data.community_specified=FALSE;
        trap_data.u.tcn.tcnReason = tcnReason;
#if (SYS_CPNT_TRAPMGMT == TRUE)
        TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);

#endif
        tcnReason = 0;
    }
    ctrl_info_p->stacking_ports_logical_link_status = current_logic_link_status;
    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;

    ctrl_info_p->stack_maintenance = TRUE;
    stktplg_stackingdb_updated = TRUE;
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

    return;
}

static void STKTPLG_ENGINE_ProcessMasterSyncState(UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    UI8_T  current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;
    UI32_T up_phy_status, down_phy_status;
    UI32_T delta;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time = SYSFUN_GetSysTick();
#endif

    xgs_stacking_debug("Process Master Sync state \n");

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

        /*enter transition mode,and re-run*/
        return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState( (UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP)\r\n", __FUNCTION__,__LINE__);
    }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,0,end_time);
#endif
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,1,end_time);
#endif

    /* Check if Hello 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,2,end_time);
#endif

    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p,up_phy_status,down_phy_status);

    xgs_stacking_debug("---- Master Sync State--- %d + %d: %ld: %d: %ld (%x: %x) (%d)\r\n",
                       ctrl_info_p->stacking_dev_id,
                       ctrl_info_p->up_phy_status,
                       (UI32_T)up_phy_status,
                       ctrl_info_p->down_phy_status,
                       (UI32_T)down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,3,end_time);
#endif

    /*add on 1220 */
    if(ctrl_info_p->expansion_module_ready && module_swapped==FALSE)
    {
        if(FALSE==STKTPLG_OM_ExpModuleIsInsert(ctrl_info_p->my_unit_id))
        {
            STKTPLG_OM_InsertExpModule(ctrl_info_p->my_unit_id,ctrl_info_p->expansion_module_type,ctrl_info_p->module_runtime_fw_ver);
        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(1,4,end_time);
#endif
    }
    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_MASTER,up_phy_status,down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,5,end_time);
#endif
    /* Check if both stacking ports are DOWN
     */
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) &&
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )                     
    {
        STKTPLG_ENGINE_LogMsg("[TCN]Mastrer Sync: Both Stacking link down\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessMasterSyncState_1 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        STKTPLG_ENGINE_ClearCtrlInfo();

       /* Restore stacking ports link to send neighbour Hello Type 0
        */
        ctrl_info_p->stacking_ports_logical_link_status = 0;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(1,6,end_time);
#endif
        return;

    }

    /* Check if either stacking ports is DOWN
     */

    STKTPLG_ENGINE_Check_Stacking_Link_Down(current_logic_link_status,FALSE,notify_msg);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,7,end_time);
#endif
    /* Check if button state has been changed
     */
    if (TRUE == STKTPLG_ENGINE_TopologyChangedByButton(ctrl_info_p) && FALSE == ctrl_info_p->button_pressed)
    {
        if (FALSE == STKTPLG_ENGINE_IsAbleToBeNextMaster() || TRUE == STKTPLG_ENGINE_CheckAllUnitsButtonStatus())
        {
            /* Current Master is enabled by button pressed,
             * but it is not a absolute Master in term of MAC address.
             */
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN);
            STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Button status Changed\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("ProcessMasterSyncState_2 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(1,8,end_time);
#endif
            return;
        }
    }
    ctrl_info_p->stable_hbt_up.payload[0].button_pressed   = ctrl_info_p->button_pressed;
    ctrl_info_p->stable_hbt_down.payload[0].button_pressed = ctrl_info_p->button_pressed;

    /* check if there is any topology change or HBT1 timeout
     */
    if ( (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP  )) ||
         (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN)) )
    {
        /* stop the timer
         */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

        xgs_stacking_debug("HBT1 timeout master sync state reset\r\n");

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: HBT1 Timeout\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessMasterSyncState_3 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(1,9,end_time);
#endif
        return;
    }

    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;
    if ( (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) ||
         (TRUE == ctrl_info_p->up_phy_status))
    {
        /* Send HBT 1 to UP stacking port if there are units connected
         * AND for open ended stack only
         */
        if ( (FALSE == ctrl_info_p->is_ring) && (ctrl_info_p->total_units_up > 1) )
        {
            if(FALSE==STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HBT1_UP,&delta))
            {
                STKTPLG_TX_SendHBTType1(FALSE ,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,7);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
/*EPR:ES3628BT-FLF-ZZ-00545
Problem: when do hotswap,sometimes it will print error info,and cannot stacking well
RootCause:sometime the STKTPLG_HBT_TYPE_1_REBOUND will received in the topo state for the pkt
          received after ctrl_info_p->bounce_msg is set 3.
Solution:when send hbt1 pkt in master_sync,clear ctrl_info_p->bounce_msg
File:stktplg_engine.c*/
                ctrl_info_p->bounce_msg &=( ~1);
            }
        }
    }
    if ( (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) ||
         (TRUE == ctrl_info_p->down_phy_status))
    {
        /* Send HBT 1 to DOWN stacking port
         */
        if (ctrl_info_p->total_units_down > 1)
        {
          /*check if timer is not active which set when receive the HBT1pkt in master_sync state,
                              if not check it will sent HBT1 again, and */
           if(FALSE==STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HBT1_DOWN,&delta))
            {
                STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,8);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
/*EPR:ES3628BT-FLF-ZZ-00545
Problem: when do hotswap,sometimes it will print error info,and cannot stacking well
RootCause:sometime the STKTPLG_HBT_TYPE_1_REBOUND will received in the topo state for the pkt
          received after ctrl_info_p->bounce_msg is set 3.
Solution:when send hbt1 pkt in master_sync,clear ctrl_info_p->bounce_msg
File:stktplg_engine.c*/
                ctrl_info_p->bounce_msg &=( ~2);
            }
        }
    }

#endif /* end of #if (SYS_CPNT_STACKING == TRUE)   */

    #if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(1,10,end_time);
    #endif

    return;
}


static void STKTPLG_ENGINE_ProcessGetTopologyInfoState(UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI8_T current_logic_link_status, pre_logic_link_status;
    UI32_T up_phy_status, down_phy_status;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    xgs_stacking_debug("Process Get Topology Info state \n");


    /*
     * Get physical link state for both stacking ports
     */

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
      STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

      memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
      STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
      *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

       /*enter transition mode,and re-run*/
      return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState( (UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP)\r\n", __FUNCTION__,__LINE__);
    }
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState((UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }


    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    /* Check if Hello 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);

    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);

    /*xgs_stacking_debug("---- Got Topology State --- %d + %d: %ld: %d: %ld (%x: %x) (%d)\r\n",
                       ctrl_info_p->stacking_dev_id,
                       ctrl_info_p->up_phy_status,
                       up_phy_status,
                       ctrl_info_p->down_phy_status,
                       down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id); */

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_MASTER,up_phy_status,down_phy_status);

    /* Check if both stacking ports are DOWN
     */
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) &&
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )                   
    {
        STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: Both stacking link down\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessTopoInfoState_1 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        STKTPLG_ENGINE_ClearCtrlInfo();

        /* Restore stacking ports link to send neighbour Hello Type 0
         */
        ctrl_info_p->stacking_ports_logical_link_status = 0;

        return;
    }

    /* Check if either stacking ports is DOWN
     */
    pre_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;     
    STKTPLG_ENGINE_Check_Stacking_Link_Down(current_logic_link_status,FALSE,notify_msg);

    /* Check if button state has been changed
     */
    if (TRUE == STKTPLG_ENGINE_TopologyChangedByButton(ctrl_info_p) && FALSE == ctrl_info_p->button_pressed)
    {
        if (FALSE == STKTPLG_ENGINE_IsAbleToBeNextMaster() || TRUE == STKTPLG_ENGINE_CheckAllUnitsButtonStatus())
        {
            /* Current Master is enabled by button pressed,
             * but it is not a absolute Master in term of MAC address.
             */
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN);
            STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: Button Changed\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("ProcessTopoInfoState_2 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

            ctrl_info_p->reset_state = 20;

            return;
        }
    }
    ctrl_info_p->stable_hbt_up.payload[0].button_pressed   = ctrl_info_p->button_pressed;
    ctrl_info_p->stable_hbt_down.payload[0].button_pressed = ctrl_info_p->button_pressed;

    /* check if there is any topology change or HBT2 timeout
     */
    if ( (pre_logic_link_status != current_logic_link_status      ) ||
         (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_GET_TPLG_INFO_UP)  ) ||
         (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_GET_TPLG_INFO_DOWN)) )
    {
        /* stop the timer
         */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN);

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: HBT2 Timeout\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessTopoInfoState_3 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        return;
    }

#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

    return;
}


static void STKTPLG_ENGINE_ProcessSlaveWaitAssignState(UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI8_T current_logic_link_status;
    UI32_T up_phy_status, down_phy_status;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time =SYSFUN_GetSysTick();
#endif

    xgs_stacking_debug("Process Slave Wait Assign state \n");

    /*
     * Get physical link state for both stacking ports
     */
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

        /*enter transition mode,and re-run*/
        return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(UP)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,0,end_time);
#endif
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,1,end_time);
#endif
    /* Check if Hello 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,2,end_time);
#endif
    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,3,end_time);
#endif
  /*  xgs_stacking_debug("---- Slave Wait State --- %d + %d: %ld: %d: %ld (%x: %x) (%d)\r\n",
                       ctrl_info_p->stacking_dev_id,
                       ctrl_info_p->up_phy_status,
                       up_phy_status,
                       ctrl_info_p->down_phy_status,
                       down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id); */

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_SLAVE,up_phy_status,down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,4,end_time);
#endif
    /* check if there is any topology change
     */
    if ( ((TRUE == ctrl_info_p->up_phy_status  ) && (FALSE == up_phy_status)  ) ||
         ((TRUE == ctrl_info_p->down_phy_status) && (FALSE == down_phy_status)) ||
         (TRUE == STKTPLG_ENGINE_TopologyChangedByButton(ctrl_info_p)         ) ||
         ((TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP)  ) ||
          (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))       ) )
    {
        STKTPLG_ENGINE_LogMsg("[TCN] Slave Wait: stacking link down or button changed or HBT1 Timeout \r\n");
        /* stop the timer
         */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration
         */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessSlaveWaitState TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        ctrl_info_p->reset_state = 20;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(2,5,end_time);
#endif
        return;
    }
    ctrl_info_p->stacking_ports_logical_link_status = current_logic_link_status;  
    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;

    /* Send Next Hello 0 PDU
     */

#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

    #if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(2,6,end_time);
    #endif

    return;
}


static void STKTPLG_ENGINE_ProcessMasterState(UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI8_T current_logic_link_status;
    UI32_T up_phy_status, down_phy_status;
    TRAP_EVENT_TrapData_T trap_data;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time = SYSFUN_GetSysTick();
#endif
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    xgs_stacking_debug("Process Master state \n");

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
     /* check if stacking button state changed,if changed change to arbitration
      * state
      */
     if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
     {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

        /*enter transition mode,and re-run*/
        return;
     }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */

    /*
     * Get physical link state for both stacking ports
     */
    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState( (UI32_T*)&up_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(UP)\r\n", __FUNCTION__);
    }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,0,end_time);
#endif

    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,1,end_time);
#endif


    /* Check if Hello Type 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,2,end_time);
#endif
    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,3,end_time);
#endif
 /*   printf("---- Master State --- %d + %d: %ld: %d: %ld (%x: %x) (%d) (%d) [%d - %d]\r\n",
                       ctrl_info_p->stacking_dev_id,
                       ctrl_info_p->up_phy_status,
                       up_phy_status,
                       ctrl_info_p->down_phy_status,
                       down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id,
                       ctrl_info_p->preempted,
                       STKTPLG_MGR_IsProvisionCompleted(), ctrl_info_p->stack_maintenance); */

    /* Check if Hello Type 1 timer time-out
     */
    /*add on 1220 */
    if(ctrl_info_p->expansion_module_ready && module_swapped==FALSE)
    {
        if(FALSE==STKTPLG_OM_ExpModuleIsInsert(ctrl_info_p->my_unit_id))
        {
            STKTPLG_OM_InsertExpModule(ctrl_info_p->my_unit_id,ctrl_info_p->expansion_module_type,ctrl_info_p->module_runtime_fw_ver);
        }
    }
    STKTPLG_ENGINE_CheckHelloTimer1(ctrl_info_p);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,4,end_time);
#endif

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_MASTER,up_phy_status,down_phy_status);

   /* if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP))
        xgs_stacking_debug("---master state HBT 1 up timeout----\r\n");

    if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))
        xgs_stacking_debug("---master state HBT 1 down timeout----\r\n"); */

    /* Check if button state has been changed
     */
    if (TRUE == STKTPLG_ENGINE_TopologyChangedByButton(ctrl_info_p) && FALSE == ctrl_info_p->button_pressed)
    {
        if (FALSE == STKTPLG_ENGINE_IsAbleToBeNextMaster() || TRUE == STKTPLG_ENGINE_CheckAllUnitsButtonStatus())
        {
            /* Current master is enabled by button pressed
             * it is NOT a absolute Master in term of MAC address.
             */
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);
            STKTPLG_ENGINE_LogMsg("[TCN] Master: Button Changed\n");
            tcnReason = 1 ;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("ProcessMasterState_1 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
            xgs_stacking_debug("ProcessMasterState STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(3,5,end_time);
#endif
            return;
        }
    }

    /* Store button state
     */
    /*  ctrl_info_p->stable_hbt_up.payload[0].button_pressed   = ctrl_info_p->button_pressed;
    ctrl_info_p->stable_hbt_down.payload[0].button_pressed = ctrl_info_p->button_pressed; */

    /* Check if both stacking ports are DOWN
     */
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) &&
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )                          
    {

        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);
        STKTPLG_ENGINE_LogMsg("[TCN] Master: both links down\n");
        tcnReason = 2;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessMasterState_2 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(3,6,end_time);
#endif
        return;
    }

    /* Check if either stacking port link state has been changed
     */
    STKTPLG_ENGINE_Check_Stacking_Link_Down(current_logic_link_status,TRUE,notify_msg);

    if ( (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP)  ) ||
         (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN)) )
    {
        if (TRUE == STKTPLG_OM_IsProvisionCompleted())
        {
            if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP))
            {
                hbt1_up_timeout_count ++;
            }

            if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))
            {
                hbt1_down_timeout_count ++;
            }

            if (hbt1_up_timeout_count >= STKTPLG_HBT1_TIMEOUT_MAX_COUNT ||
                hbt1_down_timeout_count >= STKTPLG_HBT1_TIMEOUT_MAX_COUNT )
            {
                if (TRUE == stktplg_engine_TCN_mode)
                {
                    STKTPLG_ENGINE_LogMsg("[TCN] Master: HBT1 timeout\r\n");
                }

                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);

                if (TRUE == stktplg_engine_TCN_mode)
                {
                    tcnReason = 4;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                    xgs_stacking_debug("ProcessMasterState_3 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
                    *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
                }
            }
            else
            {
                printf("Master: HBT1 timeout\r\n");
                if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP))
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                }
                if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                }
            }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(3,7,end_time);
#endif
            return;
        } /* End of if (TRUE == STKTPLG_MGR_IsProvisionCompleted) */
    }

    /* Check if Master Preemption Timer expired
     */
    if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_PREEMPTED_MASTER))
    {
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_PREEMPTED_MASTER);

        ctrl_info_p->stable_hbt_up.payload[0].preempted = TRUE;

        ctrl_info_p->stable_hbt_down.payload[0].preempted = TRUE;

        ctrl_info_p->preempted = TRUE;

        ctrl_info_p->preempted_master = TRUE;

        printf("---- Master state preemption\r\n");
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
   end_time = SYSFUN_GetSysTick()-start_time;
   STKTPLG_ENGINE_BD_SetTick(3,8,end_time);
#endif
    /* check if we need to send a new HBT1 for stack maintenance
     */
    if (FALSE == ctrl_info_p->stack_maintenance)
    {
        printf("****** Master sending maintenance packet\r\n");
        ctrl_info_p->stack_maintenance = TRUE;

        ctrl_info_p->stable_hbt_up.payload[0].master_provision_completed   = TRUE;
        ctrl_info_p->stable_hbt_down.payload[0].master_provision_completed = TRUE;

        /* Send out HBT type 1 and start related timer
         */
        if ( (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) ||
             (TRUE == ctrl_info_p->up_phy_status                            ))
        {
            /* Send HBT 1 to UP stacking port if there are units connected
             * AND for open ended stack only
             */
            if ( (FALSE == ctrl_info_p->is_ring) && (ctrl_info_p->total_units_up > 1) )
            {
                STKTPLG_TX_SendHBTType1(FALSE ,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,0);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
                hbt1_up_timeout_count = 0 ;
            }
        }

        if ( (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) ||
             (TRUE == ctrl_info_p->down_phy_status                              ))
        {
            /* Send HBT 1 to DOWN stacking port
             */
            if (ctrl_info_p->total_units_down > 1)
            {
                STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,1);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
                hbt1_down_timeout_count = 0 ;
            }
        }
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,9,end_time);
#endif
/*EPR:N/A
    Problem:when 3dut stacking ,and master in the middle, remove a slave ,the master will not do hot-remove
    RootCause: 1,2 dut stacking stable,add a slave make the topo line and master in the middle
                             if processMasterstate happend first,make up_new_link = TRUE,ctrl_info_p->stacking_ports_logical_link_status  is set by timer
                             or other type pkt instead of receive hello pkt,up_new_link is always true.
                             Then remove a dut,master will not do hot-remove
    Solution: up_new_link/down_new_link is useless. Always set up_new_link/down_new_link FALSE
    File:stktplg_engine.c
*/

#if 0
    if (ctrl_info_p->up_phy_status==FALSE && up_phy_status==TRUE)
        up_new_link = TRUE;
    if (ctrl_info_p->down_phy_status==FALSE && down_phy_status==TRUE)
    down_new_link = TRUE;
#endif

    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;


    if (TRUE == ctrl_info_p->stack_maintenance)
    {
        stktplg_stackingdb_updated = TRUE;
        if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_M))
        {
            /* Send if both HBT1s have been returned
             */
            if (FALSE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP))
            {
                /* stop the timer
                 */
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);

                /* time to send a new HBT1 for topology check
                 */
                if (LAN_TYPE_TX_UP_LINK == (LAN_TYPE_TX_UP_LINK & ctrl_info_p->stacking_ports_logical_link_status)
                    && ctrl_info_p->is_ring == FALSE)
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,2);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
                }
            }
            else
                printf("---- STKTPLG_TIMER_HBT1_UP timeout\r\n");
        }

        if (STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_M_DOWN))
        {
            /* Send if both HBT1s have been returned
             */
            if (FALSE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))
            {
                /* stop the timer
                 */
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);

                /* time to send a new HBT1 for topology check
                 */
                if (LAN_TYPE_TX_DOWN_LINK == (LAN_TYPE_TX_DOWN_LINK & ctrl_info_p->stacking_ports_logical_link_status))
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                    STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,3);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
                }
            }
            else
                printf("---- STKTPLG_TIMER_HBT1_DOWN timeout\r\n");
        }

        /* In master maintaince state, check need to send TPLG_SYNC
         */
        STKTPLG_ENGINE_CheckTplgSyncStatus();
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(3,10,end_time);
#endif
    if ((tcnReason !=0) && (STKTPLG_OM_IsProvisionCompleted() == TRUE))
    {
        trap_data.trap_type=TRAP_EVENT_TCN;
        trap_data.community_specified=FALSE;
        trap_data.u.tcn.tcnReason = tcnReason;
        printf("Send Trap \r\n");
#if (SYS_CPNT_TRAPMGMT == TRUE)
        TRAP_MGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#else
        SNMP_PMGR_ReqSendTrapOptional(&trap_data, TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP);
#endif
        tcnReason = 0;
    }

#endif /* End of SYS_CPNT_STACKING */

    return;
}


static void STKTPLG_ENGINE_ProcessSlaveState(UI32_T *notify_msg)
{
#if (SYS_CPNT_STACKING == TRUE)

    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI8_T current_logic_link_status;
    UI32_T up_phy_status, down_phy_status;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time= SYSFUN_GetSysTick();
#endif
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    xgs_stacking_debug("Process Slave state \n");

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;

        /*enter transition mode,and re-run*/
        return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */
    /*
     * Get physical link state for both stacking ports
     */
    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState( (UI32_T*)&up_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(UP)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,0,end_time);
#endif

    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,1,end_time);
#endif


    /* Check if Hello Type 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,2,end_time);
#endif

    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p, up_phy_status, down_phy_status);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,3,end_time);
#endif

   /* xgs_stacking_debug("---- Slave State --- %d + %d: %ld: %d: %ld (%x: %x) (%d) (%d) [%d]\r\n",
                       ctrl_info_p->stacking_dev_id, ctrl_info_p->up_phy_status,
                       up_phy_status,
                       ctrl_info_p->down_phy_status,
                       down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status,
                       ctrl_info_p->my_unit_id,
                       ctrl_info_p->preempted,
                       ctrl_info_p->stable_hbt_up.payload[0].button_pressed); */

    /* Check if Hello Type 1 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer1(ctrl_info_p);

    /* Show up LEDs and 7-segment
     */
    STKTPLG_MGR_SetStackRoleLed(LEDDRV_STACK_SLAVE,up_phy_status,down_phy_status);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,4,end_time);
#endif
    /* check if there is any topology change
     */
    if (TRUE == STKTPLG_ENGINE_TopologyChangedByButton(ctrl_info_p) && TRUE == ctrl_info_p->button_pressed )
    {
        if (TRUE == STKTPLG_ENGINE_IsAbleToBeNextMaster() || FALSE ==ctrl_info_p->stable_hbt_up.payload[0].button_pressed)
        {
            /* Current master is enabled by button pressed
             * it is NOT a absolute Master in term of MAC address.
             */

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

            /* Both stacking ports are opened
            */
            STKTPLG_ENGINE_LogMsg("[TCN] Slave: Button Changed\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("ProcessSlaveState_1 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
            xgs_stacking_debug("ProcessSlaveState STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(4,5,end_time);
#endif
            return;
        }
    }

    /* Check if both stacking ports are DOWN
     */
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) &&
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )    
    {
        STKTPLG_ENGINE_LogMsg("[TCN] Slave : both stacking links down \n");
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessSlaveState_2 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(4,6,end_time);
#endif
        return;
    } 
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) ||
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )      
    {
        if ( TRUE == ctrl_info_p->is_ring )
        {
            /* Closed Loop stack
             */
            if ( (TRUE == STKTPLG_ENGINE_NextUpFromMaster()) &&
                   (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK) && 
                   (current_logic_link_status & LAN_TYPE_TX_UP_LINK))) 
            {
                /* Link status changed to DOWN in Up stacking ports of Master
                 * (Down stacking port of this Slave Unit) TCN necessary
                 */
                xgs_stacking_debug("...... Slave Unit detects cable removal in DOWN port within the closed loop\r\n");
            }
            else if ( (TRUE == STKTPLG_ENGINE_NextDownFromMaster()) &&
                      ( (current_logic_link_status & LAN_TYPE_TX_DOWN_LINK   ) && 
                        (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK)) ) )                      
            {
                /* Link status changed to DOWN in Down stacking ports of Master
                 * (Up stacking port of this Slave Unit) TCN necessary ??
                 */
                printf("...... Slave detects cable removal in UP port within the closed loop\r\n");
            }
            else if(!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK) && 
                     (current_logic_link_status & LAN_TYPE_TX_UP_LINK)     )
            {
                UI32_T done, unit;

                /* Link status changed in either stacking ports of this slave unit
                 * which is NOT sitting next to Master
                 */
                printf("...... Slave in the Middle detects link Down in Down port within the closed loop\r\n");

                done = 0;
                for(unit = 1; (unit < ctrl_info_p->total_units_down) && (0 == done); unit++)
                {
                    if (ctrl_info_p->my_unit_id == ctrl_info_p->stable_hbt_down.payload[unit].unit_id)
                        done = 1;
                }
                if (1 == done)
                {
                    int j;

                    ctrl_info_p->total_units_down = unit ;
                    ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;

                    ctrl_info_p->total_units_up = ctrl_info_p->total_units - unit + 1;
                    ctrl_info_p->stable_hbt_up.header.next_unit = ctrl_info_p->total_units_up + 1;

                    for (j = ctrl_info_p->total_units_up - 1; j > 0; j--)
                    {
                        xgs_stacking_debug("[%s]%d up[%d] = %d\r\n",__FUNCTION__,__LINE__, j, ctrl_info_p->stable_hbt_up.payload[j].unit_id);
                    }

                    for (j = 0; j < ctrl_info_p->total_units_down; j++)
                    {
                        xgs_stacking_debug(" [%s]%d down[%d] = %d\r\n",__FUNCTION__,__LINE__, j, ctrl_info_p->stable_hbt_down.payload[j].unit_id);
                    }

                    /* Notify neighbors for closed loop re-topology
                     */
                    ctrl_info_p->is_ring = FALSE;

                    STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);

                    STKTPLG_ENGINE_ConfigStackInfoToIUC();
                }
            }

            ctrl_info_p->up_phy_status   = up_phy_status;
            ctrl_info_p->down_phy_status = down_phy_status;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(4,7,end_time);
#endif
        }
        else /* not ring */
        {
            if ((TRUE == ctrl_info_p->down_phy_status)&& (FALSE == down_phy_status))
            {
                if (memcmp(ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_down-1].mac_addr,
                           ctrl_info_p->my_mac,
                           STKTPLG_MAC_ADDR_LEN) != 0)
                {
                    /* Open-ended with link status changed in either stacking ports
                     * stop the timer
                     */
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                    /* change state to arbitration
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Slave: Down stacking link down\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                    xgs_stacking_debug("ProcessSlaveState_3 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
                    *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                    end_time = SYSFUN_GetSysTick()-start_time;
                    STKTPLG_ENGINE_BD_SetTick(4,8,end_time);
#endif
                    return;
                }
            }
            else /* if (FALSE == up_phy_status) */
            {
                if (memcmp(ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->total_units_up-1].mac_addr,
                           ctrl_info_p->my_mac,
                           STKTPLG_MAC_ADDR_LEN) != 0)
                {

                    /* Open-ended with link status changed in either stacking ports
                     * stop the timer
                     */
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                    /* change state to arbitration
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Slave: Up stacking link down\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                    xgs_stacking_debug("ProcessSlaveState_4 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
                    *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

                    return;
                }
           }
        } /* not ring */
    }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,9,end_time);
#endif
/*EPR:N/A
    Problem:when 3dut stacking ,and master in the middle, remove a slave ,the master will not do hot-remove
    RootCause: 1,2 dut stacking stable,add a slave make the topo line and master in the middle
                             if processMasterstate happend first,make up_new_link = TRUE,ctrl_info_p->stacking_ports_logical_link_status  is set by timer
                             or other type pkt instead of receive hello pkt,up_new_link is always true.
                             Then remove a dut,master will not do hot-remove
    Solution: up_new_link/down_new_link is useless. Always set up_new_link/down_new_link FALSE
    File:stktplg_engine.c
*/
#if 0
    if (ctrl_info_p->up_phy_status==FALSE && up_phy_status==TRUE)
        up_new_link = TRUE;
    if (ctrl_info_p->down_phy_status==FALSE && down_phy_status==TRUE)
    down_new_link = TRUE;
#endif

    ctrl_info_p->stacking_ports_logical_link_status = current_logic_link_status;
    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;
    /* Send Hello Type 0 PDU to enquire stacking ports' status
     */

    if (TRUE == ctrl_info_p->is_ring)
    {
        if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN))
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

            if (TRUE == stktplg_engine_TCN_mode)
            {
                STKTPLG_ENGINE_LogMsg("[TCN] Slave: HBT1 timeout - closed loop\r\n");

                /* change state to arbitration
                 */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                xgs_stacking_debug("ProcessSlaveState_5 TOPO change\n");
#else
                STKTPLG_ENGINE_ReturnToArbitrationState(0);
                *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
            }
        }
    }
    else
    {
        if ( (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_UP  )) ||
             (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HBT1_DOWN)) )
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

            if (TRUE == stktplg_engine_TCN_mode)
            {
                STKTPLG_ENGINE_LogMsg("[TCN] Slave : HBT1 timeout - Line\r\n");

                /* change state to arbitration
                 */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                xgs_stacking_debug("ProcessSlaveState_6 TOPO change\n");
#else
                STKTPLG_ENGINE_ReturnToArbitrationState(0);
                *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
            }
        }
    }

    /* In slave state, check need to send TPLG_SYNC
     */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,10,end_time);
#endif
    STKTPLG_ENGINE_CheckTplgSyncStatus();

#endif
    #if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(4,11,end_time);
#endif

    return;
}


static void STKTPLG_ENGINE_ProcessHaltState(UI32_T *notify_msg)
{
#if (SYS_CPNT_STACKING == TRUE)

    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI8_T current_logic_link_status;
    UI32_T up_phy_status, down_phy_status;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    xgs_stacking_debug("Process Halt state \n");

#if (SYS_CPNT_STACKING_BUTTON == TRUE)
    /* check if stacking button state changed,if changed change to arbitration
     * state
     */
    if(STKTPLG_OM_IsStackingButtonChanged()|| FALSE == ctrl_info_p->stacking_button_state)
    {
        /*change the board.c*/
        STKTPLG_MGR_SetStackingPortInBoard(ctrl_info_p->stacking_button_state);
        /*set the port attribute*/
        DEV_SWDRV_PMGR_SetStackingPort(ctrl_info_p->stacking_button_state);
        /* stop the timer        */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration  */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
       /* EPR: N/A
        Problem:Stacking:DUT displayed Error Messages continuously after

        pressed the Unit4's stack_button off and on.
        Rootcause:(1)when 2 dut stacking stable,slave pressed off stacking

        button and pressed stacking button again
                  (2)slave will send renunberPDU to master and master will
        enter transtion again
                  (3)sometimes slave will hang,because ,slave never clear

        the old master-mac-address,it will consider the same master ,but it

        send enter transition mode to stkctrl,make the whole system stay in

        the transition mode
        Solution: send normal tcn pkt to master instead of renumber pdu
                  when the stacking button changed,always clear the old

        master mac-address
        Files:stktplg_engine.c*/

        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,TRUE);

        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
        /*enter transition mode,and re-run*/
        return;
    }
#endif /* end of #if (SYS_CPNT_STACKING_BUTTON == TRUE) */
    current_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    /*
     * Get physical link state for both stacking ports
     */
    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP)\r\n", __FUNCTION__,__LINE__);
    }
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState((UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }


    /* Check if Hello Type 0 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer0(&current_logic_link_status);


  /*  xgs_stacking_debug("---- Halt State --- %d + %d: %ld: %d: %ld (%x: %x)\r\n",
                       ctrl_info_p->stacking_dev_id, ctrl_info_p->up_phy_status,
                       up_phy_status,
                       ctrl_info_p->down_phy_status,
                       down_phy_status,
                       current_logic_link_status, ctrl_info_p->stacking_ports_logical_link_status); */

    /* Check if Hello Type 1 timer time-out
     */
    STKTPLG_ENGINE_CheckHelloTimer1(ctrl_info_p);

    /* check if there is any topology change
     */
    if ( ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_UP_LINK))) ||
         ((ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(current_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )        
    {
        /* stop the timer
         */
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Halt: either stacking link down\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("ProcessHaltState TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

        ctrl_info_p->reset_state = 20;

        return;
    }

    ctrl_info_p->stacking_ports_logical_link_status = current_logic_link_status;
    ctrl_info_p->up_phy_status   = up_phy_status;
    ctrl_info_p->down_phy_status = down_phy_status;

    /* Send Hello Type 0 PDU to enquire stacking ports' status
     */
    STKTPLG_ENGINE_SendHelloPDU0(ctrl_info_p,up_phy_status,down_phy_status);

    printf("\r\n STK is halt, for more than %d units ,more than max module,or different project ID", SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */
    return;
}

#if (SYS_CPNT_STACKING == TRUE)
/* functions used to handle incoming packet under different states
 */
static void STKTPLG_ENGINE_HandlePktInitState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    UI32_T                   pdu_len;
    UI32_T                   uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt0_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (hbt0_ptr->header.type != STKTPLG_HELLO_TYPE_0)
        xgs_stacking_debug("-- Init state RX from %d type= %d \r\n", rx_port, hbt0_ptr->header.type);

    /* check if this is my HBT0
     */
    if (hbt0_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */
        STKTPLG_OM_HELLO_0_T    *hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt0_ptr;
        UI8_T rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;        
                                    
        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {        
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down);
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;     
        }
        else
        {
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;

            }
            else /*hello PDU source port == downlink_port*/
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
            }
        }            
    }
    else
    {
        /* should not happen, print out error message
         */
    }

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktArbitrationState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    UI8_T                   unit, unit_id, xxx;
    UI32_T                  i;
    int                     index;
    UI32_T  max_port_number,max_option_port_number;
#if (SYS_CPNT_UNIT_HOT_SWAP != TRUE)
    UI32_T                     k;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
#endif
    UI32_T  pdu_len;
    UI32_T                  uplink_port, downlink_port;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time =SYSFUN_GetSysTick();
#endif

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        ctrl_info_p->exp_module_id[i]=UNKNOWN_MODULE_ID;
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(5,0,end_time);
#endif
    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt0_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (hbt0_ptr->header.type != STKTPLG_HELLO_TYPE_0)
    {
        xgs_stacking_debug("-- arbit state Rx from  %d type= %d \r\n", rx_port, hbt0_ptr->header.type);
    }

    /* check if this is my HBT0
     */
    if (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_0)
    {
        if (hbt0_ptr->payload[0].image_type != ctrl_info_p->image_type)
        {
            ctrl_info_p->state = STKTPLG_STATE_HALT;

            STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

            printf("\r\n  Units in stacking with different project ID.\r\n.");

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(5,1,end_time);
#endif
            goto exit;
        }

        if (rx_port == uplink_port)
        {
            if (LAN_TYPE_TX_UP_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
            {
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;
            }
        }
        else /* if (rx_port == downlink_port) */
        {
            if (LAN_TYPE_TX_DOWN_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
            {
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
            }
        }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(5,2,end_time);
#endif
        /* Retain Preempted info from neighbor
         */
        ctrl_info_p->preempted_master = hbt0_ptr->payload[0].preempted;

        if (FALSE == STKTPLG_ENGINE_PriorityCompare(ctrl_info_p, &hbt0_ptr->payload[0]))
        {

            /* Rx other HBT0 with higher priority
             * change state to SLAVE WAIT ASSIGNMENT
             */
            //  stktplg_engine_debug_mode = FALSE;

            xgs_stacking_debug("HBT0 Priority higher than me \r\n");

            ctrl_info_p->state = STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT;
            STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT);

/*add by fen.wang,2008-7-2,for when the topo changed,it may be master for it preemped is true,
but in fact it should be slave*/
            ctrl_info_p->preempted = FALSE;
            /* This is a HBT0 from HIGH priority neighbour unit and needs to be relayed */

            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);

              /*  STKTPLG_TX_SendHBTType0Ack(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK); */

                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    /* Relay this HBT0 to DOWN stacking port
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,2);
                }
                else
                {
                    /* Reply to HIGH priority unit
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,0);
                }

                /* Setup HBT1 timer in SLAVE_WAIT_ASSIGNMENT state
                 */
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, STKTPLG_TIMER_HBT0_TIMEOUT);
            }
            else /*if (rx_port == downlink_port)*/
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

                /*  STKTPLG_TX_SendHBTType0Ack(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK); */

                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    /* Relay this HBT0 to UP stacking port
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,3);
                }
                else
                {
                    /* Reply to HIGH priority unit
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,1);
                }

                /* Setup HBT1 timer in SLAVE_WAIT_ASSIGNMENT state
                 */
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_DOWN, STKTPLG_TIMER_HBT0_TIMEOUT);
            }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
          end_time = SYSFUN_GetSysTick()-start_time;
          STKTPLG_ENGINE_BD_SetTick(5,3,end_time);
#endif
        }  /* Compare priority */
        else
        {
             xgs_stacking_debug("HBT0 Priority lower than me \r\n");
            /* This box has HIGHER priority than neighbour
             */
            if (memcmp(hbt0_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0)
            {
                /* Drop it !!
                 * Potential Master Unit is PowerUP earlier than other unit !!
                 */

                goto exit;
            }
            else
            {
                if (rx_port == uplink_port)
                {
                /* Receive self-sent HBT0 by loopback path (closed loop stacking)
                 */

                    if (inc(hbt0_ptr->header.seq_no) != ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_0_DOWN] )
                    {
                        /* phase out packets
                         */
                        goto exit;
                    }

                    /*
                     * Check if this is the ONLY unit in stack.
                     * If so, switch to standalone mode ?? possible ??
                     */
                    if (2 == hbt0_ptr->header.next_unit)
                    {

                        /* Restore Port Mapping and Unit Config
                         */

                        STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

                        ctrl_info_p->total_units = 1;
                        ctrl_info_p->total_units_up   = 1;
                        ctrl_info_p->total_units_down = 1;

                        ctrl_info_p->is_ring = 0;


                        ctrl_info_p->stable_hbt_up.payload[0].start_module_id = 0;
                        ctrl_info_p->stable_hbt_up.payload[0].chip_nums = ctrl_info_p->chip_nums;
                        ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
                        ctrl_info_p->stable_hbt_down.payload[0].chip_nums = ctrl_info_p->chip_nums;

                        memcpy(ctrl_info_p->stable_hbt_up.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);

                        memcpy(ctrl_info_p->stable_hbt_down.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                        memcpy(ctrl_info_p->stable_hbt_up.payload[0].past_master_mac ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
                        memcpy(ctrl_info_p->stable_hbt_down.payload[0].past_master_mac ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
                        STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                        STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
                        STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);

                        STKTPLG_ENGINE_AssignUnitID();

                        STKTPLG_OM_UpdateExpectedModuleType(); /* Vincent: For standalone */

                        ctrl_info_p->stable_hbt_down.payload[0].unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id ;


#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:08:09 */
                        ctrl_info_p->my_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
                        STKTPLG_OM_SetMyUnitID(ctrl_info_p->stable_hbt_up.payload[0].unit_id);
#endif
                        ctrl_info_p->master_unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/*EPR:ES3628BT-FLF-ZZ-01059
Problem:stack:mac learning error after hot remove unit from ring

stack.
Rootcause:(1)after hotswap,the module id for each dut will

re-assign.And it may changed
          (2)the mac-address learned on old module id will not be

deleted ,and re-learned for the new module id
          (3)so the pkt will forward to error place
Solution:when do hot-swap,keep the module id no change if the

unit-id not changed
Files:stktplg_engine.c*/
                        ctrl_info_p->stable_hbt_down.header.next_unit = 2;
                        if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_down, &ctrl_info_p->last_module_id))
                        {
                            ctrl_info_p->state = STKTPLG_STATE_HALT;

                            printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");
                            goto exit;
                        }
                        STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[0]));
                        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
                        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
#else
                        if(TRUE ==ctrl_info_p->expansion_module_exist)
                        {
                            /*  Store expansion module information. */
                            /*ctrl_info_p->expansion_module_type  = module_info.expansion_module_type;*/
                            ctrl_info_p->expansion_module_exist = TRUE;
                            ctrl_info_p->expansion_module_id    = ctrl_info_p->chip_nums;
                            /*module_info.expansion_module_id     = ctrl_info_p->chip_nums; */
                            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id    = ctrl_info_p->chip_nums;
                            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = TRUE;
                            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id    = ctrl_info_p->chip_nums;
                            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = TRUE;
                            xgs_stacking_debug("\r\n -- Main board : Expansion module info - (%d : %d)\r\n",
                            ctrl_info_p->expansion_module_type,
                            ctrl_info_p->expansion_module_id);
                            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=ctrl_info_p->chip_nums;
                            //STKTPLG_MGR_InsertExpModule(ctrl_info_p->my_unit_id);
                        }
                        else
                        {
                            ctrl_info_p->expansion_module_id    = UNKNOWN_MODULE_ID;
                            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id    = UNKNOWN_MODULE_ID;
                            ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = FALSE;
                            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id    = UNKNOWN_MODULE_ID;
                            ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = FALSE;
                            ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]=UNKNOWN_MODULE_ID;
                        }

                        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->my_unit_id, &max_option_port_number);
                        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);

                        if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->expansion_module_type,&module_info_p) && (TRUE ==ctrl_info_p->expansion_module_exist))
                        {
                            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable,sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
                            for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
                            {
                                port_mapping[k].module_id = ctrl_info_p->chip_nums;
                            }
                        }
                        else
                        {
                            for (k=max_port_number; k<(max_port_number+max_option_port_number); k++)
                            {
                                port_mapping[k].module_id = UNKNOWN_MODULE_ID;
                            }
                        }
                        STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
#endif
                        module_swapped = FALSE;
                        STKTPLG_ENGINE_ConfigStackInfoToIUC();
                        /*on 0108 */
                        //printf("Arb to stand TKTPLG_OM_SetDeviceInfo\r\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                        STKTPLG_OM_ENG_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                        STKTPLG_OM_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
                        if(ctrl_info_p->expansion_module_ready)
                        {
                            if(FALSE==STKTPLG_OM_ExpModuleIsInsert(ctrl_info_p->my_unit_id))
                            {
                                STKTPLG_OM_InsertExpModule(ctrl_info_p->my_unit_id,ctrl_info_p->expansion_module_type,ctrl_info_p->module_runtime_fw_ver);
                            }
                        }
                        /* Restore stacking ports link to send neighbour HelloType 0
                         */
                        ctrl_info_p->stacking_ports_logical_link_status = 0; /*(LAN_TYPE_TX_UP_LINK | LAN_TYPE_TX_DOWN_LINK);*/

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                       ctrl_info_p->state = STKTPLG_STATE_STANDALONE;
                       STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_STANDALONE);
                       if((ctrl_info_p->past_role != STKTPLG_STATE_MASTER)&&
                           (ctrl_info_p->past_role != STKTPLG_STATE_STANDALONE))
                        {
                            STKTPLG_ENGINE_MasterStorePastTplgInfo();
                            STKTPLG_OM_CopyDatabaseToSnapShot();
                            *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
                            xgs_stacking_debug("HandlePktArbitrationState STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE\n");
                        }
                        else
                        {
                            STKTPLG_ENGINE_CheckAndSetProvisionLost();
                            *notify_msg = STKTPLG_UNIT_HOT_INSERT_REMOVE;
                             xgs_stacking_debug("HandlePktArbitrationState STKTPLG_UNIT_HOT_INSERT_REMOVE\n");

                        }
                        ctrl_info_p->past_role=STKTPLG_STATE_STANDALONE;
                        memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);
                        ctrl_info_p->stable_flag = TRUE;
#else
                         /* Change to standalone mode if BOTH stacking ports are open
                         */
                        *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
                        ctrl_info_p->state = STKTPLG_STATE_STANDALONE;
                        STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_STANDALONE);
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(5,4,end_time);
#endif
                        goto exit;
                    }

                    /*
                     * Check if there are more than 8 units in stack.
                     * If so, show error message and disable the whole stack
                     */
                    if ((hbt0_ptr->header.next_unit - 1) > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
                    {
                        /* change state to Halt
                         */
                        ctrl_info_p->state = STKTPLG_STATE_HALT;

                        STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

                        printf("\r\nOnly support a 8-unit stack, please remove the 9th-unit.\r\n.");

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(5,5,end_time);
#endif
                        goto exit;
                    }

                    /* Stop both HBT0 timers
                     */
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                    /* I am the master, keep topology information
                     */
                    memcpy(&ctrl_info_p->stable_hbt_down, hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));


                    /* prepare basic information for all units
                     */
                    ctrl_info_p->start_module_id = 0;
                    ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
                    ctrl_info_p->last_module_id = ctrl_info_p->stable_hbt_down.payload[0].chip_nums;
#if 0
                    if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_down, &ctrl_info_p->last_module_id))
                    {
                        ctrl_info_p->state = STKTPLG_STATE_HALT;

                        STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

                        printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");

                        L_MM_Mref_Release(&mref_handle_p);

                        return;
                    }
#endif
                    /* Assign logical unit id and total unit number based on current topology
                     */
                    ctrl_info_p->total_units        = (hbt0_ptr->header.next_unit - 1);
                    ctrl_info_p->total_units_down   = ctrl_info_p->total_units;
                    ctrl_info_p->total_units_up     = ctrl_info_p->total_units;
                    ctrl_info_p->is_ring            = TRUE;


                    memcpy(ctrl_info_p->master_mac, hbt0_ptr->payload[0].mac_addr, STKTPLG_MAC_ADDR_LEN);

                    /* Automatic assignment of Unit ID
                     */
                    unit_id = 1;
                    for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                    {
                        ctrl_info_p->stable_hbt_down.payload[unit].total_units      = ctrl_info_p->total_units;
                        ctrl_info_p->stable_hbt_down.payload[unit].total_units_up   = ctrl_info_p->total_units;
                        ctrl_info_p->stable_hbt_down.payload[unit].total_units_down = ctrl_info_p->total_units;
                        ctrl_info_p->stable_hbt_down.payload[unit].is_ring          = TRUE;

                        ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id     = UNKNOWN_MODULE_ID;
                        ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist  = FALSE;
                    }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                    STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
                    STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
#endif


                    STKTPLG_ENGINE_AssignUnitID();
/*EPR:ES3628BT-FLF-ZZ-01059
Problem:stack:mac learning error after hot remove unit from ring

stack.
Rootcause:(1)after hotswap,the module id for each dut will

re-assign.And it may changed
          (2)the mac-address learned on old module id will not be

deleted ,and re-learned for the new module id
          (3)so the pkt will forward to error place
Solution:when do hot-swap,keep the module id no change if the

unit-id not changed
Files:stktplg_engine.c*/
                    if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_down, &ctrl_info_p->last_module_id))
                    {
                        ctrl_info_p->state = STKTPLG_STATE_HALT;

                        STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

                        printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");

                        goto exit;
                    }
                    STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[0]));
                    STKTPLG_OM_UpdateExpectedModuleType();

                    ctrl_info_p->master_unit_id     = ctrl_info_p->stable_hbt_down.payload[0].unit_id;
                    ctrl_info_p->my_logical_unit_id = ctrl_info_p->master_unit_id;
#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:09:12 */
                    ctrl_info_p->my_unit_id         = ctrl_info_p->master_unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
                    STKTPLG_OM_SetMyUnitID(ctrl_info_p->master_unit_id);
#endif
                    printf("Master elected [Closed Loop] total units=%d\n",ctrl_info_p->total_units_down);
                    /*
                     * Assign Module ID to port mapping table for ALL available units
                     */
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)

                    for (unit = 0; unit < ctrl_info_p->total_units_up; unit++)
                    {
                        for (i = 0; i < ctrl_info_p->stable_hbt_down.payload[unit].chip_nums; i++)
                        {
                            for (j = 0; j < 12; j++)
                            {
                          /*    ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id &= 0xe0;
                                ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id
                                                        |= ((ctrl_info_p->stable_hbt_down.payload[unit].start_module_id + i) & 0x1f);   */
                                ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id += ctrl_info_p->stable_hbt_down.payload[unit].start_module_id;
                            }
                        }
                    }
#endif
                    memcpy(&ctrl_info_p->stable_hbt_up.header, &hbt0_ptr->header, sizeof(STKTPLG_OM_HBT_Header_T));

                    memcpy(&ctrl_info_p->stable_hbt_up.payload[0], &ctrl_info_p->stable_hbt_down.payload[0],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));

                    for (unit = 1; unit < ctrl_info_p->total_units_down; unit++)
                    {
                        memcpy(&ctrl_info_p->stable_hbt_up.payload[unit],
                               &ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_down - unit],
                        sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                    }

                    /* send out HBT type 1 from DOWN stacking port and start its timer
                     * (ONE direction only since the current stacking topology is a loop)
                     */

                    xgs_stacking_debug("Arbitration SEND HBT1 DOWN\n");
                    DEV_SWDRV_PMGR_VID_BREAK();

                    /* Re-locate master stacking info
                     */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_OM_ENG_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                    STKTPLG_OM_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
                    STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
#endif


                    STKTPLG_TX_SendHBTType1(TRUE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,4);
                    /* Setup HBT1 receive expired routine
                     */
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);

                    /* change state to master sync
                     */

                    // STKTPLG_OM_ShowCFG() ;
                    ctrl_info_p->state = STKTPLG_STATE_MASTER_SYNC;
                    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_MASTER_SYNC);
                    ctrl_info_p->bounce_msg = 0;
                }
                else
                {
                    /* ignore HBT 1 from DOWN stacking port
                     */
                } /* if (rx_port == uplink_port) */
            }/*  if (memcmp(hbt0_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0) */
        }    /* compare priority */

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(5,6,end_time);
#endif
    } /* HBT0 */
    else if (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_0_ACK)
    {
        if (rx_port == uplink_port)
        {
           // xgs_stacking_debug("---- Ack UP rx\r\n");

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, STKTPLG_TIMER_HBT0_TIMEOUT);
        }
        else
        {
           // xgs_stacking_debug("---- Ack DOWN rx\r\n");

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_DOWN, STKTPLG_TIMER_HBT0_TIMEOUT);
        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(5,7,end_time);
#endif

    } /* HBT0_ACK */
    else if (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_0_REBOUND)
    {
        /* This is a HBT0 rebound from either edges
         */
        if (memcmp(hbt0_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0)
        {
            /* This is a HBT0 PDU from neighbor units, relay to next unit
             */
            if ( (rx_port == uplink_port                                            ) &&
                 (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) )
            {
                /* PDU from UP stacking port, relay to DOWN stacking port if connected
                 */
                STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,2);
            }
            else if ( (rx_port == downlink_port                                      ) &&
                      (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) )
            {
                /* PDU from DOWN stacking port, relay to UP stacking port if connected
                 */
                STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,3);
            }
            else
            {
                /* phase out packets
                 */
            }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(5,8,end_time);
#endif
        }
        else /* my own  HBT0 rebounded  */
        {
            /* This is a self-sent HBT0 PDU rebounded from edges
             */
            if (rx_port == uplink_port)
            {

                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);

              //  STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, STKTPLG_TIMER_HBT0_TIMEOUT);

                memcpy(&ctrl_info_p->stable_hbt_up, hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));


                ctrl_info_p->total_units_up = hbt0_ptr->header.next_unit - 1;

                /* prepare basic information for all units connected to stacking port UP
                 */
                ctrl_info_p->bounce_msg |= 1;

                if (LAN_TYPE_TX_DOWN_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    memcpy(&ctrl_info_p->stable_hbt_down, hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));
                    ctrl_info_p->total_units_down = 1;
                    ctrl_info_p->stable_hbt_down.header.next_unit = 2;
                    ctrl_info_p->stable_hbt_down.payload[0].start_module_id =
                    ctrl_info_p->stable_hbt_up.payload[0].start_module_id;

                    ctrl_info_p->bounce_msg |= 2;
                }
            }
            else /* if (rx_port == downlink_port */
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

                memcpy(&ctrl_info_p->stable_hbt_down, hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));


                ctrl_info_p->total_units_down = hbt0_ptr->header.next_unit - 1;

                ctrl_info_p->bounce_msg |=2;

                if (LAN_TYPE_TX_UP_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    memcpy(&ctrl_info_p->stable_hbt_up, hbt0_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));
                    ctrl_info_p->total_units_up = 1;
                    ctrl_info_p->stable_hbt_up.header.next_unit = 2;
                    ctrl_info_p->stable_hbt_up.payload[0].start_module_id =
                    ctrl_info_p->stable_hbt_down.payload[0].start_module_id;

                    ctrl_info_p->bounce_msg |=1;
                }
            }

            if ((ctrl_info_p->bounce_msg & 3) == 3)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

                /* ctrl_info_p->my_unit_id  = 1; */
                ctrl_info_p->total_units = ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - 1;
                ctrl_info_p->is_ring     = FALSE;
                memcpy(ctrl_info_p->master_mac, hbt0_ptr->payload[0].mac_addr, STKTPLG_MAC_ADDR_LEN);

                ctrl_info_p->expansion_module_id    = UNKNOWN_MODULE_ID;
                ctrl_info_p->expansion_module_exist = FALSE;

                 /*
                  * Check if there are more than 8 units in stack.
                  * If so, show error message and disable the whole stack
                  */
                if (ctrl_info_p->total_units > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
                {
                    /* change state to HALT STATE
                     */
                    ctrl_info_p->state = STKTPLG_STATE_HALT;

                    if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    {
                        STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_UP_LINK);
                    }

                    if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                    {
                        STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);
                    }

                    printf("\r\nOnly support a 8-unit stack, please remove the 9th-unit\r\n.");

                    goto exit;
                }


                /* Keep total units in all payload
                 * Assign and store Unit ID to all payload
                 */
                unit_id = 1;
                xxx=0;
                for (index = ctrl_info_p->total_units_up - 1; index >= 0; index--)
                {
                    ctrl_info_p->stable_hbt_up.payload[index].total_units      = ctrl_info_p->total_units;
                    ctrl_info_p->stable_hbt_up.payload[index].total_units_up   = ctrl_info_p->total_units_up;
                    ctrl_info_p->stable_hbt_up.payload[index].total_units_down = ctrl_info_p->total_units_down;
                    ctrl_info_p->stable_hbt_up.payload[index].is_ring          = FALSE;

                    ctrl_info_p->stable_hbt_up.payload[index].expansion_module_id    = UNKNOWN_MODULE_ID;
                    ctrl_info_p->stable_hbt_up.payload[index].expansion_module_exist = FALSE;
                }
                xxx--;
                for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                {
                    ctrl_info_p->stable_hbt_down.payload[unit].total_units      = ctrl_info_p->total_units;
                    ctrl_info_p->stable_hbt_down.payload[unit].total_units_up   = ctrl_info_p->total_units_up;
                    ctrl_info_p->stable_hbt_down.payload[unit].total_units_down = ctrl_info_p->total_units_down;
                    ctrl_info_p->stable_hbt_down.payload[unit].is_ring          = FALSE;

                    ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id    = UNKNOWN_MODULE_ID;
                    ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = FALSE;
                }
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
                STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);

                STKTPLG_ENGINE_AssignUnitID();
                STKTPLG_OM_UpdateExpectedModuleType();

                ctrl_info_p->master_unit_id     = ctrl_info_p->stable_hbt_up.payload[0].unit_id;
                ctrl_info_p->my_logical_unit_id = ctrl_info_p->master_unit_id;
#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:10:23 */
                ctrl_info_p->my_unit_id         = ctrl_info_p->master_unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
                STKTPLG_OM_SetMyUnitID(ctrl_info_p->master_unit_id);
#endif

                printf("Master elected [Line], total units =%d\n",ctrl_info_p->total_units);

                /* Re-locate master stacking info
                 */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                STKTPLG_OM_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
                STKTPLG_MGR_SetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
#endif

                ctrl_info_p->stable_hbt_up.payload[0].start_module_id   = 0;
                ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
                ctrl_info_p->last_module_id = ctrl_info_p->stable_hbt_up.payload[0].chip_nums;

                if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_up, &ctrl_info_p->last_module_id))
                {
                    ctrl_info_p->state = STKTPLG_STATE_HALT;

                    STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

                    printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");

                    goto exit;
                }

                if (FALSE == STKTPLG_ENGINE_PrepareUnitBasicInformation(&ctrl_info_p->stable_hbt_down, &ctrl_info_p->last_module_id))
                {
                    ctrl_info_p->state = STKTPLG_STATE_HALT;

                    STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);

                    printf("\r\nOnly support a 32 modules.\r\nPlease remove Unit or Expansion Module, and ReStart\r\n.");

                    goto exit;
                }

                /*
                 * Assign Module ID to port mapping table for ALL available units
                 * connected to UP stacking port
                 */
#if (SYS_CPNT_UNIT_HOT_SWAP == FALSE)

                for (unit = 0; unit < ctrl_info_p->total_units_up; unit++)
                {
                    for (i = 0; i < ctrl_info_p->stable_hbt_up.payload[unit].chip_nums; i++)
                    {
                        for (j = 0; j < 12; j++)
                        {
                 /*           ctrl_info_p->stable_hbt_up.payload[unit].port_mapping[i*12 + j].mod_dev_id &= 0xe0;
                            ctrl_info_p->stable_hbt_up.payload[unit].port_mapping[i*12 + j].mod_dev_id
                                                    |= ((ctrl_info_p->stable_hbt_up.payload[unit].start_module_id + i) & 0x1f);   */
                            ctrl_info_p->stable_hbt_up.payload[unit].port_mapping[i*12 + j].mod_dev_id += ctrl_info_p->stable_hbt_up.payload[unit].start_module_id;

                        }
                    }
                }

                /*
                 * Assign Module ID to port mapping table for ALL available units
                 * connected to DOWN stacking port
                 */
                for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                {
                     for (i = 0; i < ctrl_info_p->stable_hbt_down.payload[unit].chip_nums; i++)
                    {
                        for (j = 0; j < 12; j++)
                        {
              /*              ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id &= 0xe0;
                            ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id
                                                    |= ((ctrl_info_p->stable_hbt_down.payload[unit].start_module_id + i) & 0x1f);   */
                             ctrl_info_p->stable_hbt_down.payload[unit].port_mapping[i*12 + j].mod_dev_id += ctrl_info_p->stable_hbt_down.payload[unit].start_module_id;

                        }
                    }
                }
#else
                STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_up.payload[0]));
#endif /* end of #if (SYS_CPNT_UNIT_HOT_SWAP == FALSE) */

                /*
                 * Group UP and DOWN unit info into one structure
                 */
                i = ctrl_info_p->total_units_up;
                for (unit = 1; unit < ctrl_info_p->total_units_down; unit++, i++)
                {
                    memcpy(&ctrl_info_p->stable_hbt_up.payload[i], &ctrl_info_p->stable_hbt_down.payload[unit],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }

                i = ctrl_info_p->total_units_down;
                for (unit = 1; unit < ctrl_info_p->total_units_up; unit++, i++)
                {
                    memcpy(&ctrl_info_p->stable_hbt_down.payload[i], &ctrl_info_p->stable_hbt_up.payload[unit],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }

                /* Send out HBT type 1 and start related timer
                 */
                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK) )
                {
                    xgs_stacking_debug("arbitration SEND HBT1_UP\n");
                    DEV_SWDRV_PMGR_VID_BREAK();
                    STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,5);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
                }

                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    xgs_stacking_debug("arbitration SEND HBT1_DOWN\n");
                    DEV_SWDRV_PMGR_VID_BREAK();
                    STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,6);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
                }


                ctrl_info_p->state = STKTPLG_STATE_MASTER_SYNC;
                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_MASTER_SYNC);
                ctrl_info_p->bounce_msg = 0;
            }
        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(5,9,end_time);
#endif
    }
    else if (hbt0_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */
        STKTPLG_OM_HELLO_0_T    *hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt0_ptr;
        UI8_T rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;        
                                    
        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {        
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down);            
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;     
        }
        else
        {
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;

            }
            else /*hello PDU source port == downlink_port*/
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
            }        
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(5,10,end_time);
#endif
        }
    }
    else
    {
        /* should not happen, print out error message
         */
    }

#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(5,11,end_time);
#endif

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktStandAloneState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{
    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    STKTPLG_OM_HELLO_0_T    *hello_ptr;
    UI32_T                   pdu_len;
    UI32_T                   uplink_port, downlink_port;
    UI8_T                    rx_port_up_down;
    
    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }

    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt0_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    xgs_stacking_debug("-- standalone state RX frm %d : type=%d \r\n", rx_port, hbt0_ptr->header.type);


    rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;

    /* check if this is my HBT0
     */
    if (hbt0_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */
        hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt0_ptr;
        
        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {        
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down); 
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;     
        }
        else
        {
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;

            }
            else /*hello PDU source port == downlink_port*/
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
            }
        
            hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt0_ptr;
            /*if only one dut that loop ,it will do nothing,else enter arbitration */
            if(memcmp(ctrl_info_p->my_mac,hello_ptr->mac_addr,STKTPLG_MAC_ADDR_LEN)!=0)
            {
            STKTPLG_ENGINE_LogMsg("[TCN] Standalone: Rx other HBT0\n");

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("HandlePktStandAloneState_1 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
            }
        }
    }
    else if (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_0)
    {
        /* If both rx and tx port of HBT0 are the same type of stack link (UP or DOWN),
         * it's an invalid link and no act should be taken but the warning prompted.
         */
        if(!(ctrl_info_p->stacking_ports_logical_link_status & rx_port_up_down))
        {
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down); 
        }
        else
        {
            /* HBT Type 0 for Re-topology (topology change)
             */
            if (memcmp(hbt0_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0)
            {
                STKTPLG_ENGINE_LogMsg("[TCN] Standalone: Rx other HBT0\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                xgs_stacking_debug("HandlePktStandAloneState_2 TOPO change\n");
#else
                STKTPLG_ENGINE_ReturnToArbitrationState(0);
                *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
            }
        }
   }
   else
   {
        /* should not happen, print out error message
         */
   }

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktMasterSyncState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt1_ptr;
    UI32_T up_phy_status, down_phy_status;
    UI32_T pdu_len;
    UI32_T uplink_port, downlink_port;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time = SYSFUN_GetSysTick();
#endif

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

   if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt1_ptr = (STKTPLG_OM_HBT_0_1_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt1_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (hbt1_ptr->header.type != STKTPLG_HELLO_TYPE_0)
        xgs_stacking_debug("-- master sync state Rx from %d :type = %d  \r\n", rx_port, hbt1_ptr->header.type);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
     end_time = SYSFUN_GetSysTick()-start_time;
     STKTPLG_ENGINE_BD_SetTick(6,0,end_time);
#endif

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState((UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP)\r\n", __FUNCTION__,__LINE__);
    }
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState((UI32_T*)&down_phy_status))
    {
        printf("%s:DEV_SWDRV_PMGR_GetPortLinkStatusByDeviceId fail(DOWN)\r\n", __FUNCTION__);
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(6,1,end_time);
#endif

    if (STKTPLG_ENGINE_Check_Stacking_Link_Up(ctrl_info_p,rx_port, mref_handle_p,FALSE,notify_msg) == TRUE)
    {
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(6,2,end_time);
#endif
        return;
    }

    if (STKTPLG_ENGING_Check_Closed_Loop_TCN(ctrl_info_p,rx_port, mref_handle_p)== TRUE)
    {
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
       end_time = SYSFUN_GetSysTick()-start_time;
       STKTPLG_ENGINE_BD_SetTick(6,3,end_time);
#endif
       return;
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(6,4,end_time);
#endif

    if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
    {
        /* check if this is my HBT1
         */
        if (memcmp(hbt1_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
        {
            if (inc(hbt1_ptr->header.seq_no) != ctrl_info_p->seq_no[(rx_port == uplink_port)?STKTPLG_HBT_TYPE_1_DOWN:STKTPLG_HBT_TYPE_1] )
            {
                /* phase out packets
                 */
                printf("Rx sequence error packet in Master-sync state\n");
                goto exit;
            }

            STKTPLG_ENGINE_PktMasterSyncStateOmProcess(ctrl_info_p, hbt1_ptr,rx_port);

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                /* check if topology is all right
                 */
                if (TRUE == STKTPLG_ENGINE_TopologyIsAllRightDown(hbt1_ptr))
                {
                    /* topolgy is all right, check if every slave unit is ready
                     */
                    STKTPLG_ENGINE_CheckAllOmReady(hbt1_ptr);

                    if (STKTPLG_ENGINE_AllSlaveReady(hbt1_ptr))
                    {
                        /* keep total units in control information
                         */

                        /* start to send out HBT type 2 to collect all topology information
                         */
                        ctrl_info_p->query_unit_up   = 2;
                        ctrl_info_p->query_unit_down = 2;

                        /*
                         * Pretend there is a Type 2 PDU sent from UP port
                         */
                        if (TRUE == ctrl_info_p->is_ring)
                            ctrl_info_p->query_unit_up = ctrl_info_p->total_units_down + 1;

                        STKTPLG_TX_SendHBTType2(NULL,
                                                ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->query_unit_down-1].unit_id,
                                                NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,0);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN, STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT);

                        //   stktplg_engine_debug_mode = FALSE;
                        ctrl_info_p->state = STKTPLG_STATE_GET_TOPOLOGY_INFO;
                        STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_GET_TOPOLOGY_INFO);

                    }
                    else
                    {
                        /* send out HBT type 1 and start related timer
                         */
 #if 0
                        STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,7);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
 #endif
                    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                 end_time = SYSFUN_GetSysTick()-start_time;
                 STKTPLG_ENGINE_BD_SetTick(6,5,end_time);
#endif
                }
                else
                {
                    /* topology change
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Master Sync:Rx wrong topology packet\n");

                    /* change state to arbitration
                     */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0,FALSE,FALSE);
                    xgs_stacking_debug("HandlePktMasterSynState_1 TOPO change\n");
 #else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
                }
            } /* end of if (rx_port == uplink_port) */
            else
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                /* Ignore the HBT1 from Down port
                 */
                printf("HBT1 from downport \n");
#if 0
                STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,8);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
#endif
            }
        }
        else
        {
            /* not my HBT1
             */
            STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Rx HBT1 mac error\n");

            /* we should not receive other HBT1, change state to arbitration
             */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0,FALSE,FALSE);
            xgs_stacking_debug("HandlePktMasterSynState_2 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(6,6,end_time);
#endif
    }
    else if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1_REBOUND)
    {
        /* check if this is my HBT1
         */
        if (memcmp(hbt1_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
        {
            if (inc(hbt1_ptr->header.seq_no) != ctrl_info_p->seq_no[(rx_port == uplink_port)?STKTPLG_HBT_TYPE_1:STKTPLG_HBT_TYPE_1_DOWN])
            {
                /* phase out packets
                 */
                printf("Not sequence\n");
                goto exit;
            }
            STKTPLG_ENGINE_PktMasterSyncStateOmProcess(ctrl_info_p, hbt1_ptr, rx_port);

            if (rx_port == uplink_port)
            {
                /* Received rebound HBT1 from UP port
                 */
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                if (TRUE == STKTPLG_ENGINE_TopologyIsAllRightUp(hbt1_ptr))
                {
                    STKTPLG_ENGINE_CheckAllOmReady(hbt1_ptr);
                    if (STKTPLG_ENGINE_AllSlaveReady(hbt1_ptr))
                    {
                        ctrl_info_p->total_units_up = hbt1_ptr->header.next_unit - 1;

                        ctrl_info_p->bounce_msg |= 1;

                        if (FALSE == down_phy_status)
                        {
                            ctrl_info_p->total_units_down = 1;
                            ctrl_info_p->bounce_msg |= 2;
                        }

                    }
                    else
                    {

                        /* send out HBT type 1 and start related timer
                         */
                        // printf("Master Sync send HBT1 UP\n");
#if 0
                        STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,9);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
#endif
                    }
                }
                else
                {
                    /* topology change
                     */

                    /* change state to arbitration
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Rx HBT1 with wrong topo\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0,FALSE,FALSE);
                    xgs_stacking_debug("HandlePktMasterSynState_3 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
                }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(6,7,end_time);
#endif
            }
            else /* if (rx_port == downlink_port) */
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                if (TRUE == STKTPLG_ENGINE_TopologyIsAllRightDown(hbt1_ptr))
                {
                    STKTPLG_ENGINE_CheckAllOmReady(hbt1_ptr);
                    if (STKTPLG_ENGINE_AllSlaveReady(hbt1_ptr))
                    {

                        if (FALSE == ctrl_info_p->is_ring)
                        {
                            ctrl_info_p->total_units_down = hbt1_ptr->header.next_unit - 1;

                            ctrl_info_p->bounce_msg |= 2;

                            if (FALSE == up_phy_status)
                            {
                                ctrl_info_p->total_units_up = 1;
                                ctrl_info_p->bounce_msg |= 1;
                            }
                        }
                        else
                        {
                            ctrl_info_p->bounce_msg = 3;
                        }

                    }
                    else
                    {
                        /* send out HBT type 1 and start related timer
                         */
                        // printf("Master Sync Send HBT1 DOWN\n");
#if 0
                        STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,10);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT);
#endif
                    }
                }
                else
                {
                    /* topology change
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Down slave HBT1 topology changed\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0,FALSE,FALSE);
                    xgs_stacking_debug("HandlePktMasterSynState_4 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
                }
            }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(6,8,end_time);
#endif

            if (3 == (ctrl_info_p->bounce_msg & 3))
            {
                /* keep total units in control information
                 */
                /* start to send out HBT type 2 to collect all topology information
                 */
                ctrl_info_p->query_unit_up   = 2;
                ctrl_info_p->query_unit_down = 2;

                if (TRUE == ctrl_info_p->is_ring)
                {
                    /*
                     * Pretend there is a Type 2 PDU sent from UP port
                     */
                    ctrl_info_p->query_unit_up = ctrl_info_p->total_units_down + 1;
                }
                else
                {
                    if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    {
                        xgs_stacking_debug("Send HBT2 To %d\n",ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->query_unit_up-1].unit_id);
                        STKTPLG_TX_SendHBTType2(NULL,
                                                ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->query_unit_up-1].unit_id,
                                                NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,1);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP, STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT);
                    }
                }

                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    xgs_stacking_debug("Send HBT2 To %d\n",ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->query_unit_down-1].unit_id);
                    STKTPLG_TX_SendHBTType2(NULL,
                                            ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->query_unit_down-1].unit_id,
                                            NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,2);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN, STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT);
                }

                ctrl_info_p->state = STKTPLG_STATE_GET_TOPOLOGY_INFO;
                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_GET_TOPOLOGY_INFO);
                ctrl_info_p->bounce_msg = 0;

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(6,9,end_time);
#endif
            }
        }
    }
    else if (hbt1_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Rx TCN packet\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port,((hbt1_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktMasterSynState_5 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(6,10,end_time);
#endif
    }
    else if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_0)
    {


        UI32_T unit, done;

        /* Check if a HBT Type 0 sent from joined unit due to closed loop
         */
        done = 0;
        for (unit = 0; (unit < ctrl_info_p->total_units_up) && (0 == done); unit++)
        {
            if (0 == memcmp(ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, hbt1_ptr->payload[0].mac_addr,
                            STKTPLG_MAC_ADDR_LEN))
            {
                done = 1;
            }
        }

        for (unit = 1; (unit < ctrl_info_p->total_units_down) && (0 == done); unit++)
        {
            if (0 == memcmp(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, hbt1_ptr->payload[0].mac_addr,
                            STKTPLG_MAC_ADDR_LEN))
            {
                done = 1;
            }
        }

        if (0 == done)
        {
            /* Newly inserted Unit requests for join
             */

            /* Return to Arbitration state
             */
            STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Rx other new inserted HBT0 \n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("HandlePktMasterSynState_6 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

            /* Go directly to re-topology
             */
            ctrl_info_p->reset_state = 20;
        }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(6,11,end_time);
#endif
    }
    else if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_0_REBOUND)
    {
        /* not my HBT1
         */
    }
    /* Added by Vincent on 24,Aug,2004
     * TGPL_SYNC packet for mini-GBIC from slave might have been sent if
     * slave has entered slave mode.
     * We have to deal with this packet here
     */
    else if (hbt1_ptr->header.type == STKTPLG_TPLG_SYNC)
    {
        STKTPLG_ENGINE_ProcessTplgSyncPkt(rx_port, mref_handle_p);
    }
    else
    {
        /* not my HBT1
         */

        /* we should not receive other HBT1, change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Master Sync: Rx other HBT1\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("HandlePktMasterSynState_7 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

        ctrl_info_p->reset_state = 20;
    }
#endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
   end_time = SYSFUN_GetSysTick()-start_time;
   STKTPLG_ENGINE_BD_SetTick(6,12,end_time);
#endif

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;

}


static void STKTPLG_ENGINE_HandlePktGetTopologyInfoState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

    #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    #else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    #endif
    STKTPLG_OM_HBT_2_T      *hbt2_ptr;
    STKTPLG_OM_HBT_22_T     *hbt2_ptr1;
    UI32_T                  i;
    UI32_T                  uplink_port, downlink_port;
    #if 0
    UI32_T                  j,max_port_number,max_option_port_number;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
    #endif
    /*remove variables,because it isn't used in this function, changed by Jinhua Wei*/

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt2_ptr = (STKTPLG_OM_HBT_2_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy here */
    if(hbt2_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (hbt2_ptr->header.type != STKTPLG_HELLO_TYPE_0)
        xgs_stacking_debug("-- get topology RX from  %d :type = %d \r\n", rx_port, hbt2_ptr->header.type);

    if (STKTPLG_ENGINE_Check_Stacking_Link_Up(ctrl_info_p,rx_port, mref_handle_p,FALSE,notify_msg)==TRUE)
        return;

    if (STKTPLG_ENGING_Check_Closed_Loop_TCN(ctrl_info_p,rx_port, mref_handle_p)==TRUE)
        return;

    if (hbt2_ptr->header.type == STKTPLG_HBT_TYPE_2)
    {
        /* HBT Type 2 PDU
         */

        if (rx_port == uplink_port)
        {
            if (inc(hbt2_ptr->header.seq_no) != ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2])
            {
                /* phase out packets
                 */
                goto exit;
            }

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP);
        }
        else
        {
            if (inc(hbt2_ptr->header.seq_no) != ctrl_info_p->seq_no[STKTPLG_HBT_TYPE_2_DOWN])
            {
                /* phase out packets
                 */
                goto exit;
            }

            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN);
        }

        if (hbt2_ptr->header.next_unit != ctrl_info_p->my_unit_id)
        {
            /* no one replys this query, change state to arbitration
             */
            STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: Rx other HBT2 \n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("HandlePktGetTopologyInfoState_1 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

            goto exit;
        }
        else
        {
            /* set topology information of this unit to OM
             */
            //xgs_stacking_debug("TPG[%d].nport=%d\n",hbt2_ptr->payload.unit_id,hbt2_ptr->payload.unit_cfg.nport);
            /* Modified by Vincent on 15,Nov,2004
             * To resolve the issue that module type was not properly initialized for
             * older version we have to avoid module type been overwritten
             */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_OM_ENG_GetDeviceInfo(hbt2_ptr->payload.unit_id, &device_info);
#else
            STKTPLG_OM_GetDeviceInfo(hbt2_ptr->payload.unit_id, &device_info);
#endif
            for (i=0; i<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; i++)
            {
                hbt2_ptr->payload.unit_cfg.module_presented[i] = device_info.module_presented[i];
                hbt2_ptr->payload.unit_cfg.module_type[i] = device_info.module_type[i];
                hbt2_ptr->payload.unit_cfg.exp_module_presented[i] = device_info.exp_module_presented[i];
                hbt2_ptr->payload.unit_cfg.exp_module_type[i]      = device_info.exp_module_type[i];
                if(device_info.exp_module_presented[i] == 1)
                {
                    hbt2_ptr->payload.unit_cfg.nport = device_info.nport;
                }
            }
            /* Modified by Vincent on 7,Apr,2005
             * To resolve the issue that HBT2 format is different between
             * different version we have to check the run-time version and
             * use different format of structure
             */
            if (STKTPLG_OM_IsHBTTooOld(hbt2_ptr->payload.unit_cfg.board_info.runtime_sw_ver))
            {
                UI32_T pdu_len;

                hbt2_ptr1 = (STKTPLG_OM_HBT_22_T *) L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
                if(hbt2_ptr1==NULL)
                {
                    printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
                    goto exit;
                }

                memcpy(&device_info.board_info,&hbt2_ptr1->payload.unit_cfg.board_info,sizeof(STKTPLG_OM_Info_T));
                device_info.nport=hbt2_ptr1->payload.unit_cfg.nport;
                device_info.num_chips = hbt2_ptr1->payload.unit_cfg.num_chips;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_SetDeviceInfo(hbt2_ptr1->payload.unit_id, &device_info);
#else
                STKTPLG_MGR_SetDeviceInfo(hbt2_ptr1->payload.unit_id, &device_info);
#endif
            }
            else
            {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_SetDeviceInfo(hbt2_ptr->payload.unit_id, &hbt2_ptr->payload.unit_cfg);
#else
                STKTPLG_MGR_SetDeviceInfo(hbt2_ptr->payload.unit_id, &hbt2_ptr->payload.unit_cfg);
#endif
            }

            /* Advance to next unit
             */

            if (rx_port == uplink_port)
                ctrl_info_p->query_unit_up++;
            else
                ctrl_info_p->query_unit_down++;

            /* check if we already get topology information of all units
             */
            if ( (ctrl_info_p->query_unit_up   > ctrl_info_p->total_units_up  ) &&
                 (ctrl_info_p->query_unit_down > ctrl_info_p->total_units_down) )
            {
                /* Setup Port Mapping for all available units
                 */
                if (TRUE == ctrl_info_p->is_ring)
                {
                    for (i = 0; i < ctrl_info_p->total_units_down; i++)
                    {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                        STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[i]));
#else
                        for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; j++)
                        {
                            port_mapping[j].port_type
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].port_type;
                            port_mapping[j].module_id
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id & 0x1f;
                            port_mapping[j].device_id
                                = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id >> 5) & 0x07;
                            port_mapping[j].device_port_id
                                = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id ) & 0xff;
                            port_mapping[j].phy_id
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id +1;
                        }

                        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_option_port_number);
                        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_port_number);

                        if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->stable_hbt_down.payload[i].expansion_module_type,&module_info_p))
                        {
                            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable[0],sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=ctrl_info_p->stable_hbt_down.payload[i].expansion_module_id;
                            }
                        }
                        else
                        {
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=UNKNOWN_MODULE_ID;
                            }
                        }
                        STKTPLG_MGR_SetPortMapping(&port_mapping[0], ctrl_info_p->stable_hbt_down.payload[i].unit_id);
#endif
                    }

                }
                else
                {
                    for (i = 0; i < ctrl_info_p->total_units_up; i++)
                    {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

                        STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_up.payload[i]));
#else
                        for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; j++)
                        {
                            port_mapping[j].port_type
                                = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].port_type;
                            port_mapping[j].module_id
                                = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].mod_dev_id & 0x1f;
                            port_mapping[j].device_id
                                = (ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].mod_dev_id >> 5) & 0x07;
                            port_mapping[j].device_port_id
                                = (ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].device_port_phy_id) & 0xff;
                            port_mapping[j].phy_id
                                = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].device_port_phy_id +1;
                        }

                        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->stable_hbt_up.payload[i].unit_id, &max_option_port_number);
                        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->stable_hbt_up.payload[i].unit_id, &max_port_number);

                        if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->stable_hbt_up.payload[i].expansion_module_type,&module_info_p))
                        {
                            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable[0],sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=ctrl_info_p->stable_hbt_up.payload[i].expansion_module_id;
                            }
                        }
                        else
                        {
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=UNKNOWN_MODULE_ID;
                            }
                        }
                        STKTPLG_MGR_SetPortMapping(&port_mapping[0], ctrl_info_p->stable_hbt_up.payload[i].unit_id);
#endif
                    }

                    for (i = 1; i < ctrl_info_p->total_units_down; i++)
                    {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

                     STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[i]));
#else
                        for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; j++)
                        {
                            port_mapping[j].port_type
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].port_type;
                            port_mapping[j].module_id
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id & 0x1f;
                            port_mapping[j].device_id
                                = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id >> 5) & 0x07;
                            port_mapping[j].device_port_id
                                = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id ) & 0xff;
                            port_mapping[j].phy_id
                                = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id +1;
                        }

                        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_option_port_number);
                        STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_port_number);

                        if (STKTPLG_BOARD_GetModuleInformation(ctrl_info_p->stable_hbt_down.payload[i].expansion_module_type,&module_info_p))
                        {
                            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable[0],sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=ctrl_info_p->stable_hbt_down.payload[i].expansion_module_id;
                            }
                        }
                        else
                        {
                            for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            {
                                port_mapping[j].module_id=UNKNOWN_MODULE_ID;
                            }
                        }
                        STKTPLG_MGR_SetPortMapping(&port_mapping[0], ctrl_info_p->stable_hbt_down.payload[i].unit_id);
#endif
                    }
                }
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                /* prepare IUC for communication with topology information
                 */
                module_swapped = FALSE;
                STKTPLG_ENGINE_ConfigStackInfoToIUC();

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                /* change state to master and inform stack control to change
                 * system state to master state
                 */
                ctrl_info_p->state = STKTPLG_STATE_MASTER;
                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_MASTER);

                if( (ctrl_info_p->past_role==STKTPLG_STATE_INIT)||(ctrl_info_p->past_role==STKTPLG_STATE_SLAVE)||
                    ((ctrl_info_p->past_role==STKTPLG_STATE_STANDALONE)&&(!STKTPLG_OM_ENG_ProvisionCompletedOnce())))
                {
                    *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
                    xgs_stacking_debug("HandlePktGetTopologyInfoState STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE\n");
                    STKTPLG_ENGINE_MasterStorePastTplgInfo();
                    STKTPLG_OM_CopyDatabaseToSnapShot();
                    STKTPLG_MGR_ProvisionCompleted(FALSE);

                }
                else
                {
                    STKTPLG_ENGINE_CheckAndSetProvisionLost();
                    *notify_msg = STKTPLG_UNIT_HOT_INSERT_REMOVE;
                    xgs_stacking_debug("HandlePktGetTopologyInfoState STKTPLG_UNIT_HOT_INSERT_REMOVE\n");
                }

                ctrl_info_p->past_role=STKTPLG_STATE_MASTER;
                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_MASTER);
                memcpy(ctrl_info_p->past_master_mac, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);

                ctrl_info_p->stable_flag = TRUE;
#else
                /* change state to master and inform stack control to change
                 * system state to master state
                 */
                *notify_msg = STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE;
                ctrl_info_p->state = STKTPLG_STATE_MASTER;
                 STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_MASTER);
                STKTPLG_MGR_ProvisionCompleted(FALSE);

#endif


                /* Activate Master Preemption Timer
                 */
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_PREEMPTED_MASTER);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_PREEMPTED_MASTER, STKTPLG_TIMER_PREEMPTED_TIMEOUT);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);

                if (TRUE == ctrl_info_p->is_ring)
                {
                    /* Logically DISABLE Up stackng port in Master Unit if closed loop
                     */
                    ctrl_info_p->stacking_ports_logical_link_status &= ~LAN_TYPE_TX_UP_LINK;
                    ctrl_info_p->up_phy_status = TRUE;
                }
            }
            else
            {
                /* Get configuration of next unit in UP stacking port
                 */
                if (rx_port == uplink_port)
                {
                    if ( (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) &&
                         (ctrl_info_p->query_unit_up <= ctrl_info_p->total_units_up        ) )
                    {
                        xgs_stacking_debug("Send HBT2 To %d\n",ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->query_unit_up-1].unit_id);
                        STKTPLG_TX_SendHBTType2(NULL,
                                                ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->query_unit_up-1].unit_id,
                                                NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,3);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_GET_TPLG_INFO_UP, STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT);
                    }
                }
                else
                {
                    /* Get configuration of next unit in DOWN stacking port
                     */
                    if ( (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) &&
                         (ctrl_info_p->query_unit_down <= ctrl_info_p->total_units_down          ) )
                    {
                        xgs_stacking_debug("Send HBT2 To %d\n",ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->query_unit_up-1].unit_id);
                        STKTPLG_TX_SendHBTType2(NULL,
                                                ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->query_unit_down-1].unit_id,
                                                NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,4);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_GET_TPLG_INFO_DOWN, STKTPLG_TIMER_GET_TPLG_INFO_TIMEOUT);
                    }
                }
            }
        }
    }
    else if (hbt2_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: Rx TCN Packet\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port, ((hbt2_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktGetTopologyInfoState_2 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
#endif
    }
    /* Added by Vincent on 24,Aug,2004
     * TGPL_SYNC packet for mini-GBIC from slave might have been sent if
     * slave has entered slave mode.
     * We have to deal with this packet here
     */
    else if (hbt2_ptr->header.type == STKTPLG_TPLG_SYNC)
    {
        STKTPLG_ENGINE_ProcessTplgSyncPkt(rx_port, mref_handle_p);
    }
    else
    {
        /* topology has been changed
         */
        printf("[TCN] Get Topo: Rx unknown packet %d\n",hbt2_ptr->header.type);
        STKTPLG_ENGINE_LogMsg("[TCN] Get Topo: Rx unknown packet\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0,FALSE,FALSE);
        xgs_stacking_debug("HandlePktGetTopologyInfoState_3 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
    }

#endif
exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktSlaveWaitAssignState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt01_ptr;
    UI8_T                   unit, i, j;
    UI32_T                  max_port_number;
    UI32_T                  uplink_port, downlink_port;
    #if 0
    UI32_T                  max_option_port_number; /*remove variables,because it isn't used in this function, changed by Jinhua Wei*/
    #endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)

    UI32_T start_time,end_time;
    start_time =SYSFUN_GetSysTick();
#endif

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt01_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_p, &max_port_number); /* max_port_number is used as dummy here */
    if(hbt01_ptr==NULL)
    {
        xgs_stacking_debug("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(9,0,end_time);
#endif
    if (hbt01_ptr->header.type != STKTPLG_HELLO_TYPE_0)
        xgs_stacking_debug("-- slave wait state Rx from %d :type = %d \r\n", rx_port, hbt01_ptr->header.type);

    if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_1)
    {
        /* if it's my HBT1 when I launch it in previous state
         * discard it!
         */
        if (memcmp(hbt01_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
        {
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
             STKTPLG_ENGINE_BD_SetTick(9,1,end_time);
#endif
        }
        else
        {

            /* check if assigned information is correct
             */
            unit = hbt01_ptr->header.next_unit - 1;

            if (memcmp(hbt01_ptr->payload[unit].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
            {
                /* keep assigned information from master
                 */
                if (uplink_port == rx_port)
                {
                    ctrl_info_p->my_phy_unit_id_down = unit + 1;
                    ctrl_info_p->my_phy_unit_id_up   = (hbt01_ptr->payload[unit].total_units - unit) + 1;
                    memcpy(&ctrl_info_p->stable_hbt_down, hbt01_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));

                    if (TRUE == hbt01_ptr->payload[unit].is_ring)
                    {
                        memcpy(&ctrl_info_p->stable_hbt_up.header, &hbt01_ptr->header, sizeof(STKTPLG_OM_HBT_Header_T));

                        memcpy(&ctrl_info_p->stable_hbt_up.payload[0], &ctrl_info_p->stable_hbt_down.payload[0],
                        sizeof(STKTPLG_OM_HBT_0_1_Payload_T));

                        for (i = 1; i < hbt01_ptr->payload[unit].total_units_down; i++)
                        {
                            memcpy(&ctrl_info_p->stable_hbt_up.payload[i],
                                   &ctrl_info_p->stable_hbt_down.payload[hbt01_ptr->payload[unit].total_units_down - i],
                                   sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                     end_time = SYSFUN_GetSysTick()-start_time;
                     STKTPLG_ENGINE_BD_SetTick(9,2,end_time);
#endif
                    }
                    else
                    {
                        memset(&ctrl_info_p->stable_hbt_up.header, 0, sizeof(STKTPLG_OM_HBT_Header_T));
                        ctrl_info_p->stable_hbt_up.header.next_unit = hbt01_ptr->payload[0].total_units_up + 1;
                        memcpy(&ctrl_info_p->stable_hbt_up.payload[0], &hbt01_ptr->payload[0],
                               sizeof(STKTPLG_OM_HBT_0_1_Payload_T));

                        i = hbt01_ptr->payload[0].total_units_down;
                        for (j = 1; j < hbt01_ptr->payload[0].total_units_up; j++)
                        {
                            memcpy(&ctrl_info_p->stable_hbt_up.payload[j], &hbt01_ptr->payload[i++],
                                   sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                        }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,2,end_time);
#endif
                    }
                }
                else /* if (DOWN_PORT_RX == rx_port) */
                {
                    ctrl_info_p->my_phy_unit_id_up   = unit + 1;
                    ctrl_info_p->my_phy_unit_id_down = (hbt01_ptr->payload[unit].total_units - unit) + 1;
                    memcpy(&ctrl_info_p->stable_hbt_up, hbt01_ptr, sizeof(STKTPLG_OM_HBT_0_1_T));

                    if (TRUE == hbt01_ptr->payload[unit].is_ring)
                    {
                        memcpy(&ctrl_info_p->stable_hbt_down.header, &hbt01_ptr->header, sizeof(STKTPLG_OM_HBT_Header_T));

                        memcpy(&ctrl_info_p->stable_hbt_down.payload[0], &ctrl_info_p->stable_hbt_up.payload[0],
                               sizeof(STKTPLG_OM_HBT_0_1_Payload_T));

                        for (i = 1; i < hbt01_ptr->payload[unit].total_units_up; i++)
                        {
                            memcpy(&ctrl_info_p->stable_hbt_down.payload[i],
                                   &ctrl_info_p->stable_hbt_up.payload[hbt01_ptr->payload[unit].total_units_up - i],
                                   sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,3,end_time);
#endif
                    }
                    else
                    {
                        memset(&ctrl_info_p->stable_hbt_down.header, 0, sizeof(STKTPLG_OM_HBT_Header_T));
                        ctrl_info_p->stable_hbt_down.header.next_unit = hbt01_ptr->payload[0].total_units_down + 1;
                        memcpy(&ctrl_info_p->stable_hbt_down.payload[0], &hbt01_ptr->payload[0],
                               sizeof(STKTPLG_OM_HBT_0_1_Payload_T));

                        i = hbt01_ptr->payload[0].total_units_up;
                        for (j = 1; j < hbt01_ptr->payload[0].total_units_down; j++)
                        {
                            memcpy(&ctrl_info_p->stable_hbt_down.payload[j], &hbt01_ptr->payload[i++],
                                   sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                         end_time = SYSFUN_GetSysTick()-start_time;
                          STKTPLG_ENGINE_BD_SetTick(9,4,end_time);
#endif
                    }
                }
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id,&device_info);
#else
                STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
                memcpy(ctrl_info_p->master_mac, hbt01_ptr->payload[0].mac_addr, STKTPLG_MAC_ADDR_LEN);
                ctrl_info_p->master_unit_id     = hbt01_ptr->payload[0].unit_id;
#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:11:57 */
                ctrl_info_p->my_unit_id         = hbt01_ptr->payload[unit].unit_id;
#else /* SYS_CPNT_STKTPLG_SHMEM */
                STKTPLG_OM_SetMyUnitID(hbt01_ptr->payload[unit].unit_id);
#endif
                ctrl_info_p->total_units        = hbt01_ptr->payload[unit].total_units;
                ctrl_info_p->total_units_up     = hbt01_ptr->payload[unit].total_units_up;
                ctrl_info_p->total_units_down   = hbt01_ptr->payload[unit].total_units_down;
                ctrl_info_p->next_stacking_unit = hbt01_ptr->payload[unit].next_stacking_unit;
                ctrl_info_p->is_ring            = hbt01_ptr->payload[unit].is_ring;
                ctrl_info_p->start_module_id    = hbt01_ptr->payload[unit].start_module_id;
                ctrl_info_p->chip_nums          = hbt01_ptr->payload[unit].chip_nums;
                /*  ctrl_info_p->button_pressed               = hbt01_ptr->payload[unit].button_pressed; */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,5,end_time);
#endif

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                STKTPLG_OM_ENG_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#else
                STKTPLG_OM_SetDeviceInfo(ctrl_info_p->my_unit_id, &device_info);
#endif
                /* Runtime firware version of the master unit will only be set here on the slave unit.
                 * So we can call OM to set this information directly.
                 */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,6,end_time);
#endif

                STKTPLG_OM_SetUnitRuntimeFirmwareVer(ctrl_info_p->master_unit_id, hbt01_ptr->originator_runtime_fw_ver);

                for (i=0; i<hbt01_ptr->payload[0].total_units; i++)
                    STKTPLG_OM_InsertBoradID(hbt01_ptr->payload[i].unit_id, hbt01_ptr->payload[i].board_id);

                /* Setup Port Mapping for all available units
                 */
                for (i = 0; i < ctrl_info_p->total_units_up; i++)
                {

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

                   STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_up.payload[i]));
#else
                    for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; j++)
                    {
                        port_mapping[j].port_type
                            = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].port_type;
                        port_mapping[j].module_id
                            = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].mod_dev_id & 0x1f;
                        port_mapping[j].device_id
                            = (ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].mod_dev_id >> 5) & 0x07;
                        port_mapping[j].device_port_id
                            = (ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].device_port_phy_id ) & 0xff;
                        port_mapping[j].phy_id
                            = ctrl_info_p->stable_hbt_up.payload[i].port_mapping[j].device_port_phy_id +1;
                    }

                    STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->stable_hbt_up.payload[i].unit_id, &max_option_port_number);
                    STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->stable_hbt_up.payload[i].unit_id, &max_port_number);

                    for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            port_mapping[j].module_id=ctrl_info_p->stable_hbt_up.payload[i].expansion_module_id;

                    STKTPLG_MGR_SetPortMapping(&port_mapping[0], ctrl_info_p->stable_hbt_up.payload[i].unit_id);
#endif
                }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,7,end_time);
#endif

                for (i = 1; i < ctrl_info_p->total_units_down; i++)
                {

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)

                   STKTPLG_ENGINE_SetPortMappingTable(&(ctrl_info_p->stable_hbt_down.payload[i]));
#else
                    for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD; j++)
                    {
                        port_mapping[j].port_type
                            = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].port_type;
                        port_mapping[j].module_id
                            = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id & 0x1f;
                        port_mapping[j].device_id
                            = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].mod_dev_id >> 5) & 0x07;
                        port_mapping[j].device_port_id
                            = (ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id ) & 0xff;
                        port_mapping[j].phy_id
                            = ctrl_info_p->stable_hbt_down.payload[i].port_mapping[j].device_port_phy_id +1;
                    }

                    STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_option_port_number);
                    STKTPLG_OM_GetMaxPortNumberOnBoard(ctrl_info_p->stable_hbt_down.payload[i].unit_id, &max_port_number);


                    for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
                            port_mapping[j].module_id=ctrl_info_p->stable_hbt_down.payload[i].expansion_module_id;

                    STKTPLG_MGR_SetPortMapping(&port_mapping[0], ctrl_info_p->stable_hbt_down.payload[i].unit_id);
#endif
                }

                /* send the HBT1 before state change
                 */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                 end_time = SYSFUN_GetSysTick()-start_time;
                 STKTPLG_ENGINE_BD_SetTick(9,8,end_time);
#endif

                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                /*
                 * Send HBT Type 1 to kick off Get Topology in Master
                 */
                if (rx_port == uplink_port)
                {
                    if ( (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) &&
                         (ctrl_info_p->total_units_down != (unit + 1)                       ) )
                    {
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,16,end_time);
#endif
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,11);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }
                    else
                    {
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                         end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,17,end_time);
#endif
                         STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,0);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }
                }
                else /* if (rx_port == DOWN_PORT_RX) */
                {
                    if ( (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) &&
                         (ctrl_info_p->total_units_up != (unit + 1)                     ) )
                    {
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,14,end_time);
#endif
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,12);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }
                    else
                    {
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(9,15,end_time);
#endif
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,1);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }
                }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,9,end_time);
#endif

                STKTPLG_ENGINE_ConfigStackInfoToIUC();

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,10,end_time);
#endif


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                if(ctrl_info_p->past_role!=STKTPLG_STATE_SLAVE)
                {
                    *notify_msg = STKTPLG_MASTER_LOSE_MSG;
                    ctrl_info_p->past_role=STKTPLG_STATE_SLAVE;
                    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_SLAVE);
                    xgs_stacking_debug("HandlePktSlaveWaitAssignState1 STKTPLG_MASTER_LOSE_MSG\n");

                }
                else
                {
                    printf("Past_Role = SLAVE, Do SLAVE UNIT_HOT_INSERT - i.e. See if Master changed\n");
                    if( memcmp(ctrl_info_p->past_master_mac, ctrl_info_p->master_mac, STKTPLG_MAC_ADDR_LEN) != 0 )
                    {
                        printf("Master changed!, Send MASTER_LOSE_MSG again to enter Transition Mode!\n");
                        memset(ctrl_info_p->past_master_mac, 0, STKTPLG_MAC_ADDR_LEN);
                        *notify_msg = STKTPLG_MASTER_LOSE_MSG;
                        xgs_stacking_debug("HandlePktSlaveWaitAssignState2 STKTPLG_MASTER_LOSE_MSG\n");
                    }
                    else
                    {
                        printf("Same Master, Slave do nothing.\n");
                        /* master is the same and past role is slave, * it means that Slave is ready now. */
                        STKTPLG_OM_SlaveReady(TRUE);
                    }
                }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,11,end_time);
#endif
                ctrl_info_p->stable_flag = TRUE;

                ctrl_info_p->state = STKTPLG_STATE_SLAVE;

                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_SLAVE);
                STKTPLG_OM_SlaveSetUnitIsValidBits();

                STKTPLG_OM_CopyDatabaseToSnapShot();
#else
                /* Send msg to stkctrl
                 */
                *notify_msg = STKTPLG_MASTER_LOSE_MSG;

                /* State change
                 */
                ctrl_info_p->state = STKTPLG_STATE_SLAVE;
                STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_SLAVE);
                #endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(9,12,end_time);
#endif
            }
            else
            {
                // printf(" @@@@@@@ My Mac not exist in HBT1\n");
                memcpy(hbt01_ptr->payload[unit].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN);

                if (rx_port == uplink_port)
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                    if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                    {
                        /* Relay this HBT1 to DOWN stacking port
                         */
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,13);
                    }
                    else
                    {
                        /* Bounce back
                         */
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,2);
                    }

                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                }
                else
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                    if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    {
                        /* Relay this HBT0 to UP stacking port
                         */
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,14);
                    }
                    else
                    {
                        /* Bounce back
                         */
                        STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,3);
                    }

                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
             end_time = SYSFUN_GetSysTick()-start_time;
             STKTPLG_ENGINE_BD_SetTick(9,13,end_time);
#endif
            }
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_1_REBOUND)
    {
        /* if it's my HBT1 when I launch it in previous state
         * discard it!
         */
        if (memcmp(hbt01_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
        {

        }
        else
        {
            /* This is a Rebound HBT Type 1 PDU,
             * relay it to neighbour unit
             */
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    /* Relay this HBT1 to DOWN stacking port
                     */
                    STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,4);
                }
                else
                {
                    /* Bounce back ??
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,5);
                    STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                }
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP,STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
            }
            else /* rx_port == DOWN_PORT_RX */
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    /* Relay this HBT0 to UP stacking port
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,6);
                    STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                }
                else
                {
                    /* Bounce back ??
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,7);
                    STKTPLG_TX_SendHBTType1(FALSE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                }
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN,STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
            }
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */   
        UI8_T rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;
        STKTPLG_OM_HELLO_0_T* hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt01_ptr;

        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down);
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;            
        }      
        else
        {
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;
            }
            else /* if (rx_port == DOWN_PORT_RX) */
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
            }
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN} Slave Wait: Rx TCN Packet\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port, ((hbt01_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktGetTopologyInfoState_4 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
#endif
    }
    else if (hbt01_ptr->header.type == STKTPLG_TCN_TYPE_1)
    {
        STKTPLG_OM_TCN_TYPE_1_T *tcn_p;

        tcn_p = (STKTPLG_OM_TCN_TYPE_1_T *)hbt01_ptr;
        if (ctrl_info_p->my_unit_id >=1 &&
            !(tcn_p->exist_units & (1<<(ctrl_info_p->my_unit_id-1))))
        {
            *notify_msg = STKTPLG_MASTER_LOSE_MSG;
        }

        if ( (rx_port == uplink_port                                            ) &&
             (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) )
        {
            /* PDU from UP stacking port, relay to DOWN stacking port if connected
             */
            STKTPLG_TX_SendTCNType1(mref_handle_p, LAN_TYPE_TX_DOWN_LINK, 0);
            STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_TCN_TYPE_1, 0);
        }
        else if ( (rx_port == downlink_port                                      ) &&
                  (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) )
        {
            /* PDU from DOWN stacking port, relay to UP stacking port if connected
             */
            STKTPLG_TX_SendTCNType1(mref_handle_p, LAN_TYPE_TX_UP_LINK, 0);
            STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_TCN_TYPE_1,1);
        }
        else
        {

        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_0)
    {
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

        if (FALSE == STKTPLG_ENGINE_PriorityCompare(ctrl_info_p, &hbt01_ptr->payload[0]))
        {
            /* This box has LOWER priority than neighbour,
             * change state to SLAVE WAIT ASSIGNMENT.
             * ANOTHER unit claims to be MASTER !!!!!!!!!!!!!!!!!!!!!!!!!!!!
             */
            ctrl_info_p->state = STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT;
            STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_SLAVE_WAIT_ASSIGNMENT);

            stktplg_engine_debug_mode = FALSE;

            /* This is a HBT0 from HIGH priority neighbour unit and needs to be relayed */

            if (rx_port == uplink_port)
            {
                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    /* Relay this HBT0 to DOWN stacking port
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,4);
                }
                else
                {
                    /* Bounce back
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,4);
                }
               STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_UP, STKTPLG_TIMER_HBT0_TIMEOUT);
            }
            else /*if (rx_port == DOWN_PORT_RX)*/
            {
                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    /* Relay this HBT0 to UP stacking port
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0,5);
                }
                else
                {
                    /* Bounce back
                     */
                    STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,5);
                }
            }
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT0_DOWN, STKTPLG_TIMER_HBT0_TIMEOUT);
            /* Setup HBT1 timer in SLAVE_WAIT_ASSIGNMENT state
             */
        }
        else
        {
            /* This box has HIGHER priority than neighbour
             */
            /* impossible
             */
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_0_REBOUND)
    {
        /* This is a HBT0 rebound from either edges
         */
        if (memcmp(hbt01_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0)
        {
            /* This is a HBT0 PDU from neighbor units, relay to next unit
             */
            if ( (rx_port == uplink_port                                              ) &&
                 ((LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) ||
                  (TRUE == ctrl_info_p->down_phy_status                              )) )
            {
                /* PDU from UP stacking port, relay to DOWN stacking port if connected
                 */
                STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,6);
            }
            else if ( (rx_port == downlink_port                                        ) &&
                      ((LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) ||
                       (TRUE == ctrl_info_p->up_phy_status                            )) )
            {
                /* PDU from DOWN stacking port, relay to UP stacking port if connected
                 */
                STKTPLG_TX_SendHBTType0(mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_0_REBOUND,7);
            }
            else
            {
                /* phase out packets
                 */
            }
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_HALT)
    {
        /* change state to Halt
         */
        ctrl_info_p->state = STKTPLG_STATE_HALT;

        if (rx_port == uplink_port)
        {
            if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
            {
                STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);
            }
            else
            {
                STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_UP_LINK);
            }
        }
        else /* if (rx_port == DOWN_PORT_RX) */
        {
            if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
            {
                STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_UP_LINK);
            }
            else
            {
                STKTPLG_TX_SendHBT(NULL, STKTPLG_HBT_TYPE_HALT, LAN_TYPE_TX_DOWN_LINK);
            }
        }

        printf("\r\nOnly support a 8-unit stack, please remove the 9th-unit.\r\n.");
    }

#endif

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktMasterState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt1_ptr;
    UI32_T pdu_len;
    UI32_T                  uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt1_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt1_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }


    //  if (hbt1_ptr->header.type != STKTPLG_HELLO_TYPE_0)
    //    printf("-- master state Rx from %d type= %d \r\n", rx_port, hbt1_ptr->header.type);

    if (STKTPLG_ENGINE_Check_Stacking_Link_Up(ctrl_info_p,rx_port, mref_handle_p,TRUE,notify_msg) == TRUE)
        return;

    if (STKTPLG_ENGING_Check_Closed_Loop_TCN(ctrl_info_p,rx_port, mref_handle_p) == TRUE)
        return;


    if (( hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1_REBOUND) ||
        ( hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1) )
    {
        /* check if this is my HBT1
         */
        if (memcmp(hbt1_ptr->payload[0].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) == 0)
        {
            /* my HBT1
             */


            STKTPLG_ENGINE_PktMasterStateOmProcess(ctrl_info_p, hbt1_ptr, rx_port);

            if (rx_port == uplink_port)
            {
                if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                    hbt1_down_timeout_count = 0 ;
                }
                else
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    hbt1_up_timeout_count = 0 ;
                }

                /* Vincent: Because of timing issue the modules in slave units
                 * might be discovered after master enter master state. Thus we
                 * have to wait few more seconds to disable hot swap feature
                 */
                /* SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                 * on EPR ES4649-32-01434
                 */
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == FALSE)
                if (master_state_counter < MAX_HOT_SWAP_TIME)
#endif
                {
                    master_state_counter++;
                    STKTPLG_ENGINE_CheckAllOmReady(hbt1_ptr);
                }

                /* check if topology change
                 */
                if (TRUE == STKTPLG_ENGINE_TopologyIsAllRightUp(hbt1_ptr))
                {

                    /* start a timer to defer the transmiting of new HBT 1
                     * send out HBT type 1 and start related timer
                     */
                    //  printf("Start a timer to send HBT1\n");
                    if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                    else
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                }
                else
                {
                    /* topology change
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN} Master: HBT1 Topo_Up Error\n");
                    tcnReason = 5;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                    xgs_stacking_debug("HandlePktMasterState_1 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
                    *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
                }
            }
            else
            {
                if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                    hbt1_up_timeout_count = 0 ;
                }
                else
                {
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                    hbt1_down_timeout_count = 0 ;
                }

                /*for hot swap*/
                /* SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                 * on EPR ES4649-32-01434
                 */
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == FALSE)
                if (master_state_counter < MAX_HOT_SWAP_TIME)
#endif
                {
                    master_state_counter++;
                    STKTPLG_ENGINE_CheckAllOmReady(hbt1_ptr);
                }
                /* check if topology change
                 */
                if (TRUE == STKTPLG_ENGINE_TopologyIsAllRightDown(hbt1_ptr))
                {

                    //  printf("From Down Start a Timer to send HBT1\n");

                    /* start a timer to defer the transmiting of new HBT 1
                     * send out HBT type 1 and start related timer
                     */
                    if (hbt1_ptr->header.type == STKTPLG_HBT_TYPE_1)
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                    else
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                }
                else
                {
                    /* topology change
                     */
                    STKTPLG_ENGINE_LogMsg("[TCN] Master: HBT1 topo_Down Error\n");
                    tcnReason = 5;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                    STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                    xgs_stacking_debug("HandlePktMasterState_2 TOPO change\n");
#else
                    STKTPLG_ENGINE_ReturnToArbitrationState(0);
                    *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
                }
            }
            STKTPLG_ENGINE_SetStackingPortLinkStatus(ctrl_info_p->is_ring, 
                    rx_port==uplink_port, hbt1_ptr);            
        }
        else
        {
            /* topology has been changed
             */
            STKTPLG_ENGINE_LogMsg("[TCN] Mastre: Rx others HBT1\n");
            tcnReason = 5;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
             STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
             xgs_stacking_debug("HandlePktMasterState_3 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
        }
    }
    else if (hbt1_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */

        /* change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg(" TCN From unit \r\n");
        tcnReason = 6;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port, ((hbt1_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktMasterState_4 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
    }
    else if (hbt1_ptr->header.type == STKTPLG_TPLG_SYNC)
    {
        STKTPLG_ENGINE_ProcessTplgSyncPkt(rx_port, mref_handle_p);
    }
    else if (hbt1_ptr->header.type == STKTPLG_TCN_TYPE_1)
    {
        /* TCN_TYPE_1 could be received by master unit again
         * if there is a new unit inserted into the stack and
         * the stack topology is connected as ring.
         * In this case, the master unit just ignore the HBT
         */
    }
    else
    {
        /* topology has been changed
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Master: Rx unknown packet\n");
        tcnReason = 5;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("HandlePktMasterState_5 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
    }

#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


static void STKTPLG_ENGINE_HandlePktSlaveState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{

#if (SYS_CPNT_STACKING == TRUE)

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt01_ptr;
    STKTPLG_OM_HELLO_0_T    *hello_ptr;
    UI8_T                   unit;
    UI32_T                  i,j, done;
    UI32_T                  uplink_port, downlink_port;
    STKTPLG_OM_HBT_3_T      *tcn_ptr;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    UI32_T start_time,end_time;
    start_time = SYSFUN_GetSysTick();
#endif

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt01_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy here */
    if(hbt01_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    if (hbt01_ptr->header.type != STKTPLG_HELLO_TYPE_0)
        xgs_stacking_debug("-- slave state Rx from  %d :type = %d \r\n", rx_port, hbt01_ptr->header.type);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(7,0,end_time);
#endif

    if (hbt01_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */    
        UI8_T rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;

        hello_ptr = (STKTPLG_OM_HELLO_0_T*)hbt01_ptr;            
        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down); 
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;   
        }      
        else
        {
            if (rx_port == downlink_port)
            {
                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                ctrl_info_p->down_phy_status = TRUE;
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
                down_new_link = FALSE;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(7,1,end_time);
#endif
            }
            else
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
                if (FALSE == STKTPLG_ENGINE_NextDownFromMaster())
                {
                    if (LAN_TYPE_TX_UP_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    {
                        /* Link UP -- Compare Mac to check if it is a new unit or
                         *            to result a ring (should send CLOSE_LOOP_TCN)
                         */
                        //printf("A new hello packet Rxed\n");
                        hello_ptr = (STKTPLG_OM_HELLO_0_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy here */
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(8,5,end_time);
#endif

                        if(hello_ptr==NULL)
                        {
                            printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
                            goto exit;
                        }

                        for(done=0, i = 0; (i < ctrl_info_p->total_units_up) && (0 == done); i++)
                        {
                            if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_up.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                                done = 1;
                        }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                       end_time = SYSFUN_GetSysTick()-start_time;
                       STKTPLG_ENGINE_BD_SetTick(8,0,end_time);
#endif
                        for(i = 1; (i < ctrl_info_p->total_units_down) && (0 == done); i++)
                        {
                            if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_down.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                                done = 1;
                        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(8,1,end_time);
#endif
                        if (1 == done)
                        {
                            /* Join To a Ring */
                            printf("Line Joined to be Closed Loop\n");
                            for (i = ctrl_info_p->total_units_down -1; i > 0; i--)
                            {
                                memcpy(&ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - i -1],
                                       &ctrl_info_p->stable_hbt_down.payload[i],
                                       sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                            }
                            ctrl_info_p->total_units_up = ctrl_info_p->total_units_up + ctrl_info_p->total_units_down -1 ;
                            for (i = ctrl_info_p->total_units_up -1; i > 0; i--)
                            {
                                memcpy(&ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_up - i],
                                &ctrl_info_p->stable_hbt_up.payload[i],
                                sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                            }
                            ctrl_info_p->total_units_down = ctrl_info_p->total_units_up ;

                            ctrl_info_p->is_ring = TRUE ;
                            /* Notify neighbors for closed loop re-topology
                             */
                            STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);

                            /* If a link is connected to form a ring , no action */
                            /*  STKTPLG_ENGINE_ConfigStackInfoToIUC();*/

                        ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                        end_time = SYSFUN_GetSysTick()-start_time;
                        STKTPLG_ENGINE_BD_SetTick(7,2,end_time);
#endif
                        }
                        else
                        {
                            STKTPLG_ENGINE_LogMsg("[TCN] Slave: UP Add an unit \r\n");
                            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                            /* change state to arbitration
                             */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                            xgs_stacking_debug("HandlePktSlaveState_1 TOPO change\n");
#else
                            STKTPLG_ENGINE_ReturnToArbitrationState(0);
                            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                            end_time = SYSFUN_GetSysTick()-start_time;
                            STKTPLG_ENGINE_BD_SetTick(7,3,end_time);
#endif
                        }

                    } /* join a link to be a loop */
                }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(8,2,end_time);
#endif

                ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                ctrl_info_p->up_phy_status = TRUE;
                up_new_link = FALSE;
            }/* from up */

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
             end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(8,3,end_time);
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(7,4,end_time);
#endif
        }
    }/* hello_0 */
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_0)
    {
        STKTPLG_ENGINE_LogMsg("[TCN] Slave: Rx HBT0\n");
 #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("HandlePktSlaveState_2 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(7,5,end_time);
#endif

    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_1)
    {
        /* HBT Type 1 for topology maintiance
        */
        //  stktplg_engine_debug_mode = FALSE ;
        unit = hbt01_ptr->header.next_unit;
        /*add by 1219 */
        if(ctrl_info_p->expansion_module_id!=hbt01_ptr->payload[unit-1].expansion_module_id)
        {
            slave_expansion_assignevsend=FALSE;
        }
        ctrl_info_p->expansion_module_id=hbt01_ptr->payload[unit-1].expansion_module_id;
        //  printf("slaver recv the expansion_module_id =%d\r\n",ctrl_info_p->expansion_module_id);

        if ( ((unit != ctrl_info_p->my_phy_unit_id_up  ) && (downlink_port == rx_port)                     ) ||
             ((unit != ctrl_info_p->my_phy_unit_id_down) && (uplink_port == rx_port)                       ) ||
             (memcmp(hbt01_ptr->payload[0].mac_addr, ctrl_info_p->master_mac, STKTPLG_MAC_ADDR_LEN   ) != 0) ||
             (memcmp(hbt01_ptr->payload[unit - 1].mac_addr, ctrl_info_p->my_mac, STKTPLG_MAC_ADDR_LEN) != 0) )
        {
            /* Information MIS-match (topology change)
             */
            STKTPLG_ENGINE_LogMsg("[TCN] Slave: Rx HBT1 with wrong mac\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("HandlePktSlaveState_3 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
        }
        else
        {
            /* Store Preemption from Master
             */
            ctrl_info_p->preempted_master = hbt01_ptr->payload[0].preempted;
            ctrl_info_p->is_ring   = hbt01_ptr->payload[0].is_ring;

            ctrl_info_p->stable_hbt_up.payload[0].button_pressed   = hbt01_ptr->payload[0].button_pressed;
            ctrl_info_p->stable_hbt_down.payload[0].button_pressed = hbt01_ptr->payload[0].button_pressed;

            ctrl_info_p->stack_maintenance = hbt01_ptr->payload[0].master_provision_completed;
            /*1223*/
            ctrl_info_p->provision_completed_state = hbt01_ptr->payload[0].provision_completed_state;
            /* this is receive the option module  notify
             */
            //printf("############slave state insert Boradid@@@@@@@@@@@@@@@@@@\r\n");
            for (i=0; i<hbt01_ptr->payload[0].total_units; i++)
                STKTPLG_OM_InsertBoradID(hbt01_ptr->payload[i].unit_id, hbt01_ptr->payload[i].board_id);

            STKTPLG_ENGINE_SetStackingPortLinkStatus(ctrl_info_p->is_ring, 
                    rx_port==uplink_port, hbt01_ptr);            

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
             end_time = SYSFUN_GetSysTick()-start_time;
             STKTPLG_ENGINE_BD_SetTick(7,6,end_time);
#endif

            STKTPLG_ENGINE_PktSlaveStateOmProcess(ctrl_info_p ,hbt01_ptr);

            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                //printf("ring(%d) (%d - %d) units_down=%d next_unit=%d\n",ctrl_info_p->is_ring,ctrl_info_p->stacking_ports_logical_link_status,ctrl_info_p->down_phy_status,ctrl_info_p->total_units_down,hbt01_ptr->header.next_unit);
                if (TRUE == ctrl_info_p->is_ring)
                {
                    if (ctrl_info_p->total_units_down == hbt01_ptr->header.next_unit)
                    {
                        /* Last unit in closed loop and Down port of it is 'opened',
                         * so bounce back.
                         */
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,8);
                        STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE); /* ##@@@@ */
                    }
                    else
                    {
                        /* Relay this HBT1 to DOWN stacking port
                         */
                        STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,15);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                    end_time = SYSFUN_GetSysTick()-start_time;
                    STKTPLG_ENGINE_BD_SetTick(7,7,end_time);
#endif
                }
                else
                {

                    if( ( (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) &&
                       (TRUE == ctrl_info_p->down_phy_status                             ) ) &&
                         (ctrl_info_p->total_units_down != hbt01_ptr->header.next_unit)            )
                    {
                        /* Relay this HBT1 to DOWN stacking port
                         */
                        STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                         STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,16);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
                    }
                    else
                    {
                        /* Bounce back
                         */
                        STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                        STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,9);
                        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);/* @@@@ */
                    }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                     end_time = SYSFUN_GetSysTick()-start_time;
                     STKTPLG_ENGINE_BD_SetTick(7,8,end_time);
#endif
                }
            }
            else /*if (rx_port == DOWN_PORT_RX)*/
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);


#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                end_time = SYSFUN_GetSysTick()-start_time;
                STKTPLG_ENGINE_BD_SetTick(8,6,end_time);
#endif

                if (TRUE == ctrl_info_p->is_ring)
                    STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);

                if ( (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) &&
                     (TRUE == ctrl_info_p->up_phy_status ) &&
                     (ctrl_info_p->total_units_up != hbt01_ptr->header.next_unit)     )
                {
                    /* Relay this HBT1 to UP stacking port
                     */
                    STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                    end_time = SYSFUN_GetSysTick()-start_time;
                    STKTPLG_ENGINE_BD_SetTick(8,8,end_time);
#endif
                     STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,16);
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                   end_time = SYSFUN_GetSysTick()-start_time;
                   STKTPLG_ENGINE_BD_SetTick(8,7,end_time);
#endif
                }
                else
                {
                    /* Bounce back
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,10);
                    STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
                    end_time = SYSFUN_GetSysTick()-start_time;
                    STKTPLG_ENGINE_BD_SetTick(8,9,end_time);
#endif
                    STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);/* @@@@ */
                }
            }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(7,9,end_time);
#endif

            STKTPLG_ENGINE_SlaveSendAssignEvToOm(ctrl_info_p);

        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_1_REBOUND)
    {
        /* Relay the rebounced PDU to next unit down the road
         */
        if (rx_port == uplink_port)
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);/* @@@@ */

          /*  if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
            { */
            STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,11);
            STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE); /* @@@@ */
           /* }
            else
            {
                 Bounce back ???

                STKTPLG_TX_SendHBTType1(TRUE,mem_ref, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
            } */
        }
        else /*if (rx_port == DOWN_PORT_RX)*/
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN); /* @@@@ */

           /* if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
            { */
           STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1_REBOUND,12);
            STKTPLG_TX_SendHBTType1(TRUE,mref_handle_p, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE); /* @@@@ */
           /* }
            else
            {
                 Bounce back ????

                STKTPLG_TX_SendHBTType1(TRUE,mem_ref, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_SLAVE);
            } */
        }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(7,10,end_time);
#endif
    }
    else if (hbt01_ptr->header.type == STKTPLG_HBT_TYPE_2)
    {
        /* Reply topology informaiton and configuration enquire
         * to master unit
         */
        if (rx_port == uplink_port)
        {
            if (hbt01_ptr->header.next_unit == ctrl_info_p->my_unit_id)
            {
                /* This is the PDU used to query our configuration,
                 * reply this HBT 2 to UP stacking port
                 */
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,5);
                STKTPLG_TX_SendHBTType2(mref_handle_p, 0, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_DOWN,STKTPLG_TIMER_HBT1_TIMEOUT_MASTER);
            }
            else
            {
                /* This is NOT the PDU used to query OUR configuration,
                 * relay it to next available unit
                 */
                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    /* Relay this HBT 2 to DOWN stacking port
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,6);
                    STKTPLG_TX_SendHBTType2(mref_handle_p, 0, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                }
                else
                {
                    /* Bounce back
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,7);
                    STKTPLG_TX_SendHBTType2(mref_handle_p, 0, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                }
            }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
           end_time = SYSFUN_GetSysTick()-start_time;
           STKTPLG_ENGINE_BD_SetTick(7,11,end_time);
#endif
        }
        else /* if (rx_port == DOWN_PORT_RX)*/
        {
            if (hbt01_ptr->header.next_unit == ctrl_info_p->my_unit_id)
            {
                /* Reply this HBT 2 to DOWN stacking port
                 */
                STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,8);
                STKTPLG_TX_SendHBTType2(mref_handle_p, 0, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP,STKTPLG_TIMER_HBT1_TIMEOUT_MASTER);

            }
            else
            {
                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    /* Relay this HBT 2 to UP stacking port
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,9);
                    STKTPLG_TX_SendHBTType2(mref_handle_p, 0, BOUNCE_PDU, LAN_TYPE_TX_UP_LINK);
                }
                else
                {
                    /* Bounce back
                     */
                    STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_2,10);
                    STKTPLG_TX_SendHBTType2(mref_handle_p, 0, BOUNCE_PDU, LAN_TYPE_TX_DOWN_LINK);
                }
            }
#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
            end_time = SYSFUN_GetSysTick()-start_time;
            STKTPLG_ENGINE_BD_SetTick(7,12,end_time);
#endif
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_CLOSED_LOOP_TCN)
    {
        /*here should be  STKTPLG_OM_HBT_3_T,or will cause pkt parse error and will cause exception*/
        tcn_ptr = (STKTPLG_OM_HBT_3_T *) L_MM_Mref_GetPdu(mref_handle_p, &i);

        if (ctrl_info_p->is_ring != tcn_ptr->payload.is_ring)
        {
            ctrl_info_p->is_ring = tcn_ptr->payload.is_ring;

            if (TRUE == ctrl_info_p->is_ring)
            {

                for (i = ctrl_info_p->total_units_down -1; i > 0; i--)
                {
                    memcpy(&ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - i -1],
                           &ctrl_info_p->stable_hbt_down.payload[i],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }

                ctrl_info_p->total_units_up = ctrl_info_p->total_units_up + ctrl_info_p->total_units_down -1 ;
                for (i = ctrl_info_p->total_units_up -1; i > 0; i--)
                {
                    memcpy(&ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_up - i],
                           &ctrl_info_p->stable_hbt_up.payload[i],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }
                ctrl_info_p->total_units_down = ctrl_info_p->total_units_up ;
                ctrl_info_p->total_units = ctrl_info_p->total_units_up;

            }
            else
            {

                ctrl_info_p->total_units_up   = tcn_ptr->payload.total_units_up;
                ctrl_info_p->total_units_down = tcn_ptr->payload.total_units_down;

            }

            ctrl_info_p->stable_hbt_up.header.next_unit   = ctrl_info_p->total_units_up + 1;
            ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;

            if (rx_port == uplink_port)
            {
                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                    STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
            }
            else /* if (rx_port == DOWN_PORT_RX) */
            {
                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
            }
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);

            for (j = ctrl_info_p->total_units_up - 1; j > 0; j--)
                xgs_stacking_debug(" [%s]%d up[%ld] = %d\r\n",__FUNCTION__,__LINE__, j, ctrl_info_p->stable_hbt_up.payload[j].unit_id);
            for (j = 0; j < ctrl_info_p->total_units_down; j++)
                xgs_stacking_debug(" [%s]%d down[%ld] = %d\r\n", __FUNCTION__,__LINE__,j, ctrl_info_p->stable_hbt_down.payload[j].unit_id);
           /*  If a link is connected to form a ring , no action */
            if (FALSE == ctrl_info_p->is_ring)
            {
                STKTPLG_ENGINE_ConfigStackInfoToIUC();
            }
        }

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(7,13,end_time);
#endif

    }
    else if (hbt01_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */

        STKTPLG_ENGINE_LogMsg(" [TCN] Slave: Rx TCN packet \r\n");

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port, ((hbt01_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktSlaveState_3 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
        *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
        end_time = SYSFUN_GetSysTick()-start_time;
        STKTPLG_ENGINE_BD_SetTick(7,14,end_time);
#endif
    }
    else if (hbt01_ptr->header.type == STKTPLG_TCN_TYPE_1)
    {
        STKTPLG_OM_TCN_TYPE_1_T *tcn_p;

        tcn_p = (STKTPLG_OM_TCN_TYPE_1_T *)hbt01_ptr;
        if (ctrl_info_p->my_unit_id >=1 &&
            !(tcn_p->exist_units & (1<<(ctrl_info_p->my_unit_id-1))))
        {
            *notify_msg = STKTPLG_MASTER_LOSE_MSG;
        }

        if ( (rx_port == uplink_port                                            ) &&
             (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)) )
        {
            /* PDU from UP stacking port, relay to DOWN stacking port if connected
             */
            STKTPLG_TX_SendTCNType1(mref_handle_p, LAN_TYPE_TX_DOWN_LINK, 0);
            STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_TCN_TYPE_1, 2);
        }
        else if ( (rx_port == downlink_port                                      ) &&
                  (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)) )
        {
            /* PDU from DOWN stacking port, relay to UP stacking port if connected
             */
            STKTPLG_TX_SendTCNType1(mref_handle_p, LAN_TYPE_TX_UP_LINK, 0);
            STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_TCN_TYPE_1,3);
        }
    }
    else if (hbt01_ptr->header.type == STKTPLG_TPLG_SYNC)
    {
        STKTPLG_ENGINE_ProcessTplgSyncPkt(rx_port, mref_handle_p);
    }
#endif

#if(SYS_CPNT_DEBUG_STK_PERFORMANCE == TRUE)
    end_time = SYSFUN_GetSysTick()-start_time;
    STKTPLG_ENGINE_BD_SetTick(7,15,end_time);
#endif
exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;

}

static void STKTPLG_ENGINE_HandlePktHaltState(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, UI32_T *notify_msg)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_T    *hbt0_ptr;
    UI32_T                   pdu_len;
    UI32_T                   uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        goto exit;
    }

    if(mref_handle_p == NULL || notify_msg == NULL || (rx_port != uplink_port && rx_port != downlink_port))
    {
        xgs_stacking_debug("%s(%d)Unknown error.mref_handle_p=%p,notify_msg=%p,rx_port=%hu\r\n",
            __FUNCTION__, __LINE__, mref_handle_p, notify_msg, rx_port);
        goto exit;
    }


    hbt0_ptr = (STKTPLG_OM_HBT_0_1_T *)L_MM_Mref_GetPdu(mref_handle_p, &pdu_len);
    if(hbt0_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        goto exit;
    }

    xgs_stacking_debug("-- halt state : %d : %d\r\n", rx_port, hbt0_ptr->header.type);

    /* check if this is my HBT0
     */
    if (hbt0_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        if (rx_port == uplink_port)
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
        }
        else /*hello PDU source port == DOWN_PORT_RX*/
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
        }
    }
    else if (hbt0_ptr->header.type == STKTPLG_HBT_TYPE_1)
    {
        /* we should not receive other HBT1, change state to arbitration
         */
        STKTPLG_ENGINE_LogMsg("[TCN] Halt: Rx HBT0\n");
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("HandlePktHaltState_1 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif

        ctrl_info_p->reset_state = 20;

    }
 /* Added by Vincent on 26,Aug,2004
  * In Halt state if some stacking cable is disconnected
  * STKTPLG_TCN packet will be received to let every units
  * returning to arbitration mode
  */
    else if (hbt0_ptr->header.type == STKTPLG_TCN)
    {
        /* topology change
         */

        STKTPLG_ENGINE_LogMsg("[TCN] Halt: Rx TCN\n");

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port, ((hbt0_ptr->header.masters_location==RENUMBER_PDU)? TRUE:FALSE),FALSE);
        xgs_stacking_debug("HandlePktHaltState_2 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(rx_port);
#endif
    }
    else
    {
        /* should not happen, print out error message
         */
    }

exit:
    L_MM_Mref_Release(&mref_handle_p);
    return;
}


/* FUNCTION NAME: STKTPLG_ENGINE_SetStackingPortLinkStatus
 * PURPOSE: Update OM's Stacking port link status by incoming HBT1 packet
 * INPUT:   is_ring             -- is at ring topology
 *          is_rx_at_up_port    -- packet recieved at UP or Down port
 *          hbt1_ptr            -- the received HBT1 packet
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   
 */
static BOOL_T STKTPLG_ENGINE_SetStackingPortLinkStatus(
    BOOL_T is_ring, BOOL_T is_rx_at_up_port, STKTPLG_OM_HBT_0_1_T *hbt1_ptr)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    UI8_T                    total_units;
    int                      unit;
    STKTPLG_OM_HBT_0_1_Payload_T  *hbt_payload;

    if (hbt1_ptr == NULL)
    {
        return (FALSE);
    }

    if(is_ring)
    {
        total_units = ctrl_info_p->total_units ;     
        for (unit = 0; unit < total_units; unit++)
        {
            hbt_payload= &(hbt1_ptr->payload[unit]);
            /*update both stable_hbt_up and down*/
            ctrl_info_p->stable_hbt_up.payload[unit].stacking_ports_link_status = 
            hbt_payload->stacking_ports_link_status;            
            ctrl_info_p->stable_hbt_down.payload[unit].stacking_ports_link_status = 
            hbt_payload->stacking_ports_link_status;
        }
    }
    else /*LINE TOPO*/
    {
        if(is_rx_at_up_port==TRUE)  /*UP port*/
        {
            total_units = ctrl_info_p->total_units_up;  
            for (unit = 0; unit < total_units; unit++)
            {
                hbt_payload= &(hbt1_ptr->payload[unit]);
                ctrl_info_p->stable_hbt_up.payload[unit].stacking_ports_link_status = 
                hbt_payload->stacking_ports_link_status;
            }
            
            /*If no link on the master's down port, update master's link status.*/
            if(ctrl_info_p->total_units_down==1)
            {
                ctrl_info_p->stable_hbt_down.payload[0].stacking_ports_link_status
                    =ctrl_info_p->stable_hbt_up.payload[0].stacking_ports_link_status;
            }
        }
        else  /*DOWN port*/
        {
            total_units = ctrl_info_p->total_units_down;  
            for (unit = 0; unit < total_units; unit++)
            {           
                hbt_payload= &(hbt1_ptr->payload[unit]);              
                ctrl_info_p->stable_hbt_down.payload[unit].stacking_ports_link_status = 
                hbt_payload->stacking_ports_link_status;
            }
            
            /*If no link on the master's up port, update master's link status.*/
            if(ctrl_info_p->total_units_up==1)
            {
                ctrl_info_p->stable_hbt_up.payload[0].stacking_ports_link_status
                    =ctrl_info_p->stable_hbt_down.payload[0].stacking_ports_link_status;
            }            
        }
    }
    return TRUE;
}  

static BOOL_T STKTPLG_ENGINE_ButtonPressed(void)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)

    BOOL_T status;
    SYSDRV_GetPushButtonStatus(SYS_VAL_LOCAL_UNIT_ID, &status);
    ctrl_info_p->stable_hbt_up.payload[0].button_pressed = status;
    ctrl_info_p->button_pressed = status;
#else
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-15, 15:15:30 */
    ctrl_info_p->button_pressed = FALSE;
#else
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
    if(FALSE==PHYADDR_ACCESS_Read(sys_hwcfg_stack_status_addr, 1, 1, &stack_status))
    {
        SYSFUN_Debug_Printf("\r\n%s: Access SYS_HWCFG_STACK_STATUS_ADDR fail", __FUNCTION__);
        return FALSE;
    }
/*make the code common,to compare with the pressed result*/
    if ( SYS_HWCFG_STACK_BUTTON_PRESSED == (stack_status & SYS_HWCFG_STACK_BUTTON_MASK) )
    {
        ctrl_info_p->button_pressed = TRUE;
    }
    else
    {
        ctrl_info_p->button_pressed = FALSE;
    }
#else
    /* ES4649-32-1050: For compaility with older version the status of push button
     * was always TRUE.
     */
    ctrl_info_p->button_pressed = TRUE;
#endif
#endif /* ES3526MA_POE_7LF_LN */
#endif
    return (ctrl_info_p->button_pressed);

}

static BOOL_T STKTPLG_ENGINE_PrepareUnitBasicInformation(STKTPLG_OM_HBT_0_1_T *hbt1_ptr, UI8_T *module_id/* not used */)
{
    int unit;
    BOOL_T retval;

    retval = TRUE;
/*EPR:ES3628BT-FLF-ZZ-01059
Problem:stack:mac learning error after hot remove unit from ring

stack.
Rootcause:(1)after hotswap,the module id for each dut will

re-assign.And it may changed
          (2)the mac-address learned on old module id will not be

deleted ,and re-learned for the new module id
          (3)so the pkt will forward to error place
Solution:when do hot-swap,keep the module id no change if the

unit-id not changed
Files:stktplg_engine.c*/
    if(SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT *SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK-1 > MAX_MODULE_ID)
      return FALSE;

    for (unit = 0; unit < (hbt1_ptr->header.next_unit - 1); unit++)
    {
#if 0
        hbt1_ptr->payload[unit].start_module_id = *module_id;
        *module_id += hbt1_ptr->payload[unit].chip_nums;

        if (*module_id > MAX_MODULE_ID)
        {
            retval = FALSE;
            break;
        }
#else

        hbt1_ptr->payload[unit].start_module_id = (hbt1_ptr->payload[unit].unit_id-1)*SYS_ADPT_MAX_NBR_OF_CHIP_PER_UNIT;
#endif
    }

    return (retval);

}


static BOOL_T STKTPLG_ENGINE_AllSlaveReady(STKTPLG_OM_HBT_0_1_T *hbt1_ptr)
{
    int unit;

    /* check all slave machine is ready in Slave mode
     */
    for (unit = 1; /* from first slave machine( unit 2, index = 1) */
         unit < (hbt1_ptr->header.next_unit - 1); /* up to the last slave machine */
         unit++ /* next machine */)
    {

        if (!hbt1_ptr->payload[unit].slave_ready)
        {

            return (FALSE);
        }
    }
    return (TRUE);

}


static void STKTPLG_ENGINE_PrepareStackInfo(Stacking_Info_T *stack_info)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    int                     unit, index=0,uid;
    UI8_T                   device_id;

    memset(stack_info, 0, sizeof(Stacking_Info_T));

    stack_info->my_unit_id           = ctrl_info_p->my_unit_id;
    stack_info->total_units_in_stack = ctrl_info_p->total_units;
    stack_info->master_unit_id       = ctrl_info_p->master_unit_id;
    stack_info->Is_Ring              = ctrl_info_p->is_ring;
#if (SYS_CPNT_STACKING_BUTTON == TRUE)
   /*notify dev_swdrv,stacking button state*/
    stack_info->stacking_is_enable = ctrl_info_p->stacking_button_state;
#endif

    if (TRUE == ctrl_info_p->is_ring)
    {
        for(unit = 0, index = 0; unit < ctrl_info_p->total_units_down; unit++, index++)
        {
            stack_info->unit_id[index]         = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            stack_info->start_module_id[index] = ctrl_info_p->stable_hbt_down.payload[unit].start_module_id;
            stack_info->num_module_id[index]   = ctrl_info_p->stable_hbt_down.payload[unit].chip_nums;

            uid=stack_info->unit_id[index];
            stack_info->expansion_module_id[index] = ctrl_info_p->exp_module_id[uid-1];

            if (STKTPLG_OM_GetCpuEnabledDevId(ctrl_info_p->stable_hbt_down.payload[unit].board_id, &device_id) == FALSE)
            {
                printf("%s(%d):Fail to get CPU enabled device id\n", __FUNCTION__, __LINE__);
            }
            stack_info->cpu_enabled_device_id[index] = device_id;
        }
    }
    else
    {
        if (0 != ctrl_info_p->total_units_up)
        {
            for(unit = ctrl_info_p->total_units_up - 1, index = 0; unit > 0; unit--, index++)
            {
                stack_info->unit_id[index]         = ctrl_info_p->stable_hbt_up.payload[unit].unit_id;
                stack_info->start_module_id[index] = ctrl_info_p->stable_hbt_up.payload[unit].start_module_id;
                stack_info->num_module_id[index]   = ctrl_info_p->stable_hbt_up.payload[unit].chip_nums;
                uid=stack_info->unit_id[index];
                stack_info->expansion_module_id[index] = ctrl_info_p->exp_module_id[uid-1];

                if (STKTPLG_OM_GetCpuEnabledDevId(ctrl_info_p->stable_hbt_up.payload[unit].board_id, &device_id) == FALSE)
                {
                    printf("%s(%d):Fail to get CPU enabled device id\n", __FUNCTION__, __LINE__);
                }
                stack_info->cpu_enabled_device_id[index] = device_id;
            }
        }

        for(unit = 0; unit < ctrl_info_p->total_units_down; unit++, index++)
        {
            stack_info->unit_id[index]         = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            stack_info->start_module_id[index] = ctrl_info_p->stable_hbt_down.payload[unit].start_module_id;
            stack_info->num_module_id[index]   = ctrl_info_p->stable_hbt_down.payload[unit].chip_nums;
            uid=stack_info->unit_id[index];
            stack_info->expansion_module_id[index] = ctrl_info_p->exp_module_id[uid-1];

            if (STKTPLG_OM_GetCpuEnabledDevId(ctrl_info_p->stable_hbt_down.payload[unit].board_id, &device_id) == FALSE)
            {
                printf("%s(%d):Fail to get CPU enabled device id\n", __FUNCTION__, __LINE__);
            }
            stack_info->cpu_enabled_device_id[index] = device_id;
        }
    }
    STKTPLG_OM_SetStackingInfo(*stack_info);
}

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
static void STKTPLG_ENGINE_ReturnToArbitrationState(UI8_T rx_port, BOOL_T renumber,BOOL_T stkbntstate_changed)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();

    if( TRUE == renumber ||stkbntstate_changed)
    {
        /*Set do not check for unit insert/remove */
        ctrl_info_p->past_role = STKTPLG_STATE_INIT;
    }

    ctrl_info_p->state = STKTPLG_STATE_INIT;

    ctrl_info_p->reset_state = 10;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_INIT);
    /* suger, 09-22-2004,
     * set provision_completed flag to FALSE once the TCN happened to prevent
     * module state machine mis-understand mainboard already provision completed.
     */
    STKTPLG_OM_ProvisionCompleted(FALSE);

    /* set slave_ready to FALSE to prevent slave relay HBT1 with
     * slave_ready = TRUE before slave ready.
     */
    STKTPLG_OM_SlaveReady(FALSE);

    ctrl_info_p->stable_flag = FALSE;

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
    {
        UI32_T uplink_port, downlink_port;

        if (UP_PORT_RX(&uplink_port) == FALSE)
        {
            xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.\n", __FUNCTION__, __LINE__);
            return;
        }
        if (DOWN_PORT_RX(&downlink_port) == FALSE)
        {
            xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.\n", __FUNCTION__, __LINE__);
            return;
        }

#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
        /* If stacking port is enable,send pkt
         */
        if (((rx_port == 0) || (rx_port == downlink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK)&&
            (TRUE == ctrl_info_p->stacking_button_state))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_UP_LINK,renumber);
        if (((rx_port == 0) || (rx_port == uplink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK)&&
            (TRUE == ctrl_info_p->stacking_button_state))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_DOWN_LINK,renumber);
#else
        if (((rx_port == 0) || (rx_port == downlink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_UP_LINK,renumber);
        if (((rx_port == 0) || (rx_port == uplink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_DOWN_LINK,renumber);
#endif
    }
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1) */

    return;
}
#else /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
static void STKTPLG_ENGINE_ReturnToArbitrationState(UI8_T rx_port)
{
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;

    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();

    ctrl_info_p->state = STKTPLG_STATE_INIT;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_INIT);
    ctrl_info_p->reset_state = 10;

    /* suger, 09-22-2004,
     * set provision_completed flag to FALSE once the TCN happened to prevent
     * module state machine mis-understand mainboard already provision completed.
     */
    STKTPLG_OM_ProvisionCompleted(FALSE);

   // stktplg_engine_debug_mode = TRUE;

#if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1)
    {
        UI32_T uplink_port, downlink_port;

        if (UP_PORT_RX(&uplink_port) == FALSE)
        {
            xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.\n", __FUNCTION__, __LINE__);
            return;
        }
        if (DOWN_PORT_RX(&downlink_port) == FALSE)
        {
            xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.\n", __FUNCTION__, __LINE__);
            return;
        }

        if (((rx_port == 0) || (rx_port == downlink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_UP_LINK);
        if (((rx_port == 0) || (rx_port == uplink_port)) && (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
            STKTPLG_TX_SendTCN(NULL, LAN_TYPE_TX_DOWN_LINK);
    }
#endif /* end of #if (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK>1) */

    return;
}
#endif /* #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE) */
#else /* #if (SYS_CPNT_STACKING == TRUE) */
static void STKTPLG_ENGINE_ReturnToArbitrationState(UI8_T rx_port)
{
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;

    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();

    ctrl_info_p->state = STKTPLG_STATE_INIT;
    STKTPLG_BACKDOOR_SaveTopoState(STKTPLG_STATE_INIT);
    ctrl_info_p->reset_state = 10;

    /* suger, 09-22-2004,
     * set provision_completed flag to FALSE once the TCN happened to prevent
     * module state machine mis-understand mainboard already provision completed.
     */
    STKTPLG_OM_ProvisionCompleted(FALSE);

    return;
}
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

static BOOL_T STKTPLG_ENGINE_ConfigStackInfoToIUC(void)
{

#if (SYS_CPNT_STACKING == TRUE)

    Stacking_Info_T  stack_info;
    int              unit, index;
    BOOL_T           ret;

    /* Added by Vincent 14,Jun,04
     * Set Module Id to 0xff for all ports of units not exist
     */
    for(index=0; index <SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; index++)
    {
        port_mapping[index].module_id=UNKNOWN_MODULE_ID;
    }

    for (unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit ++)
    {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        if (!STKTPLG_OM_ENG_UnitExist(unit))
#else
        if (!STKTPLG_OM_UnitExist(unit))
#endif
        {
            STKTPLG_MGR_SetPortMapping(&port_mapping[0], unit);
        }
    }
    STKTPLG_ENGINE_PrepareStackInfo(&stack_info);

    ret = DEV_SWDRV_PMGR_ConfigTopologyInfo(&stack_info);
    STKTPLG_SHOM_SetConfigTopologyInfoDoneStatus(TRUE);
    return ret;

#else /* #if (SYS_CPNT_STACKING == TRUE) */

    return FALSE;

#endif /* end of #if (SYS_CPNT_STACKING == TRUE) */

}

#if (SYS_CPNT_STACKING == TRUE)
/*
 *  This function will return TRUE if self has higher priority over
 *  next.
 *
 */
static BOOL_T STKTPLG_ENGINE_PriorityCompare(STKTPLG_OM_Ctrl_Info_T *self,
                                             STKTPLG_OM_HBT_0_1_Payload_T *next)
{
    if(self == NULL || next == NULL)
        return FALSE;

    if (next->preempted == self->preempted)
    {
        if (next->button_pressed == self->button_pressed)
        {
            if (memcmp(next->mac_addr, self->my_mac, STKTPLG_MAC_ADDR_LEN) <0)
            {
               xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result FALSE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

                return FALSE;
            }
            else
            {
               xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result TRUE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

               return TRUE;
            }
        }
        else
        {
            if (TRUE == next->button_pressed)
            {
              xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result FALSE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

                return FALSE;
            }
            else
            {
               xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result TRUE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

             return TRUE;
            }
        }
    }
    else
    {
        if (TRUE == next->preempted)
        {
           xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result FALSE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

            return FALSE;
        }
        else
        {
            xgs_stacking_debug("My premp=%d button=%d  Pkt premp=%d button=%d\n",self->preempted,self->button_pressed,next->preempted,next->button_pressed);
               xgs_stacking_debug("My mac=%x-%x-%x-%x-%x-%x Pkt mac=%x-%x-%x-%x-%x-%x,result TRUE\n",self->my_mac[0],self->my_mac[1],self->my_mac[2],self->my_mac[3],
               self->my_mac[4],self->my_mac[5],next->mac_addr[0],next->mac_addr[1],next->mac_addr[2],next->mac_addr[3],next->mac_addr[4],next->mac_addr[5]);

            return TRUE;
        }
    }

}

#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
static BOOL_T STKTPLG_ENGINE_ButtonStateChanged(void)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    UI8_T                   button_status = 0;
    BOOL_T                  retval = FALSE;

    /* get status of button for master election
     */
    if(FALSE==PHYADDR_ACCESS_Read(sys_hwcfg_stack_status_addr, 1, 1, &button_status))
    {
        SYSFUN_Debug_Printf("\r\n%s: Access SYS_HWCFG_STACK_STATUS_ADDR fail", __FUNCTION__);
        return FALSE;
    }

    /* check if button is pressed or not
     *
     * SYS_HWCFG_STACK_BUTTON_MASK - 0 (button pressed)
     * SYS_HWCFG_STACK_BUTTON_MASK - 1 (button released)
     *
     */
    button_status = ((button_status & SYS_HWCFG_STACK_BUTTON_MASK)==SYS_HWCFG_STACK_BUTTON_PRESSED) ?  TRUE : FALSE;

    if (button_status != ctrl_info_p->button_pressed)
    {
        ctrl_info_p->button_pressed = button_status;
        retval = TRUE;
    }

    return (retval);
}
#endif

static BOOL_T STKTPLG_ENGINE_TopologyIsAllRightUp(STKTPLG_OM_HBT_0_1_T *hbt1_ptr)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_Payload_T  hbt_payload;
    UI8_T                         total_units;
    int                           unit, size = sizeof(STKTPLG_OM_HBT_0_1_Payload_T);

    total_units = ctrl_info_p->total_units_up;

    if (hbt1_ptr->header.next_unit != (total_units + 1))
    {
        return (FALSE);
    }

    for (unit = 0; unit < total_units; unit++)
    {
        memcpy(&hbt_payload, &ctrl_info_p->stable_hbt_up.payload[unit], size);

        hbt_payload.slave_ready = hbt1_ptr->payload[unit].slave_ready;

        hbt_payload.expansion_module_type  = hbt1_ptr->payload[unit].expansion_module_type;
        hbt_payload.expansion_module_id    = hbt1_ptr->payload[unit].expansion_module_id;
        hbt_payload.expansion_module_exist = hbt1_ptr->payload[unit].expansion_module_exist;
        hbt_payload.expansion_module_ready = hbt1_ptr->payload[unit].expansion_module_ready;
        /*1223*/
        hbt_payload.provision_completed_state= hbt1_ptr->payload[unit].provision_completed_state;
        hbt_payload.button_pressed         = hbt1_ptr->payload[unit].button_pressed;
        hbt_payload.preempted              = hbt1_ptr->payload[unit].preempted;
        hbt_payload.is_ring                = hbt1_ptr->payload[unit].is_ring;
        hbt_payload.board_id               = hbt1_ptr->payload[unit].board_id;
        memcpy(hbt_payload.module_runtime_fw_ver,hbt1_ptr->payload[unit].module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
        hbt_payload.stacking_ports_link_status = hbt1_ptr->payload[unit].stacking_ports_link_status;


#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
        memcpy(hbt_payload.teng_module_id, hbt1_ptr->payload[unit].teng_module_id, sizeof(hbt_payload.teng_module_id));
#endif
        if (memcmp(&hbt_payload, &hbt1_ptr->payload[unit], size) != 0)
        {
            return (FALSE);
        }
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
        if (first_up_button_status)
#endif
        {
            ctrl_info_p->stable_hbt_up.payload[unit].button_pressed = hbt_payload.button_pressed;
        }

    }
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
    first_up_button_status = FALSE ;
#endif
    return (TRUE);
}

static BOOL_T STKTPLG_ENGINE_TopologyIsAllRightDown(STKTPLG_OM_HBT_0_1_T *hbt1_ptr)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    STKTPLG_OM_HBT_0_1_Payload_T  hbt_payload;
    UI8_T                         total_units;
    int                           unit, size = sizeof(STKTPLG_OM_HBT_0_1_Payload_T);



    total_units = ctrl_info_p->total_units_down ;

    if (hbt1_ptr->header.next_unit != (total_units + 1))
    {
        return (FALSE);
    }

    for (unit = 0; unit < total_units; unit++)
    {
        memcpy(&hbt_payload, &ctrl_info_p->stable_hbt_down.payload[unit], size);

        hbt_payload.slave_ready = hbt1_ptr->payload[unit].slave_ready;

        hbt_payload.expansion_module_type  = hbt1_ptr->payload[unit].expansion_module_type;
        hbt_payload.expansion_module_id    = hbt1_ptr->payload[unit].expansion_module_id;
        hbt_payload.expansion_module_exist = hbt1_ptr->payload[unit].expansion_module_exist;
        hbt_payload.expansion_module_ready = hbt1_ptr->payload[unit].expansion_module_ready;
        /*1223*/
        hbt_payload.provision_completed_state= hbt1_ptr->payload[unit].provision_completed_state;
        hbt_payload.button_pressed         = hbt1_ptr->payload[unit].button_pressed;
        hbt_payload.preempted              = hbt1_ptr->payload[unit].preempted;
        hbt_payload.is_ring                = hbt1_ptr->payload[unit].is_ring;
        hbt_payload.board_id             = hbt1_ptr->payload[unit].board_id;
        memcpy(hbt_payload.module_runtime_fw_ver,hbt1_ptr->payload[unit].module_runtime_fw_ver, SYS_ADPT_FW_VER_STR_LEN + 1);
        hbt_payload.stacking_ports_link_status = hbt1_ptr->payload[unit].stacking_ports_link_status;
#if (SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT>0)
        memcpy(hbt_payload.teng_module_id, hbt1_ptr->payload[unit].teng_module_id, sizeof(hbt_payload.teng_module_id));
#endif

        if (memcmp(&hbt_payload, &hbt1_ptr->payload[unit], size) != 0)
        {
            return (FALSE);
        }
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
        if (first_down_button_status)
#endif
        {
            ctrl_info_p->stable_hbt_down.payload[unit].button_pressed = hbt_payload.button_pressed;
        }
    }
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_SOFTWARE)
    first_down_button_status = FALSE ;
#endif
    return (TRUE);

}

/* FUNCTION NAME: STKTPLG_ENGINE_CheckHelloTimer0
 * PURPOSE: Outputs logic link status by checking phy_status and Hello0 timer.
 * INPUT:   None
 * OUTPUT:  logic_link_status_p    -- updated stacking link status.
 * RETUEN:  None
 * NOTES:   Check if Hello Type 0 Timer expired in order to determine 
 *          whether stacking ports are available.
 *          If phy status is off, so is logic link.   
 *          
 *          logic_link_status has 2 bits to represent link status.
 *            -- Bit LAN_TYPE_TX_UP_LINK: 0=>up port is invalid, 1=> valid.
 *            -- Bit LAN_TYPE_TX_DOWN_LINK: 0=>down port is invalid, 1=> valid.  
 */
static void STKTPLG_ENGINE_CheckHelloTimer0(UI8_T *logic_link_status_p)
{
    UI32_T delta;
    UI32_T up_phy_status=FALSE, down_phy_status=FALSE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if(FALSE==STKTPLG_SHOM_GetHgUpPortLinkState( (UI32_T*)&up_phy_status))
    {
        printf("%s %d:STKTPLG_SHOM_GetHgUpPortLinkState fail(UP) device_id=%d\r\n", __FUNCTION__,__LINE__, ctrl_info_p->stacking_dev_id);
    }
    if(FALSE==STKTPLG_SHOM_GetHgDownPortLinkState( (UI32_T*)&down_phy_status))
    {
        printf("%s :STKTPLG_SHOM_GetHgDownPortLinkState fail(DOWN) device_id=%d\r\n", __FUNCTION__, ctrl_info_p->stacking_dev_id);
    }



    if (TRUE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_UP, &delta))
    {
        /* Hello Type 0 UP Timer has been expired, UP stacking port is OPEN
        */
        if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HELLO_0_UP))
        {
            *logic_link_status_p &= ~LAN_TYPE_TX_UP_LINK;
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);
        }

    }
    else
    {
        /* Hello Type 0 Up Timer is stopped due to the arrival of Hello Type 0 PDU
         * UP stacking port is CONNECTED.
         */
        if (TRUE == up_phy_status)
        {
            *logic_link_status_p |= LAN_TYPE_TX_UP_LINK;
        }
    }

    if (TRUE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_DOWN, &delta))
    {
        /* Hello Type 0 DOWN timer has been expired, DOWN stacking port is OPEN
         */
        if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HELLO_0_DOWN))
        {
            *logic_link_status_p &= ~LAN_TYPE_TX_DOWN_LINK;
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);
        }
    }
    else
    {
        /* Hello Type 0 Down Timer is stopped due to the arrival of Hello Type 0 PDU
         * DOWN stacking port is CONNECTED
         */
        if (TRUE == down_phy_status)
        {
            *logic_link_status_p |= LAN_TYPE_TX_DOWN_LINK;
        }
    }

    if (FALSE == up_phy_status)
        *logic_link_status_p &= ~LAN_TYPE_TX_UP_LINK;

    if (FALSE == down_phy_status)
        *logic_link_status_p &= ~LAN_TYPE_TX_DOWN_LINK;

    return;
}

/*
 * Check if Hello Type 1 Timer expired in order to determine expansion module is available
 */
static void STKTPLG_ENGINE_CheckHelloTimer1(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p)
{
    UI32_T delta;
#if 0 /* uncomment while the snippet below is ready */
    UI8_T  expansion_module_status;
#endif

    if (TRUE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_1, &delta))
    {
        /* Hello Type 1 Timer has been expired, Expansion Module is NOT THERE
         */
        if (TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_HELLO_1))
        {
            xgs_stacking_debug(";;;;; expansion module timeout\nr\r");
        }
    }

    {
        /* Hello Type 1 Timer is stopped due to the arrival of Hello Type 1,
         * Expansion Module is THERE.
         */
#if 0 /* need to use PHYADDRESS_ACCESS on linux platform, but it seems to be effectless code. */
        expansion_module_status = *((UI8_T *)SYS_HWCFG_MODULE_DETECT_ADDR);
#endif


        /*
         * Make sure hardware status match with software detected result
         */
    }

    return;
}

/*
 * Send Hello Type 0 PDU to both stacking ports if available
 */
static void STKTPLG_ENGINE_SendHelloPDU0(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p,UI32_T up_phy_status,UI32_T down_phy_status)
{
    UI32_T delta;

    if (up_phy_status == TRUE)
    {

       // if(STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_SEND_HELLO_UP))
       // {
         STKTPLG_TX_SendHello(NULL, STKTPLG_HELLO_TYPE_0, LAN_TYPE_TX_UP_LINK);
        // STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_SEND_HELLO_UP, STKTPLG_TIMER_SEND_HELLO_TIMEOUT);
        //}

    }

    if (FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_UP, &delta))
    {
        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HELLO_0_UP, STKTPLG_TIMER_HELLO_TIMEOUT);
    }

    if (down_phy_status == TRUE)
    {
        //if(STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_SEND_HELLO_DOWN))
        //{
        STKTPLG_TX_SendHello(NULL, STKTPLG_HELLO_TYPE_0, LAN_TYPE_TX_DOWN_LINK);
        // STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_SEND_HELLO_DOWN, STKTPLG_TIMER_SEND_HELLO_TIMEOUT);
        //}

    }

    if (FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_HELLO_0_DOWN, &delta))
    {
        STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HELLO_0_DOWN, STKTPLG_TIMER_HELLO_TIMEOUT);
    }

    return;
}


static BOOL_T STKTPLG_ENGINE_TopologyChangedByButton(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p)
{
#if (SYS_CPNT_MASTER_BUTTON == SYS_CPNT_MASTER_BUTTON_HARDWARE)
    if ( (TRUE == STKTPLG_ENGINE_ButtonStateChanged()) && (FALSE == ctrl_info_p->preempted_master) )
    {
        return (TRUE);
    }
    else
#endif
        return (FALSE);
}


#if 0 /* JinhuaWei, 03 August, 2008 10:38:56 */
static void STKTPLG_ENGINE_AssignExpModuleID(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port)
{

    UI8_T module_id;
    int unit,uid;


    if (TRUE == ctrl_info_p->is_ring)
    {
        for (uid=0;uid<ctrl_info_p->total_units  ;uid++)
        {
            for(unit = 0; unit < ctrl_info_p->total_units_down; unit++)
            {
                if (ctrl_info_p->stable_hbt_down.payload[unit].unit_id == hbt_ptr->payload[uid].unit_id)
                {
                    if(FALSE ==hbt_ptr->payload[uid].expansion_module_exist)
                    {
                        if(ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id!=UNKNOWN_MODULE_ID)
                        {
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)
                            //printf("1:take back the moudleid=%d\r\n",ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id);
                            STKTPLG_ENGINE_ExpModuleRemove(hbt_ptr->payload[uid].unit_id,ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id);
                            /* #else block of SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                             * on EPR ES4649-32-01434
                             */
#else
    printf("\r\nThe optional module has been removed.\r\nPlease reboot your device. Otherwise it will lead to unexpected behavior.\r\n");
#endif

                            ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id    =UNKNOWN_MODULE_ID;
                            /*add on 1209*/
                            ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_id    =UNKNOWN_MODULE_ID;
                            ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=UNKNOWN_MODULE_ID;
                            /*add on 1209*/
                            ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist=FALSE;
                            ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_exist=FALSE;
                            /*add on 1209 */
                            if (unit == 0)
                            {
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist=FALSE;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_exist=FALSE;
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id=UNKNOWN_MODULE_ID;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id=UNKNOWN_MODULE_ID;
                            }/* if (unit == 0) */
                        }/*take back the moudleid*/
                    }/*if not exist */
                    else if(UNKNOWN_MODULE_ID==ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]&&(TRUE==hbt_ptr->payload[uid].expansion_module_exist))
                    {

                        if ( (ctrl_info_p->last_module_id >= MAX_MODULE_ID) &&
                             (0 == recycle_module_id_idx                  ) )
                        {
                            module_id=UNKNOWN_MODULE_ID;
                            ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = TRUE;
                            /*add on 1209*/
                            ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_exist = TRUE;
                            ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=UNKNOWN_MODULE_ID;
                            if (unit == 0)
                            {
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist=TRUE;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_exist=TRUE;
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id=module_id;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id=module_id;
                            }

                        }
                        else
                        {
                            if (0 != recycle_module_id_idx)
                            {

                                module_id = recycle_module_id[--recycle_module_id_idx];
                                //printf("ring  the recycle_module_id =%d\r\n",module_id);

                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id = module_id;
                                /*add on 1209*/
                                ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_id = module_id;
                                ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id;
                            }
                            else
                            {
                                module_id = ctrl_info_p->last_module_id;
                                //hbt_ptr->payload[uid].expansion_module_id = module_id;
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id = ctrl_info_p->last_module_id++;
                                /*add on 1209*/
                                ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_id =ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id ;
                                ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id;
                            }

                            ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_type  = hbt_ptr->payload[uid].expansion_module_type;;
                            /*add on 1209*/
                            ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_type  =ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_type  ;
                            ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = TRUE;
                            /*add on 1209*/
                            ctrl_info_p->stable_hbt_up.payload[unit?ctrl_info_p->total_units-unit:0].expansion_module_exist=ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist;
                            if (unit == 0)
                            {
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = TRUE;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_exist=TRUE;
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id=module_id;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id=module_id;
                            }
                        }
                    }/* else assign */

                }/* if unit_id equal */
            }/* for total_downs*/
        }/* for total units*/
    } /* end of if (TRUE == ctrl_info_p->is_ring) */
    else
    {
        if (rx_port == UP_PORT_RX)
        {
        for (uid=0;uid<ctrl_info_p->total_units  ;uid++)
        {
            for(unit = ctrl_info_p->total_units_up - 1 ; unit >0 ; unit--)
            {
                if (ctrl_info_p->stable_hbt_up.payload[unit].unit_id == hbt_ptr->payload[uid].unit_id)
                {
                    if (FALSE ==hbt_ptr->payload[uid].expansion_module_exist)
                        {
                            if(hbt_ptr->payload[uid].expansion_module_id!=UNKNOWN_MODULE_ID)
                            {
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)
                                //printf("2:take back unit=%d the moudleid=%d\r\n",hbt_ptr->payload[uid].unit_id,ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id);
                                STKTPLG_ENGINE_ExpModuleRemove(hbt_ptr->payload[uid].unit_id,ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id);
                               /* #else block of SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                                * on EPR ES4649-32-01434
                                */
#else
    printf("\r\nThe optional module has been removed.\r\nPlease reboot your device. Otherwise it will lead to unexpected behavior.\r\n");
#endif
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id    =UNKNOWN_MODULE_ID;
                                ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_exist =FALSE;
                                ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=UNKNOWN_MODULE_ID;
                            }
                        }
                        else if(UNKNOWN_MODULE_ID==ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]&&(TRUE==hbt_ptr->payload[uid].expansion_module_exist))
                        {
                            if ( (ctrl_info_p->last_module_id >= MAX_MODULE_ID) &&
                                 (0 == recycle_module_id_idx                  ) )
                            {
                                /*
                                 * Disable expansion module if there are more than 32 modules in stack
                                 */
                                module_id=UNKNOWN_MODULE_ID;
                            }
                            else
                            {
                                if (0 != recycle_module_id_idx)
                                {
                                    module_id = recycle_module_id[--recycle_module_id_idx];
                                }
                                else
                                {
                                    module_id = ctrl_info_p->last_module_id++;

                                }
                            }
                            ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_exist = TRUE;
                            ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id = module_id;
                            //printf("assign:unit=%d up expmodid=%d\r\n",unit,ctrl_info_p->stable_hbt_up.payload[unit].expansion_module_id);

                        }
                    } /*if*/
                }/*for up loop */
            } /*for all uid*/
        }/* up port */
        else
        {
            for (uid=0;uid<ctrl_info_p->total_units;uid++)
            {
                for(unit = 1; unit < ctrl_info_p->total_units_down ; unit++)
                {
                    if (ctrl_info_p->stable_hbt_down.payload[unit].unit_id == hbt_ptr->payload[uid].unit_id)
                    {
                        if(FALSE ==hbt_ptr->payload[uid].expansion_module_exist)
                        {
                            if(hbt_ptr->payload[uid].expansion_module_id!=UNKNOWN_MODULE_ID)
                            {
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)
                                //printf("3:take back the moudleid=%d\r\n",ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id);
                                STKTPLG_ENGINE_ExpModuleRemove(hbt_ptr->payload[uid].unit_id,ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id);
                                /* #else block of SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                                 * on EPR ES4649-32-01434
                                 */
#else
                                printf("\r\nThe optional module has been removed.\r\nPlease reboot your device. Otherwise it will lead to unexpected behavior.\r\n");
#endif
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id    =UNKNOWN_MODULE_ID;
                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist =FALSE;
                                ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=UNKNOWN_MODULE_ID;
                            }
                        }
                        else if(UNKNOWN_MODULE_ID==ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]&&(TRUE==hbt_ptr->payload[uid].expansion_module_exist))
                        {
                            if ( (ctrl_info_p->last_module_id >= MAX_MODULE_ID) &&
                                 (0 == recycle_module_id_idx                  ) )
                            {

                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = TRUE;
                                ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=UNKNOWN_MODULE_ID;

                            }
                            else
                            {
                                if (0 != recycle_module_id_idx)
                                {
                                    module_id = recycle_module_id[--recycle_module_id_idx];
                                    ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id = module_id;
                                    ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id;

                                }
                                else
                                {
                                    module_id = ctrl_info_p->last_module_id;
                                    ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id = ctrl_info_p->last_module_id++;
                                    ctrl_info_p->exp_module_id[hbt_ptr->payload[uid].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_id;

                                }

                                ctrl_info_p->stable_hbt_down.payload[unit].expansion_module_exist = TRUE;

                            }
                        }
                    }/*if*/
                } /*for down*/
            }  /*for uid*/
        }  /* else down port */

        if(FALSE ==hbt_ptr->payload[0].expansion_module_exist)
        {
            if(ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id!=UNKNOWN_MODULE_ID)
            {
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)
                STKTPLG_ENGINE_ExpModuleRemove(hbt_ptr->payload[0].unit_id,ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id);
                /* #endif of SYS_CPNT_EXPANSION_MODULE_HOT_SWAP is appended
                 * on EPR ES4649-32-01434
                 */

#endif
                ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id    =UNKNOWN_MODULE_ID;
                ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist =FALSE;
                ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id    =UNKNOWN_MODULE_ID;
                ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist =FALSE;
                ctrl_info_p->exp_module_id[hbt_ptr->payload[0].unit_id-1]=UNKNOWN_MODULE_ID;
            }
        }
        else if(UNKNOWN_MODULE_ID==ctrl_info_p->exp_module_id[hbt_ptr->payload[0].unit_id-1]&&(TRUE==hbt_ptr->payload[0].expansion_module_exist))
        {
            if ( (ctrl_info_p->last_module_id >= MAX_MODULE_ID) &&
                (0 == recycle_module_id_idx                  ) )
            {
                ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = TRUE;
                ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = TRUE;
                ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
                ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
                ctrl_info_p->exp_module_id[hbt_ptr->payload[0].unit_id-1]=UNKNOWN_MODULE_ID;

            }
            else
            {
                if (0 != recycle_module_id_idx)
                {
                    module_id = recycle_module_id[--recycle_module_id_idx];
                    ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = module_id;
                    ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id = module_id;
                    ctrl_info_p->exp_module_id[hbt_ptr->payload[0].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id;

                 }
                 else
                 {
                    module_id = ctrl_info_p->last_module_id;

                    ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = ctrl_info_p->last_module_id++;
                    ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id = ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id ;
                    ctrl_info_p->exp_module_id[hbt_ptr->payload[0].unit_id-1]=ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id;

                 }

                 ctrl_info_p->stable_hbt_down.payload[0].expansion_module_exist = TRUE;
                 ctrl_info_p->stable_hbt_up.payload[0].expansion_module_exist = TRUE;

             }
             ctrl_info_p->stable_hbt_down.payload[0].expansion_module_type = ctrl_info_p->stable_hbt_up.payload[0].expansion_module_type = ctrl_info_p->expansion_module_type;
        } /*end else if master exist module*/
    }/*else*/

    return ;
}
#endif /* #if 0 */

static void STKTPLG_ENGINE_ClearCtrlInfo(void)
{
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    UI32_T                 i;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if (!STKTPLG_BOARD_GetBoardInformation(ctrl_info_p->board_id, board_info_p))
        {
        perror("\r\nCan not get related board information.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    /* Restore Port Mapping and Unit Config
     */
    STKTPLG_ENGINE_ResetUnitCfg(ctrl_info_p);

    ctrl_info_p->button_pressed_arbitration = FALSE;
#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:13:10 */
    ctrl_info_p->my_unit_id          = 1;
#else /* SYS_CPNT_STKTPLG_SHMEM */
    STKTPLG_OM_SetMyUnitID(1);
#endif
    ctrl_info_p->master_unit_id      = 1;
    ctrl_info_p->my_logical_unit_id  = 1;
    ctrl_info_p->my_phy_unit_id_up   = 1;
    ctrl_info_p->my_phy_unit_id_down = 1;
    ctrl_info_p->total_units         = 1;

    /* michael - assuming both UP and DOWN stacking ports are connected */
    ctrl_info_p->stacking_ports_logical_link_status = LAN_TYPE_TX_UP_DOWN_LINKS;
    ctrl_info_p->bounce_msg                 = 0;
    ctrl_info_p->total_units_up             = 1;
    ctrl_info_p->total_units_down           = 1;
    //ctrl_info_p->preempted_master           = FALSE;
    //ctrl_info_p->preempted                  = FALSE;
    //ctrl_info_p->last_module_id             = 1;
    ctrl_info_p->next_stacking_unit         = 0;
    ctrl_info_p->expansion_module_type      = 0xff;
    ctrl_info_p->expansion_module_id        = UNKNOWN_MODULE_ID;
    ctrl_info_p->expansion_module_exist     = FALSE;
    ctrl_info_p->stack_maintenance          = FALSE;
    ctrl_info_p->standalone_hello           = 0;
    ctrl_info_p->is_ring                    = 0;
    /*1223*/
    ctrl_info_p->provision_completed_state  = FALSE;
    ctrl_info_p->expansion_module_ready     = FALSE;

    memset(&ctrl_info_p->stable_hbt_up, 0, sizeof(STKTPLG_OM_HBT_0_1_T));
    memset(&ctrl_info_p->stable_hbt_down, 0, sizeof(STKTPLG_OM_HBT_0_1_T));

    ctrl_info_p->stable_hbt_up.payload[0].unit_id = 1;
    ctrl_info_p->stable_hbt_up.payload[0].start_module_id = 0;
    ctrl_info_p->stable_hbt_up.payload[0].chip_nums = board_info_p->chip_number_of_this_unit;
    ctrl_info_p->stable_hbt_up.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
    ctrl_info_p->stable_hbt_up.payload[0].board_id = ctrl_info_p->board_id;
     /*1223*/
    ctrl_info_p->stable_hbt_up.payload[0].provision_completed_state= FALSE;

    ctrl_info_p->stable_hbt_down.payload[0].unit_id = 1;
    ctrl_info_p->stable_hbt_down.payload[0].start_module_id = 0;
    ctrl_info_p->stable_hbt_down.payload[0].chip_nums = ctrl_info_p->stable_hbt_up.payload[0].chip_nums;
    ctrl_info_p->stable_hbt_down.payload[0].expansion_module_id = UNKNOWN_MODULE_ID;
    ctrl_info_p->stable_hbt_down.payload[0].board_id = ctrl_info_p->board_id;
    memcpy(ctrl_info_p->stable_hbt_up.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
    memcpy(ctrl_info_p->stable_hbt_down.payload[0].mac_addr ,ctrl_info_p->my_mac,STKTPLG_MAC_ADDR_LEN);
     /*1223*/
    ctrl_info_p->stable_hbt_down.payload[0].provision_completed_state= FALSE;

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        ctrl_info_p->exp_module_id[i]=UNKNOWN_MODULE_ID;
        /*add on 1220*/
        //printf("learCtrlInfo  RemoveExpModule\r\n");
        if(TRUE==STKTPLG_OM_ExpModuleIsInsert(i+1))
            STKTPLG_OM_RemoveExpModule(i+1);

        ctrl_info_p->synced_module_types[i] = 0; /*re-init as not present*/
    }

    ctrl_info_p->stk_unit_cfg_dirty_sync_bmp = 0;

    recycle_module_id_idx=0;
    STKTPLG_ENGINE_EventHandler(DEAD_EV);
    not_handled = FALSE;

    for (i=0; i<STKTPLG_HBT_TYPE_MAX; i++)
        ctrl_info_p->seq_no[i] = 0;

    /* Stop all timers
     */
    for (i = STKTPLG_TIMER_HBT0_UP; i < STKTPLG_TIMER_MAX; i++)
        STKTPLG_TIMER_StopTimer(i);

    //STKTPLG_MGR_ResetDeviceInfo();
}

static BOOL_T STKTPLG_ENGINE_NextUpFromMaster(void)
{
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    BOOL_T                 retval;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if (memcmp(ctrl_info_p->my_mac, ctrl_info_p->stable_hbt_up.payload[1].mac_addr, 6))
        retval = FALSE;
    else
        retval = TRUE;

    return (retval);
}

static BOOL_T STKTPLG_ENGINE_NextDownFromMaster(void)
{
    STKTPLG_OM_Ctrl_Info_T *ctrl_info_p;
    BOOL_T                 retval;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if (memcmp(ctrl_info_p->my_mac, ctrl_info_p->stable_hbt_down.payload[1].mac_addr, 6))
        retval = FALSE;
    else
        retval = TRUE;

    return (retval);
}



static BOOL_T STKTPLG_ENGINE_IsAbleToBeNextMaster(void)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    UI32_T                  unit, done;

    done = 0;

    if (TRUE == ctrl_info_p->is_ring)
    {
        for(unit = 0; unit < ctrl_info_p->total_units_down && done==0; unit++)
        {
            if (memcmp(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, ctrl_info_p->my_mac,
                STKTPLG_MAC_ADDR_LEN) < 0)
            {
                done++;
            }
        }
    }
    else
    {
        if (0 != ctrl_info_p->total_units_up)
        {
            for(unit = ctrl_info_p->total_units_up - 1 ; unit > 0 && done==0; unit--)
            {
                if (memcmp(ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, ctrl_info_p->my_mac,
                    STKTPLG_MAC_ADDR_LEN) < 0)
                {
                    done++;
                }
            }
        }

        for(unit = 0; unit < ctrl_info_p->total_units_down && done==0; unit++)
        {
            if (memcmp(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, ctrl_info_p->my_mac,
                STKTPLG_MAC_ADDR_LEN) < 0)
            {
                done++;
            }
        }
    }

    if (done == 0)
        return TRUE;
    else
        return FALSE;
}

static BOOL_T STKTPLG_ENGINE_CheckAllUnitsButtonStatus(void)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    UI32_T                  unit, done;

    done = 0;

    if (TRUE == ctrl_info_p->is_ring)
    {
        for(unit = 1; unit < ctrl_info_p->total_units_down; unit++)
        {
            if (TRUE == ctrl_info_p->stable_hbt_down.payload[unit].button_pressed)
                done++;
        }
    }
    else
    {
        if (ctrl_info_p->total_units_up > 1)
        {
            for(unit = ctrl_info_p->total_units_up - 1; unit > 0; unit--)
            {
                if (TRUE == ctrl_info_p->stable_hbt_up.payload[unit].button_pressed)
                    done++;
            }
        }

        for(unit = 1; unit < ctrl_info_p->total_units_down; unit++)
        {
            if (TRUE == ctrl_info_p->stable_hbt_down.payload[unit].button_pressed)
                done++;
        }
    }

    if (0 == done)
        return FALSE;
    else
        return TRUE;
}

#if 0 /* JinhuaWei, 03 August, 2008 10:45:30 */
static void STKTPLG_ENGINE_ExpModuleRemove(UI8_T unit_id,UI8_T module_id)
{
    //STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
    if(recycle_module_id_idx>=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        perror("recycle_module_id_idx over\r\n");
        return;
    }
    //printf("ExpModuleRemove recycle_module_id =%d\r\n",module_id);
    recycle_module_id[recycle_module_id_idx++] = module_id ; //ctrl_info_p->expansion_module_id;

    /*add on 1219 PM6:00*/
    if(TRUE==STKTPLG_OM_ExpModuleIsInsert(unit_id))
        STKTPLG_OM_RemoveExpModule(unit_id);
    //STKTPLG_ENGINE_ConfigStackInfoToIUC();
    return;
}
#endif /* #if 0 */

static void STKTPLG_ENGINE_ResetUnitCfg(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p)
{
    UI32_T         i,j,max_port_number,max_option_port_number;
    STK_UNIT_CFG_T unit_cfg;

        master_state_counter=0;
        for (i=1; i<= SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
            if(!STKTPLG_OM_GetPortMapping(port_mapping, i))
              continue;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(i, &max_option_port_number);
            STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(ctrl_info_p->my_unit_id, &max_port_number);
#else
            STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(i, &max_option_port_number);
            STKTPLG_OM_GetMaxPortNumberOnBoard(i, &max_port_number);
#endif

        for (j = 0; j < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; j++)
        {
            if (i == ctrl_info_p->my_unit_id)
            {
                /* Jeff mask ; local use default board info
                 */
                if(j < max_port_number)
                {
                    port_mapping[j].module_id = port_mapping[j].device_id;
                }
                else
                {
                   port_mapping[j].module_id = UNKNOWN_MODULE_ID;
                   port_mapping[j].device_id      = UNKNOWN_DEVICE_ID;
                   port_mapping[j].device_port_id = UNKNOWN_DEVICE_PORT_ID;
                   port_mapping[j].phy_id         = UNKNOWN_PHY_ID;
                   port_mapping[j].port_type      = VAL_portType_other;

                }
            }
            else
            {
                port_mapping[j].module_id = UNKNOWN_MODULE_ID;
                port_mapping[j].device_id      = UNKNOWN_DEVICE_ID;
                port_mapping[j].device_port_id = UNKNOWN_DEVICE_PORT_ID;
                port_mapping[j].phy_id         = UNKNOWN_PHY_ID;
                port_mapping[j].port_type      = VAL_portType_other;
            }
        }

        /* EPR:ACP_V3_Enhancement-00043
         * It is necessary to invalidate all unused entry in port_mapping array.
         * If not, DEV_SWDRV_MaintainPhysical2LogicPortMappingTable() will convert
         * incorrect table, and DEV_SWDRV_Logical2PhyDeviceID() might get wrong
         * result.
         * 2006/1/5  Charlie Chen
         */
        for(j= max_port_number;j<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT;j++)
        {
            port_mapping[j].module_id = UNKNOWN_MODULE_ID;
        }

        STKTPLG_MGR_SetPortMapping(port_mapping, i);
    }

    if (ctrl_info_p->my_unit_id > 1)
    {
        /* Restore unit config to location 0
         */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetDeviceInfo(ctrl_info_p->my_unit_id, &unit_cfg);
        STKTPLG_OM_ENG_SetDeviceInfo(1, &unit_cfg);
        STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
        DBG("Call STKTPLG_OM_SetPortMapping() from %s @ line %d\n", __FUNCTION__, __LINE__);
        STKTPLG_OM_SetPortMapping(port_mapping, 1);
#else
        STKTPLG_OM_GetDeviceInfo(ctrl_info_p->my_unit_id, &unit_cfg);
        STKTPLG_OM_SetDeviceInfo(1, &unit_cfg);
        STKTPLG_OM_GetPortMapping(port_mapping, ctrl_info_p->my_unit_id);
        DBG("Call STKTPLG_OM_SetPortMapping() from %s @ line %d\n", __FUNCTION__, __LINE__);
        STKTPLG_OM_SetPortMapping(port_mapping, 1);
#endif
#if (SYS_CPNT_STKTPLG_SHMEM == FALSE) /* FenWang, Monday, August 11, 2008 11:14:11 */
        ctrl_info_p->my_unit_id = 1 ;
#else /* SYS_CPNT_STKTPLG_SHMEM */
        STKTPLG_OM_SetMyUnitID(1);
#endif
    }

#if (SYS_CPNT_HW_PROFILE_PORT_MODE == TRUE)
    /* find max port number.
     */
    for (j = SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; j > 0; j--)
    {
        if (port_mapping[j-1].device_id != 0xff &&
            port_mapping[j-1].device_port_id != 0xff)
        {
            break;
        }
    }

    /* update max port number
     */
    STKTPLG_OM_ENG_SetMaxPortCapability(ctrl_info_p->my_unit_id, j);

    /* update hw port mode database
     */
    STKTPLG_MGR_InitHwPortModeDb();
#endif

    return;
}
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

/* FUNCTION NAME: STKTPLG_ENGINE_GetDebugMode
 * PURPOSE: This function can get the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_GetDebugMode(BOOL_T *debug_mode)
{
    *debug_mode = stktplg_engine_debug_mode;
}

/* FUNCTION NAME: STKTPLG_ENGINE_ToggleDebugMode
 * PURPOSE: This function can toggle the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_ToggleDebugMode(void)
{
    stktplg_engine_debug_mode = !stktplg_engine_debug_mode;
}

/* FUNCTION NAME: STKTPLG_ENGINE_GetTCNMode
 * PURPOSE: This function can get the debug mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_GetTCNMode(BOOL_T *TCN_mode)
{
    *TCN_mode = stktplg_engine_TCN_mode;
}

/* FUNCTION NAME: STKTPLG_ENGINE_ToggleTCNMode
 * PURPOSE: This function can force the stack not to topology change.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 */
void STKTPLG_ENGINE_ToggleTCNMode(void)
{
    stktplg_engine_TCN_mode = !stktplg_engine_TCN_mode;
}

#if (SYS_CPNT_STACKING == TRUE)

static void STKTPLG_ENGINE_AssignUnitID(void)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    int unit,xxx;
    STKTPLG_DataBase_Info_T db;
    UI8_T  unit_id;
    
    STKTPLG_OM_ResetStackingDB();
    if (stktplg_stackingdb_updated == TRUE)
    {
        stktplg_engine_id_table_exist = STKTPLG_OM_GetStackingDB();
    }
    else
    {
        stktplg_engine_id_table_exist = FALSE;
    }
    if (stktplg_engine_id_table_exist)
    {
    /*  for (i=0; i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
        {
            STKTPLG_OM_GetStackingDBEntry(&db,i);
        } */
        if (ctrl_info_p->is_ring)
        {
            for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
            {
                if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                    = STKTPLG_OM_GetStackingDBEntryByMac(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,ctrl_info_p->stable_hbt_down.payload[unit].board_id)) >0)
                {
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                }
            }
            for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
            {
                if(ctrl_info_p->stable_hbt_down.payload[unit].unit_id ==0)
                {
                    if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                        = STKTPLG_OM_GetStackingDBFreeEntry(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                    {
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                    }
                }
            }
        }
        else
        {
            for (unit = ctrl_info_p->total_units_up - 1;  unit >= 0; unit--)
            {
                if ((ctrl_info_p->stable_hbt_up.payload[unit].unit_id
                    = STKTPLG_OM_GetStackingDBEntryByMac(ctrl_info_p->stable_hbt_up.payload[unit].mac_addr,ctrl_info_p->stable_hbt_up.payload[unit].board_id))>0)
                {
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_up.payload[unit].unit_id, ctrl_info_p->stable_hbt_up.payload[unit].board_id);
                }
            }
            for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
            {
                if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                    = STKTPLG_OM_GetStackingDBEntryByMac(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                {
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                }
            }
            for (unit = ctrl_info_p->total_units_up - 1 ; unit >= 0; unit--)
            {
                if(ctrl_info_p->stable_hbt_up.payload[unit].unit_id ==0)
                {
                    if ((ctrl_info_p->stable_hbt_up.payload[unit].unit_id
                        = STKTPLG_OM_GetStackingDBFreeEntry(ctrl_info_p->stable_hbt_up.payload[unit].mac_addr,ctrl_info_p->stable_hbt_up.payload[unit].board_id))>0)
                    {
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_up.payload[unit].unit_id, ctrl_info_p->stable_hbt_up.payload[unit].board_id);
                    }
                }
            }
            for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
            {
                if(ctrl_info_p->stable_hbt_down.payload[unit].unit_id ==0)
                {
                    if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                        = STKTPLG_OM_GetStackingDBFreeEntry(ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                    {
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                    }
                }
            }
            ctrl_info_p->stable_hbt_down.payload[0].unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;
        }
    }
    else
    {
        STKTPLG_OM_DumpStackingDB();

        /*For the case of id_table not exist but temp stacking exists, 
         *  follwoing code searches dispatched id for the same unit.
         *Do this to prevent unit id changed but lower layer is not awared.
         *
         *Example: For master with slave unit on downlink, boot up with factory-default-config.
         *         When just entered master-mode, unplug downlink and plug uplink to another slave. 
         *         Because ID_table is still not ready before provision-completed, 
         *         master will change its ID 1->2 for a slave is at uplink.
         *         But this change is not notified to stkctrl, because it's a hot removal/insterion.
         *         This leads stktplg and port mapping runs as unit 2, 
         *         but other CSC remains unit 1 on their own OM.         
         */
        if(!STKTPLG_OM_IsTempStackingDBEmpty())
        {
            if (ctrl_info_p->is_ring)
            {
                for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                {        
                    /*Search tmpstackingDB for the unit record*/
                    if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                        = STKTPLG_OM_GetUnitIdFromTempStackingDB(
                            ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,
                            ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                    {
                        memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                        db.id_flag = 3 ;
                        db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
                        STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                    }
                }
            
                for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                {
                    /*If not, get a new free ID*/ 
                    if(ctrl_info_p->stable_hbt_down.payload[unit].unit_id ==0)
                    {
                        
                        if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                            = STKTPLG_OM_GetStackingDBFreeEntry(
                                ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,
                                ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                        {                      
                            memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                            db.id_flag = 3 ;
                            db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
                            STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                            STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                        }
                    }
                }
                
            }
            else
            {
                for (unit = ctrl_info_p->total_units_up - 1;  unit >= 0; unit--)
                {
                    /*Search tmpstackingDB for the unit record*/
                    if ((ctrl_info_p->stable_hbt_up.payload[unit].unit_id
                        = STKTPLG_OM_GetUnitIdFromTempStackingDB(
                            ctrl_info_p->stable_hbt_up.payload[unit].mac_addr,
                            ctrl_info_p->stable_hbt_up.payload[unit].board_id))>0)
                    {
                        memcpy(db.mac_addr, ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                        db.id_flag = 3 ;
                        db.device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id;
                        STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_up.payload[unit].unit_id);
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_up.payload[unit].unit_id, ctrl_info_p->stable_hbt_up.payload[unit].board_id);
                    }                 
                }

                for (unit = ctrl_info_p->total_units_up - 1 ; unit >= 0; unit--)
                {
                    /*If not, get a new free ID*/   
                    if(ctrl_info_p->stable_hbt_up.payload[unit].unit_id ==0)
                    {
                        if ((ctrl_info_p->stable_hbt_up.payload[unit].unit_id
                            = STKTPLG_OM_GetStackingDBFreeEntry(
                                ctrl_info_p->stable_hbt_up.payload[unit].mac_addr,
                                ctrl_info_p->stable_hbt_up.payload[unit].board_id))>0)
                        {   
                            memcpy(db.mac_addr, ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                            db.id_flag = 3 ;
                            db.device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id;
                            STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_up.payload[unit].unit_id);
                            STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_up.payload[unit].unit_id, ctrl_info_p->stable_hbt_up.payload[unit].board_id);
                        }
                    }
                }

                for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
                {        
                    /*Search tmpstackingDB for the unit record*/
                    if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                        = STKTPLG_OM_GetUnitIdFromTempStackingDB(
                            ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,
                            ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                    {
                        memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                        db.id_flag = 3 ;
                        db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
                        STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                        STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                    }
                }
            
                for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
                {
                    /*If not, get a new free ID*/ 
                    if(ctrl_info_p->stable_hbt_down.payload[unit].unit_id ==0)
                    {
                        
                        if ((ctrl_info_p->stable_hbt_down.payload[unit].unit_id
                            = STKTPLG_OM_GetStackingDBFreeEntry(
                                ctrl_info_p->stable_hbt_down.payload[unit].mac_addr,
                                ctrl_info_p->stable_hbt_down.payload[unit].board_id))>0)
                        {                      
                            memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                            db.id_flag = 3 ;
                            db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
                            STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                            STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                        }
                    }
                }
                ctrl_info_p->stable_hbt_down.payload[0].unit_id = ctrl_info_p->stable_hbt_up.payload[0].unit_id;                
            }                
        }
        else
        {
           // printf("***** Unit ID Table Not Found ****");
            if (ctrl_info_p->is_ring)
            {
                for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                {
                    ctrl_info_p->stable_hbt_down.payload[unit].unit_id          = pre_unit_id[unit];
                    memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                    db.id_flag = 3 ;
                    db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
                    STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                }

            }
            else
            {
                for (unit = ctrl_info_p->total_units_up - 1, xxx=0 ; unit >= 0; unit--)
                {
                    ctrl_info_p->stable_hbt_up.payload[unit].unit_id          = pre_unit_id[xxx++];
                    memcpy(db.mac_addr, ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                    db.id_flag = 3 ;
                    db.device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id ;
                    STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_up.payload[unit].unit_id);
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_up.payload[unit].unit_id, ctrl_info_p->stable_hbt_up.payload[unit].board_id);
                }
                for (unit = 0, xxx--; unit < ctrl_info_p->total_units_down; unit++)
                {
                    ctrl_info_p->stable_hbt_down.payload[unit].unit_id          = pre_unit_id[xxx++];
                    memcpy(db.mac_addr, ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
                    db.id_flag = 3 ;
                    db.device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id ;
                    STKTPLG_OM_SetStackingDBEntry(&db,ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
                    STKTPLG_OM_InsertBoradID(ctrl_info_p->stable_hbt_down.payload[unit].unit_id, ctrl_info_p->stable_hbt_down.payload[unit].board_id);
                }
            }
        }
    }

/* EPR: ES3628BT-FLF-ZZ-00546
     Problem: 1,when 3 dut stacking together,after system bring up ,it will produce a config file "startup" to store the stacking topo info
                      2, remove a slave A which unit ID is 3, then add a new dut.The new DUT will use unit ID 3
                      3,add A which removed before  the new dut added completed,sometimes the A will assign unit ID 3,and the new will be
                         4,but the led of the new dut still 3.Because cli has not save the topo of the new dut before the dut A added
                      4  the new dut detect the same master,and will do nothing
      Solution: check if the unit Id is used by others before assign unit id
*/
    STKTPLG_OM_SetCurrentTempStackingDB(); /*save the current topo info*/
#if (SYS_CPNT_STACKING_BUTTON == TRUE) /* by Eric for ES4626F_FLF_38 */
    if(ctrl_info_p->stacking_button_state)
#endif
    {
        printf("Assigned Unit ID:");
        if (ctrl_info_p->is_ring)
        {
            for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
                printf("[%d]",ctrl_info_p->stable_hbt_down.payload[unit].unit_id );
        }
        else
        {
            for (unit = ctrl_info_p->total_units_up - 1; unit >= 0; unit--)
                printf("[%d]",ctrl_info_p->stable_hbt_up.payload[unit].unit_id );
            for (unit = 1; unit < ctrl_info_p->total_units_down; unit++)
                printf("[%d]",ctrl_info_p->stable_hbt_down.payload[unit].unit_id);
        }
        printf("\n");
    }
}

/* FUNCTION NAME: STKTPLG_ENGINE_Check_Stacking_Link_Down
 * PURPOSE: Examine logic link status to take actions and output message.
 * INPUT:   logic_link_status       -current logic link status
 *          reset_flag              -notify_msg will be updated if required when reset_flag is TRUE
 * OUTPUT:  notify_msg    -- The message to notify STKCTRL. Just like the 
 *                            notify_msg argument of the function STKTPLG_ENGINE_StackManagement.
 * RETUEN:  None
 * NOTES:   This function is designed to check the case of either link went down.
 *          I.E. the case of both link went down is not handled here.
 *          OM (ctrl_info) will be updated.          
 */
static void STKTPLG_ENGINE_Check_Stacking_Link_Down(UI8_T logic_link_status,BOOL_T reset_flag, UI32_T *notify_msg)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    UI8_T new_logic_link_status = logic_link_status;
    UI8_T pre_logic_link_status = ctrl_info_p->stacking_ports_logical_link_status;

    if ( ((pre_logic_link_status & LAN_TYPE_TX_UP_LINK) && 
         (!(new_logic_link_status & LAN_TYPE_TX_UP_LINK))) ||
         ((pre_logic_link_status & LAN_TYPE_TX_DOWN_LINK) && 
         (!(new_logic_link_status & LAN_TYPE_TX_DOWN_LINK))) )        
    {
        //printf("one of the ports is down\r\n");
        if (TRUE == ctrl_info_p->is_ring)
        {
            /*
             * Update topology to reflect open-closed condition
             */
            ctrl_info_p->is_ring         = FALSE;

            if (!(new_logic_link_status & LAN_TYPE_TX_UP_LINK) && (new_logic_link_status & LAN_TYPE_TX_DOWN_LINK))
            {
                xgs_stacking_debug("Up stacking port is opened in closed loop Send CloseLoopTCN\r\n");

                ctrl_info_p->total_units_up = 1;
                ctrl_info_p->stable_hbt_up.header.next_unit = ctrl_info_p->total_units_up + 1;

                /* Notify neighbors for closed loop re-topology
                 */
                ctrl_info_p->is_ring = FALSE;

                STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);

                STKTPLG_ENGINE_ConfigStackInfoToIUC();


            }
            else if ((new_logic_link_status & LAN_TYPE_TX_UP_LINK) && !(new_logic_link_status & LAN_TYPE_TX_DOWN_LINK))
            {
                xgs_stacking_debug("Down stacking port is opened in closed loop Send CloseLoopTCN\r\n");

                ctrl_info_p->total_units_down = 1;
                ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;



                /* Notify neighbors for closed loop re-topology
                 */
                ctrl_info_p->is_ring = FALSE;

                STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);

                STKTPLG_ENGINE_ConfigStackInfoToIUC();

                /* Start maintain timer in UP port
                 */

                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_M_DOWN);
                STKTPLG_TX_SendHBTType1(FALSE,NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
                 STKTPLG_BACKDOOR_IncSendCounter(STKTPLG_HBT_TYPE_1,17);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_UP, STKTPLG_TIMER_HBT1_TIMEOUT);
                STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
            }
        }
        else
        {
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

            /* change state to arbitration
             */
            STKTPLG_ENGINE_LogMsg("[TCN] STKTPLG_ENGINE_Check_Stacking_Link_Down: Line\n");
            if (ctrl_info_p->state == STKTPLG_STATE_MASTER)
                tcnReason = 2;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            if ((reset_flag == TRUE)&&(ctrl_info_p->state != STKTPLG_STATE_MASTER)&&(ctrl_info_p->state != STKTPLG_STATE_SLAVE)&&(ctrl_info_p->state != STKTPLG_STATE_STANDALONE))
                {
                *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
                xgs_stacking_debug("Check_Stacking_Link_Down STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");
                }
            STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
            xgs_stacking_debug("[%s]%d check_stacking_link_down TOPO change\n",__FUNCTION__,__LINE__);
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
            if (reset_flag == TRUE)
                *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
        }
    }
    ctrl_info_p->stacking_ports_logical_link_status = logic_link_status;
}

/* FUNCTION NAME: STKTPLG_ENGINE_Check_Stacking_Link_Up
 * PURPOSE: Handle the reception of STKTPLG_HELLO_TYPE_0 type HBT packet and
 *          update ctrl_info_p->stacking_ports_logical_link_status if required.
 * INPUT:   ctrl_info_p   -  pointer to control info of STKTPLG_OM.
 *          rx_port       -  phy port id where the HBT packet is received.
 *          mref_handle_p -  mref handle of the recevied HBT packet.
 *          reset_flag    -  TRUE: modify notify_msg when TCN occurs.
 *                           FALSE: do not modify notify_msg when TCN occurs.
 * OUTPUT:  notify_msg    -  notification message for the upper functions of
 *                           STKTPLG_ENGINE.
 * RETUEN:  TRUE  - The HBT packet had been handled by this function and the
 *                  caller does not need to process it anymore.
 *          FALSE - The HBT packet is not handled by this function.
 * NOTES:
 *          The caller does not need to release mref_handle_p when the return
 *          value of this function is TRUE.
 */
static BOOL_T STKTPLG_ENGINE_Check_Stacking_Link_Up(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p,UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p, BOOL_T reset_flag,UI32_T *notify_msg)
{
    STKTPLG_OM_HELLO_0_T    *hello_ptr;
    UI32_T                  i, done;
    UI32_T                  uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        /* There is no way to handle the HBT packet correctly if the phy port id
         * of the stacking port cannot be got. So just return TRUE here to pretend
         * that the packet had been handled.
         */
        L_MM_Mref_Release(&mref_handle_p);
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        return TRUE;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        /* There is no way to handle the HBT packet correctly if the phy port id
         * of the stacking port cannot be got. So just return TRUE here to pretend
         * that the packet had been handled.
         */
        L_MM_Mref_Release(&mref_handle_p);
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        return TRUE;
    }

    hello_ptr = (STKTPLG_OM_HELLO_0_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy */
    if(hello_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if (hello_ptr->header.type == STKTPLG_HELLO_TYPE_0)
    {
        /* If both rx and tx port of HELLO are the same type of stack link(UP or DOWN),
         * it's an invalid link and set link_status of rx_port to be OFF.
         */      
        UI8_T rx_port_up_down = (rx_port == uplink_port)?LAN_TYPE_TX_UP_LINK: LAN_TYPE_TX_DOWN_LINK;
    
        if(rx_port_up_down == hello_ptr->tx_up_dw_port)
        {
            STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(rx_port_up_down);             
            ctrl_info_p->stacking_ports_logical_link_status &= ~rx_port_up_down;      
        }      
        else
        {
            if (rx_port == uplink_port)
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_UP);

                if (LAN_TYPE_TX_UP_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                {
                    /* Link UP -- Compare Mac to check if it is a new unit or
                     *            to result a ring (should send CLOSE_LOOP_TCN)
                     */

                    ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_UP_LINK;
                    ctrl_info_p->up_phy_status = TRUE;
                    up_new_link = FALSE;

                    for(done=0, i = 0; (i < ctrl_info_p->total_units_up) && (0 == done); i++)
                    {
                        if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_up.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                            done = 1;
                    }
                    for(i = 1; (i < ctrl_info_p->total_units_down) && (0 == done); i++)
                    {
                        if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_down.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                            done = 1;
                    }

                    if (1 == done && ctrl_info_p->state == STKTPLG_STATE_MASTER)
                    {
                        if (ctrl_info_p->is_ring ==FALSE)
                        {
                            /* Join To a Ring */
                            for (i = ctrl_info_p->total_units_down -1; i > 0; i--)
                            {
                                memcpy(&ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - i -1],
                                       &ctrl_info_p->stable_hbt_down.payload[i],
                                       sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                            }
                            ctrl_info_p->total_units_up =  ctrl_info_p->total_units_down ;

                            ctrl_info_p->is_ring = TRUE ;
                            /* Notify neighbors for closed loop re-topology
                             */
                            STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                            /*  If a link is connected to form a ring , no action */
                            /*  STKTPLG_ENGINE_ConfigStackInfoToIUC(); */
                            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);

                            ctrl_info_p->stable_hbt_up.header.next_unit = ctrl_info_p->total_units_up + 1;
                        }
                    }
                    else
                    {
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

                        /* change state to arbitration
                         */
                        STKTPLG_ENGINE_LogMsg("[TCN]: Check_Stacking_Link_Up\r\n");
                        if (ctrl_info_p->state == STKTPLG_STATE_MASTER)
                            tcnReason = 3;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                        if ((reset_flag == TRUE)&&(ctrl_info_p->state != STKTPLG_STATE_MASTER)&&(ctrl_info_p->state != STKTPLG_STATE_SLAVE)&&(ctrl_info_p->state != STKTPLG_STATE_STANDALONE))
                        {
                            xgs_stacking_debug("Check_Stacking_Link_Up_1 STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");
                            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
                        }
                        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                        xgs_stacking_debug("check_stacking_link_up TOPO change\n");
#else
                        STKTPLG_ENGINE_ReturnToArbitrationState(0);
                        if (reset_flag == TRUE)
                            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
                    }
                }

            }
            else /* if (rx_port == downlink_port) */
            {
                STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HELLO_0_DOWN);

                if (LAN_TYPE_TX_DOWN_LINK != (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                {
                    /* Link UP -- Compare Mac to check if it is a new unit or
                     *            to result a ring (should send CLOSE_LOOP_TCN)
                     */

                    ctrl_info_p->stacking_ports_logical_link_status |= LAN_TYPE_TX_DOWN_LINK;
                    ctrl_info_p->down_phy_status = TRUE;
                    down_new_link = FALSE;

                    for(done=0, i = 0; (i < ctrl_info_p->total_units_up) && (0 == done); i++)
                    {
                        //printf("up[%d].mac=%x %x %x %x %x %x\n",i,ctrl_info_p->stable_hbt_up.payload[i].mac_addr[0],ctrl_info_p->stable_hbt_up.payload[i].mac_addr[1],ctrl_info_p->stable_hbt_up.payload[i].mac_addr[2],ctrl_info_p->stable_hbt_up.payload[i].mac_addr[3],ctrl_info_p->stable_hbt_up.payload[i].mac_addr[4],ctrl_info_p->stable_hbt_up.payload[i].mac_addr[5]);
                        if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_up.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                            done = 1;
                    }
                    for(i = 1; (i < ctrl_info_p->total_units_down) && (0 == done); i++)
                    {
                        //printf("down[%d].mac=%x %x %x %x %x %x\n",i,ctrl_info_p->stable_hbt_down.payload[i].mac_addr[0],ctrl_info_p->stable_hbt_down.payload[i].mac_addr[1],ctrl_info_p->stable_hbt_down.payload[i].mac_addr[2],ctrl_info_p->stable_hbt_down.payload[i].mac_addr[3],ctrl_info_p->stable_hbt_down.payload[i].mac_addr[4],ctrl_info_p->stable_hbt_down.payload[i].mac_addr[5]);
                        if (memcmp(hello_ptr->mac_addr, ctrl_info_p->stable_hbt_down.payload[i].mac_addr,STKTPLG_MAC_ADDR_LEN) ==0)
                            done = 1;
                    }

                    if (1 == done && ctrl_info_p->state == STKTPLG_STATE_MASTER)
                    {
                        if (ctrl_info_p->is_ring ==FALSE)
                        {
                            /* Join To a Ring */
                            //printf("Down Line Joined to be Cosed Loop mac=%x %x %x %x %x %x\n",hello_ptr->mac_addr[0],hello_ptr->mac_addr[1],hello_ptr->mac_addr[2],hello_ptr->mac_addr[3],hello_ptr->mac_addr[4],hello_ptr->mac_addr[5]);
                            for (i = ctrl_info_p->total_units_up -1; i > 0; i--)
                            {
                                memcpy(&ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - i -1],
                                       &ctrl_info_p->stable_hbt_up.payload[i],
                                       sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                            }
                            ctrl_info_p->total_units_down =  ctrl_info_p->total_units_up ;

                            ctrl_info_p->is_ring = TRUE ;
                            /* Notify neighbors for closed loop re-topology
                             */
                            STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
                            /*  If a link is connected to form a ring , no action */
                            /*  STKTPLG_ENGINE_ConfigStackInfoToIUC();*/
                            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
                            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M, STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);

                            ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;
                        }
                    } /* done */
                    else
                    {
                        STKTPLG_ENGINE_LogMsg("[TCN] Check_Stacking_Link_Up: Down Add an unit\r\n");
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_UP);
                        STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT0_DOWN);

                        /* change state to arbitration
                         */
                        if (ctrl_info_p->state == STKTPLG_STATE_MASTER)
                            tcnReason = 3;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
                        if ((reset_flag == TRUE)&&(ctrl_info_p->state != STKTPLG_STATE_MASTER)&&(ctrl_info_p->state != STKTPLG_STATE_SLAVE)&&(ctrl_info_p->state != STKTPLG_STATE_STANDALONE))
                            {
                            xgs_stacking_debug("Check_Stacking_Link_up_2 STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE\n");
                            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
                            }
                        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
                        xgs_stacking_debug("check_stacking_link_down_2 TOPO change\n");
#else
                        STKTPLG_ENGINE_ReturnToArbitrationState(0);
                        if (reset_flag == TRUE)
                            *notify_msg = STKTPLG_IN_PROCESS_AND_ENTER_TRANSITION_MODE;
#endif
                    }/* done*/
                } /* DOWN port */
            }/* else*/
        }    
        L_MM_Mref_Release(&mref_handle_p);
        return TRUE;
    }/* Hello type 0 */
    return FALSE;
}

/* FUNCTION NAME: STKTPLG_ENGING_Check_Closed_Loop_TCN
 * PURPOSE: Handle the reception of the STKTPLG_CLOSED_LOOP_TCN type HBT packet.
 * INPUT:   ctrl_info_p   -  pointer to control info of STKTPLG_OM.
 *          rx_port       -  phy port id where the HBT packet is received.
 *          mref_handle_p -  mref handle of the recevied HBT packet.
 * OUTPUT:  None
 * RETUEN:  TRUE  - The HBT packet had been handled by this function and the
 *                  caller does not need to process it anymore.
 *          FALSE - The HBT packet is not handled by this function.
 * NOTES:
 *          The caller does not need to release mref_handle_p when the return
 *          value of this function is TRUE.
 */
static BOOL_T STKTPLG_ENGING_Check_Closed_Loop_TCN(STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p, UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p)
{
    STKTPLG_OM_HBT_3_T    *hbt3_ptr; /*here should be STKTPLG_OM_HBT_3_T,or will cause pkt parse error and will cause exception*/
    UI32_T                 i, j;
    UI32_T                 uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        /* There is no way to handle the HBT packet correctly if the phy port id
         * of the stacking port cannot be got. So just return TRUE here to pretend
         * that the packet had been handled.
         */
        L_MM_Mref_Release(&mref_handle_p);
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        return TRUE;
    }

    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        /* There is no way to handle the HBT packet correctly if the phy port id
         * of the stacking port cannot be got. So just return TRUE here to pretend
         * that the packet had been handled.
         */
        L_MM_Mref_Release(&mref_handle_p);
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        return TRUE;
    }

    /*modified by Jinhua Wei,because assignment from incompatible pointer type*/
    hbt3_ptr = (STKTPLG_OM_HBT_3_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy here */
    if(hbt3_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed. up %d ,down %d ,ring %d\n",__FUNCTION__,__LINE__,ctrl_info_p->total_units_up,ctrl_info_p->total_units_down,ctrl_info_p->is_ring);
        return FALSE;
    }

    if (hbt3_ptr->header.type == STKTPLG_CLOSED_LOOP_TCN)
    {
        hbt3_ptr = (STKTPLG_OM_HBT_3_T *)L_MM_Mref_GetPdu(mref_handle_p, &i);

        if (ctrl_info_p->is_ring != hbt3_ptr->payload.is_ring)
        {

            ctrl_info_p->is_ring = hbt3_ptr->payload.is_ring;
            if (TRUE == ctrl_info_p->is_ring)
            {
                for (i = ctrl_info_p->total_units_down -1; i > 0; i--)
                {
                    memcpy(&ctrl_info_p->stable_hbt_up.payload[ctrl_info_p->total_units_up + ctrl_info_p->total_units_down - i -1],
                           &ctrl_info_p->stable_hbt_down.payload[i],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }
                ctrl_info_p->total_units_up = ctrl_info_p->total_units_up + ctrl_info_p->total_units_down -1 ;
                for (i = ctrl_info_p->total_units_up -1; i > 0; i--)
                {
                    memcpy(&ctrl_info_p->stable_hbt_down.payload[ctrl_info_p->total_units_up - i],
                           &ctrl_info_p->stable_hbt_up.payload[i],
                           sizeof(STKTPLG_OM_HBT_0_1_Payload_T));
                }
                ctrl_info_p->total_units_down = ctrl_info_p->total_units_up;
                ctrl_info_p->total_units = ctrl_info_p->total_units_up;
            }
            else
            {
                ctrl_info_p->total_units_up   = hbt3_ptr->payload.total_units_up;
                ctrl_info_p->total_units_down = hbt3_ptr->payload.total_units_down;
            }

            ctrl_info_p->stable_hbt_up.header.next_unit   = ctrl_info_p->total_units_up + 1;
            ctrl_info_p->stable_hbt_down.header.next_unit = ctrl_info_p->total_units_down + 1;

            if (rx_port == uplink_port)
            {

                if (LAN_TYPE_TX_DOWN_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_DOWN_LINK))
                    STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_DOWN_LINK);
            }
            else /* if (rx_port == downlink_port) */
            {

                if (LAN_TYPE_TX_UP_LINK == (ctrl_info_p->stacking_ports_logical_link_status & LAN_TYPE_TX_UP_LINK))
                    STKTPLG_TX_SendClosedLoopTCN(NULL, NORMAL_PDU, LAN_TYPE_TX_UP_LINK);
            }
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_DOWN);
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_HBT1_UP);
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M,STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_HBT1_M_DOWN,STKTPLG_TIMER_HBT1_TIMEOUT_IN_MASTER);

            for (j = ctrl_info_p->total_units_up - 1; j > 0; j--)
                xgs_stacking_debug(" [%s]%d up[%ld] = %d\r\n",__FUNCTION__,__LINE__, j, ctrl_info_p->stable_hbt_up.payload[j].unit_id);
            for (j = 0; j < ctrl_info_p->total_units_down; j++)
                xgs_stacking_debug(" [%s]%d down[%ld] = %d\r\n",__FUNCTION__,__LINE__, j, ctrl_info_p->stable_hbt_down.payload[j].unit_id);
            /* If a link is connected to form a ring , no action */
            if (FALSE == ctrl_info_p->is_ring)
            {
                STKTPLG_ENGINE_ConfigStackInfoToIUC();
            }
        } /* if (ctrl_info_p->is_ring != hbt3_ptr->payload.is_ring) */
        L_MM_Mref_Release(&mref_handle_p);

        return TRUE;
    } /* if (hbt3_ptr->header.type == STKTPLG_CLOSED_LOOP_TCN) */

    return FALSE;
}

static void  STKTPLG_ENGINE_PktMasterStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port)
{
  /*added by Jinhua.Wei, because those doesn't used*/
 #if 0
    UI32_T  unit,uid,k;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
    UI32_T  max_option_port_number,max_port_number;
    int     i;
 #endif
  /*addby fen.wang, for our dut not support expansion module,it is useless*/
 #if 0
    STKTPLG_ENGINE_AssignExpModuleID(ctrl_info_p,hbt_ptr,rx_port);
#if (SYS_CPNT_EXPANSION_MODULE_HOT_SWAP == TRUE)
    om_portmap_change = FALSE ;
    for(unit=0;unit<ctrl_info_p->total_units;unit++)
    {
        uid=hbt_ptr->payload[unit].unit_id;
        STKTPLG_OM_GetPortMapping(port_mapping,uid);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(uid, &max_port_number);
#else
        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_GetMaxPortNumberOnBoard(uid, &max_port_number);
#endif
        if (STKTPLG_BOARD_GetModuleInformation(hbt_ptr->payload[uid-1].expansion_module_type,&module_info_p))
        {
            memcpy(&port_mapping[max_port_number].device_id,&module_info_p->userPortMappingTable[0].device_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].device_port_id,&module_info_p->userPortMappingTable[0].device_port_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].phy_id,&module_info_p->userPortMappingTable[0].phy_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].port_type,&module_info_p->userPortMappingTable[0].port_type,sizeof(UI8_T)*max_option_port_number);
        }
        if(port_mapping[max_port_number].module_id  != ctrl_info_p->exp_module_id[uid-1])
        {
            om_portmap_change = TRUE;
            for (i=0,k=max_port_number; k<(max_port_number+max_option_port_number);i++, k++)
            {
                port_mapping[k].module_id = ctrl_info_p->exp_module_id[uid-1];
            }
            STKTPLG_MGR_SetPortMapping(port_mapping,uid);
        }
    }

    if(om_portmap_change == TRUE )
    {
        module_swapped = FALSE;
        STKTPLG_ENGINE_ConfigStackInfoToIUC();
    }

    /*1219 modify  */
    if(ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]!=UNKNOWN_MODULE_ID)
    {
        if(ctrl_info_p->expansion_module_id   !=ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1])
        {
            ctrl_info_p->expansion_module_id    =ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1];
            STKTPLG_ENGINE_EventHandler(ASSIGNID_EV);

        }
    }

    return;
#endif
#endif
}

static void  STKTPLG_ENGINE_PktMasterSyncStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr,UI8_T rx_port)
{
#if 0/*addby fen.wang, for our dut not support expansion module,it is useless*/
    UI32_T  unit,uid,k;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
    UI32_T  max_option_port_number,max_port_number;
    int     i;




    STKTPLG_ENGINE_AssignExpModuleID(ctrl_info_p,  hbt_ptr,rx_port);
    /*remmark on 1226
      om_portmap_change = FALSE ;
     */
    for(unit=0;unit<ctrl_info_p->total_units;unit++)
    {
        uid=hbt_ptr->payload[unit].unit_id;
        STKTPLG_OM_GetPortMapping(port_mapping,uid);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(uid, &max_port_number);
#else
        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_GetMaxPortNumberOnBoard(uid, &max_port_number);
#endif
        if (STKTPLG_BOARD_GetModuleInformation(hbt_ptr->payload[uid-1].expansion_module_type,&module_info_p))
        {
            memcpy(&port_mapping[max_port_number].device_id,&module_info_p->userPortMappingTable[0].device_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].device_port_id,&module_info_p->userPortMappingTable[0].device_port_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].phy_id,&module_info_p->userPortMappingTable[0].phy_id,sizeof(UI8_T)*max_option_port_number);
            memcpy(&port_mapping[max_port_number].port_type,&module_info_p->userPortMappingTable[0].port_type,sizeof(UI8_T)*max_option_port_number);
        }
        if(port_mapping[max_port_number].module_id  !=  ctrl_info_p->exp_module_id[uid-1])
        {
            for (i=0,k=max_port_number; k<(max_port_number+max_option_port_number); i++,k++)
            {
                port_mapping[k].module_id = ctrl_info_p->exp_module_id[uid-1];
            }
            STKTPLG_MGR_SetPortMapping(port_mapping,uid);
        }
    }
    /*modify by 1219 */
    if(ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1]!=UNKNOWN_MODULE_ID)
    {
        if(ctrl_info_p->expansion_module_id   !=ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1])
        {
            ctrl_info_p->expansion_module_id    =ctrl_info_p->exp_module_id[ctrl_info_p->my_unit_id-1];
            STKTPLG_ENGINE_EventHandler(ASSIGNID_EV);
        }
    }
#endif
    return;
}


static void  STKTPLG_ENGINE_PktSlaveStateOmProcess(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p, STKTPLG_OM_HBT_0_1_T *hbt_ptr)
{
#if 0/*addby fen.wang, for our dut not support expansion module,it is useless*/
    UI32_T  unit,uid,k;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
    UI32_T  max_option_port_number,max_port_number;
    int     i;


    slave_config =FALSE;
    for(unit=0;unit<ctrl_info_p->total_units;unit++)
    {
        uid=hbt_ptr->payload[unit].unit_id;
        STKTPLG_OM_GetPortMapping(port_mapping,uid);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(uid, &max_port_number);
#else
        STKTPLG_OM_GetMaxPortNumberOfModuleOnBoard(uid, &max_option_port_number);
        STKTPLG_OM_GetMaxPortNumberOnBoard(uid, &max_port_number);
#endif
        if(port_mapping[max_port_number].module_id  !=hbt_ptr->payload[unit].expansion_module_id)
        {
            slave_config = TRUE;
            /* Modified by Vincent on 17,Dec
             * Module could be hot swapped and if a moule was found in slave state
             * the OM in slave should be modified
             */


            #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_OM_ENG_GetDeviceInfo(uid, &device_info);
            device_info.exp_module_presented[0] = (hbt_ptr->payload[unit].expansion_module_id==0xff)?0:1;
            STKTPLG_OM_ENG_SetDeviceInfo(uid, &device_info);
            #else
            STKTPLG_OM_GetDeviceInfo(uid, &device_info);
            device_info.exp_module_presented[0] = (hbt_ptr->payload[unit].expansion_module_id==0xff)?0:1;
            STKTPLG_OM_SetDeviceInfo(uid, &device_info);
            #endif
        }
        ctrl_info_p->exp_module_id[uid-1]=hbt_ptr->payload[unit].expansion_module_id;
        if (STKTPLG_BOARD_GetModuleInformation(hbt_ptr->payload[uid-1].expansion_module_type,&module_info_p))
        {
            memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable,sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
        }
        for (i=0,k=max_port_number;k<(max_port_number+max_option_port_number) ; i++,k++)
        {
            port_mapping[k].module_id = ctrl_info_p->exp_module_id[uid-1];
        }
        STKTPLG_MGR_SetPortMapping(port_mapping,uid);
    }

    if(slave_config == TRUE)
    {
        STKTPLG_ENGINE_ConfigStackInfoToIUC();
    }
#endif
    return;
}

static void  STKTPLG_ENGINE_SlaveSendAssignEvToOm(STKTPLG_OM_Ctrl_Info_T *ctrl_info_p)
{

    if((ctrl_info_p->expansion_module_id!=UNKNOWN_MODULE_ID) && (FALSE ==slave_expansion_assignevsend))
    {
        STKTPLG_ENGINE_EventHandler(ASSIGNID_EV);
        slave_expansion_assignevsend = TRUE ;
    }
    return;
}

static UI8_T  inc(UI8_T seq_no)
{
    if (seq_no == 255)
        return(0) ;
    else
        return(++seq_no);
}

#endif /* #if (SYS_CPNT_STACKING == TRUE) */

/* FUNCTION NAME: STKTPLG_ENGINE_TCN
 * PURPOSE: This function is to result STKTPLG TCN and reassigned unit ID to all units
 *          based on the stacking spec if renumber==TRUE
 * INPUT:   renumber -- Renumbering
 * OUTPUT:  None
 * RETUEN:  TRUE   -- Renumbering work (not standalone or unit ID not equal to 1)
 *          FALSE  -- otherwise
 * NOTES:
 *
 */
BOOL_T STKTPLG_ENGINE_TCN(BOOL_T renumber)
{
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    /* 181105 tc
     * reset ctrl_info_p->past_role = STKTPLG_STATE_INIT for all cases
     * Else could casue test case to fail in STKTPLG_ENGINE_ProcessArbitrationState,
     * and unable to complete provision.
     */
    ctrl_info_p->past_role = STKTPLG_STATE_INIT;
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif

    if (renumber == FALSE)
    {
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
        STKTPLG_ENGINE_ReturnToArbitrationState(0, FALSE,FALSE);
        xgs_stacking_debug("TCN_1 TOPO change\n");
#else
        STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
        return (TRUE);
    }
    else
    {
        if ((notify_stkctrl_transition==FALSE) && ((ctrl_info_p->state != STKTPLG_STATE_STANDALONE) || (ctrl_info_p->my_unit_id !=1)))
        {
            stktplg_stackingdb_updated = FALSE;
            notify_stkctrl_transition = TRUE;
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_ENGINE_ReturnToArbitrationState(0, TRUE,FALSE);
            xgs_stacking_debug("TCN_2 TOPO change\n");
#else
            STKTPLG_ENGINE_ReturnToArbitrationState(0);
#endif
            return (TRUE); /* Renumbering work */
        }
        return (FALSE); /* Renumbering not work */
    }
}

#if (SYS_CPNT_STACKING == TRUE)

static void STKTPLG_ENGINE_CheckAllOmReady(STKTPLG_OM_HBT_0_1_T *hbt1_ptr)
{
    int unit;

    /* check each slave's option module  is ready in Slave mode
     */
    for (unit = 1; /* from first slave machine( unit 2, index = 1) */
         unit < (hbt1_ptr->header.next_unit - 1); /* up to the last slave machine */
         unit++ /* next machine */)
    {
        if(TRUE ==hbt1_ptr->payload[unit].expansion_module_ready)
        {
            if(FALSE==STKTPLG_OM_ExpModuleIsInsert(hbt1_ptr->payload[unit].unit_id))
            {
                //printf(" unit=%d insert OM\r\n", hbt1_ptr->payload[unit].unit_id);
                STKTPLG_OM_InsertExpModule(hbt1_ptr->payload[unit].unit_id ,hbt1_ptr->payload[unit].expansion_module_type,hbt1_ptr->payload[unit].module_runtime_fw_ver);
            }
        }
    }
}

static void STKTPLG_ENGINE_CheckTplgSyncStatus()
{
    UI32_T timeout;
    UI8_T  sync_bmp;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    sync_bmp = STKTPLG_OM_GetTplgSyncBmp();

    if (sync_bmp)
    {
        /* printf("sync_bmp: %u.\r\n", sync_bmp); */
    }

    if (sync_bmp)
    {
        /* this database is dirty, and first time to know it in ENGINE. or
         * not first timeknow it but timeout.
         */
        if ((FALSE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_TPLG_SYNC, &timeout)) ||
            (TRUE == STKTPLG_TIMER_TimerStatus(STKTPLG_TIMER_TPLG_SYNC, &timeout) &&
             TRUE == STKTPLG_TIMER_CheckTimeOut(STKTPLG_TIMER_TPLG_SYNC)))
        {
            UI32_T port_to_send;

            if (ctrl_info_p->is_ring)/*ring*/
            {
                port_to_send = LAN_TYPE_TX_DOWN_LINK;
            }
            else
            {
                if (STKTPLG_ENGINE_IS_UPLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status))
                {
                    port_to_send = LAN_TYPE_TX_UP_LINK;
                }
                else
                {
                    port_to_send = LAN_TYPE_TX_DOWN_LINK;
                }
            }

            /* send a new TPLG_SYNC
             */
            STKTPLG_TX_SendTplgSync(0, port_to_send, sync_bmp);
            /* printf("New TplgSyn.\r\n");*/

            /* start TPLG_SYNC timer
             */
            STKTPLG_TIMER_StartTimer(STKTPLG_TIMER_TPLG_SYNC ,STKTPLG_TIMER_TPLG_SYNC_TIMEOUT);
        }
    }
}

/* FUNCTION NAME: STKTPLG_ENGINE_ProcessTplgSyncPkt
 * PURPOSE: This function handles the reception of the STKTPLG_TPLG_SYNC
 *          HBT packet.
 * INPUT:   rx_port       - phy port id where the HBT packet is received.
 *          mref_handle_p - mref handle of the recevied HBT packet.
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *          This function is only allowed to be called from
 *          the function name with prefix function name
 *          "STKTPLG_ENGINE_HandlePkt". The mref handle shall be released by
 *          the caller and will not be released in this function.
 */
static void STKTPLG_ENGINE_ProcessTplgSyncPkt(UI8_T rx_port, L_MM_Mref_Handle_T *mref_handle_p)
{
    STKTPLG_OM_TPLG_SYNC_T  *tplg_sync_ptr;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p;
    UI32_T                  i,original_src_unit;
    BOOL_T                  send_to_next;
    UI8_T                   sync_bmp;
    UI32_T                  uplink_port, downlink_port;

    if (UP_PORT_RX(&uplink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get up stacking port phy id.", __FUNCTION__, __LINE__);
        return;
    }
    if (DOWN_PORT_RX(&downlink_port) == FALSE)
    {
        xgs_stacking_debug("%s(%d)Failed to get down stacking port phy id.", __FUNCTION__, __LINE__);
        return;
    }

    tplg_sync_ptr = (STKTPLG_OM_TPLG_SYNC_T *)L_MM_Mref_GetPdu(mref_handle_p, &i); /* i is used as dummy here */
    if(tplg_sync_ptr==NULL)
    {
        printf("%s(%d)Get pdu failed.\r\n", __FUNCTION__, __LINE__);
        return;
    }

#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
    original_src_unit = tplg_sync_ptr->payload.src_unit_id;

    /* printf("Got TplgSyn.\r\n"); */

    if (original_src_unit != ctrl_info_p->my_unit_id)
    {
        /* printf("Not mine.\r\n"); */

        if (tplg_sync_ptr->payload.unit_bmp_to_get & (1<<(ctrl_info_p->my_unit_id-1))    )
        {
            /* printf("Set device info.\r\n"); */

            /* Modified by Vincent on 5,Sep,2004
             * expansion module status should be kept in order not be overwritten
             */
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
            STKTPLG_OM_ENG_GetDeviceInfo(original_src_unit,&device_info);
#else
            STKTPLG_OM_GetDeviceInfo(original_src_unit, &device_info);
#endif
            for (i=0; i<SYS_ADPT_MAX_NBR_OF_MODULE_PER_UNIT; i++)
            {
                tplg_sync_ptr->payload.unit_cfg.exp_module_type[i] = device_info.exp_module_type[i];
                tplg_sync_ptr->payload.unit_cfg.exp_module_presented[i] = device_info.exp_module_presented[i];
            }

            /* TPLG_SYNC from other unit for me, set to my database
             */
            STKTPLG_OM_SetDeviceInfo(original_src_unit, &(((STKTPLG_OM_TPLG_SYNC_T*)tplg_sync_ptr)->payload.unit_cfg));
            tplg_sync_ptr->payload.unit_bmp_to_get &= ~(1<<(ctrl_info_p->my_unit_id-1));
        }

        send_to_next = TRUE;
    }
    else
    {
        /* My TPLG_SYNC, clean the bit map of dirty.
         */
        sync_bmp = STKTPLG_OM_GetTplgSyncBmp();
        sync_bmp &= tplg_sync_ptr->payload.unit_bmp_to_get;
        STKTPLG_OM_UpdateTplgSyncBmp(sync_bmp);

        if (0 == sync_bmp)
        {
            /* stop timer.
             */
            STKTPLG_TIMER_StopTimer(STKTPLG_TIMER_TPLG_SYNC);

            /* release
             */
            send_to_next = FALSE;
        }
        else
        {
            send_to_next = TRUE;
        }
    }

    if (TRUE == send_to_next)
    {
        /* send to next hop or bounce back to sender
         */
        if ( STKTPLG_ENGINE_IS_UPLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) !=
             STKTPLG_ENGINE_IS_DOWNLINK_PORT_UP(ctrl_info_p->stacking_ports_logical_link_status) )
        {
            /* edge: bounce to source port
             */
            /* printf("Bounce!\r\n"); */
            if (rx_port == uplink_port)
            {
                STKTPLG_TX_SendTplgSync(mref_handle_p, LAN_TYPE_TX_UP_LINK, 0);
            }
            else
            {
                STKTPLG_TX_SendTplgSync(mref_handle_p, LAN_TYPE_TX_DOWN_LINK, 0);
            }
        }
        else
        {
            /* middle: relay to another port
             */
            /* printf("Relay!\r\n"); */
            if (rx_port == uplink_port)
            {
                STKTPLG_TX_SendTplgSync(mref_handle_p, LAN_TYPE_TX_DOWN_LINK, 0);
            }
            else
            {
                STKTPLG_TX_SendTplgSync(mref_handle_p, LAN_TYPE_TX_UP_LINK, 0);
            }
        }
    }
}

static void STKTPLG_ENGINE_LogMsg(char *msg_txt)
{
    printf(msg_txt);
    SYSFUN_LogMsg(msg_txt,0,0,0,0,0,0);
}


#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
/* FUNCTION NAME: STKTPLG_ENGINE_MasterStorePastTplgInfo
 * PURPOSE: Master stores stable topology info in preparation for possible Unit Hot Insert
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES: At this stage, all the payload[].unit_id should be valid
 */
static void STKTPLG_ENGINE_MasterStorePastTplgInfo(void)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    STKTPLG_DataBase_Info_T *past_stackingdb = STKTPLG_OM_GetPastStackingDB();
    BOOL_T *unit_is_valid = STKTPLG_OM_GetUnitIsValidArray();
    BOOL_T *provision_lost = STKTPLG_OM_GetProvisionLostArray();
    int unit;
    UI8_T temp_unit_id;
    int i;

    memset(past_stackingdb, 0, SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK * sizeof(STKTPLG_DataBase_Info_T));

    for(i=0;i<SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK;i++)
    {
        past_stackingdb[i].id_flag = UNIT_ABSENT;
        provision_lost[i] = FALSE;
        unit_is_valid[i] = FALSE;
    }

    if (ctrl_info_p->is_ring)
    {
        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(past_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            past_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            past_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
            unit_is_valid[temp_unit_id-1] = TRUE;
        }
    }
    else
    {
        for (unit = ctrl_info_p->total_units_up - 1; unit >= 0; unit--)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_up.payload[unit].unit_id;
            memcpy(past_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_up.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            past_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_up.payload[unit].board_id;
            past_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
            unit_is_valid[temp_unit_id-1] = TRUE;
        }

        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            memcpy(past_stackingdb[temp_unit_id-1].mac_addr,
                   ctrl_info_p->stable_hbt_down.payload[unit].mac_addr, STKTPLG_MAC_ADDR_LEN);
            past_stackingdb[temp_unit_id-1].device_type = ctrl_info_p->stable_hbt_down.payload[unit].board_id;
            past_stackingdb[temp_unit_id-1].id_flag = UNIT_PRESENT;
            unit_is_valid[temp_unit_id-1] = TRUE;
        }
    }

    ctrl_info_p->stable_flag = TRUE;

}

/* FUNCTION NAME: STKTPLG_ENGINE_CheckAndSetProvisionLost
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETUEN:
 * NOTES: If provision_lost is TRUE, it is either an Inserted Unit,
 *        or a unit who's provisioning is lost
 */
static void STKTPLG_ENGINE_CheckAndSetProvisionLost(void)
{
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    BOOL_T *provision_lost = STKTPLG_OM_GetProvisionLostArray();
    int unit;
    UI8_T temp_unit_id;
#if 1

    if (ctrl_info_p->is_ring)
    {
        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            if( memcmp(ctrl_info_p->my_mac,
                       ctrl_info_p->stable_hbt_down.payload[unit].past_master_mac, STKTPLG_MAC_ADDR_LEN) != 0 )
            {
                provision_lost[temp_unit_id-1] = TRUE;
                STKTPLG_OM_ClearSnapShotUnitEntry(temp_unit_id);
            }
        }
    }
    else /* linear(line) topology */
    {
        for (unit = ctrl_info_p->total_units_up - 1; unit >= 0; unit--)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_up.payload[unit].unit_id;
            if( memcmp(ctrl_info_p->my_mac,
                       ctrl_info_p->stable_hbt_up.payload[unit].past_master_mac, STKTPLG_MAC_ADDR_LEN) != 0 )
            {
                provision_lost[temp_unit_id-1] = TRUE;
                STKTPLG_OM_ClearSnapShotUnitEntry(temp_unit_id);
            }
        }

        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
            temp_unit_id = ctrl_info_p->stable_hbt_down.payload[unit].unit_id;
            if( memcmp(ctrl_info_p->my_mac,
                       ctrl_info_p->stable_hbt_down.payload[unit].past_master_mac, STKTPLG_MAC_ADDR_LEN) != 0 )
            {
                provision_lost[temp_unit_id-1] = TRUE;
                STKTPLG_OM_ClearSnapShotUnitEntry(temp_unit_id);
            }
        }
    }
 #endif
}

 /* FUNCTION NAME : STKTPLG_ENGINE_SetPortMappingTable
 * PURPOSE : This function is to set the whole stack's port mapping table after
 *           topology discovery finished.
 * INPUT   : payload -- the unit's info to be used for port mapping generation.
 * OUTPUT  : None.
 * RETUEN  : None.
 * NOTES   : This function is called in the state when
 *           master - Get Topology Info state
 *           slave  - Slave Wait Assignment state
 *
 */
static void STKTPLG_ENGINE_SetPortMappingTable(STKTPLG_OM_HBT_0_1_Payload_T *payload)
{
    UI8_T  stackingPort2phyDevId_ar[STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_TYPE];
    UI8_T  stackingPort2phyPortId_ar[STKTPLG_TYPE_TOTAL_NBR_OF_STACKING_PORT_TYPE];
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
    int index;
    UI32_T                  j,max_port_number,max_option_port_number;
    STKTPLG_BOARD_ModuleInfo_T *module_info_p;
#if 0 /* JinhuaWei, 03 August, 2008 10:35:57 */
    UI32_T                  i;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#endif /* #if 0 */

    if (!STKTPLG_BOARD_GetBoardInformation(payload->board_id, board_info_p))
    {
        perror("\r\nCan not get related board information.\r\n");
        assert(0);

         /* severe problem, while loop here
            */
        while (TRUE);
    }

#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    if(payload->stacking_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
    {
        memcpy(stackingPort2phyDevId_ar, board_info_p->stackingPortUserConfiguration[payload->stacking_port_option-1].stackingPort2phyDevId_ar, sizeof(stackingPort2phyDevId_ar));
        memcpy(stackingPort2phyPortId_ar, board_info_p->stackingPortUserConfiguration[payload->stacking_port_option-1].stackingPort2phyPortId_ar, sizeof(stackingPort2phyPortId_ar));
    }
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */

    memcpy(port_mapping, board_info_p->userPortMappingTable[0], sizeof(port_mapping));
    for (index = 0; index < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; index++)
    {
        if(index < board_info_p->max_port_number_on_board)
        {
#if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
            if (payload->stacking_port_option != STKTPLG_TYPE_STACKING_PORT_OPTION_OFF)
            {
                if ((port_mapping[index].device_id == stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1]) &&
                    (port_mapping[index].device_port_id == stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_UP_LINK-1]))
                {
                    port_mapping[index].port_type = STKTPLG_PORT_TYPE_STACKING;
                }
                else if ((port_mapping[index].device_id == stackingPort2phyDevId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1]) &&
                    (port_mapping[index].device_port_id == stackingPort2phyPortId_ar[STKTPLG_TYPE_STACKING_PORT_DOWN_LINK-1]))
                {
                    port_mapping[index].port_type = STKTPLG_PORT_TYPE_STACKING;
                }
            }
#endif /* #if (SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE) */
            port_mapping[index].module_id += payload->start_module_id;

            xgs_stacking_debug("set port map index %d module  %d device %d port %d phy %d type %d\n",index,port_mapping[index].module_id,
                port_mapping[index].device_id,port_mapping[index].device_port_id,port_mapping[index].phy_id,port_mapping[index].port_type);
        }
        else
        {
            port_mapping[index].module_id      = UNKNOWN_MODULE_ID;
            port_mapping[index].device_id      = UNKNOWN_DEVICE_ID;
            port_mapping[index].device_port_id = UNKNOWN_DEVICE_PORT_ID;
            port_mapping[index].phy_id         = UNKNOWN_PHY_ID;
            port_mapping[index].port_type      = VAL_portType_other;
        }
    }

    if(STKTPLG_OM_ENG_GetMaxPortNumberOfModuleOnBoard(payload->unit_id, &max_option_port_number))
    {
     if(STKTPLG_OM_ENG_GetMaxPortNumberOnBoard(payload->unit_id, &max_port_number))
     {
        if (STKTPLG_BOARD_GetModuleInformation(payload->expansion_module_type,&module_info_p))
         {
           memcpy(&port_mapping[max_port_number],&module_info_p->userPortMappingTable[0],sizeof(DEV_SWDRV_Device_Port_Mapping_T)* max_option_port_number);
           for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
            {
                 if((max_port_number+max_option_port_number)<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
                  port_mapping[j].module_id = payload->expansion_module_id;
            }
         }
         else
         {
          for(j= max_port_number;j<(max_port_number+max_option_port_number);j++)
          {
            if((max_port_number+max_option_port_number)<SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
            port_mapping[j].module_id=UNKNOWN_MODULE_ID;
          }
         }
      }
    }

    STKTPLG_MGR_SetPortMapping(&port_mapping[0], payload->unit_id);
    return;

}

static void SKTTPLG_ENGINE_SetISCUnitInfo(UI32_T notify_msg)
{
    UI32_T  drv_unit;
    UI32_T  my_drv_unit_id;
    UI16_T  exist_drv_units = 0;
    UI16_T  valid_drv_units = 0;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();

    my_drv_unit_id = ctrl_info_p->my_unit_id;

  switch(notify_msg)
  {
    case STKTPLG_UNIT_HOT_INSERT_REMOVE:
  /*when add a unit ,do nothing.For Isc unit info will changed by driver,and csc will do provision*/
  /*when remove a unit,it should update ISC unit Info quickly.Or other CSC still think the the unit is exist,and still send
     pkt to the unit,and wait for the reply*/
    for (drv_unit=1; drv_unit<=SYS_ADPT_MAX_NBR_OF_DRIVER_UNIT_PER_STACK; drv_unit++)
    {
        if (drv_unit == my_drv_unit_id)
            continue;
        if (STKTPLG_OM_ENG_UnitExist(drv_unit))
            exist_drv_units |= IUC_STACK_UNIT_BMP(drv_unit);
        else
            continue;
        if (STKTPLG_OM_ENG_IsValidDriverUnit(drv_unit))
            valid_drv_units |= IUC_STACK_UNIT_BMP(drv_unit);
    }
    break;
    /*EPR: ES3628BT-FLF-ZZ-01085
Problem:stack:hot remove and insert units cause unit not stackable.
Rootcause:(1)somes dut stacking together,and M is master then remove more than 2 dut(such as A,B,C,all them are slaves)
            And one of them A will be the new master,BC will be the slave of A.When A is doing enter master mode
            Then hotinsert to the old topo, and BC will be M slave and the unit Id may changed according  to the M mapping table.
            For A it should changed to slave but it will hang for it is send pkt to slave according to the ISC unit bit map
         (2) at this time ,the ISC unit bit map is still BC,but BC is the slave of M

Solution: update A isc unit bitmap,and clear it.
Files:stktplg_engine.c,ISC.C,ISC_OM.C
*/
   case STKTPLG_MASTER_LOSE_MSG:
   case STKTPLG_MASTER_WIN_AND_ENTER_MASTER_MODE :
     valid_drv_units = 0;
     exist_drv_units = 0;
    break;
   default:
     return;
  }
 ISC_OM_SetDrvUnitBmp(valid_drv_units,exist_drv_units);

}


static void STKTPLG_ENGINE_ShowStackingPortConnectErrMsg(UI8_T up_down_stacking_port)
{
    if(up_down_stacking_port == LAN_TYPE_TX_UP_LINK)
    {
        printf("Incorrect up stacking port connection is detected.\r\n");
        printf("Only allow the up stacking port be connected to down stacking port.\r\n");    
    }

    if(up_down_stacking_port == LAN_TYPE_TX_DOWN_LINK)
    {

        printf("Incorrect down stacking port connection is detected.\r\n");
        printf("Only allow the down stacking port be connected to up stacking port.\r\n");    
    }
    return ;
}

/*v3-hotswap, show port_mapping info. +++*/
void STKTPLG_ENGINE_ShowStktplgInfo(void)
{
    int unit;
    STKTPLG_OM_Ctrl_Info_T  ctrl_info_shp;
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
    UI32_T is_stacking,up_link_port,down_link_port;
    UI32_T uplink,downlink;
    STKTPLG_SHOM_GetCtrlInfo(&ctrl_info_shp);
    STKTPLG_SHOM_GetHgPortLinkState(&uplink,&downlink);
    STKTPLG_SHOM_GetStackingPortInfo(&is_stacking,&up_link_port,&down_link_port);
    printf("\n~~~Start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    /*STKTPLG_OM_GetAllUnitsPortMapping(all_port_mapping_tbl);

    printf("[All Units Ports Mapping]====================================\n");
    printf("Unit Port module_id device_id device_port_id phy_id port_type\n");
    for(unit = 0; unit < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
    {
        for(port = 0; port < SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; port++)
        {
            printf("%4lu %4lu %9u %9u %14u %6u %9u\n",
                    unit,
                    port,
                    all_port_mapping_tbl[unit][port].module_id,
                    all_port_mapping_tbl[unit][port].device_id,
                    all_port_mapping_tbl[unit][port].device_port_id,
                    all_port_mapping_tbl[unit][port].phy_id,
                    all_port_mapping_tbl[unit][port].port_type);
        }
    }*/
    printf("=============================================================\n");

    printf("[ctrl_info]\n");
    printf("                        state = %u\n", ctrl_info_p->state);
    printf("                   my_unit_id = %u\n", ctrl_info_p->my_unit_id);
    printf("                  total_units = %u\n", ctrl_info_p->total_units);
    printf("               button_pressed = %u\n", ctrl_info_p->button_pressed);
#if (SYS_CPNT_STACKING_BUTTON == TRUE || SYS_CPNT_STACKING_BUTTON_SOFTWARE == TRUE)
    printf("        stacking_button_state = %u\n", ctrl_info_p->stacking_button_state);
#endif
    printf("                   image_type = %u\n", ctrl_info_p->image_type);
    printf("                    chip_nums = %u\n", ctrl_info_p->chip_nums);
    printf("                     board_id = %u\n", ctrl_info_p->board_id);
    printf("               master_unit_id = %u\n", ctrl_info_p->master_unit_id);
    printf("                query_unit_up = %u\n", ctrl_info_p->query_unit_up);
    printf("              query_unit_down = %u\n", ctrl_info_p->query_unit_down);
    printf("                       my_mac = %x:%x:%x:%x:%x:%x\n", ctrl_info_p->my_mac[0], ctrl_info_p->my_mac[1], ctrl_info_p->my_mac[2], ctrl_info_p->my_mac[3], ctrl_info_p->my_mac[4], ctrl_info_p->my_mac[5]);
    printf("                   master_mac = %x:%x:%x:%x:%x:%x\n", ctrl_info_p->master_mac[0], ctrl_info_p->master_mac[1], ctrl_info_p->master_mac[2], ctrl_info_p->master_mac[3], ctrl_info_p->master_mac[4], ctrl_info_p->master_mac[5]);
    printf("   stacking_ports_logical_link_status = %u\n", ctrl_info_p->stacking_ports_logical_link_status);
    printf("                    preempted = %u\n", ctrl_info_p->preempted);
    printf("                   bounce_msg = %u\n", ctrl_info_p->bounce_msg);
    printf("            my_phy_unit_id_up = %u\n", ctrl_info_p->my_phy_unit_id_up);
    printf("          my_phy_unit_id_down = %u\n", ctrl_info_p->my_phy_unit_id_down);
    printf("               total_units_up = %u\n", ctrl_info_p->total_units_up);
    printf("             total_units_down = %u\n", ctrl_info_p->total_units_down);
    printf("               last_module_id = %u\n", ctrl_info_p->last_module_id);
    printf("              start_module_id = %u\n", ctrl_info_p->start_module_id);
    printf("                      is_ring = %u\n", ctrl_info_p->is_ring);
    printf("                  reset_state = %u\n", ctrl_info_p->reset_state);
    printf("                up_phy_status = %u\n", ctrl_info_p->up_phy_status);
    printf("              down_phy_status = %u\n", ctrl_info_p->down_phy_status);
    printf("              stacking_dev_id = %u\n", ctrl_info_p->stacking_dev_id);
    printf("            stack_maintenance = %u\n", ctrl_info_p->stack_maintenance);
    printf("             preempted_master = %u\n", ctrl_info_p->preempted_master);
    printf("    provision_completed_state = %u\n", ctrl_info_p->provision_completed_state);
    printf("  stk_unit_cfg_dirty_sync_bmp = %u\n", ctrl_info_p->stk_unit_cfg_dirty_sync_bmp);
    printf("                    past_role = %u\n", ctrl_info_p->past_role);
    printf("              past_master_mac = %x:%x:%x:%x:%x:%x\n", ctrl_info_p->past_master_mac[0], ctrl_info_p->past_master_mac[1], ctrl_info_p->past_master_mac[2], ctrl_info_p->past_master_mac[3], ctrl_info_p->past_master_mac[4], ctrl_info_p->past_master_mac[5]);
    printf("                  stable_flag = %u\n", ctrl_info_p->stable_flag);
    printf("      share memory my_unit_id = %d,uplink %ld,downlink %ld\n", ctrl_info_shp.my_unit_id,uplink,downlink);
    printf("      share memory is_stacking = %ld,uplinkport %ld,downlinkport %ld\n", is_stacking,up_link_port,down_link_port);
    if (ctrl_info_p->is_ring)
    {
        printf("ring\n");
        for (unit = 0; unit < ctrl_info_p->total_units_down; unit++)
        {
            printf(" unit %d,unit id %d,board_id %d\n",unit,ctrl_info_p->stable_hbt_down.payload[unit].unit_id,ctrl_info_p->stable_hbt_down.payload[unit].board_id);

        }
    }
    else
    {

        for (unit = ctrl_info_p->total_units_up - 1;  unit >= 0; unit--)
        {
            printf("UP unit %d,unit id %d board_id %d\n",unit,ctrl_info_p->stable_hbt_up.payload[unit].unit_id,ctrl_info_p->stable_hbt_up.payload[unit].board_id);
            printf("stacking_ports_link_status %x\n",
             ctrl_info_p->stable_hbt_up.payload[unit].stacking_ports_link_status);
            
        }
        for (unit = 1;  unit < ctrl_info_p->total_units_down; unit++)
        {
            printf("DOWN unit %d,unit id %d board_id %d\n",unit,ctrl_info_p->stable_hbt_down.payload[unit].unit_id,ctrl_info_p->stable_hbt_down.payload[unit].board_id);
            printf("stacking_ports_link_status %x\n",
             ctrl_info_p->stable_hbt_down.payload[unit].stacking_ports_link_status);            

        }
    }
    printf("~~~End~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

}
#endif
#endif /* (SYS_CPNT_STACKING == TRUE) */
