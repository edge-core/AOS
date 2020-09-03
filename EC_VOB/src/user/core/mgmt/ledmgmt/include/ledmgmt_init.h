/* Module Name: LEDMGMT_INIT.H
 * Purpose: 
 *  This module is in first thing needed to do of LED Management.  
 *  For LEDMGMT, there are two sub component 1) LEDMGMT and 2) LEDDRV.
 *  The LEDMGMT will have a LEDMGR task but LEDDRV is a just component.
 *
 * Notes: 
 *
 * History:                                                               
 *          Date      -- Modifier,        Reason
 * ------------------------------------------------------------------------
 *  V1.0  2001.10.18  -- Jason Hsue,      Creation
 *        2007.08.01  -- Echo Chen,       Modified for linux platform
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001 , 2007
 */

#ifndef LEDMGMT_INIT_H
#define LEDMGMT_INIT_H

#include "sys_cpnt.h"

/* FUNCTION NAME: LEDMGMT_INIT_Inititate_System_Resources
 *----------------------------------------------------------------------------------
 * PURPOSE: init LEDMGMT component
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDMGMT_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LEDMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LEDMGMT_INIT_Create_InterCSC_Relation(void);

/* FUNCTION NAME: LEDMGMT_INIT_Create_Tasks
 *----------------------------------------------------------------------------------
 * PURPOSE: create LEDMGR task 
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void LEDMGMT_INIT_Create_Tasks(void);

/* FUNCTION NAME: LEDMGR_Init
 * PURPOSE: Call LEDDRV_Init() to prepare display buffer space.
 *          register callback functions to SWCTRL, uport_linkup/linkdown...
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - if initialization sucessful
 *          FALSE - if initialization fail 
 * NOTES:
 *      The function will initialize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* at system startup.
 */
void LEDMGR_Init(void);

/* FUNCTION NAME: LEDMGMT_INIT_EnterMasterMode
 * PURPOSE: Call LEDMGMT_INIT_EnterMasterMode() make LEDMGMT to enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      The function will initialize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* during stacking.
 *      This function SHOULD NOT be use in conjunction with
 *      LEDMGR_EnterSlaveMode().
 */
void LEDMGMT_INIT_EnterMasterMode(void);

/* FUNCTION NAME: LEDMGMT_INIT_EnterSlaveMode
 * PURPOSE: Call LEDMGMT_INIT_EnterSlaveMode() make LEDMGMT to enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 *      The function will initialize lower level driver and set local variables
 *      to default state. It should be called *ONCE ONLY* during stacking.
 *      This function SHOULD NOT be use in conjunction with
 *      LEDMGR_EnterMasterMode().
 */
void LEDMGMT_INIT_EnterSlaveMode(void);


/* FUNCTION NAME: LEDMGR_EnterTransitionMode
 * PURPOSE: Change the working environment to the environment after calling LEDMGR_Init().
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:
 */
void LEDMGMT_INIT_EnterTransitionMode(void);

/* FUNCTION NAME: LEDMGR_SetTransitionMode
 * PURPOSE: Change the working environment to the environment after calling LEDMGR_Init().
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:
 */
void LEDMGMT_INIT_SetTransitionMode(void);

/* FUNCTION NAME: LEDMGMT_INIT_SetTransitionMode
 * PURPOSE: Tell LEDMGR to enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:
 */
void LEDMGR_CreateTask(void);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LEDMGMT_INIT_HandleHotInsertion
 * ------------------------------------------------------------------------
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
 * ------------------------------------------------------------------------
 */
void LEDMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LEDMGMT_INIT_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void LEDMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif
