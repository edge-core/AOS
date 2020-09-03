/*-----------------------------------------------------------------------------
 * MODULE NAME: AMTR_OM_PRIVATE.H
 *-----------------------------------------------------------------------------
 * PURPOSE:
 *    The declarations of AMTR OM which are only used by AMTR.
 *
 * NOTES:
 *    None.
 *
 * HISTORY:
 *    2007/07/14     --- Timon, Create and move something from amtr_om.h
 *
 * Copyright(C)      Accton Corporation, 2007
 *-----------------------------------------------------------------------------
 */

#ifndef XFER_OM_PRIVATE_H
#define XFER_OM_PRIVATE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_Init
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize XFER OM
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------*/
void XFER_OM_Init(void);

/*--------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_EnterMasterMode
 *---------------------------------------------------------------------------
 * PURPOSE:  Assign default value for XFER OM
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *--------------------------------------------------------------------------*/
void XFER_OM_EnterMasterMode(void);

/*------------------------------------------------------------------------
 * ROUTINE NAME - XFER_OM_SetTftpRetryTimes
 *------------------------------------------------------------------------
 * FUNCTION: Set TFTP retry times
 * INPUT   : UI32_T retry_times
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void XFER_OM_SetTftpRetryTimes(UI32_T retry_times); 

#endif /* end XFER_OM_PRIVATE_H */

