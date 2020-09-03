/*-----------------------------------------------------------------------------
 * Module Name: lldp_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 */
#ifndef LLDP_INIT_H
#define LLDP_INIT_H

#include "sys_type.h"

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_InitiateSystemResources
 * ------------------------------------------------------------------------
 * FUNCTION : Initiate resources.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_InitiateSystemResources(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_Create_InterCSC_Relation()
 *-------------------------------------------------------------------------
 * FUNCTION: Register other CSC
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * Note    : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_Create_InterCSC_Relation(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_CreateTask
 * ------------------------------------------------------------------------
 * FUNCTION : Create LLDP tasks.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_CreateTasks(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_SetTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Set transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_SetTransitionMode(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterTransitionMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter transition mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterTransitionMode(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterSlaveMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter slave mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterSlaveMode(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_EnterMasterMode
 * ------------------------------------------------------------------------
 * FUNCTION : Enter master mode.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_EnterMasterMode(void) ;

/* ------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_ProvisionComplete
 * ------------------------------------------------------------------------
 * FUNCTION : provision complete.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
void LLDP_INIT_ProvisionComplete(void) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_HandleHotInsert
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_HandleHotInsertion(UI32_T beg_ifindex, UI32_T end_of_index, BOOL_T use_default) ;

/*-------------------------------------------------------------------------
 * FUNCTION NAME - LLDP_INIT_HandleHotRemoval
 *-------------------------------------------------------------------------
 * PURPOSE  : Handle the module hot insert.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-------------------------------------------------------------------------
 */
void LLDP_INIT_HandleHotRemoval(UI32_T beg_ifindex, UI32_T number_of_port) ;

#endif /* End of LLDP_INIT_H */
