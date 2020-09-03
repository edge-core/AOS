/* FUNCTION NAME: add_mgr.c
 * PURPOSE:
 *	 1. Manage the ADD CSC
 *   2. Need CFGDB support: If a port in auto mode and already adds to
 *      Voice VLAN automatically. After reboot and system is ready, the
 *      port shall be added to Voice VLAN immediately.
 *   3. Need define the constant SYS_HWCFG_SUPPORT_FILTER_MAC for filter
 *      for voice VLAN port security issue.
 * NOTES:
 *
 * REASON:
 *    DESCRIPTION:
 *    CREATOR:	Junying
 *    Date       2006/04/26
 *
 * Copyright(C)      Accton Corporation, 2006
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_ADD == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sys_dflt.h"
#include "sysfun.h"
#include "add_type.h"
#include "add_om.h"
#include "add_mgr.h"
#include "leaf_es3626a.h"
#include "vlan_type.h"
#include "amtr_mgr.h"
#include "vlan_pmgr.h"
#include "swctrl.h"
#include "add_backdoor.h"

#ifdef VXWORKS_FRAMER
#include "cli_mgr.h"
#endif

#if (SYS_CPNT_CFGDB == TRUE)
#include "cfgdb_mgr.h"
#endif

#if (SYS_CPNT_LLDP == TRUE)
#include "lldp_om.h"
#include "lldp_mgr.h"
#endif /*#if (SYS_CPNT_LLDP == TRUE)*/

/* NAMING CONSTANT DECLARATIONS
 */
/*#define _ADD_MGR_DEBUG*/
#define ADD_MGR_DFLT_MASK_ADDR_BYTE1    0xFF
#define ADD_MGR_DFLT_MASK_ADDR_BYTE2    0xFF
#define ADD_MGR_DFLT_MASK_ADDR_BYTE3    0xFF
#define ADD_MGR_DFLT_MASK_ADDR_BYTE4    0x00
#define ADD_MGR_DFLT_MASK_ADDR_BYTE5    0x00
#define ADD_MGR_DFLT_MASK_ADDR_BYTE6    0x00

#if (SYS_CPNT_LLDP == TRUE)
    #define ADD_MGR_MAX_NETWORK_ADDR_LENGTH   LLDP_TYPE_MAX_MANAGEMENT_ADDR_LENGTH
#else
    #define ADD_MGR_MAX_NETWORK_ADDR_LENGTH   31L
#endif


#if (SYS_CPNT_CFGDB == TRUE)
    #define CFGDB_MGR_SECTION_ID_VOICE_VLAN_LAST_JOIN_STATE  CFGDB_MGR_SECTION_ID_VOICE_VLAN_1
#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef _ADD_MGR_DEBUG
static char lock_name[255+1];
    #define ADD_MGR_ENTER_CRITICAL_SECTION() { \
            if(add_mgr_sem_id == 1) \
            {\
               if(TRUE == add_mgr_debug_print) \
               {\
                   printf("\r\nPrev lock=%s", lock_name); \
                   printf("\r\n%s:%d lock forever", __FUNCTION__, __LINE__); \
               }\
            }\
            else \
                add_mgr_sem_id = 1; \
            strncpy(lock_name, __FUNCTION__, 255);\
        }

    #define ADD_MGR_LEAVE_CRITICAL_SECTION() { \
            if(add_mgr_sem_id == 0) \
                if(TRUE == add_mgr_debug_print) \
                printf("\r\n%s:%d error for the count of sem", __FUNCTION__, __LINE__); \
            else \
                add_mgr_sem_id = 0;\
                lock_name[0] = 0;\
        }
#endif

#ifdef LINUX_FRAMER
    #define ADD_MGR_ENTER_CRITICAL_SECTION()
    #define ADD_MGR_LEAVE_CRITICAL_SECTION()
#endif

#ifdef VXWORKS_FRAMER
    #define ADD_MGR_ENTER_CRITICAL_SECTION() { \
            SYSFUN_ENTER_CRITICAL_SECTION(add_mgr_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER); \
        }
    #define ADD_MGR_LEAVE_CRITICAL_SECTION() { \
            SYSFUN_LEAVE_CRITICAL_SECTION(add_mgr_sem_id); \
        }
#endif

#define ADD_MGR_IS_LPORT(ifindex) ((ifindex != 0) && (ifindex <= SYS_ADPT_TOTAL_NBR_OF_LPORT))


#define ADD_MGR_DGB_MSG(fmt, args...)                       \
{                                                           \
    if (add_mgr_debug_print)                                \
    {                                                       \
        printf("[%s:%d] ", __FUNCTION__, __LINE__);         \
        printf(fmt, ##args);                                \
        printf("\r\n");                                     \
    }                                                       \
}


//#define VLAN_TYPE_VLAN_STATUS_VOICE VAL_dot1qVlanStatus_other




/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T  mac_addr[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  network_addr_subtype;
    UI8_T  network_addr[ADD_MGR_MAX_NETWORK_ADDR_LENGTH];
    UI8_T  network_addr_len;
    UI32_T network_addr_ifindex;
}ADD_MGR_NetworkAddressEntry_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T ADD_MGR_SetVoiceVlanPortRuleBitmap(UI32_T lport, UI32_T rule_bitmap);
static BOOL_T ADD_MGR_EnableVoiceVlanPortOui(UI32_T lport, BOOL_T state, UI8_T protocol);
static BOOL_T ADD_MGR_EnableVoiceVlanPortLldp(UI32_T lport, BOOL_T state, UI8_T protocol);

#ifdef LINUX_FRAMER
static BOOL_T ADD_MGR_RemapAddress(AMTR_TYPE_AddrEntry_T *entry_p, UI8_T priority);
#endif

#ifdef VXWORKS_FRAMER
static BOOL_T ADD_MGR_RemapAddress(UI32_T vid, UI8_T *mac, UI32_T lport, UI32_T addr_attribute, UI8_T priority);
#endif

static BOOL_T ADD_MGR_JoinVoiceVlan(UI32_T lport, UI8_T protocol, BOOL_T *notify_change_join_state);
static BOOL_T ADD_MGR_DisjoinVoiceVlanByJoinState(UI32_T lport, BOOL_T *notify_change_join_state);
static BOOL_T ADD_MGR_PostChangeToAutoMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state);
static BOOL_T ADD_MGR_PostChangeToManualMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state);
static BOOL_T ADD_MGR_PostChangeToNoneMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state);
static BOOL_T ADD_MGR_SavePerJoinStateToCfgdb();

static BOOL_T ADD_MGR_IsValidLPortForVoiceVlan(UI32_T lport);
static BOOL_T ADD_MGR_GetPhoneMac(UI32_T lport,
                                  UI8_T mac_list[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN],
                                  UI8_T mask_list[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN],
                                  UI32_T *get_number);

#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
    static BOOL_T ADD_MGR_ProcessAgedMac(UI32_T vid, UI8_T *mac, UI32_T lport);

    static BOOL_T ADD_MGR_AuthorizeNewMac(UI32_T vid, UI8_T *mac, UI32_T lport);
    static BOOL_T ADD_MGR_AutoJoinVoiceVlanByNewMac(UI32_T vid, UI8_T *mac, UI32_T lport);

#endif /* #if (VOICE_VLAN_DO_UNIT_TEST == FALSE) */
static BOOL_T ADD_MGR_IsDetectedPhoneByOui(UI32_T lport);
static BOOL_T ADD_MGR_IsDetectedPhoneByLldp(UI32_T lport);
static BOOL_T ADD_MGR_IsExistedPhone(UI32_T lport);
static BOOL_T ADD_MGR_IsPhoneMac(UI32_T lport, UI8_T *mac);

static BOOL_T ADD_MGR_ProcessCamePhoneByLldp(UI32_T lport);
static BOOL_T ADD_MGR_ProcessLeftPhoneByLldp(UI32_T lport);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T add_mgr_sem_id;
static BOOL_T add_mgr_handle_amtr_addr_changed_callback;
static BOOL_T add_mgr_debug_print = FALSE;

#if (SYS_CPNT_CFGDB == TRUE)
    static UI32_T add_mgr_join_state_handler;
#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC


//BOOL_T VLAN_MGR_SetVoiceVlanId(UI32_T vid){return TRUE;}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_TASK_Initiate_System_Resources
 * ---------------------------------------------------------------------
 * PURPOSE: Init system resource.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_Initiate_System_Resources(void)
{

#ifdef LINUX_FRAMER
    ADD_OM_CreatSem();
#endif

#ifdef VXWORKS_FRAMER

    if (SYSFUN_CreateSem(1, SYSFUN_SEM_FIFO, &add_mgr_sem_id) != SYSFUN_OK)
    {
        SYSFUN_Debug_Printf("\r\nCreate add_mgr_sem_id failed.");
        return FALSE;
    }

#endif

    return TRUE;
} /* End of ADD_MGR_Initiate_System_Resources */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - ADD_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void ADD_MGR_Create_InterCSC_Relation()
{
#ifdef VXWORKS_FRAMER

    AMTR_MGR_Register_SetStaticMACCheck_CallBack(ADD_MGR_AMTR_SetStaticMacCheck_CallBack);
    AMTR_MGR_Register_EditAddrEntry_CallBack(ADD_MGR_AMTR_EditAddrNotify_CallBack);

#if (SYS_CPNT_LLDP == TRUE)
    LLDP_MGR_RegisterTelephoneDetect(ADD_MGR_LLDP_TelephoneDetect_CallBack);
#endif /*#if (SYS_CPNT_LLDP)*/

    CLI_MGR_Register_ProvisionComplete_CallBack(ADD_MGR_CLI_ProvisionComplete_CallBack);
    VLAN_MGR_RegisterVlanMemberDelete_CallBack(ADD_MGR_VLAN_VlanMemberDelete_CallBack);

#endif /* #ifdef VXWORKS_FRAMER */

    BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack("add",
        SYS_BLD_NETACCESS_GROUP_IPCMSGQ_KEY, ADD_Backdoor_Main_CallBack);

}

/* EXPORTED SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_CurrentOperationMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function return the current opearion mode of add 's
 *           task.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   add_operation_mode
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Stacking_Mode_T ADD_MGR_CurrentOperationMode()
{
   return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterMasterMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the master mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_EnterMasterMode()
{
#if (SYS_CPNT_CFGDB == TRUE)
    UI8_T  default_join_state_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};
    BOOL_T is_default_need_to_sync;
#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/

    /* clear database
     */
    if (FALSE == ADD_OM_Initialize())
    {
        SYSFUN_Debug_Printf("\r\n[ADD_MGR_EnterMasterMode] ADD_OM_Initialize() failed.");
    }

#if (SYS_CPNT_CFGDB == TRUE)
    if(CFGDB_MGR_Open(CFGDB_MGR_SECTION_ID_VOICE_VLAN_LAST_JOIN_STATE,
                   sizeof(UI8_T),
                   SYS_ADPT_TOTAL_NBR_OF_LPORT,
                   &add_mgr_join_state_handler,
                   CFGDB_MGR_SECTION_TYPE_EXTERNAL_AND_GLOBAL,
                   &is_default_need_to_sync) == TRUE )
    {
        if( TRUE == is_default_need_to_sync )
        {
            CFGDB_MGR_SyncSection(add_mgr_join_state_handler, (void *)default_join_state_ar);
        }
    }
    else
    {
        SYSFUN_Debug_Printf("Open CFGDB fail", __FUNCTION__);
    }
#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/

    /* set mgr in master mode */
    SYSFUN_ENTER_MASTER_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterSlaveMode
 * ---------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the Slave mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_EnterSlaveMode()
{
    /* set mgr in slave mode */
    SYSFUN_ENTER_SLAVE_MODE();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_SetTransitionMode
 * ---------------------------------------------------------------------
 * PURPOSE:  Set transition mode
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ----------------------------------------------------------------------
 */
void ADD_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}

/*----------------------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_EnterTransition Mode
 * ----------------------------------------------------------------------
 * PURPOSE:  This function will make the add enter the transition mode.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 * ----------------------------------------------------------------------
 */
void ADD_MGR_EnterTransitionMode()
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();

    ADD_MGR_ENTER_CRITICAL_SECTION();
    add_mgr_handle_amtr_addr_changed_callback = TRUE;
    add_mgr_debug_print = FALSE;
    ADD_MGR_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsVoiceVlanEnabled
 * ---------------------------------------------------------------------
 * PURPOSE: Get Voice VLAN state.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE               - Enabled
 *          FALSE              - Disabled
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_IsVoiceVlanEnabled()
{
    return ADD_OM_IsVoiceVlanEnabled();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanId
 * ---------------------------------------------------------------------
 * PURPOSE: Get voice VLAN ID.
 * INPUT:   None.
 * OUTPUT:  vid               -- the voice VLAN ID.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanId(I32_T *vid)
{
    return ADD_OM_GetVoiceVlanId(vid);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVLANId
 * ---------------------------------------------------------------------
 * PURPOSE: Get RUNNING voice VLAN ID.
 * INPUT:   None.
 * OUTPUT:  vid               -- the voice VLAN ID.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanId(I32_T *vid)
{
    if(FALSE == ADD_OM_GetVoiceVlanId(vid))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(SYS_DFLT_ADD_VOICE_VLAN_ID == *vid)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanEnabledId
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable the voice VLAN feature.
 * INPUT:   vid               -- the voice VLAN ID.
 *                               VAL_voiceVlanEnabledId_disabled:
 *                                 disable voice VLAN.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. The Voice VLAN ID shall be created and active first.
 *          2. The Voice VLAN ID shall not be modified when the voice
 *             VLAN feature is enabled.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanEnabledId(I32_T vid)
{
    UI32_T old_vid;
    UI32_T vid_ifindex;
    UI32_T lport;
    UI32_T per_mode;
    BOOL_T save_join_state     = FALSE;
    BOOL_T tmp_save_join_state = FALSE;
    VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;

    /* Exit if no changing
     */
    ADD_OM_GetVoiceVlanId(&old_vid);
    if (old_vid == vid)
    {
        return TRUE;
    }

    /* Disable voice VLAN
     */
    if(vid == VAL_voiceVlanEnabledId_disabled)
    {
        ADD_MGR_ENTER_CRITICAL_SECTION();

        /* For each lport to disable voice VLAN feature
         */
        for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        {
            ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
            ADD_MGR_PostChangeToNoneMode(lport, per_mode, &tmp_save_join_state);
            save_join_state |= tmp_save_join_state;
        }

        /* Exit if fail to set the voice VLAN ID
         */
        if(FALSE == VLAN_PMGR_SetVoiceVlanId(0))
        {
            ADD_MGR_DGB_MSG("VLAN_MGR_SetVoiceVlanId fails.");
            ADD_MGR_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        ADD_OM_SetVoiceVlanId(vid);

        ADD_MGR_LEAVE_CRITICAL_SECTION();
    }
    else/* Enable voice VLAN */
    {
        /* Check the global status:
         * The voice VLAN ID shall not be modified when the global status is enabled.
         */
        if (VAL_voiceVlanEnabledId_disabled != old_vid)
        {
            ADD_MGR_DGB_MSG("Cannot change VVID when voice VLAN is enabled.");
            return FALSE;
        }

        if(FALSE == VLAN_OM_ConvertToIfindex(vid, &vid_ifindex))
        {
            ADD_MGR_DGB_MSG("vlan id=0");
            return FALSE;
        }

        vlan_info.dot1q_vlan_index = vid_ifindex;

        /* The VLAN is not exist or exist but not active
         */
        if ((FALSE == VLAN_OM_IsVlanExisted(vid))      ||
            (FALSE == VLAN_OM_GetVlanEntry(&vlan_info)) ||
             vlan_info.dot1q_vlan_static_row_status != VAL_dot1qVlanStaticRowStatus_active)
        {
            ADD_MGR_DGB_MSG("vlan%ld is not existed or inactive.", vid);
            return FALSE;
        }

        if(FALSE == VLAN_PMGR_SetVoiceVlanId(vid))
        {
            ADD_MGR_DGB_MSG("VLAN_MGR_SetVoiceVlanId fails.");
            return FALSE;
        }

        ADD_MGR_ENTER_CRITICAL_SECTION();

        ADD_OM_SetVoiceVlanId(vid);

        for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        {
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
            ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, 0);
        }

        /* For each lport to set port mode
         */
        for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        {
            ADD_OM_GetVoiceVlanPortMode(lport, &per_mode);
            switch(per_mode)
            {
                case VAL_voiceVlanPortMode_auto:
                    if((FALSE == ADD_MGR_IsValidLPortForVoiceVlan(lport)) ||
                       (FALSE == ADD_MGR_PostChangeToAutoMode(lport, VAL_voiceVlanPortMode_none, &tmp_save_join_state)))
                    {
                        ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
                    }
                    save_join_state |= tmp_save_join_state;
                    break;

                case VAL_voiceVlanPortMode_manual:
                    if((FALSE == ADD_MGR_IsValidLPortForVoiceVlan(lport)) ||
                       (FALSE == ADD_MGR_PostChangeToManualMode(lport, VAL_voiceVlanPortMode_none, &tmp_save_join_state)))
                    {
                        ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
                    }
                    save_join_state |= tmp_save_join_state;
                    break;

                case VAL_voiceVlanPortMode_none:
                    ADD_MGR_PostChangeToNoneMode(lport, VAL_voiceVlanPortMode_none, &tmp_save_join_state);
                    save_join_state |= tmp_save_join_state;
                    break;

                default:
                    ADD_OM_SetVoiceVlanPortMode(lport, VAL_voiceVlanPortMode_none);
                    ADD_MGR_DGB_MSG("no such port mode(%lu) on port(%lu).", per_mode, lport);
                    break;
            }
        }

        ADD_MGR_LEAVE_CRITICAL_SECTION();
    }/*End of Enable*/

    if(TRUE == save_join_state)
    {
        if(FALSE == ADD_MGR_SavePerJoinStateToCfgdb())
        {
            ADD_MGR_DGB_MSG("ADD_MGR_SavePerJoinStateToCfgdb fails.");
            return FALSE;
        }
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortMode(UI32_T lport, UI32_T *mode)
{
    return ADD_OM_GetVoiceVlanPortMode(lport, mode);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortMode(UI32_T lport, UI32_T *mode)
{
    if(FALSE == ADD_OM_GetVoiceVlanPortMode(lport, mode))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(SYS_DFLT_ADD_VOICE_VLAN_PORT_MODE == *mode)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortMode
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port mode on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          mode              -- the voice VLAN port mode;
 *                               VAL_voiceVlanPortMode_none,
 *                               VAL_voiceVlanPortMode_manual,
 *                               VAL_voiceVlanPortMode_auto.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanPortMode(UI32_T lport, UI32_T mode)
{
    UI32_T prev_per_mode;
    BOOL_T ret;

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return FALSE;
    }

    /* Get OM */
    if(FALSE == ADD_OM_GetVoiceVlanPortMode(lport, &prev_per_mode))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails", lport);
        return FALSE;
    }

    /* no change */
    if(mode == prev_per_mode)
    {
        return TRUE;
    }

    /* if enable voice VLAN feature on this lport
     * check the port mode of VLAN
     * check the port status of trunk interface
     */
    if((mode != VAL_voiceVlanPortMode_none) && (FALSE == ADD_MGR_IsValidLPortForVoiceVlan(lport)))
    {
        return FALSE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    if(FALSE == ADD_OM_SetVoiceVlanPortMode(lport, mode))
    {
        ADD_MGR_DGB_MSG("No such port mode.");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
    ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, 0);

    /* Exit if the global Voice VLAN state is disable */
    if(FALSE == ADD_OM_IsVoiceVlanEnabled())
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    /* Post process
     * e.g., delete nophone mac,
     *       remap learned phone mac,
     *       join/disjoin vlan
     */
    switch(mode)
    {
    case VAL_voiceVlanPortMode_auto:
        ret = ADD_MGR_PostChangeToAutoMode(lport, prev_per_mode, 0);
        if(FALSE == ret)
        {
            ADD_OM_SetVoiceVlanPortMode(lport, prev_per_mode);
        }
        break;

    case VAL_voiceVlanPortMode_manual:
        ret = ADD_MGR_PostChangeToManualMode(lport, prev_per_mode, 0);
        if(FALSE == ret)
        {
            ADD_OM_SetVoiceVlanPortMode(lport, prev_per_mode);
        }
        break;

    case VAL_voiceVlanPortMode_none:
        ret = ADD_MGR_PostChangeToNoneMode(lport, prev_per_mode, 0);
        break;

    default:
        /* This cause shall not be happened. */
        ADD_MGR_DGB_MSG("No such port mode.");
        ret = FALSE;
        break;
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port security state on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state)
{
    return ADD_OM_GetVoiceVlanPortSecurityState(lport, state);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port security state the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortSecurityState(UI32_T lport, UI32_T *state)
{
    if(FALSE == ADD_OM_GetVoiceVlanPortSecurityState(lport, state))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(SYS_DFLT_ADD_VOICE_VLAN_PORT_SECURITY_STATE == *state)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortSecurityState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port security state on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the voice VLAN port security state;
 *                               VAL_voiceVlanPortSecurity_enabled,
 *                               VAL_voiceVlanPortSecurity_disabled.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanPortSecurityState(UI32_T lport, UI32_T state)
{
    I32_T  voice_vlan_id;
    UI32_T number_of_mac;
    UI8_T  mac_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    if(FALSE == ADD_MGR_IS_LPORT(lport) )
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return FALSE;
    }

    /* Get OM */
    if(FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails", lport);
        return FALSE;
    }

    /* No change */
    if(port_entry.security_state == state)
    {
        return TRUE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    if(FALSE == ADD_OM_SetVoiceVlanPortSecurityState(lport, state))
    {
        ADD_MGR_DGB_MSG("ADD_OM_SetVoiceVlanPortSecurityState fails");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* Exit if the global/per-port Voice VLAN state is disable */
    if((FALSE == ADD_OM_IsVoiceVlanEnabled()) || (VAL_voiceVlanPortMode_none == port_entry.mode))
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    if(FALSE == ADD_MGR_GetPhoneMac(lport, mac_list_ar, mask_list_ar, &number_of_mac))
    {
        ADD_MGR_DGB_MSG("ADD_MGR_GetPhoneMac fails");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* If the security state is ENABLED,
     * delete all non-phone mac address on Voice VLAN id
     * Else the security state is DISABLED,
     * delete all non-phone mac address that be set with drop attribute on Voice VLAN id
     */
    ADD_OM_GetVoiceVlanId(&voice_vlan_id);
    add_mgr_handle_amtr_addr_changed_callback = FALSE;

    if (FALSE == AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport, voice_vlan_id, mac_list_ar, mask_list_ar, number_of_mac))
    //if(FALSE == AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr(lport, voice_vlan_id, &mac_list_ar, &mask_list_ar, number_of_mac))
    {
        add_mgr_handle_amtr_addr_changed_callback = TRUE;
        ADD_MGR_DGB_MSG("AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr fails");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    add_mgr_handle_amtr_addr_changed_callback = TRUE;

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port OUI rule stateon the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state)
{
    UI8_T protocol;

    if(NULL == state)
    {
        return FALSE;
    }

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return FALSE;
    }

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &protocol))
    {
        return FALSE;
    }

    /*Get state from corresponding bit*/
    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        *state = VAL_voiceVlanPortRuleOui_enabled;
    }
    else
    {
        *state = VAL_voiceVlanPortRuleOui_disabled;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state)
{
    UI8_T protocol;

    if(NULL == state)
    {
        return FALSE;
    }

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return FALSE;
    }

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &protocol))
    {
        return FALSE;
    }

    /*Get state from corresponding bit*/
    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {
        *state = VAL_voiceVlanPortRuleLldp_enabled;
    }
    else
    {
        *state = VAL_voiceVlanPortRuleLldp_disabled;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port OUI rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T *state)
{
    UI8_T protocol;

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &protocol))
    {
        ADD_MGR_DGB_MSG("ADD_OM_GetVoiceVlanPortRuleBitmap on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*Get state from corresponding bit*/
    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        *state = VAL_voiceVlanPortRuleOui_enabled;
    }
    else
    {
        *state = VAL_voiceVlanPortRuleOui_disabled;
    }

    if(SYS_DFLT_ADD_VOICE_VLAN_PORT_RULE_OUI_STATUS == *state)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T *state)
{
    UI8_T protocol;

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &protocol))
    {
        ADD_MGR_DGB_MSG("ADD_OM_GetVoiceVlanPortRuleBitmap on port(%ld)", lport);
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    /*Get state from corresponding bit*/
    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {
        *state = VAL_voiceVlanPortRuleLldp_enabled;
    }
    else
    {
        *state = VAL_voiceVlanPortRuleLldp_disabled;
    }

    if(SYS_DFLT_ADD_VOICE_VLAN_PORT_RULE_LLDP_STATUS == *state)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortOuiRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port OUI rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the OUI rule state;
 *                               VAL_voiceVlanPortRuleOui_enabled,
 *                               VAL_voiceVlanPortRuleOui_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanPortOuiRuleState(UI32_T lport, UI32_T state)
{
    UI8_T per_rule_bitmap;

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_rule_bitmap))
    {
        ADD_MGR_DGB_MSG("ADD_OM_GetVoiceVlanPortRuleBitmap on port(%ld)", lport);
        return FALSE;
    }

    /*Set the corresponding bit*/
    if(VAL_voiceVlanPortRuleOui_enabled == state)
    {
        per_rule_bitmap |= ADD_TYPE_DISCOVERY_PROTOCOL_OUI;
    }
    else
    {
        per_rule_bitmap &= ~ADD_TYPE_DISCOVERY_PROTOCOL_OUI;
    }

    return ADD_MGR_SetVoiceVlanPortRuleBitmap(lport, per_rule_bitmap);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortLldpRuleState
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port LLDP rule state on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 *          state             -- the LLDP rule state;
 *                               VAL_voiceVlanPortRuleLldp_enabled,
 *                               VAL_voiceVlanPortRuleLldp_disabled.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanPortLldpRuleState(UI32_T lport, UI32_T state)
{
    UI8_T  per_rule_bitmap;

    /*Get rule bitmap from OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_rule_bitmap))
    {
        ADD_MGR_DGB_MSG("ADD_OM_GetVoiceVlanPortRuleBitmap on port(%ld)", lport);
        return FALSE;
    }

    /*Set the corresponding bit*/
    if(VAL_voiceVlanPortRuleLldp_enabled == state)
    {
        per_rule_bitmap |= ADD_TYPE_DISCOVERY_PROTOCOL_LLDP;
    }
    else
    {
        per_rule_bitmap &= ~ADD_TYPE_DISCOVERY_PROTOCOL_LLDP;
    }

    return ADD_MGR_SetVoiceVlanPortRuleBitmap(lport, per_rule_bitmap);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortRuleBitmap
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN port rule bitmap on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          rule_bitmap       -- the voice VLAN port rule bitmap supported;
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_OUI,
 *                               ADD_TYPE_DISCOVERY_PROTOCOL_LLDP.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. There shall be at least one port rule specified.
 *          2. There shall be protected by critical section.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_SetVoiceVlanPortRuleBitmap(UI32_T lport, UI32_T rule_bitmap)
{
    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret;

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)", lport);
        return FALSE;
    }

    /* There shall be at least one port rule specified. */
    if(0 == rule_bitmap)
    {
        ADD_MGR_DGB_MSG("Can't set no protocol on Voice VLAN port (%ld) fails", lport);
        return FALSE;
    }

    /* Get OM */
    if(FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails", lport);
        return FALSE;
    }

    /* no change */
    if(port_entry.protocol == rule_bitmap)
    {
        return TRUE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    /* Exit if the global/per-port Voice VLAN state is disable */
    if((FALSE == ADD_OM_IsVoiceVlanEnabled()) || (VAL_voiceVlanPortMode_none == port_entry.mode))
    {
        ret = ADD_OM_SetVoiceVlanPortRuleBitmap(lport, rule_bitmap);

        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return ret;
    }

    ret = ADD_OM_SetVoiceVlanPortRuleBitmap(lport, rule_bitmap);
    if (FALSE == ret)
    {
        ADD_MGR_DGB_MSG("Set rule failed on lport(%ld)", lport);
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return ret;
    }

    if(0 == (port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI))
    {
        /* Change the OUI rule state from disabled to enable */
        if(rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
        {
            ADD_MGR_DGB_MSG("Enable OUI on lport(%ld)", lport);
            ret = ADD_MGR_EnableVoiceVlanPortOui(lport, TRUE, port_entry.protocol);
        }
    }
    else /*if(prev_per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)*/
    {
        /* Change the OUI rule state from enabled to disable */
        if(!(rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_OUI))
        {
            ADD_MGR_DGB_MSG("Disable OUI on lport(%ld)",  lport);
            ret = ADD_MGR_EnableVoiceVlanPortOui(lport, FALSE, port_entry.protocol);
        }
    }

    if(0 == (port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
    {
        /* Change the LLDP rule state from disabled to enable */
        if(rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
        {
            ADD_MGR_DGB_MSG("Enable LLDP on lport(%ld)",  lport);
            ret = ADD_MGR_EnableVoiceVlanPortLldp(lport, TRUE, port_entry.protocol);
        }
    }
    else /*if(prev_per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)*/
    {
        /* Change the LLDP rule state from enabled to disable */
        if(!(rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
        {
            ADD_MGR_DGB_MSG("Disable LLDP on lport(%ld)",  lport);
            ret = ADD_MGR_EnableVoiceVlanPortLldp(lport, FALSE, port_entry.protocol);
        }
    }

    if(FALSE == ret)
    {
        ADD_MGR_DGB_MSG("Set rule fails on lport(%ld)",  lport);

        ADD_OM_SetVoiceVlanPortRuleBitmap(lport, port_entry.protocol);
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port priority on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortPriority(UI32_T lport, UI8_T *priority)
{
    return ADD_OM_GetVoiceVlanPortPriority(lport, priority);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN port priority on the specified
 *          lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES  : None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanPortPriority(UI32_T lport, UI8_T *priority)
{
    if(FALSE == ADD_OM_GetVoiceVlanPortPriority(lport, priority))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(SYS_DFLT_ADD_VOICE_VLAN_PORT_PRIORITY == *priority)
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanPortPriority
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VALN port priority on the specified lport.
 * INPUT:   lport             -- the logic port index.
 *          priority          -- the voice VLAN port override priority;
 *                               range: 0~6.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanPortPriority(UI32_T lport, UI8_T priority)
{
    //AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T addr_entry = {0};

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    I32_T  voice_vlan_id;
    BOOL_T ret;

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)",  lport);
        return FALSE;
    }


    /* Get OM */
    if(FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }
    /* No change */
    if(port_entry.priority == priority)
    {
        return TRUE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    if(FALSE == ADD_OM_SetVoiceVlanPortPriority(lport, priority))
    {
        ADD_MGR_DGB_MSG("ADD_OM_SetVoiceVlanPortPriority fails");

        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* Exit if the global/per-port Voice VLAN state is disable */
    if((FALSE == ADD_OM_IsVoiceVlanEnabled()) || (VAL_voiceVlanPortMode_none == port_entry.mode))
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    ADD_OM_GetVoiceVlanId(&voice_vlan_id);

    /* get all mac address on Voice VLAN */
    //memset(&addr_entry, 0, sizeof(AMTR_MGR_AddrEntry_T));
    //addr_entry.l_port = lport;
    memset(&addr_entry, 0, sizeof(addr_entry));
    addr_entry.ifindex = lport;
    addr_entry.vid = voice_vlan_id;

    ret = TRUE;
    /* Remap all mac address, exclude the mac that have drop attribute */
    while(TRUE == AMTR_MGR_GetNextIfIndexAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {
        if((addr_entry.ifindex/*l_port*/ != lport) || (addr_entry.vid != voice_vlan_id))
            break;

        ret &= ADD_MGR_RemapAddress(&addr_entry, priority);//(addr_entry.vid, addr_entry.mac, addr_entry.l_port, addr_entry.attribute, priority);
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanPortRemainAge
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN port remaining age on the specified lport.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  remain_age        -- Remaining age (mins).
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   0 means less than 1 minute.
 *
 *          ADD_TYPE_VOICE_VLAN_ERROR_NA means
 *          global voic vlan is disable or mode is disable or mode is manual
 *          or no auto join vlan now.
 *
 *          ADD_TYPE_VOICE_VLAN_ERROR_NO_START means
 *          auto join vlan now and there have phone attached
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanPortRemainAge(UI32_T lport, I32_T *age_p)
{
    ADD_OM_VoiceVlanPortEntry_T entry;

    if (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &entry))
    {
        *age_p = ADD_TYPE_VOICE_VLAN_ERROR_NA;
        return FALSE;
    }

    if (!ADD_OM_IsVoiceVlanEnabled()
        || entry.mode == VAL_voiceVlanPortMode_manual
        || entry.mode == VAL_voiceVlanPortMode_none)
    {
        *age_p = ADD_TYPE_VOICE_VLAN_ERROR_NA;
    }
    else
    {
        if (ADD_TYPE_DISCOVERY_PROTOCOL_NONE == entry.join_state)
        {
            *age_p = ADD_TYPE_VOICE_VLAN_ERROR_NA;
        }
        else if (ADD_TYPE_VOICE_VLAN_TIMER_DISABLED == entry.disjoin_when)
        {
            *age_p = ADD_TYPE_VOICE_VLAN_ERROR_NO_START;
        }
        else
        {
            *age_p = (entry.disjoin_when * ADD_TYPE_TIMER_EVENT_OF_SEC)/60;
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AddOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Add a recognised OUI entry.
 * INPUT:   entry             -- the OUI entry.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. Don't allow the multicast address for the OUI address.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_AddOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    I32_T  voice_vlan_id;
    UI32_T lport;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;

    //AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T       addr_entry;

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret;

    ADD_MGR_ENTER_CRITICAL_SECTION();

    if(FALSE == ADD_OM_AddOui(entry->oui, entry->mask, entry->description))
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /*Get OM*/
    if(FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id))
    {
        ADD_MGR_DGB_MSG("Get OM fails");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* Exit if the global Voice VLAN state is disable */
    if(FALSE == ADD_OM_IsVoiceVlanEnabled())
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    ret = TRUE;
    /* Override the action of the mac entry that match the added oui */
    for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        if((FALSE == VLAN_MGR_GetPortEntry(lport,  &vlan_port_info)) ||
           (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
        {
            ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
            continue;
        }

        if(VAL_voiceVlanPortMode_none == port_entry.mode)
            continue;

        if(0 == (ADD_TYPE_DISCOVERY_PROTOCOL_OUI & port_entry.protocol))
            continue;

        /* For PVID */
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        addr_entry.vid = vlan_port_info.port_item.dot1q_pvid_index;

        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, entry->oui, entry->mask, AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != vlan_port_info.port_item.dot1q_pvid_index)
                break;

            ADD_MGR_DGB_MSG("Get vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,port=%ld",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                lport);

            ret &= ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI, NULL);
        }

        /* For VoiceVLAN */
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        addr_entry.vid = voice_vlan_id;
        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, entry->oui, entry->mask, AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != voice_vlan_id)
                break;

            ADD_MGR_DGB_MSG("Get vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,port=%ld",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                lport);

            /* Remap */
            ret &= ADD_MGR_RemapAddress(/*addr_entry.vid, addr_entry.mac, addr_entry.l_port, addr_entry.attribute*/&addr_entry, port_entry.priority);
        }
    }/*End of for(lport...)*/

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_RemoveOui
 * ---------------------------------------------------------------------
 * PURPOSE: Remove a recognised OUI entry.
 * INPUT:   oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_RemoveOuiEntry(UI8_T *oui, UI8_T *mask)
{
    I32_T  voice_vlan_id;
    UI32_T lport;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
//    AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T       addr_entry;

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret;

    ADD_MGR_ENTER_CRITICAL_SECTION();

    if(FALSE == ADD_OM_RemoveOui(oui, mask))
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* Get OM
     */
    if(FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id))
    {
        ADD_MGR_DGB_MSG("Get OM fails");
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    /* Exit if the global Voice VLAN state is disable
     */
    if(FALSE == ADD_OM_IsVoiceVlanEnabled())
    {
        ADD_MGR_LEAVE_CRITICAL_SECTION();
        return TRUE;
    }

    ret = TRUE;

    /* Delete the learned mac entry that match the removed oui
     */
    for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
    {
        if((FALSE == VLAN_MGR_GetPortEntry(lport,  &vlan_port_info)) ||
           (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
        {
            ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
            continue;
        }

        if(VAL_voiceVlanPortMode_none == port_entry.mode)
            continue;

        if(0 == (ADD_TYPE_DISCOVERY_PROTOCOL_OUI & port_entry.protocol))
            continue;

        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, oui, mask, AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid == vlan_port_info.port_item.dot1q_pvid_index)
            {
                if(port_entry.oui_learned_count > 0)
                    ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, --(port_entry.oui_learned_count));
            }
            else if(addr_entry.vid == voice_vlan_id)
            {
                if(VAL_voiceVlanPortSecurity_disabled == port_entry.security_state)
                    continue;

                add_mgr_handle_amtr_addr_changed_callback = FALSE;
                if(TRUE == AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac))
                {
                    add_mgr_handle_amtr_addr_changed_callback = TRUE;
                    ADD_MGR_DGB_MSG("DeleteAddr vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,port=%ld",
                        addr_entry.vid,
                        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                        lport);
                }
                else
                {
                    add_mgr_handle_amtr_addr_changed_callback = TRUE;
                    ADD_MGR_DGB_MSG("AMTR_MGR_DeleteAddr(%ld,%02X%02X%02X-%02X%02X%02X) fail",
                        addr_entry.vid,
                        addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5]);
                }
                add_mgr_handle_amtr_addr_changed_callback = TRUE;
            }
        }

        /* Disjoin VoiceVLAN if no any telephone device attached
         * (include other discovery protocols)
         */
        if(FALSE == ADD_MGR_IsExistedPhone(lport))
        {
            ret &= ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
            ret &= ADD_MGR_DisjoinVoiceVlanByJoinState(lport, NULL);
        }
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiState
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets OUI entry status based on OUI's
 *          MAC address. If the OUI is not in the database, this function
 *          will create a new oui entry if the oui's capacity is not
 *          exceed the limit.
 * INPUT:   oui               -- the OUI MAC address (key).
 *          status            -- the status of OUI entry;
 *                               VAL_voiceVlanOuiStatus_valid,
 *                               VAL_voiceVlanOuiStatus_invalid.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetOuiState(UI8_T *oui, UI32_T state)
{
    ADD_MGR_VoiceVlanOui_T oui_entry;
    UI8_T temp_desc[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    UI8_T temp_mask[SYS_ADPT_MAC_ADDR_LEN];

    UI8_T dflt_mask[SYS_ADPT_MAC_ADDR_LEN] = {ADD_MGR_DFLT_MASK_ADDR_BYTE1,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE2,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE3,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE4,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE5,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE6};

    if((state != VAL_voiceVlanOuiStatus_valid) &&
       (state != VAL_voiceVlanOuiStatus_invalid))
    {
        ADD_MGR_DGB_MSG("Invalid state");
        return FALSE;
    }

    if(state == VAL_voiceVlanOuiStatus_invalid)
    {
        if(TRUE == ADD_OM_GetOui(oui, temp_mask, temp_desc))
        {
            return ADD_MGR_RemoveOuiEntry(oui, temp_mask);
        }
    }
    else
    {
        if(FALSE == ADD_OM_GetOui(oui, temp_mask, temp_desc))
        {
            memset(&oui_entry, 0, sizeof(oui_entry));
            memcpy(oui_entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(oui_entry.mask, dflt_mask, SYS_ADPT_MAC_ADDR_LEN);

            return ADD_MGR_AddOuiEntry(&oui_entry);
        }
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiMaskAddress
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets mask-address of the sepcified OUI. If
 *          the OUI is not in the database, this function will create a
 *          new OUI entry if the OUI's capacity is not exceed the limit.
 * INPUT:   oui                - the OUI MAC address (key).
 *          mask               - the mask of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetOuiMaskAddress(UI8_T *oui, UI8_T *mask)
{
    ADD_MGR_VoiceVlanOui_T oui_entry;
    UI8_T temp_desc[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    UI8_T temp_mask[SYS_ADPT_MAC_ADDR_LEN];

    BOOL_T ret = TRUE;

    memset(&oui_entry, 0, sizeof(oui_entry));

    if(TRUE == ADD_OM_GetOui(oui, temp_mask, temp_desc))
    {
        if(0 != memcmp(mask, temp_mask, SYS_ADPT_MAC_ADDR_LEN))
        {
            memcpy(oui_entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(oui_entry.mask, mask, SYS_ADPT_MAC_ADDR_LEN);
            strncpy(oui_entry.description, temp_desc, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);

            ret = ADD_MGR_RemoveOuiEntry(oui, temp_mask);
            ret = (ret && ADD_MGR_AddOuiEntry(&oui_entry));
        }
    }
    else
    {
        memcpy(oui_entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(oui_entry.mask, mask, SYS_ADPT_MAC_ADDR_LEN);

        ret = ADD_MGR_AddOuiEntry(&oui_entry);
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetOuiDescription
 * ---------------------------------------------------------------------
 * PURPOSE: This function sets description of the sepcified OUI. If
 *          the OUI is not in the database, this function will create a
 *          new OUI entry if the oui's capacity is not exceed the limit.
 * INPUT:   oui                - the OUI MAC address (key).
 *          description        - the description of the OUI MAC address.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetOuiDescription(UI8_T *oui, UI8_T *description)
{
    ADD_MGR_VoiceVlanOui_T oui_entry;
    UI8_T temp_desc[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    UI8_T temp_mask[SYS_ADPT_MAC_ADDR_LEN];

    UI8_T dflt_mask[SYS_ADPT_MAC_ADDR_LEN] = {ADD_MGR_DFLT_MASK_ADDR_BYTE1,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE2,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE3,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE4,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE5,
                                              ADD_MGR_DFLT_MASK_ADDR_BYTE6};
    BOOL_T ret = TRUE;

    memset(&oui_entry, 0, sizeof(oui_entry));

    if(TRUE == ADD_OM_GetOui(oui, temp_mask, temp_desc))
    {
        ADD_MGR_ENTER_CRITICAL_SECTION();

        ret = ADD_OM_RemoveOui(oui, temp_mask);
        ret = (ret && ADD_OM_AddOui(oui, temp_mask, description));

        ADD_MGR_LEAVE_CRITICAL_SECTION();
    }
    else
    {
        memcpy(oui_entry.oui, oui, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(oui_entry.mask, dflt_mask, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(oui_entry.description, description, SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN);

        ret = ADD_MGR_AddOuiEntry(&oui_entry);
    }

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: This function gets OUI entry based on oui's MAC-address.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entrt             -- the OUI entry.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if(NULL == entry)
    {
        return FALSE;
    }

    return ADD_OM_GetOui(entry->oui, entry->mask, entry->description);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next OUI entry.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetNextOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if(NULL == entry)
    {
        return FALSE;
    }

    return ADD_OM_GetNextOui(entry->oui, entry->mask, entry->description);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextRunningOuiEntry
 * ---------------------------------------------------------------------
 * PURPOSE: Get next RUNNING OUI entry.
 * INPUT:   entry->oui        -- the OUI MAC address (key).
 * OUTPUT:  entry             -- the OUI entry.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOuiEntry(ADD_MGR_VoiceVlanOui_T *entry)
{
    if(NULL == entry)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    while(TRUE == ADD_OM_GetNextOui(entry->oui, entry->mask, entry->description))
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next OUI entry.
 * INPUT:   None.
 * OUTPUT:  oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 *          description       -- the descritption of the OUI MAC address,
 *                               the max length is 30 character.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetNextOui(UI8_T *oui, UI8_T *mask, UI8_T *description)
{
    return ADD_OM_GetNextOui(oui, mask, description);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetNextRunningOui
 * ---------------------------------------------------------------------
 * PURPOSE: Get next RUNNING OUI entry.
 * INPUT:   None.
 * OUTPUT:  oui               -- the OUI MAC address.
 *          mask              -- the mask of the OUI MAC address.
 *          description       -- the descritption of the OUI MAC address,
 *                               the max length is 30 character.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. To get first OUI entry by pass the OUI MAC address =
 *             FFFF-FFFF-FFFF.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetNextRunningOui(UI8_T *oui, UI8_T *mask, UI8_T *description)
{
    while(TRUE == ADD_OM_GetNextOui(oui, mask, description))
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  timeout           -- the aging time, minute unit.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanAgingTime(UI32_T *timeout)
{
    return ADD_OM_GetVoiceVlanAgingTime(timeout);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetRunningVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Get the RUNNING voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  timeout           -- the aging time, minute unit.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T ADD_MGR_GetRunningVoiceVlanAgingTime(UI32_T *timeout)
{
    if(FALSE == ADD_OM_GetVoiceVlanAgingTime(timeout))
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    if(SYS_DFLT_ADD_VOICE_VLAN_TIMEOUT_MINUTE == *timeout)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   timeout           -- the aging time, minute unit.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanAgingTime(UI32_T timeout)
{
    BOOL_T ret;

    ADD_MGR_ENTER_CRITICAL_SECTION();

    ret = ADD_OM_SetVoiceVlanAgingTime(timeout);

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN time-out value.
 * INPUT:   None.
 * OUTPUT:  day/hour/minute.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(UI32_T *day, UI32_T *hour, UI32_T *minute)
{
    UI32_T timeout;

    if((NULL == day) || (NULL == hour) || (NULL == minute))
        return FALSE;

    if(FALSE == ADD_OM_GetVoiceVlanAgingTime(&timeout))
    {
        return FALSE;
    }

    *day = timeout / ADD_TYPE_VOICE_VLAN_DAY_OF_MINUTE;
    timeout = timeout % ADD_TYPE_VOICE_VLAN_DAY_OF_MINUTE;

    *hour = timeout / ADD_TYPE_VOICE_VLAN_HOUR_OF_MINUTE;
    *minute = timeout % ADD_TYPE_VOICE_VLAN_HOUR_OF_MINUTE;

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetVoiceVlanAgingTime
 * ---------------------------------------------------------------------
 * PURPOSE: Set the voice VLAN time-out value.
 * INPUT:   day/hour/minute.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   The range of this time is 5minute~30day.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(UI32_T day, UI32_T hour, UI32_T minute)
{
    UI32_T timeout;
    BOOL_T ret;

    timeout = (day*ADD_TYPE_VOICE_VLAN_DAY_OF_MINUTE) + (hour*ADD_TYPE_VOICE_VLAN_HOUR_OF_MINUTE) + minute;

    ADD_MGR_ENTER_CRITICAL_SECTION();

    ret = ADD_OM_SetVoiceVlanAgingTime(timeout);

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetVoiceVlanEnabledPortList
 * ---------------------------------------------------------------------
 * PURPOSE: Get the voice VLAN enabled port list.
 * INPUT  : None.
 * OUTPUT : port_ar  - The port list. The size of the port list must larger
 *                     than SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST.
 * RETURN : None.
 * NOTES  : The rule for mapping the lport index to port list:
 *          1. Logic port 1-8 map to port list byte 0.
 *          2. Logic port 1 map to bit 7 of byte 0.
 *             Logic port 2 map to bit 6 of byte 0.
 *             Logic port 3 map to bit 5 of byte 0, etc.
 *          3. Logic port 9-15 map to port list byte 1.
 *          4. Logic port 9 map to bit 7 of byte 1, etc.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_GetVoiceVlanEnabledPortList(UI8_T port_ar[])
{
    UI32_T lport;
    UI32_T port_mode;
    UI32_T byte;
    UI32_T shift;

    if (NULL == port_ar)
    {
        ADD_MGR_DGB_MSG("NULL pointer");
        return;
    }

    memset(port_ar, 0, SYS_ADPT_TOTAL_NBR_OF_BYTE_FOR_1BIT_PORT_LIST);

    if (FALSE == ADD_OM_IsVoiceVlanEnabled())
    {
        return;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; lport++)
    {
        byte  = (lport-1) / 8;
        shift = (lport-1) % 8;

        if (FALSE == ADD_OM_GetVoiceVlanPortMode(lport, &port_mode))
            continue;

        if (VAL_voiceVlanPortMode_none != port_mode)
        {
            port_ar[byte] |=  ((0x01) << (7 - shift));
        }

    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : ADD_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  - There is a response need to send.
 *    FALSE - There is no response to send.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T ADD_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
    {
        return FALSE;
    }

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        ADD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    cmd = ADD_MGR_MSG_CMD(ipcmsg_p);
    switch(cmd)
    {
        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_ENABLED_ID:
        {
            ADD_MGR_SetVoiceVlanEnabledIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanEnabledId(param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanEnabledIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_ID:
        {
            ADD_MGR_GetVoiceVlanIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanId(&param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_ID:
        {
            ADD_MGR_GetRunningVoiceVlanIdParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanId(&param_p->vid);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanIdParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_IS_VOICE_VLAN_ENABLED:
        {
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_IsVoiceVlanEnabled();
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_SetVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortMode(param_p->lport, param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_GetVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortMode(param_p->lport, &param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_MODE:
        {
            ADD_MGR_GetRunningVoiceVlanPortModeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortMode(param_p->lport, &param_p->mode);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortModeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_SetVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortSecurityState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_GetVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortSecurityState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_SECURITY_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortSecurityStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortSecurityState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortSecurityStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortOuiRuleState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortOuiRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_OUI_RULE_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortOuiRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortOuiRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortLldpRuleState(param_p->lport, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortLldpRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_LLDP_RULE_STATE:
        {
            ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortLldpRuleState(param_p->lport, &param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortLldpRuleStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_SetVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanPortPriority(param_p->lport, param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_GetVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortPriority(param_p->lport, &param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_PORT_PRIORITY:
        {
            ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanPortPriority(param_p->lport, &param_p->priority);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanPortPriorityParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_PORT_REMAIN_AGE:
        {
            ADD_MGR_GetVoiceVlanPortRemainAgeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanPortRemainAge(param_p->lport, &param_p->remain_age);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanPortRemainAgeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_ADD_OUI_ENTRY:
        {
            ADD_MGR_AddOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_AddOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_AddOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_OUI_ENTRY:
        {
            ADD_MGR_GetOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_OUI_ENTRY:
        {
            ADD_MGR_GetNextOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI_ENTRY:
        {
            ADD_MGR_GetNextRunningOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextRunningOuiEntry(&param_p->entry);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextRunningOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_NEXT_RUNNING_OUI:
        {
            ADD_MGR_GetNextRunningOuiParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetNextRunningOui(param_p->oui, param_p->mask, param_p->description);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetNextOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_REMOVE_OUI_ENTRY:
        {
            ADD_MGR_RemoveOuiEntryParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_RemoveOuiEntry(param_p->oui, param_p->mask);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_RemoveOuiEntryParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_STATE:
        {
            ADD_MGR_SetOuiStateParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiState(param_p->oui, param_p->state);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiStateParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_MASK_ADDRESS:
        {
            ADD_MGR_SetOuiMaskAddressParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiMaskAddress(param_p->oui, param_p->mask);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiMaskAddressParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_OUI_DESCRIPTION:
        {
            ADD_MGR_SetOuiDescriptionParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetOuiDescription(param_p->oui, param_p->description);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetOuiDescriptionParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_SetVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanAgingTime(param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_GetVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanAgingTime(&param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_RUNNING_VOICE_VLAN_AGING_TIME:
        {
            ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetRunningVoiceVlanAgingTime(&param_p->timeout);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetRunningVoiceVlanAgingTimeParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE:
        {
            ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinute(param_p->day, param_p->hour, param_p->minute);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetVoiceVlanAgingTimeByDayHourMinuteParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_GET_VOICE_VLAN_AGING_TIME_BY_DAY_HOUR_MINUTE:
        {
            ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinute(&param_p->day, &param_p->hour, &param_p->minute);
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_GetVoiceVlanAgingTimeByDayHourMinuteParam_T);
        }
        break;

        case ADD_MGR_IPC_CMD_SET_DEBUG_PRINT_STATUS:
        {
            ADD_MGR_SetDebugPrintStatusParam_T *param_p = ADD_MGR_MSG_DATA(ipcmsg_p);
            ADD_MGR_SetDebugPrintStatus(param_p->state);
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = ADD_MGR_GET_MSGBUFSIZE(ADD_MGR_SetDebugPrintStatusParam_T);
        }
        break;

        defaule:
        {
            ADD_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            return TRUE;
        }
        break;
    }

    return TRUE;
}


void ADD_MGR_ProcessTimeoutEvent()
{
    UI32_T lport;
    static UI32_T time = ADD_TYPE_TIMER_EVENT_OF_SEC;

    if (0 < time)
    {
        time --;
    }

    if (time == 0)
    {
        time = ADD_TYPE_TIMER_EVENT_OF_SEC;


        for(lport=1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        {
            ADD_MGR_ProcessTimeoutMessage(lport);
        }
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessTimeoutMessage
 * ---------------------------------------------------------------------
 * PURPOSE: Process the timeout message.
 * INPUT:   lport             -- the logic port index.
 * OUTPUT:  None.
 * RETURN:  TRUE              -- Succeeds.
 *          FALSE             -- Fails.
 * NOTES:   1. This function would be invoked by periodic timer event.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_ProcessTimeoutMessage(UI32_T lport)
{
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    if(FALSE == ADD_MGR_IS_LPORT(lport) )
    {
        return FALSE;
    }

    /*Get OM*/
    if(FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

    if((FALSE == ADD_OM_IsVoiceVlanEnabled())                ||
       (VAL_voiceVlanPortMode_auto != port_entry.mode)       ||
       (ADD_TYPE_DISCOVERY_PROTOCOL_NONE == port_entry.join_state)||
       (ADD_TYPE_VOICE_VLAN_TIMER_DISABLED == port_entry.disjoin_when))
    {
        return TRUE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();
    if (port_entry.disjoin_when > 0)
    {
        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, --(port_entry.disjoin_when));
    }

    if(0 == port_entry.disjoin_when)
    {
        ADD_MGR_DisjoinVoiceVlanByJoinState(lport, NULL);
        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SetDebugPrintStatus
 * ---------------------------------------------------------------------
 * PURPOSE: Set the debug print status.
 * INPUT:   state             -- the state of the debug print for the MGR;
 *                               TRUE:  enable the debug print.
 *                               FALSE: disable the debug print.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_SetDebugPrintStatus(BOOL_T state)
{
    ADD_MGR_ENTER_CRITICAL_SECTION();

    add_mgr_debug_print = state;

    ADD_MGR_LEAVE_CRITICAL_SECTION();
}

/* LOCAL SUBPROGRAM DEFINITIONS
 */
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_EnableVoiceVlanPortOui
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable OUI discovery protocol on specified lport.
 * INPUT:   lport                       - logic port index
 *          state                       - TRUE, enable oui
 *                                        FALSE, disable
 *          protocol                    - current protocol bitmap
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   1. delete all mac address that match oui table on voice vlan
            2. (enable)auto join to voice vlan if detected telephone mac
 *             address by oui
 *          2. (disable)disjoin from voice vlan
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_EnableVoiceVlanPortOui(UI32_T lport, BOOL_T state, UI8_T protocol)
{
    UI32_T i;
    UI32_T number_of_mac;
    I32_T  voice_vlan_id;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
//    AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T       addr_entry;

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret = TRUE;
    UI8_T mac_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    UI8_T  mac_address[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask_address[SYS_ADPT_MAC_ADDR_LEN];

    /* Get OM
     */
    if((FALSE == VLAN_MGR_GetPortEntry(lport, &vlan_port_info)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

    i = 0;

    /* Get OUI entry
     */
    memset(mac_address, 255, sizeof(mac_address));
    while(TRUE == ADD_OM_GetNextOui(mac_address, mask_address, description))
    {
        if(i >= SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT)
        {
            ADD_MGR_DGB_MSG("Out of Range.");
            return FALSE;
        }

        memcpy(mac_list_ar[i], mac_address, SYS_ADPT_MAC_ADDR_LEN);
        memcpy(mask_list_ar[i], mask_address, SYS_ADPT_MAC_ADDR_LEN);

        ++i;
    }

    number_of_mac = i;

    /* If security state is enabled,
     * delete all mac address that be set as drop (relearn)
     */
    if(VAL_voiceVlanPortSecurity_enabled == port_entry.security_state)
    {
        ADD_OM_GetVoiceVlanId(&voice_vlan_id);
        for(i=0; i<number_of_mac; ++i)
        {
            memset(&addr_entry, 0, sizeof(addr_entry));
            addr_entry.ifindex/*l_port*/ = lport;
            addr_entry.vid = voice_vlan_id;
            while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, mac_list_ar[i], mask_list_ar[i], AMTR_MGR_GET_ALL_ADDRESS))
            {
                if(addr_entry.vid != voice_vlan_id)
                    break;
#if (SYS_CPNT_LLDP == TRUE)
                if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
                {
                    if(TRUE == LLDP_MGR_IsTelephoneMac(lport, addr_entry.mac, SYS_ADPT_MAC_ADDR_LEN))
                        continue;
                }
#endif

                add_mgr_handle_amtr_addr_changed_callback = FALSE;
                AMTR_MGR_DeleteAddr(addr_entry.vid, addr_entry.mac);
                add_mgr_handle_amtr_addr_changed_callback = TRUE;
            }
        }
    }/* End of if(TRUE == per_security_state) */

    ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, 0);

    /* Enable OUI protocol
     */
    if(TRUE == state)
    {
        /* Auto join to voice VLAN if there have phone MAC address be found on
         * native VLAN
         */
        for(i=0; i<number_of_mac; ++i)
        {
            memset(&addr_entry, 0, sizeof(addr_entry));
            addr_entry.ifindex/*l_port*/ = lport;
            addr_entry.vid = vlan_port_info.port_item.dot1q_pvid_index;
            while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry, mac_list_ar[i], mask_list_ar[i], AMTR_MGR_GET_ALL_ADDRESS))
            {
                if(addr_entry.vid != vlan_port_info.port_item.dot1q_pvid_index)
                    break;
                ADD_MGR_ProcessNewMac(addr_entry.vid, addr_entry.mac, addr_entry.ifindex/*l_port*/);
            }
        }
    }
    else/* disable OUI protocol */
    {
        if(port_entry.join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
        {
            if(port_entry.join_state == ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
            {
                ret = ADD_MGR_DisjoinVoiceVlanByJoinState(lport, NULL);
                ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
            }
            else
            {
                ret = ADD_OM_ClearVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);
                ret &= ADD_MGR_SavePerJoinStateToCfgdb();
            }
        }
    }
    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_EnableVoiceVlanPortLldp
 * ---------------------------------------------------------------------
 * PURPOSE: Enable/Disable LLDP discovery protocol on specified lport.
 * INPUT:   lport                       - logic port index
 *          state                       - TRUE, enable lldp
 *                                        FALSE, disable
 *          protocol                    - current protocol bitmap
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   1. delete all mac address that match lldp telephone mac
 *             address on voice vlan
 *          2. (enable)auto join to voice vlan if detected telephone device
 *             by lldp
 *          2. (disable)disjoin from voice vlan
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_EnableVoiceVlanPortLldp(UI32_T lport, BOOL_T state, UI8_T protocol)
{
#if (SYS_CPNT_LLDP == TRUE)
    UI32_T i;
    UI32_T number_of_mac;
    I32_T  voice_vlan_id;
    UI32_T network_addr_ifindex;
    UI32_T rem_dev_index;
    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret = TRUE;
    UI8_T  mac_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mac_address[SYS_ADPT_MAC_ADDR_LEN] = {0};
    UI8_T  network_addr_subtype;
    UI8_T  network_addr[ADD_MGR_MAX_NETWORK_ADDR_LENGTH];
    UI8_T  network_addr_len;

    /* Get OM
     */
    if(FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

    i = 0;
    rem_dev_index = 0;
    while(TRUE == LLDP_OM_GetNextTelephoneDevice(lport, &rem_dev_index, mac_address, &network_addr_subtype, network_addr, &network_addr_len, &network_addr_ifindex))
    {
        ADD_MGR_DGB_MSG("mac(%02X%02X%02X-%02X%02X%02X)",
            mac_address[0],mac_address[1],mac_address[2],
            mac_address[3],mac_address[4],mac_address[5]);

        if(i >= SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT)
        {
            ADD_MGR_DGB_MSG("Out of Range.");
            return FALSE;
        }

        if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
        {
            if(TRUE == ADD_OM_IsOui(mac_address))
                continue;
        }

        ADD_MGR_DGB_MSG(" (add)");
        memcpy(mac_list_ar[i], mac_address, SYS_ADPT_MAC_ADDR_LEN);

        ++i;
    }

    number_of_mac = i;

    /* Currently if security state is enabled, delete telephone mac address that be break by oui table.
     */
    if(VAL_voiceVlanPortSecurity_enabled == port_entry.security_state)
    {
        ADD_OM_GetVoiceVlanId(&voice_vlan_id);
        for(i=0; i<number_of_mac; ++i)
        {
            add_mgr_handle_amtr_addr_changed_callback = FALSE;
            AMTR_MGR_DeleteAddr(voice_vlan_id, mac_list_ar[i]);
            add_mgr_handle_amtr_addr_changed_callback = TRUE;
        }
    }

    /* enable LLDP protocol
     */
    if(TRUE == state)
    {
        /* Auto join to voice VLAN if there have phone be found by LLDP
         */
        if(TRUE == ADD_MGR_IsDetectedPhoneByLldp(lport))
        {
            ADD_MGR_DGB_MSG("detect phone by lldp");
            ret = ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP, NULL);
        }
    }
    else /* disable lldp protocol */
    {
        if(port_entry.join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
        {
            if(port_entry.join_state == ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
            {
                ret = ADD_MGR_DisjoinVoiceVlanByJoinState(lport, NULL);
                ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
            }
            else
            {
                ret = ADD_OM_ClearVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP);
                ret &= ADD_MGR_SavePerJoinStateToCfgdb();
            }
        }
    }
    return ret;
#else
    return FALSE;
#endif /*#if (SYS_CPNT_LLDP == TRUE)*/
}


static BOOL_T ADD_MGR_RemapAddress(AMTR_TYPE_AddrEntry_T *entry_p, UI8_T priority)
{
    entry_p->action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    entry_p->priority = priority;

    if(TRUE == AMTR_MGR_SetAddrEntryWithPriority(entry_p))
    {
        ADD_MGR_DGB_MSG("Remap(%d) vid=%ld,mac(%c)=%02X%02X%02X-%02X%02X%02X,port=%ld",
            priority,
            entry_p->vid,
            (entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)?'D':
            (entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
                || entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET)?'S':'?',
            entry_p->mac[0], entry_p->mac[1], entry_p->mac[2], entry_p->mac[3], entry_p->mac[4], entry_p->mac[5],
            entry_p->ifindex
        );
        return TRUE;
    }

    ADD_MGR_DGB_MSG("Fails vid=%ld,mac(%c)=%02X%02X%02X-%02X%02X%02X,port=%ld,priority=%ld",

        entry_p->vid,
        (entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)?'D':
            (entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET
                || entry_p->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET)?'S':'?',
        entry_p->mac[0], entry_p->mac[1], entry_p->mac[2], entry_p->mac[3], entry_p->mac[4], entry_p->mac[5],
        entry_p->ifindex,
        (UI32_T)priority
    );

    return FALSE;
}

#ifdef VXWORKS_FRAMER
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_RemapAddress
 * ---------------------------------------------------------------------
 * PURPOSE: Remap the priority on the specified MAC address.
 * INPUT:   vid                         - vlan id
 *          mac                         - mac address
 *          lport                       - logic port index
 *          addr_attribute              - attribute of the mac address
 *          priority                    - priority
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_RemapAddress(UI32_T vid, UI8_T *mac, UI32_T lport, UI32_T addr_attribute, UI8_T priority)
{
    UI32_T attribute;

    switch(addr_attribute)
    {
        case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT:
        case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_PSEC:
        case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_PSEC_STATIC:
        case AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_DROP:
            attribute = AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT;
            break;

        case AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT:
        case AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT_PSEC:
        case AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT_DROP:
            attribute = AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT;
            break;

        case AMTR_MGR_ENTRY_ATTRIBUTE_DELETE_ON_RESET:
        case AMTR_MGR_ENTRY_ATTRIBUTE_DELETE_ON_RESET_DROP:
            attribute = AMTR_MGR_ENTRY_ATTRIBUTE_DELETE_ON_RESET;
            break;

        default:
            ADD_MGR_DGB_MSG("Unsupport address attribute(%ld)",  addr_attribute);
            return FALSE;
    }

    if(TRUE == AMTR_MGR_SetAddrEntryWithPriority(vid, mac, lport, attribute, priority))
    {
        ADD_MGR_DGB_MSG("Remap(%d) vid=%ld,mac(%c)=%02X%02X%02X-%02X%02X%02X,port=%ld",
            priority,
            vid,
            (attribute==AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT)?'D':(attribute==AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT)?'S':'d',
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            lport);
        return TRUE;
    }

    ADD_MGR_DGB_MSG("Fails vid=%ld,mac(%c)=%02X%02X%02X-%02X%02X%02X,port=%ld,priority=%ld",
        vid,
        (attribute==AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT)?'D':(attribute==AMTR_MGR_ENTRY_ATTRIBUTE_PERMANENT)?'S':'d',
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        lport,
        (UI32_T)priority);

    return FALSE;
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_JoinVoiceVlan
 * ---------------------------------------------------------------------
 * PURPOSE: Join to Voice VLAN.
 * INPUT:   lport                       - logic port index
 *          protocol                    - protocol
 * OUTPUT:  notify_change_join_state    - TRUE, if the join state of the
 *                                        lport was changed
 *                                        FALSE, else.
 * RETURN:  TRUE/FALSE.
 * NOTES:   If pass NULL to notify_change_join_state then this API will
 *          save join state to CFGDB immediately when the join state was
 *          changed.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_JoinVoiceVlan(UI32_T lport, UI8_T protocol, BOOL_T *notify_change_join_state)
{
    I32_T  voice_vlan_id;
    UI32_T voice_vlan_id_ifindex;
    BOOL_T change_join_state = FALSE;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    /*Get OM*/
    if (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

    if(ADD_TYPE_DISCOVERY_PROTOCOL_NONE == port_entry.join_state)
    {
        if (   (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id))
            || (FALSE == VLAN_OM_ConvertToIfindex(voice_vlan_id, &voice_vlan_id_ifindex))
           )
        {
            ADD_MGR_DGB_MSG("VLAN_OM_ConvertToIfindex(%ld, ?) failed",  voice_vlan_id);
            return FALSE;
        }

        if(FALSE == VLAN_OM_IsPortVlanMember(voice_vlan_id_ifindex, lport))
        {
            ADD_MGR_DGB_MSG("Add port(%ld) to Voice VLAN", lport);

            if(FALSE == VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE))
            {
                ADD_MGR_DGB_MSG("Fail to add port(%ld) to Voice VLAN",  lport);
                return FALSE;
            }
        }
    }

    /* update OUI learned count
     */
    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        if(0 == (port_entry.join_state & ADD_TYPE_DISCOVERY_PROTOCOL_OUI))
        {
            ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI);
            change_join_state = TRUE;
        }

        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
        ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, ++(port_entry.oui_learned_count));
    }

    if(protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {
        if(0 == (port_entry.join_state & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
        {
            ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP);
            change_join_state = TRUE;
        }

        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
    }

    if(NULL == notify_change_join_state)
    {
        ADD_MGR_SavePerJoinStateToCfgdb();
    }
    else
    {
        *notify_change_join_state = TRUE;
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_DisjoinVoiceVlanByJoinState
 * ---------------------------------------------------------------------
 * PURPOSE: Disjoin the logic port from Voice VLAN by the join state.
 * INPUT:   lport                       - logic port index
 * OUTPUT:  notify_change_join_state    - TRUE, if the join state of the
 *                                        lport was changed
 *                                        FALSE, else.
 * RETURN:  TRUE/FALSE.
 * NOTES:   If pass NULL to notify_change_join_state then this API will
 *          save join state to CFGDB immediately when the join state was
 *          changed.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_DisjoinVoiceVlanByJoinState(UI32_T lport, BOOL_T *notify_change_join_state)
{
    I32_T  voice_vlan_id;
    UI32_T voice_vlan_id_ifindex;
    UI8_T  prev_per_join_state;

    ADD_OM_GetVoiceVlanId(&voice_vlan_id);
    ADD_OM_GetVoiceVlanPortJoinState(lport, &prev_per_join_state);
    if(prev_per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
    {
        ADD_OM_GetVoiceVlanId(&voice_vlan_id);
        VLAN_OM_ConvertToIfindex(voice_vlan_id, &voice_vlan_id_ifindex);

        if(TRUE == VLAN_OM_IsPortVlanMember(voice_vlan_id_ifindex, lport))
        {
            add_mgr_handle_amtr_addr_changed_callback = FALSE;
            if(FALSE == VLAN_MGR_DeleteEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE))
            {
                /* FALSE, if this logic port join vlan by static method */
                ADD_MGR_DGB_MSG("Fail to disjoin auto joined VLAN(%ld) from port(%ld).",  voice_vlan_id, lport);

                /* Remove ALL mac address from the Voice VLAN if can't remove the Voice VLAN */
                if (FALSE == AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport, voice_vlan_id, NULL, NULL, 0))
                //if(FALSE == AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr(lport, voice_vlan_id, NULL, NULL, 0))
                {
                    ADD_MGR_DGB_MSG("AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr fails");
                }
            }
            add_mgr_handle_amtr_addr_changed_callback = TRUE;
        }

        /* Update join state and save to cfgdb if changed */
        ADD_OM_ClearVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_ALL);
        if(NULL == notify_change_join_state)
        {
            if(FALSE == ADD_MGR_SavePerJoinStateToCfgdb())
            {
                ADD_MGR_DGB_MSG("ADD_MGR_SavePerJoinStateToCfgdb fails");
            }
        }
        else
        {
            *notify_change_join_state = TRUE;
        }
    }
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PostChangeToAutoMode
 * ---------------------------------------------------------------------
 * PURPOSE: Post process for changing to auto mode.
 * INPUT:   lport                       - logic port index
 *          from                        - change from which mode
 * OUTPUT:  notify_change_join_state    - TRUE, if the join state of the
 *                                        lport was changed
 *                                        FALSE, else.
 * RETURN:  TRUE/FALSE.
 * NOTES:   If pass NULL to notify_change_join_state then this API will
 *          save join state to CFGDB immediately when the join state was
 *          changed.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_PostChangeToAutoMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state)
{
    UI32_T number_of_mac;
    I32_T  voice_vlan_id;
    UI32_T i;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
//    AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T       addr_entry;

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret;
    UI8_T  mac_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];

    /*Get OM*/
    if((FALSE == ADD_MGR_GetPhoneMac(lport, mac_list_ar, mask_list_ar, &number_of_mac)) ||
       (FALSE == VLAN_MGR_GetPortEntry(lport, &vlan_port_info)) ||
       (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

    /* If the security state is ENABLED,
     * delete all non-phone mac address on Voice VLAN id
     */
    if(VAL_voiceVlanPortSecurity_enabled == port_entry.security_state)
    {
        add_mgr_handle_amtr_addr_changed_callback = FALSE;
        if (FALSE == AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport, voice_vlan_id, mac_list_ar, mask_list_ar, number_of_mac))
        //if(FALSE == AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr(lport, voice_vlan_id, &mac_list_ar, &mask_list_ar, number_of_mac))
        {
            add_mgr_handle_amtr_addr_changed_callback = TRUE;
            ADD_MGR_DGB_MSG("AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr fails");
        }
        add_mgr_handle_amtr_addr_changed_callback = TRUE;
    }

    ret = TRUE;
    /* Auto join to VoiceVLAN if there are same phone devices detected on PVID */
    for(i = 0; i<number_of_mac; ++i)
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        addr_entry.vid = vlan_port_info.port_item.dot1q_pvid_index;

        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry,mac_list_ar[i], mask_list_ar[i], AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != vlan_port_info.port_item.dot1q_pvid_index)
            {
                break;
            }

            ADD_MGR_DGB_MSG("Get vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,lport=%ld.",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                addr_entry.ifindex/*l_port*/);

            ADD_MGR_JoinVoiceVlan(lport, port_entry.protocol, notify_change_join_state);
        }
    }

    /* Search All MAC address if security status is disabled */
    if(VAL_voiceVlanPortSecurity_disabled == port_entry.security_state)
    {
        number_of_mac = 1;
        memset(mac_list_ar, 0, SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT*SYS_ADPT_MAC_ADDR_LEN);
        memset(mask_list_ar, 0, SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT*SYS_ADPT_MAC_ADDR_LEN);
    }

    /* For the VoiceVLAN mac to remap the priority */
    for(i = 0; i<number_of_mac; ++i)
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        addr_entry.vid = voice_vlan_id;

        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry,mac_list_ar[i], mask_list_ar[i], AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != voice_vlan_id)
            {
                break;
            }

            ADD_MGR_DGB_MSG("Get vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,lport=%ld.",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                addr_entry.ifindex/*l_port*/);

            /* Support ingress-filter is disabled to auto join Voice VLAN after rebooted */
            ADD_MGR_JoinVoiceVlan(lport, port_entry.protocol, notify_change_join_state);

            /* Remap */
            ret &= ADD_MGR_RemapAddress(/*addr_entry.vid, addr_entry.mac, addr_entry.l_port, addr_entry.attribute*/&addr_entry, port_entry.priority);
        }/*End of while(TRUE == AMTR_MGR_GetNextVMAddrEntry_ByLPortNMaskMatch(... */
    }/*End of for(i = 0 ... */

    if(port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {
        if(TRUE == ADD_MGR_IsDetectedPhoneByLldp(lport))
        {
            ret &= ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP, NULL);
        }
    }/* End of if(per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP) */

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PostChangeToManualMode
 * ---------------------------------------------------------------------
 * PURPOSE: Post process for changing to manual mode.
 * INPUT:   lport                       - logic port index
 *          from                        - change from which mode
 * OUTPUT:  notify_change_join_state    - TRUE, if the join state of the
 *                                        lport was changed
 *                                        FALSE, else.
 * RETURN:  TRUE/FALSE.
 * NOTES:   If pass NULL to notify_change_join_state then this API will
 *          save join state to CFGDB immediately when the join state was
 *          changed.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_PostChangeToManualMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state)
{
    UI32_T number_of_mac;
    I32_T  voice_vlan_id;
    UI32_T i;
    //AMTR_MGR_AddrEntry_T        addr_entry;
    AMTR_TYPE_AddrEntry_T   addr_entry;

    ADD_OM_VoiceVlanPortEntry_T port_entry;
    BOOL_T ret;
    UI8_T  mac_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  mask_list_ar[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN];

    /* Disjoin the auto joined VLAN */
    if(FALSE == ADD_MGR_DisjoinVoiceVlanByJoinState(lport, notify_change_join_state))
    {
        ADD_MGR_DGB_MSG("ADD_MGR_DisjoinVoiceVlanByJoinState fails");
        return FALSE;
    }

    /*Get OM*/
    if((FALSE == ADD_MGR_GetPhoneMac(lport, mac_list_ar, mask_list_ar, &number_of_mac)) ||
       (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return FALSE;
    }

/*
    if(FALSE == ADD_MGR_GetPhoneMac(lport, mac_list_ar, mask_list_ar, &number_of_mac))
    {
        if(TRUE == add_mgr_debug_print)
        {
            ADD_MGR_DGB_MSG("ADD_MGR_GetPhoneMac fails");
        }
        return FALSE;
    }
*/

    /* If the security state is ENABLED,
     * delete all non-phone mac address on Voice VLAN id
     */
    if(VAL_voiceVlanPortSecurity_enabled == port_entry.security_state)
    {
        add_mgr_handle_amtr_addr_changed_callback = FALSE;

        if (FALSE == AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport, voice_vlan_id, mac_list_ar, mask_list_ar, number_of_mac))
        //if(FALSE == AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr(lport, voice_vlan_id, &mac_list_ar, &mask_list_ar, number_of_mac))
        {
            add_mgr_handle_amtr_addr_changed_callback = TRUE;
            ADD_MGR_DGB_MSG("AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr fails");
        }
        add_mgr_handle_amtr_addr_changed_callback = TRUE;
    }

    /* Search All MAC address if security status is disabled */
    if(VAL_voiceVlanPortSecurity_disabled == port_entry.security_state)
    {
        number_of_mac = 1;
        memset(mac_list_ar, 0, SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT*SYS_ADPT_MAC_ADDR_LEN);
        memset(mask_list_ar, 0, SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT*SYS_ADPT_MAC_ADDR_LEN);
    }

    ret = TRUE;
    /* For the VoiceVLAN mac to remap the priority */
    for(i = 0; i<number_of_mac; ++i)
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex/*l_port*/ = lport;
        addr_entry.vid = voice_vlan_id;

        ADD_MGR_DGB_MSG("mac(%02X%02X%02X-%02X%02X%02X),mask(%02X%02X%02X-%02X%02X%02X)",
              mac_list_ar[i][0],mac_list_ar[i][1],mac_list_ar[i][2],mac_list_ar[i][3],mac_list_ar[i][4],mac_list_ar[i][5],
              mask_list_ar[i][0],mask_list_ar[i][1],mask_list_ar[i][2],mask_list_ar[i][3],mask_list_ar[i][4],mask_list_ar[i][5]);

        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry,mac_list_ar[i], mask_list_ar[i], AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != voice_vlan_id)
            {
                break;
            }

            ADD_MGR_DGB_MSG("Get vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,lport=%ld.",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2],
                addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                addr_entry.ifindex/*l_port*/);

            /* Remap */
            ret &= ADD_MGR_RemapAddress(/*addr_entry.vid, addr_entry.mac, addr_entry.l_port, addr_entry.attribute*/&addr_entry, port_entry.priority);
        }/*End of while(TRUE == AMTR_MGR_GetNextVMAddrEntry_ByLPortNMaskMatch(... */
    }/*End of for(i = 0 ... */

    return ret;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PostChangeToNoneMode
 * ---------------------------------------------------------------------
 * PURPOSE: Post process for changing to none mode.
 * INPUT:   lport                       - logic port index
 *          from                        - change from which mode
 * OUTPUT:  notify_change_join_state    - TRUE, if the join state of the
 *                                        lport was changed
 *                                        FALSE, else.
 * RETURN:  TRUE/FALSE.
 * NOTES:   If pass NULL to notify_change_join_state then this API will
 *          save join state to CFGDB immediately when the join state was
 *          changed.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_PostChangeToNoneMode(UI32_T lport, UI32_T from, BOOL_T *notify_change_join_state)
{
    I32_T  voice_vlan_id;
    UI8_T  per_join_state;

    switch(from)
    {
    case VAL_voiceVlanPortMode_none:
        ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
        if(per_join_state != ADD_TYPE_DISCOVERY_PROTOCOL_NONE)
        {
            ADD_OM_ClearVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_ALL);
            if(NULL == notify_change_join_state)
            {
                if(FALSE == ADD_MGR_SavePerJoinStateToCfgdb())
                {
                    ADD_MGR_DGB_MSG("ADD_MGR_SavePerJoinStateToCfgdb fails");
                }
            }
            else
            {
                *notify_change_join_state = TRUE;
            }
        }
        return TRUE;

    case VAL_voiceVlanPortMode_manual:
        ADD_OM_GetVoiceVlanId(&voice_vlan_id);

        return AMTR_MGR_DeleteAddrByVidAndLPortExceptCertainAddr(lport, voice_vlan_id, NULL, NULL, 0);
        //return AMTR_MGR_DeleteAddr_ByLportNVidExceptCertainAddr(lport, voice_vlan_id, NULL, NULL, 0);

    case VAL_voiceVlanPortMode_auto:
        return ADD_MGR_DisjoinVoiceVlanByJoinState(lport, notify_change_join_state);

    default:
        ADD_MGR_DGB_MSG("No such port mode");
        return FALSE;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_SavePerJoinStateToCfgdb
 * ---------------------------------------------------------------------
 * PURPOSE: Save the join state to CFGDB.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_SavePerJoinStateToCfgdb()
{
#if (SYS_CPNT_CFGDB == TRUE)
    UI32_T lport;
    UI8_T join_state_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};

    if(0 == add_mgr_join_state_handler)
        return FALSE;

    for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        ADD_OM_GetVoiceVlanPortJoinState(lport, &join_state_ar[lport-1]);

    return CFGDB_MGR_WriteSection(add_mgr_join_state_handler, join_state_ar);
#else
    return TRUE;
#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsValidLPortForVoiceVlan
 * ---------------------------------------------------------------------
 * PURPOSE: Check the vailid for the logic port.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
static
#endif /* (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
BOOL_T ADD_MGR_IsValidLPortForVoiceVlan(UI32_T lport)
{
    VLAN_OM_Vlan_Port_Info_T vlan_om_port_info;

    if(FALSE == SWCTRL_LogicalPortExisting(lport))
    {
        ADD_MGR_DGB_MSG("No support VoiceVLAN feature on trunk port.");
        return FALSE;
    }

    VLAN_MGR_GetPortEntry(lport, &vlan_om_port_info);
    if(VAL_vlanPortMode_access == vlan_om_port_info.vlan_port_entry.vlan_port_mode)
    {
        ADD_MGR_DGB_MSG("No support VoiceVLAN feature on access port.");
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_GetPhoneMac
 * ---------------------------------------------------------------------
 * PURPOSE: Get all phone MAC address on spicified lport.
 * INPUT:   lport                       - logic port index
 * OUTPUT:  mac_list_p                  - 2D array for mac address.
 *          mask_list_p                 - 2D array for mask address.
 *          get_nuber                   - the number of all phone MAC
 *                                        address on this port
 * RETURN:  TRUE/FALSE.
 * NOTES:   .
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_GetPhoneMac(UI32_T lport,
                                  UI8_T mac_list[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN],
                                  UI8_T mask_list[SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT][SYS_ADPT_MAC_ADDR_LEN],
                                  UI32_T *get_number)
{
    UI32_T network_addr_ifindex;
    UI32_T rem_dev_index;
    int   i;
    UI8_T mac_address[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T mask_address[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T description[SYS_ADPT_ADD_VOICE_VLAN_MAX_OUI_DESC_LEN+1];
    UI8_T per_protocol;
    UI8_T network_addr_subtype;
    UI8_T network_addr[ADD_MGR_MAX_NETWORK_ADDR_LENGTH];
    UI8_T network_addr_len;

    if(NULL == get_number)
    {
        ADD_MGR_DGB_MSG("Null pointer.");
        return FALSE;
    }

    if(FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_protocol))
    {
        ADD_MGR_DGB_MSG("ADD_OM_GetVoiceVlanPortRuleBitmap");
        return FALSE;
    }

    i = 0;

    /* Get all OUI entry
     */
    if(per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        memset(mac_address, 255, sizeof(mac_address));
        while(TRUE == ADD_OM_GetNextOui(mac_address, mask_address, description))
        {
            if(i >= SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT)
            {
                ADD_MGR_DGB_MSG("Out of Range.");
                return FALSE;
            }

            memcpy(mac_list[i], mac_address, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(mask_list[i], mask_address, SYS_ADPT_MAC_ADDR_LEN);

            ++i;
        }
    }

    /* Get all phone MAC address from LLDP
     */
    if(per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {

#if (SYS_CPNT_LLDP == TRUE)
        memset(mac_address, 0, SYS_ADPT_MAC_ADDR_LEN);
        memset(mask_address, 1, SYS_ADPT_MAC_ADDR_LEN);
        rem_dev_index = 0;
        while(TRUE == LLDP_OM_GetNextTelephoneDevice(lport, &rem_dev_index, mac_address, &network_addr_subtype, network_addr, &network_addr_len, &network_addr_ifindex))
        {
            if(i >= SYS_ADPT_MAX_NBR_OF_PHONE_OUI_PER_PORT)
            {
                ADD_MGR_DGB_MSG("Out of Range.");
                return FALSE;
            }

            memcpy(mac_list[i], mac_address, SYS_ADPT_MAC_ADDR_LEN);
            memcpy(mask_list[i], mask_address, SYS_ADPT_MAC_ADDR_LEN);

            ++i;
        }
#endif

    }

    *get_number = i;
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsDetectedPhoneByOui
 * ---------------------------------------------------------------------
 * PURPOSE: Is detect any phone by OUI.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_IsDetectedPhoneByOui(UI32_T lport)
{
    UI32_T                  per_oui_learned_count;
    UI8_T                   per_rule_bitmap;
    AMTR_TYPE_AddrEntry_T   addr_entry;
    I32_T                   vvid;
    ADD_MGR_VoiceVlanOui_T  oui_entry;

    /* Check the OUI rule is enabled?
     */
    if(   (FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_rule_bitmap))
       || !(per_rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
      )
    {
        return FALSE;
    }

    /* Check the Learned Count of OM
     */
    /*if(   (TRUE == ADD_OM_GetVoiceVlanPortOuiLearnedCount(lport, &per_oui_learned_count))
       && (per_oui_learned_count != 0)
      )
    {
        return TRUE;
    }*/


    ADD_OM_GetVoiceVlanId(&vvid);

    if (vvid == VAL_voiceVlanEnabledId_disabled)
        return FALSE;

    memset(oui_entry.oui, 0xFF, sizeof(oui_entry.oui));

    while (ADD_OM_GetNextOui(oui_entry.oui, oui_entry.mask, oui_entry.description))
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex = lport;
        addr_entry.vid = vvid;

        while(TRUE == AMTR_MGR_GetNextVMAddrEntryByLPortNMaskMatch(&addr_entry,
                                                                   oui_entry.oui,
                                                                   oui_entry.mask,
                                                                   AMTR_MGR_GET_ALL_ADDRESS))
        {
            if(addr_entry.vid != vvid)
                break;

            ADD_MGR_DGB_MSG("Detected phone by OUI: vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,port=%ld",
                addr_entry.vid,
                addr_entry.mac[0], addr_entry.mac[1], addr_entry.mac[2], addr_entry.mac[3], addr_entry.mac[4], addr_entry.mac[5],
                lport);

            ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, 1);
            return TRUE;
        }
    }

    ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, 0);
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsDetectedPhoneByLldp
 * ---------------------------------------------------------------------
 * PURPOSE: Is detect any phone by LLDP.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_IsDetectedPhoneByLldp(UI32_T lport)
{
#if (SYS_CPNT_LLDP == TRUE)

    LLDP_MGR_PortRxStatistics_T per_statistics_entry;
    UI8_T  per_rule_bitmap;

    /* Check the LLDP rule is enabled?
     */
    if(   (FALSE == ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_rule_bitmap))
       || !(per_rule_bitmap & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
      )
    {
        return FALSE;
    }

    memset(&per_statistics_entry, 0, sizeof(LLDP_MGR_PortRxStatistics_T));
    per_statistics_entry.port_num = lport;

    /* Check the Learned Count of LLDP
     */
    if(    (TRUE == LLDP_MGR_GetPortRxStatisticsEntry(&per_statistics_entry))
        && (0 != per_statistics_entry.rx_telephone_total)
      )
    {
            return TRUE;
    }

#endif /*#if (SYS_CPNT_LLDP == TRUE)*/
    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsExistedPhone
 * ---------------------------------------------------------------------
 * PURPOSE: Is any phone attached on this logic port.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_IsExistedPhone(UI32_T lport)
{
    if(TRUE == ADD_MGR_IsDetectedPhoneByOui(lport))
        return TRUE;

    if(TRUE == ADD_MGR_IsDetectedPhoneByLldp(lport))
        return TRUE;

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_IsExistedPhone
 * ---------------------------------------------------------------------
 * PURPOSE: Is phone mac on specified lport.
 * INPUT:   lport              - logic port index
 *          mac                - MAC address
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_IsPhoneMac(UI32_T lport, UI8_T *mac)
{
    UI8_T per_protocol;

    ADD_OM_GetVoiceVlanPortRuleBitmap(lport, &per_protocol);

    if(per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        if(TRUE == ADD_OM_IsOui(mac))
        {
            return TRUE;
        }
        else
        {
            ADD_MGR_DGB_MSG("No found in OUI table.");
        }
    }

    if(per_protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP)
    {
#if (SYS_CPNT_LLDP == TRUE)
        if(TRUE == LLDP_MGR_IsTelephoneMac(lport, mac, SYS_ADPT_MAC_ADDR_LEN))
        {
            return TRUE;
        }
        else
        {
            ADD_MGR_DGB_MSG("Not detected phone by LLDP.");
        }
#endif /*#if (SYS_CPNT_LLDP == TRUE)*/
    }

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AMTR_SetStaticMac_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function by user set a static mac address.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 *          is_add
 * OUTPUT:  None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
static
#endif /* (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
BOOL_T ADD_MGR_AMTR_SetStaticMacCheck_CallBack(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    UI32_T per_mode;
    BOOL_T pass_the_mac;

    ADD_MGR_DGB_MSG("vid(%ld),mac(%02X%02X%02X-%02X%02X%02X),port(%ld).",
        vid,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        lport);

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)",  lport);
        return TRUE;
    }

    if(FALSE == ADD_OM_GetVoiceVlanPortMode(lport, &per_mode))
    {
        ADD_MGR_DGB_MSG("Get OM fails on port (%ld) fails",  lport);
        return TRUE;
    }

    if((FALSE == ADD_OM_IsVoiceVlanEnabled())   ||
       (VAL_voiceVlanPortMode_none == per_mode))
    {
        return TRUE;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    pass_the_mac = ADD_MGR_ProcessNewMac(vid, mac, lport);

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return pass_the_mac;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PortLinkUp_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be invoked when port link up.
 * INPUT:   unit                - unit
 *          port                - port
 * OUTPUT:  None.
 * RETURN:
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_MGR_PortLinkUp_CallBack(UI32_T unit, UI32_T port)
{
    UI32_T lport;
    UI32_T mode;

    SWCTRL_UserPortToIfindex(unit, port, &lport);

    ADD_MGR_DGB_MSG("lport(%ld).", lport);

    /* Because we disjoin voice VLAN when this port link down (not by callback
     * event). So that we need rejoin to voice VLAN at this port link up.
     */
    if (ADD_OM_IsVoiceVlanEnabled()
        && ADD_OM_GetVoiceVlanPortMode(lport, &mode)
        && VAL_voiceVlanPortMode_auto == mode)
    {
        if (ADD_MGR_IsDetectedPhoneByOui(lport))
        {
            ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI, NULL);
        }

        if (ADD_MGR_IsDetectedPhoneByLldp(lport))
        {
            ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP, NULL);
        }
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_PortLinkDown_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be invoked when port link down.
 * INPUT:   unit                - unit
 *          port                - port
 * OUTPUT:  None.
 * RETURN:
 * NOTES:
 * ---------------------------------------------------------------------
 */
void ADD_MGR_PortLinkDown_CallBack(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    SWCTRL_UserPortToIfindex(unit, port, &lport);

    ADD_MGR_DGB_MSG("lport(%ld).", lport);

    if (ADD_OM_IsVoiceVlanEnabled())
    {
        ADD_MGR_DisjoinVoiceVlanByJoinState(lport, NULL);
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: Process the new MAC address.
 *          1. Check for security issue
 *          2. Auto join to Voice VLAN when OUI bit is trun-on
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 *          is_add
 * OUTPUT:  None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
BOOL_T ADD_MGR_ProcessNewMac(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    ADD_MGR_DGB_MSG("vid(%ld),mac(%02X%02X%02X-%02X%02X%02X),port(%ld).",
        vid,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        lport);

    /* Deny access when authorize failure.
     */
    if (FALSE == ADD_MGR_AuthorizeNewMac(vid, mac, lport))
    {
        return FALSE;
    }

    /* Doesn't need to check the return value.
     */
    ADD_MGR_AutoJoinVoiceVlanByNewMac(vid, mac, lport);

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AuthorizeNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: To authorize the new MAC.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, success to authorize the new MAC address. FALSE, else.
 * ---------------------------------------------------------------------
 */
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
    static
#endif /* (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
BOOL_T ADD_MGR_AuthorizeNewMac(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    I32_T  voice_vlan_id;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    /* Get OM
     */
    if (    (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id))
        ||  (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
       )
    {
        ADD_MGR_DGB_MSG("Get OM fails on port(%ld) fails",  lport);
        return FALSE;
    }

    /* Pass the new MAC directly when
     *  1. Voice VLAN feature is disabled.
     *  2. The security mode of this port is disabled.
     *  3. The MAC is not on voice VLAN.
     */
    if (    (VAL_voiceVlanEnabledId_disabled == voice_vlan_id)
         || (VAL_voiceVlanPortMode_none == port_entry.mode)
         || (VAL_voiceVlanPortSecurity_disabled == port_entry.security_state)
         || (vid != voice_vlan_id)
       )
    {
        return TRUE;
    }

    return ADD_MGR_IsPhoneMac(lport, mac);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_AutoJoinToVoiceVlanByNewMac
 * ---------------------------------------------------------------------
 * PURPOSE: To authorize the new MAC.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, success to authorize the new MAC address. FALSE, else.
 * ---------------------------------------------------------------------
 */
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
    static
#endif /* (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
BOOL_T ADD_MGR_AutoJoinVoiceVlanByNewMac(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    I32_T                       voice_vlan_id;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    /* Get OM
     */
    if (    (FALSE == VLAN_MGR_GetPortEntry(lport, &vlan_port_info))
        ||  (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id))
        ||  (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry))
       )
    {
        ADD_MGR_DGB_MSG("Get OM fails on port(%ld) fails",  lport);
        return FALSE;
    }

    /* Return directly when
     *  1. Voice VLAN feature is disabled.
     *  2. The port mode is not auto mode.
     *  3. The bit of detecting by OUI bit is not be configured.
     */
    if (    (VAL_voiceVlanEnabledId_disabled == voice_vlan_id)
         || (VAL_voiceVlanPortMode_auto != port_entry.mode)
         || (!(port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI))
       )
    {
        return TRUE;
    }

#if (SYS_CPNT_ADD_DETECT_NATIVE_VLAN_ONLY == TRUE)
    if (vid != vlan_port_info.port_item.dot1q_pvid_index)
    {
        /* Bypass to increase oui_learned_count in ADD_MGR_JoinVoiceVlan()
         * when the port join to voice VLAN already and vid equals VVID.
         */
        if (vid == voice_vlan_id && TRUE == port_entry.join_state)
        {
        }
        else
        {
            return TRUE;
        }
    }
#endif

    /* Not phone device MAC, nothing to do.
     */
    if (TRUE == ADD_OM_IsOui(mac))
    {
        /* Join to voice VLAN
         */
        return ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_OUI, NULL);
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessAgedMac
 * ---------------------------------------------------------------------
 * PURPOSE: Process the aged mac address.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE, authenticated the mac address. FALSE, else.
 * NOTES:   1. Return TRUE when exception occured.
 * ---------------------------------------------------------------------
 */
#if (VOICE_VLAN_DO_UNIT_TEST == TRUE)
#else
    static
#endif /* (VOICE_VLAN_DO_UNIT_TEST == TRUE) */
BOOL_T ADD_MGR_ProcessAgedMac(UI32_T vid, UI8_T *mac, UI32_T lport)
{
    I32_T  voice_vlan_id;
    UI32_T timeout;
    VLAN_OM_Vlan_Port_Info_T    vlan_port_info;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    ADD_MGR_DGB_MSG("vid(%ld),mac(%02X%02X%02X-%02X%02X%02X),port(%ld).",
        vid,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        lport);

    /* Get OM */
    if((FALSE == VLAN_MGR_GetPortEntry(lport, &vlan_port_info)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)) ||
       (FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id)))
    {
        ADD_MGR_DGB_MSG("Get OM fails");
        return TRUE;
    }

    /* No process age event at manual mode */
    if(VAL_voiceVlanPortMode_manual == port_entry.mode)
        return TRUE;

    if(vid == voice_vlan_id)
    {
        if(VAL_voiceVlanPortSecurity_enabled == port_entry.security_state)
        {
            /* Not phone device mac */
            if(FALSE == ADD_MGR_IsPhoneMac(lport, mac))
            {
                return FALSE;
            }
        }
    }
    else if(vid == vlan_port_info.port_item.dot1q_pvid_index)
    {
        /* Not phone device mac */
        if(FALSE == ADD_OM_IsOui(mac))
            return TRUE;
    }
#if (SYS_CPNT_ADD_DETECT_NATIVE_VLAN_ONLY == TRUE)
    else
    {
        return TRUE;
    }
#endif

    if(port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_OUI)
    {
        /* Decrease the oui learned count */
        if(port_entry.oui_learned_count > 0)
            ADD_OM_SetVoiceVlanPortOuiLearnedCount(lport, --(port_entry.oui_learned_count));

        if(FALSE == ADD_MGR_IsExistedPhone(lport))
        {
            ADD_OM_GetVoiceVlanAgingTime(&timeout);
#if (ADD_DEBUG_SHORTER_TIMER_EVENT == TRUE)
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout);
#else
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout*(ADD_TYPE_VOICE_VLAN_MINUTE_OF_SECOND/ADD_TYPE_TIMER_EVENT_OF_SEC));
#endif
        }
    }

    return TRUE;
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessAgedMac
 * ---------------------------------------------------------------------
 * PURPOSE: This function be callbacked when AMTR notify a new/delete mac
 *          address.
 * INPUT:   vid                - VLAN ID
 *          mac                - MAC address
 *          lport              - logic port index
 *          is_age             - TRUE, an aged mac address. FALSE, else.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_AMTR_EditAddrNotify_CallBack(UI32_T vid, UI8_T *mac, UI32_T lport, BOOL_T is_age)
{
    I32_T voice_vlan_id;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    if(FALSE == add_mgr_handle_amtr_addr_changed_callback)
        return;

    ADD_MGR_DGB_MSG("vid(%ld),mac(%02X%02X%02X-%02X%02X%02X),port(%ld) %s.",
        vid,
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
        lport,
        (is_age==TRUE)?"age":"new");

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)",  lport);
        return;
    }

    if((FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
    {
        ADD_MGR_DGB_MSG("Get OM fails");
        return;
    }

    if((FALSE == ADD_OM_IsVoiceVlanEnabled()) ||
       (VAL_voiceVlanPortMode_none == port_entry.mode))
    {
        return;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    /* new address */
    if(FALSE == is_age)
    {
        if(TRUE == ADD_MGR_ProcessNewMac(vid, mac, lport))
        {
            if(vid == voice_vlan_id)
            {
                AMTR_TYPE_AddrEntry_T addr_entry;

                memset(&addr_entry, 0, sizeof(addr_entry));

                addr_entry.vid = vid;
                memcpy(addr_entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
                addr_entry.ifindex = lport;
                addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
                addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
                addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
                /* Remap */
                ADD_MGR_RemapAddress(/*vid, mac, lport, AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT*/&addr_entry, port_entry.priority);
            }
        }
        else
        {
            if(vid == voice_vlan_id)
            {
                /* drop */
                AMTR_TYPE_AddrEntry_T addr_entry;

                memset(&addr_entry, 0, sizeof(addr_entry));

                addr_entry.vid = vid;
                memcpy(addr_entry.mac, mac, SYS_ADPT_MAC_ADDR_LEN);
                addr_entry.ifindex = lport;
                addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_DISCARD_BY_SA_MATCH;
                addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_LEARN;
                addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;

                AMTR_MGR_SetAddrEntry(&addr_entry);

                /*
                AMTR_MGR_SetFilteringAddrrEntry(vid, mac, lport, AMTR_MGR_ENTRY_ATTRIBUTE_LEARNT_DROP);
                 */
                ADD_MGR_DGB_MSG("Drop vid=%ld,mac=%02X%02X%02X-%02X%02X%02X,port=%ld",
                    vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], lport);
            }
        }
    }
    else /* aged */
    {
        ADD_MGR_ProcessAgedMac(vid, mac, lport);
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessCamePhoneByLldp
 * ---------------------------------------------------------------------
 * PURPOSE: Process a came phone notify.
 * INPUT:   lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_ProcessCamePhoneByLldp(UI32_T lport)
{
    I32_T  voice_vlan_id;
    UI32_T voice_vlan_id_ifindex;
    UI8_T  per_join_state;

    ADD_OM_GetVoiceVlanPortJoinState(lport, &per_join_state);
    if(!(per_join_state & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
    {
        if(ADD_TYPE_DISCOVERY_PROTOCOL_NONE == per_join_state)
        {
            ADD_OM_GetVoiceVlanId(&voice_vlan_id);
            VLAN_OM_ConvertToIfindex(voice_vlan_id, &voice_vlan_id_ifindex);
            if(FALSE == VLAN_OM_IsPortVlanMember(voice_vlan_id_ifindex, lport))
            {
                if(FALSE == VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE))
                {
                    ADD_MGR_DGB_MSG("Fail to add port(%ld) to Voice VLAN",  lport);
                    return FALSE;
                }
            }
        }
        ADD_OM_SetVoiceVlanPortJoinState(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP);
        ADD_MGR_SavePerJoinStateToCfgdb();
    }

    ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, ADD_TYPE_VOICE_VLAN_TIMER_DISABLED);
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_ProcessLeftPhoneByLldp
 * ---------------------------------------------------------------------
 * PURPOSE: Process a left phone notify.
 * INPUT:   lport              - logic port index
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
static BOOL_T ADD_MGR_ProcessLeftPhoneByLldp(UI32_T lport)
{
    UI32_T timeout;

    if(FALSE == ADD_MGR_IsDetectedPhoneByOui(lport))
    {
        ADD_OM_GetVoiceVlanAgingTime(&timeout);
#if (ADD_DEBUG_SHORTER_TIMER_EVENT == TRUE)
        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout);
#else
        ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout*(ADD_TYPE_VOICE_VLAN_MINUTE_OF_SECOND/ADD_TYPE_TIMER_EVENT_OF_SEC));
#endif
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - ADD_MGR_LLDP_TelephoneDetect_CallBack
 * ---------------------------------------------------------------------
 * PURPOSE: This function be callbacked when LLDP notify a came/left
 *          phone notify.
 * INPUT:   lport              - logic port index
 *          mac_addr             - the MAC address
 *          network_addr_subtype - the subtype of the network address
 *          network_addr         - the network address
 *          network_addr_len     - the length of the network address
 *          network_addr_ifindex - the VLAN ID
 *          tel_exist          - TRUE, a came phone notify. FALSE,else.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 * ---------------------------------------------------------------------
 */
void ADD_MGR_LLDP_TelephoneDetect_CallBack(UI32_T  lport,
                                           UI8_T  *mac_addr,
                                           UI8_T   network_addr_subtype,
                                           UI8_T  *network_addr,
                                           UI8_T   network_addr_len,
                                           UI32_T  network_addr_ifindex,
                                           BOOL_T  tel_exist)
{
    I32_T voice_vlan_id;
    UI32_T timeout;
    ADD_OM_VoiceVlanPortEntry_T port_entry;

    if(TRUE == add_mgr_debug_print)
    {
        ADD_MGR_DGB_MSG("mac(%02X%02X%02X-%02X%02X%02X)",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

        if(network_addr_len != 0)
        {
            char add_str[50];

            if(network_addr_subtype == VAL_lldpLocManAddrSubtype_ipV4)
                sprintf(add_str, " ipV4(%d.%d.%d.%d)", network_addr[0], network_addr[1], network_addr[2], network_addr[3]);
            else if(network_addr_subtype == VAL_lldpLocManAddrSubtype_ipV6)
                sprintf(add_str, " ipV6");
            else if(network_addr_subtype == VAL_lldpLocManAddrSubtype_all802)
                sprintf(add_str, " MAC");

            ADD_MGR_DGB_MSG("%s", add_str);
        }

        ADD_MGR_DGB_MSG(" VID(%ld) lport(%ld) %s", network_addr_ifindex, lport, (tel_exist==TRUE)?"exist":"leave");
    }

    if(FALSE == ADD_MGR_IS_LPORT(lport))
    {
        ADD_MGR_DGB_MSG("Invalid port index (%ld)",  lport);
        return;
    }

    if((FALSE == ADD_OM_GetVoiceVlanId(&voice_vlan_id)) ||
       (FALSE == ADD_OM_GetVoiceVlanPortEntry(lport, &port_entry)))
    {
        ADD_MGR_DGB_MSG("Get OM fails");
        return;
    }

    if((VAL_voiceVlanEnabledId_disabled == voice_vlan_id) ||
       (port_entry.mode != VAL_voiceVlanPortMode_auto) ||
       !(port_entry.protocol & ADD_TYPE_DISCOVERY_PROTOCOL_LLDP))
    {
        return;
    }

    ADD_MGR_ENTER_CRITICAL_SECTION();

    /* Telephone came
     */
    if(TRUE == tel_exist)
    {
        if(FALSE == ADD_MGR_JoinVoiceVlan(lport, ADD_TYPE_DISCOVERY_PROTOCOL_LLDP, NULL))
        {
            ADD_MGR_DGB_MSG("ADD_MGR_JoinVoiceVlan fails");
        }
    }
    else/* Telephone leave */
    {
        /* Enable timeout if there have no phone on this lport
         */
        if(FALSE == ADD_MGR_IsDetectedPhoneByOui(lport))
        {
            ADD_OM_GetVoiceVlanAgingTime(&timeout);
        #if (ADD_TYPE_DEBUG_SHORTER_TIMER_EVENT == TRUE)
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout);
        #else
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout*(ADD_TYPE_VOICE_VLAN_MINUTE_OF_SECOND/ADD_TYPE_TIMER_EVENT_OF_SEC));
        #endif
        }
    }

    ADD_MGR_LEAVE_CRITICAL_SECTION();
    return;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_CLI_ProvisionComplete_CallBack
 *-----------------------------------------------------------
 * FUNCTION: Auto join the port to voice VLAN when the join
 *           state of the port had be set.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
void ADD_MGR_CLI_ProvisionComplete_CallBack()
{
#if (SYS_CPNT_CFGDB == TRUE)
    UI32_T lport;
    UI32_T per_mode;
    I32_T  voice_vlan_id;
    UI32_T voice_vlan_id_ifindex;
    UI32_T timeout;
    UI8_T  default_join_state_ar[SYS_ADPT_TOTAL_NBR_OF_LPORT] = {0};

    if(0 == add_mgr_join_state_handler)
    {
        return;
    }

    ADD_OM_GetVoiceVlanId(&voice_vlan_id);

    /* Voice VLAN Enable */
    if(VAL_voiceVlanEnabledId_disabled != voice_vlan_id)
    {
        if(FALSE == CFGDB_MGR_ReadSection(add_mgr_join_state_handler, default_join_state_ar))
        {
            SYSFUN_Debug_Printf("Read join state fail", __FUNCTION__);
            return;
        }

        if(FALSE == VLAN_OM_ConvertToIfindex(voice_vlan_id, &voice_vlan_id_ifindex))
        {
            SYSFUN_Debug_Printf("Vlan%ld is not exist",  voice_vlan_id);
            return;
        }

//SYSFUN_Sleep(500); // wait VLAN ready ??

        ADD_MGR_ENTER_CRITICAL_SECTION();

        for(lport = 1; lport <= SYS_ADPT_TOTAL_NBR_OF_LPORT; ++lport)
        {
            if(ADD_TYPE_DISCOVERY_PROTOCOL_NONE == default_join_state_ar[lport-1])
            {
                continue;
            }

            /* Process next if geting port mode fails */
            if(FALSE == ADD_OM_GetVoiceVlanPortMode(lport, &per_mode))
            {
                default_join_state_ar[lport-1] = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
                continue;
            }

            /* Process next if the port mode is not auot mode */
            if(per_mode != VAL_voiceVlanPortMode_auto)
            {
                default_join_state_ar[lport-1] = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
                continue;
            }

            /* Join the lport to voice VLAN by voice status */
            if(FALSE == VLAN_OM_IsPortVlanMember(voice_vlan_id_ifindex, lport))
            {
                if(FALSE == VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport, VLAN_TYPE_VLAN_STATUS_VOICE))
                {
                    SYSFUN_Debug_Printf("Join lport(%ld) to voice VLAN fail",  lport);
                    default_join_state_ar[lport-1] = ADD_TYPE_DISCOVERY_PROTOCOL_NONE;
                    continue;
                }
            }

            ADD_OM_SetVoiceVlanPortJoinState(lport, default_join_state_ar[lport-1]);

            /* Enable timer if auto join to Voice VLAN. */
            ADD_OM_GetVoiceVlanAgingTime(&timeout);
#if (ADD_TYPE_DEBUG_SHORTER_TIMER_EVENT == TRUE)
            /* easy to debug with the shorter timer event */
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout);
#else
            ADD_OM_SetVoiceVlanPortDisjoinWhen(lport, timeout*(ADD_TYPE_VOICE_VLAN_MINUTE_OF_SECOND/ADD_TYPE_TIMER_EVENT_OF_SEC));
#endif
        }

        ADD_MGR_LEAVE_CRITICAL_SECTION();
    }

    if(FALSE == CFGDB_MGR_WriteSection(add_mgr_join_state_handler, default_join_state_ar))
    {
        SYSFUN_Debug_Printf("Save join state fail", __FUNCTION__);
    }

#endif /*#if (SYS_CPNT_CFGDB == TRUE)*/
}

/*-----------------------------------------------------------
 * ROUTINE NAME - ADD_MGR_VLAN_VlanMemberDelete_CallBack
 *-----------------------------------------------------------
 * FUNCTION: Process same stuff when a member of vlan be removed.
 * INPUT   :  vid_ifindex    -- specify which vlan's member set to delete from.
 *            lport_ifindex  -- sepcify which lport to delete from the member set.
 *            vlan_status    -- VAL_dot1qVlanStatus_other
 *                              VAL_dot1qVlanStatus_permanent
 *                              VAL_dot1qVlanStatus_dynamicGvrp.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : The port shall re-join to voice VLAN.
 *           step1. The port is auto port and join to voice VLAN already.
 *           step2. It be change from voice to static membership of VLAN by user.
 *           step3. Then user remove it from voice VLAN as like manually join.
 *----------------------------------------------------------*/
void ADD_MGR_VLAN_VlanMemberDelete_CallBack(UI32_T vid_ifindex, UI32_T lport_ifindex, UI32_T vlan_status)
{
    I32_T  voice_vlan_id;
    UI32_T voice_vlan_id_ifindex;
    UI32_T per_mode;
    UI8_T  per_join_state;

    /* trigger by add
     * event when disable voice VLAN or remove OUI
     */
    if (FALSE == add_mgr_handle_amtr_addr_changed_callback)
    {
        return;
    }

    ADD_OM_GetVoiceVlanId(&voice_vlan_id);

    if(FALSE == VLAN_OM_ConvertToIfindex(voice_vlan_id, &voice_vlan_id_ifindex))
    {
        ADD_MGR_DGB_MSG("Fail to conver voice VLAN ID(%ld) to ifindex",  voice_vlan_id);
        return;
    }

    /* vid_ifindex is not voice VLAN
     */
    if (voice_vlan_id_ifindex != vid_ifindex)
        return;

    if (    (FALSE == ADD_OM_GetVoiceVlanPortMode(lport_ifindex, &per_mode))
         || (FALSE == ADD_OM_GetVoiceVlanPortJoinState(lport_ifindex, &per_join_state))
       )
    {
        ADD_MGR_DGB_MSG("Fail to get information from OM on port(%ld)",  lport_ifindex);
        return;
    }

    /* Note: No use critical section here
     */

    /* re-join to voice VLAN, if the port is auto port and join to voice VLAN already.
     */
    if (   (VAL_voiceVlanPortMode_auto == per_mode)
        && (TRUE == per_join_state)
       )
    {
        if(FALSE == VLAN_MGR_AddEgressPortMember(voice_vlan_id, lport_ifindex, VLAN_TYPE_VLAN_STATUS_VOICE))
        {
            ADD_MGR_DGB_MSG("Fail to add port(%ld) to Voice VLAN",  lport_ifindex);

            /* to do: fail to re-join to voice VLAN
             */
        }
    }
}
#endif  /* #if (SYS_CPNT_ADD == TRUE) */

