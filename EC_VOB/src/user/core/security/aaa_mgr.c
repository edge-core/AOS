/* MODULE NAME: aaa_mgr.c
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2004/03/024 : mfhorng      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sys_module.h"
#include "sysfun.h"
#include "sys_time.h"
#include "aaa_mgr.h"
#include "aaa_om.h"
#include "aaa_om_private.h"
#include "radius_mgr.h"

#if (SYS_CPNT_TACACS == TRUE)
#include "tacacs_om_private.h"
#include "tacacs_mgr.h"
#include "tacacs_om.h"
#endif /* SYS_CPNT_TACACS */

#include "sys_bld.h"
#include "security_backdoor.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AAA_COOPERATE_WITH_MULTI_RADIUS_HOST          TRUE /* let aaa can compile with single radius host version */

#if (SYS_CPNT_TACACS == TRUE)
#define AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST     TRUE /* let aaa can compile with single tacacs host version */
#else
#define AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST     FALSE
#endif

#define AAA_MGR_DEBUG_MODE   /* define AAA_MGR_DEBUG_MODE
                                to build AAA_MGR with DEBUG version
                                And let following macros print debug messages
                                */


/* MACRO FUNCTION DECLARATIONS
 */
#ifdef AAA_MGR_DEBUG_MODE
#include "backdoor_mgr.h"
#define AAA_MGR_TRACE(fmt, args...)                         \
    {                                                       \
        if(SECURITY_BACKDOOR_IsOn(aaa_mgr_backdoor_reg_no)) \
            {BACKDOOR_MGR_Printf(fmt"\r\n", ##args);}       \
    }
#else
#define AAA_MGR_TRACE(fmt, args...)        ((void)0)
#endif

#define AAA_MGR_USE_CSC(a)
#define AAA_MGR_RELEASE_CSC()
#define AAA_MGR_USE_CSC_WITHOUT_RETURN_VALUE()

#define AAA_MGR_REQUEST_TYPE_STRING(type) \
            (\
            ((type)==AAA_ACC_START)?                        "start":\
            ((type)==AAA_ACC_STOP)?                         "stop":\
            ((type)==AAA_ACC_CMD_START)?                    "start":\
            ((type)==AAA_ACC_CMD_STOP)?                     "stop":"unknow"\
            )

#define AAA_MGR_USE_CSC_CHECK_OPER_MODE(RET_VAL) \
    AAA_MGR_USE_CSC(RET_VAL); \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
       AAA_MGR_RELEASE_CSC(); \
       return (RET_VAL); \
    }

#define AAA_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE() \
    AAA_MGR_USE_CSC_WITHOUT_RETURN_VALUE(); \
    if (SYSFUN_GET_CSC_OPERATING_MODE() != SYS_TYPE_STACKING_MASTER_MODE) { \
        AAA_MGR_RELEASE_CSC(); \
        return; \
    }

#define AAA_MGR_RETURN_AND_RELEASE_CSC(RET_VAL) { \
        AAA_MGR_RELEASE_CSC(); \
        return (RET_VAL); \
    }

#define AAA_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE() { \
        AAA_MGR_RELEASE_CSC(); \
        return; \
    }

#define AAA_MGR_LOCK()
#define AAA_MGR_UNLOCK()

/* DATA TYPE DECLARATIONS
 */
#if (SYS_CPNT_ACCOUNTING == TRUE)
typedef enum AAA_AccQueryAccUserType_E
{
    AAA_ACC_QUERY_BY_INDEX,
    AAA_ACC_QUERY_BY_NAME_TYPE,
    AAA_ACC_QUERY_BY_TYPE,
    AAA_ACC_QUERY_BY_PORT,
} AAA_AccQueryAccUserType_T;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void AAA_MGR_SetConfigSettingToDefault();

#if (SYS_CPNT_ACCOUNTING == TRUE)
static void AAA_MGR_HandleGroupChangeEvent(AAA_AccRequestType_T request_type, const char *group_name);
static void AAA_MGR_HandleListChangeEvent(AAA_ClientType_T client_type, AAA_AccRequestType_T request_type, const char *list_name);

static void AAA_MGR_TryToSendStartRequest(AAA_ClientType_T client_type, UI32_T super_ifindex);
static void AAA_MGR_TryToSendStopRequest(AAA_ClientType_T client_type, UI32_T super_ifindex);

static void AAA_MGR_StopAllAccountingRequests();
static BOOL_T AAA_MGR_QueryNextAccUser(AAA_AccUserInfoInterface_T *entry, AAA_AccQueryAccUserType_T type);
#endif /* SYS_CPNT_ACCOUNTING == TRUE */


/* STATIC VARIABLE DECLARATIONS
 */


SYSFUN_DECLARE_CSC  /* declare variables used for Transition mode */

#if (SYS_CPNT_ACCOUNTING == TRUE)
static AAA_AccCpntAsyncNotify_Callback_T   accounting_cpnt_callback[AAA_ACC_CPNT_SUPPORT_NUMBER];
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#ifdef AAA_MGR_DEBUG_MODE
static UI32_T   aaa_mgr_backdoor_reg_no;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */


SYS_TYPE_Stacking_Mode_T AAA_MGR_CurrentOperationMode()
{
    return SYSFUN_GET_CSC_OPERATING_MODE();
}

void AAA_MGR_EnterMasterMode()
{
    /* TO DO: insert code at here */
    AAA_MGR_TRACE("\r\n[AAA_MGR_EnterMasterMode] WARNING ! the running version is DEBUG MODE !");


    /* change current operation mode */
    SYSFUN_ENTER_MASTER_MODE();
}

void AAA_MGR_EnterSlaveMode()
{
    /* TO DO: insert code at here */


    /* change current operation mode */
    SYSFUN_ENTER_SLAVE_MODE();
}

void AAA_MGR_SetTransitionMode()
{
    /* set transition flag to prevent calling request */
    SYSFUN_SET_TRANSITION_MODE();
}

void AAA_MGR_EnterTransitionMode()
{
    /* wait other callers leave */
    SYSFUN_ENTER_TRANSITION_MODE();

    /* TO DO: insert code at here */

#if (SYS_CPNT_ACCOUNTING == TRUE)
    AAA_MGR_StopAllAccountingRequests();
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    AAA_MGR_SetConfigSettingToDefault();
}

BOOL_T AAA_MGR_Initiate_System_Resources()
{
    if(AAA_OM_CreatSem() == FALSE)
        return FALSE;

#ifdef AAA_MGR_DEBUG_MODE
    SECURITY_BACKDOOR_Register("aaa_mgr", &aaa_mgr_backdoor_reg_no);
#endif

    /* must care about the Initiate_System_Resources() relationships between aaa and radius/tacacs */
#if (SYS_CPNT_ACCOUNTING == TRUE)
    memset(accounting_cpnt_callback, 0, sizeof(AAA_AccCpntAsyncNotify_Callback_T) * AAA_ACC_CPNT_SUPPORT_NUMBER);
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    return TRUE;
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusGroupEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusGroupEntryByIndex(AAA_RadiusGroupEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetRadiusGroupEntryInterfaceByIndex(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusGroupEntry(entry, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, entry->group_name); /* should unlock before call this func */
    }
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DestroyRadiusGroupEntry(name, warning);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, name); /* should unlock before call this func */
    }
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetRadiusGroupEntryModifiedTime(group_index, modified_time);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusGroupEntry(entry, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, entry->group_name); /* should unlock before call this func */
    }
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DestroyTacacsPlusGroupEntry(name, warning);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, name); /* should unlock before call this func */
    }
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetTacacsPlusGroupEntryModifiedTime(group_index, modified_time);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : radius_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetRadiusEntryInterface(group_index, entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusEntryByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th radius entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusEntryByOrder(UI16_T group_index, UI16_T order, AAA_RadiusEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetRadiusEntryInterfaceByOrder(group_index, order, entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:
 *--------------------------------AAA_MGR_GetRadiusEntryOrder-----------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetRadiusEntryOrder(group_index, radius_index, order);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntry
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
BOOL_T AAA_MGR_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
#if (AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE)

    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* must call radius API before AAA_MGR_LOCK()
       because radius call AAA also
     */
    if (RADIUS_MGR_IsServerHostValid(entry->radius_server_index) == FALSE)
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetRadiusEntry] bad server_index(%lu)", entry->radius_server_index);
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusEntry(group_index, entry, warning, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusEntryJoinDefaultRadiusGroup(radius_server_index, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusEntryDepartDefaultRadiusGroup(radius_server_index, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address)
{
#if (AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE)

    AAA_RadiusEntryInterface_T  entry;

    if (RADIUS_MGR_LookupServerIndexByIpAddress(ip_address, &entry.radius_server_index) == FALSE)
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetRadiusEntryByIpAddress] bad server ip address(%lu)", ip_address);
        return FALSE;
    }

    return AAA_MGR_SetRadiusEntry(group_index, &entry, NULL);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and entry content
 * INPUT    : group_index, entry->radius_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_DestroyRadiusEntry(group_index, entry, warning, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyRadiusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning)
{
#if (AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE)

    AAA_RadiusEntryInterface_T  entry;

    if (FALSE == RADIUS_MGR_LookupServerIndexByIpAddress(ip_address, &entry.radius_server_index))
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_DestroyRadiusEntryByIpAddress] can't find specified server host");
        return FALSE;
    }

    return AAA_MGR_DestroyRadiusEntry(group_index, &entry, warning);

#else
    return TRUE;
#endif /* AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE */
}


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_IsRadiusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_IsRadiusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_IsRadiusGroupValid(group_index, client_type, ifindex);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_IsRadiusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the radius entry is valid or not according to input params
 * INPUT    : radius_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_IsRadiusEntryValid(UI16_T radius_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_IsRadiusEntryValid(radius_index, client_type, ifindex);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusEntryByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th tacacs plus entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusEntryByOrder(UI16_T group_index, UI16_T order, AAA_TacacsPlusEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetTacacsPlusEntryInterfaceByOrder(group_index, order, entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry's order in group
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusEntryOrder(UI16_T group_index, UI16_T tacacs_index, UI16_T *order)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetTacacsPlusEntryOrder(group_index, tacacs_index, order);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntry
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
BOOL_T AAA_MGR_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
#if (AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST  == TRUE)

    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* must call tacacs API before AAA_MGR_LOCK()
       because tacacs call AAA also
     */
    if (TACACS_OM_IsServerHostValid(entry->tacacs_server_index) == FALSE)
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetTacacsPlusEntry] bad server_index(%lu)", entry->tacacs_server_index);
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusEntry(group_index, entry, warning, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST  == TRUE */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry join default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(tacacs_server_index, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry depart default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(tacacs_server_index, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry by group_index
 * INPUT    : group_index, ip_address
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group must exist, server must exist
 *            create specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address)
{
#if (AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE)

    AAA_TacacsPlusEntryInterface_T  entry;

    if (TACACS_OM_LookupServerIndexByIpAddress(ip_address, &entry.tacacs_server_index) == FALSE)
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetTacacsPlusEntryByIpAddress] bad server ip address(%lu)", ip_address);
        return FALSE;
    }

    return AAA_MGR_SetTacacsPlusEntry(group_index, &entry, NULL);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and entry content
 * INPUT    : group_index, entry->tacacs_server_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_DestroyTacacsPlusEntry(group_index, entry, warning, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyTacacsPlusEntryByIpAddress
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus entry by group_index and ip_address
 * INPUT    : group_index, ip_address
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete any entry from the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning)
{
#if (AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE)

    AAA_TacacsPlusEntryInterface_T  entry;

    if (FALSE == TACACS_OM_LookupServerIndexByIpAddress(ip_address, &entry.tacacs_server_index))
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_DestroyTacacsPlusEntryByIpAddress] can't find specified server host");
        return FALSE;
    }

    return AAA_MGR_DestroyTacacsPlusEntry(group_index, &entry, warning);

#else
    return TRUE;
#endif /* AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE */
}


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_IsTacacsPlusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_IsTacacsPlusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_IsTacacsPlusGroupValid(group_index, client_type, ifindex);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_IsTacacsPlusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the tacacs plus entry is valid or not according to input params
 * INPUT    : tacacs_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_IsTacacsPlusEntryValid(UI16_T tacacs_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_IsTacacsPlusEntryValid(tacacs_index, client_type, ifindex);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the accounting update interval (minutes)
 * INPUT    : update_interval
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccUpdateInterval(UI32_T update_interval)
{
    BOOL_T      ret;
    UI32_T      tmp, max_retransmission_timeout; /* seconds */

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    tmp = max_retransmission_timeout = 0;

    /* must call radius / tacacs API before AAA_MGR_LOCK()
       because radius / tacacs call AAA also
     */

#if (AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE)
    if (FALSE == RADIUS_MGR_GetServerHostMaxRetransmissionTimeout(&tmp))
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetAccUpdateInterval] failed to call RADIUS_MGR_GetServerHostMaxRetransmissionTimeout");
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (max_retransmission_timeout < tmp)
        max_retransmission_timeout = tmp;
#endif /* AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE */

#if (AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE)
    if (FALSE == TACACS_OM_GetServerHostMaxRetransmissionTimeout(&tmp))
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetAccUpdateInterval] failed to call TACACS_OM_GetServerHostMaxRetransmissionTimeout");
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (max_retransmission_timeout < tmp)
        max_retransmission_timeout = tmp;
#endif /* AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE */

    AAA_MGR_LOCK();

    if ((update_interval * 60) < (max_retransmission_timeout * 2))
    {
        AAA_MGR_TRACE("\r\n[AAA_MGR_SetAccUpdateInterval] update_interval * 60 must large than %lu",
            max_retransmission_timeout * 2);

        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_SetAccUpdateInterval(update_interval);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccUpdateInterval()
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAccUpdateInterval();

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextRunningAccDot1xEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = AAA_OM_GetNextRunningAccDot1xEntry(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetNextAccDot1xEntry(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry)
{
    BOOL_T  ret;

    AAA_ApiUpdateResult_T   result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    result = AAA_OM_SetAccDot1xEntry(entry);

    AAA_MGR_UNLOCK();

    switch (result)
    {
        case AAA_API_UPDATE_CHANGE:
            ret = TRUE;
            /* if setting complete, send those users' requests to accounting component */
            AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_DOT1X, entry->ifindex);
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccDot1xEntry(UI32_T ifindex)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAccDot1xEntry(ifindex);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        /* if setting is borken, send those users' stop requests to accounting component */
        AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRunningAccExecEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetRunningAccExecEntry(AAA_AccExecEntry_T *entry)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = AAA_OM_GetRunningAccExecEntry(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextRunningAccExecEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAccExecEntry(AAA_AccExecEntry_T *entry)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = AAA_OM_GetNextRunningAccExecEntry(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccExecEntry(const AAA_AccExecEntry_T *entry)
{
    BOOL_T  ret;

    AAA_ApiUpdateResult_T   result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    result = AAA_OM_SetAccExecEntry(entry);

    AAA_MGR_UNLOCK();

    switch (result)
    {
        case AAA_API_UPDATE_CHANGE:
            ret = TRUE;
            /* if setting complete, send those users' requests to accounting component */
            AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_EXEC, entry->exec_type);
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccExecEntry(AAA_ExecType_T exec_type)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAccExecEntry(exec_type);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        /* if setting is borken, send those users' stop requests to accounting component */
        AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_EXEC, exec_type);
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRunningAccCommandEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = AAA_OM_GetRunningAccCommandEntry(entry_p);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p)
{
    BOOL_T  ret;
    UI32_T  i;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if(index == NULL || entry_p == NULL)
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);

    i = *index;
    entry_p->priv_lvl  = i / AAA_EXEC_TYPE_SUPPORT_NUMBER;
    entry_p->exec_type =(i % AAA_EXEC_TYPE_SUPPORT_NUMBER) + 1; /* because AAA_ExecType_T begin with 1 */
    ret = AAA_OM_GetAccCommandEntryInterface(entry_p);
    if(ret == TRUE)
    {
        *index = ++i;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccCommandEntry(AAA_AccCommandEntry_T *entry_p)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAccCommandEntryInterface(entry_p);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command accounting by specified privilege level
 * INPUT    : privilege, list name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore to change the configure mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p)
{
    BOOL_T  ret;

    AAA_ApiUpdateResult_T   result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    result = AAA_OM_SetAccCommandEntry(entry_p);

    AAA_MGR_UNLOCK();

    ret = (result == AAA_API_UPDATE_FAILED) ? FALSE : TRUE;

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the command accounting entry on specified privilege level
 *            and EXEC type
 * INPUT    : priv_lvl
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccCommandEntry(const AAA_AccCommandEntry_T *entry_p)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAccCommandEntry(entry_p->priv_lvl, entry_p->exec_type);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting list by name and client_type
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccListEntry(AAA_AccListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAccListEntryInterface(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by index.
 * INPUT    : list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccListEntryByIndex(AAA_AccListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAccListEntryInterfaceByIndex(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccListEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by ifindex.
 * INPUT    : ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccListEntryByPort(UI32_T ifindex, AAA_AccListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAccListEntryInterfaceByPort(ifindex, entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type, entry->working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccListEntry(const AAA_AccListEntryInterface_T *entry)
{
    BOOL_T  ret, old_flag;

    AAA_AccListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    if (NULL == entry)
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if((strlen((char *)entry->list_name) < MINSIZE_aaaMethodName) ||
        (strlen((char *)entry->list_name) > MAXSIZE_aaaMethodName))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if((strlen((char *)entry->group_name) < MINSIZE_aaaMethodGroupName) ||
        (strlen((char *)entry->group_name) > MAXSIZE_aaaMethodGroupName))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if((entry->client_type < AAA_CLIENT_TYPE_DOT1X) ||
        (entry->client_type > AAA_CLIENT_TYPE_NUMBER))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if(entry->working_mode != ACCOUNTING_START_STOP)
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    old_entry.client_type = entry->client_type;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    old_entry.priv_lvl    = entry->priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    strncpy((char *)old_entry.list_name, (char *)entry->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    old_flag = AAA_OM_GetAccListEntryInterface(&old_entry);

    ret = AAA_OM_SetAccListEntry(entry);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        if ((TRUE == old_flag) && (strcmp((char *)old_entry.group_name, (char *)entry->group_name) != 0)) /* group_name changed */
        {
            /* we sould stop all users on old server_group, then start them on new server_group */
            AAA_MGR_HandleListChangeEvent(entry->client_type, AAA_ACC_STOP, entry->list_name); /* should unlock before call this func */
        }

        AAA_MGR_HandleListChangeEvent(entry->client_type, AAA_ACC_START, entry->list_name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_DestroyAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DestroyAccListEntry(name, client_type, warning);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        AAA_MGR_HandleListChangeEvent(client_type, AAA_ACC_STOP, name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_DestroyAccListEntry2
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyAccListEntry2(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DestroyAccListEntryInterface(entry, warning);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        AAA_MGR_HandleListChangeEvent(entry->client_type, AAA_ACC_STOP, entry->list_name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_DestroyAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by list_index
 * INPUT    : list_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyAccListEntryByIndex(UI16_T list_index, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_AccListEntryInterface_T list_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    list_entry.list_index = list_index;
    if (FALSE == AAA_OM_GetAccListEntryInterfaceByIndex(&list_entry))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_DestroyAccListEntryByIndex(list_index, warning);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        AAA_MGR_HandleListChangeEvent(list_entry.client_type, AAA_ACC_STOP, list_entry.list_name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p)
{
    BOOL_T      ret;

    AAA_ApiUpdateResult_T   update_result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    if (NULL == entry_p)
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    update_result = AAA_OM_SetDefaultList(entry_p);

    AAA_MGR_UNLOCK();

    ret = TRUE;
    switch (update_result)
    {
        case AAA_API_UPDATE_CHANGE:
            /* we sould stop all users on old server_group, then start them on new server_group */
            AAA_MGR_HandleListChangeEvent(entry_p->client_type, AAA_ACC_STOP, SYS_DFLT_AAA_METHOD_LIST_NAME); /* should unlock before call this func */
            AAA_MGR_HandleListChangeEvent(entry_p->client_type, AAA_ACC_START, SYS_DFLT_AAA_METHOD_LIST_NAME); /* should unlock before call this func */
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccountingGroupIndex_ByInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccountingGroupIndex_ByInterface(const AAA_AccInterface_T *entry_p, AAA_QueryGroupIndexResult_T *query_result_p)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAccountingGroupIndexByInterface(entry_p, query_result_p);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccountingGroupIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccountingGroupIndex(AAA_ClientType_T client_type, UI32_T ifindex, AAA_QueryGroupIndexResult_T *query_result)
{
    BOOL_T  ret;

    AAA_AccInterface_T   acc_interface;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    memset(&acc_interface, 0, sizeof(AAA_AccInterface_T));
    acc_interface.client_type = client_type;
    acc_interface.ifindex     = ifindex;

    ret = AAA_OM_GetAccountingGroupIndexByInterface(&acc_interface, query_result);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_Register_AccComponent_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : register the accounting component function
 * INPUT    : cpnt_type, call_back_func
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AAA_MGR_Register_AccComponent_Callback(AAA_AccCpntType_T cpnt_type, AAA_AccCpntAsyncNotify_Callback_T call_back_func)
{
//    AAA_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();

    accounting_cpnt_callback[cpnt_type] = call_back_func;

//    AAA_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_AsyncAccountingRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : ifindex, client_type, request_type, identifier,
 *            user_name       --  User name (terminated with '\0')
 *            call_back_func  --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_AsyncAccountingRequest(AAA_AccRequest_T *request)
{
    UI32_T                  sys_time;
    AAA_AccSettingInfo_T    setting_info;
    UI16_T                  user_index;

    AAA_AccCpntAsyncNotify_Callback_T   callback_func;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    AAA_ClientType_T orig_client_type = AAA_CLIENT_TYPE_NUMBER;
#endif

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    if (NULL == request)
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    switch (request->request_type)
    {
        case AAA_ACC_START:

            AAA_MGR_TRACE("\r\n[AAA_MGR_AsyncAccountingRequest] demand start accounting");

            SYS_TIME_GetRealTimeBySec(&sys_time);
            user_index = AAA_OM_CreateAccUser(request, sys_time);
            if (0 == user_index)
            {
                /* if there is an user have the same value of
                   (request->ifindex, request->user_name, request->client_type) => failed
                   if beyond the max number of users => failed
                 */
                AAA_MGR_TRACE("[%s] failed to create user account", __FUNCTION__);
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        case AAA_ACC_STOP:

            AAA_MGR_TRACE("\r\n[AAA_MGR_AsyncAccountingRequest] demand stop accounting");
            user_index = AAA_OM_QueryAccUserIndex(request->ifindex, request->user_name, request->client_type);
            if (0 == user_index)
            {
                AAA_MGR_TRACE("[%s] failed to query user account", __FUNCTION__);
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_ACC_CMD_START:
        case AAA_ACC_CMD_STOP:
            {
                AAA_MGR_TRACE("Demanded a command %s accounting, cmd=%s",
                    AAA_MGR_REQUEST_TYPE_STRING(request->request_type), request->command);

                /* command accounting support exec only
                 */
                user_index = AAA_OM_QueryAccUserIndex(request->ifindex, request->user_name, request->client_type);
                if (0 != user_index)
                {
                    AAA_AccUserInfoInterface_T user_entry;
                    user_entry.user_index = user_index;
                    if (TRUE == AAA_OM_GetNextAccUserEntryInfo(&user_entry))
                    {
                        request->auth_by_whom   = user_entry.auth_by_whom;
                        request->auth_privilege = user_entry.auth_privilege;
                    }
                }
                else
                {
                    AAA_MGR_TRACE("Can't find user, ifindex=%ld, name=%s, EXEC",
                        request->ifindex, request->user_name);
                }
            }

            break;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

        default:
            AAA_MGR_TRACE("[%s] unknown request type(%d)", __FUNCTION__, request->request_type);

            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    AAA_MGR_TRACE("user_name(%s), client_type(%s), ifindex(%lu)",
        request->user_name,
        (request->client_type == AAA_CLIENT_TYPE_DOT1X)?"dot1x":
        (request->client_type == AAA_CLIENT_TYPE_EXEC)?"exec":"??",
        request->ifindex
        )

    /* for command accounting, need change the client_type to COMMANDS for collect
     * some information from OM
     */
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    if (    (AAA_ACC_CMD_START == request->request_type)
         || (AAA_ACC_CMD_STOP  == request->request_type)
        )
    {
        orig_client_type = request->client_type;
        request->client_type = AAA_CLIENT_TYPE_COMMANDS;
    }
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

    if (FALSE == AAA_OM_CollectSettingInfo_ByAccRequest(request, &setting_info))
    {
        if (AAA_ACC_STOP == request->request_type)
            AAA_OM_FreeAccUser(user_index);
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    if (    (AAA_ACC_CMD_START == request->request_type)
         || (AAA_ACC_CMD_STOP  == request->request_type)
        )
    {
        request->client_type = orig_client_type;
    }
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

    if (AAA_ACC_STOP == request->request_type)
        AAA_OM_FreeAccUser(user_index);

    request->current_working_mode = setting_info.working_mode;
    switch (setting_info.group_type)
    {
        case GROUP_RADIUS:
            callback_func = accounting_cpnt_callback[AAA_ACC_CPNT_RADIUS];
            AAA_OM_SetAccUserRadiusStartFlag(user_index, (AAA_ACC_START == request->request_type));
            break;

        case GROUP_TACACS_PLUS:
            callback_func = accounting_cpnt_callback[AAA_ACC_CPNT_TACACS_PLUS];
            AAA_OM_SetAccUserTacacsStartFlag(user_index, (AAA_ACC_START == request->request_type));

            /* Generate a new serial number if request type are AAA_ACC_START or AAA_ACC_CMD_START
             */
            if (    (AAA_ACC_START == request->request_type)
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
                ||  (AAA_ACC_CMD_START == request->request_type)
#endif
               )
            {
                if (FALSE == AAA_OM_GenerateSerialNumber(&request->serial_number))
                {
                    AAA_MGR_TRACE("%s", "Fail to get a new serial number");
                    AAA_MGR_UNLOCK();
                    AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
                }
            }

            break;

        case GROUP_UNKNOWN:
        default:
            callback_func = NULL;
            break;
    }

    AAA_MGR_UNLOCK();

    if (NULL != callback_func)
    {
        /* must call radius/tacacs+ API after AAA_MGR_UNLOCK()
           because radius/tacacs+ call AAA also
         */
        callback_func(request);
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(TRUE);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_ConvertToExecType
 * ---------------------------------------------------------------------
 * PURPOSE  : convert index to exec_type
 * INPUT    : exec_id
 * OUTPUT   : exec_type
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_MGR_ConvertToExecType(UI32_T exec_id, AAA_ExecType_T *exec_type)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_ConvertToExecType(exec_id, exec_type);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the moethod_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
 *              method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetMethodName(UI16_T method_index, char* method_name)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret, old_flag;

    AAA_AccListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAccListEntryInterfaceByIndex(&old_entry);

    ret = AAA_OM_SetAccListEntryName(method_index, method_name);

    AAA_MGR_UNLOCK();

    if ((TRUE == ret) && (TRUE == old_flag))
    {
        /* we sould stop all users on old method-list name, then start them on new method-list name */
        AAA_MGR_HandleListChangeEvent(old_entry.client_type, AAA_ACC_STOP, old_entry.list_name); /* should unlock before call this func */
        AAA_MGR_HandleListChangeEvent(old_entry.client_type, AAA_ACC_START, method_name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the group_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetMethodGroupName(UI16_T method_index, char* method_group_name)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret, old_flag;

    AAA_AccListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAccListEntryInterfaceByIndex(&old_entry);

    ret = AAA_OM_SetAccListEntryGroupName(method_index, method_group_name);

    AAA_MGR_UNLOCK();

    if ((TRUE == ret) && (TRUE == old_flag))
    {
        /* we sould stop all users on old server_group, then start them on new server_group */
        AAA_MGR_HandleListChangeEvent(old_entry.client_type, AAA_ACC_STOP, old_entry.list_name); /* should unlock before call this func */
        AAA_MGR_HandleListChangeEvent(old_entry.client_type, AAA_ACC_START, old_entry.list_name); /* should unlock before call this func */
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodClientType(UI16_T method_index, AAA_ClientType_T client_type)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret, old_flag;

    AAA_ApiUpdateResult_T           update_result;
    AAA_AccListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAccListEntryInterfaceByIndex(&old_entry);

    update_result = AAA_OM_SetAccListEntryClientType(method_index, client_type);

    AAA_MGR_UNLOCK();

    ret = TRUE;
    switch (update_result)
    {
        case AAA_API_UPDATE_CHANGE:
            if (TRUE == old_flag)
            {
                /* we sould stop all users on old client_type, then start them on new client_type */
                AAA_MGR_HandleListChangeEvent(old_entry.client_type, AAA_ACC_STOP, old_entry.list_name); /* should unlock before call this func */
                AAA_MGR_HandleListChangeEvent(client_type, AAA_ACC_START, old_entry.list_name); /* should unlock before call this func */
            }
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodPrivilege
 *------------------------------------------------------------------------
 * PURPOSE  : set the privilege level by list_index.
 * INPUT    : method_index, priv_lvl
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodPrivilege(UI16_T method_index, UI32_T priv_lvl)
{
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)

    BOOL_T  ret, old_flag;

    AAA_ApiUpdateResult_T           update_result;
    AAA_AccListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAccListEntryInterfaceByIndex(&old_entry);

    update_result = AAA_OM_SetAccListEntryPrivilege(method_index, priv_lvl);

    AAA_MGR_UNLOCK();

    ret = TRUE;
    switch (update_result)
    {
        case AAA_API_UPDATE_CHANGE:
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the mode into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_mode    -- VAL_aaaMethodMode_start_stop
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetMethodMode(UI16_T method_index, UI8_T method_mode)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret;
    AAA_AccWorkingMode_T working_mode;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    /* currently, only support start-stop */
    switch (method_mode)
    {
        case VAL_aaaAccountMethodMode_start_stop:
            working_mode = ACCOUNTING_START_STOP;
            break;

        default:
            AAA_MGR_TRACE("\r\n[AAA_MGR_SetMethodMode] bad method_mode(%d)", method_mode);
            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_SetAccListEntryWokringMode(method_index, working_mode);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the status into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
                method_status  -- Set to 1 to initiate the aaaMethodTable, 2 to destroy the table.
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetMethodStatus(UI16_T method_index, UI8_T method_status)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret;

    AAA_EntryStatus_T   entry_status;

    AAA_AccListEntryInterface_T list_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    switch (method_status)
    {
        case VAL_aaaAccountMethodStatus_valid: /* create */
            entry_status = AAA_ENTRY_READY;
            break;

        case VAL_aaaAccountMethodStatus_invalid: /* destroy */
            entry_status = AAA_ENTRY_DESTROYED;

            list_entry.list_index = method_index;
            if (FALSE == AAA_OM_GetAccListEntryInterfaceByIndex(&list_entry)) /* keep it in mgr */
            {
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        default:
            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_SetSetAccListEntryStatus(method_index, entry_status);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
    {
        if (AAA_ENTRY_READY == entry_status)
        {
            list_entry.list_index = method_index;
            if (TRUE == AAA_OM_GetAccListEntryInterfaceByIndex(&list_entry))
            {
                AAA_MGR_HandleListChangeEvent(list_entry.client_type, AAA_ACC_START, list_entry.list_name);
            }
        }
        else if (AAA_ENTRY_DESTROYED == entry_status)
        {
            AAA_MGR_HandleListChangeEvent(list_entry.client_type, AAA_ACC_STOP, list_entry.list_name);
        }
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}
#endif /* SYS_CPNT_ACCOUNTING == TRUE */





/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetRadiusGroupName(UI16_T radius_group_index, char* radius_group_name)
{
    BOOL_T  ret, old_flag;
    UI32_T  sys_time;

    AAA_RadiusGroupEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.group_index = radius_group_index;
    old_flag = AAA_OM_GetRadiusGroupEntryInterfaceByIndex(&old_entry);

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusGroupEntryName(radius_group_index, radius_group_name, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if ((TRUE == ret) || (TRUE == old_flag))
    {
        /* we sould stop all users on old server_group, then start them on new server_group */
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, old_entry.group_name);
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, radius_group_name);
    }
#endif /*SYS_CPNT_ACCOUNTING == TRUE*/

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
                radius_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetRadiusGroupServerBitMap(UI16_T radius_group_index, UI8_T radius_group_server_bitmap)
{
#if (AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE)

    const UI32_T        bitmap_size = sizeof(radius_group_server_bitmap) * 8;
    const UI32_T        bitmap_mask = 1 << (bitmap_size - 1);

    UI8_T   server_bitmap;
    UI32_T  bit_index, server_index;
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* must call radius API before AAA_MGR_LOCK()
       because radius call AAA also
     */
    server_bitmap = radius_group_server_bitmap;
    for (bit_index = bitmap_size; 0 < bit_index; --bit_index)
    {
        /* check all servers in bitmap are available */
        if (server_bitmap & bitmap_mask)
        {
            server_index = bitmap_size - bit_index + 1;
            if (RADIUS_MGR_IsServerHostValid(server_index) == FALSE)
            {
                AAA_MGR_TRACE("\r\n[AAA_MGR_SetRadiusGroupServerBitMap] bad server_index(%lu)", server_index);
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
        server_bitmap <<= 1;
    }

    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusGroupEntryServerBitMap(radius_group_index, radius_group_server_bitmap, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_RADIUS_HOST == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status
 * INPUT    :   radius_group_index
                radius_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetRadiusGroupStatus(UI16_T radius_group_index, UI32_T radius_group_status)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_EntryStatus_T   entry_status;

    AAA_RadiusGroupEntryInterface_T group_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    switch (radius_group_status)
    {
        case VAL_aaaRadiusGroupStatus_valid: /* create */
            entry_status = AAA_ENTRY_READY;
            break;

        case VAL_aaaRadiusGroupStatus_invalid: /* destroy */
            entry_status = AAA_ENTRY_DESTROYED;

            group_entry.group_index = radius_group_index;
            if (FALSE == AAA_OM_GetRadiusGroupEntryInterfaceByIndex(&group_entry)) /* keep it in mgr */
            {
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        default:
            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetRadiusGroupEntryStatus(radius_group_index, entry_status, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        if (AAA_ENTRY_READY == entry_status)
        {
            group_entry.group_index = radius_group_index;
            if (TRUE == AAA_OM_GetRadiusGroupEntryInterfaceByIndex(&group_entry))
            {
                AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, group_entry.group_name);
            }
        }
        else if (AAA_ENTRY_DESTROYED == entry_status)
        {
            AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, group_entry.group_name);
        }
    }
#endif /*SYS_CPNT_ACCOUNTING == TRUE*/

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupName(UI16_T tacacsplus_group_index, char* tacacsplus_group_name)
{
    BOOL_T  ret, old_flag;
    UI32_T  sys_time;

    AAA_TacacsPlusGroupEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.group_index = tacacsplus_group_index;
    old_flag = AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex(&old_entry);

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusGroupEntryName(tacacsplus_group_index, tacacsplus_group_name, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if ((TRUE == ret) || (TRUE == old_flag))
    {
        /* we sould stop all users on old server_group, then start them on new server_group */
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, old_entry.group_name);
        AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, tacacsplus_group_name);
    }
#endif /*SYS_CPNT_ACCOUNTING == TRUE*/

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupServerBitMap
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set group name into AAA_TacacsPlusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_server_bitmap
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupServerBitMap(UI16_T tacacsplus_group_index, UI8_T tacacsplus_group_server_bitmap)
{
#if (AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE)

    const UI32_T        bitmap_size = sizeof(tacacsplus_group_server_bitmap) * 8;
    const UI32_T        bitmap_mask = 1 << (bitmap_size - 1);

    UI8_T   server_bitmap;
    UI32_T  bit_index, server_index;
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    /* must call tacacs API before AAA_MGR_LOCK()
       because tacacs call AAA also
     */
    server_bitmap = tacacsplus_group_server_bitmap;
    for (bit_index = bitmap_size; 0 < bit_index; --bit_index)
    {
        /* check all servers in bitmap are available */
        if (server_bitmap & bitmap_mask)
        {
            server_index = bitmap_size - bit_index + 1;

            if (TACACS_OM_IsServerHostValid(server_index) == FALSE)
            {
                AAA_MGR_TRACE("\r\n[AAA_MGR_SetRadiusGroupServerBitMap] bad server_index(%lu)", server_index);
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }
        }
        server_bitmap <<= 1;
    }

    AAA_MGR_LOCK();

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusGroupEntryServerBitMap(tacacsplus_group_index, tacacsplus_group_server_bitmap, sys_time);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);

#else
    return FALSE;
#endif /* AAA_COOPERATE_WITH_MULTI_TACACS_PLUS_HOST == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   Set the group status of TacacsPlusGroupTable into AAA_MGR_SetTacacsPlusGroupStatus
 *              by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
                tacacsplus_group_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupStatus(UI16_T tacacsplus_group_index, UI32_T tacacsplus_group_status)
{
    BOOL_T  ret;
    UI32_T  sys_time;

    AAA_EntryStatus_T   entry_status;

    AAA_TacacsPlusGroupEntryInterface_T group_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    switch (tacacsplus_group_status)
    {
        case VAL_aaaTacacsPlusGroupStatus_valid: /* create */
            entry_status = AAA_ENTRY_READY;
            break;

        case VAL_aaaTacacsPlusGroupStatus_invalid: /* destroy */
            entry_status = AAA_ENTRY_DESTROYED;

            group_entry.group_index = tacacsplus_group_index;
            if (FALSE == AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex(&group_entry)) /* keep it in mgr */
            {
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        default:
            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    SYS_TIME_GetRealTimeBySec(&sys_time);
    ret = AAA_OM_SetTacacsPlusGroupEntryStatus(tacacsplus_group_index, entry_status, sys_time);

    AAA_MGR_UNLOCK();

#if (SYS_CPNT_ACCOUNTING == TRUE)
    if (TRUE == ret)
    {
        if (AAA_ENTRY_READY == entry_status)
        {
            group_entry.group_index = tacacsplus_group_index;
            if (TRUE == AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex(&group_entry))
            {
                AAA_MGR_HandleGroupChangeEvent(AAA_ACC_START, group_entry.group_name);
            }
        }
        else if (AAA_ENTRY_DESTROYED == entry_status)
        {
            AAA_MGR_HandleGroupChangeEvent(AAA_ACC_STOP, group_entry.group_name);
        }
    }
#endif /*SYS_CPNT_ACCOUNTING == TRUE*/

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetUpdate(UI32_T update_interval)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    return AAA_MGR_SetAccUpdateInterval(update_interval);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAccountMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-name into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetAccountMethodName(UI32_T ifindex, char* account_method_name)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_SetAccDot1xEntryListName(ifindex, account_method_name);

    AAA_MGR_UNLOCK();

    if (TRUE == ret)
        AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAccountStatus
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-status into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_status
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetAccountStatus(UI32_T ifindex, UI8_T account_status)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    AAA_AccDot1xEntry_T entry;

    switch (account_status)
    {
        case VAL_aaaAccountStatus_valid: /* create */
            entry.ifindex = ifindex;
            strncpy((char *)entry.list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
            entry.list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */
            return AAA_MGR_SetAccDot1xEntry(&entry);

        case VAL_aaaAccountStatus_invalid: /* destroy */
            return AAA_MGR_DisableAccDot1xEntry(ifindex);

        default:
            break;
    }
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    return FALSE;
}

#endif /*SYS_CPNT_ACCOUNTING == TRUE*/
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccUserEntryQty(UI32_T *qty)
{
    BOOL_T  ret;
    UI32_T  sum;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TRUE;
    sum = 0;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    ret &= RADIUS_MGR_GetAccUserEntryQty(qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    ret &= TACACS_OM_GetAccUserEntryQty(qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    if (TRUE == ret)
        *qty = sum;

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    BOOL_T  ret;
    UI32_T  sum;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TRUE;
    sum = 0;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    ret &= RADIUS_MGR_GetAccUserEntryQtyFilterByNameAndType(name, client_type, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    ret &= TACACS_OM_GetAccUserEntryQtyFilterByNameAndType(name, client_type, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    if (TRUE == ret)
        *qty = sum;

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    BOOL_T  ret;
    UI32_T  sum;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TRUE;
    sum = 0;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    ret &= RADIUS_MGR_GetAccUserEntryQtyFilterByType(client_type, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
    ret &= TACACS_OM_GetAccUserEntryQtyFilterByType(client_type, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

    if (TRUE == ret)
        *qty = sum;

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccUserEntryQtyFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty)
{
    BOOL_T  ret;
    UI32_T  sum;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = TRUE;
    sum = 0;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    ret &= RADIUS_MGR_GetAccUserEntryQtyFilterByPort(ifindex, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

#if ((SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE) && (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == TRUE))
    ret &= TACACS_OM_GetAccUserEntryQtyFilterByPort(ifindex, qty);
    if (TRUE == ret)
        sum += *qty;
#endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE && AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == TRUE */

    if (TRUE == ret)
        *qty = sum;

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccUserEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user entry by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccUserEntry(AAA_AccUserInfoInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_MGR_QueryNextAccUser(entry, AAA_ACC_QUERY_BY_INDEX);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccUserEntryFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name and client_type by index.
 * INPUT    : entry->user_index, entry->user_name, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_MGR_QueryNextAccUser(entry, AAA_ACC_QUERY_BY_NAME_TYPE);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_MGR_QueryNextAccUser(entry, AAA_ACC_QUERY_BY_TYPE);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccUserEntryFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user from specified port by index.
 * INPUT    : entry->user_index, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_MGR_QueryNextAccUser(entry, AAA_ACC_QUERY_BY_PORT);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)

#if (SYS_CPNT_ACCOUNTING == TRUE)


#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */


/* LOCAL SUBPROGRAM BODIES
 */

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_SetConfigSettingToDefault
 * ---------------------------------------------------------------------
 * PURPOSE  : This function will set all the config values which are belongs
 *            to AAA_MGR to default value.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_MGR_SetConfigSettingToDefault()
{
    AAA_OM_Initialize();
}



#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_HandleGroupChangeEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : try to find all entries refer to this group out
 * INPUT    : request_type, group_name
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : **SHOULD NOT LOCK before call this function
 * ---------------------------------------------------------------------*/
static void AAA_MGR_HandleGroupChangeEvent(AAA_AccRequestType_T request_type, const char *group_name)
{
    UI32_T              ifindex;
    AAA_ExecType_T   exec_type;

    /* find every dot1x entry refer to this group out */
    ifindex = 0;

    while (TRUE == AAA_OM_GetNextAccDot1xEntryFilterGroup(&ifindex, group_name))
    {
        switch (request_type)
        {
            case AAA_ACC_START:
                AAA_MGR_TRACE("\r\n[AAA_MGR_HandleGroupChangeEvent] dot1x start at ifindex(%lu)", ifindex);
                AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);
                break;

            case AAA_ACC_STOP:
                AAA_MGR_TRACE("\r\n[AAA_MGR_HandleGroupChangeEvent] dot1x stop at ifindex(%lu)", ifindex);
                AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);
                break;

            default:
                return;
        }
    }

    /* find every exec entry refer to this group out */
    exec_type = AAA_EXEC_TYPE_NONE;

    while (TRUE == AAA_OM_GetNextAccExecEntryFilterGroup(&exec_type, group_name))
    {
        switch (request_type)
        {
            case AAA_ACC_START:
                AAA_MGR_TRACE("\r\n[AAA_MGR_HandleGroupChangeEvent] exec start at exec_type(%d)", exec_type);
                AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_EXEC, exec_type);
                break;

            case AAA_ACC_STOP:
                AAA_MGR_TRACE("\r\n[AAA_MGR_HandleGroupChangeEvent] exec stop at exec_type(%d)", exec_type);
                AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_EXEC, exec_type);
                break;

            default:
                return;
        }
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_HandleListChangeEvent
 * ---------------------------------------------------------------------
 * PURPOSE  : try to find all entries refer to this list out
 * INPUT    : client_type, request_type, list_name
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : **SHOULD NOT LOCK before call this function
 * ---------------------------------------------------------------------*/
static void AAA_MGR_HandleListChangeEvent(AAA_ClientType_T client_type, AAA_AccRequestType_T request_type, const char *list_name)
{
    UI32_T              ifindex;
    AAA_ExecType_T   exec_type;

    switch (client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            /* find every dot1x entry refer to this method-list out */
            ifindex = 0;

            while (TRUE == AAA_OM_GetNextAccDot1xEntryFilterList(&ifindex, list_name))
            {
                switch (request_type)
                {
                    case AAA_ACC_START:
                        AAA_MGR_TRACE("\r\n[AAA_MGR_HandleListChangeEvent] dot1x start at ifindex(%lu)", ifindex);
                        AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);
                        break;

                    case AAA_ACC_STOP:
                        AAA_MGR_TRACE("\r\n[AAA_MGR_HandleListChangeEvent] dot1x stop at ifindex(%lu)", ifindex);
                        AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_DOT1X, ifindex);
                        break;

                    default:
                        return;
                }
            }
            break;

        case AAA_CLIENT_TYPE_EXEC:
            /* find every exec entry refer to this method-list out */
            exec_type = AAA_EXEC_TYPE_NONE;

            while (TRUE == AAA_OM_GetNextAccExecEntryFilterList(&exec_type, list_name))
            {
                switch (request_type)
                {
                    case AAA_ACC_START:
                        AAA_MGR_TRACE("\r\n[AAA_MGR_HandleListChangeEvent] exec start at exec_type(%d)", exec_type);
                        AAA_MGR_TryToSendStartRequest(AAA_CLIENT_TYPE_EXEC, exec_type);
                        break;

                    case AAA_ACC_STOP:
                        AAA_MGR_TRACE("\r\n[AAA_MGR_HandleListChangeEvent] exec stop at exec_type(%d)", exec_type);
                        AAA_MGR_TryToSendStopRequest(AAA_CLIENT_TYPE_EXEC, exec_type);
                        break;

                    default:
                        return;
                }
            }
            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
        default:
            break;
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_TryToSendStartRequest
 * ---------------------------------------------------------------------
 * PURPOSE  : try to notify suitable component to send start request for users
 * INPUT    : client_type, super_ifindex
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : **SHOULD NOT LOCK before call this function
 *            if client_type == DOT1X, super_ifindex == l_port
 *            if client_tpye == EXEC, super_ifindex == exec_type
 * ---------------------------------------------------------------------*/
static void AAA_MGR_TryToSendStartRequest(AAA_ClientType_T client_type, UI32_T super_ifindex)
{
    AAA_AccCpntAsyncNotify_Callback_T   callback_func;

    AAA_AccInterface_T                  acc_interface;
    AAA_QueryGroupIndexResult_T         query_result;
    AAA_AccUserEntryInterface_T         user_entry;
    AAA_AccRequest_T                    request;

    AAA_MGR_LOCK();

    memset(&acc_interface, 0, sizeof(AAA_AccInterface_T));
    acc_interface.client_type = client_type;
    if (client_type == AAA_CLIENT_TYPE_EXEC)
    {
        if(FALSE == AAA_OM_ConvertToIfindex(super_ifindex, &acc_interface.ifindex))
        {
            AAA_MGR_TRACE("Bad super_ifindex(%ld)", super_ifindex);
        }
    }
    else
        acc_interface.ifindex = super_ifindex;

    if (FALSE == AAA_OM_GetAccountingGroupIndexByInterface(&acc_interface, &query_result))
    {
        AAA_MGR_UNLOCK();
        return;
    }

    switch (query_result.group_type)
    {
        case GROUP_RADIUS:
            callback_func = accounting_cpnt_callback[AAA_ACC_CPNT_RADIUS];
            break;

        case GROUP_TACACS_PLUS:
            callback_func = accounting_cpnt_callback[AAA_ACC_CPNT_TACACS_PLUS];
            break;

        case GROUP_UNKNOWN:
        default:
            callback_func = NULL;
            break;
    }

    if (NULL == callback_func)
    {
        AAA_MGR_UNLOCK();
        return;
    }

    user_entry.user_index = 0;
    user_entry.client_type = client_type;
    user_entry.ifindex = super_ifindex;
    while (TRUE == AAA_OM_GetNextAccUserEntryInterfaceFilterByParams(&user_entry))
    {
        switch (query_result.group_type)
        {
            case GROUP_RADIUS:
                if (1 == user_entry.notify_bitmap.radius_start)
                    continue;
                AAA_OM_SetAccUserRadiusStartFlag(user_entry.user_index, TRUE); /* flag on */
                break;

            case GROUP_TACACS_PLUS:
                if (1 == user_entry.notify_bitmap.tacacs_start)
                    continue;
                AAA_OM_SetAccUserTacacsStartFlag(user_entry.user_index, TRUE); /* flag on */
                break;

            case GROUP_UNKNOWN:
            default:
                continue;
        }

        if (FALSE == AAA_OM_GenerateRequestFromUserInfo(user_entry.user_index, AAA_ACC_START, &request))
        {
            AAA_MGR_TRACE("\r\n[AAA_MGR_TryToSendStartRequest] failed to generate request");
            continue;
        }

        /* need to notify accounting component,
           we must unlock first because radius_mgr/tacacs_mgr may call aaa_mgr's API
         */
        AAA_MGR_UNLOCK();
        callback_func(&request);
        AAA_MGR_LOCK();

        if (AAA_CLIENT_TYPE_EXEC == client_type)
            user_entry.ifindex = super_ifindex; /* can't use original ifindex as search key */
    }

    AAA_MGR_UNLOCK();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_TryToSendStopRequest
 * ---------------------------------------------------------------------
 * PURPOSE  : try to notify suitable component to send stop request for users
 * INPUT    : client_type, super_ifindex
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : **SHOULD NOT LOCK before call this function
 *            if client_type == DOT1X, super_ifindex == l_port
 *            if client_tpye == EXEC, super_ifindex == exec_type
 * ---------------------------------------------------------------------*/
static void AAA_MGR_TryToSendStopRequest(AAA_ClientType_T client_type, UI32_T super_ifindex)
{
    AAA_AccUserEntryInterface_T     user_entry;
    AAA_AccRequest_T                request;

    AAA_MGR_LOCK();

    user_entry.user_index = 0;
    user_entry.client_type = client_type;
    user_entry.ifindex = super_ifindex;
    while (TRUE == AAA_OM_GetNextAccUserEntryInterfaceFilterByParams(&user_entry))
    {
        if (FALSE == AAA_OM_GenerateRequestFromUserInfo(user_entry.user_index, AAA_ACC_STOP, &request))
        {
            AAA_MGR_TRACE("\r\n[AAA_MGR_TryToSendStopRequest] failed to generate request");
            continue;
        }

        request.terminate_cause = AAA_ACC_TERM_BY_SERVICE_UNAVAILABLE; /* Coz broken setting */

        /* need to notify accounting component,
           we must unlock first because radius_mgr/tacacs_mgr may call aaa_mgr's API
         */
        if (1 == user_entry.notify_bitmap.radius_start)
        {
            AAA_OM_SetAccUserRadiusStartFlag(user_entry.user_index, FALSE); /* flag off */

            AAA_MGR_UNLOCK();
            accounting_cpnt_callback[AAA_ACC_CPNT_RADIUS](&request);
            AAA_MGR_LOCK();
        }

        if (1 == user_entry.notify_bitmap.tacacs_start)
        {
            AAA_OM_SetAccUserTacacsStartFlag(user_entry.user_index, FALSE); /* flag off */

            AAA_MGR_UNLOCK();
            accounting_cpnt_callback[AAA_ACC_CPNT_TACACS_PLUS](&request);
            AAA_MGR_LOCK();
        }

        if (AAA_CLIENT_TYPE_EXEC == client_type)
            user_entry.ifindex = super_ifindex; /* can't use original ifindex as search key */
    }

    AAA_MGR_UNLOCK();
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_StopAllAccountingRequests
 * ---------------------------------------------------------------------
 * PURPOSE  : try to notify suitable component to send stop request for users
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : ** this func should only be called by AAA_MGR_EnterTransitionMode()
 *            doesn't protect by samephore
 * ---------------------------------------------------------------------*/
static void AAA_MGR_StopAllAccountingRequests()
{
    AAA_AccUserEntryInterface_T     user_entry;
    AAA_AccRequest_T                request;

    user_entry.user_index = 0;
    while (TRUE == AAA_OM_GetNextAccUserEntryInterface(&user_entry))
    {
        if (FALSE == AAA_OM_GenerateRequestFromUserInfo(user_entry.user_index, AAA_ACC_STOP, &request))
        {
            AAA_MGR_TRACE("\r\n[AAA_MGR_StopAllAccountingRequests] failed to generate request");
            continue;
        }

        request.terminate_cause = AAA_ACC_TERM_BY_ADMIN_REBOOT; /* TCN occurred */

        /* need to notify accounting component,
           we must unlock first because radius_mgr/tacacs_mgr may call aaa_mgr's API
         */
        if (1 == user_entry.notify_bitmap.radius_start)
        {
            AAA_OM_SetAccUserRadiusStartFlag(user_entry.user_index, FALSE); /* flag off */
            accounting_cpnt_callback[AAA_ACC_CPNT_RADIUS](&request);
        }

        if (1 == user_entry.notify_bitmap.tacacs_start)
        {
            AAA_OM_SetAccUserTacacsStartFlag(user_entry.user_index, FALSE); /* flag off */
            accounting_cpnt_callback[AAA_ACC_CPNT_TACACS_PLUS](&request);
        }
    }
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_QueryNextAccUser
 * ---------------------------------------------------------------------
 * PURPOSE  : to avoid code duplicate
 * INPUT    : idx
 * OUTPUT   : entry
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_MGR_QueryNextAccUser(AAA_AccUserInfoInterface_T *entry, AAA_AccQueryAccUserType_T type)
{
    BOOL_T  ret;
    BOOL_T (*query_func)(AAA_AccUserInfoInterface_T *entry);

    switch (type)
    {
        case AAA_ACC_QUERY_BY_INDEX:
            query_func = AAA_OM_GetNextAccUserEntryInfo;
            break;

        case AAA_ACC_QUERY_BY_NAME_TYPE:
            query_func = AAA_OM_GetNextAccUserEntryFilterByNameAndType;
            break;

        case AAA_ACC_QUERY_BY_TYPE:
            query_func = AAA_OM_GetNextAccUserEntryFilterByType;
            break;

        case AAA_ACC_QUERY_BY_PORT:
            query_func = AAA_OM_GetNextAccUserEntryFilterByPort;
            break;

        default:
            return FALSE;
    }

    do
    {
        ret = query_func(entry);
        if (FALSE == ret)
            break;

        switch (entry->group_type)
        {
        #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
            case GROUP_RADIUS:
                ret = RADIUS_MGR_GetAccUserRunningInfo(entry->ifindex, entry->user_name, entry->client_type, &entry->running_info);
                break;
        #endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

        #if (SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE)
            case GROUP_TACACS_PLUS:
                ret = TACACS_OM_GetAccUserRunningInfo(entry->ifindex, entry->user_name, entry->client_type, &entry->running_info);
                break;
        #endif /* SYS_CPNT_TACACS_PLUS_ACCOUNTING == TRUE */

            case GROUP_UNKNOWN:
            default:
                continue;
        }

        if (FALSE == ret)
        {
            ret = TRUE;
            continue;
        }

        switch (entry->running_info.connect_status)
        {
            case AAA_ACC_CNET_DORMANT:
            case AAA_ACC_CNET_IDLE:
            case AAA_ACC_CNET_FAILED:
            default:
                continue;

            case AAA_ACC_CNET_CONNECTING:
            case AAA_ACC_CNET_CONNECTED:
            case AAA_ACC_CNET_TIMEOUT:
                break;
        }

        break;
    }
    while (TRUE == ret);

    return ret;
}

#if 0
/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_MGR_DumpAccUserTable
 * ---------------------------------------------------------------------
 * PURPOSE  : try to dump user table
 * INPUT    : none
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : this func is used to debug only
 * ---------------------------------------------------------------------*/
void AAA_MGR_DumpAccUserTable()
{
    AAA_MGR_USE_CSC_CHECK_OPER_MODE_WITHOUT_RETURN_VALUE();
    AAA_MGR_LOCK();

    AAA_OM_DumpAccUserTable();

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC_WITHOUT_RETUEN_VALUE();
}
#endif /* if 0 */

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_AuthorRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : request to do authorization function
 * INPUT    : *request -- input of authorization request
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_AuthorRequest(AAA_AuthorRequest_T *request, AAA_AuthorReply_T *reply)
{
    BOOL_T  ret;
    TACACS_AuthorRequest_T  tacacs_request;
    TACACS_AuthorReply_T    tacacs_reply;
    TACACS_Server_Host_T    server_host;
    AAA_TacacsPlusEntryInterface_T tacacs_entry;
    AAA_QueryGroupIndexResult_T query_result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    AAA_MGR_TRACE("AuthorRequest(user_name = %s, client type = %d, ifindex = %lu)",
                  request->user_name,
                  request->client_type,
                  request->ifindex
                  );

    if (request->client_type != AAA_CLIENT_TYPE_EXEC
#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        && request->client_type != AAA_CLIENT_TYPE_COMMANDS
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */
        )
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if (FALSE == AAA_MGR_GetAuthorizationGroupIndex(request->command_privilege,
                                                    request->client_type,
                                                    request->ifindex,
                                                    &query_result))
    {
        AAA_MGR_TRACE("AAA_AuthorRequest_CONFIG_IMCOMPLETE");

        reply->return_type = AAA_AuthorRequest_CONFIG_IMCOMPLETE;

        AAA_MGR_TRACE("AuthorRequest(return_type = %d, new_privilege=%lu)",
                      reply->return_type,
                      reply->new_privilege);

        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    memset(&tacacs_request, 0, sizeof(TACACS_AuthorRequest_T));
    memset(&tacacs_reply, 0, sizeof(TACACS_AuthorReply_T));

    /* connection information
     */
    tacacs_request.ifindex = request->ifindex;
    memcpy(&tacacs_request.rem_ip_addr, &request->rem_ip_addr, sizeof(tacacs_request.rem_ip_addr));

    /* user name */
    memcpy(tacacs_request.user_name, request->user_name, (SYS_ADPT_MAX_USER_NAME_LEN));
    tacacs_request.user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0';
    /* password */
    memcpy(tacacs_request.password, request->password, (SYS_ADPT_MAX_PASSWORD_LEN));
    tacacs_request.password[SYS_ADPT_MAX_PASSWORD_LEN] = '\0';

    tacacs_request.current_privilege = request->current_privilege;
    tacacs_request.authen_by_whom = request->authen_by_whom;

#if (SYS_CPNT_TACACS_PLUS_AUTHORIZATION_COMMAND == TRUE)
    strncpy(tacacs_request.command, request->command, sizeof(tacacs_request.command)-1);
    tacacs_request.command[sizeof(tacacs_request.command)-1] = '\0';
#endif

    memset(&tacacs_entry, 0, sizeof(AAA_TacacsPlusEntryInterface_T));
    while(TRUE == AAA_OM_GetNextTacacsPlusEntry(query_result.group_index, &tacacs_entry))
    {
        if(TRUE == TACACS_OM_Get_Server_Host(tacacs_entry.tacacs_server_index, &server_host))
        {
            tacacs_request.server_ip = server_host.server_ip;
            tacacs_request.server_port = server_host.server_port;
            memcpy(tacacs_request.secret, server_host.secret, (TACACS_AUTH_KEY_MAX_LEN + 1));

            tacacs_request.timeout = server_host.timeout;
            tacacs_request.retransmit = server_host.retransmit;

            /* support exec authorization only
             */
            if (request->ifindex == 0)
            {
                tacacs_request.sess_type = TACACS_SESS_TYPE_CONSOLE;
            }
            else if ((1 <= request->ifindex) && (request->ifindex <= SYS_ADPT_MAX_TELNET_NUM))
            {
                tacacs_request.sess_type = TACACS_SESS_TYPE_TELNET;
            }
            else if (request->ifindex == (SYS_ADPT_MAX_TELNET_NUM + 1))
            {
                tacacs_request.sess_type = TACACS_SESS_TYPE_HTTP;
            }

            tacacs_request.ifindex = request->ifindex;

            ret = TACACS_MGR_Author_Check(&tacacs_request, &tacacs_reply);
            if(tacacs_reply.return_type == TACACS_AuthorRequest_TIMEOUT)
            {
                continue;
            }
            reply->new_privilege = tacacs_reply.new_privilege;
            switch(tacacs_reply.return_type)
            {
            case 	TACACS_AuthorRequest_SUCCEEDED:
                reply->return_type = AAA_AuthorRequest_SUCCEEDED;
                break;
            case TACACS_AuthorRequest_FAILED:
                reply->return_type = AAA_AuthorRequest_FAILED;
                break;
            case TACACS_AuthorRequest_TIMEOUT:
                reply->return_type = AAA_AuthorRequest_TIMEOUT;
                break;
            case TACACS_AuthorRequest_CONFIG_IMCOMPLETE:
                reply->return_type = AAA_AuthorRequest_CONFIG_IMCOMPLETE;
                break;
            case TACACS_AuthorRequest_SUCCEEDED_WITH_NO_PRIV:
                reply->return_type = AAA_AuthorRequest_SUCCEEDED_WITH_NO_PRIV;
                break;
            case TACACS_AuthorRequest_START:
            default:
                reply->return_type = AAA_AuthorRequest_START;
            }

            AAA_MGR_TRACE("AuthorRequest(return_type = %d, new_privilege=%lu)",
                          reply->return_type,
                          reply->new_privilege);

            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
        }
    }
    reply->return_type = AAA_AuthorRequest_CONFIG_IMCOMPLETE;

    AAA_MGR_TRACE("AuthorRequest(return_type = %d, new_privilege=%lu)",
                  reply->return_type,
                  reply->new_privilege);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorizationGroupIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_GetAuthorizationGroupIndex(UI32_T priv_lvl, AAA_ClientType_T client_type, UI32_T ifindex, AAA_QueryGroupIndexResult_T *query_result)
{
    BOOL_T  ret;

    AAA_ExecType_T       exec_type;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    if (AAA_CLIENT_TYPE_EXEC == client_type)
    {
        /* must convert ifindex to EXEC TYPE */
        if (FALSE == AAA_OM_ConvertToExecType(ifindex, &exec_type))
        {
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        ifindex = exec_type;
    }

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    if (AAA_CLIENT_TYPE_COMMANDS == client_type)
    {
        /* must convert ifindex to EXEC TYPE */
        if (FALSE == AAA_OM_ConvertToExecType(ifindex, &exec_type))
        {
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
        }

        ifindex = exec_type;
    }
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

    ret = AAA_OM_GetAuthorizationGroupIndex(priv_lvl, client_type, ifindex, query_result);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry)
{
    BOOL_T  ret;

    AAA_ApiUpdateResult_T   result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    result = AAA_OM_SetAuthorExecEntry(entry);

    AAA_MGR_UNLOCK();

    switch (result)
    {
        case AAA_API_UPDATE_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAuthorExecEntry(AAA_ExecType_T exec_type)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAuthorExecEntry(exec_type);

    AAA_MGR_UNLOCK();

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0')
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name)
{
    BOOL_T      ret;

    AAA_ApiUpdateResult_T   update_result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

   update_result = AAA_OM_SetAuthorDefaultList(list_type, group_name);

    AAA_MGR_UNLOCK();

    ret = TRUE;
    switch (update_result)
    {
        case AAA_API_UPDATE_CHANGE:
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }


    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : modify or create a method-list entry by list_name and client_type
 * INPUT    : entry->list_name, entry->group_name, entry->client_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then insert else replace
 *            list_index, group_type will be ignored
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    if (NULL == entry)
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if((strlen(entry->list_name) < MINSIZE_aaaAuthMethodName) ||
        (strlen(entry->list_name) > MAXSIZE_aaaAuthMethodName))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if((strlen(entry->group_name) < MINSIZE_aaaAuthMethodGroupName) ||
        (strlen(entry->group_name) > MAXSIZE_aaaAuthMethodGroupName))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    if ((entry->list_type.client_type < AAA_CLIENT_TYPE_MIN) ||
        (entry->list_type.client_type > AAA_CLIENT_TYPE_NUMBER))
    {
        AAA_MGR_UNLOCK();
        AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_SetAuthorListEntry(entry);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_DestroyAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DestroyAuthorListEntry(list_name, list_type, warning);

    AAA_MGR_UNLOCK();

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAuthorListEntry(AAA_AuthorListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetNextAuthorListEntryInterface(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}



/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextRunningAuthorExecEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    SYS_TYPE_Get_Running_Cfg_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(SYS_TYPE_GET_RUNNING_CFG_FAIL);

    ret = AAA_OM_GetNextRunningAuthorExecEntry(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the authorization list by name and client_type
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorListEntry(AAA_AuthorListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAuthorListEntryInterface(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization list by index.
 * INPUT    : list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorListEntryByIndex(AAA_AuthorListEntryInterface_T *entry)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);

    ret = AAA_OM_GetAuthorListEntryInterfaceByIndex(entry);

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_GetAuthorMethodTable
 *------------------------------------------------------------------------
 * PURPOSE  : This function to get the AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorMethodTable(UI16_T method_index, AAA_AuthorListEntryInterface_T *entry)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)

    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->list_index = method_index;
    return AAA_MGR_GetAuthorListEntryByIndex(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_GetNextAuthorMethodTable
 *------------------------------------------------------------------------
 * PURPOSE  : This function to get the next AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAuthorMethodTable(UI16_T method_index, AAA_AuthorListEntryInterface_T *entry)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)
    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->list_index = method_index;
    return AAA_MGR_GetNextAuthorListEntry(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAuthorMethodName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the moethod_name into AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AuthorListEntryInterface_T
 *            method_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorMethodName(UI16_T method_index, char* method_name)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)

    BOOL_T  ret, old_flag;

    AAA_AuthorListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAuthorListEntryInterfaceByIndex(&old_entry);

    ret = AAA_OM_SetAuthorListEntryName(method_index, method_name);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAuthorMethodGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the group_name into AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AuthorListEntryInterface_T
              method_group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorMethodGroupName(UI16_T method_index, char* method_group_name)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)

    BOOL_T  ret, old_flag;

    AAA_AuthorListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAuthorListEntryInterfaceByIndex(&old_entry);

    ret = AAA_OM_SetAuthorListEntryGroupName(method_index, method_group_name);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAuthorMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorMethodClientType(UI16_T method_index, const AAA_ListType_T *list_type)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)

    BOOL_T  ret, old_flag;

    AAA_ApiUpdateResult_T           update_result;
    AAA_AuthorListEntryInterface_T     old_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    old_entry.list_index = method_index;
    old_flag = AAA_OM_GetAuthorListEntryInterfaceByIndex(&old_entry);

    update_result = AAA_OM_SetAuthorListEntryClientType(method_index, list_type);

    AAA_MGR_UNLOCK();

    ret = TRUE;
    switch (update_result)
    {
        case AAA_API_UPDATE_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAuthorMethodStatus
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the status into AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AuthorListEntryInterface_T
 *            method_status  -- Set to VAL_aaaMethodStatus_valid to initiate the aaaAuthorMethodTable,
 *            VAL_aaaMethodStatus_invalid to destroy the table.
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorMethodStatus(UI16_T method_index, UI8_T method_status)
{
#if (SYS_CPNT_AUTHORIZATION == TRUE)

    BOOL_T  ret;

    AAA_EntryStatus_T   entry_status;

    AAA_AuthorListEntryInterface_T list_entry;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    switch (method_status)
    {
        case VAL_aaaAccountMethodStatus_valid: /* create */
            entry_status = AAA_ENTRY_READY;
            break;

        case VAL_aaaAccountMethodStatus_invalid: /* destroy */
            entry_status = AAA_ENTRY_DESTROYED;

            list_entry.list_index = method_index;
            if (FALSE == AAA_OM_GetAuthorListEntryInterfaceByIndex(&list_entry)) /* keep it in mgr */
            {
                AAA_MGR_UNLOCK();
                AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
            }

            break;

        default:
            AAA_MGR_UNLOCK();
            AAA_MGR_RETURN_AND_RELEASE_CSC(FALSE);
    }

    ret = AAA_OM_SetAuthorListEntryStatus(method_index, entry_status);

    AAA_MGR_UNLOCK();
    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
#else
    return FALSE;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */
}

#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry)
{
    BOOL_T  ret;

    AAA_ApiUpdateResult_T   result;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    result = AAA_OM_SetAuthorCommandEntry(entry);

    AAA_MGR_UNLOCK();

    switch (result)
    {
        case AAA_API_UPDATE_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_NO_CHANGE:
            ret = TRUE;
            break;

        case AAA_API_UPDATE_FAILED:
        default:
            ret = FALSE;
            break;
    }

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable command authorization by exec_type
 * INPUT    : exec_type, priv_lvl
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type)
{
    BOOL_T  ret;

    AAA_MGR_USE_CSC_CHECK_OPER_MODE(FALSE);
    AAA_MGR_LOCK();

    ret = AAA_OM_DisableAuthorCommandEntry(priv_lvl, exec_type);

    AAA_MGR_UNLOCK();

    AAA_MGR_RETURN_AND_RELEASE_CSC(ret);
}
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


/*-------------------------------------------------------------------------
 * ROUTINE NAME : AAA_MGR_HandleIPCReqMsg
 *-------------------------------------------------------------------------
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
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    AAA_MGR_IPCMsg_T *msg_data_p;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
    UI8_T tmep_arg_spac_p[sizeof(AAA_MGR_IPCMsg_ALL_ARG_T)];
    AAA_MGR_IPCMsg_ALL_ARG_T *tmep_arg_data_p=(AAA_MGR_IPCMsg_ALL_ARG_T *)tmep_arg_spac_p;
#endif
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p = (AAA_MGR_IPCMsg_T*)ipcmsg_p->msg_buf;
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
         *Modify file:aaa_mgr.c
         *Approved by:Hardsun
         */
        ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE;
        msg_data_p->type.result_ui32 = FALSE;
        goto exit;
    }

    switch(cmd)
    {
        case AAA_MGR_IPCCMD_SETRADIUSGROUPENTRY:
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusgroupentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusGroupEntry(
                &msg_data_p->data.setradiusgroupentry.req.entry);
            break;

        case AAA_MGR_IPCCMD_DESTROYRADIUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyradiusgroupentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyRadiusGroupEntry(
                msg_data_p->data.destroyradiusgroupentry.req.name,
                &msg_data_p->data.destroyradiusgroupentry.resp.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusgroupentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusGroupEntry(
                &msg_data_p->data.settacacsplusgroupentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYTACACSPLUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroytacacsplusgroupentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyTacacsPlusGroupEntry(
                msg_data_p->data.destroytacacsplusgroupentry.req.name,
                &msg_data_p->data.destroytacacsplusgroupentry.resp.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusEntry(
                msg_data_p->data.setradiusentry.req.group_index,
                &msg_data_p->data.setradiusentry.req.entry,
                &msg_data_p->data.setradiusentry.resp.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSENTRYJOINDEFAULTRADIUSGROUP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusentryjoindefaultradiusgroup.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup(
                msg_data_p->data.setradiusentryjoindefaultradiusgroup.req.radius_server_index);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSENTRYDEPARTDEFAULTRADIUSGROUP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusentrydepartdefaultradiusgroup.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup(
                msg_data_p->data.setradiusentrydepartdefaultradiusgroup.req.radius_server_index);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSENTRYBYIPADDRESS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusentrybyipaddress.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusEntryByIpAddress(
                msg_data_p->data.setradiusentrybyipaddress.req.group_index,
                msg_data_p->data.setradiusentrybyipaddress.req.ip_address);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYRADIUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyradiusentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyRadiusEntry(
                msg_data_p->data.destroyradiusentry.req.group_index,
                &msg_data_p->data.destroyradiusentry.req.entry,
                &msg_data_p->data.destroyradiusentry.req.warning);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYRADIUSENTRYBYIPADDRESS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyradiusentrybyipaddress.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyRadiusEntryByIpAddress(
                msg_data_p->data.destroyradiusentrybyipaddress.req.group_index,
                msg_data_p->data.destroyradiusentrybyipaddress.req.ip_address,
                &msg_data_p->data.destroyradiusentrybyipaddress.req.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusEntry(
                msg_data_p->data.settacacsplusentry.req.group_index,
                &msg_data_p->data.settacacsplusentry.req.entry,
                &msg_data_p->data.settacacsplusentry.resp.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSENTRYJOINDEFAULTTACACSPLUSGROUP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusentryjoindefaulttacacsplusgroup.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(
                msg_data_p->data.settacacsplusentryjoindefaulttacacsplusgroup.req.tacacs_server_index);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSENTRYDEPARTDEFAULTTACACSPLUSGROUP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusentrydepartdefaulttacacsplusgroup.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(
                msg_data_p->data.settacacsplusentrydepartdefaulttacacsplusgroup.req.tacacs_server_index);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSENTRYBYIPADDRESS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusentrybyipaddress.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusEntryByIpAddress(
                msg_data_p->data.settacacsplusentrybyipaddress.req.group_index,
                msg_data_p->data.settacacsplusentrybyipaddress.req.ip_address);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroytacacsplusentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyTacacsPlusEntry(
                msg_data_p->data.destroytacacsplusentry.req.group_index,
                &msg_data_p->data.destroytacacsplusentry.req.entry,
                &msg_data_p->data.destroytacacsplusentry.req.warning);
        }
            break;


        case AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRYBYIPADDRESS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroytacacsplusentrybyipaddress.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyTacacsPlusEntryByIpAddress(
                msg_data_p->data.destroytacacsplusentrybyipaddress.req.group_index,
                msg_data_p->data.destroytacacsplusentrybyipaddress.req.ip_address,
                &msg_data_p->data.destroytacacsplusentrybyipaddress.req.warning);
        }
            break;

#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_MGR_IPCCMD_SETACCUPDATEINTERVAL:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setaccupdateinterval.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccUpdateInterval(
                msg_data_p->data.setaccupdateinterval.req.update_interval);
        }
            break;

        case AAA_MGR_IPCCMD_DISABLEACCUPDATEINTERVAL:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.disableaccupdateinterval.resp);
            msg_data_p->type.result_bool=AAA_MGR_DisableAccUpdateInterval();
        }
            break;

        case AAA_MGR_IPCCMD_SETACCDOT1XENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setaccdot1xentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccDot1xEntry(
                &msg_data_p->data.setaccdot1xentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DISABLEACCDOT1XENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.disableaccdot1xentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DisableAccDot1xEntry(
                msg_data_p->data.disableaccdot1xentry.req.ifindex);
        }
            break;

        case AAA_MGR_IPCCMD_SETACCEXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setaccexecentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccExecEntry(
                &msg_data_p->data.setaccexecentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DISABLEACCEXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.disableaccexecentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DisableAccExecEntry(
                msg_data_p->data.disableaccexecentry.req.exec_type);
        }
            break;

        case AAA_MGR_IPCCMD_SETACCLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setacclistentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccListEntry(
                &msg_data_p->data.setacclistentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYACCLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyacclistentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyAccListEntry(
                msg_data_p->data.destroyacclistentry.req.name,
                msg_data_p->data.destroyacclistentry.req.client_type,
                &msg_data_p->data.destroyacclistentry.req.warning);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYACCLISTENTRY2:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyacclistentry2.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyAccListEntry2(
                &msg_data_p->data.destroyacclistentry2.req.entry,
                &msg_data_p->data.destroyacclistentry2.req.warning);
        }
            break;

        case AAA_MGR_IPCCMD_SETDEFAULTLIST:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setdefaultlist.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetDefaultList(
                &msg_data_p->data.setdefaultlist.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_ASYNC_ASYNCACCOUNTINGREQUEST:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.asyncaccountingrequest.resp);
            msg_data_p->type.result_bool=AAA_MGR_AsyncAccountingRequest(
                &msg_data_p->data.asyncaccountingrequest.req.request);
        }
            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_MGR_IPCCMD_SETACCCOMMANDENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setacccmdentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccCommandEntry(
                &msg_data_p->data.setacccmdentry.req.cmd_entry);
        }
            break;

        case AAA_MGR_IPCCMD_DISABLEACCCOMMANDENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setacccmdentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DisableAccCommandEntry(
                &msg_data_p->data.setacccmdentry.req.cmd_entry);
        }
            break;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)

        case AAA_MGR_IPCCMD_SETMETHODNAME:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setmethodname.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetMethodName(
                msg_data_p->data.setmethodname.req.method_index,
                msg_data_p->data.setmethodname.req.method_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETMETHODGROUPNAME:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setmethodgroupname.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetMethodGroupName(
                msg_data_p->data.setmethodgroupname.req.method_index,
                msg_data_p->data.setmethodgroupname.req.method_group_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETMETHODCLIENTTYPE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setmethodclienttype.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetMethodClientType(
                msg_data_p->data.setmethodclienttype.req.method_index,
                msg_data_p->data.setmethodclienttype.req.client_type);
        }
            break;

        case AAA_MGR_IPCCMD_SETMETHODMODE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setmethodmode.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetMethodMode(
                msg_data_p->data.setmethodmode.req.method_index,
                msg_data_p->data.setmethodmode.req.method_mode);
        }
            break;

        case AAA_MGR_IPCCMD_SETMETHODSTATUS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setmethodstatus.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetMethodStatus(
                msg_data_p->data.setmethodstatus.req.method_index,
                msg_data_p->data.setmethodstatus.req.method_status);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSSERVERGROUPBITMAP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusgrpsvrbmp.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusGroupServerBitMap(
                msg_data_p->data.setradiusgrpsvrbmp.req.radius_grp_idx,
                msg_data_p->data.setradiusgrpsvrbmp.req.radius_grp_svr_bmp);
        }
            break;

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

        case AAA_MGR_IPCCMD_SETRADIUSGROUPNAME:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusgroupname.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusGroupName(
                msg_data_p->data.setradiusgroupname.req.radius_group_index,
                msg_data_p->data.setradiusgroupname.req.radius_group_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETRADIUSGROUPSTATUS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setradiusgroupstatus.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetRadiusGroupStatus(
                msg_data_p->data.setradiusgroupstatus.req.radius_group_index,
                msg_data_p->data.setradiusgroupstatus.req.radius_group_status);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSGROUPNAME:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusgroupname.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusGroupName(
                msg_data_p->data.settacacsplusgroupname.req.tacacsplus_group_index,
                msg_data_p->data.settacacsplusgroupname.req.tacacsplus_group_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSERVERBITMAP:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusgroupserverbitmap.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusGroupServerBitMap(
                msg_data_p->data.settacacsplusgroupserverbitmap.req.tacacsplus_group_index,
                msg_data_p->data.settacacsplusgroupserverbitmap.req.tacacsplus_group_server_bitmap);
        }
            break;

        case AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSTATUS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.settacacsplusgroupstatus.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetTacacsPlusGroupStatus(
                msg_data_p->data.settacacsplusgroupstatus.req.tacacsplus_group_index,
                msg_data_p->data.settacacsplusgroupstatus.req.tacacsplus_group_status);
        }
            break;

        case AAA_MGR_IPCCMD_SETUPDATE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setupdate.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetUpdate(
                msg_data_p->data.setupdate.req.update_interval);
        }
            break;
#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_MGR_IPCCMD_SETACCOUNTMETHODNAME:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setaccountmethodname.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccountMethodName(
                msg_data_p->data.setaccountmethodname.req.ifindex,
                msg_data_p->data.setaccountmethodname.req.account_method_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETACCOUNTSTATUS:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setaccountstatus.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAccountStatus(
                msg_data_p->data.setaccountstatus.req.ifindex,
                msg_data_p->data.setaccountstatus.req.account_status);
        }
            break;
#endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */

#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_MGR_IPCCMD_GETACCUSERENTRYQTY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getaccuserentryqty.resp);
            msg_data_p->type.result_bool=AAA_MGR_GetAccUserEntryQty(
                &msg_data_p->data.getaccuserentryqty.resp.qty);
        }
            break;

        case AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getaccuserentryqtyfilterbynameandtype.resp);
            memcpy(tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.name,
                msg_data_p->data.getaccuserentryqtyfilterbynameandtype.req.name,
                sizeof(tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.name));
            tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.client_type
                =msg_data_p->data.getaccuserentryqtyfilterbynameandtype.req.client_type;
            msg_data_p->type.result_bool=AAA_MGR_GetAccUserEntryQtyFilterByNameAndType(
                tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.name,
                tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.client_type,
                &tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.qty);
            msg_data_p->data.getaccuserentryqtyfilterbynameandtype.resp.qty
                =tmep_arg_data_p->getaccuserentryqtyfilterbynameandtype.qty;
        }
            break;

        case AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getaccuserentryqtyfilterbytype.resp);
            tmep_arg_data_p->getaccuserentryqtyfilterbytype.client_type
                =msg_data_p->data.getaccuserentryqtyfilterbytype.req.client_type;
            msg_data_p->type.result_bool=AAA_MGR_GetAccUserEntryQtyFilterByType(
                tmep_arg_data_p->getaccuserentryqtyfilterbytype.client_type,
                &tmep_arg_data_p->getaccuserentryqtyfilterbytype.qty);
            msg_data_p->data.getaccuserentryqtyfilterbytype.resp.qty
                =tmep_arg_data_p->getaccuserentryqtyfilterbytype.qty;

        }
            break;

        case AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYPORT:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getaccuserentryqtyfilterbyport.resp);
            tmep_arg_data_p->getaccuserentryqtyfilterbyport.ifindex
                =msg_data_p->data.getaccuserentryqtyfilterbyport.req.ifindex;
            msg_data_p->type.result_bool=AAA_MGR_GetAccUserEntryQtyFilterByPort(
                tmep_arg_data_p->getaccuserentryqtyfilterbyport.ifindex,
                &tmep_arg_data_p->getaccuserentryqtyfilterbyport.qty);
            msg_data_p->data.getaccuserentryqtyfilterbyport.resp.qty
                =tmep_arg_data_p->getaccuserentryqtyfilterbyport.qty;
        }
            break;

        case AAA_MGR_IPCCMD_GETNEXTACCUSERENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getnextaccuserentry.resp);
            tmep_arg_data_p->getnextaccuserentry.entry.user_index
                =msg_data_p->data.getnextaccuserentry.req.user_index;
            msg_data_p->type.result_bool=AAA_MGR_GetNextAccUserEntry(
                &tmep_arg_data_p->getnextaccuserentry.entry);
            msg_data_p->data.getnextaccuserentry.resp.entry
                =tmep_arg_data_p->getnextaccuserentry.entry;
        }
            break;

        case AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYNAMEANDTYPE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getnextaccuserentry.resp);
            tmep_arg_data_p->getnextaccuserentryfilterbynameandtype.entry.user_index
                =msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.user_index;
            tmep_arg_data_p->getnextaccuserentryfilterbynameandtype.entry.ifindex
                =msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.ifindex;
            tmep_arg_data_p->getnextaccuserentryfilterbynameandtype.entry.client_type
                =msg_data_p->data.getnextaccuserentryfilterbynameandtype.req.client_type;
            msg_data_p->type.result_bool=AAA_MGR_GetNextAccUserEntryFilterByNameAndType(
                &tmep_arg_data_p->getnextaccuserentryfilterbynameandtype.entry);
            msg_data_p->data.getnextaccuserentryfilterbynameandtype.resp.entry
                =tmep_arg_data_p->getnextaccuserentryfilterbynameandtype.entry;
        }
            break;

        case AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getnextaccuserentryfilterbytype.resp);
            tmep_arg_data_p->getnextaccuserentryfilterbytype.entry.user_index
                =msg_data_p->data.getnextaccuserentryfilterbytype.req.user_index;
            tmep_arg_data_p->getnextaccuserentryfilterbytype.entry.client_type
                =msg_data_p->data.getnextaccuserentryfilterbytype.req.client_type;
            msg_data_p->type.result_bool=AAA_MGR_GetNextAccUserEntryFilterByType(
                &tmep_arg_data_p->getnextaccuserentryfilterbytype.entry);
            msg_data_p->data.getnextaccuserentryfilterbytype.resp.entry
                =tmep_arg_data_p->getnextaccuserentryfilterbytype.entry;
        }
            break;

        case AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYPORT:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.getnextaccuserentryfilterbyport.resp);
            tmep_arg_data_p->getnextaccuserentryfilterbyport.entry.user_index
                =msg_data_p->data.getnextaccuserentryfilterbyport.req.user_index;
            tmep_arg_data_p->getnextaccuserentryfilterbyport.entry.ifindex
                =msg_data_p->data.getnextaccuserentryfilterbyport.req.ifindex;
            msg_data_p->type.result_bool=AAA_MGR_GetNextAccUserEntryFilterByPort(
                &tmep_arg_data_p->getnextaccuserentryfilterbyport.entry);
            msg_data_p->data.getnextaccuserentryfilterbyport.resp.entry
                =tmep_arg_data_p->getnextaccuserentryfilterbyport.entry;
        }
            break;
#endif

#if (SYS_CPNT_AUTHORIZATION == TRUE)
        case AAA_MGR_IPCCMD_AUTHORREQUEST:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.authorrequest.resp);
            msg_data_p->type.result_bool=AAA_MGR_AuthorRequest(
                &msg_data_p->data.authorrequest.req.request,
                &msg_data_p->data.authorrequest.resp.reply);
        }
            break;

        case AAA_MGR_IPCCMD_SETAUTHOREXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setauthorexecentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAuthorExecEntry(
                &msg_data_p->data.setauthorexecentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DISABLEAUTHOREXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.disableauthorexecentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DisableAuthorExecEntry(
                msg_data_p->data.disableauthorexecentry.req.exec_type);
        }
            break;

        case AAA_MGR_IPCCMD_SETAUTHORDEFAULTLIST:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setauthordefaultlist.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAuthorDefaultList(
                &msg_data_p->data.setauthordefaultlist.req.list_type,
                msg_data_p->data.setauthordefaultlist.req.group_name);
        }
            break;

        case AAA_MGR_IPCCMD_SETAUTHORLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.setauthorlistentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_SetAuthorListEntry(
                &msg_data_p->data.setauthorlistentry.req.entry);
        }
            break;

        case AAA_MGR_IPCCMD_DESTROYAUTHORLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE+
                sizeof(msg_data_p->data.destroyauthorlistentry.resp);
            msg_data_p->type.result_bool=AAA_MGR_DestroyAuthorListEntry(
                msg_data_p->data.destroyauthorlistentry.req.list_name,
                &msg_data_p->data.destroyauthorlistentry.req.list_type,
                &msg_data_p->data.destroyauthorlistentry.resp.warning);
        }
            break;
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        case AAA_MGR_IPCCMD_SET_AUTHOR_COMMAND_ENTRY:
        {
            ipcmsg_p->msg_size = AAA_MGR_MSGBUF_TYPE_SIZE +
                         sizeof(msg_data_p->data.set_author_command_entry.resp);
            msg_data_p->type.result_bool = AAA_MGR_SetAuthorCommandEntry(
                          &msg_data_p->data.set_author_command_entry.req.entry);
            break;
        }

        case AAA_MGR_IPCCMD_DISABLE_AUTHOR_COMMAND_ENTRY:
        {
            ipcmsg_p->msg_size = AAA_MGR_MSGBUF_TYPE_SIZE +
                     sizeof(msg_data_p->data.disable_author_command_entry.resp);
            msg_data_p->type.result_bool = AAA_MGR_DisableAuthorCommandEntry(
                     msg_data_p->data.disable_author_command_entry.req.priv_lvl,
                   msg_data_p->data.disable_author_command_entry.req.exec_type);
            break;
        }

#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


        default:
            ipcmsg_p->msg_size=AAA_MGR_MSGBUF_TYPE_SIZE;
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;
    }

    exit:

        /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
         */
        if(cmd<AAA_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    return TRUE;
}

#endif /* SYS_CPNT_AAA == TRUE */

