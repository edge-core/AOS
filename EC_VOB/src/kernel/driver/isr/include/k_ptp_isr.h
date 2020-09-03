/* MODULE NAME:  k_ptp_isr.h
 * PURPOSE:
 *  header file of PTP_ISR for linux kernel
 *
 * NOTES:
 *
 * HISTORY
 *    4/27/2012 - Charlie Chen, Created
 *
 * Copyright(C)      Edgecore Corporation, 2012
 */
#ifndef K_PTP_ISR_H
#define K_PTP_ISR_H

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
 * 	FUNCTION NAME : PTP_ISR_Init
 *--------------------------------------------------------------------------
 * 	PURPOSE: 
 *   Initialize PTP_ISR.
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
void PTP_ISR_Init(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_Create_InterCSC_Relation
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
void PTP_ISR_Create_InterCSC_Relation(void);

#endif    /* End of K_PTP_ISR_H */

