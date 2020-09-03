/*-------------------------------------------------------------------------
 * Module Name: rspan_init.h
 *-------------------------------------------------------------------------
 * PURPOSE: Initiate the system resources and create the task.
 *-------------------------------------------------------------------------
 * NOTES:
 *
 *-------------------------------------------------------------------------
 * HISTORY:
 *    07/23/2007 - Tien Kuo, Created
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2007
 *-------------------------------------------------------------------------
 */
#ifndef 	RSPAN_INIT_H
#define 	RSPAN_INIT_H

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
#if 0
/*---------------------------------------------------------------------------------
 * FUNCTION NAME: RSPAN_INIT_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for RSPAN in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void RSPAN_INIT_AttachSystemResources(void);

/* FUNCTION NAME: RSPAN_INIT_GetShMemInfo
 *----------------------------------------------------------------------------------
 * PURPOSE: Get share memory info
 *----------------------------------------------------------------------------------
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 *----------------------------------------------------------------------------------
 * NOTES:   
 */
void RSPAN_INIT_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);
#endif
/* FUNCTION NAME : RSPAN_INIT_InitiateSystemResources
 * PURPOSE: This function is used to initialize the RSPAN module.
 * INPUT:   None.
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *
 */
void RSPAN_INIT_InitiateSystemResources(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_INIT_SetTransitionMode
 *--------------------------------------------------------------------------
 * PURPOSE:  This function will make the RSPAN_INIT to enter transition mdoe
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *--------------------------------------------------------------------------
 */
void RSPAN_INIT_SetTransitionMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_INIT_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the RSPAN_INIT enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RSPAN_INIT_EnterMasterMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_INIT_EnterEnterSlaveMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the RSPAN_INIT enter the slave mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RSPAN_INIT_EnterSlaveMode(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - RSPAN_INIT_EnterTransitionMode
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will make the RSPAN_INIT to enter transition mdoe
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RSPAN_INIT_EnterTransitionMode(void);

#endif   /* RSPAN_INIT_H */
