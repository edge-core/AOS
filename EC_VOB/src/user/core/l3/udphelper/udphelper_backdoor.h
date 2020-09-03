/* MODULE NAME: udphelper_backdoor.h
 * PURPOSE: 
 *  This file supports a backdoor for the UDPHELPER
 *
 * NOTES:
 *
 * History:
 *    03/31/2009 : LinLi      Create
 *
 * Copyright(C)      Accton Corporation, 2009
 */
#ifndef _UDPHELPER_BACKDOOR_H
#define _UDPHELPER_BACKDOOR_H
     
/* INCLUDE FILE DECLARATIONS
 */


/* NAMING CONSTANT DECLARATIONS
 */
#define UDPHELPER_SUPPORT_ACCTON_BACKDOOR        TRUE    /* support UDPHELPER backdoor */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#if (UDPHELPER_SUPPORT_ACCTON_BACKDOOR == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  UDPHELPER_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : UDPHELPER backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void UDPHELPER_Backdoor_CallBack();

#endif /* UDPHELPER_SUPPORT_ACCTON_BACKDOOR == TRUE */

#endif

