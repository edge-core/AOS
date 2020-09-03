#include "sys_type.h"
#include "sys_bld.h"
#include "sys_dflt.h"
#include "sys_cpnt.h"
#include "sysfun.h"
#include "lan_om.h"
#include "sysrsc_mgr.h"
#include "string.h"
#include "amtr_mgr.h"

#define LAN_OM_ENTER_CRITICAL_SECTION() SYSFUN_TakeSem(lan_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define LAN_OM_LEAVE_CRITICAL_SECTION() SYSFUN_GiveSem(lan_om_sem_id)
#define LAN_ATOM_EXPRESSION(exp) { LAN_OM_ENTER_CRITICAL_SECTION();\
                                   exp;\
                                   LAN_OM_LEAVE_CRITICAL_SECTION();\
                                 }

typedef struct 
{
    SYSFUN_DECLARE_CSC_ON_SHMEM
    BOOL_T  is_allocated;
    UI32_T  my_unit_id;
    UI32_T  my_stacking_port;
    UI8_T   my_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   master_unit_id;
#if (SYS_CPNT_EFM_OAM == TRUE)    
    BOOL_T  is_oam_loopback[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
#endif
#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
    BOOL_T  is_internal_loopback[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT];
#endif
    BOOL_T  port_disable_learning[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];
    BOOL_T  port_security[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];
    BOOL_T  port_discard_untagged[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];
    BOOL_T  port_discard_tagged[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT_ON_BOARD];
    UI32_T  lan_backdoor_counter[LAN_BACKDOOR_COUNTER_MAX];
    BOOL_T  lan_backdoor_toggle[LAN_BACKDOOR_TOGGLE_MAX];
    UI8_T   lan_backdoor_macaddr[LAN_BACKDOOR_MACADDR_MAX][SYS_ADPT_MAC_ADDR_LEN];
    UI8_T   lan_backdoor_port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST];
    BOOL_T  lan_stacking_backdoor_toggle[LAN_STACKING_BACKDOOR_TOGGLE_MAX];
    UI8_T   vlan_learn_dis[AMTR_MGR_NBR_OF_BYTE_FOR_1BIT_VLAN_LIST]; /* vid learning disable status bitmap */
} LAN_Shmem_Data_T;


static LAN_Shmem_Data_T   *shmem_data_p;
static UI32_T             lan_om_sem_id;

/*---------------------------------------------------------------------------------
 * FUNCTION : void LAN_OM_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for LAN OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void LAN_OM_InitiateSystemResources(void)
{
    shmem_data_p = (LAN_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_LANDRV_SHMEM_SEGID);
    SYSFUN_INITIATE_CSC_ON_SHMEM(shmem_data_p);
    shmem_data_p->is_allocated = FALSE; 
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_LAN_OM, &lan_om_sem_id);
    return;
}
 
/*---------------------------------------------------------------------------------
 * FUNCTION : LAN_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for LAN OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void LAN_OM_AttachSystemResources(void)
{
    shmem_data_p = (LAN_Shmem_Data_T*)SYSRSC_MGR_GetShMem(SYSRSC_MGR_LANDRV_SHMEM_SEGID);
    SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_LAN_OM, &lan_om_sem_id);    
    return;
}

void LAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_LANDRV_SHMEM_SEGID;
    *seglen_p = sizeof(LAN_Shmem_Data_T);
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : LAN_OM_InitateProcessResource
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initiate resource used in this process.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE   -- Success
 *    FALSE  -- Fail
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
BOOL_T LAN_OM_InitateProcessResource(void)
{
    if(SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_LAN_OM, &lan_om_sem_id)!=SYSFUN_OK)
    {
        printf("%s:get om sem id fail.\n", __FUNCTION__);
        return FALSE;
    }
    return TRUE;
}

SYS_TYPE_Stacking_Mode_T LAN_OM_GetOperatingMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE_ON_SHMEM(shmem_data_p);
}

void LAN_OM_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

void LAN_OM_EnterTransitionMode(void)
{
    SYSFUN_ENTER_TRANSITION_MODE_ON_SHMEM(shmem_data_p);
}

void LAN_OM_EnterMasterMode(void)
{
    SYSFUN_ENTER_MASTER_MODE_ON_SHMEM(shmem_data_p);
}

void LAN_OM_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE_ON_SHMEM(shmem_data_p);
}

UI32_T LAN_OM_GetMyUnitId(void)
{
    return shmem_data_p->my_unit_id;
}

void LAN_OM_SetMyUnitId(UI32_T unit_id)
{
    LAN_ATOM_EXPRESSION(shmem_data_p->my_unit_id = unit_id);
}

UI8_T LAN_OM_GetMasterUnitId(void)
{
    return shmem_data_p->master_unit_id;
}

void LAN_OM_SetMasterUnitId(UI8_T unit_id)
{
    LAN_ATOM_EXPRESSION(shmem_data_p->master_unit_id = unit_id);
}

UI32_T LAN_OM_GetMyStackingPort(void)
{
    return shmem_data_p->my_stacking_port;
}

void LAN_OM_SetMyStackingPort(UI32_T stacking_port)
{
    LAN_ATOM_EXPRESSION(shmem_data_p->my_stacking_port= stacking_port);
}

void LAN_OM_GetMyMac(UI8_T my_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    LAN_ATOM_EXPRESSION(memcpy(my_mac, shmem_data_p->my_mac, SYS_ADPT_MAC_ADDR_LEN));
    return;
}

void LAN_OM_SetMyMac(UI8_T my_mac[SYS_ADPT_MAC_ADDR_LEN])
{
    LAN_ATOM_EXPRESSION(memcpy(shmem_data_p->my_mac, my_mac, SYS_ADPT_MAC_ADDR_LEN));
    return;
}

#if (SYS_CPNT_EFM_OAM == TRUE)
BOOL_T LAN_OM_GetOamLoopback(UI32_T unit, UI32_T port)
{
    return shmem_data_p->is_oam_loopback[unit-1][port-1];
}

BOOL_T LAN_OM_SetOamLoopback(UI32_T unit, UI32_T port, BOOL_T enable)
{
    LAN_OM_ENTER_CRITICAL_SECTION();
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        LAN_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    /* Oam and Internal loopback can't enable simutanously
     */
    if ((enable == TRUE)&&
        (shmem_data_p->is_internal_loopback[unit-1][port-1] == TRUE))
    {
        LAN_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    shmem_data_p->is_oam_loopback[unit-1][port-1] = enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

void LAN_OM_ClearAllOamLoopback(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->is_oam_loopback, 0, sizeof(shmem_data_p->is_oam_loopback)));
}
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
BOOL_T LAN_OM_GetInternalLoopback(UI32_T unit, UI32_T port)
{
    return shmem_data_p->is_internal_loopback[unit-1][port-1];
}

BOOL_T LAN_OM_SetInternalLoopback(UI32_T unit, UI32_T port, BOOL_T enable)
{
    LAN_OM_ENTER_CRITICAL_SECTION();
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        LAN_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
#if (SYS_CPNT_EFM_OAM == TRUE) /* Thomas added for 3628bt build error */   
    /* Oam and Internal loopback can't enable simutanously
     */
    if ((enable == TRUE)&&
        (shmem_data_p->is_oam_loopback[unit-1][port-1] == TRUE))
    {
        LAN_OM_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
#endif /*  SYS_CPNT_EFM_OAM == TRUE   */
    shmem_data_p->is_internal_loopback[unit-1][port-1] = enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

void LAN_OM_ClearAllInternalLoopback(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->is_internal_loopback, 0, sizeof(shmem_data_p->is_internal_loopback)));
}
#endif

BOOL_T LAN_OM_GetPortLearning(UI32_T unit, UI32_T port)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }        
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    return !(shmem_data_p->port_disable_learning[unit-1][port-1]);
}

BOOL_T LAN_OM_SetPortLearning(UI32_T unit, UI32_T port, BOOL_T enable)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    LAN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_disable_learning[unit-1][port-1] = !enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : LAN_OM_GetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get vlan learning status of specified vlan
 * INPUT    : vid
 * OUTPUT   : None
 * RETURN   : TRUE - vlan learning is enabled
 *            FALE - vlan learning is disabled
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T LAN_OM_GetVlanLearningStatus(UI32_T vid)
{
    if (vid == 0 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return FALSE;
    }

    return !(AMTR_MGR_VLAN_LEARNING_TST_BIT(shmem_data_p->vlan_learn_dis, vid));
}

/*------------------------------------------------------------------------------
 * Function : LAN_OM_SetVlanLearningStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Enable/disable vlan learning of specified vlan
 * INPUT    : vid
 *            learning
 * OUTPUT   : None
 * RETURN   : TRUE/FALE
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
BOOL_T LAN_OM_SetVlanLearningStatus(UI32_T vid, BOOL_T learning)
{
    if (vid == 0 || vid > SYS_ADPT_MAX_VLAN_ID)
    {
        return FALSE;
    }

    LAN_OM_ENTER_CRITICAL_SECTION();
    if (TRUE == !learning)
    {
        AMTR_MGR_VLAN_LEARNING_SET_BIT(shmem_data_p->vlan_learn_dis, vid);
    }
    else
    {
        AMTR_MGR_VLAN_LEARNING_CLR_BIT(shmem_data_p->vlan_learn_dis, vid);
    }
    LAN_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Function : LAN_OM_ClearAllVlanLearning
 *------------------------------------------------------------------------------
 * PURPOSE  : Initialize vlan learning of all vlan
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : None
 *-----------------------------------------------------------------------------
 */
void LAN_OM_ClearAllVlanLearning(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->vlan_learn_dis, 0, sizeof(shmem_data_p->vlan_learn_dis)));
}

void LAN_OM_ClearAllPortLearning(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->port_disable_learning, 0, sizeof(shmem_data_p->port_disable_learning)));
}

BOOL_T LAN_OM_GetPortSecurity(UI32_T unit, UI32_T port)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }        
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    return shmem_data_p->port_security[unit-1][port-1];
}

BOOL_T LAN_OM_SetPortSecurity(UI32_T unit, UI32_T port, BOOL_T enable)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    LAN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_security[unit-1][port-1] = enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

void LAN_OM_ClearAllPortSecurity(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->port_security, 0, sizeof(shmem_data_p->port_security)));
}

BOOL_T LAN_OM_GetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }        
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    return shmem_data_p->port_discard_untagged[unit-1][port-1];
}

BOOL_T LAN_OM_SetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    LAN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_discard_untagged[unit-1][port-1] = enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

void LAN_OM_ClearAllPortDiscardUntaggedFrame(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->port_discard_untagged, 0, sizeof(shmem_data_p->port_discard_untagged)));
}

BOOL_T LAN_OM_GetPortDiscardTaggedFrame(UI32_T unit, UI32_T port)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }        
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    return shmem_data_p->port_discard_tagged[unit-1][port-1];
}

BOOL_T LAN_OM_SetPortDiscardTaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable)
{
    if (unit>SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
    {
        return FALSE;
    }
    if (port>SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
    {
        return FALSE;
    }

    LAN_OM_ENTER_CRITICAL_SECTION();
    shmem_data_p->port_discard_tagged[unit-1][port-1] = enable;
    LAN_OM_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

void LAN_OM_ClearAllPortDiscardTaggedFrame(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->port_discard_tagged, 0, sizeof(shmem_data_p->port_discard_tagged)));
}

UI32_T LAN_OM_GetBackdoorCounter(UI32_T index)
{
    if (index > LAN_BACKDOOR_COUNTER_MAX)
        return 0;
    return shmem_data_p->lan_backdoor_counter[index];
}

BOOL_T LAN_OM_IncreaseBackdoorCounter(UI32_T index)
{
    if (index > LAN_BACKDOOR_COUNTER_MAX)
        return FALSE;
    LAN_ATOM_EXPRESSION(shmem_data_p->lan_backdoor_counter[index]++);
    return TRUE;
}

BOOL_T LAN_OM_GetBackdoorToggle(UI32_T index)
{
    if (index > LAN_BACKDOOR_TOGGLE_MAX)
        return FALSE;
    return shmem_data_p->lan_backdoor_toggle[index];
}

BOOL_T LAN_OM_SetBackdoorToggle(UI32_T index, BOOL_T value)
{
    if (index > LAN_BACKDOOR_TOGGLE_MAX)
        return FALSE;
    LAN_ATOM_EXPRESSION(shmem_data_p->lan_backdoor_toggle[index] = value);
    return TRUE;
}

BOOL_T LAN_OM_GetBackdoorStackingToggle(UI32_T dbg_idx)
{
    if (dbg_idx >= LAN_STACKING_BACKDOOR_TOGGLE_MAX )
        return FALSE;
    return shmem_data_p->lan_stacking_backdoor_toggle[dbg_idx];
}

BOOL_T LAN_OM_SetBackdoorStackingToggle(UI32_T dbg_idx, BOOL_T value)
{
    if (dbg_idx >= LAN_STACKING_BACKDOOR_TOGGLE_MAX)
        return FALSE;
    LAN_ATOM_EXPRESSION(shmem_data_p->lan_stacking_backdoor_toggle[dbg_idx] = value);
    return TRUE;
}

BOOL_T LAN_OM_GetBackdoorMac(UI32_T index, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN])
{
    if (index > LAN_BACKDOOR_MACADDR_MAX)
        return FALSE;
    memcpy(mac, shmem_data_p->lan_backdoor_macaddr[index], SYS_ADPT_MAC_ADDR_LEN);
    return TRUE;
}

BOOL_T LAN_OM_SetBackdoorMac(UI32_T index, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN])
{
    if (index > LAN_BACKDOOR_MACADDR_MAX)
        return FALSE;
    LAN_ATOM_EXPRESSION(memcpy(shmem_data_p->lan_backdoor_macaddr[index], mac, SYS_ADPT_MAC_ADDR_LEN));
    return TRUE;
}

BOOL_T LAN_OM_GetBackdoorPortFilterMask(UI8_T port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if (port_filter_mask == NULL)
        return FALSE;
    memcpy(port_filter_mask, shmem_data_p->lan_backdoor_port_filter_mask, sizeof(shmem_data_p->lan_backdoor_port_filter_mask));
    return TRUE;
}

BOOL_T LAN_OM_SetBackdoorPortFilterMask(UI8_T port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST])
{
    if (port_filter_mask == NULL)
        return FALSE;
    LAN_ATOM_EXPRESSION(memcpy(shmem_data_p->lan_backdoor_port_filter_mask, port_filter_mask, sizeof(shmem_data_p->lan_backdoor_port_filter_mask)));
    return TRUE;
}

BOOL_T LAN_OM_IsBackdoorPortFilter(UI32_T unit, UI32_T port)
{
    if (unit > SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK)
        return FALSE;
    if (port > SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT)
        return FALSE;

    return !!(shmem_data_p->lan_backdoor_port_filter_mask[unit-1][(port-1)/8] & BIT_VALUE(7 - ((port-1) % 8)));
}

BOOL_T LAN_OM_IsBackdoorPortListFilter(UI8_T *uport_list)
{
    UI8_T (*p)[SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST] = (void *)uport_list;
    int i, j;

    if (uport_list == NULL)
        return FALSE;

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK; i++)
    {
        for (j = 0; j < SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST; j++)
        {
            if (p[i][j] & ~ shmem_data_p->lan_backdoor_port_filter_mask[i][j])
            {
                return FALSE;
            }
        }
    }

    return FALSE;
}

void LAN_OM_ClearAllBackdoorCounter(void)
{
    LAN_ATOM_EXPRESSION(memset(shmem_data_p->lan_backdoor_counter, 0, sizeof(shmem_data_p->lan_backdoor_counter)));
}

void LAN_OM_ClearAllBackdoorToggle(void)
{
    LAN_OM_ENTER_CRITICAL_SECTION();
    memset(shmem_data_p->lan_backdoor_toggle, 0, sizeof(shmem_data_p->lan_backdoor_toggle));
    memset(shmem_data_p->lan_stacking_backdoor_toggle, 0, sizeof(shmem_data_p->lan_stacking_backdoor_toggle));
    LAN_OM_LEAVE_CRITICAL_SECTION();
}


