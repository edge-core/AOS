/* MODULE NAME:  vrrp_pmgr.c
 * PURPOSE:
 *     VRRP PMGR APIs.
 *
 * NOTES:
 *
 * HISTORY
 *    3/28/2009 - Donny.Li     , Created
 *
 * Copyright(C)      Accton Corporation, 2009     

 */
#include "vrrp_type.h"

BOOL_T VRRP_PMGR_InitiateProcessResource(void);
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ip address for the specific vrrp on the interface 
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 *            row_status: VAL_vrrpAssoIpAddrRowStatus_notInService or 
 *                        VAL_vrrpAssoIpAddrRowStatus_destroy to delete this ip address
 *                        VAL_vrrpAssoIpAddrRowStatus_createAndGo or
 *                        VAL_vrrpAssoIpAddrRowStatus_active to add this ip address
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_PARAMETER_ERROR/VRRP_TYPE_OPER_ENTRY_NOT_EXIST/
 *            VRRP_TYPE_EXCESS_VRRP_NUMBER_ON_CHIP/TRUE/FALSE/
 * NOTES    : 
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr, UI32_T row_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPrimaryIp
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the primary ip address for the spicific vrrp on the interface 
 *            for MIB
 * INPUT    : if_index:   ifindex
 *            vrid:       id of vrrp
 *            ip_addr:    ip address to be set or delete
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPrimaryIp(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextIpAddress
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next ip address for vrrp 
 * INPUT    : if_index: the specific interface
 *            vrid:     the specific vrrp group id
 *            ip_addr:  the buffer to get the ip address
 * OUTPUT   : next associated ip address of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextIpAddress(UI32_T if_index, UI8_T vrid, UI8_T *ip_addr);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetOperAdminStatus                       
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Set.  
 *            Otherwise, return false.                  
 * INPUT    : vrrp_admin_status -  
              VAL_vrrpOperAdminState_up 1L \ 
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid                      
 * OUTPUT   : none                                                             
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : The administrative status requested by management for VRRP.  The 
 *            value enabled(1) indicates that specify VRRP virtual router should 
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router 
 *            is not operated. Row status must be up before set enabled(1).
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T vrrp_admin_status);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperAdminStatus                       
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.  
 *            Otherwise, return false.                  
 * INPUT    : vrrp_admin_status -  
              VAL_vrrpOperAdminState_up 1L \ 
 *            VAL_vrrpOperAdminState_down 2L
              ifIndex  ,  vrid                      
 * OUTPUT   : vrrp admin status for the specific ifindex and vrid                                                             
 * RETURN   : TRUE/FALSE/VRRP_TYPE_OPER_ENTRY_NOT_EXIST
 * NOTES    :  
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperAdminStatus(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_admin_status);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication type for each interface 
 * INPUT    : if_Index, vrid,
 *            auth_type - 
 *               VAL_vrrpOperAuthType_noAuthentication	     1L 
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L 
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L 
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_auth_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAuthenticationType
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication type for each interface 
 * INPUT    : if_Index, vrid
 * OUTPUT   : authentication type of the specific ifindex and vrid
 *               VAL_vrrpOperAuthType_noAuthentication	     1L 
 *               VAL_vrrpOperAuthType_simpleTextPassword	 2L 
 *               VAL_vrrpOperAuthType_ipAuthenticationHeader 3L 
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAuthenticationType(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_auth_type);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the authentication key for each interface 
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAuthenticationKey
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the authentication key for each interface 
 * INPUT    : ifindex, vrid
 * OUTPUT   : authentication key of the specific ifindex and vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAuthenticationKey(UI32_T if_index, UI8_T vrid, UI8_T *vrrp_oper_auth_key);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAuthentication                       
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the authentication data is set successfully. 
              Otherwise, return false.
 * INPUT    : ifIndex, vrid,
 *            auth_type - 
              	VAL_vrrpOperAuthType_noAuthentication	1L \
              	VAL_vrrpOperAuthType_simpleTextPassword	2L \
              	VAL_vrrpOperAuthType_ipAuthenticationHeader 3L 
              auth_key   
 * OUTPUT   : none                                                             
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    :
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetAuthentication(UI32_T if_index, UI8_T vrid,UI32_T auth_type, UI8_T *auth_key);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the priority for each vrrp 
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : MIN_vrrpOperPriority	0L
 *            MAX_vrrpOperPriority	255L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPriority(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetPriority
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the priority for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : vrrp priority of the specific ifindex oand vrid
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetPriority(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_priority);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp 
 * INPUT    : ifindex, vrid, preempt mode and delay time for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode, UI32_T delay);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPreemptMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the preemption mode for each vrrp  for MIB
 * INPUT    : ifindex, vrid, preempt mode for this vrrp group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : the preempt_delay_time is private for our implementation.
 *            SYS_ADPT_MIN_VIRTUAL_ROUTER_PRE_DELAY   0
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_PRE_DELAY   120
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetPreemptMode(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_preempt_mode);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetPreemptionMode
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the preemption mode for each vrrp 
 * INPUT    : ifindex, vrid
 * OUTPUT   : preempt mode and preempt delay time for this group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetPreemptionMode(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_preempt_mode, UI32_T *delay);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the advertisement interval for each vrrp 
 * INPUT    : ifindex, vrid, advertisement interval for the group
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : SYS_ADPT_MIN_VIRTUAL_ROUTER_ADVER_INT   1
 *            SYS_ADPT_MAX_VIRTUAL_ROUTER_ADVER_INT 255
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_advertise_interval);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetAdvertisementInterval
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the advertisement interval for each vrrp 
 * INPUT    : ifindex, vrid
 * OUTPUT   : advertisement interval of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetAdvertisementInterval(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_advertise_interval);

#if (SYS_CPNT_VRRP_PING == TRUE)
/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetPingStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the ping enable status of VRRP
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : None
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR	
 * NOTES    : ping_status:
 * 			  VRRP_TYPE_PING_STATUS_ENABLE/
 *			  VRRP_TYPE_PING_STATUS_DISABLE
 *            
 *-------------------------------------------------------------------------- 
 */
UI32_T VRRP_PMGR_SetPingStatus(UI32_T ping_status);
#endif

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpOperRowStatus
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry->if_index, vrrp_oper_entry->vrid,
 *            action    row_status to be set to the specific vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : action -- 
 *            VAL_vrrpOperRowStatus_active	      1L
 *            VAL_vrrpOperRowStatus_notInService    2L
 *            VAL_vrrpOperRowStatus_notReady        3L
 *            VAL_vrrpOperRowStatus_createAndGo	  4L
 *            VAL_vrrpOperRowStatus_createAndWait   5L
 *            VAL_vrrpOperRowStatus_destroy	      6L
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpOperRowStatus(VRRP_OPER_ENTRY_T *vrrp_oper_entry, UI32_T action);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *		      VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_DeleteVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP operation entry of the specific vrrp
 * INPUT    : vrrp_oper_entry
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->ifindex, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_DeleteVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP operation entry
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : next ifindex, next vrid, next vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetDefaultVrrpOperEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the default VRRP operation entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_oper_entry
 * OUTPUT   : vrrp oper entry
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *		      VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_oper_entry->if_index, vrrp_oper_entry->vrid) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetDefaultVrrpOperEntry(VRRP_OPER_ENTRY_T *vrrp_oper_entry);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_SetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_DeleteAssoIpEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Delete the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : vrrp_assoc_ip_addr
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_DeleteAssoIpEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpAssocIpAddrEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry of the specific vrrp
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *            vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetNextVrrpAssocIpAddrEntry(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpAssocIpAddrEntryBySnmp
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the next VRRP associated IP addresses entry
 * INPUT    : buffer which pointers to vrrp_assoc_ip_addr
 * OUTPUT   : next ifindex, next vrid, next vrrp_assoc_ip_addr
 * RETURN   : TRUE/FALSE
 * NOTES    :(1)use (vrrp_assoc_ip_addr->ifindex, vrrp_assoc_ip_addr->vrid,
 *              vrrp_assoc_ip_addr->assoc_ip_addr) as index
 *           (2)This API is used by snmp to getnext AssocIpAddrEntry through 
 *              whole system
 *-------------------------------------------------------------------------- */
BOOL_T VRRP_PMGR_GetNextVrrpAssocIpAddrEntryBySnmp(VRRP_ASSOC_IP_ENTRY_T *vrrp_assoc_ip_addr);

/*--------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetIpAddrCount
 *--------------------------------------------------------------------------
 * PURPOSE  : Get the number of IP addresses for each vrrp (RFC2338, p.14)
 * INPUT    : ifindex, vrid
 * OUTPUT   : ip address count of the specific vrrp group
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : none
 *-------------------------------------------------------------------------- */
UI32_T VRRP_PMGR_GetIpAddrCount(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_ip_addr_count);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperState                       
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton gets the operation state of the specific vrid and ifindex
 * INPUT    : ifIndex  ,  vrid                      
 * OUTPUT   : vrrp oper state of the specific vrrp group - 
 *            VAL_vrrpOperState_initialize       1L
 *            VAL_vrrpOperState_backup           2L
 *            VAL_vrrpOperState_master           3L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 * NOTES    : The administrative status requested by management for VRRP.  The 
 *            value enabled(1) indicates that specify VRRP virtual router should 
 *            be enabled on this interface.  When disabled(2), VRRP virtual Router 
 *            is not operated. This object affects all VRRP Applicant and Registrar 
 *            state machines. 
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperState(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_state);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_SetOperProtocol                       
 *------------------------------------------------------------------------------
 * PURPOSE  : Set operation protocol for the specific ifindex and vrid                  
 * INPUT    : ifIndex  ,  vrid, operation protocol
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -  
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : This funciton returns true if the VRRP status is successfully Get.  
 *            Otherwise, return false.
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_SetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T vrrp_oper_protocol);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetOperProtocol                       
 *------------------------------------------------------------------------------
 * PURPOSE  : This funciton returns true if the VRRP status is successfully Get.  
 *            Otherwise, return false.                  
 * INPUT    : ifIndex  ,  vrid                      
 * OUTPUT   : vrrp oper protocol of the specific vrrp group -  
 *            VAL_vrrpOperProtocol_ip                 1L
 *            VAL_vrrpOperProtocol_bridge             2L
 *            VAL_vrrpOperProtocol_decnet             3L
 *            VAL_vrrpOperProtocol_other              4L
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : None
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetOperProtocol(UI32_T if_index, UI8_T vrid, UI32_T *vrrp_oper_protocol);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVersionNumber
 *------------------------------------------------------------------------------
 * PURPOSE  : It is used to identify the vrrp version of the system.            
 * INPUT    : buffer to be put in the version number
 * OUTPUT   : The vrrp version number                                                             
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : 1. Please refer to RFC 2338 , section 5.3.1 for detailed information.
 *            2. It's always return "2" right now.                              
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVersionNumber(UI32_T *version);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the system's vrrp statistics. 
 * INPUT    : none                                                              
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors                                                              
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none                                                            
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVrrpSysStatistics(VRRP_OM_Router_Statistics_Info_T *vrrp_router_statistics);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_ClearVrrpSysStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the system's vrrp statistics. 
 * INPUT    : none                                                              
 * OUTPUT   : none                                                           
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none                                                            
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_ClearVrrpSysStatistics(void);

/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_ClearVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To clear the specific vrrp group statistics.
 * INPUT    : ifindex, vrid
 * OUTPUT   : none
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_INTERNAL_ERROR
 * NOTES    : none                                                            
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_ClearVrrpGroupStatistics(UI32_T if_index, UI8_T vrid);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetVrrpGroupStatistics(UI32_T if_index, UI8_T vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextVrrpGroupStatistics
 *------------------------------------------------------------------------------
 * PURPOSE  : To retrive the next specific vrrp group statistics.
 * INPUT    : ifindex, vrid, buffer to be put in statistics info
 * OUTPUT   : *vrrp_sys_statistics -- the statistics of the vrrp sytem,
 *            ChkSumErrors \ versionErrors \ vridErrors
 * RETURN   : VRRP_TYPE_OK/
 *			  VRRP_TYPE_PARAMETER_ERROR
 * NOTES    : none
 *------------------------------------------------------------------------------*/
UI32_T VRRP_PMGR_GetNextVrrpGroupStatistics(UI32_T *if_index, UI8_T *vrid, VRRP_OM_Vrrp_Statistics_Info_T *vrrp_group_statistics);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPriority(UI32_T ifindex, UI8_T vrid, UI32_T *priority);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPriority
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            priority -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            priority -- output vrrp priority
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPriority(UI32_T *ifindex, UI8_T *vrid, UI32_T *priority);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAuthType(UI32_T ifindex, UI8_T vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAuthType
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            auth_type -- output buffer
 * OUTPUT   : ifindex   -- next ifindex
 *            vrid      -- next vrid
 *            auth_type -- output vrrp authentication type
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAuthType(UI32_T *ifindex, UI8_T *vrid, UI32_T *auth_type, UI8_T *vrrp_oper_auth_key);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAdverInt(UI32_T ifindex, UI8_T vrid, UI32_T *interval);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAdverInt
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            interval -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            interval -- output vrrp advertisement interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAdverInt(UI32_T *ifindex, UI8_T *vrid, UI32_T *interval);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPreemptMode(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_mode);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPreemptMode
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPreemptMode(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_mode);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpPreemptDelay(UI32_T ifindex, UI8_T vrid, UI32_T *preempt_delay);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpPreemptDelay
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            preempt_mode -- output buffer
 * OUTPUT   : ifindex      -- next ifindex
 *            vrid         -- next vrid
 *            preempt_mode -- output vrrp preempt mode
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpPreemptDelay(UI32_T *ifindex, UI8_T *vrid, UI32_T *preempt_delay);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpProtocol(UI32_T ifindex, UI8_T vrid, UI32_T *protocol);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpProtocol
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            protocol -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            protocol -- output vrrp protocol
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpProtocol(UI32_T *ifindex, UI8_T *vrid, UI32_T *protocol);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ip_addr_count  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningVrrpAssocIp(UI32_T ifindex, UI8_T vrid, UI8_T *ip_addr_count);
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetNextRunningVrrpAssocIp
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP status and check whether it is default value.
 * INPUT    : ifindex,
 *            vrid,
 *            ip_addr  -- output buffer
 * OUTPUT   : ifindex  -- next ifindex
 *            vrid     -- next vrid
 *            ip_addr  -- output vrrp associated ip address
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetNextRunningVrrpAssocIp(UI32_T *ifindex, UI8_T *vrid, UI8_T *ip_addr);

#if (SYS_CPNT_VRRP_PING == TRUE)
/*------------------------------------------------------------------------------
 * FUNCTION NAME - VRRP_PMGR_GetRunningPingStatus
 *------------------------------------------------------------------------------
 * PURPOSE  : Get VRRP ping status and check whether it is default value.
 * INPUT    : ping_status	--	ping enable status
 * OUTPUT   : ping_status	--	ping enable status
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS   -- not same as default
 *          : SYS_TYPE_GET_RUNNING_CFG_FAIL      -- get failure
 *          : SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE -- same as default
 * NOTES    : none
 *------------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T VRRP_PMGR_GetRunningPingStatus(UI32_T *ping_status);
#endif
