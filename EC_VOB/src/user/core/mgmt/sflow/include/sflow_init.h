/* =====================================================================================*
 * FILE NAME: SFLOW_INIT.h                                                              *
 *                                                                                      *
 * ABSTRACT:  The two primary functions of this file is to Initialize sflow resouce     *
 *            information and to create Task.                                           *
 *                                                                                      *
 * MODIFICATION HISOTRY:                                                                *
 *                                                                                      *
 * MODIFIER        DATE        DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * Joeanne       10-25-2007    First Create                                             *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)        Accton Techonology Corporation 2007                              *
 * =====================================================================================*/

#include "sys_type.h"
 
/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            sflow module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_Initiate_System_Resources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - SFLOW_INIT_Create_InterCSC_Relation
 *---------------------------------------------------------------------------
 * PURPOSE: This function initializes all function pointer registration operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTE:    None.
 *---------------------------------------------------------------------------
 */
void SFLOW_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_Create_Tasks(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set sflow_mgr into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void SFLOW_INIT_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SFLOW_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  SFLOW_INIT_SetTransitionMode(void);
