/* MODULE NAME: aaa_om_private.h
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2004/03/24 : mfhorng      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef AAA_OM_PRIVATE_H
#define AAA_OM_PRIVATE_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

#include "sys_adpt.h"
#include "aaa_def.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS     FALSE /* CLI: aaa accounting dot1x {list-name} {start-stop} group {tacacs+} */

#define AAA_MAX_NBR_OF_RADIUS_ENTRY             SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS

#ifdef SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS
    #define AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY    SYS_ADPT_MAX_NBR_OF_TACACS_SERVERS
#else
    #define AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY    5
#endif

#define AAA_OM_SIZEOF(type_name) \
    (AAA_OM_MSGBUF_TYPE_SIZE + sizeof(type_name))

#define AAA_OM_MSGBUF_TYPE_SIZE     sizeof(((AAA_OM_IPCMsg_T *)0)->type)

/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */
#if 0
typedef enum AAA_EntryStatus_E
{
    AAA_ENTRY_DESTROYED,
    AAA_ENTRY_READY,
} AAA_EntryStatus_T;
#endif

typedef enum AAA_ApiUpdateResult_E
{
    AAA_API_UPDATE_CHANGE,
    AAA_API_UPDATE_NO_CHANGE, /* there is no effect in OM */
    AAA_API_UPDATE_FAILED,
} AAA_ApiUpdateResult_T;

#if (SYS_CPNT_ACCOUNTING == TRUE)

typedef struct AAA_AccSettingInfo_S
{
    AAA_ServerGroupType_T   group_type;
    AAA_AccWorkingMode_T    working_mode;
} AAA_AccSettingInfo_T;

typedef struct AAA_AccNotifyMap_S
{
    UI8_T   radius_start:1;     /* whether notify radius mgr or not */
    UI8_T   tacacs_start:1;     /* whether notify tacacs mgr or not */
    UI8_T   reserved_bits:6;
} AAA_AccNotifyMap_T;

typedef struct AAA_AccUserEntryInterface_S
{
    UI16_T  user_index;     /* array index + 1 */
    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
    AAA_ClientType_T            client_type;    /* caller's type */
    AAA_AccNotifyMap_T          notify_bitmap;
} AAA_AccUserEntryInterface_T;

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_Initialize
 *-------------------------------------------------------------------------
 * PURPOSE  : intialize om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_Initialize();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_Finalize
 *-------------------------------------------------------------------------
 * PURPOSE  : cleanup om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void AAA_OM_Finalize();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningRadiusGroupEntryInterface
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryInterfaceByIndex(AAA_RadiusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle radius group
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeRadiusGroupEntry(UI16_T group_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetRadiusGroupEntryName
 * ------------------------------------------------------------------------
 * PURPOSE  : set group name and modified time by radius_group_index.
 * INPUT    : group_index, group_name, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "radius" group name
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntryName(UI16_T group_index, char* group_name, UI32_T sys_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetRadiusGroupEntryServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  : set server bitmap and modified time by group_index.
 * INPUT    : group_index, server_bitmap, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "radius" group's radius entry
 *            server index 1 at highest bit (e.g. 1000 0000)
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntryServerBitMap(UI16_T group_index, UI8_T server_bitmap, UI32_T sys_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetRadiusGroupEntryStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : set the group status and modified time
 * INPUT    : group_index, entry_status, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't delete the default "radius" group
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntryStatus(UI16_T group_index, AAA_EntryStatus_T entry_status, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningTacacsPlusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next tacacs plue group entry by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs plus group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs+ group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlustGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle tacacs group
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * RETURN   : can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeTacacsPlusGroupEntry(UI16_T group_index);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetTacacsPlusGroupEntryName
 * ------------------------------------------------------------------------
 * PURPOSE  : set group name and modified time by tacacs_group_index.
 * INPUT    : group_index, group_name, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "tacacs+" group name
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntryName(UI16_T group_index, char* group_name, UI32_T sys_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetTacacsPlusGroupEntryServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  : set server bitmap and modified time by group_index.
 * INPUT    : group_index, server_bitmap, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "tacacs+" group's tacacs entry
 *            server index 1 at highest bit (e.g. 1000 0000)
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntryServerBitMap(UI16_T group_index, UI8_T server_bitmap, UI32_T sys_time);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetTacacsPlusGroupEntryStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : set the group status and modified time
 * INPUT    : group_index, entry_status, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't delete the default "tacacs+" group
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntryStatus(UI16_T group_index, AAA_EntryStatus_T entry_status, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index, entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusEntryInterface(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : radius_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryInterface(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryInterfaceByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th radius entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryInterfaceByOrder(UI16_T group_index, UI16_T order, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry and group modified time by group_index
 * INPUT    : group_index, radius_server_index, sys_time
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            radius_index will be ignored
 *            don't allow to modify the members of "default radius group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group and update group modified time
 * INPUT    : radius_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group and update group modified time
 * INPUT    : radius_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and entry content (update group modified time)
 * INPUT    : group_index, entry->radius_server_index, sys_time
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time);



#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsRadiusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsRadiusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsRadiusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the radius entry is valid or not according to input params
 * INPUT    : radius_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsRadiusEntryValid(UI16_T radius_index, AAA_ClientType_T client_type, UI32_T ifindex);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs accounting entry by group_index and tacacs_index.
 * INPUT    : group_index, entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusEntryInterface(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : tacacs_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryInterface(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryInterfaceByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th tacacs entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryInterfaceByOrder(UI16_T group_index, UI16_T order, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs entry's order in group
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryOrder(UI16_T group_index, UI16_T tacacs_index, UI16_T *order);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs entry and group modified time by group_index
 * INPUT    : group_index, tacacs_server_index, sys_time
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *            tacacs_index will be ignored
 *            don't allow to modify the members of "default tacacs+ group"
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs entry join default "tacacs+" group and update group modified time
 * INPUT    : tacacs_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs entry depart default "tacacs+" group and update group modified time
 * INPUT    : tacacs_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs entry by group_index and entry content (update group modified time)
 * INPUT    : group_index, entry->tacacs_server_index, sys_time
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time);



#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsTacacsPlusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsTacacsPlusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsTacacsPlusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the tacacs entry is valid or not according to input params
 * INPUT    : tacacs_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsTacacsPlusEntryValid(UI16_T tacacs_index, AAA_ClientType_T client_type, UI32_T ifindex);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */




#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the accounting update interval (minutes)
 * INPUT    : update_interval
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUpdateInterval(UI32_T update_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccUpdateInterval();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntryFilterList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next dot1x accounting entry's ifindex with specific list_name by ifindex.
 * INPUT    : ifindex, list_name
 * OUTPUT   : ifindex
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntryFilterList(UI32_T *ifindex, const char *list_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntryFilterGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next dot1x accounting entry's ifindex with specific group_name by ifindex.
 * INPUT    : ifindex, group_name
 * OUTPUT   : ifindex
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntryFilterGroup(UI32_T *ifindex, const char *group_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccDot1xEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by ifindex.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccDot1xEntryInterface(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccDot1xEntry(UI32_T ifindex);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccDot1xEntryListName
 * ------------------------------------------------------------------------
 * PURPOSE  : set list-name by ifindex.
 * INPUT    : ifindex, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccDot1xEntryListName(UI32_T ifindex, char *list_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAccExecEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntryFilterList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next exec accounting entry's exec_type with specific list_name by exec_type.
 * INPUT    : exec_type, list_name
 * OUTPUT   : exec_type
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntryFilterList(AAA_ExecType_T *exec_type, const char *list_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntryFilterGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next exec accounting entry's exec_type with specific group_name by exec_type.
 * INPUT    : exec_type, group_name
 * OUTPUT   : exec_type
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntryFilterGroup(AAA_ExecType_T *exec_type, const char *group_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccExecEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccExecEntryInterface(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccExecEntry(const AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccExecEntry(AAA_ExecType_T exec_type);

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAccCommandEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccCommandEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the command accounting entry by specified privilege level
 *            and EXEC type
 * INPUT    : entry_p->priv_lvl
 * OUTPUT   : entry_p
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccCommandEntryInterface(AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup the command accounting entry by specified privilege level
 *            and EXEC type
 * INPUT    : privilege, list name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore to change the configure mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the command accounting entry by specified privilege level
 *            and EXEC type
 * INPUT    : priv_lvl
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccCommandEntry(UI32_T priv_lvl, AAA_AccExecType_T exec_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p);

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAccListEntryInterface
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccListEntryInterface(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting list by name.
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterface(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterfaceByIndex(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterfaceByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by ifindex.
 * INPUT    : ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterfaceByPort(UI32_T ifindex, AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntryInterface(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry
 * INPUT    : entry->list_name, entry->group_name, entry->client_type, entry->working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntry(const AAA_AccListEntryInterface_T *entry);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryName
 *------------------------------------------------------------------------
 * PURPOSE  : set the list_name by list_index.
 * INPUT    : list_index, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntryName(UI16_T list_index, char* list_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : set the group_name by list_index
 * INPUT    : list_index, group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntryGroupName(UI16_T list_index, char* group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the working mode by list_index.
 * INPUT    : list_index, client_type
 * OUTPUT   : None
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : None
 *------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccListEntryClientType(UI16_T list_index, AAA_ClientType_T client_type);

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryPrivilege
 *------------------------------------------------------------------------
 * PURPOSE  : set the privilege level by list_index.
 * INPUT    : list_index, priv-lvl
 * OUTPUT   : None
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : None
 *------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccListEntryPrivilege(UI16_T list_index, UI32_T priv_lvl);

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryWokringMode
 *------------------------------------------------------------------------
 * PURPOSE  : set the working mode by list_index.
 * INPUT    : list_index, working_mode
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntryWokringMode(UI16_T list_index, AAA_AccWorkingMode_T working_mode);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetSetAccListEntryStatus
 *------------------------------------------------------------------------
 * PURPOSE  : set the status by list_index.
 * INPUT    : list_index, entry_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetSetAccListEntryStatus(UI16_T list_index, AAA_EntryStatus_T entry_status);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_DestroyAccListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyAccListEntryInterface(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_DestroyAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_DestroyAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by index
 * INPUT    : list_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyAccListEntryByIndex(UI16_T list_index, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle accounting method list
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeAccListEntry(UI16_T list_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : none
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccountingGroupIndex_ByInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : acc_interface
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccountingGroupIndexByInterface(const AAA_AccInterface_T *acc_interface, AAA_QueryGroupIndexResult_T *query_result);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next accounting user by user_index
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryInterface(AAA_AccUserEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryInterfaceFilterByParams
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next accounting user with specific client_type and ifindex by user_index
 * INPUT    : entry->user_index, entry->client_type, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if client_type == DOT1X, ifindex == l_port
 *            if client_tpye == EXEC, ifindex == exec_type
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryInterfaceFilterByParams(AAA_AccUserEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_CreateAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : create an accounting user according to request
 * INPUT    : request, sys_time
 * OUTPUT   : none.
 * RETURN   : user_index, 0 - can't create this user
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
UI16_T AAA_OM_CreateAccUser(const AAA_AccRequest_T *request, UI32_T sys_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle specific user entry from user list
 * INPUT    : user_index (1-based)
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AAA_OM_FreeAccUser(UI16_T user_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUserRadiusStartFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : setup user radius start flag
 * INPUT    : user_index (1-based), start_flag
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUserRadiusStartFlag(UI16_T user_index, BOOL_T start_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUserTacacsStartFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : setup user tacacs start flag
 * INPUT    : user_index (1-based), start_flag
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUserTacacsStartFlag(UI16_T user_index, BOOL_T start_flag);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_QueryAccUserIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified user exist or not
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : none
 * RETURN   : user_index, 0 - not exist
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
UI16_T AAA_OM_QueryAccUserIndex(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQty(UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryInfo(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name and client_type by index.
 * INPUT    : entry->user_index, entry->user_name, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user from specified port by index.
 * INPUT    : entry->user_index, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CollectSettingInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : query setting by user index
 * INPUT    : user_index
 * OUTPUT   : setting_info
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_CollectSettingInfo(UI16_T user_index, AAA_AccSettingInfo_T *setting_info);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CollectSettingInfo_ByAccRequest
 * ---------------------------------------------------------------------
 * PURPOSE  : query setting by AAA_AccRequest_T
 * INPUT    : acc_request
 * OUTPUT   : setting_info
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_CollectSettingInfo_ByAccRequest(const AAA_AccRequest_T *acc_request, AAA_AccSettingInfo_T *setting_info);


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GenerateRequestFromUserInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : generate necessary information for request from user_index
 * INPUT    : user_index, request_type
 * OUTPUT   : request_entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_GenerateRequestFromUserInfo(UI16_T user_index, AAA_AccRequestType_T request_type, AAA_AccRequest_T *request_entry);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GenerateSerialNumber
 * ---------------------------------------------------------------------
 * PURPOSE  : generate a new serial number
 * INPUT    : none
 * OUTPUT   : sn_p
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_GenerateSerialNumber(UI32_T *sn_p);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_ConvertToExecType
 * ---------------------------------------------------------------------
 * PURPOSE  : convert index to exec_type
 * INPUT    : exec_id
 * OUTPUT   : exec_type
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_ConvertToExecType(UI32_T exec_id, AAA_ExecType_T *exec_type);

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_ConvertToIfindex
 * ---------------------------------------------------------------------
 * PURPOSE  : convert ifindex to exec_type
 * INPUT    : exec_id
 * OUTPUT   : exec_type
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_ConvertToIfindex(AAA_ExecType_T exec_type, UI32_T *ifindex);


#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorizationGroupIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if client_type == DOT1X, super_ifindex == l_port
 *            if client_tpye == EXEC, super_ifindex == exec_type
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorizationGroupIndex(UI32_T priv_lvl, AAA_ClientType_T client_type, UI32_T super_ifindex, AAA_QueryGroupIndexResult_T *query_result);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAuthorExecEntry(AAA_ExecType_T exec_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0')
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : none
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the authorization list by name.
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry
 * INPUT    : entry->list_name, entry->group_name, entry->client_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyAuthorListEntry(const char *name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle accounting method list
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeAuthorListEntry(UI16_T list_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorExecEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorExecEntryInterface(AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : SYS_TYPE_GET_RUNNING_CFG_SUCCESS, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE, or
 *            SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES    : 1. This function shall only be invoked by CLI to save the
 *               "running configuration" to local or remote files.
 *            2. Since only non-default configuration will be saved, this
 *               function shall return non-default structure for each field for the device.
 *-------------------------------------------------------------------------*/
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAuthorListEntryInterface
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorListEntryInterfaceByIndex(AAA_AuthorListEntryInterface_T *entry);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryName
 *------------------------------------------------------------------------
 * PURPOSE  : set the list_name by list_index.
 * INPUT    : list_index, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryName(UI16_T list_index, char* list_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : set the group_name by list_index
 * INPUT    : list_index, group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryGroupName(UI16_T list_index, char* group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the working mode by list_index.
 * INPUT    : list_index, client_type
 * OUTPUT   : None
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : None
 *------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAuthorListEntryClientType(UI16_T list_index, const AAA_ListType_T *list_type);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryStatus
 *------------------------------------------------------------------------
 * PURPOSE  : set the status by list_index.
 * INPUT    : list_index, entry_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryStatus(UI16_T list_index, AAA_EntryStatus_T entry_status);

#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
AAA_ApiUpdateResult_T AAA_OM_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable command authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_OM_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type);
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


#endif /* SYS_CPNT_AAA == TRUE */

#endif /* End of AAA_OM_H */

