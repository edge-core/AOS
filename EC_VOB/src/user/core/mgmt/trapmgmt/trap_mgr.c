/* FILE NAME  -  TRAP_MGR.C
 *
 *  ABSTRACT :  This package provides the services to send out the SNMP TRAPs
 *              to all of the specified trap receivers.
 *
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch product lines.
 *
 *
 *
 *
 *  History
 *
 *   Anderson   12/30/2001      new created
 *   KingHong   07/16/2002      Add SNMPv2 version option for trap
 *   AmyTu      07/22/2002      Modify Trap architecture in order to allow
 *                              remote management blade to handle all
 *                              trap send process.
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */

#include <envoy/h/buffer.h>
#include <envoy/h/asn1conf.h>
#include <envoy/h/snmpstat.h>
#include <envoy/h/snmpdefs.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "l_mm.h"
#include "sys_bld.h"
#include "sys_adpt.h"
#include "leaf_1213.h"
#include "sys_dflt.h"
#include "mib2_mgr.h"
#include "trap_mgr.h"
#include "skt_vx.h"
#include "l_stdlib.h"
#include "socket.h"
#include "stktplg_mgr.h"
#include "stktplg_om.h"
#include "sys_mgr.h"
#include "netcfg.h"
#include "netif_om.h"
#include "syslog_mgr.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "snmp_mgr.h"
#include "trap1215.h"
#include "trap1493.h"
#include "trap1757.h"
#include "swctrl.h"
#include "vlan_mgr.h"
#include "leaf_es3626a.h"
#include "Backdoor_mgr.h"
/* Amy Add 8-02-2002*/
#include "trap_event.h"
#include "trap_util.h"
#include "netcfg_mgr.h"
#if (SYS_CPNT_VDSL == TRUE)
    //2003/8/28 ,phoebe add for ui64_t converting
    #include "l_stdlib.h"
#endif


/* TrapOID for power change trap is project dependent and the naming contant for each project
   should be defined in SYS_ADPT.H
 */
#include "trapEs3626a.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_EH == TRUE)
#include "eh_type.h"
#include "eh_mgr.h"
#endif//end of #if (SYS_CPNT_EH == TRUE)

/* NAMING CONSTANT DECLARATIONS
 */
#define TRAP_MGR_FUNCTION_NUMBER        1
#define TRAP_MGR_ERROR_NUMBER           1
#define TRAP_MGR_MAX_QUE_CNT            (SYS_ADPT_TOTAL_NBR_OF_LPORT * 2)
#define STA_UNSTABLED_STATE             0
#define STA_BECOME_STABLED_STATE        1
#define STA_STABLED_STATE               2
#define TRAP_MGR_EVENT_TRAP_ARRIVAL     BIT_0
#define TRAP_MGR_EVENT_ENTER_TRANSITION BIT_1
#define TRAP_MGR_EVENT_STA_STATE_CHANGED BIT_2
#define TRAP_MGR_EVENT_RIF_UP           BIT_3
#define TRAP_MGR_EVENT_RIF_DOWN           BIT_4
#define TRAP_MGR_NUMBER_OF_GENERIC_TRAP 10
#define TRAP_MGR_MAX_INDIVIDUAL_EVENT   32
#define TRAP_MGR_MAX_SEND_TRAP_RETRY_TIMES   10

/* TYPE DECLARATIONS
 */
enum
{
    TRAP_MGR_ALL_TRAP_ENABLE = 1,
    TRAP_MGR_CARD_TRAP_ENABLE,
    TRAP_MGR_SET_PORT_TRAP_ENABLE,
    TRAP_MGR_GET_PORT_TRAP_ENABLE,
    TRAP_MGR_ALRAM_SENSITIVITY_ENABLE,
    TRAP_MGR_ALL_ALARM_ENABLE,
    TRAP_MGR_SET_INDIVIDUAL_TRAP_ENABLE,
} TRAP_MGR_CHECKTYPE_E;


typedef struct TRAP_MGR_Octet_S
{
    UI8_T   *octet_P;
    I32_T   len;                /* original:int */

}TRAP_MGR_Octet_T;

typedef struct  TRAP_MGR_Lcb_S
{
    BOOL_T      init_flag;                  /* TRUE: TRAP_TASK initialized */
    UI32_T      trap_task_id;                 /* TRAP_TASK ID               */
}TRAP_MGR_Lcb_T;


typedef struct
{
    I32_T               que_elements_cnt;
    TRAP_EVENT_TrapQueueData_T     *front;
    TRAP_EVENT_TrapQueueData_T     *rear;
} TRAP_MGR_Queue_T;

/* for trace_id of user_id when allocate buffer with l_mm
 */
enum
{
    TRAP_TYPE_TRACE_ID_TRAP_MGR_MALLOC=0
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void TRAP_MGR_InitDefaultSetting(void);
static TRAP_EVENT_TrapQueueData_T *TRAP_MGR_QueueDequeue(TRAP_MGR_Queue_T *trap_queue);
static BOOL_T TRAP_MGR_QueueEnqueue (TRAP_EVENT_TrapQueueData_T *qData, TRAP_MGR_Queue_T *q);
static BOOL_T TRAP_MGR_SendTrap (TRAP_EVENT_TrapQueueData_T *trap_data);
static void TRAP_MGR_Task(void);
static void TRAP_MGR_SendColdStartTrap(void);
static void TRAP_MGR_SendWarmStartTrap(void);
static void TRAP_MGR_HandleTrapQueue(void);
static BOOL_T TRAP_MGR_CreateTrapSocket(void);
static BOOL_T TRAP_MGR_ProcessSendTrap(EBUFFER_T                  *ebuff,
                                       OCTET_T                    *local_ip,
#if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                       TRAP_MGR_TrapDestEntry_T   *trap_receiver,
#endif
                                       TRAP_EVENT_TrapQueueData_T *trap_data,
                                       UI32_T                     system_time);

static void TRAP_MGR_TrapLog(TRAP_EVENT_TrapData_T  *trap_data);
static  void   TRAP_MGR_BackdoorInfo_CallBack(void);
static void TRAP_MGR_Print_BackdoorHelp(void);
#if ((SYS_CPNT_SNMP_VERSION == 2) && (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE))
static BOOL_T TRAP_MGR_GetDefaultIpAddress(UI32_T *ip);
#endif//end of #if ((SYS_CPNT_SNMP_VERSION == 2) && (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE))
static void TRAP_MGR_RifUp_CallBack(UI32_T ip_address, UI32_T ip_mask);
static void TRAP_MGR_RifDown_CallBack(UI32_T ip_address, UI32_T ip_mask);
static void TRAP_MGR_ClearTrapQueue(void);
static TRAP_EVENT_TrapQueueData_T *TRAP_MGR_BuildQueueData(TRAP_EVENT_TrapData_T *trapData);
static void TRAP_MGR_free(void *ptr);
static void* TRAP_MGR_malloc(size_t size);

/* STATIC VARIABLE DECLARATIONS
 */
#if (SYS_CPNT_SNMP_VERSION == 2)
static TRAP_MGR_TrapDestEntry_T     trap_mgr_trap_receiver[SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER];
#endif//end of #if (SYS_CPNT_SNMP_VERSION == 2)
static TRAP_MGR_Queue_T             trap_queue;
static TRAP_MGR_Lcb_T               trap_mgr_lcb;
static BOOL_T                       flush_trap_queue_done_flag;
static BOOL_T                       spanning_tree_state;
static int                          trap_socket=-1;
static UI32_T                       trap_mgr_sem_id;
static UI8_T                        trap_mgr_snmp_enable_authen_traps;
static UI8_T                        trap_mgr_link_up_down_traps;
static BOOL_T                       debug_mode=FALSE,EHMsgEnable=FALSE,not_check_rif_cnt=FALSE;
static UI32_T                       up_rif_cnt=0;
static UI32_T                       memory_used=0;//max:257552
static UI32_T                       trapDataInQCnt=0;//max:524
SYSFUN_DECLARE_CSC



/* EXPORTED SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_Init
 * ---------------------------------------------------------------------
 *  FUNCTION: Initialize the trap manager.
 *
 *  INPUT    : NONE.
 *  OUTPUT   : NONE.
 *  RETURN   : NONE.
 *  NOTE     : This routine should be called before Trap_Mgr_CreateTask.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_Init(void)
{

   /* BODY */
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
    if (SYSFUN_CreateSem (1, SYSFUN_SEM_FIFO, &trap_mgr_sem_id) != SYSFUN_OK)
    {
        if (debug_mode)
    		printf("FUNCTION:TRAP_MGR_Init; DESC:CreateSem failure\n");
#if (SYS_CPNT_EH == TRUE)
        if (EHMsgEnable)
        {
            EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
            printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        }
        return;
#endif
    }
    trap_mgr_lcb.init_flag = TRUE;
    spanning_tree_state = STA_UNSTABLED_STATE;

    /* Initialize the trap queue
     */
    trap_queue.front = (TRAP_EVENT_TrapQueueData_T *)NULL;
    trap_queue.rear  = (TRAP_EVENT_TrapQueueData_T *)NULL;
    trap_queue.que_elements_cnt = 0;
} /* End of TRAP_MGR_Init() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRAP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void TRAP_MGR_Create_InterCSC_Relation(void)
{
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("trap",TRAP_MGR_BackdoorInfo_CallBack);
    NETCFG_MGR_RegisterRifUp_CallBack(TRAP_MGR_RifUp_CallBack);
    NETCFG_MGR_RegisterRifDown_CallBack(TRAP_MGR_RifDown_CallBack);
} /* end of TRAP_MGR_Create_InterCSC_Relation */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_RifUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: this is a callBack function, for Rif to Notify trapmgmt when
 *          Rif get IP sucessfully
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
 static void TRAP_MGR_RifUp_CallBack(UI32_T ip_address, UI32_T ip_mask)
{
    if (debug_mode)
        printf("TRAP_MGR_RifUp_CallBack..\n");
    if(SYSFUN_SendEvent(trap_mgr_lcb.trap_task_id, TRAP_MGR_EVENT_RIF_UP)!= SYSFUN_OK)
    {
        if (debug_mode)
            printf("TRAP_MGR_RifUp_CallBack:SYSFUN_SendEvent return false\n");
    }
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_RifDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: this is a callBack function, for Rif to Notify trapmgmt when
 *          Rif get IP sucessfully
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
 static void TRAP_MGR_RifDown_CallBack(UI32_T ip_address, UI32_T ip_mask)
{
    if (debug_mode)
        printf("TRAP_MGR_RifDown_CallBack..\n");
    if(SYSFUN_SendEvent(trap_mgr_lcb.trap_task_id, TRAP_MGR_EVENT_RIF_DOWN)!= SYSFUN_OK)
    {
        if (debug_mode)
            printf("TRAP_MGR_RifDown_CallBack:SYSFUN_SendEvent return false\n");
    }
}
/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_BackdoorInfo_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
static  void   TRAP_MGR_BackdoorInfo_CallBack(void)
{
    UI8_T ch;
    BOOL_T  backdoor_continue ;

    printf("\n Trap Backdoor Selection");

    backdoor_continue = TRUE;

    while(backdoor_continue)
    {
        TRAP_MGR_Print_BackdoorHelp();

        ch = getchar();
        switch(ch)
        {
            case '1':
                debug_mode=TRUE;
                printf("\n Debug mode is enabled\n");
                break;
            case '2':
                debug_mode=FALSE;
                printf("\n Debug mode is disable\n");
                break;
            case '3':    	              	
            	EHMsgEnable=TRUE;
            	printf("\n EH Message is enabled\n");      	
            	break;
            case '4':    	              	
            	EHMsgEnable=FALSE;
            	printf("\n EH Message is disable\n");      	
            	break;
			case '5':    	              	
            	printf("\n Total memory used=[%lu], TrapDataCnt=[%lu]\n", memory_used, trapDataInQCnt);                    	
            	break;            	
            case '6':
                printf("\n up_rif_cnt=[%lu]\n", up_rif_cnt);                    	
            	break;                             	
            case '7':
            	not_check_rif_cnt=TRUE;
            	break;
            case '8':
            	not_check_rif_cnt=FALSE;
            	break;
            case 'Q':
            case 'q':
                backdoor_continue = FALSE;
                break;
            default:
                printf("\n Invalid command\n");
                break;
        }
    }

    return;
}/* End of TRAP_MGR_BackdoorInfo_CallBack() */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_Print_BackdoorHelp
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
static void TRAP_MGR_Print_BackdoorHelp(void)
{
    printf("\n\tEnable Debug Mode:1");
    printf("\n\tDisable Debug Mode:2");
    printf("\n\tEnable EH Message:3");
    printf("\n\tDisable EH Message:4");
    printf("\n\tCheck Trap Queue:5");
    printf("\n\tCheck up_rif_cnt:6");
    printf("\n\tDisable rif cnt checking:7");
    printf("\n\tEnable rif cnt checking:8");
    printf("\n\tQuit:Q\n");
     return;
}

#if ((SYS_CPNT_SNMP_VERSION == 2) && (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE))
static BOOL_T TRAP_MGR_GetDefaultIpAddress(UI32_T *ip)
{
    UI32_T defaultIpList[5];
    UI32_T i,j,okFlag=1;
    defaultIpList[0]=10*(256*256*256) + 1*(256*256) + 0*256 + 1;//10.1.0.1
    defaultIpList[1]=10*(256*256*256) + 1*(256*256) + 0*256 + 2;//10.1.0.2
    defaultIpList[2]=10*(256*256*256) + 1*(256*256) + 0*256 + 3;//10.1.0.3
    defaultIpList[3]=10*(256*256*256) + 1*(256*256) + 0*256 + 4;//10.1.0.4
    defaultIpList[4]=10*(256*256*256) + 1*(256*256) + 0*256 + 5;//10.1.0.5
    for (i=0; i<=4; i++)
    {
         okFlag=1;
         for (j=0; j<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; j++)
         {
            if (trap_mgr_trap_receiver[j].trap_dest_status==VAL_trapDestStatus_2_valid)
            {
                if (defaultIpList[i]==trap_mgr_trap_receiver[j].trap_dest_address)
                {
                    okFlag=0;
                    break;
                }
            }
         }
         if (okFlag)
            break;
    }
    if (okFlag)
    {
        *ip=defaultIpList[i];
        return TRUE;
    }
    else
        return FALSE;
}
#endif//end of #if ((SYS_CPNT_SNMP_VERSION == 2) && (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE))

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the trap enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_EnterMasterMode()
{
    TRAP_MGR_InitDefaultSetting();
    SYSFUN_ENTER_MASTER_MODE();
} /* end of TRAP_MGR_EnterMasterMode() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the trap enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_EnterSlaveMode()
{
    SYSFUN_ENTER_SLAVE_MODE();
} /* end of TRAP_MGR_EnterSlaveMode() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the Trap enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_EnterTransitionMode()
{
    return;
} /* end of TRAP_MGR_EnterTransitionMode() */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE:
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_SetTransitionMode()
{
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)

    //1. Set mode to transition mode, prevent new traps comes in
    SYSFUN_SET_TRANSITION_MODE();

    //2. wait all csc leave
    SYSFUN_ENTER_TRANSITION_MODE();

    //3. init flushqueue flag as false
    flush_trap_queue_done_flag=FALSE;

    //4. send tcn event to traptask to flush trap queue
    if (SYSFUN_SendEvent (trap_mgr_lcb.trap_task_id, TRAP_MGR_EVENT_ENTER_TRANSITION)!=SYSFUN_OK)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTransitionMode:SYSFUN_SendEvent return false\n");
#if (SYS_CPNT_EH == TRUE)
        if (EHMsgEnable)
        {
            EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
            printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        }
#endif//end of #if (SYS_CPNT_EH == TRUE)
    }

    //5. raise trap priority to ensure traptask can wake up to receive the tcn event and flush trap queue successfully.
    if (SYSFUN_SetTaskPriority(trap_mgr_lcb.trap_task_id, SYS_BLD_RAISE_TO_HIGH_PRIORITY)!=SYSFUN_OK)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTransitionMode:SYSFUN_SetTaskPriority(SYS_BLD_RAISE_TO_HIGH_PRIORITY) return false\n");
    }

    //6. wait all trap done
    SYSFUN_TASK_ENTER_TRANSITION_MODE(flush_trap_queue_done_flag);

    //7. restore trap priority
    if (SYSFUN_SetTaskPriority(trap_mgr_lcb.trap_task_id, SYS_BLD_TRAPMGR_TASK_PRIORITY)!=SYSFUN_OK)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTransitionMode:SYSFUN_SetTaskPriority(SYS_BLD_TRAPMGR_TASK_PRIORITY) return false\n");
    }
} /* end of TRAP_MGR_SetTransitionMode() */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - TRAP_MGR_Make_Oid_From_Dot_String
 * -------------------------------------------------------------------------
 * FUNCTION: This function will make object ID from given text string
 * INPUT   : text_p - Pointer to text string
 * OUTPUT  : oid_P  - Pointer to object ID
 * RETURN  : BOOL_T Status   - TRUE   : Convert successfully
 *                             FALSE  : Convert failed
 * Note:     oid_P : buffer allocated by caller.
 * -------------------------------------------------------------------------*/
BOOL_T TRAP_MGR_Make_Oid_From_Dot_String(UI32_T *oid_P, I8_T *text_p)
{
   I8_T    *tp;
   UI32_T  no;

   if ( oid_P == NULL )
      return FALSE;

   for ( tp=text_p, no = 0; ;tp++)
   {
      if ( *tp == '.' || *tp==0)
      {
         oid_P[no++] =  atoi(text_p);
         if ( *tp==0 ) break;
         text_p= tp+1;
      }
   }

   return TRUE;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_CreateTask
 * ---------------------------------------------------------------------
 *  FUNCTION: Create and start trap manager task
 *
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_CreateTask(void)
{
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;

   /* BODY */
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
    if (SYSFUN_SpawnTask(SYS_BLD_TRAPMGR_TASK,
                         SYS_BLD_TRAPMGR_TASK_PRIORITY,
                         SYS_BLD_TASK_LARGE_STACK_SIZE,
                         0,
                         TRAP_MGR_Task,
                         0,
                         &trap_mgr_lcb.trap_task_id) != SYSFUN_OK )
    {
#if (SYS_CPNT_EH == TRUE)
        if (EHMsgEnable)
        {
            EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
            printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        }
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_CreateTask; DESC:Create task failure\n");

        trap_mgr_lcb.trap_task_id = 0;
        owner_info.level = SYSLOG_LEVEL_CRIT;
        owner_info.module_no = SYS_MODULE_TRAPMGMT;
        owner_info.function_no = TRAP_MGR_FUNCTION_NUMBER;
        owner_info.error_no = TRAP_MGR_ERROR_NUMBER;
        SYSLOG_MGR_AddFormatMsgEntry(&owner_info, CREATE_TASK_FAIL_MESSAGE_INDEX, "TRAP_MGR_CreateTask", 0, 0);
    } /* End of if */

} /* End of TRAP_MGR_CreateTask() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ProvisionComplete
 * ---------------------------------------------------------------------
 *  FUNCTION: Cold start trap must wait until provision complete before
 *            send request can be process properly.
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_ProvisionComplete(void)
{
    UI32_T boot_reason;
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
    UI32_T my_unit_id;


    if (debug_mode)
        printf("TRAP_MGR_ProvisionComplete...\n");

    if(!STKTPLG_MGR_GetMyUnitID(&my_unit_id))
    {
        printf("TRAP_MGR_ProvisionComplete:STKTPLG_MGR_GetMyUnitID failure\n");
        return;
    }
    if(!STKTPLG_MGR_GetUnitBootReason(my_unit_id, &boot_reason))
    {
        printf("TRAP_MGR_ProvisionComplete:STKTPLG_MGR_GetUnitBootReason failure\n");
#if (SYS_CPNT_EH == TRUE)
        if (EHMsgEnable)
        {
            EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
            printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        }
#endif//end of #if (SYS_CPNT_EH == TRUE)
        return;
    }
    switch (boot_reason)
    {
        case STKTPLG_OM_REBOOT_FROM_COLDSTART:
            TRAP_MGR_SendColdStartTrap();
            break;
        case STKTPLG_OM_REBOOT_FROM_WARMSTART:
            TRAP_MGR_SendWarmStartTrap();
            break;
        default:
            printf("TRAP_MGR_ProvisionComplete:STKTPLG_OM_SYSTEM_REBOOT_FROM_RUNTIME_PROVISION...\n");
            break;
    }
    return;

} /* end of TRAP_MGR_ProvisionComplete() */


/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_NotifyStaTplgChanged
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that network topology is changed due to the
 *          STA enabled.
 * Parameter:
 * Return: None.
 * Note: When STA enabled, all the ports will go through STA algorithm to
 *       determine its operation state. During this period, the trap management
 *       shall wait until STA becomes stable. Otherwise, the trap message
 *       will be lost if the port is not in forwarding state.
 * -----------------------------------------------------------------------------
 */
void TRAP_MGR_NotifyStaTplgChanged (void)
{
   /* BODY */
#if (SYS_CPNT_SNMP_VERSION == 3)
        SNMP_MGR_NotifyStaTplgChanged();
#else
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();//stacking
    if (debug_mode)
         printf("TRAP_MGR_NotifyStaTplgChanged..\n");
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_NotifyStaTplgChanged; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return;
    }
   spanning_tree_state = STA_UNSTABLED_STATE;
   SYSFUN_RELEASE_CSC();
#endif
} /* End of TRAP_MGR_NotifyStaTplgChanged() */



/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_NotifyStaTplgStabled
 * -----------------------------------------------------------------------------
 * Purpose: This procedure notify that STA has been enabled, and at least one of the port enters
 *          forwarding state. The network topology shall be stabled after couple seconds.
 * Parameter:
 * Return: None.
 * Note: This notification only informs that at least one of STA port enters forwarding state.
 *       To make sure all the STA ports enters stable state, we shall wait for few more seconds
 *       before we can send trap messages.
 * -----------------------------------------------------------------------------
 */
void TRAP_MGR_NotifyStaTplgStabled (void)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
        SNMP_MGR_NotifyStaTplgStabled();
#else
   /* BODY */
    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_NotifyStaTplgStabled; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return;
    }
   if (spanning_tree_state == STA_UNSTABLED_STATE)
   {
      if (debug_mode)
            printf("FUNCTION:TRAP_MGR_NotifyStaTplgStabled; DESC:Send event TRAP_MGR_EVENT_STA_STATE_CHANGED\n");
      spanning_tree_state = STA_BECOME_STABLED_STATE;
      SYSFUN_SendEvent (trap_mgr_lcb.trap_task_id, TRAP_MGR_EVENT_STA_STATE_CHANGED);
   }
   SYSFUN_RELEASE_CSC();
   /* End of if */
#endif
} /* End of trap_Notify_Sta_State() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ReqSendTrap
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : Variable's instance and value that should be bound in
 *             trap PDU.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before TRAP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */

void TRAP_MGR_ReqSendTrap(TRAP_EVENT_TrapData_T *trap_data_p)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    SNMP_MGR_ReqSendTrap( trap_data_p);
#else
	/* BODY */
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
	TRAP_EVENT_TrapQueueData_T                     *new_blk;
    if (debug_mode)
        printf("FUNCTION:TRAP_MGR_ReqSendTrap; DESC:Receive trap request, TrapType=[%lu]\n",trap_data_p->trap_type );

    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_ReqSendTrap:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return;
    }
	TRAP_MGR_TrapLog(trap_data_p);
	
    if ((trap_data_p->trap_type == TRAP_EVENT_LINK_UP) || (trap_data_p->trap_type == TRAP_EVENT_LINK_DOWN))
    {
        if (trap_mgr_link_up_down_traps == TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED)
        {
            if (debug_mode)
                printf("TRAP_MGR_ReqSendTrap:link up down trap disable, log only\n");
            SYSFUN_RELEASE_CSC();
            return;
        }
    } /* end of if */

    if (trap_data_p->trap_type == TRAP_EVENT_AUTHENTICATION_FAILURE)
    {
        if (trap_mgr_snmp_enable_authen_traps == VAL_snmpEnableAuthenTraps_disabled)
        {
            if (debug_mode)
                printf("TRAP_MGR_ReqSendTrap:authentication trap disable, log only\n");
            SYSFUN_RELEASE_CSC();
            return;
        }
    } /* end of if */

    trap_data_p->remainRetryTimes=TRAP_MGR_MAX_SEND_TRAP_RETRY_TIMES;
    MIB2_MGR_GetSysUpTime(&trap_data_p->trap_time);

    new_blk=TRAP_MGR_BuildQueueData(trap_data_p);
    if (!TRAP_MGR_QueueEnqueue(new_blk, &trap_queue))
    {
#if (SYS_CPNT_EH == TRUE)
        if (EHMsgEnable)
        {
            EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
            printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
        }
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
                printf("TRAP_MGR_ReqSendTrap:add queue failure\n");
    }
    SYSFUN_RELEASE_CSC();
    return;
#endif/*end of #if (SYS_CPNT_SNMP_VERSION == 3)*/
} /* End of TRAP_MGR_ReqSendTrap() */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ReqSendTrapOptional
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.
 *
 *  INPUT    : 1. trap_data_p: Variable's instance and value that should be bound in
 *                                                    trap PDU.
 *             2. flag    1:send trap and log; 0:log only, don't send trap
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : This procedure shall not be invoked before TRAP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_ReqSendTrapOptional(TRAP_EVENT_TrapData_T *trap_data_p, TRAP_EVENT_SendTrapOption_E flag)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
    SNMP_MGR_ReqSendTrapOptional( trap_data_p, flag);
#else
    TRAP_EVENT_TrapQueueData_T                     *new_blk;
    /* BODY */
    if (debug_mode)
        printf("TRAP_MGR_ReqSendTrapOptional:Receive trap request, TrapType=[%lu]\n",trap_data_p->trap_type );

    SYSFUN_USE_CSC_WITHOUT_RETURN_VALUE();//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_ReqSendTrapOptional:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return;
    }

    if(flag == TRAP_EVENT_SEND_TRAP_OPTION_LOG_AND_TRAP || flag == TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY)
    {
        TRAP_MGR_TrapLog(trap_data_p);
    }

    trap_data_p->flag = flag;
    if (flag==TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY)
    {
        if (debug_mode)
            printf("TRAP_MGR_ReqSendTrapOptional:TRAP_EVENT_SEND_TRAP_OPTION_LOG_ONLY\n");
        SYSFUN_RELEASE_CSC();
        return;
    }


    if ((trap_data_p->trap_type == TRAP_EVENT_LINK_UP) || (trap_data_p->trap_type == TRAP_EVENT_LINK_DOWN))
    {
        if (trap_mgr_link_up_down_traps == TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED)
        {
            if (debug_mode)
                printf("TRAP_MGR_ReqSendTrapOptional:link up down trap disable, log only\n");
            SYSFUN_RELEASE_CSC();
            return;
        }
    } /* end of if */

    if (trap_data_p->trap_type == TRAP_EVENT_AUTHENTICATION_FAILURE)
    {
        if (trap_mgr_snmp_enable_authen_traps == VAL_snmpEnableAuthenTraps_disabled)
        {
            if (debug_mode)
                printf("TRAP_MGR_ReqSendTrapOptional:authentication trap disable, log only\n");
            SYSFUN_RELEASE_CSC();
            return;
        }
    } /* end of if */

    trap_data_p->remainRetryTimes=TRAP_MGR_MAX_SEND_TRAP_RETRY_TIMES;
    MIB2_MGR_GetSysUpTime(&trap_data_p->trap_time);
    new_blk=TRAP_MGR_BuildQueueData(trap_data_p);
    if (!TRAP_MGR_QueueEnqueue(new_blk, &trap_queue))
    {
        if (debug_mode)
                printf("TRAP_MGR_ReqSendTrapOptional:add queue failure\n");
    }
    SYSFUN_RELEASE_CSC();
    return;
#endif/*end of #if (SYS_CPNT_SNMP_VERSION == 3)*/
} /* End of TRAP_MGR_ReqSendTrapOptional() */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_GetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 *  FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           is successfully retrieved.  Otherwise, return false.
 * INPUT   : None.
 * OUTPUT  : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
        SNMP_MGR_GetSnmpEnableAuthenTraps(snmp_enable_authen_traps);
#else
    SYSFUN_USE_CSC(FALSE);//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetSnmpEnableAuthenTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    *snmp_enable_authen_traps = trap_mgr_snmp_enable_authen_traps;
    SYSFUN_RELEASE_CSC();
    return TRUE;
#endif
} /* end of TRAP_MGR_GetSnmpEnableAuthenTraps() */


/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SetSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the permission for SNMP process to generate trap
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled /
 *                                      VAL_snmpEnableAuthenTraps_disabled
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetSnmpEnableAuthenTraps(UI8_T snmp_enable_authen_traps)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
        SNMP_MGR_SetSnmpEnableAuthenTraps();
#else
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetSnmpEnableAuthenTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    trap_mgr_snmp_enable_authen_traps = snmp_enable_authen_traps;
    SYSFUN_RELEASE_CSC();
    return TRUE;
#endif
} /* end of TRAP_MGR_SetSnmpEnableAuthenTraps() */




/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_GetLinkUpDownTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if the link up down trap of the device
 *           can be successfully configured.  Otherwise, return false.
 * OUTPUT  : link_up_down_trap - TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED /
 *                               TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED
 * INPUT   : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetLinkUpDownTraps(UI8_T *link_up_down_traps)
{
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetLinkUpDownTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    *link_up_down_traps = trap_mgr_link_up_down_traps;
    SYSFUN_RELEASE_CSC();
    return TRUE;

} /* end of TRAP_MGR_GetLinkUpDownTraps() */



/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SetLinkUpDownTraps
 * ---------------------------------------------------------------------
 * FUNCTION: This function returns true if thelink up down trap of the device
 *           can be successfully configured.  Otherwise, return false.
 * INPUT   : link_up_down_trap - TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED /
 *                               TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED
 * OUTPUT  : None.
 * RETURN  : TRUE / FALSE
 * NOTE    : It is strongly recommended that this object be stored in non-volatile memory so
 *           that it remains constant between re-initializations of the network management system.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetLinkUpDownTraps(UI8_T link_up_down_traps)
{
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetLinkUpDownTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if ((link_up_down_traps != TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED) &&
        (link_up_down_traps != TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED))
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T msg[256];
        sprintf(msg, "link_up_down_traps=[%d]",link_up_down_traps);
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetLinkUpDownTraps; DESC:Reject, invalid parameter, link_up_down_traps=[%d]\n", link_up_down_traps);
        SYSFUN_RELEASE_CSC();
        return FALSE;
     }
    trap_mgr_link_up_down_traps = link_up_down_traps;
    SYSFUN_RELEASE_CSC();
    return TRUE;


} /* end of TRAP_MGR_SetLinkUpDownTraps() */

#if (SYS_CPNT_SNMP_VERSION == 2)
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified trap receiver
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: trap_receiver->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: trap_receiver            - trap receiver info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The trap receiver can only be accessed by CLI and Web.
 *           SNMP management station CAN NOT access the trap receiver.
 *        2. There is no MIB to define the trap receiver.
 *        3. Status of each trap receiver is defined as following:
 *              - invalid(0): the entry of this trap receiver is deleted/purged
 *              - valid(1): this trap receiver is enabled
 *
 *           Set status to invalid(0) will delete/purge a trap receiver.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *        4. The total number of trap receivers supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *        5. By default, there is no trap receiver configued in the system.
 *        6. For any trap event raised by any subsystem, a SNMP Trap shall be
 *           sent to all of the enabled trap receivers.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver)
{
    UI32_T  i;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetTrapReceiver; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
            continue;

        if (trap_receiver->trap_dest_address == trap_mgr_trap_receiver[i].trap_dest_address)
            break;

    }

    if (i == SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_GET,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 ":trap receiver not found");
#endif
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    *trap_receiver = trap_mgr_trap_receiver[i];

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;

} /* end of TRAP_MGR_GetTrapReceiver() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetNextTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available trap receiver
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: trap_receiver->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: trap_receiver            - next available trap receiver info
 * RETURN: TRUE/FALSE
 * NOTES: This function will return trap receiver from the smallest ip addr.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetNextTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver)
{
    TRAP_MGR_TrapDestEntry_T        tmp_entry;
    UI32_T                          i;
    BOOL_T                          found=FALSE;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetNextTrapReceiver; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }


    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
            continue;

        if (trap_mgr_trap_receiver[i].trap_dest_address <= trap_receiver->trap_dest_address)
            continue;

        if (found == FALSE)
        {
            found = TRUE;
            tmp_entry = trap_mgr_trap_receiver[i];
            continue;
        }

        if ((trap_mgr_trap_receiver[i].trap_dest_address < tmp_entry.trap_dest_address) &&
            (trap_mgr_trap_receiver[i].trap_dest_address > trap_receiver->trap_dest_address))
        {
            tmp_entry = trap_mgr_trap_receiver[i];
        }

    } /* end of for */


    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);

    if (found == TRUE)
    {
        *trap_receiver = tmp_entry;
        SYSFUN_RELEASE_CSC();
        return TRUE;
    }
#if (SYS_CPNT_EH == TRUE)
    EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_GET,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 ":trap receiver not found");
#endif
    SYSFUN_RELEASE_CSC();
    return FALSE;

} /* end of TRAP_MGR_GetNextTrapReceiver() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetNextRunningTrapReceiver
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          next available non-default trap receiver can be retrieved
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: trap_receiver->trap_dest_address    - (key) to specify a unique trap receiver
 * OUTPUT: trap_receiver            - next available non-default trap receiver info
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default trap receiver.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T TRAP_MGR_GetNextRunningTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver)
{
    TRAP_MGR_TrapDestEntry_T     tmp_entry;
    UI32_T                      i;
    BOOL_T                      found=FALSE;

    /* BODY */
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetNextRunningTrapReceiver; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_SNMP_COMMUNITY_STRING; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
            continue;

        if (trap_mgr_trap_receiver[i].trap_dest_address <= trap_receiver->trap_dest_address)
            continue;

        if (found == FALSE)
        {
            found = TRUE;
            tmp_entry = trap_mgr_trap_receiver[i];
            continue;
        }

        if ((trap_mgr_trap_receiver[i].trap_dest_address < tmp_entry.trap_dest_address) &&
            (trap_mgr_trap_receiver[i].trap_dest_address > trap_receiver->trap_dest_address))
        {
            tmp_entry = trap_mgr_trap_receiver[i];
        }
    }


    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);

    if (found == TRUE)
    {
        *trap_receiver = tmp_entry;
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    SYSFUN_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_FAIL;


} /* end of TRAP_MGR_GetNextRunningTrapReceiver() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverCommStringName
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the community string name can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_trap_dest_address - (key) to specify a unique trap receiver
 *        trap_dest_community      - the SNMP community string for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified trap_dest_address does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           status of this new trap receiver will be set to disabled(2)
 *           by default.
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverCommStringName(UI32_T   trap_receiver_trap_dest_address,
                                              UI8_T    *comm_string_name)
{
    UI32_T  i, j=0;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetTrapReceiverCommStringName; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (strlen(comm_string_name) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 "community too long");
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetTrapReceiverCommStringName; DESC:comm string too long, commstr=[%s]\n",comm_string_name);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
        {
            /* record the first empty entry.  When the entry is not existed, this
             * entry will be updated as the input.
             */
            if (available_entry == -1)
                available_entry = i;
            continue;
        }

        if (trap_receiver_trap_dest_address == trap_mgr_trap_receiver[i].trap_dest_address)
            break;
    }


    if (i == SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER)
    {
        /* No found, create a new one, if no more available entry then return FALSE
         */
        if (available_entry != -1)
        {
            strcpy(trap_mgr_trap_receiver[available_entry].trap_dest_community, comm_string_name);

            /* Append the null character for the rest of characters which are not used
             */
            for (i=strlen(comm_string_name); i<=SYS_ADPT_MAX_COMM_STR_NAME_LEN; i++)
                trap_mgr_trap_receiver[available_entry].trap_dest_community[i] = 0;

            trap_mgr_trap_receiver[available_entry].trap_dest_address = trap_receiver_trap_dest_address;
            return_val = TRUE;
        }
        else
        {
            /* no available entry, can not create new entry
             */
#if (SYS_CPNT_EH == TRUE)
             EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_ERR,
                                 ":no available entry");
#endif//end of #if (SYS_CPNT_EH == TRUE)
             if (debug_mode)
                printf("FUNCTION:TRAP_MGR_SetTrapReceiverCommStringName; DESC:no available entry, can not create new entry\n");
            return_val = FALSE;
        }
    }
    else
    {
        /* found an entry, update status.  By logically, it should be deleted.
         */
        strcpy(trap_mgr_trap_receiver[i].trap_dest_community, comm_string_name);

        /* Append the null character for the rest of characters which are not used
         */
        for (j=strlen(comm_string_name); j<=SYS_ADPT_MAX_COMM_STR_NAME_LEN; j++)
            trap_mgr_trap_receiver[i].trap_dest_community[j] = 0;

        return_val = TRUE;
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return return_val;
} /* end of TRAP_MGR_SetTrapReceiverCommStringName() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverStatus
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the status can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        status                - the status for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverStatus(UI32_T    trap_receiver_ip_addr,
                                      UI32_T    status)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetTrapReceiverStatus; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if ((status != VAL_trapDestStatus_2_valid) &&
        (status != VAL_trapDestStatus_2_invalid) )
    {
#if (SYS_CPNT_EH == TRUE)
        UI8_T msg[256];
        sprintf(msg, "status=[%lu]",status);
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
                printf("FUNCTION:TRAP_MGR_SetTrapReceiverStatus; DESC:invalid parameter, status=[%lu]\n",status);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {

        /* amy add 1-24-2002 */
        /* set the specific trap receiver status */
        if (trap_receiver_ip_addr == trap_mgr_trap_receiver[i].trap_dest_address)
        {
            if (status==VAL_trapDestStatus_2_invalid)
            {
            if (trap_mgr_trap_receiver[i].trap_dest_status == status)//if specific ip already exist, and the status already the same as u want to set now, return false
            {
                i=SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER;
                available_entry=-1;
            }
            else
            {
                available_entry = i;
            }
            }
            else
            {
                available_entry = i;
            }
            break;
        }

        if ((status == VAL_trapDestStatus_2_valid)&&(trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid))
        {
            /* record the first empty entry.  When the entry is not existed, this
             * entry will be updated as the input.
             */
            if (available_entry == -1)
                available_entry = i;
            continue;
        }

    }


    if (i == SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER)
    {
        /* No found, create a new one, if no more available entry then return FALSE
         */
        if (available_entry != -1)
        {
            memset(trap_mgr_trap_receiver[available_entry].trap_dest_community, 0, SYS_ADPT_MAX_COMM_STR_NAME_LEN+1);
            trap_mgr_trap_receiver[available_entry].trap_dest_address = trap_receiver_ip_addr;
            trap_mgr_trap_receiver[available_entry].trap_dest_version = SYS_DFLT_TRAP_SNMP_VERSION;
            trap_mgr_trap_receiver[available_entry].trap_dest_port = SYS_DFLT_TRAP_UDP_PORT;
            trap_mgr_trap_receiver[available_entry].trap_dest_status = status;
#if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)
            strcpy(trap_mgr_trap_receiver[available_entry].trap_dest_owner, "");
            trap_mgr_trap_receiver[available_entry].trap_dest_protocol=TRAP_MGR_TRAP_DEST_PROTOCOL_IP;
#endif
            return_val = TRUE;
        }
        else
        {
            /* no available entry, can not create new entry
             */
#if (SYS_CPNT_EH == TRUE)
            EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_ERR,
                                 ":no available entry");
#endif//end of #if (SYS_CPNT_EH == TRUE)
            if (debug_mode)
                printf("FUNCTION:TRAP_MGR_SetTrapReceiverStatus; DESC:no available entry, can not create new entry\n");
            return_val = FALSE;
        }
    }
    else
    {
        /* found an entry, update status.  By logically, it should be deleted.
         */
        trap_mgr_trap_receiver[available_entry].trap_dest_status = status;
        /* default version set to 1 */
        trap_mgr_trap_receiver[available_entry].trap_dest_version = SYS_DFLT_TRAP_SNMP_VERSION;

        return_val = TRUE;
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return return_val;

} /* end of TRAP_MGR_SetTrapReceiverStatus() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverVersion
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the version can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        version                - the version for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverVersion(UI32_T    trap_receiver_ip_addr,
                                       UI32_T    version)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SetTrapReceiverVersion; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

   if ((version != VAL_trapDestVersion_version1) &&
        (version != VAL_trapDestVersion_version2) )
   {
#if (SYS_CPNT_EH == TRUE)
        UI8_T msg[256];
        sprintf(msg, "version=[%lu]",version);
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
                printf("FUNCTION:TRAP_MGR_SetTrapReceiverVersion; DESC:invalid parameter, version=[%lu]\n",version);
        SYSFUN_RELEASE_CSC();
        return FALSE;
   }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
        {
            /* record the first empty entry.  When the entry is not existed, this
             * entry will be updated as the input.
             */
            if (available_entry == -1)
                available_entry = i;
            continue;
        }

        if (trap_receiver_ip_addr == trap_mgr_trap_receiver[i].trap_dest_address)
            break;
    }


    if (i == SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER)
    {
        /* No found, create a new one, if no more available entry then return FALSE
         */
        if (available_entry != -1)
        {
            trap_mgr_trap_receiver[available_entry].trap_dest_version = version;
            trap_mgr_trap_receiver[available_entry].trap_dest_address = trap_receiver_ip_addr;
            return_val = TRUE;
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
            EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_ERR,
                                 ":no available entry");
#endif//end of #if (SYS_CPNT_EH == TRUE)
            /* no available entry, can not create new entry
             */
             if (debug_mode)
                printf("FUNCTION:TRAP_MGR_SetTrapReceiverVersion; DESC:no available entry, can not create new entry\n");
            return_val = FALSE;
        }
    }
    else
    {
        /* found an entry, update status.  By logically, it should be deleted.
         */
        trap_mgr_trap_receiver[i].trap_dest_version = version;
        return_val = TRUE;
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return return_val;

} /* end of TRAP_MGR_SetTrapReceiverVersion() */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverPort     							
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the port can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 * 																		
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        port                - the trap destination port
 * OUTPUT: None                                      				
 * RETURN: TRUE/FALSE 		
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverPort(UI32_T    trap_receiver_ip_addr,
                                       UI32_T    port)
{
    UI32_T  i;
    I32_T   available_entry = -1;
    BOOL_T  return_val = FALSE;

    /* BODY */
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif

        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverPort:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

   if (port<0 )
   {
#if (SYS_CPNT_EH == TRUE)
        UI8_T msg[256];
        sprintf(msg, "port=[%lu]",port);
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_INVALID_PARAMETER,
                                 SYSLOG_LEVEL_ERR,
                                 msg);
#endif
        if (debug_mode)
                printf("TRAP_MGR_SetTrapReceiverPort:invalid parameter, port=[%lu]\n",port);
        SYSFUN_RELEASE_CSC();
        return FALSE;
   }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_invalid)
        {
            /* record the first empty entry.  When the entry is not existed, this
             * entry will be updated as the input.
             */
            if (available_entry == -1)
                available_entry = i;
            continue;
        }

        if (trap_receiver_ip_addr == trap_mgr_trap_receiver[i].trap_dest_address)
            break;
    }


    if (i == SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER)
    {
        /* No found, create a new one, if no more available entry then return FALSE
         */
        if (available_entry != -1)
        {
            trap_mgr_trap_receiver[available_entry].trap_dest_port = port;
            trap_mgr_trap_receiver[available_entry].trap_dest_address = trap_receiver_ip_addr;
            return_val = TRUE;
        }
        else
        {
#if (SYS_CPNT_EH == TRUE)
            EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_FAILED_TO_ADD,
                                 SYSLOG_LEVEL_ERR,
                                 ":no available entry");
#endif
            /* no available entry, can not create new entry
             */
             if (debug_mode)
                printf("TRAP_MGR_SetTrapReceiverPort:no available entry, can not create new entry\n");
            return_val = FALSE;
        }
    }
    else
    {
        /* found an entry, update status.  By logically, it should be deleted.
         */
        trap_mgr_trap_receiver[i].trap_dest_port = port;
        return_val = TRUE;
    }

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return return_val;

} /* end of TRAP_MGR_SetTrapReceiverPort() */

#if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)
/*
=========================================
Get/GetNext TrapReceiver By Index API:
==========================================
*/
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified trap receiver
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 * OUTPUT: trap_receiver            - trap receiver info
 * RETURN: TRUE/FALSE
 * NOTES: 1. Status of each trap receiver is defined as following:
 *              - invalid(0): the entry of this trap receiver is deleted/purged
 *              - valid(1): this trap receiver is enabled
 *
 *           Set status to invalid(0) will delete/purge a trap receiver.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0').
 *
 *        4. The total number of trap receivers supported by the system
 *           is defined by SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *        5. By default, there is no trap receiver configued in the system.
 *        6. For any trap event raised by any subsystem, a SNMP Trap shall be
 *           sent to all of the enabled trap receivers.
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetTrapReceiverByIndex(UI32_T index, TRAP_MGR_TrapDestEntry_T *trap_receiver)
{
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_GetTrapReceiverByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_GetTrapReceiverByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_GetTrapReceiverByIndex:this entry is in a invalid status\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    else
    {
        *trap_receiver = trap_mgr_trap_receiver[index];
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return TRUE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetNextTrapReceiverByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available trap receiver
 *          can be retrieved successfully. Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 * OUTPUT: trap_receiver            - next available trap receiver info
 * RETURN: TRUE/FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_GetNextTrapReceiverByIndex(UI32_T *index, TRAP_MGR_TrapDestEntry_T *trap_receiver)
{
    UI32_T i,idx;
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_GetNextTrapReceiverByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if ((*index != 0xFFFFFFFF)&& ((*index < 0) || (*index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1)))
    {
        if (debug_mode)
            printf("TRAP_MGR_GetTrapReceiverByIndex:invalid index=[%lu]\n",*index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (*index == 0xFFFFFFFF)
        idx=0;
    else
        idx=*index+1;

    for (i=idx; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if (trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_valid)
        {
            *index=i;
            *trap_receiver = trap_mgr_trap_receiver[i];
            SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
            SYSFUN_RELEASE_CSC();
            return TRUE;
        }
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverCommStringNameByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the community string name can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        trap_dest_community      - the SNMP community string for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified trap_dest_address does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           status of this new trap receiver will be set to disabled(2)
 *           by default.
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverCommStringNameByIndex(UI32_T index, UI8_T *comm_string_name)
{
    SYSFUN_USE_CSC(FALSE);//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverCommStringNameByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (strlen(comm_string_name) > SYS_ADPT_MAX_COMM_STR_NAME_LEN)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverCommStringNameByIndex:comm string too long, commstr=[%s]\n",comm_string_name);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverCommStringNameByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverCommStringNameByIndex:this entry 's status is invalid\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    strcpy(trap_mgr_trap_receiver[index].trap_dest_community, comm_string_name);
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapDestProtocolByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the TrapDestProtocol can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        TrapDestProtocol
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapDestProtocolByIndex(UI32_T   index, TRAP_MGR_TRAP_DEST_PROTOCOL_E  protocol)
{
    SYSFUN_USE_CSC(FALSE);//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestProtocolByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if (protocol != TRAP_MGR_TRAP_DEST_PROTOCOL_IP)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestProtocolByIndex:Reject, only support TRAP_MGR_TRAP_DEST_PROTOCOL_IP now\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
     if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestProtocolByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestProtocolByIndex:this entry 's status is invalid\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    trap_mgr_trap_receiver[index].trap_dest_protocol=protocol;
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapDestAddressByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the TrapDestProtocol can be
 *          successfully set to the specified trap receiver .
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        TrapDestAddress
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapDestAddressByIndex(UI32_T index, UI32_T  addr)
{
    UI32_T i;
    SYSFUN_USE_CSC(FALSE);//stacking
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestAddressByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
     if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestAddressByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestAddressByIndex:this entry 's status is invalid\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    //reject if this ip address already exist in the entry
    for (i=0; i<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; i++)
    {
        if ((i != index)&&(trap_mgr_trap_receiver[i].trap_dest_status == VAL_trapDestStatus_2_valid))
        {
          if (trap_mgr_trap_receiver[i].trap_dest_address==addr)
          {
            if (debug_mode)
                printf("TRAP_MGR_SetTrapDestAddressByIndex:this ipaddress is already exist in the entry\n");
            SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
            SYSFUN_RELEASE_CSC();
            return FALSE;
           }
        }
    }
    trap_mgr_trap_receiver[index].trap_dest_address=addr;
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverStatusByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the status can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: index of trap receiver(just a seq#) -> key
 *        status                - the status for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: 1. This function will create a new trap receiver to the system if
 *           the specified ip_addr does not exist, and total number
 *           of trap receiver configured is less than
 *           SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *           When a new trap receiver is created by this function, the
 *           comm_string_name of this new trap receiver will be set to
 *           "DEFAULT".
 *        2. This function will update an existed trap receiver if
 *           the specified comm_string_name existed already.
 *        3. False is returned if total number of trap receiver configured
 *           is greater than SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverStatusByIndex(UI32_T index, UI32_T status)
{

    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverStatusByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if ((status != VAL_trapDestStatus_2_valid) &&
        (status != VAL_trapDestStatus_2_invalid) )
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverStatusByIndex:invalid parameter, status=[%lu]\n",status);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverStatusByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
    if (trap_mgr_trap_receiver[index].trap_dest_status==status)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverStatusByIndex:(trap_mgr_trap_receiver[index].trap_dest_status==status)\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 1\n");
    trap_mgr_trap_receiver[index].trap_dest_status=status;
    //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 2\n");
    //set default value for this entry
    if (status==VAL_trapDestStatus_2_valid)
    {
      //  printf("TRAP_MGR_SetTrapReceiverStatusByIndex 3\n");
         TRAP_MGR_GetDefaultIpAddress(&trap_mgr_trap_receiver[index].trap_dest_address);
        // printf("TRAP_MGR_SetTrapReceiverStatusByIndex 4\n");
         memset(trap_mgr_trap_receiver[index].trap_dest_community, 0, SYS_ADPT_MAX_COMM_STR_NAME_LEN+1);
         //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 5\n");
         trap_mgr_trap_receiver[index].trap_dest_version = VAL_trapDestVersion_version1;
         //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 6\n");
         strcpy(trap_mgr_trap_receiver[index].trap_dest_owner, "");
         //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 7\n");
         trap_mgr_trap_receiver[index].trap_dest_protocol=TRAP_MGR_TRAP_DEST_PROTOCOL_IP;
         //printf("TRAP_MGR_SetTrapReceiverStatusByIndex 8\n");
    }
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapReceiverVersionByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the version can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        version                - the version for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapReceiverVersionByIndex(UI32_T index, UI32_T version)
{
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverVersionByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

   if ((version != VAL_trapDestVersion_version1) &&
        (version != VAL_trapDestVersion_version2) )
   {
        if (debug_mode)
                printf("TRAP_MGR_SetTrapReceiverVersionByIndex:invalid parameter, version=[%lu]\n",version);
        SYSFUN_RELEASE_CSC();
        return FALSE;
   }
   if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverVersionByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
   SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
   if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapReceiverVersionByIndex:this entry 's status is invalid\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    trap_mgr_trap_receiver[index].trap_dest_version=version;
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_SetTrapDestOwnerByIndex
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the trapDestOwner can be successfully
 *          set to the specified trap receiver.
 *          Otherwise, false is returned.
 *
 * INPUT: trap_receiver_ip_addr - (key) to specify a unique trap receiver
 *        version                - the version for this trap receiver
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES: None
 * ---------------------------------------------------------------------
 */
BOOL_T TRAP_MGR_SetTrapDestOwnerByIndex(UI32_T index, UI8_T *owner)
{
    SYSFUN_USE_CSC(FALSE);//stacking

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestOwnerByIndex:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
   if ((strlen(owner) < MINSIZE_trapDestOwner) || (strlen(owner) > MAXSIZE_trapDestOwner))
   {
        if (debug_mode)
                printf("TRAP_MGR_SetTrapDestOwnerByIndex:invalid parameter, strlen(owner)=[%d]\n",strlen(owner));
        SYSFUN_RELEASE_CSC();
        return FALSE;
   }
   if ((index < 0) || (index > SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER-1))
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestOwnerByIndex:invalid index=[%lu]\n",index);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
   SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);
   if (trap_mgr_trap_receiver[index].trap_dest_status == VAL_trapDestStatus_2_invalid)
    {
        if (debug_mode)
            printf("TRAP_MGR_SetTrapDestOwnerByIndex:this entry 's status is invalid\n");
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }
    strcpy(trap_mgr_trap_receiver[index].trap_dest_owner, owner);
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    SYSFUN_RELEASE_CSC();
    return TRUE;
}
#endif //end of #if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)
#endif //end of #if (SYS_CPNT_SNMP_VERSION == 2)




/* Running Config API
 */



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetRunningSnmpEnableAuthenTraps
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          snmp authentication trap status can be retrive
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: none
 * OUTPUT: snmp_enable_authen_traps - VAL_snmpEnableAuthenTraps_enabled \
 *                                    VAL_snmpEnableAuthenTraps_disabled
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default trap receiver.
 * ---------------------------------------------------------------------
 */
UI32_T TRAP_MGR_GetRunningSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps)
{
#if (SYS_CPNT_SNMP_VERSION == 3)
        SNMP_MGR_GetRunningSnmpEnableAuthenTraps(snmp_enable_authen_traps);
#else
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);//stacking
    //fibi change for stacking, add new return val "SYS_TYPE_GET_RUNNING_CFG_FAIL"
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetRunningSnmpEnableAuthenTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    *snmp_enable_authen_traps = trap_mgr_snmp_enable_authen_traps;
    if (trap_mgr_snmp_enable_authen_traps != SYS_DFLT_SNMP_ENABLE_AUTHEN_TRAPS)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
#endif
} /* end of TRAP_MGR_GetNextRunningSnmpEnableAuthenTraps() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetRunningLinkUpDownTraps
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          link up down trap status  of the device can be retrieve
 *          successfully. Otherwise, SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:  none
 * OUTPUT: link_up_down_trap  - TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED \
 *                              TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *         SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default trap receiver.
 * ---------------------------------------------------------------------
 */
UI32_T TRAP_MGR_GetRunningLinkUpDownTraps(UI8_T *link_up_down_trap)
{
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);//stacking
    //fibi change for stacking, add new return val "SYS_TYPE_GET_RUNNING_CFG_FAIL"
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)//stacking
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NOT_IN_MASTER_MODE,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_GetRunningLinkUpDownTraps; DESC:Reject, not in master mode\n");
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    *link_up_down_trap = trap_mgr_link_up_down_traps;
    if (trap_mgr_link_up_down_traps != SYS_DFLT_IF_LINK_UP_DOWN_TRAP_ENABLE)
    {
       SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

} /* end of TRAP_MGR_GetRunningLinkUpDownTraps() */



/* LOCAL SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_QueueDequeue
 * ---------------------------------------------------------------------
 *  FUNCTION: remove a trap data block from queue
 *
 *  INPUT    : TRAP_MGR_Queue_T *q
 *  OUTPUT   : NONE.
 *  RETURN   : TRAP_EVENT_TrapData_T    *p  - if has element in the queue
 *             NULL             - no element in queue
 *  NOTE     : NONE.
 * ---------------------------------------------------------------------
 */
static TRAP_EVENT_TrapQueueData_T *TRAP_MGR_QueueDequeue(TRAP_MGR_Queue_T *trap_queue)
{
   /* LOCAL VARIABLE DECLARATIONS */
    TRAP_EVENT_TrapQueueData_T    *trap_data;

    /* BODY */

    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    /* Queue is empty now!
     */
    if (trap_queue->front == (TRAP_EVENT_TrapQueueData_T *)NULL)
    {
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        return NULL;
    }
    /* More items in the queue.
     */
    trap_data = trap_queue->front;             /* Return the first element */
    trap_queue->front = trap_data->next;        /* Move queue head to next element  */

    if (trap_queue->front == (TRAP_EVENT_TrapQueueData_T *)NULL)
    {
        trap_queue->rear = (TRAP_EVENT_TrapQueueData_T *)NULL;      /*  queue is empty */
    } /* End of if */
    trap_queue->que_elements_cnt--;
    trap_data->remainRetryTimes--;
    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);

    return trap_data;

} /* End of TRAP_MGR_QueueDequeue() */

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_GetDynamicDataSize
 * ---------------------------------------------------------------------
 *  FUNCTION: Get the size of the dynamic data block
 *
 *  INPUT    : trap_type
 *  OUTPUT   : NONE.
 *  RETURN   : data size
 *  NOTE     :
 * ---------------------------------------------------------------------
 */
static UI32_T TRAP_MGR_GetDynamicDataSize(UI32_T trap_type)
{
	switch(trap_type)
	{
		case TRAP_EVENT_LINK_DOWN:    				
		case TRAP_EVENT_LINK_UP:
			return sizeof(TRAP_EVENT_LinkTrap_T);
			break;
                case TRAP_EVENT_CRAFT_PORT_LINK_DOWN:
                case TRAP_EVENT_CRAFT_PORT_LINK_UP:
                        return sizeof(TRAP_EVENT_CraftLinkTrap_T);
                        break;
		case TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP:
			return sizeof(TRAP_EVENT_PowerStatusChangeTrap_T);
			break;
		case TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP:
			return sizeof(TRAP_EVENT_ActivePowerChangeTrap_T);
			break;
		case TRAP_EVENT_RISING_ALARM:
		case TRAP_EVENT_FALLING_ALARM:
			return sizeof(TRAP_EVENT_RisingFallingAlarmTrap_T);
			break;	
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE) 				
		case TRAP_EVENT_PORT_SECURITY_TRAP:
			return sizeof(TRAP_EVENT_PortSecurityTrap_T);
			break;
#endif			
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE) 				
		case TRAP_EVENT_LOOPBACK_TEST_FAILURE_TRAP:
			return sizeof(TRAP_EVENT_LoopBackFailureTrap_T);
			break;	
#endif	
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)					
		case TRAP_EVENT_FAN_FAILURE:
    	case TRAP_EVENT_FAN_RECOVER:
    		return sizeof(TRAP_EVENT_FanTrap_T);
#endif
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)    		
    	case TRAP_EVENT_IPFILTER_REJECT_TRAP:
    		return sizeof(TRAP_EVENT_IpFilterRejectTrap_T);    		
    		break;

    	case TRAP_EVENT_IPFILTER_INET_REJECT_TRAP:
    		return sizeof(TRAP_EVENT_IpFilterInetRejectTrap_T);
    		break;
#endif	
#if (SYS_CPNT_SMTP == TRUE)   		
    	case TRAP_EVENT_SMTP_CONN_FAILURE_TRAP:
    		return sizeof(TRAP_EVENT_swSmtpConnFailureTrap_T);    		
    		break;	    	
#endif
#if (SYS_CPNT_POE == TRUE)
		case TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP:
			return sizeof(TRAP_EVENT_PethPsePortOnOffTrap_T);    		
    		break;
    	case TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP:
    		return sizeof(TRAP_EVENT_PethMainPowerUsageOnTrap_T);    		
    		break;
    	case TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP:
    		return sizeof(TRAP_EVENT_PethMainPowerUsageOffTrap_T);    		
    		break;
#endif  //end of #if (SYS_CPNT_POE == TRUE)  			
#if (SYS_CPNT_VDSL == TRUE)
		case TRAP_EVENT_VDSL_PERF_LOFS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfLofsThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_LOSS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfLossThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_LPRS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfLprsThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_LOLS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfLolsThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_ESS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfESsThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_SESS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfSESsThresh_T);
			break;
		case TRAP_EVENT_VDSL_PERF_UASS_THRESH:
			return sizeof(TRAP_EVENT_vdslPerfUASsThresh_T);
			break;
#endif//end of #if (SYS_CPNT_VDSL == TRUE)
case TRAP_EVENT_MAIN_BOARD_VER_MISMATCH:
            return sizeof(TRAP_EVENT_mainBoardVerMismatch_T);
            break;
        case TRAP_EVENT_MODULE_VER_MISMATCH:
            return sizeof(TRAP_EVENT_moduleVerMismatch_T);
            break;
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case TRAP_EVENT_THERMAL_RISING:
            return sizeof(TRAP_EVENT_thermalRising_T);
            break;
        case TRAP_EVENT_THERMAL_FALLING:
            return sizeof(TRAP_EVENT_thermalFalling_T);
            break;
#endif

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
        case TRAP_EVENT_MODULE_INSERTION:
            return sizeof(TRAP_EVENT_moduleInsertion_T);
            break;
        case TRAP_EVENT_MODULE_REMOVAL:
            return sizeof(TRAP_EVENT_moduleRemoval_T);
            break;
#endif
	case TRAP_EVENT_TCN:
	     return sizeof(TRAP_EVENT_tcn_T);
	     break;
#if(SYS_CPNT_LLDP == TRUE)
    case TRAP_EVENT_LLDP_REM_TABLES_CHANGED:
         return sizeof(TRAP_EVENT_lldpRemTablesChange_T);
         break;
#endif
#if(SYS_CPNT_LLDP_MED == TRUE)
    case TRAP_EVENT_LLDP_MED_TOPOLOGY_CHANGE_DETECTED:
        return sizeof(TRAP_EVENT_lldpMedTopologyChange_T);
        break;
#endif
#if (SYS_CPNT_EFM_OAM == TRUE)
    case TRAP_EVENT_DOT3_OAM_THRESHOLD:
        return sizeof(TRAP_EVENT_dot3OamThreshold_T);
        break;

    case TRAP_EVENT_DOT3_OAM_NON_THRESHOLD:
        return sizeof(TRAP_EVENT_dot3OamNonThreshold_T);
        break;
#endif  /* #if (SYS_CPNT_EFM_OAM == TRUE) */

#if (SYS_CPNT_ATC_STORM == TRUE)
        case TRAP_EVENT_BCAST_STORM_ALARM_FIRE:
            return sizeof(TRAP_EVENT_BcastStormAlarmFire_T);
            break;

        case TRAP_EVENT_BCAST_STORM_ALARM_CLEAR:
            return sizeof(TRAP_EVENT_BcastStormAlarmClear_T);
            break;

        case TRAP_EVENT_BCAST_STORM_TC_APPLY:
            return sizeof(TRAP_EVENT_BcastStormTcApply_T);
            break;

        case TRAP_EVENT_BCAST_STORM_TC_RELEASE:
            return sizeof(TRAP_EVENT_BcastStormTcRelease_T);
            break;

        case TRAP_EVENT_MCAST_STORM_ALARM_FIRE:
            return sizeof(TRAP_EVENT_McastStormAlarmFire_T);
            break;

        case TRAP_EVENT_MCAST_STORM_ALARM_CLEAR:
            return sizeof(TRAP_EVENT_McastStormAlarmClear_T);
            break;

        case TRAP_EVENT_MCAST_STORM_TC_APPLY:
            return sizeof(TRAP_EVENT_McastStormTcApply_T);
            break;

        case TRAP_EVENT_MCAST_STORM_TC_RELEASE:
            return sizeof(TRAP_EVENT_McastStormTcRelease_T);
            break;

#endif

#if(SYS_CPNT_CFM==TRUE)
        case TRAP_EVENT_CFM_MEP_UP:    				
            return sizeof(TRAP_EVENT_CfmMepUp_T);
            break;

        case TRAP_EVENT_CFM_MEP_DOWN:    				
            return sizeof(TRAP_EVENT_CfmMepDown_T);
            break;

        case TRAP_EVENT_CFM_CONFIG_FAIL:    				
            return sizeof(TRAP_EVENT_CfmConfigFail_T);
            break;

        case TRAP_EVENT_CFM_LOOP_FIND:    				
            return sizeof(TRAP_EVENT_CfmLoopFind_T);
            break;

        case TRAP_EVENT_CFM_MEP_UNKNOWN:    				
            return sizeof(TRAP_EVENT_CfmMepUnknown_T);
            break;

        case TRAP_EVENT_CFM_MEP_MISSING:    				
            return sizeof(TRAP_EVENT_CfmMepMissing_T);
            break;

        case TRAP_EVENT_CFM_MA_UP:    				
            return sizeof(TRAP_EVENT_CfmMaUp_T);
            break;			

        case TRAP_EVENT_CFM_FAULT_ALARM:
            return sizeof(TRAP_EVENT_CfmFaultAlarm_T);
            break;
#endif  /* #if(SYS_CPNT_CFM==TRUE) */

	default:
	     return 0;
	}
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_malloc
 * ---------------------------------------------------------------------
 *  FUNCTION: alloc memory
 *
 *  INPUT    : size
 *  OUTPUT   :
 *  RETURN   : pointer of allocated memory block
 *  NOTE     :
 * ---------------------------------------------------------------------
 */
static void *TRAP_MGR_malloc(size_t size)
{
	void *p;	
	if ((p=L_MM_Malloc(size, L_MM_USER_ID2(SYS_MODULE_TRAPMGMT, TRAP_TYPE_TRACE_ID_TRAP_MGR_MALLOC)))!=NULL)
	{
		memory_used=memory_used+size;
		trapDataInQCnt++;
	}
	return p;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_free
 * ---------------------------------------------------------------------
 *  FUNCTION: free memory block
 *
 *  INPUT    : pointer of memory block to be free
 *  OUTPUT   :
 *  RETURN   :
 *  NOTE     :
 * ---------------------------------------------------------------------
 */
static void TRAP_MGR_free(void *ptr)
{
	memory_used = memory_used - (sizeof(TRAP_EVENT_TrapQueueData_T) + TRAP_MGR_GetDynamicDataSize(((TRAP_EVENT_TrapQueueData_T*)ptr)->trap_type));
	trapDataInQCnt--;
	L_MM_Free(ptr);
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_BuildQueueData
 * ---------------------------------------------------------------------
 *  FUNCTION: alloc memory and build a trap queue data
 *
 *  INPUT    : TRAP_EVENT_TrapData_T     *p
 *  OUTPUT   :
 *  RETURN   : *TRAP_EVENT_TrapQueueData_T
 *  NOTE     :
 * ---------------------------------------------------------------------
 */
static TRAP_EVENT_TrapQueueData_T *TRAP_MGR_BuildQueueData(TRAP_EVENT_TrapData_T *trapData)
{		
	TRAP_EVENT_TrapQueueData_T                     *new_blk;
	UI32_T dynamicDataSize;
	
	dynamicDataSize=TRAP_MGR_GetDynamicDataSize(((TRAP_EVENT_TrapQueueData_T*)(trapData))->trap_type);	
	new_blk = TRAP_MGR_malloc(sizeof(TRAP_EVENT_TrapQueueData_T)+dynamicDataSize);   	

	if (new_blk == NULL)
	{
	    if (debug_mode)
            printf("TRAP_EVENT_TrapQueueData_T:alloc memory failure, new_blk\n");
            return NULL;
	}
	memcpy(new_blk,trapData,sizeof(TRAP_EVENT_TrapQueueData_T));//copy the fix data
	memcpy(new_blk->dynamicData,&(((TRAP_EVENT_TrapData_T*)(trapData))->u), dynamicDataSize);//copy dynamic data Part	
	return new_blk;
}

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_QueueEnqueue
 * ---------------------------------------------------------------------
 *  FUNCTION: add a trap data block to queue and send a event to trap
 *            task notify a send trap request in the queue.
 *
 *  INPUT    : TRAP_EVENT_TrapData_T     *p
 *             TRAP_MGR_Queue_T *q
 *  OUTPUT   : NONE.
 *  RETURN   : NONE.
 *  NOTE     : This procedure shall not be invoked before L_MEM_Init() is called.
 * ---------------------------------------------------------------------
 */
static BOOL_T TRAP_MGR_QueueEnqueue(TRAP_EVENT_TrapQueueData_T *qData, TRAP_MGR_Queue_T *q)
{
   /* LOCAL VARIABLE DECLARATIONS */

   /* BODY */
    if (qData == NULL)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NULL_POINTER,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_QueueEnqueue; DESC:qData=NULL\n");
        return FALSE;
    } /* End of if */

    if (q == NULL)
    {

#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_NULL_POINTER,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_QueueEnqueue; DESC:q=NULL\n");
        return FALSE;
    } /* End of if */

    /* Should not takes up more then TRAP_MGR_MAX_QUE_CNT trap data in the dynamic queue.
       This is to prevent trap_mgr allocate too much system memory in the future.
     */
    if (q->que_elements_cnt >= TRAP_MGR_MAX_QUE_CNT)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "too many traps");
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_QueueEnqueue; DESC:too many traps\n");
        TRAP_MGR_free(qData);
        return FALSE;
     }
    SYSFUN_ENTER_CRITICAL_SECTION(trap_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER);

    if(SYSFUN_SendEvent(trap_mgr_lcb.trap_task_id, TRAP_MGR_EVENT_TRAP_ARRIVAL)!= SYSFUN_OK)
    {

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_QueueEnqueue; DESC:SYSFUN_SendEvent failure\n");
        TRAP_MGR_free(qData);
        SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
        return FALSE;
    }

    if (q->rear == (TRAP_EVENT_TrapQueueData_T *)NULL )   /* empty queue */
    {
        q->rear = qData;
        q->front = qData;
        qData->next = (TRAP_EVENT_TrapQueueData_T *)NULL;
    }
    else
    {
        q->rear->next = qData;
        q->rear = qData;
        qData->next = (TRAP_EVENT_TrapQueueData_T *)NULL;

    } /* End of if */
    q->que_elements_cnt++;

    SYSFUN_LEAVE_CRITICAL_SECTION(trap_mgr_sem_id);
    return TRUE;

} /* End of TRAP_MGR_QueueEnqueue() */



/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SendTrap
 * ---------------------------------------------------------------------------
 *  FUNCTION: Send the trap specified by the trap data block.
 *
 *  INPUT    : TRAP_EVENT_TrapData_T *trap_data.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static BOOL_T TRAP_MGR_SendTrap(TRAP_EVENT_TrapQueueData_T *trap_data)
{
   /* LOCAL VARIABLE DECLARATIONS */

    UI32_T      local_ip;
    UI32_T      dest_ip;
    I32_T       send_length=0;
    UI8_T       trap_dest_oct[6]; /* 4- IP, 2- UDP port*/
    UI8_T       trap_comm_str[SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER+1];

    TRAP_MGR_Octet_T            trap_dest_addr;
    TRAP_MGR_Octet_T            trap_comm_name;
    EBUFFER_T                   ebuff;
    struct sockaddr_in          traps_to;
#if (SYS_CPNT_SNMP_VERSION == 3)
    SNMP_MGR_TrapDestEntry_T     trap_receiver;
#else
    TRAP_MGR_TrapDestEntry_T     trap_receiver;
#endif
    NETIF_OM_RifNode_T          rif_node;
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)

    /* BODY */
    memset(&rif_node, 0, sizeof(NETIF_OM_RifNode_T));
    trap_dest_addr.octet_P = trap_dest_oct;
    trap_comm_name.octet_P = trap_comm_str;
    memset(&trap_receiver, 0, sizeof(trap_receiver));

    /* Get local IP address
         */
    rif_node.vid_ifIndex = 0;
    rif_node.ipAddress = 0;
    rif_node.ipMask = 0;

    if (NETCFG_GetNextRifInfo(&rif_node) < 0)
    {
        //if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SendTrap; DESC:NETCFG_GetNextRifInfo < 0\n");
        return FALSE;
    }
    /* Copy local source IP address
     */
    memcpy (&local_ip,&rif_node.ipAddress,4);
#if (SYS_CPNT_SNMP_VERSION == 3)
    while (SNMP_MGR_GetNextTrapReceiver(&trap_receiver)== SNMP_MGR_ERROR_OK)
#else
    while (TRAP_MGR_GetNextTrapReceiver(&trap_receiver))
#endif
    {
        /* set the get buffer length
         */
        trap_dest_addr.len = 4+2;
        trap_comm_name.len = SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER+1;

        /* 1. Get informations from trap destination table
           2. This API only return trap receiver with VALID status
         */
        if (trap_data->community_specified == TRUE)
        {
            if (strcmp(trap_receiver.trap_dest_community,trap_data->community) != 0)
            {
                if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_SendTrap; DESC:community not match, trap_data->community=[%s]\n",trap_data->community);
                continue;/* community names are not match */
            }

        } /* End of if */

        /* Copy destionation IP address
         */
        memcpy (&dest_ip,&trap_receiver.trap_dest_address,4);

        EBufferInitialize(&ebuff);

        if (!TRAP_MGR_ProcessSendTrap(&ebuff,
                                      (OCTET_T *)&local_ip,
                                      &trap_receiver,
                                      trap_data,
                                      trap_data->trap_time))
        {
#if (SYS_CPNT_EH == TRUE)
            if (EHMsgEnable)
            {
                EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
                printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
            }
#endif//end of #if (SYS_CPNT_EH == TRUE)
            if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_SendTrap; DESC:TRAP_MGR_ProcessSendTrap return false\n");
            continue;
        }
      traps_to.sin_family = AF_INET;
      traps_to.sin_port = L_STDLIB_Hton16(SYS_DFLT_TRAP_UDP_PORT); // rfc1157, p16
      traps_to.sin_addr = dest_ip;  /* host_addr */

      send_length = sendto (trap_socket,
                            (char *) ebuff.start_bp,
                            EBufferUsed(&ebuff),
                            0,
                            (struct sockaddr*)&traps_to,
                            sizeof(traps_to));
     if (send_length <= 0)
     {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 "Sendto failed");
#endif//end of #if (SYS_CPNT_EH == TRUE)
        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_SendTrap; DESC:socket send_length <= 0, TrapType=[%lu], trap receiver ip=[%lu.%lu.%lu.%lu]\n",trap_data->trap_type,dest_ip>>24,dest_ip>>16 & 0x000000FF,dest_ip>>8  & 0x000000FF,dest_ip>>0  & 0x000000FF);
        s_close(trap_socket);//close socket;
        trap_socket=-1;
        return FALSE;
     }
     if (debug_mode)
        printf("FUNCTION:TRAP_MGR_SendTrap; DESC:Send Trap, TrapType=[%lu], trap receiver ip=[%lu.%lu.%lu.%lu]\n",trap_data->trap_type,dest_ip>>24,dest_ip>>16 & 0x000000FF,dest_ip>>8  & 0x000000FF,dest_ip>>0  & 0x000000FF);

     SNMP_MGR_IncrementTrapSendCounter();
      EBufferClean(&ebuff );
    } /* End of while (TRAP_MGR_GetNextTrapReceiver(&trap_receiver))  */
    return TRUE;
} /* End of TRAP_MGR_SendTrap() */


/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_Task
 * ---------------------------------------------------------------------------
 *  FUNCTION: This function is entry point of TRAP task. It is a forever loop
 *            wait for event and send the trap from trap queue to network.
 *
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static void TRAP_MGR_Task(void)
{
    UI32_T              event;
    static UI32_T      keepsEvt=0;
#if (SYS_CPNT_EH == TRUE)
     UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
    /* BODY */

    /* Task initiation.
     */
    if (!trap_mgr_lcb.init_flag)
    {
        if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_Task; DESC:trap_mgr_lcb.init_flag==false\n");
        return;
    }

    /* Task body
     */

    while (TRUE)
    {
        if (SYSFUN_ReceiveEvent((TRAP_MGR_EVENT_TRAP_ARRIVAL | TRAP_MGR_EVENT_ENTER_TRANSITION |
                                TRAP_MGR_EVENT_STA_STATE_CHANGED | TRAP_MGR_EVENT_RIF_UP |
                                TRAP_MGR_EVENT_RIF_DOWN ),
                                SYSFUN_EVENT_WAIT_ANY, SYSFUN_TIMEOUT_WAIT_FOREVER,
                                &event)!=SYSFUN_OK)
        {
#if (SYS_CPNT_EH == TRUE)
            if (EHMsgEnable)
            {
                EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
                printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
            }
#endif//end of #if (SYS_CPNT_EH == TRUE)
            if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_Task; DESC:SYSFUN_ReceiveEvent!=SYSFUN_OK\n");
        }
        keepsEvt |= event;
        if (keepsEvt & TRAP_MGR_EVENT_ENTER_TRANSITION)
        {
	    if (not_check_rif_cnt)
	    {
	    	    if ((spanning_tree_state == STA_STABLED_STATE) && (TRAP_MGR_CreateTrapSocket()))
	    	TRAP_MGR_HandleTrapQueue();
                else
                    TRAP_MGR_ClearTrapQueue();
            }
            else
            {        	
                if ((spanning_tree_state == STA_STABLED_STATE) && (up_rif_cnt > 0) && (TRAP_MGR_CreateTrapSocket()))
            TRAP_MGR_HandleTrapQueue();
            else
                TRAP_MGR_ClearTrapQueue();
            }
            flush_trap_queue_done_flag=TRUE;
            keepsEvt ^= TRAP_MGR_EVENT_ENTER_TRANSITION;
        }
        if (keepsEvt & TRAP_MGR_EVENT_STA_STATE_CHANGED)
        {
            if (spanning_tree_state == STA_BECOME_STABLED_STATE)
            {
                if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_Task; DESC:receive event TRAP_MGR_EVENT_STA_STATE_CHANGED\n");
                spanning_tree_state = STA_STABLED_STATE;
                //SYSFUN_Sleep(35*SYS_BLD_TICKS_PER_SECOND);
            }
            keepsEvt ^= TRAP_MGR_EVENT_STA_STATE_CHANGED;
        }
        if (keepsEvt & TRAP_MGR_EVENT_RIF_DOWN)
        {
            up_rif_cnt--;
            keepsEvt ^= TRAP_MGR_EVENT_RIF_DOWN;
        }
        if (keepsEvt & TRAP_MGR_EVENT_RIF_UP)
        {
            up_rif_cnt++;
            keepsEvt ^= TRAP_MGR_EVENT_RIF_UP;
        }
        if (keepsEvt & TRAP_MGR_EVENT_TRAP_ARRIVAL)
        {
            /* Note: The trap messages cant be sent out only if the netwrok interface is available and
             * spanning tree stablized.
             */
            if (not_check_rif_cnt)
            {
            	if ((spanning_tree_state == STA_STABLED_STATE) && (TRAP_MGR_CreateTrapSocket()))
	        {
	                TRAP_MGR_HandleTrapQueue();
	                keepsEvt ^= TRAP_MGR_EVENT_TRAP_ARRIVAL;
	        }
            }
            else
            {
                if ((spanning_tree_state == STA_STABLED_STATE) && (up_rif_cnt > 0) && (TRAP_MGR_CreateTrapSocket()))
            {
                TRAP_MGR_HandleTrapQueue();
                keepsEvt ^= TRAP_MGR_EVENT_TRAP_ARRIVAL;
            }
        }
        }
    }//end of while
} /* End of TRAP_MGR_Task() */


/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_CreateTrapSocket
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static BOOL_T TRAP_MGR_CreateTrapSocket(void)
{
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;
    struct sockaddr_in   srvr;
    /* BODY */
    if (trap_socket > 0)
    {
        return TRUE;//socket already open, don't need to open anymore
    }

    if ((trap_socket = socket(AF_INET, SOCK_DGRAM, 0)) <=0)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_SOCKET_OP_FAILED,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_CreateTrapSocket; DESC:socket(AF_INET, SOCK_DGRAM, 0)==-1\n");
        owner_info.level = SYSLOG_LEVEL_ERR;
        owner_info.module_no = SYS_MODULE_TRAPMGMT;
        owner_info.function_no = TRAP_MGR_FUNCTION_NUMBER;
        owner_info.error_no = TRAP_MGR_ERROR_NUMBER;
        SYSLOG_MGR_AddFormatMsgEntry(&owner_info, FUNCTION_RETURN_FAIL_INDEX, "TRAP_MGR_CreateTrapSocket", 0, 0);
        return FALSE;
    } /* End of if */

    srvr.sin_family = AF_INET;
    srvr.sin_port = L_STDLIB_Hton16(SYS_DFLT_TRAP_UDP_PORT);
    srvr.sin_addr = 0L;


    if (bind(trap_socket, (struct sockaddr*)&srvr, sizeof (srvr)) < 0)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_SOCKET_OP_FAILED,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_CRIT));
#endif//end of #if (SYS_CPNT_EH == TRUE)

        if (debug_mode)
            printf("FUNCTION:TRAP_MGR_CreateTrapSocket; DESC:cannot bind socket\n");
        owner_info.level = SYSLOG_LEVEL_ERR;
        owner_info.module_no = SYS_MODULE_TRAPMGMT;
        owner_info.function_no = TRAP_MGR_FUNCTION_NUMBER;
        owner_info.error_no = TRAP_MGR_ERROR_NUMBER;
        SYSLOG_MGR_AddFormatMsgEntry(&owner_info, FUNCTION_RETURN_FAIL_INDEX, "Can't bind trap socket", 0, 0);
        return FALSE;

    } /* End of if */
    return TRUE;

} /* end of TRAP_MGR_CreateTrapSocket() */


/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SendColdStartTrap
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static void TRAP_MGR_SendColdStartTrap()
{
    /* LOCAL VARIABLE DECLARATIONS */
    TRAP_EVENT_TrapData_T       cold_start_trap;
    /* BODY */

    /* Request to send cold start trap
     * Note: This request must be sent early than any other traps!
     */
    cold_start_trap.trap_type = TRAP_EVENT_COLD_START;
    cold_start_trap.community_specified = FALSE;
    TRAP_MGR_ReqSendTrap(&cold_start_trap);

    return;

} /* end of TRAP_MGR_SendColdStartTrap() */

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_SendWarmStartTrap
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static void TRAP_MGR_SendWarmStartTrap()
{
    /* LOCAL VARIABLE DECLARATIONS */
    TRAP_EVENT_TrapData_T       warm_start_trap;
    /* BODY */

    /* Request to send cold start trap
     * Note: This request must be sent early than any other traps!
     */
    warm_start_trap.trap_type = TRAP_EVENT_WARM_START;
    warm_start_trap.community_specified = FALSE;
    TRAP_MGR_ReqSendTrap(&warm_start_trap);
    return;

} /* end of TRAP_MGR_SendWarmStartTrap() */


/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ClearTrapQueue
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static void TRAP_MGR_ClearTrapQueue(void)
{
    TRAP_EVENT_TrapQueueData_T     *trap_data;
    if (debug_mode)
         printf("TRAP_MGR_ClearTrapQueue\n");
    while ((trap_data = TRAP_MGR_QueueDequeue(&trap_queue)) != NULL)
    {
        TRAP_MGR_free(trap_data);
    }
    return;
}

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_HandleTrapQueue
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static void TRAP_MGR_HandleTrapQueue(void)
{
    TRAP_EVENT_TrapQueueData_T     *trap_data;

    /* BODY */

    while ((trap_data = TRAP_MGR_QueueDequeue(&trap_queue)) != NULL)
    {
        if (trap_socket >0)
            TRAP_MGR_SendTrap(trap_data);
        TRAP_MGR_free(trap_data);
        //Need to add delay between sending traps.
        //Otherwise, just after start-up, some trap will not be received by the PC.
        SYSFUN_Sleep(SYS_BLD_TICKS_PER_SECOND/10);
    }
    return;
} /* end of TRAP_MGR_HandleTrapQueue() */

/* ---------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ProcessSendTrap
 * ---------------------------------------------------------------------------
 *  FUNCTION:
 *  INPUT    : None.
 *  OUTPUT   : None.
 *  RETURN   : None.
 *  NOTE     : None.
 * ---------------------------------------------------------------------------
 */
static BOOL_T TRAP_MGR_ProcessSendTrap(EBUFFER_T                  *ebuff,
                                       OCTET_T                    *local_ip,
#if (SYS_CPNT_SNMP_VERSION == 3)
                                       SNMP_MGR_TrapDestEntry_T     *trap_receiver,
#else
                                       TRAP_MGR_TrapDestEntry_T    *trap_receiver,
#endif
                                       TRAP_EVENT_TrapQueueData_T  *trap_data,
                                       UI32_T                     system_time)
{
    UI32_T      unit, port, trunk_id, vid;
    SWCTRL_Lport_Type_T     port_type;
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;
#if (SYS_CPNT_EH == TRUE)
    UI8_T msg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)

    /* BODY */
    switch (trap_data->trap_type)
    {
        case TRAP_EVENT_COLD_START:
            return TRAP_UTIL_SendColdStartTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_WARM_START:
            return TRAP_UTIL_SendWarmStartTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_LINK_DOWN:
		{
			TRAP_EVENT_LinkTrap_T *pDynamicData;			
			pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;				
            port_type = SWCTRL_LogicalPortToUserPort(pDynamicData->ifindex, &unit, &port, &trunk_id);

            if (pDynamicData->ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
            {
                VLAN_MGR_ConvertFromIfindex(pDynamicData->ifindex, &vid);

                if ((vid < SYS_DFLT_1Q_PORT_VLAN_PVID) || (vid > SYS_ADPT_MAX_VLAN_ID))
                {
#if (SYS_CPNT_EH == TRUE)
                    sprintf(msg, "invalid vid range=[%lu]", vid);
                    EH_MGR_Handle_Exception1(SYS_MODULE_SNMP,
                                 0,
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
                    if (debug_mode)
                        printf("FUNCTION:TRAP_MGR_ProcessSendTrap; DESC:vid range error, vid=[%lu]\n",vid);
                    return FALSE;
                }

            } /* end of if */
            return TRAP_UTIL_SendLinkDownTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);

            break;
		}
        case TRAP_EVENT_LINK_UP:
		{
			TRAP_EVENT_LinkTrap_T *pDynamicData;			
			pDynamicData=(TRAP_EVENT_LinkTrap_T *)trap_data->dynamicData;				
            port_type = SWCTRL_LogicalPortToUserPort(pDynamicData->ifindex, &unit, &port, &trunk_id);

            if (pDynamicData->ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
            {
                VLAN_MGR_ConvertFromIfindex(pDynamicData->ifindex, &vid);

                if ((vid < SYS_DFLT_1Q_PORT_VLAN_PVID) || (vid > SYS_ADPT_MAX_VLAN_ID))
                {
#if (SYS_CPNT_EH == TRUE)
                    sprintf(msg, "vid range error, vid=[%lu]",vid);
                    EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
                    if (debug_mode)
                        printf("FUNCTION:TRAP_MGR_ProcessSendTrap; DESC:vid range error, vid=[%lu]\n",vid);
                    return FALSE;
                }

            } /* end of if */
            return TRAP_UTIL_SendLinkUpTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);

            break;
		}
        case TRAP_EVENT_CRAFT_PORT_LINK_DOWN:
            return TRAP_UTIL_SendCraftLinkDownTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_CRAFT_PORT_LINK_UP:
            return TRAP_UTIL_SendCraftLinkUpTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_AUTHENTICATION_FAILURE:
            return TRAP_UTIL_SendAutehticationFailureTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_NEW_ROOT:
            return TRAP_UTIL_SendNewRootTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_TOPOLOGY_CHANGE:
            return TRAP_UTIL_SendTCNTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_RISING_ALARM:
            return TRAP_UTIL_SendRaisingAlarmTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_FALLING_ALARM:
            return TRAP_UTIL_SendFallingAlarmTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP:
            return TRAP_UTIL_SendPowerStatusChangeTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

        case TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP:
            return TRAP_UTIL_SendActivePowerChangeTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        case TRAP_EVENT_PORT_SECURITY_TRAP:
            return TRAP_UTIL_SendPortSecurityTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        case TRAP_EVENT_LOOPBACK_TEST_FAILURE_TRAP:
            return TRAP_UTIL_SendLoopbackTestFailureTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case TRAP_EVENT_FAN_FAILURE:
            return TRAP_UTIL_SendFanFailureTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
        case TRAP_EVENT_FAN_RECOVER:
            return TRAP_UTIL_SendFanRecoverTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
    case TRAP_EVENT_IPFILTER_REJECT_TRAP:
            return TRAP_UTIL_SendIpFilterRejectTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif
#if (SYS_CPNT_SMTP == TRUE)
    case TRAP_EVENT_SMTP_CONN_FAILURE_TRAP:
            return TRAP_UTIL_SendSmtpConnFailureTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif
#if (SYS_CPNT_POE == TRUE)
    	case TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP:
            return TRAP_UTIL_SendPethPsePortOnOffTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

    	case TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP:
            return TRAP_UTIL_SendPethMainPowerUsageOnTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;

    	case TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP:
            return TRAP_UTIL_SendPethMainPowerUsageOffTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif//end of #if (SYS_CPNT_POE == TRUE)
#if (SYS_CPNT_VDSL == TRUE)
		case TRAP_EVENT_VDSL_PERF_LOFS_THRESH:
			return TRAP_UTIL_SendVdslPerfLofsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
		case TRAP_EVENT_VDSL_PERF_LOSS_THRESH:
			return TRAP_UTIL_SendVdslPerfLossThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;		
		case TRAP_EVENT_VDSL_PERF_LPRS_THRESH:
			return TRAP_UTIL_SendVdslPerfLprsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;		
		case TRAP_EVENT_VDSL_PERF_LOLS_THRESH:
			return TRAP_UTIL_SendVdslPerfLolsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;		
		case TRAP_EVENT_VDSL_PERF_ESS_THRESH:
			return TRAP_UTIL_SendVdslPerfESsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;			
		case TRAP_EVENT_VDSL_PERF_SESS_THRESH:
			return TRAP_UTIL_SendVdslPerfSESsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;		
		case TRAP_EVENT_VDSL_PERF_UASS_THRESH:		
			return TRAP_UTIL_SendVdslPerfUASsThreshTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
            break;
#endif//end of #if (SYS_CPNT_VDSL == TRUE)
case TRAP_EVENT_MAIN_BOARD_VER_MISMATCH:
			return TRAP_UTIL_SendSwMainBoardVerMismatchTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;
        case TRAP_EVENT_MODULE_VER_MISMATCH:
			return TRAP_UTIL_SendSwModuleVerMismatchTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;	
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case TRAP_EVENT_THERMAL_RISING:
			return TRAP_UTIL_SendSwThermalRisingTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;
		case TRAP_EVENT_THERMAL_FALLING:
			return TRAP_UTIL_SendSwThermalFallingTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;
#endif

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
        case TRAP_EVENT_MODULE_INSERTION:
			return TRAP_UTIL_SendSwModuleInsertionTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;
        case TRAP_EVENT_MODULE_REMOVAL:
			return TRAP_UTIL_SendSwModuleRemovalTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;			
#endif
	case TRAP_EVENT_TCN:
			return TRAP_UTIL_TrapReason(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
			break;			
#if (SYS_CPNT_LLDP == TRUE)
    case TRAP_EVENT_LLDP_REM_TABLES_CHANGED:
            return TRAP_UTIL_SendLldpRemTablesChangedTrap(ebuff,
                                        local_ip,
                                        trap_receiver,
                                        trap_data,
                                        system_time);
#endif
        default:
        {
#if (SYS_CPNT_EH == TRUE)
            sprintf(msg, "unknow trap_type=[%lu]",trap_data->trap_type);
            EH_MGR_Handle_Exception1(SYS_MODULE_TRAPMGMT,
                                 0,
                                 EH_TYPE_MSG_DEB_MSG,
                                 (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR),
                                 msg);
#endif//end of #if (SYS_CPNT_EH == TRUE)
            if (debug_mode)
                printf("FUNCTION:TRAP_MGR_ProcessSendTrap; DESC:unknown trap_type=[%lu]\n",trap_data->trap_type);
            owner_info.level = SYSLOG_LEVEL_ERR;
            owner_info.module_no = SYS_MODULE_TRAPMGMT;
            owner_info.function_no = TRAP_MGR_FUNCTION_NUMBER;
            owner_info.error_no = TRAP_MGR_ERROR_NUMBER;
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "TRAP_MGR_ProcessSendTrap", 0, 0);
            return FALSE;
            break;
        }
    }

    return FALSE;

} /* end of TRAP_MGR_ProcessSendTrap() */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_InitDefaultSetting
 * ---------------------------------------------------------------------
 * PURPOSE: This function will set all the config values which are belongs
 *          to TRAP_MGR to default value.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : This is local function and only will be implemented by
 *         TRAP_MGR_EnterMasterMode()
 * ---------------------------------------------------------------------
 */
static void TRAP_MGR_InitDefaultSetting()
{
#if (SYS_CPNT_SNMP_VERSION == 2)
    UI32_T          index;
#endif
        /* BODY */
    trap_socket=-1;
     trap_mgr_link_up_down_traps = TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED;
    trap_mgr_snmp_enable_authen_traps = SYS_DFLT_SNMP_ENABLE_AUTHEN_TRAPS;
    spanning_tree_state = STA_UNSTABLED_STATE;
    up_rif_cnt=0;
    initTrapEs3626a();
    Sys_Object_ID_Init();
    /* Clear all Entry
     */
#if (SYS_CPNT_SNMP_VERSION == 2)
    for (index = 0; index<SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER; index++)
    {
        trap_mgr_trap_receiver[index].trap_dest_address = 0;
        memset(trap_mgr_trap_receiver[index].trap_dest_community, 0, SYS_ADPT_MAX_COMM_STR_NAME_LEN);
        trap_mgr_trap_receiver[index].trap_dest_status = VAL_trapDestStatus_2_invalid;
        trap_mgr_trap_receiver[index].trap_dest_port = SYS_DFLT_TRAP_UDP_PORT;
        trap_mgr_trap_receiver[index].trap_dest_version = SYS_DFLT_TRAP_SNMP_VERSION;
    }
#endif
} /* end of TRAP_MGR_InitDefaultSetting() */



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_TrapLog
 * ---------------------------------------------------------------------
 * PURPOSE: This function will call system log to store all trap request
 *          information to RAM before trap socket is process.
 * INPUT : None
 * OUTPUT: None
 * RETURN: TRUE/FALSE
 * NOTES : None
 * ---------------------------------------------------------------------
 */
static void TRAP_MGR_TrapLog(TRAP_EVENT_TrapData_T  *trap_data)
{
    UI32_T                  unit, port, trunk_id,vid;
    SWCTRL_Lport_Type_T     port_type;
    SYSLOG_OM_RecordOwnerInfo_T       owner_info;
#if (SYS_CPNT_EH == TRUE)
    UI32_T module_id;
    UI32_T function_no;
    UI32_T msg_flag;
    UI8_T  ehmsg[256];
#endif//end of #if (SYS_CPNT_EH == TRUE)
    /* BODY */

    owner_info.level = SYSLOG_LEVEL_INFO;
    owner_info.module_no = SYS_MODULE_TRAPMGMT;
    owner_info.function_no = TRAP_MGR_FUNCTION_NUMBER;
    owner_info.error_no = TRAP_MGR_ERROR_NUMBER;

    switch (trap_data->trap_type)
    {
        case TRAP_EVENT_COLD_START:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, SYSTEM_COLDSTART_MESSAGE_INDEX, 0, 0, 0);
            break;

        case TRAP_EVENT_WARM_START:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, SYSTEM_WARMSTART_MESSAGE_INDEX, 0, 0, 0);
            break;

        case TRAP_EVENT_LINK_DOWN:
            port_type = SWCTRL_LogicalPortToUserPort(trap_data->u.link_down.ifindex, &unit, &port, &trunk_id);

            if (trap_data->u.link_down.ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
            {
                VLAN_MGR_ConvertFromIfindex(trap_data->u.link_down.ifindex, &vid);

                if ((vid < SYS_DFLT_1Q_PORT_VLAN_PVID) || (vid > SYS_ADPT_MAX_VLAN_ID))
                {
                    if (debug_mode)
                        printf("FUNCTION:TRAP_MGR_TrapLog; DESC:vid range error, vid=[%lu]\n",vid);
                    return;
                 }

            } /* end of if */
            if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_DOWN_MESSAGE_INDEX, &unit, &port, 0);
            else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, TRUNK_PORT_LINK_DOWN_MESSAGE_INDEX, &trunk_id, 0, 0);
            else
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VLAN_LINK_DOWN_MESSAGE_INDEX, &vid, 0, 0);
            break;
        case TRAP_EVENT_LINK_UP:
            port_type = SWCTRL_LogicalPortToUserPort(trap_data->u.link_up.ifindex, &unit, &port, &trunk_id);

            if (trap_data->u.link_up.ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
            {
                VLAN_MGR_ConvertFromIfindex(trap_data->u.link_up.ifindex, &vid);

                if ((vid < SYS_DFLT_1Q_PORT_VLAN_PVID) || (vid > SYS_ADPT_MAX_VLAN_ID))
                {
                    if (debug_mode)
                        printf("FUNCTION:TRAP_MGR_TrapLog; DESC:vid range error, vid=[%lu]\n",vid);
                    return;
                 }

            } /* end of if */

            if ((port_type == SWCTRL_LPORT_NORMAL_PORT) || (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, NORMAL_PORT_LINK_UP_MESSAGE_INDEX, &unit, &port, 0);
            else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, TRUNK_PORT_LINK_UP_MESSAGE_INDEX, &trunk_id, 0, 0);
            else
                SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VLAN_LINK_UP_MESSAGE_INDEX, &vid, 0, 0);
            break;
        case TRAP_EVENT_CRAFT_PORT_LINK_DOWN:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, CRAFT_PORT_LINK_DOWN_MESSAGE_INDEX, &trap_data->u.craft_link_down.unit_id, 0, 0);
            break;

        case TRAP_EVENT_CRAFT_PORT_LINK_UP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, CRAFT_PORT_LINK_UP_MESSAGE_INDEX, &trap_data->u.craft_link_up.unit_id, 0, 0);
            break;

        case TRAP_EVENT_AUTHENTICATION_FAILURE:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, AUTHENTICATION_FAILURE_MESSAGE_INDEX, 0, 0, 0);
            break;

        case TRAP_EVENT_NEW_ROOT:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, STA_ROOT_CHANGE_MESSAGE_INDEX, 0, 0, 0);
            break;

        case TRAP_EVENT_TOPOLOGY_CHANGE:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, STA_TOPOLOGY_CHANGE_MESSAGE_INDEX, 0, 0, 0);
            break;

        case TRAP_EVENT_RISING_ALARM:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info,
                                         RMON_RISING_ALARM_MESSAGE_INDEX,
                                         &trap_data->u.rising_alarm.alarm_index,
                                         &trap_data->u.rising_alarm.alarm_value, 0);
            break;

        case TRAP_EVENT_FALLING_ALARM:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info,
                                         RMON_FALLING_ALARM_MESSAGE_INDEX,
                                         &trap_data->u.falling_alarm.alarm_index,
                                         &trap_data->u.falling_alarm.alarm_value, 0);
            break;
        case TRAP_EVENT_SW_POWER_STATUS_CHANGE_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info,
                                         POWER_STATUS_CHANGE_MESSAGE_INDEX,
                                         &trap_data->u.sw_power_status_change_trap.sw_indiv_power_unit_index,
                                         &trap_data->u.sw_power_status_change_trap.sw_indiv_power_index,
                                         &trap_data->u.sw_power_status_change_trap.sw_indiv_power_status);
            break;
        case TRAP_EVENT_SW_ACTIVE_POWER_CHANGE_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info,
                                         ACTIVE_POWER_CHANGE_MESSAGE_INDEX,
                                         &trap_data->u.sw_active_power_change_trap.sw_indiv_power_unit_index,
                                         &trap_data->u.sw_active_power_change_trap.sw_indiv_power_index);
            break;
#if (SYS_CPNT_INTRUSION_MSG_TRAP == TRUE)
        case TRAP_EVENT_PORT_SECURITY_TRAP:
            if (!SWCTRL_LogicalPortExisting(trap_data->u.port_security_trap.ifindex))
            {
#if (SYS_CPNT_EH == TRUE)
                if (EHMsgEnable)
                {
                    EH_MGR_Get_Exception_Info (&module_id, &function_no, &msg_flag, ehmsg, sizeof(ehmsg));
                    printf("EHMsg:moduleId=[%lu],funNo=[%lu]:%s",module_id,function_no,ehmsg);
                }
#endif//end of #if (SYS_CPNT_EH == TRUE)
                if (debug_mode)
                    printf("FUNCTION:TRAP_MGR_ReqSendTrap; DESC:Reject trap request, no such lport, ifindex=[%lu]\n", trap_data->u.port_security_trap.ifindex);
                return;
            }

            port_type = SWCTRL_LogicalPortToUserPort(trap_data->u.port_security_trap.ifindex, &unit, &port, &trunk_id);

            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, PORT_SECURITY_TRAP_INDEX, &unit, &port,trap_data->u.port_security_trap.mac);
            break;
#endif
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)
        case TRAP_EVENT_LOOPBACK_TEST_FAILURE_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, LOOPBACK_TEST_FAILURE_MESSAGE_INDEX, 0, 0, 0);
            break;
#endif
#if (SYS_CPNT_STKTPLG_FAN_DETECT == TRUE)
        case TRAP_EVENT_FAN_FAILURE:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, FAN_FAIL_MESSAGE_INDEX,
                 &trap_data->u.fan_trap.trap_unit_index, &trap_data->u.fan_trap.trap_fan_index, 0);
            break;
        case TRAP_EVENT_FAN_RECOVER:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, FAN_RECOVERY_MESSAGE_INDEX,
                &trap_data->u.fan_trap.trap_unit_index, &trap_data->u.fan_trap.trap_fan_index, 0);
            break;
#endif
#if (SYS_CPNT_MGMT_IP_FLT == TRUE)
        case TRAP_EVENT_IPFILTER_REJECT_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MGMT_IP_FLT_REJECT_MESSAGE_INDEX,
                 &trap_data->u.ipFilter_reject_trap.mode,
                 &trap_data->u.ipFilter_reject_trap.ip, 0);
            break;

        case TRAP_EVENT_IPFILTER_INET_REJECT_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MGMT_IP_FLT_INET_REJECT_MESSAGE_INDEX,
                 &trap_data->u.ipFilterInet_reject_trap.mode
                 &trap_data->u.ipFilterInet_reject_trap.inet_ip, 0);
    		break;
#endif
#if (SYS_CPNT_SMTP == TRUE)
        case TRAP_EVENT_SMTP_CONN_FAILURE_TRAP:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, SMTP_CONN_FAILURE_MESSAGE_INDEX,
                                         &trap_data->u.sw_smtp_conn_failure_trap.smtpServerIp, 0, 0);
            break;
#endif
#if (SYS_CPNT_POE == TRUE)
        case TRAP_EVENT_PETH_PSE_PORT_ON_OFF_TRAP:
        {        	        	
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, PETH_PSE_PORT_ON_OFF_MESSAGE_INDEX,
                &trap_data->u.peth_pse_port_on_off_trap.instance_pethPsePortDetectionStatus[0],
                &trap_data->u.peth_pse_port_on_off_trap.instance_pethPsePortDetectionStatus[1],
                &trap_data->u.peth_pse_port_on_off_trap.pethPsePortDetectionStatus);
            break;
        }
        case TRAP_EVENT_PETH_MAIN_POWER_USAGE_ON_TRAP:
        {        	
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, PETH_MAIN_POWER_USAGE_ON_MESSAGE_INDEX,
                &trap_data->u.peth_main_power_usage_on_trap.instance_pethMainPseConsumptionPower,
                &trap_data->u.peth_main_power_usage_on_trap.pethMainPseConsumptionPower,
                0);
            break;
        }
        case TRAP_EVENT_PETH_MAIN_POWER_USAGE_OFF_TRAP:
        {
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, PETH_MAIN_POWER_USAGE_OFF_MESSAGE_INDEX,
                &trap_data->u.peth_main_power_usage_off_trap.instance_pethMainPseConsumptionPower,
                &trap_data->u.peth_main_power_usage_off_trap.pethMainPseConsumptionPower,
                0);
            break;
        }
#endif //end of #if (SYS_CPNT_POE == TRUE)
#if (SYS_CPNT_VDSL == TRUE)
		case TRAP_EVENT_VDSL_PERF_LOFS_THRESH:
		{
			
			//"vdslPerfLofsThresh:%s,%lu,%lu."	
			UI8_T buffer[20];				
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_lofs_thresh.vdslPerfCurr15MinLofs.high,
			         		 trap_data->u.vdsl_perf_lofs_thresh.vdslPerfCurr15MinLofs.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);			         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_LOFS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_lofs_thresh.instance_vdslPerfCurr15MinLofs[0],
                &trap_data->u.vdsl_perf_lofs_thresh.instance_vdslPerfCurr15MinLofs[1]);
			break;
		}
		case TRAP_EVENT_VDSL_PERF_LOSS_THRESH:
		{
			//"vdslPerfLossThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_loss_thresh.vdslPerfCurr15MinLoss.high,
			         		 trap_data->u.vdsl_perf_loss_thresh.vdslPerfCurr15MinLoss.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_LOSS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_loss_thresh.instance_vdslPerfCurr15MinLoss[0],
                &trap_data->u.vdsl_perf_loss_thresh.instance_vdslPerfCurr15MinLoss[1]);
			break;
		}				
		case TRAP_EVENT_VDSL_PERF_LPRS_THRESH:
		{
			//"vdslPerfLprsThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_lprs_thresh.vdslPerfCurr15MinLprs.high,
			         		 trap_data->u.vdsl_perf_lprs_thresh.vdslPerfCurr15MinLprs.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_LPRS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_lprs_thresh.instance_vdslPerfCurr15MinLprs[0],
                &trap_data->u.vdsl_perf_lprs_thresh.instance_vdslPerfCurr15MinLprs[1]);
			break;
		}		
		case TRAP_EVENT_VDSL_PERF_LOLS_THRESH:
		{
			//"vdslPerfLolsThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_lols_thresh.vdslPerfCurr15MinLols.high,
			         		 trap_data->u.vdsl_perf_lols_thresh.vdslPerfCurr15MinLols.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_LOLS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_lols_thresh.instance_vdslPerfCurr15MinLols[0],
                &trap_data->u.vdsl_perf_lols_thresh.instance_vdslPerfCurr15MinLols[1]);
			break;
		}	
		case TRAP_EVENT_VDSL_PERF_ESS_THRESH:
		{
			//"vdslPerfESsThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_ess_thresh.vdslPerfCurr15MinESs.high,
			         		 trap_data->u.vdsl_perf_ess_thresh.vdslPerfCurr15MinESs.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_ESS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_ess_thresh.instance_vdslPerfCurr15MinESs[0],
                &trap_data->u.vdsl_perf_ess_thresh.instance_vdslPerfCurr15MinESs[1]);
			break;
		}		
		case TRAP_EVENT_VDSL_PERF_SESS_THRESH:
		{
			//"vdslPerfSESsThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_sess_thresh.vdslPerfCurr15MinSESs.high,
			         		 trap_data->u.vdsl_perf_sess_thresh.vdslPerfCurr15MinSESs.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_SESS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_sess_thresh.instance_vdslPerfCurr15MinSESs[0],
                &trap_data->u.vdsl_perf_sess_thresh.instance_vdslPerfCurr15MinSESs[1]);
			break;
		}	
		case TRAP_EVENT_VDSL_PERF_UASS_THRESH:
		{
			//"vdslPerfUASsThresh:%s,%lu,%lu."		
			UI8_T buffer[20];					
			L_STDLIB_UI64toa(trap_data->u.vdsl_perf_uass_thresh.vdslPerfCurr15MinUASs.high,
			         		 trap_data->u.vdsl_perf_uass_thresh.vdslPerfCurr15MinUASs.low,
			         		 buffer);
			L_STDLIB_Trim_Left(buffer, 20);				         		
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, VDSL_PERF_UASS_THRESH_MESSAGE_INDEX,
                buffer,
                &trap_data->u.vdsl_perf_uass_thresh.instance_vdslPerfCurr15MinUASs[0],
                &trap_data->u.vdsl_perf_uass_thresh.instance_vdslPerfCurr15MinUASs[1]);
			break;
		}	
#endif
                case TRAP_EVENT_MAIN_BOARD_VER_MISMATCH:
		{
			//"MainBoardVerMismatch:%lu,%s,%lu,%s."
			UI8_T msg[128];
			sprintf(msg, "MainBoardVerMismatch:%lu,%s,%lu,%s.",
			        trap_data->u.main_board_ver_mismatch.instance_swOpCodeVerMaster[0],
			        trap_data->u.main_board_ver_mismatch.swOpCodeVerMaster,
			        trap_data->u.main_board_ver_mismatch.instance_swOpCodeVerSlave[0],
			        trap_data->u.main_board_ver_mismatch.swOpCodeVerSlave);
			
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MAIN_BOARD_VER_MISMATCH_MESSAGE_INDEX,
                msg, 0, 0);
                   break;
		}
		case TRAP_EVENT_MODULE_VER_MISMATCH:
		{
			//"ModuleVerMismatch:%lu,%s,%lu,%lu,%s."
			UI8_T msg[128];
			sprintf(msg, "ModuleVerMismatch:%lu,%s,%lu,%lu,%s.",
			        trap_data->u.module_ver_mismatch.instance_swModuleExpectedOpCodeVer[0],
			        trap_data->u.module_ver_mismatch.swModuleExpectedOpCodeVer,
			        trap_data->u.module_ver_mismatch.instance_swModuleOpCodeVer[0],
			        trap_data->u.module_ver_mismatch.instance_swModuleOpCodeVer[1],
			        trap_data->u.module_ver_mismatch.swModuleOpCodeVer);
			
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MODULE_VER_MISMATCH_MESSAGE_INDEX,
                msg, 0, 0);
                   break;
		}
#if (SYS_CPNT_THERMAL_DETECT == TRUE)
        case TRAP_EVENT_THERMAL_RISING:
		{
			//"ThermalRising:%lu,%lu,%lu,%lu,%lu,%lu,%lu."
			UI8_T msg[128];
			sprintf(msg, "ThermalRising:%lu,%lu,%ld,%lu,%lu,%lu,%lu.",
			        trap_data->u.thermal_rising.instance_switchThermalTempValue[0],
			        trap_data->u.thermal_rising.instance_switchThermalTempValue[1],
			        trap_data->u.thermal_rising.switchThermalTempValue,
			        trap_data->u.thermal_rising.instance_switchThermalActionRisingThreshold[0],
			        trap_data->u.thermal_rising.instance_switchThermalActionRisingThreshold[1],
			        trap_data->u.thermal_rising.instance_switchThermalActionRisingThreshold[2],
			        trap_data->u.thermal_rising.switchThermalActionRisingThreshold);
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, THERMAL_RISING_MESSAGE_INDEX,
                msg, 0, 0);
		}
		case TRAP_EVENT_THERMAL_FALLING:
		{
			//"ThermalFalling:%lu,%lu,%lu,%lu,%lu,%lu,%lu."
			UI8_T msg[128];
			sprintf(msg, "ThermalFalling:%lu,%lu,%ld,%lu,%lu,%lu,%lu.",
			        trap_data->u.thermal_Falling.instance_switchThermalTempValue[0],
			        trap_data->u.thermal_Falling.instance_switchThermalTempValue[1],
			        trap_data->u.thermal_Falling.switchThermalTempValue,
			        trap_data->u.thermal_Falling.instance_switchThermalActionFallingThreshold[0],
			        trap_data->u.thermal_Falling.instance_switchThermalActionFallingThreshold[1],
			        trap_data->u.thermal_Falling.instance_switchThermalActionFallingThreshold[2],
			        trap_data->u.thermal_Falling.switchThermalActionFallingThreshold);
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, THERMAL_FALLING_MESSAGE_INDEX,
                msg, 0, 0);
                   break;
		}
#endif

#if (SYS_CPNT_MODULE_WITH_CPU == TRUE)
        case TRAP_EVENT_MODULE_INSERTION:
		{
			//"ModuleInsertion:%lu,%lu,%s."
			UI8_T msg[128];
			sprintf(msg, "ModuleInsertion:%lu,%lu,%s.",
			        trap_data->u.module_insertion.instance_swModuleOpCodeVer[0],
			        trap_data->u.module_insertion.instance_swModuleOpCodeVer[1],
			        trap_data->u.module_insertion.swModuleOpCodeVer);
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MODULE_INSERTION_MESSAGE_INDEX,
                msg, 0, 0);
                   break;
		}
		case TRAP_EVENT_MODULE_REMOVAL:
		{			
			//"ModuleRemoval:%lu,%lu,%s."
			UI8_T msg[128];
			sprintf(msg, "ModuleRemoval:%lu,%lu,%s.",
			        trap_data->u.module_removal.instance_swModuleOpCodeVer[0],
			        trap_data->u.module_removal.instance_swModuleOpCodeVer[1],
			        trap_data->u.module_removal.swModuleOpCodeVer);
			SYSLOG_MGR_AddFormatMsgEntry(&owner_info, MODULE_REMOVAL_MESSAGE_INDEX,
                msg, 0, 0);			
		}
#endif
		case TRAP_EVENT_TCN:
		{
		     UI8_T msg[128];	
		
		     switch(trap_data->u.tcn.tcnReason)
		     {
		        case VAL_trapVarTcnReason_pushButton: sprintf(msg, "pushButton(%lu)",trap_data->u.tcn.tcnReason); 		break;
		        case VAL_trapVarTcnReason_stackingLinkDown: sprintf(msg, "stackingLinkDown(%lu)",trap_data->u.tcn.tcnReason); 	break;
		        case VAL_trapVarTcnReason_stackingLinkUp: sprintf(msg, "stackingLinkUp(%lu)",trap_data->u.tcn.tcnReason); 	break;
		        case VAL_trapVarTcnReason_hbtTimeout: sprintf(msg, "hbtTimeout(%lu)",trap_data->u.tcn.tcnReason); 		break;
		        case VAL_trapVarTcnReason_hbtError: sprintf(msg, "hbtError(%lu)",trap_data->u.tcn.tcnReason); 			break;
		        case VAL_trapVarTcnReason_slave: sprintf(msg, "slave(%lu)",trap_data->u.tcn.tcnReason); 			break;
		        default: sprintf(msg, "none(%lu)",trap_data->u.tcn.tcnReason);							break;
		     }
		     SYSLOG_MGR_AddFormatMsgEntry(&owner_info, TCN_MESSAGE_INDEX, msg, 0, 0);
		     break;
		}
#if (SYS_CPNT_LLDP == TRUE)
        case TRAP_EVENT_LLDP_REM_TABLES_CHANGED:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, LLDP_REM_TABLE_CHANGED_INDEX, 0, 0, 0);
        break;
#endif
#if (SYS_CPNT_LLDP_MED == TRUE)
        case TRAP_EVENT_LLDP_MED_TOPOLOGY_CHANGE_DETECTED:
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info,
                                         LLDP_MED_TOPOLOGY_CHANGE_DETECTED_INDEX,
                                         &trap_data->u.lldp_med_topology_change.lldpRemChassisIdSubtype,
                                         trap_data->u.lldp_med_topology_change.lldpRemChassisId,
                                         &trap_data->u.lldp_med_topology_change.lldpMedRemDeviceClass);
            break;
#endif
        default:
            if (debug_mode)
                printf("FUNCTION:TRAP_MGR_TrapLog; DESC:unknow trap type=[%lu]\n",trap_data->trap_type);
            SYSLOG_MGR_AddFormatMsgEntry(&owner_info, SWITCH_TO_DEFAULT_MESSAGE_INDEX, "TRAP_EVENT_TrapLog", 0, 0);
            break;
    } /* end of switch */
    return;
} /* end of TRAP_MGR_TrapLog() */



