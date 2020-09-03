
#ifndef		_DHCPSNP_INIT_H
#define		_DHCPSNP_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAME CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME : DHCPSNP_INIT_Initiate_System_Resources
 * PURPOSE:
 *        Initialize DHCPSNP OM, ENGINE, MGR and TASK working resources.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_InitiateSystemResources (void);

/* FUNCTION NAME : DHCPSNP_INIT_Create_Tasks
 * PURPOSE:
 *        Create DHCPSNP tasks.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_Create_Tasks (void);

/* FUNCTION NAME : DHCPSNP_INIT_SetTransitionMode
 * PURPOSE:
 *        This call will set DHCPSNP_MGR into transition mode to prevent calling request.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_SetTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_INIT_EnterTransitionMode
 * PURPOSE:
 *        This call will set DHCPSNP_MGR into transition mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_EnterTransitionMode(void);

/* FUNCTION NAME : DHCPSNP_INIT_EnterMasterMode
 * PURPOSE:
 *        This call will set DHCPSNP_MGR into master mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_EnterMasterMode(void);

/* FUNCTION NAME : DHCPSNP_INIT_EnterSlaveMode
 * PURPOSE:
 *        This call will set DHCPSNP_MGR into slave mode.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_EnterSlaveMode(void);

/* FUNCTION NAME : DHCPSNP_INIT_ProvisionComplete
 * PURPOSE:
 *        This call will inform DHCPSNP_INIT provision has completed.
 *
 * INPUT:
 *        None.
 *
 * OUTPUT:
 *        None.
 *
 * RETURN:
 *        None.
 *
 * NOTES:
 *        None.
 */
void DHCPSNP_INIT_ProvisionComplete(void);
#endif
