/* Module Name: BGP_INIT.H
 * Purpose: This module is used to initialize the component 
 *          of orignal BGP.
 *        
 * Notes: 
 *        
 * History:                                                               
 *       Date           Modifier     Reason
 *       2011/03/04     KH SHi       Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef BGP_INIT_H
#define BGP_INIT_H

/* FUNCTION NAME:  BGP_INIT_InitiateSystemResources
 * PURPOSE:
 *    Initialization of BGP
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
void BGP_INIT_InitiateSystemResources(void);

/* FUNCTION NAME:  BGP_INIT_CreateTasks
 * PURPOSE:
 *    This function will create a thread for orignal BGP.
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
void BGP_INIT_CreateTasks(void *arg);   

/* FUNCTION NAME:  BGP_INIT_SetTransitionMode
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
void BGP_INIT_SetTransitionMode(void);

/* FUNCTION NAME:  BGP_INIT_EnterTransitionMode
 * PURPOSE:
 *    This function will force BGP to enter transition state.
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
void BGP_INIT_EnterTransitionMode(void);

/* FUNCTION NAME:  BGP_INIT_EnterMasterMode
 * PURPOSE:
 *    This function will force BGP to enter master state.
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
void BGP_INIT_EnterMasterMode(void);

/* FUNCTION NAME:  BGP_INIT_EnterSlaveMode
 * PURPOSE:
 *    This function will force BGP to enter slave state.
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
void BGP_INIT_EnterSlaveMode(void);

#endif

