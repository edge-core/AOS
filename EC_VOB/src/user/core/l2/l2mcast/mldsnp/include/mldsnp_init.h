/* MODULE NAME: mldsnp_init.C
* PURPOSE:
*    {1. What is covered in this file - function and scope}
*    {2. Related documents or hardware information}
* NOTES:
*    {Something must be known or noticed}
*    {1. How to use these functions - give an example}
*    {2. Sequence of messages if applicable}
*    {3. Any design limitations}
*    {4. Any performance limitations}
*    {5. Is it a reusable component}
*
* HISTORY:
*    mm/dd/yy (A.D.)
*    12/03/2007     Macauley_Cheng Create
*
* Copyright(C)      Accton Corporation, 2007
*/
#ifndef _MLDSNP_INIT_H
#define _MLDSNP_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

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

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_InitiateProcessResources
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to initialize the igmp snooping module.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_InitiateProcessResources(void);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_Create_Tasks
 *-------------------------------------------------------------------------
 * PURPOSE : This function is used to create task.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_Create_Tasks(void);


/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters transition mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_EnterTransitionMode (void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters master mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_EnterMasterMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_EnterSlaveMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_EnterSlaveMode(void);
/*-------------------------------------------------------------------------
 * FUNCTION NAME - MLDSNP_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * PURPOSE : The function enters slave mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
void MLDSNP_INIT_SetTransitionMode(void);

#endif



