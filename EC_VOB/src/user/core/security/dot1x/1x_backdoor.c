#include "stdio.h"
#include "sys_cpnt.h"
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "string.h"
#include "stdlib.h"
#include "1x_om.h"
#include "psec_mgr.h"
#include "l_ipcio.h"
#include "l_threadgrp.h"
#include "l2_l4_proc_comm.h"
#include "backdoor_mgr.h"
#include "1x_common.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSLOG_FILE_1   "$logfile_1"
#define SYSLOG_FILE_2   "$logfile_2"

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void     DOT1X_BACKDOOR_SetDebugMessage(BOOL_T flage);

/* buffer for doing ipcio
 */
static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

/* FUNCTION NAME: SYSLOG_BACKDOOR_Main
 * PURPOSE:
 * INPUT:
 * OUTPUT:
 * RETURN:
 * NOTES:
 *
 */
void DOT1X_BACKDOOR_Main (void)
{
#define MAXLINE 80
#define MIN_DUMMY_FILE    1
#define MAX_DUMMY_FILE    2

    char ipcio_buffer[MAXLINE+1];
    int  select_value = 0;

    tg_handle = (L_THREADGRP_Handle_T)L2_L4_PROC_COMM_GetNetaccessGroupTGHandle();

    /* Join thread group
     */
    if(L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY, &backdoor_member_id)==FALSE)
    {
        printf("%s: L_THREADGRP_Join fail.\n", __FUNCTION__);
        return;
    }

    while(1)
    {
        /* Get System Log Status */
        BACKDOOR_MGR_Printf("\r\nPress <enter> to continue.");
        BACKDOOR_MGR_RequestKeyIn( ipcio_buffer, sizeof(ipcio_buffer));

		BACKDOOR_MGR_Printf("\r\n=======================================");
		BACKDOOR_MGR_Printf("\r\n  802.1x Engineer Menu 2001/10/18  ");
		BACKDOOR_MGR_Printf("\r\n=======================================");

        BACKDOOR_MGR_Printf("\r\n [1] Open Debug Message .");
        BACKDOOR_MGR_Printf("\r\n [2] Close Debug Message .");

        BACKDOOR_MGR_Printf("\r\n [99] Exit 802.1x Engineer Menu!!");
        BACKDOOR_MGR_Printf("\r\n Enter Selection: ");

        BACKDOOR_MGR_RequestKeyIn( ipcio_buffer, sizeof(ipcio_buffer));

        if (strlen(ipcio_buffer) > 0)
        {
            select_value = atoi(ipcio_buffer);
            snprintf(ipcio_buffer,sizeof(ipcio_buffer),"\r\nSelect value is %d",select_value);
            ipcio_buffer[sizeof(ipcio_buffer)-1]='\0';
            BACKDOOR_MGR_Printf(ipcio_buffer ); /* Debug message */
        }

        switch(select_value)
        {
            case 1:
                DOT1X_BACKDOOR_SetDebugMessage(TRUE);
                break;
            case 2:
                DOT1X_BACKDOOR_SetDebugMessage(FALSE);
                break;

            case 99:
                BACKDOOR_MGR_Printf("\r\n Exit 802.1x Engineer Menu");
                L_THREADGRP_Leave(tg_handle, backdoor_member_id);
                return;
        }
    }

} /* End of SYSLOG_BACKDOOR_Main */

static void DOT1X_BACKDOOR_SetDebugMessage(BOOL_T flag)
{
    UI32_T auth_flag = 0xFFFFFFFF;

    if(flag == TRUE)
    {
        /* not open supplicant debug flags
        dbg_flag &= ~(MESS_DBG_SUP_CONFIG | MESS_DBG_SUP_STATE
                            | MESS_DBG_SUP_TIMER | MESS_DBG_SUP_ERROR | MESS_DBG_SUP_ALL);
         */

        BACKDOOR_MGR_Printf("\nStar show DOT1X Debug message\n");
        DOT1X_OM_SetDebugFlag(auth_flag);
    }
    else
    {
        BACKDOOR_MGR_Printf("\nStop show DOT1X Debug message\n");
        DOT1X_OM_SetDebugFlag(0);
    }
}

