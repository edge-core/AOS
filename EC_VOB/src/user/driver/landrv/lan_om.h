#ifndef _LAN_OM_H
#define _LAN_OM_H

#include "sys_type.h"
#include "sysrsc_mgr.h"

typedef enum
{
    LAN_RX_PACKET_TOTAL = 0,    /* all packet arrive in lan (call LAN_RecvPacket) */
    LAN_RX_UNKNOWN_CLASS_DROP,  /* packet drop because of unknown class */
    LAN_RX_BLOCKING_PORT_DROP,  /* drop because of blocking port */
    LAN_RX_INTRUSION_DROP,      /* not announce because it is an intrusion packet */
    LAN_RX_UNTAGGED_DROP,       /* discard untagged frame */
    LAN_RX_TAGGED_DROP,         /* discard tagged frame */
    LAN_ZERO_SA_ERROR,          /* in LAN_ClassifyPkt, if SA == 0 */
    LAN_MULTICAST_SA_ERROR,     /* in LAN_ClassifyPkt, if SA is multicast mac address */
    LAN_BACKDOOR_COUNTER_MAX
} BackdoorCounter_T;

typedef enum
{
    LAN_DEBUG_RX = 0,
    LAN_DEBUG_RX_DA,
    LAN_DEBUG_RX_SA,
    LAN_DEBUG_TX,
    LAN_DEBUG_TX_DA,
    LAN_DEBUG_TX_DATA,
    LAN_DEBUG_TX_DA_DATA,
    LAN_DEBUG_RX_DROP,
    LAN_BACKDOOR_TOGGLE_MAX
} BackdoorToggle_T;

typedef enum
{
    LAN_DEBUG_RX_SA_ADDR = 0,
    LAN_DEBUG_RX_DA_ADDR,
    LAN_DEBUG_TX_DA_ADDR,
    LAN_BACKDOOR_MACADDR_MAX
} BackdoorMacAddr_T;

typedef enum
{
    DISPLAY_TRANSMIT_PACKET = 0,
    DISPLAY_RECEIVE_PACKET,
    ISC_DEBUG_SHOW_MESSAGE,
    LAN_STACKING_BACKDOOR_TOGGLE_MAX
} StackingBackdoorToggle_T;


/*---------------------------------------------------------------------------------
 * FUNCTION : void LAN_OM_InitiateSystemResources(void)
 *---------------------------------------------------------------------------------
 * PURPOSE  : Initiate system resource for LAN OM
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------------*/
void LAN_OM_InitiateSystemResources(void);
 
/*---------------------------------------------------------------------------------
 * FUNCTION : LAN_OM_AttachSystemResources
 *---------------------------------------------------------------------------------
 * PURPOSE  : Attach system resource for LAN OM in the context of the calling process.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : None
 *---------------------------------------------------------------------------------*/
void LAN_OM_AttachSystemResources(void);

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
BOOL_T LAN_OM_InitateProcessResource(void);

void LAN_OM_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p);

SYS_TYPE_Stacking_Mode_T LAN_OM_GetOperatingMode(void);

void LAN_OM_SetTransitionMode(void);

void LAN_OM_EnterTransitionMode(void);

void LAN_OM_EnterMasterMode(void);

void LAN_OM_EnterSlaveMode(void);

UI32_T LAN_OM_GetMyUnitId(void);

void LAN_OM_SetMyUnitId(UI32_T unit_id);

UI8_T LAN_OM_GetMasterUnitId(void);

void LAN_OM_SetMasterUnitId(UI8_T unit_id);

UI32_T LAN_OM_GetMyStackingPort(void);

void LAN_OM_SetMyStackingPort(UI32_T stacking_port);

void LAN_OM_GetMyMac(UI8_T my_mac[SYS_ADPT_MAC_ADDR_LEN]);

void LAN_OM_SetMyMac(UI8_T my_mac[SYS_ADPT_MAC_ADDR_LEN]);

#if (SYS_CPNT_EFM_OAM == TRUE)
BOOL_T LAN_OM_GetOamLoopback(UI32_T unit, UI32_T port);

BOOL_T LAN_OM_SetOamLoopback(UI32_T unit, UI32_T port, BOOL_T enable);

void LAN_OM_ClearAllOamLoopback(void);
#endif

#if (SYS_CPNT_INTERNAL_LOOPBACK_TEST == TRUE)
BOOL_T LAN_OM_GetInternalLoopback(UI32_T unit, UI32_T port);

BOOL_T LAN_OM_SetInternalLoopback(UI32_T unit, UI32_T port, BOOL_T enable);

void LAN_OM_ClearAllInternalLoopback(void);
#endif

BOOL_T LAN_OM_GetPortLearning(UI32_T unit, UI32_T port);
BOOL_T LAN_OM_SetPortLearning(UI32_T unit, UI32_T port, BOOL_T enable);

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
BOOL_T LAN_OM_GetVlanLearningStatus(UI32_T vid);

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
BOOL_T LAN_OM_SetVlanLearningStatus(UI32_T vid, BOOL_T learning);

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
void LAN_OM_ClearAllVlanLearning(void);

void LAN_OM_ClearAllPortLearning(void);

BOOL_T LAN_OM_GetPortSecurity(UI32_T unit, UI32_T port);

BOOL_T LAN_OM_SetPortSecurity(UI32_T unit, UI32_T port, BOOL_T enable);

void LAN_OM_ClearAllPortSecurity(void);

BOOL_T LAN_OM_GetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port);
BOOL_T LAN_OM_SetPortDiscardUntaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable);
void LAN_OM_ClearAllPortDiscardUntaggedFrame(void);

BOOL_T LAN_OM_GetPortDiscardTaggedFrame(UI32_T unit, UI32_T port);
BOOL_T LAN_OM_SetPortDiscardTaggedFrame(UI32_T unit, UI32_T port, BOOL_T enable);
void LAN_OM_ClearAllPortDiscardTaggedFrame(void);

UI32_T LAN_OM_GetBackdoorCounter(UI32_T index);

BOOL_T LAN_OM_IncreaseBackdoorCounter(UI32_T index);

BOOL_T LAN_OM_GetBackdoorToggle(UI32_T index);

BOOL_T LAN_OM_SetBackdoorToggle(UI32_T index, BOOL_T value);

BOOL_T LAN_OM_GetBackdoorStackingToggle(UI32_T dbg_type);

BOOL_T LAN_OM_SetBackdoorStackingToggle(UI32_T dbg_type, BOOL_T value);

BOOL_T LAN_OM_GetBackdoorMac(UI32_T index, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN]);

BOOL_T LAN_OM_SetBackdoorMac(UI32_T index, UI8_T mac[SYS_ADPT_MAC_ADDR_LEN]);

BOOL_T LAN_OM_GetBackdoorPortFilterMask(UI8_T port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

BOOL_T LAN_OM_SetBackdoorPortFilterMask(UI8_T port_filter_mask[SYS_ADPT_MAX_NBR_OF_UNIT_PER_STACK][SYS_ADPT_NBR_OF_BYTE_FOR_1BIT_UPORT_LIST]);

BOOL_T LAN_OM_IsBackdoorPortFilter(UI32_T unit, UI32_T port);
BOOL_T LAN_OM_IsBackdoorPortListFilter(UI8_T *uport_list);

void LAN_OM_ClearAllBackdoorCounter(void);

void LAN_OM_ClearAllBackdoorToggle(void);

#endif

