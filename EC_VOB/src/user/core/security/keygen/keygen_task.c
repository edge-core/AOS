/* MODULE NAME:  keygen_task.c
* PURPOSE:
*   KEYGEN initiation and KEYGEN task creation
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2002-10-16      -- Isiah , created.
*     2007-07-04      -- Rich ,  Porting to Linux Platform
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */
#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"

#include "keygen_type.h"
#include "keygen_task.h"
#include "keygen_mgr.h"
#include "l_threadgrp.h"
#include "cli_proc_comm.h"

#include "sshd_type.h"
#include "sshd_om.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
//static BOOL_T KEYGEN_TASK_IsProvisionComplete(void);
static void KEYGEN_TASK_Main();



/* STATIC VARIABLE DECLARATIONS
 */
static  BOOL_T  kgen_is_provision_complete = FALSE;


/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  KEYGEN_TASK_Init
 * PURPOSE:
 *          This function init the message queue.
 *
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          TRUE to indicate successful and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in KEYGEN_INIT_Initiate_System_Resources.
 */
BOOL_T KEYGEN_TASK_Init(void)
{

    return TRUE;
}

/* FUNCTION NAME:  KEYGEN_TASK_CreateTask
 * PURPOSE:
 *			This function create keygen main task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in KEYGEN_INIT_Create_Tasks().
 */
BOOL_T KEYGEN_TASK_CreateTask()
{
	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/
	UI32_T keygen_task_main_id;

	/* BODY */
    if(SYSFUN_SpawnThread(SYS_BLD_KEYGEN_CSC_THREAD_PRIORITY,
                          SYS_BLD_KEYGEN_CSC_THREAD_SCHED_POLICY,
                          SYS_BLD_KEYGEN_CSC_THREAD_NAME,
#ifdef ES3526MA_POE_7LF_LN /* Yongxin.Zhao added, 2009-06-23, 09:46:14 */
                          (SYS_BLD_TASK_COMMON_STACK_SIZE * 8),
#else
                          SYS_BLD_TASK_COMMON_STACK_SIZE*4,
#endif /* ES3526MA_POE_7LF_LN */
                          SYSFUN_TASK_NO_FP,
                          KEYGEN_TASK_Main,
                          NULL,
                          &keygen_task_main_id)!=SYSFUN_OK)
    {
        printf("%s:SYSFUN_SpawnThread fail.\n", __FUNCTION__);
    }

	return TRUE;

} /* end of KEYGEN_TASK_CreateTask() */

/* FUNCTION NAME : KEYGEN_TASK_SetTransitionMode
 * PURPOSE:
 *		Sending enter transition event to task calling by stkctrl.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void KEYGEN_TASK_SetTransitionMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return;
}

/* FUNCTION NAME : KEYGEN_TASK_EnterTransitionMode
 * PURPOSE:
 *		Leave CSC Task while transition done.
 * INPUT:
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *     None.
 *
 */
void KEYGEN_TASK_EnterTransitionMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    return;
}

/* FUNCTION NAME:  KEYGEN_TASK_ProvisionComplete
 * PURPOSE:
 *          This function will tell the KEYGEN module to start.
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
 *          This function is invoked in KEYGEN_INIT_ProvisionComplete().
 */
void KEYGEN_TASK_ProvisionComplete(void)
{
    kgen_is_provision_complete = TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:  KEYGEN_TASK_Main
 * PURPOSE:
 *			KEYGEN starting routine.
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
 *          This function is invoked in KEYGEN_TASK_CreateTask().
 */
static void KEYGEN_TASK_Main()
{
	/* LOCAL CONSTANT DECLARATIONS
	 */

	/* LOCAL VARIABLES DECLARATIONS
	 */

	/* BODY */
	while (TRUE)
    {
        switch ( KEYGEN_MGR_GetOperationMode() )
        {
        case SYS_TYPE_STACKING_MASTER_MODE:
            /* Generate server key periodically.
             *
             * There have one security issue. Before the server key update
             * the SSHv1 session will not be safe.
             *
             */
            if (    (TRUE == kgen_is_provision_complete)
                &&  (SSHD_STATE_ENABLED == SSHD_OM_GetSshdStatus()))
            {
                KEYGEN_MGR_GenTempPublicKeyPair();
                SYSFUN_Sleep(6000);
            }
            SYSFUN_Sleep(10);
            break;

        case SYS_TYPE_STACKING_TRANSITION_MODE:
            SYSFUN_Sleep(10);
            break;

        case SYS_TYPE_STACKING_SLAVE_MODE:
            SYSFUN_Sleep(10);
            break;

        default:
            /* log error; */
            SYSFUN_Sleep(10);
            break;

        } /* End of switch */
    } /* End of while */

} /* End of KEYGEN_TASK_Main () */


