/* MODULE NAME: aaa_pom.h
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2007/08/07 : eli     Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef AAA_POM_H
#define AAA_POM_H

#include "sys_type.h"
#include "aaa_def.h"

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting update interval (minutes)
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccUpdateInterval(UI32_T *update_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next radius group entry by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry);


 /*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusGroupEntry_Ex(AAA_RadiusGroupEntryInterface_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next tacacs group entry by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetTacacsPlusGroupEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusGroupEntry_Ex(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index, entry->radius_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index (1-based), entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRadiusEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : radius_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusEntry_Ex(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:
 *--------------------------------AAA_POM_GetRadiusEntryOrder-----------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order);

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_IsRadiusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_IsRadiusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex);
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_IsRadiusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the radius entry is valid or not according to input params
 * INPUT    : radius_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_IsRadiusEntryValid(UI16_T radius_index, AAA_ClientType_T client_type, UI32_T ifindex);
#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index, entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index (1-based), entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : tacacs_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusEntry_Ex(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

#if (SYS_CPNT_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting update interval (minutes)
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccUpdateInterval(UI32_T *update_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccExecEntry_Ex(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccDot1xEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccDot1xEntry_Ex(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningAccListEntry(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccListEntry(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccListEntryFilterByClientType(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_QueryAccDot1xPortList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port list associated with specific list_index
 * INPUT    : query_result->list_index
 * OUTPUT   : query_result->port_list
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_QueryAccDot1xPortList(AAA_QueryAccDot1xPortListResult_T *query_result);

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAccCommandEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the command accounting entry by specified privilege level
 *            and EXEC type
 * INPUT    : entry_p->priv_lvl
 * OUTPUT   : entry_p
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAccCommandEntryInterface(AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the command accounting entry by priv-lvl and exec type.
 * INPUT    : entry_p->priv_lvl, entry_p->exec_type
 * OUTPUT   : entry_p
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p);

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the next AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetUpdate(UI32_T *update_interval);


#if (SYS_CPNT_ACCOUNTING == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_POM_GetAccountTable entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_POM_GetNextAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetNextAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry);

#endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)
#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_POM_Backdoor_ShowAccUser
 * ---------------------------------------------------------------------
 * PURPOSE  : show accounting user information
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTES    : for aaa backdoor
 * ---------------------------------------------------------------------*/
void AAA_POM_Backdoor_ShowAccUser();
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */
#if (SYS_CPNT_AUTHORIZATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAuthorListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAuthorListEntryFilterByClientType(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetNextAuthorExecEntry(AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_POM_GetAuthorExecEntry_Ex(AAA_AuthorExecEntry_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetRunningAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetNextRunningAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetNextRunningAuthorListEntry(AAA_AuthorListEntryInterface_T *entry);

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_POM_GetAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_POM_GetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : Get command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T AAA_POM_GetRunningAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry);

#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AAA_POM_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for CSCA_POM.
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    None.
 *------------------------------------------------------------------------------
 */
void AAA_POM_Init(void);

#endif /* End of AAA_POM_H */

