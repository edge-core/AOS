/* Project Name: New Feature
 * File_Name : netaccess_task.h
 * Purpose     : NETACCESS initiation and NETACCESS task creation
 *
 * 2006/01/27    : Ricky Lin     Create this file
 *
 * Copyright(C)      Accton Corporation, 2001, 2002
 *
 * Note    : Designed for new platform (ACP_V3_Enhancement)
 */

#ifndef NETACCESS_TASK_H
#define NETACCESS_TASK_H

/* INCLUDE FILE DECLARATIONS */
#include "sys_type.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_NETACCESS == TRUE)

/* NAMING CONSTANT DECLARATIONS */
/* MACRO FUNCTION DECLARATIONS */
/* DATA TYPE DECLARATIONS */
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_CreateTask()
 *---------------------------------------------------------------------------
 * Function : Create and start NETACCESS task
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_TASK_CreateTask(void);

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_InitiateSystemResources
 *---------------------------------------------------------------------------
 * Function : Initialize NETACCESS's Task .
 * Input    : None
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *---------------------------------------------------------------------------
 */
BOOL_T NETACCESS_TASK_InitiateSystemResources(void);

/*---------------------------------------------------------------------------
 * Routine Name : NETACCESS_TASK_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * Function : This function initializes all function pointer registration operations.
 * Input    : None
 * Output   : None
 * Return   : None
 * Note     : None
 *---------------------------------------------------------------------------
 */
void NETACCESS_TASK_Create_InterCSC_Relation(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_SetTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE: Sending enter transition event to task calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void    NETACCESS_TASK_EnterTransitionMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Master mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_EnterMasterMode(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - NETACCESS_TASK_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE: Enter Slave mode calling by stkctrl.
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None
 * NOTES:  None
 * ---------------------------------------------------------------------
 */
void NETACCESS_TASK_EnterSlaveMode(void);

#endif /* #if (SYS_CPNT_NETACCESS == TRUE) */
#endif /*NETACCESS_TASK_H*/
