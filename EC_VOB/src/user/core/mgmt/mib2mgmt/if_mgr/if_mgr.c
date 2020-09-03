/* -------------------------------------------------------------------------
 * FILE NAME - IF_MGR.C
 * -------------------------------------------------------------------------
 * Purpose: This package provides the services to manage/support the RFC2863 MIB
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *        2. The Interfaces group defined by RFC1213 MIB has been obsoleted by
 *           the new RFC2863.
 *        3. The following deprecated objects defined in ifTable will not be supported:
 *
 *          ifInNUcastPkts          Counter32,  -- deprecated
 *          ifOutNUcastPkts         Counter32,  -- deprecated
 *          ifOutQLen               Gauge32,    -- deprecated
 *          ifSpecific              OBJECT IDENTIFIER -- deprecated
 *
 *        4. The Interface Test Table will not be supported by this device.
 *           This group of objects is optional.  However, a media-specific
 *           MIB may make implementation of this group mandatory.
 *
 *        5. The Interface Receive Address Table will not be supported in this
 *           package.
 *
 *
 *          Written by:     Nike Chen, first create
 *          Date:           07/15/01
 *
 * Modification History:
 *   By              Date     Ver.   Modification Description
 *   --------------- -------- -----  ---------------------------------------
 *    Amytu         9-25-2002         EnterTransitionMode for stacking
 *  * Allen Cheng  12-19-2002  V2.0    Revised for the trap mechanism changed
-------------------------------------------------------------------------
 * Copyright(C)                              ACCTON Technology Corp., 1998
 * -------------------------------------------------------------------------
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "sysfun.h"
#include "l_cvrt.h"
#include "leaf_2863.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "if_mgr.h"
#include "vlan_pom.h"
#include "vlan_pmgr.h"
#include "vlan_lib.h"
#include "swctrl.h"
#include "swctrl_pom.h"
#include "swctrl_pmgr.h"
#include "swdrv_type.h"
#include "nmtr_pmgr.h"
#include "trk_pmgr.h"
#include "stktplg_pom.h"
#include "sys_dflt.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "cmgr.h"

/* LOCAL DEFINES
 */
#if 0  /* moved to "sys_adpt.h" */
#define IF_MGR_LPORT_IF_DESC_STR        "EtherNet Port on unit %d, port:%d"
#define IF_MGR_TRUNK_MEMBER_IF_DESC_STR "Trunk Member port on Trunk ID %d"
#define IF_MGR_TRUNK_IF_DESC_STR        "Trunk Port ID %04d"
#define IF_MGR_RS232_IF_DESC_STR        "Console port"
#define IF_MGR_VLAN_IF_DESC_STR         "VLAN interface ID %04d"
#define IF_MGR_LOCAL_IF_NAME_STR        "SYS_ADPT_PORT%d_IFNAME"
#define IF_MGR_PORTNAME                 "Port%d"
#endif

#define MAX_BUF 64

#define IF_MGR_USE_CSC(a)
#define IF_MGR_RELEASE_CSC()

/* TYPE DECLARATION
 */
typedef struct
{
    UI32_T  ifindex;
    UI8_T   port_name[32];
} IF_MGR_PortNameEntry_T;


/* LOCAL SUBPROGRAM DECLARATION
 */

static void IF_MGR_IfindexToCategory(UI32_T ifindex, UI32_T *ifindex_category);
static BOOL_T IF_MGR_GetNextLowerIndexForVlan(UI8_T *portlist, UI32_T *lower_index);
static BOOL_T IF_MGR_GetNextLowerIndexForTrunk(UI32_T trunk_id, UI32_T *lower_index);
static BOOL_T IF_MGR_GetNextIfIndex(UI32_T *index);

#if 0  /* no one use it */
static void IF_MGR_IfOperStatusChanged_CallBack (UI32_T vid_ifindex, UI32_T vlan_status);
static void IF_MGR_VlanDestroy_CallBack (UI32_T vid_ifindex, UI32_T vlan_status);
#endif

static void IF_MGR_InitPortTable(void);
static  BOOL_T  IF_MGR_SetIfLinkUpDownTrapEnable_(UI32_T if_x_index, UI32_T trap_status);
static  BOOL_T  IF_MGR_GetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status);
static  BOOL_T  IF_MGR_GetIfName(UI32_T ifindex, UI8_T *str);
static  BOOL_T  IF_MGR_GetIfDescr(UI32_T ifindex, UI8_T *str);
static  BOOL_T  IF_MGR_IfSprintf(UI8_T *buf, UI8_T *str, UI32_T index);
static  UI8_T   IF_MGR_CalculateCharCount(UI8_T *str, UI8_T character);
static  UI32_T  IF_MGR_ConvertDirectiveIndexToArgument(UI8_T dir_char, UI32_T ifindex);
static  UI8_T   IF_MGR_Remove1stDirectiveIndex(UI8_T *str, UI8_T character);
static  BOOL_T  IF_MGR_Vsprintf(UI8_T *buf, UI8_T *str, UI32_T *arg_list);

/* STATIC VARIABLE DECLARATIONS
 */

static IF_MGR_IfEntry_T             if_entry;
static IF_MGR_IfXEntry_T            if_x_entry;
static IF_MGR_IfStackEntry_T        if_stack_entry;

#if 0 /* XXX steven.jiang for warnings */
static SYS_TYPE_CallBack_T          *ifOperStatusChangedHandler;
static SYS_TYPE_CallBack_T          *vlanDestroyHandler;
#endif /* 0 */

SYSFUN_DECLARE_CSC




/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: IF_MGR_InitiateSystemResources
 * PURPOSE: To register callback function in VLAN_MGR.
 * INPUT:  none
 * OUTPUT: none
 * RETURN: none
 */
void IF_MGR_InitiateSystemResources(void)
{
    memset(&if_entry, 0, sizeof(IF_MGR_IfEntry_T));
    memset(&if_x_entry, 0, sizeof(IF_MGR_IfXEntry_T));
    memset(&if_stack_entry, 0, sizeof(IF_MGR_IfStackEntry_T));

/* Allen Cheng: Deleted
    operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/

    return;

} /* end of IF_MGR_InitiateSystemResources() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_Create_InterCSC_Relation
 *--------------------------------------------------------------------------
 * PURPOSE  : This function initializes all function pointer registration operations.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void IF_MGR_Create_InterCSC_Relation(void)
{
#if 0 /*no use for linux platform,,because no one is registered*/
    VLAN_MGR_RegisterIfOperStatusChanged_CallBack(&IF_MGR_IfOperStatusChanged_CallBack);
    VLAN_MGR_RegisterVlanDestroy_CallBack(&IF_MGR_VlanDestroy_CallBack);
#endif
} /* end of IF_MGR_Create_InterCSC_Relation */

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_SetIfLinkUpDownTrapEnableGlobal
 * ------------------------------------------------------------------------
 * PURPOSE: Set trap status to all interfaces.
 * INPUT:   trap_status -- trap status
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 * ------------------------------------------------------------------------
 */
BOOL_T
IF_MGR_SetIfLinkUpDownTrapEnableGlobal(
    UI32_T trap_status)
{
    UI32_T if_x_index = 0;

    while (TRUE == IF_MGR_GetNextIfIndex(&if_x_index))
    {
        IF_MGR_SetIfLinkUpDownTrapEnable_(if_x_index, trap_status);
    }

    return  TRUE;
} /* End of IF_MGR_SetIfLinkUpDownTrapEnableGlobal */

/* ------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_SetIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the trap status of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index  -- key to specify which index to configured.
 *          trap_status -- trap status
 *                        VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                        VAL_ifLinkUpDownTrapEnable_disabled (2)
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ------------------------------------------------------------------------
 */
BOOL_T IF_MGR_SetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T trap_status)
{
    if (TRUE != IF_MGR_SetIfLinkUpDownTrapEnable_(if_x_index, trap_status))
    {
        return  FALSE;
    }

    return  TRUE;
} /* End of IF_MGR_SetIfLinkUpDownTrapEnable */

/* FUNCTION NAME: IF_MGR_EnterMasterMode
 * PURPOSE: Enable the IF_MGR activities as master mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterMasterMode (void)
{
/* Allen Cheng: Deleted
    operation_mode = SYS_TYPE_STACKING_MASTER_MODE;
*/
    IF_MGR_InitPortTable();
    IF_MGR_SetIfLinkUpDownTrapEnable(1, SYS_DFLT_IF_LINK_UP_DOWN_TRAP_ENABLE);
    SYSFUN_ENTER_MASTER_MODE();
    return;
} /* end of IF_MGR_EnterMasterMode() */


/* FUNCTION NAME: IF_MGR_EnterSlaveMode
 * PURPOSE: Enable the IF_MGR activities as slave mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterSlaveMode (void)
{
    SYSFUN_ENTER_SLAVE_MODE();
/* Allen Cheng: Deleted
    operation_mode = SYS_TYPE_STACKING_SLAVE_MODE;
*/
    return;

} /* end of IF_MGR_Enter_Slave_Mode() */



/* FUNCTION NAME: IF_MGR_EnterTransitionMode
 * PURPOSE: Enable the IF_MGR activities as transition mode
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none.
 * NOTE:
 */
void IF_MGR_EnterTransitionMode (void)
{
    SYSFUN_ENTER_TRANSITION_MODE();
/* Allen Cheng: Deleted
    operation_mode = SYS_TYPE_STACKING_TRANSITION_MODE;
*/

    memset(&if_entry, 0, sizeof(IF_MGR_IfEntry_T));
    memset(&if_x_entry, 0, sizeof(IF_MGR_IfXEntry_T));
    memset(&if_stack_entry, 0, sizeof(IF_MGR_IfStackEntry_T));
//    memset(port_name_table, 0, sizeof(IF_MGR_PortNameEntry_T) *SYS_ADPT_TOTAL_NBR_OF_LPORT);

    return;
} /* end of IF_MGR_Enter_Transition_Mode() */


/* FUNCTION NAME - IF_MGR_SetTransitionMode
 * PURPOSE  : This function sets the component to temporary transition mode
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void  IF_MGR_SetTransitionMode(void)
{
    SYSFUN_SET_TRANSITION_MODE();

} /* end of IF_MGR_SetTransitionMode() */


/* FUNCTION NAME - IF_MGR_ProvisionComplete
 * PURPOSE  : mib2mgmt_init will call this function when provision completed
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
void IF_MGR_ProvisionComplete(void)
{
	return;
}

/* FUNCTION NAME - IF_MGR_GetOperationMode
 * PURPOSE  : This functions returns the current operation mode of this component
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *
 */
SYS_TYPE_Stacking_Mode_T  IF_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
} /* end of IF_MGR_GetOperationMode() */

#if 0 /*no one use it*/

/* FUNCTION NAME: IF_MGR_Register_IfOperStatusChanged_CallBack
 * PURPOSE: For NETCFG to register Vlan up/down callback function
 * INPUT:  callback function
 * OUTPUT: none
 * RETURN: none
 * NOTES:
 */
void IF_MGR_Register_IfOperStatusChanged_CallBack (void (*fun)(UI32_T if_index, UI32_T status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(ifOperStatusChangedHandler);
	return;

} /* end of IF_MGR_Register_IfOperStatusChanged_CallBack() */


/* FUNCTION NAME: IF_MGR_Register_VlanDestroy_CallBack
 * PURPOSE: For NETCFG to register Vlan destroyed callback function
 * INPUT:  callback function
 * OUTPUT: none
 * RETURN: none
 * NOTES:
 */
void IF_MGR_Register_VlanDestroy_CallBack (void (*fun)(UI32_T if_index, UI32_T vlan_status))
{
    SYS_TYPE_REGISTER_CALLBACKFUN(vlanDestroyHandler);
	return;
}/* end of IF_MGR_Register_VlanDestroy_CallBack() */

#endif

/* FUNCTION NAME: IF_MGR_GetIfNumber
 * PURPOSE: This funtion returns the number of network interfaces
 *          (regardless of their current state) present on this system.
 * INPUT:  None.
 * OUTPUT: if_number        - the total number of network interfaces presented on the system
 * RETURN: TRUE/FALSE
 * NOTES: For those interfaces which are not installed shall not be count into this number.
 */
BOOL_T IF_MGR_GetIfNumber(UI32_T *if_number)
{
    UI32_T      port, vlan, trunk, console, loopback;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    /* Get current sys_avil_port and current config vlan + trunk + rs232
     */
    port = SWCTRL_POM_GetSystemPortNumber();
    vlan = VLAN_POM_GetDot1qNumVlans();
    trunk = TRK_PMGR_GetTrunkCounts();

    loopback = 1;   /* at SYS_ADPT_LOOPBACK_IF_INDEX_BASE */

    if (!STKTPLG_POM_GetNumberOfUnit(&console))
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    *if_number = port + vlan + trunk + console + loopback;
    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_GetIfNumber() */



/* FUNCTION NAME: IF_MGR_GetIfNumber
 * PURPOSE: This funtion returns the value of sysUpTime at the time of the
 *          last creation or deletion of an entry in the ifTable. If the number of
 *          entries has been unchanged since the last re-initialization
 *          of the local network management subsystem, then this object
 *          contains a zero value.
 * INPUT:  None.
 * OUTPUT: if_table_last_change_time
 * RETURN: TRUE/FALSE
 * NOTES: None.
 */
BOOL_T IF_MGR_GetIfTableLastChange (UI32_T *if_table_last_change_time)
{
    UI32_T      swctrl_time, vlan_time;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    swctrl_time = SWCTRL_POM_GetLastChangeTime();
    vlan_time = VLAN_POM_IfTableLastUpdateTime();

    if (swctrl_time > vlan_time)
        *if_table_last_change_time = swctrl_time;
    else
        *if_table_last_change_time = vlan_time;
    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_GetIfTableLastChange() */


/* FUNCTION NAME: IF_MGR_GetIfEntry
 * PURPOSE: This funtion returns true if the specified interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_MGR_GetIfEntry (IF_MGR_IfEntry_T  *if_entry)
{
    UI32_T                  ifindex_category, vid;
    UI32_T                  unit, port, trunk_id;
    Port_Info_T             port_info;
    IF_MGR_IfEntry_T        local_if_entry;
    SWDRV_IfTableStats_T    if_info;
	VLAN_OM_Dot1qVlanCurrentEntry_T		vlan_info;
	SWCTRL_Lport_Type_T     port_type;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    memset(&local_if_entry, 0, sizeof(IF_MGR_IfEntry_T));
    local_if_entry.if_index = if_entry->if_index;
    IF_MGR_GetIfDescr(local_if_entry.if_index, local_if_entry.if_descr);
    local_if_entry.if_descr_length  = strlen((char *)if_entry->if_descr);

    IF_MGR_IfindexToCategory(local_if_entry.if_index, &ifindex_category);

    /* As long as interface is valid, if_mgr must return true and struct with valid value
     * return false only if if_index is invalid.
     */
    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
            if ((port_type = SWCTRL_POM_LogicalPortToUserPort(local_if_entry.if_index, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
            {
                EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_GetIfEntry_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            /* The return range of this swctrl API will return user port
             * as well as trunk port, excluding trunk port member.
             */
            if (!SWCTRL_POM_GetPortInfo(local_if_entry.if_index,&port_info))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
                ! NMTR_PMGR_GetSystemwideIfTableStats(local_if_entry.if_index, &if_info)
#else
                ! NMTR_PMGR_GetIfTableStats(local_if_entry.if_index, &if_info)
#endif
                )
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            if (!SWCTRL_POM_GetPortMac(local_if_entry.if_index, local_if_entry.if_phys_address))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            local_if_entry.if_type = VAL_ifType_ethernetCsmacd;
            local_if_entry.if_mtu = port_info.mtu; /*changed by jinhua.wei*/
            local_if_entry.if_admin_status = port_info.admin_state;
            local_if_entry.if_oper_status = port_info.link_oper_status;
            local_if_entry.if_last_change = port_info.link_oper_status_last_change;
            local_if_entry.if_in_octets = if_info.ifInOctets;
            local_if_entry.if_in_ucast_pkts = if_info.ifInUcastPkts;
            local_if_entry.if_in_nucast_pkts = if_info.ifInNUcastPkts;
            local_if_entry.if_in_discards = if_info.ifInDiscards;
            local_if_entry.if_in_errors = if_info.ifInErrors;
            local_if_entry.if_in_unknown_protos = if_info.ifInUnknownProtos;
            local_if_entry.if_out_octets = if_info.ifOutOctets;
            local_if_entry.if_out_ucast_pkts = if_info.ifOutUcastPkts;
            local_if_entry.if_out_nucast_pkts = if_info.ifOutNUcastPkts;
            local_if_entry.if_out_discards = if_info.ifOutDiscards;
            local_if_entry.if_out_errors = if_info.ifOutErrors;
            local_if_entry.if_out_qlen = if_info.ifOutQLen;

            /* 2004/8/31 tc_wang
             * Added corresponding value for 10G,
             * 10G is greater then Maximum(0xFFFFFFFF=4294967295),
             * thus set to Maximum according the MIB description
             */
            local_if_entry.if_speed = 0xFFFFFFFF;

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex10) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex10))
                local_if_entry.if_speed = 10000000;

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex100) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex100))
                local_if_entry.if_speed = 100000000;

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex1000) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex1000))
                local_if_entry.if_speed = 1000000000;

            break;

        case IF_MGR_RS232_IFINDEX:
            local_if_entry.if_type = VAL_ifType_rs232;

            local_if_entry.if_admin_status = VAL_ifAdminStatus_up;
            local_if_entry.if_oper_status = VAL_ifOperStatus_up;

            break;

        case IF_MGR_LOOPBACK_IFINDEX:
            local_if_entry.if_type = VAL_ifType_softwareLoopback;
            local_if_entry.if_admin_status = VAL_ifAdminStatus_up;
            local_if_entry.if_oper_status = VAL_ifOperStatus_up;
            break;

        case IF_MGR_VLAN_IFINDEX:

            VLAN_IFINDEX_CONVERTTO_VID(local_if_entry.if_index,vid);
            if (!VLAN_POM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            VLAN_VID_CONVERTTO_IFINDEX(vlan_info.dot1q_vlan_index, local_if_entry.if_index);

            /*
             *Fix bug:ES4827G-FLF-ZZ-00435
             *Problem:Node ifType show wrong values for vlan.
             *RootCause:Because here we set other vaule for this node.
             *Solution:change this value get path.
             *changed file:rfc_2863.c,if_mgr.c
             *approved by :Hard.Sun
             *Fixed by:Jinhua.Wei
             */
            local_if_entry.if_type = vlan_info.if_entry.ifType;

            local_if_entry.if_admin_status = vlan_info.if_entry.admin_status;
            local_if_entry.if_oper_status = vlan_info.if_entry.vlan_operation_status;
            local_if_entry.if_last_change = vlan_info.if_entry.if_last_change;

            if (!VLAN_PMGR_GetVlanMac(local_if_entry.if_index, local_if_entry.if_phys_address))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            local_if_entry.if_mtu = SYS_ADPT_IF_MTU;

            break;

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_GetIfEntry_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
            IF_MGR_RELEASE_CSC();
            return FALSE;
            break;
    } /* end of switch */
    memcpy(if_entry, &local_if_entry, sizeof(IF_MGR_IfEntry_T));
    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_GetIfEntry() */


/* FUNCTION NAME: IF_MGR_GetNextIfEntry
 * PURPOSE: This funtion returns true if the next available interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_entry->if_index      - key to specify a unique interface entry
 * OUTPUT:  if_entry                - interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available interface entry is available, the if_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 */
BOOL_T IF_MGR_GetNextIfEntry (IF_MGR_IfEntry_T  *if_entry)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (!IF_MGR_GetNextIfIndex(&if_entry->if_index))
        return FALSE;

    return IF_MGR_GetIfEntry(if_entry);

} /* end of IF_MGR_GetNextIfEntry() */



/* FUNCTION NAME: IF_MGR_SetIfAdminStatus
 * PURPOSE: This funtion returns true if desired/new admin status is successfully set to
 *          the specified interface. Otherwise, false is returned.
 * INPUT:  if_index         - key to specify a unique interface
 *         if_admin_status  - VAL_ifAdminStatus_up / VAL_ifAdminStatus_down /
 *                            VAL_ifAdminStatus_testing
 * OUTPUT: None.
 * RETURN: TRUE/FALSE
 * NOTES: 1. The testing(3) state indicates that no operational packets can be passed.
 *        2. When a managed system initializes, all interfaces start with ifAdminStatus
 *           in the down(2) state.
 *        3. As a result of either explicit management action or per configuration
 *           information retained by the managed system, ifAdminStatus is then changed
 *           to either the up(1) or  testing(3) states (or remains in the down(2) state).
 */
BOOL_T IF_MGR_SetIfAdminStatus (UI32_T if_index, UI32_T if_admin_status)
{
    UI32_T                  ifindex_category, vid;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    IF_MGR_IfindexToCategory(if_index, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
            if (!CMGR_SetPortAdminStatus(if_index, if_admin_status))
	        {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;

        case IF_MGR_VLAN_IFINDEX:
            VLAN_IFINDEX_CONVERTTO_VID(if_index,vid);

            /* VLAN_MGR does not support command to set admin_status.
               This API alwz returns FALSE from VLAN_MGR
             */
            if (!VLAN_PMGR_SetVlanAdminStatus(vid, if_admin_status))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_SetIfAdminStatus_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
            IF_MGR_RELEASE_CSC();
            return FALSE;
            break;
    } /* end of switch */

    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_SetIfAdminStatus() */


/* FUNCTION NAME: IF_MGR_GetIfXEntry
 * PURPOSE: This funtion returns true if the specified extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_entry                - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 */
BOOL_T IF_MGR_GetIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry)
{
    UI32_T                      ifindex_category, vid;
    IF_MGR_IfXEntry_T           local_if_entry;
    Port_Info_T                 port_info;
    SWDRV_IfXTableStats_T       if_x_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T         vlan_info;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    memset(&local_if_entry, 0, sizeof(IF_MGR_IfXEntry_T));

    local_if_entry.if_index = if_x_entry->if_index;
    IF_MGR_GetIfName(local_if_entry.if_index, local_if_entry.if_name);

    IF_MGR_IfindexToCategory(local_if_entry.if_index, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
            /* The return range of this swctrl API will return user port
             * as well as trunk port, excluding trunk port member.
             */
            if (!SWCTRL_POM_GetPortInfo(local_if_entry.if_index,&port_info ))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            if (
#if (SYS_CPNT_SYSTEMWIDE_COUNTER == TRUE)
                ! NMTR_PMGR_GetSystemwideIfXTableStats(local_if_entry.if_index, &if_x_info)
#else
                ! NMTR_PMGR_GetIfXTableStats(local_if_entry.if_index, &if_x_info)
#endif
                )
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            local_if_entry.if_name_length = strlen((char *)local_if_entry.if_name);

            local_if_entry.if_in_multicast_pkts = if_x_info.ifInMulticastPkts;
            local_if_entry.if_in_broadcast_pkts = if_x_info.ifInBroadcastPkts;
            local_if_entry.if_out_multicast_pkts = if_x_info.ifOutMulticastPkts;
            local_if_entry.if_out_broadcast_pkts = if_x_info.ifOutBroadcastPkts;
            local_if_entry.if_hc_in_octets = if_x_info.ifHCInOctets;
            local_if_entry.if_hc_in_ucast_pkts = if_x_info.ifHCInUcastPkts;
            local_if_entry.if_hc_in_multicast_pkts =if_x_info.ifHCInMulticastPkts;
            local_if_entry.if_hc_in_broadcast_pkts =if_x_info.ifHCInBroadcastPkts;
            local_if_entry.if_hc_out_octets = if_x_info.ifHCOutOctets;
            local_if_entry.if_hc_out_ucast_pkts = if_x_info.ifHCOutUcastPkts;
            local_if_entry.if_hc_out_multicast_pkts = if_x_info.ifHCOutMulticastPkts;
            local_if_entry.if_hc_out_broadcast_pkts = if_x_info.ifHCOutBroadcastPkts;
            local_if_entry.if_link_up_down_trap_enable = port_info.link_change_trap;
            local_if_entry.if_counter_discontinuity_time = if_x_info.ifCounterDiscontinuityTime;
            local_if_entry.if_promiscuous_mode = VAL_ifPromiscuousMode_true;

            /*EPR: ES4827G-FLF-ZZ-00211
             *Problem:MIB node ifconnectorpresent can't get correct value
             *Solution:Follow ECN430 use the link status to this node
             *Approved by:Hardsun
             *Modify file:if_mgr.c
             */
            local_if_entry.if_connector_present = port_info.link_status;

            /*EPR:ES4827G-FLF-ZZ-00232
             *Problem: CLI:size of vlan name different in console and mib
             *Solution: add CLI command "alias" for interface set,the
             *          alias is different from name and port descrition,so
             *          need add new command.
             *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
             *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
             *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
             *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
             *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
             *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
             *Approved by:Hardsun
             *Fixed by:Dan Xie
             */

            memcpy(local_if_entry.if_alias, port_info.port_alias, sizeof(port_info.port_alias));

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex10) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex10))
                local_if_entry.if_high_speed = 10;

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex100) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex100))
                local_if_entry.if_high_speed = 100;

            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex1000) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex1000))
                local_if_entry.if_high_speed = 1000;

            /* 2004/8/31 tc_wang
             * Added corresponding value for 10G,
             */
            if ((port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_halfDuplex10g) ||
               (port_info.speed_duplex_oper == VAL_portSpeedDpxStatus_fullDuplex10g))
                local_if_entry.if_high_speed = 10000;

            break;

        case IF_MGR_RS232_IFINDEX:
            local_if_entry.if_promiscuous_mode = VAL_ifPromiscuousMode_false;
            local_if_entry.if_link_up_down_trap_enable = VAL_ifLinkUpDownTrapEnable_disabled;
            local_if_entry.if_connector_present = VAL_ifConnectorPresent_true;

            break;

        case IF_MGR_LOOPBACK_IFINDEX:
            local_if_entry.if_promiscuous_mode = VAL_ifPromiscuousMode_false;
            local_if_entry.if_link_up_down_trap_enable = VAL_ifLinkUpDownTrapEnable_disabled;
            local_if_entry.if_connector_present = VAL_ifConnectorPresent_true;

            break;

        case IF_MGR_VLAN_IFINDEX:
            VLAN_IFINDEX_CONVERTTO_VID(local_if_entry.if_index,vid);
            if (!VLAN_POM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            VLAN_VID_CONVERTTO_IFINDEX(vid, local_if_entry.if_index);

           /*EPR:ES4827G-FLF-ZZ-00232
            *Problem: CLI:size of vlan name different in console and mib
            *Solution: add CLI command "alias" for interface set,the
            *          alias is different from name and port descrition,so
            *          need add new command.
            *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
            *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
            *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
            *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
            *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
            *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
            *Approved by:Hardsun
            *Fixed by:Dan Xie
            */
            strncpy((char *)local_if_entry.if_alias, vlan_info.dot1q_vlan_alias, MAXSIZE_ifAlias);
            local_if_entry.if_promiscuous_mode = VAL_ifPromiscuousMode_true;
            local_if_entry.if_link_up_down_trap_enable = vlan_info.if_entry.link_up_down_trap_enabled;
            /*EPR: ES4827G-FLF-ZZ-00211
             *Problem:MIB node ifconnectorpresent can't get correct value
             *Solution:Follow ECN430 use the link status to this node
             *Approved by:Hardsun
             *Modify file:if_mgr.c
             */
            local_if_entry.if_connector_present = vlan_info.if_entry.vlan_operation_status;
            break;

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_GetIfXEntry_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
            IF_MGR_RELEASE_CSC();
            return FALSE;
            break;

    } /* end of switch */
    memcpy(if_x_entry, &local_if_entry, sizeof(IF_MGR_IfXEntry_T));
    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_GetIfXEntry() */


/*-------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_GetNextIfXEntry
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the next available extension interface entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   if_x_entry->if_index    - key to specify a unique extension interface entry
 * OUTPUT:  if_x_entry              - extension interface entry info of specified if_index
 * RETURN:  TRUE/FALSE
 * NOTES:   If next available extension interface entry is available, the if_x_entry->if_index
 *          will be updated and the entry info will be retrieved from the table.
 * ------------------------------------------------------------------------
 */
BOOL_T IF_MGR_GetNextIfXEntry (IF_MGR_IfXEntry_T  *if_x_entry)
{
    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    if (!IF_MGR_GetNextIfIndex(&if_x_entry->if_index))
        return FALSE;

    return IF_MGR_GetIfXEntry(if_x_entry);

} /* end of IF_MGR_GetNextIfXEntry() */




/*-------------------------------------------------------------------------
 * FUNCTION NAME: IF_MGR_SetIfLinkUpDownTrapEnable_
 * ------------------------------------------------------------------------
 * PURPOSE: This funtion returns true if the trap status of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   trap_status - trap status
 *                      VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                      VAL_ifLinkUpDownTrapEnable_disabled (2)
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 * ------------------------------------------------------------------------
 */
static  BOOL_T IF_MGR_SetIfLinkUpDownTrapEnable_(UI32_T if_x_index, UI32_T trap_status)
{
    UI32_T  ifindex_category;
#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
    UI32_T  vid;
#endif /* if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    if ((VAL_ifLinkUpDownTrapEnable_enabled != trap_status) &&
        (VAL_ifLinkUpDownTrapEnable_disabled != trap_status))
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    }

    IF_MGR_IfindexToCategory(if_x_index, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
            if (!SWCTRL_PMGR_SetPortLinkChangeTrapEnable(if_x_index, trap_status))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;

#if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE)
        case IF_MGR_VLAN_IFINDEX:
            VLAN_IFINDEX_CONVERTTO_VID(if_x_index,vid);
            if (!VLAN_PMGR_SetLinkUpDownTrapEnabled(vid, trap_status))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;
#endif /* if (SYS_CPNT_IF_LINK_TRAP_PORT_BASE == FALSE) */

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_SetIfLinkUpDownTrapEnable_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
            IF_MGR_RELEASE_CSC();
            return FALSE;
            break;
    } /* end of switch */
    IF_MGR_RELEASE_CSC();
    return TRUE;

} /* end of IF_MGR_SetIfLinkUpDownTrapEnable_() */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global LinkUpDownTrapEnable status.
 * INPUT    :   if_x_index              -- the specified IfIndex
 * OUTPUT   :   UI32_T *trap_status     -- pointer of the status value
 * RETURN   :   TRUE/FALSE
 * NOTES    :   None
 * REF      :   For global setting: set to all the interfaces
 * ------------------------------------------------------------------------
 */
static  BOOL_T  IF_MGR_GetIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status)
{
    IF_MGR_IfXEntry_T   if_x_entry;
    BOOL_T              result;

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    result  = TRUE;
    if (if_x_index)
    {
        if_x_entry.if_index    = if_x_index;
        if (IF_MGR_GetIfXEntry (&if_x_entry) )
        {
            *trap_status    = if_x_entry.if_link_up_down_trap_enable;
        }
        else
        {
            result  = FALSE;
        }
    }
    else
    {
        result  = FALSE;
    }
    IF_MGR_RELEASE_CSC();

    return  result;
} /* End of IF_MGR_GetIfLinkUpDownTrapEnable */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetRunningIfLinkUpDownTrapEnable
 * ------------------------------------------------------------------------
 * PURPOSE  :   Get the global LinkUpDownTrapEnable status.
 * INPUT    :   if_x_index              -- the specified IfIndex
 * OUTPUT   :   UI32_T *trap_status     -- pointer of the status value
 *                      VAL_ifLinkUpDownTrapEnable_enabled  (1)
 *                      VAL_ifLinkUpDownTrapEnable_disabled (2)
 *
 * RETURN   :   SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *              SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE,
 *              SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    :   1. This function shall only be invoked by CLI to save the
 *                 "running configuration" to local or remote files.
 *              2. Since only non-default configuration will be saved, this
 *                 function shall return non-default value.
 * REF      :   None
 * ------------------------------------------------------------------------
 */
UI32_T  IF_MGR_GetRunningIfLinkUpDownTrapEnable(UI32_T if_x_index, UI32_T *trap_status)
{
    UI32_T  result;

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    if (IF_MGR_GetIfLinkUpDownTrapEnable(if_x_index, trap_status) )
    {
        if (SYS_DFLT_IF_LINK_UP_DOWN_TRAP_ENABLE == (*trap_status))
        {
            result  = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
        }
        else
        {
            result  = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
        }
    }
    else
    {
        result  = SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    IF_MGR_RELEASE_CSC();
    return  result;
} /* End of IF_MGR_GetRunningIfLinkUpDownTrapEnable */


/* FUNCTION NAME: IF_MGR_SetIfAlias
 * PURPOSE: This funtion returns true if the if_alias of the specific interface
 *          can be set successfully.  Otherwise, return FALSE.
 * INPUT:   if_x_index - key to specify which index to configured.
 *			if_alias - the read/write name of the specific interface
 * OUTPUT:  none
 * RETURN:  TRUE/FALSE
 * NOTES:   none
 */
BOOL_T IF_MGR_SetIfAlias(UI32_T if_x_index, UI8_T *if_alias)
{
    UI32_T                  ifindex_category, vid;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    IF_MGR_IfindexToCategory(if_x_index, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
          /*EPR:ES4827G-FLF-ZZ-00232
           *Problem: CLI:size of vlan name different in console and mib
           *Solution: add CLI command "alias" for interface set,the
           *          alias is different from name and port descrition,so
           *          need add new command.
           *modify file: cli_cmd.c,cli_cmd.h,cli_arg.c,cli_arg.h,cli_msg.c,
           *             cli_msg.h,cli_api_vlan.c,cli_api_vlan.h,cli_api_ehternet.c
           *             cli_api_ethernet.h,cli_api_port_channel.c,cli_api_port_channel.h,
           *             cli_running.c,rfc_2863.c,swctrl.h,trk_mgr.h,trk_pmgr.h,swctrl.c
           *             swctrl_pmgr.c,trk_mgr.c,trk_pmgr.c,vlan_mgr.h,vlan_pmgr.h,
           *             vlan_type.h,vlan_mgr.c,vlan_pmgr.c,if_mgr.c
           *Approved by:Hardsun
           *Fixed by:Dan Xie
           */
            if (!SWCTRL_PMGR_SetPortAlias(if_x_index, if_alias))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;

        case IF_MGR_VLAN_IFINDEX:
            VLAN_IFINDEX_CONVERTTO_VID(if_x_index,vid);
            if (!VLAN_PMGR_SetDot1qVlanAlias(vid, (char *)if_alias))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            break;

        default:
            EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_SetIfAlias_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
            IF_MGR_RELEASE_CSC();
            return FALSE;
            break;
    } /* end of switch */
    IF_MGR_RELEASE_CSC();
    return TRUE;
} /* end of IF_MGR_SetIfAlias() */



/* FUNCTION NAME: IF_MGR_GetIfStackEntry
 * PURPOSE: This funtion returns true if specified interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - the specified interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_MGR_GetIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry)
{
    UI32_T      ifindex_category, vid, trunk_ifindex;
    UI32_T      unit, port, trunk_id;
    TRK_MGR_TrunkEntry_T                trunk_info;
    VLAN_OM_Dot1qVlanCurrentEntry_T     vlan_info;

    /* BODY */


    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    IF_MGR_IfindexToCategory(if_stack_entry->if_stack_higher_layer, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_TRUNK_IFINDEX:

            /* Input a trunk_member port and obtain a trunk_ifindex which the specific port join.
             */
            if (!SWCTRL_POM_GetTrunkIfIndexByUport(if_stack_entry->if_stack_lower_layer, &trunk_ifindex))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            /* Return error if the specific stack lower index is not a member of stack higher index
             */
            if (trunk_ifindex != if_stack_entry->if_stack_higher_layer )
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            /* Obtain trunk_id once lower index is confirmed to be a trunk member of higher index
             */
            if (SWCTRL_POM_LogicalPortToUserPort(if_stack_entry->if_stack_higher_layer,
                                             &unit, &port, &trunk_id) != SWCTRL_LPORT_TRUNK_PORT)
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            trunk_info.trunk_index = trunk_id;
            if (!TRK_PMGR_GetTrunkEntry(&trunk_info))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            if_stack_entry->if_stack_status = trunk_info.trunk_status;
            IF_MGR_RELEASE_CSC();
            return TRUE;

            break;

        case IF_MGR_VLAN_IFINDEX:

            VLAN_IFINDEX_CONVERTTO_VID(if_stack_entry->if_stack_higher_layer,vid);

            /* If the specific higher_layer_ifindex does not exist in vlan_om, then
               this entry does not exist in the database.
             */
            if (!VLAN_POM_IsVlanExisted(vid))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */

            /* If lower_layer_ifindex is not one of the members of the higher_layer_ifindex,
               then the specific entry does not exist in the database.
             */
            if (!VLAN_POM_IsPortVlanMember(if_stack_entry->if_stack_higher_layer,
                                           if_stack_entry->if_stack_lower_layer))
            {
                IF_MGR_RELEASE_CSC();
                return FALSE;
            } /* End of if */
            else
            {
                if (!VLAN_POM_GetDot1qVlanCurrentEntry(0,vid, &vlan_info))
                {
                    IF_MGR_RELEASE_CSC();
                    return FALSE;
                }
                if_stack_entry->if_stack_status = vlan_info.dot1q_vlan_static_row_status;
                IF_MGR_RELEASE_CSC();
                return TRUE;
            }
            break;

        default:
            break;
    } /* end of switch */
    EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_GetIfStackEntry_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, "This ifindex");
    IF_MGR_RELEASE_CSC();
    return FALSE;

} /* end of IF_MGR_GetIfStackEntry() */



/* FUNCTION NAME: IF_MGR_GetNextIfStackEntry
 * PURPOSE: This funtion returns true if next available interface stack entry info
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:  if_stack_entry->if_stack_higher_layer_if_index   - primary key to specify
 *                                                          - a unique interface stack entry
 *         if_stack_entry->if_stack_lower_layer_if_index    - secondary key
 * OUTPUT: if_stack_entry                   - next available interface stack entry info
 * RETURN: TRUE/FALSE
 * NOTES: 1. The write-access to this object will not be supported by this device.
 *        2. Changing the value of this object from 'active' to 'notInService' or 'destroy'
 *           will likely have consequences up and down the interface stack.
 *           Thus, write access to this object is likely to be inappropriate for some types of
 *           interfaces, and many implementations will choose not to support write-access for
 *           any type of interface.
 */
BOOL_T IF_MGR_GetNextIfStackEntry(IF_MGR_IfStackEntry_T *if_stack_entry)
{
    UI32_T                  ifindex_category, vid;
    UI32_T                  trunk_id, unit, port;
    BOOL_T                  not_found;

    /* BODY */

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
        return FALSE;

    not_found   = TRUE;
    do
    {
        if (if_stack_entry->if_stack_higher_layer < SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER)
        {
            if_stack_entry->if_stack_higher_layer   = SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER - 1;
            if_stack_entry->if_stack_lower_layer    = 0;
        }
        else
        if (    (if_stack_entry->if_stack_higher_layer >= (SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_NBR_OF_TRUNK_PER_SYSTEM) )
             && (if_stack_entry->if_stack_higher_layer  < SYS_ADPT_LOOPBACK_IF_INDEX_BASE)
           )
        {
            if_stack_entry->if_stack_higher_layer   = SYS_ADPT_LOOPBACK_IF_INDEX_BASE;
            if_stack_entry->if_stack_lower_layer    = 0;
        }
        else if ((if_stack_entry->if_stack_higher_layer >= SYS_ADPT_LOOPBACK_IF_INDEX_BASE) &&
            (if_stack_entry->if_stack_higher_layer < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER))
        {
            if_stack_entry->if_stack_higher_layer   = SYS_ADPT_VLAN_1_IF_INDEX_NUMBER - 1;
            if_stack_entry->if_stack_lower_layer    = 0;
        }
        else
        {
            IF_MGR_IfindexToCategory(if_stack_entry->if_stack_higher_layer, &ifindex_category);
            switch (ifindex_category)
            {
                case IF_MGR_TRUNK_IFINDEX:
                    if (    SWCTRL_POM_LogicalPortToUserPort(if_stack_entry->if_stack_higher_layer, &unit, &port, &trunk_id)
                        ==  SWCTRL_LPORT_TRUNK_PORT
                       )
                    {
                        if (IF_MGR_GetNextLowerIndexForTrunk(trunk_id,  &if_stack_entry->if_stack_lower_layer))
                        {
                            TRK_MGR_TrunkEntry_T    trunk_info;
                            trunk_info.trunk_index = trunk_id;
                            if (TRK_PMGR_GetTrunkEntry(&trunk_info))
                            {
                                if_stack_entry->if_stack_status = trunk_info.trunk_status;
                                not_found   = FALSE;
                            } /* End of if */
                        }
                        else
                        {
                            if_stack_entry->if_stack_lower_layer    = 0;
                        }
                     }
                     break;

                case IF_MGR_VLAN_IFINDEX:
                    /* 1. If higher_layer_ifindex does not exist, return the next available vlan entry.
                       2. If lower_layer_ifindex is one of the member of higher_layer_ifindex, return the next
                          available member of higher_layer_ifindex.
                       3. If lower_layer_ifindex is the LAST member of higher_layer_ifindex, return the next
                          available vlan and its FIRST member.
                       4. If higher_layer_ifindex is the last vlan entry in the vlan_om and lower_layer_ifindex
                          is the last member of it, end of database is reached and return FALSE.
                     */
                    if (VLAN_IFINDEX_CONVERTTO_VID(if_stack_entry->if_stack_higher_layer,vid) )
                    {
                        VLAN_OM_Dot1qVlanCurrentEntry_T vlan_info;
                        if (VLAN_POM_GetDot1qVlanCurrentEntry(0, vid, &vlan_info) )
                        {
                            if (IF_MGR_GetNextLowerIndexForVlan((UI8_T *)&vlan_info.dot1q_vlan_current_egress_ports,
                                                                &if_stack_entry->if_stack_lower_layer
                                                               )
                               )
                            {
                                if_stack_entry->if_stack_status = vlan_info.dot1q_vlan_static_row_status;
                                not_found   = FALSE;
                            }
                            else
                            {
                                if_stack_entry->if_stack_lower_layer    = 0;
                            }
                        }
                    }
                    break;

                default:
                    break;
            } /* End of switch */
        } /* End of if */
    } while ( not_found && IF_MGR_GetNextIfIndex(&if_stack_entry->if_stack_higher_layer) );
    /* End of do while */

    return (!not_found);
} /* end of IF_MGR_GetNextIfStackEntry() */



/* FUNCTION NAME: IF_MGR_GetIfStackLastChange
 * PURPOSE: This funtion returns true if the last update time of specified interface stack
 *          can be successfully retrieved. Otherwise, false is returned.
 * INPUT:   none
 * OUTPUT:  if_stack_last_change_time - The value of sysUpTime at the time of the last change of
 *                                      the (whole) interface stack.
 * RETURN: TRUE/FALSE
 * NOTES:  A change of the interface stack is defined to be any creation, deletion, or change in
 *         value of any instance of ifStackStatus.  If the interface stack has been unchanged
 *         since the last re-initialization of the local network management subsystem, then
 *         this object contains a zero value.
 */
BOOL_T IF_MGR_GetIfStackLastChange (UI32_T *if_stack_last_change_time)
{
    UI32_T      vlan_updates, trunk_updates;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */
    /* Get Update time from Trunk_mgr and VLAN_MGR
     */

    vlan_updates = VLAN_POM_IfStackLastUpdateTime();
    trunk_updates = TRK_PMGR_GetLastChangeTime();

    if (vlan_updates > trunk_updates )
        *if_stack_last_change_time = vlan_updates;
    else
        *if_stack_last_change_time = trunk_updates;
    IF_MGR_RELEASE_CSC();
    return TRUE;
} /* end of IF_MGR_GetIfStackLastChange() */


/* FUNCTION NAME: IF_MGR_IfnameToIfindex
 * PURPOSE: This function returns true if the given ifname has a corresponding ifindex existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifname - a read-only name for each interface defined during intialization
 * OUTPUT:  ifindex - corresponding interface index for the specific name.
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_MGR_IfnameToIfindex (UI8_T *ifname, UI32_T *ifindex)
{
    UI32_T      compare_index = 0;
    BOOL_T      found = FALSE;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    for (compare_index = 0; ifname[compare_index] != '\0'; compare_index++)
    {
        ifname[compare_index] = toupper(ifname[compare_index]);
    }

    for (compare_index = 0; compare_index < SYS_ADPT_TOTAL_NBR_OF_LPORT; compare_index++)
    {
        UI8_T   name[32];
        if (IF_MGR_GetIfName(compare_index, name))
        {
            if(strcmp((char *)name, (char *)ifname) == 0)
            {
                found = TRUE;
                *ifindex = compare_index;
                break;
            } /* end of if */
        }
    } /* end of for */
    if (found == FALSE)
    {
        char  strPortName[32];
        sprintf (strPortName, "Port name (%s)", (char *)ifname);
        EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_IfnameToIfindex_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, strPortName);
    }
    IF_MGR_RELEASE_CSC();
	return found;

} /* end of IF_MGR_IfnameToIfindex() */


/* FUNCTION NAME: IF_MGR_IfindexToIfname
 * PURPOSE: This function returns true if the given ifindex has a corresponding ifname existed
 *          in the system.  Otherwise, returns false.
 * INPUT:   ifindex - interface index
 * OUTPUT:  ifname - corresponding name for the specific interface index
 * RETURN:  TRUE/FALSE
 * NOTES:   None
 */
BOOL_T IF_MGR_IfindexToIfname (UI32_T ifindex, UI8_T *ifname)
{
    BOOL_T      found = FALSE;

    /* BODY */

    IF_MGR_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        IF_MGR_RELEASE_CSC();
        return FALSE;
    } /* End of if */

    if (IF_MGR_GetIfName(ifindex, ifname))
       {
          found = TRUE;
    }
    if (found == FALSE)
    {
        char  strIfIndex[32];
        sprintf (strIfIndex, "Interface index (%lu) ", (unsigned long)ifindex);
        EH_MGR_Handle_Exception1(SYS_MODULE_IFMGR, IF_MGR_IfindexToIfname_Fun_No, EH_TYPE_MSG_NOT_EXIST, SYSLOG_LEVEL_INFO, strIfIndex);
    }
    IF_MGR_RELEASE_CSC();
    return found;

} /* end of IF_MGR_IfindexToIfname() */



/* LOCAL SUBPROGRAM Implementation
 */
/* FUNCTION NAME: IF_MGR_IfindexToCategory
 * PURPOSE: This function will return the type of interface base on the input
 *          ifindex key.
 * INPUT:  ifindex         - key to specify a unique interface
 * OUTPUT: *ifindex_type_p - type of interface base on the ifindex number
 * RETURN: none
 * NOTES:  none
 */
static void IF_MGR_IfindexToCategory(UI32_T ifindex, UI32_T *ifindex_category_p)
{
    if (ifindex <= 0)
    {
        *ifindex_category_p = IF_MGR_ERROR_IFINDEX;
        return;
    } /* end of if */

    if ((ifindex >= SYS_ADPT_ETHER_1_IF_INDEX_NUMBER) && (ifindex < SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER))
    {
        *ifindex_category_p = IF_MGR_NORMAL_IFINDEX;
        return;

    } /* end of if */

    if ((ifindex >= SYS_ADPT_TRUNK_1_IF_INDEX_NUMBER) && (ifindex < SYS_ADPT_RS232_1_IF_INDEX_NUMBER))
    {
        *ifindex_category_p = IF_MGR_TRUNK_IFINDEX;
        return;
    } /* end of if */

    if ((ifindex >= SYS_ADPT_RS232_1_IF_INDEX_NUMBER) && (ifindex < SYS_ADPT_LOOPBACK_IF_INDEX_BASE))
    {
        *ifindex_category_p = IF_MGR_RS232_IFINDEX;
        return;
    } /* end of if */

    if ((ifindex >= SYS_ADPT_LOOPBACK_IF_INDEX_BASE) && (ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER))
    {
        *ifindex_category_p = IF_MGR_LOOPBACK_IFINDEX;
        return;
    } /* end of if */

    if ( (ifindex >= SYS_ADPT_VLAN_1_IF_INDEX_NUMBER)
        && (ifindex < SYS_ADPT_VLAN_1_IF_INDEX_NUMBER + SYS_ADPT_MAX_VLAN_ID) )
    {
        *ifindex_category_p = IF_MGR_VLAN_IFINDEX;
        return;
    } /* end of if */

    *ifindex_category_p = IF_MGR_ERROR_IFINDEX;
    return;
}  /* end of IF_MGR_IfindexToCategory()*/


/* FUNCTION NAME: IF_MGR_GetNextLowerIndexForVlan
 * PURPOSE: This function will return true if the next available lower layer index for vlan
 *          interface can be found.  Otherwise, return false.
 * INPUT:  portlist         - egress port list of specific vlan
 *         lower_index      - current lport_ifindex joined the specific vlan
 * OUTPUT: lower_index      - next available lport_ifindex joined the specific vlan.
 * RETURN: none
 * NOTES:  1. lower_index represents the logical port number joined the specific vlan.  The
 *            next available lower_index is the next available port joined the vlan.
 *         2. if the specific lower_index is the last lport joined the specific vlan, then
 *            get next vlan and first egress port list member.
 */
static BOOL_T IF_MGR_GetNextLowerIndexForVlan(UI8_T *portlist, UI32_T *lower_index)
{
    UI8_T   port_map[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    UI32_T  port_num;

    /* BODY */

    memset(port_map, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);

    L_CVRT_convert_portList_2_portMap(portlist, port_map, SYS_ADPT_TOTAL_NBR_OF_LPORT, 0x31);

    for(port_num =*lower_index; SWCTRL_POM_GetNextLogicalPort(&port_num) != SWCTRL_LPORT_UNKNOWN_PORT; )
    {
        if(port_map[port_num -1] == '1')
        {
            *lower_index = port_num;
            return TRUE;
        } /* end of if */
    } /* end of for */

    return FALSE;

} /* end of IF_MGR_GetNextLowerIndexForVlan() */


/* FUNCTION NAME: IF_MGR_GetNextLowerIndexForTrunk
 * PURPOSE: This function will return true if the next available lower layer index for trunk
 *          interface can be found.  Otherwise, return false.
 * INPUT:  trunk_id         - specific trunk index to search
 *         lower_index      - current lport_ifindex joined the specific trunk
 * OUTPUT: lower_index      - next available lport_ifindex joined the specific trunk.
 * RETURN: none
 * NOTES:  1. lower_index represents the logical port number joined the specific trunk.  The
 *            next available lower_index is the next available port joined the trunk.
 *         2. if the specific lower_index is the last lport joined the specific trunk, then
 *            get next trunk id and it's first member.
 */
static BOOL_T IF_MGR_GetNextLowerIndexForTrunk(UI32_T  trunk_id, UI32_T *lower_index)
{
    UI8_T                   port_map[SYS_ADPT_TOTAL_NBR_OF_LPORT];
    TRK_MGR_TrunkEntry_T    trunk_info;
    SWCTRL_PortEntry_T      port_entry;

    /* BODY */
    memset(port_map, 0, SYS_ADPT_TOTAL_NBR_OF_LPORT);

    trunk_info.trunk_index = trunk_id;

    if (!TRK_PMGR_GetTrunkEntry(&trunk_info))
        return FALSE;


    L_CVRT_convert_portList_2_portMap(trunk_info.trunk_ports, port_map, SYS_ADPT_TOTAL_NBR_OF_LPORT, 1);

    port_entry.port_index = *lower_index;
    while (SWCTRL_POM_GetNextPortEntry(&port_entry))
    {
        if(port_map[port_entry.port_index -1] == 1)
        {
            *lower_index = port_entry.port_index;
            return TRUE;
        } /* end of if */
    } /* end of while */

    return FALSE;

} /* end of IF_MGR_GetNextLowerIndexForTrunk() */


/* FUNCTION NAME: IF_MGR_GetNextIfIndex
 * PURPOSE: This function will return true if the next available interface index can be
 *          retrieve.  Otherwise, return false.
 * INPUT:  *current_index_p - specific current interface index
 * OUTPUT: *current_index_p - next available interface index
 * RETURN: none
 * NOTES:  none
 */
static BOOL_T IF_MGR_GetNextIfIndex(UI32_T *current_index_p)
{
    BOOL_T     				next_index_found = FALSE;
    BOOL_T     				end_of_interface = FALSE;
    UI32_T      			ifindex_category;
	UI32_T      			num_of_unit, trunk_member;
	UI32_T                  unit, port, trunk_id=0, vid;
	SWCTRL_Lport_Type_T     port_type;
	Port_Info_T             port_info;

    /* BODY */

    while (!(next_index_found || end_of_interface))
    {
        /* special case for "get-first":
         *
         * If the following check is TRUE, "IF_MGR_IfindexToCategory" returns
         * IF_MGR_ERROR_IFINDEX, but we still need to call SwCtrl to get the first port
         * from index 0.
         */
        if (*current_index_p < SYS_ADPT_ETHER_1_IF_INDEX_NUMBER)
        {
            *current_index_p = 0;
            ifindex_category = IF_MGR_NORMAL_IFINDEX;
        }
        else
        {
            IF_MGR_IfindexToCategory(*current_index_p, &ifindex_category);
        }

        switch (ifindex_category)
        {
            /* user port (non-trunk-member or member) or trunk
             */
            case IF_MGR_NORMAL_IFINDEX:
            case IF_MGR_TRUNK_IFINDEX:

#if 0  /* not used */
                /* This API only return Normal port and Trunk port.
                 */
                if (SWCTRL_POM_GetNextLogicalPort(current_index_p)!=SWCTRL_LPORT_UNKNOWN_PORT)
#endif
                /* March-24-2002.  Trunk member port information also needs to be returned.
                 */
                if (SWCTRL_POM_GetNextPortInfo(current_index_p, &port_info))
                {
                    port_type = SWCTRL_POM_LogicalPortToUserPort(*current_index_p, &unit, &port, &trunk_id);

                    /* user port
                     */
                    if ((port_type == SWCTRL_LPORT_NORMAL_PORT) ||
                        (port_type == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                    {
                        /* if this port exists
                         */
                        if (STKTPLG_POM_PortExist(unit, port))
                        {
                            next_index_found = TRUE;
                        }
                    }

                    /* trunk
                     */
                    else if (port_type == SWCTRL_LPORT_TRUNK_PORT)
                    {
                        /* if this trunk has members, it appears in the ifTable
                         */
                        if ((trunk_member = TRK_PMGR_GetTrunkMemberCounts(trunk_id)) != 0)
                        {
                            next_index_found = TRUE;
                        }
                    }
                }

                /* no more ports or trunks
                 */
                else
                {
                    /* next group: console port
                     */
                    *current_index_p = SYS_ADPT_RS232_1_IF_INDEX_NUMBER;
                    next_index_found = TRUE;
                }
                break;

            /* console port
             */
            case IF_MGR_RS232_IFINDEX:
                /* get number of units (means number of this type of ports)
                 */
                if (!STKTPLG_POM_GetNumberOfUnit(&num_of_unit))
                {
                    return FALSE;
                }

                /* try next console port
                 */
                *current_index_p = *current_index_p + 1;

                /* if no more units, it means no more console ports
                 */
                if (*current_index_p >= SYS_ADPT_RS232_1_IF_INDEX_NUMBER + num_of_unit)
                {
                    /* next group: IP loopback interface
                     */
                    *current_index_p = SYS_ADPT_LOOPBACK_IF_INDEX_BASE;
                }

                next_index_found = TRUE;
                break;

            /* IP loopback interface: next ifIndex is first VLAN
             */
            case IF_MGR_LOOPBACK_IFINDEX:
                *current_index_p = SYS_ADPT_VLAN_1_IF_INDEX_NUMBER;
                next_index_found = TRUE;
                break;

            /* VLAN
             */
            case IF_MGR_VLAN_IFINDEX:
                VLAN_IFINDEX_CONVERTTO_VID(*current_index_p, vid);

                /* try next VLAN
                 */
                if (VLAN_POM_GetNextVlanId(0, &vid))
                {
                    VLAN_VID_CONVERTTO_IFINDEX(vid, *current_index_p);
                    next_index_found = TRUE;
                }

                /* no more VLAN
                 */
                else
                {
                    /* no more interface
                     */
                    end_of_interface = TRUE;
                }
    
                break;

            default:
                end_of_interface = TRUE;
                break;

        } /* end of switch */

    } /* end of while */

    return next_index_found;

}  /* end of IF_MGR_GetNextIfIndex() */

#if 0 /*no one use it*/
/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_IfOperStatusChanged_CallBack
. *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a port operation status is UP.
 * INPUT    : vid_ifindex     -- specify which vlan's this port joined.
 *          : status   -- specify the status of the vlan after its status changed.
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void IF_MGR_IfOperStatusChanged_CallBack (UI32_T vid_ifindex, UI32_T status)
{
    SYS_TYPE_CallBack_T  *fun_list;

    for(fun_list=ifOperStatusChangedHandler; fun_list; fun_list=fun_list->next)
        fun_list->func(vid_ifindex, status);
    return;

} /*end of IF_MGR_IfOperStatusChanged_CallBack()  */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VLAN_MGR_Notify_VlanDestroy
 *------------------------------------------------------------------------------
 * PURPOSE  : Call CallBack function when a vlan is deleted.
 * INPUT    : vid_ifindex -- specify which vlan has just been destroyed
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void IF_MGR_VlanDestroy_CallBack (UI32_T vid_ifindex, UI32_T vlan_status)
{
    SYS_TYPE_CallBack_T  *fun_list;

    for(fun_list=vlanDestroyHandler; fun_list; fun_list=fun_list->next)
        fun_list->func(vid_ifindex, vlan_status);
    return;

} /* end of IF_MGR_VlanDestroy_CallBack () */
#endif
/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_InitPortTable
 *------------------------------------------------------------------------------
 * PURPOSE  : This function initialize port table so that each interface will have
 *            a corresponding name associate to it.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *------------------------------------------------------------------------------*/
static void IF_MGR_InitPortTable(void)
{
    return;
} /* end of IF_MGR_InitPortTable() */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetIfName
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return TRUE if the IfName corresponding to the
 *            specified IfIndex is available, else return FALSE.
 * INPUT    : ifindex   -- the IfIndex
 * OUTPUT   : str       -- the IfName
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  BOOL_T  IF_MGR_GetIfName(UI32_T ifindex, UI8_T *str)
{
    UI32_T          ifindex_category, unit, port, trunk_id;
    Port_Info_T     port_info;
    //BOOL_T          result  = FALSE;
    UI8_T format_str[MAX_BUF+1]={0};

    IF_MGR_IfindexToCategory(ifindex, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
            if (SWCTRL_POM_GetPortInfo(ifindex,&port_info ))
            {
                switch(port_info.speed_duplex_oper)
                {
                    case VAL_portSpeedDpxStatus_halfDuplex10:
                    case VAL_portSpeedDpxStatus_fullDuplex10:
                        strncpy((char *)format_str, SYS_ADPT_ETHERNET_IFNAME, MAX_BUF);
                        break;
                    case VAL_portSpeedDpxStatus_halfDuplex100:
                    case VAL_portSpeedDpxStatus_fullDuplex100:
                        strncpy((char *)format_str, SYS_ADPT_FASTETH_IFNAME, MAX_BUF);
                        break;
                    case VAL_portSpeedDpxStatus_halfDuplex1000:
                    case VAL_portSpeedDpxStatus_fullDuplex1000:
                        strncpy((char *)format_str, SYS_ADPT_GIGABIT_IFNAME, MAX_BUF);
                        break;
                    default:
                        strncpy((char *)format_str, SYS_ADPT_ETHERNET_IFNAME, MAX_BUF);
                        break;
                }
            } /* End of if */
            break;

        case IF_MGR_TRUNK_IFINDEX:
            /* Add brace for good convention
             */
            if (( SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT)
            {
                strncpy((char *)format_str, SYS_ADPT_TRUNK_IFNAME, MAX_BUF);
            }
            else
            {
                str[0]=0;
                return FALSE;
            }
            break;

        case IF_MGR_RS232_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_RS232_IFNAME, MAX_BUF);
            break;

        case IF_MGR_LOOPBACK_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_LOOPBACK_IFNAME, MAX_BUF);
            break;

        case IF_MGR_VLAN_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_VLAN_IFNAME, MAX_BUF);
            break;
        default:
            str[0]=0;
            return FALSE;
            break;
    } /* End of switch */
    return IF_MGR_IfSprintf(str, format_str, ifindex);
} /* End of IF_MGR_GetIfName */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetIfDescr
 *------------------------------------------------------------------------------
 * PURPOSE  : This function will return TRUE if the IfDescr corresponding to the
 *            specified IfIndex is available, else return FALSE.
 * INPUT    : ifindex   -- the IfIndex
 * OUTPUT   : str       -- the IfDescr
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  BOOL_T  IF_MGR_GetIfDescr(UI32_T ifindex, UI8_T *str)
{
    UI32_T              ifindex_category, unit, port, trunk_id;
    SWCTRL_Lport_Type_T port_type;
    UI8_T format_str[MAX_BUF+1]={0};

    IF_MGR_IfindexToCategory(ifindex, &ifindex_category);

    switch (ifindex_category)
    {
        case IF_MGR_NORMAL_IFINDEX:
        case IF_MGR_TRUNK_IFINDEX:
            port_type   = SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);
            switch (port_type)
            {
                case SWCTRL_LPORT_NORMAL_PORT:
#if (SYS_CPNT_MGMT_PORT == TRUE)
                    if (ifindex == SYS_ADPT_MGMT_PORT)
                        strncpy((char *)format_str, SYS_ADPT_MPORT_IF_DESC_STR, MAX_BUF);
                    else
                        strncpy((char *)format_str, SYS_ADPT_LPORT_IF_DESC_STR, MAX_BUF);
#else
                    strncpy((char *)format_str, SYS_ADPT_LPORT_IF_DESC_STR, MAX_BUF);
#endif
                    break;

                case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
                    strncpy((char *)format_str, SYS_ADPT_TRUNK_MEMBER_IF_DESC_STR, MAX_BUF);
                    break;

                case SWCTRL_LPORT_TRUNK_PORT:
                    strncpy((char *)format_str, SYS_ADPT_TRUNK_IF_DESC_STR, MAX_BUF);
                    break;

                default:
                    str[0] = 0;
                    return FALSE;
                    break;
            }
            break;

        case IF_MGR_RS232_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_RS232_IF_DESC_STR, MAX_BUF);
            break;

        case IF_MGR_LOOPBACK_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_LOOPBACK_IF_DESC_STR, MAX_BUF);
            break;

        case IF_MGR_VLAN_IFINDEX:
            strncpy((char *)format_str, SYS_ADPT_VLAN_IF_DESC_STR, MAX_BUF);
            break;
        default:
            str[0] = 0;
            return FALSE;
            break;
    } /* End of switch */

    return IF_MGR_IfSprintf(str, format_str, ifindex);
} /* End of IF_MGR_GetIfDescr */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_IfSprintf
 *------------------------------------------------------------------------------
 * PURPOSE  : This function formats the string into the buffer
 * INPUT    : str       -- input string
 *            index     -- the index
 * OUTPUT   : buf       -- output buffer
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  BOOL_T  IF_MGR_IfSprintf(UI8_T *buf, UI8_T *str, UI32_T index)
{
    BOOL_T  result=TRUE;
    UI32_T  unit, port, trunk_id;
    UI8_T arg_num, dir_index, i=0;
    UI8_T buffer[MAX_BUF+1]={0};
    UI32_T value[10];

    arg_num = IF_MGR_CalculateCharCount(str, '$');
    if (arg_num)
    {
        strncpy((char*)buffer, (char*)str, MAX_BUF);
        while ((dir_index = IF_MGR_Remove1stDirectiveIndex(buffer, '$')))
        {
            value[i++]=IF_MGR_ConvertDirectiveIndexToArgument(dir_index, index);
        }
        IF_MGR_Vsprintf(buf, buffer, value);
    }
    else
    {
        switch (IF_MGR_CalculateCharCount(str, '%') )
        {
            case 0:
                strncpy((char*)buf, (char*)str, MAX_BUF);
                break;
            case 1:
                /* Format 1 */
                sprintf((char *)buf, (char *)str, index);
                break;
            case 2:
                /* Format 2: index need to be converted to unit:port */
                if (( SWCTRL_POM_LogicalPortToUserPort(index, &unit, &port, &trunk_id)) == SWCTRL_LPORT_UNKNOWN_PORT)
                    result  = FALSE;
                else
                    sprintf((char *)buf, (char *)str, unit, port);
                break;
            default:
                result  = FALSE;
        } /* End of switch */
    }
    return result;
}

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_CalculateCharCount
 *------------------------------------------------------------------------------
 * PURPOSE  : This function calculates the count of the specified character for
 *            the input string.
 * INPUT    : str       -- input string
 *            character -- specified character
 * OUTPUT   : None
 * RETURN   : count of the specified character
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  UI8_T   IF_MGR_CalculateCharCount(UI8_T *str, UI8_T character)
{
    UI8_T   index, count, ch;

    index   = 0;
    count   = 0;
    while ((ch = *(str+index)))
    {
        if (ch == character)
        {
            count++;
        } /* End of if */
        index++;
    } /* End of while */
    return count;
} /* End of IF_MGR_CalculateCharCount */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_ConvertDirectiveIndexToArgument
 *------------------------------------------------------------------------------
 * PURPOSE  : This function removes the first directive index for the input string.
 * INPUT    : dir_char  -- directive character
 *            ifindex   -- ifindex
 * OUTPUT   : none
 * RETURN   : the value of the argument specified by the directive character
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  UI32_T  IF_MGR_ConvertDirectiveIndexToArgument(UI8_T dir_char, UI32_T ifindex)
{
    UI32_T  arg_val=0;
    UI32_T  unit, port, trunk_id;
    switch (dir_char)
    {
        case 'i':
            arg_val=ifindex;
            break;
        case 'p':
            if (( SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_NORMAL_PORT)
                arg_val=port;
            break;
        case 'u':
            if (( SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)) != SWCTRL_LPORT_UNKNOWN_PORT)
                arg_val=unit;
            break;
        case 't':
            if ((( SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT) ||
            	(( SWCTRL_POM_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id)) == SWCTRL_LPORT_TRUNK_PORT_MEMBER))
                arg_val=trunk_id;
            break;
        case 'b':
            arg_val = ifindex - SYS_ADPT_LOOPBACK_IF_INDEX_BASE;
            break;
        case 'v':
            arg_val=ifindex-SYS_ADPT_VLAN_1_IF_INDEX_NUMBER+1;
            break;
    } /* End of switch */
    return arg_val;
} /* End of IF_MGR_ConvertDirectiveIndexToArgument */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_Remove1stDirective
 *------------------------------------------------------------------------------
 * PURPOSE  : This function removes the first directive index for the input string.
 * INPUT    : str       -- input string
 *            character -- specified character reserved for the directive
 * OUTPUT   : buf       -- output string
 * RETURN   : the directive character
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  UI8_T   IF_MGR_Remove1stDirectiveIndex(UI8_T *str, UI8_T character)
{
    UI8_T buffer[MAX_BUF+1]={0};
    UI8_T *pFound;
    UI8_T c;
    UI8_T str_len;

    strncpy((char*)buffer, (char*)str, MAX_BUF);
    str_len = strlen((char*)buffer);
    if ((pFound=(UI8_T *)strchr((char*)buffer, character))!=NULL)
    {
        c=*(pFound+1);
        memcpy(str, buffer, pFound-buffer);
        strncpy((char*)(str+(pFound-buffer)), (char*)pFound+2, str_len-(pFound-buffer)-2);
        *(str+str_len-2) = 0;
        return c;
    }
    else
        return 0;
} /* End of IF_MGR_Remove1stDirectiveIndex */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_Vsprintf
 *------------------------------------------------------------------------------
 * PURPOSE  : This function formats the string into the buffer using the specified
 *            argument list
 * INPUT    : str       -- input string
 *            arg_list  -- the list, or the array, of the arguments
 * OUTPUT   : buf       -- output buffer
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------*/
static  BOOL_T  IF_MGR_Vsprintf(UI8_T *buf, UI8_T *str, UI32_T *arg_list)
{
    UI8_T   i, j, ch, arg_index;
    UI8_T   tmp_buf[MAX_BUF+1]={0};
    BOOL_T  arg_flag;
    UI32_T  arg_val;

    memset(tmp_buf, 0, sizeof(tmp_buf));
    i   = 0;
    j   = 0;
    arg_index   = 0;
    arg_flag    = FALSE;
    while (     (i < MAX_BUF)
            &&  (ch = str[i++])
          )
    {
        tmp_buf[j++]    = ch;
        if (ch == '%')
        {
            arg_flag    = TRUE;
        }
        if (    arg_flag
             && (ch >= 'c')
             && (ch <= 'x')
           )
        {
            arg_flag    = FALSE;
            arg_val     = arg_list[arg_index++];
            sprintf((char *)buf, (char *)tmp_buf, arg_val);
            strncpy((char *)tmp_buf, (char *)buf, MAX_BUF);
            j   = strlen((char *)tmp_buf);
        }
    }

    return TRUE;
} /* End of IF_MGR_Vsprintf */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - IF_MGR_GetIfType
 *------------------------------------------------------------------------------
 * PURPOSE  : This function determines the type of interface based on ifindex
 *            value.
 * INPUT    : ifindex -- interface index
 * OUTPUT   : iftype -- type of interface based on ifindex
 *                      (IF_MGR_NORMAL_IFINDEX,
 *                       IF_MGR_TRUNK_IFINDEX,
 *                       IF_MGR_RS232_IFINDEX,
 *                       IF_MGR_VLAN_IFINDEX,
 *                       IF_MGR_ERROR_IFINDEX)
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *------------------------------------------------------------------------------
 */
BOOL_T  IF_MGR_GetIfType(UI32_T ifindex, UI32_T *iftype)
{
    if (ifindex == 0 || iftype == NULL)
        return FALSE;

    IF_MGR_IfindexToCategory(ifindex, iftype);

    if (*iftype == IF_MGR_ERROR_IFINDEX)
        return FALSE;
    else
        return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME :  IF_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca mgr.
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
BOOL_T   IF_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    IF_MGR_IPCMsg_T *msg_data_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p = ( IF_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        /*EPR:NULL
         *Problem:When slave enter transition mode,if msg_size have not
         *        any value,will cause sender receive reply overflow.
         *Solution: use a default msg_size to reply the sender.
         *Fixed by:DanXie
         *Modify file:if_mgr.c
         *Approved by:Hardsun
         */
        ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
        msg_data_p->type.result_ui32 = FALSE;
        goto exit;
    }

    switch(cmd)
    {

        case IF_MGR_IPCCMD_GETIFNUMBER:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_bool= IF_MGR_GetIfNumber(
                &msg_data_p->data.ui32_v);
            break;

        case IF_MGR_IPCCMD_GETIFTABLELASTCHANGE:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_bool= IF_MGR_GetIfTableLastChange(
                &msg_data_p->data.ui32_v);
            break;

        case IF_MGR_IPCCMD_GETIFENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_entry);
            msg_data_p->type.result_bool= IF_MGR_GetIfEntry(
                &msg_data_p->data.if_entry);
            break;

        case IF_MGR_IPCCMD_GETNEXTIFENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_entry);
            msg_data_p->type.result_bool= IF_MGR_GetNextIfEntry(
                &msg_data_p->data.if_entry);
            break;

        case IF_MGR_IPCCMD_SETIFADMINSTATUS:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= IF_MGR_SetIfAdminStatus(
                msg_data_p->data.index_adminstatus.if_index,
                msg_data_p->data.index_adminstatus.if_admin_status);
            break;

        case IF_MGR_IPCCMD_GETIFXENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_x_entry);
            msg_data_p->type.result_bool= IF_MGR_GetIfXEntry(
                &msg_data_p->data.if_x_entry);
            break;

        case IF_MGR_IPCCMD_GETNEXTIFXENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_x_entry);
            msg_data_p->type.result_bool= IF_MGR_GetNextIfXEntry(
                &msg_data_p->data.if_x_entry);
            break;

        case IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLEGLOBAL:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool = IF_MGR_SetIfLinkUpDownTrapEnableGlobal(
                msg_data_p->data.ui32_v);
            break;

        case IF_MGR_IPCCMD_SETIFLINKUPDOWNTRAPENABLE:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= IF_MGR_SetIfLinkUpDownTrapEnable(
                msg_data_p->data.index_trapstatus.if_x_index,
                msg_data_p->data.index_trapstatus.trap_status);
            break;

        case IF_MGR_IPCCMD_GETRUNNINGIFLINKUPDOWNTRAPENABLE:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_trapstatus);
            msg_data_p->type.result_ui32= IF_MGR_GetRunningIfLinkUpDownTrapEnable(
                msg_data_p->data.index_trapstatus.if_x_index,
                &msg_data_p->data.index_trapstatus.trap_status);
            break;

        case IF_MGR_IPCCMD_SETIFALIAS:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_bool= IF_MGR_SetIfAlias(
                msg_data_p->data.index_alias.if_x_index,
                msg_data_p->data.index_alias.if_alias);
            break;

        case IF_MGR_IPCCMD_GETIFSTACKENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_stack_entry);
            msg_data_p->type.result_bool= IF_MGR_GetIfStackEntry(
                &msg_data_p->data.if_stack_entry);
            break;


        case IF_MGR_IPCCMD_GETNEXTIFSTACKENTRY:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.if_stack_entry);
            msg_data_p->type.result_bool= IF_MGR_GetNextIfStackEntry(
                &msg_data_p->data.if_stack_entry);
            break;

        case IF_MGR_IPCCMD_GETIFSTACKLASTCHANGE:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.ui32_v);
            msg_data_p->type.result_bool= IF_MGR_GetIfStackLastChange(
                &msg_data_p->data.ui32_v);
            break;

        case IF_MGR_IPCCMD_IFNAMETOIFINDEX:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_name);
            msg_data_p->type.result_bool= IF_MGR_IfnameToIfindex(
                msg_data_p->data.index_name.ifname,
                &msg_data_p->data.index_name.ifindex);
            break;

        case IF_MGR_IPCCMD_GETIFTYPE:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.index_type);
            msg_data_p->type.result_bool= IF_MGR_GetIfType(
                msg_data_p->data.index_type.ifindex,
                &msg_data_p->data.index_type.iftype);
            break;

        default:
            ipcmsg_p->msg_size= IF_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd< IF_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    return TRUE;
}

