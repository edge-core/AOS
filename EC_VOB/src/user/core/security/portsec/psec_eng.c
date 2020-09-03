/* Module Name: PSEC_ENG.C
 * Purpose:
 *        ( 1. Whole module function and scope.                 )
 *         This file provides the hardware independent interface of port security
 *         control functions to applications.
 *        ( 2.  The domain MUST be handled by this module.      )
 *         This module includes port security manipulation.
 *        ( 3.  The domain would not be handled by this module. )
 *         None.
 * Notes:
 *        ( Something must be known or noticed by developer     )
 * History:
 *       Date        Modifier    Reason
 *      2002/5/30    Arthur Wu   Create this file
 *      2003/4/24    Arden Chiu  Enhancement->Delete address by port rather
 *                               than one by one when changing port state.
 *
 * Copyright(C)      Accton Corporation, 2002
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "swctrl_pmgr.h"
#include "swctrl.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "amtrdrv_pom.h"
#include "psec_eng.h"
#include "psec_mgr.h"
#include "psec_om.h"
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "netaccess_backdoor.h"


/* NAMING CONSTANT
 */
#define LOCAL_HOST                              1


/* MACRO DEFINITIONS
 */
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
#define LOG(fmt, args...) \
    {                                       \
        if(NETACCESS_BACKDOOR_IsOn(psec_eng_backdoor_reg_no)) {printf(fmt, ##args);printf("%s","\n");}  \
    }
#else
#define LOG(fmt, args...)
#endif

/* TYPE DECLARATIONS
 */
enum /* function number */
{
    PSEC_ENG_SetPortSecurityStatus_FUNC_NO = 0,
    PSEC_ENG_SetPortSecurityActionStatus_FUNC_NO
};

/* STATIC VARIABLE DECLARATIONS
 */
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
static UI32_T psec_eng_backdoor_reg_no;
#endif

/* LOCAL FUNCTION DECARATION
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert the learnt secured MAC address into manual configured
 *           on the specified interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
PSEC_ENG_PrivConvertSecuredAddressIntoManual(
    UI32_T ifindex
);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSecMacLifeTime
 *------------------------------------------------------------------------
 * FUNCTION: Get the life time for secure MAC address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static AMTR_TYPE_AddressLifeTime_T PSEC_ENG_PrivSecMacLifeTime();

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivNormalMacLifeTime
 *------------------------------------------------------------------------
 * FUNCTION: Get the life time for normal MAC address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static AMTR_TYPE_AddressLifeTime_T PSEC_ENG_PrivNormalMacLifeTime();

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSetAmtrPortInfo
 *------------------------------------------------------------------------
 * FUNCTION: Set port infor to AMTR
 * INPUT   : ifindex, port_info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_ENG_PrivSetAmtrPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSetSwctrlPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set port security status to SWCTRL
 * INPUT   : ifindex, port_info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_ENG_PrivSetSwctrlPortSecurityStatus(UI32_T ifindex, UI32_T port_security_status);

static BOOL_T PSEC_ENG_PrivGetAmtrPortInfoByStatus(UI32_T ifindex, UI32_T psec_status, UI32_T mac_count, AMTR_MGR_PortInfo_T *port_info_p);
static BOOL_T PSEC_ENG_PrivSetPortSecurityStatusOperation(UI32_T ifindex, UI32_T psec_status);

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivDeleteAllSecAndLearntAddress
 *------------------------------------------------------------------------
 * FUNCTION: Delete all secured and learnt MAC address on the specified port
 * INPUT   : ifindex    -- ifindex
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_ENG_PrivDeleteAllSecAndLearntAddress(UI32_T ifindex);


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_ENG_Init (void)
{
#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
    NETACCESS_BACKDOOR_Register("psec_eng", &psec_eng_backdoor_reg_no);
#endif
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security status
 * INPUT    : ifindex : the logical port
 *            psec_status : VAL_portSecPortStatus_enabled
 *                          VAL_portSecPortStatus_disabled
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: It should call from UI config. So it updates AMTR port info, too.
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityStatus(UI32_T ifindex, UI32_T psec_status)
{
    UI32_T mac_count;
    AMTR_MGR_PortInfo_T port_info;
    AMTR_MGR_PortInfo_T backup_port_info;

    if (   (FALSE == PSEC_OM_GetMaxMacCount(ifindex, &mac_count))
        || (FALSE == AMTR_OM_GetPortInfo(ifindex, &backup_port_info))
        || (FALSE == PSEC_ENG_PrivGetAmtrPortInfoByStatus(ifindex, psec_status, mac_count, &port_info))
        )
    {
        return FALSE;
    }

    /* Change life time of port info need to modify all mac entries on OM and CHIP
     * learnt on the port, since all of the mac entries on the port need to be
     * removed when calling PSEC_ENG_PrivSetPortSecurityStatusOperation() on port security
     * being changed to disabled state, it is better to call PSEC_ENG_PrivSetPortSecurityStatusOperation()
     * before changing AMTR port info to avoid extra operations in AMTR.
     */
    if (FALSE == PSEC_ENG_PrivSetPortSecurityStatusOperation(ifindex, psec_status))
    {
        /*PSEC_ENG_PrivSetAmtrPortInfo(ifindex, &backup_port_info);*/
        return FALSE;
    }

    if (FALSE == PSEC_ENG_PrivSetAmtrPortInfo(ifindex, &port_info))
    {
        PSEC_ENG_PrivSetPortSecurityStatusOperation(ifindex,
            (psec_status==VAL_portSecPortStatus_enabled)?VAL_portSecPortStatus_disabled:VAL_portSecPortStatus_enabled);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityStatus_Callback
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security status
 * INPUT    : ifindex : the logical port
 *            psec_status : VAL_portSecPortStatus_enabled
 *                          VAL_portSecPortStatus_disabled
 * OUTPUT	: None
 * RETURN   : TRUE/FALSE
 * NOTE		: It should call from netaccess group only. And, it doesn't update
 *            AMTR port info because it only tries to change port status and
 *            delete MAC addresses if necessary.
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityStatus_Callback(UI32_T ifindex, UI32_T psec_status)
{
    if (FALSE == PSEC_ENG_PrivSetPortSecurityStatusOperation(ifindex, psec_status))
    {
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityMaxMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security max mac count
 * INPUT    : ifindex   : the logical port
 *            mac_count : max-mac-count
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE	    : None
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecurityMaxMacCount(UI32_T ifindex, UI32_T mac_count)
{
    UI32_T psec_status;

    if (FALSE == PSEC_OM_GetPortSecurityStatus(ifindex, &psec_status))
    {
        return FALSE;
    }


    if (psec_status == VAL_portSecPortStatus_enabled)
    {
        UI32_T learnt_count =0;

        learnt_count = AMTRDRV_OM_GetSecurityCounterByport(ifindex);
        if (mac_count < learnt_count)
        {
            if (FALSE == PSEC_ENG_PrivDeleteAllSecAndLearntAddress(ifindex))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_SetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will set the port security action status
 * INPUT    : ifindex : the logical port
 *            action_status: VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_ENG_SetPortSecurityActionStatus( UI32_T ifindex, UI32_T  action_status)
{
    if(action_status == VAL_portSecAction_none)
    {
        SWCTRL_SetPortSecurityActionStatus (ifindex, VAL_portSecAction_trap);
    }
    else
    {
        SWCTRL_SetPortSecurityActionStatus (ifindex, action_status);
    }

    return TRUE;
} /* End of PSEC_ENG_SetPortSecurityActionStatus () */

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_ENG_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_ENG_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T port_security_enabled_by_who;/*kevin Mercury_V2-00430*/

    if (port_security_status == 0)
        return FALSE;

    if (SWCTRL_IsSecurityPort (ifindex,&port_security_enabled_by_who))/*kevin Mercury_V2-00430*/
    {
        if( port_security_enabled_by_who == SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC)/*kevin Mercury_V2-00430*/
        {
            *port_security_status = VAL_portSecPortStatus_enabled;
        }
        else
        {
            *port_security_status = VAL_portSecPortStatus_disabled;
        }
    }
    else
    {
        *port_security_status = VAL_portSecPortStatus_disabled;
    }
    return TRUE;
#else
    return FALSE;
#endif
} /* End of PSEC_ENG_GetPortSecurityStatus () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_GetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->vid  -- vid
 *              port_sec_addr_entry->mac  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_GetPortSecAddrEntry(PSEC_ENG_PortSecAddrEntry_T *port_sec_addr_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    AMTR_TYPE_AddrEntry_T addr_entry;
    AMTR_MGR_PortInfo_T amtr_port_info;

    if (port_sec_addr_entry == 0)
        return FALSE;

    addr_entry.vid = port_sec_addr_entry->vid;
    memcpy (addr_entry.mac,
            port_sec_addr_entry->mac,
            SYS_ADPT_MAC_ADDR_LEN);
    if (!AMTR_MGR_GetExactAddrEntry (&addr_entry))
        return FALSE;
    if(!AMTR_OM_GetPortInfo(addr_entry.ifindex, &amtr_port_info))
        return FALSE;
    if(amtr_port_info.protocol!=AMTR_MGR_PROTOCOL_PSEC)
        return FALSE;

    port_sec_addr_entry->ifindex = addr_entry.ifindex;

    return TRUE;
#else
    return FALSE;
#endif
} /* End of PSEC_ENG_GetPortSecAddrEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_GetNextPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified next addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->vid  -- vid
 *              port_sec_addr_entry->mac  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_GetNextPortSecAddrEntry(PSEC_ENG_PortSecAddrEntry_T *port_sec_addr_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    AMTR_TYPE_AddrEntry_T addr_entry;
    AMTR_MGR_PortInfo_T amtr_port_info;

    if (port_sec_addr_entry == 0)
        return FALSE;

    addr_entry.vid = port_sec_addr_entry->vid;
    memcpy (addr_entry.mac,
            port_sec_addr_entry->mac,
            SYS_ADPT_MAC_ADDR_LEN);
    while (AMTR_MGR_GetNextMVAddrEntry (&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {
        if (AMTR_OM_GetPortInfo(addr_entry.ifindex, &amtr_port_info))
        {
            if(amtr_port_info.protocol==AMTR_MGR_PROTOCOL_PSEC)
            {
                memcpy (port_sec_addr_entry->mac,
                        addr_entry.mac,
                        SYS_ADPT_MAC_ADDR_LEN);
                port_sec_addr_entry->vid = addr_entry.vid;
                port_sec_addr_entry->vid = addr_entry.ifindex;
                return TRUE;
            }
        }
    }

    return FALSE;
#else
    return FALSE;
#endif
} /* End of PSEC_ENG_GetNextPortSecAddrEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_ENG_SetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to create/update/remove port scure address
 *              entry.
 * INPUT    :   vid -- vid
 *              mac -- mac
 *              ifindex -- ifindex
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   ifindex = 0 => remove the entry
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_SetPortSecAddrEntry (   UI32_T vid,
                                        UI8_T *mac,
                                        UI32_T ifindex )
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T port_security_enabled_by_who;/*kevin Mercury_V2-00430*/
    AMTR_TYPE_AddrEntry_T addr_entry;   /*water_huang add*/

    /* ifindex = 0 ==> destroy the entry
     */
    if (ifindex == 0)
        return AMTR_MGR_DeleteAddr (vid ,
                                    mac);

    if (!SWCTRL_IsSecurityPort (ifindex,&port_security_enabled_by_who/*kevin Mercury_V2-00430*/))
        return FALSE;

    /* ifindex != 0 ==> create/update addr entry
     */
    addr_entry.ifindex=ifindex;
    addr_entry.vid=vid;
    memcpy(addr_entry.mac,mac,AMTR_TYPE_MAC_LEN);
    addr_entry.life_time=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    addr_entry.source=AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
    addr_entry.action=AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
    return AMTR_MGR_SetAddrEntry (&addr_entry);
#else
    return FALSE;
#endif
} /* End of PSEC_ENG_SetPortSecAddrEntry () */

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_DeleteAllSecAddress
 *------------------------------------------------------------------------
 * FUNCTION: Delete all secured MAC address on the specified port
 * INPUT   : ifindex    -- ifindex
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_DeleteAllSecAddress(UI32_T ifindex)
{
    return PSEC_ENG_PrivDeleteAllSecAndLearntAddress(ifindex);
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_ConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert port security learnt secured MAC address into manual
 *           configured on the specified interface. If interface is
 *           PSEC_MGR_INTERFACE_INDEX_FOR_ALL, it will apply to all interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_ConvertSecuredAddressIntoManual(
    UI32_T ifindex)
{
    UI32_T i;

    if (PSEC_MGR_INTERFACE_INDEX_FOR_ALL == ifindex)
    {
        for (i = 1; i <= SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        {
            if (TRUE != PSEC_ENG_PrivConvertSecuredAddressIntoManual(i))
            {
                return FALSE;
            }
        }
    }
    else
    {
        if (TRUE != PSEC_ENG_PrivConvertSecuredAddressIntoManual(ifindex))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/* LOCAL FUNCTION DEFINICTION
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivConvertSecuredAddressIntoManual
 *------------------------------------------------------------------------
 * FUNCTION: To convert the learnt secured MAC address into manual configured
 *           on the specified interface.
 * INPUT   : ifindex  -- interface
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T
PSEC_ENG_PrivConvertSecuredAddressIntoManual(
    UI32_T ifindex)
{
    AMTR_TYPE_AddrEntry_T addr_entry;
    UI32_T  port_security_status;

    if ((TRUE != PSEC_OM_GetPortSecurityStatus(ifindex, &port_security_status)) ||
        (VAL_portSecPortStatus_enabled != port_security_status))
    {
        return TRUE;
    }

    memset(&addr_entry, 0, sizeof(addr_entry));
    addr_entry.ifindex = ifindex;

    while (TRUE == AMTR_MGR_GetNextIfIndexAddrEntry(&addr_entry, AMTR_MGR_GET_ALL_ADDRESS))
    {
        if (addr_entry.ifindex != ifindex)
        {
            break;
        }

        if ((AMTR_TYPE_ADDRESS_SOURCE_SECURITY == addr_entry.source) &&
            (AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT != addr_entry.life_time))
        {
            addr_entry.life_time = AMTR_TYPE_ADDRESS_LIFETIME_PERMANENT;
            addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_CONFIG;
            addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;

            if (TRUE != AMTR_MGR_SetAddrEntry(&addr_entry))
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSecMacLifeTime
 *------------------------------------------------------------------------
 * FUNCTION: Get the life time for secure MAC address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static AMTR_TYPE_AddressLifeTime_T PSEC_ENG_PrivSecMacLifeTime()
{
    AMTR_TYPE_AddressLifeTime_T life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;

#ifdef SYS_CPNT_NETACCESS_AGING_MODE

#if (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_DYNAMIC)
    life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
#elif (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_STATIC)
    life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
#elif (SYS_CPNT_NETACCESS_AGING_MODE == SYS_CPNT_NETACCESS_AGING_MODE_CONFIGURABLE)
    {
        UI32_T mode;

        NETACCESS_OM_GetMacAddressAgingMode(&mode);

        if (VAL_networkAccessAging_enabled == mode)
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
        else
            life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    }
#endif

#endif /* #ifdef SYS_CPNT_NETACCESS_AGING_MODE */

    return life_time;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivNormalMacLifeTime
 *------------------------------------------------------------------------
 * FUNCTION: Get the life time for normal MAC address
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static AMTR_TYPE_AddressLifeTime_T PSEC_ENG_PrivNormalMacLifeTime()
{
    AMTR_TYPE_AddressLifeTime_T life_time;
    UI32_T status;

    if (FALSE == AMTR_MGR_GetAgingStatus(&status))
    {
        return AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    }

    if (VAL_amtrMacAddrAgingStatus_enabled == status)
        life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    else
        life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;

    return life_time;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSetAmtrPortInfo
 *------------------------------------------------------------------------
 * FUNCTION: Set port infor to AMTR
 * INPUT   : ifindex, port_info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_ENG_PrivSetAmtrPortInfo(UI32_T ifindex, AMTR_MGR_PortInfo_T *port_info)
{
    LOG("AMTR_SetPortInfo, Port(%lu), Protocol(%s), LifeTime(%s), LearnCnt(%lu)",
        ifindex,
        (port_info->protocol == AMTR_MGR_PROTOCOL_NORMAL)  ? "Norm":
        (port_info->protocol == AMTR_MGR_PROTOCOL_PSEC)    ? "PSec":
        (port_info->protocol == AMTR_MGR_PROTOCOL_DOT1X)   ? "1X":
        (port_info->protocol == AMTR_MGR_PROTOCOL_MACAUTH) ? "MAuth":"1X+MAuth",
        (port_info->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT) ? "DelOnTimeout" :
        (port_info->life_time == AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET) ?  "DelOnReset" : "Other",
        port_info->learn_with_count
    );

    if (FALSE == AMTR_MGR_SetPortInfo(ifindex, port_info))
    {
        LOG("Failed to set AMTR_SetPortInfo");
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivSetSwctrlPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: Set port security status to SWCTRL
 * INPUT   : ifindex, port_info
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_ENG_PrivSetSwctrlPortSecurityStatus(UI32_T ifindex, UI32_T port_security_status)
{
    UI32_T port_security_called_by_who = (VAL_portSecPortStatus_enabled == port_security_status)?
        SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC : SWCTRL_PORT_SECURITY_ENABLED_BY_NONE;

    LOG("SWCTRL_SetPortSecurityStatus, Port(%lu), %s MAC Learn",
        ifindex,
        (VAL_portSecPortStatus_enabled == port_security_status) ? "Disable" : "Enable"
        );

    if (FALSE == SWCTRL_PMGR_SetPortSecurityStatus(ifindex, port_security_status, port_security_called_by_who))
    {
        LOG("Failed to set SWCTRL_SetPortSecurityStatus");
        return FALSE;
    }

    if (VAL_portSecPortStatus_enabled == port_security_status)
    {
        UI32_T psec_action;

        PSEC_OM_GetIntrusionAction(ifindex, &psec_action);
        if(VAL_portSecAction_none == psec_action)
        {
            SWCTRL_PMGR_SetPortSecurityActionStatus (ifindex, VAL_portSecAction_trap);
        }
    }

    return TRUE;
}

static BOOL_T PSEC_ENG_PrivGetAmtrPortInfoByStatus(UI32_T ifindex, UI32_T psec_status, UI32_T mac_count, AMTR_MGR_PortInfo_T *port_info_p)
{
    LOG("%s, Port(%lu), PSecStatus(%s), MacCnt(%lu)",
        __FUNCTION__,
        ifindex,
        PSEC_OM_StrPortSecurityStatus(psec_status),
        mac_count);

    if (FALSE == AMTR_OM_GetPortInfo(ifindex, port_info_p))
    {
        return FALSE;
    }

    /* set to normal port if "no port security max-mac-count" and "no port security"
     */
    if (VAL_portSecPortStatus_disabled == psec_status)
    {
        port_info_p->protocol = AMTR_MGR_PROTOCOL_NORMAL;
        port_info_p->learn_with_count = 0;
        port_info_p->life_time = AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT;
    }
    /* else, it shall be a secure port.
       User issue "port security max-count x" or "port security" commands
       The learn_with_count shall be 0 if "port security" had issued.
     */
    else
    {
        port_info_p->protocol = AMTR_MGR_PROTOCOL_PSEC;
        port_info_p->learn_with_count = 0; /* refined psec schemas */
        port_info_p->life_time = PSEC_ENG_PrivSecMacLifeTime();
    }

    return TRUE;
}

static BOOL_T PSEC_ENG_PrivSetPortSecurityStatusOperation(UI32_T ifindex, UI32_T psec_status)
{
    if (VAL_portSecPortStatus_enabled == psec_status)
    {
        if (FALSE == PSEC_ENG_PrivSetSwctrlPortSecurityStatus(ifindex, psec_status))
        {
            return FALSE;
        }

        /* keep learnt secure mac when changing port security status to enable(lock port) ??
         */
        if (FALSE == AMTR_MGR_IsPortSecurityEnableByAutoLearn(ifindex))
        {
            if (FALSE == PSEC_ENG_PrivDeleteAllSecAndLearntAddress(ifindex))
            {
                return FALSE;
            }
        }
    }
    else
    {
        /* 1. Delete all security mac
         */
        if (FALSE == PSEC_ENG_PrivDeleteAllSecAndLearntAddress(ifindex))
        {
            return FALSE;
        }

        /* Restore as a normal port
         */
        if (FALSE == PSEC_ENG_PrivSetSwctrlPortSecurityStatus(ifindex, psec_status))
        {
            return FALSE;
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_ENG_PrivDeleteAllSecAndLearntAddress
 *------------------------------------------------------------------------
 * FUNCTION: Delete all secured and learnt MAC address on the specified port
 * INPUT   : ifindex    -- ifindex
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_ENG_PrivDeleteAllSecAndLearntAddress(UI32_T ifindex)
{
    BOOL_T ret=TRUE;

    if (FALSE == AMTR_MGR_DeleteAddrBySourceAndLPort(ifindex, AMTR_TYPE_ADDRESS_SOURCE_LEARN))
    {
        LOG("%s, Failed to delete learnt MAC address", __FUNCTION__);
        ret = FALSE;
    }

    if (FALSE == AMTR_MGR_DeleteAddrBySourceAndLPort(ifindex, AMTR_TYPE_ADDRESS_SOURCE_SECURITY))
    {
        LOG("%s, Failed to delete secured MAC address", __FUNCTION__);
        ret = FALSE;
    }

    return ret;
}

