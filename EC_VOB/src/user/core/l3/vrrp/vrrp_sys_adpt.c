/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_SYS_ADPT.c                                                                 
 *                                                                                      
 * PURPOSE: This package provides functions for providing default and range value
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009    

 * -------------------------------------------------------------------------------------*/

#include "vrrp_type.h"
#include "vrrp_sys_adpt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"


/* STATIC VARIABLE DECLARATIONS  
 */
#if 0
static UI32_T VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_GROUP_PER_SYSTEM = SYS_ADPT_MAX_NBR_OF_VRRP_GROUP;
static UI32_T VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_GROUP_PER_INTERFACE = 0;
static UI32_T VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP = SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP;
static UI32_T VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_ASSOC_IP = 0;
static UI32_T VRRP_SYS_ADPT_MAX_VRRP_ID = SYS_ADPT_MAX_VRRP_ID;
static UI32_T VRRP_SYS_ADPT_MIN_VRRP_ID = SYS_ADPT_MIN_VRRP_ID;
static UI32_T VRRP_SYS_ADPT_MAX_VRRP_PRIORITY = MAX_vrrpOperPriority;
static UI32_T VRRP_SYS_ADPT_MIN_VRRP_PRIORITY = MIN_vrrpOperPriority;
static UI32_T VRRP_SYS_ADPT_MAX_VRRP_ADVER_INTERVAL = SYS_ADPT_MAX_VRRP_ADVER_INTERVAL;
static UI32_T VRRP_SYS_ADPT_MIN_VRRP_ADVER_INTERVAL = SYS_ADPT_MIN_VRRP_ADVER_INTERVAL;
static UI32_T VRRP_SYS_ADPT_MAX_VRRP_PREEMPT_DELAY = SYS_ADPT_MAX_VRRP_PREEMPT_DELAY;
static UI32_T VRRP_SYS_ADPT_MIN_VRRP_PREEMPT_DELAY = SYS_ADPT_MIN_VRRP_PREEMPT_DELAY;
static UI32_T VRRP_SYS_ADPT_MAX_OPER_PROTOCOL = SYS_ADPT_MAX_VRRP_OPER_PROTOCOL;
static UI32_T VRRP_SYS_ADPT_MIN_OPER_PROTOCOL = SYS_ADPT_MIN_VRRP_OPER_PROTOCOL;

static UI32_T VRRP_DEFAULT_ADMIN_STATE = VAL_vrrpOperAdminState_down;
static UI32_T VRRP_DEFAULT_AUTH_LEN = SYS_ADPT_VRRP_AUTHENTICATION_KEY_LEN;
static UI8_T VRRP_DEFAULT_AUTH_TYPE = SYS_DFLT_VRRP_AUTH_TYPE;
static UI32_T VRRP_DEFAULT_PRIORITY = SYS_DFLT_VRRP_PRIORITY;
static BOOL_T VRRP_DEFAULT_PREEMPT_MODE = SYS_DFLT_VRRP_PREEMPT_MODE;
static UI32_T VRRP_DEFAUTL_PREEMPT_DELAY = SYS_DFLT_VRRP_PREEMPT_DELAY;
static UI32_T VRRP_DEFAULT_ADVER_INTERVAL = SYS_DFLT_VRRP_ADVER_INTERVAL;
static UI32_T VRRP_DEFAULT_PRIMARY_IP_ADDR = 0;
static UI32_T VRRP_DEFAULT_OPER_STATE = VAL_vrrpOperState_initialize;
static UI32_T VRRP_DEFAULT_OPER_PROTOCOL = VAL_vrrpOperProtocol_ip;
static UI32_T VRRP_DEFAULT_IP_ADDR_COUNT = 0;

/* EXPORTED SUBPROGRAM DECLARATION
 */

/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_Initiate_System_Resources
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function initialize vrrp sys_adpt.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------*/
void VRRP_SYS_ADPT_Initiate_System_Resources(void)
{
    VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_GROUP_PER_SYSTEM = 0;
    VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_GROUP_PER_INTERFACE = 0;
    VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP = 0;
    VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_ASSOC_IP = 0;
    VRRP_SYS_ADPT_MAX_VRRP_ID = 0;
    VRRP_SYS_ADPT_MIN_VRRP_ID = 0;
    VRRP_SYS_ADPT_MAX_VRRP_PRIORITY = 0;
    VRRP_SYS_ADPT_MIN_VRRP_PRIORITY = 0;
    VRRP_SYS_ADPT_MAX_VRRP_ADVER_INTERVAL = 0;
    VRRP_SYS_ADPT_MIN_VRRP_ADVER_INTERVAL = 0;
    VRRP_SYS_ADPT_MAX_VRRP_PREEMPT_DELAY = 0;
    VRRP_SYS_ADPT_MIN_VRRP_PREEMPT_DELAY = 0;
    VRRP_SYS_ADPT_MAX_OPER_PROTOCOL = 0;
    VRRP_SYS_ADPT_MIN_OPER_PROTOCOL = 0;
    VRRP_DEFAULT_ADMIN_STATE = 0;
    VRRP_DEFAULT_AUTH_LEN = 0;
    VRRP_DEFAULT_AUTH_TYPE = 0;
    VRRP_DEFAULT_PRIORITY = 0;
    VRRP_DEFAULT_PREEMPT_MODE = FALSE;
    VRRP_DEFAUTL_PREEMPT_DELAY = 0;
    VRRP_DEFAULT_ADVER_INTERVAL = 0;
    VRRP_DEFAULT_PRIMARY_IP_ADDR = 0;
    VRRP_DEFAULT_OPER_STATE = 0;
    VRRP_DEFAULT_OPER_PROTOCOL = 0;
    VRRP_DEFAULT_IP_ADDR_COUNT = 0;
} /* VRRP_SYS_ADPT_Init() */

/*************    To set and get the default value of parameters    ********/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultAdminState
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of admin state for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultAdminState(UI32_T admin_state)
{
    VRRP_DEFAULT_ADMIN_STATE = admin_state;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultAdminState() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultAdminState
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of admin state for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultAdminState(UI32_T *admin_state)
{
    *admin_state = VRRP_DEFAULT_ADMIN_STATE;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultPriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of priority for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultPriority(UI32_T priority)
{
    VRRP_DEFAULT_PRIORITY = priority;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultPriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of priority for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultPriority(UI32_T *priority)
{
    *priority = VRRP_DEFAULT_PRIORITY;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultPriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of preempt mode for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultPreemptMode(UI32_T mode)
{
    VRRP_DEFAULT_PREEMPT_MODE = mode;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultPreemptMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of preempt mode for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultPreemptMode(UI32_T *mode)
{
    *mode = VRRP_DEFAULT_PREEMPT_MODE;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultPreemptMode() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultPreemptDelayTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of preempt delay time 
 *             for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultPreemptDelayTime(UI32_T delay)
{
    VRRP_DEFAUTL_PREEMPT_DELAY = delay;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultPreemptDelayTime() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultPreemptDelayTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of preempt delay time 
 *             for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultPreemptDelayTime(UI32_T *delay)
{
    *delay = VRRP_DEFAUTL_PREEMPT_DELAY;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultPreemptDelayTime() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of advertisement interval for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultAdvertisementInterval(UI32_T adver_interval)
{
    VRRP_DEFAULT_ADVER_INTERVAL = adver_interval;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultAdvertisementInterval() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of advertisement interval for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultAdvertisementInterval(UI32_T *adver_interval)
{
    *adver_interval = VRRP_DEFAULT_ADVER_INTERVAL;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultAdvertisementInterval() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of authentication string
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultAuthenticationType(UI32_T auth_type)
{
    VRRP_DEFAULT_AUTH_TYPE = auth_type;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultAuthenticationType() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of authentication string 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultAuthenticationType(UI32_T *auth_type)
{
    *auth_type = VRRP_DEFAULT_AUTH_TYPE;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultAuthenticationType() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultAuthenticationLength
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of authentication string
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultAuthenticationLength(UI32_T auth_length)
{
    VRRP_DEFAULT_AUTH_LEN = auth_length;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultAuthenticationLength() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultAuthenticationLength
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of authentication string 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultAuthenticationLength(UI32_T *auth_length)
{
    *auth_length = VRRP_DEFAULT_AUTH_LEN;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultAuthenticationLength() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultPrimaryIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of primary ip address 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultPrimaryIpAddress(UI32_T ip_addr)
{
    VRRP_DEFAULT_PRIMARY_IP_ADDR = ip_addr;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultPrimaryIpAddress() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultPrimaryIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of primary ip address 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultPrimaryIpAddress(UI32_T *ip_addr)
{
    *ip_addr = VRRP_DEFAULT_PRIMARY_IP_ADDR;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultPrimaryIpAddress() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultOperProtocol
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of primary ip address 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultOperProtocol(UI32_T oper_protocol)
{
    VRRP_DEFAULT_OPER_PROTOCOL = oper_protocol;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultOperProtocol() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultOperProtocol
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of primary ip address 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultOperProtocol(UI32_T *oper_protocol)
{
    *oper_protocol = VRRP_DEFAULT_OPER_PROTOCOL;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultOperProtocol() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultOperState
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of operation state 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultOperState(UI32_T *state)
{
    *state = VRRP_DEFAULT_OPER_STATE;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultOperState() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetDefaultIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the default value of ip address count 
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetDefaultIpAddrCount(UI32_T ip_addr_count)
{
    VRRP_DEFAULT_IP_ADDR_COUNT = ip_addr_count;
    return TRUE;
} /* VRRP_SYS_ADPT_SetDefaultIpAddrCount() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetDefaultIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the default value of ip address count
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetDefaultIpAddrCount(UI32_T *ip_addr_count)
{
    *ip_addr_count = VRRP_DEFAULT_IP_ADDR_COUNT;
    return TRUE;
} /* VRRP_SYS_ADPT_GetDefaultIpAddrCount() */


/*************    To set and get the max and min of parameters    ********/

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangePriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of priority for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangePriority(UI32_T max_priority, UI32_T min_priority)
{
    VRRP_SYS_ADPT_MAX_VRRP_PRIORITY = max_priority;
    VRRP_SYS_ADPT_MIN_VRRP_PRIORITY = min_priority;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangePriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangePriority
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of priority for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangePriority(UI32_T *max_priority, UI32_T *min_priority)
{
    *max_priority = VRRP_SYS_ADPT_MAX_VRRP_PRIORITY;
    *min_priority = VRRP_SYS_ADPT_MIN_VRRP_PRIORITY;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangePriority() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangePreemptDelayTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of preempt delay
 *            time for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangePreemptDelayTime(UI32_T max_delay, UI32_T min_delay)
{
    VRRP_SYS_ADPT_MAX_VRRP_PREEMPT_DELAY = max_delay;
    VRRP_SYS_ADPT_MIN_VRRP_PREEMPT_DELAY = min_delay;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangePreemptDelayTime() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangePreemptDelayTime
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of preempt delay
 *            time for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangePreemptDelayTime(UI32_T *max_delay, UI32_T *min_delay)
{
    *max_delay = VRRP_SYS_ADPT_MAX_VRRP_PREEMPT_DELAY;
    *min_delay = VRRP_SYS_ADPT_MIN_VRRP_PREEMPT_DELAY;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangePreemptDelayTime() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangeAdverInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of hello time for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeAdverInterval(UI32_T max_interval, UI32_T min_interval)
{
    VRRP_SYS_ADPT_MAX_VRRP_ADVER_INTERVAL = max_interval;
    VRRP_SYS_ADPT_MIN_VRRP_ADVER_INTERVAL = min_interval;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeAdverInterval() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangeAdverInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of hello time for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangeAdverInterval(UI32_T *max_interval, UI32_T *min_interval)
{
    *max_interval = VRRP_SYS_ADPT_MAX_VRRP_ADVER_INTERVAL;
    *min_interval = VRRP_SYS_ADPT_MIN_VRRP_ADVER_INTERVAL;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangeAdverInterval() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangeOperProtocol
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of operation protocol for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeOperProtocol(UI32_T max_oper_protocol, UI32_T min_oper_protocol)
{
    VRRP_SYS_ADPT_MAX_OPER_PROTOCOL = max_oper_protocol;
    VRRP_SYS_ADPT_MIN_OPER_PROTOCOL = min_oper_protocol;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeOperProtocol() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangeOperProtocol
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of operation protocol for 
 *            VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangeOperProtocol(UI32_T *max_oper_protocol, UI32_T *min_oper_protocol)
{
    *max_oper_protocol = VRRP_SYS_ADPT_MAX_OPER_PROTOCOL;
    *min_oper_protocol = VRRP_SYS_ADPT_MIN_OPER_PROTOCOL;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangeOperProtocol() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangeAuthenticationString
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of authentication
 *            length for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeAuthenticationString(UI32_T max_auth_length, UI32_T min_auth_length)
{
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeAuthenticationString() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangeAuthenticationString
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of authentication
 *            length for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangeAuthenticationString(UI32_T *max_auth_length, UI32_T *min_auth_length)
{
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangeAuthenticationString() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangeVrrpNumber
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of VRRP group number
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeVrrpNumber(UI32_T max_vrid_num, UI32_T min_vrid_num)
{
    VRRP_SYS_ADPT_MAX_VRRP_ID = max_vrid_num;
    VRRP_SYS_ADPT_MIN_VRRP_ID = min_vrid_num;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeVrrpNumber() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangeVrrpNumber
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of VRRP group number
 *            for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangeVrrpNumber(UI32_T *max_vrid_num, UI32_T *min_vrid_num)
{
    *max_vrid_num = VRRP_SYS_ADPT_MAX_VRRP_ID;
    *min_vrid_num = VRRP_SYS_ADPT_MIN_VRRP_ID;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangeVrrpNumber() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetRangeAssocIpNumber
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max and min value of VRRP virtual ip
 *            address number for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeAssocIpNumber(UI32_T max_assoc_ip_num, UI32_T min_assoc_ip_num)
{
    VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP = max_assoc_ip_num;
    VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_ASSOC_IP = min_assoc_ip_num;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeAssocIpNumber() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetRangeAssocIpNumber
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max and min value of VRRP virtual ip
 *            address number for VRRP component.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetRangeAssocIpNumber(UI32_T *max_assoc_ip_num, UI32_T *min_assoc_ip_num)
{
    *max_assoc_ip_num = VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_ASSOC_IP;
    *min_assoc_ip_num = VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_ASSOC_IP;
    return TRUE;
} /* VRRP_SYS_ADPT_GetRangeAssocIpNumber() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetMaxVrrpGrpNumberPerSystem
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the max number of VRRP group per system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetRangeVrrpGrpNumberPerSystem(UI32_T max_grp_num)
{
    VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_GROUP_PER_SYSTEM = max_grp_num;
    return TRUE;
} /* VRRP_SYS_ADPT_SetRangeVrrpGrpNumberPerSystem() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetMaxVrrpGrpNumberPerSystem
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the max number of VRRP group per system.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetMaxVrrpGrpNumberPerSystem(UI32_T *max_grp_num)
{
    *max_grp_num = VRRP_SYS_ADPT_MAX_NBR_OF_VRRP_GROUP_PER_SYSTEM;
    return TRUE;
} /* VRRP_SYS_ADPT_GetMaxVrrpGrpNumberPerSystem() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_SetMinVrrpGrpNumberPerInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will set the min number of VRRP group per interface.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_SetMinVrrpGrpNumberPerInterface(UI32_T min_grp_num)
{
    VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_GROUP_PER_INTERFACE = min_grp_num;
    return TRUE;
} /* VRRP_SYS_ADPT_SetMinVrrpGrpNumberPerInterface() */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_GetMinVrrpGrpNumberPerInterface
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will get the min number of VRRP group per interface.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE  -- success
 *            FALSE -- failure
 * NOTES    : none
 *--------------------------------------------------------------------------*/
BOOL_T VRRP_SYS_ADPT_GetMinVrrpGrpNumberPerInterface(UI32_T *min_grp_num)
{
    *min_grp_num = VRRP_SYS_ADPT_MIN_NBR_OF_VRRP_GROUP_PER_INTERFACE;
    return TRUE;
} /* VRRP_SYS_ADPT_GetMinVrrpGrpNumberPerInterface() */
#endif

