/* Module Name: DHCP_INIT.C
 * Purpose:
 *       *		This module provides initialized function. Because DHCP is running in root stage,
 *		different from BootP, so we keep init. module in DHCP.
 *
 * Notes:
 *      .
 *
 * History:
 *       Date       --  Modifier,  Reason
 *  0.1 2001.12.26  --  William, Created
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef UNIT_TEST_DHCP
#include "unit_test_dhcp.h"
#endif

#ifndef UNIT_TEST_DHCP
#include "dhcp_txrx.h"
#include "dhcp_time.h"
#endif

//#include "dhcp_task.h"
#include "dhcp_algo.h"

#include "dhcp_wa.h"
#include "dhcp_om.h"
#include "dhcp_init.h"
#include "dhcp_mgr.h"
#include "dhcp_backdoor.h"



/* NAMING CONSTANT DECLARATIONS
 */


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

/* FUNCTION	NAME : DHCP_INIT_InitiateSystemResources
 * PURPOSE:
 *		Initialize DHCP client, server, and relay working resources.
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
 *		None.
 */
void	DHCP_INIT_InitiateSystemResources (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
    DHCP_OM_Init();
    DHCP_WA_Init();
#ifndef UNIT_TEST_DHCP
    DHCP_TIME_Init();
    DHCP_TXRX_Init();
    DHCP_BACKDOOR_Init();
#endif
    DHCP_ALGO_Init();
    DHCP_MGR_Init();
//    DHCP_TASK_Init();

}	/* end of DHCP_INIT_InitiateSystemResources */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DHCP_INIT_Create_InterCSC_Relation(void)
{
    DHCP_BACKDOOR_Create_InterCSC_Relation();
} /* end of DHCP_INIT_Create_InterCSC_Relation */

/* FUNCTION	NAME : DHCP_INIT_Create_Tasks
 * PURPOSE:
 *		Create DHCP client, server, and relay tasks if required.
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
 *		None.
 */
void	DHCP_INIT_Create_Tasks (void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY */
//    DHCP_TASK_CreateTask();
}	/*	end of DHCP_INIT_Create_Tasks	*/

/* FUNCTION	NAME : DHCP_INIT_EnterMasterMode
 * PURPOSE:
 *		This call will set dhcp_mgr into master mode
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
 *		None.
 */
void DHCP_INIT_EnterMasterMode(void)
{
	DHCP_MGR_EnterMasterMode();
}

/* FUNCTION	NAME : DHCP_INIT_EnterTransitionMode
 * PURPOSE:
 *		This call will set dhcp_mgr into transition mode
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
 *		None.
 */
void DHCP_INIT_EnterTransitionMode(void)
{
	DHCP_MGR_EnterTransitionMode();
//	DHCP_TASK_EnterTransitionMode();
}

/* FUNCTION	NAME : DHCP_INIT_SetTransitionMode
 * PURPOSE:
 *		This call will set dhcp_mgr into transition mode to prevent
 *		calling request.
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
 *		None.
 */
void DHCP_INIT_SetTransitionMode(void)
{
	DHCP_MGR_SetTransitionMode();
//	DHCP_TASK_SetTransitionMode();
}
/* FUNCTION	NAME : DHCP_INIT_EnterSlaveMode
 * PURPOSE:
 *		This call will set DHCP into slave mode.
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
 *		None.
 */
void DHCP_INIT_EnterSlaveMode(void)
{
	DHCP_MGR_EnterSlaveMode();
}

/* suger, 12-31-2003, add dummy function for hot swap */
/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_INIT_HandleHotInsertion
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
void DHCP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* do nothing */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_INIT_HandleHotRemoval
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
void DHCP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* do nothing */
}

/*===========================
 * LOCAL SUBPROGRAM BODIES
 *===========================
 */




