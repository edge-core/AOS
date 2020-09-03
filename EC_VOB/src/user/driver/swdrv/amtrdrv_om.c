/*-------------------------------------------------------------------------
 * MODULE NAME: AMTRDRV_OM.C
 *-------------------------------------------------------------------------
 * PURPOSE: To store and manage AMTRDRV Hash Table.
 *          It also provides Hash utility for amtrdrv mgr and task.
 *
 * NOTES:
 *
 * Modification History:
 *      Date          Modifier,   Reason
 *      ------------------------------------
 *      08-31-2004    MIKE_YEH    create
 *
 * COPYRIGHT(C)         Accton Corporation, 2004
 *------------------------------------------------------------------------*/


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysfun.h"
#include "amtrdrv_om.h"
#include "amtrdrv_lib.h"
#include "l_hash.h"
#include "sys_bld.h"
#include "sysrsc_mgr.h"
#include "l_cvrt.h"
#include "amtrdrv_mgr.h"
#if (SYS_CPNT_VXLAN == TRUE)
#include "vxlan_type.h"
#endif
/*----------------
 *  NAME CONSTANT
 *----------------*/
/* constants needed when create hash table */

/* AMTR_TYPE_MAX_LOGICAL_VLAN_ID would contain logical vfi id when SYS_CPNT_VXLAN is TRUE
 */
#if(SYS_CPNT_VXLAN == TRUE)
#define AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER       (SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN \
                                                       + AMTR_TYPE_MAX_LOGICAL_VLAN_ID \
                                                       + AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME \
                                                       + AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE) /*this variable to have extra space to store group operation event */
#else
#define AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER       (SYS_ADPT_TOTAL_NBR_OF_LPORT \
                                                       + AMTR_TYPE_MAX_LOGICAL_VLAN_ID \
                                                       + AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME \
                                                       + AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE) /*this variable to have extra space to store group operation event */
#endif
#define AMTRDRV_OM_NUMBER_OF_HASH_BUCKET              1024
#define AMTRDRV_OM_NUMBER_OF_NA_BUCKET                64
#define AMTRDRV_OM_NUMBER_OF_NA_RECORD                512
/* Each unit will send AMTRDRV_OM_NUM_ISC_IN_PROCESS in one processing.
 * And reserve one space for each unit.
 */
#define AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER    ((SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK    \
                                                        *AMTRDRV_TYPE_NUM_ISC_IN_PROCESS        \
                                                        +SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)  \
                                                        *AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET)
#define AMTRDRV_OM_SYNC_VALUE_SYNC_QUEUE              BIT_0
#define AMTRDRV_OM_SYNC_VALUE_CALLBACK_QUEUE          BIT_1
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
#define AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX             ((UI32_T)-2)
#define AMTRDRV_OM_SEARCH_END                         ((UI32_T)-1)
/* use AMTRDRV_OM_LocalDelAddr() instead of AMTRDRV_OM_LocalDelAddr()
 * AMTR have had more checking before deletion.
 */
#define AMTRDRV_OM_LocalDelAddr(desc, user_rec,index) AMTRDRV_OM_DelHashRecord(desc, user_rec,index)
#else
#define AMTRDRV_OM_LocalDelAddr(desc, user_rec,index) L_HASH_ShMem_DeleteRecord(desc, user_rec,index)
#endif

/*----------------
 * MACRO FUNCTION
 *----------------*/
#define AMTRDRV_OM_SIZEOF(type, field)                sizeof (_##type->field)
//#define AMTRDRV_OM_OFFSET(offset, type, field)        {type v; offset=(UI32_T)&v.##field - (UI32_T)&v;}
#define AMTRDRV_OM_OFFSET_MAC(offset, type)        {type v; offset=(UI8_T *)&v.address.mac - (UI8_T *)&v;}
#define AMTRDRV_OM_OFFSET_VID(offset, type)        {type v; offset=(UI8_T *)&v.address.vid - (UI8_T *)&v;}

/*-----------------
 * LOCAL TYPES
 *----------------*/
/* This data structure is to store the information for callback function using  */
typedef struct
{
    void *arg1;   /* vid or is_update_local_aging_list */
    void *arg2;   /* Source or life_time */
    void *arg3;   /* vlan list */
    void *arg4;   /* add for del_by_port+vid_except_certain_addr */
}AMTRDRV_OM_Cookie_T;

/* This data structure is to store aged out address information */
typedef struct
{
    UI16_T   ifindex;
    UI16_T   vid;
    UI8_T    mac[AMTR_TYPE_MAC_LEN];
}AMTRDRV_OM_AgingOutEntry_T;

typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T  is_allocated;
    BOOL_T  is_asic_command_transition_done;
    BOOL_T  is_address_transition_done;
    BOOL_T  is_provision_completed;
    AMTRDRV_OM_SyncState_T          is_synchronizing;
    UI8_T   synq_debug_info; /* for AMTRDRV backdoor, see AMTRDRV_OM_SYNCQ_DBG_FLAG_XXX for defined constants*/
    UI32_T  amtrdrv_asic_command_task_tid;
    UI32_T  amtrdrv_address_task_tid;
    UI32_T  amtr_address_task_tid;

    AMTR_TYPE_Counters_T counters;

    /* Variables used for keeping the address table information
     */
    L_HASH_ShMem_Desc_T              amtrdrv_om_hash_desc;
    UI32_T                           amtrdrv_om_hash_buffer_sz;
    L_DLST_ShMem_Indexed_Dblist_T    sync_queue_desc;
    UI32_T                           sync_queue_buffer_sz;
    L_DLST_ShMem_Indexed_Dblist_T    call_back_queue_desc;
    UI32_T                           call_back_queue_buffer_sz;

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    /* Variables used for creating temp buffer to store agingout entries
     */
    L_HASH_ShMem_Desc_T              amtrdrv_om_na_buffer_desc;
    UI32_T                           amtrdrv_om_na_buffer_buffer_sz;
    L_DLST_ShMem_Indexed_Dblist_T    local_aging_check_list_desc;
    UI32_T                           local_aging_check_list_buffer_sz;
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
	/*add a new list for making sure the aging MAC can be deleted.
	* the idea that  the database is located  in share memory may be not good. Master will spend more memory
	* But follow the original.
	*In  the c/s design between master and slave,the lost of message can not   be avioded . The acknowledge mechanism is needed.
	* We must guarantee the arriving of the message ,and aslo ensure to aviod the rabish message
	* the probility of losing message is low . So the default size is 512.
	* Modified by  Tony.Lei 
	*/
	L_DLST_ShMem_Indexed_Dblist_T    slave_aging_ak_list_desc;
	UI32_T                           slave_aging_ak_list_buffer_sz;
#endif	
    AMTRDRV_OM_AgingOutEntry_T       amtrdrv_om_agingout_buffer[AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER];
    UI32_T                           amtrdrv_om_agingout_buffer_head;
    UI32_T                           amtrdrv_om_agingout_buffer_tail;
    UI32_T                           amtrdrv_om_agingout_counter;
#else
    /* AMTRDRV task query del_on_timeout group to search marked entry.
     * This static variable will keep a index which will be checked next time.
     */
    UI32_T                           amtrdrv_om_wait_age_out_checking_index;
    /* the dblist of del_on_timeout query group
     */
    L_DLST_ShMem_Indexed_Dblist_T       *amtrdrv_om_dyn_dblist;
    //UI32_T                              amtrdrv_om_dyn_buffer_sz;
#endif
    AMTR_TYPE_BlockedCommand_T        amtrdrv_om_blocked_command;
    UI32_T amtrdrv_mgr_oper_aging_time;       /* the variable operates the aging out mechanism, like as operation. */
    UI32_T amtrdrv_mgr_my_driver_unit_id;

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
    AMTRDRV_OM_CollisionMacEntry_T amtrdrv_collision_mac_table[SYS_ADPT_MAX_NBR_OF_AMTRDRV_COLLISION_MAC_ENTRY];
    UI32_T  amtrdrv_collision_mac_num;
#endif
#if (SYS_CPNT_VXLAN == TRUE && (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR))
    #if (SYS_ADPT_VXLAN_MAX_NBR_VNI>0xFFFF)
        #error "Data type of amtrdrv_om_vxlan_lvfi_map is UI16 so SYS_ADPT_VXLAN_MAX_NBR_VNI cannot be larger than 0xFFFF"
    #endif
    UI16_T amtrdrv_om_vxlan_lvfi_map[SYS_ADPT_VXLAN_MAX_NBR_VNI]; /* amtrdrv_om_vxlan_lvfi_map[lvfi]=rvfi */
#endif

} AMTRDRV_Shmem_Data_T;
/*when u add the define , ur must care for the init order , make sure the  shmem_data_p->xxx  has assigned 
 *  Tony.lei
 */
#define AMTRDRV_OM_HASH_BUFFER_OFFSET        sizeof(AMTRDRV_Shmem_Data_T)
#define SYNC_QUEUE_BUFFER_OFFSET             (AMTRDRV_OM_HASH_BUFFER_OFFSET+shmem_data_p->amtrdrv_om_hash_buffer_sz)
#define CALL_BACK_QUEUE_BUFFER_OFFSET        (SYNC_QUEUE_BUFFER_OFFSET + shmem_data_p->sync_queue_buffer_sz)
#define LOCAL_AGING_CHECK_LIST_BUFFER_OFFSET (CALL_BACK_QUEUE_BUFFER_OFFSET + shmem_data_p->call_back_queue_buffer_sz)
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
/*add by Tony.Lei*/
#define AMTRDRV_OM_SLAVE_AGINGOUT_BUFFER_BUFFER_OFFSET   (LOCAL_AGING_CHECK_LIST_BUFFER_OFFSET + shmem_data_p->local_aging_check_list_buffer_sz)
#define AMTRDRV_OM_NA_BUFFER_BUFFER_OFFSET   (AMTRDRV_OM_SLAVE_AGINGOUT_BUFFER_BUFFER_OFFSET + shmem_data_p->slave_aging_ak_list_buffer_sz)
#else
#define AMTRDRV_OM_NA_BUFFER_BUFFER_OFFSET   (LOCAL_AGING_CHECK_LIST_BUFFER_OFFSET + shmem_data_p->local_aging_check_list_buffer_sz)
#endif
//#define AMTRDRV_OM_DYN_BUFFER_OFFSET         (AMTRDRV_OM_NA_BUFFER_BUFFER_OFFSET + shmem_data_p->call_back_queue_buffer_sz)

/******************
 LOCAL SUBROUTINES
 ******************/
static void    AMTRDRV_OM_GetRecordByIndex(UI32_T index,UI8_T *addr_entry,UI8_T *action);
static void    AMTRDRV_OM_InitCount(void);
static UI32_T  AMTRDRV_OM_UpdateRecordLifeTime_Callback(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteAddrCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteSourceCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteLifeTimeCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteVidCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteVidAndLifeTimeCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteVidAndSourceCallBack(L_HASH_Index_T index, void *cookie);
static UI32_T  AMTRDRV_OM_DeleteVidExceptCertainAddrCallback(L_HASH_Index_T index, void *cookie);
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
static UI32_T AMTRDRV_OM_MarkAddrCallBack(L_HASH_Index_T index, void *cookie);
static BOOL_T AMTRDRV_OM_DelHashRecord (L_HASH_ShMem_Desc_T *desc, UI8_T  *user_rec, L_HASH_Index_T *index);
#endif
static BOOL_T _AMTRDRV_OM_SyncQueueEnqueue(UI32_T index);
static void   _AMTRDRV_OM_SyncQueueSetEntryID(UI32_T index,UI32_T dlist_id);

#ifdef BACKDOOR_OPEN
static UI32_T  AMTRDRV_OM_ShowAddrCallBack(L_HASH_Index_T index, void *cookie); /* This API currently is supported for AMTRDRV_MGR backdoor to show address info only */
#endif

/***************
 LOCAL VARIABLES
 ***************/
/* Variable for macro using */
static AMTRDRV_TYPE_Record_T           *_AMTRDRV_TYPE_Record_T;

static AMTRDRV_Shmem_Data_T   *shmem_data_p;
static UI32_T                  amtrdrv_om_sem_id;
/*add by tony.lei*/
static UI32_T                  amtrdrv_om_enque_deque_sem_id;
#define AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(amtrdrv_om_enque_deque_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(amtrdrv_om_enque_deque_sem_id)
#define AMTRDRV_OM_END_DE_ATOM_EXPRESSION(exp) { AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();\
                                          exp;\
                                          AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();\
                                        }

#define AMTRDRV_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(amtrdrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define AMTRDRV_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(amtrdrv_om_sem_id)
#define AMTRDRV_OM_ATOM_EXPRESSION(exp) { AMTRDRV_OM_ENTER_CRITICAL_SECTION();\
                                          exp;\
                                          AMTRDRV_OM_LEAVE_CRITICAL_SECTION();\
                                        }

/*--------------------
 * EXPORT SUBROUTINES
 *--------------------*/

SYS_TYPE_Stacking_Mode_T AMTRDRV_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

UI32_T AMTRDRV_OM_GetAmtrID()
{
    return shmem_data_p->amtr_address_task_tid;
}

void AMTRDRV_OM_SetAmtrID(UI32_T tid)
{
    shmem_data_p->amtr_address_task_tid = tid;
}

void AMTRDRV_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
	shmem_data_p->is_asic_command_transition_done=TRUE;
	shmem_data_p->is_address_transition_done=TRUE;
}

void AMTRDRV_OM_SetAsicCommandTransitionDone(void)
{
	shmem_data_p->is_asic_command_transition_done=TRUE;
}

void AMTRDRV_OM_SetAddressTransitionDone(void)
{
	shmem_data_p->is_address_transition_done=TRUE;
}

void AMTRDRV_OM_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
	//SYSFUN_TASK_ENTER_TRANSITION_MODE(shmem_data_p->is_asic_command_transition_done);
	//SYSFUN_TASK_ENTER_TRANSITION_MODE(shmem_data_p->is_address_transition_done);
}

void AMTRDRV_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
}

void AMTRDRV_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
}

void AMTRDRV_OM_SetProvisionComplete(BOOL_T provision_complete)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->is_provision_completed = provision_complete);
    return;
}

BOOL_T AMTRDRV_OM_GetProvisionComplete(void)
{
    return shmem_data_p->is_provision_completed;
}

void AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SyncState_T synchronizing)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->is_synchronizing = synchronizing);
    return;    
}

AMTRDRV_OM_SyncState_T AMTRDRV_OM_GetSynchronizing(void)
{
    return shmem_data_p->is_synchronizing;
}

UI32_T AMTRDRV_OM_GetAsicComTaskId(void)
{
    return shmem_data_p->amtrdrv_asic_command_task_tid;
}

void AMTRDRV_OM_SetAsicComTaskId(UI32_T tid)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->amtrdrv_asic_command_task_tid = tid);
    return;
}

UI32_T AMTRDRV_OM_GetAddressTaskId(void)
{
    return shmem_data_p->amtrdrv_address_task_tid;
}

void AMTRDRV_OM_SetAddressTaskId(UI32_T tid)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->amtrdrv_address_task_tid = tid);
    return;
}

UI32_T AMTRDRV_OM_GetOperAgingTime(void)
{
    return shmem_data_p->amtrdrv_mgr_oper_aging_time;
}

void AMTRDRV_OM_SetOperAgingTime(UI32_T value)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->amtrdrv_mgr_oper_aging_time = value);
    return;
}

UI32_T AMTRDRV_OM_GetMyDrvUnitId(void)
{
    return shmem_data_p->amtrdrv_mgr_my_driver_unit_id;
}

void AMTRDRV_OM_SetMyDrvUnitId(UI32_T value)
{
    AMTRDRV_OM_ATOM_EXPRESSION(shmem_data_p->amtrdrv_mgr_my_driver_unit_id = value);
    return;
}

AMTR_TYPE_BlockedCommand_T * AMTRDRV_OM_GetBlockCommand(AMTR_TYPE_BlockedCommand_T *block_command_p)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if(block_command_p)
        memcpy(block_command_p,&(shmem_data_p->amtrdrv_om_blocked_command),sizeof(AMTR_TYPE_BlockedCommand_T));
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    
    return &(shmem_data_p->amtrdrv_om_blocked_command) ;
}

void AMTRDRV_OM_SetBlockCommand(AMTR_TYPE_BlockedCommand_T *block_command_p)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    memcpy(&(shmem_data_p->amtrdrv_om_blocked_command),block_command_p,sizeof(AMTR_TYPE_BlockedCommand_T));
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return;
}

BOOL_T AMTRDRV_OM_IsBlockCommandNull(void)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if (shmem_data_p->amtrdrv_om_blocked_command.blocked_command == AMTR_TYPE_COMMAND_NULL)
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
    else
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

void AMTRDRV_OM_SetBlockCommandNull(void)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->amtrdrv_om_blocked_command.blocked_command = AMTR_TYPE_COMMAND_NULL;
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

void AMTRDRV_OM_InitiateSystemResources(void)
{
    shmem_data_p = (AMTRDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_AMTRDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTRDRV_OM, &amtrdrv_om_sem_id);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NAQUEUE_OM, &amtrdrv_om_enque_deque_sem_id);
}

void AMTRDRV_OM_AttachSystemResources(void)
{
    shmem_data_p = (AMTRDRV_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_AMTRDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AMTRDRV_OM, &amtrdrv_om_sem_id);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_NAQUEUE_OM, &amtrdrv_om_enque_deque_sem_id);
}
void AMTRDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    L_HASH_ShMem_Desc_T amtrdrv_om_hash_desc;
    L_HASH_ShMem_Desc_T amtrdrv_om_na_buffer_desc;
    UI32_T        shmem_buffer_size = sizeof(AMTRDRV_Shmem_Data_T);

    /* 1. Initialize OM hash table */
    memset(&amtrdrv_om_hash_desc,0,sizeof(amtrdrv_om_hash_desc));
    amtrdrv_om_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY;
    amtrdrv_om_hash_desc.nbr_of_hash_bucket = AMTRDRV_OM_NUMBER_OF_HASH_BUCKET;
    AMTRDRV_OM_OFFSET_MAC(amtrdrv_om_hash_desc.key_offset[0], AMTRDRV_TYPE_Record_T);
    amtrdrv_om_hash_desc.key_size[0] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.mac);
    AMTRDRV_OM_OFFSET_VID(amtrdrv_om_hash_desc.key_offset[1], AMTRDRV_TYPE_Record_T);
    amtrdrv_om_hash_desc.key_size[1] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.vid);
    amtrdrv_om_hash_desc.element_of_query_group[0] = AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME;
#if(SYS_CPNT_VXLAN == TRUE)
    amtrdrv_om_hash_desc.element_of_query_group[1] = SYS_ADPT_TOTAL_NBR_OF_LPORT_INCLUDE_VXLAN+1;
#else
    amtrdrv_om_hash_desc.element_of_query_group[1] = SYS_ADPT_TOTAL_NBR_OF_LPORT+1;
#endif
    amtrdrv_om_hash_desc.element_of_query_group[2] = AMTR_TYPE_MAX_LOGICAL_VLAN_ID;
    amtrdrv_om_hash_desc.element_of_query_group[3] = AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE;
    amtrdrv_om_hash_desc.acquire_element_index_of_query_group_fun_id = L_HASH_SHMEM_ACQUIRE_TYPE_AMTRDRV_BYGROUP;
    amtrdrv_om_hash_desc.record_size = sizeof(AMTRDRV_TYPE_Record_T);
    amtrdrv_om_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;
  
    /* A.  plus the size of OM hash table */
    shmem_buffer_size += L_HASH_ShMem_GetWorkingBufferRequiredSize(&amtrdrv_om_hash_desc);
	/* B. plus the size of local aging check list*/
    shmem_buffer_size += L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(amtrdrv_om_hash_desc.nbr_of_rec);
	/* C. plus the size of  sync queue */
    shmem_buffer_size += L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(amtrdrv_om_hash_desc.nbr_of_rec+AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER);
    /* D. plus the size of  Callback queue */
	shmem_buffer_size += L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(amtrdrv_om_hash_desc.nbr_of_rec+AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
	/* E. plus the size of  slave	aging check list */
	shmem_buffer_size += L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(amtrdrv_om_hash_desc.nbr_of_rec);
#endif	
    /* 1. Initiate hash_descriptor */
    memset(&amtrdrv_om_na_buffer_desc,0,sizeof(amtrdrv_om_na_buffer_desc));
    amtrdrv_om_na_buffer_desc.nbr_of_rec = AMTRDRV_OM_NUMBER_OF_NA_RECORD;
    amtrdrv_om_na_buffer_desc.nbr_of_hash_bucket = AMTRDRV_OM_NUMBER_OF_NA_BUCKET;
    AMTRDRV_OM_OFFSET_MAC(amtrdrv_om_na_buffer_desc.key_offset[0], AMTRDRV_TYPE_Record_T);
    amtrdrv_om_na_buffer_desc.key_size[0] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.mac);
    AMTRDRV_OM_OFFSET_VID(amtrdrv_om_na_buffer_desc.key_offset[1], AMTRDRV_TYPE_Record_T);
    amtrdrv_om_na_buffer_desc.key_size[1] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.vid);
    amtrdrv_om_na_buffer_desc.acquire_element_index_of_query_group_fun_id = 0;
    amtrdrv_om_na_buffer_desc.record_size = sizeof(AMTRDRV_TYPE_Record_T);
    amtrdrv_om_na_buffer_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;
    /*F:plus the size of NA buffer */
    shmem_buffer_size += L_HASH_ShMem_GetWorkingBufferRequiredSize(&amtrdrv_om_na_buffer_desc);

    *segid_p = SYSRSC_MGR_AMTRDRV_SHMEM_SEGID;
    *seglen_p = shmem_buffer_size;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_L2TableInit
 * -------------------------------------------------------------------------
 * PURPOSE: This function is to create amtrdrv_om(Hash table).
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_L2TableInit(void)
{
    /* A. L2 Database */
    memset(&(shmem_data_p->amtrdrv_om_hash_desc),0,sizeof(shmem_data_p->amtrdrv_om_hash_desc));
    /* 1. Initialize OM hash table */
    shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec = SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY;
    shmem_data_p->amtrdrv_om_hash_desc.nbr_of_hash_bucket = AMTRDRV_OM_NUMBER_OF_HASH_BUCKET;
    AMTRDRV_OM_OFFSET_MAC(shmem_data_p->amtrdrv_om_hash_desc.key_offset[0], AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_hash_desc.key_size[0] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.mac);
    AMTRDRV_OM_OFFSET_VID(shmem_data_p->amtrdrv_om_hash_desc.key_offset[1], AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_hash_desc.key_size[1] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.vid);
    shmem_data_p->amtrdrv_om_hash_desc.element_of_query_group[0] = AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME;
    shmem_data_p->amtrdrv_om_hash_desc.element_of_query_group[1] = AMTR_TYPE_MAX_LOGICAL_PORT_ID+1;
    shmem_data_p->amtrdrv_om_hash_desc.element_of_query_group[2] = AMTR_TYPE_MAX_LOGICAL_VLAN_ID;
    shmem_data_p->amtrdrv_om_hash_desc.element_of_query_group[3] = AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE;
    shmem_data_p->amtrdrv_om_hash_desc.acquire_element_index_of_query_group_fun_id = L_HASH_SHMEM_ACQUIRE_TYPE_AMTRDRV_BYGROUP;
    shmem_data_p->amtrdrv_om_hash_desc.record_size = sizeof(AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_hash_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;

    shmem_data_p->amtrdrv_om_hash_desc.buffer_offset = AMTRDRV_OM_HASH_BUFFER_OFFSET - L_CVRT_GET_OFFSET(shmem_data_p, &shmem_data_p->amtrdrv_om_hash_desc);

    shmem_data_p->amtrdrv_om_hash_buffer_sz = L_HASH_ShMem_GetWorkingBufferRequiredSize(&shmem_data_p->amtrdrv_om_hash_desc);

    /* 2. Create OM hash table */
    if (!L_HASH_ShMem_Create(&(shmem_data_p->amtrdrv_om_hash_desc)))
    {
        printf("%s: L_HASH_ShMem_Create return FALSE !\n",__FUNCTION__);
        return FALSE;
    }
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    shmem_data_p->local_aging_check_list_buffer_sz = L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
    /*add by Tony.lei*/
	shmem_data_p->slave_aging_ak_list_buffer_sz = L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec);
#endif
#endif
    shmem_data_p->sync_queue_buffer_sz = L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec + AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER);
    shmem_data_p->call_back_queue_buffer_sz = L_DLST_SHMEM_GET_WORKING_BUFFER_REQUIRED_SZ(shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec + AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER);

    /* C. Create sync queue */
    if(!L_DLST_ShMem_Indexed_Dblist_Create(&shmem_data_p->sync_queue_desc,
                                     L_CVRT_GET_PTR(shmem_data_p, SYNC_QUEUE_BUFFER_OFFSET),
                                     shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec+AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER))
    {
        return FALSE;
    }

    /* D. Create Callback queue */
    if(!L_DLST_ShMem_Indexed_Dblist_Create(&shmem_data_p->call_back_queue_desc,
                                     L_CVRT_GET_PTR(shmem_data_p, CALL_BACK_QUEUE_BUFFER_OFFSET),
                                     shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec+AMTRDRV_OM_NUMBER_OF_GROUP_EVENT_BUFFER))
    {
        return FALSE;
    }
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        /* B. Create local aging check list */
        if(!L_DLST_ShMem_Indexed_Dblist_Create(&shmem_data_p->local_aging_check_list_desc,
                                         L_CVRT_GET_PTR(shmem_data_p, LOCAL_AGING_CHECK_LIST_BUFFER_OFFSET),
                                         shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec))
        {
            return FALSE;
        }
#endif
#if ((SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)&&(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1))
	/* E. Create slave  aging check list */
	if(!L_DLST_ShMem_Indexed_Dblist_Create(&shmem_data_p->slave_aging_ak_list_desc,
                                               L_CVRT_GET_PTR(shmem_data_p, MTRDRV_OM_SLAVE_AGINGOUT_BUFFER_BUFFER_OFFSET),
                                               shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec))
	{
		return FALSE;
	}
#endif

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    if(!L_HASH_ShMem_GetQueryGroupDescriptor(&shmem_data_p->amtrdrv_om_hash_desc,
                                        AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,
                                        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT-1,
                                        &shmem_data_p->amtrdrv_om_dyn_dblist))
    {
        return FALSE;
    }
    shmem_data_p->amtrdrv_om_dyn_dblist = (L_DLST_ShMem_Indexed_Dblist_T*)L_CVRT_GET_OFFSET(shmem_data_p, shmem_data_p->amtrdrv_om_dyn_dblist);
#endif
    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_NABufferInit
 * -------------------------------------------------------------------------
 * PURPOSE: This function is to create NA buffer(Hash table).
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_NABufferInit(void)
{
    /* A. NA buffer */
    memset(&shmem_data_p->amtrdrv_om_na_buffer_desc,0,sizeof(shmem_data_p->amtrdrv_om_na_buffer_desc));
    /* 1. Initiate hash_descriptor */
    shmem_data_p->amtrdrv_om_na_buffer_desc.nbr_of_rec = AMTRDRV_OM_NUMBER_OF_NA_RECORD;
    shmem_data_p->amtrdrv_om_na_buffer_desc.nbr_of_hash_bucket = AMTRDRV_OM_NUMBER_OF_NA_BUCKET;
    AMTRDRV_OM_OFFSET_MAC(shmem_data_p->amtrdrv_om_na_buffer_desc.key_offset[0], AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_na_buffer_desc.key_size[0] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.mac);
    AMTRDRV_OM_OFFSET_VID(shmem_data_p->amtrdrv_om_na_buffer_desc.key_offset[1], AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_na_buffer_desc.key_size[1] = AMTRDRV_OM_SIZEOF(AMTRDRV_TYPE_Record_T, address.vid);
    shmem_data_p->amtrdrv_om_na_buffer_desc.acquire_element_index_of_query_group_fun_id = 0;
    shmem_data_p->amtrdrv_om_na_buffer_desc.record_size = sizeof(AMTRDRV_TYPE_Record_T);
    shmem_data_p->amtrdrv_om_na_buffer_desc.hash_method = L_HASH_HASH_METHOD_WORD_XOR;
    shmem_data_p->amtrdrv_om_na_buffer_desc.buffer_offset = AMTRDRV_OM_NA_BUFFER_BUFFER_OFFSET  - L_CVRT_GET_OFFSET(shmem_data_p, &shmem_data_p->amtrdrv_om_na_buffer_desc);
    shmem_data_p->amtrdrv_om_na_buffer_buffer_sz = L_HASH_ShMem_GetWorkingBufferRequiredSize(&shmem_data_p->amtrdrv_om_na_buffer_desc);

    /* 2. Create NA hash buffer */
    if (!L_HASH_ShMem_Create(&shmem_data_p->amtrdrv_om_na_buffer_desc))
    {
        printf("AMTRDRV: Create NA buffer Error !\n");
        return FALSE;
    }
    return TRUE;
}
#ifdef AMTRDRV_SLAVE_REPEATING_TEST
/*add by Tony.Lei */
typedef struct 
{
    UI16_T vid;
    UI16_T ifindex;
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];
}__attribute__((packed, aligned(1)))AMTR_TYPE_NARESTORE_T;

AMTR_TYPE_NARESTORE_T amtr_type_narestore;
static L_HASH_Desc_T amtrdrv_om_na_buffer_restore_desc;
static UI32_T   amtrdrv_om_nabuffer_restore_sem_id;
#define AMTRDRV_OM_NA_RESTORE_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(amtrdrv_om_nabuffer_restore_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define AMTRDRV_OM_NA_RESTORE_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(amtrdrv_om_nabuffer_restore_sem_id)

BOOL_T AMTRDRV_OM_InitNABufferRestore(void)
{
	memset(&amtrdrv_om_na_buffer_restore_desc, 0, sizeof(amtrdrv_om_na_buffer_restore_desc));

    /* 1. Initialize OM hash table */
    amtrdrv_om_na_buffer_restore_desc.nbr_of_rec		= SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY;
    amtrdrv_om_na_buffer_restore_desc.nbr_of_hash_bucket = AMTRDRV_OM_NUMBER_OF_HASH_BUCKET;
    amtrdrv_om_na_buffer_restore_desc.key_offset[0] = (UI32_T)&amtr_type_narestore.mac - (UI32_T)&amtr_type_narestore ;
    shmem_data_p->amtrdrv_om_na_buffer_desc.key_size[0] = 6;
    amtrdrv_om_na_buffer_restore_desc.key_offset[1] =  (UI32_T)&amtr_type_narestore.vid - (UI32_T)&amtr_type_narestore;
    shmem_data_p->amtrdrv_om_na_buffer_desc.key_size[1] = 2;
    amtrdrv_om_na_buffer_restore_desc.key_offset[2] = (UI32_T)&amtr_type_narestore.ifindex -(UI32_T)&amtr_type_narestore ;
    shmem_data_p->amtrdrv_om_na_buffer_desc.key_size[1] = 2;
    amtrdrv_om_na_buffer_restore_desc.record_size 		=  sizeof(AMTR_TYPE_NARESTORE_T);
    amtrdrv_om_na_buffer_restore_desc.hash_method 		= L_HASH_HASH_METHOD_WORD_XOR;
	SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &amtrdrv_om_nabuffer_restore_sem_id);

	/* 2. Create OM hash table */
    if (FALSE == L_HASH_Create(&amtrdrv_om_na_buffer_restore_desc))
    {
        printf("\r\n[%s]: Create amtrdrv_om_na_buffer_restore_desc  Database Error !", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}
BOOL_T AMTRDRV_OM_NABufferRestoreSet(UI8_T *addr_entry, UI32_T *index)
{
    /* Local Variable Declaration
      */
    BOOL_T  rc=FALSE;

    AMTRDRV_OM_NA_RESTORE_ENTER_CRITICAL_SECTION();
    /* add address entry to hash table
     */
    ((rc = L_HASH_SetRecord(&amtrdrv_om_na_buffer_restore_desc,addr_entry,index)));

    AMTRDRV_OM_NA_RESTORE_LEAVE_CRITICAL_SECTION();

    return rc;
}
BOOL_T AMTRDRV_OM_NABufferRestoreGetExactRecord(UI8_T *addr_entry)
{
    BOOL_T rc = FALSE;

    AMTRDRV_OM_NA_RESTORE_ENTER_CRITICAL_SECTION();
    rc = L_HASH_GetExactRecord(&amtrdrv_om_na_buffer_restore_desc,addr_entry);
    AMTRDRV_OM_NA_RESTORE_LEAVE_CRITICAL_SECTION();
    return rc;
}

BOOL_T AMTRDRV_OM_NABufferRestoreDeleteRecord(UI8_T *addr_entry)
{
    BOOL_T rc = FALSE;
    L_HASH_Index_T index;
    AMTRDRV_OM_NA_RESTORE_ENTER_CRITICAL_SECTION();
    rc = L_HASH_DeleteRecord(&amtrdrv_om_na_buffer_restore_desc,addr_entry,&index);
    AMTRDRV_OM_NA_RESTORE_LEAVE_CRITICAL_SECTION();
    return rc;
}

#endif
#endif

/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearDatabase()
 * -------------------------------------------------------------------------
 * PURPOSE: Clear all databases -- Hash table , aging_list , synq_list & callbackq_list.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_ClearDatabase(void)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    /* A. clean L2 Hash table */
    L_HASH_ShMem_DeleteAll(&shmem_data_p->amtrdrv_om_hash_desc);

    /* B. initialize count */
    AMTRDRV_OM_InitCount();

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    /* C. reset aging out buffer */
    shmem_data_p->amtrdrv_om_agingout_buffer_head = 0;
    shmem_data_p->amtrdrv_om_agingout_buffer_tail = 0;
    shmem_data_p->amtrdrv_om_agingout_counter     = 0;
    memset(shmem_data_p->amtrdrv_om_agingout_buffer,0, AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER);

    /* D. clean local checking aging out list */
    L_DLST_ShMem_Indexed_Dblist_DeleteAll_ByListArray(&shmem_data_p->local_aging_check_list_desc);

#else
    shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX;
#endif

    /* E. clean sync queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteAll_ByListArray(&shmem_data_p->sync_queue_desc);

    /* F. clean call back queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteAll_ByListArray(&shmem_data_p->call_back_queue_desc);
    /*G: clean the slave aging*/
#if( (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)&&(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1))
   L_DLST_ShMem_Indexed_Dblist_DeleteAll_ByListArray(&shmem_data_p->slave_aging_ak_list_desc);
#endif
#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
    shmem_data_p->amtrdrv_collision_mac_num = 0;
#endif

#if (SYS_CPNT_VXLAN == TRUE && (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR))
    memset(shmem_data_p->amtrdrv_om_vxlan_lvfi_map, 0, sizeof(shmem_data_p->amtrdrv_om_vxlan_lvfi_map));
#endif

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearNABuffer()
 * -------------------------------------------------------------------------
 * PURPOSE: Clear NA buffer.
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_ClearNABuffer(void)
{
    AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
    /* A. clean NA hash buffer */
    L_HASH_ShMem_DeleteAll(&shmem_data_p->amtrdrv_om_na_buffer_desc);
    AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();

    return TRUE;
}
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SetAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set address table entry into Hash table & chip
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : index -- hash_record_index
 * RETURN : BOOL_T Status             - True : successs, False : failed
 * NOTES  : This function will set the record into OM and through OM's FSM to
 *          program chip.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SetAddr(UI8_T *addr_entry, UI32_T *index)
{
    /* Local Variable Declaration
      */
    BOOL_T  rc=FALSE;
    L_HASH_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* add address entry to hash table
     */
    if((rc = L_HASH_ShMem_SetRecord(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry,&tmp_index)))
    {
        AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T*)addr_entry, TRUE, &(shmem_data_p->counters));
        L_HASH_ShMem_ClearReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,AMTR_TYPE_COUNTER_FLAGS|AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS);
        SYSFUN_STATIC_ASSERT(sizeof(UI32_T)>=sizeof(L_HASH_Index_T), "Data type of index is not big enough to keep entire value of tmp_index");
        *index = tmp_index;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SetAddrHashOnly
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set address entry into hash table only.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : None
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : This function is only setting record into OM and not running FSM to config
 *          chip. This is because the record's life_time=other only can stay in OM
 *          not chip.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SetAddrHashOnly(UI8_T *addr_entry, UI32_T *record_index)
{
    /* Local Variable Declaration
      */
    BOOL_T    rc = FALSE;
    L_HASH_Index_T tmp_index;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* add address entry to hash table
     */
    if((rc = L_HASH_ShMem_SetRecordToHashOnly(&shmem_data_p->amtrdrv_om_hash_desc, addr_entry , &tmp_index)))
    {
        AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T*)addr_entry,TRUE, &(shmem_data_p->counters));
        L_HASH_ShMem_ClearReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,AMTR_TYPE_COUNTER_FLAGS|AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS);
        *record_index = tmp_index;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;

}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will delete address table entrie from Hash table & chip
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : index -- hash_record index
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : This function will delte the record from OM and through OM's FSM to
 *          delete the record from chip.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_DeleteAddr(UI8_T *addr_entry, UI32_T *index)
{
    /* Local Variable Declaration
     */
    BOOL_T   rc = FALSE;
    UI8_T    state ;
    L_HASH_Index_T tmp_index;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    /* delete an address entry from Hash
     */
    if((rc = AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry,&tmp_index)))
    {
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,tmp_index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,tmp_index);
#endif

#endif
    
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
           AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T*)addr_entry, FALSE, &(shmem_data_p->counters));
           L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,AMTR_TYPE_COUNTER_FLAGS);
        }
        *index = tmp_index;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteAddrFromHashOnly
 *------------------------------------------------------------------------------
 * PURPOSE: This function will delete address table entrie from Hash table & chip
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : None
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : This function is only deleting record from OM and not running FSM to delete
 *          record from chip. This is because the record's life_time-- other only stay
 *          in OM not chip.
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_OM_DeleteAddrFromHashOnly(UI8_T *addr_entry, UI32_T *index)
{
    /* Local Variable Declaration
     */
    BOOL_T    rc = FALSE;
    L_HASH_Index_T tmp_index;

    /* BODY
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
    /* If record was deleted form OM, it would be removed from all guery group at the same time.
     * We want to get the next index. This function have to be called earlier than deletion.
     */
    rc  = L_HASH_ShMem_GetRecordIndex(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry,&tmp_index);

    if ((rc == TRUE)&& (tmp_index == shmem_data_p->amtrdrv_om_wait_age_out_checking_index))
    {
        if (L_DLST_ShMem_Indexed_Dblist_GetNextEntry(L_CVRT_GET_PTR(shmem_data_p,(uintptr_t)shmem_data_p->amtrdrv_om_dyn_dblist), (L_DLST_Index_T *)&shmem_data_p->amtrdrv_om_wait_age_out_checking_index) == FALSE)
        {
            /* Can't find next entry
             */
            shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_END;
        }
    }
#endif
    /* delete record from hash table directly
     */
    if((rc = L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry, &tmp_index)))
    {
        UI8_T state;
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
           AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T*)addr_entry, FALSE, &(shmem_data_p->counters));
           L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,AMTR_TYPE_COUNTER_FLAGS);
        }
        *index = tmp_index;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetReseverdState
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get the reserve field,it indicate the addr need 
 *          delete from chip or not
 * INPUT  : index -- hash_record index
 * OUTPUT : None
 * RETURN : The resever value:
 *          AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS
 * NOTES  : 
 *-----------------------------------------------------------------------------*/
UI8_T AMTRDRV_OM_GetReseverdState(UI32_T index)
{
    /* Local Variable Declaration
     */
    UI8_T state;

    /* BODY
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return state;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SetReseverdState
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set the reserve field,it indicate the addr need 
 *          delete from chip or not
 * INPUT  : index -- hash_record index,flags-----AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_SetReseverdState(UI32_T index,UI8_T flags)
{

    /* BODY
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,flags);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return ;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ResetReseverdState
 *------------------------------------------------------------------------------
 * PURPOSE: This function will reset the reserve field,it indicate the addr need 
 *          delete from chip or not
 * INPUT  : index -- hash_record index,flags-----AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ClearReseverdState(UI32_T index,UI8_T flags)
{

    /* BODY
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    L_HASH_ShMem_ClearReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,flags);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return ;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteAddrFromHashAndWaitSync
 *------------------------------------------------------------------------------
 * PURPOSE: This function will delete address table entrie from Hash table & chip
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : None
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : 1. This function is only deleting record from OM and not running FSM to delete
 *             record from chip. But record will be true deleted after synchroniztion.
 *          2. This API is for AMTR Hardware Learning.
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_OM_DeleteAddrFromHashAndWaitSync(UI8_T *addr_entry, UI32_T *index)
{
    /* Local Variable Declaration
     */
    BOOL_T    rc = FALSE;
    L_HASH_Index_T tmp_index;

    /* BODY
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* delete record from hash table directly
     */
    if((rc = L_HASH_ShMem_DeleteRecordNWaitSync(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry, &tmp_index)))
    {
        UI8_T state;
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
           AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T*)addr_entry, FALSE, &(shmem_data_p->counters));
           L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,AMTR_TYPE_COUNTER_FLAGS);
        }
        *index = tmp_index;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SearchNDelete
 *------------------------------------------------------------------------------
 * PURPOSE: This function will search from the giving query group and delete specified
 *          entries
 * INPUT  : ifindex    -- which ifindex
 *          vid        -- which vid
 *          life_time  -- which life_time
 *          source     -- which source
 *          mode       -- which entries want to be deleted
 * OUTPUT : TRUE / FALSE
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : if the cookie isn't used please fill 0 in the function.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SearchNDelete(UI32_T ifindex,UI32_T vid,UI32_T life_time,UI32_T source, AMTR_TYPE_Command_T mode,UI16_T *vlan_p,UI32_T vlan_counter)
{
    AMTRDRV_OM_Cookie_T    cookie;
    BOOL_T               rc = FALSE;
    UI32_T               counter;
    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTRDRV access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    switch(mode)
    {
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,0,AMTRDRV_OM_DeleteAddrCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_LIFE_TIME:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,(void *)(uintptr_t)life_time,AMTRDRV_OM_DeleteLifeTimeCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_SOURCE:

            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,(void *)(uintptr_t)source,AMTRDRV_OM_DeleteSourceCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_LIFETIME:
            cookie.arg2 = (void *)(uintptr_t)life_time;
            for(counter = 0 ; counter < vlan_counter; ){
                if(vlan_counter >= 100 + counter){
                    cookie.arg3 = (void *)(uintptr_t)100;
                }else 
                    cookie.arg3 = (void *)(uintptr_t)(vlan_counter - counter);
                
                cookie.arg1 = &vlan_p[counter];
                
                AMTRDRV_OM_ENTER_CRITICAL_SECTION();
                rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,&cookie,AMTRDRV_OM_DeleteVidAndLifeTimeCallBack);
                AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
                
                counter = counter + (uintptr_t)cookie.arg3 ;
            }
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_PORT_N_VID_N_SOURCE:
            cookie.arg1 = (void *)(uintptr_t)vid;
            cookie.arg2 = (void *)(uintptr_t)source;
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,&cookie,AMTRDRV_OM_DeleteVidAndSourceCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_VID:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_VID,vid-1,0,AMTRDRV_OM_DeleteAddrCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_PORT:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();  
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,(void *)(uintptr_t)vid,AMTRDRV_OM_DeleteVidCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_LIFETIME:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_VID,vid-1,(void *)(uintptr_t)life_time,AMTRDRV_OM_DeleteLifeTimeCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_VID_N_SOURCE:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_VID,vid-1,(void *)(uintptr_t)source,AMTRDRV_OM_DeleteSourceCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,life_time-1,0,AMTRDRV_OM_DeleteAddrCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();            
            break;
        case AMTR_TYPE_COMMAND_DELETE_BY_SOURCE:
            AMTRDRV_OM_ENTER_CRITICAL_SECTION();
            rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_SOURCE,source-1,0,AMTRDRV_OM_DeleteAddrCallBack);
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            break;
        default :
            rc = FALSE;
            break;
    }

    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SearchNDeleteExceptCertainAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will search from the giving query group and delete specified
 *          entries
 * INPUT  : ifindex    -- which ifindex
 *          vid        -- which vid
 *          mac_list_p
 *          mask_list_p
 *          number_of_entry_in_list
 * OUTPUT : TRUE / FALSE
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : if the cookie isn't used please fill 0 in the function.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SearchNDeleteExceptCertainAddr(UI32_T ifindex,
                                                 UI32_T vid,
                                                 UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                 UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                 UI32_T number_of_entry_in_list)
{
    AMTRDRV_OM_Cookie_T    cookie;
    BOOL_T                 rc=FALSE;

    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTRDRV access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    cookie.arg1 = (void *)(uintptr_t)vid;
    cookie.arg2 = mac_list_p;
    cookie.arg3 = mask_list_p;
    cookie.arg4 = (void *)(uintptr_t)number_of_entry_in_list;
    rc = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,&cookie,AMTRDRV_OM_DeleteVidExceptCertainAddrCallback);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetExactRecord
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get exact record from hash table
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry - partial record infor but vid & mac are required
 * OUTPUT : AMTR_TYPE_AddrEntry_T *addr_entry - return exact record info
 * RETURN : TRUE / FALSE
 * NOTES  : The input record should include mac & vid
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetExactRecord(UI8_T *addr_entry)
{
    BOOL_T rc = FALSE;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    rc = L_HASH_ShMem_GetExactRecord(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetRecordIndex
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get record index by giving address info
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry - user record
 * OUTPUT : UI32_T index                      - the index for hash_record
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetRecordIndex(UI8_T *addr_entry, UI32_T *index)
{
    BOOL_T  rc = FALSE;
    L_HASH_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if ((rc = L_HASH_ShMem_GetRecordIndex(&shmem_data_p->amtrdrv_om_hash_desc,addr_entry,&tmp_index)))
    {
        *index = tmp_index;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DequeueJobList
 *------------------------------------------------------------------------------
 * PURPOSE: This function will process Job Queue
 * INPUT  : None
 * OUTPUT : addr_entry   - user record
 *          action       - SET/DELETE
 *          idx          - record index
 * RETURN : TRUE--Job queue has a record / FALSE--Job queue doesn't have any record
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_DequeueJobList(UI8_T *addr_entry,UI8_T *action,UI32_T *idx)
{
    BOOL_T  result = FALSE;
    UI8_T   *temp_address;
    L_HASH_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    if((result = L_HASH_ShMem_DequeueJobList(&shmem_data_p->amtrdrv_om_hash_desc,&temp_address, action, &tmp_index)))
    {
        memcpy(addr_entry,temp_address, sizeof(AMTRDRV_TYPE_Record_T));
        *idx = tmp_index;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SetOperationResultEvent
 *------------------------------------------------------------------------------
 * PURPOSE: This function will process operation result to trigger HASH to run FSM
 * INPUT  : addr_entry   - user record
 *        : event_type   - Set_Success_Ev / Del_Success_Ev / Fail_Ev
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SetOperationResultEvent(UI8_T *addr_entry,UI32_T event_type)
{
    BOOL_T result = FALSE;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    result = L_HASH_ShMem_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc, event_type ,addr_entry);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_UpdateRecordLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function will UpdateRecord's life time and adjust the position in
            query group
 * INPUT  : ifindex                      - port
 *        : life_time                    - life_time
 *        : is_updated_local_aging_list  - TRUE(need to update) else(don't need to)
 * OUTPUT : None
 * RETURN : TURE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_UpdateRecordLifeTime(UI32_T ifindex,UI32_T life_time,BOOL_T is_update_local_aging_list)
{
    AMTRDRV_OM_Cookie_T  cookie;
    BOOL_T               result = FALSE;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    cookie.arg1 = (void *)(uintptr_t)is_update_local_aging_list;
    cookie.arg2 = (void *)(uintptr_t)life_time;
    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTRDRV access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    result = L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,ifindex,&cookie,AMTRDRV_OM_UpdateRecordLifeTime_Callback);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_UpdateLocalRecordHitBitValue
 *------------------------------------------------------------------------------
 * PURPOSE:This function will Update Record's previous hitbit value
 * INPUT  : index          -- Hash_record_index
 *          hitbit_value   -- giving hitbit value
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_UpdateLocalRecordHitBitValue(UI32_T index,UI8_T hitbit_value)
{
    UI8_T   *addr_entry;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);
    ((AMTRDRV_TYPE_Record_T *)addr_entry)->hit_bit_value_on_local_unit = hitbit_value;
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_UpdateSystemTrunkHitBit
 *------------------------------------------------------------------------------
 * PURPOSE: This function will Update Record's trunk hitbit value
 * INPUT  : index          --Hash_record_index
 *        : hitbit_value   --hitbit value
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTE   : This function is only called in master unit
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_UpdateSystemTrunkHitBit(UI32_T index,UI16_T hitbit_value)
{
    UI8_T               *addr_entry;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);
    ((AMTRDRV_TYPE_Record_T *)addr_entry)->trunk_hit_bit_value_for_each_unit = hitbit_value;
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get first entry from giving query group
 * INPUT  : query_group - A specific query category to delete.
 *          query_element - Index of the specific category to operate.
 * OUTPUT : L_DLST_ShMem_Indexed_Dblist_T dblist     -- the specific query group's descriptor
 *          UI32_T                  index      -- first entry's index
 *          UI8_T                   addr_entry -- address info
 *          UI8_T                   action     -- record action
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(UI32_T query_group,UI32_T query_element,L_DLST_ShMem_Indexed_Dblist_T **dblist,UI32_T *index,UI8_T *addr_entry,UI8_T *action)
{
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Get the query group element's descriptor */
    if(!L_HASH_ShMem_GetQueryGroupDescriptor(&shmem_data_p->amtrdrv_om_hash_desc,query_group,query_element,dblist))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    /* Get the first node in query group */
    if(L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(*dblist, &tmp_index))
    {
        /* base on index to get record and record action */
        AMTRDRV_OM_GetRecordByIndex(tmp_index,addr_entry,action);
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        *index = tmp_index;
        return TRUE;
    }
    else
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetNextEntryFromGivingQueryGroup
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get the next entry by index from giving query group
 * INPUT  : L_DLST_ShMem_Indexed_Dblist_T dblist     -- the specific query group's descriptor
          : UI32_T                  index      -- the giving index
 * OUTPUT : UI32_T                  index      -- the new record's index
 *          UI8_T                   addr_entry -- address info
 *          UI8_T                   action     -- record action
 * RETURN : TRUE /FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetNextEntryFromGivingQueryGroup(L_DLST_ShMem_Indexed_Dblist_T *dblist,UI32_T *index,UI8_T *addr_entry,UI8_T *action)
{
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if(L_DLST_ShMem_Indexed_Dblist_GetNextEntry(dblist, &tmp_index))
    {
        /* base on index to get record and record action */
        AMTRDRV_OM_GetRecordByIndex(tmp_index,addr_entry,action);
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        *index = tmp_index;
        return TRUE;
    }
    else
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
#if 0
/*add by tony.Lei*/
/*improve the method of NA Enqueue, for the master we will improve the rate of  dealing*/
/* add by Tony.Le i */
#define AMTRDRV_OM_NAENETRY_ARRYS_NUM 50
AMTR_TYPE_NAEntry_T  amtrdrv_om_naentry[AMTRDRV_OM_NAENETRY_ARRYS_NUM][AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET];
AMTR_TYPE_NAEntry_Array_T amtrdrv_om_naentry_array[AMTRDRV_OM_NAENETRY_ARRYS_NUM];
AMTR_TYPE_NAEntry_Array_P_T  amtrdrv_om_naentry_array_p;
void AMTRDRV_OM_NAEntryInit(void){
    int i,j;
	AMTR_TYPE_NAEntry_Array_T * previous = NULL;
	amtrdrv_om_naentry_array_p.used_header= NULL;
	amtrdrv_om_naentry_array_p.used_tailer= NULL;
	amtrdrv_om_naentry_array_p.freed_header= amtrdrv_om_naentry_array;
	amtrdrv_om_naentry_array_p.freed_tailer = &amtrdrv_om_naentry_array[AMTRDRV_OM_NAENETRY_ARRYS_NUM -1];
	/*init array*/
    for(i=0;i < AMTRDRV_OM_NAENETRY_ARRYS_NUM ;i++){
		if(previous)
			previous->next =& amtrdrv_om_naentry_array[i];

		amtrdrv_om_naentry_array[i].previous = previous ;
		previous = &amtrdrv_om_naentry_array[i];
		amtrdrv_om_naentry_array[i].addr_entry_p = amtrdrv_om_naentry[i];
		
		for(j=0;j < AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET;j++){
           amtrdrv_om_naentry[i][j].next_offset = j+1;
		}
		amtrdrv_om_naentry[i][AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET-1].next_offset = 0;
	}	
		

}
/*get the used array , if no used array  , return null
 * For the used array ,we will  get them greedly
 */
AMTR_TYPE_NAEntry_Array_T * AMTRDRV_OM_GetUsedArray(){
     AMTR_TYPE_NAEntry_Array_T *temp;    
     AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
       temp = amtrdrv_om_naentry_array_p.used_header; 
	   amtrdrv_om_naentry_array_p.used_tailer = NULL;
	   amtrdrv_om_naentry_array_p.used_header = NULL;
	 AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();

    return temp;
}
/*the freedarray will be used by NIC and ISC , if we use greed method ,it will cause one of them hunger */
AMTR_TYPE_NAEntry_Array_T * AMTRDRV_OM_GetFreedArray(){
     AMTR_TYPE_NAEntry_Array_T *temp;    
     AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
       temp = amtrdrv_om_naentry_array_p.freed_header; 
	   if(temp)
	   amtrdrv_om_naentry_array_p.freed_header = temp->next ;
	   if(amtrdrv_om_naentry_array_p.freed_header == NULL)
    	   amtrdrv_om_naentry_array_p.freed_tailer = NULL;
	 AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();
   /*clear no used information*/
     if(temp)
     temp->next = temp->previous = NULL;
    return temp;
}
int   AMTRDRV_OM_RestoreFreedArray(AMTR_TYPE_NAEntry_Array_T * array_p ){
      AMTR_TYPE_NAEntry_Array_T * tailer = array_p;
	  
      if(!array_p)
	  	return -1;
	  
	  while((tailer->counter = 0)&&array_p->next){
	  	tailer = array_p->next;
	  }
      AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
      if(amtrdrv_om_naentry_array_p.freed_tailer){
	  	amtrdrv_om_naentry_array_p.freed_tailer->next = array_p;	  
      }else{		
		  amtrdrv_om_naentry_array_p.freed_header = array_p;
	  }
	  amtrdrv_om_naentry_array_p.freed_tailer = tailer;
      AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();
	  return 0;
}
int   AMTRDRV_OM_RestoreUsedArray(AMTR_TYPE_NAEntry_Array_T * array_p ){
      AMTR_TYPE_NAEntry_Array_T * tailer = array_p;
	  
      if(!array_p)
	  	return -1;
	  
	  while(array_p->next)
	  	tailer = array_p->next;
	  
      AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
      if(amtrdrv_om_naentry_array_p.used_tailer){
	  	amtrdrv_om_naentry_array_p.used_tailer->next = array_p;	  
      }else{		
		  amtrdrv_om_naentry_array_p.used_header = array_p;
	  }
	  amtrdrv_om_naentry_array_p.used_tailer = tailer;
      AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();
	  return 0;
}
extern AMTR_TYPE_NAEntry_Array_T * dev_nicdrv_naentry_array_current = NULL;
extern int dev_nicdrv_naentry_array_current_nacount = 0;

int AMTRDRV_OM_NABufferEnqueue1(UI8_T * src_mac,UI16_T vid,UI32_T src_unit,UI32_T src_port)
{
   if(!dev_nicdrv_naentry_array_current)
     dev_nicdrv_naentry_array_current = AMTRDRV_OM_GetFreedArray();
     
   if(!dev_nicdrv_naentry_array_current)
   	   return -1;
   	   
   (dev_nicdrv_naentry_array_current->addr_entry_p + dev_nicdrv_naentry_array_current_nacount)->ifindex = src_port;
   (dev_nicdrv_naentry_array_current->addr_entry_p + dev_nicdrv_naentry_array_current_nacount)->vid = vid;
   memcpy((dev_nicdrv_naentry_array_current->addr_entry_p + dev_nicdrv_naentry_array_current_nacount)->mac, src_mac, AMTR_TYPE_MAC_LEN);
   dev_nicdrv_naentry_array_current_nacount = (++dev_nicdrv_naentry_array_current_nacount)%AMTRDRV_TYPE_MAX_ENTRIES_IN_ONE_PACKET;
   if(!dev_nicdrv_naentry_array_current_nacount){
   	AMTRDRV_OM_RestoreUsedArray(dev_nicdrv_naentry_array_current);
    dev_nicdrv_naentry_array_current = NULL;
    dev_nicdrv_naentry_array_current = AMTRDRV_OM_GetFreedArray();
   }
   return 0;
}
  AMTR_TYPE_NAEntry_Array_T * amtr_om_naentry_array_current_dequeue = NULL;
static  int amtrdrv_om_naentry_array_current_nacount = 0;
int AMTRDRV_OM_NABufferDequeue1(AMTR_TYPE_AddrEntry_T *addr_entry, int  number_of_entries)
{
	   int i = 0;
	   if(!amtr_om_naentry_array_current_dequeue){
	     amtr_om_naentry_array_current_dequeue = AMTRDRV_OM_GetUsedArray();
	     amtrdrv_om_naentry_array_current_nacount = 0;
	   }
	   for(;(amtr_om_naentry_array_current_dequeue)&&(number_of_entries-- > 0);i++){
	   	(addr_entry+i)->ifindex = (amtr_om_naentry_array_current_dequeue->addr_entry_p + amtrdrv_om_naentry_array_current_nacount)->ifindex;
	   	(addr_entry+i)->vid = (amtr_om_naentry_array_current_dequeue->addr_entry_p + amtrdrv_om_naentry_array_current_nacount)->vid;
	    memcpy((addr_entry+i)->mac,(amtr_om_naentry_array_current_dequeue->addr_entry_p + amtrdrv_om_naentry_array_current_nacount)->mac, AMTR_TYPE_MAC_LEN);   
	    amtrdrv_om_naentry_array_current_nacount = (++amtrdrv_om_naentry_array_current_nacount)%amtr_om_naentry_array_current_dequeue->counter ;
	       if(amtrdrv_om_naentry_array_current_nacount == 0){
		   		AMTR_TYPE_NAEntry_Array_T * temp = amtr_om_naentry_array_current_dequeue;
		   	    amtr_om_naentry_array_current_dequeue = amtr_om_naentry_array_current_dequeue->next;		
		   	    temp->next = NULL;
		   		AMTRDRV_OM_RestoreFreedArray(temp);
	       }
	   	}
    return i;

}
#endif
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)

/*add by tony.lei */
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SlaveAgingOutEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to Slave  aging out AK list
 *          and wait for ACK from Master. If no ACK is recveived in the expected time , the Slave will resend the message
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SlaveAgingOutEnqueue(UI32_T index)
{
	BOOL_T    rc = FALSE;
	/* BODY
	 */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Insert the node into local aging check list */
    rc = L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->slave_aging_ak_list_desc, index);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SlaveAgingOutResend
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to resend the agingout entry
 * INPUT  : index -- The record's hash index
 * OUTPUT : addr_entry
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SlaveAgingOutResend(UI32_T *index,UI8_T *addr_entry)
{

    BOOL_T    rc = FALSE;
    UI8_T     *temp_address;
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Since we don't the record will be aged out or not we have to read first.
       If it already passed the aging out time we will delete this node from aging list */
    if( L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->slave_aging_ak_list_desc, &tmp_index)){
        L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&temp_address, tmp_index);
		/*remove from the header */
		L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc, tmp_index);
		/*insert  from the header */
		L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->slave_aging_ak_list_desc, tmp_index);
        memcpy(addr_entry,temp_address,sizeof(AMTRDRV_TYPE_Record_T));
        *index = tmp_index;
        rc = TRUE ;
	}
	AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}
#endif
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_NABufferEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function will put an NA entry into NA Hash buffer
 * INPUT  : UI8_T   *addr_entry -- NA record
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is full or hash fail to allocate it simply discare
 *          this packet and return FALSE.
 *-------------------------------------------------------------------------*/
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
 static UI32_T amtrdrv_om_naentry_counter =0;
#endif
BOOL_T AMTRDRV_OM_NABufferEnqueue(UI8_T *addr_entry)
{
    BOOL_T     rc = FALSE;
    L_HASH_Index_T index;

    /* BODY
     */
    AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
    /* Setting record into NA buffer */
    rc = L_HASH_ShMem_SetRecord(&shmem_data_p->amtrdrv_om_na_buffer_desc, addr_entry,&index);
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
    if(rc)
		amtrdrv_om_naentry_counter ++;
#endif
    AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_NABufferDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will get an NA entry from NA Hash Buffer
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : TRUE / FALSE(queue empty)
 * NOTES  : This function is called at interrupt time, so it need to be fast.
 *          When NA queue is empty, this function return FALSE
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_NABufferDequeue(UI8_T *addr_entry
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
,UI32_T * queue_counter
#endif
)
{
    L_HASH_Index_T idx;
    UI8_T       *temp_addr;
    UI8_T       action;
    BOOL_T      rc = FALSE;

    AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
    /* Pop out New address entry from NA Buffer */
    if((rc = L_HASH_ShMem_DequeueJobList(&shmem_data_p->amtrdrv_om_na_buffer_desc,&temp_addr, &action, &idx)))
    {
#if(AMTRDRV_MGR_DEBUG_PERFORMANCE == TRUE)
        *queue_counter = amtrdrv_om_naentry_counter;
        amtrdrv_om_naentry_counter --;
#endif
        /* copy the record to addr_entry and also delete that entry from NA buffer */
        memcpy(addr_entry,temp_addr, sizeof(AMTR_TYPE_AddrEntry_T));
        L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_na_buffer_desc,addr_entry, &idx);
    }
    AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();

    return rc;

}
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_NABufferDequeueBat
 *-------------------------------------------------------------------------
 * PURPOSE: This funciton will get an NA entry from NA Hash Buffer
 * INPUT  : None
 * OUTPUT : UI8_T *addr_entry      -- NA
 * RETURN : addr_entry counter
 * NOTES  : 
 *-------------------------------------------------------------------------*/
int  AMTRDRV_OM_NABufferDequeueBat(AMTR_TYPE_AddrEntry_T *addr_entry,int bat_counter,UI32_T *queue_counter)
{
    L_HASH_Index_T idx;
    UI8_T       *temp_addr;
    UI8_T       action;
    int i ;
    AMTRDRV_OM_EN_DE_ENTER_CRITICAL_SECTION();
	*queue_counter = amtrdrv_om_naentry_counter;
    /* Pop out New address entry from NA Buffer */
  	for(i = 0 ;(bat_counter-- > 0);i++){	  	 
        if(L_HASH_ShMem_DequeueJobList(&shmem_data_p->amtrdrv_om_na_buffer_desc,&temp_addr, &action, &idx))
        {
			amtrdrv_om_naentry_counter --;
            /* copy the record to addr_entry and also delete that entry from NA buffer */
            memcpy(&addr_entry[i],temp_addr,sizeof(AMTR_TYPE_AddrEntry_T));
            L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_na_buffer_desc,(UI8_T  *)addr_entry, &idx);
        }else
          break;
  	}
    AMTRDRV_OM_EN_DE_LEAVE_CRITICAL_SECTION();

    return i;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_AgingOutBufferEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert age out entries info into AgingOut Buffer
 * INPUT  : UI8_T   mac   -- mac
 *          UI16_T  vid   -- vid
 *          UI16_T  ifindex - ifindex of the mac to be aged out.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE(If buffer is full)
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_AgingOutBufferEnqueue(UI8_T *mac, UI16_T vid, UI16_T ifindex)
{
    BOOL_T   rc = FALSE;
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    if(shmem_data_p->amtrdrv_om_agingout_counter < AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER)
    {
        shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_tail].ifindex = ifindex;
        memcpy(shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_tail].mac,mac,AMTR_TYPE_MAC_LEN);
        shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_tail].vid = vid;
        shmem_data_p->amtrdrv_om_agingout_buffer_tail = (shmem_data_p->amtrdrv_om_agingout_buffer_tail+1)%AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER;
        shmem_data_p->amtrdrv_om_agingout_counter ++;
        rc =TRUE;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_AgingOutBufferDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to get age Out entries info from Aging out buffer
 * INPUT  : num_of_entry  -- how many entries need to get this time
 *          addr_buff[]   -- aging out entries information
 * OUTPUT : None
 * RETURN : UI32_T the return vaule is indicated how many entries are got this time.
 * NOTE   : This API is only used in master mode... master will dequeue aged entries from this
 *          buffer and notify AMTR to delete such entries.
 *-------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_AgingOutBufferDequeue(UI32_T num_of_entry,AMTR_TYPE_AddrEntry_T addr_buff[])
{
    UI32_T    i, num_of_get_entries=0;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    for(i=0;i<num_of_entry;i++)
    {
        if(shmem_data_p->amtrdrv_om_agingout_counter != 0)
        {
            addr_buff[i].ifindex = shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_head].ifindex;
            memcpy(addr_buff[i].mac,shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_head].mac,AMTR_TYPE_MAC_LEN);
            #if (SYS_CPNT_VXLAN == TRUE)
            if(AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_head].vid))
            {
                UI16_T r_vfi;

                if(AMTRDRV_OM_ConvertLvfiToRvfi(shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_head].vid, &r_vfi)==FALSE)
                {
                    shmem_data_p->amtrdrv_om_agingout_buffer_head = (shmem_data_p->amtrdrv_om_agingout_buffer_head + 1)%AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER;
                    shmem_data_p->amtrdrv_om_agingout_counter--;

                    AMTRDRV_MGR_DBGMSG("AMTRDRV_OM_ConvertLvfiToRvfi returns error");
                    continue;
                }
                addr_buff[i].vid=r_vfi;
            }
            #else
            addr_buff[i].vid = shmem_data_p->amtrdrv_om_agingout_buffer[shmem_data_p->amtrdrv_om_agingout_buffer_head].vid;
            #endif
            shmem_data_p->amtrdrv_om_agingout_buffer_head = (shmem_data_p->amtrdrv_om_agingout_buffer_head + 1)%AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER;
            shmem_data_p->amtrdrv_om_agingout_counter --;
            num_of_get_entries +=1;
        }
        else
        {
            break;
        }
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return num_of_get_entries;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_IsAgingOutBufferFull
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to check aging Out buffer still has enough space
 *            to store the age out entries info which are from slave unit.
 * INPUT  : UI32_T checking_number   -- to check the space is still have that space to
                                        store those records.
 * OUTPUT : TRUE / FALSE
 * RETURN : TRUE (Full)/ FALSE ( Not Full)
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_IsAgingOutBufferFull(UI32_T checking_number)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if(( AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER - shmem_data_p->amtrdrv_om_agingout_counter) < checking_number)
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
    else
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListUpdateTimeStamp
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to search each node in local aging checking list and
 *          also update timestamp as current system time
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : This function is called when aging out function is from disable to enable
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListUpdateTimeStamp()
{
    L_DLST_Index_T index=0;
    UI32_T    current_time;
    UI8_T     *addr_entry;
    BOOL_T    ret = FALSE;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    /* Get first record */
    ret = L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->local_aging_check_list_desc,&index);
    current_time = SYSFUN_GetSysTick();
    while ( ret )
    {   /* Base on index to get the record pointer to update timestamp as system time */
        L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);
        ((AMTRDRV_TYPE_Record_T *)addr_entry)->aging_timestamp = current_time;
        /* Get next record index */
        ret = L_DLST_ShMem_Indexed_Dblist_GetNextEntry(&shmem_data_p->local_aging_check_list_desc, &index);
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;

}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to local aging out check list
 *          and also update record timestamp as current system time
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListEnqueue(UI32_T index)
{
    UI8_T     *addr_entry;
    BOOL_T    rc = FALSE;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Insert the node into local aging check list */
    rc = L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->local_aging_check_list_desc, index);
    /* update record timestamp as current system time */
    L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);
    ((AMTRDRV_TYPE_Record_T *)addr_entry)->aging_timestamp = SYSFUN_GetSysTick();

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAginListDeleteEntry
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to delete MAC entries from aginglist
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAginListDeleteEntry(UI32_T index)
{
    BOOL_T    rc = FALSE;
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
      rc = L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#endif
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListEnqueue2Head
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to the head of local aging out check list
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListEnqueue2Head(UI32_T index)
{
    BOOL_T    rc = FALSE;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Insert the node into local aging check list */
    rc = L_DLST_ShMem_Indexed_Dblist_Enqueue2Head(&shmem_data_p->local_aging_check_list_desc, index);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to dequeue node from local checking aging list
 * INPUT  : aging_time     -- aging out time (Sec)
 * OUTPUT : index          -- The record's hash index
 *          addr_entry     -- address record info
 * RETURN : TRUE / FALSE(queue empty or the record isn't reached the aging out time)
 * NOTES  : None
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListDequeue(UI32_T aging_time,UI32_T *index,UI8_T *addr_entry)
{

    BOOL_T    rc = FALSE;
    UI32_T    current_time;
    UI32_T    alive_time;
    UI8_T     *temp_address;
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Since we don't the record will be aged out or not we have to read first.
       If it already passed the aging out time we will delete this node from aging list */
    if( L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->local_aging_check_list_desc, &tmp_index))
    {
        L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&temp_address, tmp_index);
        current_time = SYSFUN_GetSysTick();

        if(current_time >= ((AMTRDRV_TYPE_Record_T *)temp_address)->aging_timestamp)
        {
            alive_time = current_time - ((AMTRDRV_TYPE_Record_T *)temp_address)->aging_timestamp;
        }
        else
        {
            alive_time = current_time + (~(((AMTRDRV_TYPE_Record_T *)temp_address)->aging_timestamp));
        }
        if(alive_time >= aging_time*100)
        {
            rc = L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc, tmp_index);
        }

        *index = tmp_index;
    }

    if(rc)
    {
        memcpy(addr_entry,temp_address,sizeof(AMTRDRV_TYPE_Record_T));
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;
}
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueEnqueueAndSetEventID
 *------------------------------------------------------------------------------
 * PURPOSE: This function will insert a node to sync queue according
 *          to specified event_index
 * INPUT  : event_index  - event index to be inserted to sync queue
 *          cookie       - addition information for setting event id
 *                         for the inserted node
 *          action       - AMTR_TYPE_COMMAND_DELETE_BY_PORT, etc.
 * OUTPUT : None
 * RETURN : TRUE         - enqueue and set event id sucessfully
 *          FALSE        - error occurs when enqueue
 * NOTES  : 1. event_index must be greater than
               SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
 *          2. This API only allowed to be called by AMTRDRV internally
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(UI32_T event_index, UI32_T cookie, AMTR_TYPE_Command_T action)
{
    UI32_T event_id=0;
    BOOL_T rc;
    BOOL_T ret=TRUE;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    rc = L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->sync_queue_desc, event_index, &event_id);
    if(rc==FALSE)
    {
        printf("%s(%d): Failed to get entry id (event_index=%lu)\r\n",
            __FUNCTION__, __LINE__, (unsigned long)event_index);
        ret=FALSE;
        goto exit;
    }

    if(event_id==0)
    {
        /* Failed to get event id means that the event_index had not
         * been enqueued
         */
        rc = _AMTRDRV_OM_SyncQueueEnqueue(event_index);
        if(rc==FALSE)
        {
            printf("%s(%d) Sync Queue Enqueue failed(event_index=%lu)\r\n", __FUNCTION__,
                __LINE__, (unsigned long)event_index);
            ret=FALSE;
		    goto exit;
        }
    }
    else
    {
        if((shmem_data_p->synq_debug_info) & AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_ENQ_INFO)
        {
            printf("%s(%d) Event index already exists(event_index=%lu)\r\n", __FUNCTION__,
                __LINE__, (unsigned long)event_index);
        }
    }

    /* Setup event_id for the corresponding event_index
     * event_id would contains existing events if  L_DLST_ShMem_Indexed_Dblist_GetEntryID() returns TRUE
     * In usual, OM should not call MGR, since AMTRDRV_MGR_Event2ID just do simple translation, it is no harm to call.
     */
    AMTRDRV_MGR_Event2ID(cookie, action , &event_id);

    if((shmem_data_p->synq_debug_info) & AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_ENQ_INFO)
    {
        printf("%s(%d): event_id=0x%08lX\r\n", __FUNCTION__, __LINE__, (unsigned long)event_id);
    }

    _AMTRDRV_OM_SyncQueueSetEntryID(event_index, event_id);

exit:
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node into sync queue
 * INPUT  : index   -- The record's hash index
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : If the address record need to be sync to Hisam table we have to insert it
 *          into this queue
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SyncQueueEnqueue(UI32_T index)
{
    BOOL_T rc;
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
	rc=_AMTRDRV_OM_SyncQueueEnqueue(index);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return rc;	
}

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ClearCollisionVlanMacTable(void)
{

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->amtrdrv_collision_mac_num = 0;
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Get next entry from the collision vlan mac table.
 * INPUT    : idx_p - The entry next to the value of *idx_p will be output
 * OUTPUT   : idx_p - The index of the output entry
 *            vlan_id_p - The vlan id of the collision mac address
 *            mac       - the collision mac address
 *            count_p   - the count of hash collision occurence
 * RETURN   : TRUE  - An entry is output sucessfully
 *            FALSE - No more entry to output.
 * NOTE     : To get the first entry, set value of *idx_p as
 *            AMTRDRV_MGR_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable(UI8_T* idx_p, UI16_T* vlan_id_p, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T *count_p)
{
    BOOL_T retval=FALSE;

    if(idx_p==NULL || vlan_id_p==NULL || mac==NULL || count_p==NULL)
    {
        /* critical error, show error message on console
         */
        printf("%s(%d):Invalid arguments\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    if(shmem_data_p->amtrdrv_collision_mac_num == 0)
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    if(*idx_p == AMTRDRV_MGR_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX)
    {
        *vlan_id_p = shmem_data_p->amtrdrv_collision_mac_table[0].vlan;
        memcpy(mac, shmem_data_p->amtrdrv_collision_mac_table[0].mac, SYS_ADPT_MAC_ADDR_LEN);
        *idx_p = 0;
        *count_p = shmem_data_p->amtrdrv_collision_mac_table[0].count;
        retval = TRUE;
    }
    else if(*idx_p < (shmem_data_p->amtrdrv_collision_mac_num-1))
    {
        (*idx_p)++;
        *vlan_id_p = shmem_data_p->amtrdrv_collision_mac_table[*idx_p].vlan;
        memcpy(mac, shmem_data_p->amtrdrv_collision_mac_table[*idx_p].mac, SYS_ADPT_MAC_ADDR_LEN);
        *count_p = shmem_data_p->amtrdrv_collision_mac_table[*idx_p].count;
        retval = TRUE;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return retval;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_LogCollisionVlanMac
 *------------------------------------------------------------------------------
 * Purpose  : Log collision vlan and mac address to collision mac table.
 * INPUT    : vlan_id  - vlan id of the collision mac address.
 *            mac      - collision mac address.
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void AMTRDRV_OM_LogCollisionVlanMac(UI16_T vlan_id, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN])
{
    UI8_T  idx;
    BOOL_T update_ok=FALSE;

#if 0
    printf("%s():vlan=%hu, mac=%02X-%02X-%02X-%02X-%02X-%02X\r\n",
        __FUNCTION__, vlan_id, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* check whether the mac already exists in the table
     */
    for(idx=0; idx<shmem_data_p->amtrdrv_collision_mac_num; idx++)
    {
        if((shmem_data_p->amtrdrv_collision_mac_table[idx].vlan==vlan_id) &&
           (memcmp(shmem_data_p->amtrdrv_collision_mac_table[idx].mac, mac, SYS_ADPT_MAC_ADDR_LEN)==0))
        {
            /* entry is found, update count
             */
            shmem_data_p->amtrdrv_collision_mac_table[idx].count++;
            update_ok=TRUE;
            break;
        }
    }

    /* add a new entry if not found in the table
     */
    if(update_ok==FALSE && shmem_data_p->amtrdrv_collision_mac_num<SYS_ADPT_MAX_NBR_OF_AMTRDRV_COLLISION_MAC_ENTRY)
    {
        shmem_data_p->amtrdrv_collision_mac_num++;
        shmem_data_p->amtrdrv_collision_mac_table[idx].vlan=vlan_id;
        memcpy(shmem_data_p->amtrdrv_collision_mac_table[idx].mac, mac, SYS_ADPT_MAC_ADDR_LEN);
        shmem_data_p->amtrdrv_collision_mac_table[idx].count=1;
        update_ok=TRUE;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

#if 0
    if(update_ok==FALSE)
    {
        printf("collision mac table full.\r\n");
    }
#endif

}
#endif

#if (SYS_CPNT_VXLAN == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_GetAndAlocRVfiMapToLVfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified real vfi to logical vfi. Allocate the new
 *            mapping between the real vfi and logical vfi when the mapping does
 *            not exist yet.
 * INPUT    : r_vfi   -  real vfi
 * OUTPUT   : l_vfi_p -  logical vfi
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_GetAndAlocRVfiMapToLVfi(UI16_T r_vfi, UI16_T *l_vfi_p)
{
    if (r_vfi<SYS_ADPT_MIN_VXLAN_VFI_ID)
        return FALSE;

    if (l_vfi_p==NULL)
        return FALSE;

#if (SYS_ADPT_VXLAN_MAX_NBR_VNI==SYS_ADPT_MAX_VFI_NBR)
    *l_vfi_p = r_vfi-SYS_ADPT_MIN_VXLAN_VFI_ID+AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;

    return TRUE;
#elif (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR)
    {
        UI32_T i;

        AMTRDRV_OM_ENTER_CRITICAL_SECTION();

        /* search existing mapping entries
         */
        for(i=0; i< SYS_ADPT_VXLAN_MAX_NBR_VNI; i++)
        {
            if(shmem_data_p->amtrdrv_om_vxlan_lvfi_map[i]==r_vfi)
            {
                 *l_vfi_p=i + AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;
                 AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
                 return TRUE;
            }
        }

        /* search for empty mapping entry to allocate a new mapping
         */
        for(i=0; i< SYS_ADPT_VXLAN_MAX_NBR_VNI; i++)
        {
            if(shmem_data_p->amtrdrv_om_vxlan_lvfi_map[i]==0)
            {
                shmem_data_p->amtrdrv_om_vxlan_lvfi_map[i]=r_vfi;
                *l_vfi_p=i + AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;
                AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
                return TRUE;
            }
        }

        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    }
#endif
    return FALSE;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_ConvertLvfiToRvfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified logical vfi to real vfi
 * INPUT    : l_vfi   -  logical vfi
 * OUTPUT   : r_vfi_p -  real vfi
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ConvertLvfiToRvfi(UI16_T l_vfi, UI16_T *r_vfi_p)
{
    if (!AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(l_vfi))
        return FALSE;

    if (r_vfi_p==NULL)
        return FALSE;

#if (SYS_ADPT_VXLAN_MAX_NBR_VNI==SYS_ADPT_MAX_VFI_NBR)
    *r_vfi_p = l_vfi-AMTR_TYPE_LOGICAL_VID_L_VFI_BASE+SYS_ADPT_MIN_VXLAN_VFI_ID;
    return TRUE;
#elif (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR)
    {
        UI32_T i, map_idx;

        map_idx = l_vfi - AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;

        AMTRDRV_OM_ENTER_CRITICAL_SECTION();

        if(shmem_data_p->amtrdrv_om_vxlan_lvfi_map[map_idx]==0)
        {
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
        else
        {
            *r_vfi_p =  shmem_data_p->amtrdrv_om_vxlan_lvfi_map[map_idx];
        }

        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }
#endif
    return FALSE;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_ConvertRvfiToLvfi
 *------------------------------------------------------------------------------
 * Purpose  : Convert the specified real vfi to logical vfi
 * INPUT    : r_vfi   -  real vfi
 * OUTPUT   : l_vfi_p -  logical vfi
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_ConvertRvfiToLvfi(UI16_T r_vfi, UI16_T *l_vfi_p)
{
    if (r_vfi<SYS_ADPT_MIN_VXLAN_VFI_ID)
        return FALSE;

    if (l_vfi_p==NULL)
        return FALSE;

#if (SYS_ADPT_VXLAN_MAX_NBR_VNI==SYS_ADPT_MAX_VFI_NBR)	
    *l_vfi_p = r_vfi-SYS_ADPT_MIN_VXLAN_VFI_ID+AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;
    return TRUE;
#elif (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR)
    {
        UI32_T map_idx;

        AMTRDRV_OM_ENTER_CRITICAL_SECTION();

        for(map_idx=0; map_idx< SYS_ADPT_VXLAN_MAX_NBR_VNI; map_idx++)
        {
            if(shmem_data_p->amtrdrv_om_vxlan_lvfi_map[map_idx]==r_vfi)
            {
                *l_vfi_p=map_idx+AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;
                AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
                return TRUE;
            }
        }

        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    }
#endif

    return FALSE;
}

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_RemoveLvfiMap
 *------------------------------------------------------------------------------
 * Purpose  : Remove the specified logical vfi mapping
 * INPUT    : l_vfi  -  logical vfi
 * OUTPUT   : None
 * RETURN   : TRUE  - Success
 *            FALSE - Failed
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_RemoveLvfiMap(UI16_T l_vfi)
{
    if (!AMTR_TYPE_LOGICAL_VID_IS_L_VFI_ID(l_vfi))
        return FALSE;

#if (SYS_ADPT_VXLAN_MAX_NBR_VNI==SYS_ADPT_MAX_VFI_NBR)
    return TRUE;
#elif (SYS_ADPT_VXLAN_MAX_NBR_VNI<SYS_ADPT_MAX_VFI_NBR)
    {
        UI32_T map_idx;

        map_idx = l_vfi - AMTR_TYPE_LOGICAL_VID_L_VFI_BASE;

        AMTRDRV_OM_ENTER_CRITICAL_SECTION();
        
        shmem_data_p->amtrdrv_om_vxlan_lvfi_map[map_idx]=0;
        
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    }
    return TRUE;
#endif
    return FALSE;
}
#endif /*SYS_CPNT_VXLAN == TRUE*/

static BOOL_T _AMTRDRV_OM_SyncQueueEnqueue(UI32_T index)
{
    BOOL_T      rc = FALSE;
    UI8_T       sync_value = 0;

    /* BODY
     */


    rc = L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->sync_queue_desc, index);

    if(index<shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec)
    {
        L_HASH_ShMem_GetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, &sync_value, index);
        sync_value |= AMTRDRV_OM_SYNC_VALUE_SYNC_QUEUE;
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, sync_value, index);
    }

    return rc;
}



#if 0
/* This API should not be used. Because it cannot protect the
 * whole transaction in one critical section and will result to
 * race condition.
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueSetEntryID
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to set Entry ID in L_Dlist
 * INPUT  : index     -- The record's hash index
 *          dlist_id  -- element_id to indicate what kind of delete group event
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is only called if the index > SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
 *          Since only in this condition we have to set EntryID as we want for AMTRDRV.
 *          The EntryID's value is the combination of events
 *-------------------------------------------------------------------------*/
void AMTRDRV_OM_SyncQueueSetEntryID(UI32_T index,UI32_T dlist_id)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    _AMTRDRV_OM_SyncQueueSetEntryID(index, dlist_id);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}
#endif

static void _AMTRDRV_OM_SyncQueueSetEntryID(UI32_T index,UI32_T dlist_id)
{
    L_DLST_ShMem_Indexed_Dblist_SetEntryID(&shmem_data_p->sync_queue_desc, index, dlist_id);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueGetEntryID
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to get Entry ID in L_Dlist
 * INPUT  : index     -- The record's hash index
 *          dlist_id  -- element_id to indicate what kind of delete group event
 * RETURN : TRUE  -- this entry is used...
 *          FALSE -- this entry isn't used ....
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SyncQueueGetEntryID(UI32_T index,UI32_T *dlist_id)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    if(L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->sync_queue_desc, index, dlist_id)==FALSE)
    {
        printf("%s(%d):Get entry id failed. (index=%lu)\r\n",
            __FUNCTION__, __LINE__, (unsigned long)index);
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    if(*dlist_id == 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueDequeue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to dequeue node from sync queue
 * INPUT  : None
 * OUTPUT : index      -- The record's hash index
 *          dlist_id   -- element_id to indicate what kind of delete group events
 *          addr_entry -- address record
 *          action     -- record action
 * RETURN : TRUE/FALSE(queue empty)
 * NOTE   : If index >SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
 *          it will return the dlist_id otherwise we will return record & record action
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SyncQueueDequeue(UI32_T *index,UI32_T *dlist_id,UI8_T *addr_entry,UI8_T *action)
{
    UI8_T       sync_value=0;
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* if there isn't any record in the sync queue then return false */
    if(!L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->sync_queue_desc, &tmp_index))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    /* If index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY means this is a single entry we need to
     * return record & record action to caller. Or we only need to return dlist_id to caller since
     * its a combination events
     */
    if(tmp_index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
    {
        L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,addr_entry,action);
    }
    else
    {
        L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->sync_queue_desc, tmp_index, dlist_id);
    }

    /* Update sync value for the node */
    if(tmp_index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
    {
        L_HASH_ShMem_GetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, &sync_value, tmp_index);
        sync_value ^= AMTRDRV_OM_SYNC_VALUE_SYNC_QUEUE;
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, sync_value, tmp_index);
        /* If sync_value = 0 we need to trigger an SYNC DONE EV to run HASH FSM */
        if(sync_value == 0)
        {
            L_HASH_ShMem_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,addr_entry);
        }
    }
    /* delete that node from sync_queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc, tmp_index);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    *index = tmp_index;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SyncQueueDeleteEntry
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to delete node from sync queue
 * INPUT  : index   -- The record's hash index
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : If the address record don't need to be sync to Hisam table we have to delete it
 *          from this queue
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SyncQueueDeleteEntry(UI32_T index)
{
    BOOL_T      rc = FALSE;
    UI8_T       sync_value = 0;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    rc = L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc, index);

    L_HASH_ShMem_GetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, &sync_value, index);
    sync_value ^= AMTRDRV_OM_SYNC_VALUE_SYNC_QUEUE;
    L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, sync_value, index);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_CallBackQueueEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to callback queue and let the
 *          sys callback function to handle notify other related components
 * INPUT  : index   -- The record's hash index
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_CallBackQueueEnqueue(UI32_T index)
{
    BOOL_T      rc = FALSE;
    UI8_T       sync_value =0;

    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    rc = L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->call_back_queue_desc, index);

    if(index<shmem_data_p->amtrdrv_om_hash_desc.nbr_of_rec)
    {
    L_HASH_ShMem_GetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, &sync_value, index);
    sync_value |= AMTRDRV_OM_SYNC_VALUE_CALLBACK_QUEUE;
    L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, sync_value, index);
    }

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return rc;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_CallBackQueueSetEntryID
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to set Entry ID in L_Dlist
 * INPUT  : index     -- The record's hash index
 *          dlist_id  -- element_id to indicate what kind of delete group event
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is only called if the index > SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
 *          Since only in this condition we have to set EntryID as we want for AMTRDRV.
 *          The EntryID's value is the combination of events
 *-------------------------------------------------------------------------*/
void AMTRDRV_OM_CallBackQueueSetEntryID(UI32_T index,UI32_T dlist_id)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    L_DLST_ShMem_Indexed_Dblist_SetEntryID(&shmem_data_p->call_back_queue_desc, index, dlist_id);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_CallBackQueueGetEntryID
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to get Entry ID in L_Dlist
 * INPUT  : index     -- The record's hash index
 *          dlist_id  -- element_id to indicate what kind of delete group event
 * RETURN : TRUE  -- this entry is used...
 *          FALSE -- this entry isn't used ....
 * NOTES  :
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_CallBackQueueGetEntryID(UI32_T index,UI32_T *dlist_id)
{
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->call_back_queue_desc, index, dlist_id);

    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    if(*dlist_id ==0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_CallBackQueueDequeue
 *-------------------------------------------------------------------------
  * PURPOSE: This function is to dequeue node from sync queue
 * INPUT  : None
 * OUTPUT : index      -- The record's hash index
 *          dlist_id   -- element_id to indicate what kind of delete group events
 *          addr_entry -- address record
 *          action     -- record action
 * RETURN : TRUE/FALSE(queue empty)
 * NOTE   : If index >SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY
 *          it will return the dlist_id otherwise we will return record & record action
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_CallBackQueueDequeue(UI32_T *index,UI32_T *dlist_id,UI8_T *addr_entry,UI8_T *action)
{

    UI8_T       sync_value=0;
    L_DLST_Index_T tmp_index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* if there isn't any record in the callback queue then return false */
    if(!L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->call_back_queue_desc, &tmp_index))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    /* If index <= SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY means this is a single entry we need to
     * return record & record action to caller. Or we only need to return dlist_id to caller since
     * its a combination events*/
    if(tmp_index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
    {
        L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,tmp_index,addr_entry,action);
    }
    else
    {
        L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->call_back_queue_desc, tmp_index, dlist_id);
    }

    /* Update sync value for the node */
    if(tmp_index <= SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY)
    {
        L_HASH_ShMem_GetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, &sync_value, tmp_index);
        sync_value ^= AMTRDRV_OM_SYNC_VALUE_CALLBACK_QUEUE;
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, sync_value, tmp_index);
        /* If sync_value = 0 we need to trigger an SYNC DONE EV to run HASH FSM */
        if(sync_value == 0 )
        {
            L_HASH_ShMem_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,addr_entry);
        }
    }
    /* delete that node from callback_queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,tmp_index);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    *index = tmp_index;

    return TRUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_MarkAllDynamicRecord
 *-------------------------------------------------------------------------
 * PURPOSE: This function will mark all hash records.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTE   : When HW Learning, AMTRDRV Task will synchronize OM with chip.
 *          Procedure1: Mark all
 *          Procedure2: Sync. ASIC ARL Table with OM.
 *                      If Chip entry can be find in OM, remove OM record's mark.
 *          Procedure3: Delete all marked OM records(age out).
 *-------------------------------------------------------------------------*/
void AMTRDRV_OM_MarkAllDynamicRecord(void)
{
    UI32_T               task_id;

    /* There are two method to protect OM.
     * 1. non-preempty. 2. raise task priority to highest.
     * Method1 has less cost. but if non-preempty too long time, there are messages from interrupt level lost.
     * So, if AMTRDRV access few OM records, it will use non-preempty to protect OM.
     * Otherwise, AMTRDRV raises task priority to protect OM.
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    task_id = SYSFUN_TaskIdSelf();
    L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,
                               AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,
                               AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT-1,
                               0,AMTRDRV_OM_MarkAddrCallBack);
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetMarkedEntry
 *-------------------------------------------------------------------------
 * PURPOSE: Caller can sequencing get marked entry by this function.
 * INPUT  : None
 * OUTPUT : UI8_T   *addr_entry -- OM record
 * RETURN : BOOL_T  TRUE        -- success to get a marked entry.
 *                  FALSE       -- there is no marked entry in OM.
 * NOTE   : This function querys del_on_timeout group to find marked entry.
 *          We identify a point to start by a static pointer.
 *          If this static pointer is NULL, this fuction will search from the beginning.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetMarkedEntry(UI8_T *addr_entry)
{
    L_DLST_Index_T          index;
    UI8_T                   action;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    /* Search marked entries by query group can't be interrupt.
     * When searching query group, we find next link by current index.
     * If UI deleted hash record which is this current index, the searching can't get next index.
     * So, AMTRDRV will keep a index. We will start from this index next checking.
     * ex. query group: A-> B ->c
     *      1. A, B, C are marked entries.
     *      2. task collect A, B then notify core layer.
     *      3. context switch, UI delete B. B->NULL.
     *      4. task can't find C by B-> next.
     */
    if (shmem_data_p->amtrdrv_om_wait_age_out_checking_index==AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX)
    {
        /* search from beginning of query group
         */
        if (L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(L_CVRT_GET_PTR(shmem_data_p,(uintptr_t)shmem_data_p->amtrdrv_om_dyn_dblist),&index) == FALSE)
        {
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
    }
    else if (shmem_data_p->amtrdrv_om_wait_age_out_checking_index==AMTRDRV_OM_SEARCH_END)
    {
        shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX;
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    else
    {
        index = shmem_data_p->amtrdrv_om_wait_age_out_checking_index;
    }

    do
    {

        L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,addr_entry,&action);

        if (((AMTRDRV_TYPE_Record_T *)addr_entry)->mark == TRUE)
        {
            /* save next index which will be checked next time.
             */
            if (L_DLST_ShMem_Indexed_Dblist_GetNextEntry(L_CVRT_GET_PTR(shmem_data_p,(uintptr_t)shmem_data_p->amtrdrv_om_dyn_dblist), &index) == TRUE)
            {
                shmem_data_p->amtrdrv_om_wait_age_out_checking_index = index;
            }
            else
            {
                /* Because function will return TRUE, can't set amtrdrv_om_wait_age_out_checking_index
                 * = AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX.
                 * If amtrdrv_om_wait_age_out_checking_index == AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX,
                 * This function will get first entry next time, then return TRUE again.
                 * Even if only one marked entry is exist, this function would output the same entry every time.
                 */
                shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_END;
            }
            AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }

    }while(L_DLST_ShMem_Indexed_Dblist_GetNextEntry(L_CVRT_GET_PTR(shmem_data_p,(uintptr_t)shmem_data_p->amtrdrv_om_dyn_dblist), &index)==TRUE);
    /* There is no marked entry in OM
     */
    shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_FROM_HEAD_INDEX;
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearMark
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set "mark field" to FALSE.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is for AMTR Hardware Learning.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ClearMark(UI8_T *addr_entry)
{
    /* Local Variable Declaration
      */
    L_HASH_Index_T record_index;
    UI8_T     *record;
    /* BODY
     */
    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if (L_HASH_ShMem_GetRecordIndex(&shmem_data_p->amtrdrv_om_hash_desc, addr_entry, &record_index)==TRUE)
    {
        L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&record,record_index);
        ((AMTRDRV_TYPE_Record_T *)record)->mark = FALSE;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return;
}


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DelHashRecord
 * -------------------------------------------------------------------------
 * PURPOSE: Delete a record from HASH database.
 * INPUT  : desc - A specific desciptor to process.
 *          rec  - A specific record to be removed.
 * OUTPUT : index - hash record index.
 * RETURN : TRUE \ FALSE
 * NOTES  : 1. When HW Learning, all functions call AMTRDRV_OM_LocalDelAddr() instead of
 *             AMTRDRV_OM_LocalDelAddr(). In AMTRDRV_OM_LocalDelAddr(), it will check index
 *             then calling AMTRDRV_OM_LocalDelAddr().
 *          2. This function can't enter om critical section.
 *             Caller will enter om critical section then call this function.
 * -------------------------------------------------------------------------*/
static BOOL_T AMTRDRV_OM_DelHashRecord (L_HASH_ShMem_Desc_T *desc, UI8_T  *user_rec, L_HASH_Index_T *index)
{
    BOOL_T retval;

    /* If record was deleted form OM, it would be removed from all guery group at the same time.
     * We want to get the next index. This function have to be called earlier than deletion.
     */
    retval = L_HASH_ShMem_GetRecordIndex(desc,user_rec,index);

    if ((retval == TRUE)&& (*index == shmem_data_p->amtrdrv_om_wait_age_out_checking_index))
    {
        if (L_DLST_ShMem_Indexed_Dblist_GetNextEntry(L_CVRT_GET_PTR(shmem_data_p,(uintptr_t)shmem_data_p->amtrdrv_om_dyn_dblist), (L_DLST_Index_T *)&shmem_data_p->amtrdrv_om_wait_age_out_checking_index) == FALSE)
        {
            /* Can't find next entry
             */
            shmem_data_p->amtrdrv_om_wait_age_out_checking_index = AMTRDRV_OM_SEARCH_END;
        }
    }

    retval = L_HASH_ShMem_DeleteRecord(desc, user_rec,index);
    return retval;
}

#endif

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total counter.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : numbers of total counter
 * NOTES  : Total counter means how many entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalCounter(void)
{
    UI32_T total_address_count;

    AMTRDRV_OM_ATOM_EXPRESSION(total_address_count=shmem_data_p->counters.total_address_count)
    return total_address_count;
}


/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalDynamicCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total dynamic counter in system
 * INPUT  : None
 * OUTPUT : None
 * RETURN : numbers of total dynamic counter
 * NOTES  : Total counter means how many dynamic entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalDynamicCounter()
{
    UI32_T total_dynamic_address_count;

    AMTRDRV_OM_ATOM_EXPRESSION(total_dynamic_address_count=shmem_data_p->counters.total_dynamic_address_count)
    return total_dynamic_address_count;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetTotalStaticCounter
 *------------------------------------------------------------------------
 * PURPOSE: This function will get total static counter in system.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : number of total static counter
 * NOTES  : Total counter means how many static entries in this system.
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetTotalStaticCounter()
{
    UI32_T total_static_address_count;

    AMTRDRV_OM_ATOM_EXPRESSION(total_static_address_count=shmem_data_p->counters.total_static_address_count)
    return total_static_address_count;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetStaticCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get static counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of static counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetStaticCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_port=shmem_data_p->counters.static_address_count_by_port[ifindex-1])
    return address_count_by_port;
}


/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetDynCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of dynamic counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetDynCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_port=shmem_data_p->counters.dynamic_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetDynCounterByVid
 *------------------------------------------------------------------------
 * PURPOSE: This function will get dynamic counter by specific vid.
 * INPUT  : vid	       -specific vid
 * OUTPUT : None
 * RETURN : number of dynamic counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetDynCounterByVid(UI32_T vid)
{
    UI32_T address_count_by_vid;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_vid=shmem_data_p->counters.dynamic_address_count_by_vid[vid-1])
    return address_count_by_vid;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetLearntCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get learnt counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of learnt counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetLearntCounterByport(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_port=shmem_data_p->counters.learnt_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetSecurityCounterByport
 *------------------------------------------------------------------------
 * PURPOSE: This function will get security counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of security counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetSecurityCounterByport(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_port=shmem_data_p->counters.security_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetConfigCounterByPort
 *------------------------------------------------------------------------
 * PURPOSE: This function will get configuration addresses counter by specific port.
 * INPUT  : ifindex	-specific port
 * OUTPUT : None
 * RETURN : number of config counter
 * NOTES  : None
 *------------------------------------------------------------------------*/
UI32_T AMTRDRV_OM_GetConfigCounterByPort(UI32_T ifindex)
{
    UI32_T address_count_by_port;

    AMTRDRV_OM_ATOM_EXPRESSION(address_count_by_port=shmem_data_p->counters.config_address_count_by_port[ifindex-1])
    return address_count_by_port;
}

#ifdef BACKDOOR_OPEN
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ShowAddrInfo
 *------------------------------------------------------------------------------
 * PURPOSE: This function will show all addresses info which are located in Hash table
 * INPUT  : None
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : cookie is discarded in this function
 *          And this function is only call by backdoor function to dump all records
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_ShowAddrInfo()
{
    /* Local Variable Declaration
     */

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();

    if(!L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,AMTR_TYPE_ADDRESS_LIFETIME_OTHER-1,0,AMTRDRV_OM_ShowAddrCallBack))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    if(!L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT-1,0,AMTRDRV_OM_ShowAddrCallBack))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    if(!L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET-1,0,AMTRDRV_OM_ShowAddrCallBack))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    if(!L_HASH_ShMem_SearchByQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc,AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME,AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT-1,0,AMTRDRV_OM_ShowAddrCallBack))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;

}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ShowRecordsInfoInAgingOutBuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function will show addresses info which are in aging out buffer
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : cookie is discarded in this function
 *          And this function is only call by backdoor function to dump records info
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ShowRecordsInfoInAgingOutBuffer()
{
    UI32_T head,i;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    head = shmem_data_p->amtrdrv_om_agingout_buffer_head;
    printf(" \r\n The records in the aging out buffer: \r\n ");

    for(i=0;i<shmem_data_p->amtrdrv_om_agingout_counter;i++)
    {
        printf("  Mac:%02x-%02x-%02x-%02x-%02x-%02x",
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[0],
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[1],
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[2],
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[3],
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[4],
            shmem_data_p->amtrdrv_om_agingout_buffer[head].mac[5]);
        printf("  Vid:%d\r\n",shmem_data_p->amtrdrv_om_agingout_buffer[head].vid);
        i++;
        head = (head + 1)%AMTRDRV_OM_TOTAL_NUMBER_OF_AGINGOUT_BUFFER;
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ShowRecordInfoInCheckingAgingList
 *------------------------------------------------------------------------------
 * PURPOSE: This function will show addresses info which are in checking aging list
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : cookie is discarded in this function
 *          And this function is only call by backdoor function to dump records info
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ShowRecordInfoInCheckingAgingList()
{
    AMTRDRV_TYPE_Record_T address_record;
    L_DLST_Index_T      index;
    UI8_T               action;
    BOOL_T              ret;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    printf("\r\n The reocrds in local aging checking list :\r\n");
    ret = L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->local_aging_check_list_desc,&index);

    while(ret)
    {
        L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record, &action);
        printf("  Mac:%02x-%02x-%02x-%02x-%02x-%02x",
            address_record.address.mac[0], address_record.address.mac[1], address_record.address.mac[2], address_record.address.mac[3], address_record.address.mac[4], address_record.address.mac[5]);
        printf("  Vid:%d",address_record.address.vid);
        printf("  Ifindex:%d",address_record.address.ifindex);
        printf("  Source:%d",address_record.address.source);
        printf("  Life_time:%d",address_record.address.life_time);
        printf("  Record_action(set/delete):%d \r\n",action);

        ret =L_DLST_ShMem_Indexed_Dblist_GetNextEntry(&shmem_data_p->local_aging_check_list_desc, &index);

    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
    return;
}
#endif/*End of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get synq debug info flag which is used by
 *          AMTRDRV backdoor.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : The synq debug info flag
 * NOTES  : 1. AMTRDRV will show synq debug message based on the synq debug info
 *             flag.
 *          2. This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
UI8_T AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo(void)
{
    return shmem_data_p->synq_debug_info;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set synq debug info flag which is used by
 *          AMTRDRV backdoor.
 * INPUT  : The value to be set to synq debug info flag.
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 1. AMTRDRV will show synq debug message based on the synq debug info
 *             flag.
 *          2. This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo(BOOL_T synq_debug_info)
{
    shmem_data_p->synq_debug_info = synq_debug_info;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo
 *------------------------------------------------------------------------------
 * PURPOSE: This function will dump the synq event id of all lports.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_BACKDOOR_DumpSynqPortEventId(void)
{
    L_DLST_Index_T index;
    UI32_T event_id;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    for(index=SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY;
        index < SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY+AMTR_TYPE_MAX_LOGICAL_PORT_ID;
        index++)
    {
        if(L_DLST_ShMem_Indexed_Dblist_GetEntryID(&shmem_data_p->sync_queue_desc, index, &event_id)==TRUE)
        {
            printf("index[%05lu]ifindex[%03d]:event_id=%08lX\r\n",
                   (unsigned long)index,(int)(index-SYS_ADPT_MAX_NBR_OF_L2_MAC_ADDR_ENTRY+1),
                   (unsigned long)event_id);
        }
    }
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_DumpSynQ
 *------------------------------------------------------------------------------
 * PURPOSE: This function will dump the content of syncq
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_BACKDOOR_DumpSynQ(void)
{
    L_DLST_Index_T index;

    AMTRDRV_OM_ENTER_CRITICAL_SECTION();
    if(!L_DLST_ShMem_Indexed_Dblist_GetFirstEntry(&shmem_data_p->sync_queue_desc,&index))
    {
        AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
        printf("SynQ is empty.\r\n");
        return;
    }

    printf("[%05lu] ", (unsigned long)index);

    while(L_DLST_ShMem_Indexed_Dblist_GetNextEntry(&shmem_data_p->sync_queue_desc, &index) == TRUE)
    {
        printf("[%05lu] ", (unsigned long)index);
    }
    printf("\r\n");
    AMTRDRV_OM_LEAVE_CRITICAL_SECTION();
}

#endif /* end of #ifdef BACKDOOR_OPEN */

/* Local SubRoutine ***************************************************************/

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_GetElementByGroupEntry
 *------------------------------------------------------------------------------
 * Purpose  : This function is callback function to provide the service to insert
 *            the node to the suitable query groups
 * INPUT    : group    -- which query group
 *            record   -- user record
 *            element  -- which element
 * OUTPUT   : None
 * RETURN   : TRUE /FALSE
 * NOTES    :
 *            1. This function will call by l_hash.c, so it could not add semaphore
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetElementByGroupEntry(UI32_T group, void *record, UI32_T *element)
{
    if (group >= L_HASH_MAX_QUERY_GROUP)
    {
        return FALSE;
    }
    if ((record == 0) || (element == 0))
    {
       return FALSE;
    }

    switch(group)
    {
        case 0:
            *element = ((AMTR_TYPE_AddrEntry_T *)record)->life_time - 1 ;
            break;
        case 1:
            *element = ((AMTR_TYPE_AddrEntry_T *)record)->ifindex;
            break;
        case 2:
            *element = ((AMTR_TYPE_AddrEntry_T  *)record)->vid - 1;
            break;
        case 3:
            *element = ((AMTR_TYPE_AddrEntry_T  *)record)->source -1;
            break;
        default:
            break;
    }
    return TRUE;
} /* AMTRDRV_GetElementByGroupEntry() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetRecordByIndex
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get the record info by giving index
 * INPUT  : UI32_T           index         - the index for hash_record
 * OUTPUT : AMTR_TYPE_AddrEntry_T *addr_entry   - user record
 *                                *action       - record action
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_OM_GetRecordByIndex(UI32_T index, UI8_T *addr_entry, UI8_T *action)
{
    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,addr_entry,action);
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_UpdateRecordLifeTime_Callback
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update record time stamp
 *          as system time.
 * INPUT  : UI32_T index     - the index for hash_record
 *          void * cookie    - arg1 : is_update_local_aging_list  arg2 : life_time
 * OUTPUT : None
 * RETURN : UI32_T
 * NOTES  : This API only return L_DLST_SEARCH_CONTINUE
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_UpdateRecordLifeTime_Callback(L_HASH_Index_T index, void *cookie)
{
    UI8_T   *addr_entry;


    L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);

    if(((AMTRDRV_TYPE_Record_T *)addr_entry)->address.source == AMTR_TYPE_ADDRESS_SOURCE_LEARN)
    {
        UI8_T state;

        /* decrease counter for original life_time by calling AMTRDRV_LIB_UpdateCount()
         */
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
             AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)addr_entry, FALSE, &(shmem_data_p->counters));
             L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        
        ((AMTRDRV_TYPE_Record_T *)addr_entry)->address.life_time = (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg2;
        /* increase counter for new life_time by calling AMTRDRV_LIB_UpdateCount()
         */
        AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)addr_entry, TRUE, &(shmem_data_p->counters));
        /* restore AMTR_ISSET_COUNTER_FLAG of reserved state
         */
        L_HASH_ShMem_ClearReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);

        /* EPR ID: ASF4512MP-FLF-EC-00292
         *         Need to change the attribute in chips to make OM and CHIP consistent
         *         use L_HASH_ShMem_SetRecord() to set entry again so that the entry will
         *         be added to job queue. Thus attribute in CHIP will be the same with OM
         *         finally.
         *         charlie_chen 2011/06/22
         */
        /* L_HASH_ShMem_UpdateQueryGroup() is performed in L_HASH_ShMem_SetRecord() already
        L_HASH_ShMem_UpdateQueryGroup(&shmem_data_p->amtrdrv_om_hash_desc, index,addr_entry);
         */
        L_HASH_ShMem_SetRecord(&shmem_data_p->amtrdrv_om_hash_desc, addr_entry, &index);
        /* At this point, both CHIP and OM contains the entry, but the attribute between
         * CHIP and OM is different, set AMTR_TYPE_OM_CHIP_ATTRIBUTE_NOT_SYNC_FLAGS for
         * this entry.
         */
        L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_OM_CHIP_ATTRIBUTE_NOT_SYNC_FLAGS);

        /* If the record is learnt on local unit and its attribute is dynamic
         * then need to insert to local_aging_check_list.Otherwise delete anyway
         */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        if((BOOL_T)((AMTRDRV_OM_Cookie_T *)cookie)->arg1)
        {
            L_DLST_ShMem_Indexed_Dblist_Enqueue(&shmem_data_p->local_aging_check_list_desc, index);
            ((AMTRDRV_TYPE_Record_T *)addr_entry)->aging_timestamp =SYSFUN_GetSysTick();
        }
        else
        {
            /* No matter the record in the list or not just delete any way
             * since if the record isn't in the list this API will doing nothing
             */
             L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc, index);
        }
#endif
    }

    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteAddrCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete a record
 * INPUT  : UI32_T index     - the index for hash_record
 *          void *cookie    - extra conditions value. If cookie =0 means no conditions
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : the cookie isn't used in this function
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteAddrCallBack(L_HASH_Index_T index, void *cookie)
{
    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;
    AMTRDRV_OM_QueryGroup_T query_group = (uintptr_t)cookie;
    
    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    /* 1. Update OM */
    AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
    /* 2. Update counter if deleteRecord success */
    
    L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
    if(!AMTR_ISSET_COUNTER_FLAG(state)){
        AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
        L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
    }
    /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
#endif

#endif
    /* 4. delete record from sync_queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
    /*5. delete record from callback queue */
    L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
    /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be deleted */
    L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
    /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
    /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/


    /* There are 4 blocked_commands will call this API:
     * AMTR_TYPE_COMMAND_DELETE_BY_PORT       (deleted by DEV_AMTRDRV_PMGR_DeleteDynamicAddrByPort())	
     * AMTR_TYPE_COMMAND_DELETE_BY_VID	      (deleted by DEV_AMTRDRV_PMGR_DeleteDynamicAddrByVlan())		
     * AMTR_TYPE_COMMAND_DELETE_BY_LIFETIME   (deleted by DEV_AMTRDRV_PMGR_DeleteAllDynamicAddr())
     * AMTR_TYPE_COMMAND_DELETE_BY_SOURCE     None gateway API for it.
     *
     * Because SDK cannot distinguish Source Learn and delete on timeout(Security), 
     * AMTR_TYPE_COMMAND_DELETE_BY_SOURCE must be done through job queue.
     */
    if( query_group==AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX   ||
        query_group==AMTRDRV_OM_QUERY_GROUP_BY_VID       ||
        query_group==AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME)
    {
        /* wakka,
         * only the entries deleted by AMTRDRV_MGR_DeleteFromChip
         * need to do following actions.
         */
        if (address_record.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
        {
            /* add by Tony.Lei
             * all the callback will delete the OM directly
             */
            /* EPR ID: ASF4512MP-FLF-EC-00292
             *         Do not remove the entry from job queue in hash if
             *         ATTRIBUTE between CHIP and OM is not synchronized.
             *         charlie_chen 2011/06/22
             */
            if(!AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(state))
                L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        }
    }
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteSourceCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which source is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          UI32_T source    - the specified source attribute
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteSourceCallBack(L_HASH_Index_T index, void *cookie)
{
    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T   state;
    AMTR_TYPE_AddressSource_T source = (uintptr_t)cookie;

    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    if(address_record.address.source == source)
    {
        /* 1. Update OM */
        AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        /* 2. Update counter if deleteRecord success */
        
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
            AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
            L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
#endif
#endif
        /* 4. delete record from sync_queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
        /*5. delete record from callback queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
        /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
        /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
        /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/
    }
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteLifeTimeCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which life_time is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          UI32_T life      - specified life attribute
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteLifeTimeCallBack(L_HASH_Index_T index, void *cookie)
{

    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;
    AMTR_TYPE_AddressLifeTime_T life_time = (uintptr_t)cookie;

    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    if(address_record.address.life_time== life_time)
    {
        /* 1. Update OM */
        AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        /* 2. Update counter if deleteRecord success */
        
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
            AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
            L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
#endif
#endif
        /* 4. delete record from sync_queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
        /*5. delete record from callback queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
        /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
        /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
        /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/

            /* wakka,
             * only the entries deleted by AMTRDRV_MGR_DeleteFromChip
             * need to do following actions.
             */
            if (address_record.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                /* add by Tony.Lei
                 * all the callback will delete the OM directly
                 */
                /* EPR ID: ASF4512MP-FLF-EC-00292
                 *         Do not remove the entry from job queue in hash if
                 *         ATTRIBUTE between CHIP and OM is not synchronized.
                 *         charlie_chen 2011/06/22
                 */
                if(!AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(state))
                    L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
            }
        }     
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteVidCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which vid is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          UI32_T vid       - specified vid
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteVidCallBack(L_HASH_Index_T index, void *cookie)
{

    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;
    UI32_T vid = (uintptr_t)cookie;

    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    if(address_record.address.vid == vid)
    {
        /* 1. Update OM */
        AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        /* 2. Update counter if deleteRecord success */
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
            AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
            L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc, index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc, index);
#endif
#endif
        /* 4. delete record from sync_queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
        /*5. delete record from callback queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
        /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
        /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
        /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/
        
        /* wakka,
         * only the entries deleted by AMTRDRV_MGR_DeleteFromChip
         * need to do following actions.
         */
        if (address_record.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
        {
            /* add by Tony.Lei
             * all the callback will delete the OM directly
             */
            /* EPR ID: ASF4512MP-FLF-EC-00292
             *         Do not remove the entry from job queue in hash if
             *         ATTRIBUTE between CHIP and OM is not synchronized.
             *         charlie_chen 2011/06/22
             */
            if(!AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(state))
                L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        }
    }
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteVidAndLifeTimeCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which vid & life_time is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          void * cookie    - this field include two variable- condition 1 = vid & condition 2 = life_time
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteVidAndLifeTimeCallBack(L_HASH_Index_T index, void *cookie)
{

    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;
    UI32_T vlan_index;
    UI16_T *  vlan_list =(UI16_T*)((AMTRDRV_OM_Cookie_T *)cookie)->arg1;
    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);
    
    for(vlan_index = 0;vlan_list[vlan_index]!=0&&(vlan_index<(uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg3);vlan_index++){
        
        
        if(address_record.address.vid == vlan_list[vlan_index]
            &&(address_record.address.life_time== (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg2))
        {
            /* 1. Update OM */
            AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
            /* 2. Update counter if deleteRecord success */
            
            L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
            if(!AMTR_ISSET_COUNTER_FLAG(state)){
                AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
                L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
            }
            /* 3. delete record from local_aging_check_list */
    #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
            L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
    #if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
            L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
    #endif
    #endif
            /* 4. delete record from sync_queue */
            L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
            /*5. delete record from callback queue */
            L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
            /*6.Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
            L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
            /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
            /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/

            /* wakka,
             * only the entries deleted by AMTRDRV_MGR_DeleteFromChip
             * need to do following actions.
             */
            if (address_record.address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                /* add by Tony.Lei
                 * all the callback will delete the OM directly
                 */
                /* EPR ID: ASF4512MP-FLF-EC-00292
                 *         Do not remove the entry from job queue in hash if
                 *         ATTRIBUTE between CHIP and OM is not synchronized.
                 *         charlie_chen 2011/06/22
                 */
                if(!AMTR_ISSET_OMCHIPATTRIBUTENOTSYNC_FLAG(state))
                    L_HASH_ShMem_DeleteRecordFromHashOnly(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
            }
        }
    }
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteVidAndSourceCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which vid & source is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          void * cookie    - this field include two variable- condition 1 = vid & condition 2 = source
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteVidAndSourceCallBack(L_HASH_Index_T index, void *cookie)
{

    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;

    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    if((address_record.address.vid== (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg1)
       &&(address_record.address.source== (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg2))
    {
        /* 1. Update OM */
        AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        /* 2. Update counter if deleteRecord success */
        
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
            AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
            L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
#endif
#endif
        /* 4. delete record from sync_queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
        /*5. delete record from callback queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
        /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
        /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
        /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/
    }
    return L_DLST_SEARCH_CONTINUE;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_DeleteVidExceptCertainAddrCallback
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to provide the service to update delete specified
 *          addresses which vid & source is matched.
 * INPUT  : UI32_T index     - the index for hash_record
 *          void * cookie    - this field include two variable- condition 1 = vid & condition 2 = source
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_DeleteVidExceptCertainAddrCallback(L_HASH_Index_T index, void *cookie)
{

    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    UI8_T state;

    UI32_T      mac_mask_index;
    UI32_T      mac_index;
    UI32_T      vid = (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg1;
    UI8_T       (*mac_list_p)[SYS_ADPT_MAC_ADDR_LEN] = (UI8_T(*)[])((AMTRDRV_OM_Cookie_T *)cookie)->arg2;
    UI8_T       (*mask_list_p)[SYS_ADPT_MAC_ADDR_LEN] = (UI8_T(*)[])((AMTRDRV_OM_Cookie_T *)cookie)->arg3;
    UI32_T      number_of_entry_in_list = (uintptr_t)((AMTRDRV_OM_Cookie_T *)cookie)->arg4;


    L_HASH_ShMem_GetRecordByIndex(&shmem_data_p->amtrdrv_om_hash_desc,index,(UI8_T *)&address_record,&action);

    if (address_record.address.vid != vid)
    {
        return L_DLST_SEARCH_CONTINUE;
    }

    /* check record is matched with mac_list or not.
     */
    for (mac_mask_index = 0; mac_mask_index < number_of_entry_in_list; mac_mask_index++)
    {
        for (mac_index = 0; mac_index < SYS_ADPT_MAC_ADDR_LEN; mac_index++)
        {
            if ((address_record.address.mac[mac_index] & mask_list_p[mac_mask_index][mac_index]) !=
                (mac_list_p[mac_mask_index][mac_index] & mask_list_p[mac_mask_index][mac_index]))
            {
                break;
            }
        }
        if (mac_index >= SYS_ADPT_MAC_ADDR_LEN)
        {
            return L_DLST_SEARCH_CONTINUE;
        }
    }

    {
        /* 1. Update OM */
        AMTRDRV_OM_LocalDelAddr(&shmem_data_p->amtrdrv_om_hash_desc,(UI8_T *)&address_record,&index);
        /* 2. Update counter if deleteRecord success */
        
        L_HASH_ShMem_GetReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,&state);
        if(!AMTR_ISSET_COUNTER_FLAG(state)){
            AMTRDRV_LIB_UpdateCount((AMTR_TYPE_AddrEntry_T *)&address_record, FALSE, &(shmem_data_p->counters));
            L_HASH_ShMem_UpdateReservedState(&shmem_data_p->amtrdrv_om_hash_desc,index,AMTR_TYPE_COUNTER_FLAGS);
        }
        /* 3. delete record from local_aging_check_list */
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->local_aging_check_list_desc,index);
#if(AMTR_TYPE_ENABLE_RESEND_MECHANISM == 1)
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->slave_aging_ak_list_desc,index);
#endif
#endif
        /* 4. delete record from sync_queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->sync_queue_desc,index);
        /*5. delete record from callback queue */
        L_DLST_ShMem_Indexed_Dblist_DeleteEntry(&shmem_data_p->call_back_queue_desc,index);
        /*6. Set sync_value = 0 & trigger a SYNC_DONE_EVENT to tell FSM this entry can be delete */
        L_HASH_ShMem_SetRecordSyncValueByIndex(&shmem_data_p->amtrdrv_om_hash_desc, 0, index);
        /* In this case this operation is useless since the record state in the dirty state and its nothing to do with SYNC_DONE_EV*/
        /* L_HASH_OperationResult(&shmem_data_p->amtrdrv_om_hash_desc,L_HASH_SYNC_DONE_EV,(UI8_T *)&address_record);*/
    }
    return L_DLST_SEARCH_CONTINUE;
}

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_MarkAddrCallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This is call back function to mark OM record.
 * INPUT  : UI32_T index     - the index for hash_record
 *          void * cookie    - this field include two variable- condition 1 = vid & condition 2 = source
 * OUTPUT : None
 * RETURN : L_DLST_SEACH_CONTINUE (search for next record)
 * NOTES  : Parameter "cookie" is useless in this callback function
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_MarkAddrCallBack(L_HASH_Index_T index, void *cookie)
{
    UI8_T   *addr_entry;
    L_HASH_ShMem_GetRecordPtrByIndex(&shmem_data_p->amtrdrv_om_hash_desc,&addr_entry,index);
    if ( ((AMTRDRV_TYPE_Record_T *)addr_entry)->address.life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
    {
        ((AMTRDRV_TYPE_Record_T *)addr_entry)->mark = TRUE;
    }
    return L_DLST_SEARCH_CONTINUE;
}
#endif

#ifdef BACKDOOR_OPEN
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_ShowAddrCallBack
 *------------------------------------------------------------------------------
 * Purpose  : This function is callback function to provide the service for
 *            showing all entries' infomation
 * INPUT    : UI32_T index   - the hash record's index in OM
 *            UI32_T cookie  - the checking condition
 * OUTPUT   : None
 * RETURN   : UI32_T L_DLST_SEARCH_CONTINUE
 * NOTES    :
 *-----------------------------------------------------------------------------*/
static UI32_T AMTRDRV_OM_ShowAddrCallBack(L_HASH_Index_T index, void *cookie)
{
    AMTRDRV_TYPE_Record_T address_record;
    UI8_T               action;
    AMTRDRV_OM_GetRecordByIndex(index,(UI8_T *)&address_record,&action);

    printf("  Mac:%02x-%02x-%02x-%02x-%02x-%02x",
           address_record.address.mac[0], address_record.address.mac[1], address_record.address.mac[2], address_record.address.mac[3], address_record.address.mac[4], address_record.address.mac[5]);
    printf("  Vid:%d",address_record.address.vid);
    printf("  Ifindex:%d",address_record.address.ifindex);
    printf("  Source:%d",address_record.address.source);
    printf("  Life_time:%d",address_record.address.life_time);
    printf("  FSM_action:%d \r\n",action);

    return L_DLST_SEARCH_CONTINUE;

}
#endif /* BACKDOOR_OPEN */

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_OM_InitCount
 *------------------------------------------------------------------------------
 * Purpose  : This function is to initialize the counters value
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *-----------------------------------------------------------------------------*/
static void AMTRDRV_OM_InitCount(void)
{
    shmem_data_p->counters.total_address_count = 0;
    shmem_data_p->counters.total_dynamic_address_count = 0;
    shmem_data_p->counters.total_static_address_count = 0;
    memset((UI32_T *)shmem_data_p->counters.static_address_count_by_port, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_PORT_ID);	/*water_huang add*/
    memset((UI32_T *)shmem_data_p->counters.learnt_address_count_by_port, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_PORT_ID);  /*water_huang add*/
    memset((UI32_T *)shmem_data_p->counters.security_address_count_by_port, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_PORT_ID);
    memset((UI32_T *)shmem_data_p->counters.dynamic_address_count_by_port, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_PORT_ID);	/*water_huang add*/
    memset((UI32_T *)shmem_data_p->counters.config_address_count_by_port, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_PORT_ID);	/*water_huang add*/
    memset((UI32_T *)shmem_data_p->counters.static_address_count_by_vid, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_VLAN_ID);	        /*water_huang add*/
    memset((UI32_T *)shmem_data_p->counters.dynamic_address_count_by_vid, 0, sizeof(UI32_T)*AMTR_TYPE_MAX_LOGICAL_VLAN_ID);
    return;
}


