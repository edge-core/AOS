/* ------------------------------------------------------------------------
 *  FILE NAME  -  SNMP_TASK.H
 * ------------------------------------------------------------------------
 * Note: None
 * ------------------------------------------------------------------------
 *  Copyright(C) Accton Technology Corporation, 2000
 * ------------------------------------------------------------------------*/

#ifndef _SNMP_TASK_H
#define _SNMP_TASK_H
#include "netsnmp_port.h"
#include "l_threadgrp.h"
enum SNMP_TASK_LOG_FUN_E
{
    SNMP_TASK_LOG_FUN_CREATE_TASK = 0,
    SNMP_TASK_LOG_FUN_CREATE_SOCKET,
    SNMP_TASK_LOG_FUN_WAITEVENT,
    SNMP_TASK_LOG_FUN_OTHER
};

enum SNMP_TASK_LOG_ERR_E
{
    SNMP_TASK_LOG_ERR_CREATE_TASK = 0,
    SNMP_TASK_LOG_ERR_CREATE_SOCKET,
    SNMP_TASK_LOG_ERR_WAITEVENT,
    SNMP_TASK_LOG_ERR_OTHER
};

/*---------------------------------------------------------------------------
 * Routine Name : SNMP_TASK_Init()                                    
 *---------------------------------------------------------------------------
 * Function : Init SNMP task	                                
 * Input    : None								                             +
 * Output   :                                                                
 * Return   : never returns                                                  
 * Note     :                                                                
 *---------------------------------------------------------------------------*/
void SNMP_TASK_Init(void);

/*---------------------------------------------------------------------------
 * Routine Name : SNMP_TASK_CreateSnmpTask()                                    
 *---------------------------------------------------------------------------
 * Function : Create and start SNMP task	                                
 * Input    : None								                             +
 * Output   :                                                                
 * Return   : never returns                                                  
 * Note     :                                                                
 *---------------------------------------------------------------------------*/
void SNMP_TASK_CreateSnmpTask(void);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Socket_Init                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : initialize SNMP agent socket (create and bind socket ...)        
 *  INPUT    : None.                                                   
 *  OUTPUT   : None.                                                    
 *  RETURN   : None.
 *  NOTE     : None.                                                    
 * ------------------------------------------------------------------------*/
BOOL_T SNMP_TASK_Socket_Init(void);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Get_Socket_ID                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : Get the SNMP socket ID.      
 *  INPUT    : socket_id.                                                   
 *  OUTPUT   : socket_id.                                                    
 *  RETURN   : None.
 *  NOTE     : None.                                                    
 * ------------------------------------------------------------------------*/
BOOL_T  SNMP_TASK_Get_Socket_ID(I32_T *socket_id);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Set_Socket_ID                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : Set the SNMP socket ID variable.       
 *  INPUT    : s_id                                                   
 *  OUTPUT   : None.                                                    
 *  RETURN   : True/False.
 *  NOTE     : None.                                                    
 * ------------------------------------------------------------------------*/
BOOL_T SNMP_TASK_Set_Socket_ID(I32_T s_id);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME - SNMP_TASK_GetTaskId                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : Get the SNMP task ID.       
 *  INPUT    : None.                                                   
 *  OUTPUT   : id_p.                                                    
 *  RETURN   : TRUE/FALSE
 *  NOTE     : None.                                                    
 * ------------------------------------------------------------------------
 */
BOOL_T SNMP_TASK_GetTaskId(UI32_T *id_p);

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_EnterMasterMode                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP TASK enter master mode.       
 *  INPUT    : none.                                                   
 *  OUTPUT   : none.                                                    
 *  RETURN   : none.
 *  NOTE     : none.                                                    
 * ------------------------------------------------------------------------*/
void SNMP_TASK_EnterMasterMode();

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_EnterTransitionMode                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP TASK enter transition mode.       
 *  INPUT    : none.                                                   
 *  OUTPUT   : none.                                                    
 *  RETURN   : none.
 *  NOTE     : none.                                                    
 * ------------------------------------------------------------------------*/
void SNMP_TASK_EnterTransitionMode();

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_EnterSlaveMode                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP TASK enter slave mode.       
 *  INPUT    : none.                                                   
 *  OUTPUT   : none.                                                    
 *  RETURN   : none.
 *  NOTE     : none.                                                    
 * ------------------------------------------------------------------------*/
void SNMP_TASK_EnterSlaveMode();



/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_SetTransitionMode                                      
 * ------------------------------------------------------------------------
 *  FUNCTION : SNMP TASK Set Transition mode.       
 *  INPUT    : none.                                                   
 *  OUTPUT   : none.                                                    
 *  RETURN   : none.
 *  NOTE     : none.                                                    
 * ------------------------------------------------------------------------*/
void SNMP_TASK_SetTransitionMode();

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - SNMP_TASK_ProvisionComplete   							
 * ---------------------------------------------------------------------
 * PURPOSE: This function is for SNMP provision complete.
 *          																	
 * INPUT:  none               		
 * OUTPUT: none        				
 * RETURN: none;	
 * NOTES:1.none
 *       
 * ---------------------------------------------------------------------
 */
void SNMP_TASK_ProvisionComplete();

BOOL_T SNMP_TASK_Marked_Inform_Request_Socket( UI32_T inform_sock_num, BOOL_T flag);

I32_T SNMP_TASK_GetAvailableInformSocket();

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_Body
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_Body();

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  - SNMP_TASK_ThreadgrpExecutionRequest
 * ------------------------------------------------------------------------
 *  FUNCTION : THREADGRP Execution Request for snmp task
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_ThreadgrpExecutionRequest();

/* ------------------------------------------------------------------------
 *  ROUTINE NAME  -  THREADGRP Execution Release for snmp task
 * ------------------------------------------------------------------------
 *  FUNCTION : TBD
 *  INPUT    : None
 *  OUTPUT   : None.
 *  RETURN   : None
 *  NOTE     : TBD
 * ------------------------------------------------------------------------*/
void SNMP_TASK_ThreadgrpExecutionRelease();

#endif /* end of _SNMP_TASK_H */

/* end of SNMP_TASK.H */         
