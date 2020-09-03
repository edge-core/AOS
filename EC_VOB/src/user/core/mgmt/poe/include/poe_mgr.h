/*-----------------------------------------------------------------------------
 * FILE NAME: poe_mgr.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file declares the APIs for upper-layer to access the PoE controller.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    04/7/2003 - Kelly Hung, Created	
 *    12/03/2008 - Eugene Yu, porting POE to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef POE_MGR_H
#define POE_MGR_H

#include "sys_type.h"
#include "sys_dflt.h"
#include "leaf_3621.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "poe_type.h"
#include "sysfun.h"
#include "sys_adpt.h"

/* struct */

/* command used in IPC message
 */
enum
{
    POE_MGR_IPC_SETMAINPOWERMAXIMUMALLOCATION,
    POE_MGR_IPC_SETPSEPORTADMIN,
    POE_MGR_IPC_SETPORTPOWERMAXIMUMALLOCATION,
    POE_MGR_IPC_SETPSEPORTPOWERPRIORITY,
    POE_MGR_IPC_SETLEGACYDECTECTION,
    POE_MGR_IPC_SETPORTMANUALHIGHPOWERMODE,
    POE_MGR_IPC_SETPSEPORTPOWERPAIRS,
    POE_MGR_IPC_SETPSEPORTPOWERTYPE,
    POE_MGR_IPC_SETMAINPSEUSAGETHRESHOLD,
    POE_MGR_IPC_SETNOTIFICATIONCTRL,
    POE_MGR_IPC_BINDTIMERANGETOPSEPORT,
    POE_MGR_IPC_UNBINDTIMERANGETOPSEPORT,
};


/* NAMING CONSTANT DECLARATIONS
 */
#define POE_MGR_IPCMSG_TYPE_SIZE sizeof(union POE_MGR_IpcMsg_Type_U)
/* Macro function for computation of IPC msg_buf size based on field name
 * used in POE_MGR_IpcMsg_T.data
 */
#define POE_MGR_GET_MSG_SIZE(field_name)                       \
            (POE_MGR_IPCMSG_TYPE_SIZE +                        \
            sizeof(((POE_MGR_IpcMsg_T*)0)->data.field_name))
/* IPC message structure
 */
typedef struct
{
    union POE_MGR_IpcMsg_Type_U
    {
        UI32_T cmd;
        UI32_T ret_ui32;
        BOOL_T ret_bool;
    } type; /* the intended action or return value */

    union
    {
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
        } arg_grp_ui32_ui32;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
        } arg_grp_ui32_ui32_ui32;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI32_T arg_ui32_3;
            UI8_T arg_ui8[MAXSIZE_pethPsePortType+1];
        } arg_grp_ui32_ui32_ui32_ui8;

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI8_T  arg_ui8[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH+1];
        } arg_grp_ui32_ui32_ui8;
#endif

    } data; /* the argument(s) for the function corresponding to cmd */
} POE_MGR_IpcMsg_T;
/*End of Eugene added*/


/* function */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_InitiateSystemResources                               
 * -------------------------------------------------------------------------
 * FUNCTION: init POE system
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_MGR_InitiateSystemResources(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterMasterMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter master state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterMasterMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE POE_MGR_HandleHotInsertion                          
 * -------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * -------------------------------------------------------------------------*/
void POE_MGR_HandleHotInsertion(UI32_T starting_port_ifindex,
        UI32_T number_of_port,
        BOOL_T use_default);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterSlaveMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter slave state
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_EnterTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: enter transition state     
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_MGR_EnterTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetTransitionMode                               
 * -------------------------------------------------------------------------
 * FUNCTION: set transition state flag         
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_MGR_SetTransitionMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortAdmin                               
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.           
 * INPUT   : group_index                                                     
             port_index                                                      
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.               
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerPairs                          
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.                        
 * INPUT   : group_index                                                     
             port_index                                                      
             value = signal(1)means that the signal pairs only are in use.  
                   = spare(2) means that the spare pairs only are in use.   
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : If the value of pethPsePortPowerPairsControl is true,          
             this object is writable.                                       
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerDetectionCtrl             
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.            
 * INPUT   : group_index                                                     
             port_index                                                      
             value = auto(1)enables the power detection mechanism of the port.
                   = test(2)puts the port in a testmode:                    
                     force continuous discovery without applying            
                     power regardless of whether PD detected.               
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerDetectionCtrl(UI32_T group_index, UI32_T port_index, UI32_T value);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point   
             of view of a power management algorithm.                       
 * INPUT   : group_index                                                     
             port_index                                                      
             value = critical(1)                                            
                   = high(2)                                                
                   = low(3)                                                 
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : The priority that                                              
             is set by this variable could be used by a control mechanism   
             that prevents over current situations by disconnecting first   
             ports with lower power priority. Ports that connect devices    
             critical to the operation of the network - like the E911       
             telephones ports - should be set to higher priority.                                                                 
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value       
             that indicates the type of the device that is connected        
             to theport. This value can be the result of the mapping        
             the address of the station connected to the port and of        
             the value of the pethPdPortType of the respective PD port.     
 * INPUT   : group_index                                                     
             port_index                                                      
             value 
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value       
             that indicates the type of the device that is connected        
             to theport. This value can be the result of the mapping        
             the address of the station connected to the port and of        
             the value of the pethPdPortType of the respective PD port.     
 * INPUT   : group_index                                                     
             port_index                                                      
             value 
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: set the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.                         
 * INPUT   : group_index                                                     
             value = (1..99)
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetMainPseUsageThreshold(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to set the power, available for Power
 *           Management on PoE.                
 * INPUT   : group_index : unit
 *           value       : watt
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetMainpowerMaximumAllocation(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetNotificationCtrl                        
 * -------------------------------------------------------------------------
 * FUNCTION: Enable Notification from Agent.                                
 * INPUT   : group_index                                                     
             value = enable(1) 
                   = disable(2)
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetNotificationCtrl(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Register_PortOverloadStatusChange_CallBack                               
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used for LED MGR to register its callback 
 *           function for overload status.
 * INPUT:    fun -- the pointer of callback function.
 * OUTPUT  : None                                                           
 * RETURN  : None
 * NOTE    : void *fun(UI32_T unit, UI32_T port, BOOL_T is_overload)
 *           unit -- unit ID
 *           port -- port ID
 *           is_overload -- TRUE: overload, FALSE: normal condition
 * -------------------------------------------------------------------------*/
void POE_MGR_Register_PortOverloadStatusChange_CallBack(void (*fun)(UI32_T unit,
                                                                    UI32_T port,
                                                                    BOOL_T is_overload));


/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SoftwareDownload
 * -------------------------------------------------------------------------
 * FUNCTION: Software download to PoE controller                                        
 * INPUT   : unit -- unit ID
 *         : filename -- filename to be downloaded to PoE controller                                                     
 * OUTPUT  : None                                                     
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SoftwareDownload(UI32_T unit, UI8_T *filename);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetLegacyDetection                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection	       
 * INPUT   : unit
 *           value (1 for Enable, 0 for disable)
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetLegacyDetection(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Notify_PortOverloadStatusChange
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_Notify_PortOverloadStatusChange(UI32_T group_index, UI32_T port_index, BOOL_T is_overload);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Notify_PowerDeniedOccurFrequently
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_Notify_PowerDeniedOccurFrequently(UI32_T group_index, UI32_T port_index);

/* Ed_huang 2006.7.19, modeifiy for 3COM style poe implementation */
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPortPowerMaximumAllocationByPri
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Set a specified port the maximum power
             according to the priority.
 * INPUT   : group_index                                                     
             port_index                                                      
             value 
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_MGR_SetPortPowerMaximumAllocationByPri(UI32_T group_index, UI32_T port_index, UI32_T priority, UI32_T power);

#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_GetOperationMode
 *-------------------------------------------------------------------------
 * PURPOSE  : get the bridge operation mode
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
UI32_T POE_MGR_GetOperationMode();

#ifdef SYS_CPNT_POE_PSE_DOT3AT
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_ResetPort
 *-------------------------------------------------------------------------
 * PURPOSE  : reset the a poe port in ASIC
 * INPUT    : lport
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_ResetPort(UI32_T unit,UI32_T port);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortDot3atHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for DLL)
 * INPUT    : lport
 *            mode : 1 - high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortDot3atHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode);

#if 0 /* Eugene call poedrv directly */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortDot3atHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for DLL)
 * INPUT    : lport
 *            mode : 1 - high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortForceHighPowerMode(UI32_T lport, UI32_T mode);
#endif

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_GetPortDetectionStatusPortMask
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the port status change mask
 * INPUT    : None
 * OUTPUT   : portmask: bitmask to indict the status-changed port
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_GetPortDetectionStatusPortMask(UI8_T portmask[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST]);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPoeMgtId
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the PoE task ID, message queue id
 * INPUT    : taskId : the ID of PoE Task
 *            msgid: the ID of message queue for PoE
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void POE_MGR_SetPoeMgtId(UI32_T taskId, UI32_T msgid);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_SetPortManualHighPowerMode
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the high power mode or normal mode (for UI manual setting)
 * INPUT    : unit
 *            port
 *            mode : 1 - force high power, 0 - normal
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_SetPortManualHighPowerMode(UI32_T unit, UI32_T port, UI32_T mode);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_ProcessTimerEvent
 *-------------------------------------------------------------------------
 * PURPOSE  : Process timer event
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     :
 *-------------------------------------------------------------------------
 */
void POE_MGR_ProcessTimerEvent(void);

#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for LLDP MGR.
 *
 * INPUT   : msgbuf_p -- input request ipc message buffer
 *
 * OUTPUT  : msgbuf_p -- output response ipc message buffer
 *
 * RETURN  : TRUE  - there is a response required to be sent
 *           FALSE - there is no response required to be sent
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
BOOL_T POE_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_NotifyLldpFameReceived_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : Set the PoE task ID
 * INPUT    : lport
 *            info: TLV data for dot3at
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_NotifyLldpFameReceived_Callback(UI32_T unit, UI32_T port, POE_TYPE_Dot3atPowerInfo_T *info);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortDetectionStatus_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortDetectionStatus_callback(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortAdmin_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortAdmin_callback(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPoweConsumption_callback
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.
 * INPUT   : group_index
             port_index
             value power consumption
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortPoweConsumption_callback(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetPsePortPowerClassification_callback
 * -------------------------------------------------------------------------
 * FUNCTION: update power classification for this port.
 * INPUT   : group_index
             port_index
             value power class
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : it diff with POE_MGR_SetPethPsePortAdmin(), here is no semaphore
 * -------------------------------------------------------------------------*/
void POE_MGR_SetPsePortPowerClassification_callback(UI32_T group_index, UI32_T port_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPsePower_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE power
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPsePower_callback(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseConsumptionPower_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE power consumption by callback
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPseConsumptionPower_callback(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_SetMainPseOperStatus_callback
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_SetMainPseOperStatus_callback(UI32_T group_index, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Notify_PowerDeniedOccurFrequently
 * -------------------------------------------------------------------------
 * FUNCTION: set Main PSE operation status
 * INPUT   : group_index
             value
 * OUTPUT  : None
 * RETURN  : TURE or FALSE
 * NOTE    : This is read only for user
 * -------------------------------------------------------------------------*/
void POE_MGR_Notify_PowerDeniedOccurFrequently(UI32_T group_index, UI32_T port_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_Register_PortOverloadStatusChange_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used for LED MGR to register its callback
 *           function for overload status.
 * INPUT:    fun -- the pointer of callback function.
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : void *fun(UI32_T unit, UI32_T port, BOOL_T is_overload)
 *           unit -- unit ID
 *           port -- port ID
 *           is_overload -- TRUE: overload, FALSE: normal condition
 * -------------------------------------------------------------------------*/
void POE_MGR_Register_PortOverloadStatusChange_CallBack(void (*fun)(UI32_T unit,UI32_T port,BOOL_T is_overload));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_PowerStatusChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: when power status changes, 
 * INPUT   : unit
 *           power  -- VAL_swIndivPowerIndex_externalPower
 *                     VAL_swIndivPowerIndex_internalPower
 *           status -- VAL_swIndivPowerStatus_green
 *                     VAL_swIndivPowerStatus_red
 *                     VAL_swIndivPowerStatus_notPresent
 *                     
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    :
 * -------------------------------------------------------------------------
 */
void POE_MGR_PowerStatusChanged_CallBack(UI32_T unit, UI32_T power, UI32_T status);

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_BindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Bind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index, time_range
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_BindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index, UI8_T* time_range);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_MGR_UnbindTimeRangeToPsePort
 *-------------------------------------------------------------------------
 * PURPOSE  : Unbind POE PSE port to a specified time range with name
 * INPUT    : group_index, port_index
 * OUTPUT   : None
 * RETURN   : POE_TYPE_RETURN_OK/POE_TYPE_RETURN_ERROR
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_MGR_UnbindTimeRangeToPsePort(UI32_T group_index, UI32_T port_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_MGR_TimeRangeStatusChange_callback
 * -------------------------------------------------------------------------
 * FUNCTION: time range status change callback function, register to time_range
 * INPUT   : isChanged_list -- if status changed by time range index list
 *           status_list    -- status by time range index list
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_MGR_TimeRangeStatusChange_callback(UI8_T *isChanged_list, UI8_T *status_list);
#endif

#endif /* END OF POE_MGR_H */
