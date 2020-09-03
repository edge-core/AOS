/*-----------------------------------------------------------------------------
 * Module Name: dcbx_backdoor.c
 *-----------------------------------------------------------------------------
 * PURPOSE: Implementations for the DCBX backdoor
 *-----------------------------------------------------------------------------
 * NOTES:
 *
 *-----------------------------------------------------------------------------
 * HISTORY:
 *    9/20/2012 - Ricky Lin, Created
 *
 *
 *-----------------------------------------------------------------------------
 * Copyright(C)                               Accton Corporation, 2012
 *-----------------------------------------------------------------------------
 */

#include "sys_bld.h"
#include "sys_type.h"
#include "l_threadgrp.h"
#include "backdoor_lib.h"
#include "backdoor_mgr.h"
#include "l2_l4_proc_comm.h"
#include "dcbx_backdoor.h"
#include "dcbx_om.h"
#include "dcbx_om_private.h"
#include "dcbx_type.h"

static L_THREADGRP_Handle_T tg_handle;
static UI32_T               backdoor_member_id;

/* ------------------------------------------------------------------------
 * ROUTINE NAME - DCBX_BACKDOOR_Main
 * ------------------------------------------------------------------------
 * FUNCTION : This function is the main routine of the backdoor
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 * ------------------------------------------------------------------------
 */

void DCBX_BACKDOOR_Main(void)
{
    int choice;

    tg_handle = L2_L4_PROC_COMM_GetDcbGroupTGHandle();

    /* Join thread group
     */
    if (L_THREADGRP_Join(tg_handle, SYS_BLD_BACKDOOR_THREAD_PRIORITY,
            &backdoor_member_id) == FALSE)
    {
        printf("\n%s: L_THREADGRP_Join fail.\n", __func__);
        return;
    }

    while (1)
    {
        BACKDOOR_MGR_Print("\r\n---------------------------------------------");
        BACKDOOR_MGR_Print("\r\n DCBX Backdoor Menu");
        BACKDOOR_MGR_Print("\r\n---------------------------------------------");
        BACKDOOR_MGR_Print("\r\n 1 - Get system operation data");
        BACKDOOR_MGR_Print("\r\n 2 - Get port config data");
        BACKDOOR_MGR_Print("\r\n x - Exit backdoor");
        BACKDOOR_MGR_Print("\r\n---------------------------------------------");
        BACKDOOR_MGR_Print("\r\n Enter your choice: ");

        choice = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c", choice);

        switch (choice)
        {
        case '1':
        {
            DCBX_OM_SystemOperEntry_T   *entry_p;

            entry_p = DCBX_OM_GetSysOper();
            BACKDOOR_MGR_Printf("\r\n is_cfg_src_selected = %hu", entry_p->is_cfg_src_selected);
            BACKDOOR_MGR_Printf("\r\n is_manual_cfg_src = %hu", entry_p->is_manual_cfg_src);
            BACKDOOR_MGR_Printf("\r\n cfg_src_ifindex = %lu", (unsigned long)entry_p->cfg_src_ifindex);
        }
            break;

        case '2':
        {
            DCBX_OM_PortConfigEntry_T   *entry_p;
            UI32_T                      lport;

            lport = BACKDOOR_LIB_RequestUI32("\r\n specify a lport", 0);
            entry_p = DCBX_OM_GetPortConfigEntryPtr(lport);
            BACKDOOR_MGR_Printf(" port_status = %hu", entry_p->port_status);
            BACKDOOR_MGR_Printf("\r\n port_mode = %lu", (unsigned long)entry_p->port_mode);
            BACKDOOR_MGR_Printf("\r\n ets_sm_state = %hu", entry_p->ets_sm_state);
            BACKDOOR_MGR_Printf("\r\n pfc_sm_state = %hu", entry_p->pfc_sm_state);
            BACKDOOR_MGR_Printf("\r\n is_peer_detected = %hu", entry_p->is_peer_detected);
        }
            break;

        case 'x':
            return;
        }

        BACKDOOR_MGR_Print("\r\n");
    } /* end while 1 */

    /* Leave thread group
     */
    L_THREADGRP_Leave(tg_handle, backdoor_member_id);
    return;
}
