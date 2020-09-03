/* Module Name: NSM_INIT.H
 * Purpose: This module is used to initialize the component 
 *          of orignal NSM.
 *        
 * Notes: 
 *        
 * History:                                                               
 *       Date           Modifier        Reason
 *       01/14/2008     Vai Wang        Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef NSM_INIT_H
#define NSM_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  NSM_INIT_Initiate_System_Resources
 * PURPOSE:
 *    Initialization of NSM
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_Initiate_System_Resources(void);   


/* FUNCTION NAME:  NSM_INIT_Create_InterCSC_Relation
 * PURPOSE:
 *    This function initializes all function pointer registration operations.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_Create_InterCSC_Relation(void);


/* FUNCTION NAME:  NSM_INIT_Create_Tasks
 * PURPOSE:
 *    This function will create a thread for orignal NSM.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_Create_Tasks(void *arg);  


/* FUNCTION NAME:  NSM_INIT_SetTransitionMode
 * PURPOSE:
 *    This function will set transition state flag.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_SetTransitionMode(void);


/* FUNCTION NAME:  NSM_INIT_EnterTransitionMode
 * PURPOSE:
 *    This function will force NSM to enter transition state.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_EnterTransitionMode(void);


/* FUNCTION NAME:  NSM_INIT_EnterMasterMode
 * PURPOSE:
 *    This function will force NSM to enter master state.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_EnterMasterMode(void);


/* FUNCTION NAME:  NSM_INIT_EnterSlaveMode
 * PURPOSE:
 *    This function will force NSM to enter slave state.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *
 */
void NSM_INIT_EnterSlaveMode(void);

#endif /* NSM_INIT_H */

