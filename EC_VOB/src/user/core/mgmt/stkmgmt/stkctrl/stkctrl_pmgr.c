/* MODULE NAME:  stkctrl_pmgr.c
 * PURPOSE:
 * stkctrl pmgr
 *
 * NOTES:
 *
 * HISTORY
 *    08/01/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
 
/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun.h"

#include "stkctrl_task.h"
#include "stkctrl_pmgr.h"
#include "stktplg_pom.h"

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
static SYSFUN_MsgQ_T msgq_handle;

/* EXPORTED SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_InitiateProcessResources
 * ---------------------------------------------------------------------
 * PURPOSE: This function initializes STKCTRL PMGR for the calling
 *          process
 * INPUT:   None.
 * OUTPUT:  None.
 * RETUEN:  TRUE         -- successful.
 *          FALSE        -- unspecified failure.
 * NOTES:
 * ---------------------------------------------------------------------
 */
BOOL_T STKCTRL_PMGR_InitiateProcessResources(void)
{
    if(SYSFUN_OK!=SYSFUN_GetMsgQ(SYS_BLD_STKCTRL_GROUP_IPCMSGQ_KEY,
        SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle))
    {
        printf("\r\n%s:SYSFUN_GetMsgQ fail.", __FUNCTION__);
        return FALSE;
    }

    /* This module will call STKTPLG_POM. So we need to initiate it here
     * to ensure STKTPLG_POM can work.
     */
    if(FALSE==STKTPLG_POM_InitiateProcessResources())
    {
        printf("\r\n%s:STKTPLG_POM_InitiateProcessResources fail.", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_ReloadSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send reload event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_ReloadSystem(void)
{
    UI32_T task_id,rc;

    do{
        task_id=SYSFUN_GetMsgQOwner(msgq_handle);
        /* when the owner of the msgq hasn't be created, take a snap and retry
         * latter
         */
        if(task_id==0)
        {
            SYSFUN_Sleep(5);
        }
    }while(task_id==0);

    if(SYSFUN_OK!=(rc=SYSFUN_SendEvent(task_id, STKCTRL_TASK_EVENT_RELOAD_SYSTEM)))
    {
        printf("\r\n%s:send event fail.(ret=%d)", __FUNCTION__, (int)rc);
    }

}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_WarmStartSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send warm start event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_WarmStartSystem(void)
{
    UI32_T task_id,rc;

    do{
        task_id=SYSFUN_GetMsgQOwner(msgq_handle);
        /* when the owner of the msgq hasn't be created, take a snap and retry
         * latter
         */
        if(task_id==0)
        {
            SYSFUN_Sleep(5);
        }
    }while(task_id==0);

    if(SYSFUN_OK!=(rc=SYSFUN_SendEvent(task_id, STKCTRL_TASK_EVENT_WARM_START_SYSTEM)))
    {
        printf("\r\n%s:send event fail.(ret=%d)", __FUNCTION__, (int)rc);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_ColdStartSystem
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send cold start event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
void STKCTRL_PMGR_ColdStartSystem(void)
{
    UI32_T task_id,rc;

    do{
        task_id=SYSFUN_GetMsgQOwner(msgq_handle);
        /* when the owner of the msgq hasn't be created, take a snap and retry
         * latter
         */
        if(task_id==0)
        {
            SYSFUN_Sleep(5);
        }
    }while(task_id==0);

    if(SYSFUN_OK!=(rc=SYSFUN_SendEvent(task_id, STKCTRL_TASK_EVENT_COLD_START_SYSTEM)))
    {
        printf("\r\n%s:send event fail.(ret=%d)", __FUNCTION__, (int)rc);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - STKCTRL_PMGR_UnitIDReNumbering
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will send renumbering event to STKCTRL task.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : TRUE  -- Renumbering work (not standalone or unit ID not 
 *                     equal to 1)
 *            FALSE -- otherwise
 * NOTES    : This API will not send BI-DIRECTIONAL message. It is okay
 *            to be called by CSCs which lies in the layer lower than
 *            STKCTRL.
 * ---------------------------------------------------------------------
 */
BOOL_T STKCTRL_PMGR_UnitIDReNumbering(void)
{
    UI32_T task_id,rc;
    UI32_T  my_unit_id, stacking_state;

    STKTPLG_POM_GetMyUnitID(&my_unit_id);
    STKTPLG_POM_GetStackingState(&stacking_state);

    /* If standalone and unit ID = 1, then there is no need to renumber.
     * For example, there may be a special model of product (ES4648C-32),
     * which shares a universal images with stackings, but
     * only supports standalone mode.
     *
     * Otherwise, we need to renumber.
     */
    if (! ((stacking_state == STKTPLG_STATE_STANDALONE) && (my_unit_id == 1)))
    {
        do{
            task_id=SYSFUN_GetMsgQOwner(msgq_handle);
            /* when the owner of the msgq hasn't be created, take a snap and retry
             * latter
             */
            if(task_id==0)
            {
                SYSFUN_Sleep(5);
            }
        }while(task_id==0);

        if(SYSFUN_OK!=(rc=SYSFUN_SendEvent(task_id, STKCTRL_TASK_EVENT_UNIT_ID_RENUMBER)))
        {
            printf("\r\n%s:send event fail.(ret=%d)", __FUNCTION__, (int)rc);
        }

        return TRUE;
    }

    return FALSE;

}

/* LOCAL SUBPROGRAM BODIES
 */

