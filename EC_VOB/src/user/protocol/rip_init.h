/* Module Name: RIP_INIT.H
 * Purpose: This module is used to initialize the component 
 *          of orignal RIP.
 *        
 * Notes: 
 *        
 * History:                                                               
 *       Date               Modifier        Reason
 *       05/12/2008     Lin.Li           Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef RIP_INIT_H
#define RIP_INIT_H
/* FUNCTION NAME:  RIP_INIT_Initiate_System_Resources
* PURPOSE:
*    Initialization of RIP
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
void RIP_INIT_Initiate_System_Resources(void);

/* FUNCTION NAME:  RIP_INIT_Create_Tasks
* PURPOSE:
*    This function will create a thread for orignal RIP.
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
void RIP_INIT_Create_Tasks(void *arg);   

/* FUNCTION NAME:  RIP_INIT_SetTransitionMode
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
void RIP_INIT_SetTransitionMode(void);

/* FUNCTION NAME:  RIP_INIT_EnterTransitionMode
* PURPOSE:
*    This function will force RIP to enter transition state.
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
void RIP_INIT_EnterTransitionMode(void);

/* FUNCTION NAME:  RIP_INIT_EnterMasterMode
* PURPOSE:
*    This function will force RIP to enter master state.
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
void RIP_INIT_EnterMasterMode(void);

/* FUNCTION NAME:  RIP_INIT_EnterSlaveMode
* PURPOSE:
*    This function will force RIP to enter slave state.
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
void RIP_INIT_EnterSlaveMode(void);

#endif

