/* Module Name: SWCTRL_TASK.C
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the task level functions of switch
 *         control.
 *        ( 2.  The domain MUST be handled by this module.      )
 *        ( 3.  The domain would not be handled by this module. )
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2001/6/1    Jimmy Lin   Create this file
 *       2001/11/1   Arthur Wu   Take over
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "sys_time.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "leaf_sys.h"
#include "leaf_2863.h"
#include "leaf_2674p.h"
#include "leaf_2674q.h"
#include "leaf_2933.h"
#include "leaf_es3626a.h"
//#include "stktplg_type.h"
//#include "stktplg_mgr.h"
#include "swctrl.h"
#include "swctrl_init.h"
#include "swctrl_task.h"
//#include "syslog_type.h"
//#include "syslog_mgr.h"
//#include "syslog_task.h"
// eli test #include "trap_mgr.h"
#include "sys_module.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "stktplg_om.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

/* NAMING CONSTANT
 */
#define LOCAL_HOST                              1


#if (SYS_CPNT_STACKING == TRUE)
/* "3" means if system up and all port link up at the same time, the
 * 1. link up message
 * 2. speed-duplex change message
 * 3. flow control change message
 */
#define SWCTRL_TASK_MSG_Q_LEN                   (3*(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)*(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT))
#else
#define SWCTRL_TASK_MSG_Q_LEN                   256
#endif


#define SWCTRL_TASK_EVENT_DIRTY                     BIT_1
#define SWCTRL_TASK_EVENT_ENTER_TRANSITION_MODE     BIT_2
#define SWCTRL_TASK_EVENT_PERIODIC                  BIT_3

#define SWCTRL_TASK_EVENT_PERIODIC_INTERVAL         100

/* MACRO DEFINITIONS
 */
#define SWCTRL_TASK_CHECK_ENABLE()     if( SWCTRL_GetOperationMode()!=SYS_TYPE_STACKING_MASTER_MODE ) return ;



/* LOCAL VARIABLES
 */
static UI32_T                   swctrl_task_id;     /* task id */
static BOOL_T                   swctrl_task_is_transition_done ;


/* Local Functions
 */
 #if (SYS_CPNT_ATC_STORM == TRUE)
static UI32_T SWCTRL_TASK_PacketFlowAnalyser(void);
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function will init all the resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_Init(void)
{
    return;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_Create_InterCSC_Relation(void)
{
    /* register callback functions
     */
#if 0 /*linux platfrom do not need this.*/
    SWDRV_Register_PortLinkUp_CallBack(SWCTRL_TASK_PortLinkUp_CallBack);
    SWDRV_Register_PortLinkDown_CallBack(SWCTRL_TASK_PortLinkDown_CallBack);
    SWDRV_Register_PortTypeChanged_CallBack(SWCTRL_TASK_PortTypeChanged_CallBack);
    SWDRV_Register_PortSpeedDuplex_CallBack(SWCTRL_TASK_PortSpeedDuplex_CallBack);
    SWDRV_Register_PortFlowCtrl_CallBack(SWCTRL_TASK_PortFlowCtrl_CallBack);
#endif
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: this will set the trasition mode, wait until the flag is set TRUE
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_SetTransitionMode(void)
{
    swctrl_task_is_transition_done = FALSE;
    SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_ENTER_TRANSITION_MODE);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterTransitionMode(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE(swctrl_task_is_transition_done);

    /*
    SWCTRL_EnterTransitionMode();
    */
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterMasterMode(void)
{

    ;

 }


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterSlaveMode(void)
{
    ;

    /*

    SWCTRL_EnterSlaveMode();
    */

}


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_CreateTask
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create Switch Control Task. This function
 *           will be called by root.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_CreateTask(void)
{
    UI32_T thread_id;

    /* create a thread for CSCA task only when it needs to take care of timer
     * event.
     */
    if(SYSFUN_SpawnThread(SYS_BLD_SWCTRL_CSC_THREAD_PRIORITY,
                          SYS_BLD_SWCTRL_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_SWCTRL_CSC_THREAD_NAME,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_FP,
                          SWCTRL_TASK_Main,
                          NULL,
                          &thread_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
    SW_WATCHDOG_MGR_RegisterMonitorThread ( SW_WATCHDOG_SWCTRL, thread_id, SYS_ADPT_SWCTRL_SW_WATCHDOG_TIMER );
#endif

    swctrl_task_id=thread_id;
} /* End of SWCTRL_Task_CreateTask() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION: Switch control main task routine
 * INPUT   : arg -- null argument
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_Main(int arg)
{
    UI32_T wait_events, event_var, rcv_events;
    UI32_T timeout, current_state, ret_value;
    UI32_T member_id;
    L_THREADGRP_Handle_T tg_handle = L2_L4_PROC_COMM_GetSwctrlGroupTGHandle();
    void* timer_id = NULL;
    SWCTRL_LinkScanReturnCode_T linkscan_ret = SWCTRL_LINKSCAN_E_ERROR;

    timer_id = SYSFUN_PeriodicTimer_Create();
    if (NULL == timer_id)
    {
        printf("%s: SYSFUN_PeriodicTimer_Create fail.\r\n", __FUNCTION__);
        return;
    }

    if (FALSE == SYSFUN_PeriodicTimer_Start(timer_id, SWCTRL_TASK_EVENT_PERIODIC_INTERVAL, SWCTRL_TASK_EVENT_PERIODIC))
    {
        printf("%s: SYSFUN_PeriodicTimer_Start fail.\r\n", __FUNCTION__);
        return;
    }

    /* join the thread group
     */
    if (FALSE == L_THREADGRP_Join(tg_handle, SYS_BLD_SWCTRL_CSC_THREAD_PRIORITY, &member_id))
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    wait_events = SWCTRL_TASK_EVENT_DIRTY |
                  SWCTRL_TASK_EVENT_ENTER_TRANSITION_MODE |
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
                  SYSFUN_SYSTEM_EVENT_SW_WATCHDOG |
#endif
                  SWCTRL_TASK_EVENT_PERIODIC ;

    event_var = 0;

    while (1)
    {
        timeout = (event_var != 0 ? SYSFUN_TIMEOUT_NOWAIT : SYSFUN_TIMEOUT_WAIT_FOREVER);
        ret_value = SYSFUN_ReceiveEvent(wait_events, 
                                        SYSFUN_EVENT_WAIT_ANY,
                                        timeout, 
                                        &rcv_events);
       
        if (SYSFUN_OK != ret_value)
        {
            if (SYSFUN_RESULT_NO_EVENT_SET != ret_value)
            {
                /* Log to system : unexpect return value
                 */
                ;
            }
        }

        event_var |= rcv_events;

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
        if (event_var & SYSFUN_SYSTEM_EVENT_SW_WATCHDOG)
        {
            SW_WATCHDOG_MGR_ResetTimer(SW_WATCHDOG_SWCTRL);
            event_var ^= SYSFUN_SYSTEM_EVENT_SW_WATCHDOG;
        }
#endif

        current_state = SWCTRL_GetOperationMode();
        /* once MGR in the transition mode, clear all the events and skip processing them.
         */
        if (current_state == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if (event_var &  SWCTRL_TASK_EVENT_ENTER_TRANSITION_MODE)
            {
                swctrl_task_is_transition_done = TRUE;
            }
            event_var = 0;
            continue;

        }
        else if (current_state == SYS_TYPE_STACKING_SLAVE_MODE)
        {
            event_var = 0;
            continue;
        }

        if (event_var & SWCTRL_TASK_EVENT_DIRTY)
        {

            /* request thread group execution permission
             */
            if (TRUE != L_THREADGRP_Execution_Request(tg_handle, member_id))
            {
                printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
            }

            linkscan_ret = SWCTRL_CallbackPostProcessor();

            if (SWCTRL_LINKSCAN_E_DIRTY != linkscan_ret)
            {
                event_var ^= SWCTRL_TASK_EVENT_DIRTY;
            }

            /* release thread group execution permission
             */
            if (TRUE != L_THREADGRP_Execution_Release(tg_handle, member_id))
            {
                printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
            }

            continue;
        }

        /* Start of ATC Broadcast Multicast Storm. */
        if (event_var & SWCTRL_TASK_EVENT_PERIODIC)
        {
            if (current_state == SYS_TYPE_STACKING_SLAVE_MODE)
            {
                /* SWCTRL task don't do any thing in slave mode
                 */
                event_var &= ~SWCTRL_TASK_EVENT_PERIODIC;
                continue;
            }

            if (SWCTRL_LINKSCAN_E_PENDING_DIRTY == linkscan_ret)
            {
                event_var |= SWCTRL_TASK_EVENT_DIRTY;
            }

#if (SYS_CPNT_ATC_STORM == TRUE)
            /* update counters of some unit normally, when
             * 1) unit stacking mode is master mode, and
             * 2) provision has been completed
             */
            SWCTRL_TASK_PacketFlowAnalyser();
#endif
            event_var &= ~SWCTRL_TASK_EVENT_PERIODIC;
        }
        /* End of ATC Broadcast Multicast Storm. */

    }
} /* End of SWCTRL_TASK_Main() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortLinkUp_CallBack(UI32_T unit,
                                            UI32_T port)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if (TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_LINKUP, unit, port, 0, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortLinkUp_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortLinkDown_CallBack(  UI32_T unit,
                                                UI32_T port)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if (TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_LINKDOWN, unit, port, 0, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortLinkDown_CallBack() */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_CraftPortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_CraftPortLinkUp_CallBack(UI32_T unit)
{
    SWCTRL_TASK_CHECK_ENABLE();

    SWCTRL_ProcessCraftPortLinkUp(unit);

} /* End of SWCTRL_TASK_CraftPortLinkUp_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_CraftPortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_CraftPortLinkDown_CallBack(  UI32_T unit)
{
    SWCTRL_TASK_CHECK_ENABLE();

    SWCTRL_ProcessCraftPortLinkDown(unit);

} /* End of SWCTRL_TASK_CraftPortLinkDown_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           module_id -- nonzero for module inserted event
 *           port_type -- port type to set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortTypeChanged_CallBack( UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T module_id,
                                                  UI32_T port_type)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if (TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_TYPE_CHANGED, unit, port, module_id, port_type))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortTypeChanged_CallBack() */



/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSpeedDuplex_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit         -- in which unit
 *           port         -- which port
 *           speed_duplex -- the speed/duplex status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortSpeedDuplex_CallBack(UI32_T unit,
                                                 UI32_T port,
                                                 UI32_T speed_duplex)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if (TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_SPEEDDUPLEX, unit, port, speed_duplex, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortSpeedDuplex_CallBack() */


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortFlowCtrl_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit      -- in which unit
 *           port      -- which port
 *           flow_ctrl -- the flow control status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortFlowCtrl_CallBack(UI32_T unit,
                                              UI32_T port,
                                              UI32_T flow_ctrl)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if (TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_FLOWCTRL, unit, port, flow_ctrl, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortFlowCtrl_CallBack() */

#if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpPresent_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit      -- in which unit
 *           port      -- which port
 *           is_present -- the sfp present status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpPresent_CallBack(UI32_T unit,
                                          UI32_T port,
                                          BOOL_T is_present)
{
    SWCTRL_TASK_CHECK_ENABLE();

    if(TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_SFP_PRESENT, unit, port, is_present, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }

} /* End of SWCTRL_TASK_PortSfpPresent_CallBack() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpInfo_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_info_p -- sfp eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpInfo_CallBack(UI32_T unit,
                                       UI32_T sfp_index,
                                       SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    SWCTRL_TASK_CHECK_ENABLE();

    SWCTRL_SetPortSfpInfo(unit, sfp_index, sfp_info_p);
} /* End of SWCTRL_TASK_PortSfpInfo_CallBack() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpDdmInfo_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp ddm info callback function
 *           update ddm info including measured ddm info
 *           update ddm threshold if auto_mode is enabled
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_ddm_info_p -- sfp DDM eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpDdmInfo_CallBack(UI32_T unit,
                                          UI32_T sfp_index,
                                          SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    UI32_T ifindex, port;
    BOOL_T auto_mode;
    SWCTRL_TASK_CHECK_ENABLE();

    SWCTRL_SetPortSfpDdmInfo(unit, sfp_index, sfp_ddm_info_p);

    if(FALSE == STKTPLG_OM_SfpIndexToUserPort(unit, sfp_index, &port))
        return;

    if(SWCTRL_LPORT_UNKNOWN_PORT == SWCTRL_UserPortToIfindex(unit, port, &ifindex))
        return;

    if(TRUE == SWCTRL_GetPortSfpDdmThresholdAutoMode(ifindex, &auto_mode))
    {
        if(auto_mode == TRUE)
            SWCTRL_SetPortSfpDdmThresholdAll(ifindex, &sfp_ddm_info_p->threshold);
    }
} /* End of SWCTRL_TASK_PortSfpDdmInfo_CallBack() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp ddm measured inf callback function, register to swdrv
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_ddm_info_measured_p -- sfp DDM measured eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack(UI32_T unit,
                                          UI32_T sfp_index,
                                          SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p)
{
    UI32_T port;
    SWCTRL_TASK_CHECK_ENABLE();

    SWCTRL_SetPortSfpDdmInfoMeasured(unit, sfp_index, sfp_ddm_info_measured_p);

#if (SYS_CPNT_SFP_DDM_ALARMWARN_TRAP == TRUE)
    STKTPLG_OM_SfpIndexToUserPort(unit, sfp_index, &port);
    if(TRUE == SWCTRL_CallbackPreProcessor(SWCTRL_CALLBACK_EVENT_PORT_SFP_DDM_INFO_MEASURED, unit, port, 0, 0))
    {
        SYSFUN_SendEvent (swctrl_task_id, SWCTRL_TASK_EVENT_DIRTY);
    }
#endif
} /* End of SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack() */
#endif /* #if (SYS_CPNT_SWDRV_MONITOR_SFP_DDM == TRUE) */

#if (SYS_CPNT_ATC_STORM == TRUE)
static UI32_T SWCTRL_TASK_PacketFlowAnalyser(void)
{
    SWCTRL_PacketFlowAnalyser();
    return TRUE;
}
#endif

