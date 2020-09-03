/* -------------------------------------------------------------------------------------
 * FILE	NAME: VRRP_SYS_ADPT.h                                                                 
 *                                                                                      
 * PURPOSE: This package provides header file for providing default and range value
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009    

 * -------------------------------------------------------------------------------------*/

#ifndef _VRRP_SYS_ADPT_H
#define _VRRP_SYS_ADPT_H
#if 0
/*-----------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_SYS_ADPT_Initiate_System_Resources
 *-----------------------------------------------------------------------------------
 * PURPOSE  : This function initialize vrrp sys_adpt.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-----------------------------------------------------------------------------------*/
void VRRP_SYS_ADPT_Initiate_System_Resources(void);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultAdminState(UI32_T admin_state);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultAdminState(UI32_T *admin_state);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultPriority(UI32_T priority);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultPriority(UI32_T *priority);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultPreemptMode(UI32_T mode);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultPreemptMode(UI32_T *mode);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultPreemptDelayTime(UI32_T delay);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultPreemptDelayTime(UI32_T *delay);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultAdvertisementInterval(UI32_T adver_interval);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultAdvertisementInterval(UI32_T *adver_interval);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultAuthenticationType(UI32_T auth_type);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultAuthenticationType(UI32_T *auth_type);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultAuthenticationLength(UI32_T auth_length);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultAuthenticationLength(UI32_T *auth_length);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultPrimaryIpAddress(UI32_T ip_addr);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultPrimaryIpAddress(UI32_T *ip_addr);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultOperProtocol(UI32_T oper_protocol);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultOperProtocol(UI32_T *oper_protocol);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultOperState(UI32_T *state);

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
BOOL_T VRRP_SYS_ADPT_SetDefaultIpAddrCount(UI32_T ip_addr_count);

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
BOOL_T VRRP_SYS_ADPT_GetDefaultIpAddrCount(UI32_T *ip_addr_count);


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
BOOL_T VRRP_SYS_ADPT_SetRangePriority(UI32_T max_priority, UI32_T min_priority);

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
BOOL_T VRRP_SYS_ADPT_GetRangePriority(UI32_T *max_priority, UI32_T *min_priority);

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
BOOL_T VRRP_SYS_ADPT_SetRangePreemptDelayTime(UI32_T max_delay, UI32_T min_delay);

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
BOOL_T VRRP_SYS_ADPT_GetRangePreemptDelayTime(UI32_T *max_delay, UI32_T *min_delay);

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
BOOL_T VRRP_SYS_ADPT_SetRangeAdverInterval(UI32_T max_interval, UI32_T min_interval);

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
BOOL_T VRRP_SYS_ADPT_GetRangeAdverInterval(UI32_T *max_interval, UI32_T *min_interval);

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
BOOL_T VRRP_SYS_ADPT_SetRangeOperProtocol(UI32_T max_oper_protocol, UI32_T min_oper_protocol);

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
BOOL_T VRRP_SYS_ADPT_GetRangeOperProtocol(UI32_T *max_oper_protocol, UI32_T *min_oper_protocol);

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
BOOL_T VRRP_SYS_ADPT_SetRangeAuthenticationString(UI32_T max_auth_length, UI32_T min_auth_length);

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
BOOL_T VRRP_SYS_ADPT_GetRangeAuthenticationString(UI32_T *max_auth_length, UI32_T *min_auth_length);

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
BOOL_T VRRP_SYS_ADPT_SetRangeVrrpNumber(UI32_T max_vrid_num, UI32_T min_vrid_num);

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
BOOL_T VRRP_SYS_ADPT_GetRangeVrrpNumber(UI32_T *max_vrid_num, UI32_T *min_vrid_num);

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
BOOL_T VRRP_SYS_ADPT_SetRangeAssocIpNumber(UI32_T max_assoc_ip_num, UI32_T min_assoc_ip_num);

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
BOOL_T VRRP_SYS_ADPT_GetRangeAssocIpNumber(UI32_T *max_assoc_ip_num, UI32_T *min_assoc_ip_num);

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
BOOL_T VRRP_SYS_ADPT_SetRangeVrrpGrpNumberPerSystem(UI32_T max_grp_num);

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
BOOL_T VRRP_SYS_ADPT_GetMaxVrrpGrpNumberPerSystem(UI32_T *max_grp_num);

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
BOOL_T VRRP_SYS_ADPT_SetMinVrrpGrpNumberPerInterface(UI32_T min_grp_num);

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
BOOL_T VRRP_SYS_ADPT_GetMinVrrpGrpNumberPerInterface(UI32_T *min_grp_num);

#endif
#endif /* _VRRP_SYS_ADPT_H */

