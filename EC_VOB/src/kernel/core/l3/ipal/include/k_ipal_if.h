/* MODULE NAME:  k_ipal_if.h
 * PURPOSE:
 *  header file of IPAL_IF for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    11/20/2014 - Irene Pan, Created
 *
 * Copyright(C)      Accton Corporation, 2014
 */
#ifndef K_IPAL_IF_H
#define K_IPAL_IF_H

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
 * 	FUNCTION NAME : IPAL_IF_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize IPAL_IF.
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
void IPAL_IF_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IPAL_IF_Create_InterCSC_Relation
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
void IPAL_IF_Create_InterCSC_Relation(void);

#endif    /* End of K_IPAL_IF_H */

