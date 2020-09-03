#ifndef TRK_INIT_H
#define TRK_INIT_H

#include "sys_type.h"
#include "sys_cpnt.h"

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION	NAME : TRK_INIT_InitiateSystemResources
 * PURPOSE:
 *		Initialize TRK client, server, and relay working resources.
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
void	TRK_INIT_InitiateSystemResources(void);

/* FUNCTION	NAME : TRK_INIT_Create_InterCSC_Relation
 * PURPOSE:
 *		This function initializes all function pointer registration operations.
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
void TRK_INIT_Create_InterCSC_Relation(void);

/* FUNCTION	NAME : TRK_INIT_EnterMasterMode
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
void TRK_INIT_EnterMasterMode(void);

/* FUNCTION	NAME : TRK_INIT_EnterTransitionMode
 * PURPOSE:
 *		This call will set dhcp_mgr and dhcp_task into transition mode
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
void TRK_INIT_EnterTransitionMode(void);



/* FUNCTION	NAME : TRK_INIT_SetTransitionMode
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
void TRK_INIT_SetTransitionMode(void);

/* FUNCTION	NAME : TRK_INIT_EnterSlaveMode
 * PURPOSE:
 *		This call will set TRK into slave mode.
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
void TRK_INIT_EnterSlaveMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME: TRK_INIT_HandleHotInsertion
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

 * -------------------------------------------------------------------------*/
void TRK_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default);


/*-------------------------------------------------------------------------
 * FUNCTION NAME: TRK_INIT_HandleHotRemoval
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * -------------------------------------------------------------------------*/
void TRK_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port);

#endif
