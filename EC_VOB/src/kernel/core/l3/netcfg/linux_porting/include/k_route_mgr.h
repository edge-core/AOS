/* MODULE NAME:  k_route_mgr.h
 * PURPOSE:
 *   linux kernel module to provide APIs for ROUTE_MGR
 *
 * NOTES:
 *
 * HISTORY
 *    3/2/2010 - KH Shi Created
 *
 * Copyright(C)      Accton Corporation, 2010
 */
#ifndef K_ROUTE_MGR_H
#define K_ROUTE_MGR_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*--------------------------------------------------------------------------
 * 	FUNCTION NAME : ROUTE_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize ROUTE_MGR.
 *
 * 	INPUT:   
 *   None.
 *
 * 	OUTPUT:
 *   None.
 *
 * 	RETURN:
 *   None.
 *
 * 	NOTES:
 *   None.
 *--------------------------------------------------------------------------
 */
void ROUTE_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ROUTE_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE:
 *   This function initializes all function pointer registration operations.
 *
 * INPUT:
 *   None.
 *    
 * OUTPUT:
 *   None.
 *
 * RETURN
 *   None.
 *
 * NOTES:
 *   None.
 *--------------------------------------------------------------------------*/
void ROUTE_MGR_Create_InterCSC_Relation(void);


#endif    /* End of K_ROUTE_MGR_H */

