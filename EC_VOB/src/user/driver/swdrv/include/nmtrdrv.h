/*--------------------------------------------------------------------------+ 
 * FILE NAME - nmtrdrv.h                                                 +
 * -------------------------------------------------------------------------+
 * ABSTRACT: This file defines Network Monitor driver APIs                  +
 *                                                                          +
 * -------------------------------------------------------------------------+
 *                                                                          +
 * Modification History:                                                    +
 *   By              Date     Ver.   Modification Description               +
 *   --------------- -------- -----  ---------------------------------------+
 *   Arthur        07/23/2001        New Creation                           +
 * -------------------------------------------------------------------------+
 * Copyright(C)                              ACCTON Technology Corp., 2001  +
 * ------------------------------------------------------------------------*/

#ifndef NMTRDRV_H
#define NMTRDRV_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"   
#include "swdrv_type.h"
#include "sysrsc_mgr.h"

/* DATA TYPE DECLARATIONS
 */
enum{
    NMTRDRV_UPDATE_IFTABLE_STATS = 0,
    NMTRDRV_UPDATE_IFXTABLE_STATS,
    NMTRDRV_UPDATE_RMON_STATS,
    NMTRDRV_UPDATE_ETHERLIKE_STATS,
    NMTRDRV_UPDATE_ETHERLIKE_PAUSE_STATS,
    NMTRDRV_UPDATE_IFPERQ_STATS,                    /* SYS_CPNT_NMTR_PERQ_COUNTER */
    NMTRDRV_UPDATE_PFC_STATS,                       /* SYS_CPNT_PFC */
    NMTRDRV_UPDATE_QCN_STATS,                       /* SYS_CPNT_CN */
    NMTRDRV_UPDATE_300S_UTILIZATION,
    NMTRDRV_UPDATE_IFXTABLE_STATS_FOR_VLAN,         /* SYS_CPNT_NMTR_VLAN_COUNTER */
} NMTRDRV_UPDATE;

/* NAMING CONSTANT
 */
 
/* EXPORTED SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------
 * ROUTINE NAME - NMTRDRV_TASK_CreateTask                                        
 *------------------------------------------------------------------------
 * FUNCTION: This function will create address management task            
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTRDRV_TASK_CreateTask(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_InitiateSystemResources                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function will initialize resources               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None
 * NOTE    : invoked by NMTR_Init()                                                         
 *------------------------------------------------------------------------*/
void NMTRDRV_InitiateSystemResources(void);

void NMTRDRV_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

void NMTRDRV_AttachSystemResources(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_Create_InterCSC_Relation                                               
 *------------------------------------------------------------------------|
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void NMTRDRV_Create_InterCSC_Relation(void);

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initialize data variables for transition mode operation
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTRDRV_SetTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterMasterMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initial data variables for master mode operation
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterMasterMode(void);

/*------------------------------------------------------------------------------
 * Function : NMTRDRV_EnterSlaveMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function initial data variables for slave mode operation
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void NMTRDRV_EnterSlaveMode(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - NMTRDRV_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the Net monitor module to start
 *           action
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void NMTRDRV_ProvisionComplete(void);

/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearPortCounter                      
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the port conuter                 
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T port    -   port number                                                  
 * OUTPUT  : None                                                  
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing the port counter after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearPortCounter(UI32_T unit, UI32_T port);


#if (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE)
/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearVlanCounter
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the vlan conuter
 * INPUT   : UI32_T unit    -   unit number
 *           UI32_T vid     -   vid
 * OUTPUT  : None
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : None
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearVlanCounter(UI32_T unit, UI32_T vid);
#endif /* (SYS_CPNT_NMTR_VLAN_COUNTER == TRUE) */


/*------------------------------------------------------------------------|
 * ROUTINE NAME - NMTRDRV_ClearAllCounters
 *------------------------------------------------------------------------|
 * FUNCTION: This function will clear the all counters in whole system
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : Boolean    - TRUE : success , FALSE: failed
 * NOTE    : For clearing all the counters after system start up
 *------------------------------------------------------------------------*/
BOOL_T NMTRDRV_ClearAllCounters(void);

#endif  /* NMTRDRV_H */
