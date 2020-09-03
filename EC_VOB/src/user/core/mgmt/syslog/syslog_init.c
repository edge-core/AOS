/* Module Name: SYSLOG_INIT.C
 * Purpose: Initialize the resources for the system log module.
 *
 * Notes:
 *
 * History:
 *    10/17/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "stdio.h"
#include "sys_type.h"
#include "sys_adpt.h"
#include "syslog_task.h"
#include "syslog_mgr.h"


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


/* FUNCTION NAME: SYSLOG_INIT_Initiate_System_Resources
 * PURPOSE: This function is used to initialize the system log module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the system log module.
 *
 */
void SYSLOG_INIT_InitiateSystemResources(void)
{
    if (!SYSLOG_MGR_Initiate_System_Resources())
    {
        perror ("\r\nSYSLOG_MGR_Initiate_System_Resources Error\n");
        while (TRUE);
    }

    if (!SYSLOG_TASK_Initiate_System_Resources())
    {
        perror ("\r\nSYSLOG_TASK_Initiate_System_Resources Error\n");
        while (TRUE);
    }


    return;
} /* End of SYSLOG_INIT_Initiate_System_Resources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SYSLOG_INIT_Create_InterCSC_Relation(void)
{
    SYSLOG_MGR_Create_InterCSC_Relation();
    SYSLOG_TASK_Create_InterCSC_Relation();
} /* end of SYSLOG_INIT_Create_InterCSC_Relation */

/* FUNCTION NAME: SYSLOG_INIT_Create_Tasks
 * PURPOSE: This function is used to creat the system log task.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Create the system log task.
 *
 */
void SYSLOG_INIT_Create_Tasks(void)
{
    if (!SYSLOG_TASK_Create_Tasks())
    {
        perror ("\r\nSYSLOG_TASK_Create_Tasks Error\n");
        while (TRUE);
    }

    return;
} /* End of SYSLOG_INIT_Create_Tasks() */


/* FUNCTION NAME: SYSLOG_INIT_EnterTransitionMode
 * PURPOSE: The function enter transition mode and free all SYSLOG resources
 *          and reset database to factory default.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:   1. The function MUST be called before calling the
 *             SYSLOG_INIT_EnterMasterMode.
 */
BOOL_T SYSLOG_INIT_EnterTransitionMode (void)
{
    BOOL_T ret;

    ret = SYSLOG_MGR_EnterTransitionMode();

    if (ret == FALSE)
        return ret;

    ret = SYSLOG_TASK_EnterTransitionMode();
    return ret;
} /* End of SYSLOG_INIT_EnterTransitionMode */


/* FUNCTION NAME: SYSLOG_INIT_EnterMasterMode
 * PURPOSE: The function enter master mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_EnterMasterMode(void)
{
    BOOL_T ret;

    ret = SYSLOG_MGR_EnterMasterMode();

    if (ret == FALSE)
        return ret;

    ret = SYSLOG_TASK_EnterMasterMode();
    return ret;
} /* End of SYSLOG_INIT_EnterMasterMode */


/* FUNCTION NAME: SYSLOG_INIT_EnterSlaveMode
 * PURPOSE: The function enter slave mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_EnterSlaveMode(void)
{
    BOOL_T ret;

    ret = SYSLOG_MGR_EnterSlaveMode();

    if (ret == FALSE)
        return ret;

    ret = SYSLOG_TASK_EnterSlaveMode();
    return ret;
} /* End of SYSLOG_INIT_EnterSlaveMode */


/* FUNCTION NAME: SYSLOG_INIT_SetTransitionMode
 * PURPOSE: The function set transition mode.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  TRUE  -- success
 *          FALSE -- failure
 * NOTES:
 */
BOOL_T SYSLOG_INIT_SetTransitionMode (void)
{
    BOOL_T ret;

    ret = SYSLOG_MGR_SetTransitionMode();

    if (ret == FALSE)
        return ret;

    ret = SYSLOG_TASK_SetTransitionMode();
    return ret;
} /* End of SYSLOG_INIT_SetTransitionMode */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_HandleHotInsertion
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
void SYSLOG_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    SYSLOG_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
} /* End of SYSLOG_INIT_HandleHotInsertion */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - SYSLOG_INIT_HandleHotRemoval
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
void SYSLOG_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    SYSLOG_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
} /* End of SYSLOG_INIT_HandleHotRemoval */
