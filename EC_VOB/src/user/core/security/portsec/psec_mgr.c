/* Module Name: PSEC_MGR.C
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
 *      2004/10/22   Hendra Lin  Added Learning ON/OFF API
 *
 *
 * Copyright(C)      Accton Corporation, 2002
 */


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"
#if (SYS_CPNT_PORT_SECURITY == TRUE)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sys_bld.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "swctrl_pmgr.h"
#include "swctrl.h"
#include "syslog_type.h"
#include "amtr_mgr.h"
#include "amtr_om.h"
#include "amtrdrv_pom.h"
#include "sys_callback_mgr.h"
#include "psec_mgr.h"
#include "psec_eng.h"
#include "psec_task.h"
#include "psec_om.h"
#include "leaf_es3626a.h"
#include "trap_event.h"
#include "snmp_pmgr.h"
#if (SYS_CPNT_DOT1X == TRUE)
#include "1x_mgr.h"
#include "1x_om.h"
#endif
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#include "lacp_pmgr.h"
#include "netaccess_backdoor.h"

#if (SYS_CPNT_RSPAN == TRUE)
#include "rspan_om.h"
#endif
#include "sys_time.h"

/* NAMING CONSTANT
 */

/* MACRO DEFINITIONS
 */
#define PSEC_MGR_LOCK()
#define PSEC_MGR_UNLOCK()

#ifndef SYSFUN_USE_CSC
#define SYSFUN_USE_CSC(r)
#define SYSFUN_RELEASE_CSC()
#endif
#define SYS_CPNT_PORT_SECURITY_TRUNK_DEBUG                       FALSE

#if (NETACCESS_SUPPORT_ACCTON_BACKDOOR == TRUE)
#define LOG(fmt, args...) \
    {                                       \
        if(NETACCESS_BACKDOOR_IsOn(psec_mgr_backdoor_reg_no)) {printf(fmt, ##args);printf("%s","\n");}  \
    }
#else
#define LOG(fmt, args...)
#endif

/* TYPE DECLARATIONS
 */
enum /* function number */
{
    PSEC_MGR_GetPortSecurityStatus_FUNC_NO = 0,
    PSEC_MGR_GetPortSecurityActionActive_FUNC_NO,
    PSEC_MGR_SetPortSecurityActionActive_FUNC_NO,
    PSEC_MGR_GetPortSecurityLastIntrusionMac_FUNC_NO,
    PSEC_MGR_GetPortSecurityLastIntrusionTime_FUNC_NO,
    PSEC_MGR_GetPortSecurityActionStatus_FUNC_NO,
    PSEC_MGR_GetPortSecurityEntry_FUNC_NO,
    PSEC_MGR_GetNextPortSecurityEntry_FUNC_NO,
    PSEC_MGR_SetPortSecurityMacCount_FUNC_NO,
    PSEC_MGR_SetPortSecurityStatus_FUNC_NO,
    PSEC_MGR_GetPortSecAddrEntry_FUNC_NO,
    PSEC_MGR_GetNextPortSecAddrEntry_FUNC_NO,
    PSEC_MGR_SetPortSecAddrEntry_FUNC_NO
};

/* LOCAL FUNCTION DECARATION
 */
static void PSEC_MGR_SendIntrusionMacTrap(UI32_T lport, UI8_T *mac_p);
static BOOL_T PSEC_MGR_IsRSPANEnabled(UI32_T ifindex);
static BOOL_T PSEC_MGR_IsOtherCscEnabled(UI32_T ifindex);
static BOOL_T PSEC_MGR_IsPortSecurityStatusOperationValid(UI32_T ifindex, UI32_T portsec_status);

/* STATIC VARIABLE DECLARATIONS
 */
static UI32_T psec_learning_status[SYS_ADPT_TOTAL_NBR_OF_LPORT+1];
static UI32_T psec_mgr_backdoor_reg_no;

SYSFUN_DECLARE_CSC ;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_Init
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize kernel resources
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : Invoked by root.c()
 *------------------------------------------------------------------------*/
void PSEC_MGR_Init (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T i; /* looping */

    for(i=0;i<SYS_ADPT_TOTAL_NBR_OF_LPORT+1;i++)
    {
        psec_learning_status[i] = VAL_portSecLearningStatus_enabled;
    }

#endif

    NETACCESS_BACKDOOR_Register("psec_mgr", &psec_mgr_backdoor_reg_no);

    PSEC_ENG_Init();
    PSEC_OM_InitiateSystemResources();
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_Create_InterCSC_Relation
 *------------------------------------------------------------------------
 * FUNCTION: This function initializes all function pointer registration operations.
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_Create_InterCSC_Relation(void)
{
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will initialize all system resource
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterTransitionMode (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T i;

    SYSFUN_ENTER_TRANSITION_MODE();

    for(i=0;i<SYS_ADPT_TOTAL_NBR_OF_LPORT+1;i++)
    {
        psec_learning_status[i] = VAL_portSecLearningStatus_enabled;
    }

    return;
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetOperationMode
 *------------------------------------------------------------------------
 * FUNCTION: to get the operation mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : SYS_TYPE_STACKING_TRANSITION_MODE | SYS_TYPE_STACKING_MASTER_MODE |
 *           SYS_TYPE_SYSTEM_STATE_SLAVE
 * NOTE    : None
 *------------------------------------------------------------------------*/
UI32_T PSEC_MGR_GetOperationMode(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    return SYSFUN_GET_CSC_OPERATING_MODE();

#else
    return SYS_TYPE_STACKING_TRANSITION_MODE;
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetTransitionMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will set the transition mode
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_SetTransitionMode (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_SET_TRANSITION_MODE();

    return;

#endif
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterMasterMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will enable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterMasterMode (void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T i;

    SYSFUN_ENTER_MASTER_MODE();

// TODO: remove; replace by sys_callback
// SYS_CALLBACK_MGR_CALLBACKEVENTID_AUTO_LEARN
// SYS_CALLBACK_MGR_CALLBACKEVENTID_PORT_ADMIN_ENABLE
#if 0
    //AMTR_MGR_Register_AutoLearn_CallBack((void *)PSEC_MGR_SetPortSecurityStatus);
    AMTR_MGR_Register_AutoLearn_CallBack((void *)PSEC_MGR_SetPortSecurityStatusOperation);

    SWCTRL_Register_LPortAdminEnable_CallBack((void *)PSEC_MGR_NoShutdown_CallBack);
#endif

    for(i=0;i<SYS_ADPT_TOTAL_NBR_OF_LPORT+1;i++)
    {
        psec_learning_status[i] = VAL_portSecLearningStatus_enabled;
    }

    return;

#endif
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_EnterSlaveMode
 *------------------------------------------------------------------------
 * FUNCTION: This function will disable address monitor services
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : None
 * NOTE    : None
 *------------------------------------------------------------------------*/
void PSEC_MGR_EnterSlaveMode(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_ENTER_SLAVE_MODE();
    return;
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_IsInvalidPort
 *------------------------------------------------------------------------
 * FUNCTION: Check whether the ifindex is invalid port
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE - the ifindex is invalid; FALSE - else
 * NOTE    : None
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_IsInvalidPort(UI32_T ifindex)
{
    SWCTRL_Lport_Type_T port_type;
    UI32_T unit;
    UI32_T port;
    UI32_T trunk_id;

    port_type = SWCTRL_LogicalPortToUserPort(ifindex, &unit, &port, &trunk_id);

    if (SWCTRL_LPORT_NORMAL_PORT != port_type
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
        && SWCTRL_LPORT_TRUNK_PORT != port_type
#endif
        )
    {
        return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : The port security status is the current status of port security
 *            of lower layer. This value is not user configured value by
 *            "port security" command.
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityStatus( UI32_T ifindex, UI32_T  *port_security_status_p)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (port_security_status_p == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityStatus_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_ENG_GetPortSecurityStatus (ifindex, port_security_status_p))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityActionActive
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action active state
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_active
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityActionActive( UI32_T ifindex, UI32_T  *action_active_p)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (action_active_p == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityActionActive_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetPortSecurityActionActive(ifindex, action_active_p))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityActionActive
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action active state
 * INPUT    : ifindex   : the logical port
 *            action_active: TRUE/FALSE
 * OUTPUT   :
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityActionActive( UI32_T ifindex, UI32_T  action_active)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (action_active != TRUE &&
		action_active != FALSE)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_SetPortSecurityActionActive_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_SetPortSecurityActionActive (ifindex, action_active))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}


/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityLastIntrusionMac
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion mac
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac address
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityLastIntrusionMac( UI32_T ifindex, UI8_T  *mac)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (mac == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityLastIntrusionMac_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetLastIntrusionMac(ifindex, mac))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityLastIntrusionTime
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the last intrusion time
 * INPUT    : ifindex : the logical port
 * OUTPUT   : last intrusion time
 * RETURN   : TRUE/FALSE
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityLastIntrusionTime( UI32_T ifindex, UI32_T  *seconds)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (seconds == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityLastIntrusionTime_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetLastIntrusionTime(ifindex, seconds))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the action status of port security
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_status
 * RETURN   : TRUE/FALSE
 * NOTE     : none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityActionStatus( UI32_T ifindex, UI32_T  *action_status_p)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (action_status_p == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityActionStatus_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetIntrusionAction(ifindex, action_status_p))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* get running port security status from structur 'amtr_port_info' */
#if 0
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityStatus( UI32_T ifindex,
                                                                  UI32_T  *port_security_status)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    }

    if (port_security_status == NULL)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (!PSEC_ENG_GetPortSecurityStatus (ifindex, port_security_status))
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (*port_security_status != SYS_DFLT_PORT_SECURITY_STATUS)
    {
        /* The port security status should not be got if it is enabled by Auto Learn
         */
        if ( *port_security_status == VAL_portSecPortStatus_enabled &&
             AMTR_MGR_IsPortSecurityEnableByAutoLearn(ifindex)  )
        {
            SYSFUN_RELEASE_CSC();
            return SYS_TYPE_GET_RUNNING_CFG_FAIL;
        }

        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    else
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    SYSFUN_RELEASE_CSC();
    return SYS_TYPE_GET_RUNNING_CFG_FAIL;

#endif
} /* End of PSEC_MGR_GetRunningPortSecurityStatus () */
#else
/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security status for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : port_security_status
 * RETURN   : TRUE/FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityStatus( UI32_T ifindex,
                                                                  UI32_T  *port_security_status)
{
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    }

    if (port_security_status == NULL)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }
    if (FALSE == PSEC_OM_GetPortSecurityStatus(ifindex, port_security_status))
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYSFUN_RELEASE_CSC();
    return (*port_security_status == SYS_DFLT_PORT_SECURITY_STATUS) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;

} /* End of PSEC_MGR_GetRunningPortSecurityStatus () */


#endif

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityActionStatus
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security action status
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : action_status
 * RETURN   : TRUE/FALSE
 * NOTE     : none(1)/trap(2)/shutdown(3)/trapAndShutdown(4)
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityActionStatus( UI32_T ifindex,
                                                                  UI32_T  *action_status)
{
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;

    }

    if (action_status == NULL)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (FALSE == PSEC_OM_GetIntrusionAction (ifindex, action_status))
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYSFUN_RELEASE_CSC();
    return (*action_status == SYS_DFLT_PORT_SECURITY_ACTION_STATUS) ?
       SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of PSEC_MGR_GetRunningPortSecurityActionStatus () */

/*---------------------------------------------------------------------- */
/* ( PortSecurityMgt 1 )--VS2524 */
/*
 *      INDEX       { portSecPortIndex }
 *      portSecPortEntry ::= SEQUENCE
 *      {
 *          portSecPortIndex      INTEGER,
            portSecPortStatus     INTEGER
 *      }
 */
 /*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    BOOL_T retval = FALSE;
    AMTR_MGR_PortInfo_T amtr_port_info; /*water_huang add*/

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;

    }

    if (portsec_entry == NULL)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecurityEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }

    /*phoebe 2004/7/29 epr ES4649-ZZ-00447
     incorrect api called: AMTR_MGR_GetPortSecureMacCount should be => AMTR_MGR_GetPortAutoLearnMacCount */
    if (PSEC_MGR_GetPortSecurityStatus( portsec_entry->portsec_port_index,
                                      &(portsec_entry->portsec_port_status))&&
        (retval = AMTR_OM_GetPortInfo(portsec_entry->portsec_port_index,&amtr_port_info))&&
        (PSEC_MGR_GetPortSecurityLearningStatus(portsec_entry->portsec_port_index,
                                                &(portsec_entry->portsec_learning_status))))
    {
        portsec_entry->portsec_port_mac_count = amtr_port_info.learn_with_count;
        SYSFUN_RELEASE_CSC();
        return TRUE;
    }
    if (retval)
        portsec_entry->portsec_port_mac_count = amtr_port_info.learn_with_count;

    SYSFUN_RELEASE_CSC();
    /* UIMSG_MGR_SetErrorCode ( Error_Code_No); */
    return FALSE;

#else
    return FALSE;
#endif
}


/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetNextPortSecurityEntry
 *------------------------------------------------------------------------
 * FUNCTION: This function will get the next port security management entry
 * INPUT   : portsec_entry->portsec_port_index - interface index
 * OUTPUT  : portsec_entry                     - port security entry
 * RETURN  : TRUE/FALSE
 * NOTE    : VS2524 MIB/PortSecurityMgt 1
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetNextPortSecurityEntry (PSEC_MGR_PortSecurityEntry_T *portsec_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)
    BOOL_T retval = FALSE;
    AMTR_MGR_PortInfo_T amtr_port_info; /*water_huang*/

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;

    }

    if (portsec_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetNextPortSecurityEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }
#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    if (SWCTRL_GetNextLogicalPort (&(portsec_entry->portsec_port_index)) != SWCTRL_LPORT_NORMAL_PORT &&
	  SWCTRL_GetNextLogicalPort (&(portsec_entry->portsec_port_index)) != SWCTRL_LPORT_TRUNK_PORT	)
#else
    if (SWCTRL_GetNextLogicalPort (&(portsec_entry->portsec_port_index)) != SWCTRL_LPORT_NORMAL_PORT)
#endif
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }

    /*phoebe 2004/7/29 epr ES4649-ZZ-00447
     incorrect api called: AMTR_MGR_GetPortSecureMacCount should be => AMTR_MGR_GetPortAutoLearnMacCount */
    if (PSEC_MGR_GetPortSecurityStatus( portsec_entry->portsec_port_index,
                                      &(portsec_entry->portsec_port_status))&&
        (retval = AMTR_OM_GetPortInfo(portsec_entry->portsec_port_index,&amtr_port_info))&&
       (PSEC_MGR_GetPortSecurityLearningStatus(portsec_entry->portsec_port_index,
                                             &(portsec_entry->portsec_learning_status))))
    {
        portsec_entry->portsec_port_mac_count = amtr_port_info.learn_with_count;
        SYSFUN_RELEASE_CSC();
        return TRUE;
    }
    if (retval)
        portsec_entry->portsec_port_mac_count = amtr_port_info.learn_with_count;

    SYSFUN_RELEASE_CSC();
    /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
    return FALSE;

#else
    return FALSE;
#endif
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityMacCount
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :    water_huang modify(93.8.26)
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityMacCount(UI32_T ifindex, UI32_T mac_count)
{
    SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_MGR_SetPortSecurityMacCountOperation(ifindex, mac_count))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_SetMaxMacCount(ifindex, mac_count))
    {
        LOG("Failed to set PSEC_OM(%lu, %lu)", ifindex, mac_count);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* -------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityMacCountOperation
 * -------------------------------------------------------------------------
 * FUNCTION: This function will set port auto learning mac counts
 * INPUT   : ifindex        -- which port to
 *           mac_count      -- mac learning count
 * OUTPUT  : None
 * RETURN  : TRUE: Successfully, FALSE: If not available
 * NOTE    :
 * -------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityMacCountOperation(UI32_T ifindex, UI32_T mac_count)
{
    LACP_MGR_Dot3adLacpPortEntry_T  lacp_entry;

    if (mac_count > SYS_ADPT_MAX_NBR_OF_AUTO_LEARN_MAC)
    {
        LOG("Out of range(%lu)", mac_count);
        EH_MGR_Handle_Exception1(SYS_MODULE_PSEC, PSEC_MGR_SetPortSecurityMacCount_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO, "max-mac-count(0-20)");
        return FALSE;
    }

    if (PSEC_MGR_IsInvalidPort(ifindex))
    {
        LOG("Invalid port(%lu)", ifindex);
        return FALSE;
    }

    /* ES4549-08-00082
       don't allow lacp port to setup max-mac-count
    */
    lacp_entry.dot3ad_lacp_port_index = ifindex;
    if ((TRUE == LACP_PMGR_GetDot3adLacpPortEntry(&lacp_entry)) &&
        (VAL_lacpPortStatus_enabled == lacp_entry.dot3ad_lacp_port_status))
    {
        return FALSE;
    }

    if (TRUE == PSEC_MGR_IsOtherCscEnabled(ifindex))
    {
        return FALSE;
    }

{
    UI32_T old_mac_count;

    if (FALSE == PSEC_OM_GetMaxMacCount(ifindex, &old_mac_count))
    {
        LOG("Failed to get MAC count");
        return FALSE;
    }

    if (mac_count == old_mac_count)
    {
        return TRUE;
    }
}

    if (FALSE == PSEC_ENG_SetPortSecurityMaxMacCount(ifindex, mac_count))
    {
        LOG("Failed to set PSEC_ENG(%lu, %lu)", ifindex, mac_count);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------|
 * ROUTINE NAME - PSEC_MGR_GetRunningPortSecurityMacCount
 * ------------------------------------------------------------------------|
 * FUNCTION : The function will get the port security max mac count
 *            for running config
 * INPUT    : ifindex : the logical port
 * OUTPUT   : mac_count
 * RETURN   : SYS_TYPE_Get_Running_Cfg_T
 * NOTE     : none
 * ------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T PSEC_MGR_GetRunningPortSecurityMacCount( UI32_T ifindex,
                                                                    UI32_T  *mac_count)
{
    SYSFUN_USE_CSC(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (mac_count == NULL)
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (FALSE == PSEC_OM_GetMaxMacCount(ifindex, mac_count))
    {
        SYSFUN_RELEASE_CSC();
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    SYSFUN_RELEASE_CSC();
    return (*mac_count==SYS_DFLT_PORT_SECURITY_MAX_MAC_COUNT) ?
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE : SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of PSEC_MGR_GetRunningPortSecurityMacCount () */



/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityStatus (UI32_T ifindex, UI32_T portsec_status)
{
    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_MGR_SetPortSecurityStatusOperation(ifindex, portsec_status))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_SetPortSecurityStatus(ifindex, portsec_status))
    {
        LOG("Failed to set PSEC_OM(%lu, %s)", ifindex, PSEC_OM_StrPortSecurityStatus(portsec_status));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
} /* End of PSEC_MGR_SetPortSecurityStatus () */

/*------------------------------------------------------------------------
 * ROUTINE NAME - SetPortSecurityStatusOperation
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_SetPortSecurityStatusOperation(UI32_T ifindex, UI32_T portsec_status)
{
    if (FALSE == PSEC_MGR_IsPortSecurityStatusOperationValid(ifindex, portsec_status))
    {
        return FALSE;
    }

    if (FALSE == PSEC_ENG_SetPortSecurityStatus(ifindex, portsec_status))
    {
        LOG("Failed to set PSEC_ENG(%lu, %s)", ifindex, PSEC_OM_StrPortSecurityStatus(portsec_status));
        return FALSE;
    }

    return TRUE;
} /* End of SetPortSecurityStatusOperation() */

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityStatusOperation_Callback
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security status
 * INPUT   : ifindex                - interface index
 *           portsec_status         - VAL_portSecPortStatus_enabled
 *                                    VAL_portSecPortStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_SetPortSecurityStatusOperation_Callback(UI32_T ifindex, UI32_T portsec_status)
{
    if (FALSE == PSEC_MGR_IsPortSecurityStatusOperationValid(ifindex, portsec_status))
    {
        return FALSE;
    }

    if (FALSE == PSEC_ENG_SetPortSecurityStatus_Callback(ifindex, portsec_status))
    {
        LOG("Failed to set PSEC_ENG(%lu, %s)", ifindex, PSEC_OM_StrPortSecurityStatus(portsec_status));
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityActionStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security action status
 * INPUT   : ifindex                - interface index
 *           action_status - VAL_portSecAction_none(1)
 *                           VAL_portSecAction_trap(2)
 *                           VAL_portSecAction_shutdown(3)
 *                           VAL_portSecAction_trapAndShutdown(4)
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityActionStatus (UI32_T ifindex, UI32_T action_status)
{
    UI32_T old_action;

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (PSEC_MGR_IsInvalidPort(ifindex))
    {
        LOG("Invalid port(%lu)", ifindex);
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetIntrusionAction(ifindex, &old_action))
    {
        LOG("Failed to get intrusion action");
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (old_action == action_status)
    {
        SYSFUN_RELEASE_CSC();
        return TRUE;
    }

    if (FALSE == PSEC_ENG_SetPortSecurityActionStatus (ifindex, action_status))
    {
        LOG("Failed to set PSEC_ENG(%lu, %s)", ifindex, PSEC_OM_StrIntrusionAction(action_status));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_SetIntrusionAction(ifindex, action_status))
    {
        LOG("Failed to set PSEC_OM(%lu, %s)", ifindex, PSEC_OM_StrIntrusionAction(action_status));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
} /* End of PSEC_MGR_SetPortSecurityActionStatus () */


/*---------------------------------------------------------------------- */
/* portSecAddrTable */
/*
 *       INDEX       { portSecAddrFdbId, portSecAddrAddress}
 *       ::= { portSecAddrTable 1 }
 *
 *       PortSecAddrEntry ::= SEQUENCE
 *       {
 *             portSecAddrFdbId       Integer32,
 *             portSecAddrAddress     MacAddress,
 *             portSecAddrPort        INTEGER
 *       }
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_GetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->port_sec_addr_fdb_id   -- vid
 *              port_sec_addr_entry->port_sec_addr_address  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_GetPortSecAddrEntry(PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    BOOL_T status = FALSE;

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;
    }

    if (port_sec_addr_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetPortSecAddrEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode ( Error_Code_No); */
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();

    status =  PSEC_ENG_GetPortSecAddrEntry ((PSEC_ENG_PortSecAddrEntry_T *) port_sec_addr_entry);

    if (status == FALSE)
    {
        /* UIMSG_MGR_SetErrorCode ( Error_Code_No); */
        return FALSE;
    }
    else
    {
        return TRUE;
    }

#else
    return FALSE;
#endif
} /* End of PSEC_MGR_GetPortSecAddrEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_GetNextPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion returns true if the specified next addr entry
 *              info can be successfully retrieved. Otherwise, false is
 *              returned.
 * INPUT    :   port_sec_addr_entry->port_sec_addr_fdb_id   -- vid
 *              port_sec_addr_entry->port_sec_addr_address  -- mac
 * OUTPUT   :   port_sec_addr_entry       -- port secury entry info
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_GetNextPortSecAddrEntry (PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;

    }

    if (port_sec_addr_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_GetNextPortSecAddrEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return PSEC_ENG_GetNextPortSecAddrEntry ((PSEC_ENG_PortSecAddrEntry_T *) port_sec_addr_entry);

#else
    return FALSE;
#endif
} /* End of PSEC_MGR_GetNextPortSecAddrEntry () */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_SetPortSecAddrEntry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to create/update/remove port scure address
 *              entry.
 * INPUT    :   port_sec_addr_entry
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   portSecAddrTable
 *              port_sec_addr_port = 0 => remove the entry
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_SetPortSecAddrEntry (PSEC_MGR_PortSecAddrEntry_T *port_sec_addr_entry)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    BOOL_T status=FALSE;

    SYSFUN_USE_CSC(FALSE);
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode(UI32_T error_code); */
        return FALSE;

    }

    if (port_sec_addr_entry == 0)
    {
        EH_MGR_Handle_Exception(SYS_MODULE_PSEC, PSEC_MGR_SetPortSecAddrEntry_FUNC_NO, EH_TYPE_MSG_NULL_POINTER, (EH_MGR_FOR_DEBUG_MSG_PURPOSE | SYSLOG_LEVEL_ERR));
        SYSFUN_RELEASE_CSC();
        /* UIMSG_MGR_SetErrorCode ( Error_Code_No); */
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    status = PSEC_ENG_SetPortSecAddrEntry (   port_sec_addr_entry->port_sec_addr_fdb_id,
                                            port_sec_addr_entry->port_sec_addr_address,
                                            port_sec_addr_entry->port_sec_addr_port);

    if (status == FALSE)
    {
        /* UIMSG_MGR_SetErrorCode ( Error_Code_No); */
        return FALSE;
    }
    else
    {
        return TRUE;
    }

#else
    return FALSE;
#endif
} /* End of PSEC_MGR_SetPortSecAddrEntry () */
#if 0 /* There is no need in refined psec schema */
/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_TimerExpiry
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to check the timer expiry.
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :   1) Check the port security is enable/disable? <enable, then go ahead>
 *              2) Check the
 * ------------------------------------------------------------------------
 */
void PSEC_MGR_TimerExpiry(void)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T  tmp_lport_no = 0,port_security_enabled_by_who/*kevin Mercury_V2-00430*/;

#if (SYS_CPNT_PORT_SECURITY_TRUNK == TRUE)
    while (SWCTRL_GetNextLogicalPort (&tmp_lport_no) == SWCTRL_LPORT_NORMAL_PORT ||
		 SWCTRL_GetNextLogicalPort (&tmp_lport_no) == SWCTRL_LPORT_TRUNK_PORT)
#else
    while (SWCTRL_GetNextLogicalPort (&tmp_lport_no) == SWCTRL_LPORT_NORMAL_PORT)
#endif
    {
        UI32_T  action_status = VAL_portSecAction_none;

        if (!SWCTRL_IsSecurityPort (tmp_lport_no,&port_security_enabled_by_who/*kevin Mercury_V2-00430*/))
            continue;

        //SWCTRL_GetSecurityActionStatus(tmp_lport_no, &action_status);
        if (FALSE == PSEC_OM_GetIntrusionAction(tmp_lport_no, &action_status))
            continue;

        switch (action_status)
        {
            case VAL_portSecAction_trap:
                if (SWCTRL_IsSecurityOperActionTrapPort(tmp_lport_no))
                    continue;

                /* if trap operation status == disable */
                /* if trap operation status is disable(== VAL_portSecAction_none)
                     if have waited for enough time, open up the trap action.
                     otherwise keep waiting.
		 */
                if (SWCTRL_IsSecurityActionTrapTimerExpiry(tmp_lport_no, 60 * SYS_BLD_TICKS_PER_SECOND))
                    SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus(tmp_lport_no, VAL_portSecAction_trap);
                else
                    SWCTRL_PMGR_AddSecurityActionTrapTimeStamp(tmp_lport_no, SYS_BLD_PSEC_LOOKUP_INTERVAL_TICKS);
                break;

            case VAL_portSecAction_trapAndShutdown:
#if 0
#if defined(NOVAL) || defined(NOVAL_12G)
                if (BCMDRV_IsSecViolationDisablePort(tmp_lport_no))
                {
                    SWCTRL_SetPortAdminStatus(tmp_lport_no, VAL_ifAdminStatus_down);

                    if (SWCTRL_IsSecurityOperActionTrapPort(tmp_lport_no))
                    {
                        PSEC_MGR_SendIntrusionMacTrap(tmp_lport_no);
                        SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus(tmp_lport_no, VAL_portSecAction_none);
                        SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp(tmp_lport_no);
                    }
                }
#endif
#else
                if (SWCTRL_IsSecurityOperActionTrapPort(tmp_lport_no))
                    continue;

                /* if trap operation status is disable(== VAL_portSecAction_none)
                     if have waited for enough time(60s), open up the trap action.
                     otherwise keep waiting.
		    */
                if (SWCTRL_IsSecurityActionTrapTimerExpiry(tmp_lport_no, 60 * SYS_BLD_TICKS_PER_SECOND))
                	{
                    SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus(tmp_lport_no, VAL_portSecAction_trap);
                	}
                else
                    SWCTRL_PMGR_AddSecurityActionTrapTimeStamp(tmp_lport_no, SYS_BLD_PSEC_LOOKUP_INTERVAL_TICKS);
#endif

                break;
        }
    }

    return;

#endif
} /* end of PSEC_MGR_TimerExpiry */
#endif
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_MGR_IsMacFilterMatched
 * ---------------------------------------------------------------------
 * PURPOSE: Match the MAC address with each entry of MAC filter table
 * INPUT  : mac, lport
 * OUTPUT : None
 * RETURN : TRUE - Matched; FALSE - No match
 * NOTES  : None
 * ---------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_IsMacFilterMatched(UI32_T lport, UI8_T *mac)
{
#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)
    UI32_T filter_id;
    UI8_T  filter_mac[SYS_ADPT_MAC_ADDR_LEN];
    UI8_T  filter_mask[SYS_ADPT_MAC_ADDR_LEN];

    if ((TRUE == NETACCESS_MGR_GetFilterIdOnPort(lport, &filter_id)) &&
        (0 != filter_id))
    {
        memset(filter_mac, 0, sizeof(filter_mac));
        memset(filter_mask, 0, sizeof(filter_mask));
        while (TRUE == NETACCESS_MGR_GetNextFilterMacByFilterId(filter_id, filter_mac, filter_mask))
        {
            if( ((filter_mac[0] & filter_mask[0]) == (mac[0] & filter_mask[0])) &&
                ((filter_mac[1] & filter_mask[1]) == (mac[1] & filter_mask[1])) &&
                ((filter_mac[2] & filter_mask[2]) == (mac[2] & filter_mask[2])) &&
                ((filter_mac[3] & filter_mask[3]) == (mac[3] & filter_mask[3])) &&
                ((filter_mac[4] & filter_mask[4]) == (mac[4] & filter_mask[4])) &&
                ((filter_mac[5] & filter_mask[5]) == (mac[5] & filter_mask[5])) )
                    return TRUE;
        }

        return FALSE;
    }
#endif /*#if (SYS_CPNT_NETACCESS_MAC_FILTER_TABLE == TRUE)*/
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_ProcessIntrusionMac
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to process intrusion mac
 * INPUT    :   msg_type =  PSEC_TYPE_MSG_INTRUSION_MAC
 *              port = port no.
 *              mac[6] = SA of intrusion mac
 *              reason = L2MUX_MGR_RECV_REASON_INTRUDER
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_ProcessIntrusionMac(PSEC_TYPE_MSG_T *msg)
{
    UI32_T ifindex = msg->port;
    UI32_T agemode=AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET;
    UI32_T action_active=FALSE;
    UI32_T action;
    UI32_T max_mac_count = 0, learnt_count = 0;
    AMTR_TYPE_AddrEntry_T addr_entry;
    BOOL_T retval;

    LOG("%s, port(%lu) MAC(%02X:%02X:%02X:%02X:%02X:%02X)",
        __FUNCTION__,
        ifindex,
        msg->mac[0], msg->mac[1], msg->mac[2], msg->mac[3], msg->mac[4], msg->mac[5]);

    if (TRUE == PSEC_OM_GetPortSecurityActionActive(ifindex, &action_active) &&
        TRUE == PSEC_OM_GetIntrusionAction(ifindex, &action) &&
        action_active == TRUE &&
        (VAL_portSecAction_shutdown == action || VAL_portSecAction_trapAndShutdown == action))
    {

        LOG("Discard packet due to port is shutdowning");

        if (msg->cookie)
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_PSEC, SYS_CALLBACK_MGR_AUTH_FAILED, msg->cookie);
        }
        return TRUE;
    }

    if (FALSE == PSEC_MGR_IsMacFilterMatched(ifindex, msg->mac))
    {
        LOG("%s(): the mac doesn't match mac filter.\n", __FUNCTION__);
        goto un_auth_mac;
    }

    memset(&addr_entry, 0, sizeof(addr_entry));
    addr_entry.vid = msg->vid;
    memcpy(addr_entry.mac, msg->mac, AMTR_TYPE_MAC_LEN);

    if (TRUE == AMTR_MGR_GetExactAddrEntry (&addr_entry) &&
        AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT != addr_entry.life_time)
    {
        if (addr_entry.ifindex != ifindex)
        {
            LOG("Station move detected. Exist MAC on port(%lu).\n",
                (UI32_T)addr_entry.ifindex);

            PSEC_MGR_TakeIntrusionAction(ifindex, msg->mac);
            goto un_auth_mac;
        }
        else
        {
            LOG("This mac entry has been processed, so do nothing.\n");
            goto un_auth_mac;
        }
    }

    LOG("Check psec max_mac_count");
    PSEC_OM_GetMaxMacCount(ifindex, &max_mac_count);
    learnt_count = AMTRDRV_OM_GetSecurityCounterByport(ifindex);

    if (learnt_count >= max_mac_count
#if (SYS_CPNT_PORT_SECURITY_ZERO_MAX_MAC_COUNT_AS_DISABLE == TRUE)
        && max_mac_count > 0
#endif /* SYS_CPNT_PORT_SECURITY_ZERO_MAX_MAC_COUNT_AS_DISABLE */
       )
    {
        LOG("%s(%d):  learn_count(%lu) >= max_count(%lu)", __FUNCTION__, __LINE__, learnt_count, max_mac_count);

        PSEC_MGR_TakeIntrusionAction(ifindex, msg->mac);

        if (msg->cookie)
        {
            SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_PSEC, SYS_CALLBACK_MGR_AUTH_FAILED, msg->cookie);
        }

        return FALSE;
    }

    if (PSEC_OM_GetPortSecurityActionActive(ifindex, &action_active)
        && action_active == TRUE)
    {
        PSEC_OM_SetPortSecurityActionActive(ifindex, FALSE);
    }

    LOG("Authenticate success.\n");
    NETACCESS_OM_GetMacAddressAgingMode(&agemode);

    if (msg->cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_PSEC,
                                                        VAL_networkAccessAging_enabled == agemode ?
                                                        SYS_CALLBACK_MGR_AUTH_AUTHENTICATED_TEMPORARILY :
                                                        SYS_CALLBACK_MGR_AUTH_AUTHENTICATED,
                                                        msg->cookie);
    }
    else
    {
        memset(&addr_entry, 0, sizeof(addr_entry));
        addr_entry.ifindex = ifindex;
        addr_entry.vid = msg->vid;
        memcpy(addr_entry.mac, msg->mac, AMTR_TYPE_MAC_LEN);
        addr_entry.source = AMTR_TYPE_ADDRESS_SOURCE_SECURITY;
        addr_entry.life_time = (agemode==VAL_networkAccessAging_enabled?AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT:AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_RESET);
        addr_entry.action = AMTR_TYPE_ADDRESS_ACTION_FORWARDING;
        retval = AMTR_MGR_SetAddrEntry (&addr_entry);

        if (retval == FALSE)
        {
            LOG("%s(): Failed to AMTR_MGR_SetAddrEntry() \n", __FUNCTION__);
        }
    }

    return TRUE;

un_auth_mac:
    if (msg->cookie)
    {
        SYS_CALLBACK_MGR_AuthenticatePacket_AsyncReturn(SYS_MODULE_PSEC, SYS_CALLBACK_MGR_AUTH_UNAUTHENTICATED, msg->cookie);
    }
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_TakeIntrusionAction
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to take action for intrusion mac
 * INPUT    :   lport = port no.
 *              mac_p = SA of intrusion mac
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_TakeIntrusionAction(UI32_T lport, UI8_T *mac_p)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

#define SECONDS_OF_60 60
    UI32_T  action_status, action_active;
    UI32_T last_intrusion_time=0, trap_time=0;
    UI32_T seconds=0;
    int    year, month, day, hour, minute, second;

    SYS_TIME_GetRealTimeClock(&year, &month, &day, &hour, &minute, &second);

    LOG("%s, port(%lu) MAC(%02X:%02X:%02X:%02X:%02X:%02X) at %d:%d:%d",
        __FUNCTION__,
        lport,
        mac_p[0], mac_p[1], mac_p[2], mac_p[3], mac_p[4], mac_p[5],
        hour, minute, second);

    SYS_TIME_ConvertDateTimeToSeconds(year, month, day, hour, minute, second, &seconds);
    PSEC_OM_SetLastIntrusionTime(lport, seconds);
    PSEC_OM_SetLastIntrusionMac(lport, mac_p);

    if (FALSE == PSEC_OM_GetIntrusionAction(lport, &action_status))
    {
        LOG("Failed to get intrustion action");
        return FALSE;
    }


    if( PSEC_OM_GetPortSecurityActionActive(lport, &action_active)
        && action_active==FALSE)
    {
        if (action_status != VAL_portSecAction_none)
        {
            PSEC_OM_SetPortSecurityActionActive(lport, TRUE);
        }
    }


    if (action_status == VAL_portSecAction_trap || action_status == VAL_portSecAction_trapAndShutdown)
    {
        PSEC_OM_GetLastIntrusionTime(lport, &last_intrusion_time);
        PSEC_OM_GetTrapTime(lport, &trap_time);
        if (trap_time==0 ||
            (last_intrusion_time-trap_time>=SECONDS_OF_60))
        {
            PSEC_MGR_SendIntrusionMacTrap(lport, mac_p);

            trap_time = last_intrusion_time;
            PSEC_OM_SetTrapTime(lport, trap_time);
        }
    }

    if (action_status == VAL_portSecAction_shutdown || action_status == VAL_portSecAction_trapAndShutdown)
    {
        PSEC_ENG_DeleteAllSecAddress(lport);

        LOG("Notifies port(%lu) shutdown", lport);
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_PSEC, lport, FALSE, SWCTRL_PORT_STATUS_SET_BY_PORTSEC);
    }

    return TRUE;

#else
    return FALSE;
#endif
} /* end of PSEC_MGR_ProcessIntrusionMac */


/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_ProcessStationMove
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to process station move
 * INPUT    :   msg_type =  PSEC_TYPE_MSG_STATION_MOVE
 *              port = port no.
 *              mac[6] = SA of intrusion mac
 *              reason = L2MUX_MGR_RECV_REASON_STATION_MOVE
 * OUTPUT   :   None
 * RETURN   :   TRUE/FALSE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_ProcessStationMove(PSEC_TYPE_MSG_T *msg)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    UI32_T  action_status = VAL_portSecAction_none,port_security_enabled_by_who/*kevin Mercury_V2-00430*/;
    UI32_T ifindex = msg->port;

    LOG("PSEC_MGR_ProcessStationMove, port(%lu)", ifindex);

    if (!SWCTRL_IsSecurityPort(ifindex,&port_security_enabled_by_who/*kevin Mercury_V2-00430*/))
    {
        LOG("This is not PSec enabled port");
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetIntrusionAction(ifindex, &action_status))
    {
        LOG("Failed to get intrustion action");
        return FALSE;
    }

    if (action_status == VAL_portSecAction_trap || action_status == VAL_portSecAction_trapAndShutdown)
    {
        if (SWCTRL_IsSecurityOperActionTrapPort(ifindex))
        {
            PSEC_MGR_SendIntrusionMacTrap(ifindex, msg->mac);

            /* Trap need to come down 60 seconds */
            SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus(ifindex, VAL_portSecAction_none);
            SWCTRL_PMGR_ResetSecurityActionTrapTimeStamp(ifindex);
        }
    }

    if (action_status == VAL_portSecAction_shutdown || action_status == VAL_portSecAction_trapAndShutdown)
    {
        SYS_CALLBACK_MGR_SetPortStatusCallback(SYS_MODULE_PSEC, ifindex, FALSE, SWCTRL_PORT_STATUS_SET_BY_PORTSEC);
    }

    return TRUE;

#else
    return FALSE;
#endif
} /* end of PSEC_MGR_ProcessIntrusionMac */

/* FUNCTION NAME - PSEC_MGR_HandleHotInsertion
 *
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is inserted at a time.
 */
void PSEC_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;

#endif
}



/* FUNCTION NAME - PSEC_MGR_HandleHotRemoval
 *
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 *
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 *
 * OUTPUT   : None
 *
 * RETURN   : None
 *
 * NOTE     : Only one module is removed at a time.
 */
void PSEC_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    /* LOCAL CONSTANT DECLARATIONS
    */

    /* LOCAL VARIABLES DECLARATIONS
    */

    /* BODY */
    return;

#endif
}

/* FUNCTION NAME: PSEC_MGR_NoShutdown_CallBack
 * PURPOSE: Notification port admin up with a port argument.
 * INPUT:   port
 * OUTPUT:  None
 * RETURN:  None
 * NOTES:
 */
void PSEC_MGR_NoShutdown_CallBack (UI32_T ifindex)
{
    UI32_T action_status;

    if(PSEC_MGR_GetPortSecurityActionStatus(ifindex, &action_status))
    {
        if((action_status == VAL_portSecAction_trap)||(action_status == VAL_portSecAction_trapAndShutdown))
            SWCTRL_PMGR_SetPortSecurityActionTrapOperStatus(ifindex, VAL_portSecAction_trap);
    }
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_SetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Set the port security learning status
 * INPUT   : ifindex                - interface index
 *           learning_status        - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * OUTPUT  : None
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_SetPortSecurityLearningStatus (UI32_T ifindex, UI32_T learning_status)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    AMTR_TYPE_AddrEntry_T addr_entry;

    SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (TRUE == PSEC_MGR_IsOtherCscEnabled(ifindex))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (learning_status == VAL_portSecLearningStatus_enabled)
    {
        /* 1. Delete Learned & Learned-PSEC */
        if (FALSE == AMTR_MGR_DeleteAddrBySourceAndLPort(ifindex,AMTR_TYPE_ADDRESS_SOURCE_LEARN))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        /* 2. Disable Port Security */
        if (FALSE == SWCTRL_PMGR_SetPortSecurityStatus (ifindex, VAL_portSecPortStatus_disabled, SWCTRL_PORT_SECURITY_ENABLED_BY_NONE))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        /* 3. Set Database */
        PSEC_MGR_LOCK();
        psec_learning_status[ifindex] = VAL_portSecLearningStatus_enabled;
        PSEC_MGR_UNLOCK();

    }
    else /* VAL_portSecLearningStatus_disabled */
    {
        /* 1. Enable Port Security */
        if (FALSE == SWCTRL_PMGR_SetPortSecurityStatus (ifindex, VAL_portSecPortStatus_enabled, SWCTRL_PORT_SECURITY_ENABLED_BY_PSEC))
        {
            SYSFUN_RELEASE_CSC();
            return FALSE;
        }

        /* 2. Set Database */
        PSEC_MGR_LOCK();
        psec_learning_status[ifindex] = VAL_portSecLearningStatus_disabled;
        PSEC_MGR_UNLOCK();

        /* 3. Change all dynamic address to static */
        memset (&addr_entry, 0, sizeof (AMTR_TYPE_AddrEntry_T));
        addr_entry.ifindex = ifindex;
        while (AMTR_MGR_GetNextIfIndexAddrEntry (&addr_entry,0))
        {
            if (!AMTR_MGR_SetAddrEntry(&addr_entry))
            {
                SYSFUN_RELEASE_CSC();
                return FALSE;
            }
        }
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;

#else
    return FALSE;
#endif
} /* End of PSEC_MGR_SetPortSecurityStatus () */

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_GetPortSecurityLearningStatus
 *------------------------------------------------------------------------
 * FUNCTION: This function will Get the port security learning status
 * INPUT   : ifindex                - interface index
 * OUTPUT  : learning_status        - VAL_portSecLearningStatus_enabled
 *                                    VAL_portSecLearningStatus_disabled
 * RETURN  : TRUE/FALSE
 * NOTE    :
 *------------------------------------------------------------------------*/
BOOL_T PSEC_MGR_GetPortSecurityLearningStatus (UI32_T ifindex, UI32_T *learning_status)
{
#if (SYS_CPNT_PORT_SECURITY == TRUE)&& (SYS_CPNT_NETWORK_ACCESS == FALSE)

    SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    PSEC_MGR_LOCK();
    *learning_status = psec_learning_status[ifindex];
    PSEC_MGR_UNLOCK();

    SYSFUN_RELEASE_CSC();
    return TRUE;

#else
    return FALSE;
#endif
}

/* Function - PSEC_MGR_GetPortSecurityMacCount
 * Purpose  - This function will get port auto learning mac counts
 * Input    - ifindex        -- which port to
 * Output   - mac_count      -- mac learning count
 * Return  : TRUE: Successfully, FALSE: If not available
 * Note     -
 */
BOOL_T PSEC_MGR_GetPortSecurityMacCount( UI32_T ifindex, UI32_T * mac_count )
{
    SYSFUN_USE_CSC(FALSE);

    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    if (FALSE == PSEC_OM_GetMaxMacCount(ifindex, mac_count))
    {
        SYSFUN_RELEASE_CSC();
        return FALSE;
    }

    SYSFUN_RELEASE_CSC();
    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - PSEC_MGR_GetMinNbrOfMaxMacCount
 * ---------------------------------------------------------------------
 * PURPOSE  : Get the minimum value that max-mac-count can be set to.
 * INPUT    : none
 * OUTPUT   : min_number
 * RETURN   : none
 * NOTES    : for 3com CLI & WEB
 * ---------------------------------------------------------------------
 */
void PSEC_MGR_GetMinNbrOfMaxMacCount(UI32_T *min_number)
{
#define PSEC_MGR_MIN_NBR_OF_MAX_MAC_COUNT   0;

    if (NULL != min_number)
        *min_number = PSEC_MGR_MIN_NBR_OF_MAX_MAC_COUNT;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_ConvertSecuredAddressIntoManual
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
BOOL_T
PSEC_MGR_ConvertSecuredAddressIntoManual(
    UI32_T ifindex)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    if ((PSEC_MGR_INTERFACE_INDEX_FOR_ALL != ifindex) &&
        (TRUE == PSEC_MGR_IsInvalidPort(ifindex)))
    {
        LOG("Invalid port(%lu)", ifindex);
        return FALSE;
    }

    if (TRUE != PSEC_ENG_ConvertSecuredAddressIntoManual(ifindex))
    {
        LOG("Failed to set PSEC_ENG(%lu)", ifindex);
        return FALSE;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for psec_mgr.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *-------------------------------------------------------------------------
 */
BOOL_T PSEC_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    UI32_T  cmd;

    if(ipcmsg_p == NULL)
        return FALSE;

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_TRANSITION_MODE)
    {
        PSEC_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    switch((cmd = PSEC_MGR_MSG_CMD(ipcmsg_p)))
    {
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_STATUS:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityStatus(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_ACTIVE:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityActionActive(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_MAC:
        {
            PSEC_MGR_IPCMsg_GetMac_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityLastIntrusionMac(data_p->ifindex, data_p->mac);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMac_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LAST_INTRUSION_TIME:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityLastIntrusionTime(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ACTION_STATUS:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityActionStatus(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_STATUS:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetRunningPortSecurityStatus(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_ACTION_STATUS:
        {
            PSEC_MGR_IPCMsg_GetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetRunningPortSecurityActionStatus(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT:
        {
            PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityMacCount(data_p->ifindex, data_p->mac_count);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_MAC_COUNT_OPERATION:
        {
            PSEC_MGR_IPCMsg_SetPortSecurityMacCount_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityMacCount(data_p->ifindex, data_p->mac_count);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_RUNNING_PORT_SECURITY_MAC_COUNT:
        {
            PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetRunningPortSecurityMacCount(data_p->ifindex, &data_p->mac_count);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_ENTRY:
        {
            PSEC_MGR_IPCMsg_PortSecurityEntry_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityEntry(&data_p->portsec_entry);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_NEXT_PORT_SECURITY_ENTRY:
        {
            PSEC_MGR_IPCMsg_PortSecurityEntry_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetNextPortSecurityEntry(&data_p->portsec_entry);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_PortSecurityEntry_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityStatus(data_p->ifindex, data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_ACTIVE:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityActionActive(data_p->ifindex, data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_STATUS_OPERATION:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityStatusOperation(data_p->ifindex, data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_ACTION_STATUS:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityActionStatus(data_p->ifindex, data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_SET_PORT_SECURITY_LEARNING_STATUS:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_SetPortSecurityLearningStatus(data_p->ifindex, data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_LEARNING_STATUS:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityLearningStatus(data_p->ifindex, &data_p->status);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_SetStatus_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_PORT_SECURITY_MAC_COUNT:
        {
            PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_GetPortSecurityMacCount(data_p->ifindex, &data_p->mac_count);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetPortSecurityMacCount_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_GET_MIN_NBR_OF_MAX_MAC_COUNT:
        {
            PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_GetMinNbrOfMaxMacCount(&data_p->min_number);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = TRUE;
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE(PSEC_MGR_IPCMsg_GetMinNbrOfMaxMacCount_T);
            break;
        }
        case PSEC_MGR_IPC_CMD_CONVERT_SECURED_ADDRESS_INTO_MANUAL:
        {
            PSEC_MGR_IPCMsg_SetStatus_T *data_p = PSEC_MGR_MSG_DATA(ipcmsg_p);
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = PSEC_MGR_ConvertSecuredAddressIntoManual(data_p->ifindex);
            ipcmsg_p->msg_size = PSEC_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        default:
            PSEC_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
            SYSFUN_Debug_Printf("*** %s(): Invalid cmd.\n", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    if (PSEC_MGR_MSG_RETVAL(ipcmsg_p) == FALSE)
    {
        SYSFUN_Debug_Printf("*** %s(): [cmd: %ld] failed\n", __FUNCTION__, cmd);
    }

    return TRUE;
} /* PSEC_MGR_HandleIPCReqMsg */

/* LOCAL FUNCTION DEFINICTION
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - PSEC_MGR_SendIntrusionMacTrap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This funtion is used to send intrusion mac trap
 * INPUT    :   port    --  port number
 * OUTPUT   :   None
 * RETURN   :   NONE
 * NOTES    :
 * ------------------------------------------------------------------------
 */
static void PSEC_MGR_SendIntrusionMacTrap(UI32_T lport, UI8_T *mac_p)
{
//#if(SYS_CPNT_TRAPMGMT == TRUE)
    TRAP_EVENT_TrapData_T   trap_data;

    /* Trap for intrusion mac address */
    trap_data.trap_type = TRAP_EVENT_PORT_SECURITY_TRAP;
    trap_data.community_specified = FALSE;
    trap_data.flag = TRAP_EVENT_SEND_TRAP_OPTION_DEFAULT;
    trap_data.u.port_security_trap.instance_ifindex = lport;
    trap_data.u.port_security_trap.ifindex = lport;

    memcpy(trap_data.u.port_security_trap.mac, mac_p,
        sizeof(trap_data.u.port_security_trap.mac));

    /* send trap to all community */
    SNMP_PMGR_ReqSendTrap(&trap_data);
//#endif
    return;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_IsDot1xEnabled
 *------------------------------------------------------------------------
 * FUNCTION: Check whether the port is DOT1X enabled port
 * INPUT   : ifindex    - ifindex
 * OUTPUT  : None
 * RETURN  : TRUE - the ifindex is DOT1X port; FALSE - else
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_MGR_IsDot1xEnabled(UI32_T ifindex)
{
#if (SYS_CPNT_DOT1X == TRUE)
    UI32_T port_control = DOT1X_OM_Get_PortControlMode(ifindex);

    if (VAL_dot1xPaeSystemAuthControl_enabled == DOT1X_OM_Get_SystemAuthControl()
        && ((VAL_dot1xAuthAuthControlledPortControl_auto == port_control)
         || (VAL_dot1xAuthAuthControlledPortControl_forceUnauthorized == port_control)))
    {
        return TRUE;
    }
#endif /* #if (SYS_CPNT_DOT1X == TRUE) */

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_IsRSPANEnabled
 *------------------------------------------------------------------------
 * FUNCTION: Check whether the port is RSPAN enabled port
 * INPUT   : ifindex    - ifindex
 * OUTPUT  : None
 * RETURN  : TRUE - the ifindex is RSPAN up link port; FALSE - else
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_MGR_IsRSPANEnabled(UI32_T ifindex)
{
#if (SYS_CPNT_RSPAN == TRUE)
    return RSPAN_OM_IsRspanUplinkPort(ifindex);
#else
    return FALSE;
#endif
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_IsOtherCscEnabled
 *------------------------------------------------------------------------
 * FUNCTION: Check whether the port is enabled by other mutual csc
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE - the ifindex is enable by other mutual csc; FALSE - else
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_MGR_IsOtherCscEnabled(UI32_T ifindex)
{
    if (TRUE == PSEC_MGR_IsDot1xEnabled(ifindex))
    {
        LOG("DOT1X enabled port");
        return TRUE;
    }

    if (TRUE == PSEC_MGR_IsRSPANEnabled(ifindex))
    {
        LOG("RSPEN enabled port");
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------
 * ROUTINE NAME - PSEC_MGR_IsOtherCscEnabled
 *------------------------------------------------------------------------
 * FUNCTION: Check arguments are all valid or not before to set port security status
 * INPUT   : None
 * OUTPUT  : None
 * RETURN  : TRUE - the argumets are all valid; FALSE - else
 * NOTE    : None
 *------------------------------------------------------------------------
 */
static BOOL_T PSEC_MGR_IsPortSecurityStatusOperationValid(UI32_T ifindex, UI32_T portsec_status)
{
    if (   (VAL_portSecPortStatus_enabled != portsec_status)
        && (VAL_portSecPortStatus_disabled != portsec_status)
        )
    {
        LOG("Invalid port security status(%lu)", portsec_status);
        return FALSE;
    }

    if (PSEC_MGR_IsInvalidPort(ifindex))
    {
        LOG("Invalid port(%lu)", ifindex);
        return FALSE;
    }

    if (TRUE == PSEC_MGR_IsOtherCscEnabled(ifindex))
    {
        return FALSE;
    }

    return TRUE;
}

#endif /* (SYS_CPNT_PORT_SECURITY == TRUE) */
