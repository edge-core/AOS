/* =====================================================================================*
  * FILE  NAME: VRRP_INIT.h                                                          
  *                                                                                  
  * ABSTRACT:  The two primary functions of this file is to initialize VRRP resouce 
  *  information and to create Task.
  *
  * NOTES:
  *
  * HISTORY
  *    3/28/2009 - Donny.Li     , Created
  *
  * Copyright(C)      Accton Corporation, 2009

 * =====================================================================================*/
 
#ifndef __VRRP_INIT_H
#define __VRRP_INIT_H


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_Initiate_System_Resources
 *--------------------------------------------------------------------------
 * PURPOSE  : This function allocates and initiates the system resource for
 *            VRRP module.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_Initiate_System_Resources(void);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_Create_Tasks
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create task. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_Create_Tasks(void *arg);


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into transition mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr to set transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process necessary procedures when provision completes
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_ProvisionComplete(void);

#endif  /* end of _VRRP_INIT_H_ */
