/* =====================================================================================*
 * FILE	NAME: VLAN_INIT.c                                                               *
 *                                                                                      *
 * ABSTRACT:  The two primary functions	of this	file is	to Initialize vlan resouce      *
 *	          information and to create Task.									        *
 *                                                                                      *
 * MODIFICATION	HISOTRY:	                                                            *
 *                                                                                      *
 * MODIFIER		   DATE		   DESCRIPTION                                              *
 * -------------------------------------------------------------------------------------*
 * amytu		10-22-2001	   First Create     							            *
 *                                                                                      *
 * -------------------------------------------------------------------------------------*
 * Copyright(C)		   Accton Techonology Corporation 2001                              *
 * =====================================================================================*/

#ifndef VLAN_INIT_H
#define VLAN_INIT_H


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_InitiateSystemResources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            vlan module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_Create_InterCSC_Relation(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_Create_Tasks(void);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vlan_mgr into master mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vlan_mgr into slave mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vlan_mgr into transition mode.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void VLAN_INIT_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------
 */
void  VLAN_INIT_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_INIT_ProvisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell VLAN that provision is completed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:    This function is invoked by STKCTRL_TASK_ProvisionComplete().
 *-------------------------------------------------------------------------
 */
void  VLAN_INIT_ProvisionComplete(void);

/*-------------------------------------------------------------------------
 * ROUTINE NAME - VLAN_INIT_PreprovisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell VLAN that preprovision is completed
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:    This function is invoked by STKCTRL_TASK_PreProvisionComplete().
 *-------------------------------------------------------------------------
 */
void  VLAN_INIT_PreProvisionComplete(void);

#if (SYS_CPNT_STACKING == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_HandleHotInsertion
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
void VLAN_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_INIT_HandleHotRemoval
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
void VLAN_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);
#endif /* #if (SYS_CPNT_STACKING == TRUE) */

#endif
