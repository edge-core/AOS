/* INCLUDE FILE DECLARATIONS
 */
#include <assert.h>

#include "sysrsc_mgr.h"
#include "sys_bld.h"
#include "sys_hwcfg.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "swdrv_type.h"
#include "swdrv_om.h"
#include "stktplg_om.h"
#include "string.h"
#include "sys_dflt.h"
#include "uc_mgr.h"

/* STATIC VARIABLE DEFINITIONS
 */
static SWDRV_TYPE_ShmemData_T *shmem_data_p;

static UI32_T swdrv_om_sem_id;

/* MACRO FUNCTION DECLARATIONS
 */
#define SWDRV_OM_EnterCriticalSection() SYSFUN_TakeSem(swdrv_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define SWDRV_OM_LeaveCriticalSection() SYSFUN_GiveSem(swdrv_om_sem_id)

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SWDRV_OM_ResetSfpPresentBitShift(void);
static void SWDRV_OM_ResetSfpPresentStatusAddr(void);
static void SWDRV_OM_ResetSfpPresentMask(void);
static void SWDRV_OM_ResetSfpRxLos(void);

/* EXPORTED SUBPROGRAM BODIES
 */
void SWDRV_OM_HotSwapRemove(UI32_T unit_id)
{
     SWDRV_OM_EnterCriticalSection();
    //memset(shmem_data_p->swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
     if (0<unit_id && unit_id<=SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
     {
         memset(&(shmem_data_p->swdrv_port_link_status[unit_id-1]), 0, sizeof(SWDRV_LinkStatus_T));
         memset(&(shmem_data_p->swdrv_port_sfp_present_status[unit_id-1]), 0, sizeof(SWDRV_TYPE_SfpPresentStatus_T));
     }
     SWDRV_OM_LeaveCriticalSection();
}

void SWDRV_OM_Reset()
{
    SWDRV_OM_EnterCriticalSection();

    memset(shmem_data_p->swdrv_port_info, 0, sizeof(SWDRV_Port_Info_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
    memset(&(shmem_data_p->swdrv_craft_port_info), 0, sizeof(SWDRV_CraftPort_Info_T));
    memset(shmem_data_p->swdrv_trunk_info, 0, sizeof(SWDRV_Trunk_Info_T) *(SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM + 1));

    /* water_huang add; 95_9_13
     */
    memset(shmem_data_p->swdrv_port_link_status, 0, sizeof(SWDRV_LinkStatus_T)*(SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK));
    #if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    memset(shmem_data_p->workaround_status, 0, sizeof(UI32_T) *(SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT + 1));
    #endif
    /* clear register function list */
    shmem_data_p->swdrv_unit_bitmap = 0;
    /* clear register function list */
    shmem_data_p->swdrv_provision_complete = FALSE;

#if (SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE)
    memset(shmem_data_p->swdrv_port_local_admin_status, 0, sizeof(shmem_data_p->swdrv_port_local_admin_status));
    memset(shmem_data_p->swdrv_port_local_admin_status_dirty, 0xFF, sizeof(shmem_data_p->swdrv_port_local_admin_status_dirty));
    shmem_data_p->swdrv_pse_check_enable=SYS_DFLT_PSE_CHECK_STATUS;
#endif
    /* Init shmem_data_p->swdrv_sfp_present_bit_shift
     */
    SWDRV_OM_ResetSfpPresentBitShift();

    /* Init shmem_data_p->swdrv_sfp_present_status_addr
     */
    SWDRV_OM_ResetSfpPresentStatusAddr();

    /* Init shmem_data_p->swdrv_sfp_present_mask
     */
    SWDRV_OM_ResetSfpPresentMask();

    /* Init shmem_data_p->swdrv_sfp_rx_los_status_addr
     *      shmem_data_p->swdrv_sfp_rx_los_mask
     *      shmem_data_p->swdrv_sfp_rx_los_bit_shift
     */
    SWDRV_OM_ResetSfpRxLos();

    memset(shmem_data_p->swdrv_port_sfp_present, 0x0, sizeof(BOOL_T)*SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT);
    memset(shmem_data_p->swdrv_port_sfp_info_valid, 0x0, sizeof(BOOL_T)*SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT);
    memset(shmem_data_p->swdrv_port_sfp_present_status, 0, sizeof(SWDRV_TYPE_SfpPresentStatus_T)*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);

    /* initial stack_id with 1*/
    shmem_data_p->swdrv_system_info.stack_id = 1;

    shmem_data_p->swdrv_sfp_present_delay_ticks = SWDRV_TYPE_SFP_PRESENT_DELAY_TICKS;

#if (SYS_CPNT_HASH_SELECTION == TRUE)
    memset(shmem_data_p->hash_block, 0, sizeof(SWDRV_HashSelBlockListMapping_T)*SYS_ADPT_HW_HASH_SELECTION_BLOCK_SIZE);
#endif

    SWDRV_OM_LeaveCriticalSection();
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE: Attach system resource for BUFFERMGMT in the context of the calling
 *          process.
 * INPUT:   None
 * OUTPUT:  None
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_AttachSystemResources(void)
{
    shmem_data_p = (SWDRV_TYPE_ShmemData_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SWDRV_OM, &swdrv_om_sem_id);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterMasterMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterMasterMode (void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterSlaveMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_ProvisionComplete
 *---------------------------------------------------------------------------------
 * PURPOSE: All provision commands are settle down.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  None
 * NOTES:
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_ProvisionComplete(void)
{
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_SetTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Sets to temporary transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_SetTransitionMode (void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_EnterTransitionMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Enter transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
    return;
}


/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_GetOperatingMode
 *---------------------------------------------------------------------------------
 * PURPOSE: Get operating mode
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
UI32_T SWDRV_OM_GetOperatingMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_SetTaskTransitionDone
 *---------------------------------------------------------------------------------
 * PURPOSE: Set task transition done flag.
 * INPUT:   is_transition_done
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_SetTaskTransitionDone(BOOL_T is_transition_done)
{
    shmem_data_p->swdrv_task_is_transition_done = is_transition_done;
}

/*---------------------------------------------------------------------------------
 * FUNCTION NAME: SWDRV_OM_WaitTaskTransitionDone
 *---------------------------------------------------------------------------------
 * PURPOSE: To wait until task receives ENTER TRANSITION MODE.
 * INPUT:   none
 * OUTPUT:  none
 * RETUEN:  none
 * NOTES:   none
 *---------------------------------------------------------------------------------*/
void SWDRV_OM_WaitTaskTransitionDone(void)
{
    SYSFUN_TASK_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p->swdrv_task_is_transition_done);
}

void SWDRV_OM_Init()
{
    UI32_T ret_value;

    shmem_data_p = (SWDRV_TYPE_ShmemData_T *)SYSRSC_MGR_GetShMem(SYSRSC_MGR_SWDRV_SHMEM_SEGID);

    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);

    if ((ret_value = SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_SWDRV_OM, &swdrv_om_sem_id))!=SYSFUN_OK)
    {
        printf("%s: SYSFUN_GetSem return != SYSFUN_OK value=%lu\n",__FUNCTION__, (unsigned long)ret_value);
    }

    SWDRV_OM_Reset();
}

void SWDRV_OM_SetDebugFlag(UI32_T debug_flag, BOOL_T enable)
{
    if (enable)
    {
        shmem_data_p->debug_flag |= debug_flag;
    }
    else
    {
        shmem_data_p->debug_flag &= ~debug_flag;
    }
}

BOOL_T SWDRV_OM_GetDebugFlag(UI32_T debug_flag)
{
    return !!(shmem_data_p->debug_flag & debug_flag);
}

BOOL_T SWDRV_OM_GetChipTaskStatus(BOOL_T *status)
{
    SWDRV_OM_EnterCriticalSection();
    *status = shmem_data_p->swdrv_chip_task_is_created;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetChipTaskStatus(BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_chip_task_is_created = status;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetTrunkMode(UI32_T mode)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_mode = mode;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetUnitBitMap(UI32_T bitmap)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_unit_bitmap = bitmap;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetThreadId(UI32_T thread_id)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_thread_id = thread_id;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetSfpThreadId(UI32_T thread_id)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_sfp_thread_id = thread_id;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetSystemInfoBasePortId(UI16_T base_port_id)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_system_info.base_port_id = base_port_id;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;

}

BOOL_T SWDRV_OM_SetSystemInfoPortNum(UI16_T port_num)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_system_info.port_number = port_num;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoPortType(UI32_T port, UI32_T port_type)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].port_type = port_type;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoExisting(UI32_T port, BOOL_T exist)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].existing  = exist;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_GetSystemInfoStackId(UI32_T *stack_id)
{
    SWDRV_OM_EnterCriticalSection();
    *stack_id = shmem_data_p->swdrv_system_info.stack_id;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetSystemInfoStackUnit(UI32_T unit_idx, UI32_T stack_id)
{
    SWDRV_OM_EnterCriticalSection();
#if (SYS_CPNT_STACKING == TRUE)
    shmem_data_p->swdrv_system_info.stack_unit_tbl[unit_idx] = stack_id;
#endif
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;

}

BOOL_T SWDRV_OM_GetSystemInfo(SWDRV_Switch_Info_T *swdrv_system_info)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(swdrv_system_info, &shmem_data_p->swdrv_system_info, sizeof(SWDRV_Switch_Info_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_ResetSystemInfoStackUnitTable()
{
    SWDRV_OM_EnterCriticalSection();
#if (SYS_CPNT_STACKING == TRUE)
    memset(&shmem_data_p->swdrv_system_info.stack_unit_tbl, 0, sizeof(UI32_T) * SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK);
#endif
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetSystemInfoStackId(UI32_T stack_id)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_system_info.stack_id = (UI8_T)stack_id;
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetComboForceMode(UI32_T port, UI32_T mode)
{
#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].forcedmode = mode;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_GetComboForceSpeed(UI32_T port, UI32_T *fiber_speed_p)
{
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
    SWDRV_OM_EnterCriticalSection();
    *fiber_speed_p = shmem_data_p->swdrv_port_info[port].fiber_speed;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_SetComboForceSpeed(UI32_T port, UI32_T fiber_speed)
{
#if (SYS_CPNT_COMBO_PORT_FORCED_MODE_SFP_SPEED == TRUE)
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].fiber_speed = fiber_speed;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_GetPortInfo(UI32_T port, SWDRV_Port_Info_T *swdrv_port_info)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(swdrv_port_info, &shmem_data_p->swdrv_port_info[port], sizeof(SWDRV_Port_Info_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_GetCraftPortInfo(SWDRV_CraftPort_Info_T *swdrv_craftport_info)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(swdrv_craftport_info, &shmem_data_p->swdrv_craft_port_info, sizeof(SWDRV_CraftPort_Info_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetCraftPortInfoLinkStatus(BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_craft_port_info.link_status = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetSystemInfoStackingPort(UI32_T port)
{
    SWDRV_OM_EnterCriticalSection();
#if (SYS_CPNT_STACKING == TRUE)
    shmem_data_p->swdrv_system_info.stacking_port = port;
#endif
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoLinkStatus(UI32_T port, BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].link_status = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoSpeedDuplexOper(UI32_T port, UI8_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].speed_duplex_oper = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_SetPortInfoFlowControlOper(UI32_T port, UI8_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].flow_control_oper = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoIsAutoNeg(UI32_T port, BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].is_autoneg = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetPortSfpPresent(UI32_T sfp_index, BOOL_T *status_p)
{
    SWDRV_OM_EnterCriticalSection();
    *status_p = shmem_data_p->swdrv_port_sfp_present[sfp_index-1];
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortSfpPresent(UI32_T sfp_index, BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_sfp_present[sfp_index-1] = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetPortSfpInfoValid(UI32_T sfp_index, BOOL_T *valid_p)
{
    SWDRV_OM_EnterCriticalSection();
    *valid_p = shmem_data_p->swdrv_port_sfp_info_valid[sfp_index-1];
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortSfpInfoValid(UI32_T sfp_index, BOOL_T valid)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_sfp_info_valid[sfp_index-1] = valid;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpPresentStatusBitmaps
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp present status bitmaps
 *
 * INPUT   : unit
 * OUTPUT  : present_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpPresentStatusBitmaps(UI32_T unit, SWDRV_TYPE_SfpPresentStatus_T *present_status_p)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(present_status_p, &shmem_data_p->swdrv_port_sfp_present_status[unit-1], sizeof(SWDRV_TYPE_SfpPresentStatus_T));
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpPresentStatusBitmaps
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp present status bitmaps
 *
 * INPUT   : unit
 *           flag -- SWDRV_OM_F_ANNOUNCED: update previous_sfp_present_st_bitmap
 *                   SWDRV_OM_F_REALTIME : update sfp_present_st_bitmap
 *                   SWDRV_OM_F_ALL      : update both
 * OUTPUT  : present_status_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpPresentStatusBitmaps(UI32_T unit, SWDRV_TYPE_SfpPresentStatus_T *present_status_p, SWDRV_OM_FilterFlag_T flag)
{
    SWDRV_OM_EnterCriticalSection();
    switch (flag)
    {
        case SWDRV_OM_F_ANNOUNCED:
            memcpy(&shmem_data_p->swdrv_port_sfp_present_status[unit-1].previous_sfp_present_st_bitmap, present_status_p->previous_sfp_present_st_bitmap, SWDRV_TOTAL_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
            break;

        case SWDRV_OM_F_REALTIME:
            memcpy(&shmem_data_p->swdrv_port_sfp_present_status[unit-1].sfp_present_st_bitmap, present_status_p->sfp_present_st_bitmap, SWDRV_TOTAL_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
            break;

        case SWDRV_OM_F_ALL:
            memcpy(&shmem_data_p->swdrv_port_sfp_present_status[unit-1], present_status_p, sizeof(SWDRV_TYPE_SfpPresentStatus_T));
            break;

        default:
            break;
    }
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp eeprom info
 *
 * INPUT   : sfp_index
 * OUTPUT  : sfp_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpInfo(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_info_p, &shmem_data_p->swdrv_port_sfp_info[sfp_index-1], sizeof(SWDRV_TYPE_SfpInfo_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : sfp_index
 *           sfp_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpInfo(UI32_T sfp_index, SWDRV_TYPE_SfpInfo_T *sfp_info_p)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(&shmem_data_p->swdrv_port_sfp_info[sfp_index-1], sfp_info_p, sizeof(SWDRV_TYPE_SfpInfo_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info
 *
 * INPUT   : sfp_index
 * OUTPUT  : sfp_ddm_info_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpDdmInfo(UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_ddm_info_p, &shmem_data_p->swdrv_port_sfp_ddm_info[sfp_index-1], sizeof(SWDRV_TYPE_SfpDdmInfo_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info
 *
 * INPUT   : sfp_index
 *           sfp_ddm_info_p
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpDdmInfo(UI32_T sfp_index, SWDRV_TYPE_SfpDdmInfo_T *sfp_ddm_info_p)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(&shmem_data_p->swdrv_port_sfp_ddm_info[sfp_index-1], sfp_ddm_info_p, sizeof(SWDRV_TYPE_SfpDdmInfo_T));
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Get port sfp ddm eeprom info raw data
 *
 * INPUT   : sfp_index
 * OUTPUT  : info_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetPortSfpDdmInfoRaw(UI32_T sfp_index, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH])
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(info_ar, &shmem_data_p->swdrv_port_sfp_ddm_info_raw[sfp_index-1], sizeof(UI8_T)*SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH);
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPortSfpDdmInfo
 * -------------------------------------------------------------------------
 * FUNCTION: Set port sfp eeprom info raw data
 *
 * INPUT   : sfp_index
 *           info_ar
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_SetPortSfpDdmInfoRaw(UI32_T sfp_index, UI8_T info_ar[SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH])
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(&shmem_data_p->swdrv_port_sfp_ddm_info_raw[sfp_index-1], info_ar, sizeof(UI8_T)*SWDRV_TYPE_GBIC_EEPROM_MAX_LENGTH);
    SWDRV_OM_LeaveCriticalSection();

    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoSpeedDuplexCfg(UI32_T port, UI8_T status)
{
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].speed_duplex_cfg = status;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortInfoForced1000tMode(UI32_T port, UI8_T status)
{
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].forced_1000t_mode = status;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;

}

BOOL_T SWDRV_OM_SetPortInfoModuleId(UI32_T port, UI8_T module_id)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_port_info[port].module_id = module_id;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_GetTrunkMode(UI32_T *mode)
{
    SWDRV_OM_EnterCriticalSection();
    *mode = shmem_data_p->swdrv_trunk_mode;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_SetTrunkInfoBcmTrunkId(UI32_T trk_idx)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_info[trk_idx-1].bcmdrv_trunk_id = trk_idx;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetTrunkInfoUsed(UI32_T trk_idx, UI8_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_info[trk_idx-1].used = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetSystemInfoNumOfUnit(UI32_T num_of_units)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_system_info.num_of_units = num_of_units;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetProvisionComplete(UI32_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_provision_complete = TRUE;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetProvisionComplete(BOOL_T *status)
{
    SWDRV_OM_EnterCriticalSection();
    *status = shmem_data_p->swdrv_provision_complete;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortLoopbackList(UI8_T loop_back_list[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST*SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK])
{
#if (SYS_CPNT_3COM_LOOPBACK_TEST == TRUE)

    SWDRV_OM_EnterCriticalSection();
    memcpy(shmem_data_p->swdrv_port_loopback_list, loop_back_list, sizeof(loop_back_list));
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_GetTrunkInfo(UI32_T trunk_id, SWDRV_Trunk_Info_T *swdrv_trunk_info)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(swdrv_trunk_info, &shmem_data_p->swdrv_trunk_info[trunk_id-1], sizeof(SWDRV_Trunk_Info_T) );
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_SetThreadIdle(BOOL_T status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_thread_idle = status;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetPortLinkStatusBitmaps(UI32_T unit, SWDRV_LinkStatus_T *link_status)
{
    SWDRV_OM_EnterCriticalSection();
    memcpy(link_status, &shmem_data_p->swdrv_port_link_status[unit-1], sizeof(SWDRV_LinkStatus_T));
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetPortLinkStatusBitmaps(UI32_T unit, SWDRV_LinkStatus_T *link_status, SWDRV_OM_FilterFlag_T flag)
{
    SWDRV_OM_EnterCriticalSection();
    switch (flag)
    {
        case SWDRV_OM_F_ANNOUNCED:
            memcpy(&shmem_data_p->swdrv_port_link_status[unit-1].previous_link_st_bitmap, link_status->previous_link_st_bitmap, SWDRV_TOTAL_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
            break;

        case SWDRV_OM_F_REALTIME:
            memcpy(&shmem_data_p->swdrv_port_link_status[unit-1].link_st_bitmap, link_status->link_st_bitmap, SWDRV_TOTAL_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST);
            break;

        case SWDRV_OM_F_ALL:
            memcpy(&shmem_data_p->swdrv_port_link_status[unit-1], link_status, sizeof(SWDRV_LinkStatus_T));
            break;

        default:
            ;
    }
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetTrunkInfoMemberUnit(UI32_T trk_idx, UI32_T member_idx, UI32_T unit)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_info[trk_idx-1].member_list[member_idx].unit = unit;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_SetTrunkInfoMemberPort(UI32_T trk_idx, UI32_T member_idx, UI32_T port)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_info[trk_idx-1].member_list[member_idx].port = port;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_SetTrunkInfoMemberNum(UI32_T trunk_id, UI32_T port_count)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_trunk_info[trunk_id-1].member_number = port_count;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetThreadId(UI32_T *swdrv_thread_id)
{
    SWDRV_OM_EnterCriticalSection();
    *swdrv_thread_id = shmem_data_p->swdrv_thread_id;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetSfpThreadId(UI32_T *thread_id)
{
    SWDRV_OM_EnterCriticalSection();
    *thread_id = shmem_data_p->swdrv_sfp_thread_id;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

BOOL_T SWDRV_OM_GetUnitBitmap(UI32_T *swdrv_unit_bitmap)
{
    SWDRV_OM_EnterCriticalSection();
    *swdrv_unit_bitmap = shmem_data_p->swdrv_unit_bitmap;
    SWDRV_OM_LeaveCriticalSection();
    return TRUE;

}

BOOL_T SWDRV_OM_GetComboForceMode(UI32_T port, UI32_T *combo_forced_mode)
{
#if (SYS_CPNT_COMBO_PORT_FORCE_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    *combo_forced_mode = shmem_data_p->swdrv_port_info[port].forcedmode;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

BOOL_T SWDRV_OM_GetWorkAroundStauts(UI32_T port, UI32_T *workaround_status)
{
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    *workaround_status = shmem_data_p->workaround_status;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;

}

BOOL_T SWDRV_OM_SetWorkAroundStatus(UI32_T port, UI32_T workaround_status)
{
#if (SYS_CPNT_SUPPORT_FORCED_1000BASE_T_MODE == TRUE)
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->workaround_status = workaround_status;
    SWDRV_OM_LeaveCriticalSection();
#endif
    return TRUE;
}

#if (SYS_HWCFG_SUPPORT_PD==TRUE) && (SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT==TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_UpdateLocalPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Update port admin status of the local unit.
 *
 * INPUT   : unit -- in which unit
 *           port -- which port
 *           is_admin_enable -- TRUE if port admin is enabled.
 * OUTPUT  : None
 * RETURN  : TRUE if update succesfully.
 * NOTE    : When SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT is TRUE
 *           the PHY admin status might be overriden by SWDRV. So need to keep
 *           the PHY admin status set by upper layer.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_UpdateLocalPortAdminStatus(UI32_T unit, UI32_T port, BOOL_T is_admin_enable)
{
    UI32_T my_unit_id;

    if(STKTPLG_OM_GetMyUnitID(&my_unit_id)==FALSE)
    {
        printf("%s(%d)Failed to get my unit id.\r\n", __FUNCTION__, __LINE__);
        return FALSE;
    }

    if(my_unit_id==unit)
    {
        SWDRV_OM_EnterCriticalSection();
        if(is_admin_enable)
        {
            shmem_data_p->swdrv_port_local_admin_status[SWDRV_LPORT_INDEX(port)] |= SWDRV_LPORT_BIT_IN_UI8_T(port);
        }
        else
        {
            shmem_data_p->swdrv_port_local_admin_status[SWDRV_LPORT_INDEX(port)] &= ~(SWDRV_LPORT_BIT_IN_UI8_T(port));
        }
        shmem_data_p->swdrv_port_local_admin_status_dirty[SWDRV_LPORT_INDEX(port)] |= SWDRV_LPORT_BIT_IN_UI8_T(port);
        SWDRV_OM_LeaveCriticalSection();
        return TRUE;
    }
    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetLocalPortAdminStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get port admin status of the local unit.
 *
 * INPUT   : port -- which port
 *           admin_status_p -- port admin status set by upper layer
 *           status_changed_p -- TRUE if the port admin status had ever been
 *                               changed since last read on the specified port
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When SYS_CPNT_SWDRV_ONLY_ALLOW_PD_PORT_LINKUP_WITH_PSE_PORT is TRUE
 *           the PHY admin status might be overriden by SWDRV.
 *           In SWDRV_MonitorPDPortPowerGoodStatus(), it will call this function
 *           to determine the real PHY admin status.
 * -------------------------------------------------------------------------*/
void SWDRV_OM_GetLocalPortAdminStatus(UI32_T port, BOOL_T *admin_status_p, BOOL_T* status_changed_p)
{
    SWDRV_OM_EnterCriticalSection();
    *admin_status_p = (shmem_data_p->swdrv_port_local_admin_status[SWDRV_LPORT_INDEX(port)] & SWDRV_LPORT_BIT_IN_UI8_T(port))?TRUE:FALSE;
    if(shmem_data_p->swdrv_port_local_admin_status_dirty[SWDRV_LPORT_INDEX(port)] & SWDRV_LPORT_BIT_IN_UI8_T(port))
    {
        *status_changed_p=TRUE;
        shmem_data_p->swdrv_port_local_admin_status_dirty[SWDRV_LPORT_INDEX(port)] ^= SWDRV_LPORT_BIT_IN_UI8_T(port);
    }
    else
        *status_changed_p=FALSE;
    SWDRV_OM_LeaveCriticalSection();
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Get PSE check status
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE  -- PSE heck is enabled.
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_GetPSECheckStatus(void)
{
    BOOL_T ret;
    SWDRV_OM_EnterCriticalSection();
    ret= shmem_data_p->swdrv_pse_check_enable;
    SWDRV_OM_LeaveCriticalSection();
    return ret;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetPSECheckStatus
 * -------------------------------------------------------------------------
 * FUNCTION: Set PSE check status
 *
 * INPUT   : pse_check_status  --  TRUE: enable, FALSE: disable
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : When PSE check status is enabled, all of the ports with POE PD
 *           capability are able to link up when the link partner is a PSE port.
 *           When PSE check status is disabled, all of the ports with POE PD
 *           capability will not be blocked by SWDRV to link up. However, if
 *           upper layer CSC shutdown the port, the port is never link up.
 * -------------------------------------------------------------------------*/
void SWDRV_OM_SetPSECheckStatus(BOOL_T pse_check_status)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_pse_check_enable=pse_check_status;
    SWDRV_OM_LeaveCriticalSection();
    return;
}
#endif

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentStatusAddr
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present status address
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_status_addr_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentStatusAddr(UI32_T *sfp_present_status_addr_ar)
{
#if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD != SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
#if defined(SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_present_status_addr_ar, &shmem_data_p->swdrv_sfp_present_status_addr, sizeof(shmem_data_p->swdrv_sfp_present_status_addr));
    SWDRV_OM_LeaveCriticalSection();
#endif
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentMask
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present mask
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_mask_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentMask(UI8_T *sfp_present_mask_ar)
{
#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_present_mask_ar, &shmem_data_p->swdrv_sfp_present_mask, sizeof(shmem_data_p->swdrv_sfp_present_mask));
    SWDRV_OM_LeaveCriticalSection();
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentBitShift
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp present bit shift
 *
 * INPUT   : None
 * OUTPUT  : sfp_present_bit_shift_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpPresentBitShift(UI8_T *sfp_present_bit_shift_ar)
{
#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_present_bit_shift_ar, &shmem_data_p->swdrv_sfp_present_bit_shift, sizeof(shmem_data_p->swdrv_sfp_present_bit_shift));
    SWDRV_OM_LeaveCriticalSection();
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosStatusAddr
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los status address
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_status_addr_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosStatusAddr(UI32_T *sfp_rx_los_status_addr_ar)
{
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_rx_los_status_addr_ar, &shmem_data_p->swdrv_sfp_rx_los_status_addr, sizeof(shmem_data_p->swdrv_sfp_rx_los_status_addr));
    SWDRV_OM_LeaveCriticalSection();
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosMask
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los mask
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_mask_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosMask(UI8_T *sfp_rx_los_mask_ar)
{
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_rx_los_mask_ar, &shmem_data_p->swdrv_sfp_rx_los_mask, sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));
    SWDRV_OM_LeaveCriticalSection();
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpRxLosBitShift
 * -------------------------------------------------------------------------
 * FUNCTION: Get sfp rx los bit shift
 *
 * INPUT   : None
 * OUTPUT  : sfp_rx_los_bit_shift_ar
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
BOOL_T SWDRV_OM_GetSfpRxLosBitShift(UI8_T *sfp_rx_los_bit_shift_ar)
{
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    SWDRV_OM_EnterCriticalSection();
    memcpy(sfp_rx_los_bit_shift_ar, &shmem_data_p->swdrv_sfp_rx_los_bit_shift, sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));
    SWDRV_OM_LeaveCriticalSection();
#endif

    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetSfpPresentDelay
 * -------------------------------------------------------------------------
 * FUNCTION: To get the sfp present delay value in SWDRV_OM
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : The value of Sfp Present Delay in ticks.
 * NOTE    : 1. This function is for debug purpose.
 * -------------------------------------------------------------------------
 */
UI32_T SWDRV_OM_GetSfpPresentDelay(void)
{
    return shmem_data_p->swdrv_sfp_present_delay_ticks;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_SetSfpPresentDelay
 * -------------------------------------------------------------------------
 * FUNCTION: To set the sfp present delay value to SWDRV_OM
 * INPUT   : sfp_present_delay  -  the sfp present delay value(in ticks)
 *                                  to be set
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : 1. This function is for debug purpose.
 * -------------------------------------------------------------------------
 */
void SWDRV_OM_SetSfpPresentDelay(UI32_T sfp_present_delay)
{
    SWDRV_OM_EnterCriticalSection();
    shmem_data_p->swdrv_sfp_present_delay_ticks = sfp_present_delay;
    SWDRV_OM_LeaveCriticalSection();
}

/* LOCAL SUBPROGRAM BODIES
 */
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_ResetSfpPresentBitShift
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize sfp present bit shit from sys_hwcfg
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_OM_ResetSfpPresentBitShift(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }
#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY)
    #if defined(AOS5700_54X)
    if(uc_sys_info.board_id == 0)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_0};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 1)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_1};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else
    {
        printf("%s(%d)Invalid board id %d\r\n", __FUNCTION__, __LINE__, uc_sys_info.board_id);
    }
    #elif defined(ES4627MB) /* #if defined(AOS5700_54X) */
    if(uc_sys_info.board_id <= 1 || uc_sys_info.board_id == 5)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 6)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_2};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 3 || uc_sys_info.board_id == 4)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_0_1};
         memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    #elif defined(ASF4512MP)/* #if defined(ES4627MB) */
    if(uc_sys_info.board_id == 0)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_0};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 1)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
              {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_1};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
               local_port_sfp_module_present_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    #elif defined(ES4552MA_HPOE)/* #elif defined(ASF4512MP) */
    if(uc_sys_info.board_id == 0 || uc_sys_info.board_id == 1)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
        {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
                local_port_sfp_module_present_bit_shift,
                sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 3)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
        {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_2_3};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
                local_port_sfp_module_present_bit_shift,
                sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    #elif defined(ES3510MA_FLF_38)/* #elif defined(ES4552MA_HPOE) */
    if(uc_sys_info.board_id >= 0 && uc_sys_info.board_id <= 3)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
        {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
                local_port_sfp_module_present_bit_shift,
                sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    else if(uc_sys_info.board_id == 4)
    {
        UI8_T local_port_sfp_module_present_bit_shift[] =
        {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY_BID_4};

        memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
                local_port_sfp_module_present_bit_shift,
                sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
              );
    }
    #else /* #elif defined(ES3510MA_FLF_38) */
    UI8_T local_port_sfp_module_present_bit_shift[] =
          {SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY};

    memcpy(shmem_data_p->swdrv_sfp_present_bit_shift,
           local_port_sfp_module_present_bit_shift,
           sizeof(shmem_data_p->swdrv_sfp_present_bit_shift)
          );
    #endif /* end of #if defined(ES4627MB) */
#endif /* end of #if defined(SYS_HWCFG_SFP_MODULE_PRESENT_BIT_SHIFT_ARRAY_BODY) */
}/* End of SWDRV_OM_ResetSfpPresentBitShift */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_ResetSfpPresentStatusAddr
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize sfp present status address from sys_hwcfg
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_OM_ResetSfpPresentStatusAddr(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

#if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD != SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL)
#if defined(SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY)
    #if defined(ES4627MB)
    if(uc_sys_info.board_id <= 1 || uc_sys_info.board_id == 5)
    {
        UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
              {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
               local_sys_hwcfg_port_sfp_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
              );
    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 6)
    {
        UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
              {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY_BID_2};

        memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
               local_sys_hwcfg_port_sfp_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
              );
    }
    else if(uc_sys_info.board_id == 3 || uc_sys_info.board_id == 4)
    {
        UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
              {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
               local_sys_hwcfg_port_sfp_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
              );
    }
    #elif defined(ES3510MA_FLF_38)/* #if defined(ES4627MB) */
    if(uc_sys_info.board_id >= 0 && uc_sys_info.board_id <= 3)
    {
        UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
              {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY};

        memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
               local_sys_hwcfg_port_sfp_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
              );
    }
    else if(uc_sys_info.board_id == 4)
    {
        UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
              {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY_BID_4};

        memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
               local_sys_hwcfg_port_sfp_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
              );
    }
    #else /* #elif defined(ES3510MA_FLF_38) */
    UI32_T local_sys_hwcfg_port_sfp_status_addr[] =
          {SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY};

    memcpy(shmem_data_p->swdrv_sfp_present_status_addr,
           local_sys_hwcfg_port_sfp_status_addr,
           sizeof(shmem_data_p->swdrv_sfp_present_status_addr)
          );
    #endif /* end of #if defined(ES4627MB) */
#endif /* end of #if defined(SYS_HWCFG_SFP_STATUS_ADDR_ARRAY_BODY) */
#endif /* end of #if (SYS_HWCFG_SFP_PRESENT_STATUS_ACCESS_METHOD != SYS_HWCFG_REG_ACCESS_METHOD_I2C_WITH_CHANNEL) */

}/* End of SWDRV_OM_ResetSfpPresentStatusAddr */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_ResetSfpPresentMask
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize sfp present mask from sys_hwcfg
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_OM_ResetSfpPresentMask(void)
{
    UC_MGR_Sys_Info_T         uc_sys_info;

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

#if defined(SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY)
#if defined(AOS5700_54X)
    if(uc_sys_info.board_id == 0)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_0};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 1)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_1};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else
    {
        printf("%s(%d): Invalid board id %d\r\n", __FUNCTION__, __LINE__, (int)(uc_sys_info.board_id));
    }

#elif defined(ES4627MB) /* #if defined(AOS5700_54X) */
    if(uc_sys_info.board_id <= 1 || uc_sys_info.board_id == 5)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 6)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_2};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 3 || uc_sys_info.board_id == 4)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_0_1};
         memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
#elif defined(ASF4512MP)/* #if defined(ES4627MB) */
    if(uc_sys_info.board_id == 0)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_0};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 1)
    {
        UI8_T local_sfp_present_mask[]=
              {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_1};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
               local_sfp_present_mask,
               sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
#elif defined(ES4552MA_HPOE)/* #elif defined(ASF4512MP) */
    if(uc_sys_info.board_id == 0 || uc_sys_info.board_id == 1)
    {
        UI8_T local_sfp_present_mask[]=
        {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
                local_sfp_present_mask,
                sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 3)
    {
        UI8_T local_sfp_present_mask[]=
        {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_2_3};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
                local_sfp_present_mask,
                sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
#elif defined(ES3510MA_FLF_38)/* #elif defined(ES4552MA_HPOE) */
    if(uc_sys_info.board_id >= 0 && uc_sys_info.board_id <= 3)
    {
        UI8_T local_sfp_present_mask[]=
        {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
                local_sfp_present_mask,
                sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }
    else if(uc_sys_info.board_id == 4)
    {
        UI8_T local_sfp_present_mask[]=
        {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY_BID_4};

        memcpy(shmem_data_p->swdrv_sfp_present_mask,
                local_sfp_present_mask,
                sizeof(shmem_data_p->swdrv_sfp_present_mask));
    }

#else /* #elif defined(ES3510MA_FLF_38) */
    UI8_T local_sfp_present_mask[]=
          {SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY};

    memcpy(shmem_data_p->swdrv_sfp_present_mask,
           local_sfp_present_mask,
           sizeof(shmem_data_p->swdrv_sfp_present_mask));
#endif /* end of #if defined(ES4627MB) */
#endif /* end of #if defined(SYS_HWCFG_SFP_MODULE_PRESENT_MASK_ARRAY_BODY) */

}/* End of SWDRV_OM_ResetSfpPresentMask */

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_ResetSfpRxLos
 * -------------------------------------------------------------------------
 * FUNCTION: Initialize sfp rx los status address, bit shift, mask from sys_hwcfg
 *
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 * -------------------------------------------------------------------------
 */
static void SWDRV_OM_ResetSfpRxLos(void)
{
#if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE)
    UC_MGR_Sys_Info_T         uc_sys_info;

    /* Need to call UC_MGR_InitiateProcessResources before entering this
     * function
     */
    if (!UC_MGR_GetSysInfo(&uc_sys_info))
    {
        perror("\r\nGet UC System Information Fail.");
        assert(0);

        /* severe problem, while loop here
         */
        while (TRUE);
    }

    #if defined(ES4627MB)
    if(uc_sys_info.board_id <= 1 || uc_sys_info.board_id == 5)
    {
        UI32_T local_sfp_rx_los_status_addr[] =
               {SYS_HWCFG_SFP_RX_LOSS_STATUS_ADDR_ARRAY_BODY_BID_0_1};
        UI8_T  local_sfp_rx_los_mask[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY_BID_0_1};
        UI8_T  local_sfp_rx_los_bit_shift[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_rx_los_status_addr,
               local_sfp_rx_los_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_status_addr));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
               local_sfp_rx_los_mask,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
               local_sfp_rx_los_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));

    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 6)
    {
        UI32_T local_sfp_rx_los_status_addr[] =
               {SYS_HWCFG_SFP_RX_LOSS_STATUS_ADDR_ARRAY_BODY_BID_2};
        UI8_T  local_sfp_rx_los_mask[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY_BID_2};
        UI8_T  local_sfp_rx_los_bit_shift[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY_BID_2};

        memcpy(shmem_data_p->swdrv_sfp_rx_los_status_addr,
               local_sfp_rx_los_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_status_addr));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
               local_sfp_rx_los_mask,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
               local_sfp_rx_los_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));
    }
    else if(uc_sys_info.board_id == 3 || uc_sys_info.board_id == 4)
    {
        UI32_T local_sfp_rx_los_status_addr[] =
               {SYS_HWCFG_SFP_RX_LOSS_STATUS_ADDR_ARRAY_BODY_BID_0_1};
        UI8_T  local_sfp_rx_los_mask[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY_BID_0_1};
        UI8_T  local_sfp_rx_los_bit_shift[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_rx_los_status_addr,
               local_sfp_rx_los_status_addr,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_status_addr));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
               local_sfp_rx_los_mask,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
               local_sfp_rx_los_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));

    }
    #elif defined(ES4552MA_HPOE)
    if(uc_sys_info.board_id == 0 || uc_sys_info.board_id == 1)
    {
        UI8_T  local_sfp_rx_los_mask[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY_BID_0_1};
        UI8_T  local_sfp_rx_los_bit_shift[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY_BID_0_1};

        memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
               local_sfp_rx_los_mask,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
               local_sfp_rx_los_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));

    }
    else if(uc_sys_info.board_id == 2 || uc_sys_info.board_id == 3)
    {
        UI8_T  local_sfp_rx_los_mask[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY_BID_2_3};
        UI8_T  local_sfp_rx_los_bit_shift[] =
               {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY_BID_2_3};

        memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
               local_sfp_rx_los_mask,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

        memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
               local_sfp_rx_los_bit_shift,
               sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));
    }
    #else /* #if defined(ES4627MB) */
    UI32_T local_sfp_rx_los_status_addr[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT] =
           {SYS_HWCFG_SFP_RX_LOSS_STATUS_ADDR_ARRAY_BODY};
    UI8_T  local_sfp_rx_los_mask[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT] =
           {SYS_HWCFG_SFP_MODULE_RX_LOS_MASK_ARRAY_BODY};
    UI8_T  local_sfp_rx_los_bit_shift[SYS_ADPT_NBR_OF_SFP_PORT_PER_UNIT] =
           {SYS_HWCFG_SFP_MODULE_RX_LOS_BIT_SHIFT_ARRAY_BODY};

    memcpy(shmem_data_p->swdrv_sfp_rx_los_status_addr,
           local_sfp_rx_los_status_addr,
           sizeof(shmem_data_p->swdrv_sfp_rx_los_status_addr));

    memcpy(shmem_data_p->swdrv_sfp_rx_los_mask,
           local_sfp_rx_los_mask,
           sizeof(shmem_data_p->swdrv_sfp_rx_los_mask));

    memcpy(shmem_data_p->swdrv_sfp_rx_los_bit_shift,
           local_sfp_rx_los_bit_shift,
           sizeof(shmem_data_p->swdrv_sfp_rx_los_bit_shift));
    #endif /* end of #if defined(ES4627MB) */
#endif /* end of #if (SYS_HWCFG_GBIC_HAS_RX_LOS==TRUE) */
}/* End of SWDRV_OM_ResetSfpRxLos */

#if (SYS_CPNT_HASH_SELECTION == TRUE)
/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_GetHashSelDevBlockIndex
 * -------------------------------------------------------------------------
 * FUNCTION: get hash-selection list index and block index for the service
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 * OUTPUT  : list_index - the index of hash-selection list
 *           hw_block_index - the index of hash-selection block
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_GetHashSelDevBlockIndex(
    SWDRV_HashSelService_T service,
    UI8_T *list_index, 
    UI8_T *hw_block_index)
{
    UI32_T i;

    SWDRV_OM_EnterCriticalSection();
    for (i=0; i<SYS_ADPT_HW_HASH_SELECTION_BLOCK_SIZE; i++)
    {
        if (shmem_data_p->hash_block[i].ref_service_bmp & service )
        {
            *list_index = shmem_data_p->hash_block[i].list_index;
            *hw_block_index = i+1;
            SWDRV_OM_LeaveCriticalSection();
            return TRUE;
        }
    }
    
    SWDRV_OM_LeaveCriticalSection();
    return FALSE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_BindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: add service reference of the hash-selection block
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : hw_block_index - the index of hash-selection block
 *           share          - TRUE: share with other services
 *                            FALSE: no services use the same block
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_BindHashSelForService(
    SWDRV_HashSelService_T service, 
    UI8_T list_index,
    UI8_T *hw_block_index,
    BOOL_T *share)
{
    UI32_T i, candidate=0;

    *share = FALSE;

    SWDRV_OM_EnterCriticalSection();
    for (i=0; i<SYS_ADPT_HW_HASH_SELECTION_BLOCK_SIZE; i++)
    {    
        if (shmem_data_p->hash_block[i].ref_service_bmp == 0)
        {
            if (candidate == 0)
            {
                candidate = i+1;
            }
        }
        /*shmem_data_p->hash_block[i].ref_service_bmp != 0*/
        else if (shmem_data_p->hash_block[i].list_index == list_index)
        {
            *share == TRUE;
            candidate = i+1;
            break;
        }
    }

    if (candidate == 0)
    {
        SWDRV_OM_LeaveCriticalSection();
        return FALSE;
    }

    *hw_block_index = candidate;
    shmem_data_p->hash_block[candidate-1].list_index = list_index;
    shmem_data_p->hash_block[candidate-1].ref_service_bmp |= service;

    SWDRV_OM_LeaveCriticalSection();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - SWDRV_OM_UnBindHashSelForService
 * -------------------------------------------------------------------------
 * FUNCTION: remove service reference of the hash-selection block
 * INPUT   : service     - SWCTRL_OM_HASH_SEL_SERVICE_ECMP,
 *                         SWCTRL_OM_HASH_SEL_SERVICE_TRUNK
 *           list_index - the index of hash-selection list
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 * -------------------------------------------------------------------------*/
BOOL_T SWDRV_OM_UnBindHashSelForService(
    SWDRV_HashSelService_T service, 
    UI8_T list_index)
{
    UI32_T i;

    SWDRV_OM_EnterCriticalSection();
    for (i=0; i<SYS_ADPT_HW_HASH_SELECTION_BLOCK_SIZE; i++)
    {
        if (shmem_data_p->hash_block[i].list_index == list_index &&
            shmem_data_p->hash_block[i].ref_service_bmp & service)
        {
            shmem_data_p->hash_block[i].ref_service_bmp &= ~service;
            SWDRV_OM_LeaveCriticalSection();
            return TRUE;
        }
    }
    SWDRV_OM_LeaveCriticalSection();

    return FALSE;
}
#endif /*#if (SYS_CPNT_HASH_SELECTION == TRUE)*/

