/* MODULE NAME:  dbsync_txt_task.c
* PURPOSE:
*   DBSYNC_TXT initiation and DBSYNC_TXT task creation
*
* NOTES:
*
* History:
*       Date          -- Modifier,  Reason
*     2003-2-10       -- poli , created.
*
* Copyright(C)      Accton Corporation, 2002
*/



/* INCLUDE FILE DECLARATIONS
 */

#if (SYS_CPNT_DBSYNC_TXT == TRUE)

#include "sys_bld.h"
#include "sys_type.h"
#include "sysfun.h"
#include "cli_om_exp.h"
#include "dbsync_txt_task.h"
#include "dbsync_txt_mgr.h"
#include "sys_dflt.h"
#include "l_threadgrp.h"
#include "stktplg_pom.h"
#include "xfer_proc_comm.h"

extern int printf(const char *_format, ...);


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* FUNCTION NAME:  DBSYNC_TXT_TASK_Main
 * PURPOSE:
 *			DBSYNC_TXT starting routine.
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
 *          This function is invoked in DBSYNC_TXT_TASK_CreateTask().
 */
static void DBSYNC_TXT_TASK_Main();

/*
 *  DATA TYPE DECLARATIONS
 */
typedef enum
{
    DBSYNC_TXT_TASK_SAVEMASTER = 0,
    DBSYNC_TXT_TASK_SYNCSLAVE
}   DBSYNC_TXT_TASK_NEXTACTION_T;


/* STATIC VARIABLE DECLARATIONS
 */
static  BOOL_T  DBSYNC_TASK_MasterSaveDone = FALSE;
static  BOOL_T  is_transition_done;
static  UI32_T  dbsync_txt_task_main_id;
static  DBSYNC_TXT_TASK_NEXTACTION_T   dbsync_txt_task_next_action = DBSYNC_TXT_TASK_SAVEMASTER;



/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  DBSYNC_TXT_TASK_Init
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
 *          This function is invoked in DBSYNC_TXT_INIT_Initiate_System_Resources.
 */
BOOL_T DBSYNC_TXT_TASK_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    is_transition_done = FALSE;
    dbsync_txt_task_next_action = DBSYNC_TXT_TASK_SAVEMASTER;

    return TRUE;
}



/* FUNCTION NAME:  DBSYNC_TXT_TASK_CreateTask
 * PURPOSE:
 *			This function create dbsync_txt main task.
 * INPUT:
 *          none.
 *
 * OUTPUT:
 *          none.
 *
 * RETURN:
 *          return TRUE to indicate success and FALSE to indicate failure.
 * NOTES:
 *          This function is invoked in DBSYNC_TXT_INIT_Create_Tasks().
 */
BOOL_T DBSYNC_TXT_TASK_CreateTask()
{

	/* LOCAL CONSTANT DECLARATIONS
	*/

	/* LOCAL VARIABLES DECLARATIONS
	*/

	/* BODY */
	if (SYSFUN_SpawnThread(SYS_BLD_DBSYNC_TXT_CSC_THREAD_PRIORITY,
        SYS_BLD_DBSYNC_TXT_CSC_THREAD_SCHED_POLICY,
        SYS_BLD_DBSYNC_TXT_CSC_THREAD_NAME,
        SYS_BLD_TASK_COMMON_STACK_SIZE*4,
        SYSFUN_TASK_FP,
        DBSYNC_TXT_TASK_Main,
        NULL,
        &dbsync_txt_task_main_id) != SYSFUN_OK)
    {
		printf("\r\nDBSYNC_TXT main task creation error\r\n");
		return FALSE;
	} /* End of if */

	return TRUE;

} /* end of DBSYNC_TXT_TASK_CreateTask() */



/* FUNCTION NAME : DBSYNC_TXT_TASK_SetTransitionMode
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
void DBSYNC_TXT_TASK_SetTransitionMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    is_transition_done = FALSE;
    return;
}



/* FUNCTION NAME : DBSYNC_TXT_TASK_EnterTransitionMode
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
void DBSYNC_TXT_TASK_EnterTransitionMode()
{
	/* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */

    /* BODY */
    SYSFUN_TASK_ENTER_TRANSITION_MODE(is_transition_done);
    return;
}


/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  DBSYNC_TXT_TASK_Main
 * PURPOSE:
 *			DBSYNC_TXT starting routine.
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
 *          This function is invoked in DBSYNC_TXT_TASK_CreateTask().
 */
#ifndef SYS_DFLT_DBSYNC_TXT_INTERVAL
#define SYS_DFLT_DBSYNC_TXT_INTERVAL                    1 /*30 based on the suggestion of SJ, and is same as Hagrid*//* seconds */
#endif
static void DBSYNC_TXT_TASK_Main()
{
    UI32_T member_id;
    L_THREADGRP_Handle_T tg_handle = XFER_PROC_COMM_GetXFER_GROUPTGHandle();

    /* join the thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_DBSYNC_TXT_CSC_THREAD_PRIORITY, &member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while (TRUE)
    {
    	/* request thread group execution permission
         */
        if(L_THREADGRP_Execution_Request(tg_handle, member_id)!=TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Request fail.\n", __FUNCTION__);
        }


        switch ( DBSYNC_TXT_MGR_GetOperationMode() )
        {
            case SYS_TYPE_STACKING_MASTER_MODE:

                {
                   BOOL_T is_provision_complete;
                   BOOL_T is_cli_reproduce_done;
                    UI32_T  num_unit = 0;

                   is_provision_complete = DBSYNC_TXT_MGR_IsProvisionComplete();
                   is_cli_reproduce_done = CLI_OM_EXP_GetIsReproduceDone();

                    if ( is_cli_reproduce_done == TRUE
                        && is_provision_complete == TRUE
                        && DBSYNC_TXT_MGR_Get_IsDoingAutosave( ) == FALSE )
                    {
                        switch( dbsync_txt_task_next_action )
                        {
                            case DBSYNC_TXT_TASK_SAVEMASTER:
                   {
                                    if( DBSYNC_TXT_MGR_SaveRunningCfg( ) == FALSE )
                                    {
                                        DBSYNC_TASK_MasterSaveDone = FALSE;
                                    }
                                    else
                                    {
                                        DBSYNC_TASK_MasterSaveDone = TRUE;
                                    }
                                }
                                dbsync_txt_task_next_action = DBSYNC_TXT_TASK_SYNCSLAVE;
                                break;

                            case DBSYNC_TXT_TASK_SYNCSLAVE:
                                {
                                    if( STKTPLG_POM_GetNumberOfUnit( &num_unit ) == FALSE )
                                    {
                                        SYSFUN_Debug_Printf( "DBSYNC_TXT: Stacking mode error!!!\r\n" );
                                    }
                                    if( num_unit > 1 )
                                    {
                                        DBSYNC_TXT_MGR_SyncSlaveCfg( );
                   }
                }
                                dbsync_txt_task_next_action = DBSYNC_TXT_TASK_SAVEMASTER;
                                break;

                            default:
                                break;
                        }
                    }
                }
                /*
                 *  Do not sleep if Master Save Config fail and next action is slave cfg sync.
                 */
                if( !( DBSYNC_TASK_MasterSaveDone == FALSE && dbsync_txt_task_next_action == DBSYNC_TXT_TASK_SYNCSLAVE ) )
                {
                    SYSFUN_Sleep(SYS_DFLT_DBSYNC_TXT_INTERVAL*SYS_BLD_TICKS_PER_SECOND);
                }
                break;

            case SYS_TYPE_STACKING_TRANSITION_MODE:
                is_transition_done = TRUE;
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

        /* release thread group execution permission
         */
        if(L_THREADGRP_Execution_Release(tg_handle, member_id)!=TRUE)
        {
            SYSFUN_Debug_Printf("%s: L_THREADGRP_Execution_Release fail.\n", __FUNCTION__);
        }

    } /* End of while */

} /* End of DBSYNC_TXT_TASK_Main () */


#endif /* End of #if (SYS_CPNT_DBSYNC_TXT == TRUE) */


