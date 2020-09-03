/* MODULE NAME:  sshd_init.c
* PURPOSE: 
*   SSHD initiation and SSHD task creation
*   
* NOTES:
*   
* History:                                                               
*       Date          -- Modifier,  Reason
*     2002-05-24      -- Isiah , created.
*   
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */
#include <sys_type.h>
#include "sshd_init.h"
#include "sshd_task.h"
#include "sshd_mgr.h"



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

/* FUNCTION NAME:  SSHD_INIT_Initiate_System_Resources
 * PURPOSE: 
 *          This function initaites the system resources, such as queue, semaphore, 
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          1. This function must be invoked before any tasks in this subsystem can be created.
 *          2. This function must be invoked before any services in this subsystem can be executed.
 */
void SSHD_INIT_Initiate_System_Resources(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SSHD_TASK_Init();
    SSHD_MGR_Init();

	return;
	
} 

/*--------------------------------------------------------------------------
 * FUNCTION NAME - SSHD_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/ 
void SSHD_INIT_Create_InterCSC_Relation(void)
{
    return;
} /* end of SSHD_INIT_Create_InterCSC_Relation */


/* FUNCTION NAME:  SSHD_INIT_Create_Tasks
 * PURPOSE: 
 *          This function creates all the task of this subsystem.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          1. This function shall not be invoked before SSHD_INIT_InitiateSystemResources() is performed.
 */
void SSHD_INIT_Create_Tasks(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SSHD_TASK_CreateTask();

	return;
	
} 



/* FUNCTION NAME:  SSHD_INIT_EnterMasterMode
 * PURPOSE: 
 *          This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the SSHD subsystem will enter the
 *          Master Operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          1. If "System Boot Configruation File" does not exist, the system database and
 *             switch will be initiated to the factory default value.
 *          2. SSHD will handle network requests only when this subsystem
 *             is in the Master Operation mode 
 */
void SSHD_INIT_EnterMasterMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SSHD_MGR_EnterMasterMode();

	return;
	
} 



/* FUNCTION NAME:  SSHD_INIT_EnterSlaveMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Slave Operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          In Slave Operation mode, any network requests 
 *          will be ignored.
 */
void SSHD_INIT_EnterSlaveMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SSHD_MGR_EnterSlaveMode();

	return;
	
} 



/* FUNCTION NAME:  SSHD_INIT_EnterTransitionMode
 * PURPOSE: 
 *          This function forces this subsystem enter the Teansition Operation mode.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 * NOTES:
 *          In Transition Operation mode, any network requests 
 *          will be ignored.
 */
void SSHD_INIT_EnterTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
    SSHD_MGR_EnterTransitionMode();
    SSHD_TASK_EnterTransitionMode();

	return;
	
} 



/* FUNCTION	NAME : SSHD_INIT_SetTransitionMode
 * PURPOSE:
 *		This call will set sshd_mgr into transition mode to prevent
 *		calling request.
 *
 * INPUT:
 *		None.
 *
 * OUTPUT:
 *		None.
 *
 * RETURN:
 *		None.
 *
 * NOTES:
 *		None.
 */
void SSHD_INIT_SetTransitionMode(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	SSHD_MGR_SetTransitionMode();
	SSHD_TASK_SetTransitionMode();
}



/* FUNCTION NAME:  SSHD_INIT_ProvisionComplete
 * PURPOSE: 
 *          This function will tell the SSHD module to start.
 *
 * INPUT:   
 *          none.
 *                                  
 * OUTPUT:  
 *          none.
 *                                   
 * RETURN:  
 *          none.
 *          
 * NOTES:
 *          This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call SSHD_TASK_ProvisionComplete().
 *          If it is necessary this function will call SSHD_MGR_ProvisionComplete().
 */
void SSHD_INIT_ProvisionComplete(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */
    
    /* LOCAL VARIABLES DECLARATIONS 
    */

    /* BODY */
	SSHD_TASK_ProvisionComplete();
}



/* FUNCTION NAME - SSHD_INIT_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void SSHD_INIT_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    SSHD_MGR_HandleHotInsertion(starting_port_ifindex, number_of_port, use_default);
    return;
}



/* FUNCTION NAME - SSHD_INIT_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void SSHD_INIT_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS 
    */
 
    /* BODY */
    SSHD_MGR_HandleHotRemoval(starting_port_ifindex, number_of_port);
    return;
}



/* LOCAL SUBPROGRAM BODIES
 */



