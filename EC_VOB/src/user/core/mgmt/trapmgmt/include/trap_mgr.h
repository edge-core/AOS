/* FILE NAME  -  TRAP_MGR.H                                                     
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
 *   Amytu      08/02/2002      Add new trap feature for FTTH
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2001      
 * ------------------------------------------------------------------------
 */

#ifndef TRAP_MGR_H
#define TRAP_MGR_H

/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h" 
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "trap_event.h"
#include "leaf_es3626a.h"
#if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)  
#include "leaf_2021.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define TRAP_MGR_ENTRY_VALID                VAL_trapDestStatus_2_valid
#define TRAP_MGR_ENTRY_INVALID              VAL_trapDestStatus_2_invalid
#define TRAP_MGR_INDIVIDUAL_EVENT_ENABLE    VAL_trapEnableAll_enabled   
#define TRAP_MGR_INDIVIDUAL_EVENT_DISABLE   VAL_trapEnableAll_disabled

/* TYPE DECLARATIONS 
 */
 
 
/* Define SNMP Trap Destination data type 
 *
 * NOTES: 1. The trap receiver can only be accessed by CLI and Web.
 *           SNMP management station CAN NOT access the trap receiver.
 *        2. There is no MIB to define the trap receiver.
 *        3. Status of each trap receiver is defined as following:
 *              - invalid(0): the entry of this trap receiver is deleted/purged
 *              - enabled(1): this trap receiver is enabled
 *              - disabled(2): this trap receiver is disabled

 *           Set status to invalid(0) will delete/purge a trap receiver.
 *
 *           Commuinty string name is an "ASCII Zero" string (char array ending with '\0'). 
 *
 *        4. The total number of trap receivers supported by the system 
 *           is defined by SYS_ADPT_MAX_NBR_OF_TRAP_RECEIVER.
 *        5. By default, there is no trap receiver configued in the system.
 *        6. For any trap event raised by any subsystem, a SNMP Trap shall be
 *           sent to all of the enabled trap receivers.
 */ 
#if (SYS_CPNT_SNMP_VERSION == 2) 
#if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)    
typedef enum
{
    TRAP_MGR_TRAP_DEST_PROTOCOL_IP = VAL_trapDestProtocol_ip,
    TRAP_MGR_TRAP_DEST_PROTOCOL_IPX = VAL_trapDestProtocol_ipx    
} TRAP_MGR_TRAP_DEST_PROTOCOL_E;
#endif
typedef struct
{
    UI32_T      trap_dest_address;
    UI8_T       trap_dest_community[SYS_ADPT_MAX_COMM_STR_NAME_LEN + 1];
    UI32_T      trap_dest_status;
    UI32_T      trap_dest_version; 
#if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)         
    UI8_T       trap_dest_owner[MAXSIZE_trapDestOwner+1]; 
    TRAP_MGR_TRAP_DEST_PROTOCOL_E  trap_dest_protocol;  
#endif
    UI32_T      trap_dest_port;   
} TRAP_MGR_TrapDestEntry_T;
#endif//end of #if (SYS_CPNT_SNMP_VERSION == 2)

typedef enum
{
    TRAP_MGR_LINK_UP_DOWN_TRAP_ENABLED = 1,
    TRAP_MGR_LINK_UP_DOWN_TRAP_DISABLED = 2    
} TRAP_MGR_LINKUPDOWNTRAPSTATUS_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
void TRAP_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRAP_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRAP_MGR_Create_InterCSC_Relation(void);

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
void TRAP_MGR_EnterMasterMode();


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
void TRAP_MGR_EnterSlaveMode();


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
void TRAP_MGR_EnterTransitionMode();

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
void TRAP_MGR_SetTransitionMode();

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
BOOL_T TRAP_MGR_Make_Oid_From_Dot_String(UI32_T *oid_P, I8_T *text_p);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_CreateTask                                 
 * ---------------------------------------------------------------------
 *  FUNCTION: Create and start trap manager task                        
 *  INPUT    : None.                                                    
 *  OUTPUT   : None.                                                    
 *  RETURN   : None.                                                    
 *  NOTE     : None.                                                    
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_CreateTask(void);


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
void TRAP_MGR_ProvisionComplete(void);


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
void TRAP_MGR_ReqSendTrap(TRAP_EVENT_TrapData_T *trap_data_p);

/* ---------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_ReqSendTrapOptional                                    
 * ---------------------------------------------------------------------
 *  FUNCTION: Request trap manager for send trap.                       
 *                                                                      
 *  INPUT    : 1. trap_data_p: Variable's instance and value that should be bound in
 *                                                    trap PDU.                                             
 *             2. flag:TRAP_MGR_SEND_TRAP_OPTION_LOG_AND_TRAP: send trap and log; 
                       TRAP_MGR_SEND_TRAP_OPTION_LOG_ONLY: log only, don't send trap
 *  OUTPUT   : None.                                                    
 *  RETURN   : None.                                                    
 *  NOTE     : This procedure shall not be invoked before TRAP_MGR_Init() is called.
 * ---------------------------------------------------------------------
 */
void TRAP_MGR_ReqSendTrapOptional(TRAP_EVENT_TrapData_T *trap_data_p, TRAP_EVENT_SendTrapOption_E flag);
  
/* ----------------------------------------------------------------------------
 *  ROUTINE NAME  - TRAP_MGR_CurrentOperationMode                                   
 * -----------------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of Trap 's task
 * INPUT:    None.
 * OUTPUT:   SYS_TYPE_STACKING_MASTER_MODE \
 *           SYS_TYPE_STACKING_TRANSITION_MODE \
 *           SYS_TYPE_STACKING_SLAVE_MODE
 * RETURN:   none
 * NOTE:     None.
 * -----------------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T TRAP_MGR_CurrentOperationMode();


/* Trap status
 */

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
BOOL_T TRAP_MGR_GetSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps);

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
BOOL_T TRAP_MGR_SetSnmpEnableAuthenTraps(UI8_T snmp_enable_authen_traps);


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
BOOL_T TRAP_MGR_GetLinkUpDownTraps(UI8_T *link_up_down_traps); 


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
BOOL_T TRAP_MGR_SetLinkUpDownTraps(UI8_T link_up_down_traps); 


/* Notify Function
 */
 
 
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
void TRAP_MGR_NotifyStaTplgChanged (void);


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
void TRAP_MGR_NotifyStaTplgStabled (void);
 
#if (SYS_CPNT_SNMP_VERSION == 2) 
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetTrapReceiver                             
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the specified trap receiver 
 *          can be retrieved successfully. Otherwise, false is returned.
 *                                                                      
 * INPUT: trap_receiver->ip_addr    - (key) to specify a unique trap receiver                               
 * OUTPUT: trap_receiver            - trap receiver info
 * RETURN: TRUE/FALSE       
 * NOTES: 1. The trap receiver can only be accessed by CLI and Web.
 *           SNMP management station CAN NOT access the trap receiver.
 *        2. There is no MIB to define the trap receiver.
 *        3. Status of each trap receiver is defined as following:
 *              - invalid(0): the entry of this trap receiver is deleted/purged
 *              - enabled(1): this trap receiver is enabled
 *              - disabled(2): this trap receiver is disabled

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
BOOL_T TRAP_MGR_GetTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - TRAP_MGR_GetNextTrapReceiver                             
 * ---------------------------------------------------------------------
 * PURPOSE: This function returns true if the next available trap receiver 
 *          can be retrieved successfully. Otherwise, false is returned.
 *                                                                      
 * INPUT: trap_receiver->ip_addr    - (key) to specify a unique trap receiver
 * OUTPUT: trap_receiver            - next available trap receiver info      
 * RETURN: TRUE/FALSE       
 * NOTES: None.
 * ---------------------------------------------------------------------
 */         
BOOL_T TRAP_MGR_GetNextTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver);


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
                                              UI8_T    *comm_string_name);


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
BOOL_T TRAP_MGR_SetTrapReceiverStatus(UI32_T     trap_receiver_ip_addr, 
                                      UI32_T     status);
                                      
                                      
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
                                       UI32_T    version);

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
                                       UI32_T    port);

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
BOOL_T TRAP_MGR_GetTrapReceiverByIndex(UI32_T index, TRAP_MGR_TrapDestEntry_T *trap_receiver);

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
BOOL_T TRAP_MGR_GetNextTrapReceiverByIndex(UI32_T *index, TRAP_MGR_TrapDestEntry_T *trap_receiver);

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
BOOL_T TRAP_MGR_SetTrapReceiverCommStringNameByIndex(UI32_T index, UI8_T *comm_string_name);

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
BOOL_T TRAP_MGR_SetTrapDestProtocolByIndex(UI32_T   index, TRAP_MGR_TRAP_DEST_PROTOCOL_E  protocol);

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
BOOL_T TRAP_MGR_SetTrapDestAddressByIndex(UI32_T index, UI32_T  addr);

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
BOOL_T TRAP_MGR_SetTrapReceiverStatusByIndex(UI32_T index, UI32_T status);

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
BOOL_T TRAP_MGR_SetTrapReceiverVersionByIndex(UI32_T index, UI32_T version);

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
BOOL_T TRAP_MGR_SetTrapDestOwnerByIndex(UI32_T index, UI8_T *owner);
#endif//end of #if (SYS_CPNT_3COM_RMON2_PROBE_CONFIG_MIB == TRUE)  


/* Running Config API
 */

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
SYS_TYPE_Get_Running_Cfg_T TRAP_MGR_GetNextRunningTrapReceiver(TRAP_MGR_TrapDestEntry_T *trap_receiver);
#endif//end of #if (SYS_CPNT_SNMP_VERSION == 2)

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
UI32_T TRAP_MGR_GetRunningSnmpEnableAuthenTraps(UI8_T *snmp_enable_authen_traps);


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
UI32_T TRAP_MGR_GetRunningLinkUpDownTraps(UI8_T *link_up_down_trap);



#endif /* TRAP_MGR_H */
