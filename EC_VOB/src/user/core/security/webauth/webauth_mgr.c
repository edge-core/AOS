/* MODULE NAME:  WEBAUTH_MGR.C
 * PURPOSE:
 *   Initialize the resource and provide some functions for the WebAuth module.
 *
 * NOTES:
 *
 * HISTORY:
 *    01/29/07 --  Rich Lee, Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sysfun.h"
#include "sys_type.h"
#include "sys_module.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "l_stdlib.h"
#include "swctrl_pom.h"
#include "radius_pmgr.h"
#include "l4_pmgr.h"
#include "iml_pmgr.h"
#include "webauth_type.h"
#include "webauth_mgr.h"
#include "webauth_om.h"
#include "userauth.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#define WEBAUTH_MGR_DEBUG_MSG(msg) \
    { \
        if (webauth_mgr_debug_flag) \
            printf(msg); \
    }

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static UI32_T WEBAUTH_MGR_ProcessSuccessTimer(void);
static UI32_T WEBAUTH_MGR_ProcessBlackTimer(void);
static BOOL_T WEBAUTH_MGR_SetRuleByLport(UI32_T lport);
static BOOL_T WEBAUTH_MGR_ClearRuleByLport(UI32_T lport);

/* STATIC VARIABLE DEFINITIONS
 */

static UI32_T webauth_mgr_debug_flag = 0;
static BOOL_T webauth_is_provision_complete = FALSE;

/*  declare variables used for Transition mode  */
SYSFUN_DECLARE_CSC

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:  WEBAUTH_MGR_EnterMasterMode
 * PURPOSE: Enable webauth operation while in master mode.
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_MGR_EnterMasterMode(void)
{
    WEBAUTH_TYPE_System_Info_T         *system_info_p;
    UI32_T  i;

    system_info_p = WEBAUTH_OM_GetSystemInfoPointer();

    /* default value */
    system_info_p->status = SYS_DFLT_WEBAUTH_STATUS;
    system_info_p->session_timeout = SYS_DFLT_WEBAUTH_SESSION_TIMEOUT;
    system_info_p->max_login_attempt = SYS_DFLT_WEBAUTH_MAX_LOGIN_ATTEMPTS;
    system_info_p->quiet_period = SYS_DFLT_WEBAUTH_QUIET_PERIOD;

    for(i=1; i<=SYS_ADPT_TOTAL_NBR_OF_LPORT; i++)
        WEBAUTH_OM_InitArrayByLPort(i);

    SYSFUN_ENTER_MASTER_MODE();

} /* End of WEBAUTH_MGR_EnterMasterMode */

/* FUNCTION NAME - WEBAUTH_MGR_Initiate_System_Resources
 * PURPOSE: This function will clear OM of WEBAUTH
 * INPUT:   none
 * OUTPUT:  none
 * RETURN:  none
 * NOTES:   none
 */
void WEBAUTH_MGR_Initiate_System_Resources(void)
{
    WEBAUTH_OM_InitWebAuthOm();

#if 0
    /* Semaphore for critical section, for one sema so give 1*/
    if (SYSFUN_CreateSem(1, SYSFUN_SEM_FIFO, &webauth_mgr_sem_id) != SYSFUN_OK)
    {
        WEBAUTH_MGR_DEBUG_MSG("\r\nCreate webauth_mgr_sem_id failed.");
    }

    /* register port link down call back function */
    SWCTRL_Register_UPortLinkDown_CallBack(WEBAUTH_MGR_PortLinkDown_CallBack);
#endif /* #if 0 */
}

/* FUNCTION NAME:  WEBAUTH_MGR_EnterTransitionMode
 * PURPOSE: This function forces this subsystem enter the Transition
 *          Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_MGR_EnterTransitionMode(void)
{
    /* set mgr in transition mode */
    SYSFUN_ENTER_TRANSITION_MODE();
    WEBAUTH_OM_InitWebAuthOm();
} /*End of WEBAUTH_MGR_EnterTransitionMode */

/* FUNCTION NAME:  WEBAUTH_MGR_EnterSlaveMode
 * PURPOSE: This function forces this subsystem enter the Slave Operation mode.
 * INPUT:   none.
 * OUTPUT:  none.
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_MGR_EnterSlaveMode(void)
{
    SYSFUN_ENTER_SLAVE_MODE();
} /* End of WEBAUTH_MGR_EnterSlaveMode */

/* FUNCTION NAME : WEBAUTH_MGR_SetTransitionMode
 * PURPOSE: Set transition mode.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 */
void WEBAUTH_MGR_SetTransitionMode(void)
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
} /* End of WEBAUTH_MGR_SetTransitionMode */

/* FUNCTION NAME:  WEBAUTH_MGR_ProvisionComplete
 * PURPOSE: This function will tell webauth that provision is completed
 * INPUT:   notify_flag
 * OUTPUT:  None
 * RETURN:  None
 * NOTE:    None
 */
void  WEBAUTH_MGR_ProvisionComplete(BOOL_T notify_flag)
{
    webauth_is_provision_complete = notify_flag;
} /* End of WEBAUTH_MGR_ProvisionComplete */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleHotInsertion
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will initialize the port OM of the module ports
 *            when the option module is inserted.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     inserted
 *            number_of_port        -- the number of ports on the inserted
 *                                     module
 *            use_default           -- the flag indicating the default
 *                                     configuration is used without further
 *                                     provision applied; TRUE if a new module
 *                                     different from the original one is
 *                                     inserted
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is inserted at a time.
 * ------------------------------------------------------------------------
 */
void WEBAUTH_MGR_HandleHotInsertion(UI32_T starting_port_ifindex, UI32_T number_of_port, BOOL_T use_default)
{
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleHotRemoval
 * ------------------------------------------------------------------------
 * PURPOSE  : This function will clear the port OM of the module ports when
 *            the option module is removed.
 * INPUT    : starting_port_ifindex -- the ifindex of the first module port
 *                                     removed
 *            number_of_port        -- the number of ports on the removed
 *                                     module
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Only one module is removed at a time.
 * ------------------------------------------------------------------------
 */
void WEBAUTH_MGR_HandleHotRemoval(UI32_T starting_port_ifindex, UI32_T number_of_port)
{
    UI32_T i;

    for (i=0; i<number_of_port; i++)
    {
        WEBAUTH_OM_InitArrayByLPort(starting_port_ifindex+i);
    }
}

/* FUNCTION NAME: WEBAUTH_MGR_SetSystemStatus
 * PURPOSE: This function will set global status of webauth
 * INPUT:   status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_SetSystemStatus(UI8_T status)
{
#if (SYS_CPNT_WEBAUTH == TRUE)

    WEBAUTH_TYPE_System_Info_T *system_info_p;
    UI32_T lport;
    BOOL_T rule_ret = TRUE;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    if((status != VAL_webauthEnable_enabled) && (status != VAL_webauthEnable_disabled))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    system_info_p = WEBAUTH_OM_GetSystemInfoPointer();

    if (status == WEBAUTH_OM_GetSystemStatus())
    {
        return WEBAUTH_TYPE_RETURN_OK;
    }

    system_info_p->status = status;

    if (VAL_webauthEnable_enabled == status)
    {
        for (lport = 1; lport <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; lport++)
        {
            WEBAUTH_TYPE_Port_Info_T *lport_info_p;
            UI32_T unit, port, trunk_id;

            if(SWCTRL_LPORT_NORMAL_PORT !=
                SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
            {
                continue;
            }

            lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

            if (VAL_webAuthPortConfigStatus_enabled == lport_info_p->status)
            {
                IML_PMGR_SetWebauthStatus(lport, TRUE);

                rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
                rule_ret &= WEBAUTH_MGR_SetRuleByLport(lport);
            }
        }
    }
    else
    {
        for (lport = 1; lport <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; lport++)
        {
            UI32_T unit, port, trunk_id;

            if(SWCTRL_LPORT_NORMAL_PORT !=
                SWCTRL_POM_LogicalPortToUserPort(lport, &unit, &port, &trunk_id))
            {
                continue;
            }

            IML_PMGR_SetWebauthStatus(lport, FALSE);
            rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
        }

        for (lport =1; lport <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; lport++)
        {
            WEBAUTH_TYPE_Port_Info_T *lport_info_p;
            UI8_T status;

            lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);
            status = lport_info_p->status;

            WEBAUTH_OM_InitArrayByLPort(lport);

            lport_info_p->status = status;
        }
    }

    if (FALSE == rule_ret)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_MGR_SetSystemStatus */

/* FUNCTION NAME: WEBAUTH_MGR_GetSystemStatus
 * PURPOSE: This function will return the status of webath
 * INPUT:   *status
 * OUTPUT:  VAL_webauthEnable_enabled/VAL_webauthEnable_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSystemStatus(UI8_T *status)
{
    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    *status = WEBAUTH_OM_GetSystemStatus();

    return WEBAUTH_TYPE_RETURN_OK;

} /* End of WEBAUTH_MGR_GetSystemStatus */

/* FUNCTION NAME: WEBAUTH_MGR_GetOperationMode
 * PURPOSE: Get current webauth operation mode (master / slave / transition).
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  SYS_TYPE_Stacking_Mode_T - opmode.
 * NOTES:   None.
 */
SYS_TYPE_Stacking_Mode_T WEBAUTH_MGR_GetOperationMode(void)
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

/* FUNCTION NAME: WEBAUTH_MGR_SetStatusByLPort
 * PURPOSE: this function will set webauth per port status
 * INPUT:   lport
 *          status
 * OUTPUT:  VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    none
 */
UI32_T WEBAUTH_MGR_SetStatusByLPort(UI32_T lport, UI8_T status)
{
#if (SYS_CPNT_WEBAUTH == TRUE)

    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    UI8_T g_status;
    BOOL_T rule_ret = TRUE;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) ||
        (SWCTRL_POM_LogicalPortExisting(lport)!= TRUE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    if((status != VAL_webAuthPortConfigStatus_enabled) &&
        (status != VAL_webAuthPortConfigStatus_disabled))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    if (status == lport_info_p->status)
    {
        return WEBAUTH_TYPE_RETURN_OK;
    }

    g_status = WEBAUTH_OM_GetSystemStatus();
    if (VAL_webauthEnable_enabled == g_status)
    {
        if (VAL_webAuthPortConfigStatus_enabled == status)
        {
            IML_PMGR_SetWebauthStatus(lport, TRUE);

            rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
            rule_ret &= WEBAUTH_MGR_SetRuleByLport(lport);
        }
        else
        {
            IML_PMGR_SetWebauthStatus(lport, FALSE);
            rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
        }
    }

    if (VAL_webAuthPortConfigStatus_enabled == status)
    {
        lport_info_p->status = VAL_webAuthPortConfigStatus_enabled;
    }
    else
    {
        memset(lport_info_p, 0, sizeof(WEBAUTH_TYPE_Port_Info_T));
        lport_info_p->status = VAL_webAuthPortConfigStatus_disabled;
    }

    if (FALSE == rule_ret)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

#endif /* #if (SYS_CPNT_WEBAUTH == TRUE) */

    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetStatusByLPort
 * PURPOSE: This function will return webauth per port status
 * INPUT:   lport
 * OUTPUT:  *status -- VAL_webAuthPortConfigStatus_enabled/VAL_webAuthPortConfigStatus_disabled
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetStatusByLPort(UI32_T lport, UI8_T *status)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    *status = lport_info_p->status ;

    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetPortInfoByLPort
 * PURPOSE: This function will get port info by lport
 * INPUT:   lport
 * OUTPUT:  *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetPortInfoByLPort(UI32_T lport, WEBAUTH_TYPE_Port_Info_T *lport_info_p)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p_tmp;
    UI32_T ret = WEBAUTH_TYPE_RETURN_ERROR;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ret;
    }

    lport_info_p_tmp = WEBAUTH_OM_GetLPortInfoPTR(lport);

    if(lport_info_p_tmp != NULL)
    {
        memcpy(lport_info_p, lport_info_p_tmp, sizeof(WEBAUTH_TYPE_Port_Info_T));
        ret = WEBAUTH_TYPE_RETURN_OK;
    }

    return ret;
} /* End of WEBAUTH_MGR_GetPortInfoByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetNextPortInfoByLPort
 * PURPOSE: this function will get next port info
 * INPUT:   *index
 * OUTPUT:  *index
 *          *lport_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    key is index; index=0: get initial
 */
UI32_T WEBAUTH_MGR_GetNextPortInfoByLPort(UI32_T *index, WEBAUTH_TYPE_Port_Info_T *lport_info_p)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p_tmp;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    {
        UI32_T  unit, port, trk_id, ret = WEBAUTH_TYPE_RETURN_ERROR;

        while (++(*index) <= SYS_ADPT_TOTAL_NBR_OF_LPORT)
        {
            switch (SWCTRL_POM_LogicalPortToUserPort(*index, &unit, &port, &trk_id))
            {
            /* not support trunk now
             */
            case SWCTRL_LPORT_TRUNK_PORT_MEMBER:
            case SWCTRL_LPORT_NORMAL_PORT:
                ret = WEBAUTH_TYPE_RETURN_OK;
                break;

            default:
                continue;
            }

            break;
        }

        if (WEBAUTH_TYPE_RETURN_OK != ret)
            return ret;
    }

    lport_info_p_tmp = WEBAUTH_OM_GetLPortInfoPTR(*index);

    memcpy(lport_info_p, lport_info_p_tmp, sizeof(WEBAUTH_TYPE_Port_Info_T));

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_MGR_GetNextPortInfoByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_ProcessTimerEvent
 * PURPOSE: this function will process timer event
 * INPUT:   none
 * OUTPUT:  None
 * RETURN:  none.
 * NOTE:    process two list
 *          1.success: check from head, if expire
 *                     delete: host,list,rule
 *          2.black  : check from head, if expire
 *                     delete: host,list,rule
 */
void WEBAUTH_MGR_ProcessTimerEvent(void)
{
    if (   (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
        || (VAL_webauthEnable_disabled == WEBAUTH_OM_GetSystemStatus()))
    {
        return;
    }

    WEBAUTH_MGR_ProcessSuccessTimer();
    WEBAUTH_MGR_ProcessBlackTimer();
}/* End of WEBAUTH_MGR_ProcessTimerEvent */

/* FUNCTION NAME: WEBAUTH_MGR_SetQuietPeriod
 * PURPOSE: this function will set quiet period
 * INPUT:   quiet_period
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetQuietPeriod(UI16_T quiet_period)
{
    UI32_T ret;

    if(quiet_period<SYS_ADPT_WEBAUTH_MIN_QUIET_PERIOD ||
       quiet_period > SYS_ADPT_WEBAUTH_MAX_QUIET_PERIOD ||
      (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_SetQuietPeriod(quiet_period);

    return ret;
} /* End of WEBAUTH_MGR_SetQuietPeriod */

/* FUNCTION NAME: WEBAUTH_MGR_GetQuietPeriod
 * PURPOSE: This function will return the quiet period value
 * INPUT:   none.
 * OUTPUT:  *quiet_period
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetQuietPeriod(UI16_T *quiet_period)
{
    UI32_T  ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_GetQuietPeriod(quiet_period);

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessCountByLPort
 * PURPOSE: This function will get success count by lport
 * INPUT:   lport
 * OUTPUT:  *success_count_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSuccessCountByLPort(UI32_T lport, UI16_T *success_count_p)
{
    WEBAUTH_TYPE_Port_Info_T *port_info_p;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    /* lport_info_p = WEBAUTH_OM_GetLPortInfoPTR();*/
    port_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);
    *success_count_p = port_info_p->success_count;

    return WEBAUTH_TYPE_RETURN_OK;
}

/* FUNCTION NAME: WEBAUTH_MGR_SetSystemSessionTimeout
 * PURPOSE: This function will set global session timeout value
 * INPUT:   session_timeout
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetSystemSessionTimeout(UI16_T session_timeout)
{
    UI32_T ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    if( session_timeout < MIN_webAuthSessionTimeout ||
        session_timeout > MAX_webAuthSessionTimeout)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_SetSessionTimeout(session_timeout);

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetSystemSessionTimeout
 * PURPOSE: This function will get global session timeout value
 * INPUT:   none.
 * OUTPUT:  *session_timeout
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSystemSessionTimeout(UI16_T *session_timeout)
{
    UI32_T  ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_GetSessionTimeout(session_timeout);

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_SetMaxLoginAttempts
 * PURPOSE: this function will set max login attempts
 * INPUT:   max_login_attempt
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:    None
 */
UI32_T WEBAUTH_MGR_SetMaxLoginAttempts(UI8_T max_login_attempt)
{
    UI32_T ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    if(max_login_attempt < MIN_webAuthLoginAttempts ||
       max_login_attempt > MAX_webAuthLoginAttempts)
    {
         return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_SetMaxLoginAttempts(max_login_attempt);

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetMaxLoginAttempts
 * PURPOSE: This function will return global value of max login attempt
 * INPUT:   none
 * OUTPUT:  *max login attempt
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetMaxLoginAttempts(UI8_T *max_login_attempt)
{
    UI32_T  ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_GetMaxLoginAttempts(max_login_attempt);

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_SetExternalURL
 * PURPOSE: This function set external URL by it's type
 * INPUT:   *url,
 *          url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_SetExternalURL(char *url, WEBAUTH_TYPE_EXTERNAL_URL_T url_type)
{
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR;

    /* check if master mode and other constraints */
    if(WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE||
      (strlen(url)>WEBAUTH_TYPE_MAX_URL_LENGTH)  ||
      strncmp(url, "http://", strlen("http://"))!=0         ||
      strncmp(url, "HTTP://", strlen("HTTP://"))!=0)
    {
        return  WEBAUTH_TYPE_RETURN_ERROR;
    }


    switch(url_type)
    {
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN:
            ret = WEBAUTH_OM_SetExternalLoginURL(url);
            break;
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_FAIL:
            ret = WEBAUTH_OM_SetExternalLoginFailURL(url);
            break;
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_SUCCESS:
            ret = WEBAUTH_OM_SetExternalLoginSuccessURL(url);
            break;
     }

    return ret;

} /* End of WEBAUTH_MGR_SetExternalURL */

/* FUNCTION NAME: WEBAUTH_MGR_GetExternalURL
 * PURPOSE: This function get external URL by it's type
 * INPUT:   url type:
 *             {WEBAUTH_EXTERNAL_URL_Login
 *              WEBAUTH_EXTERNAL_URL_Login_fail
 *              WEBAUTH_EXTERNAL_URL_Login_Success}
 * OUTPUT:  *url
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetExternalURL(
       char *url,
       WEBAUTH_TYPE_EXTERNAL_URL_T url_type)
{
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    switch(url_type)
    {
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN:
            ret = WEBAUTH_OM_GetExternalLoginURL(url);
            break;
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_FAIL:
            ret = WEBAUTH_OM_GetExternalLoginFailURL(url);
            break;
        case WEBAUTH_TYPE_EXTERNAL_URL_LOGIN_SUCCESS:
            ret = WEBAUTH_OM_GetExternalLoginSuccessURL(url);
            break;
     }

    return ret;
} /* End of WEBAUTH_MGR_GetExternalURL */

/* FUNCTION NAME: WEBAUTH_MGR_CheckIPIsBlackByLPort
 * PURPOSE: This function will check whether host is in black
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   if is black , return WEBAUTH_TYPE_RETURN_OK
 *          else return WEBAUTH_TYPE_RETURN_ERROR
 */
UI32_T WEBAUTH_MGR_CheckIPIsBlackByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T *port_info_p;
    UI16_T i, ret =WEBAUTH_TYPE_RETURN_ERROR ;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return ret;
    }

    /*  lport_info_p = WEBAUTH_OM_GetLPortInfoPTR();*/
    port_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if(port_info_p->black_entries_ar[i].ip == ip_addr)
        {
            ret = WEBAUTH_TYPE_RETURN_OK;
            break;
        }
    }

    return ret;

} /* End of WEBAUTH_MGR_CheckIPIsBlackByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetTryingHostCount
 * PURPOSE: This function will get trying host login attempt by ip
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  *login_attempt_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetTryingHostCount(
       UI32_T ip_addr, UI32_T lport,UI8_T *login_attempt_p )
{
    WEBAUTH_TYPE_Host_Trying_T    *trying_list_tmp_p;
    UI32_T ret = WEBAUTH_TYPE_RETURN_ERROR;
    UI16_T count = 0;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    trying_list_tmp_p = WEBAUTH_OM_GetTryingListHead();

    while(trying_list_tmp_p != NULL)
    {
        if(trying_list_tmp_p->ip == ip_addr)
        {
            *login_attempt_p = trying_list_tmp_p->login_attempt;
            ret = WEBAUTH_TYPE_RETURN_OK;
            break;
        }
        if(++count > WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT)
            break;

        trying_list_tmp_p = trying_list_tmp_p->next_host_p;
    }

    return ret;

} /* End of WEBAUTH_MGR_GetTryingHostCount */

/* FUNCTION NAME: WEBAUTH_MGR_CreateSuccessHostByLPort
 * PURPOSE: create success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   lport start from 1
 */
UI32_T WEBAUTH_MGR_CreateSuccessHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    UI32_T  ret, i;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (SWCTRL_POM_LogicalPortExisting(lport)!= TRUE) ||
        (lport == 0) || (ip_addr == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if(lport_info_p->success_entries_ar[i].ip == ip_addr)
        {
            return WEBAUTH_TYPE_RETURN_ERROR;
        }
    }

    ret = WEBAUTH_OM_CreateSuccessHostByLPort(ip_addr,lport);

    return ret;
} /* End of WEBAUTH_MGR_CreateSuccessHostByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_DeleteSuccessListByHostIP
 * PURPOSE: delete success host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteSuccessListByHostIP(UI32_T ip_addr, UI32_T lport)
{
    UI32_T  ret;

    /* check if master mode and port constraints*/
    if( (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) ||(lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_DeleteSuccessListHost(ip_addr, lport);

    return ret;

} /* End of WEBAUTH_MGR_DeleteSuccessListByHostIP */

/* FUNCTION NAME: WEBAUTH_MGR_DeleteBlackListByHostIP
 * PURPOSE: delete black host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteBlackListByHostIP(UI32_T ip_addr, UI32_T lport)
{
    UI32_T  ret;

    /* check if master mode and port constraints*/
    if( (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) ||(lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_DeleteBlackListHostByIP(ip_addr);

    return ret;

} /* End of WEBAUTH_MGR_DeleteSuccessListByHostIP */

/* FUNCTION NAME: WEBAUTH_MGR_DeleteTryingHost
 * PURPOSE: delete trying host data by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_DeleteTryingHost(UI32_T ip_addr,UI32_T lport)
{
    UI32_T  ret;

     /* check if master mode and port constraints*/
    if( (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) ||(lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_DeleteTryingHost(ip_addr, lport);

    return ret;
} /* End of WEBAUTH_MGR_DeleteTryingHost */

/* FUNCTION NAME: WEBAUTH_MGR_CreateBlackHostByLPort
 * PURPOSE: This function will create black hostby lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for each logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT, if full kill
 *          oldest and add to tail.
 */
UI32_T WEBAUTH_MGR_CreateBlackHostByLPort(UI32_T ip_addr, UI32_T lport)
{
    UI32_T  ret;

    /* check if master mode and port constraints*/
    if( (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (lport > SYS_ADPT_TOTAL_NBR_OF_LPORT) ||(lport == 0))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_CreateBlackHostByLPort(ip_addr,lport);

    return ret;

} /* End of WEBAUTH_MGR_CreateBlackHostByLPort */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningSystemStatus
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          system status with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *status
 * OUTPUT:  status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningSystemStatus(UI8_T *status)
{
    UI32_T  ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    *status = WEBAUTH_OM_GetSystemStatus();

    if(SYS_DFLT_WEBAUTH_STATUS == *status)
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        ret = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }
    return ret;
}/* End of WEBAUTH_MGR_GetRunningSystemStatus */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningSessionTimeout
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific session timeout with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *session_timeout
 * OUTPUT:  session timeout value .
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningSessionTimeout(
                           UI16_T *session_timeout)
{
    SYS_TYPE_Get_Running_Cfg_T  result;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetSessionTimeout(session_timeout);

    if (*session_timeout != SYS_DFLT_WEBAUTH_SESSION_TIMEOUT )
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
} /* End of WEBAUTH_MGR_GetRunningSessionTimeout */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningQuietPeriod
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          quiet period with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *quiet_period
 * OUTPUT:  status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningQuietPeriod(
                           UI16_T *quiet_period)
{
    SYS_TYPE_Get_Running_Cfg_T  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetQuietPeriod(quiet_period);

    if (*quiet_period != SYS_DFLT_WEBAUTH_QUIET_PERIOD )
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
} /* End of WEBAUTH_MGR_GetRunningQuietPeriod */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  none
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_MGR_GetRunningLoginURL(char *url)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetExternalLoginURL(url);

    if(*url=='\0')
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of WEBAUTH_MGR_GetRunningLoginURL */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginFailURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login fail URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  none
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T  WEBAUTH_MGR_GetRunningLoginFailURL(char *url)
{
    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetExternalLoginFailURL(url);

    if(*url=='\0')
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of WEBAUTH_MGR_GetRunningLoginFailURL */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningStatusByLPort
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          port status with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   lport
 *          *status
 * OUTPUT:  *status
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningStatusByLPort(
                           UI32_T lport, UI8_T *status)
{
    WEBAUTH_TYPE_Port_Info_T* port_info_p;


    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    port_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    *status = port_info_p->status;

    if (*status == SYS_DFLT_WEBAUTH_PORT_STATUS)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }
    else
    {
        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

} /* End of WEBAUTH_MGR_GetRunningStatusByLPort */


/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningLoginSuccessURL
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          login success URL with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 *          or SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *url
 * OUTPUT:  *url
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field for
 *             the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningLoginSuccessURL(char *url)
{

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetExternalLoginSuccessURL(url);

    if(*url=='\0')
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
} /* End of WEBAUTH_MGR_GetRunningLoginSuccessURL */

/* FUNCTION NAME:  WEBAUTH_MGR_GetRunningMaxLoginAttempt
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if the
 *          specific max login attempt with non-default values
 *          can be retrieved successfully. Otherwise,
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL or
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT:   *login_attempt
 * OUTPUT:  max login attempt value.
 * RETURN:  SYS_TYPE_GET_RUNNING_CFG_SUCCESS,
 *          SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *          SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES:   1. This function shall only be invoked by CLI to save the
 *             "running configuration" to local or remote files.
 *          2. Since only non-default configuration will be saved, this
 *             function shall return non-default structure for each field
 *             for the device.
 */
SYS_TYPE_Get_Running_Cfg_T WEBAUTH_MGR_GetRunningMaxLoginAttempt(
                           UI8_T *login_attempt)
{
    SYS_TYPE_Get_Running_Cfg_T  result;

    if (SYSFUN_GET_CSC_OPERATING_MODE() == SYS_TYPE_STACKING_SLAVE_MODE)
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    WEBAUTH_OM_GetMaxLoginAttempts(login_attempt);

    if (*login_attempt != SYS_DFLT_WEBAUTH_MAX_LOGIN_ATTEMPTS )
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;

    return result;
} /* End of WEBAUTH_MGR_GetRunningMaxLoginAttempt */


/* FUNCTION NAME: WEBAUTH_MGR_ProcessAuth
 * PURPOSE: This function do auth job
 * INPUT:   *username_p
 *          *paassword_p
 *          ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_HOST_Success;WEBAUTH_TYPE_RETURN_HOST_BLACK
 *          WEBAUTH_TYPE_RETURN_HOST_TRYING
 * NOTES:   1. for now auth is done by radius(username and pwd)
 *          2. auth success -> set rule (return WEBAUTH_TYPE_RETURN_OK)
 *                             remove from try list
 *          3. auth fail
 *              3.1 if new challenge -> add to try list
 *              3.2 if trying before, check trying count
 *                  3.2.1 if trying count < max -> add trying count++
 *                  3.2.2 trying count = max-1 -> add block list,
 *                        delete host in trying list
 */
UI32_T WEBAUTH_MGR_ProcessAuth(
        char *username_p, char *password_p, UI32_T ip_addr, UI32_T lport)
{
    I32_T radius_privilege;
    UI32_T ret = WEBAUTH_TYPE_RETURN_ERROR;

    if (OK_RC == RADIUS_PMGR_Auth_Check((UI8_T *)username_p, (UI8_T *)password_p, &radius_privilege, SYS_BLD_WEB_GROUP_IPCMSGQ_KEY))
    {
        BOOL_T rule_ret = TRUE;

        rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
        WEBAUTH_MGR_CreateSuccessHostByLPort(ip_addr,lport);
        WEBAUTH_MGR_DeleteTryingHost(ip_addr,lport);
        rule_ret &= WEBAUTH_MGR_SetRuleByLport(lport);

        if (TRUE == rule_ret)
        {
            ret = WEBAUTH_TYPE_RETURN_HOST_SUCCESS;
        }
    }
    else
    {
        /* create function will first check if exist then add 1 or delete when exceed max attempt */
        ret = WEBAUTH_MGR_CreateTryingHost(ip_addr,lport);

        if (WEBAUTH_TYPE_RETURN_HOST_BLACK == ret)
        {
            WEBAUTH_MGR_CreateBlackHostByLPort(ip_addr, lport);
        }
    }

    return ret;
} /* End of WEBAUTH_MGR_ProcessAuth */

/* FUNCTION NAME: WEBAUTH_MGR_ReAuthByLPort
 * PURPOSE: This function do reauth all host by lport
 * INPUT:   lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_ReAuthByLPort(UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    UI32_T ip_addr_delete, ret=WEBAUTH_TYPE_RETURN_OK;
    UI16_T i;
    BOOL_T rule_ret = TRUE;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) ||
        (lport >  SYS_ADPT_TOTAL_NBR_OF_LPORT) || (lport == 0) ||
        (SWCTRL_POM_LogicalPortExisting(lport)!= TRUE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    /* delete data */
    if((lport_info_p != NULL) || (lport_info_p->success_count != 0))
    {
        for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            if(lport_info_p->success_entries_ar[i].state != WEBAUTH_TYPE_HOST_STATE_SUCCESS)
                continue;
            ip_addr_delete = lport_info_p->success_entries_ar[i].ip;

            rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);

            /* delete success list for this host ip, and reset port data */
            ret &= WEBAUTH_OM_DeleteSuccessListHost(ip_addr_delete, lport);

            rule_ret &= WEBAUTH_MGR_SetRuleByLport(lport);
        }
    }

    if (FALSE == rule_ret)
    {
        ret = WEBAUTH_TYPE_RETURN_ERROR;
    }

    return ret;
} /* End of WEBAUTH_MGR_ReAuthByLPort */


/* FUNCTION NAME: WEBAUTH_MGR_ReAuthHostByLPort
 * PURPOSE: This function do reauth host by lport
 * INPUT:   lport
 *          ip_addr
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_ReAuthHostByLPort(UI32_T lport, UI32_T ip_addr)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p=NULL;
    UI32_T ip_addr_om, ret = WEBAUTH_TYPE_RETURN_OK;
    UI16_T i;
    BOOL_T rule_ret = TRUE;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) ||
        (lport >  SYS_ADPT_TOTAL_NBR_OF_LPORT) || (lport == 0) ||
        (SWCTRL_POM_LogicalPortExisting(lport)!= TRUE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    /* delete data */
    if((lport_info_p != NULL) || (lport_info_p->success_count != 0))
    {
        for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            ip_addr_om = lport_info_p->success_entries_ar[i].ip;
            if(ip_addr_om == ip_addr)
            {
                rule_ret &= WEBAUTH_MGR_ClearRuleByLport(lport);
                ret &= WEBAUTH_OM_DeleteSuccessListHost(ip_addr, lport);
                rule_ret &= WEBAUTH_MGR_SetRuleByLport(lport);
                break;
            }
        }
    }

    if (FALSE == rule_ret)
    {
        ret = WEBAUTH_TYPE_RETURN_ERROR;
    }

    return ret;
}/* End of WEBAUTH_MGR_ReAuthHostByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetWebAuthInfo
 * PURPOSE: This function will get webauth global status
 * INPUT:   none
 * OUTPUT:  *sys_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetWebAuthInfo(WEBAUTH_TYPE_System_Info_T *sys_info_p)
{
    WEBAUTH_TYPE_System_Info_T         *sys_info_om_p;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    sys_info_om_p = WEBAUTH_OM_GetSystemInfoPointer();

    memcpy(sys_info_p, sys_info_om_p, sizeof(WEBAUTH_TYPE_System_Info_T));

    return WEBAUTH_TYPE_RETURN_OK;
}/* End of WEBAUTH_MGR_GetWebAuthInfo */

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPort
 * PURPOSE: This function will get next success host by lport
 * INPUT:   lport    -- key 1
 *          *index_p -- key 2
 * OUTPUT:  host_info_p
 *          *index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPort(
       WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T *index_p)
{
    WEBAUTH_TYPE_Port_Info_T*  lport_info_p;
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR;
    UI16_T  i, get_count=0;


    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    if(lport_info_p->success_count == 0 || *index_p >=lport_info_p->success_count)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {

        if(lport_info_p->success_entries_ar[i].state != WEBAUTH_TYPE_HOST_STATE_INITIAL)
        {
            if(++get_count > *index_p )
            {
                memcpy(host_info_p, &lport_info_p->success_entries_ar[i], sizeof(WEBAUTH_TYPE_Host_Info_T));
                ret = WEBAUTH_TYPE_RETURN_OK;
                break;
            }
        }
    }
    (*index_p)++;

    return ret;
} /* End of WEBAUTH_MGR_GetNextSuccessHostByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPortAndIndex
 * PURPOSE: This function will get next success host by lport and index
 *          index means success count.
 * INPUT:   host_info_p->lport -- key 1
 *          host_info_p->ip    -- key 2
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 *          loop end: lport not exist, or got one
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p)
{
    WEBAUTH_TYPE_Port_Info_T  *lport_info_p;
    WEBAUTH_TYPE_Host_Info_T  *local_host_info_p = NULL;
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR, check_ip,got_big=0, got_ip=0, ip_addr, lport;
    UI16_T  i;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (host_info_p == NULL))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport = host_info_p->lport;
    if(lport == 0)
        lport = 1;

    ip_addr = host_info_p->ip;
    /* check current port */
    while(SWCTRL_POM_LogicalPortExisting(lport)== TRUE)
    {
        lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);
        for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            if(lport_info_p->success_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_SUCCESS)
            {
                check_ip = lport_info_p->success_entries_ar[i].ip;
                if(got_big == 0)
                {
                    if(memcmp(&check_ip,&ip_addr, sizeof(UI32_T)) >0 )
                    {
                        got_big = 1;
                        got_ip = check_ip;
                        local_host_info_p = &lport_info_p->success_entries_ar[i];
                    }
                }
                else
                {
                    if(memcmp(&got_ip, &check_ip, sizeof(UI32_T)) >0 && (memcmp(&check_ip, &ip_addr, sizeof(UI32_T))>0))
                    {
                        got_ip = check_ip;
                        local_host_info_p = &lport_info_p->success_entries_ar[i];
                    }
                }
            }
        }
        if(got_ip == 0)
        {
            ip_addr = 0; /* for next port , just got one bigger than zero */
            lport++;
        }
        else
        {
            break;
        }
    }

    if(got_ip != 0)
    {
        memcpy(host_info_p, local_host_info_p, sizeof(WEBAUTH_TYPE_Host_Info_T));
        ret = WEBAUTH_TYPE_RETURN_OK;
    }

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessHostByLPortAndIP
 * PURPOSE: This function will get success host by lport and ip
 * INPUT:   host_info_p->lport -- key 1
 *          host_info_p->ip    -- key 2
 * OUTPUT:  host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetSuccessHostByLPortAndIP(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p)
{
    WEBAUTH_TYPE_Port_Info_T  *lport_info_p;
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR,ip_addr, lport;
    UI16_T  i;

    /* check if master mode */
    if ((WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)||
        (host_info_p == NULL) ||
        (host_info_p->ip == 0) ||
        (host_info_p->lport == 0) ||
        (SWCTRL_POM_LogicalPortExisting(host_info_p->lport)!= TRUE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport = host_info_p->lport;
    ip_addr = host_info_p->ip;

    /* check current port */
    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if(lport_info_p->success_entries_ar[i].state == WEBAUTH_TYPE_HOST_STATE_SUCCESS)
        {
            if(lport_info_p->success_entries_ar[i].ip == ip_addr)
            {
                memcpy(host_info_p, &lport_info_p->success_entries_ar[i], sizeof(WEBAUTH_TYPE_Host_Info_T));
                ret = WEBAUTH_TYPE_RETURN_OK;
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPortAndIndex
 * PURPOSE: This function will get next success host by lport and index
 *          index means success count.
 * INPUT:   *lport_p
 *          *index_p
 * OUTPUT:  *host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 *          key: *lport, *index
 *          loop end: lport not exist, or got one
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPortAndIndex(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T *lport_p, UI8_T *index_p)
{
    WEBAUTH_TYPE_Port_Info_T*  lport_info_p;
    UI32_T  ret = WEBAUTH_TYPE_RETURN_ERROR;
    UI16_T  i, get_count=0;

    /* check if master mode */
    if( (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE) ||

        (SWCTRL_POM_LogicalPortExisting(*lport_p)!= TRUE))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    (*index_p)++;
    if(*lport_p == 0)/*if get port 0 means from port 1 as initial */
        *lport_p = 1;

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(*lport_p);

    if(lport_info_p->success_count == 0)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    if(*index_p<=(lport_info_p->success_count ))/* in the same port */
    {
        for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
        {
            if(lport_info_p->success_entries_ar[i].state != WEBAUTH_TYPE_HOST_STATE_INITIAL)
            {
                if(++get_count == *index_p )
                {
                    memcpy(host_info_p, &lport_info_p->success_entries_ar[i], sizeof(WEBAUTH_TYPE_Host_Info_T));
                    break;
                }
            }

        }
    }
    else /* jump to next succount host by port */
    {
        while(SWCTRL_POM_LogicalPortExisting((*lport_p)++)== TRUE)
        {
            lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(*lport_p);
            for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
            {
                if(lport_info_p->success_entries_ar[i].state != WEBAUTH_TYPE_HOST_STATE_INITIAL)
                {
                    memcpy(host_info_p, &lport_info_p->success_entries_ar[i], sizeof(WEBAUTH_TYPE_Host_Info_T));
                    ret = WEBAUTH_TYPE_RETURN_OK;
                    break;
                }
            }
            if(ret == WEBAUTH_TYPE_RETURN_OK)
                break;
        }
    }

    return ret;
} /* End of WEBAUTH_MGR_GetNextSuccessHostByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetSuccessHostByLPort
 * PURPOSE: This function will get success host by lport and index
 * INPUT:   lport
 *          index
 * OUTPUT:  *host_info_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for ecah logical port, it will have at most
 *          SYS_ADPT_MAX_NBR_OF_WEBAUTH_HOST_PER_PORT success hosts.
 */
UI32_T WEBAUTH_MGR_GetSuccessHostByLPort(
        WEBAUTH_TYPE_Host_Info_T  *host_info_p, UI32_T lport, UI8_T index)
{
    WEBAUTH_TYPE_Port_Info_T*  lport_info_p;
    UI16_T  i, get_count=0;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    if (   (NULL == lport_info_p)
        || (lport_info_p->success_count == 0)
        || (index >=lport_info_p->success_count))
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }
    for(i=0; i<SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if(lport_info_p->success_entries_ar[i].state != WEBAUTH_TYPE_HOST_STATE_INITIAL)
        {
            if(++get_count == index )
            {
                memcpy(host_info_p, &lport_info_p->success_entries_ar[i], sizeof(WEBAUTH_TYPE_Host_Info_T));
                break;
            }
        }
    }

    return WEBAUTH_TYPE_RETURN_OK;
} /* End of WEBAUTH_MGR_GetNextSuccessHostByLPort */

/* FUNCTION NAME: WEBAUTH_MGR_GetNextSuccessHostByLPortAndSuccIndex
 * PURPOSE: This function will get success host by lport and success index
 * INPUT:   lport_p, succ_index_p
 * OUTPUT:  host_info_p, lport_p, succ_index_p
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   none
 */
UI32_T WEBAUTH_MGR_GetNextSuccessHostByLPortAndSuccIndex(WEBAUTH_TYPE_Host_Info_T *host_info_p, UI32_T *lport_p, UI8_T *succ_index_p)
{
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    UI32_T ret = WEBAUTH_TYPE_RETURN_ERROR;
    UI32_T lport_index;
    UI32_T host_index;

    if (SYS_TYPE_STACKING_MASTER_MODE != WEBAUTH_MGR_GetOperationMode())
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    for (lport_index = *lport_p; lport_index <= SYS_ADPT_MAX_NBR_OF_PORT_PER_UNIT; lport_index++)
    {
        UI32_T current_succ_index = 0;

        lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport_index);
        if (   (NULL == lport_info_p)
            || (0 == lport_info_p->success_count))
        {
            continue;
        }

        for (host_index = 0; host_index < SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; host_index++)
        {
            if (lport_info_p->success_entries_ar[host_index].state != WEBAUTH_TYPE_HOST_STATE_SUCCESS)
            {
                continue;
            }

            current_succ_index++;

            if (   (lport_index == *lport_p)
                && (current_succ_index <= *succ_index_p))
            {
                continue;
            }

            *lport_p = lport_index;
            *succ_index_p = current_succ_index;
            memcpy(host_info_p, &lport_info_p->success_entries_ar[host_index], sizeof(WEBAUTH_TYPE_Host_Info_T));
            ret = WEBAUTH_TYPE_RETURN_OK;
            break;
        }

        if (WEBAUTH_TYPE_RETURN_OK == ret)
        {
            break;
        }
    }

    return ret;
}

/* FUNCTION NAME: WEBAUTH_MGR_CreateTryingHost
 * PURPOSE: This function will create trying host to trying list
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTES:   for trying list , it has length
 *          WEBAUTH_TYPE_MAX_TRYING_HOST_COUNT, if reach max count, delete
 *          oldest and add to tail of list.
 */
UI32_T WEBAUTH_MGR_CreateTryingHost(UI32_T ip_addr, UI32_T lport)
{
    UI32_T  ret;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return WEBAUTH_TYPE_RETURN_ERROR;
    }

    ret = WEBAUTH_OM_CreateTryingHostByLPort(ip_addr,lport);

    return ret;
} /* End of WEBAUTH_MGR_CreateTryingHost */

/* FUNCTION NAME: WEBAUTH_MGR_SetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   debug_flag
 * OUTPUT:  none
 * RETURN:  none.
 * NOTES:   none.
 */
void WEBAUTH_MGR_SetDebugFlag(UI32_T debug_flag)
{
    webauth_mgr_debug_flag = debug_flag;
}

/* FUNCTION NAME: WEBAUTH_MGR_GetDebugFlag
 * PURPOSE: This function will set debug flag
 * INPUT:   void
 * OUTPUT:  none
 * RETURN:  webauth_mgr_debug_flag
 * NOTES:   none.
 */
UI32_T WEBAUTH_MGR_GetDebugFlag(void)
{
    return webauth_mgr_debug_flag;
}

/* FUNCTION NAME: WEBAUTH_MGR_IsIPValidByLPort
 * PURPOSE: This function will check ip is valid by lport
 * INPUT:   ip_addr
 *          lport
 * OUTPUT:  none
 * RETURN:  TRUE: this ip is valid for this port
 *          FALSE: this ip is invalid for this port
 * NOTES:   none.
 */
BOOL_T WEBAUTH_MGR_IsIPValidByLPort(UI32_T ip_addr, UI32_T lport)
{
    WEBAUTH_TYPE_Port_Info_T    *lport_info_p;
    BOOL_T      ret=FALSE;
    UI32_T      entry_idx;

    /* check if master mode */
    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        return FALSE;
    }

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);

    /* if global disable, can't get port info, or port disabled,
     * return true
     */
    if((WEBAUTH_OM_GetSystemStatus() == VAL_webauthEnable_disabled)||
       (lport_info_p->status == VAL_webAuthPortConfigStatus_disabled))
    {
        return TRUE;
    }

    return WEBAUTH_OM_IsIPValidByLPort(ip_addr, lport);
}

/* FUNCTION NAME: WEBAUTH_MGR_PortLinkDown_CallBack
 * PURPOSE: this function will process port link down
 * INPUT:   unit
 *          port
 * OUTPUT:  none
 * RETURN:  none
 * NOTE:    when port link down, reset all hosts in this port
 */
void WEBAUTH_MGR_PortLinkDown_CallBack(UI32_T unit, UI32_T port)
{
    UI32_T lport;

    if (WEBAUTH_MGR_GetOperationMode() != SYS_TYPE_STACKING_MASTER_MODE )
    {
        return ;
    }

    if (SWCTRL_POM_UserPortToLogicalPort(unit, port, &lport) != SWCTRL_LPORT_NORMAL_PORT )
    {
	return ;
    }

    /* process success host */
    if(WEBAUTH_MGR_ReAuthByLPort( lport) != WEBAUTH_TYPE_RETURN_OK)
        WEBAUTH_MGR_DEBUG_MSG("\n port link down, but kill success error ");
}

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME: WEBAUTH_MGR_ProcessSuccessTimer
 * PURPOSE: this function will process success timer
 * INPUT:   none
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:
 */
static UI32_T WEBAUTH_MGR_ProcessSuccessTimer(void)
{
    WEBAUTH_TYPE_Host_Info_T *host_p;
    UI32_T ret = WEBAUTH_TYPE_RETURN_OK;
    BOOL_T rule_ret = TRUE;

    host_p = WEBAUTH_OM_GetSuccessListHead();

    while (host_p !=  NULL)
    {
        host_p->remaining_time--;

        if (host_p->remaining_time <= 0)
        {
            rule_ret &= WEBAUTH_MGR_ClearRuleByLport(host_p->lport);
            ret &= WEBAUTH_OM_DeleteSuccessListHost(host_p->ip, host_p->lport);
            rule_ret &= WEBAUTH_MGR_SetRuleByLport(host_p->lport);

            host_p = WEBAUTH_OM_GetSuccessListHead();
        }
        else
        {
            host_p = host_p->next_host_p;
        }
    }

    if (FALSE == rule_ret)
    {
        ret = WEBAUTH_TYPE_RETURN_ERROR;
    }

    return ret;
} /* End of WEBAUTH_MGR_ProcessSuccessTimer */

/* FUNCTION NAME: WEBAUTH_MGR_ProcessBlackTimer
 * PURPOSE: this function will process black timer
 * INPUT:   none
 * OUTPUT:  None
 * RETURN:  WEBAUTH_TYPE_RETURN_OK ; WEBAUTH_TYPE_RETURN_ERROR
 * NOTE:
 */
static UI32_T WEBAUTH_MGR_ProcessBlackTimer(void)
{
    WEBAUTH_TYPE_Host_Info_T *host_p;
    UI32_T ret = WEBAUTH_TYPE_RETURN_OK;

    host_p = WEBAUTH_OM_GetBlackListHead();

    while (host_p !=  NULL)
    {
        host_p->remaining_time--;

        if (host_p->remaining_time <= 0)
        {
            /* rule is not changed when the host is added to the black list, so
             * that the rule is unnecessary to change in here
             */
            ret &= WEBAUTH_OM_DeleteBlackListHost(host_p->ip, host_p->lport);

            host_p = WEBAUTH_OM_GetBlackListHead();
        }
        else
        {
            host_p = host_p->next_host_p;
        }
    }

    return ret;
} /* End of WEBAUTH_MGR_ProcessBlackTimer */

/*------------------------------------------------------------------------------
 * FUNCTION NAME - WEBAUTH_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
 * PURPOSE  : Handle the ipc request message for netaccess mgr.
 * INPUT    : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT   : ipcmsg_p  --  input request ipc message buffer
 * RETURN   : TRUE  - There is a response need to send.
 *            FALSE - There is no response to send.
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
BOOL_T WEBAUTH_MGR_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    UI32_T cmd;

    if (ipcmsg_p == NULL)
    {
        return FALSE;
    }

    /* Every ipc request will fail when operating mode is transition mode
     */
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE)
    {
        WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = FALSE;
        ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
        return TRUE;
    }

    cmd = WEBAUTH_MGR_MSG_CMD(ipcmsg_p);

    switch (cmd)
    {
        case WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetSystemStatus(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetSystemStatus(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetStatusByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetStatusByLPort(data_p->u32_1, &value);
            data_p->u32_2 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_QUIET_PERIOD:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetQuietPeriod(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_QUIET_PERIOD:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI16_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetQuietPeriod(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_MAX_LOGIN_ATTEMPTS:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetMaxLoginAttempts(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_MAX_LOGIN_ATTEMPTS:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetMaxLoginAttempts(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_SYSTEM_SESSION_TIMEOUT:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetSystemSessionTimeout(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_SYSTEM_SESSION_TIMEOUT:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI16_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetSystemSessionTimeout(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_EXTERNAL_URL:
        {
            WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_SetExternalURL(data_p->url, data_p->url_type);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_EXTERNAL_URL:
        {
            WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetExternalURL(data_p->url, data_p->url_type);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_REAUTH_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_ReAuthByLPort(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_REAUTH_HOST_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_ReAuthHostByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SESSION_TIMEOUT:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI16_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningSessionTimeout(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_QUIET_PERIOD:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI16_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningQuietPeriod(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_MAX_LOGIN_ATTEMPT:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningMaxLoginAttempt(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_SYSTEM_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningSystemStatus(&value);
            data_p->u32 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_STATUS:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningStatusByLPort(data_p->u32_1, &value);
            data_p->u32_2 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_URL:
        {
            WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningLoginURL(data_p->url);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_FAIL_URL:
        {
            WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningLoginFailURL(data_p->url);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_RUNNING_LOGIN_SUCCESS_URL:
        {
            WEBAUTH_MGR_IPCMsg_ExternelUrl_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetRunningLoginSuccessURL(data_p->url);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_ExternelUrl_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            value = data_p->u32_2;
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetNextSuccessHostByLPort(&data_p->host_info, data_p->u32_1, &value);
            data_p->u32_2 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_CREATE_SUCCESS_HOST_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_CreateSuccessHostByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_DELETE_SUCCESS_LIST_BY_HOST_IP:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_DeleteSuccessListByHostIP(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_CREATE_BLACK_HOST_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_CreateBlackHostByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_CHECK_IP_IS_BLACK_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_CheckIPIsBlackByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_CREATE_TRYING_HOST:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_CreateTryingHost(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_DELETE_BLACK_LIST_BY_HOST_IP:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_DeleteBlackListByHostIP(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_DELETE_TRYING_HOST:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_DeleteTryingHost(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_PORT_INFO_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_PortInfo_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetPortInfoByLPort(data_p->u32, &data_p->port_info);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GETNEXT_PORT_INFO_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_PortInfo_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetNextPortInfoByLPort(&data_p->u32, &data_p->port_info);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_PortInfo_U32_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT_AND_IP:
        {
            WEBAUTH_MGR_IPCMsg_HostInfo_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetSuccessHostByLPortAndIP(&data_p->host_info);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_PROCESS_AUTH:
        {
            WEBAUTH_MGR_IPCMsg_Username_Password_U32_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_ProcessAuth(data_p->username, data_p->password, data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_PROCESS_TIMER_EVENT:
        {
            WEBAUTH_MGR_ProcessTimerEvent();
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_IP:
        {
            WEBAUTH_MGR_IPCMsg_HostInfo_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetNextSuccessHostByLPortAndIP(&data_p->host_info);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_HOST_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetSuccessHostByLPort(&data_p->host_info, data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_SUCCESS_COUNT_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI16_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetSuccessCountByLPort(data_p->u32_1, &value);
            data_p->u32_2 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_TRYING_HOST_COUNT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetTryingHostCount(data_p->u32_1, data_p->u32_2, &value);
            data_p->u32_3 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_U32_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_SET_DEBUG_FLAG:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_SetDebugFlag(data_p->u32);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_DEBUG_FLAG:
        {
            WEBAUTH_MGR_IPCMsg_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            data_p->u32 = WEBAUTH_MGR_GetDebugFlag();
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_U32_Data_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GET_WEBAUTH_INFO:
        {
            WEBAUTH_MGR_IPCMsg_SystemInfo_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetWebAuthInfo(&data_p->system_info);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_SystemInfo_T);
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_IS_IP_VALID_BY_LPORT:
        {
            WEBAUTH_MGR_IPCMsg_U32_U32_Data_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_IsIPValidByLPort(data_p->u32_1, data_p->u32_2);
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }

        case WEBAUTH_MGR_IPC_CMD_GETNEXT_SUCCESS_HOST_BY_LPORT_AND_SUCC_INDEX:
        {
            WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T *data_p = WEBAUTH_MGR_MSG_DATA(ipcmsg_p);
            UI8_T value;

            value = data_p->u32_2;
            WEBAUTH_MGR_MSG_RETVAL(ipcmsg_p) = WEBAUTH_MGR_GetNextSuccessHostByLPortAndSuccIndex(&data_p->host_info, &data_p->u32_1, &value);
            data_p->u32_2 = value;
            ipcmsg_p->msg_size = WEBAUTH_MGR_GET_MSGBUFSIZE(WEBAUTH_MGR_IPCMsg_HostInfo_U32_U32_T);
            break;
        }

        default:
        {
            char buf[128];

            sprintf(buf, "%s(): Invalid cmd.\n", __FUNCTION__);
            WEBAUTH_MGR_DEBUG_MSG(buf);
            return FALSE;
        }
    }

    return TRUE;
}

static BOOL_T WEBAUTH_MGR_SetRuleByLport(UI32_T lport)
{
    UI32_T i;
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    BOOL_T ret = TRUE;

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);
    for (i = 0 ; i < SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if (WEBAUTH_TYPE_HOST_STATE_SUCCESS == lport_info_p->success_entries_ar[i].state)
        {
            ret &= L4_PMGR_SetPermitSrcIpPacketsByLport(lport, lport_info_p->success_entries_ar[i].ip, TRUE);
        }
    }

    ret &= L4_PMGR_SetRedirectHTTPClientPackets(lport, TRUE);
    ret &= L4_PMGR_SetDenyIpPacketsByLport(lport, TRUE);

    return ret;
}

static BOOL_T WEBAUTH_MGR_ClearRuleByLport(UI32_T lport)
{
    UI32_T i;
    WEBAUTH_TYPE_Port_Info_T *lport_info_p;
    BOOL_T ret = TRUE;

    lport_info_p = WEBAUTH_OM_GetLPortInfoPTR(lport);
    for (i = 0 ; i < SYS_ADPT_WEBAUTH_MAX_NBR_OF_HOSTS_PER_PORT; i++)
    {
        if (WEBAUTH_TYPE_HOST_STATE_SUCCESS == lport_info_p->success_entries_ar[i].state)
        {
            ret &= L4_PMGR_SetPermitSrcIpPacketsByLport(lport, lport_info_p->success_entries_ar[i].ip, FALSE);
        }
    }

    ret &= L4_PMGR_SetRedirectHTTPClientPackets(lport, FALSE);
    ret &= L4_PMGR_SetDenyIpPacketsByLport(lport, FALSE);

    return ret;
}
