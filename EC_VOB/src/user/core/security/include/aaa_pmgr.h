/* MODULE NAME: aaa_pmgr.h
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2007/08/07 : eli      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef AAA_PMGR_H
#define AAA_PMGR_H

#include "aaa_mgr.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius group by name.
 * INPUT    : name (terminated with '\0');
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs group by name.
 * INPUT    : name (terminated with '\0');
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs group by name.
 * INPUT    : name (terminated with '\0');
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry by group_index
 * INPUT    : group_index, radius_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            radius_index will be ignored
 *            don't allow to modify the members of "default radius group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and entry content
 * INPUT    : group_index, entry->radius_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry by group_index
 * INPUT    : group_index, tacacs_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            tacacs_index will be ignored
 *            don't allow to modify the members of "default tacacs+ group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry join default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry depart default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and entry content
 * INPUT    : group_index, entry->tacacs_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning);

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the accounting update interval (minutes);
 * INPUT    : update_interval
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccUpdateInterval(UI32_T update_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccUpdateInterval();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccDot1xEntry(UI32_T ifindex);



/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccExecEntry(const AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccExecEntry(AAA_ExecType_T exec_type);

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command accounting by specified privilege level
 * INPUT    : privilege, list name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore to change the configure mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the command accounting entry on specified privilege level
 *            and EXEC type
 * INPUT    : priv_lvl
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAccCommandEntry(const AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_AsyncAccountingRequestForAccCmd
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : request  - request to send
 * OUTPUT   : ser_no_p - serial number for request sent.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_AsyncAccountingRequestForAccCmd(
        AAA_AccRequest_T *request,
        UI32_T           *ser_no_p);

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type, entry->working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAccListEntry(const AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_DestroyAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_DestroyAccListEntry2
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyAccListEntry2(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_Register_AccComponent_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : register the accounting component function
 * INPUT    : cpnt_type, call_back_func
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AAA_PMGR_Register_AccComponent_Callback(AAA_AccCpntType_T cpnt_type, AAA_AccCpntAsyncNotify_Callback_T call_back_func);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_AsyncAccountingRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : ifindex, client_type, request_type, identifier,
 *            user_name       --  User name (terminated with '\0');
 *            call_back_func  --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_AsyncAccountingRequest(AAA_AccRequest_T *request);

#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the moethod_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
 *              method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodName(UI16_T method_index, char* method_name);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the group_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodGroupName(UI16_T method_index, char* method_group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetMethodClientType(UI16_T method_index, AAA_ClientType_T client_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the mode into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_mode    -- VAL_aaaMethodMode_start_stop
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodMode(UI16_T method_index, UI8_T method_mode);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetMethodStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the status into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_status  -- Set to 1 to initiate the aaaMethodTable, 2 to destroy the table.
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetMethodStatus(UI16_T method_index, UI8_T method_status);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupName(UI16_T radius_group_index, char* radius_group_name);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupServerBitMap(UI16_T radius_group_index, UI8_T radius_group_server_bitmap);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetRadiusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status
 * INPUT    :   radius_group_index
                radius_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetRadiusGroupStatus(UI16_T radius_group_index, UI32_T radius_group_status);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupName(UI16_T tacacsplus_group_index, char* tacacsplus_group_name);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupServerBitMap(UI16_T tacacsplus_group_index, UI8_T tacacsplus_group_server_bitmap);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetTacacsPlusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status of TacacsPlusGroupTable into AAA_PMGR_SetTacacsPlusGroupStatus
 *              by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetTacacsPlusGroupStatus(UI16_T tacacsplus_group_index, UI32_T tacacsplus_group_status);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetUpdate(UI32_T update_interval);

#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccountMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-name into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAccountMethodName(UI32_T ifindex, char* account_method_name);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_PMGR_SetAccountStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-status into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_PMGR_SetAccountStatus(UI32_T ifindex, UI8_T account_status);

#endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQty(UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetAccUserEntryQtyFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntry(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name and client_type by index.
 * INPUT    : entry->user_index, entry->user_name, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_GetNextAccUserEntryFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user from specified port by index.
 * INPUT    : entry->user_index, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry);

#endif

#if (SYS_CPNT_AUTHORIZATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_AuthorRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : request to do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_AuthorRequest(AAA_AuthorRequest_T *request,AAA_AuthorReply_T *reply);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAuthorExecEntry(AAA_ExecType_T exec_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0');
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DestroyAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning);

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_SetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_PMGR_DisableAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable command authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_PMGR_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type);
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


/*-----------------------------------------------------------------------------
 * ROUTINE NAME : AAA_PMGR_Init
 *-----------------------------------------------------------------------------
 * PURPOSE : Initiate resource for AAA_PMGR in the calling process.
 *
 * INPUT   : None.
 *
 * OUTPUT  : None.
 *
 * RETURN  : None.
 *
 * NOTES   : None.
 *-----------------------------------------------------------------------------
 */
void AAA_PMGR_Init(void);


#endif

