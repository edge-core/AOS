/* MODULE NAME:  k_dying_gasp.h
 * PURPOSE:
 *  header file of DYING_GASP for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    7/29/2010 - Charlie Chen, Created
 *
 * Copyright(C)      Edgecore Corporation, 2010
 */
#ifndef K_DYING_GASP_H
#define K_DYING_GASP_H

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
 * 	FUNCTION NAME : DYING_GASP_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize DYING_GASP.
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
void DYING_GASP_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - DYING_GASP_Create_InterCSC_Relation
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
void DYING_GASP_Create_InterCSC_Relation(void);

#endif    /* End of K_DYING_GASP_H */

