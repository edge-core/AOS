/* Module Name: STKTPLG_TASK.C
 *
 * Purpose:
 *
 * Notes:   
 *    The operations done by STKTPLG_TASK will be handled by STKTPLG_PROC mgr
 *    thread on linux platform.
 *
 * History:                                                               
 *    10/04/2002       -- David Lin, Create
 *    07/31/2007       -- Charlie Chen, Port to linux platform
 *
 * Copyright(C)      Accton Corporation, 2002 - 2007
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#if 0
#include <vxWorks.h>
#include <semLib.h>
#include <taskLib.h>
#endif
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "sys_type.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "sys_hwcfg.h"
#include "stkmgmt_type.h"
#include "stktplg_type.h"
#include "stktplg_task.h"
#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "stktplg_timer.h"
#include "stktplg_tx.h"
#include "stktplg_engine.h"
#include "stktplg_board.h"
#include "uc_mgr.h"
#include "sysfun.h"
#include "l_mm.h"
#include "l_ipcmem.h"
#include "sys_callback_mgr.h"
#include "syslog_type.h"
#include "syslog_om.h"
#include "sys_time.h"
#include "sys_module.h"
#include "isc.h"
#include "leaf_sys.h"
#include "phyaddr_access.h"
#include "l_pbmp.h"

/* NAMING CONSTANT DECLARATIONS
 */
 
/* pattern for security check
 */
#define STKTPLG_TASK_COOKIE_PATTERN         0xDEADBEEF 

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct STKTPLG_TASK_MSG_S
{
    UI32_T    cookie;		
    L_MM_Mref_Handle_T  *mref_handle_p;
    UI8_T     rx_port;
    UI8_T     pad[7];
   
}__attribute__((packed, aligned(1))) STKTPLG_TASK_MSG_T;	

#ifdef STKTPLG_TASK_DEBUG
#define DBG(x...) printf(x)
#else
#define DBG(x...) 
#endif


/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static void STKTPLG_TASK_ModuleStatusPolling(void);

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
static void                 STKTPLG_TASK_ModuleStatusDetection(void);
#endif
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

/* STATIC VARIABLE DECLARATIONS 
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKTPLG_TASK_InitiateProcessResources
 * PURPOSE: This function initializes all releated variables and restarts
 *          the state machine.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
BOOL_T STKTPLG_TASK_InitiateProcessResources(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;
    STKTPLG_BOARD_BoardInfo_T  board_info;
    STKTPLG_BOARD_BoardInfo_T *board_info_p = &board_info;
  
    /* UC_MGR_InitiateProcessResources() must be called before here
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);
    }

    if (!STKTPLG_BOARD_GetBoardInformation(uc_sys_info.board_id, board_info_p))
    {
    	perror("\r\nCan not get related board information.");
        assert(0);
    }

    return (TRUE);

} /* End of STKTPLG_TASK_Initiate_System_Resources() */

/* FUNCTION NAME : STKTPLG_TASK_HandleEvents
 * PURPOSE: This procedure will handle the events belong to STKTPLG_TASK
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   None.
 */
void STKTPLG_TASK_HandleEvents(UI32_T events)
{
    UI32_T notify_msg = STKTPLG_NO_ACTION;

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE) && (SYS_CPNT_MODULE_WITH_CPU == FALSE)
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_ENG_GetCtrlInfo();
#else
    STKTPLG_OM_Ctrl_Info_T  *ctrl_info_p = STKTPLG_OM_GetCtrlInfo();
#endif
#endif

    if(events & STKTPLG_TASK_EVENT_PERIODIC_TIMER)
    {
        STKTPLG_ENGINE_StackManagement(TRUE, 0, NULL, &notify_msg);

        STKTPLG_TASK_ModuleStatusPolling();

#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
    /* EPR: ES4626F-SW-FLF-LNA-00158
     * Config module after provision's completed, or slave's 
     * 10G port is not UP after warm reboot.
     * The root cause is unknown.
     *
     * EPR: ES4626F-SW-FLF-38-01722
     * 10GBASE-T module can't up after boot if the module is
     * inserted before power on.
     * Postpone module detection until provision complete and
     * then 10GBASE-T module can work correctly.
     * The root cause is unknown.
     */
    if(ctrl_info_p->state == STKTPLG_STATE_STANDALONE && STKTPLG_OM_ENG_ProvisionCompletedOnce())
    {       
        STKTPLG_TASK_ModuleStatusDetection();
    }
    if(ctrl_info_p->state == STKTPLG_STATE_MASTER && STKTPLG_OM_ENG_ProvisionCompletedOnce()) 
    {           
        STKTPLG_TASK_ModuleStatusDetection();
    }
    if(ctrl_info_p->state == STKTPLG_STATE_SLAVE && STKTPLG_OM_SlaveIsReady() && ctrl_info_p->provision_completed_state)
    {
        STKTPLG_TASK_ModuleStatusDetection();
    }

#endif
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */

        /* check if we need to notify stack control to change system state
         */
        if (notify_msg != STKTPLG_NO_ACTION)
        {
            DBG("%s: notify_msg = %lu\n",__FUNCTION__, notify_msg);
#if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
           if( notify_msg == STKTPLG_UNIT_HOT_INSERT_REMOVE )
            {
                SYS_CALLBACK_MGR_UnitHotInsertRemoveCallBack(SYS_MODULE_STKTPLG, notify_msg);
            }
           else
#endif
            {
            STKTPLG_MGR_SlaveReady(FALSE);
            SYS_CALLBACK_MGR_StackStateCallBack(SYS_MODULE_STKTPLG, notify_msg);
            }
        }
    }

}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME : STKTPLG_TASK_ReceivePackets
 * PURPOSE: 
 * INPUT:   key           -- isc key passed from ISC
 *          mref_handle_p -- pointer to mref handle that cotains the packet coming for stack management
 *          rx_port       -- from which port the packet is received(UP or DOWN)
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 */
BOOL_T STKTPLG_TASK_ReceivePackets(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p, UI8_T rx_port)
{
    UI32_T              notify_msg;

    notify_msg = STKTPLG_NO_ACTION;

    STKTPLG_ENGINE_StackManagement(FALSE, rx_port, mref_handle_p, &notify_msg);

    /* check if we need to notify stack control to change system state
     */
    if (notify_msg != STKTPLG_NO_ACTION)
    {
       #if (SYS_CPNT_UNIT_HOT_SWAP == TRUE)
           if( notify_msg == STKTPLG_UNIT_HOT_INSERT_REMOVE )
            {
                SYS_CALLBACK_MGR_UnitHotInsertRemoveCallBack(SYS_MODULE_STKTPLG, notify_msg);
            }
           else
       #endif
        SYS_CALLBACK_MGR_StackStateCallBack(SYS_MODULE_STKTPLG, notify_msg);
    }

    return TRUE;
} /* End of STKTPLG_TASK_ReceivePackets() */	

/*-------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_TASK_ModuleStatusPolling
 *-------------------------------------------------------------------------
 * PURPOSE : This routine is used to poll module present existing status
 *           from database, and then callback registered CSC.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : This function is for expansion module
 *           board which contains a CPU.
 *-------------------------------------------------------------------------
 */
static void STKTPLG_TASK_ModuleStatusPolling(void)
{
    BOOL_T status_dirty[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK];
    UI32_T unit;
    
    if (TRUE == STKTPLG_MGR_UpdateModuleStateChanged(status_dirty))
    {
        for(unit=1; unit<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; unit++)
        {
            if (TRUE == status_dirty[unit-1])
            {
                SYS_CALLBACK_MGR_ModuleStateChangedCallBack(SYS_MODULE_STKTPLG, unit);
            }
        }
    }
}
#if (SYS_CPNT_10G_MODULE_SUPPORT == TRUE)
#if (SYS_CPNT_MODULE_WITH_CPU == FALSE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - STKTPLG_TASK_ModuleStatusDetection
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to detect module status.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : 1. If module status change, re-config module.
 *           2. If module status not change, do nothing.
 *-------------------------------------------------------------------------
 */
static void STKTPLG_TASK_ModuleStatusDetection(void)
{
    BOOL_T module_is_present;
    UI8_T module_id;
    UI8_T module_slot_idx;

    if (!STKTPLG_SHOM_GetConfigTopologyInfoDoneStatus())
    {
        return;
    }

    for (module_slot_idx = 0; module_slot_idx < SYS_HWCFG_MAX_NUM_OF_TENG_MODULE_SLOT; module_slot_idx++)
    {
        if (STKTPLG_MGR_IsModulePrzStatusChanged(module_slot_idx, &module_is_present, &module_id))
        {
            STKTPLG_MGR_ConfigModule(module_slot_idx, module_is_present, module_id);
        }
    }
}
#endif /* #if (SYS_CPNT_MODULE_WITH_CPU == FALSE) */
#endif /*end of SYS_CPNT_10G_MODULE_SUPPORT */


