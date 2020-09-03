/*-------------------------------------------------------------------------
 * MODULE NAME: AMTRDRV_MGR.h
 *-------------------------------------------------------------------------
 * PURPOSE: To manage AMTRDRV Hash Table.
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
 
#ifndef AMTRDRV_MGR_H
#define AMTRDRV_MGR_H

#include "amtr_type.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "xstp_type.h"
#include "isc.h"
#include "sysrsc_mgr.h"
#include "dev_amtrdrv.h"

/* NAME CONSTANT DECLARATIONS
 */
#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC==TRUE)
#define AMTRDRV_MGR_FIRST_COLLISION_MAC_TABLE_ENTRY_INDEX 0xFF
#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE==TRUE)
    #error "Log hash collision mac only supports on AMTR software learn!"
#endif
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* This data structure is only for sys callback function using */
typedef struct
{
    AMTR_TYPE_Command_T        type;
    UI16_T                vid;
    UI16_T                ifindex;
    UI8_T                 mac[6];
    UI8_T                 source;
    UI8_T                 life_time;
}AMTRDRV_MGR_SysCallBackMsg_T;

/* This callback function declaration is for new address or aging out notify AMTR_MGR to set addresses */
typedef void (*AMTRDRV_MGR_AddrCallbackFunction_T)
        (UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

/* This callback function declaration is for AMTRDRV nofity AMTR for announcing NA */
typedef UI32_T (*AMTRDRV_MGR_SecurityCheckFunction_T)
        (UI32_T     src_lport,
         UI16_T     vid,
         UI8_T      src_mac[6],
         UI8_T 	    dst_mac[6],
         UI16_T     ether_type     /* packet type  */);

/*--------------------
 * EXPORTED CONSTANTS
 *-------------------*/

/*--------------------
 * EXPORTED ROUTINES
 *-------------------*/

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_TASK_CreateTask
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to create Tasks for AMTRDRV
 * INPUT  :
 * OUTPUT :
 * RETURN : 
 * NOTES  : AMTRDRV this component has two tasks --
 *          1.AMTRDRV_ADDRESS_TASK: 
 *                   a. to learn new address
 *                   b. to check address need to be aged out or not
 *                   c. to delete the address which is aged
 *          2.AMTRDRV_ASIC_COMMAND_TASK:
 *                   a. to program chip (set or delete)
 *                   b. to process command order
 *-----------------------------------------------------------------------------*/
void  AMTRDRV_TASK_CreateTask(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_NewAddress_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(New address) 
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : num_of_entries     - how many records in the buffer
 *          addr_buf[]         - a buffer to store addresses
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_NewAddress_CallBack(AMTRDRV_MGR_AddrCallbackFunction_T callbackfunction);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_SecurityCheck_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(security checking for NA) 
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : callback function
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_SecurityCheck_CallBack(AMTRDRV_MGR_SecurityCheckFunction_T  callbackfunction);

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Register_AgingOut_CallBack
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to provide a register service. If the event(Aging out) 
 *          is happened we can notify upper layer's CSCs who need to know this event.
 * INPUT  : num_of_entries     - how many records in the buffer
 *          addr_buf[]         - a buffer to store addresses
 * OUTPUT : None
 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Register_AgingOut_CallBack(AMTRDRV_MGR_AddrCallbackFunction_T callbackfunction);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Init
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to reserve the resources which will be used
 *          in whole system
 * INPUT  : None                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                        
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
//void AMTRDRV_MGR_Init(void);
void AMTRDRV_MGR_InitiateSystemResources(void);

void AMTRDRV_MGR_AttachSystemResources(void);

void AMTRDRV_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT  : None                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                        
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Create_InterCSC_Relation();

 /*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterMasterMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set up the resources which are used in master mode
 * INPUT  : None                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                        
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetTransitionMode()
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set the operation mode to transition mode
 * INPUT  : None
 * OUTPUT : None                                                           
 * RETURN : None
 * NOTES  : None 
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterTransitionMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to initialize the resources which are used in whole system
 *          no mater the unit is master or slave mode.
 * INPUT  : None                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                        
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_EnterSlaveMode
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set up resources which are used in slave mode         
 * INPUT  : None                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                         
 * NOTES  : None   
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_EnterSlaveMode(void);

#if (SYS_CPNT_AMTR_HW_LEARN_ON_STANDALONE == TRUE)
/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_ProvisionComplete
 *------------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_ProvisionComplete(void);
#define AMTRDRV_MGR_CallByAgent_ISC_Handler   NULL    /* use in isc_agent.c */
#else/*SW Learning*/
#define AMTRDRV_MGR_ProvisionComplete()
#if (SYS_CPNT_STACKING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_CallByAgent_ISC_Handler
 *------------------------------------------------------------------------------
 * PURPOSE: This function will handle isc request from ISC_Agent.
 * INPUT  : ISC_Key_T *key              - key of isc
 *          L_MM_Mref_Handle_T *mem_ref 
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTE   : callbacked by isc_agent                                                       
 *-----------------------------------------------------------------------------*/  
BOOL_T AMTRDRV_MGR_CallByAgent_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mem_ref);
#endif /* End of #if (SYS_CPNT_STACKING == TRUE)*/
#endif


#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_HandleHotInsertion
 *------------------------------------------------------------------------------
 * FURPOSE: This function is to set all addresses to new insertion unit          
 * INPUT  : hot_insertion unit   - the new insertion unit id                                                         
 * OUTPUT : None                                                         
 * RETURN : None                                                         
 * NOTES  : None   
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_HandleHotInsertion(UI32_T hot_insertion_unit);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAgingTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set aging time for the whole system
 * INPUT  : UI32_T value  - Aging time to set
 * OUTPUT : None
 * RETURN : TRUE /FALSE
 * NOTES  : 1. aging time is in [10..1000000] seconds
 *          2. If you want to disable aging out function please assign value =0
 *          3. We only keep the value in local static variable since we set AGE=0
 *             to chip to let the software control aging out function.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAgingTime(UI32_T value);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry into Hash table & chip and 
 *          aslo program remote units
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry  
 *          UI32_T number_of_entries             - total number of entries in this buffer 
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *          2. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrList(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList2LocalUnit
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry into Hash table & chip in local unit
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTES  : 1. If one of the entry in the buffer can't setup successfully we will
 *          return AMTR_TYPE_RET_ERROR_UNKNOWN. This is because if the num_of_entries = 1 the behavior
 *          can match that if setup failed then failed otherwise it will return AMTR_TYPE_RET_SUCCESS.
 *          2. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *-----------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T AMTRDRV_MGR_SetAddrList2LocalUnit(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrList2RemoteUnit
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry to remote units
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry  
 *          UI32_T number_of_entries             - total number of entries in this buffer 
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrList2RemoteUnit(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetAddrDirectly
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set address entry quickly without running hash FSM
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry          - Address entry                                    
 *          BOOL_T                is_local_hash_only   - only update hash or need to update chip
 * OUTPUT : None                                                                             
 * RETURN : TRUE /FALSE                
 * NOTES  : 1.If we need to update chip we need to send by ISC to notify other units to update
 *            there table and chip as well. If only need to update hash it means this may the request
 *            from MIB to store the record with life_time = other in the master database only 
 *          2.We support this function so the caller need to update his database by itself
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddrDirectly(AMTR_TYPE_AddrEntry_T *addr_entry,BOOL_T is_local_hash_only);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrEntryList
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry from Hash table & chip in whole system
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 *          UI32_T num_of_entries               - how many entries need to be set
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will not execute in the module unit and it will return TRUE directly
 *             Due to isc callback function can't be null in module part we can't block
 *             the function by SYS_CPNT_MAINBOARD. We only can do it inside the function.
 *             In the module part it will return true directly. Actually this case
 *             won't happen in module unit.
 *          2. We support this function so the caller need to update his database as well since
 *             we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table, Job Queue, Sync Queue
 *             (learnt entry and security entry that will age out) and callback Queue.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrEntryList(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrEntryListFromOm
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry from Hash table in whole system
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1. This function will not execute in the module unit and it will return TRUE directly
 *             Due to isc callback function can't be null in module part we can't block
 *             the function by SYS_CPNT_MAINBOARD. We only can do it inside the function.
 *             In the module part it will return true directly. Actually this case
 *             won't happen in module unit.
 *          2. We support this function so the caller need to update his database as well since
 *             we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table, Job Queue, Sync Queue
 *             (learnt entry and security entry that will age out) and callback Queue.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrEntryListFromOm(AMTR_TYPE_AddrEntry_T* addr_buf);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrFromChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry quickly without running hash FSM
 * INPUT  :        addr_entry         - address info
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API is only support deleting (life_time = Other) or (ifindex =0 but not cpu mac)
 *            to hash table since this kind of records only store in hash table only not chip.
 *          2.We support this function so the caller need to update his database as well since
 *            we won't have any information for this entry after finishing.
 *          3. This function will update Hash Table and ASIC ARL Table.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrFromChip(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrDirectly
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete address entry quickly without running hash FSM
 * INPUT  :        addr_entry         - address info
 *          BOOL_T is_local_hash_only - only need to update hash table or need to update chip as well
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This API shall only be called for deleting (life_time = Other) or (ifindex =0 but not cpu mac)
 *            to hash table since this kind of records only store in hash table only and not in chip.
 *            (When specify is_local_has_only as TRUE)
 *          2.We support this function so the caller need to update his database as well since
 *            we won't have any information for this entry after finishing.
 *          3.This function will update Hash Table and ASIC ARL Table.
 *            (When specify is_local_has_only as FALSE)
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrDirectly(AMTR_TYPE_AddrEntry_T *addr_entry, BOOL_T is_local_hash_only);

#if (SYS_CPNT_MAINBOARD == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DelNumOfDynamicAddrs
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to handle to delete number of dynamic addresses when system ARL 
 *          capacity is become smaller.
 * INPUT  : UI32_T num_of_entries   - how many entries need to be set                  
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : This API is called by AMTR when ARL capacity is become smaller and AMTR
 *          need to tell how many entries need to be deleted.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DelNumOfDynamicAddrs(UI32_T num_of_entries);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAllAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete all addresses from Hash table & chip
 * INPUT  : None
 * OUTPUT : None                                                           
 * RETURN : TRUE /FALSE              
 * NOTES  : None        
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAllAddr(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which life_time is matced
 * INPUT  : AMTR_TYPE_AddressLifeTime_T life_time      -- specified life_time   
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.For example if we need to delete all dynamic addresses we need to 
 *            set life_time = delete_on_time_out
 *          2.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly. 
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByLifeTime(AMTR_TYPE_AddressLifeTime_T life_time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrBySource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which source is matched
 * INPUT  : AMTR_TYPE_AddressSource_T source      --source
 * OUTPUT : None                                                           
 * RETURN : TRUE /FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.       
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrBySource(AMTR_TYPE_AddressSource_T source);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex is matched
 * INPUT  : UI32_T ifindex      - which port/trunk 
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPort(UI32_T ifindex);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex & life_time is matched
 * INPUT  : UI32_T                   ifindex     - which port/trunk
 *          AMTR_TYPE_AddressLifeTime_T   life_time   - which life_time
 * OUTPUT : None                                                          
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndLifeTime(UI32_T ifindex,AMTR_TYPE_AddressLifeTime_T life_time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which ifindex & source is matched
 * INPUT  : UI32_T                   ifindex     - which port/trunk
 *        : AMTR_TYPE_AddressSource_T     source      - which source
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.         
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndSource(UI32_T ifindex,AMTR_TYPE_AddressSource_T source);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVID
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid is matched
 * INPUT  : UI32_T                   vid         - VLan number
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVID(UI32_T vid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVidAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & life_time is matched
 * INPUT  : UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressLifeTime_T   life_time   - which life_time
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVidAndLifeTime(UI32_T vid,AMTR_TYPE_AddressLifeTime_T life_time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & source is matched
 * INPUT  : UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressSource_T     source      - which source
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVidAndSource(UI32_T vid,AMTR_TYPE_AddressSource_T source);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByVIDnPort
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & ifindex is matched
 * INPUT  : UI32_T                   vid         - VLan number
 *          UI32_T                   ifindex     - which port / trunk
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.          
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByVIDnPort(UI32_T ifindex,UI32_T vid);

/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime
 *------------------------------------------------------------------------------
 * Purpose  : Delete all of the addresses with the specified life time of a
 *            specific port and vid from Hash table & chip
 * INPUT    : UI32_T ifindex              - which port/trunk
 *            UI32_T vid                  - vlan id
 *            AMTR_TYPE_AddressLifeTime_T - other, invalid, permanent, del on reset, del on timeout
 *            BOOL_T sync_op              - TRUE : The function will not
 *                                                 return until the
 *                                                 delete operation is
 *                                                 done.
 *                                          FALSE: The function is
 *                                                 returned when the
 *                                                 delete command had
 *                                                 been passed to the
 *                                                 AMTRDRV task.
 *
 * OUTPUT   : None
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *            2. For sync_op==TRUE, only support life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_MGR_DeleteAddrByPortAndVidAndLifeTime(UI32_T ifindex, UI32_T vid, AMTR_TYPE_AddressLifeTime_T life_time, BOOL_T sync_op);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vlan bitmap , port & life_time is matched
 * INPUT  : UI32_T                        infindex    - Which port / Trunk
            UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressLifeTime_T   life_time   - which life_time
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE               
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.        
 *-----------------------------------------------------------------------------*/
BOOL_T  AMTRDRV_MGR_DeleteAddrByPortAndVlanBitMapAndLifeTime(UI32_T ifindex, UI16_T *vlan_p, UI32_T vlan_count,AMTR_TYPE_AddressLifeTime_T life_time,XSTP_MGR_MstpInstanceEntry_T * mstp_entry_p);



/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddr_ByPortAndVidAndSource
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid , port & source is matched
 * INPUT  : UI32_T                        ifindex     - which port / trunk
            UI32_T                        vid         - VLan number
 *          AMTR_TYPE_AddressSource_T     source   - which life_time
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.         
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndVidAndSource(UI32_T ifindex, UI32_T vid, AMTR_TYPE_AddressSource_T source);


/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrByPortAndVidExceptCertainAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to delete addresses from Hash table and chip
 *          which vid & ifindex is matched
 * INPUT  : UI32_T                   vid         - VLan number
 *          UI32_T                   ifindex     - which port / trunk
 *          UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *          UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN]
 *          UI32_T number_of_entry_in_list
 * OUTPUT : None
 * RETURN : TRUE / FALSE
 * NOTES  : 1.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrByPortAndVidExceptCertainAddr(UI32_T ifindex,
                                                           UI32_T vid,
                                                           UI8_T mac_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                           UI8_T mask_list_p[][SYS_ADPT_MAC_ADDR_LEN],
                                                           UI32_T number_of_entry_in_list);


/*------------------------------------------------------------------------------
 * Function : AMTRDRV_MGR_GetExactAddrFromChipWithoutISC
 *------------------------------------------------------------------------------
 * Purpose  : This function will search port related to input mac&vid in ARL
 * INPUT    : addr_entry->mac - mac address
 *            addr_entry->vid - vlan id
 * OUTPUT   : addr_entry      - addresss entry info
 * RETURN   : BOOL_T Status - True : successs, False : failed
 * NOTE     : Called by amtr_mgr
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_GetExactAddrFromChipWithoutISC(AMTR_TYPE_AddrEntry_T *addr_entry);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ChangePortLifeTime
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to update addresses life_time attribute by giving ifindex 
 *          and life_time               
 * INPUT  : ifindex    - port number
 *          life_time  - life_time
 * OUTPUT : None
 * RETURN : TRUE / FALSE                
 * NOTES  : 1.This function is call by AMTR_MGR when the port_info is changed 
 *          2.For this procedure we only need to update OM or local_aging list
 *            we don't need to care about the chip because the attribtue in chip 
 *            side is always static and nothing changed.
 *          3.This function won't be execute in the module unit since module unit
 *            doesn't have Database to support. It will return TRUE directly.  
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_ChangePortLifeTime(UI32_T ifindex,UI32_T life_time);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_CreateMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to create a multicast address table for each device
 * INPUT  : UI32_T   vid     - vlan ID                             
 *          UI8_T    *mac    - multicast mac address         
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : None                                                            
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_CreateMulticastAddr(UI32_T vid, UI8_T *mac);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DestroyMulticastAddr
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to destroy a multicast address table for each device
 * INPUT  : UI32_T   vid     - vlan ID                                       
 *          UI8_T    *mac    - multicast mac address               
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : None                                                                    
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DestroyMulticastAddr(UI32_T vid, UI8_T* mac);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetMulticastPortMember
 *------------------------------------------------------------------------------
 * PURPOSE: This function will set port members for a specified multicast group
 * INPUT  : UI32_T    unit    - Unit number
 *          UI32_T    vid     - VLAN ID
 *          UI8_T     *mac    - multicast MAC address
 *          UI8_T     *pbmp   - port member list
 *          UI8_T     *tbmp   - tagged port member list
 * OUTPUT: None
 * RETURN: TRUE / FALSE
 * NOTES : 1. per device dependent
 *         2. Caller doesn't need to pass "unit", this API will set all valid remote units via ISC.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetMulticastPortMember(UI32_T vid, UI8_T *mac, UI8_T *pbmp, UI8_T *tbmp);

#if (SYS_CPNT_MAINBOARD == TRUE) 
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SyncToHisam
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get a record or event to sync to Hisam table                
 * INPUT  : None
 * OUTPUT : *addr_entry           - user_record or event
 *          *action               - sync case : set_entry / delete_entry / delete_group ....... 
 * RETURN : TRUE(still has a record or event to sync) / FALSE (no record or evnet to sync)                
 * NOTES  : This function is called by AMTR_task periodically to get record or event to sync.
 *          If the sync case is "set_entry" or "delete_entry" *addr_entry means a record or
 *          it contain evnet info
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SyncToHisam(AMTR_TYPE_AddrEntry_T *addr_entry,AMTR_TYPE_Command_T *action);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetCallBackMsg
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get a callback message for syscallbac task to notify other cscs.                
 * INPUT  : None
 * OUTPUT : msg  -- include message type and entry information
 * RETURN : TRUE(still has a record or event to notify other cscs) / FALSE(no event)               
 * NOTES  : This function is call by SYS_Callback task. 
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_GetCallBackMsg(SYS_TYPE_MSG_T *event);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteChipOnly
 *------------------------------------------------------------------------------
 * PURPOSE: This function only delete entry from ASIC ARL Table.
 * INPUT  : UI16_T vid                       - vlan id        
 *          UI8_T mac[SYS_ADPT_MAC_ADDR_LEN] - mac
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1. This function only update ASIC ARL Table  
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteChipOnly(UI16_T vid, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN]);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrFromOMWithoutChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function only set entry to OM.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry  
 *          UI32_T num_of_entries               - how many entries need to be set                  
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1. This function will update Hash Table, and Callback Queue.
 *          2. This function should be used in HW Learning.
 *          3. This function doesn't notify remote unit.
 *          4. This function won't put entry in Job Queue, therefore AMTR won't 
 *             program it to ASIC ARL Table.
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_SetAddr2OMWithoutChip(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_DeleteAddrFromOMWithoutChip
 *------------------------------------------------------------------------------
 * PURPOSE: This function only delete entry from OM.
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry  
 *          UI32_T num_of_entries               - how many entries need to be set                  
 * OUTPUT : None                                                           
 * RETURN : TRUE / FALSE                
 * NOTES  : 1. This function will update Hash Table, Sync Queue(only learnt entry) and Callback Queue.
 *          2. This function should be used in HW Learning.
 *          3. This function doesn't notify remote unit.
 *          4. This function won't put entry in Job Queue, therefore AMTR won't 
 *             program it to ASIC ARL Table. 
 *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteAddrFromOMWithoutChip(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessAgingOutEntries
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to process Aging out buffer
 *          if master mode => notify upper layer to delete agingout entries
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *-----------------------------------------------------------------------------*/
UI32_T AMTRDRV_MGR_ProcessAgingOutEntries(AMTR_TYPE_AddrEntry_T *addr_buf,UI32_T buf_size);
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ProcessMasterNABuffer
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to get NA information from NA buffer
 *          if master mode => dequeue to deal with NA
 * INPUT  : None
 * OUTPUT : None
 * RETURN : None
 * NOTES  :
 *-----------------------------------------------------------------------------*/
UI32_T AMTRDRV_MGR_ProcessMasterNABuffer(AMTR_TYPE_AddrEntry_T *addr_buf, UI32_T buf_size);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_SetInterventionEntry
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to set InterventionEntry address entry into Hash table
 * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry    - Address entry
 *          UI32_T number_of_entries             - total number of entries in this buffer
 * OUTPUT : None
 * RETURN   : AMTR_TYPE_Ret_T     Success, or cause of fail.
 * NOTES  : 1. This function will set entries to OM, Job Queue, Callback Queue(learnt entry).
 *          2. Notify entries to remote unit.
 *-----------------------------------------------------------------------------*/
AMTR_TYPE_Ret_T AMTRDRV_MGR_SetInterventionEntry(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);


 /*------------------------------------------------------------------------------
  * FUNCTION NAME: AMTRDRV_MGR_DeleteInterventionEntry
  *------------------------------------------------------------------------------
  * PURPOSE: This function is to delete address entry from Hash table 
  * INPUT  : AMTR_TYPE_AddrEntry_T *addr_entry   - Address entry
  *          UI32_T num_of_entries               - how many entries need to be set
  * OUTPUT : None
  * RETURN : TRUE / FALSE
  *-----------------------------------------------------------------------------*/
BOOL_T AMTRDRV_MGR_DeleteInterventionEntry(UI32_T num_of_entries,AMTR_TYPE_AddrEntry_T addr_buf[]);

BOOL_T AMTRDRV_MGR_SetInterfaceConfig(UI32_T unit,UI32_T port,UI32_T mode);


#if(AMTR_TYPE_MEMCPY_MEMCOM_PERFORMANCE_TEST == 1)
void AMTRDRV_MGR_GetMemActionCounter();
#endif





UI32_T AMTRDRV_MGR_GetAmtrID();

void AMTRDRV_MGR_SetAmtrID(UI32_T tid);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_Event2ID
 *------------------------------------------------------------------------------
 * PURPOSE: This function is to convert Event type to ID
 * INPUT  : UI32_T               cookie         -- maybe be the information for life_time or source
 *          AMTR_TYPE_Command_T *action         --
 * OUTPUT : UI32_T              *event_id       --

 * RETURN : None
 * NOTES  : None
 *-----------------------------------------------------------------------------*/
void AMTRDRV_MGR_Event2ID(UI32_T cookie,AMTR_TYPE_Command_T action,UI32_T *event_id);

#if (SYS_CPNT_AMTR_LOG_HASH_COLLISION_MAC == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ClearCollisionVlanMacTable
 *------------------------------------------------------------------------------
 * PURPOSE  : Remove all entries in the collision mac table.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_ClearCollisionVlanMacTable(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetNextEntryOfCollisionVlanMacTable
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
BOOL_T AMTRDRV_MGR_GetNextEntryOfCollisionVlanMacTable(UI8_T* idx_p, UI16_T* vlan_id_p, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN], UI16_T *count_p);
#endif

#if (SYS_CPNT_HASH_LOOKUP_DEPTH_CONFIGURABLE == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_GetHashLookupDepthFromChip
 *------------------------------------------------------------------------------
 * PURPOSE  : Get hash lookup depth from chip
 * INPUT    : None
 * OUTPUT   : lookup_depth_p -- hash lookup depth
 * RETURN   : TRUE  - success
 *            FALSE - fail
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_GetHashLookupDepthFromChip(UI32_T *lookup_depth_p);
#endif

/*------------------------------------------------------------------------------
 * FUNCTION NAME: AMTRDRV_MGR_ReadAndClearLocalHitBit
 *------------------------------------------------------------------------------
 * PURPOSE  : Get and clear hit bit of specified {vid, mac}
 * INPUT    : mac_p - MAC address to check hit bit
 *            vid - vid to check hit bit
 *            ifindex - ifindex to check hit bit
 * OUTPUT   : hitbit_value_p -- hit or not
 * RETURN   : TRUE  - success
 *            FALSE - fail
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_MGR_ReadAndClearLocalHitBit(
    UI8_T *hitbit_value_p,
    UI8_T *mac_p,
    UI32_T vid,
    UI32_T ifindex);
#endif  // end of AMTRDRV_MGR_H
