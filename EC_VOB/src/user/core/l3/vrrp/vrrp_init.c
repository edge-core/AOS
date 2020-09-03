/* =====================================================================================*
 * FILE  NAME: VRRP_INIT.c                                                           
 *                                                                                  
 * ABSTRACT:  The two primary functions	of this	file is	to initialize VRRP resouce 
 *  information and to create Task.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */
 
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sysfun.h"
#include "vrrp_task.h"
#include "vrrp_init.h"
#include "vrrp_mgr.h"
#include "syslog_type.h"
#include "syslog_mgr.h"
#include "syslog_task.h"
#include "sys_module.h"

//static BOOL_T  vrrpInitErrorLogFlag[8];

/* EXPORTED SUBPROGRAM BODIES
 */
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterMasterMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into master mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterMasterMode(void)
{
    VRRP_MGR_EnterMasterMode();
    return;
} /* end of VRRP_INIT_EnterMasterMode() */


/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterSlaveMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into slave mode. 
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterSlaveMode(void)
{
    VRRP_MGR_EnterSlaveMode();
    return;
} /* end of VRRP_INIT_EnterSlaveMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_EnterTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr into transition mode.  
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_EnterTransitionMode(void)
{
    VRRP_MGR_EnterTransitionMode();
    return;
} /* end of VRRP_INIT_EnterMasterMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will call set vrrp_mgr to set transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_SetTransitionMode(void)
{    
    VRRP_MGR_SetTransitionMode();
    return;
}   /* End of VRRP_INIT_SetTransitionMode */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_INIT_ProvisionComplete
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will process necessary procedures when provision completes
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void VRRP_INIT_ProvisionComplete(void)
{
    return;
} /* VRRP_INIT_ProvisionComplete() */


