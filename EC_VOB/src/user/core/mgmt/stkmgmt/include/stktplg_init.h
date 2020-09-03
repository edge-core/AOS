/* MODULE NAME:  stktplg_init.h
 * PURPOSE:
 *    For initialization of stktplg.
 *
 * NOTES:
 *
 * HISTORY
 *    7/30/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef STKTPLG_INIT_H
#define STKTPLG_INIT_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : STKTPLG_INIT_InitiateProcessResources
 * PURPOSE: This function initializes STKTPLG which inhabits in the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 *          
 */
BOOL_T STKTPLG_INIT_InitiateProcessResources(void);

#endif    /* End of STKTPLG_INIT_H */

