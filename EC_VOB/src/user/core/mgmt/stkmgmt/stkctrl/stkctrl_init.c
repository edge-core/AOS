/* Module Name: STKCTRL_INIT.C
 * Purpose: This file contains the information of stack control:
 *          1. Initialize resource.
 *          2. Create task.
 * Notes: 
 * History:                                                               
 *    07/03/01       -- Aaron Chuang, Create
 *    09/20/01       -- Seperate the old file STR_CTRL.C to STKCTRL_INIT.C and STKCTRL_TASK.C
 *    07/10/2007     -- Charlie Chen, port to linux platform
 *
 * Copyright(C)      Accton Corporation, 1999 - 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "stkctrl_task.h"

/* NAMING CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS 
 */


/* STATIC VARIABLE DECLARATIONS 
 */


/* MACRO FUNCTIONS DECLARACTION
 */


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: STKCTRL_INIT_InitiateProcessResources
 * PURPOSE: This function is used to initialize the stack control module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE    -- successful.
 *          FALSE   -- unspecified failure, system error.
 * NOTES:
 *
 */
BOOL_T STKCTRL_INIT_InitiateProcessResources(void)
{
    BOOL_T  ret;
    ret = STKCTRL_TASK_InitiateProcessResources();
    return ret;
} /* End of STKCTRL_INIT_Initiate_System_Resources */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - STKCTRL_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void STKCTRL_INIT_Create_InterCSC_Relation(void)
{
    STKCTRL_TASK_Create_InterCSC_Relation();
} /* end of STKCTRL_INIT_Create_InterCSC_Relation */

