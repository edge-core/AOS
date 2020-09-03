/* MODULE NAME: amtrl3_backdoor.h
 * PURPOSE: 
 *  This file supports a backdoor for the AMTRL3
 *
 * NOTES:
 *
 * History:
 *    2008/02/282 : djd      Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef _AMTRL3_BACKDOOR_H
#define _AMTRL3_BACKDOOR_H
     
/* INCLUDE FILE DECLARATIONS
 */


/* NAMING CONSTANT DECLARATIONS
 */
#define AMTRL3_SUPPORT_ACCTON_BACKDOOR        TRUE    /* support AMTRL3 backdoor */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (AMTRL3_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AMTRL3_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : AMTRL3 backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AMTRL3_Backdoor_CallBack();

#endif /* AMTRL3_SUPPORT_ACCTON_BACKDOOR == TRUE */

#endif
