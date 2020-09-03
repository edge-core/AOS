/* MODULE NAME:  sys_callback_om.c
 * PURPOSE:
 *     OM of SYS_CALLBACK
 *
 * NOTES:
 *
 * HISTORY
 *    7/4/2007 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sysfun.h"
#include "sysrsc_mgr.h"
#include "sys_bld.h"
#include "sys_callback_om.h"
#include "sys_callback_om_private.h"
#include "l_cvrt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define NUM_OF_CSCGROUP (SYS_BLD_CSCGROUP_IPCMSGQ_KEY_MAX-SYS_BLD_CSCGROUP_IPCMSGQ_KEY_BASE)
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
#define SIZE_OF_VLAN_BITMAP     ((SYS_ADPT_MAX_VLAN_ID + 7) / 8)
#define SIZE_OF_VLAN_SM_BITMAP  ((SYS_ADPT_MAX_VLAN_ID * 2 + 7) / 8)
enum
{
    SYS_CALLBACK_OM_STATE_NO_CHANGE,
    SYS_CALLBACK_OM_STATE_OFF_CHANGE,
    SYS_CALLBACK_OM_STATE_ON_CHANGE,
    SYS_CALLBACK_OM_STATE_ON_CHANGE_MERGE,
    SYS_CALLBACK_OM_MAX_STATE
};
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/* MACRO FUNCTION DECLARATIONS
 */
/* MACRO FUNCTION NAME : SYS_CALLBACK_OM_ResetFailEntry
 * PURPOSE:
 *      Reset the fail entry to initialized state.
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
 *      None.
 */
#define SYS_CALLBACK_OM_ResetFailEntry(fail_entry_p) (fail_entry_p)->csc_list_counter=0

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
/* the smallest acceptable key value is 1; each key uses two bits of bitmap */
#define SYS_CALLBACK_OM_BITMAP_GET(bitmap, key)                     \
    ((bitmap[(key - 1) >> 2] >> ((3 - ((key - 1) & 3)) * 2)) & 3)
#define SYS_CALLBACK_OM_BITMAP_CLEAR(bitmap, key)                   \
    (bitmap[(key - 1) >> 2] &= ~(3 << ((3 - ((key - 1) & 3)) * 2)))
#define SYS_CALLBACK_OM_FSM(bitmap, key, action)                        \
{                                                                       \
    UI8_T   state = SYS_CALLBACK_OM_BITMAP_GET(bitmap, key);            \
                                                                        \
    state = state_trasition_table[state][action];                       \
    bitmap[(key - 1) >> 2] &= ~(3 << ((3 - ((key - 1) & 3)) * 2));      \
    bitmap[(key - 1) >> 2] |= (state << ((3 - ((key - 1) & 3)) * 2));   \
}
#define SYS_CALLBACK_OM_BITMAP_TO_DATA(bitmap, key, state, change, merge)   \
{                                                                           \
    state = change = merge = FALSE;                                         \
    switch (SYS_CALLBACK_OM_BITMAP_GET(bitmap, key))                        \
    {                                                                       \
    case SYS_CALLBACK_OM_STATE_ON_CHANGE_MERGE:                             \
        merge = TRUE;                                                       \
    case SYS_CALLBACK_OM_STATE_ON_CHANGE:                                   \
        state = TRUE;                                                       \
    case SYS_CALLBACK_OM_STATE_OFF_CHANGE:                                  \
        change = TRUE;                                                      \
    }                                                                       \
}
#define SYS_CALLBACK_OM_BITMAP_TO_BITMAP(sm_bitmap, state_bitmap,           \
            change_bitmap, merge_bitmap)                                    \
{                                                                           \
    UI32_T  i, j;                                                           \
                                                                            \
    memset(state_bitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST); \
    memset(change_bitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);\
    memset(merge_bitmap, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST); \
    for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST; i++)     \
    {                                                                       \
        if (sm_bitmap[i] == 0)                                              \
        {                                                                   \
            continue;                                                       \
        }                                                                   \
                                                                            \
        for (j = 0; j < 4; j++)                                             \
        {                                                                   \
            switch ((sm_bitmap[i] >> ((3 - j) << 1)) & 3)                   \
            {                                                               \
            case SYS_CALLBACK_OM_STATE_ON_CHANGE_MERGE:                     \
                L_CVRT_ADD_MEMBER_TO_PORTLIST(merge_bitmap, i*4+j+1);       \
            case SYS_CALLBACK_OM_STATE_ON_CHANGE:                           \
                L_CVRT_ADD_MEMBER_TO_PORTLIST(state_bitmap, i*4+j+1);       \
            case SYS_CALLBACK_OM_STATE_OFF_CHANGE:                          \
                L_CVRT_ADD_MEMBER_TO_PORTLIST(change_bitmap, i*4+j+1);      \
                break;                                                      \
            default:                                                        \
                L_CVRT_DEL_MEMBER_FROM_PORTLIST(change_bitmap, i*4+j+1);    \
            }                                                               \
        }                                                                   \
    }                                                                       \
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/* DATA TYPE DECLARATIONS
 */
/* SYS_CALLBACK_OM_Shmem_Data_T is the data type used on the shared memory.
 * It contains a table to keep the information about failure of callback message
 * delivery. Each CSC Group owns an entry in cbfail_table.
 */
typedef struct
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    SYS_CALLBACK_OM_FailEntry_T cbfail_table[NUM_OF_CSCGROUP];
    SYS_CALLBACK_OM_Backdoor_T  backdoor_db;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
    UI32_T  cmgr_thread_id;

    UI8_T   change_bitmap[(SYS_CALLBACK_OM_MAX_KIND+7)/8];

    UI8_T   vlan_sm[SIZE_OF_VLAN_SM_BITMAP];
    UI8_T   vlan_member_sm[SYS_ADPT_MAX_VLAN_ID][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST];

    UI8_T   gvrp_vlan_sm[SIZE_OF_VLAN_SM_BITMAP];
    UI8_T   gvrp_vlan_member_sm[SYS_ADPT_MAX_VLAN_ID][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST];

    UI8_T   l3_vlan_destroy[SIZE_OF_VLAN_BITMAP];
    UI8_T   l3_if_oper_status_change[SIZE_OF_VLAN_BITMAP];

    UI8_T   port_vlan_change[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    UI8_T   port_vlan_mode_change[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    UI8_T   pvid_change[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
    UI32_T  old_pvid_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI32_T  new_pvid_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT];

    UI8_T   vlan_name_change[SIZE_OF_VLAN_BITMAP];

    UI8_T   protocol_vlan_change[SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    UI8_T   vlan_member_tag_change[SYS_ADPT_MAX_VLAN_ID][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];

    UI8_T   xstp_port_sm[SYS_ADPT_MAX_NBR_OF_MST_INSTANCE][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST];

    BOOL_T  xstp_version_change;

    UI8_T   xstp_port_topo_change[SYS_ADPT_MAX_NBR_OF_MST_INSTANCE][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST];
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
} SYS_CALLBACK_OM_Shmem_Data_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYS_CALLBACK_OM_InitBackdoorDb(void);
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T SYS_CALLBACK_OM_FindNextVlan(UI8_T *vlan_bitmap,
    UI8_T member_bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST],
    UI32_T *key_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_1(UI8_T *bitmap, UI32_T size,
    UI32_T *key_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_2(UI8_T *bitmap, UI32_T size,
    UI32_T *key_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_3(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_4(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_5(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p);
static BOOL_T SYS_CALLBACK_OM_FindNextKey_8(UI8_T *bitmap1, UI8_T *bitmap2,
    UI32_T size, UI32_T *key_p);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static SYS_CALLBACK_OM_Shmem_Data_T* shmem_data_p;
static UI32_T shmem_sem_id;
static UI32_T orig_priority;
#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
/*  state \ action  | TO_OFF     | TO_ON
 * ------------------------------|-----------------
 * NO_CHANGE        | OFF_CHANGE | ON_CHANGE
 * OFF_CHANGE       | OFF_CHANGE | ON_CHANGE_MERGE
 * ON_CHANGE        | NO_CHANGE  | ON_CHANGE
 * ON_CHANGE_MERGE  | OFF_CHANGE | ON_CHANGE_MERGE
 */
static UI8_T state_trasition_table[SYS_CALLBACK_OM_MAX_STATE][SYS_CALLBACK_OM_MAX_ACTION] =
{
    {SYS_CALLBACK_OM_STATE_OFF_CHANGE,  SYS_CALLBACK_OM_STATE_ON_CHANGE},
    {SYS_CALLBACK_OM_STATE_OFF_CHANGE,  SYS_CALLBACK_OM_STATE_ON_CHANGE_MERGE},
    {SYS_CALLBACK_OM_STATE_NO_CHANGE,   SYS_CALLBACK_OM_STATE_ON_CHANGE},
    {SYS_CALLBACK_OM_STATE_OFF_CHANGE,  SYS_CALLBACK_OM_STATE_ON_CHANGE_MERGE}
};
#endif

/* EXPORTED SUBPROGRAM BODIES
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
void SYS_CALLBACK_OM_InitiateSystemResources(void)
{
    shmem_data_p = (SYS_CALLBACK_OM_Shmem_Data_T*) SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYS_CALLBACK_SHMEM_SEGID);

    memset(shmem_data_p, 0, sizeof(SYS_CALLBACK_OM_Shmem_Data_T));
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    SYS_CALLBACK_OM_ResetAllFailEntries();
    SYS_CALLBACK_OM_InitBackdoorDb();
}

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
void SYS_CALLBACK_OM_AttachSystemResources(void)
{
    shmem_data_p = (SYS_CALLBACK_OM_Shmem_Data_T*) SYSRSC_MGR_GetShMem(SYSRSC_MGR_SYS_CALLBACK_SHMEM_SEGID);
    if(SYSFUN_OK!=SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SYS_CALLBACK_OM, &shmem_sem_id))
        printf("%s(): SYSFUN_GetSem fails.\n", __FUNCTION__);
}

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
void SYS_CALLBACK_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_SYS_CALLBACK_SHMEM_SEGID;
    *seglen_p = sizeof(SYS_CALLBACK_OM_Shmem_Data_T);
}

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
void SYS_CALLBACK_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter slave mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Let SYS_CALLBACK enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    SYS_CALLBACK_OM_ResetAllFailEntries();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Set SYS_CALLBACK to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
void SYS_CALLBACK_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SYS_CALLBACK_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get current SYS_CALLBACK operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  current operating mode
 * NOTES:   private om API
 *---------------------------------------------------------------------------------*/
SYS_TYPE_Stacking_Mode_T SYS_CALLBACK_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}


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
BOOL_T SYS_CALLBACK_OM_GetAndClearFailInfo(UI32_T cscgroup_mgr_msgqkey, SYS_CALLBACK_OM_FailEntry_T *fail_entry_p)
{
    if((cscgroup_mgr_msgqkey>=SYS_BLD_CSCGROUP_IPCMSGQ_KEY_MAX) || fail_entry_p==NULL)
        return FALSE;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(shmem_sem_id);
    memcpy(fail_entry_p, &(shmem_data_p->cbfail_table[cscgroup_mgr_msgqkey - SYS_BLD_CSCGROUP_IPCMSGQ_KEY_BASE]),
        sizeof(SYS_CALLBACK_OM_FailEntry_T));
    SYS_CALLBACK_OM_ResetFailEntry(&(shmem_data_p->cbfail_table[cscgroup_mgr_msgqkey - SYS_BLD_CSCGROUP_IPCMSGQ_KEY_BASE]));
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
    return TRUE;
}

/* review note: SYS_CALLBACK_OM_UpdateFailEntry review done only
 */
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
void SYS_CALLBACK_OM_UpdateFailEntry(UI32_T cscgroup_mgr_msgqkey, UI32_T sender_csc_id)
{
    UI32_T i;
    SYS_CALLBACK_OM_FailEntry_T *entry_p;

    if(cscgroup_mgr_msgqkey>=SYS_BLD_CSCGROUP_IPCMSGQ_KEY_MAX)
    {
        SYSFUN_Debug_Printf("%s(): Invalid msgqkey(%d)\n", __FUNCTION__, cscgroup_mgr_msgqkey);
        return;
    }

    entry_p = &(shmem_data_p->cbfail_table[cscgroup_mgr_msgqkey-SYS_BLD_CSCGROUP_IPCMSGQ_KEY_BASE]);

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(shmem_sem_id);
    if(entry_p->csc_list_counter == SYS_CALLBACK_OM_CSC_LIST_OVERFLOW)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
        return;
    }

    /* check whether the csc id alreay exists in the list
     */
    for(i=0; i<entry_p->csc_list_counter; i++)
    {
        if(entry_p->csc_list[i]==sender_csc_id)
        {
            /* csc id already in csc_list
             */
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
            return;
        }
    }

    if(entry_p->csc_list_counter<SYS_CALLBACK_OM_CALLBACK_FAIL_MAX_NBR_OF_CSC_LIST)
    {
        entry_p->csc_list[entry_p->csc_list_counter] = sender_csc_id;
        entry_p->csc_list_counter++;
    }
    else
    {
        entry_p->csc_list_counter=SYS_CALLBACK_OM_CSC_LIST_OVERFLOW;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);

}

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
void SYS_CALLBACK_OM_ResetAllFailEntries(void)
{
    UI32_T i;

    for(i=1;i<NUM_OF_CSCGROUP; i++)
    {
        SYS_CALLBACK_OM_ResetFailEntry(&(shmem_data_p->cbfail_table[i]));
    }
}

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
void SYS_CALLBACK_OM_SetDebugFlag(UI32_T debug_flag, BOOL_T enable)
{
    if (enable)
    {
        shmem_data_p->backdoor_db.dbg_flag |= debug_flag;
    }
    else
    {
        shmem_data_p->backdoor_db.dbg_flag &= ~ debug_flag;
    }
}

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
BOOL_T SYS_CALLBACK_OM_IsDebugFlagOn(UI32_T debug_flag, BOOL_T match_all)
{
    BOOL_T is_matched;

    if (match_all)
    {
        is_matched = ((shmem_data_p->backdoor_db.dbg_flag & debug_flag) == debug_flag);
    }
    else
    {
        is_matched = ((shmem_data_p->backdoor_db.dbg_flag & debug_flag) != 0);
    }

    return is_matched;
}

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
SYS_CALLBACK_OM_Backdoor_T *SYS_CALLBACK_OM_GetBackdoorDb(void)
{
    return &shmem_data_p->backdoor_db;
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
void SYS_CALLBACK_OM_SetCmgrThreadId(UI32_T tid)
{
    shmem_data_p->cmgr_thread_id = tid;
}

UI32_T SYS_CALLBACK_OM_GetCmgrThreadId()
{
    return shmem_data_p->cmgr_thread_id;
}

BOOL_T SYS_CALLBACK_OM_SetChange_0(UI32_T kind)
{
    return SYS_CALLBACK_OM_SetChange_3(kind, 0, 0, 0);
}

BOOL_T SYS_CALLBACK_OM_SetChange_1(UI32_T kind, UI32_T arg1)
{
    return SYS_CALLBACK_OM_SetChange_3(kind, arg1, 0, 0);
}

BOOL_T SYS_CALLBACK_OM_SetChange_2(UI32_T kind, UI32_T arg1, UI32_T arg2)
{
    return SYS_CALLBACK_OM_SetChange_3(kind, arg1, arg2, 0);
}

BOOL_T SYS_CALLBACK_OM_SetChange_3(UI32_T kind, UI32_T arg1, UI32_T arg2,
        UI32_T arg3)
{
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(shmem_sem_id);

    switch (kind)
    {
    case SYS_CALLBACK_OM_KIND_VLAN:
        SYS_CALLBACK_OM_FSM(shmem_data_p->vlan_sm, arg1, arg2);
        if (arg2 == SYS_CALLBACK_OM_ACTION_TO_OFF)
        {
            memset(shmem_data_p->vlan_member_sm[arg1-1], 0,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST);
        }
        break;

    case SYS_CALLBACK_OM_KIND_VLAN_MEMBER:
        SYS_CALLBACK_OM_FSM(shmem_data_p->vlan_member_sm[arg1-1], arg2, arg3);
        kind = SYS_CALLBACK_OM_KIND_VLAN;
        break;

    case SYS_CALLBACK_OM_KIND_GVRP_VLAN:
        SYS_CALLBACK_OM_FSM(shmem_data_p->gvrp_vlan_sm, arg1, arg2);
        if (arg2 == SYS_CALLBACK_OM_ACTION_TO_OFF)
        {
            memset(shmem_data_p->gvrp_vlan_member_sm[arg1-1], 0,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST);
        }
        break;

    case SYS_CALLBACK_OM_KIND_GVRP_VLAN_MEMBER:
        SYS_CALLBACK_OM_FSM(shmem_data_p->gvrp_vlan_member_sm[arg1-1], arg2,
            arg3);
        kind = SYS_CALLBACK_OM_KIND_GVRP_VLAN;
        break;

    case SYS_CALLBACK_OM_KIND_L3_VLAN:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->l3_vlan_destroy, arg1);
        L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->l3_if_oper_status_change,
            arg1);
        break;

    case SYS_CALLBACK_OM_KIND_L3_IF_OPER_STATUS:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->l3_if_oper_status_change,
            arg1);
        kind = SYS_CALLBACK_OM_KIND_L3_VLAN;
        break;

    case SYS_CALLBACK_OM_KIND_PORT_VLAN:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->port_vlan_change, arg1);
        break;

    case SYS_CALLBACK_OM_KIND_PORT_VLAN_MODE:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->port_vlan_mode_change,
            arg1);
        break;

    case SYS_CALLBACK_OM_KIND_PVID:
        if (!L_CVRT_IS_MEMBER_OF_PORTLIST(shmem_data_p->pvid_change, arg1))
        {
            L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->pvid_change, arg1);
            shmem_data_p->old_pvid_ar[arg1-1] = arg2;
        }
        shmem_data_p->new_pvid_ar[arg1-1] = arg3;
        break;

    case SYS_CALLBACK_OM_KIND_VLAN_NAME:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->vlan_name_change, arg1);
        break;

    case SYS_CALLBACK_OM_KIND_PROTOCOL_VLAN:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->protocol_vlan_change, arg1);
        break;

    case SYS_CALLBACK_OM_KIND_VLAN_MEMBER_TAG:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(
            shmem_data_p->vlan_member_tag_change[arg1-1], arg2);
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE:
        SYS_CALLBACK_OM_FSM(shmem_data_p->xstp_port_sm[arg1], arg2, arg3);
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_VERSION:
        shmem_data_p->xstp_version_change = TRUE;
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_PORT_TOPO:
        L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->xstp_port_topo_change[arg1],
            arg2);
        break;

    default:
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
        return FALSE;
    }

    L_CVRT_ADD_MEMBER_TO_PORTLIST(shmem_data_p->change_bitmap, kind+1);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
    return TRUE;
}

BOOL_T SYS_CALLBACK_OM_GetAndResetNextChange(UI32_T kind, UI32_T *key_p,
        void *buffer_p)
{
    BOOL_T  result;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(shmem_sem_id);

    if (!L_CVRT_IS_MEMBER_OF_PORTLIST(shmem_data_p->change_bitmap, kind+1))
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
        return FALSE;
    }

    result = FALSE;
    switch (kind)
    {
    case SYS_CALLBACK_OM_KIND_VLAN:
        if (SYS_CALLBACK_OM_FindNextVlan(shmem_data_p->vlan_sm,
                shmem_data_p->vlan_member_sm, key_p) == TRUE)
        {
            SYS_CALLBACK_OM_Vlan_T  *data_p;

            data_p = (SYS_CALLBACK_OM_Vlan_T*)buffer_p;
            SYS_CALLBACK_OM_BITMAP_TO_DATA(shmem_data_p->vlan_sm, *key_p,
                data_p->vlan_state, data_p->vlan_change, data_p->vlan_merge);
            SYS_CALLBACK_OM_BITMAP_CLEAR(shmem_data_p->vlan_sm, *key_p);
            SYS_CALLBACK_OM_BITMAP_TO_BITMAP(
                shmem_data_p->vlan_member_sm[*key_p-1], data_p->member_state,
                data_p->member_change, data_p->member_merge);
            memset(shmem_data_p->vlan_member_sm[*key_p-1], 0,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_GVRP_VLAN:
        if (SYS_CALLBACK_OM_FindNextVlan(shmem_data_p->gvrp_vlan_sm,
                shmem_data_p->gvrp_vlan_member_sm, key_p) == TRUE)
        {
            SYS_CALLBACK_OM_Vlan_T  *data_p;

            data_p = (SYS_CALLBACK_OM_Vlan_T*)buffer_p;
            SYS_CALLBACK_OM_BITMAP_TO_DATA(shmem_data_p->gvrp_vlan_sm, *key_p,
                data_p->vlan_state, data_p->vlan_change, data_p->vlan_merge);
            SYS_CALLBACK_OM_BITMAP_CLEAR(shmem_data_p->gvrp_vlan_sm, *key_p);
            SYS_CALLBACK_OM_BITMAP_TO_BITMAP(
                shmem_data_p->gvrp_vlan_member_sm[*key_p-1],
                data_p->member_state, data_p->member_change,
                data_p->member_merge);
            memset(shmem_data_p->gvrp_vlan_member_sm[*key_p-1], 0,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_L3_VLAN:
        if (SYS_CALLBACK_OM_FindNextKey_8(shmem_data_p->l3_vlan_destroy,
                shmem_data_p->l3_if_oper_status_change, SIZE_OF_VLAN_BITMAP,
                key_p) == TRUE)
        {
            SYS_CALLBACK_OM_L3Vlan_T    *data_p;

            data_p = (SYS_CALLBACK_OM_L3Vlan_T*)buffer_p;
            if (L_CVRT_IS_MEMBER_OF_PORTLIST(shmem_data_p->l3_vlan_destroy,
                    *key_p))
            {
                data_p->vlan_destroy = TRUE;
                L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->l3_vlan_destroy,
                    *key_p);
            }
            else
            {
                data_p->vlan_destroy = FALSE;
            }
            if (L_CVRT_IS_MEMBER_OF_PORTLIST(
                    shmem_data_p->l3_if_oper_status_change, *key_p))
            {
                data_p->oper_status_change = TRUE;
                L_CVRT_DEL_MEMBER_FROM_PORTLIST(
                    shmem_data_p->l3_if_oper_status_change, *key_p);

            }
            else
            {
                data_p->oper_status_change = FALSE;
            }
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_PORT_VLAN:
        if (SYS_CALLBACK_OM_FindNextKey_1(shmem_data_p->port_vlan_change,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key_p) == TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->port_vlan_change,
                *key_p);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_PORT_VLAN_MODE:
        if (SYS_CALLBACK_OM_FindNextKey_1(shmem_data_p->port_vlan_mode_change,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key_p) ==  TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->port_vlan_mode_change,
                *key_p);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_PVID:
        if (SYS_CALLBACK_OM_FindNextKey_1(shmem_data_p->pvid_change,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key_p) == TRUE)
        {
            SYS_CALLBACK_OM_Pvid_T  *data_p;

            data_p = (SYS_CALLBACK_OM_Pvid_T*)buffer_p;
            data_p->old_pvid = shmem_data_p->old_pvid_ar[*key_p-1];
            data_p->new_pvid = shmem_data_p->new_pvid_ar[*key_p-1];
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->pvid_change, *key_p);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_VLAN_NAME:
        if (SYS_CALLBACK_OM_FindNextKey_1(shmem_data_p->vlan_name_change,
                SIZE_OF_VLAN_BITMAP, key_p) == TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->vlan_name_change,
                *key_p);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_PROTOCOL_VLAN:
        if (SYS_CALLBACK_OM_FindNextKey_1(shmem_data_p->protocol_vlan_change,
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key_p) ==  TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->protocol_vlan_change,
                *key_p);
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_VLAN_MEMBER_TAG:
        if (SYS_CALLBACK_OM_FindNextKey_5(shmem_data_p->vlan_member_tag_change,
                SYS_ADPT_MAX_VLAN_ID, key_p, key_p+1) == TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(
                shmem_data_p->vlan_member_tag_change[*key_p-1], *(key_p+1));
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_PORT_STATE:
        if (SYS_CALLBACK_OM_FindNextKey_4(shmem_data_p->xstp_port_sm,
                SYS_ADPT_MAX_NBR_OF_MST_INSTANCE, key_p, key_p+1) == TRUE)
        {
            SYS_CALLBACK_OM_Common_T    *data_p;

            data_p = (SYS_CALLBACK_OM_Common_T*)buffer_p;
            SYS_CALLBACK_OM_BITMAP_TO_DATA(shmem_data_p->xstp_port_sm[*key_p],
                *(key_p+1), data_p->state, data_p->change, data_p->merge);
            SYS_CALLBACK_OM_BITMAP_CLEAR(shmem_data_p->xstp_port_sm[*key_p],
                *(key_p+1));
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_VERSION:
        if (shmem_data_p->xstp_version_change == TRUE)
        {
            shmem_data_p->xstp_version_change = FALSE;
            result = TRUE;
        }
        break;

    case SYS_CALLBACK_OM_KIND_XSTP_PORT_TOPO:
        if (SYS_CALLBACK_OM_FindNextKey_3(shmem_data_p->xstp_port_topo_change,
                SYS_ADPT_MAX_NBR_OF_MST_INSTANCE, key_p, key_p+1) == TRUE)
        {
            L_CVRT_DEL_MEMBER_FROM_PORTLIST(
                shmem_data_p->xstp_port_topo_change[*key_p], *(key_p+1));
            result = TRUE;
        }
        break;

    default:
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
        return FALSE;
    } /* end switch */

    if (result == FALSE)
    {
        L_CVRT_DEL_MEMBER_FROM_PORTLIST(shmem_data_p->change_bitmap, kind+1);
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(shmem_sem_id, orig_priority);
    return result;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */

/* LOCAL SUBPROGRAM BODIES
 */
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
static void SYS_CALLBACK_OM_InitBackdoorDb(void)
{
    memset(&shmem_data_p->backdoor_db, 0, sizeof(shmem_data_p->backdoor_db));

    shmem_data_p->backdoor_db.dbg_event_id = 0xffffffff;
    shmem_data_p->backdoor_db.dbg_src_id = 0xffffffff;
    shmem_data_p->backdoor_db.dbg_dst_id = 0xffffffff;
}

#if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE)
static BOOL_T SYS_CALLBACK_OM_FindNextVlan(UI8_T *vlan_bitmap,
    UI8_T member_bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST],
    UI32_T *key_p)
{
    UI32_T  vid = *key_p, i;
    BOOL_T  result;

    result = FALSE;
    while (result == FALSE)
    {
        vid++;
        if (vid > SYS_ADPT_MAX_VLAN_ID)
        {
            if (*key_p == 0)
            {
                break;
            }
            vid = 1;
        }
        if (SYS_CALLBACK_OM_BITMAP_GET(vlan_bitmap, vid))
        {
            result = TRUE;
            break;
        }
        for (i = 0; i < SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST; i++)
        {
            if (member_bitmap[vid-1][i])
            {
                result = TRUE;
                break;
            }
        }
        if (vid == *key_p)
        {
            break;
        }
    } /* end while */

    if (result == TRUE)
    {
        *key_p = vid;
    }
    else
    {
        *key_p = 0;
    }

    return result;
}

/* the smallest real value for key is 1; each key uses one bit of bitmap */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_1(UI8_T *bitmap, UI32_T size,
    UI32_T *key_p)
{
    UI32_T  byte, bit, beg_byte, beg_bit;
    BOOL_T  result;

    if (*key_p == 0)
    {
        byte = bit = beg_byte = beg_bit = 0;
    }
    else
    {
        beg_byte = (*key_p - 1) / 8;
        beg_bit = (*key_p -1 ) % 8;
        byte = beg_byte;
        bit = beg_bit + 1;
    }

    result = FALSE;
    while (result == FALSE)
    {
        if (bitmap[byte] > 0)
        {
            while (bit < 8)
            {
                if (bitmap[byte] & (1 << (7 - bit)))
                {
                    result = TRUE;
                    break;
                }
                bit++;
            }
            if (result == TRUE)
            {
                break;
            }
        }
        byte++;
        if (byte >= size)
        {
            byte = 0;
        }
        bit = 0;
        if (byte == beg_byte)
        {
            if (bitmap[byte] > 0)
            {
                while (bit <= beg_bit)
                {
                    if (bitmap[byte] & (1 << (7 - bit)))
                    {
                        result = TRUE;
                        break;
                    }
                    bit++;
                }
            }
            break;
        }
    } /* end while */

    if (result == TRUE)
    {
        *key_p = byte * 8 + bit + 1;
    }
    else
    {
        *key_p = 0;
    }

    return result;
}

/* the smallest real value for key is 1; each key uses two bits of bitmap */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_2(UI8_T *bitmap, UI32_T size,
    UI32_T *key_p)
{
    UI32_T  byte, bit, beg_byte, beg_bit;
    BOOL_T  result;

    if (*key_p == 0)
    {
        byte = bit = beg_byte = beg_bit = 0;
    }
    else
    {
        beg_byte = (*key_p - 1) / 4;
        beg_bit = (*key_p -1 ) % 4;
        byte = beg_byte;
        bit = beg_bit + 1;
    }

    result = FALSE;
    while (result == FALSE)
    {
        if (bitmap[byte] > 0)
        {
            while (bit < 4)
            {
                if (bitmap[byte] & (3 << ((3 - bit) * 2)))
                {
                    result = TRUE;
                    break;
                }
                bit++;
            }
            if (result == TRUE)
            {
                break;
            }
        }
        byte++;
        if (byte >= size)
        {
            byte = 0;
        }
        bit = 0;
        if (byte == beg_byte)
        {
            if (bitmap[byte] > 0)
            {
                while (bit <= beg_bit)
                {
                    if (bitmap[byte] & (3 << ((3 - bit) * 2)))
                    {
                        result = TRUE;
                        break;
                    }
                    bit++;
                }
            }
            break;
        }
    } /* end while */

    if (result == TRUE)
    {
        *key_p = byte * 4 + bit + 1;
    }
    else
    {
        *key_p = 0;
    }

    return result;
}

/* the size of bitmap for key2 is SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * the smallest real value for key1 is 0, for key2 is 1
 */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_3(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p)
{
    UI32_T  first_key;
    BOOL_T  result;

    first_key = *key1_p;
    result = FALSE;
    while (result == FALSE)
    {
        if (SYS_CALLBACK_OM_FindNextKey_1(bitmap[first_key],
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key2_p) == TRUE)
        {
            result = TRUE;
            break;
        }
        first_key++;
        if (first_key >= size)
        {
            first_key = 0;
        }
        if (first_key == *key1_p)
        {
            break;
        }
    }

    if (result == TRUE)
    {
        *key1_p = first_key;
    }
    else
    {
        *key1_p = 0;
    }

    return result;
}

/* the size of bitmap for key2 is SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST
 * the smallest real value for key1 is 0, for key2 is 1
 */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_4(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p)
{
    UI32_T  first_key;
    BOOL_T  result;

    first_key = *key1_p;
    result = FALSE;
    while (result == FALSE)
    {
        if (SYS_CALLBACK_OM_FindNextKey_2(bitmap[first_key],
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_2BIT_PORT_LIST, key2_p) == TRUE)
        {
            result = TRUE;
            break;
        }
        first_key++;
        if (first_key >= size)
        {
            first_key = 0;
        }
        if (first_key == *key1_p)
        {
            break;
        }
    }

    if (result == TRUE)
    {
        *key1_p = first_key;
    }
    else
    {
        *key1_p = 0;
    }

    return result;
}

/* the size of bitmap for key2 is SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST
 * the smallest real value for key1 is 1, for key2 is 1
 */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_5(
    UI8_T bitmap[][SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST], UI32_T size,
    UI32_T *key1_p, UI32_T *key2_p)
{
    UI32_T  first_key;
    BOOL_T  result;

    first_key = *key1_p;
    if (first_key == 0)
    {
        first_key = *key1_p = 1;
    }
    result = FALSE;
    while (result == FALSE)
    {
        if (SYS_CALLBACK_OM_FindNextKey_1(bitmap[first_key-1],
                SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST, key2_p) == TRUE)
        {
            result = TRUE;
            break;
        }
        first_key++;
        if (first_key > size)
        {
            first_key = 1;
        }
        if (first_key == *key1_p)
        {
            break;
        }
    }

    if (result == TRUE)
    {
        *key1_p = first_key;
    }
    else
    {
        *key1_p = 0;
    }

    return result;
}

/* find any in two bitmaps with the same size; each key uses one bit */
static BOOL_T SYS_CALLBACK_OM_FindNextKey_8(UI8_T *bitmap1, UI8_T *bitmap2,
    UI32_T size, UI32_T *key_p)
{
    UI32_T  byte, bit, beg_byte, beg_bit;
    BOOL_T  result;

    if (*key_p == 0)
    {
        byte = bit = beg_byte = beg_bit = 0;
    }
    else
    {
        beg_byte = (*key_p - 1) / 8;
        beg_bit = (*key_p -1 ) % 8;
        byte = beg_byte;
        bit = beg_bit + 1;
    }

    result = FALSE;
    while (result == FALSE)
    {
        if ((bitmap1[byte] > 0) || (bitmap2[byte] > 0))
        {
            while (bit < 8)
            {
                if (    (bitmap1[byte] & (1 << (7 - bit)))
                     || (bitmap2[byte] & (1 << (7 - bit)))
                   )
                {
                    result = TRUE;
                    break;
                }
                bit++;
            }
        }
        if (result == TRUE)
        {
            break;
        }
        byte++;
        if (byte >= size)
        {
            byte = 0;
        }
        bit = 0;
        if (byte == beg_byte)
        {
            if ((bitmap1[byte] > 0) || (bitmap2[byte] > 0))
            {
                while (bit <= beg_bit)
                {
                    if (    (bitmap1[byte] & (1 << (7 - bit)))
                         || (bitmap2[byte] & (1 << (7 - bit)))
                       )
                    {
                        result = TRUE;
                        break;
                    }
                    bit++;
                }
            }
            break;
        }
    } /* end while */

    if (result == TRUE)
    {
        *key_p = byte * 8 + bit + 1;
    }
    else
    {
        *key_p = 0;
    }

    return result;
}
#endif /* #if (SYS_CPNT_SYS_CALLBACK_ENHANCEMENT == TRUE) */
