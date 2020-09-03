/* FILE NAME  -  TraceRoute_mgr.h
 *
 *  ABSTRACT :  This packet provides basic TraceRoute functionality.
 *                                                                         
 *  NOTES: This package shall be a reusable component of BNBU L2/L3/L4 switch 
 *         product lines. 
 *
 *
 * Modification History:                                        
 *   By            Date      Ver.    Modification Description                
 * ------------ ----------   -----   --------------------------------------- 
 *   Amytu       2003-07-01          Modify
 * ------------------------------------------------------------------------
 * Copyright(C)                   ACCTON Technology Corp. 2003      
 * ------------------------------------------------------------------------ 
 */
 
#ifndef TRACEROUTE_INIT_H
#define TRACEROUTE_INIT_H


/* FUNCTION NAME - TRACEROUTE_INIT_Initiate_System_Resources
 * PURPOSE  : This function allocates and initiates the system resource for
 *            TRACEROUTE module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void TRACEROUTE_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TRACEROUTE_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TRACEROUTE_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME - TRACEROUTE_INIT_Create_Tasks
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_Create_Tasks(void);


/* FUNCTION NAME - TRACEROUTE_INIT_EnterMasterMode
 * PURPOSE  : This function will call set TRACEROUTE into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterMasterMode(void);


/* FUNCTION NAME - TRACEROUTE_INIT_EnterSlaveMode
 * PURPOSE  : This function will call set TRACEROUTE into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterSlaveMode(void);


/* FUNCTION NAME - TRACEROUTE_INIT_EnterTransitionMode
 * PURPOSE  : This function will call set TRACEROUTE into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void TRACEROUTE_INIT_EnterTransitionMode(void);


/* FUNCTION NAME - TRACEROUTE_INIT_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_INIT_SetTransitionMode(void);

/* FUNCTION NAME - TRACEROUTE_INIT_ProvisionComplete
 * PURPOSE  : This function sets the component to temporary transition mode           
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 */
void  TRACEROUTE_INIT_ProvisionComplete(void);

#endif
