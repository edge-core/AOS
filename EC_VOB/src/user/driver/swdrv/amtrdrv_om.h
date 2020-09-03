/*-------------------------------------------------------------------------
 * MODULE NAME: AMTRDRV_OM.h
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

#ifndef AMTRDRV_OM_H
#define AMTRDRV_OM_H

#define BACKDOOR_OPEN

#include "sys_type.h"
#include "sys_adpt.h"
#include "sys_cpnt.h"
#include "amtr_type.h"
#include "amtrdrv_pom.h"
#include "l_dlist.h"
#include "sysrsc_mgr.h"

/*-----------------
 * NAMING CONSTANT
 *----------------*/

/* need to be consistent with the enum definition AMTR_TYPE_AddressLifeTime_T
 * defined in amtr_type.h
 */
#define AMTRDRV_OM_TOTAL_NUMBER_OF_LIFETIME              5

/* need to be consistent with the enum definition AMTR_TYPE_AddressSource_T
 * defined in amtr_type.h
 */
#define AMTRDRV_OM_TOTAL_NUMBER_OF_SOURCE                (AMTR_TYPE_ADDRESS_SOURCE_MAX-1)

/* for AMTRDRV backdoor
 */
#define AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_TRACE_MSG     BIT_0
#define AMTRDRV_OM_SYNCQ_DBG_FLAG_SHOW_ENQ_INFO      BIT_1
#define AMTRDRV_OM_SYNCQ_DBG_FLAG_FORCE_CLEAR_EVENT  BIT_2

/*----------------
 * MACRO FUNCTION
 *----------------*/

/*-----------------
 * DATA TYPES
 *----------------*/

/* The type of Query Group in Hash OM */
typedef enum
{
    AMTRDRV_OM_QUERY_GROUP_BY_LIFE_TIME =0,
    AMTRDRV_OM_QUERY_GROUP_BY_IFINDEX,
    AMTRDRV_OM_QUERY_GROUP_BY_VID,
    AMTRDRV_OM_QUERY_GROUP_BY_SOURCE
}AMTRDRV_OM_QueryGroup_T;

/* The type of Query Group in Hash OM */
typedef enum {
    AMTRDRV_OM_SYNC_CEASING,
    AMTRDRV_OM_SYNC_SYNCHRONIZING,
    AMTRDRV_OM_SYNC_ANNOUNCING,
} AMTRDRV_OM_SyncState_T;

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
typedef struct
{
    UI16_T vlan;                        /* hash collision vlan id */
    UI8_T  mac[SYS_ADPT_MAC_ADDR_LEN];  /* hash collision mac address */
    UI16_T count;                       /* count of occurence of hash collision */
}AMTRDRV_OM_CollisionMacEntry_T;
#endif
/*--------------------
 * EXPORTED CONSTANTS
 *-------------------*/

/*--------------------
 * EXPORTED ROUTINES
 *-------------------*/
BOOL_T  AMTRDRV_OM_GetElementByGroupEntry(UI32_T group, void *record, UI32_T *element);

SYS_TYPE_Stacking_Mode_T AMTRDRV_OM_GetOperatingMode(void);

void AMTRDRV_OM_SetTransitionMode(void);

void AMTRDRV_OM_SetAsicCommandTransitionDone(void);

void AMTRDRV_OM_SetAddressTransitionDone(void);

void AMTRDRV_OM_EnterTransitionMode(void);

void AMTRDRV_OM_EnterMasterMode(void);

void AMTRDRV_OM_EnterSlaveMode(void);

void AMTRDRV_OM_SetProvisionComplete(BOOL_T provision_complete);

BOOL_T AMTRDRV_OM_GetProvisionComplete(void);

void AMTRDRV_OM_SetSynchronizing(AMTRDRV_OM_SyncState_T synchronizing);

AMTRDRV_OM_SyncState_T AMTRDRV_OM_GetSynchronizing(void);

UI32_T AMTRDRV_OM_GetAsicComTaskId(void);

void AMTRDRV_OM_SetAsicComTaskId(UI32_T tid);

UI32_T AMTRDRV_OM_GetAddressTaskId(void);

void AMTRDRV_OM_SetAddressTaskId(UI32_T tid);

UI32_T AMTRDRV_OM_GetOperAgingTime(void);

void AMTRDRV_OM_SetOperAgingTime(UI32_T value);

UI32_T AMTRDRV_OM_GetMyDrvUnitId(void);

void AMTRDRV_OM_SetMyDrvUnitId(UI32_T value);

AMTR_TYPE_BlockedCommand_T * AMTRDRV_OM_GetBlockCommand(AMTR_TYPE_BlockedCommand_T *block_command_p);

void AMTRDRV_OM_SetBlockCommand(AMTR_TYPE_BlockedCommand_T *block_command_p);

BOOL_T AMTRDRV_OM_IsBlockCommandNull(void);

void AMTRDRV_OM_SetBlockCommandNull(void);

void AMTRDRV_OM_InitiateSystemResources(void);

void AMTRDRV_OM_AttachSystemResources(void);

void AMTRDRV_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

BOOL_T AMTRDRV_OM_GetNAEventToAmtr(void);


void AMTRDRV_OM_SetAmtrID(UI32_T tid);

UI32_T AMTRDRV_OM_GetAmtrID(void);


/* -------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_L2TableInit
 * -------------------------------------------------------------------------
 * PURPOSE: This function is to create amtrdrv_om(Hash table).
 * INPUT  : None.
 * OUTPUT : None.
 * RETURN : TRUE / FALSE
 * NOTES  :
 * -------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_L2TableInit(void);

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
BOOL_T AMTRDRV_OM_NABufferInit(void);
#else   /*AMTR HW Learning*/
#define AMTRDRV_OM_NABufferInit() TRUE
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
BOOL_T AMTRDRV_OM_ClearDatabase(void);

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
BOOL_T AMTRDRV_OM_ClearNABuffer(void);
#else
#define AMTRDRV_OM_ClearNABuffer()
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_SetAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set address table entry into Hash table & chip
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT :                       index        -- hash_record_index
 * RETURN : BOOL_T Status                 - True : successs, False : failed
 * NOTES  : This function will set the record into OM and through OM's FSM to
 *          program chip.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SetAddr(UI8_T *addr_entry, UI32_T *index);

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
BOOL_T AMTRDRV_OM_SetAddrHashOnly(UI8_T *addr_entry, UI32_T *record_index);

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
BOOL_T AMTRDRV_OM_DeleteAddr(UI8_T *addr_entry, UI32_T *index);

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
BOOL_T  AMTRDRV_OM_DeleteAddrFromHashOnly(UI8_T *addr_entry, UI32_T *index);

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
UI8_T AMTRDRV_OM_GetReseverdState(UI32_T index);

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
void AMTRDRV_OM_SetReseverdState(UI32_T index,UI8_T flags);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearReseverdState
 *------------------------------------------------------------------------------
 * PURPOSE: This function will reset the reserve field,it indicate the addr need 
 *          delete from chip or not
 * INPUT  : index -- hash_record index,flags-----AMTR_TYPE_DONOTDELETEFROMCHIP_FLAGS
 * OUTPUT : None
 * RETURN : None
 * NOTES  : 
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ClearReseverdState(UI32_T index,UI8_T flags);



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
BOOL_T  AMTRDRV_OM_DeleteAddrFromHashAndWaitSync(UI8_T *addr_entry, UI32_T *index);

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
 * OUTPUT : None
 * RETURN : BOOL_T Status - True : successs, False : failed
 * NOTES  : if the cookie isn't used please fill 0 in the function.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_SearchNDelete(UI32_T ifindex,UI32_T vid,UI32_T life_time,UI32_T source, AMTR_TYPE_Command_T mode,UI16_T *vlan_p,UI32_T vlan_counter);

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
                                                 UI32_T number_of_entry_in_list);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetRecordIndex
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get record index by giving address info
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry - user record
 * OUTPUT : UI32_T index                      - the index for hash_record
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetRecordIndex(UI8_T *addr_entry, UI32_T *index);

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
BOOL_T AMTRDRV_OM_DequeueJobList(UI8_T *addr_entry,UI8_T *action,UI32_T *idx);

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
BOOL_T AMTRDRV_OM_SetOperationResultEvent(UI8_T *addr_entry,UI32_T event_type);

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
BOOL_T AMTRDRV_OM_UpdateRecordLifeTime(UI32_T ifindex,UI32_T life_time,BOOL_T is_update_local_aging_list);

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
BOOL_T AMTRDRV_OM_UpdateLocalRecordHitBitValue(UI32_T index,UI8_T hitbit_value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_UpdateSystemTrunkHitBit
 *------------------------------------------------------------------------------
 * PURPOSE: This function will Update Record's trunk hitbit value
 * INPUT  : index          --Hash_record_index
 *        : hitbit_value   --hitbit value
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTE   : This API is used in master mode.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_UpdateSystemTrunkHitBit(UI32_T index,UI16_T hitbit_value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get first entry from giving query group
 * INPUT  : query_group - A specific query category to delete.
 *          query_element - Index of the specific category to operate.
 * OUTPUT : L_DLST_Indexed_Dblist_T dblist     -- the specific query group's descriptor
 *          UI32_T                  index      -- first entry's index
 *          UI8_T                   addr_entry -- address info
 *          UI8_T                   action     -- record action
 * RETURN : TRUE / FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetFirstEntryFromGivingQueryGroup(UI32_T query_group,UI32_T query_element,L_DLST_ShMem_Indexed_Dblist_T **dblist,UI32_T *index,UI8_T *addr_entry,UI8_T *action);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_GetNextEntryFromGivingQueryGroup
 *------------------------------------------------------------------------------
 * PURPOSE: This function will get the next entry by index from giving query group
 * INPUT  : L_DLST_Indexed_Dblist_T dblist     -- the specific query group's descriptor
          : UI32_T                  index      -- the giving index
 * OUTPUT : UI32_T                  index      -- the new record's index
 *          UI8_T                   addr_entry -- address info
 *          UI8_T                   action     -- record action
 * RETURN : TRUE /FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_GetNextEntryFromGivingQueryGroup(L_DLST_ShMem_Indexed_Dblist_T *dblist,UI32_T *index,UI8_T *addr_entry,UI8_T *action);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
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
BOOL_T AMTRDRV_OM_NABufferEnqueue(UI8_T *addr_entry);

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
,UI32_T *queue_counter
#endif
);

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
BOOL_T AMTRDRV_OM_AgingOutBufferEnqueue(UI8_T *mac, UI16_T vid, UI16_T ifindex);

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
UI32_T AMTRDRV_OM_AgingOutBufferDequeue(UI32_T num_of_entry,AMTR_TYPE_AddrEntry_T addr_buff[]);

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
BOOL_T AMTRDRV_OM_IsAgingOutBufferFull(UI32_T checking_number);

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
BOOL_T AMTRDRV_OM_LocalAgingListUpdateTimeStamp();

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListEnqueue
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to local aging out check list
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListEnqueue(UI32_T index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAginListDeleteEntry
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to delete MAC entry from aginglist
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAginListDeleteEntry(UI32_T index);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_LocalAgingListEnqueue2Head
 *-------------------------------------------------------------------------
 * PURPOSE: This function is to insert node to the head of local aging out check list
 * INPUT  : index -- The record's hash index
 * RETURN : TRUE / FALSE
 * NOTES  : If the entry is learnt on this unit and its life time is delete on time out.
 *          But if the entry is learnt on trunk port the master always keep one record.
 *-------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_LocalAgingListEnqueue2Head(UI32_T index);

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
BOOL_T AMTRDRV_OM_LocalAgingListDequeue(UI32_T aging_time,UI32_T *index,UI8_T *addr_entry);
#else
#define AMTRDRV_OM_NABufferEnqueue(addr_entry) FALSE
#define AMTRDRV_OM_NABufferDequeue(addr_entry) FALSE
#define AMTRDRV_OM_AgingOutBufferEnqueue(mac, vid, ifindex) FALSE
#define AMTRDRV_OM_AgingOutBufferDequeue(num_of_entry,addr_buff) 0
#define AMTRDRV_OM_IsAgingOutBufferFull(checking_number) TRUE
#define AMTRDRV_OM_LocalAgingListUpdateTimeStamp()
#define AMTRDRV_OM_LocalAgingListEnqueue(index)
#define AMTRDRV_OM_LocalAginListDeleteEntry(index)
#define AMTRDRV_OM_LocalAgingListEnqueue2Head(index)
#define AMTRDRV_OM_LocalAgingListDequeue(aging_time,index,addr_entry) FALSE
#endif

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
BOOL_T AMTRDRV_OM_SyncQueueEnqueue(UI32_T index);

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
BOOL_T AMTRDRV_OM_ClearCollisionVlanMacTable(void);

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
 *            AMTRDRV_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX.
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_OM_GetNextEntryOfCollisionVlanMacTable(UI8_T* idx_p, UI16_T* vlan_id_p, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T *count_p);

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
void AMTRDRV_OM_LogCollisionVlanMac(UI16_T vlan_id, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN]);

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
BOOL_T AMTRDRV_OM_SyncQueueGetEntryID(UI32_T index,UI32_T *dlist_id);

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
BOOL_T AMTRDRV_OM_SyncQueueDequeue(UI32_T *index,UI32_T *dlist_id,UI8_T *addr_entry,UI8_T *action);

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
BOOL_T AMTRDRV_OM_SyncQueueDeleteEntry(UI32_T index);

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
BOOL_T AMTRDRV_OM_CallBackQueueEnqueue(UI32_T index);

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
void AMTRDRV_OM_CallBackQueueSetEntryID(UI32_T index,UI32_T dlist_id);

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
BOOL_T AMTRDRV_OM_CallBackQueueGetEntryID(UI32_T index,UI32_T *dlist_id);

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
BOOL_T AMTRDRV_OM_CallBackQueueDequeue(UI32_T *index,UI32_T *dlist_id,UI8_T *addr_entry,UI8_T *action);

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
void AMTRDRV_OM_MarkAllDynamicRecord(void);

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
BOOL_T AMTRDRV_OM_GetMarkedEntry(UI8_T *addr_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_ClearMark
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set "mark field" to FALSE.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry  - Address entry
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is for AMTR Hardware Learning.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_ClearMark(UI8_T *addr_entry);

#endif
int  AMTRDRV_OM_NABufferDequeueBat(AMTR_TYPE_AddrEntry_T *addr_entry,int bat_counter,UI32_T *queue_counter);
#ifdef AMTRDRV_SLAVE_REPEATING_TEST
BOOL_T AMTRDRV_OM_InitNABufferRestore(void);
#endif

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
BOOL_T AMTRDRV_OM_ShowAddrInfo();

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)
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
BOOL_T AMTRDRV_OM_SlaveAgingOutEnqueue(UI32_T index);

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

BOOL_T AMTRDRV_OM_SlaveAgingOutResend(UI32_T *index,UI8_T *addr_entry);
#endif
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
void AMTRDRV_OM_ShowRecordsInfoInAgingOutBuffer();
/*added by Jinhua Wei ,to remove warning ,becaued the relative function's head file not declared */
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_NABufferRestoreDeleteRecord
 *------------------------------------------------------------------------------
 * PURPOSE: This function will restore deleted Record
 * INPUT  : addr_entry
 * OUTPUT : None
 * RETURN : TRUE/FALSE
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_OM_NABufferRestoreDeleteRecord(UI8_T *addr_entry);
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
void AMTRDRV_OM_ShowRecordInfoInCheckingAgingList();
#else
#define AMTRDRV_OM_ShowRecordsInfoInAgingOutBuffer()
#define AMTRDRV_OM_ShowRecordInfoInCheckingAgingList()
#endif/*End of #if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == FALSE)*/

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
BOOL_T AMTRDRV_OM_SyncQueueEnqueueAndSetEventID(UI32_T event_index, UI32_T cookie, AMTR_TYPE_Command_T action);

/* AMTRDRV BACKDOOR functions
 */

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
UI8_T  AMTRDRV_OM_BACKDOOR_GetSynqDebugInfo(void);

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
void   AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo(UI8_T synq_debug_info);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_SetSynqDebugInfo
 *------------------------------------------------------------------------------
 * PURPOSE: This function will dump the synq event id of all lports.
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
void   AMTRDRV_OM_BACKDOOR_DumpSynqPortEventId(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_OM_BACKDOOR_DumpSynQ
 *------------------------------------------------------------------------------
 * PURPOSE: This function will dump the content of syncq
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  : This function is used by AMTRDRV_OM backdoor only.
 *-----------------------------------------------------------------------------*/
void AMTRDRV_OM_BACKDOOR_DumpSynQ(void);
#endif  // end of AMTRDRV_OM_H

