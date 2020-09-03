/* Module Name: SWCTRL_TASK.H
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the task level functions of switch
 *         control.
 *        ( 2.  The domain MUST be handled by this module.      )
 *        ( 3.  The domain would not be handled by this module. )
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *       2001/6/1    Jimmy Lin   Create this file
 *
 * Copyright(C)      Accton Corporation, 1999, 2000
 */


#ifndef _SWCTRL_TASK_H_
#define _SWCTRL_TASK_H_




/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_adpt.h"
#include "swdrv_type.h"




/* NAMING CONSTANT DECLARATIONS
 */
#define SYS_ADPT_MAX_NBR_OF_ANALYZER_PORT       1
#define SYS_ADPT_MAX_NBR_OF_MONITORED_PORT      4
#define SYS_ADPT_MAX_NBR_OF_MONITOR_SESSION     256

#define SWCTRL_TASK_Create_Task_FunNo           0
#define SWCTRL_TASK_Create_Task_ErrNo           0

/* TYPE DECLARATIONS
 */



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_Init
 * -------------------------------------------------------------------------
 * FUNCTION: This function will init all the resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_Init(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_Task_CreateTask
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create Switch Control Task. This function
 *           will be called by root.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_Task_CreateTask(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_Main
 * -------------------------------------------------------------------------
 * FUNCTION: Switch control main task routine
 * INPUT   : arg -- null argument
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_Main(int arg);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: this will set the trasition mode, wait until the flag is set TRUE
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_SetTransitionMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterTransitionMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterMasterMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void SWCTRL_TASK_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortLinkUp_CallBack(UI32_T unit,
                                            UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortLinkDown_CallBack(  UI32_T unit,
                                                UI32_T port);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_CraftPortLinkUp_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link up callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_CraftPortLinkUp_CallBack(UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_CraftPortLinkDown_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_CraftPortLinkDown_CallBack(  UI32_T unit);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortTypeChanged_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port link down callback function, register to swdrv
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           module_id -- nonzero for module inserted event
 *           port_type -- port type to set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortTypeChanged_CallBack( UI32_T unit,
                                                  UI32_T port,
                                                  UI32_T module_id,
                                                  UI32_T port_type);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSpeedDuplex_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit         -- in which unit
 *           port         -- which port
 *           speed_duplex -- the speed/duplex status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortSpeedDuplex_CallBack(UI32_T unit,
                                                 UI32_T port,
                                                 UI32_T speed_duplex);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortFlowCtrl_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port speed/duplex callback function, register to swdrv
 * INPUT   : unit      -- in which unit
 *           port      -- which port
 *           flow_ctrl -- the flow control status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_TASK_PortFlowCtrl_CallBack(UI32_T unit,
                                              UI32_T port,
                                              UI32_T flow_ctrl);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpPresent_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit      -- in which unit
 *           port      -- which port
 *           is_present -- the sfp present status
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpPresent_CallBack(UI32_T unit,
                                          UI32_T port,
                                          BOOL_T is_present);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpInfo_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_info_p -- sfp eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpInfo_CallBack(UI32_T unit, UI32_T sfp_index,
                                       SWDRV_TYPE_SfpInfo_T *sfp_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpDdmInfo_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp present callback function, register to swdrv
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_ddm_info_p -- sfp DDM eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpDdmInfo_CallBack(UI32_T unit, UI32_T sfp_index,
                                          SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack
 * -------------------------------------------------------------------------
 * FUNCTION: Port sfp ddm measured inf callback function, register to swdrv
 * INPUT   : unit        -- in which unit
 *           sfp_index  -- which sfp_index
 *           sfp_ddm_info_measured_p -- sfp DDM measured eeprom info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
void SWCTRL_TASK_PortSfpDdmInfoMeasured_CallBack(UI32_T unit,
                                          UI32_T sfp_index,
                                          SWDRV_TYPE_SfpDdmInfoMeasured_T *sfp_ddm_info_measured_p);

#endif /* _SWCTRL_TASK_H_ */

