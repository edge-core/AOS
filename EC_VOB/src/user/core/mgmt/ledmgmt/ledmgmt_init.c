/* Module Name: LEDMGMT_INIT.C
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
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "led_mgr.h"
#include "ledmgmt_init.h"
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
void LEDMGMT_INIT_Initiate_System_Resources(void)
{
    LED_MGR_Init();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - LEDMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void LEDMGMT_INIT_Create_InterCSC_Relation(void)
{
    LED_MGR_Create_InterCSC_Relation();
} /* end of LEDMGMT_INIT_Create_InterCSC_Relation */

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
void LEDMGMT_INIT_Create_Tasks(void)
{
    /* Create a task for LED management
     */
    LED_MGR_CreateTask();
}    


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
void LEDMGMT_INIT_EnterMasterMode(void)
{
    LED_MGR_EnterMasterMode();
}

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
void LEDMGMT_INIT_EnterSlaveMode(void)
{
    LED_MGR_EnterSlaveMode();
}


/* FUNCTION NAME: LEDMGR_EnterTransitionMode
 * PURPOSE: Change the working environment to the environment after calling LEDMGR_Init().
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 */
void LEDMGMT_INIT_EnterTransitionMode(void)
{
    LED_MGR_EnterTransitionMode();
}

/* FUNCTION NAME: LEDMGR_EnterTransitionMode
 * PURPOSE: Change the working environment to the environment after calling LEDMGR_Init().
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  TRUE - 
 *          FALSE -
 * NOTES:
 */
void LEDMGMT_INIT_SetTransitionMode(void)
{
    return;
}

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
void LEDMGMT_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    LED_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
} /* End of LEDMGMT_INIT_HandleHotInsertion */

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
void LEDMGMT_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    LED_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
} /* End of LEDMGMT_INIT_HandleHotRemoval */
