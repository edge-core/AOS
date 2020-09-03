/*-----------------------------------------------------------------------------
 * FILE NAME: poe_om.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    This file declares the APIs for POE MGR to read/write the database.
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

#ifndef POE_OM_H
#define POE_OM_H

#include "sys_type.h"
#include "poe_type.h"

/*Eugene added*/
#include "poe_mgr.h"

/* command used in IPC message
 */
enum
{
    POE_OM_IPC_GETPSEPORTADMIN,
    POE_OM_IPC_GETPSEPORTPOWERPRIORITY,
    POE_OM_IPC_GETPSEPORTDECTECTIONSTATUS,
    POE_OM_IPC_GETPORTPOWERMAXIMUMALLOCATION,
    POE_OM_IPC_GETPORTPOWERCONSUMPTION,
    POE_OM_IPC_GETPORTMANUALHIGHPOWERMODE,
    POE_OM_IPC_GETMAINPOWERMAXIMUMALLOCATION,
    POE_OM_IPC_GETPOESOFTWAREVERSION,
    POE_OM_IPC_GETNEXTLEGACYDETECTION,
    POE_OM_IPC_GETNEXTPSEPORTADMIN,
    POE_OM_IPC_GETNEXTMAINPSEENTRY,
    POE_OM_IPC_GETPETHMAINPSEENTRY,
    POE_OM_IPC_GETPSEPORTENTRY,
    POE_OM_IPC_GETRUNNINGPSEPORTADMIN,
    POE_OM_IPC_GETRUNNINGPSEPORTPOWERPRIORITY,
    POE_OM_IPC_GETRUNNINGPORTPOWERMAXIMUMALLOCATION,
    POE_OM_IPC_GETRUNNINGMAINPOWERMAXIMUMALLOCATION,
    POE_OM_IPC_GETRUNNINGLEGACYDETECTION,
    POE_OM_IPC_GETMAINPSEOPERSTATUS,
    POE_OM_IPC_GETPSEPORTPOWERPAIRSCTRLABILITY,
    POE_OM_IPC_GETPSEPORTPOWERPAIRS,
    POE_OM_IPC_GETPSEPORTPOWERCLASSIFICATIONS,
    POE_OM_IPC_GETPORTPOWERCURRENT,
    POE_OM_IPC_GETPORTPOWERVOLTAGE,
    POE_OM_IPC_GETNEXTPSEPORTENTRY,
    POE_OM_IPC_GETPSENOTIFICATIONCONTROL,
    POE_OM_IPC_GETNEXTNOTIFICATIONCONTROL,
    POE_OM_IPC_GETDOT3ATPORTPOWERINFO,
    POE_OM_IPC_GETRUNNINGPORTMANUALHIGHPOWERMODE,
    POE_OM_IPC_GETUSELOCALPOWER,
    POE_OM_IPC_GETPSEPORTTIMERANGENAME,
    POE_OM_IPC_GETNEXTPSEPORTTIMERANGENAME,
    POE_OM_IPC_GETRUNNINGPSEPORTTIMERANGENAME,
    POE_OM_IPC_GETNEXTRUNNINGPSEPORTTIMERANGENAME,
    POE_OM_IPC_GETPSEPORTTIMERANGESTATUS,
    POE_OM_IPC_GETNEXTPSEPORTTIMERANGESTATUS,
};

/* NAMING CONSTANT DECLARATIONS
 */
#define POE_OM_IPCMSG_TYPE_SIZE sizeof(union POE_OM_IpcMsg_Type_U)

/* Macro function for computation of IPC msg_buf size based on field name
 * used in POE_OM_IpcMsg_T.data
 */
#define POE_OM_GET_MSG_SIZE(field_name)                        \
            (POE_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((POE_OM_IpcMsg_T *)0)->data.field_name))

/* IPC message structure
 */
typedef struct
{
    union POE_OM_IpcMsg_Type_U
    {
        UI32_T cmd;
        UI32_T ret_ui32;
        BOOL_T ret_bool;
    } type; /* the intended action or return value */

    union
    {
        struct
        {
            UI32_T arg_ui32;
            BOOL_T arg_bool;
        } arg_grp_ui32_bool;

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
            UI32_T arg_ui32;
            UI8_T arg_ui8_1;
            UI8_T arg_ui8_2;
            UI8_T arg_ui8_3;
        } arg_grp_ui32_ui8_ui8_ui8;

        struct
        {
            UI32_T arg_ui32;
            POE_OM_MainPse_T arg_entry;
        } arg_grp_ui32_mainpse;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            POE_OM_PsePort_T arg_entry;
        } arg_grp_ui32_ui32_pseport;

        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            POE_TYPE_Dot3atPowerInfo_T arg_entry;
        } arg_grp_ui32_ui32_3atInfo;

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
        struct
        {
            UI32_T arg_ui32_1;
            UI32_T arg_ui32_2;
            UI8_T  arg_ui8[SYS_ADPT_TIME_RANGE_MAX_NAME_LENGTH+1];
        } arg_grp_ui32_ui32_ui8;
#endif

    } data; /* the argument(s) for the function corresponding to cmd */
}POE_OM_IpcMsg_T;

/*End of Eugene added*/


/* ------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_InitSemaphore
 * ------------------------------------------------------------------------
 * PURPOSE  : Initialize the semaphore for POE objects
 * INPUT    : None
 * OUTPUT   : None
 * RETUEN   : None
 * NOTES    : None
 * ------------------------------------------------------------------------*/
void POE_OM_InitSemaphore(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetDBDefaultValue                               
 * -------------------------------------------------------------------------
 * FUNCTION: get database's default value
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_GetDBDefaultValue(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetDBToDefault                               
 * -------------------------------------------------------------------------
 * FUNCTION: set database to default value
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetDBToDefault(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetDBToDefaultForHotInsert
 * -------------------------------------------------------------------------
 * FUNCTION: set database to default value for specific unit when hot insert
 * INPUT   : unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetDBToDefaultForHotInsert(UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_ClearDB                               
 * -------------------------------------------------------------------------
 * FUNCTION: clear database
 * INPUT   : None                                                   
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_ClearDB(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port of poe device is existing
 * INPUT   : unit -- unit ID, port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_UserPortExisting(UI32_T group_index, UI32_T port_index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortAdmin                               
 * -------------------------------------------------------------------------
 * FUNCTION: enables power and detection mechanism for this port.           
 * INPUT   : unit                                                     
             port                                                      
             value = enable(1) enables power and detection mechanism for this port.
                   = disable(2) disables power for this port.               
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortAdmin(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortAdmin                              
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port admin status on this port
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortAdmin(UI32_T unit, UI32_T port, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortAdmin(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortAdmin
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortAdmin(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPairsCtrlAbility                              
 * -------------------------------------------------------------------------
 * FUNCTION: Describes the capability of controlling the power pairs
 *           functionality to switch pins for sourcing power.                                  
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPairsCtrlAbility(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPairsCtrlAbility
 * -------------------------------------------------------------------------
 * FUNCTION: Describes the capability of controlling the power pairs
 *           functionality to switch pins for sourcing power.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPairsCtrlAbility(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerPairs                          
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.                        
 * INPUT   : unit                                                     
             port                                                      
             value = signal(1)means that the signal pairs only are in use.  
                   = spare(2) means that the spare pairs only are in use.   
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : If the value of pethPsePortPowerPairsControl is true,          
             this object is writable.                                       
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerPairs(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPairs                           
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.                        
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPairs(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortPowerPairs(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortPowerPairs
 * -------------------------------------------------------------------------
 * FUNCTION: Describes or controls the pairs in use.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortPowerPairs(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerDetectionCtrl       
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.            
 * INPUT   : unit                                                     
             port                                                      
             value = auto(1)enables the power detection mechanism of the port.
                   = test(2)puts the port in a testmode:                    
                     force continuous discovery without applying            
                     power regardless of whether PD detected.               
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerDetectionCtrl(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerDetectionCtrl                            
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the power detection mechanism of the port.            
 * INPUT   : unit                                                     
             port                                                      
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_GetPsePortPowerDetectionCtrl(UI32_T unit, UI32_T port, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortDetectionStatus               
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the operational status of the port PD detection            
 * INPUT   : unit                                                     
             port                                                      
             value = disabled(1), searching(2), deliveringPower(4), 
                     fault(5), test(7), denyLowPriority(8)              
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortDetectionStatus(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortDetectionStatus                        
 * -------------------------------------------------------------------------
 * FUNCTION: the operational status of the port PD detection                              
 * INPUT   : group_index
             port_index
 * OUTPUT  : value : disabled(1), searching(2), deliveringPower(4), fault(5), test(7), denyLowPriority(8)
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortDetectionStatus(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortDetectionStatus
 * -------------------------------------------------------------------------
 * FUNCTION: the operational status of the port PD detection
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value : disabled(1), searching(2), deliveringPower(4), fault(5), test(7), denyLowPriority(8)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortDetectionStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerConsumption               
 * -------------------------------------------------------------------------
 * FUNCTION: Controls the operational status of the port PD detection            
 * INPUT   : unit                                                     
             port                                                      
             value = disabled(1), searching(2), deliveringPower(4), 
                     fault(5), test(7), denyLowPriority(8)              
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerConsumption(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerConsumption                        
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the power consumption
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerConsumption(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerPriority                       
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point   
             of view of a power management algorithm.                       
 * INPUT   : unit                                                     
             port                                                      
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
void POE_OM_SetPsePortPowerPriority(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point   
             of view of a power management algorithm.                       
 * INPUT   : group_index
             port_index
 * OUTPUT  : value = critical(1)                                            
                   = high(2)                                                
                   = low(3)                                                 

 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value = critical(1)
                   = high(2)
                   = low(3)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerPriority(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value = critical(1)
                   = high(2)
                   = low(3)

 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortPowerPriority(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortPowerPriority
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the priority of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value = critical(1)
                   = high(2)
                   = low(3)
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortPowerPriority(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerMaintenanceStatus
 * -------------------------------------------------------------------------
 * FUNCTION: get PSE power maintenance status  
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : The value ok(1) indicates the Power Maintenance
             Signature is present and the overcurrent condition has not been
             detected.
             The value overCurrent (2) indicates an overcurrent condition
             has been detected.
             The value mPSAbsent(3) indicates that the Power Maintenance
             Signature is absent.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerMaintenanceStatus(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerMaintenanceStatus
 * -------------------------------------------------------------------------
 * FUNCTION: get PSE power maintenance status
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : The value ok(1) indicates the Power Maintenance
             Signature is present and the overcurrent condition has not been
             detected.
             The value overCurrent (2) indicates an overcurrent condition
             has been detected.
             The value mPSAbsent(3) indicates that the Power Maintenance
             Signature is absent.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerMaintenanceStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortMPSAbsentCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the
             pethPsePortPowerMaintenanceStatus attribute changes from any
             value to the value mPSAbsent(3)          
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortMPSAbsentCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortMPSAbsentCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the
             pethPsePortPowerMaintenanceStatus attribute changes from any
             value to the value mPSAbsent(3)
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortMPSAbsentCounter(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortOverCurrCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the aPSEPowerCurrentStatus
             attribute changes from any value to the value overCurrent(2).
 * INPUT   : group_index
             port_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortOverCurrCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortOverCurrCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Counts the number of times that the aPSEPowerCurrentStatus
             attribute changes from any value to the value overCurrent(2).
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortOverCurrCounter(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

#if (SYS_CPNT_POE_COUNTER_SUPPORT==TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortInvalidSignCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port invalid signature counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortInvalidSignCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerDeniedCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port power denied counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerDeniedCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortOverloadCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port overload counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortOverloadCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortShortCounter
 * -------------------------------------------------------------------------
 * FUNCTION: Get the port short counters.
 *
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : counter
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortShortCounter(UI32_T group_index, UI32_T port_index, UI32_T *value);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPoeSoftwareVersion
 * -------------------------------------------------------------------------
 * FUNCTION: Get software version from PoE controller
 * INPUT   : unit -- unit ID
 * OUTPUT  : version1 -- version number 1
 *           version2 -- version number 2
 *           build    -- build number
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPoeSoftwareVersion(UI32_T unit, UI8_T *version1, UI8_T *version2, UI8_T *build);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerTemperature
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port temperature
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerTemperature(UI32_T group_index, UI32_T port_index, I32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerVoltage
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port voltage
 * INPUT   : group_index
             port_index
 * OUTPUT  : value (V)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerVoltage(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerCurrent
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port current
 * INPUT   : group_index
             port_index
 * OUTPUT  : value (mA)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerCurrent(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortType                                
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value       
             that indicates the type of the device that is connected        
             to theport. This value can be the result of the mapping        
             the address of the station connected to the port and of        
             the value of the pethPdPortType of the respective PD port.     
 * INPUT   : unit                                                     
             port                                                      
             value
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortType(UI32_T unit, UI32_T port, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value       
             that indicates the type of the device that is connected        
             to theport. This value can be the result of the mapping        
             the address of the station connected to the port and of        
             the value of the pethPdPortType of the respective PD port.     
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
             that indicates the type of the device that is connected
             to theport. This value can be the result of the mapping
             the address of the station connected to the port and of
             the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortType(UI32_T *group_index, UI32_T *port_index, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
             that indicates the type of the device that is connected
             to theport. This value can be the result of the mapping
             the address of the station connected to the port and of
             the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortType(UI32_T group_index, UI32_T port_index, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortType
 * -------------------------------------------------------------------------
 * FUNCTION: A manager will set the value of this variable to a value
             that indicates the type of the device that is connected
             to theport. This value can be the result of the mapping
             the address of the station connected to the port and of
             the value of the pethPdPortType of the respective PD port.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortType(UI32_T *group_index, UI32_T *port_index, UI8_T *value, UI32_T len);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortPowerClassification               
 * -------------------------------------------------------------------------
 * FUNCTION: Update the power class of a port            
 * INPUT   : unit                                                     
             port                                                      
             value 
 * OUTPUT  : None                                                           
 * RETURN  : 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortPowerClassification(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortPowerClassifications
 * -------------------------------------------------------------------------
 * FUNCTION: Classification is a way to tag different terminals on the
             Power over LAN network according to their power consumption.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value : class0(1),
                     class1(2),
                     class2(3),
                     class3(4),
                     class4(5)

 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortPowerClassifications(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortPowerClassifications
 * -------------------------------------------------------------------------
 * FUNCTION: Classification is a way to tag different terminals on the
             Power over LAN network according to their power consumption.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value : class0(1),
                     class1(2),
                     class2(3),
                     class3(4),
                     class4(5)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortPowerClassifications(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPortPowerMaximumAllocation                                
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Set a specified port the maximum power
 * INPUT   : unit                                                     
             port                                                      
             value
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPortPowerMaximumAllocation(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
             port_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPortPowerMaximumAllocation(UI32_T group_index, UI32_T port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPortPowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get a specified port the maximum power
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPortPowerMaximumAllocation(UI32_T *group_index, UI32_T *port_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPsePower                              
 * -------------------------------------------------------------------------
 * FUNCTION: set the nominal power of the PSE expressed in Watts.           
 * INPUT   : unit                                                      
             value             
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                      
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPsePower(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: The nominal power of the PSE expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPsePower(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPsePower
 * -------------------------------------------------------------------------
 * FUNCTION: The nominal power of the PSE expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPsePower(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPseOperStatus                            
 * -------------------------------------------------------------------------
 * FUNCTION: set the operational status of the main PSE.          
 * INPUT   : unit                                                      
             value = on(1)
                   = off(2) 
                   = faulty(3)            
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                         
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseOperStatus(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: The operational status of the main PSE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseOperStatus(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseOperStatus
 * -------------------------------------------------------------------------
 * FUNCTION: The operational status of the main PSE.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseOperStatus(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPsePower                              
 * -------------------------------------------------------------------------
 * FUNCTION: set the available power of the PSE expressed in Watts.           
 * INPUT   : unit                                                      
             value             
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                      
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseConsumptionPower(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseConsumptionPower
 * -------------------------------------------------------------------------
 * FUNCTION: Measured usage power expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseConsumptionPower(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseConsumptionPower
 * -------------------------------------------------------------------------
 * FUNCTION: Measured usage power expressed in Watts.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseConsumptionPower(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainPseUsageThreshold                      
 * -------------------------------------------------------------------------
 * FUNCTION: set the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.                         
 * INPUT   : unit                                                     
             value = (1..99)
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainPseUsageThreshold(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.                         
 * INPUT   : group_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainPseUsageThreshold(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseUsageThreshold(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningMainPseUsageThreshold(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningMainPseUsageThreshold
 * -------------------------------------------------------------------------
 * FUNCTION: get the usage threshold expressed in percents for
             comparing the measured power and initiating
             an alarm if the threshold is exceeded.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningMainPseUsageThreshold(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetMainpowerMaximumAllocation                      
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get the power, available for Power
 *           Management on PoE.                
 * INPUT   : unit                                                     
             value
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 36 to 800 watts.                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetMainpowerMaximumAllocation(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainpowerMaximumAllocation(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningMainpowerMaximumAllocation(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningMainpowerMaximumAllocation
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to get the power, available for Power
 *           Management on PoE.
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : Power can be set from 36 to 800 watts.
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningMainpowerMaximumAllocation(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetNotificationCtrl                               
 * -------------------------------------------------------------------------
 * FUNCTION: set notification control        
 * INPUT   : unit
 *           value
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetNotificationCtrl(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPseNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port                                           
 * INPUT   : group_index
 * OUTPUT  : value 
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPseNotificationCtrl(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 * OUTPUT  : group_index
             value
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextNotificationCtrl(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
 * OUTPUT  : value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningNotificationCtrl(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningNotificationCtrl
 * -------------------------------------------------------------------------
 * FUNCTION: Enables power supply on this port
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             value
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningNotificationCtrl(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetLegacyDetection                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection	       
 * INPUT   : unit
 *           value (1 for Enable, 0 for disable)
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetLegacyDetection(UI32_T unit, UI32_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetLegacyDetection      
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection                                          
 * INPUT   : group_index                                                     
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetLegacyDetection(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : unit
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextLegacyDetection(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : unit
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningLegacyDetection(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningLegacyDetection
 * -------------------------------------------------------------------------
 * FUNCTION: Get Legacy Detection
 * INPUT   : unit
 * OUTPUT  : value (1 for Enable, 0 for disable)
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningLegacyDetection(UI32_T *group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control the power
             characteristics power Ethernet ports on a Power Source
             Entity (PSE) device.
 * INPUT   : group_index
             port_index
 * OUTPUT  : entry (POE_MGR_PETH_PSE_PORT_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortEntry(UI32_T group_index, UI32_T port_index, POE_OM_PsePort_T *entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control next power
             characteristics power Ethernet ports on a Power Source
             Entity (PSE) device.
 * INPUT   : group_index
             port_index
 * OUTPUT  : group_index
             port_index
             entry (POE_MGR_PETH_PSE_PORT_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortEntry(UI32_T *group_index, UI32_T *port_index, POE_OM_PsePort_T *entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPethMainPseEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control the Main power
             on a PSE  device. Example - an Ethernet switch midspan device can
               control an Ethnternet port and the Main Power supply unit's.
 * INPUT   : group_index
 * OUTPUT  : entry (POE_MGR_PETH_MAIN_PSE_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPethMainPseEntry(UI32_T group_index, POE_OM_MainPse_T *entry);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextMainPseEntry
 * -------------------------------------------------------------------------
 * FUNCTION: This function will display and control next Main power
             on a PSE  device. Example - an Ethernet switch midspan device can
               control an Ethnternet port and the Main Power supply unit's.
 * INPUT   : group_index
 * OUTPUT  : entry (POE_MGR_PETH_MAIN_PSE_T)
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextMainPseEntry(UI32_T *group_index, POE_OM_MainPse_T *entry);

#if (SYS_CPNT_POE_TIME_RANGE == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeIndex                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range index on this port
 * INPUT   : group_index
 *           port_index
 *           index - time range index 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeIndex(UI32_T group_index, UI32_T port_index, UI32_T index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeIndex
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range index on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : index - time range index 
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeIndex(UI32_T group_index, UI32_T port_index, UI32_T *index);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeName                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 *           time_range - time range name 
 * OUTPUT  : None                                                           
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T* time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range - time range name 
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range - time range name 
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPsePortTimeRangeName(UI32_T group_index, UI32_T port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextRunningPsePortTimeRangeName
 * -------------------------------------------------------------------------
 * FUNCTION: Get running POE port binding time range name on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           time_range
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetNextRunningPsePortTimeRangeName(UI32_T *group_index, UI32_T *port_index, UI8_T *time_range);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetPsePortTimeRangeStatus                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 *           status - time range status 
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POE_OM_SetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on this port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : status - time range status 
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPsePortTimeRangeStatus(UI32_T group_index, UI32_T port_index, UI32_T *status);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetNextPsePortTimeRangeStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get POE port binding time range status on next port
 * INPUT   : group_index
 *           port_index
 * OUTPUT  : group_index
 *           port_index
 *           status - time range status 
 * RETURN  : TURE or FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetNextPsePortTimeRangeStatus(UI32_T *group_index, UI32_T *port_index, UI32_T *status);
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_SetUseLocalPower                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection        
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
void POE_OM_SetUseLocalPower(UI32_T unit, BOOL_T value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetUseLocalPower                               
 * -------------------------------------------------------------------------
 * FUNCTION: Set Legacy Detection        
 * INPUT   : unit
 *           value (TRUE => Local power, FALSE => RPS)
 * OUTPUT  : None                           
 * RETURN  : None                                                 
 * NOTE    : None                                                           
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetUseLocalPower(UI32_T unit, BOOL_T *value);

/* Ed_huang 2006.7.19, modeifiy for 3COM style poe implementation */
#ifdef SYS_CPNT_POE_STYLE_3COM_HUAWEI
/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetAllHighPriorityPsePortPowerMaxAllocation               
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get total maximum power of the high 
             priority port
 * INPUT   : group_index                                                                                                        
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.                                        
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetAllHighPriorityPsePortPowerMaxAllocation(UI32_T group_index, UI32_T *value);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetExistedHighPriorityPowerMaxAllocation               
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get maximum power of the high 
             priority port except the specific port.
 * INPUT   : group_index                                                                                                        
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    : Power can be set from 3000 to 21000 milliwatts.                                        
 * -------------------------------------------------------------------------*/
void POE_OM_GetExistedHighPriorityPowerMaxAllocation(UI32_T unit, UI32_T port_index, UI32_T *value);
#endif

#ifdef SYS_CPNT_POE_PSE_DOT3AT
/* -------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_SetPortManualHighPowerMode               
 * -------------------------------------------------------------------------
 * FUNCTION: This function is used to Get current manual high power mode
 * INPUT   : group_index 
 *           port_index
 * OUTPUT  : value
 * RETURN  : TURE or FALSE                                                  
 * NOTE    :                                     
 * -------------------------------------------------------------------------*/
void POE_OM_SetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T mode);

/* -------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_GetPortManualHighPowerMode               
 * -------------------------------------------------------------------------
 * FUNCTION : This function is used to Get current manual high power mode
 * INPUT    : group_index 
 *            port_index
 * OUTPUT   : mode : 1 - force high power, 0 - normal
 * RETURN   : TURE or FALSE                                                  
 * NOTE     :                                     
 * -------------------------------------------------------------------------*/
BOOL_T POE_OM_GetPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *mode);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POE_OM_GetRunningPortManualHighPowerMode
 * -------------------------------------------------------------------------
 * FUNCTION: This object controls the high-power of the port from the point
             of view of a power management algorithm.
 * INPUT   : group_index
             port_index
 * OUTPUT  : value = normal(0)
                   = high-power(1)

 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS : success
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL : fail
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : data no changed
 * NOTE    : None
 * -------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T POE_OM_GetRunningPortManualHighPowerMode(UI32_T group_index, UI32_T port_index, UI32_T *value);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_GetPortDot3atPowerInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : Get the poe infomation for LLDP to transmition frame
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
BOOL_T POE_OM_GetPortDot3atPowerInfo(UI32_T group_index, UI32_T port_index, POE_TYPE_Dot3atPowerInfo_T *info);
#endif

/*-----------------------------------------------------------------------------
 * FUNCTION NAME - POE_OM_HandleIPCReqMsg
 *-----------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for POE OM.
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
BOOL_T POE_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);


#endif /* END OF POE_OM_H */
