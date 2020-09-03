/* Module Name:	_DHCP_INIT.H
 * Purpose:
 *		This module provides initialized function. Because DHCP is running in root stage,
 *		different from BootP, so we keep init. module in DHCP.
 *
 * Notes:
 *		None.
 *
 * History:
 *		 Date		-- Modifier,  Reason
 *	0.1	2001.12.25	--	William, Created
 *
 * Copyright(C)		 Accton	Corporation, 1999, 2000, 2001.
 */



#ifndef		_DHCP_INIT_H
#define		_DHCP_INIT_H


/* INCLUDE FILE	DECLARATIONS
 */
#include "sys_type.h"


/* NAME	CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA	TYPE DECLARATIONS
 */


/* EXPORTED	SUBPROGRAM SPECIFICATIONS
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
void	DHCP_INIT_InitiateSystemResources (void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DHCP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void DHCP_INIT_Create_InterCSC_Relation(void);

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
void	DHCP_INIT_Create_Tasks (void);

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
void DHCP_INIT_EnterMasterMode(void);

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
void DHCP_INIT_EnterTransitionMode(void);

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
void DHCP_INIT_SetTransitionMode(void);


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
void DHCP_INIT_EnterSlaveMode(void);

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
void DHCP_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);

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
void DHCP_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif	 /*	_DHCP_INIT_H */
