/*------------------------------------------------------------------------
 * Module Name  :   udphelper_backdoor.c
 *-------------------------------------------------------------------------
 * Purpose      :   This file supports a backdoor for the UDPHELPER
 *-------------------------------------------------------------------------
 * Notes:
 * History:
 *    
 *
 *-------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2009
 *-------------------------------------------------------------------------
 */
#include "udphelper_backdoor.h"

#if (UDPHELPER_SUPPORT_ACCTON_BACKDOOR == TRUE)

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "backdoor_mgr.h"
#include "l_inet.h"
#include "ip_lib.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "udphelper_om.h"
#include "udphelper_vm.h"
#include "udphelper_mgr.h"
#include "udphelper_type.h"
#include "udphelper_backdoor.h"

static L_THREADGRP_Handle_T tg_udphelper_handle;
static UI32_T               backdoor_udphelper_id;



/* ---------------------------------------------------------------------
 * common functions
 * --------------------------------------------------------------------- */
static void UDPHELPER_BackDoor_Main(void);
static void UDPHELPER_BackDoor_DisplayMainMenu(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  UDPHELPER_Backdoor_CallBack
 *-------------------------------------------------------------------------
 * PURPOSE  : UDPHELPER backdoor callback function
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void UDPHELPER_Backdoor_CallBack()
{
    tg_udphelper_handle = L2_L4_PROC_COMM_GetUdphelperGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_udphelper_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_udphelper_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_Printf("UDPHELPER Backdoor!!\n");
    UDPHELPER_BackDoor_Main();

    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_udphelper_handle, backdoor_udphelper_id);
    return;
}


/* Local Subprogram Definition
 */
/* ------------------------------------------------------------------------
 * ROUTINE NAME - UDPHELPER_BackDoor_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function ititiates the backdoor function
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
static void UDPHELPER_BackDoor_Main(void)
{
    int     ch;
    BOOL_T  eof = FALSE;
    char    buf[18];
    char    *terminal;
    
    /* BODY
     */
    while(!eof)
    {
        UDPHELPER_BackDoor_DisplayMainMenu();

        BACKDOOR_MGR_RequestKeyIn(buf, 15);
        ch = (int)strtoul(buf, &terminal, 10);
        BACKDOOR_MGR_Printf("\n");

        switch(ch) 
        {
            case 0: 
                eof = TRUE;               
                break;
                
            case 1:
                UDPHELPER_OM_SetDebugStatus(TRUE);
                UDPHELPER_VM_SetDebugStatus(TRUE);
                break;
                
            case 2:
                UDPHELPER_OM_SetDebugStatus(FALSE);
                UDPHELPER_VM_SetDebugStatus(FALSE);
                break;
                
            case 3:

                break;
                
			case 4:

				break;
                
            case 5: 

                break;
                
            case 6:

                break;
                
            case 7:

                break;
                
            case 8:

                break;
                
            case 9:

                break;
                
            case 10:

                break;
                
            case 11:

                break;
                
            case 12:

                break;
                
            case 13:

                break;
                
            default:
                break;
        } /* end of switch */   
    } /* end of while */
    return;
} /* end of UDPHELPER_BackDoor_Main() */

static void UDPHELPER_BackDoor_DisplayMainMenu(void)
{
    BACKDOOR_MGR_Printf("\n");
    BACKDOOR_MGR_Printf(" 0. Exit\n");
    BACKDOOR_MGR_Printf(" 1. Enable debug UDPHELPER\n");
    BACKDOOR_MGR_Printf(" 2. Disable debug UDPHELPER\n");
    BACKDOOR_MGR_Printf(" 3. \n");
    BACKDOOR_MGR_Printf(" 4. \n");
    BACKDOOR_MGR_Printf(" 5. \n");
    BACKDOOR_MGR_Printf(" 6. \n");
    BACKDOOR_MGR_Printf(" 7. \n");
    BACKDOOR_MGR_Printf(" 8. \n");
    BACKDOOR_MGR_Printf(" 9. \n");
    BACKDOOR_MGR_Printf("10. \n");
    BACKDOOR_MGR_Printf("11. \n");
    BACKDOOR_MGR_Printf("12. \n");
    BACKDOOR_MGR_Printf("13. \n");
    BACKDOOR_MGR_Printf("     select = ");
    return;
} /* end of UDPHELPER_BackDoor_DisplayMainMenu() */


#endif

