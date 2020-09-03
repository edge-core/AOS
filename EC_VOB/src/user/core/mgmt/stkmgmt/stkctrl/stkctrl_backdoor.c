/* MODULE NAME:  stkctrl_backdoor.c
 * PURPOSE:
 *    stack control backdoor
 *
 * NOTES:
 *
 * HISTORY
 *    8/10/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include "sys_bld.h"

#include "stkctrl_backdoor.h"
#include "backdoor_mgr.h"
#include "stkctrl_pmgr.h"
#include "stkctrl_task.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void STKCTRL_BACKDOOR_Main (void);

/* STATIC VARIABLE DECLARATIONS
 */
static STKCTRL_BACKDOOR_CPNT_TIMER_T    cpnt_ready_time;
static BOOL_T stkctrl_dbgmsg_shown_flag = FALSE;
static BOOL_T stkctrl_group_dbgmsg_shown_flag = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME : STKCTRL_BACKDOOR_Create_InterCSC_Relation
 * PURPOSE: This function initializes all function pointer registration
 *          operations.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  None.
 * NOTES:
 *
 */
void STKCTRL_BACKDOOR_Create_InterCSC_Relation(void)
{
    /* Init backdoor call back functions
     */
    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("stkctrl",
        SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY, STKCTRL_BACKDOOR_Main);

}

BOOL_T STKCTRL_BACKDOOR_GetTimeInfo(STKCTRL_BACKDOOR_CPNT_TIMER_T **pCpnt_ready_time)
{
	*pCpnt_ready_time = &cpnt_ready_time;
	
	return (TRUE);
}

/* FUNCTION NAME : STKCTRL_BACKDOOR_IsStkCtrlDbgMsgOn
 * PURPOSE: This function is used to know the if the debug message of STKCTRL should be 
 *          shown pr not.
 * INPUT:   None.
 * OUTPUT:  none.
 * RETUEN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T STKCTRL_BACKDOOR_IsStkCtrlDbgMsgOn(void)
{
    return stkctrl_dbgmsg_shown_flag;
}

/* FUNCTION NAME : STKCTRL_BACKDOOR_SetStkCtrlDbgMsgOn
 * PURPOSE: This function set the flag of stkctrl_dbgmsg_shown_flag
 * INPUT:   value -- TRUE/FALSE flag
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKCTRL_BACKDOOR_SetStkCtrlDbgMsgOn()
{
    stkctrl_dbgmsg_shown_flag = !stkctrl_dbgmsg_shown_flag;
}

BOOL_T STKCTRL_GROUP_BACKDOOR_IsStkCtrlDbgMsgOn(void)
{
    return stkctrl_group_dbgmsg_shown_flag;
}

/* FUNCTION NAME : STKCTRL_GROUP_BACKDOOR_SetStkCtrlDbgMsgOn
 * PURPOSE: This function set the flag of stkctrl_dbgmsg_shown_flag
 * INPUT:   value -- TRUE/FALSE flag
 * OUTPUT:  None.
 * RETUEN:  None.          
 * NOTES:
 *          
 */
void STKCTRL_GROUP_BACKDOOR_SetStkCtrlDbgMsgOn()
{
    stkctrl_group_dbgmsg_shown_flag = !stkctrl_group_dbgmsg_shown_flag;
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: STKCTRL_BACKDOOR_Main
 * PURPOSE: 
 * INPUT:  
 * OUTPUT: 
 * RETURN: 
 * NOTES:
 *
 */
static void STKCTRL_BACKDOOR_Main (void)
{
#define MAXLINE 255
    char line_buffer[MAXLINE];
    int  select_value = 0;
    int  index;

    STKCTRL_PMGR_InitiateProcessResources();

    while(1)
    {
        
		BACKDOOR_MGR_Printf("\r\n===============================");
		BACKDOOR_MGR_Printf("\r\n  Stack Control Engineer Menu  ");
		BACKDOOR_MGR_Printf("\r\n================================");
        BACKDOOR_MGR_Printf("\r\n [1] Show time-consumed by every CPNT");
        BACKDOOR_MGR_Printf("\r\n [2] %s STKCTRL debug message", stkctrl_dbgmsg_shown_flag ? "Hide" : "Show"); /* STKCTRL_BACKDOOR */
        BACKDOOR_MGR_Printf("\r\n [3] Warm start system");
		BACKDOOR_MGR_Printf("\r\n [4] Dump Stkctrl enter transition mode performance");
		BACKDOOR_MGR_Printf("\r\n [5] Dump Stkctrl enter master mode performance");
		BACKDOOR_MGR_Printf("\r\n [6] Dump Stkctrl enter slave mode performance");
		BACKDOOR_MGR_Printf("\r\n [7] Dump Stkctrl enter provision mode performance");
		BACKDOOR_MGR_Printf("\r\n [8] Dump Stkctrl hotswap add  performance");
		BACKDOOR_MGR_Printf("\r\n [9] Dump Stkctrl hotswap removal performance");
		BACKDOOR_MGR_Printf("\r\n [10] Clear 5-9 time");
		BACKDOOR_MGR_Printf("\r\n [11] %s Set StkCtrl GROUP DbgMsg On",stkctrl_group_dbgmsg_shown_flag?"show":"hide");
        BACKDOOR_MGR_Printf("\r\n [99] Exit");
        BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

        if (BACKDOOR_MGR_RequestKeyIn(line_buffer, MAXLINE) == TRUE)
        {
            select_value = atoi(line_buffer);
            BACKDOOR_MGR_Printf("\r\nSelect value is %d", select_value);
        }

        switch(select_value)
        {
            case 1:
		    	BACKDOOR_MGR_Printf("\r\n");
		    	BACKDOOR_MGR_Printf("========report begin========\r\n");
		        for(index=0; index<cpnt_ready_time.cpnt_nbr; index++)
		        {
		        	BACKDOOR_MGR_Printf("%10s consumes %5ld ticks(10ms)\r\n", &cpnt_ready_time.cpnt_name[index][0], cpnt_ready_time.cpnt_time[index]);
		        }		        
		        BACKDOOR_MGR_Printf("========report done========\r\n");
                break;
            case 2:
                stkctrl_dbgmsg_shown_flag = !stkctrl_dbgmsg_shown_flag;
                break;
            case 3:
                STKCTRL_PMGR_WarmStartSystem();
                break;
#if (STKCTRL_DEBUG_PERFORMACE == TRUE)
			case 4:
				STKCTRL_TASK_BD_ShowTranstionPerformace();
				break;
		    case 5:
				STKCTRL_TASK_BD_ShowMasterPerformace();
				break;
			case 6:
				STKCTRL_TASK_BD_ShowSlavePerformace();
				break;
		    case 7:
				STKCTRL_TASK_BD_ShowProvisionPerformace();
				break;
		    case 8:
				STKCTRL_TASK_BD_ShowHotAddPerformace();
				break;
			case 9:
				STKCTRL_TASK_BD_ShowHotRemovePerformace();
				break;
		    case 10:
				STKCTRL_TASK_BD_ClearPerformaceDB();
				break;
#endif
            case 11:
				STKCTRL_GROUP_BACKDOOR_SetStkCtrlDbgMsgOn();
				break;
				break;

            case 99:
                BACKDOOR_MGR_Printf("\r\n Exit Stack Topology Engineer Menu");
                return;
        }
    }    
    
}

