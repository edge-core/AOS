/* Module Name: ISC_AGENT.C
 * Purpose: 
 *      This module provides two queues for those CSCs which don't have their own queue to 
 *      store packet sent from remote unit. The role of ISC_AGENT is in fact the "agent" of
 *      those ISC clients which mentioned above: register callback functions to ISC, queue 
 *      and dequeue ISC's packet before passing it to corresponding CSC. 
 * Notes: 
 * History:                                                               
 *    
 * Copyright(C)      Accton Corporation, 2005   				
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdio.h>
#include "isc.h"
#include "isc_agent.h"
#include "l_mm.h"
#include "sysfun.h"
#include "sys_bld.h"
#include "lan.h"
#include "amtrdrv_mgr.h"
#include "swdrv.h"
#include "leddrv.h"
#include "fs.h"
#include "swdrvl4.h"
#include "rule_ctrl.h"
#include "sysdrv.h"
#include "hrdrv.h"
#include "swdrvl3.h"
#include "swdrv_cache_task.h"
#include "backdoor_mgr.h"
#if(SYS_CPNT_L2MCAST == TRUE)
#include "msl_pmgr.h"
#endif
#include "dev_swdrvl4_pmgr.h"
#if (SYS_CPNT_POE == TRUE)
#include "poedrv.h"
#endif
#include "sys_time_stk.h"
#define Driver_Group_Stacking_Mode_OK

/* NAMING CONSTANT DECLARATIONS
 */

#define ISC_AGENT_MSG_SIZE        sizeof(IscAgentMessage_T)
/*  rewrite the register method
 *add by tony.lei*/
typedef BOOL_T (* ISC_AGENT_ServiceFunc_T)(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref);

 struct isc_agent_callback {
    UI8_T isc_agent_service_mode;
    ISC_AGENT_ServiceFunc_T   isc_agent_handler_func;
};
typedef struct isc_agent_callback  ISC_AGENT_CALLBACK_T;
/* define the service mode of isc agent 
 */
#define ISC_AGENT_DIRECT_CALLBACK    0
#define ISC_AGENT_HIGH_SPEED_QUEUE   1
#define ISC_AGENT_LOW_SPEED_QUEUE    2
 
/* define all event that isc_agent task will receive
 */ 
#define ISC_AGENT_ENTER_TRANSITION_MODE      BIT_0      /* system enter transition mode event */
#define ISC_AGENT_PACKET_ARRIVAL             BIT_1      /* packet pass to isc_agent event     */
#define ISC_AGENT_TIME_EVENT                 BIT_2

/* DATA TYPE DECLARACTIONS
 */
typedef struct
{    
    ISC_Key_T           key;               /* 8-byte */
    L_MM_Mref_Handle_T  *mref_handle_p;    /* 4-byte */
    UI8_T               svc_id;            /* 1-byte */
    UI8_T               reserved[3];       /* padding to make ISC_AGENT_Msg_T size equal 16 bytes */
} ISC_AGENT_Msg_T; 

typedef struct
{
    UI32_T task_id;
    SYSFUN_MsgQ_T msgQ_id;
    BOOL_T is_transition_done;
} Task_Info_T;


typedef struct
{
    UI32_T  total;      
    UI32_T  high_speed_enqueue_success;
    UI32_T  high_speed_enqueue_fail;
    UI32_T  high_speed_dequeue_success;
    UI32_T  low_speed_enqueue_success;
    UI32_T  low_speed_enqueue_fail;    
    UI32_T  low_speed_dequeue_success;
}QueueTrafficInfo_T;

typedef struct
{
  UI32_T max_time;
  UI8_T svc_id;
}ISC_AGENT_BD_MAX_INFO_T;
/* LOCAL FUNCTION DECLARATIONS
 */
static BOOL_T   ISC_AGENT_IscService_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref, UI8_T svc_id);
static void     ISC_AGENT_HighSpeedTask(void);
static void     ISC_AGENT_LowSpeedTask(void);
 
/* backdoor functions
 */
static void     ISC_AGENT_BD_DisplayQueueStatus (void);
static void   ISC_AGENT_BD_BackDoorMenu(void);
static void ISC_AGENT_BD_SetHighSpeedMaxTime(UI32_T time,UI8_T sid);
static void ISC_AGENT_BD_SetLowSpeedMaxTime(UI32_T time,UI8_T sid);

/*template solution*/
extern SYS_TYPE_Stacking_Mode_T ISC_OM_GetOperatingMode(void);
 
/* STATIC VARIABLE DECLARACTIONS
 */
static Task_Info_T low_speed_task_info, high_speed_task_info;
/*The isc_agent should not has the isc information, but for the performance */
/*Must read:  keep the item order as the same as it in the  ISC_ServiceId_T, or here will have unexpected result*/
static ISC_AGENT_CALLBACK_T isc_agent_callback_register[ISC_SID_UNSUPPORTED] = {
            {ISC_AGENT_DIRECT_CALLBACK,NULL},/* ISC_RESERVED_SID*/
            {ISC_AGENT_DIRECT_CALLBACK,NULL},/* ISC_INTERNAL_SID*/
            {ISC_AGENT_DIRECT_CALLBACK,NULL},/* ISC_LAN_DIRECTCALL_SID*/
            {ISC_AGENT_HIGH_SPEED_QUEUE, LAN_CallByAgent_ISC_Handler},/* ISC_LAN_CALLBYAGENT_SID*/  
            {ISC_AGENT_DIRECT_CALLBACK,  NULL},/*ISC_AMTRDRV_DIRECTCALL_SID   */  
            {ISC_AGENT_HIGH_SPEED_QUEUE, AMTRDRV_MGR_CallByAgent_ISC_Handler},/* ISC_AMTRDRV_CALLBYAGENT_SID */  
            {ISC_AGENT_DIRECT_CALLBACK,  NULL},/* ISC_NMTRDRV_SID */  
            {ISC_AGENT_HIGH_SPEED_QUEUE,SWDRV_ISC_Handler}, /* ISC_SWDRV_SID */  
            {ISC_AGENT_HIGH_SPEED_QUEUE, LEDDRV_ISC_Handler},/* ISC_LEDDRV_SID*/  
            {ISC_AGENT_LOW_SPEED_QUEUE,  FS_ISC_Handler},/* ISC_FS_SID*/  
            {ISC_AGENT_HIGH_SPEED_QUEUE, SWDRVL4_ISC_Handler}, /* ISC_SWDRVL4_SID */ 
            {ISC_AGENT_HIGH_SPEED_QUEUE, SYSDRV_ISC_Handler},/* ISC_SYSDRV_SID  */  
            {ISC_AGENT_DIRECT_CALLBACK, NULL}, /* ISC_HRDRV_SID */
#if (SYS_CPNT_SWDRVL3 == TRUE)
            {ISC_AGENT_HIGH_SPEED_QUEUE, SWDRVL3_ISC_Handler},/* ISC_SWDRVL3_SID  */  
#else
            {ISC_AGENT_HIGH_SPEED_QUEUE, NULL},/* ISC_SWDRVL3_SID  */
#endif
            {ISC_AGENT_HIGH_SPEED_QUEUE,NULL},  /* ISC_SWDRV_CACHE_SID*/ 
            {ISC_AGENT_DIRECT_CALLBACK, NULL},/* ISC_CFGDB_SID*/
#if (SYS_CPNT_POE == TRUE) && (SYS_CPNT_STACKING == TRUE) && (SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK > 1)
            {ISC_AGENT_HIGH_SPEED_QUEUE, POEDRV_ISC_Handler},/* ISC_POEDRV_SID */                         
#else
            {ISC_AGENT_DIRECT_CALLBACK , NULL},/* ISC_POEDRV_SID*/ 
#endif
            {ISC_AGENT_DIRECT_CALLBACK, NULL},/* ISC_STK_TPLG_SID */  
            {ISC_AGENT_HIGH_SPEED_QUEUE,RULE_CTRL_ISC_Handler},/*ISC_RULE_CTRL_SID*/
#if(SYS_CPNT_L2MCAST == TRUE)
            {ISC_AGENT_HIGH_SPEED_QUEUE,msl_pmgr_recv_isc},/*ISC_MSL_SID*/
#endif
            {ISC_AGENT_HIGH_SPEED_QUEUE, SYS_TIME_ISC_Handler},
};
#define ISC_AGENT_CHECK_IS_X_SPEED(svc_id,x)  (isc_agent_callback_register[svc_id].isc_agent_service_mode == x)
#define ISC_AGENT_CHECK_ISNOT_X_SPEED(svc_id,x)  (isc_agent_callback_register[svc_id].isc_agent_service_mode != x)
#define ISC_AGENT_CHECK_IS_FUNCTION_NULL(svc_id)   (isc_agent_callback_register[svc_id].isc_agent_handler_func == NULL)
#define ISC_AGENT_CHECK_FUNCTION_CALL(svc_id,key,mref_handle_p)   (isc_agent_callback_register[svc_id].isc_agent_handler_func)(key,mref_handle_p)

static SYS_TYPE_Stacking_Mode_T  stacking_mode = SYS_TYPE_STACKING_TRANSITION_MODE;

/* for backdoor use
 */
static QueueTrafficInfo_T isc_agent_bd_queue_info;
/*add by fen.wang,debug iscagent,and it just called in driver process,so it cannot be share memory*/
static UI32_T isc_agent_high_bd_packet_input_max;
static UI32_T isc_agent_high_bd_packet_input_rate;
/*add by fen.wang,isc_agent_max_process_time[0] is for high speed task,isc_agent_max_process_time[1] 
  for low high speed task*/
static ISC_AGENT_BD_MAX_INFO_T isc_agent_max_process_time[2];

/* EXPORTED FUNCTION BODIES
 */

/* FUNCTION NAME : ISC_AGENT_Init
 * PURPOSE: 
 *          ISC_AGENT_Init is called to allocate some resource for ISC_AGENT
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *
 */
void ISC_AGENT_Init(void)
{
    memset(&isc_agent_bd_queue_info, 0, sizeof(QueueTrafficInfo_T));  
    isc_agent_high_bd_packet_input_max =0;
    isc_agent_high_bd_packet_input_rate = 0;
    memset(isc_agent_max_process_time,0,sizeof(isc_agent_max_process_time));
#if (SYS_CPNT_STACKING == TRUE) /*add by fen.wang*/
    SWDRV_Init();
#endif  /*SYS_CPNT_STACKING*/

    DEV_SWDRVL4_PMGR_InitiateProcessResource();
#if (SYS_CPNT_L2MCAST == TRUE)
    msl_pmgr_msgq_get();
#endif
}


 /* FUNCTION NAME : ISC_AGENT_Create_Tasks
 * PURPOSE: 
 *          ISC_AGENT_Create_Tasks is called to create tasks for ISC_AGENT
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *
 */ 
void ISC_AGENT_CreateTasks(void)
{
    if(SYSFUN_SpawnThread(SYS_BLD_ISC_AGENT_HIGH_SPEED_THREAD_PRIORITY,
                          SYS_BLD_ISC_AGENT_HIGH_SPEED_THREAD_SCHED_POLICY,
                          SYS_BLD_ISC_AGENT_HIGH_SPEED_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          ISC_AGENT_HighSpeedTask,
                          NULL,
                          &high_speed_task_info.task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }    

    if(SYSFUN_SpawnThread(SYS_BLD_ISC_AGENT_LOW_SPEED_THREAD_PRIORITY,
                          SYS_BLD_ISC_AGENT_LOW_SPEED_THREAD_SCHED_POLICY,
                          SYS_BLD_ISC_AGENT_LOW_SPEED_TASK,
                          SYS_BLD_TASK_COMMON_STACK_SIZE,
                          SYSFUN_TASK_NO_FP,
                          ISC_AGENT_LowSpeedTask,
                          NULL,
                          &low_speed_task_info.task_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

    return; 
}
    

/* FUNCTION NAME : ISC_AGENT_IscService_Callback
 * PURPOSE: 
 *          enqueue packet into corresponding queue in ISC_AGENT according to it's processing speed level
 * INPUT:   
 *          key     --  key for ISC service
 *          mem_ref --  the pointer to memroy reference of the sending packet
 *          svc_id  --  Service ID, an calling entiry must have a Service ID to identify itself
 * OUTPUT:  
 *          None
 * RETURN:  
 *          TRUE    -- enqueue successful
 *          FALSE   -- enqueue failed
 * NOTES:
 *          None
 */
static BOOL_T ISC_AGENT_IscService_Callback(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref, UI8_T svc_id)
{
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(ISC_AGENT_Msg_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;
    ISC_AGENT_Msg_T *isc_agent_msg_p = (ISC_AGENT_Msg_T *)msg_p->msg_buf;
    
    isc_agent_msg_p->key             = *key;   
    isc_agent_msg_p->mref_handle_p   = mem_ref;
    isc_agent_msg_p->svc_id          = svc_id;
	msg_p->msg_size=sizeof(ISC_AGENT_Msg_T);
#ifdef Driver_Group_Stacking_Mode_OK
    if(ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        L_MM_Mref_Release(&mem_ref);
		printf("[%s]%d %d\n",__FUNCTION__,__LINE__,SYS_TYPE_STACKING_TRANSITION_MODE);
        return FALSE;        
    }
    else
#endif
    {
        isc_agent_bd_queue_info.total++;
        
        if (ISC_AGENT_CHECK_IS_X_SPEED(svc_id,ISC_AGENT_HIGH_SPEED_QUEUE))
        {
            if (SYSFUN_SendRequestMsg(high_speed_task_info.msgQ_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) != SYSFUN_OK)
            {
                isc_agent_bd_queue_info.high_speed_enqueue_fail++;            
                L_MM_Mref_Release(&mem_ref);
                return FALSE;
            }
            else
            {  
                if (SYSFUN_SendEvent(high_speed_task_info.task_id, ISC_AGENT_PACKET_ARRIVAL) != SYSFUN_OK)
                {
                    SYSFUN_Debug_Printf("ISC_AGENT_IscService_Callback: send event to high speed task error!\n");
                }

                isc_agent_bd_queue_info.high_speed_enqueue_success++;
                return TRUE;
            }
        }
        else if(ISC_AGENT_CHECK_IS_X_SPEED(svc_id,ISC_AGENT_LOW_SPEED_QUEUE))
        {
                
            if (SYSFUN_SendRequestMsg(low_speed_task_info.msgQ_id, msg_p, SYSFUN_TIMEOUT_NOWAIT, 0, 0, NULL) != SYSFUN_OK)
            {
                isc_agent_bd_queue_info.low_speed_enqueue_fail++;
                L_MM_Mref_Release(&mem_ref);   
                return FALSE;  
            }
            else 
            {
                if (SYSFUN_SendEvent(low_speed_task_info.task_id, ISC_AGENT_PACKET_ARRIVAL) != SYSFUN_OK)
                {
                    SYSFUN_Debug_Printf("ISC_AGENT_IscService_Callback: send event to low speed task error!\n");
                }

                isc_agent_bd_queue_info.low_speed_enqueue_success++;
                return TRUE;
            }
        } 
        else
        {
            L_MM_Mref_Release(&mem_ref);
            return FALSE;   
        }
    }
}/* ISC_AGENT_IscService_Callback */ 


/* FUNCTION NAME : ISC_AGENT_EnterMasterMode
 * PURPOSE: 
 *          ISC_AGENT_EnterMasterMode is called to Enter Master Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_EnterMasterMode(void)
{
    stacking_mode = SYS_TYPE_STACKING_MASTER_MODE; 

    return;
}


/* FUNCTION NAME : ISC_AGENT_EnterSlaveMode
 * PURPOSE: 
 *          ISC_AGENT_EnterSlaveMode is called to Enter Slave Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_EnterSlaveMode(void)
{
    stacking_mode = SYS_TYPE_STACKING_SLAVE_MODE;       
       
    return;
} 
  

/* FUNCTION NAME : ISC_AGENT_EnterTransitionMode
 * PURPOSE: 
 *          ISC_AGENT_EnterTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_EnterTransitionMode(void)
{
#if 0 /* JinhuaWei, 07 August, 2008 9:49:19 */
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(ISC_AGENT_Msg_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;    
    ISC_AGENT_Msg_T *isc_agent_msg_p = msg_p->msg_buf;
#endif /* #if 0 */
#if 0 /*add by fen.wang,2008-6-25,for it is directly called by stkctrl_task,and the database of ISC is not shareMM,so it is uesless*/
    while (SYSFUN_ReceiveMsg(high_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
    {
        L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
        isc_agent_bd_queue_info.high_speed_dequeue_success++;
    }
    high_speed_task_info.is_transition_done = TRUE;

    while (SYSFUN_ReceiveMsg(low_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
    {
        L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
        isc_agent_bd_queue_info.low_speed_dequeue_success++;
    }
    low_speed_task_info.is_transition_done = TRUE;
#endif   
#if 0    
    SYSFUN_TASK_ENTER_TRANSITION_MODE(high_speed_task_info.is_transition_done &&
                                      low_speed_task_info.is_transition_done)    
#endif                                      
    return;
}


/* FUNCTION NAME : ISC_AGENT_SetTransitionMode
 * PURPOSE: 
 *          ISC_AGENT_SetTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_SetTransitionMode(void)
{
    stacking_mode = SYS_TYPE_STACKING_TRANSITION_MODE; 

#if 0
    if (SYSFUN_SendEvent(high_speed_task_info.task_id, ISC_AGENT_ENTER_TRANSITION_MODE) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("ISC_AGENT_SetTransitionMode: send event to high speed task error!\n");
    }

    if (SYSFUN_SendEvent(low_speed_task_info.task_id, ISC_AGENT_ENTER_TRANSITION_MODE) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("ISC_AGENT_SetTransitionMode: send event to low speed task error!\n");
    }
#endif    
  
    return;
}


/* FUNCTION NAME : ISC_AGENT_Create_InterCSC_Relation
 * PURPOSE: 
 *          This function initializes all function pointer registration operations.
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_Create_InterCSC_Relation(void)
{
    UI8_T i;
    
    for (i=0; i< ISC_SID_UNSUPPORTED; i++)
    {
        if (ISC_AGENT_CHECK_ISNOT_X_SPEED(i,ISC_AGENT_DIRECT_CALLBACK)){
           ISC_Register_Service_CallBack(i,ISC_AGENT_IscService_Callback); 
        }
    }
    
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("ISC_AG", 
                                                      SYS_BLD_DRIVER_GROUP_IPCMSGQ_KEY,
                                                      ISC_AGENT_BD_BackDoorMenu);
}


/* FUNCTION NAME : ISC_AGENT_HighSpeedTask
 * PURPOSE: 
 *          Receive message from high speed queue and then call corresponding csc's handler funtion
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
static void ISC_AGENT_HighSpeedTask(void)
{
    UI32_T wait_events, all_events = 0, rcv_event = 0;

    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(ISC_AGENT_Msg_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;    
    ISC_AGENT_Msg_T *isc_agent_msg_p = (ISC_AGENT_Msg_T *)msg_p->msg_buf;
    UI32_T start_time,end_time;
     #define ISC_AGENT_HIGHSPEEDTASK_TIMER_TICK 100

    if(SYSFUN_CreateMsgQ (SYSFUN_MSGQKEY_PRIVATE, SYSFUN_MSGQ_UNIDIRECTIONAL, &high_speed_task_info.msgQ_id)!= SYSFUN_OK)
    {
    	printf("%s: SYSFUN_CreateMsgQ fail.\n",__FUNCTION__);    
    }
    
    wait_events = ISC_AGENT_ENTER_TRANSITION_MODE | ISC_AGENT_PACKET_ARRIVAL|ISC_AGENT_TIME_EVENT;
    
    /* Engross CPU too long,modify by michael.wang,approved by wangfen 2008-10-14*/
    /*Isc_Agent_HighSpeedTask_timer_id = SYSFUN_PeriodicTimer_Create(); */
    /*SYSFUN_PeriodicTimer_Start(Isc_Agent_HighSpeedTask_timer_id, ISC_AGENT_HIGHSPEEDTASK_TIMER_TICK, ISC_AGENT_TIME_EVENT); */
    while (TRUE)
    {
        SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY,
                            (all_events!=0)?SYSFUN_TIMEOUT_NOWAIT : SYSFUN_TIMEOUT_WAIT_FOREVER, 
                            &rcv_event);
        all_events |= rcv_event;
        
        if (0 == all_events)
        {
            SYSFUN_Debug_Printf("DEV_NICDRV_Task: all_events == 0\n");
            continue;
        }
#ifdef Driver_Group_Stacking_Mode_OK

        if (ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if (all_events & ISC_AGENT_ENTER_TRANSITION_MODE)
            {
#if 1            
                while (SYSFUN_ReceiveMsg(high_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
                {
                    L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
                    isc_agent_bd_queue_info.high_speed_dequeue_success++;
                }                
                high_speed_task_info.is_transition_done = TRUE;
#endif                
                all_events ^= ISC_AGENT_ENTER_TRANSITION_MODE;
            }
        }
        else
#endif
        {
            start_time = SYSFUN_GetSysTick();
            if (all_events & ISC_AGENT_PACKET_ARRIVAL)
            {
                if(SYSFUN_ReceiveMsg(high_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
                {
                    /* call CSC's handler function*/
                    if (!(ISC_AGENT_CHECK_IS_FUNCTION_NULL(isc_agent_msg_p->svc_id))){
                        ISC_AGENT_CHECK_FUNCTION_CALL(isc_agent_msg_p->svc_id,&isc_agent_msg_p->key,isc_agent_msg_p->mref_handle_p);
                    }else{
                        printf("%s: isc_agent_handler_func[%u] is NULL\n", __FUNCTION__, isc_agent_msg_p->svc_id);
                        L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
                    }
                    isc_agent_bd_queue_info.high_speed_dequeue_success++;
                    isc_agent_high_bd_packet_input_rate++;
                    end_time = SYSFUN_GetSysTick()-start_time;
                    ISC_AGENT_BD_SetHighSpeedMaxTime(end_time,isc_agent_msg_p->svc_id);
                }else{
                    all_events ^= ISC_AGENT_PACKET_ARRIVAL;
                }
            }
            
			if (all_events & ISC_AGENT_TIME_EVENT)
			{	   
		
			    if(isc_agent_high_bd_packet_input_max<isc_agent_high_bd_packet_input_rate)
					 isc_agent_high_bd_packet_input_max =isc_agent_high_bd_packet_input_rate;
					 
				isc_agent_high_bd_packet_input_rate = 0; /* restart counter for every time event */			
				all_events ^= ISC_AGENT_TIME_EVENT;
			}
        }    
    }    
} /*ISC_AGENT_HighSpeedTask*/


/* FUNCTION NAME : ISC_AGENT_LowSpeedTask
 * PURPOSE: 
 *          Receive message from low speed queue and then call corresponding csc's handler funtion
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
static void ISC_AGENT_LowSpeedTask(void)
{
    UI32_T wait_events, all_events = 0, rcv_event = 0;
    
    UI8_T           msg_buf[SYSFUN_SIZE_OF_MSG(sizeof(ISC_AGENT_Msg_T))];
    SYSFUN_Msg_T    *msg_p = (SYSFUN_Msg_T *)msg_buf;    
    ISC_AGENT_Msg_T *isc_agent_msg_p =(ISC_AGENT_Msg_T *)(msg_p->msg_buf); 
    UI32_T start_time,end_time;

    if(SYSFUN_CreateMsgQ (SYSFUN_MSGQKEY_PRIVATE, SYSFUN_MSGQ_UNIDIRECTIONAL, &low_speed_task_info.msgQ_id)!= SYSFUN_OK)
    {
    	printf("%s: SYSFUN_CreateMsgQ fail.\n",__FUNCTION__);    
    }
    
    wait_events = ISC_AGENT_ENTER_TRANSITION_MODE | ISC_AGENT_PACKET_ARRIVAL;
    
    while (TRUE)
    {
        SYSFUN_ReceiveEvent(wait_events, SYSFUN_EVENT_WAIT_ANY,
                            (all_events!=0)?SYSFUN_TIMEOUT_NOWAIT : SYSFUN_TIMEOUT_WAIT_FOREVER, 
                            &rcv_event);
        all_events |= rcv_event;
        
        if (0 == all_events)
        {
            SYSFUN_Debug_Printf("DEV_NICDRV_Task: all_events == 0\n");
            continue;
        }
#ifdef Driver_Group_Stacking_Mode_OK
        if (ISC_OM_GetOperatingMode() == SYS_TYPE_STACKING_TRANSITION_MODE)
        {
            if (all_events & ISC_AGENT_ENTER_TRANSITION_MODE)
            {
#if 1            
                while (SYSFUN_ReceiveMsg(low_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
                {
                    L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
                    isc_agent_bd_queue_info.low_speed_dequeue_success++;
                }
                low_speed_task_info.is_transition_done = TRUE;
#endif                
                all_events ^= ISC_AGENT_ENTER_TRANSITION_MODE;
            }
        }
        else
#endif
        {
            if (all_events & ISC_AGENT_PACKET_ARRIVAL){
                 start_time = SYSFUN_GetSysTick();
                if (SYSFUN_ReceiveMsg(low_speed_task_info.msgQ_id, 0, SYSFUN_TIMEOUT_NOWAIT, sizeof(ISC_AGENT_Msg_T), msg_p)==SYSFUN_OK)
                {
                    /* call CSC's handler function*/
					if (!(ISC_AGENT_CHECK_IS_FUNCTION_NULL(isc_agent_msg_p->svc_id))){
                        ISC_AGENT_CHECK_FUNCTION_CALL(isc_agent_msg_p->svc_id,&isc_agent_msg_p->key, isc_agent_msg_p->mref_handle_p);
                    }else{
                        printf("%s: isc_agent_handler_func[%u] is NULL\n", __FUNCTION__, isc_agent_msg_p->svc_id);
                        L_MM_Mref_Release(&(isc_agent_msg_p->mref_handle_p));
                    }
                    isc_agent_bd_queue_info.low_speed_dequeue_success++;                
                    end_time = SYSFUN_GetSysTick()-start_time;
                    ISC_AGENT_BD_SetLowSpeedMaxTime(end_time,isc_agent_msg_p->svc_id);
                }else{
                    all_events ^= ISC_AGENT_PACKET_ARRIVAL;
                }
            }
        }    
    }    
}/*ISC_AGENT_LowSpeedTask*/


/* FUNCTION NAME : ISC_AGENT_BD_DisplayQueueStatus
 * PURPOSE:
 *      Display some queue process counters (success,fail,etc)
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void ISC_AGENT_BD_DisplayQueueStatus (void)
{
    printf("%-17s%-7s%-7s%-7s\r\n","","EnQ_S","EnQ_F","DeQ_S");
    printf("----------------------------------------\r\n");
    printf("%-17s%-7lu%-7lu%-7lu\r\n","HighSpeedQueue",
                                      isc_agent_bd_queue_info.high_speed_enqueue_success,
                                      isc_agent_bd_queue_info.high_speed_enqueue_fail,
                                      isc_agent_bd_queue_info.high_speed_dequeue_success);

    printf("%-17s%-7lu%-7lu%-7lu\r\n","LowSpeedQueue",
                                      isc_agent_bd_queue_info.low_speed_enqueue_success,
                                      isc_agent_bd_queue_info.low_speed_enqueue_fail,
                                      isc_agent_bd_queue_info.low_speed_dequeue_success);
    printf("----------------------------------------\r\n");
    printf("Enqueue Total = %lu/%lu\r\n",isc_agent_bd_queue_info.high_speed_enqueue_success +
                                         isc_agent_bd_queue_info.high_speed_enqueue_fail    +
                                         isc_agent_bd_queue_info.low_speed_enqueue_success  +
                                         isc_agent_bd_queue_info.low_speed_enqueue_fail,
                                         isc_agent_bd_queue_info.total);  
    printf("\r\n");
}

/* FUNCTION NAME : ISC_AGENT_BD_BackDoorMenu
 * PURPOSE:
 *      Display ISC back door available function and accept user seletion.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
static void ISC_AGENT_BD_SetHighSpeedMaxTime(UI32_T time,UI8_T sid)
{
   if(isc_agent_max_process_time[0].max_time<time)
   {
     isc_agent_max_process_time[0].max_time = time;
     isc_agent_max_process_time[0].svc_id = sid;
   }
}
static void ISC_AGENT_BD_SetLowSpeedMaxTime(UI32_T time,UI8_T sid)
{
    if(isc_agent_max_process_time[1].max_time<time)
    {
      isc_agent_max_process_time[1].max_time = time;
      isc_agent_max_process_time[1].svc_id = sid;
    }
}
static void ISC_AGENT_BD_GetMaxProcessTime()
{
 printf("\r\n============ISC_AGENT======================");
  if(isc_agent_max_process_time[0].max_time != 0)
  {
    printf("\r\n High speed task: time %lu, serverid %d",isc_agent_max_process_time[0].max_time,isc_agent_max_process_time[0].svc_id);
  }
    if(isc_agent_max_process_time[1].max_time != 0)
  {
    printf("\r\n Low speed task: time %lu, serverid %d",isc_agent_max_process_time[1].max_time,isc_agent_max_process_time[1].svc_id);
  }
  printf("\r\n==================================");
}

static void ISC_AGENT_BD_ClearMaxProcessTime()
{
  memset(isc_agent_max_process_time,0,sizeof(isc_agent_max_process_time));
}
static void ISC_AGENT_BD_BackDoorMenu (void)
{
    int ch;
    BOOL_T eof = FALSE;
    
    while (! eof)
    {
        printf("\n");
        printf("===============ISC AGENT BackDoor Menu================\n");
        printf(" 0. Exit\n");
        printf(" 1. Show High and Low Speed Queue status.\n");
        printf(" 2. Show SWDRV detail counter.\n");
        printf(" 3. Clear SWDRV counter.\n");
        printf(" 4. Show ISC Agent High Speed MAX counter per second.\n");
        printf(" 5. Clear SWDRV ISC Agent High Speed MAX counter per second.\n");
        printf(" 6. Show ISC_ANGENT and Swdrv process max tick.\n");
        printf(" 7. Clear 6 DB.\n");
        printf("======================================================\n");
        printf("    select = ");
        ch = getchar();

        printf ("%c\n", ch);

        switch (ch)
        {
            case '0' :
                eof = TRUE;
                break;  
            case '1':
                ISC_AGENT_BD_DisplayQueueStatus();
                break;
            case '2':
                SWDRV_BD_ShowRxCounter();
                break;
            case '3':
                SWDRV_BD_ClearRxCounter();
                break;
            case '4':
                printf("\r\n===max counter per sec is %ld",isc_agent_high_bd_packet_input_max);
                break;
            case '5':
                isc_agent_high_bd_packet_input_max = 0;
                break;
            case '6':
                ISC_AGENT_BD_GetMaxProcessTime();
                SWDRV_BD_DumpRxMaxTick();
                break;
            case '7':
                ISC_AGENT_BD_ClearMaxProcessTime();
                SWDRV_BD_ClearRxMaxTick();
                break;
                
            default:
                break;                   
        }
    }
}

