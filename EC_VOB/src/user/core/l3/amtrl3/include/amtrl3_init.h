/*-----------------------------------------------------------------------------
 * Module Name: amtrl3_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *
 *   By              Date       Ver.   Modification Description
 *   --------------- ---------- -----  ---------------------------------------
 *   Ted             03/16/2002         Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */
 
#ifndef AMTRL3_INIT_H
#define AMTRL3_INIT_H

/* INCLUDE FILE DECLARATIONS
 */


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_Initiate_System_Resources
 * PURPOSE: init AMTR data
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_Initiate_System_Resources(void);


/* -------------------------------------------------------------------------
 * ROUTINE NAME - AMTRL3_INIT_ProvisionComplete
 * -------------------------------------------------------------------------
 * FUNCTION: This function will tell the AMTRL3 to start action.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void AMTRL3_INIT_ProvisionComplete(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_Create_Tasks
 * PURPOSE: create AMTR task
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_Create_Tasks(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterTransitionMode(void);

/*------------------------------------------------------------------------------
 * Function : AMTRL3_INIT_SetTransitionMode()
 *------------------------------------------------------------------------------
 * Purpose  : This function will set the operation mode to transition mode
 * INPUT    : None
 * OUTPUT   : None                                                           
 * RETURN   : None
 * NOTE     : 
 *-----------------------------------------------------------------------------*/
void AMTRL3_INIT_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_HandleHotInsertion
 * PURPOSE: Hot swap init function for insertion
 * INPUT:   starting_port_ifindex
 *          number_of_port
 *          use_default
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_HandleHotInsertion(UI32_T starting_port_ifindex,
                                    UI32_T number_of_port,
                                    BOOL_T use_default);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: AMTRL3_INIT_HandleHotRemoval
 * PURPOSE: Hot swap init function for removal
 * INPUT:   starting_port_ifindex
 *          number_of_port
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void AMTRL3_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif /* End of AMTRL3_INIT_H */
