/* Module Name: OSPF6_INIT.H
 * Purpose: This module is used to initialize the component 
 *          of orignal OSPF6.
 *        
 * Notes: 
 *        
 * History:                                                               
 *       Date               Modifier        Reason
 *       7/13/2009          Steven.Gao      Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef OSPF6_INIT_H
#define OSPF6_INIT_H

/* FUNCTION NAME:  OSPF6_INIT_Initiate_System_Resources
* PURPOSE:
*    Initialization of OSPF6
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
void OSPF6_INIT_Initiate_System_Resources(void);

/* FUNCTION NAME:  OSPF6_INIT_Create_Tasks
* PURPOSE:
*    This function will create a thread for orignal OSPF6.
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
void OSPF6_INIT_Create_Tasks(void *arg);   

/* FUNCTION NAME:  OSPF6_INIT_SetTransitionMode
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
void OSPF6_INIT_SetTransitionMode(void);

/* FUNCTION NAME:  OSPF6_INIT_EnterTransitionMode
* PURPOSE:
*    This function will force OSPF6 to enter transition state.
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
void OSPF6_INIT_EnterTransitionMode(void);

/* FUNCTION NAME:  OSPF6_INIT_EnterMasterMode
* PURPOSE:
*    This function will force OSPF6 to enter master state.
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
void OSPF6_INIT_EnterMasterMode(void);

/* FUNCTION NAME:  OSPF6_INIT_EnterSlaveMode
* PURPOSE:
*    This function will force OSPF6 to enter slave state.
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
void OSPF6_INIT_EnterSlaveMode(void);

#endif


