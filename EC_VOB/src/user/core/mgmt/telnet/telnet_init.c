/*****************************************************************************
;
;    Project : Poppadom.prj
;    Creator : spchen
;    File    : TN_ROOT.C
;    Abstract: Telnet root
;
;Modification History:
;   Location    Resonder   Modification Description
; ------------ ---------- ----------------------------------------------
 *	2001.10.28, William, 
 *				1.  Adjust the include-files sequence, we had made declaration-check
 *					in socket.h, so socket.h must be included after psh.h, this file will
 *					include stdio.h, consults include VxWorks.h. In VxWorks.h, there also
 *					define some socket constants, structure,and type-def.
 *				2.	there is not priority definition in SYS_BLD.h, we use CLI's priority
 *					as telnet task's priority.
;
;*****************************************************************************/

/* INCLUDE FILE DECLARATIONS
 */


#include "sys_bld.h"
#include "sysfun.h"
#include "sys_adpt.h"
#include "tnpdcfg.h"
#include "tnshd.h"
#include "telnet_mgr.h"
#include "telnet_om.h"
#include "telnetd.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define	SYS_BLD_MAX_TELNET_SESSIONS		SYS_ADPT_MAX_TELNET_NUM


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM BODIES
 */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_INIT_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will create om sema id from call om init function
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : This init function will init from cli_proc.c (for telnet in cli process)
 *--------------------------------------------------------------------------*/ 
 BOOL_T TELNET_INIT_InitiateProcessResource(void)
{
    /* create OM sema id
     */
    return TELNET_OM_InitateProcessResource();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - TELNET_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void TELNET_INIT_Create_InterCSC_Relation(void)
{
    TELNET_MGR_Create_InterCSC_Relation();
} /* end of TELNET_INIT_Create_InterCSC_Relation */

/*	ROUTINE NAME : TELNET_INIT_Create_Tasks
 *	FUNCTION	 : Create Telnet Daemon and Telnet Shell two tasks.
 *	INPUT		 : None.
 *	OUTPUT		 : None.
 *	RETURN		 : None.
 *	NOTES		 : None.
 */ 
void	TELNET_INIT_Create_Tasks (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    unsigned long   rc;
    static tnpcfg_t tnpcfg;
    static pshcfg_t tnshcfg;             /* tnSH configuration table */
    /* BODY */
    /*----------------------------------------------------------------------*/
    /* Start the shell                                                      */
    /*----------------------------------------------------------------------*/
 	/*    tnshcfg.task_prio = 200; -- old value before modification by orlando 3/4/1999 */
    tnshcfg.task_prio = SYS_BLD_TELNET_SHELL_THREAD_PRIORITY;      /* Priority for shell task */
    for (rc = 0; rc < 3; rc++)
        tnshcfg.reserved[rc] = 0;

    if (TNSHD_CreateTask(&tnshcfg))
        TELNETD_ErrorPrintf("ERROR: tnSH startup failure\n");

    /*----------------------------------------------------------------------*/
    /* Start the Telnet server daemon                                       */
    /*----------------------------------------------------------------------*/
 	/*    tnpcfg.task_prio = 200; -- old value before mod. by orlando 3/4/1999 */
    tnpcfg.task_prio = SYS_BLD_TELNET_SHELL_THREAD_PRIORITY;       /* Priority for server task */
    tnpcfg.max_sessions = SYS_BLD_MAX_TELNET_SESSIONS;      /* Maximum of 4 concurrent sessions */
    tnpcfg.hlist = 0;             /* No access restrictions */
    tnpcfg.reserved[0] = 0;
    tnpcfg.reserved[1] = 0;
	
    if (tnpd_start(&tnpcfg))
	    TELNETD_ErrorPrintf("ERROR: Telnet server daemon startup failure\n");
}	/*	end of TELNET_INIT_Create_Tasks	*/


/* FUNCTION	NAME : TELNET_INIT_Set_Transition_Mode
 * PURPOSE:
 *		Set Transition Mode flag to prevent future request.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resource and reallocate.
 */
void	TELNET_INIT_SetTransitionMode (void)
{
    TELNET_MGR_SetTransitionMode();
    TNPD_SetTransitionMode();
}   /* end of TELNET_INIT_Set_Transition_Mode */


/* FUNCTION	NAME : TELNET_INIT_Enter_Transition_Mode
 * PURPOSE:
 *		Enter Transition Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Reset all modules, release all resource and reallocate.
 */
void	TELNET_INIT_EnterTransitionMode (void)
{
    TELNET_MGR_EnterTransitionMode();

    TNPD_EnterTransitionMode();
    TNSHD_EnterTransitionMode();

}    /* end of TELNET_INIT_Enter_Transition_Mode */


void    TELNET_INIT_ProvisionComplete(void)
{
    TNPD_ProvisionComplete();
    TNSHD_ProvisionComplete();
}

/* FUNCTION	NAME : TELNET_INIT_Enter_Master_Mode
 * PURPOSE:
 *		Enter Master Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		1. Do nothing.
 */
void	TELNET_INIT_EnterMasterMode (void)
{
    TELNET_MGR_EnterMasterMode();
}   /* end of TELNET_INIT_Enter_Master_Mode */


/* FUNCTION	NAME : TELNET_INIT_Enter_Slave_Mode
 * PURPOSE:
 *		Enter Slave	Mode.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *
 * NOTES:
 *		1. Do nothing.
 */
void	TELNET_INIT_EnterSlaveMode (void)
{
    TELNET_MGR_EnterSlaveMode();
}   /* end of TELNET_INIT_Enter_Slave_Mode */



/* FUNCTION NAME - TELNET_INIT_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void TELNET_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    TELNET_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}



/* FUNCTION NAME - TELNET_INIT_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void TELNET_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    TELNET_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}

