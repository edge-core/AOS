/* MODULE NAME:  ptp_isr.h
 * PURPOSE:
 *     User space interrupt service routine for PTP.
 *
 * NOTES:
 *     None.
 *
 * CREATOR:      Charlie Chen
 * HISTORY
 *    4/27/2012 - Charlie Chen, Created
 *
 * Copyright(C)      EdgeCore Corporation, 2012
 */
#ifndef PTP_ISR_H
#define PTP_ISR_H

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

/* FUNCTION NAME : PTP_ISR_CreateTask
 * PURPOSE:
 *      Spawn task for PTP ISR.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void PTP_ISR_CreateTask(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_InitiateProcessResource
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all process resource.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PTP_ISR_InitiateProcessResource(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - PTP_ISR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void PTP_ISR_Create_InterCSC_Relation(void);

/* FUNCTION NAME : PTP_ISR_CreateTask
 * PURPOSE:
 *      Spawn task for PTP ISR.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void PTP_ISR_CreateTask(void);

#endif    /* End of PTP_ISR_H */
