/*-----------------------------------------------------------------------------
 * FILE NAME: poedrv.h
 *-----------------------------------------------------------------------------
 * PURPOSE: 
 *    The PoE (Power over Ethernet) driver provides the services for
 *    upper-layer modules to access PoE controller.
 * 
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    03/31/2003 - Benson Hsu, Created	
 *    07/01/2009 - Eugene Yu, Porting to Linux platform
 *
 * Copyright(C)      Accton Corporation, 2008
 *-----------------------------------------------------------------------------
 */

#ifndef POEDRV_H
#define POEDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_STACKING == TRUE)
#include "isc.h"
#endif


/* NAME CONSTANT DECLARATIONS
 */
enum POEDRV_DRAGONITE_OPERATION_COMMAND_E
{
    POEDRV_DRAGONITE_MIN_COMMAND_EVENT = 0,
    POEDRV_DRAGONITE_INIT_SYSTEM,
    POEDRV_DRAGONITE_INIT_SYSTEM_2,
    POEDRV_DRAGONITE_GET_SYSTEM_INFORMATION = 1000,
    POEDRV_DRAGONITE_GET_CHIPSET_INFORMATION,
    POEDRV_DRAGONITE_GET_CONFIG_INFORMATION,
    POEDRV_DRAGONITE_GET_PORT_STATUS,
    POEDRV_DRAGONITE_GET_PORT_MEASURE,
    POEDRV_DRAGONITE_GET_POE_SOFTWARE_VERSION,
    POEDRV_DRAGONITE_SET_SYSTEM_MAINPOWER_MAXIMUM_ALLOCATION = 2000,
    POEDRV_DRAGONITE_SET_PORT_ADMIN,
    POEDRV_DRAGONITE_SET_PORT_PRIORITY,
    POEDRV_DRAGONITE_SET_PORT_POWER_MAXIMUM_ALLOCATION,
    POEDRV_DRAGONITE_SET_CONFIG_CAPACITOR_DETECTION,
    POEDRV_DRAGONITE_MAX_COMMAND_EVENT
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */ 

/* FUNCTION NAME : POEDRV_InitiateSystemResources
 * PURPOSE: This function initializes all releated variables and restarts
 *          the PoE driver.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T POEDRV_InitiateSystemResources(void);

/* FUNCTION NAME : POEDRV_AttachSystemResources
 * PURPOSE: Attach system resource for POEDRV
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_AttachSystemResources(void);

/* FUNCTION NAME : POEDRV_SetTransitionMode
 * PURPOSE: This function is used to set POEDRV in transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_SetTransitionMode(void);


/* FUNCTION NAME : POEDRV_EnterTransitionMode
 * PURPOSE: This function is used to force POEDRV to enter transition mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterTransitionMode(void);


/* FUNCTION NAME : POEDRV_EnterMasterMode
 * PURPOSE: This function is used to force POEDRV to enter master mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterMasterMode(void);


/* FUNCTION NAME : POEDRV_EnterSlaveMode
 * PURPOSE: This function is used to force POEDRV to enter slave mode
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void POEDRV_EnterSlaveMode(void);


 /* ROUTINE NAME - POEDRV_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Switch Driver module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void POEDRV_ProvisionComplete(void);


/* FUNCTION NAME : POEDRV_CreateTasks
 * PURPOSE: This function is used to create the main task of PoE driver.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T POEDRV_CreateTasks(void);


/* FUNCTION NAME : POEDRV_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 */
void POEDRV_Create_InterCSC_Relation(void);

void POEDRV_HotSwapInsert(void);

void POEDRV_HotSwapremove(void);


/* FUNCTION NAME : POEDRV_Register_PortDetectionStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If status of a port has been changed, the registered function 
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *func(UI32_T unit, UI32_T port, UI32_T detection_status)
 *          unit -- unit ID
 *          port -- port ID
 *          detection_status -- detection status of a port
 */
void POEDRV_Register_PortDetectionStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T detection_status));


/* FUNCTION NAME : POEDRV_Register_PortPowerConsumptionChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If status of a port has been changed, the registered function 
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T detection_status)
 *          unit -- unit ID
 *          port -- port ID
 *          power_consumption -- power consumption change  of a port
 */
void POEDRV_Register_PortPowerConsumptionChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T power_consumption));


/* FUNCTION NAME : POEDRV_Register_PortOverloadStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If overload status of a port has been changed, the registered 
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_overload)
 *          unit -- unit ID
 *          port -- port ID
 *          is_overload -- overload status of a port
 */
void POEDRV_Register_PortOverloadStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_overload));

#ifdef BES50
/* FUNCTION NAME : POEDRV_Register_PortOverPowerManagementStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If overload status of a port has been changed, the registered 
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_over_power_budget)
 *          unit -- unit ID
 *          port -- port ID
 *          is_over_power_budget -- over power budget
 */
void POEDRV_Register_PortOverPowerManagementStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_over_power_budget));
#endif

/* FUNCTION NAME : POEDRV_Register_PortFailureStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If port failure status has been changed, the registered 
 *          function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, BOOL_T is_port_failure)
 *          unit -- unit ID
 *          port -- port ID
 *          is_port_failure -- port failure status
 */
void POEDRV_Register_PortFailureStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, BOOL_T is_port_failure));


/* FUNCTION NAME : POEDRV_Register_PortPowerClassificationChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If power class of a port has been changed, the registered function 
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T port, UI32_T power_class)
 *          unit -- unit ID
 *          port -- port ID
 *          power_class -- power class of a port
 */
void POEDRV_Register_PortPowerClassificationChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T power_class));


/* FUNCTION NAME : POEDRV_Register_MainPseConsumptionChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If status of main power has been changed, the registered function 
 *          should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *func(UI32_T unit, UI32_T port, UI32_T power_consumption)
 *          unit -- unit ID
 *          power_consumption -- main PSE power consumption
 */
void POEDRV_Register_MainPseConsumptionChange_CallBack(void (*fun)(UI32_T unit, UI32_T power_consumption));


/* FUNCTION NAME : POEDRV_Register_PseOperStatusChange_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If the operational status of the main PSE has been changed,  
 *          the registered function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *func(UI32_T unit, UI32_T oper_status)
 *          unit -- unit ID
 *          oper_status -- VAL_pethMainPseOperStatus_on
 *                         VAL_pethMainPseOperStatus_off
 *                         VAL_pethMainPseOperStatus_faulty
 */
void POEDRV_Register_PseOperStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T oper_status));


/* FUNCTION NAME : POEDRV_Register_Legacy_Detection_CallBack
 * PURPOSE: This function is used to register the callback function. 
 *          If the operational status of the legacy detection falg has 
            been changed,  the registered function should be notified.
 * INPUT:   fun -- the pointer of callback function.
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   void *fun(UI32_T unit, UI32_T oper_status)
 *          unit -- unit ID
 *          oper_status -- 1 :enable
 *                         0 :disable
 *                         
 */
void POEDRV_Register_Legacy_Detection_CallBack(void (*fun)(UI32_T unit, UI8_T oper_status));

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_UserPortExisting
 * -------------------------------------------------------------------------
 * FUNCTION: This function will return if this user port of poe device is existing
 * INPUT   : unit -- unit ID, port -- port number
 * OUTPUT  : None
 * RETURN  : TRUE: Existing, FALSE: Not existing
 * NOTE    : None
 * -------------------------------------------------------------------------*/
BOOL_T POEDRV_UserPortExisting(UI32_T unit, UI32_T port);

/* FUNCTION NAME : POEDRV_SetPortAdminStatus
 * PURPOSE: This function is used to enable or disable power and detection mechanism 
 *          for a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          admin_status -- VAL_pethPsePortAdminEnable_enable
 *                          VAL_pethPsePortAdminEnable_disable
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 *          
 */
BOOL_T POEDRV_SetPortAdminStatus(UI32_T unit, UI32_T port, UI32_T admin_status);


/* FUNCTION NAME : POEDRV_SetPortPowerDetectionControl
 * PURPOSE: This function is used to Controls the power detection mechanism 
 *          of a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          mode -- VAL_pethPsePortPowerDetectionControl_auto
 *                  VAL_pethPsePortPowerDetectionControl_test
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   auto: enables the power detection mechanism of the port.
 *          test: force continuous discovery without applying power 
 *          regardless of whether PD detected.
 */
BOOL_T POEDRV_SetPortPowerDetectionControl(UI32_T unit, UI32_T port, UI32_T mode);


/* FUNCTION NAME: POEDRV_SetPortPowerPriority
 * PURPOSE: This function is used to set a priority to a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          priority -- VAL_pethPsePortPowerPriority_critical
 *                      VAL_pethPsePortPowerPriority_high
 *                      VAL_pethPsePortPowerPriority_low
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   The priority that is set by this variable could be used by a 
 *          control mechanism that prevents over current situations by 
 *          disconnecting first ports with lower power priority.
 */
BOOL_T POEDRV_SetPortPowerPriority(UI32_T unit, UI32_T port, UI32_T priority);


/* FUNCTION NAME: POEDRV_SetPortPowerMaximumAllocation
 * PURPOSE: This function is used to set a specified port the maximum power
 *          in milliwatts.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 *          milliwatts -- power limit
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 3000 to 21000 milliwatts.
 */
BOOL_T POEDRV_SetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T milliwatts);


/* FUNCTION NAME: POEDRV_SetMainpowerMaximumAllocation
 * PURPOSE: This function is used to set the power, available for Power
 *          Management on PoE.
 * INPUT:   watts -- power available on PoE
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   Power can be set from 36 to 800 watts.
 */
BOOL_T POEDRV_SetMainpowerMaximumAllocation(UI32_T unit, UI32_T watts);


/* FUNCTION NAME: POEDRV_GetPortPowerConsumption
 * PURPOSE: This function is used to get power consumption of a port from
 *          local database.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_consumption -- power consumption of a port in milliwatts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPortPowerConsumption(UI32_T unit, UI32_T port, UI32_T *power_consumption);
/* FUNCTION NAME: POEDRV_GetGetPortPowerClassification
 * PURPOSE: This function is used to get power classification of a port from
 *          local database.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_class-- power classification of a port 
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPortPowerClassification(UI32_T unit, UI32_T port, UI32_T *power_class);


/* FUNCTION NAME: POEDRV_SetMainpowerConsumption
 * PURPOSE: This function is used to get mainpower consumption on PoE.
 * INPUT:   unit -- unit ID
 * OUTPUT:  watts -- power consumption of PoE in watts
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetMainpowerConsumption(UI32_T unit, UI32_T *watts);


/* FUNCTION NAME: POEDRV_GetPortsStatus
 * PURPOSE: This function is used to get status of a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  port_status -- data buffer for status of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPortStatus(UI32_T unit, UI32_T port, UI8_T *port_status);


/* FUNCTION NAME: POEDRV_GetPortPriority
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPortPriority(UI32_T unit, UI32_T port, UI8_T *priority);


/* FUNCTION NAME: POEDRV_GetPortPowerMaximumAllocation
 * PURPOSE: This function is used to get power limit for a port.
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  power_limit -- port defined power in milliwatts.
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPortPowerMaximumAllocation(UI32_T unit, UI32_T port, UI32_T *power_limit);


/* FUNCTION NAME: POEDRV_GetMainPowerParameters
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetMainPowerParameters(UI32_T unit, UI32_T *power_available);


/* FUNCTION NAME : POEDRV_HardwareReset
 * PURPOSE: This function is used to issue a hardware reset to PoE controller.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An hardware reset, bit 6 in system reset register, will be issued 
 *          to PoE controller, as shown in following.
 *
 *          /reset _____          ____     
 *                      |________|   
 *                        
 *                      |<-10ms->|
 */
void POEDRV_HardwareReset(void);


/* FUNCTION NAME : POEDRV_ReleaseSoftwareReset
 * PURPOSE: This function is used to release/hold software reset for PoE controller.
 *          PoE controller will start/stop powering connected PDs.
 * INPUT:   is_enable -- TRUE : port powering enabled
 *                       FALSE: port powering disabled
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:   An software reset, by setting bit-5 in system reset register, will be issued 
 *          to PoE controller, as shown in following.
 *
 *          /reset       _________     
 *                 _____|           
 *                        
 *                      |<-10ms->|
 */
void POEDRV_ReleaseSoftwareReset(BOOL_T is_enable);

/* FUNCTION NAME : POEDRV_SetSystemMask
 * PURPOSE: This function is used to set system mask
 * INPUT:   system_mask
 * OUTPUT:  none
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_SetSystemMask(UI8_T system_mask);

/* FUNCTION NAME : POEDRV_GetSystemMask
 * PURPOSE: This function is used to query system mask
 * INPUT:   None
 * OUTPUT:  system_mask : system mask on PoE controller
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetSystemMask(UI8_T *system_mask);


/* FUNCTION NAME : POEDRV_SendRawPacket
 * PURPOSE: This function is used to send a raw packet from engineering backdoor
 * INPUT:   transmit: data pointer of 13-byte packet to be transmitted
 * OUTPUT:  receive : data pointer of receiving packet
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_SendRawPacket(UI8_T *transmit, UI8_T *receive);


/* FUNCTION NAME : POEDRV_SoftwareDownload
 * PURPOSE: This function is used to upgrade software version of PoE controller
 * INPUT:   unit -- unit ID
 *          filename -- filename to be downloaded
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:  
 */
BOOL_T POEDRV_SoftwareDownload(UI32_T unit, UI8_T *filename);

/* FUNCTION NAME : POEDRV_GetPoeSoftwareVersion
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : unit -- unit ID
 * OUTPUT  : version1 -- version number 1
 *           version2 -- version number 2
 *           build    -- build number     
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPoeSoftwareVersion(UI32_T unit, UI8_T *version1, UI8_T *version2, UI8_T *build);
/* FUNCTION NAME : POEDRV_SetCapacitorDetectionControl
 * PURPOSE: This function is used to query software version on PoE ASIC
 * INPUT   : mask
 * OUTPUT  : none
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_SetCapacitorDetectionControl(UI32_T unit, UI8_T value);
/* FUNCTION NAME: POEDRV_SetClassMode
 * PURPOSE: This function is used to set class mode
 *      
 * INPUT:   unit -- unit ID
 *          class -- POE mode
 * OUTPUT:  None
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_SetClassMode(UI32_T unit, BOOL_T ClassMode);
/* FUNCTION NAME: POEDRV_GetClassMode
 * PURPOSE: This function is used to get the priority for a port
 * INPUT:   unit -- unit ID
 *          port -- port ID
 * OUTPUT:  priority -- the priority of a port
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetClassMode(BOOL_T *ClassMode);

/* FUNCTION NAME: POEDRV_GetPoeDeviceTemperature
 * PURPOSE: This function is used to get the temperature for poe device
 * INPUT:   deviceid -- poe device id
 * OUTPUT:  temperature -- the temperature of the device
 * RETURN:  TRUE  -- successful
 *          FALSE -- failed
 * NOTES:   
 */
BOOL_T POEDRV_GetPoeDeviceTemperature(UI32_T unit, UI32_T deviceid, UI32_T *temperature);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - POEDRV_GetPortIsSupportPoe
 * -------------------------------------------------------------------------
 * PURPOSE: This function is used to check whether the port support POE
 * INPUT:   port -- port ID
 * OUTPUT:
 * RETURN:  TRUE: support POE, 
 *          FALSE: Not support POE
 * NOTES:   2006.11.29 aken, for WEB check port
 * -------------------------------------------------------------------------*/
BOOL_T POEDRV_GetPortIsSupportPoe(UI32_T port);

/*for POE Led*/
void POEDRV_Register_PortStatusChange_CallBack(void (*fun)(UI32_T unit, UI32_T port, UI32_T actual_port_status));

/* -------------------------------------------------------------------------
 * Function : POEDRV_ISC_Handler
 * -------------------------------------------------------------------------
 * Purpose  : This function will manipulate all of POEDRV via ISC
 * INPUT    : *key      -- key of ISC
 *            *mref_handle_p  -- transfer data
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : call by ISC_Agent
 * -------------------------------------------------------------------------
 */
#if (SYS_CPNT_STACKING == TRUE)
BOOL_T POEDRV_ISC_Handler(ISC_Key_T *key, L_MM_Mref_Handle_T *mref_handle_p);
#endif

#endif   /* POEDRV_H */

