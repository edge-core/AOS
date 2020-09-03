/*-----------------------------------------------------------------------------
 * Module   : mflt_init.h
 *-----------------------------------------------------------------------------
 * PURPOSE  : Initiate the system resources.
 *-----------------------------------------------------------------------------
 * NOTES    :
 *
 *-----------------------------------------------------------------------------
 * HISTORY  : 10/22/2001 - Aaron Chuang, Created
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2002
 *-----------------------------------------------------------------------------
 */

#ifndef	MFLT_INIT_H
#define	MFLT_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAME CONSTANT DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function is to initialize the multicast filtering module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_InitiateProcessResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_Create_InterCSC_Relation
 *-------------------------------------------------------------------------
 * PURPOSE : This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_Create_InterCSC_Relation(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters transition mode and free all MFLT
 *           resources and resets database to factory default.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterTransitionMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterMasterMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_EnterSlaveMode(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MFLT_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : This function sets transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MFLT_INIT_SetTransitionMode(void);


#endif  /* MFLT_INIT_H */
