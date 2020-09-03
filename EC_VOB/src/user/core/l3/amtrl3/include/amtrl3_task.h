/*
 * MODULE NAME: amtrl3_task.h
 *
 * PURPOSE: 
 *          The definition for address monitor task.   
 *
 * NOTES:	API List #
 *			-------------------------------------------------------------------
 *		    AMTRL3_TASK_Init()
 *          AMTRL3_TASK_CreateTask()
 *          AMTRL3_TASK_EnterTransitionMode()
 *          AMTRL3_TASK_EnterMasterMode()
 *          AMTRL3_TASK_EnterSlaveMode()
 *
 * HISTORY:
 *		Date		--	Modifier,	Reason
 *		-----------------------------------------------------------------------
 *		04-28-2003	--	hyliao,	     Design change and merge with original IPLRN
 *
 * COPYRIGHT(C)			Accton Corporation, 2002
 */
 
#ifndef AMTRL3_TASK_H
#define AMTRL3_TASK_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* TYPE DEFINITIONS  
 */
 
/* NAMING CONSTANT
 */

/* EXPORTED SUBPROGRAM DECLARACTION */

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_Init                                               
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources               
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : Invoked by root.c()                                        
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_Init(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_CreateTask                                        
 *------------------------------------------------------------------------
 * FUNCTION: This function will create address management task            
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                     
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_CreateTask(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterTransitionMode                      
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource     
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                        
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterTransitionMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set transition state flag
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_SetTransitionMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterMasterMode                      
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services     
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                        
 * NOTE    : None
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterMasterMode(void);


/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_EnterSlaveMode                                   
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services          
 * INPUT   : None                                                         
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_EnterSlaveMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_TASK_MacDeleteByLifeTimeCallbackHandler
 *------------------------------------------------------------------------
 * FUNCTION: Handler function of MAC address delete by life time callback
 * INPUT   : life_time --- which kind of life time
 * OUTPUT  : None                                                         
 * RETURN  : None                                                         
 * NOTE    : None                                                         
 *------------------------------------------------------------------------*/
void AMTRL3_TASK_MacDeleteByLifeTimeCallbackHandler(UI32_T life_time);


#endif /* End of #ifndef AMTRL3_TASK_H */
