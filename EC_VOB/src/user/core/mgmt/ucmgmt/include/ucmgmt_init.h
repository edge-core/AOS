/* Module Name: UCMGMT_INIT.H
 * Purpose: Initialize the resources for the ucmgmt module.
 *
 * Notes:
 *
 * History:
 *    11/28/01       -- Aaron Chuang, Create
 *
 * Copyright(C)      Accton Corporation, 1999 - 2001
 *
 */
#ifndef _UCMGMT_INIT_H
#define _UCMGMT_INIT_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */


/* STATIC VARIABLE DECLARATIONS
 */


/* MACRO FUNCTIONS DECLARACTION
 */


/* EXPORTED SUBPROGRAM BODIES
 */


/* FUNCTION NAME: UCMGMT_INIT_InitiateProcessResources
 * PURPOSE: This function is used to initialize the un-cleared memory management module.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:   Initialize the ucmgmt module.
 *
 */
void UCMGMT_INIT_InitiateProcessResources(void);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - UCMGMT_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void UCMGMT_INIT_Create_InterCSC_Relation(void);

#endif
