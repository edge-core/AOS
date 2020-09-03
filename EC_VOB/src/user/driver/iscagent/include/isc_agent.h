/* Module Name: ISC_AGENT.H
 * Purpose:  
 *      This module provides two queues for those CSCs which don't have their own queue to 
 *      store packet sent from remote unit. The role of ISC_AGENT is in fact the "agent" of
 *      those ISC clients which mentioned above: register callback functions to ISC, queue 
 *      and dequeue ISC's packet before passing it to corresponding CSC.  
 * Notes: 
 * History:                                                               
 *    
 * Copyright(C)      Accton Corporation, 2005   				
 */

#ifndef _ISC_AGENT_H
#define _ISC_AGENT_H

/* INCLUDE FILE DECLARATIONS
 */
 
 /* NAMING CONSTANT DECLARATIONS
 */
 
 /* DATA TYPE DECLARATIONS
 */

/* EXPORTED FUNCTION SPECIFICATIONS
 */

/* FUNCTION NAME : ISC_AGENT_Init
 * PURPOSE: 
 *          ISC_AGENT_Init is called to allocate some resource for ISC_AGENT
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_Init(void);


 /* FUNCTION NAME : ISC_AGENT_Create_Tasks
 * PURPOSE: 
 *          ISC_AGENT_Create_Tasks is called to create tasks for ISC_AGENT
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_CreateTasks(void);


/* FUNCTION NAME : ISC_AGENT_EnterMasterMode
 * PURPOSE: 
 *          ISC_AGENT_EnterMasterMode is called to Enter Master Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_EnterMasterMode(void);


/* FUNCTION NAME : ISC_AGENT_EnterSlaveMode
 * PURPOSE: 
 *          ISC_AGENT_EnterSlaveMode is called to Enter Slave Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_EnterSlaveMode(void);


/* FUNCTION NAME : ISC_AGENT_EnterTransitionMode
 * PURPOSE: 
 *          ISC_AGENT_EnterTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */ 
void ISC_AGENT_EnterTransitionMode(void);


/* FUNCTION NAME : ISC_AGENT_SetTransitionMode
 * PURPOSE: 
 *          ISC_AGENT_SetTransitionMode is called to Enter Transition Mode
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_SetTransitionMode(void);


/* FUNCTION NAME : ISC_AGENT_Create_InterCSC_Relation
 * PURPOSE: 
 *          This function initializes all function pointer registration operations.
 * INPUT:   
 *          None
 * OUTPUT:  
 *          None
 * RETURN:  
 *          None
 * NOTES:
 *          None
 */
void ISC_AGENT_Create_InterCSC_Relation(void);

#endif
