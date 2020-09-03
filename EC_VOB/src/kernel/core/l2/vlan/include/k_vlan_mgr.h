/* MODULE NAME:  k_vlan_mgr.h
 * PURPOSE:
 *  header file of VLAN_MGR for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    7/24/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef K_VLAN_MGR_H
#define K_VLAN_MGR_H

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
 * 	FUNCTION NAME : VLAN_MGR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize VLAN_MGR.
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
void VLAN_MGR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Create_InterCSC_Relation
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
void VLAN_MGR_Create_InterCSC_Relation(void);

#endif    /* End of K_VLAN_MGR_H */

