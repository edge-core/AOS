/* static char SccsId[] = "+-<>?!NTP_INIT.C   22.1  05/08/02  11:00:00";
 * ------------------------------------------------------------------------
 *  FILE NAME  -  _NTP_INIT.C
 * ------------------------------------------------------------------------
 *  ABSTRACT:
 *
 *  Modification History:
 *  Modifier           Date        Description
 *  -----------------------------------------------------------------------
 *   HardSun, 2005 02 17 10:59     Created    Created
 * ------------------------------------------------------------------------
 *  Copyright(C)        Accton Corporation, 2005
 * ------------------------------------------------------------------------
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ntp_mgr.h"
#include "ntp_task.h"
#include "ntp_dbg.h"
#include "ntp_recvbuff.h"

/* NAME CONSTANT DECLARATIONS
 */



/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */



/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

 /*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_Init
 *---------------------------------------------------------------------------
 * PURPOSE: This function initaites the system resources, such as queue, semaphore,
 *          and events used by this subsystem. All the call back functions shall be
 *          registered during subsystem initiation.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function must be invoked before any tasks in this subsystem can be created.
 *       2. This function must be invoked before any services in this subsystem can be executed.
 *       3. This function initialize mapping tables to their default values
 *---------------------------------------------------------------------------
 */
void NTP_INIT_Initiate_System_Resources(void)
{
    NTP_MGR_Init();
    NTP_TASK_Init();
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - NTP_INIT_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void NTP_INIT_Create_InterCSC_Relation(void)
{
    NTP_DBG_Create_InterCSC_Relation();
}

/*--------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_Create_Task()
 *---------------------------------------------------------------------------
 * PURPOSE: This function creates all the task of this subsystem.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE: 1. This function shall not be invoked before NTP_INIT_Init() is performed
 *       2. NTP is a totally passive module, that is there is no task will be created
 *---------------------------------------------------------------------------
 */
void NTP_INIT_Create_Tasks(void)
{
    NTP_TASK_CreateTask();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_EnterMasterMode
 *-------------------------------------------------------------------------
 * PURPOSE: This function initiates all the system database, and also configures
 *          the switch to the initiation state based on the specified "System Boot
 *          Configruation File". After that, the COS subsystem will enter the
 *          Master Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. All the other subsystems must enter the master mode before this function
 *          can be invoked.
 *       2. If "System Boot Configruation File" does not exist, the database and
 *          switch will be initiated to the factory default value.
 *-------------------------------------------------------------------------
 */
void NTP_INIT_EnterMasterMode(void)
{
    NTP_MGR_EnterMasterMode();
    NTP_TASK_EnterMasterMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_EnterTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Transition Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: 1. Reallocate all resource for this subsystem
 *       2. Will be called when operation mode change between Master and Slave mode
 *-------------------------------------------------------------------------
 */
void NTP_INIT_EnterTransitionMode(void)
{
    NTP_MGR_EnterTransitionMode();
    NTP_TASK_EnterTransitionMode();
}

 /*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_SetTransitionMode
 *-------------------------------------------------------------------------
 * Purpose: TThis call will set ntp_mgr into transition mode to prevent
 *      calling request.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:
 *-------------------------------------------------------------------------
 */
void NTP_INIT_SetTransitionMode(void)
{
    NTP_MGR_SetTransitionMode();
    NTP_TASK_SetTransitionMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_Enter_Slave_Mode
 *-------------------------------------------------------------------------
 * Purpose: This function forces this subsystem enter the Slave Operation mode.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE: In Slave Operation mode, any requests issued from applications will be ignored.
 *-------------------------------------------------------------------------
 */
void NTP_INIT_EnterSlaveMode(void)
{
    NTP_MGR_EnterSlaveMode();
}

/*-------------------------------------------------------------------------
 * ROUTINE NAME - NTP_INIT_ProvisionComplete
 *-------------------------------------------------------------------------
 * Purpose: This function will tell the SSHD module to start.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE:    This function is invoked in STKCTRL_TASK_ProvisionComplete().
 *          This function shall call NTP_TASK_ProvisionComplete().
 *          If it is necessary this function will call NTP_MGR_ProvisionComplete().
 *-------------------------------------------------------------------------
 */
void NTP_INIT_ProvisionComplete(void)
{
    NTP_TASK_ProvisionComplete();
}

