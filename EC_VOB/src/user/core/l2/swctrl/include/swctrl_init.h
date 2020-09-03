/* Module Name: SWCTRL_INIT.H
 * Purpose: 
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the initialization function of switch control
 *         module.
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


#ifndef _SWCTRL_INIT_H_
#define _SWCTRL_INIT_H_




/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysrsc_mgr.h"


/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Initiate_System_Resources
 * -------------------------------------------------------------------------
 * FUNCTION: This function will init all the compoents which is located at
 *           directory of swctrl,
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Initiate_System_Resources(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Create_InterCSC_Relation
 * -------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Create_InterCSC_Relation(void);

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWCTRL_INIT_Create_Tasks
 * -------------------------------------------------------------------------
 * FUNCTION: This function will create all the compoent tasks which are located
 *           at directory of swctrl,
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_Create_Tasks(void);
    
/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_SetTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void SWCTRL_INIT_SetTransitionMode(void);
    

/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterTransitionMode
 * PURPOSE: enter transition state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void SWCTRL_INIT_EnterTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterMasterMode
 * PURPOSE: enter master state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void SWCTRL_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_EnterSlaveMode
 * PURPOSE: enter slave state
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:   
 */
void SWCTRL_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_HandleHotInsertion
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.

 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: SWCTRL_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void SWCTRL_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
#endif /* _SWCTRL_INIT_H_ */

