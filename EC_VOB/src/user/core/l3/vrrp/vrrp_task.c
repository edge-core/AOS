/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_TASK.c                                                                 
 *                                                                                      
 * PURPOSE: 
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009                             
 * -------------------------------------------------------------------------------------*/



/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include "sys_adpt.h"
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "vrrp_task.h"
#include "vrrp_type.h"
#include "vrrp_mgr.h"
#include "vrrp_om.h"
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "syslog_task.h"
#include "sys_module.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define VRRP_TASK_EVENT_1_SEC              0x0001L
#define VRRP_TASK_EVENT_MESSAGE_ARRIVED    0x0002L
#define VRRP_TASK_EVENT_ENTER_TRANSITION   0x0004L
#define VRRP_TASK_PROVISION_COMPLETE_EVENT 0x0008L
#define VRRP_TASK_TIMER_TICKS_1_SEC        100 /* 1 sec = 100 ticks */
#define VRRP_ENTER_TRANSITION_MODE_SLEEP_TICK  100
#define VRRP_TASK_MSGQ_LEN			       10

/* LOCAL DATATYPE DECLARATION
 */
 
typedef	struct VRRP_TASK_LCB_S
{
	BOOL_T			task_created_flag;		/*	TRUE-task created, FALSE-no	*/
	BOOL_T			enable_vrrp;			/*	TRUE-enable, FALSE-disable	*/
	UI32_T			task_id;		        /*	VRRP task id	*/
	UI32_T			msg_id;			        /*	The message queue to receive incoming VRRP 
	                                         *  Advertisement packet.       */
}	VRRP_TASK_LCB_T, *VRRP_TASK_LCB_P_T; 

typedef struct  VRRP_TASK_MSG_S
{
    UI32_T              mtype;      /* encoding message type, source    */
    UI8_T               *mtext;     /* point message block  */
    UI32_T              mreserved;  /* not defined for future extension */
}   VRRP_TASK_MSG_T;

typedef struct VRRP_TASK_MSG_BLK_S
{
    VRRP_TASK_MSG_QUEUE_ITEM_T   vrrpMsgQueItem; 
}   VRRP_TASK_MSG_BLK_T;

enum VRRP_TASK_MESSAGE_ACTION_MODE_E
{
    RX_MULTI_PACKET
};


 
/* STATIC VARIABLE DECLARATIONS  
 */ 
static VRRP_TASK_LCB_T   vrrp_task_lcb;
static BOOL_T  is_transition_done;


/* EXPORTED SUBPROGRAM BODIES
 */ 

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_EnterTransitionMode                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will configured VRRP to enter transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/
void VRRP_TASK_EnterTransitionMode (void)
{  
#if 0
    /*SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);*/
    is_transition_done = FALSE;
    SYSFUN_SendEvent(vrrp_task_lcb.task_id, VRRP_TASK_EVENT_ENTER_TRANSITION);

    while (is_transition_done == FALSE)
    {
       SYSFUN_Sleep(VRRP_ENTER_TRANSITION_MODE_SLEEP_TICK);
    }
    return;
#endif
}   /* End of VRRP_TASK_EnterTransitionMode */


/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_SetTransitionMode                                  
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will configured VRRP to set transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none                                                              
 * NOTES    : none                                                              
 *------------------------------------------------------------------------------*/
void VRRP_TASK_SetTransitionMode(void)
{
    return;
} /* VRRP_TASK_SetTransitionMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_TASK_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process necessary procedures when provision completes
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE/FALSE
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_TASK_ProvisionComplete(void)
{
#if 0
    SYSFUN_SendEvent(vrrp_task_lcb.task_id, VRRP_TASK_PROVISION_COMPLETE_EVENT);
#endif
    return;
} /* VRRP_TASK_ProvisionComplete() */



 