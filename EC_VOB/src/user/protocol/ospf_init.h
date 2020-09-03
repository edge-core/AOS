/* Module Name: OSPF_INIT.H
 * Purpose: This module is used to initialize the component 
 *          of orignal OSPF.
 *        
 * Notes: 
 *        
 * History:                                                               
 *       Date               Modifier        Reason
 *       11/27/2008     Lin.Li           Create this file
 *
 * Copyright(C)      Accton Corporation, 2008
 */
#ifndef OSPF_INIT_H
#define OSPF_INIT_H

/* FUNCTION NAME:  OSPF_INIT_Initiate_System_Resources
* PURPOSE:
*    Initialization of OSPF
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
void OSPF_INIT_Initiate_System_Resources(void);

/* FUNCTION NAME:  OSPF_INIT_Create_Tasks
* PURPOSE:
*    This function will create a thread for orignal OSPF.
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
void OSPF_INIT_Create_Tasks(void *arg);   

/* FUNCTION NAME:  OSPF_INIT_SetTransitionMode
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
void OSPF_INIT_SetTransitionMode(void);

/* FUNCTION NAME:  OSPF_INIT_EnterTransitionMode
* PURPOSE:
*    This function will force OSPF to enter transition state.
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
void OSPF_INIT_EnterTransitionMode(void);

/* FUNCTION NAME:  OSPF_INIT_EnterMasterMode
* PURPOSE:
*    This function will force OSPF to enter master state.
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
void OSPF_INIT_EnterMasterMode(void);

/* FUNCTION NAME:  OSPF_INIT_EnterSlaveMode
* PURPOSE:
*    This function will force OSPF to enter slave state.
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
void OSPF_INIT_EnterSlaveMode(void);

#endif


