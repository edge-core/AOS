/* MODULE NAME:  stkctrl_pmgr.h
 * PURPOSE:
 *  stkctrl pmgr
 *
 * NOTES:
 *
 * HISTORY
 *    08/01/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKCTRL_PMGR_H
#define STKCTRL_PMGR_H

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
/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_InitiateProcessResources
 * ---------------------------------------------------------------------
 * PURPOSE: This function initializes STKCTRL PMGR for the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T STKCTRL_PMGR_InitiateProcessResources(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_ReloadSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send reload event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_ReloadSystem(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_WarmStartSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send warm start event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_WarmStartSystem(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_ColdStartSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send cold start event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_ColdStartSystem(void);

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_UnitIDReNumbering
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send renumbering event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  -- Renumbering work (not standalone or unit ID not 
 *                     equal to 1)
 *            FALSE -- otherwise
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
BOOL_T STKCTRL_PMGR_UnitIDReNumbering(void);

#endif    /* End of STKCTRL_PMGR_H */

