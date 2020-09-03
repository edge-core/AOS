/* MODULE NAME:  sys_callback_om_private.h
 * PURPOSE:
 *  Declares the prototype of functions that can only be used within SYS_CALLBACK
 *
 * NOTES:
 *
 *    7/4/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef SYS_CALLBACK_OM_PRIVATE_H
#define SYS_CALLBACK_OM_PRIVATE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sysrsc_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
enum
{
    SYS_CALLBACK_OM_DBG_IPC_SENT            = BIT_0,
    SYS_CALLBACK_OM_DBG_IPC_FAILED          = BIT_1,
    SYS_CALLBACK_OM_DBG_IPC_DETALL          = BIT_2,
    SYS_CALLBACK_OM_DBG_IPC_DUMP            = BIT_3,
    SYS_CALLBACK_OM_DBG_IPC_COUNTING        = BIT_4,
};

enum
{
    SYS_CALLBACK_OM_ACTION_TO_OFF,
    SYS_CALLBACK_OM_ACTION_TO_ON,
    SYS_CALLBACK_OM_MAX_ACTION
};

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T dbg_flag;
    UI32_T dbg_ipc_count;
    UI32_T dbg_event_id;
    UI32_T dbg_src_id;
    UI32_T dbg_dst_id;
} SYS_CALLBACK_OM_Backdoor_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*************************
 *  SYSRSC related APIs  *
 *************************
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_OM_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Initiate system resource for SYS_CALLBACK_OM
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_OM_InitiateSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_OM_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Attach system resource for SYS_CALLBACK_OM in the context of the
 *      calling process.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_OM_AttachSystemResources(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME : SYS_CALLBACK_OM_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Provide shared memory information of SYS_CALLBACK_OM for SYSRSC.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      segid_p  --  shared memory segment id
 *      seglen_p --  length of the shared memroy segment
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void SYS_CALLBACK_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

/************************
 *  Stacking mode APIs  *
 ************************
 */

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_EnterMasterMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_EnterSlaveMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_EnterTransitionMode(void);

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_SetTransitionMode(void);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - SYS_CALLBACK_OM_GetAndClearFailInfo
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get and clear the information about failed sender.
 * INPUT    :   cscgroup_mgr_msgqkey   -  The mgr msgq key of the csc group who
 *                                        retrieves its callback information
 * OUTPUT   :   fail_entry_p - The buffer to store the retrieval information.
 * RETURN   :   TRUE  - Successful.
 *              FALSE - Failed.
 * NOTES    :
 *            1.The csc who fails to deliver the callback message will be kept
 *              in fail_entry_p->csc_list. The repeated failure from same csc
 *              will occupy only one entry in fail_entry_p->csc_list.
 *            2.fail_entry_p->csc_list_counter indicate the number of csc contained
 *              in csc_list. Note that if fail_entry_p->csc_list is too small to
 *              keep all cscs who fails to deliver. fail_entry_p->csc_list_counter
 *              will be set as SYS_CALLBACK_MGR_CSC_LIST_OVERFLOW. Caller should
 *              assume that all cscs that will send callback have ever failed to
 *              delivery at least once.
 *            3.private om API
 * ------------------------------------------------------------------------
 */
BOOL_T SYS_CALLBACK_OM_GetAndClearFailInfo(UI32_T cscgroup_mgr_msgqkey, SYS_CALLBACK_OM_FailEntry_T *fail_entry_p);

/* FUNCTION NAME : SYS_CALLBACK_OM_UpdateFailEntry
 * PURPOSE:
 *      Update the fail entry for the specified cscgroup.
 *
 * INPUT:
 *      cscgroup_mgr_msgqkey -- the mgr message queue key of the csc group which
 *                              fails to receive callback event
 *      sender_csc_id        -- the sender of the callback event
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 */
void SYS_CALLBACK_OM_UpdateFailEntry(UI32_T cscgroup_mgr_msgqkey, UI32_T sender_csc_id);

/* FUNCTION NAME : SYS_CALLBACK_OM_ResetAllFailEntries
 * PURPOSE:
 *      Reset all fail entries to initialized state.
 *
 * INPUT:
 *      fail_entry -- The entry to be reset
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 */
void SYS_CALLBACK_OM_ResetAllFailEntries(void);

/* FUNCTION NAME : SYS_CALLBACK_OM_SetDebugFlag
 * PURPOSE:
 *      enable/disable debug flag
 *
 * INPUT:
 *      debug_flag  -- SYS_CALLBACK_OM_DBG_xxx
 *      enable      -- TRUE/FALSE
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 */
void SYS_CALLBACK_OM_SetDebugFlag(UI32_T debug_flag, BOOL_T enable);

/* FUNCTION NAME : SYS_CALLBACK_OM_SetDebugFlag
 * PURPOSE:
 *      get status of debug flag
 *
 * INPUT:
 *      debug_flag  -- SYS_CALLBACK_OM_DBG_xxx
 *      match_all   -- TRUE to match all; FALSE to match any
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      private om API
 */
BOOL_T SYS_CALLBACK_OM_IsDebugFlagOn(UI32_T debug_flag, BOOL_T match_all);

/* FUNCTION NAME : SYS_CALLBACK_OM_GetBackdoorDb
 * PURPOSE:
 *      get database of backdoor
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      pointer of backdoor db buffer.
 *
 * NOTES:
 *      private om API
 */
SYS_CALLBACK_OM_Backdoor_T *SYS_CALLBACK_OM_GetBackdoorDb(void);

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
BOOL_T SYS_CALLBACK_OM_SetChange_0(UI32_T kind);
BOOL_T SYS_CALLBACK_OM_SetChange_1(UI32_T kind, UI32_T arg1);
BOOL_T SYS_CALLBACK_OM_SetChange_2(UI32_T kind, UI32_T arg1, UI32_T arg2);
BOOL_T SYS_CALLBACK_OM_SetChange_3(UI32_T kind, UI32_T arg1, UI32_T arg2,
        UI32_T arg3);
#endif

#endif /* End of SYS_CALLBACK_OM_PRIVATE_H */
