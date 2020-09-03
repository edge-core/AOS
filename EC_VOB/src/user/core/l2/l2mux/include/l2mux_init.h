/*-----------------------------------------------------------------------------
 * Module Name: L2MUX_INIT.H   										 		   
 *-----------------------------------------------------------------------------
 * PURPOSE: A header file for the definition of Data Structure of l2mux_init.c		                                     	   
 *-----------------------------------------------------------------------------
 * NOTES: 
 *
 * HISTORY:																	   
 *    10/21/2002 - Benson Hsu, Created	
 * 						   
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002 		   
 *-----------------------------------------------------------------------------
 */

#ifndef	L2MUX_INIT_H
#define	L2MUX_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* FUNCTION NAME: L2MUX_INIT_InitiateProcessResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_InitiateProcessResources(void);

/* FUNCTION NAME: L2MUX_INIT_InitiateSystemResources
 *----------------------------------------------------------------------------------
 * PURPOSE: Initialization of L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_InitiateSystemResources(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: L2MUX_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for L2MUX in the context of the calling process.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void L2MUX_INIT_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - L2MUX_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void L2MUX_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME: L2MUX_INIT_SetTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will set L2MUX to transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_SetTransitionMode(void);


/* FUNCTION NAME: L2MUX_INIT_EnterTransitionMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter transition mode
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterTransitionMode(void);


/* FUNCTION NAME: L2MUX_INIT_EnterMasterMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter master mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterMasterMode(void);


/* FUNCTION NAME: L2MUX_INIT_EnterSlaveMode
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will force L2MUX to enter slave mode	
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void L2MUX_INIT_EnterSlaveMode(void);


/* FUNCTION NAME: L2MUX_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: This function will create a task for L2MUX
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   There is no task on L2MUX.
 */
void L2MUX_INIT_Create_Tasks(void);


#endif /* L2MUX_INIT_H */