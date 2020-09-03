/* MODULE NAME: aaa_om.c
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


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

#include <string.h>
#include <stdio.h>
#include "sysfun.h"
#include "aaa_om_private.h"
#include "aaa_om.h"
#include "sys_bld.h"
#include "security_backdoor.h"
/* NAMING CONSTANT DECLARATIONS
 */
/* we have two kinds of method-list (dot1x & exec) so need double space */
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
#define AAA_OM_MAX_NBR_OF_ACC_LIST                  (SYS_ADPT_MAX_NBR_OF_ACCOUNTING_LIST * 2 + SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE)
#else
#define AAA_OM_MAX_NBR_OF_ACC_LIST                  (SYS_ADPT_MAX_NBR_OF_ACCOUNTING_LIST * 2)
#endif

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
#define AAA_OM_MAX_NBR_OF_AUTHOR_LIST               (SYS_ADPT_MAX_NBR_OF_AUTHORIZATION_LIST * 2 + SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE)
#else
#define AAA_OM_MAX_NBR_OF_AUTHOR_LIST               (SYS_ADPT_MAX_NBR_OF_AUTHORIZATION_LIST * 2)
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

/* let default methods have default group name */
/*#define AAA_DFLT_GROUP_NAME_FOR_DFLT_LIST           SYS_DFLT_AAA_RADIUS_GROUP_NAME*/
/* Dot1x only support radius method */
#define AAA_DFLT_GROUP_NAME_FOR_DFLT_DOT1X_LIST     SYS_DFLT_AAA_RADIUS_GROUP_NAME

/* may modified by project */
#define AAA_DFLT_GROUP_NAME_FOR_DFLT_EXEC_LIST      SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME

/* may modified by project */
#define AAA_DFLT_GROUP_NAME_FOR_DFLT_COMMAND_LIST   SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME

#define AAA_CREATE_DFLT_WORKING_MODE                ACCOUNTING_START_STOP

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE)
#define AAA_SNMP_CREATE_DFLT_METHOD_LIST_NAME       "method-list"
#define AAA_SNMP_CREATE_DFLT_RADIUS_GROUP_NAME      "radiusGroup"
#define AAA_SNMP_CREATE_DFLT_TACACS_PLUS_GROUP_NAME "tacacs+Group"
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
#define AAA_SNMP_CREATE_DFLT_AUTHOR_METHOD_LIST_NAME       "method-list"
#define AAA_SNMP_CREATE_DFLT_AUTHOR_TACACS_PLUS_GROUP_NAME "tacacs+Group"
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

/* currently, we don't support tacacs+ accounting */
//#if (SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS > SYS_ADPT_MAX_NBR_OF_TACACS_ACC_USERS)
    #define AAA_MAX_NBR_OF_ACC_USERS     SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS
/*
#else
    #define AAA_MAX_NBR_OF_ACC_USERS     SYS_ADPT_MAX_NBR_OF_TACACS_PLUS_ACC_USERS
#endif
*/

#define AAA_OM_DEBUG_MODE    /* define AAA_OM_DEBUG_MODE
                                to build AAA_OM with DEBUG version
                                And let following macros print debug messages
                                */

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef AAA_OM_DEBUG_MODE
#define AAA_OM_TRACE(fmt, args...)                              \
    {                                                           \
        if(SECURITY_BACKDOOR_IsOn(aaa_om_backdoor_reg_no))      \
        {                                                       \
             printf("[%s:%d] ", __FUNCTION__, __LINE__);         \
             printf(fmt, ##args);                               \
             printf("\r\n");                                    \
        }                                                       \
    }

#define AAA_OM_LOCK_TRACE(fmt, args...)                         \
    {                                                           \
        if(SECURITY_BACKDOOR_IsOn(aaa_om_lock_trace_reg_no))    \
        {                                                       \
             printf("[%s:%d] ", __FUNCTION__, __LINE__);         \
             printf(fmt, ##args);                               \
             printf("\r\n");                                    \
        }                                                       \
    }
#else
#define AAA_OM_TRACE(fmt, args...)        ((void)0)
#define AAA_OM_LOCK_TRACE(fmt, args...)   ((void)0)
#endif

#define AAA_OM_ENTER_CRITICAL_SECTION() {                       \
        AAA_OM_LOCK_TRACE("Enter criical section");             \
        aaa_om_orig_priority                                    \
            = SYSFUN_OM_ENTER_CRITICAL_SECTION(aaa_om_sem_id);  \
    }

#define AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(RET_VAL) {     \
        AAA_OM_LOCK_TRACE("Leave criical section");             \
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(aaa_om_sem_id,         \
            aaa_om_orig_priority);                              \
        return (RET_VAL);                                       \
    }

#define AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE() {   \
        AAA_OM_LOCK_TRACE("Leave criical section");             \
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(aaa_om_sem_id,         \
        aaa_om_orig_priority);                                  \
        return;                                                 \
    }

#define AAA_OM_COUNT_OF(ary)                                    \
    (sizeof(ary)/sizeof(*ary))

/* DATA TYPE DECLARATIONS
 */
typedef struct AAA_RadiusEntry_S
{
    UI16_T  radius_index; /* array index + 1 */
    UI32_T  radius_server_index; /* mapping to radius_om */

    AAA_EntryStatus_T       entry_status;

    struct AAA_RadiusEntry_S    *prev_radius_entry;
    struct AAA_RadiusEntry_S    *next_radius_entry;
} AAA_RadiusEntry_T;

typedef struct AAA_TacacsPlusEntry_S
{
    UI16_T  tacacs_index; /* array index + 1 */
    UI32_T  tacacs_server_index; /* mapping to tacacs_om */

    AAA_EntryStatus_T       entry_status;

    struct AAA_TacacsPlusEntry_S    *prev_tacacs_entry;
    struct AAA_TacacsPlusEntry_S    *next_tacacs_entry;
} AAA_TacacsPlusEntry_T;

typedef struct AAA_RadiusGroupEntry_S
{
    UI16_T  group_index; /* array index + 1 */
    UI32_T  modified_time;

    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_RadiusEntry_T       radius_entry[AAA_MAX_NBR_OF_RADIUS_ENTRY];
    AAA_RadiusEntry_T       *head_of_radius_entry;
    AAA_RadiusEntry_T       *tail_of_radius_entry;

    AAA_EntryStatus_T       entry_status;
} AAA_RadiusGroupEntry_T;

typedef struct AAA_TacacsPlusGroupEntry_S
{
    UI16_T  group_index; /* array index + 1 */
    UI32_T  modified_time;

    char   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_TacacsPlusEntry_T       tacacs_entry[AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY];
    AAA_TacacsPlusEntry_T       *head_of_tacacs_entry;
    AAA_TacacsPlusEntry_T       *tail_of_tacacs_entry;

    AAA_EntryStatus_T       entry_status;
} AAA_TacacsPlusGroupEntry_T;

#if (SYS_CPNT_ACCOUNTING == TRUE)

typedef struct AAA_AccListEntry_S
{
    UI16_T  list_index; /* array index + 1 */

    char    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
    char    group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

    AAA_ClientType_T     client_type;    /* distinguish between DOT1X and EXEC method-list */
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    UI32_T                  priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
    AAA_AccWorkingMode_T    working_mode;
    AAA_EntryStatus_T       entry_status;

//    struct AAA_AccListEntry_S   *prev_list_entry;
//    struct AAA_AccListEntry_S   *next_list_entry;
} AAA_AccListEntry_T;

/* Accounting Method List Entry Second Index (KEY)
 * Use AAA_AccListEntrySecIndex_T to get the entry of AAA_AccListEntry_T
 */
typedef struct AAA_AccListEntry2rdIndex_S
{
    char                    list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
    AAA_ClientType_T        client_type;    /* distinguish between DOT1X and EXEC method-list */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    UI32_T                  priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

} AAA_AccListEntry2rdIndex_T;

typedef struct AAA_AccUserEntry_S
{
    UI16_T  user_index;     /* array index + 1 */

    UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
    UI8_T   user_name[SYS_ADPT_MAX_USER_NAME_LEN + 1];

    UI32_T  accounting_start_time;  /* unit is seconds */

    /* reserved identifier and call_back_func if can't hook a request currently
       and them will be transfer to request if successfully to hook a request
     */
    UI32_T  identifier; /* callback function's parameter
                           so caller can know which request's result back */
    AAA_AccRequest_Callback_T   call_back_func; /* callback to the CSC which placed this request */

    AAA_ClientType_T            client_type;    /* caller's type */
    AAA_AccNotifyMap_T          notify_bitmap;

    UI32_T                      auth_privilege;     /* meaningfully only while START */
    AAA_Authentic_T             auth_by_whom;       /* meaningfully only while START */
    AAA_AccTerminateCause_T     terminate_cause;    /* meaningfully only while STOP */

    AAA_EntryStatus_T           entry_status;

    struct AAA_AccUserEntry_S   *prev_user_entry;
    struct AAA_AccUserEntry_S   *next_user_entry;
} AAA_AccUserEntry_T;


#endif /* SYS_CPNT_ACCOUNTING == TRUE */



/* LOCAL SUBPROGRAM DECLARATIONS
 */

static AAA_RadiusGroupEntry_T *AAA_OM_GetRadiusGroupEntry(const char *group_name);
static AAA_RadiusGroupEntry_T *AAA_OM_GetRadiusGroupEntryByIndex(UI16_T group_index);
static AAA_RadiusGroupEntry_T *AAA_OM_AllocRadiusGroupEntry();

static AAA_TacacsPlusGroupEntry_T *AAA_OM_GetTacacsPlusGroupEntry(const char *group_name);
static AAA_TacacsPlusGroupEntry_T *AAA_OM_GetTacacsPlusGroupEntryByIndex(UI16_T group_index);
static AAA_TacacsPlusGroupEntry_T *AAA_OM_AllocTacacsPlusGroupEntry();

#if (SYS_CPNT_ACCOUNTING == TRUE)

static AAA_AccDot1xEntry_T *AAA_OM_GetAccDot1xEntry(UI32_T ifindex);
static AAA_AccExecEntry_T *AAA_OM_GetAccExecEntry(AAA_ExecType_T exec_type);

static AAA_AccListEntry_T *AAA_OM_GetAccListEntry(const char *list_name, AAA_ClientType_T client_type);
static AAA_AccListEntry_T *AAA_OM_GetAccListEntryByIndex(UI16_T list_index);
static AAA_AccListEntry_T *AAA_OM_GetAccListEntryByPort(UI32_T ifindex);
static AAA_AccListEntry_T *AAA_OM_AllocAccListEntry(AAA_ClientType_T client_type);

static AAA_AccUserEntry_T *AAA_OM_GetAccFirstUser();
static AAA_AccUserEntry_T *AAA_OM_GetAccUser(UI16_T user_index);
static AAA_AccUserEntry_T *AAA_OM_GetNextAccUser(UI16_T user_index);
static AAA_AccUserEntry_T *AAA_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type);
static AAA_AccUserEntry_T *AAA_OM_AllocAccUser();

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

static AAA_RadiusEntry_T *AAA_OM_GetRadiusEntry(AAA_RadiusGroupEntry_T *group_entry, UI16_T radius_index);
static void AAA_OM_ResetRadiusEntry(AAA_RadiusGroupEntry_T *group_entry);
static AAA_RadiusEntry_T *AAA_OM_AllocRadiusEntry(AAA_RadiusGroupEntry_T *group_entry);
static void AAA_OM_FreeRadiusEntry(AAA_RadiusGroupEntry_T *group_entry, UI16_T radius_index);

static void AAA_OM_CopyRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *det, const AAA_RadiusGroupEntry_T *src);
static void AAA_OM_CopyRadiusEntryInterface(AAA_RadiusEntryInterface_T *det, const AAA_RadiusEntry_T *src);
//static void AAA_OM_CopyRadiusEntry(AAA_RadiusEntry_T *det, const AAA_RadiusEntryInterface_T *src);

static AAA_TacacsPlusEntry_T *AAA_OM_GetTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *tacacs_group, UI16_T tacacs_index);
static void AAA_OM_ResetTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *tacacs_group);
static AAA_TacacsPlusEntry_T *AAA_OM_AllocTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *tacacs_group);
static void AAA_OM_FreeTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *tacacs_group, UI16_T tacacs_index);

static void AAA_OM_CopyTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *det, const AAA_TacacsPlusGroupEntry_T *src);
static void AAA_OM_CopyTacacsPlusEntryInterface(AAA_TacacsPlusEntryInterface_T *det, const AAA_TacacsPlusEntry_T *src);
//static void AAA_OM_CopyTacacsPlusEntry(AAA_TacacsPlusEntry_T *det, const AAA_TacacsPlusEntryInterface_T *src);


static BOOL_T AAA_OM_IsValidGroupName(const char *name);

#if (SYS_CPNT_ACCOUNTING == TRUE)

static BOOL_T AAA_OM_IsValidMethodListName(const char *name);

static BOOL_T AAA_OM_IsAnyListReferThisGroup(const char *group_name);
static BOOL_T AAA_OM_IsAnyPortReferThisList(const char *list_name);
static BOOL_T AAA_OM_IsAnyExecReferThisList(const char *list_name);

static void AAA_OM_CopyListEntryInterface(AAA_AccListEntryInterface_T *det, const AAA_AccListEntry_T *src);
static AAA_ApiUpdateResult_T AAA_OM_CopyListEntry(AAA_AccListEntry_T *det, const AAA_AccListEntryInterface_T *src);

static void AAA_OM_CopyUserEntryInterface(AAA_AccUserEntryInterface_T *det, const AAA_AccUserEntry_T *src);
static void AAA_OM_CopyUserInfoInterface(AAA_AccUserInfoInterface_T *det, const AAA_AccUserEntry_T *src);

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)
static BOOL_T AAA_OM_FindGroupNameOfDot1xList(const char *group_name);
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
static AAA_AuthorExecEntry_T *AAA_OM_GetAuthorExecEntry(AAA_ExecType_T exec_type);
static AAA_AuthorListEntry_T *AAA_OM_GetAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type);
static void AAA_OM_CopyAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *det, const AAA_AuthorListEntry_T *src);
static AAA_AuthorListEntry_T *AAA_OM_AllocAuthorListEntry(const AAA_ListType_T *list_type);
static AAA_ApiUpdateResult_T AAA_OM_CopyAuthorListEntry(AAA_AuthorListEntry_T *det, const AAA_AuthorListEntryInterface_T *src);
static BOOL_T AAA_OM_IsAnyExecReferThisAuthorList(const char *list_name);
static AAA_AuthorListEntry_T *AAA_OM_GetAuthorListEntryByIndex(UI16_T list_index);
static BOOL_T AAA_OM_IsValidAuthorMethodListName(const char *name);
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
static BOOL_T AAA_OM_GetAuthorCommandEntryIPC(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry);
static SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry);
static AAA_AuthorCommandEntry_T *AAA_OM_GetAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type);
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T       aaa_om_intialized = FALSE;

#ifdef AAA_OM_DEBUG_MODE
static UI32_T       aaa_om_backdoor_reg_no;
static UI32_T       aaa_om_lock_trace_reg_no;
#endif

static AAA_RadiusGroupEntry_T       aaa_om_radius_group[SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP];
static AAA_TacacsPlusGroupEntry_T   aaa_om_tacacs_group[SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP];

static UI32_T aaa_om_sem_id;
static UI32_T aaa_om_orig_priority;

#if (SYS_CPNT_ACCOUNTING == TRUE)

static UI32_T   accounting_update_interval;
static UI16_T   acc_dot1x_list_qty; /* to count the qty of aaa accounting dot1x method-list */
static UI16_T   acc_exec_list_qty;  /* to count the qty of aaa accounting exec method-list */
static UI16_T   acc_command_list_qty;  /* to count the qty of aaa accounting exec method-list */


static AAA_AccListEntry_T           method_list[AAA_OM_MAX_NBR_OF_ACC_LIST];

static AAA_AccDot1xEntry_T          acc_dot1x_entry[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static AAA_AccExecEntry_T           acc_exec_entry[AAA_EXEC_TYPE_SUPPORT_NUMBER];

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
static AAA_AccCommandEntry_T        acc_command_entry[SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE][AAA_EXEC_TYPE_SUPPORT_NUMBER];
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

static AAA_AccUserEntry_T           acc_user_info[AAA_MAX_NBR_OF_ACC_USERS];
static AAA_AccUserEntry_T           *head_of_acc_user;
static AAA_AccUserEntry_T           *tail_of_acc_user;

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
static UI16_T   author_exec_list_qty;  /* to count the qty of aaa authorization exec method-list */

static AAA_AuthorListEntry_T        author_method_list[AAA_OM_MAX_NBR_OF_AUTHOR_LIST];
static AAA_AuthorExecEntry_T        author_exec_entry[AAA_EXEC_TYPE_SUPPORT_NUMBER];
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
static UI16_T                       author_command_list_qty;
static AAA_AuthorCommandEntry_T     author_command_entry[SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE][AAA_EXEC_TYPE_SUPPORT_NUMBER];
#endif /* SYS_CPNT_AUTHORIZATION_COMMAND == TRUE */

/* EXPORTED SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_CreatSem
 * ------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for AAA objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_OM_CreatSem(void)
{
    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_AAA_OM, &aaa_om_sem_id) != SYSFUN_OK)
    {
        printf("\r\n[AAA_MGR_Initiate_System_Resources] Can't create semaphore.");
        return FALSE;
    }
    return TRUE;
} /* End of AAA_OM_CreatSem */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_Initialize
 *-------------------------------------------------------------------------
 * PURPOSE  : intialize om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_Initialize()
{
    UI16_T  index, entry_index;

    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;
    AAA_RadiusEntry_T           *radius_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

#if (SYS_CPNT_ACCOUNTING == TRUE)
    AAA_AccListEntry_T          *list_entry;
    AAA_AccDot1xEntry_T         *dot1x_entry;
    AAA_AccExecEntry_T          *exec_entry;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
#if (SYS_CPNT_AUTHORIZATION == TRUE)
    AAA_AuthorListEntry_T       *author_list_entry;
    AAA_AuthorExecEntry_T       *author_exec_entry_p;
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#ifdef AAA_OM_DEBUG_MODE
    SECURITY_BACKDOOR_Register("aaa_om", &aaa_om_backdoor_reg_no);
    SECURITY_BACKDOOR_Register("aaa_om lock trace", &aaa_om_lock_trace_reg_no);
#endif

    AAA_OM_Finalize();

    /* initialize radius group */
    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP > index; ++index)
    {
        radius_group = &aaa_om_radius_group[index];

        radius_group->group_index = index + 1;
        radius_group->modified_time = 0;
        radius_group->entry_status = AAA_ENTRY_DESTROYED;
        radius_group->group_name[0] = '\0';
        radius_group->head_of_radius_entry = NULL;
        radius_group->tail_of_radius_entry = NULL;

        for (entry_index = 0; AAA_MAX_NBR_OF_RADIUS_ENTRY > entry_index; ++entry_index)
        {
           radius_entry = &radius_group->radius_entry[entry_index];

           radius_entry->radius_index = entry_index + 1;

           radius_entry->next_radius_entry = NULL;
           radius_entry->prev_radius_entry = NULL;
           radius_entry->radius_server_index = 0;

           radius_entry->entry_status = AAA_ENTRY_DESTROYED;
        }
    }

    /* set default radius at zero index entry */
    radius_group = &aaa_om_radius_group[0];
    radius_group->entry_status = AAA_ENTRY_READY;
    strncpy((char *)radius_group->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    radius_group->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    /* initialize tacacs+ group */
    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP > index; ++index)
    {
        tacacs_group = &aaa_om_tacacs_group[index];

        tacacs_group->group_index = index + 1;
        tacacs_group->modified_time = 0;
        tacacs_group->entry_status = AAA_ENTRY_DESTROYED;
        tacacs_group->group_name[0] = '\0';
        tacacs_group->head_of_tacacs_entry = NULL;
        tacacs_group->tail_of_tacacs_entry = NULL;

        for (entry_index = 0; AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY > entry_index; ++entry_index)
        {
           tacacs_entry = &tacacs_group->tacacs_entry[entry_index];

           tacacs_entry->tacacs_index = entry_index + 1;

           tacacs_entry->next_tacacs_entry = NULL;
           tacacs_entry->prev_tacacs_entry = NULL;
           tacacs_entry->tacacs_server_index = 0;

           tacacs_entry->entry_status = AAA_ENTRY_DESTROYED;
        }
    }

    /* set default tacacs+ at zero index entry */
    tacacs_group = &aaa_om_tacacs_group[0];
    tacacs_group->entry_status = AAA_ENTRY_READY;
    strncpy((char *)tacacs_group->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    tacacs_group->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

#if (SYS_CPNT_ACCOUNTING == TRUE)

    accounting_update_interval = SYS_DFLT_ACCOUNTING_UPDATE_INTERVAL;

    acc_dot1x_list_qty = 0;
    acc_exec_list_qty = 0;
    acc_command_list_qty = 0;

    /* initialize method list */
    for (index = 0; AAA_OM_MAX_NBR_OF_ACC_LIST > index; ++index)
    {
        list_entry = &method_list[index];
        list_entry->list_index = index + 1;

        list_entry->list_name[0] = '\0';
        list_entry->group_name[0] = '\0';
        list_entry->client_type = AAA_CLIENT_TYPE_DOT1X;
        list_entry->working_mode = ACCOUNTING_START_STOP;

        list_entry->entry_status = AAA_ENTRY_DESTROYED;
    }

    /* set default DOT1X list at zero index entry */
    ++acc_dot1x_list_qty;

    list_entry = &method_list[0];

    strncpy((char *)list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    strncpy((char *)list_entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_DOT1X_LIST, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    list_entry->client_type = AAA_CLIENT_TYPE_DOT1X;
    list_entry->entry_status = AAA_ENTRY_READY;

    /* set default EXEC list at 1 index entry */
    ++acc_exec_list_qty;

    list_entry = &method_list[1];

    strncpy((char *)list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    strncpy((char *)list_entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_EXEC_LIST, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    list_entry->client_type = AAA_CLIENT_TYPE_EXEC;
    list_entry->entry_status = AAA_ENTRY_READY;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    /* set default COMMANDS list at 2+ index entry */
    {
        UI32_T                  priv_lvl;

        /* initialize command entry */
        for (priv_lvl = 0; SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE > priv_lvl; ++priv_lvl)
        {
            ++acc_command_list_qty;
            list_entry = &method_list[2 + priv_lvl];

            strncpy(list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
            list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

            strncpy(list_entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_COMMAND_LIST, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            list_entry->client_type  = AAA_CLIENT_TYPE_COMMANDS;
            list_entry->priv_lvl     = priv_lvl;
            list_entry->entry_status = AAA_ENTRY_READY;
        }
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

    /* initialize dot1x entry */
    for (index = 0; SYS_ADPT_TOTAL_NBR_OF_LPORT > index; ++index)
    {
        dot1x_entry = &acc_dot1x_entry[index];

        dot1x_entry->ifindex = index + 1;
        dot1x_entry->list_name[0] = '\0';
        dot1x_entry->configure_mode = AAA_AUTO_CONFIGURE;
    }

    /* initialize exec entry */
    for (index = AAA_EXEC_TYPE_NONE; AAA_EXEC_TYPE_SUPPORT_NUMBER > index; ++index)
    {
        exec_entry = &acc_exec_entry[index];

        exec_entry->exec_type = (AAA_ExecType_T)index + 1; /* because AAA_ExecType_T begin with 1 */
        exec_entry->list_name[0] = '\0';
        exec_entry->configure_mode = AAA_AUTO_CONFIGURE;
    }

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    {
        UI32_T                  priv_lvl, exec_type;
        AAA_AccCommandEntry_T  *cmd_entry_p;

        /* initialize command entry */
        for (priv_lvl = 0; SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE > priv_lvl; ++priv_lvl)
        {
            for (exec_type = AAA_EXEC_TYPE_NONE; AAA_EXEC_TYPE_SUPPORT_NUMBER > exec_type; ++exec_type)
            {
                cmd_entry_p = &acc_command_entry[priv_lvl][exec_type];

                cmd_entry_p->priv_lvl  = priv_lvl;
                cmd_entry_p->exec_type = (AAA_ExecType_T)exec_type + 1; /* because AAA_ExecType_T begin with 1 */
                cmd_entry_p->list_name[0] = '\0';
                cmd_entry_p->configure_mode = AAA_AUTO_CONFIGURE;
            }
        }
    }
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

    /* initialize user entry */
    for (index = 0; AAA_MAX_NBR_OF_ACC_USERS > index; ++index)
    {
        memset(&acc_user_info[index], 0, sizeof(AAA_AccUserEntry_T));

        acc_user_info[index].user_index = index + 1;
        acc_user_info[index].entry_status = AAA_ENTRY_DESTROYED;
    }

    head_of_acc_user = NULL;
    tail_of_acc_user = NULL;


#endif /* SYS_CPNT_ACCOUNTING == TRUE */

#if (SYS_CPNT_AUTHORIZATION == TRUE)
    author_exec_list_qty = 0;

    /* initialize method list */
    for (index = 0; AAA_OM_MAX_NBR_OF_AUTHOR_LIST > index; ++index)
    {
        author_list_entry = &author_method_list[index];
        author_list_entry->list_index = index + 1;

        author_list_entry->list_name[0] = '\0';
        author_list_entry->group_name[0] = '\0';
        author_list_entry->list_type.client_type = AAA_CLIENT_TYPE_EXEC;
        author_list_entry->entry_status = AAA_ENTRY_DESTROYED;
    }

    /* set default EXEC list at 1 index entry */
    ++author_exec_list_qty;

    author_list_entry = &author_method_list[0];

    strncpy((char *)author_list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
    author_list_entry->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

    strncpy((char *)author_list_entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_EXEC_LIST, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    author_list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    author_list_entry->list_type.client_type = AAA_CLIENT_TYPE_EXEC;
    author_list_entry->entry_status = AAA_ENTRY_READY;

    /* initialize exec entry */
    for (index = AAA_EXEC_TYPE_NONE; AAA_EXEC_TYPE_SUPPORT_NUMBER > index; ++index)
    {
        author_exec_entry_p = &author_exec_entry[index];

        author_exec_entry_p->exec_type = (AAA_ExecType_T)index + 1; /* because AAA_ExecType_T begin with 1 */
        author_exec_entry_p->list_name[0] = '\0';
        author_exec_entry_p->configure_mode = AAA_AUTO_CONFIGURE;
    }
#endif /* SYS_CPNT_AUTHORIZATION == TRUE */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    author_command_list_qty = 0;

    /* set default COMMANDS list at 2+ index entry */
    {
        UI32_T                  priv_lvl;

        /* initialize command entry */
        for (priv_lvl = 0; SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE > priv_lvl; ++priv_lvl)
        {
            ++author_command_list_qty;

            author_list_entry = &author_method_list[2 + priv_lvl];

            strncpy(author_list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
            author_list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

            strncpy(author_list_entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_COMMAND_LIST, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            author_list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            author_list_entry->list_type.client_type  = AAA_CLIENT_TYPE_COMMANDS;
            author_list_entry->list_type.priv_lvl     = priv_lvl;
            author_list_entry->entry_status           = AAA_ENTRY_READY;
        }
    }

    memset(author_command_entry, 0, sizeof(author_command_entry));

    {
        UI32_T                      priv_lvl, exec_type;
        AAA_AuthorCommandEntry_T    *author_entry_p;

        /* initialize command entry */
        for (priv_lvl = 0; SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE > priv_lvl; ++priv_lvl)
        {
            for (exec_type = AAA_EXEC_TYPE_NONE; AAA_EXEC_TYPE_SUPPORT_NUMBER > exec_type; ++exec_type)
            {
                author_entry_p = &author_command_entry[priv_lvl][exec_type];

                author_entry_p->priv_lvl  = priv_lvl;
                author_entry_p->exec_type = (AAA_ExecType_T)exec_type + 1; /* because AAA_ExecType_T begin with 1 */
                author_entry_p->configure_mode = AAA_AUTO_CONFIGURE;
            }
        }
    }

#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

    AAA_OM_TRACE("Initialize done");
    aaa_om_intialized = TRUE;

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_Finalize
 *-------------------------------------------------------------------------
 * PURPOSE  : cleanup om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
void AAA_OM_Finalize()
{
    if (TRUE != aaa_om_intialized)
    {
        /* om doesn't initialize yet */
        return;
    }

    aaa_om_intialized = FALSE;

    /* currently, nothing need to free */
    AAA_OM_TRACE("Finialize done");
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry)
{
    UI16_T      group_index;

    AAA_RadiusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    for (group_index = entry->group_index + 1; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP >= group_index; ++group_index)
    {
        group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
        if (NULL == group_entry)
            continue;

        if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
            continue;

        AAA_OM_CopyRadiusGroupEntryInterface(entry, group_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntry(entry->group_name);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyRadiusGroupEntryInterface(entry, group_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryInterfaceByIndex(AAA_RadiusGroupEntryInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(entry->group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyRadiusGroupEntryInterface(entry, group_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *entry)
{
    UI16_T      group_index;

    AAA_RadiusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (group_index = entry->group_index + 1; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP >= group_index; ++group_index)
    {
        group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
        if (NULL == group_entry)
            continue;

        AAA_OM_CopyRadiusGroupEntryInterface(entry, group_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (AAA_OM_IsValidGroupName(entry->group_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntry(entry->group_name);
    if (NULL == group_entry)
    {
        if (NULL != AAA_OM_GetTacacsPlusGroupEntry(entry->group_name))
        {
            AAA_OM_TRACE("Radius group name can't be duplicate with tacacs+ group name");
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        /* create a new entry */
        group_entry = AAA_OM_AllocRadiusGroupEntry();
        if (NULL == group_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        strncpy(group_entry->group_name, entry->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        group_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

        group_entry->head_of_radius_entry = NULL;
        group_entry->tail_of_radius_entry = NULL;

        group_entry->entry_status = AAA_ENTRY_READY;

        AAA_OM_TRACE("Create a new entry (%s)", group_entry->group_name);
    }
    else
    {
        /* modify a existent entry */

        /* nothing to do */
    }

    group_entry->modified_time = sys_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    AAA_RadiusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetRadiusGroupEntry(name);
    if (NULL == entry)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (FALSE == AAA_OM_FreeRadiusGroupEntry(entry->group_index)) /* should not use entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
    #if (SYS_CPNT_ACCOUNTING == TRUE)
        warning->warning_type = AAA_OM_IsAnyListReferThisGroup(name) ? AAA_LIST_REF2_BAD_GROUP : AAA_NO_WARNING;
    #else
        warning->warning_type = AAA_NO_WARNING;
    #endif /* SYS_CPNT_ACCOUNTING == TRUE */
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time)
{
    AAA_RadiusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == modified_time)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *modified_time = entry->modified_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle radius group
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : can't delete default "radius" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeRadiusGroupEntry(UI16_T group_index)
{
    AAA_RadiusGroupEntry_T  *entry;
    AAA_RadiusEntry_T       *radius_entry, *next_radius_entry;

    entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        return FALSE;
    }

    if (strcmp((char *)entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't delete default ""radius"" group");
        return FALSE;
    }

    AAA_OM_TRACE("Free group_index(%d)", group_index);

    entry->entry_status = AAA_ENTRY_DESTROYED;
    entry->group_name[0] = '\0';

    for (radius_entry = entry->head_of_radius_entry, next_radius_entry = NULL; NULL != radius_entry;
        radius_entry = next_radius_entry)
    {
        next_radius_entry = radius_entry->next_radius_entry;

        radius_entry->next_radius_entry = NULL;
        radius_entry->prev_radius_entry = NULL;
        radius_entry->radius_server_index = 0;

        radius_entry->entry_status = AAA_ENTRY_DESTROYED;
    }

    entry->head_of_radius_entry = NULL;
    entry->tail_of_radius_entry = NULL;

    return TRUE;
}



#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  : get AAA_RadiusGroupEntryMIBInterface_T entry by group_index.
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetRadiusGroupTable(UI16_T group_index, AAA_RadiusGroupEntryMIBInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    UI32_T        bitmap_size;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry->group_index = group_entry->group_index;
    strcpy((char *)entry->group_name, (char *)group_entry->group_name);
    entry->radius_server_list = 0;

	bitmap_size = sizeof(entry->radius_server_list) * 8;

    for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
        radius_entry = radius_entry->next_radius_entry)
    {
        if (bitmap_size < radius_entry->radius_server_index)
        {
            AAA_OM_TRACE("radius_server_index(%lu) large than bi1tmap size", radius_entry->radius_server_index);
            continue;
        }

        entry->radius_server_list |= 1 << (bitmap_size - radius_entry->radius_server_index);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  : get the next AAA_RadiusGroupEntryMIBInterface_T entry by group_index.
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetNextRadiusGroupTable(UI16_T group_index, AAA_RadiusGroupEntryMIBInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    UI32_T      bitmap_size;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

	bitmap_size = sizeof(entry->radius_server_list) * 8;

    for (++group_index; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP >= group_index; ++group_index)
    {
        group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
        if (NULL == group_entry)
            continue;

        entry->group_index = group_entry->group_index;
        strcpy((char *)entry->group_name, (char *)group_entry->group_name);
        entry->radius_server_list = 0;

        for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
            radius_entry = radius_entry->next_radius_entry)
        {
            if (bitmap_size < radius_entry->radius_server_index)
            {
                AAA_OM_TRACE("radius_server_index(%lu) large than bitmap size", radius_entry->radius_server_index);
                continue;
            }

            entry->radius_server_list |= 1 << (bitmap_size - radius_entry->radius_server_index);
        }

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetRadiusGroupEntryName
 * ------------------------------------------------------------------------
 * PURPOSE  : set group name and modified time by group_index.
 * INPUT    : group_index, group_name, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change default "radius" group name
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_SetRadiusGroupEntryName(UI16_T group_index, char* group_name, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == group_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        AAA_OM_TRACE("Bad group_index(%d)", group_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)entry->group_name, (char *)group_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (strcmp((char *)entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't change the default ""radius"" group name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (AAA_OM_IsValidGroupName(group_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if ((AAA_OM_GetRadiusGroupEntry(group_name) != NULL) ||
        (AAA_OM_GetTacacsPlusGroupEntry(group_name) != NULL))
    {
        /* group name can't duplicate */
        AAA_OM_TRACE("There is a group with the same name(%s)", group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    strncpy((char *)entry->group_name, (char *)group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    entry->modified_time = sys_time;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_SetRadiusGroupEntryServerBitMap(UI16_T group_index, UI8_T server_bitmap, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    const UI32_T        bitmap_size = sizeof(server_bitmap) * 8;
    const UI32_T        bitmap_mask = 1 << (bitmap_size - 1);

    UI32_T  bit_index;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't change the default ""radius"" group's radius entry");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* clean all entry */
    AAA_OM_ResetRadiusEntry(group_entry);

    /* allocate server entry */
    for (bit_index = bitmap_size; 0 < bit_index; --bit_index)
    {
        if (server_bitmap & bitmap_mask)
        {
            radius_entry = AAA_OM_AllocRadiusEntry(group_entry);
            if (NULL == radius_entry)
            {
                AAA_OM_TRACE("Should not go here Coz size is equal to radius_om!");
                break;
            }

            radius_entry->radius_server_index = bitmap_size - bit_index + 1;

            AAA_OM_TRACE("server_index(%lu) join group(%s)",
                radius_entry->radius_server_index, group_entry->group_name);
        }
        server_bitmap <<= 1;
    }

    group_entry->modified_time = sys_time;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetRadiusGroupEntryStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : set the group status and modified time
 * INPUT    : group_index, entry_status, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't delete the default "radius" group
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusGroupEntryStatus(UI16_T group_index, AAA_EntryStatus_T entry_status, UI32_T sys_time)
{
    BOOL_T  ret;

    AAA_RadiusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch (entry_status)
    {
        case AAA_ENTRY_READY:

            entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
            if (NULL != entry) /* already existed */
                break;

            entry = AAA_OM_GetRadiusGroupEntry(AAA_SNMP_CREATE_DFLT_RADIUS_GROUP_NAME);
            if (NULL != entry)
            {
                AAA_OM_TRACE("Group name can not duplicate");
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            if ((0 >= group_index) || (SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP < group_index))
            {
                AAA_OM_TRACE("Bad group_index(%d)", group_index);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            entry = &aaa_om_radius_group[group_index - 1]; /* to zero-based */

            strncpy((char *)entry->group_name, AAA_SNMP_CREATE_DFLT_RADIUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            entry->head_of_radius_entry = NULL;
            entry->tail_of_radius_entry = NULL;

            entry->entry_status = AAA_ENTRY_READY;
            break;

        case AAA_ENTRY_DESTROYED:
            ret = AAA_OM_FreeRadiusGroupEntry(group_index);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);

        default:
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry->modified_time = sys_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    UI16_T      group_index;

    AAA_TacacsPlusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    for (group_index = entry->group_index + 1; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP >= group_index; ++group_index)
    {
        group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
        if (NULL == group_entry)
            continue;

        if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
            continue;

        AAA_OM_CopyTacacsPlusGroupEntryInterface(entry, group_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(entry->group_name);
    if (NULL == tacacs_group)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyTacacsPlusGroupEntryInterface(entry, tacacs_group);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs+ group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryInterfaceByIndex(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(entry->group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyTacacsPlusGroupEntryInterface(entry, group_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    UI16_T      group_index;

    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (group_index = entry->group_index + 1; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP >= group_index; ++group_index)
    {
        tacacs_group = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
        if (NULL == tacacs_group)
            continue;

        AAA_OM_CopyTacacsPlusGroupEntryInterface(entry, tacacs_group);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (AAA_OM_IsValidGroupName(entry->group_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

#if (SYS_CPNT_ACCOUNTING == TRUE)
#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a tacacs+ group associate with a dot1x method-list */
    if (TRUE == AAA_OM_FindGroupNameOfDot1xList(entry->group_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(entry->group_name);
    if (NULL == tacacs_group)
    {
        if (NULL != AAA_OM_GetRadiusGroupEntry(entry->group_name))
        {
            AAA_OM_TRACE("TACACS+ group name can't be duplicate with RADIUS group name");
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        /* create a new entry */
        tacacs_group = AAA_OM_AllocTacacsPlusGroupEntry();
        if (NULL == tacacs_group)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        strncpy((char *)tacacs_group->group_name, (char *)entry->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        tacacs_group->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

        tacacs_group->head_of_tacacs_entry = NULL;
        tacacs_group->tail_of_tacacs_entry = NULL;

        tacacs_group->entry_status = AAA_ENTRY_READY;

        AAA_OM_TRACE("Create a new entry (%s)", tacacs_group->group_name);
    }
    else
    {
        /* modify a existent entry */

        /* nothing to do */
    }

    tacacs_group->modified_time = sys_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning)
{
    AAA_TacacsPlusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetTacacsPlusGroupEntry(name);
    if (NULL == entry)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (FALSE == AAA_OM_FreeTacacsPlusGroupEntry(entry->group_index)) /* should not use entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
    #if (SYS_CPNT_ACCOUNTING == TRUE)
        warning->warning_type = AAA_OM_IsAnyListReferThisGroup(name) ? AAA_LIST_REF2_BAD_GROUP : AAA_NO_WARNING;
    #else
        warning->warning_type = AAA_NO_WARNING;
    #endif /* SYS_CPNT_ACCOUNTING == TRUE */
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time)
{
    AAA_TacacsPlusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == modified_time)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *modified_time = entry->modified_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle tacacs group
 * INPUT    : group_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * RETURN   : can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeTacacsPlusGroupEntry(UI16_T group_index)
{
    AAA_TacacsPlusGroupEntry_T  *entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry, *next_tacacs_entry;

    entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        return FALSE;
    }

    if (strcmp((char *)entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't delete default ""tacacs+"" group");
        return FALSE;
    }

    AAA_OM_TRACE("Free group_index(%d)", group_index);

    entry->entry_status = AAA_ENTRY_DESTROYED;
    entry->group_name[0] = '\0';

    for (tacacs_entry = entry->head_of_tacacs_entry, next_tacacs_entry = NULL; NULL != tacacs_entry;
        tacacs_entry = next_tacacs_entry)
    {
        next_tacacs_entry = tacacs_entry->next_tacacs_entry;

        tacacs_entry->next_tacacs_entry = NULL;
        tacacs_entry->prev_tacacs_entry = NULL;
        tacacs_entry->tacacs_server_index = 0;

        tacacs_entry->entry_status = AAA_ENTRY_DESTROYED;

        tacacs_entry = next_tacacs_entry;
    }

    entry->head_of_tacacs_entry = NULL;
    entry->tail_of_tacacs_entry = NULL;

    return TRUE;
}



#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE)

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  : get AAA_TacacsPlusGroupEntryMIBInterface_T entry by group_index.
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetTacacsPlusGroupTable(UI16_T group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    UI32_T        bitmap_size;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry->group_index = group_entry->group_index;
    strcpy((char *)entry->group_name, (char *)group_entry->group_name);
    entry->tacacsplus_server_list = 0;

	bitmap_size = sizeof(entry->tacacsplus_server_list) * 8;

    for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
        tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (bitmap_size < tacacs_entry->tacacs_server_index)
        {
            AAA_OM_TRACE("tacacs_server_index(%lu) large than bi1tmap size", tacacs_entry->tacacs_server_index);
            continue;
        }

        entry->tacacsplus_server_list |= 1 << (bitmap_size - tacacs_entry->tacacs_server_index);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  : get the next AAA_TacacsPlusGroupEntryMIBInterface_T entry by group_index.
 * INPUT    : group_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetNextTacacsPlusGroupTable(UI16_T group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    UI32_T      bitmap_size;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

	bitmap_size = sizeof(entry->tacacsplus_server_list) * 8;

    for (++group_index; SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP >= group_index; ++group_index)
    {
        group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
        if (NULL == group_entry)
            continue;

        entry->group_index = group_entry->group_index;
        strcpy((char *)entry->group_name, (char *)group_entry->group_name);
        entry->tacacsplus_server_list = 0;

        for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
            tacacs_entry = tacacs_entry->next_tacacs_entry)
        {
            if (bitmap_size < tacacs_entry->tacacs_server_index)
            {
                AAA_OM_TRACE("tacacs_server_index(%lu) large than bitmap size", tacacs_entry->tacacs_server_index);
                continue;
            }

            entry->tacacsplus_server_list |= 1 << (bitmap_size - tacacs_entry->tacacs_server_index);
        }

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetTacacsPlusGroupEntryName
 * ------------------------------------------------------------------------
 * PURPOSE  : set group name and modified time by group_index.
 * INPUT    : group_index, group_name, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change default "tacacs+" group name
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_SetTacacsPlusGroupEntryName(UI16_T group_index, char* group_name, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == group_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == entry)
    {
        AAA_OM_TRACE("Bad group_index(%d)", group_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)entry->group_name, (char *)group_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (strcmp((char *)entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't change the default ""tacacs+"" group name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (AAA_OM_IsValidGroupName(group_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if ((AAA_OM_GetRadiusGroupEntry(group_name) != NULL) ||
        (AAA_OM_GetTacacsPlusGroupEntry(group_name) != NULL))
    {
        /* group name can't duplicate */
        AAA_OM_TRACE("There is a group with the same name(%s)", group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

#if (SYS_CPNT_ACCOUNTING == TRUE)
#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a tacacs+ group associate with a dot1x method-list */
    if (TRUE == AAA_OM_FindGroupNameOfDot1xList(group_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

    strncpy((char *)entry->group_name, (char *)group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    entry->modified_time = sys_time;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_SetTacacsPlusGroupEntryServerBitMap(UI16_T group_index, UI8_T server_bitmap, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    const UI32_T        bitmap_size = sizeof(server_bitmap) * 8;
    const UI32_T        bitmap_mask = 1 << (bitmap_size - 1);

    UI32_T  bit_index;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't change the default ""tacacs+"" group's tacacs entry");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    /* clean all entry */
    AAA_OM_ResetTacacsPlusEntry(group_entry);

    /* allocate server entry */
    for (bit_index = bitmap_size; 0 < bit_index; --bit_index)
    {
        if (server_bitmap & bitmap_mask)
        {
            tacacs_entry = AAA_OM_AllocTacacsPlusEntry(group_entry);
            if (NULL == tacacs_entry)
            {
                AAA_OM_TRACE("Should not go here Coz size is equal to tacacs_om!");
                break;
            }

            tacacs_entry->tacacs_server_index = bitmap_size - bit_index + 1;

            AAA_OM_TRACE("server_index(%lu) join group(%s)",
                tacacs_entry->tacacs_server_index, group_entry->group_name);
        }
        server_bitmap <<= 1;
    }

    group_entry->modified_time = sys_time;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetTacacsPlusGroupEntryStatus
 * ------------------------------------------------------------------------
 * PURPOSE  : set the group status and modified time
 * INPUT    : group_index, entry_status, sys_time
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't delete the default "tacacs+" group
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusGroupEntryStatus(UI16_T group_index, AAA_EntryStatus_T entry_status, UI32_T sys_time)
{
    BOOL_T  ret;

    AAA_TacacsPlusGroupEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch (entry_status)
    {
        case AAA_ENTRY_READY:

            entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
            if (NULL != entry) /* already existed */
                break;

            entry = AAA_OM_GetTacacsPlusGroupEntry(AAA_SNMP_CREATE_DFLT_TACACS_PLUS_GROUP_NAME);
            if (NULL != entry)
            {
                AAA_OM_TRACE("Group name can not duplicate");
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            if ((0 >= group_index) || (SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP < group_index))
            {
                AAA_OM_TRACE("Bad group_index(%d)", group_index);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

        #if (SYS_CPNT_ACCOUNTING == TRUE)
        #if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

            /* do not allow a tacacs+ group associate with a dot1x method-list */
            if (TRUE == AAA_OM_FindGroupNameOfDot1xList(AAA_SNMP_CREATE_DFLT_TACACS_PLUS_GROUP_NAME))
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
        #endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */
        #endif /* SYS_CPNT_ACCOUNTING == TRUE */

            entry = &aaa_om_tacacs_group[group_index - 1]; /* to zero-based */

            strncpy((char *)entry->group_name, AAA_SNMP_CREATE_DFLT_TACACS_PLUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            entry->head_of_tacacs_entry = NULL;
            entry->tail_of_tacacs_entry = NULL;

            entry->entry_status = AAA_ENTRY_READY;
            break;

        case AAA_ENTRY_DESTROYED:
            ret = AAA_OM_FreeTacacsPlusGroupEntry(group_index);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);

        default:
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    entry->modified_time = sys_time;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index, entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusEntryInterface(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (0 == entry->radius_index)
    {
        /* get the first entry */
        radius_entry = group_entry->head_of_radius_entry;
    }
    else
    {
        /* get the next entry */
        radius_entry = group_entry->radius_entry[entry->radius_index - 1].next_radius_entry; /* to zero-based */
    }

    if (NULL == radius_entry)
    {
        /* can't find the next one */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyRadiusEntryInterface(entry, radius_entry);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : radius_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryInterface(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_entry = AAA_OM_GetRadiusEntry(group_entry, entry->radius_index);
    if (NULL == radius_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyRadiusEntryInterface(entry, radius_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryInterfaceByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th radius entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryInterfaceByOrder(UI16_T group_index, UI16_T order, AAA_RadiusEntryInterface_T *entry)
{
    UI16_T  radius_counter;

    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_counter = 1;
    radius_entry = group_entry->head_of_radius_entry;
    for ( ; NULL == radius_entry; radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_counter != order)
            continue;

        AAA_OM_CopyRadiusEntryInterface(entry, radius_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order)
{
    UI16_T  radius_counter;

    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == order)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_counter = 1;
    radius_entry = group_entry->head_of_radius_entry;
    for ( ; NULL != radius_entry; radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_index != radius_entry->radius_index)
            continue;

        *order = radius_counter;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

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
BOOL_T AAA_OM_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_TRACE("Entry is null pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't add any entry to default ""radius"" group");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
        radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_entry->radius_server_index == entry->radius_server_index)
        {
            /* already exist */
            if (warning)
            {
                /* Currently, we don't allow an invalid radius join a group */
                warning->warning_type = AAA_NO_WARNING;
            }

            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    radius_entry = AAA_OM_AllocRadiusEntry(group_entry);
    if (NULL == radius_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_entry->radius_server_index = entry->radius_server_index;
    group_entry->modified_time = sys_time;

    AAA_OM_TRACE("server_index(%lu) join group(%s)",
        radius_entry->radius_server_index, group_entry->group_name);

    if (warning)
    {
        /* Currently, we don't allow an invalid radius join a group */
        warning->warning_type = AAA_NO_WARNING;
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group and update group modified time
 * INPUT    : radius_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetRadiusGroupEntry(SYS_DFLT_AAA_RADIUS_GROUP_NAME);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
        radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_entry->radius_server_index == radius_server_index)
        {
            /* already exist */
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    radius_entry = AAA_OM_AllocRadiusEntry(group_entry);
    if (NULL == radius_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_entry->radius_server_index = radius_server_index;
    group_entry->modified_time = sys_time;

    AAA_OM_TRACE("server_index(%lu) join group(%s)",
        radius_entry->radius_server_index, group_entry->group_name);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group and update group modified time
 * INPUT    : radius_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetRadiusGroupEntry(SYS_DFLT_AAA_RADIUS_GROUP_NAME);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
        radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_entry->radius_server_index == radius_server_index)
        {
            /* found */
            AAA_OM_FreeRadiusEntry(group_entry, radius_entry->radius_index); /* should not use radius_entry thereafter */
            group_entry->modified_time = sys_time;
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time)
{
    AAA_RadiusGroupEntry_T  *group_entry;
    AAA_RadiusEntry_T       *radius_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't delete any entry from default ""radius"" group");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (radius_entry = group_entry->head_of_radius_entry; NULL != radius_entry;
        radius_entry = radius_entry->next_radius_entry)
    {
        if (radius_entry->radius_server_index == entry->radius_server_index)
        {
            /* found */
            AAA_OM_FreeRadiusEntry(group_entry, radius_entry->radius_index); /* should not use radius_entry thereafter */
            group_entry->modified_time = sys_time;

            if (warning)
            {
                warning->warning_type =
                    (NULL == group_entry->head_of_radius_entry) ? AAA_GROUP_HAS_NO_ENTRY : AAA_NO_WARNING;
            }

            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}


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
BOOL_T AAA_OM_IsRadiusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    AAA_AccDot1xEntry_T         *dot1x_entry;
    AAA_AccExecEntry_T          *exec_entry;
    AAA_AccListEntry_T          *list_entry;
    AAA_RadiusGroupEntry_T      *radius_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch(client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
            if (NULL == dot1x_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(dot1x_entry->list_name, AAA_CLIENT_TYPE_DOT1X);
            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry(%s) not found", dot1x_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;

        case AAA_CLIENT_TYPE_EXEC:
            exec_entry = AAA_OM_GetAccExecEntry(ifindex);
            if (NULL == exec_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(exec_entry->list_name, AAA_CLIENT_TYPE_EXEC);
            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry(%s) not found", exec_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;

        default:
            AAA_OM_TRACE("Bad client type");
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_group = AAA_OM_GetRadiusGroupEntryByIndex(group_index);
    if (NULL == radius_group)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->group_name, (char *)radius_group->group_name) == 0)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsRadiusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the radius entry is valid or not according to input params
 * INPUT    : radius_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsRadiusEntryValid(UI16_T radius_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T      ret;

    AAA_AccInterface_T              acc_interface;
    AAA_QueryGroupIndexResult_T     query_result;
    AAA_RadiusGroupEntry_T          *entry;

    memset(&acc_interface, 0, sizeof(acc_interface));
    acc_interface.client_type = client_type;
    acc_interface.ifindex     = ifindex;

    if (FALSE == AAA_OM_GetAccountingGroupIndexByInterface(&acc_interface, &query_result))
    {
        return FALSE;
    }

    switch (query_result.group_type)
    {
        case GROUP_RADIUS:
            break;

        case GROUP_TACACS_PLUS:
        case GROUP_UNKNOWN:
        default:
            return FALSE;
    }

    AAA_OM_ENTER_CRITICAL_SECTION();

    entry = AAA_OM_GetRadiusGroupEntryByIndex(query_result.group_index);
    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    ret = (NULL != AAA_OM_GetRadiusEntry(entry, radius_index));

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

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
BOOL_T AAA_OM_GetNextTacacsPlusEntryInterface(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (0 == entry->tacacs_index)
    {
        /* get the first entry */
        tacacs_entry = group_entry->head_of_tacacs_entry;
    }
    else
    {
        /* get the next entry */
        tacacs_entry = group_entry->tacacs_entry[entry->tacacs_index - 1].next_tacacs_entry; /* to zero-based */
    }

    if (NULL == tacacs_entry)
    {
        /* can't find the next one */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyTacacsPlusEntryInterface(entry, tacacs_entry);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : tacacs_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryInterface(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_entry = AAA_OM_GetTacacsPlusEntry(group_entry, entry->tacacs_index);
    if (NULL == tacacs_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyTacacsPlusEntryInterface(entry, tacacs_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryInterfaceByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th tacacs entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryInterfaceByOrder(UI16_T group_index, UI16_T order, AAA_TacacsPlusEntryInterface_T *entry)
{
    UI16_T  tacacs_counter;

    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_counter = 1;
    tacacs_entry = group_entry->head_of_tacacs_entry;
    for ( ; NULL == tacacs_entry; tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_counter != order)
            continue;

        AAA_OM_CopyTacacsPlusEntryInterface(entry, tacacs_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs entry's order in group
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntryOrder(UI16_T group_index, UI16_T tacacs_index, UI16_T *order)
{
    UI16_T  tacacs_counter;

    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == order)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_counter = 1;
    tacacs_entry = group_entry->head_of_tacacs_entry;
    for ( ; NULL != tacacs_entry; tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_index != tacacs_entry->tacacs_index)
            continue;

        *order = tacacs_counter;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

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
BOOL_T AAA_OM_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't add any entry to default ""tacacs+"" group");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
        tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_entry->tacacs_server_index == entry->tacacs_server_index)
        {
            /* already exist */
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    tacacs_entry = AAA_OM_AllocTacacsPlusEntry(group_entry);
    if (NULL == tacacs_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_entry->tacacs_server_index = entry->tacacs_server_index;
    group_entry->modified_time = sys_time;

    AAA_OM_TRACE("server_index(%lu) join group(%s)",
        tacacs_entry->tacacs_server_index, group_entry->group_name);

    if (warning)
    {
        /* Currently, we don't allow an invalid tacacs join a group */
        warning->warning_type = AAA_NO_WARNING;
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs entry join default "tacacs+" group and update group modified time
 * INPUT    : tacacs_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetTacacsPlusGroupEntry(SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
        tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_entry->tacacs_server_index == tacacs_server_index)
        {
            /* already exist */
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    tacacs_entry = AAA_OM_AllocTacacsPlusEntry(group_entry);
    if (NULL == tacacs_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_entry->tacacs_server_index = tacacs_server_index;
    group_entry->modified_time = sys_time;

    AAA_OM_TRACE("server_index(%lu) join group(%s)",
        tacacs_entry->tacacs_server_index, group_entry->group_name);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs entry depart default "tacacs+" group and update group modified time
 * INPUT    : tacacs_server_index, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    group_entry = AAA_OM_GetTacacsPlusGroupEntry(SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
        tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_entry->tacacs_server_index == tacacs_server_index)
        {
            /* found */
            AAA_OM_FreeTacacsPlusEntry(group_entry, tacacs_entry->tacacs_index); /* should not use tacacs_entry thereafter */
            group_entry->modified_time = sys_time;
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning, UI32_T sys_time)
{
    AAA_TacacsPlusGroupEntry_T  *group_entry;
    AAA_TacacsPlusEntry_T       *tacacs_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    group_entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == group_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)group_entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0)
    {
        AAA_OM_TRACE("Can't delete any entry from default ""tacacs+"" group");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (tacacs_entry = group_entry->head_of_tacacs_entry; NULL != tacacs_entry;
        tacacs_entry = tacacs_entry->next_tacacs_entry)
    {
        if (tacacs_entry->tacacs_server_index == entry->tacacs_server_index)
        {
            /* found */
            AAA_OM_FreeTacacsPlusEntry(group_entry, tacacs_entry->tacacs_index); /* should not use tacacs_entry thereafter */
            group_entry->modified_time = sys_time;

            if (warning)
            {
                warning->warning_type =
                    (NULL == group_entry->head_of_tacacs_entry) ? AAA_GROUP_HAS_NO_ENTRY : AAA_NO_WARNING;
            }

            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAccUpdateInterval
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccUpdateInterval(UI32_T *update_interval)
{

    if (FALSE == AAA_OM_GetAccUpdateInterval(update_interval))
    {
        return SYS_TYPE_GET_RUNNING_CFG_FAIL;
    }

    if (SYS_DFLT_ACCOUNTING_UPDATE_INTERVAL == *update_interval)
    {
        return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    }

    return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsTacacsPlusGroupValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the group entry is valid or not according to input params
 * INPUT    : group_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - yes; FALSE - no
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsTacacsPlusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    AAA_AccDot1xEntry_T         *dot1x_entry;
    AAA_AccExecEntry_T          *exec_entry;
    AAA_AccListEntry_T          *list_entry;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch(client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
            if (NULL == dot1x_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(dot1x_entry->list_name, AAA_CLIENT_TYPE_DOT1X);
            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry(%s) not found", dot1x_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;

        case AAA_CLIENT_TYPE_EXEC:
            exec_entry = AAA_OM_GetAccExecEntry(ifindex);
            if (NULL == exec_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(exec_entry->list_name, AAA_CLIENT_TYPE_EXEC);
            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry(%s) not found", exec_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;

        default:
            AAA_OM_TRACE("Bad client type");
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_group = AAA_OM_GetTacacsPlusGroupEntryByIndex(group_index);
    if (NULL == tacacs_group)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->group_name, (char *)tacacs_group->group_name) == 0)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_IsTacacsPlusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the tacacs entry is valid or not according to input params
 * INPUT    : tacacs_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_IsTacacsPlusEntryValid(UI16_T tacacs_index, AAA_ClientType_T client_type, UI32_T ifindex)
{
    BOOL_T      ret;

    AAA_AccInterface_T              acc_interface;
    AAA_QueryGroupIndexResult_T     query_result;
    AAA_TacacsPlusGroupEntry_T          *entry;

    memset(&acc_interface, 0, sizeof(acc_interface));
    acc_interface.client_type = client_type;
    acc_interface.ifindex     = ifindex;

    if (FALSE == AAA_OM_GetAccountingGroupIndexByInterface(&acc_interface, &query_result))
    {
        return FALSE;
    }

    switch (query_result.group_type)
    {
        case GROUP_TACACS_PLUS:
            break;

        case GROUP_RADIUS:
        case GROUP_UNKNOWN:
        default:
            return FALSE;
    }

    AAA_OM_ENTER_CRITICAL_SECTION();

    entry = AAA_OM_GetTacacsPlusGroupEntryByIndex(query_result.group_index);
    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    ret = (NULL != AAA_OM_GetTacacsPlusEntry(entry, tacacs_index));

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting update interval (minutes)
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUpdateInterval(UI32_T *update_interval)
{
    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == update_interval)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *update_interval = accounting_update_interval;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the accounting update interval (minutes)
 * INPUT    : update_interval
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUpdateInterval(UI32_T update_interval)
{
    AAA_OM_ENTER_CRITICAL_SECTION();

    accounting_update_interval = update_interval;

    AAA_OM_TRACE("Set update interval (%lu)", accounting_update_interval);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccUpdateInterval()
{
    AAA_OM_ENTER_CRITICAL_SECTION();

    accounting_update_interval = 0;

    AAA_OM_TRACE("Disable update interval");
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAccDot1xEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    dot1x_entry = AAA_OM_GetAccDot1xEntry(entry->ifindex);
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (AAA_MANUAL_CONFIGURE == dot1x_entry->configure_mode)
    {
        memcpy(entry, dot1x_entry, sizeof(AAA_AccDot1xEntry_T));
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    UI32_T      ifindex;

    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    for (ifindex = entry->ifindex + 1; SYS_ADPT_TOTAL_NBR_OF_LPORT >= ifindex; ++ifindex) /* 1-based */
    {
        dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
        if (NULL == dot1x_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        }

        if (AAA_MANUAL_CONFIGURE == dot1x_entry->configure_mode)
        {
            memcpy(entry, dot1x_entry, sizeof(AAA_AccDot1xEntry_T));
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntry(AAA_AccDot1xEntry_T *entry)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    dot1x_entry = AAA_OM_GetAccDot1xEntry(entry->ifindex + 1); /* next one */
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, dot1x_entry, sizeof(AAA_AccDot1xEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntryFilterList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next dot1x accounting entry's ifindex with specific list_name by ifindex.
 * INPUT    : ifindex, list_name
 * OUTPUT   : ifindex
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntryFilterList(UI32_T *ifindex, const char *list_name)
{
    UI32_T      index;

    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == ifindex) || (NULL == list_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (index = *ifindex + 1; SYS_ADPT_TOTAL_NBR_OF_LPORT >= index; ++index) /* 1-based */
    {
        dot1x_entry = AAA_OM_GetAccDot1xEntry(index);
        if (NULL == dot1x_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        if (strcmp((char *)list_name, (char *)dot1x_entry->list_name) != 0)
            continue;

        *ifindex = index;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_TRACE("No more entry");
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccDot1xEntryFilterGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next dot1x accounting entry's ifindex with specific group_name by ifindex.
 * INPUT    : ifindex, group_name
 * OUTPUT   : ifindex
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccDot1xEntryFilterGroup(UI32_T *ifindex, const char *group_name)
{
    UI32_T      index;

    AAA_AccDot1xEntry_T     *dot1x_entry;
    AAA_AccListEntry_T      *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == ifindex) || (NULL == group_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (index = *ifindex + 1; SYS_ADPT_TOTAL_NBR_OF_LPORT >= index; ++index) /* 1-based */
    {
        dot1x_entry = AAA_OM_GetAccDot1xEntry(index);
        if (NULL == dot1x_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        if (strlen((char *)dot1x_entry->list_name) <= 0)
            continue;

        list_entry = AAA_OM_GetAccListEntry(dot1x_entry->list_name, AAA_CLIENT_TYPE_DOT1X);
        if (NULL == list_entry)
            continue;

        if (strcmp((char *)list_entry->group_name, (char *)group_name) != 0)
            continue;

        *ifindex = index;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_TRACE("No more entry");
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccDot1xEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by ifindex.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccDot1xEntryInterface(AAA_AccDot1xEntry_T *entry)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    dot1x_entry = AAA_OM_GetAccDot1xEntry(entry->ifindex);
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, dot1x_entry, sizeof(AAA_AccDot1xEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    dot1x_entry = AAA_OM_GetAccDot1xEntry(entry->ifindex);
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strlen((char *)entry->list_name) == 0)
    {
        AAA_OM_TRACE("list_name can not be an empty string");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strcmp((char *)entry->list_name, (char *)dot1x_entry->list_name) == 0) /* the same name */
    {
        if (strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) /* default method list */
            dot1x_entry->configure_mode = AAA_MANUAL_CONFIGURE;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    strncpy((char *)dot1x_entry->list_name, (char *)entry->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    dot1x_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    dot1x_entry->configure_mode = AAA_MANUAL_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccDot1xEntry(UI32_T ifindex)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    dot1x_entry->list_name[0] = '\0';
    dot1x_entry->configure_mode = AAA_AUTO_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccDot1xEntryListName
 * ------------------------------------------------------------------------
 * PURPOSE  : set list-name by ifindex.
 * INPUT    : ifindex, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccDot1xEntryListName(UI32_T ifindex, char *list_name)
{
    I32_T  str_len;

    AAA_AccDot1xEntry_T  *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == list_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
    if (NULL == dot1x_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    str_len = strlen((char *)list_name);
    if (0 >= str_len)
    {
        AAA_OM_TRACE("list_name has a bad length(%ld)", str_len);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    strncpy((char *)dot1x_entry->list_name, (char *)list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    dot1x_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    dot1x_entry->configure_mode = AAA_MANUAL_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_QueryAccDot1xPortList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port list associated with specific list_index
 * INPUT    : query_result->list_index
 * OUTPUT   : query_result->port_list
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_QueryAccDot1xPortList(AAA_QueryAccDot1xPortListResult_T *query_result)
{
    UI16_T  index;

    AAA_AccListEntry_T      *list_entry;
    AAA_AccDot1xEntry_T     *dot1x_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == query_result)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntryByIndex(query_result->list_index);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (index = 0; SYS_ADPT_TOTAL_NBR_OF_LPORT > index; ++index)
    {
        dot1x_entry = &acc_dot1x_entry[index];

        if (strcmp((char *)dot1x_entry->list_name, (char *)list_entry->list_name) == 0)
            query_result->port_list[index] = '1';
        else
            query_result->port_list[index] = '0';
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}



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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccExecEntry(AAA_AccExecEntry_T *entry)
{
    UI32_T      index;

    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    for (index = entry->exec_type + 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index) /* 1-based */
    {
        exec_entry = AAA_OM_GetAccExecEntry(index);
        if (NULL == exec_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        }

        if (AAA_MANUAL_CONFIGURE == exec_entry->configure_mode)
        {
            memcpy(entry, exec_entry, sizeof(AAA_AccExecEntry_T));
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntry(AAA_AccExecEntry_T *entry)
{
    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry = AAA_OM_GetAccExecEntry(entry->exec_type + 1); /* next one */
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, exec_entry, sizeof(AAA_AccExecEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntryFilterList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next exec accounting entry's exec_type with specific list_name by exec_type.
 * INPUT    : exec_type, list_name
 * OUTPUT   : exec_type
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntryFilterList(AAA_ExecType_T *exec_type, const char *list_name)
{
    UI32_T      index;

    AAA_AccExecEntry_T      *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == exec_type) || (NULL == list_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (index = *exec_type + 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index)
    {
        exec_entry = AAA_OM_GetAccExecEntry(index);
        if (NULL == exec_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        if (strcmp((char *)list_name, (char *)exec_entry->list_name) != 0)
            continue;

        *exec_type = exec_entry->exec_type;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_TRACE("No more entry");
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntryFilterGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next exec accounting entry's exec_type with specific group_name by exec_type.
 * INPUT    : exec_type, group_name
 * OUTPUT   : exec_type
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntryFilterGroup(AAA_ExecType_T *exec_type, const char *group_name)
{
    UI32_T      index;

    AAA_AccExecEntry_T      *exec_entry;
    AAA_AccListEntry_T      *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if ((NULL == exec_type) || (NULL == group_name))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (index = *exec_type + 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index)
    {
        exec_entry = AAA_OM_GetAccExecEntry(index);
        if (NULL == exec_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        if (strlen((char *)exec_entry->list_name) <= 0)
            continue;

        list_entry = AAA_OM_GetAccListEntry(exec_entry->list_name, AAA_CLIENT_TYPE_EXEC);
        if (NULL == list_entry)
            continue;

        if (strcmp((char *)list_entry->group_name, (char *)group_name) != 0)
            continue;

        *exec_type = exec_entry->exec_type;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_TRACE("No more entry");
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccExecEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccExecEntryInterface(AAA_AccExecEntry_T *entry)
{
    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry = AAA_OM_GetAccExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, exec_entry, sizeof(AAA_AccExecEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccExecEntry(const AAA_AccExecEntry_T *entry)
{
    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    exec_entry = AAA_OM_GetAccExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strlen((char *)entry->list_name) == 0)
    {
        AAA_OM_TRACE("list_name can not be an empty string");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strcmp((char *)entry->list_name, (char *)exec_entry->list_name) == 0) /* the same name */
    {
        if (strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) /* default method list */
            exec_entry->configure_mode = AAA_MANUAL_CONFIGURE;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    strncpy((char *)exec_entry->list_name, (char *)entry->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    exec_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    exec_entry->configure_mode = AAA_MANUAL_CONFIGURE;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAccExecEntry(AAA_ExecType_T exec_type)
{
    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    exec_entry = AAA_OM_GetAccExecEntry(exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry->list_name[0] = '\0';
    exec_entry->configure_mode = AAA_AUTO_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the command accounting entry by priv level and exec_type.
 * INPUT    : priv_lvl, exec_type
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccCommandEntry_T *AAA_OM_GetAccCommandEntry(UI32_T priv_lvl, UI32_T exec_type)
{
    if ((priv_lvl < 0) || (priv_lvl > SYS_ADPT_MAX_LOGIN_PRIVILEGE) ||
        (exec_type <= AAA_EXEC_TYPE_NONE) || (exec_type > AAA_EXEC_TYPE_SUPPORT_NUMBER))
    {
        AAA_OM_TRACE("Bad priv_lvl(%ld) or exec_type(%ld)",
            priv_lvl, exec_type);
        return NULL;
    }

    return &acc_command_entry[priv_lvl][exec_type-1];
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p)
{
    AAA_AccCommandEntry_T     *cmd_entry_p;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry_p)
    {
        AAA_OM_TRACE("%s", "null pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    cmd_entry_p = AAA_OM_GetAccCommandEntry(entry_p->priv_lvl, entry_p->exec_type);
    if (NULL == cmd_entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (AAA_MANUAL_CONFIGURE == cmd_entry_p->configure_mode)
    {
        memcpy(entry_p, cmd_entry_p, sizeof(AAA_AccCommandEntry_T));
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

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
BOOL_T AAA_OM_GetAccCommandEntryInterface(AAA_AccCommandEntry_T *entry_p)
{
    AAA_AccCommandEntry_T     *ret_entry_p;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    ret_entry_p = AAA_OM_GetAccCommandEntry(entry_p->priv_lvl, entry_p->exec_type);
    if (NULL == ret_entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry_p, ret_entry_p, sizeof(*entry_p));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
AAA_ApiUpdateResult_T AAA_OM_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p)
{
    AAA_AccCommandEntry_T     *cmd_entry_p;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strlen(entry_p->list_name) == 0)
    {
        AAA_OM_TRACE("%s", "list name can not be an empty string");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    cmd_entry_p = AAA_OM_GetAccCommandEntry(entry_p->priv_lvl, entry_p->exec_type);
    if (NULL == cmd_entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strcmp(entry_p->list_name, cmd_entry_p->list_name) == 0) /* the same name */
    {
        if (strcmp(entry_p->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) /* default method list */
            cmd_entry_p->configure_mode = AAA_MANUAL_CONFIGURE;
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    strncpy(cmd_entry_p->list_name, entry_p->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    cmd_entry_p->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    cmd_entry_p->configure_mode = AAA_MANUAL_CONFIGURE;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

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
BOOL_T AAA_OM_DisableAccCommandEntry(UI32_T priv_lvl, AAA_AccExecType_T exec_type)
{
    AAA_AccCommandEntry_T     *cmd_entry_p;

    AAA_OM_ENTER_CRITICAL_SECTION();

    cmd_entry_p = AAA_OM_GetAccCommandEntry(priv_lvl, exec_type);
    if (NULL == cmd_entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    cmd_entry_p->list_name[0] = '\0';
    cmd_entry_p->configure_mode = AAA_AUTO_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p)
{
    BOOL_T  ret;
    UI32_T  i;

    if(index == NULL || entry_p == NULL)
        return FALSE;

    i = *index;
    entry_p->priv_lvl  = i / AAA_EXEC_TYPE_SUPPORT_NUMBER;
    entry_p->exec_type =(i % AAA_EXEC_TYPE_SUPPORT_NUMBER) + 1; /* because AAA_ExecType_T begin with 1 */
    ret = AAA_OM_GetAccCommandEntryInterface(entry_p);
    if(ret == TRUE)
    {
        *index = ++i;
    }

    return ret;
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccListEntryInterface(AAA_AccListEntryInterface_T *entry)
{
    while (AAA_OM_GetNextAccListEntryInterface(entry) == TRUE)
    {
        if (entry->client_type == AAA_CLIENT_TYPE_DOT1X)
        {
            if ((strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) &&       /* default method-list */
                (strcmp((char *)entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_DOT1X_LIST) == 0))    /* with default group-name */
                continue;
        }
        else if (entry->client_type == AAA_CLIENT_TYPE_EXEC)
        {
            if ((strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) &&       /* default method-list */
                (strcmp((char *)entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_EXEC_LIST) == 0))    /* with default group-name */
                continue;
        }
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        else if (entry->client_type == AAA_CLIENT_TYPE_COMMANDS)
        {
            if ((strcmp(entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) &&         /* default method-list */
                (strcmp((char *)entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_COMMAND_LIST) == 0))    /* with default group-name */
                continue;
        }
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryBy2rdIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting list by
 *              client_type +
 *              list_name   +
 *              priv_lvl (if necessary).
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccListEntry_T* AAA_OM_GetAccListEntryBy2rdIndex(const AAA_AccListEntry2rdIndex_T *key_p)
{
    UI32_T               index;

    if (NULL == key_p)
    {
        AAA_OM_TRACE("%s", "null pointer");
        return NULL;
    }

    for (index = 0; AAA_OM_MAX_NBR_OF_ACC_LIST > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED == method_list[index].entry_status)
            continue;
        if (key_p->client_type != method_list[index].client_type)
            continue;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        if (AAA_CLIENT_TYPE_COMMANDS == key_p->client_type)
        {
            if (key_p->priv_lvl != method_list[index].priv_lvl)
                continue;
        }
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

        if (strcmp(method_list[index].list_name, key_p->list_name) == 0)
            return &method_list[index];
    }

    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntry_ByInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding method list from interface
 * INPUT    : acc_interface
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static BOOL_T AAA_OM_GetAccListEntryByInterface(const AAA_AccInterface_T *acc_interface, AAA_AccListEntryInterface_T *list_entry)
{
    AAA_AccDot1xEntry_T         *dot1x_entry;
    AAA_AccExecEntry_T          *exec_entry;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    AAA_AccCommandEntry_T       *command_entry;
#endif

    AAA_AccListEntry_T          *return_list_entry_p;
    AAA_AccListEntry2rdIndex_T  list_entry_search_key;
    AAA_ExecType_T              exec_type;

    if (NULL == acc_interface || NULL ==list_entry)
    {
        AAA_OM_TRACE("%s", "null pointer");
        return FALSE;
    }

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));

    switch (acc_interface->client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            dot1x_entry = AAA_OM_GetAccDot1xEntry(acc_interface->ifindex);
            if (NULL == dot1x_entry)
            {
                return FALSE;
            }

            strcpy(list_entry_search_key.list_name, dot1x_entry->list_name);
            list_entry_search_key.client_type = AAA_CLIENT_TYPE_DOT1X;

            break;

        case AAA_CLIENT_TYPE_EXEC:
            if (FALSE == AAA_OM_ConvertToExecType(acc_interface->ifindex, &exec_type))
            {
                return FALSE;
            }
            exec_entry = AAA_OM_GetAccExecEntry(exec_type);
            if (NULL == exec_entry)
            {
                return FALSE;
            }

            strcpy(list_entry_search_key.list_name, exec_entry->list_name);
            list_entry_search_key.client_type = AAA_CLIENT_TYPE_EXEC;

            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            if (FALSE == AAA_OM_ConvertToExecType(acc_interface->ifindex, &exec_type))
            {
                return FALSE;
            }
            command_entry = AAA_OM_GetAccCommandEntry(acc_interface->priv_lvl, exec_type);
            if (NULL == command_entry)
            {
                return FALSE;
            }

            strcpy(list_entry_search_key.list_name, command_entry->list_name);
            list_entry_search_key.client_type = AAA_CLIENT_TYPE_COMMANDS;
            list_entry_search_key.priv_lvl    = command_entry->priv_lvl;

            break;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

        default:
            AAA_OM_TRACE("user_entry with bad client_type(%lu)",
                (UI32_T)acc_interface->client_type);
            return FALSE;
    }

    return_list_entry_p = AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key);

    if (NULL == return_list_entry_p)
    {
        return FALSE;
    }

    AAA_OM_CopyListEntryInterface(list_entry, return_list_entry_p);
    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting list by name.
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterface(AAA_AccListEntryInterface_T *entry)
{
    AAA_AccListEntry2rdIndex_T   list_entry_search_key;
    AAA_AccListEntry_T           *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));

    strcpy(list_entry_search_key.list_name, entry->list_name);
    list_entry_search_key.client_type = entry->client_type;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    list_entry_search_key.priv_lvl    = entry->priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    list_entry = AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyListEntryInterface(entry, list_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterfaceByIndex(AAA_AccListEntryInterface_T *entry)
{
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntryByIndex(entry->list_index);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyListEntryInterface(entry, list_entry);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryInterfaceByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by ifindex.
 * INPUT    : ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccListEntryInterfaceByPort(UI32_T ifindex, AAA_AccListEntryInterface_T *entry)
{
    AAA_AccListEntry_T      *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntryByPort(ifindex);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyListEntryInterface(entry, list_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntryInterface(AAA_AccListEntryInterface_T *entry)
{
    UI16_T  list_index;

    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (list_index = entry->list_index + 1; AAA_OM_MAX_NBR_OF_ACC_LIST >= list_index; ++list_index)
    {
        list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
        if (NULL == list_entry)
            continue;

        AAA_OM_CopyListEntryInterface(entry, list_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntryFilterByClientType(AAA_AccListEntryInterface_T *entry)
{
    UI16_T  list_index;

    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (list_index = entry->list_index + 1; AAA_OM_MAX_NBR_OF_ACC_LIST >= list_index; ++list_index)
    {
        list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
        if (NULL == list_entry)
            continue;

        if (entry->client_type != list_entry->client_type)
            continue;

        AAA_OM_CopyListEntryInterface(entry, list_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

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
BOOL_T AAA_OM_SetAccListEntry(const AAA_AccListEntryInterface_T *entry_p)
{
    BOOL_T      ret;

    AAA_AccListEntry2rdIndex_T  list_entry_search_key;
    AAA_AccListEntry_T          *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry_p)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == entry_p->client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(entry_p->group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)

    /* do not allow a command method-list associate with radius group */
    if ((AAA_CLIENT_TYPE_COMMANDS == entry_p->client_type) &&
        (NULL != AAA_OM_GetRadiusGroupEntry(entry_p->group_name)))
    {
        AAA_OM_TRACE("Do not allow cmd-list(%s) associate with radius group(%s)",
            entry_p->list_name, entry_p->group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));
    strcpy(list_entry_search_key.list_name, entry_p->list_name);
    list_entry_search_key.client_type = entry_p->client_type;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    list_entry_search_key.priv_lvl    = entry_p->priv_lvl;
#endif

    list_entry = AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key);
    if (NULL == list_entry)
    {
        /* create a new entry */
        list_entry = AAA_OM_AllocAccListEntry(entry_p->client_type);
        if (NULL == list_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        AAA_OM_TRACE("Create a new entry (%s) client_type(%d)", entry_p->list_name, entry_p->client_type);
    }

    ret = (AAA_API_UPDATE_FAILED != AAA_OM_CopyListEntry(list_entry, entry_p));

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryName
 *------------------------------------------------------------------------
 * PURPOSE  : set the list_name by list_index.
 * INPUT    : list_index, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntryName(UI16_T list_index, char* list_name)
{
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == list_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Can't modify default method-list name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->list_name, (char *)list_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (AAA_OM_IsValidMethodListName(list_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if ((AAA_OM_GetAccListEntry(list_name, AAA_CLIENT_TYPE_DOT1X) != NULL) ||
        (AAA_OM_GetAccListEntry(list_name, AAA_CLIENT_TYPE_EXEC) != NULL))
    {
        /* method-list name can't duplicate */
        AAA_OM_TRACE("There is a method-list with the same name(%s)", list_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    strncpy((char *)list_entry->list_name, (char *)list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : set the group_name by list_index
 * INPUT    : list_index, group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccListEntryGroupName(UI16_T list_index, char* group_name)
{
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == group_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->group_name, (char *)group_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == list_entry->client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

    strncpy((char *)list_entry->group_name, (char *)group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccListEntryClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the working mode by list_index.
 * INPUT    : list_index, client_type
 * OUTPUT   : None
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : None
 *------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAccListEntryClientType(UI16_T list_index, AAA_ClientType_T client_type)
{
    AAA_AccListEntry_T           *entry;
    AAA_AccListEntry2rdIndex_T   list_entry_search_key;

    AAA_OM_ENTER_CRITICAL_SECTION();

    entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (client_type == entry->client_type)
    {
        /* the same mode, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));
    strncpy(list_entry_search_key.list_name, entry->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    list_entry_search_key.list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = 0;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    list_entry_search_key.client_type = client_type;
    list_entry_search_key.priv_lvl    = entry->priv_lvl;
#endif/*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    if (NULL != AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key))
    {
        /* method-list name can't duplicate */
        AAA_OM_TRACE("There is a method-list with the same name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(entry->group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_COMMANDS == client_type) &&
        (NULL != AAA_OM_GetRadiusGroupEntry(entry->group_name)))
    {
        AAA_OM_TRACE("Don't allow a command method-list associate with radius group(%s)",
            entry->group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

    entry->client_type = client_type;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

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
AAA_ApiUpdateResult_T AAA_OM_SetAccListEntryPrivilege(UI16_T list_index, UI32_T priv_lvl)
{
    AAA_AccListEntry_T          *entry;
    AAA_AccListEntry2rdIndex_T   list_entry_search_key;

    AAA_OM_ENTER_CRITICAL_SECTION();

    entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (priv_lvl == entry->priv_lvl)
    {
        /* the same mode, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));
    strncpy(list_entry_search_key.list_name, entry->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    list_entry_search_key.list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = 0;
    list_entry_search_key.client_type = entry->client_type;
    list_entry_search_key.priv_lvl    = priv_lvl;

    /*if (AAA_OM_GetAccListEntry(entry->list_name, client_type) != NULL)*/
    if (FALSE != AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key))
    {
        /* method-list name can't duplicate */
        AAA_OM_TRACE("%s", "There is a method-list with the same name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    entry->priv_lvl = priv_lvl;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}
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
BOOL_T AAA_OM_SetAccListEntryWokringMode(UI16_T list_index, AAA_AccWorkingMode_T working_mode)
{
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (working_mode == list_entry->working_mode)
    {
        /* the same mode, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    list_entry->working_mode = working_mode;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetSetAccListEntryStatus
 *------------------------------------------------------------------------
 * PURPOSE  : set the status by list_index.
 * INPUT    : list_index, entry_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetSetAccListEntryStatus(UI16_T list_index, AAA_EntryStatus_T entry_status)
{
    BOOL_T      ret;

    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch (entry_status)
    {
        case AAA_ENTRY_READY:

            list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
            if (NULL != list_entry) /* already existed */
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
            }

            list_entry = AAA_OM_GetAccListEntry(AAA_SNMP_CREATE_DFLT_METHOD_LIST_NAME, AAA_CLIENT_TYPE_DOT1X);
            if (NULL != list_entry) /* already existed */
            {
                AAA_OM_TRACE("Can't create two dot1x list with default method-list name");
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_ACC_LIST < list_index))
            {
                AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_ACC_LIST);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = &method_list[list_index - 1]; /* to zero-based */

            strncpy((char *)list_entry->list_name, AAA_SNMP_CREATE_DFLT_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
            list_entry->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

            strncpy((char *)list_entry->group_name, SYS_DFLT_AAA_RADIUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            list_entry->client_type = AAA_CLIENT_TYPE_DOT1X;
            list_entry->working_mode = ACCOUNTING_START_STOP;
            list_entry->entry_status = AAA_ENTRY_READY;
            break;

        case AAA_ENTRY_DESTROYED:
            ret = AAA_OM_FreeAccListEntry(list_index);

            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);

        default:
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_DestroyAccListEntryInterface(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning)
{
    if (AAA_OM_GetAccListEntryInterface(entry) == FALSE)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        return TRUE;
    }

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (FALSE == AAA_OM_FreeAccListEntry(entry->list_index)) /* should not use list_entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
        switch (entry->client_type)
        {
            case AAA_CLIENT_TYPE_DOT1X:
                warning->warning_type = AAA_OM_IsAnyPortReferThisList(entry->list_name) ? AAA_ACC_DOT1X_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

            case AAA_CLIENT_TYPE_EXEC:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(entry->list_name) ? AAA_ACC_EXEC_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
            case AAA_CLIENT_TYPE_COMMANDS:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(entry->list_name) ? AAA_ACC_COMMAND_REF2_BAD_LIST : AAA_NO_WARNING;
                break;
#endif

            default:
                warning->warning_type = AAA_NO_WARNING;
                break;
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);

}

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
BOOL_T AAA_OM_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning)
{
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAccListEntry(name, client_type);
    if (NULL == list_entry)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (FALSE == AAA_OM_FreeAccListEntry(list_entry->list_index)) /* should not use list_entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
        switch (client_type)
        {
            case AAA_CLIENT_TYPE_DOT1X:
                warning->warning_type = AAA_OM_IsAnyPortReferThisList(name) ? AAA_ACC_DOT1X_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

            case AAA_CLIENT_TYPE_EXEC:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(name) ? AAA_ACC_EXEC_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
            case AAA_CLIENT_TYPE_COMMANDS:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(name) ? AAA_ACC_COMMAND_REF2_BAD_LIST : AAA_NO_WARNING;
                break;
#endif

            default:
                warning->warning_type = AAA_NO_WARNING;
                break;
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_DestroyAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by list_index
 * INPUT    : list_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DestroyAccListEntryByIndex(UI16_T list_index, AAA_WarningInfo_T *warning)
{
    char    name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];

    AAA_AccListEntry_T      *list_entry;
    AAA_ClientType_T     client_type;

    AAA_OM_ENTER_CRITICAL_SECTION();

    list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

	client_type = list_entry->client_type;
    strcpy((char *)name, (char *)list_entry->list_name);

    if (FALSE == AAA_OM_FreeAccListEntry(list_entry->list_index)) /* should not use list_entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
        switch (client_type)
        {
            case AAA_CLIENT_TYPE_DOT1X:
                warning->warning_type = AAA_OM_IsAnyPortReferThisList(name) ? AAA_ACC_DOT1X_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

            case AAA_CLIENT_TYPE_EXEC:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(name) ? AAA_ACC_EXEC_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
            case AAA_CLIENT_TYPE_COMMANDS:
                warning->warning_type = AAA_OM_IsAnyExecReferThisList(name) ? AAA_ACC_COMMAND_REF2_BAD_LIST : AAA_NO_WARNING;
                break;
#endif

            default:
                warning->warning_type = AAA_NO_WARNING;
                break;
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle accounting method list
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeAccListEntry(UI16_T list_index)
{
    AAA_AccListEntry_T  *entry;

    if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_ACC_LIST < list_index))
    {
        AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_ACC_LIST);
        return FALSE;
    }

    entry = &method_list[list_index - 1]; /* to zero-based */

    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("list_index(%d) no need to free", list_index);
        return FALSE;
    }

    if (strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Can't free ""default"" list");
        return FALSE;
    }

    AAA_OM_TRACE("Free list_index(%d)", list_index);

    entry->entry_status = AAA_ENTRY_DESTROYED;
    entry->list_name[0] = '\0';
    entry->group_name[0] = '\0';

    switch (entry->client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            --acc_dot1x_list_qty;
            break;

        case AAA_CLIENT_TYPE_EXEC:
            --acc_exec_list_qty;
            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            --acc_command_list_qty;
            break;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

        default:
            AAA_OM_TRACE("Bad client_type(%d)", entry->client_type);
            break;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : none
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p)
{
    I32_T  str_len;

    AAA_ApiUpdateResult_T       result;
    AAA_AccListEntry2rdIndex_T  list_entry_search_key;
    AAA_AccListEntry_T          *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();
    if (NULL == entry_p->group_name)
    {
        AAA_OM_TRACE("Null pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    str_len = strlen((char *)entry_p->group_name);
    if (0 >= str_len)
    {
        AAA_OM_TRACE("group_name has a bad length(%ld)", str_len);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    memset(&list_entry_search_key, 0, sizeof(list_entry_search_key));
    strcpy(list_entry_search_key.list_name, SYS_DFLT_AAA_METHOD_LIST_NAME);
    list_entry_search_key.client_type = entry_p->client_type;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    list_entry_search_key.priv_lvl    = entry_p->priv_lvl;
#endif

    list_entry = AAA_OM_GetAccListEntryBy2rdIndex(&list_entry_search_key);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Should not go here, AAA_OM_Initialize() forgot to insert or AAA_OM_FreeAccListEntry() free the default entry");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == list_entry->client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(entry_p->group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)

    /* do not allow a command method-list associate with radius group */
    if ((AAA_CLIENT_TYPE_COMMANDS == entry_p->client_type) &&
        (NULL != AAA_OM_GetRadiusGroupEntry(entry_p->group_name)))
    {
        AAA_OM_TRACE("Do not allow cmd-list(%s) associate with radius group(%s)",
            entry_p->list_name, entry_p->group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    result = AAA_API_UPDATE_NO_CHANGE;

    if (entry_p->working_mode != list_entry->working_mode)
    {
        list_entry->working_mode = entry_p->working_mode;
        result = AAA_API_UPDATE_CHANGE;
    }

    if (strcmp((char *)entry_p->group_name, (char *)list_entry->group_name) != 0)
    {
        strncpy((char *)list_entry->group_name, (char *)entry_p->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

        result = AAA_API_UPDATE_CHANGE;
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(result);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccountingGroupIndex_ByInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : acc_interface
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccountingGroupIndexByInterface(const AAA_AccInterface_T *acc_interface, AAA_QueryGroupIndexResult_T *query_result)
{
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;
    AAA_AccListEntryInterface_T list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == acc_interface || NULL == query_result)
    {
        AAA_OM_TRACE("%s", "null pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (FALSE == AAA_OM_GetAccListEntryByInterface(acc_interface, &list_entry))
    {
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        AAA_OM_TRACE("Can't found method, client_type(%lu) ifindex(%lu) priv-lvl(%lu)",
            (UI32_T)acc_interface->client_type, acc_interface->ifindex, acc_interface->priv_lvl);
#else
        AAA_OM_TRACE("Can't found method, client_type(%lu) ifindex(%lu)",
            (UI32_T)acc_interface->client_type, acc_interface->ifindex);
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    radius_group = AAA_OM_GetRadiusGroupEntry(list_entry.group_name);
    if (NULL != radius_group)
    {
        query_result->group_index = radius_group->group_index;
        query_result->group_type = GROUP_RADIUS;
    }
    else
    {
        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(list_entry.group_name);
        if (NULL != tacacs_group)
        {
            query_result->group_index = tacacs_group->group_index;
            query_result->group_type = GROUP_TACACS_PLUS;
        }
        else
        {
            AAA_OM_TRACE("Can't found group(%s)", list_entry.group_name);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next accounting user by user_index
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryInterface(AAA_AccUserEntryInterface_T *entry)
{
    AAA_AccUserEntry_T  *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (0 == entry->user_index)
    {
        user_entry = AAA_OM_GetAccFirstUser();
    }
    else
    {
        user_entry = AAA_OM_GetAccUser(entry->user_index);
        if (NULL == user_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        user_entry = user_entry->next_user_entry;
    }

    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyUserEntryInterface(entry, user_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_GetNextAccUserEntryInterfaceFilterByParams(AAA_AccUserEntryInterface_T *entry)
{
    AAA_AccUserEntry_T  *user_entry;
    AAA_ExecType_T   exec_type;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (0 == entry->user_index)
    {
        user_entry = AAA_OM_GetAccFirstUser();
    }
    else
    {
        user_entry = AAA_OM_GetAccUser(entry->user_index);
        if (NULL == user_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        user_entry = user_entry->next_user_entry;
    }

    switch (entry->client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            for ( ; NULL != user_entry; user_entry = user_entry->next_user_entry)
            {
                if ((user_entry->client_type == entry->client_type) &&
                    (user_entry->ifindex == entry->ifindex))
                    break;
            }
            break;

        case AAA_CLIENT_TYPE_EXEC:
            for ( ; NULL != user_entry; user_entry = user_entry->next_user_entry)
            {
                if (user_entry->client_type != entry->client_type)
                    continue;

                if (FALSE == AAA_OM_ConvertToExecType(user_entry->ifindex, &exec_type))
                    continue;

                if (exec_type == entry->ifindex)
                    break;
            }
            break;

        default:
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyUserEntryInterface(entry, user_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_CreateAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : create an accounting user according to request
 * INPUT    : request, sys_time
 * OUTPUT   : none.
 * RETURN   : user_index, 0 - can't create this user
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
UI16_T AAA_OM_CreateAccUser(const AAA_AccRequest_T *request, UI32_T sys_time)
{
    AAA_AccUserEntry_T   *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == request)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(0);
    }


    /* check whether the user exist or not */
    user_entry = AAA_OM_QueryAccUser(request->ifindex, request->user_name, request->client_type);
    if (NULL != user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(0);
    }

    /* prepare user info */
    user_entry = AAA_OM_AllocAccUser();
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(0);
    }

    user_entry->ifindex = request->ifindex;

    strncpy((char *)user_entry->user_name, (char *)request->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    user_entry->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    user_entry->accounting_start_time = sys_time;

    user_entry->identifier = request->identifier;
    user_entry->call_back_func = request->call_back_func;

    user_entry->client_type = request->client_type;
    memset(&user_entry->notify_bitmap, 0, sizeof(AAA_AccNotifyMap_T));

    user_entry->auth_privilege = request->auth_privilege;
    user_entry->auth_by_whom = request->auth_by_whom;
    user_entry->terminate_cause = request->terminate_cause; /* it should be meaningless */

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(user_entry->user_index);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAccUser
 *-------------------------------------------------------------------------
 * PURPOSE: recycle specific user entry from user list
 * INPUT:   user_index (1-based)
 * OUTPUT:  none.
 * RETURN:  none
 * NOTES:   none.
 *-------------------------------------------------------------------------*/
void AAA_OM_FreeAccUser(UI16_T user_index)
{
    AAA_AccUserEntry_T   *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if ((0 >= user_index) || (AAA_MAX_NBR_OF_ACC_USERS < user_index))
    {
        AAA_OM_TRACE("Bad user_index(%d)", user_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
    }

    entry = &acc_user_info[user_index - 1];
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("Should not free a destroyed entry (%d)", user_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
    }

    if (entry == head_of_acc_user)
    {
        AAA_OM_TRACE("Entry(%d) is the head entry", user_index);
        head_of_acc_user = entry->next_user_entry;
    }

    if (entry == tail_of_acc_user)
    {
        AAA_OM_TRACE("Entry(%d) is the tail entry", user_index);
        tail_of_acc_user = entry->prev_user_entry;
    }

    if (NULL != entry->prev_user_entry)
        entry->prev_user_entry->next_user_entry = entry->next_user_entry;

    if (NULL != entry->next_user_entry)
        entry->next_user_entry->prev_user_entry = entry->prev_user_entry;

    entry->prev_user_entry = NULL;
    entry->next_user_entry = NULL;
    entry->entry_status = AAA_ENTRY_DESTROYED;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION_WITHOUT_RETUEN_VALUE();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUserRadiusStartFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : setup user radius start flag
 * INPUT    : user_index (1-based), start_flag
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUserRadiusStartFlag(UI16_T user_index, BOOL_T start_flag)
{
    AAA_AccUserEntry_T          *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    user_entry = AAA_OM_GetAccUser(user_index);
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry->notify_bitmap.radius_start = (TRUE == start_flag) ? 1 : 0;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAccUserTacacsStartFlag
 *-------------------------------------------------------------------------
 * PURPOSE  : setup user tacacs start flag
 * INPUT    : user_index (1-based), start_flag
 * OUTPUT   : none.
 * RETURN   : TRUE - success, FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAccUserTacacsStartFlag(UI16_T user_index, BOOL_T start_flag)
{
    AAA_AccUserEntry_T          *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    user_entry = AAA_OM_GetAccUser(user_index);
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry->notify_bitmap.tacacs_start = (TRUE == start_flag) ? 1 : 0;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_QueryAccUserIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : check whether the specified user exist or not
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : none
 * RETURN   : user_index, 0 - not exist
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
UI16_T AAA_OM_QueryAccUserIndex(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type)
{
    AAA_AccUserEntry_T          *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    user_entry = AAA_OM_QueryAccUser(ifindex, user_name, client_type);
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(0);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(user_entry->user_index);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQty
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQty(UI32_T *qty)
{
    AAA_AccUserEntry_T       *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == qty)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (*qty = 0, entry = AAA_OM_GetAccFirstUser(); NULL != entry;
        entry = entry->next_user_entry)
    {
        ++(*qty);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    AAA_AccUserEntry_T       *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == qty)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *qty = 0;
    entry = AAA_OM_GetAccFirstUser();
    for ( ; NULL != entry; entry = entry->next_user_entry)
    {
        if ((client_type == entry->client_type) &&
            (strcmp((char *)name, (char *)entry->user_name) == 0))
            ++(*qty);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    AAA_AccUserEntry_T       *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == qty)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *qty = 0;
    entry = AAA_OM_GetAccFirstUser();
    for ( ; NULL != entry; entry = entry->next_user_entry)
    {
        if (client_type == entry->client_type)
            ++(*qty);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUserEntryQtyFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty)
{
    AAA_AccUserEntry_T       *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == qty)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    *qty = 0;
    entry = AAA_OM_GetAccFirstUser();
    for ( ; NULL != entry; entry = entry->next_user_entry)
    {
        if (AAA_CLIENT_TYPE_DOT1X != entry->client_type)
            continue;

        if (ifindex == entry->ifindex)
            ++(*qty);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryInfo
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user by index.
 * INPUT    : entry->user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryInfo(AAA_AccUserInfoInterface_T *entry)
{
    AAA_AccUserEntry_T       *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry = AAA_OM_GetNextAccUser(entry->user_index);
    if (NULL != user_entry)
    {
        AAA_OM_CopyUserInfoInterface(entry, user_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByNameAndType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified name and client_type by index.
 * INPUT    : entry->user_index, entry->user_name, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry)
{
    AAA_AccUserEntry_T       *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry = AAA_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_entry; user_entry = user_entry->next_user_entry)
    {
        if (entry->client_type != user_entry->client_type)
            continue;

        if (strcmp((char *)entry->user_name, (char *)user_entry->user_name) != 0)
            continue;

        AAA_OM_CopyUserInfoInterface(entry, user_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByType
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user with specified client_type by index.
 * INPUT    : entry->user_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry)
{
    AAA_AccUserEntry_T       *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry = AAA_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_entry; user_entry = user_entry->next_user_entry)
    {
        if (entry->client_type != user_entry->client_type)
            continue;

        AAA_OM_CopyUserInfoInterface(entry, user_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUserEntryFilterByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next user from specified port by index.
 * INPUT    : entry->user_index, entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry)
{
    AAA_AccUserEntry_T       *user_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry = AAA_OM_GetNextAccUser(entry->user_index);
    for ( ; NULL != user_entry; user_entry = user_entry->next_user_entry)
    {
        if (AAA_CLIENT_TYPE_DOT1X != user_entry->client_type)
            continue;

        if (entry->ifindex != user_entry->ifindex)
            continue;

        AAA_OM_CopyUserInfoInterface(entry, user_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CollectSettingInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : query setting by user index
 * INPUT    : user_index
 * OUTPUT   : setting_info
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_CollectSettingInfo(UI16_T user_index, AAA_AccSettingInfo_T *setting_info)
{
    AAA_AccUserEntry_T          *user_entry;
    AAA_AccDot1xEntry_T         *dot1x_entry;
    AAA_AccExecEntry_T          *exec_entry;
    AAA_AccListEntry_T          *list_entry;
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;
    AAA_ExecType_T              exec_type;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == setting_info)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
    user_entry = AAA_OM_GetAccUser(user_index);
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    switch (user_entry->client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            dot1x_entry = AAA_OM_GetAccDot1xEntry(user_entry->ifindex);
            if (NULL == dot1x_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(dot1x_entry->list_name, AAA_CLIENT_TYPE_DOT1X);
            break;

        case AAA_CLIENT_TYPE_EXEC:
            if (FALSE == AAA_OM_ConvertToExecType(user_entry->ifindex, &exec_type))
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            exec_entry = AAA_OM_GetAccExecEntry(exec_type);
            if (NULL == exec_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAccListEntry(exec_entry->list_name, AAA_CLIENT_TYPE_EXEC);
            break;

        default:
            AAA_OM_TRACE("user_entry with bad client_type(%d)", user_entry->client_type);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Cannot get method list for %s",
            (user_entry->client_type==AAA_CLIENT_TYPE_DOT1X)?"dot1x":"exec");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    setting_info->working_mode = list_entry->working_mode;

    radius_group = AAA_OM_GetRadiusGroupEntry(list_entry->group_name);
    if (NULL == radius_group)
    {
        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(list_entry->group_name);
        if (NULL == tacacs_group)
        {
            AAA_OM_TRACE("Group (%s) not found", list_entry->group_name);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        switch (user_entry->client_type)
        {
            case AAA_CLIENT_TYPE_DOT1X:

            #if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == TRUE)

                setting_info->group_type = GROUP_TACACS_PLUS;
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);

            #else

                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE); /* tacacs+ didn't support dot1x accounting */

            #endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == TRUE */

            case AAA_CLIENT_TYPE_EXEC:
                setting_info->group_type = GROUP_TACACS_PLUS;
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);

            default:
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }
    }
    setting_info->group_type = GROUP_RADIUS;
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CollectSettingInfo_ByAccRequest
 * ---------------------------------------------------------------------
 * PURPOSE  : query setting by AAA_AccRequest_T
 * INPUT    : acc_request
 * OUTPUT   : setting_info
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_CollectSettingInfo_ByAccRequest(const AAA_AccRequest_T *acc_request, AAA_AccSettingInfo_T *setting_info)
{
    AAA_AccInterface_T           acc_interface;
    AAA_AccListEntryInterface_T  list_entry;
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == acc_request || NULL == setting_info)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memset(&acc_interface, 0, sizeof(acc_interface));
    acc_interface.client_type = acc_request->client_type;
    acc_interface.ifindex     = acc_request->ifindex;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    acc_interface.priv_lvl    = acc_request->command_privilege;
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

    if(FALSE == AAA_OM_GetAccListEntryByInterface(&acc_interface, &list_entry))
    {
#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        AAA_OM_TRACE("Can't found the method client_type(%lu) ifindex(%lu) priv-lvl(%lu)",
            (UI32_T)acc_interface.client_type, acc_interface.ifindex, acc_interface.priv_lvl);
#else
        AAA_OM_TRACE("Can't found the method, client_type(%u) ifindex(%lu)",
            acc_interface.client_type, acc_interface.ifindex);
#endif /* SYS_CPNT_ACCOUNTING_COMMAND == TRUE */

        AAA_OM_TRACE("%s", "Cannot find the method for the request");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    setting_info->working_mode = list_entry.working_mode;

    radius_group = AAA_OM_GetRadiusGroupEntry(list_entry.group_name);
    if (NULL != radius_group)
    {
        setting_info->group_type = GROUP_RADIUS;

        AAA_OM_TRACE("Found method=%s, group=%s, type=radius",
            list_entry.list_name, list_entry.group_name);
    }
    else
    {
        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(list_entry.group_name);
        if (NULL != tacacs_group)
        {
            setting_info->group_type = GROUP_TACACS_PLUS;

            AAA_OM_TRACE("Found method=%s, group=%s, type=tacacs+",
                list_entry.list_name, list_entry.group_name);
        }
        else
        {
            AAA_OM_TRACE("%s", "Cannot find the group for the request");
            AAA_OM_TRACE("Can't found the group=%s", list_entry.group_name);
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GenerateRequestFromUserInfo
 * ---------------------------------------------------------------------
 * PURPOSE  : generate necessary information for request from user_index
 * INPUT    : user_index, request_type
 * OUTPUT   : request_info
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_GenerateRequestFromUserInfo(UI16_T user_index, AAA_AccRequestType_T request_type, AAA_AccRequest_T *request_entry)
{
    AAA_AccUserEntry_T  *user_entry;
    AAA_AccListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == request_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    user_entry = AAA_OM_GetAccUser(user_index);
    if (NULL == user_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    request_entry->ifindex = user_entry->ifindex;

    strncpy((char *)request_entry->user_name, (char *)user_entry->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    request_entry->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    request_entry->client_type = user_entry->client_type;
    request_entry->request_type = request_type;

    list_entry = AAA_OM_GetAccListEntryByPort(user_entry->ifindex);
    if (NULL == list_entry)
        request_entry->current_working_mode = AAA_CREATE_DFLT_WORKING_MODE;
    else
        request_entry->current_working_mode = list_entry->working_mode;

    request_entry->identifier = user_entry->identifier;
    request_entry->call_back_func = user_entry->call_back_func;

    request_entry->auth_privilege = user_entry->auth_privilege;
    request_entry->auth_by_whom = user_entry->auth_by_whom;
    request_entry->terminate_cause = user_entry->terminate_cause;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GenerateSerialNumber
 * ---------------------------------------------------------------------
 * PURPOSE  : generate a new serial number
 * INPUT    : none
 * OUTPUT   : sn_p
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_GenerateSerialNumber(UI32_T *sn_p)
{
    enum {MAX_SERIAL_NUMBER = 0XFFFFFFFF};
    static UI32_T serial_number = 0;

    if (NULL == sn_p)
        return FALSE;

    if (serial_number == MAX_SERIAL_NUMBER)
        serial_number = 0;

    *sn_p = ++serial_number;
    return TRUE;
}
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
BOOL_T AAA_OM_ConvertToExecType(UI32_T exec_id, AAA_ExecType_T *exec_type)
{


    if (NULL == exec_type)
    {
        return FALSE;
    }

    if (0 == exec_id)
    {
        *exec_type = AAA_EXEC_TYPE_CONSOLE;
    }
    else if ((1 <= exec_id) && (SYS_ADPT_MAX_TELNET_NUM >= exec_id))
    {
        *exec_type = AAA_EXEC_TYPE_VTY;
    }
    else if (SYS_ADPT_MAX_TELNET_NUM + 1 == exec_id)
    {
        *exec_type = AAA_EXEC_TYPE_HTTP;
    }
    else
    {
        AAA_OM_TRACE("bad exec_id(%lu)", exec_id);
        return FALSE;
    }
    return TRUE;

}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_ConvertToIfindex
 * ---------------------------------------------------------------------
 * PURPOSE  : convert ifindex to exec_type
 * INPUT    : exec_id
 * OUTPUT   : exec_type
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 * ---------------------------------------------------------------------*/
BOOL_T AAA_OM_ConvertToIfindex(AAA_ExecType_T exec_type, UI32_T *ifindex)
{
    if (NULL == ifindex)
    {
        return FALSE;
    }

    if (AAA_EXEC_TYPE_CONSOLE == exec_type)
    {
        *ifindex = 0;
    }
    else if (AAA_EXEC_TYPE_VTY == exec_type)
    {
        *ifindex = 1;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius group entry by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_RadiusGroupEntry_T *AAA_OM_GetRadiusGroupEntry(const char *group_name)
{
    UI16_T  index;
    AAA_RadiusGroupEntry_T          *entry;

    if (NULL == group_name)
    {
        AAA_OM_TRACE("Null pointer");
        return NULL;
    }

    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP > index; ++index)
    {
        entry = &aaa_om_radius_group[index];
        if (AAA_ENTRY_DESTROYED == entry->entry_status)
            continue;

        if (strcmp((char *)entry->group_name, (char *)group_name) == 0)
            return entry;
    }

    AAA_OM_TRACE("Group (%s) not found", group_name);
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius group entry by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_RadiusGroupEntry_T *AAA_OM_GetRadiusGroupEntryByIndex(UI16_T group_index)
{
    AAA_RadiusGroupEntry_T          *entry;

    if ((0 >= group_index) || (SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP < group_index))
    {
        AAA_OM_TRACE("Bad group_index(%d)", group_index);
        return NULL;
    }

    entry = &aaa_om_radius_group[group_index - 1]; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("group_index(%d) is destroyed", group_index);
        return NULL;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty radius group
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_RadiusGroupEntry_T *AAA_OM_AllocRadiusGroupEntry()
{
    UI16_T      index;

    AAA_RadiusGroupEntry_T  *entry;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_RADIUS_GROUP > index; ++index)
    {
        entry = &aaa_om_radius_group[index];

        if (AAA_ENTRY_DESTROYED != entry->entry_status)
            continue;

        entry->group_name[0] = '\0';

        entry->head_of_radius_entry = NULL;
        entry->tail_of_radius_entry = NULL;

        entry->entry_status = AAA_ENTRY_READY;

        AAA_OM_TRACE("Alloc group_index(%d)", entry->group_index);

        return entry;
    }

    AAA_OM_TRACE("No more available entry");
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs group entry by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_TacacsPlusGroupEntry_T *AAA_OM_GetTacacsPlusGroupEntry(const char *group_name)
{
    UI16_T  index;

    AAA_TacacsPlusGroupEntry_T  *entry;

    if (NULL == group_name)
    {
        AAA_OM_TRACE("Null pointer");
        return NULL;
    }

    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP > index; ++index)
    {
        entry = &aaa_om_tacacs_group[index];

        if (AAA_ENTRY_DESTROYED == entry->entry_status)
            continue;

        if (strcmp((char *)entry->group_name, (char *)group_name) == 0)
            return entry;
    }

    AAA_OM_TRACE("Group(%s) not found", group_name);
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs group entry by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_TacacsPlusGroupEntry_T *AAA_OM_GetTacacsPlusGroupEntryByIndex(UI16_T group_index)
{
    AAA_TacacsPlusGroupEntry_T  *entry;

    if ((0 >= group_index) || (SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP < group_index))
    {
        AAA_OM_TRACE("Bad group_index(%d)", group_index);
        return NULL;
    }

    entry = &aaa_om_tacacs_group[group_index - 1]; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("group_index(%d) is destroyed", group_index);
        return NULL;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty tacacs group
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_TacacsPlusGroupEntry_T *AAA_OM_AllocTacacsPlusGroupEntry()
{
    UI16_T      index;

    AAA_TacacsPlusGroupEntry_T  *entry;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_AAA_TACACS_PLUS_GROUP > index; ++index)
    {
        entry = &aaa_om_tacacs_group[index];

        if (AAA_ENTRY_DESTROYED != entry->entry_status)
            continue;

        entry->group_name[0] = '\0';

        entry->head_of_tacacs_entry = NULL;
        entry->tail_of_tacacs_entry = NULL;

        entry->entry_status = AAA_ENTRY_READY;

        AAA_OM_TRACE("Alloc group_index(%d)", entry->group_index);

        return entry;
    }

    AAA_OM_TRACE("No more available entry");

    return NULL;
}

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by ifindex.
 * INPUT    : ifindex (1-based)
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccDot1xEntry_T *AAA_OM_GetAccDot1xEntry(UI32_T ifindex)
{
    if ((0 >= ifindex) || (SYS_ADPT_TOTAL_NBR_OF_LPORT < ifindex))
    {
        AAA_OM_TRACE("Bad ifindex(%lu) {1 - %d}", ifindex, SYS_ADPT_TOTAL_NBR_OF_LPORT);
        return NULL;
    }

    return &acc_dot1x_entry[ifindex - 1]; /* to zero-based */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by exec_type.
 * INPUT    : exec_type (1-based)
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccExecEntry_T *AAA_OM_GetAccExecEntry(AAA_ExecType_T exec_type)
{
    if ((AAA_EXEC_TYPE_NONE >= exec_type) || (AAA_EXEC_TYPE_SUPPORT_NUMBER < exec_type))
    {
        AAA_OM_TRACE("Bad exec_type(%d)", exec_type);
        return NULL;
    }

    return &acc_exec_entry[exec_type - 1]; /* to zero-based */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the accounting list by name.
 * INPUT    : list_name, client_type
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccListEntry_T *AAA_OM_GetAccListEntry(const char *list_name, AAA_ClientType_T client_type)
{
    UI16_T  index;

    if (NULL == list_name)
    {
        AAA_OM_TRACE("Null pointer");
        return NULL;
    }
    for (index = 0; AAA_OM_MAX_NBR_OF_ACC_LIST > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED == method_list[index].entry_status)
            continue;
        if (client_type != method_list[index].client_type)
            continue;
        if (strcmp((char *)method_list[index].list_name, (char *)list_name) == 0)
            return &method_list[index];
    }

    AAA_OM_TRACE("List (%s) client_type(%d) not found", list_name, client_type);
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by index.
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccListEntry_T *AAA_OM_GetAccListEntryByIndex(UI16_T list_index)
{
    AAA_AccListEntry_T  *entry;

    if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_ACC_LIST < list_index))
    {
        AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_ACC_LIST);
        return NULL;
    }

    entry = &method_list[list_index - 1]; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("list_index(%d) is destroyed", list_index);
        return NULL;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccListEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by ifindex.
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccListEntry_T *AAA_OM_GetAccListEntryByPort(UI32_T ifindex)
{
    AAA_AccDot1xEntry_T     *dot1x_entry;

    dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
    if (NULL == dot1x_entry)
        return NULL;

    if (strlen((char *)dot1x_entry->list_name) == 0)
    {
        AAA_OM_TRACE("Ifindex(%lu) disable accounting dot1x ", ifindex);
        return FALSE;
    }

    return AAA_OM_GetAccListEntry(dot1x_entry->list_name, AAA_CLIENT_TYPE_DOT1X);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty accounting method list
 * INPUT    : client_type
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_AccListEntry_T *AAA_OM_AllocAccListEntry(AAA_ClientType_T client_type)
{
    UI16_T      index;
    UI16_T      *current_qty;

    AAA_AccListEntry_T  *entry;

    switch (client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            current_qty = &acc_dot1x_list_qty;
            break;

        case AAA_CLIENT_TYPE_EXEC:
            current_qty = &acc_exec_list_qty;
            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            current_qty = &acc_command_list_qty;
            break;
#endif

        default:
            AAA_OM_TRACE("Bad client type %d", client_type);
            return NULL;
    }

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    if (client_type == AAA_CLIENT_TYPE_COMMANDS)
    {
        if (SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE+SYS_ADPT_MAX_NBR_OF_ACCOUNTING_LIST <= *current_qty)
        {
            AAA_OM_TRACE("No more available entry for client_type(%d)", client_type);
            return NULL;
        }
    }
    else
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
    {
        if (SYS_ADPT_MAX_NBR_OF_ACCOUNTING_LIST <= *current_qty)
        {
            AAA_OM_TRACE("No more available entry for client_type(%d)", client_type);
            return NULL;
        }
    }

    for (index = 0; AAA_OM_MAX_NBR_OF_ACC_LIST > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED != method_list[index].entry_status)
            continue;

        entry = &method_list[index];

        entry->list_name[0] = '\0';
        entry->group_name[0] = '\0';

        entry->client_type = client_type;
        entry->entry_status = AAA_ENTRY_READY;

        ++(*current_qty); /* increase the qty */

        AAA_OM_TRACE("Alloc list_index(%d)", entry->list_index);

        return entry;
    }

    AAA_OM_TRACE("No more available entry");

    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetAccFirstUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get the first entry
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTE     : none.
 *-------------------------------------------------------------------------*/
static AAA_AccUserEntry_T *AAA_OM_GetAccFirstUser()
{
    return head_of_acc_user;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get specific user entry from user list
 * INPUT    : user_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccUserEntry_T *AAA_OM_GetAccUser(UI16_T user_index)
{
    AAA_AccUserEntry_T  *entry;

    if ((0 >= user_index) || (AAA_MAX_NBR_OF_ACC_USERS < user_index))
    {
        AAA_OM_TRACE("Bad user_index(%d) {1 - %d}", user_index, AAA_MAX_NBR_OF_ACC_USERS);
        return NULL;
    }

    entry = &acc_user_info[user_index - 1]; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("user_index(%d) is destroyed", user_index);
        return NULL;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : get next accounting user
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccUserEntry_T *AAA_OM_GetNextAccUser(UI16_T user_index)
{
    AAA_AccUserEntry_T   *entry;

    if (0 == user_index)
    {
        entry = AAA_OM_GetAccFirstUser();
    }
    else
    {
        entry = AAA_OM_GetAccUser(user_index);
        if (NULL != entry)
            entry = entry->next_user_entry;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_QueryAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AccUserEntry_T *AAA_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type)
{
    UI16_T      index;

    AAA_AccUserEntry_T  *entry;

    if (NULL == user_name)
        return FALSE;

    for (index = 0; AAA_MAX_NBR_OF_ACC_USERS > index; ++index)
    {
        entry = &acc_user_info[index];

        if (AAA_ENTRY_DESTROYED == entry->entry_status)
            continue;

        if (ifindex != entry->ifindex)
            continue;

        if (client_type != entry->client_type)
            continue;

        if (strcmp((char *)user_name, (char *)entry->user_name) != 0)
            continue;

        AAA_OM_TRACE("user_index(%d) found", entry->user_index);

        return entry;
    }

    AAA_OM_TRACE("Ifindex(%lu) user(%s) client_type(%d) not found",
        ifindex, user_name, client_type);

    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocAccUser
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty user info
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_AccUserEntry_T *AAA_OM_AllocAccUser()
{
    UI16_T      index;

    AAA_AccUserEntry_T  *entry;

    for (index = 0; AAA_MAX_NBR_OF_ACC_USERS > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED != acc_user_info[index].entry_status)
            continue;

        entry = &acc_user_info[index];
        entry->entry_status = AAA_ENTRY_READY;
        entry->next_user_entry = NULL;

        if (NULL == head_of_acc_user)
        {
            /* first entry */
            entry->prev_user_entry = NULL;
            head_of_acc_user = entry;

            AAA_OM_TRACE("First entry");
        }
        else
        {
            entry->prev_user_entry = tail_of_acc_user;
            tail_of_acc_user->next_user_entry = entry;

            AAA_OM_TRACE("Prev user_index(%d)", entry->prev_user_entry->user_index);
        }

        tail_of_acc_user = entry;

        AAA_OM_TRACE("Alloc user_index(%d)", entry->user_index);

        return entry;
    }

    AAA_OM_TRACE("No more available entry");

    return NULL;
}

#endif /* SYS_CPNT_ACCOUNTING == TRUE */



/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GetRadiusEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : get the specified radius entry
 * INPUT    : group_entry, radius_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static AAA_RadiusEntry_T *AAA_OM_GetRadiusEntry(AAA_RadiusGroupEntry_T *group_entry, UI16_T radius_index)
{
    if (NULL == group_entry)
        return NULL;

    if ((0 >= radius_index) || (AAA_MAX_NBR_OF_RADIUS_ENTRY < radius_index))
        return NULL;

    --radius_index; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == group_entry->radius_entry[radius_index].entry_status)
        return NULL;

    return &group_entry->radius_entry[radius_index];
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_ResetRadiusEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : clean radius_entry, head_of_radius_entry, tail_of_radius_entry in AAA_RadiusGroupEntry_T
 * INPUT    : group_entry
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_ResetRadiusEntry(AAA_RadiusGroupEntry_T *group_entry)
{
    AAA_RadiusEntry_T   *current_entry, *next_entry;

    if (NULL == group_entry)
    {
       AAA_OM_TRACE("Null pointer!");
       return;
    }

    if (NULL == group_entry->head_of_radius_entry) /* no entry */
        return;

    current_entry = group_entry->head_of_radius_entry;
    while (NULL != current_entry)
    {
        next_entry = current_entry->next_radius_entry;
        current_entry->entry_status = AAA_ENTRY_DESTROYED;
        current_entry->next_radius_entry = NULL;
        current_entry->prev_radius_entry = NULL;

        current_entry = next_entry;
    }

    group_entry->head_of_radius_entry = NULL;
    group_entry->tail_of_radius_entry = NULL;

    AAA_OM_TRACE("Clean radius entries!");
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty radius entry
 * INPUT    : group_entry
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_RadiusEntry_T *AAA_OM_AllocRadiusEntry(AAA_RadiusGroupEntry_T *group_entry)
{
    UI16_T  radius_index;

    AAA_RadiusEntry_T   *entry;

    if (NULL == group_entry)
    {
        AAA_OM_TRACE("Null pointer!");
        return NULL;
    }

    for (radius_index = 0; AAA_MAX_NBR_OF_RADIUS_ENTRY > radius_index; ++radius_index)
    {
        if (AAA_ENTRY_DESTROYED != group_entry->radius_entry[radius_index].entry_status)
            continue;

        entry = &group_entry->radius_entry[radius_index];
        entry->entry_status = AAA_ENTRY_READY;
        entry->next_radius_entry = NULL;

        AAA_OM_TRACE("Alloc(%d)", entry->radius_index);

        if (NULL == group_entry->head_of_radius_entry)
        {
            /* first entry */
            entry->prev_radius_entry = NULL;
            group_entry->head_of_radius_entry = entry;

            AAA_OM_TRACE("First entry");
        }
        else
        {
            entry->prev_radius_entry = group_entry->tail_of_radius_entry;
            group_entry->tail_of_radius_entry->next_radius_entry = entry;

            AAA_OM_TRACE("Prev server index(%lu)", entry->prev_radius_entry->radius_server_index);
        }

        group_entry->tail_of_radius_entry = entry;

        return entry;
    }

    AAA_OM_TRACE("No more available entry");
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle server entry
 * INPUT    : server_group, server_index (1-based)
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static void AAA_OM_FreeRadiusEntry(AAA_RadiusGroupEntry_T *group_entry, UI16_T radius_index)
{
    AAA_RadiusEntry_T   *entry;

    if (NULL == group_entry)
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    if ((0 >= radius_index) || (AAA_MAX_NBR_OF_RADIUS_ENTRY < radius_index))
    {
        AAA_OM_TRACE("Bad radius_index(%d)", radius_index);
        return;
    }

    entry = &group_entry->radius_entry[radius_index - 1];
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("Should not free a destroyed entry (%d)", radius_index);
        return;
    }

    if (entry == group_entry->head_of_radius_entry)
    {
        AAA_OM_TRACE("entry(%d) is the head of radius entry", radius_index);
        group_entry->head_of_radius_entry = entry->next_radius_entry;
    }

    if (entry == group_entry->tail_of_radius_entry)
    {
        AAA_OM_TRACE("entry(%d) is the tail of radius entry", radius_index);
        group_entry->tail_of_radius_entry = entry->prev_radius_entry;
    }

    if (NULL != entry->prev_radius_entry)
        entry->prev_radius_entry->next_radius_entry = entry->next_radius_entry;

    if (NULL != entry->next_radius_entry)
        entry->next_radius_entry->prev_radius_entry = entry->prev_radius_entry;

    entry->prev_radius_entry = NULL;
    entry->next_radius_entry = NULL;
    entry->entry_status = AAA_ENTRY_DESTROYED;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyRadiusGroupEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyRadiusGroupEntryInterface(AAA_RadiusGroupEntryInterface_T *det, const AAA_RadiusGroupEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->group_index = src->group_index;

    strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    AAA_OM_TRACE("idx(%d) %s", det->group_index, det->group_name);
}


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyRadiusEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyRadiusEntryInterface(AAA_RadiusEntryInterface_T *det, const AAA_RadiusEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->radius_index = src->radius_index;
    det->radius_server_index = src->radius_server_index;

    AAA_OM_TRACE("idx(%d) server_index(%lu)",
        det->radius_index, det->radius_server_index);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyRadiusEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
#if 0
static void AAA_OM_CopyRadiusEntry(AAA_RadiusEntry_T *det, const AAA_RadiusEntryInterface_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("\r\n[AAA_OM_CopyRadiusEntryInterface] null pointer!");
        return;
    }

    det->radius_server_index = src->radius_server_index;

    AAA_OM_TRACE("\r\n[AAA_OM_CopyRadiusEntry] idx(%d) server_index(%lu)",
        det->radius_index, det->radius_server_index);
}
#endif

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_GetTacacsPlusEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : get the specified tacacs entry
 * INPUT    : group_entry, tacacs_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static AAA_TacacsPlusEntry_T *AAA_OM_GetTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *group_entry, UI16_T tacacs_index)
{
    if (NULL == group_entry)
        return NULL;

    if ((0 >= tacacs_index) || (AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY < tacacs_index))
        return NULL;

    --tacacs_index; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == group_entry->tacacs_entry[tacacs_index].entry_status)
        return NULL;

    return &group_entry->tacacs_entry[tacacs_index];
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_ResetTacacsPlusEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : clean tacacs_entry, head_of_tacacs_entry, tail_of_tacacs_entry in AAA_TacacsPlusGroupEntry_T
 * INPUT    : group_entry
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_ResetTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *group_entry)
{
    AAA_TacacsPlusEntry_T   *current_entry, *next_entry;

    if (NULL == group_entry)
    {
       AAA_OM_TRACE("Null pointer!");
       return;
    }

    if (NULL == group_entry->head_of_tacacs_entry) /* no entry */
        return;

    current_entry = group_entry->head_of_tacacs_entry;
    while (NULL != current_entry)
    {
        next_entry = current_entry->next_tacacs_entry;
        current_entry->entry_status = AAA_ENTRY_DESTROYED;
        current_entry->next_tacacs_entry = NULL;
        current_entry->prev_tacacs_entry = NULL;

        current_entry = next_entry;
    }

    group_entry->head_of_tacacs_entry = NULL;
    group_entry->tail_of_tacacs_entry = NULL;

    AAA_OM_TRACE("Clean tacacs entries!");
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty tacacs entry
 * INPUT    : group_entry
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static AAA_TacacsPlusEntry_T *AAA_OM_AllocTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *group_entry)
{
    UI16_T  tacacs_index;

    AAA_TacacsPlusEntry_T   *entry;

    if (NULL == group_entry)
    {
        AAA_OM_TRACE("Null pointer!");
        return NULL;
    }

    for (tacacs_index = 0; AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY > tacacs_index; ++tacacs_index)
    {
        if (AAA_ENTRY_DESTROYED != group_entry->tacacs_entry[tacacs_index].entry_status)
            continue;

        entry = &group_entry->tacacs_entry[tacacs_index];
        entry->entry_status = AAA_ENTRY_READY;
        entry->next_tacacs_entry = NULL;

        AAA_OM_TRACE("Alloc(%d)", entry->tacacs_index);

        if (NULL == group_entry->head_of_tacacs_entry)
        {
            /* first entry */
            entry->prev_tacacs_entry = NULL;
            group_entry->head_of_tacacs_entry = entry;

            AAA_OM_TRACE("First entry");
        }
        else
        {
            entry->prev_tacacs_entry = group_entry->tail_of_tacacs_entry;
            group_entry->tail_of_tacacs_entry->next_tacacs_entry = entry;

            AAA_OM_TRACE("Prev server index(%lu)", entry->prev_tacacs_entry->tacacs_server_index);
        }

        group_entry->tail_of_tacacs_entry = entry;

        return entry;
    }

    AAA_OM_TRACE("No more available entry");
    return NULL;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle server entry
 * INPUT    : server_group, server_index (1-based)
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *-------------------------------------------------------------------------*/
static void AAA_OM_FreeTacacsPlusEntry(AAA_TacacsPlusGroupEntry_T *group_entry, UI16_T tacacs_index)
{
    AAA_TacacsPlusEntry_T   *entry;

    if (NULL == group_entry)
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    if ((0 >= tacacs_index) || (AAA_MAX_NBR_OF_TACACS_PLUS_ENTRY < tacacs_index))
    {
        AAA_OM_TRACE("Bad tacacs_index(%d)", tacacs_index);
        return;
    }

    entry = &group_entry->tacacs_entry[tacacs_index - 1];
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("Should not free a destroyed entry (%d)", tacacs_index);
        return;
    }

    if (entry == group_entry->head_of_tacacs_entry)
    {
        AAA_OM_TRACE("entry(%d) is the head of tacacs entry", tacacs_index);
        group_entry->head_of_tacacs_entry = entry->next_tacacs_entry;
    }

    if (entry == group_entry->tail_of_tacacs_entry)
    {
        AAA_OM_TRACE("entry(%d) is the tail of tacacs entry", tacacs_index);
        group_entry->tail_of_tacacs_entry = entry->prev_tacacs_entry;
    }

    if (NULL != entry->prev_tacacs_entry)
        entry->prev_tacacs_entry->next_tacacs_entry = entry->next_tacacs_entry;

    if (NULL != entry->next_tacacs_entry)
        entry->next_tacacs_entry->prev_tacacs_entry = entry->prev_tacacs_entry;

    entry->prev_tacacs_entry = NULL;
    entry->next_tacacs_entry = NULL;
    entry->entry_status = AAA_ENTRY_DESTROYED;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyTacacsPlusGroupEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyTacacsPlusGroupEntryInterface(AAA_TacacsPlusGroupEntryInterface_T *det, const AAA_TacacsPlusGroupEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->group_index = src->group_index;

    strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    AAA_OM_TRACE("idx(%d) %s", det->group_index, det->group_name);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyTacacsPlusEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyTacacsPlusEntryInterface(AAA_TacacsPlusEntryInterface_T *det, const AAA_TacacsPlusEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->tacacs_index = src->tacacs_index;
    det->tacacs_server_index = src->tacacs_server_index;

    AAA_OM_TRACE("idx(%d) server_index(%lu)",
        det->tacacs_index, det->tacacs_server_index);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsValidGroupName
 * ---------------------------------------------------------------------
 * PURPOSE  : check name
 * INPUT    : name
 * OUTPUT   : none
 * RETURN   : TRUE - valid, FALSE - invalid
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsValidGroupName(const char *name)
{
    UI32_T      name_len;

    if (NULL == name)
        return FALSE;

    name_len = strlen((char *)name);
    if ((0 >= name_len) || (SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME < name_len))
    {
        AAA_OM_TRACE("Bad name(%s) length {1-%lu}", name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        return FALSE;
    }

    if ((strcmp((char *)name, SYS_DFLT_AAA_RADIUS_GROUP_NAME) == 0) ||
        (strcmp((char *)name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME) == 0))
    {
        AAA_OM_TRACE("Bad name(%s)", name);
        return FALSE;
    }

    return TRUE;
}

#if (SYS_CPNT_ACCOUNTING == TRUE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsValidMethodListName
 * ---------------------------------------------------------------------
 * PURPOSE  : check name
 * INPUT    : name
 * OUTPUT   : none
 * RETURN   : TRUE - valid, FALSE - invalid
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsValidMethodListName(const char *name)
{
    UI32_T      name_len;

    if (NULL == name)
        return FALSE;

    name_len = strlen((char *)name);
    if ((0 >= name_len) || (SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME < name_len))
    {
        AAA_OM_TRACE("Bad name(%s) length {1-%lu}", name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
        return FALSE;
    }

    if (strcmp((char *)name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Bad name(%s)", name);
        return FALSE;
    }

    return TRUE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsAnyListReferThisGroup
 * ---------------------------------------------------------------------
 * PURPOSE  : check if any method list refer to a group named group_name
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not found
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsAnyListReferThisGroup(const char *group_name)
{
    UI16_T  list_index;

    AAA_AccListEntry_T  *list_entry;

    if (NULL == group_name)
        return FALSE;

    for (list_index = 1; AAA_OM_MAX_NBR_OF_ACC_LIST >= list_index; ++list_index)
    {
        list_entry = AAA_OM_GetAccListEntryByIndex(list_index);
        if (NULL == list_entry)
            return FALSE;

        if (strcmp((char *)list_entry->group_name, (char *)group_name) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsAnyPortReferThisList
 * ---------------------------------------------------------------------
 * PURPOSE  : check if any port refer to a list named list_name
 * INPUT    : list_name
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not found
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsAnyPortReferThisList(const char *list_name)
{
    UI16_T  ifindex;

    AAA_AccDot1xEntry_T  *dot1x_entry;

    if (NULL == list_name)
        return FALSE;

    for (ifindex = 1; SYS_ADPT_TOTAL_NBR_OF_LPORT >= ifindex; ++ifindex) /* 1-based */
    {
        dot1x_entry = AAA_OM_GetAccDot1xEntry(ifindex);
        if (NULL == dot1x_entry)
            return FALSE;

        if (strcmp((char *)dot1x_entry->list_name, (char *)list_name) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsAnyExecReferThisList
 * ---------------------------------------------------------------------
 * PURPOSE  : check if any exec refer to a list named list_name
 * INPUT    : list_name
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not found
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsAnyExecReferThisList(const char *list_name)
{
    UI16_T  index;

    AAA_AccExecEntry_T  *exec_entry;

    if (NULL == list_name)
        return FALSE;

    for (index = 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index) /* 1-based */
    {
        exec_entry = AAA_OM_GetAccExecEntry(index);
        if (NULL == exec_entry)
            return FALSE;

        if (strcmp((char *)exec_entry->list_name, (char *)list_name) == 0)
            return TRUE;
    }

    return FALSE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyListEntryInterface(
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyListEntryInterface(AAA_AccListEntryInterface_T *det, const AAA_AccListEntry_T *src)
{
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->list_index  = src->list_index;
    det->client_type = src->client_type;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    det->priv_lvl    = src->priv_lvl;
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    det->working_mode = src->working_mode;

    strncpy((char *)det->list_name, (char *)src->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
    det->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */

    strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    det->group_type = GROUP_UNKNOWN;

    radius_group = AAA_OM_GetRadiusGroupEntry(src->group_name);
    if (NULL != radius_group)
    {
        det->group_type = GROUP_RADIUS;
    }
    else
    {
        /* not found in radius group */

        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(src->group_name);
        if (NULL != tacacs_group)
        {
            det->group_type = GROUP_TACACS_PLUS;
        }
    }

    AAA_OM_TRACE("idx(%d) name(%s) group(%s) mode(%d)",
        det->list_index, det->list_name, det->group_name, det->working_mode);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyListEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static AAA_ApiUpdateResult_T AAA_OM_CopyListEntry(AAA_AccListEntry_T *det, const AAA_AccListEntryInterface_T *src)
{
    BOOL_T      is_modified;

    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return AAA_API_UPDATE_FAILED;
    }

    is_modified = FALSE;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
    if(det->priv_lvl != src->priv_lvl)
    {
        is_modified = TRUE;
        det->priv_lvl = src->priv_lvl;
    }
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/

    if (det->working_mode != src->working_mode)
    {
        is_modified = TRUE;
        det->working_mode = src->working_mode;
    }

    if (strcmp((char *)det->list_name, (char *)src->list_name) != 0)
    {
        is_modified = TRUE;
        strncpy((char *)det->list_name, (char *)src->list_name, SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME);
        det->list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME] = '\0'; /* force to end a string */
    }

    if (strcmp((char *)det->group_name, (char *)src->group_name) != 0)
    {
        is_modified = TRUE;
        strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */
    }

    AAA_OM_TRACE("idx(%d) name(%s) group(%s) mode(%d)",
        det->list_index, det->list_name, det->group_name, det->working_mode);

    return is_modified ? AAA_API_UPDATE_CHANGE : AAA_API_UPDATE_NO_CHANGE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyUserEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyUserEntryInterface(AAA_AccUserEntryInterface_T *det, const AAA_AccUserEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->user_index = src->user_index;
    det->ifindex = src->ifindex;
    det->client_type = src->client_type;
    memcpy(&det->notify_bitmap, &src->notify_bitmap, sizeof(AAA_AccNotifyMap_T));
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyUserInfoInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyUserInfoInterface(AAA_AccUserInfoInterface_T *det, const AAA_AccUserEntry_T *src)
{
    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->user_index = src->user_index;
    det->ifindex = src->ifindex;

    strncpy((char *)det->user_name, (char *)src->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    det->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    det->accounting_start_time = src->accounting_start_time;
    det->client_type = src->client_type;

    if (1 == src->notify_bitmap.radius_start)
        det->group_type = GROUP_RADIUS;
    else if (1 == src->notify_bitmap.tacacs_start)
        det->group_type = GROUP_TACACS_PLUS;
    else
        det->group_type = GROUP_UNKNOWN;
}

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_FindGroupNameOfDot1xList
 * ---------------------------------------------------------------------
 * PURPOSE  : try to find whether exist a dot1x list associated with specified group
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - found, FALSE - not found
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_FindGroupNameOfDot1xList(const char *group_name)
{
    UI16_T  index;

    if (NULL == group_name)
    {
        AAA_OM_TRACE("Null pointer");
        return FALSE;
    }

    for (index = 0; AAA_OM_MAX_NBR_OF_ACC_LIST > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED == method_list[index].entry_status)
            continue;

        if (AAA_CLIENT_TYPE_DOT1X != method_list[index].client_type)
            continue;

        if (strcmp((char *)method_list[index].group_name, (char *)group_name) == 0)
            return TRUE;
    }

    return FALSE;
}

#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */


/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_DumpAccUserTable
 * ---------------------------------------------------------------------
 * PURPOSE  : try to dump user table
 * INPUT    : none
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : this func is used to debug only
 * ---------------------------------------------------------------------*/
#if 0
void AAA_OM_DumpAccUserTable()
{
    UI16_T      index;

    AAA_AccUserEntry_T  *entry;

    for (index = 0; AAA_MAX_NBR_OF_ACC_USERS > index; ++index)
    {
        entry = &acc_user_info[index];

        if (AAA_ENTRY_DESTROYED == entry->entry_status)
            continue;

        printf("\r\n[AAA_OM_DumpAccUserTable] index(%d)", entry->user_index);
        printf("\r\n%15s(%10s) %15s(%5lu), %15s(%5lu)",
            "name", entry->user_name,
            "ifindex", entry->ifindex,
            "start time", entry->accounting_start_time);

        printf("\r\n%15s(%10lu) %15s(%10s)",
            "identifier", entry->identifier,
            "client type",
            (AAA_CLIENT_TYPE_DOT1X == entry->client_type) ? "AAA_CLIENT_TYPE_DOT1X" :
            "unknown client type");

        printf("\r\n%15s(%10s) %15s(%10s)",
            "radius_start", (1 == entry->notify_bitmap.radius_start) ? "Yes" : "No",
            "tacacs_start", (1 == entry->notify_bitmap.tacacs_start) ? "Yes" : "No");
    }
 }
#endif /* if 0 */

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

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
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetAuthorizationGroupIndex(UI32_T priv_lvl, AAA_ClientType_T client_type, UI32_T super_ifindex, AAA_QueryGroupIndexResult_T *query_result)
{
    AAA_AuthorExecEntry_T       *exec_entry;
#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    AAA_AuthorCommandEntry_T    *command_entry;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */
    AAA_AuthorListEntry_T       *list_entry;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;
    AAA_ListType_T              list_type;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == query_result)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memset(&list_type, 0, sizeof(list_type));

    list_type.client_type = client_type;

    switch (client_type)
    {
        case AAA_CLIENT_TYPE_EXEC:
            exec_entry = AAA_OM_GetAuthorExecEntry(super_ifindex);
            if (NULL == exec_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = AAA_OM_GetAuthorListEntry(exec_entry->list_name, &list_type);

            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry (%s) not found", exec_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            command_entry = AAA_OM_GetAuthorCommandEntry(priv_lvl, super_ifindex);
            if (NULL == command_entry)
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_type.priv_lvl = priv_lvl;
            list_entry = AAA_OM_GetAuthorListEntry(command_entry->list_name, &list_type);

            if (NULL == list_entry)
            {
                AAA_OM_TRACE("list_entry (%s) not found", command_entry->list_name);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }
            break;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        default:
            AAA_OM_TRACE("Bad client type");
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(list_entry->group_name);
    if (NULL == tacacs_group)
    {
        AAA_OM_TRACE("group (%s) not found", list_entry->group_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
    query_result->group_index = tacacs_group->group_index;
    query_result->group_type = GROUP_TACACS_PLUS;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry)
{
    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    exec_entry = AAA_OM_GetAuthorExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strlen((char *)entry->list_name) == 0)
    {
        AAA_OM_TRACE("list_name can not be an empty string");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strcmp((char *)entry->list_name, (char *)exec_entry->list_name) == 0) /* the same name */
    {
        if (strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) /* default method list */
            exec_entry->configure_mode = AAA_MANUAL_CONFIGURE;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    strncpy((char *)exec_entry->list_name, (char *)entry->list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
    exec_entry->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

    exec_entry->configure_mode = AAA_MANUAL_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_DisableAuthorExecEntry(AAA_ExecType_T exec_type)
{
    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    exec_entry = AAA_OM_GetAuthorExecEntry(exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry->list_name[0] = '\0';
    exec_entry->configure_mode = AAA_AUTO_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0')
 * OUTPUT   : none
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
AAA_ApiUpdateResult_T AAA_OM_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name)
{
    I32_T  str_len;

    AAA_ApiUpdateResult_T   result;
    AAA_AuthorListEntry_T      *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == list_type || NULL == group_name)
    {
        AAA_OM_TRACE("Null pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    str_len = strlen((char *)group_name);
    if (0 >= str_len)
    {
        AAA_OM_TRACE("group_name has a bad length(%ld)", str_len);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    list_entry = AAA_OM_GetAuthorListEntry(SYS_DFLT_AAA_METHOD_LIST_NAME, list_type);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Should not go here, AAA_OM_Initialize() forgot to insert or AAA_OM_FreeAuthorListEntry() free the default entry");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == list_entry->list_type.client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

    result = AAA_API_UPDATE_NO_CHANGE;
#if 0
    if (working_mode != list_entry->working_mode)
    {
        list_entry->working_mode = working_mode;
        result = AAA_API_UPDATE_CHANGE;
    }
#endif
    if (strcmp((char *)group_name, (char *)list_entry->group_name) != 0)
    {
        strncpy((char *)list_entry->group_name, (char *)group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

        result = AAA_API_UPDATE_CHANGE;
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(result);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the authorization list by name.
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry)
{
    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAuthorListEntry(entry->list_name, &entry->list_type);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyAuthorListEntryInterface(entry, list_entry);
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
BOOL_T AAA_OM_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry)
{
    BOOL_T      ret;

    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    AAA_OM_TRACE("list name(%s), group name(%s)", entry->list_name, entry->group_name);

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

#if (AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE)

    /* do not allow a dot1x method-list associate with tacacs+ group */
    if ((AAA_CLIENT_TYPE_DOT1X == entry->list_type.client_type) &&
        (NULL != AAA_OM_GetTacacsPlusGroupEntry(entry->group_name)))
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }
#endif /* AAA_ACCOUNTING_DOT1X_SUPPORT_BY_TACACS_PLUS == FALSE */

    list_entry = AAA_OM_GetAuthorListEntry(entry->list_name, &entry->list_type);
    if (NULL == list_entry)
    {
        /* create a new entry */
        list_entry = AAA_OM_AllocAuthorListEntry(&entry->list_type);
        if (NULL == list_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
        }

        AAA_OM_TRACE("Create a new entry (%s) client_type(%d)", entry->list_name, entry->list_type.client_type);
    }

    ret = (AAA_API_UPDATE_FAILED != AAA_OM_CopyAuthorListEntry(list_entry, entry));

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_OM_DestroyAuthorListEntry(const char *name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning)
{
    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == name || NULL == list_type)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAuthorListEntry(name, list_type);
    if (NULL == list_entry)
    {
        if (warning)
            warning->warning_type = AAA_NO_WARNING;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (FALSE == AAA_OM_FreeAuthorListEntry(list_entry->list_index)) /* should not use list_entry thereafter */
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (warning)
    {
        switch (list_type->client_type)
        {
            case AAA_CLIENT_TYPE_DOT1X:
                //warning->warning_type = AAA_OM_IsAnyPortReferThisList(name) ? AAA_ACC_DOT1X_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

            case AAA_CLIENT_TYPE_EXEC:
                warning->warning_type = AAA_OM_IsAnyExecReferThisAuthorList(name) ? AAA_AUTHOR_EXEC_REF2_BAD_LIST : AAA_NO_WARNING;
                break;

            default:
                warning->warning_type = AAA_NO_WARNING;
                break;
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_FreeAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : recycle accounting method list
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_FreeAuthorListEntry(UI16_T list_index)
{
    AAA_AuthorListEntry_T  *entry;

    if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_AUTHOR_LIST < list_index))
    {
        AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_AUTHOR_LIST);
        return FALSE;
    }

    entry = &author_method_list[list_index - 1]; /* to zero-based */

    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("list_index(%d) no need to free", list_index);
        return FALSE;
    }

    if (strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Can't free ""default"" list");
        return FALSE;
    }

    AAA_OM_TRACE("Free list_index(%d)", list_index);

    entry->entry_status = AAA_ENTRY_DESTROYED;
    entry->list_name[0] = '\0';
    entry->group_name[0] = '\0';

    switch (entry->list_type.client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            //--acc_dot1x_list_qty;
            break;

        case AAA_CLIENT_TYPE_EXEC:
            --author_exec_list_qty;
            break;

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            --author_command_list_qty;
            break;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        default:
            AAA_OM_TRACE("Bad client_type(%d)", entry->list_type.client_type);
            break;
    }

    return TRUE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorListEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry)
{
    UI16_T  list_index;

    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (list_index = entry->list_index + 1; AAA_OM_MAX_NBR_OF_AUTHOR_LIST >= list_index; ++list_index)
    {
        list_entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
        if (NULL == list_entry)
            continue;

        AAA_OM_CopyAuthorListEntryInterface(entry, list_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorListEntryFilterByClientType(AAA_AuthorListEntryInterface_T *entry)
{
    UI16_T  list_index;

    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    for (list_index = entry->list_index + 1; AAA_OM_MAX_NBR_OF_AUTHOR_LIST >= list_index; ++list_index)
    {
        list_entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
        if (NULL == list_entry)
            continue;

        if (entry->list_type.client_type != list_entry->list_type.client_type)
            continue;

        AAA_OM_CopyAuthorListEntryInterface(entry, list_entry);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry = AAA_OM_GetAuthorExecEntry(entry->exec_type + 1); /* next one */
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, exec_entry, sizeof(AAA_AuthorExecEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization entry by exec_type.
 * INPUT    : exec_type (1-based)
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AuthorExecEntry_T *AAA_OM_GetAuthorExecEntry(AAA_ExecType_T exec_type)
{
    if ((AAA_EXEC_TYPE_NONE >= exec_type) || (AAA_EXEC_TYPE_SUPPORT_NUMBER  < exec_type))
    {
        AAA_OM_TRACE("Bad exec_type(%d)", exec_type);
        return NULL;
    }

    return &author_exec_entry[exec_type - 1]; /* to zero-based */
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the authorization list by name.
 * INPUT    : list_name, client_type
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------
 */
static AAA_AuthorListEntry_T *AAA_OM_GetAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type)
{
    UI16_T  index;

    if (NULL == list_name || NULL == list_type)
    {
        AAA_OM_TRACE("Null pointer");
        return NULL;
    }

    for (index = 0; AAA_OM_MAX_NBR_OF_AUTHOR_LIST > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED == author_method_list[index].entry_status)
            continue;

        if (list_type->client_type != author_method_list[index].list_type.client_type)
            continue;

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        if (list_type->client_type == AAA_CLIENT_TYPE_COMMANDS)
        {
            if (list_type->priv_lvl != author_method_list[index].list_type.priv_lvl)
            continue;
        }
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        if (strcmp(author_method_list[index].list_name, list_name) == 0)
            return &author_method_list[index];
    }

    AAA_OM_TRACE("list (%s) client_type(%d) not found", list_name, list_type->client_type);

    return NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyAuthorListEntryInterface
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static void AAA_OM_CopyAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *det, const AAA_AuthorListEntry_T *src)
{
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;

    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return;
    }

    det->list_index = src->list_index;
    det->list_type = src->list_type;

    strncpy((char *)det->list_name, (char *)src->list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
    det->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

    strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

#if 1
    det->group_type = GROUP_UNKNOWN;

    radius_group = AAA_OM_GetRadiusGroupEntry(src->group_name);
    if (NULL != radius_group)
    {
        det->group_type = GROUP_RADIUS;
    }
    else
    {
        /* not found in radius group */

        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(src->group_name);
        if (NULL != tacacs_group)
        {
            det->group_type = GROUP_TACACS_PLUS;
        }
    }

    AAA_OM_TRACE("idx(%d) name(%s) group(%s)",
        det->list_index, det->list_name, det->group_name);
#endif
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_AllocAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : allocate an empty authorization method list
 * INPUT    : client_type
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none
 *-------------------------------------------------------------------------
 */
static AAA_AuthorListEntry_T *AAA_OM_AllocAuthorListEntry(const AAA_ListType_T *list_type)
{
    UI16_T      index;
    UI16_T      *current_qty = NULL;

    AAA_AuthorListEntry_T  *entry;

    if (NULL == list_type)
    {
        AAA_OM_TRACE("NULL pointer");
        return NULL;
    }

    switch (list_type->client_type)
    {
        case AAA_CLIENT_TYPE_DOT1X:
            //current_qty = &acc_dot1x_list_qty;
            break;

        case AAA_CLIENT_TYPE_EXEC:
            current_qty = &author_exec_list_qty;
            break;

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        case AAA_CLIENT_TYPE_COMMANDS:
            current_qty = &author_command_list_qty;
            break;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        default:
            AAA_OM_TRACE("Bad client type %d", list_type->client_type);
            return NULL;
    }

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
    if (list_type->client_type == AAA_CLIENT_TYPE_COMMANDS)
    {
        if (SYS_ADPT_NBR_OF_LOGIN_PRIVILEGE+SYS_ADPT_MAX_NBR_OF_AUTHORIZATION_LIST <= *current_qty)
        {
            AAA_OM_TRACE("No more available entry for client_type(%d)", list_type->client_type);
            return NULL;
        }
    }
    else
#endif /*#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)*/
    if (SYS_ADPT_MAX_NBR_OF_AUTHORIZATION_LIST <= *current_qty)
    {
        AAA_OM_TRACE("No more available entry for client_type(%d)", list_type->client_type);
        return NULL;
    }

    for (index = 0; AAA_OM_COUNT_OF(author_method_list) > index; ++index)
    {
        if (AAA_ENTRY_DESTROYED != author_method_list[index].entry_status)
            continue;

        entry = &author_method_list[index];

        entry->list_name[0] = '\0';
        entry->group_name[0] = '\0';

        entry->list_type = *list_type;
        entry->entry_status = AAA_ENTRY_READY;

        ++(*current_qty); /* increase the qty */

        AAA_OM_TRACE("Alloc list_index(%d)", entry->list_index);

        return entry;
    }

    AAA_OM_TRACE("No more available entry");

    return NULL;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_CopyAuthorListEntry
 * ---------------------------------------------------------------------
 * PURPOSE  : copy src to det
 * INPUT    : src
 * OUTPUT   : det
 * RETURN   : none
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static AAA_ApiUpdateResult_T AAA_OM_CopyAuthorListEntry(AAA_AuthorListEntry_T *det, const AAA_AuthorListEntryInterface_T *src)
#if 0
{
    AAA_RadiusGroupEntry_T      *radius_group;
    AAA_TacacsPlusGroupEntry_T  *tacacs_group;
    BOOL_T      is_modified;

    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("\r\n[AAA_OM_CopyAuthorListEntry] null pointer!");
        return AAA_API_UPDATE_FAILED;
    }
    is_modified = FALSE;

    det->list_index = src->list_index;
    det->client_type = src->client_type;
//    det->working_mode = src->working_mode;

    if (strcmp(det->list_name, src->list_name) != 0)
    {
        is_modified = TRUE;
        strncpy(det->list_name, src->list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
        det->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */
    }

    if (strcmp(det->group_name, src->group_name) != 0)
    {
        is_modified = TRUE;
        strncpy(det->group_name, src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */
    }

    det->group_type = GROUP_UNKNOWN;

    radius_group = AAA_OM_GetRadiusGroupEntry(src->group_name);
    if (NULL != radius_group)
    {
        det->group_type = GROUP_RADIUS;
    }
    else
    {
        /* not found in radius group */

        tacacs_group = AAA_OM_GetTacacsPlusGroupEntry(src->group_name);
        if (NULL != tacacs_group)
        {
            det->group_type = GROUP_TACACS_PLUS;
        }
    }

    AAA_OM_TRACE("\r\n[AAA_OM_CopyAuthorListEntry] idx(%d) name(%s) group(%s) mode(%d)",
        det->list_index, det->list_name, det->group_name, det->working_mode);
    return is_modified ? AAA_API_UPDATE_CHANGE : AAA_API_UPDATE_NO_CHANGE;

}
#endif
{
    BOOL_T      is_modified;

    if ((NULL == det) || (NULL == src))
    {
        AAA_OM_TRACE("Null pointer!");
        return AAA_API_UPDATE_FAILED;
    }

    is_modified = FALSE;
#if 0
    if (det->working_mode != src->working_mode)
    {
        is_modified = TRUE;
        det->working_mode = src->working_mode;
    }
#endif
    if (strcmp((char *)det->list_name, (char *)src->list_name) != 0)
    {
        is_modified = TRUE;
        strncpy((char *)det->list_name, (char *)src->list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
        det->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */
    }

    if (strcmp((char *)det->group_name, (char *)src->group_name) != 0)
    {
        is_modified = TRUE;
        strncpy((char *)det->group_name, (char *)src->group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
        det->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */
    }

    AAA_OM_TRACE("idx(%d) name(%s) group(%s)",
        det->list_index, det->list_name, det->group_name);

    return is_modified ? AAA_API_UPDATE_CHANGE : AAA_API_UPDATE_NO_CHANGE;
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsAnyExecReferThisAuthorList
 * ---------------------------------------------------------------------
 * PURPOSE  : check if any exec refer to a list named list_name
 * INPUT    : list_name
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not found
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsAnyExecReferThisAuthorList(const char *list_name)
{
    UI16_T  index;

    AAA_AuthorExecEntry_T  *exec_entry;

    if (NULL == list_name)
        return FALSE;

    for (index = 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index) /* 1-based */
    {
        exec_entry = AAA_OM_GetAuthorExecEntry(index);
        if (NULL == exec_entry)
            return FALSE;

        if (strcmp((char *)exec_entry->list_name, (char *)list_name) == 0)
            return TRUE;
    }

    return FALSE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization list by index.
 * INPUT    : list_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
static AAA_AuthorListEntry_T *AAA_OM_GetAuthorListEntryByIndex(UI16_T list_index)
{
    AAA_AuthorListEntry_T  *entry;

    if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_AUTHOR_LIST < list_index))
    {
        AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_AUTHOR_LIST);
        return NULL;
    }

    entry = &author_method_list[list_index - 1]; /* to zero-based */
    if (AAA_ENTRY_DESTROYED == entry->entry_status)
    {
        AAA_OM_TRACE("list_index(%d) is destroyed", list_index);
        return NULL;
    }

    return entry;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorExecEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorExecEntryInterface(AAA_AuthorExecEntry_T *entry)
{
    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    exec_entry = AAA_OM_GetAuthorExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, exec_entry, sizeof(AAA_AuthorExecEntry_T));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAuthorExecEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    exec_entry = AAA_OM_GetAuthorExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (AAA_MANUAL_CONFIGURE == exec_entry->configure_mode)
    {
        memcpy(entry, exec_entry, sizeof(AAA_AuthorExecEntry_T));
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry)
{
    UI32_T      index;

    AAA_AuthorExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    for (index = entry->exec_type + 1; AAA_EXEC_TYPE_SUPPORT_NUMBER >= index; ++index) /* 1-based */
    {
        exec_entry = AAA_OM_GetAuthorExecEntry(index);
        if (NULL == exec_entry)
        {
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
        }

        if (AAA_MANUAL_CONFIGURE == exec_entry->configure_mode)
        {
            memcpy(entry, exec_entry, sizeof(AAA_AuthorExecEntry_T));
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
        }
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorListEntryInterface(AAA_AuthorListEntryInterface_T *entry)
{
    while (AAA_OM_GetNextAuthorListEntryInterface(entry) == TRUE)
    {
        if (entry->list_type.client_type == AAA_CLIENT_TYPE_EXEC)
        {
            if ((strcmp((char *)entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) &&       /* default method-list */
                (strcmp((char *)entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_EXEC_LIST) == 0))    /* with default group-name */
                continue;
        }
#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        else if (entry->list_type.client_type == AAA_CLIENT_TYPE_COMMANDS)
        {
            if ((strcmp(entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) &&
                (strcmp(entry->group_name, AAA_DFLT_GROUP_NAME_FOR_DFLT_COMMAND_LIST) == 0))
                continue;
        }
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        return SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    }

    return SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorListEntryInterfaceByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorListEntryInterfaceByIndex(AAA_AuthorListEntryInterface_T *entry)
{
    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAuthorListEntryByIndex(entry->list_index);
    if (NULL == list_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_CopyAuthorListEntryInterface(entry, list_entry);

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryName
 *------------------------------------------------------------------------
 * PURPOSE  : set the list_name by list_index.
 * INPUT    : list_index, list_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryName(UI16_T list_index, char* list_name)
{
    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == list_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Can't modify default method-list name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->list_name, (char *)list_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    if (AAA_OM_IsValidAuthorMethodListName(list_name) == FALSE)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (AAA_OM_GetAuthorListEntry(list_name, &list_entry->list_type) != NULL)
    {
        /* method-list name can't duplicate */
        AAA_OM_TRACE("There is a method-list with the same name(%s)", list_name);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    strncpy((char *)list_entry->list_name, (char *)list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
    list_entry->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/* ---------------------------------------------------------------------
 * ROUTINE NAME  - AAA_OM_IsValidAuthorMethodListName
 * ---------------------------------------------------------------------
 * PURPOSE  : check name
 * INPUT    : name
 * OUTPUT   : none
 * RETURN   : TRUE - valid, FALSE - invalid
 * NOTES    : none
 * ---------------------------------------------------------------------*/
static BOOL_T AAA_OM_IsValidAuthorMethodListName(const char *name)
{
    UI32_T      name_len;

    if (NULL == name)
        return FALSE;

    name_len = strlen((char *)name);
    if ((0 >= name_len) || (SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME < name_len))
    {
        AAA_OM_TRACE("Bad name(%s) length {1-%lu}", name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
        return FALSE;
    }

    if (strcmp((char *)name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0)
    {
        AAA_OM_TRACE("Bad name(%s)", name);
        return FALSE;
    }

    return TRUE;
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : set the group_name by list_index
 * INPUT    : list_index, group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryGroupName(UI16_T list_index, char* group_name)
{
    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == group_name)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    list_entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
    if (NULL == list_entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    if (strcmp((char *)list_entry->group_name, (char *)group_name) == 0)
    {
        /* the same name, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
    }

    strncpy((char *)list_entry->group_name, (char *)group_name, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
    list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the working mode by list_index.
 * INPUT    : list_index, client_type
 * OUTPUT   : None
 * RETURN   : AAA_ApiUpdateResult_T
 * NOTE     : None
 *------------------------------------------------------------------------*/
AAA_ApiUpdateResult_T AAA_OM_SetAuthorListEntryClientType(UI16_T list_index, const AAA_ListType_T *list_type)
{
    AAA_AuthorListEntry_T  *entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
    if (NULL == entry)
    {
        AAA_OM_TRACE("Bad list_index(%d)", list_index);
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (list_type->client_type == entry->list_type.client_type)
    {
        /* the same mode, nothing to do */
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    if (AAA_OM_GetAuthorListEntry(entry->list_name, list_type) != NULL)
    {
        /* method-list name can't duplicate */
        AAA_OM_TRACE("There is a method-list with the same name");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }
    entry->list_type = *list_type;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAuthorListEntryStatus
 *------------------------------------------------------------------------
 * PURPOSE  : set the status by list_index.
 * INPUT    : list_index, entry_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_OM_SetAuthorListEntryStatus(UI16_T list_index, AAA_EntryStatus_T entry_status)
{
    BOOL_T      ret;

    AAA_AuthorListEntry_T  *list_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    switch (entry_status)
    {
        case AAA_ENTRY_READY:

            list_entry = AAA_OM_GetAuthorListEntryByIndex(list_index);
            if (NULL != list_entry) /* already existed */
            {
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
            }

            list_entry = AAA_OM_GetAuthorListEntry(AAA_SNMP_CREATE_DFLT_AUTHOR_METHOD_LIST_NAME, &list_entry->list_type);
            if (NULL != list_entry) /* already existed */
            {
                AAA_OM_TRACE("Can't create two exec list with default method-list name");
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            if ((0 >= list_index) || (AAA_OM_MAX_NBR_OF_AUTHOR_LIST < list_index))
            {
                AAA_OM_TRACE("Bad list_index(%d) {1 - %d}", list_index, AAA_OM_MAX_NBR_OF_AUTHOR_LIST);
                AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
            }

            list_entry = &author_method_list[list_index - 1]; /* to zero-based */

            strncpy((char *)list_entry->list_name, AAA_SNMP_CREATE_DFLT_AUTHOR_METHOD_LIST_NAME, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
            list_entry->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

            strncpy((char *)list_entry->group_name, SYS_DFLT_AAA_TACACS_PLUS_GROUP_NAME, SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME);
            list_entry->group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME] = '\0'; /* force to end a string */

            list_entry->list_type.client_type = AAA_CLIENT_TYPE_EXEC;
            list_entry->entry_status = AAA_ENTRY_READY;
            break;

        case AAA_ENTRY_DESTROYED:
            ret = AAA_OM_FreeAuthorListEntry(list_index);

            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(ret);

        default:
            AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

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
AAA_ApiUpdateResult_T AAA_OM_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry)
{
    AAA_AuthorCommandEntry_T     *command_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_TRACE("NULL pointer");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strlen(entry->list_name) == 0)
    {
        AAA_OM_TRACE("list_name can not be an empty string");
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    command_entry = AAA_OM_GetAuthorCommandEntry(entry->priv_lvl, entry->exec_type);
    if (NULL == command_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_FAILED);
    }

    if (strcmp(entry->list_name, command_entry->list_name) == 0) /* the same name */
    {
        if (strcmp(entry->list_name, SYS_DFLT_AAA_METHOD_LIST_NAME) == 0) /* default method list */
            command_entry->configure_mode = AAA_MANUAL_CONFIGURE;

        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_NO_CHANGE);
    }

    strncpy(command_entry->list_name, entry->list_name, SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME);
    command_entry->list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME] = '\0'; /* force to end a string */

    command_entry->configure_mode = AAA_MANUAL_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(AAA_API_UPDATE_CHANGE);
}

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
BOOL_T AAA_OM_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type)
{
    AAA_AuthorCommandEntry_T     *command_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    command_entry = AAA_OM_GetAuthorCommandEntry(priv_lvl, exec_type);
    if (NULL == command_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    command_entry->list_name[0] = '\0';
    command_entry->configure_mode = AAA_AUTO_CONFIGURE;

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorCommandEntryIPC
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *-------------------------------------------------------------------------
 */
static BOOL_T AAA_OM_GetAuthorCommandEntryIPC(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry)
{
    AAA_AuthorCommandEntry_T     *command_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    command_entry = AAA_OM_GetAuthorCommandEntry(priv_lvl, exec_type);
    if (NULL == command_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(FALSE);
    }

    memcpy(entry, command_entry, sizeof(*entry));
    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(TRUE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRunningAuthorCommandEntry
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
 *-------------------------------------------------------------------------
 */
static SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type, AAA_AuthorCommandEntry_T *entry)
{
    AAA_AuthorCommandEntry_T     *command_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    command_entry = AAA_OM_GetAuthorCommandEntry(priv_lvl, exec_type);
    if (NULL == command_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (AAA_MANUAL_CONFIGURE == command_entry->configure_mode)
    {
        memcpy(entry, command_entry, sizeof(*entry));
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization entry by exec_type.
 * INPUT    : exec_type (1-based)
 * OUTPUT   : entry
 * RETURN   : NULL - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------
 */
static AAA_AuthorCommandEntry_T *AAA_OM_GetAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type)
{
    if (priv_lvl > SYS_ADPT_MAX_LOGIN_PRIVILEGE)
    {
        AAA_OM_TRACE("Bad priv_lvl(%lu)", priv_lvl);
        return NULL;
    }

    if ((AAA_EXEC_TYPE_NONE >= exec_type) || (AAA_EXEC_TYPE_SUPPORT_NUMBER  < exec_type))
    {
        AAA_OM_TRACE("Bad exec_type(%d)", exec_type);
        return NULL;
    }

    return &author_command_entry[priv_lvl][exec_type - 1]; /* to zero-based */
}
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningRadiusGroupEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetNextRunningRadiusGroupEntryInterface(entry);
}


 /*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntry_Ex(AAA_RadiusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetRadiusGroupEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusGroupEntryInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetNextRadiusGroupEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based)
 * OUTPUT   : radius_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntry_Ex(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    return AAA_OM_GetRadiusEntryInterface(group_index, entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningTacacsPlusGroupEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetNextRunningTacacsPlusGroupEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntry_Ex(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetTacacsPlusGroupEntryInterface(entry);
}

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccExecEntry(AAA_AccExecEntry_T *entry)
{
    AAA_AccExecEntry_T     *exec_entry;

    AAA_OM_ENTER_CRITICAL_SECTION();

    if (NULL == entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    exec_entry = AAA_OM_GetAccExecEntry(entry->exec_type);
    if (NULL == exec_entry)
    {
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_FAIL);
    }

    if (AAA_MANUAL_CONFIGURE == exec_entry->configure_mode)
    {
        memcpy(entry, exec_entry, sizeof(AAA_AccExecEntry_T));
        AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_SUCCESS);
    }

    AAA_OM_RETURN_AND_LEAVE_CRITICAL_SECTION(SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccExecEntry_Ex(AAA_AccExecEntry_T *entry)
{
    return AAA_OM_GetAccExecEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccDot1xEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccDot1xEntry_Ex(AAA_AccDot1xEntry_T *entry)
{
    return AAA_OM_GetAccDot1xEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAccListEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccListEntry(AAA_AccListEntryInterface_T *entry)
{
    return AAA_OM_GetNextRunningAccListEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntry(AAA_AccListEntryInterface_T *entry)
{
    return AAA_OM_GetNextAccListEntryInterface(entry);
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)

    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->list_index = method_index;
    return AAA_OM_GetAccListEntryInterfaceByIndex(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */

}


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextMethodTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get the next AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetNextMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->list_index = method_index;
    return AAA_OM_GetNextAccListEntry(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}
#endif

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetUpdate
 * ------------------------------------------------------------------------
 * PURPOSE  :   Setting update interval
 * INPUT    :   update_interval
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetUpdate(UI32_T *update_interval)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    return AAA_OM_GetAccUpdateInterval(update_interval);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_OM_GetAccountTable entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->ifindex = ifindex;
    return AAA_OM_GetAccDot1xEntry_Ex(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextAccountTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetNextAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry)
{
#if (SYS_CPNT_ACCOUNTING == TRUE)
    /* using MGR APIs forms new SNMP API
     */
    if (NULL == entry)
        return FALSE;

    entry->ifindex = ifindex;
    return AAA_OM_GetNextAccDot1xEntry(entry);
#else
    return FALSE;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
}
#endif

#if (SYS_CPNT_AUTHORIZATION == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorExecEntry_Ex(AAA_AuthorExecEntry_T *entry)
{
    return AAA_OM_GetAuthorExecEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningAuthorListEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorListEntry(AAA_AuthorListEntryInterface_T *entry)
{
    return AAA_OM_GetNextRunningAuthorListEntryInterface(entry);
}

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry)
{
    return AAA_OM_GetNextTacacsPlusGroupEntryInterface(entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningRadiusEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    return ((AAA_OM_GetNextRadiusEntry(group_index, entry) == TRUE) ?
        SYS_TYPE_GET_RUNNING_CFG_SUCCESS :
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index (1-based), entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry)
{
    return AAA_OM_GetNextRadiusEntryInterface(group_index, entry);
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRunningTacacsPlusEntry
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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    return ((AAA_OM_GetNextTacacsPlusEntry(group_index, entry) == TRUE) ?
        SYS_TYPE_GET_RUNNING_CFG_SUCCESS :
        SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE);
}


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index (1-based), entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    return AAA_OM_GetNextTacacsPlusEntryInterface(group_index, entry);
}

#if (SYS_CPNT_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : tacacs_server_index (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntry_Ex(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry)
{
    return AAA_OM_GetTacacsPlusEntryInterface(group_index, entry);
}
#endif

/*-------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_OM_HandleIPCReqMsg
 *-------------------------------------------------------------------------
 * PURPOSE:
 *    Handle the ipc request message for csca om.
 * INPUT:
 *    ipcmsg_p  --  input request ipc message buffer
 *
 * OUTPUT:
 *    ipcmsg_p  --  output response ipc message buffer
 *
 * RETURN:
 *    TRUE  --  There is a response need to send.
 *    FALSE --  No response need to send.
 *
 * NOTES:
 *    1.The size of ipcmsg_p.msg_buf must be large enough to carry any response
 *      messages.
 *-------------------------------------------------------------------------
 */
BOOL_T AAA_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    AAA_OM_IPCMsg_T *msg_data_p;
    UI8_T tmep_arg_spac_p[sizeof(AAA_OM_IPCMsg_ALL_ARG_T)];
    AAA_OM_IPCMsg_ALL_ARG_T *tmep_arg_data_p=(AAA_OM_IPCMsg_ALL_ARG_T *)tmep_arg_spac_p;
    UI32_T cmd;

    if(ipcmsg_p==NULL)
        return FALSE;

    msg_data_p= (AAA_OM_IPCMsg_T*)ipcmsg_p->msg_buf;
    cmd = msg_data_p->type.cmd;

    switch(cmd)
    {
        case AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSGROUPENTRY:
        {

            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningradiusgroupentry.resp);
            tmep_arg_data_p->getnextrunningradiusgroupentry.entry.group_index
                =msg_data_p->data.getnextrunningradiusgroupentry.req.group_index;
            msg_data_p->type.result_ui32=
                AAA_OM_GetNextRunningRadiusGroupEntry(&tmep_arg_data_p->getnextrunningradiusgroupentry.entry);
            msg_data_p->data.getnextrunningradiusgroupentry.resp.entry
                =tmep_arg_data_p->getnextrunningradiusgroupentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETRADIUSGROUPENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getradiusgroupentry_ex.resp);
            memcpy(tmep_arg_data_p->getradiusgroupentry_ex.entry.group_name,
                msg_data_p->data.getradiusgroupentry_ex.req.group_name,
                sizeof(tmep_arg_data_p->getradiusgroupentry_ex.entry.group_name));
            msg_data_p->type.result_bool=
                AAA_OM_GetRadiusGroupEntry_Ex(&tmep_arg_data_p->getradiusgroupentry_ex.entry);
            msg_data_p->data.getradiusgroupentry_ex.resp.group_index
                =tmep_arg_data_p->getradiusgroupentry_ex.entry.group_index;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRADIUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextradiusgroupentry.resp);
            tmep_arg_data_p->getnextradiusgroupentry.entry.group_index
                =msg_data_p->data.getnextradiusgroupentry.req.group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextRadiusGroupEntry(&tmep_arg_data_p->getnextradiusgroupentry.entry);
            msg_data_p->data.getnextradiusgroupentry.resp.entry
                =tmep_arg_data_p->getnextradiusgroupentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningtacacsplusgroupentry.resp);
            tmep_arg_data_p->getnextrunningtacacsplusgroupentry.entry.group_index
                =msg_data_p->data.getnextrunningtacacsplusgroupentry.req.group_index;
            msg_data_p->type.result_ui32 =
                AAA_OM_GetNextRunningTacacsPlusGroupEntry(&tmep_arg_data_p->getnextrunningtacacsplusgroupentry.entry);
            msg_data_p->data.getnextrunningtacacsplusgroupentry.resp.entry
                =tmep_arg_data_p->getnextrunningtacacsplusgroupentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETTACACSPLUSGROUPENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.gettacacsplusgroupentry_ex.resp);
            memcpy(tmep_arg_data_p->gettacacsplusgroupentry_ex.entry.group_name,
                msg_data_p->data.gettacacsplusgroupentry_ex.req.group_name,
                sizeof(tmep_arg_data_p->gettacacsplusgroupentry_ex.entry.group_name));
            msg_data_p->type.result_bool=
                AAA_OM_GetTacacsPlusGroupEntry_Ex(&tmep_arg_data_p->gettacacsplusgroupentry_ex.entry);
            msg_data_p->data.gettacacsplusgroupentry_ex.resp.group_index
                =tmep_arg_data_p->gettacacsplusgroupentry_ex.entry.group_index;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnexttacacsplusgroupentry.resp);
            tmep_arg_data_p->getnexttacacsplusgroupentry.entry.group_index
                =msg_data_p->data.getnexttacacsplusgroupentry.req.group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextTacacsPlusGroupEntry(&tmep_arg_data_p->getnexttacacsplusgroupentry.entry);
            msg_data_p->data.getnexttacacsplusgroupentry.resp.entry
                =tmep_arg_data_p->getnexttacacsplusgroupentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningradiusentry.resp);
            tmep_arg_data_p->getnextrunningradiusentry.group_index
                =msg_data_p->data.getnextrunningradiusentry.req.group_index;
            tmep_arg_data_p->getnextrunningradiusentry.entry.radius_index
                =msg_data_p->data.getnextrunningradiusentry.req.radius_index;
            msg_data_p->type.result_ui32=
                AAA_OM_GetNextRunningRadiusEntry(tmep_arg_data_p->getnextrunningradiusentry.group_index,
                &tmep_arg_data_p->getnextrunningradiusentry.entry);
            msg_data_p->data.getnextrunningradiusentry.resp.entry
                =tmep_arg_data_p->getnextrunningradiusentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRADIUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextradiusentry.resp);
            tmep_arg_data_p->getnextradiusentry.group_index
                =msg_data_p->data.getnextradiusentry.req.group_index;
            tmep_arg_data_p->getnextradiusentry.entry.radius_index
                =msg_data_p->data.getnextradiusentry.req.radius_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextRadiusEntry(tmep_arg_data_p->getnextradiusentry.group_index,
                &tmep_arg_data_p->getnextradiusentry.entry);
            msg_data_p->data.getnextradiusentry.resp.entry
                =tmep_arg_data_p->getnextradiusentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETRADIUSENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getradiusentry_ex.resp);
            tmep_arg_data_p->getradiusentry_ex.group_index
                =msg_data_p->data.getradiusentry_ex.req.group_index;
            tmep_arg_data_p->getradiusentry_ex.entry.radius_index
                =msg_data_p->data.getradiusentry_ex.req.radius_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetRadiusEntry_Ex(tmep_arg_data_p->getradiusentry_ex.group_index,
                &tmep_arg_data_p->getradiusentry_ex.entry);
            msg_data_p->data.getradiusentry_ex.resp.radius_server_index
                =tmep_arg_data_p->getradiusentry_ex.entry.radius_server_index;
        }
            break;

        case AAA_OM_IPCCMD_GETRADIUSENTRYORDER:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getradiusentryorder.resp);
            tmep_arg_data_p->getradiusentryorder.group_index
                =msg_data_p->data.getradiusentryorder.req.group_index;
            tmep_arg_data_p->getradiusentryorder.radius_index
                =msg_data_p->data.getradiusentryorder.req.radius_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetRadiusEntryOrder(tmep_arg_data_p->getradiusentryorder.group_index,
                tmep_arg_data_p->getradiusentryorder.radius_index,
                &tmep_arg_data_p->getradiusentryorder.order);
            msg_data_p->data.getradiusentryorder.resp.order
                =tmep_arg_data_p->getradiusentryorder.order;
        }
            break;
#if (SYS_CPNT_ACCOUNTING == TRUE)

        case AAA_OM_IPCCMD_GETRUNNINGACCUPDATEINTERVAL:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getrunningaccupdateinterval.resp);
            msg_data_p->type.result_ui32 =
                AAA_OM_GetRunningAccUpdateInterval(
                    &msg_data_p->data.getrunningaccupdateinterval.resp.update_interval);
        }
            break;

        case AAA_OM_IPCCMD_ISRADIUSGROUPVALID:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.isradiusgroupvalid.resp);
            msg_data_p->type.result_bool=
                AAA_OM_IsRadiusGroupValid(msg_data_p->data.isradiusgroupvalid.req.group_index,
                msg_data_p->data.isradiusgroupvalid.req.client_type,
                msg_data_p->data.isradiusgroupvalid.req.ifindex);

        }
            break;

        case AAA_OM_IPCCMD_ISRADIUSENTRYVALID:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.isradiusentryvalid.resp);
            msg_data_p->type.result_bool=
                AAA_OM_IsRadiusEntryValid(msg_data_p->data.isradiusentryvalid.req.radius_index,
                msg_data_p->data.isradiusentryvalid.req.client_type,
                msg_data_p->data.isradiusentryvalid.req.ifindex);
        }
            break;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
        case AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningtacacsplusentry.resp);
            tmep_arg_data_p->getnextrunningtacacsplusentry.group_index
                =msg_data_p->data.getnextrunningtacacsplusentry.req.group_index;
            tmep_arg_data_p->getnextrunningtacacsplusentry.entry.tacacs_index
                =msg_data_p->data.getnextrunningtacacsplusentry.req.tacacs_index;
            msg_data_p->type.result_ui32=
                AAA_OM_GetNextRunningTacacsPlusEntry(tmep_arg_data_p->getnextrunningtacacsplusentry.group_index,
                &tmep_arg_data_p->getnextrunningtacacsplusentry.entry);
            msg_data_p->data.getnextrunningtacacsplusentry.resp.entry
                =tmep_arg_data_p->getnextrunningtacacsplusentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTTACACSPLUSENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnexttacacsplusentry.resp);
            tmep_arg_data_p->getnexttacacsplusentry.group_index
                =msg_data_p->data.getnexttacacsplusentry.req.group_index;
            tmep_arg_data_p->getnexttacacsplusentry.entry.tacacs_index
                =msg_data_p->data.getnexttacacsplusentry.req.tacacs_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextTacacsPlusEntry(tmep_arg_data_p->getnexttacacsplusentry.group_index,
                &tmep_arg_data_p->getnexttacacsplusentry.entry);
            msg_data_p->data.getnexttacacsplusentry.resp.entry
                =tmep_arg_data_p->getnexttacacsplusentry.entry;
        }
            break;

#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_OM_IPCCMD_GETTACACSPLUSENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.gettacacsplusentry_ex.resp);
            tmep_arg_data_p->gettacacsplusentry_ex.group_index
                =msg_data_p->data.gettacacsplusentry_ex.req.group_index;
            tmep_arg_data_p->gettacacsplusentry_ex.entry.tacacs_index
                =msg_data_p->data.gettacacsplusentry_ex.req.tacacs_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetTacacsPlusEntry_Ex(tmep_arg_data_p->gettacacsplusentry_ex.group_index,
                &tmep_arg_data_p->gettacacsplusentry_ex.entry);
            msg_data_p->data.gettacacsplusentry_ex.resp.tacacs_server_index
                =tmep_arg_data_p->gettacacsplusentry_ex.entry.tacacs_server_index;
        }
            break;

        case AAA_OM_IPCCMD_GETACCUPDATEINTERVAL:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getaccupdateinterval.resp);
            msg_data_p->type.result_bool=
                AAA_OM_GetAccUpdateInterval(&msg_data_p->data.getaccupdateinterval.resp.update_interval);
        }
            break;

        case AAA_OM_IPCCMD_GETRUNNINGACCEXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getrunningaccexecentry.resp);
            tmep_arg_data_p->getrunningaccexecentry.entry.exec_type
                =msg_data_p->data.getrunningaccexecentry.req.exec_type;
            msg_data_p->type.result_ui32=
                AAA_OM_GetRunningAccExecEntry(&tmep_arg_data_p->getrunningaccexecentry.entry);
            memcpy(msg_data_p->data.getrunningaccexecentry.resp.list_name,
                tmep_arg_data_p->getrunningaccexecentry.entry.list_name,
                sizeof(msg_data_p->data.getrunningaccexecentry.resp.list_name));
            msg_data_p->data.getrunningaccexecentry.resp.configure_mode=
                tmep_arg_data_p->getrunningaccexecentry.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTACCEXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextaccexecentry.resp);
            tmep_arg_data_p->getnextaccexecentry.entry.exec_type
                =msg_data_p->data.getnextaccexecentry.req.exec_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAccExecEntry(&tmep_arg_data_p->getnextaccexecentry.entry);
            msg_data_p->data.getnextaccexecentry.resp.entry
                =tmep_arg_data_p->getnextaccexecentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETACCEXECENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getaccexecentry_ex.resp);
            tmep_arg_data_p->getaccexecentry_ex.entry.exec_type
                =msg_data_p->data.getaccexecentry_ex.req.exec_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetAccExecEntry_Ex(&tmep_arg_data_p->getaccexecentry_ex.entry);
            memcpy(msg_data_p->data.getaccexecentry_ex.resp.list_name,
                tmep_arg_data_p->getaccexecentry_ex.entry.list_name,
                sizeof(msg_data_p->data.getaccexecentry_ex.resp.list_name));
            msg_data_p->data.getaccexecentry_ex.resp.configure_mode=
                tmep_arg_data_p->getaccexecentry_ex.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETRUNNINGACCDOT1XENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getrunningaccdot1xentry.resp);
            tmep_arg_data_p->getrunningaccdot1xentry.entry.ifindex
                =msg_data_p->data.getrunningaccdot1xentry.req.ifindex;
            msg_data_p->type.result_ui32=
                AAA_OM_GetRunningAccDot1xEntry(&tmep_arg_data_p->getrunningaccdot1xentry.entry);
            memcpy(msg_data_p->data.getrunningaccdot1xentry.resp.list_name,
                tmep_arg_data_p->getrunningaccdot1xentry.entry.list_name,
                sizeof(msg_data_p->data.getrunningaccdot1xentry.resp.list_name));
            msg_data_p->data.getrunningaccdot1xentry.resp.configure_mode=
                tmep_arg_data_p->getrunningaccdot1xentry.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETACCDOT1XENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getaccdot1xentry_ex.resp);
            tmep_arg_data_p->getaccdot1xentry_ex.entry.ifindex
                =msg_data_p->data.getaccdot1xentry_ex.req.ifindex;
            msg_data_p->type.result_bool=
                AAA_OM_GetAccDot1xEntry_Ex(&tmep_arg_data_p->getaccdot1xentry_ex.entry);
            memcpy(msg_data_p->data.getaccdot1xentry_ex.resp.list_name,
                tmep_arg_data_p->getaccdot1xentry_ex.entry.list_name,
                sizeof(msg_data_p->data.getaccdot1xentry_ex.resp.list_name));
            msg_data_p->data.getaccdot1xentry_ex.resp.configure_mode=
                tmep_arg_data_p->getaccdot1xentry_ex.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRUNNINGACCLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningacclistentry.resp);
            tmep_arg_data_p->getnextrunningacclistentry.entry.list_index
                =msg_data_p->data.getnextrunningacclistentry.req.list_index;
            msg_data_p->type.result_ui32=
                AAA_OM_GetNextRunningAccListEntry(&tmep_arg_data_p->getnextrunningacclistentry.entry);
            msg_data_p->data.getnextrunningacclistentry.resp.entry
                =tmep_arg_data_p->getnextrunningacclistentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTACCLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextacclistentry.resp);
            tmep_arg_data_p->getnextacclistentry.entry.list_index
                =msg_data_p->data.getnextacclistentry.req.list_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAccListEntry(&tmep_arg_data_p->getnextacclistentry.entry);
            msg_data_p->data.getnextacclistentry.resp.entry
                =tmep_arg_data_p->getnextacclistentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTACCLISTENTRYFILTERBYCLIENTTYPE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextacclistentryfilterbyclienttype.resp);
            tmep_arg_data_p->getnextacclistentryfilterbyclienttype.entry.list_index
                =msg_data_p->data.getnextacclistentryfilterbyclienttype.req.list_index;
            tmep_arg_data_p->getnextacclistentryfilterbyclienttype.entry.client_type
                =msg_data_p->data.getnextacclistentryfilterbyclienttype.req.client_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAccListEntryFilterByClientType(&tmep_arg_data_p->getnextacclistentryfilterbyclienttype.entry);
            msg_data_p->data.getnextacclistentryfilterbyclienttype.resp.entry
                =tmep_arg_data_p->getnextacclistentryfilterbyclienttype.entry;
        }
            break;

        case AAA_OM_IPCCMD_QUERYACCDOT1XPORTLIST:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.queryaccdot1xportlist.resp);
            tmep_arg_data_p->queryaccdot1xportlist.entry.list_index
                =msg_data_p->data.queryaccdot1xportlist.req.list_index;
            msg_data_p->type.result_bool=
                AAA_OM_QueryAccDot1xPortList(&tmep_arg_data_p->queryaccdot1xportlist.entry);
            memcpy(msg_data_p->data.queryaccdot1xportlist.resp.port_list,
                tmep_arg_data_p->queryaccdot1xportlist.entry.port_list,
                sizeof(msg_data_p->data.queryaccdot1xportlist.resp.port_list));
        }
            break;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        case AAA_OM_IPCCMD_GETACCCOMMANDENTRYINTERFACE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getacccommandentryinf.resp);
            msg_data_p->type.result_bool=
                AAA_OM_GetAccCommandEntryInterface(&msg_data_p->data.getacccommandentryinf.req.entry);
        }
            break;
        case AAA_OM_IPCCMD_GETNEXTACCCOMMANDENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextacccommandentry.resp);
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAccCommandEntry(
                    &msg_data_p->data.getnextacccommandentry.req.index,
                    &msg_data_p->data.getnextacccommandentry.req.entry);
        }
            break;
        case AAA_OM_IPCCMD_GETRUNNINGACCCOMMANDENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getacccommandentryinf.resp);
            msg_data_p->type.result_bool=
                AAA_OM_GetRunningAccCommandEntry(&msg_data_p->data.getacccommandentryinf.req.entry);
        }
            break;
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */

#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_OM_IPCCMD_GETMETHODTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getmethodtable.resp);
            tmep_arg_data_p->getmethodtable.method_index
                =msg_data_p->data.getmethodtable.req.method_index;
            tmep_arg_data_p->getmethodtable.entry.list_index
                =msg_data_p->data.getmethodtable.req.list_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetMethodTable(tmep_arg_data_p->getmethodtable.method_index,
                &tmep_arg_data_p->getmethodtable.entry);
            memcpy(msg_data_p->data.getmethodtable.resp.list_name,
                tmep_arg_data_p->getmethodtable.entry.list_name,
                sizeof(msg_data_p->data.getmethodtable.resp.list_name));
            memcpy(msg_data_p->data.getmethodtable.resp.group_name,
                tmep_arg_data_p->getmethodtable.entry.group_name,
                sizeof(msg_data_p->data.getmethodtable.resp.group_name));
            msg_data_p->data.getmethodtable.resp.client_type
                =tmep_arg_data_p->getmethodtable.entry.client_type;
            msg_data_p->data.getmethodtable.resp.group_type
                =tmep_arg_data_p->getmethodtable.entry.group_type;
            msg_data_p->data.getmethodtable.resp.working_mode
                =tmep_arg_data_p->getmethodtable.entry.working_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTMETHODTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextmethodtable.resp);
            tmep_arg_data_p->getnextmethodtable.method_index
                =msg_data_p->data.getnextmethodtable.req.method_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextMethodTable(tmep_arg_data_p->getnextmethodtable.method_index,
                &tmep_arg_data_p->getnextmethodtable.entry);
            msg_data_p->data.getnextmethodtable.resp.entry
                =tmep_arg_data_p->getnextmethodtable.entry;
        }
            break;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
        case AAA_OM_IPCCMD_GETRADIUSGROUPTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getradiusgrouptable.resp);
            tmep_arg_data_p->getradiusgrouptable.radius_group_index
                =msg_data_p->data.getradiusgrouptable.req.radius_group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetRadiusGroupTable(tmep_arg_data_p->getradiusgrouptable.radius_group_index,
                &tmep_arg_data_p->getradiusgrouptable.entry);
            msg_data_p->data.getradiusgrouptable.resp.entry
                =tmep_arg_data_p->getradiusgrouptable.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRADIUSGROUPTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextradiusgrouptable.resp);
            tmep_arg_data_p->getnextradiusgrouptable.radius_group_index
                =msg_data_p->data.getnextradiusgrouptable.req.radius_group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextRadiusGroupTable(tmep_arg_data_p->getnextradiusgrouptable.radius_group_index,
                &tmep_arg_data_p->getnextradiusgrouptable.entry);
            msg_data_p->data.getnextradiusgrouptable.resp.entry
                =tmep_arg_data_p->getnextradiusgrouptable.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETTACACSPLUSGROUPTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.gettacacsplusgrouptable.resp);
            tmep_arg_data_p->gettacacsplusgrouptable.tacacsplus_group_index
                =msg_data_p->data.gettacacsplusgrouptable.req.tacacsplus_group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetTacacsPlusGroupTable(tmep_arg_data_p->gettacacsplusgrouptable.tacacsplus_group_index,
                &tmep_arg_data_p->gettacacsplusgrouptable.entry);
            msg_data_p->data.gettacacsplusgrouptable.resp.entry
                =tmep_arg_data_p->gettacacsplusgrouptable.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnexttacacsplusgrouptable.resp);
            tmep_arg_data_p->getnexttacacsplusgrouptable.tacacsplus_group_index
                =msg_data_p->data.getnexttacacsplusgrouptable.req.tacacsplus_group_index;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextTacacsPlusGroupTable(tmep_arg_data_p->getnexttacacsplusgrouptable.tacacsplus_group_index,
                &tmep_arg_data_p->getnexttacacsplusgrouptable.entry);
            msg_data_p->data.getnexttacacsplusgrouptable.resp.entry
                =tmep_arg_data_p->getnexttacacsplusgrouptable.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETUPDATE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getupdate.resp);
            msg_data_p->type.result_bool=
                AAA_OM_GetUpdate(&msg_data_p->data.getupdate.resp.update_interval);
        }
            break;
#if (SYS_CPNT_ACCOUNTING == TRUE)
        case AAA_OM_IPCCMD_GETACCOUNTTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getaccounttable.resp);
            tmep_arg_data_p->getaccounttable.entry.ifindex
                =msg_data_p->data.getaccounttable.req.ifindex;
            msg_data_p->type.result_bool=
                AAA_OM_GetAccountTable(tmep_arg_data_p->getaccounttable.entry.ifindex,
                &tmep_arg_data_p->getaccounttable.entry);
            msg_data_p->data.getaccounttable.resp.entry
                =tmep_arg_data_p->getaccounttable.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTACCOUNTTABLE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextaccounttable.resp);
            tmep_arg_data_p->getnextaccounttable.ifindex
                =msg_data_p->data.getnextaccounttable.req.ifindex;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAccountTable(tmep_arg_data_p->getnextaccounttable.ifindex,
                &tmep_arg_data_p->getnextaccounttable.entry);
            msg_data_p->data.getnextaccounttable.resp.entry
                =tmep_arg_data_p->getnextaccounttable.entry;
        }
            break;
#endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


#if (AAA_SUPPORT_ACCTON_BACKDOOR == TRUE)
#if (SYS_CPNT_ACCOUNTING == TRUE)

        case AAA_OM_IPCCMD_BACKDOOR_SHOWACCUSER:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.backdoor_showaccuser.resp);
            AAA_OM_Backdoor_ShowAccUser();
        }
            break;
#endif /* SYS_CPNT_ACCOUNTING == TRUE */
#endif /* AAA_SUPPORT_ACCTON_BACKDOOR == TRUE */
#if (SYS_CPNT_AUTHORIZATION == TRUE)

        case AAA_OM_IPCCMD_GETNEXTAUTHORLISTENTRYFILTERBYCLIENTTYPE:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextauthorlistentryfilterbyclienttype.resp);
            tmep_arg_data_p->getnextauthorlistentryfilterbyclienttype.entry.list_index
                =msg_data_p->data.getnextauthorlistentryfilterbyclienttype.req.list_index;
            tmep_arg_data_p->getnextauthorlistentryfilterbyclienttype.entry.list_type
                =msg_data_p->data.getnextauthorlistentryfilterbyclienttype.req.list_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAuthorListEntryFilterByClientType(&tmep_arg_data_p->getnextauthorlistentryfilterbyclienttype.entry);
            msg_data_p->data.getnextauthorlistentryfilterbyclienttype.resp.entry
                =tmep_arg_data_p->getnextauthorlistentryfilterbyclienttype.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTAUTHOREXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextauthorexecentry.resp);
            tmep_arg_data_p->getnextauthorexecentry.entry.exec_type
                =msg_data_p->data.getnextauthorexecentry.req.exec_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetNextAuthorExecEntry(&tmep_arg_data_p->getnextauthorexecentry.entry);
            msg_data_p->data.getnextauthorexecentry.resp.entry
                =tmep_arg_data_p->getnextauthorexecentry.entry;
        }
            break;

        case AAA_OM_IPCCMD_GETAUTHOREXECENTRY_EX:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getauthorexecentry_ex.resp);
            tmep_arg_data_p->getauthorexecentry_ex.entry.exec_type
                =msg_data_p->data.getauthorexecentry_ex.req.exec_type;
            msg_data_p->type.result_bool=
                AAA_OM_GetAuthorExecEntry_Ex(&tmep_arg_data_p->getauthorexecentry_ex.entry);
            memcpy(msg_data_p->data.getauthorexecentry_ex.resp.list_name,
                tmep_arg_data_p->getauthorexecentry_ex.entry.list_name,
                sizeof(msg_data_p->data.getauthorexecentry_ex.resp.list_name));
            msg_data_p->data.getauthorexecentry_ex.resp.configure_mode=
                tmep_arg_data_p->getauthorexecentry_ex.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETRUNNINGAUTHOREXECENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getrunningauthorexecentry.resp);
            tmep_arg_data_p->getrunningauthorexecentry.entry.exec_type
                =msg_data_p->data.getrunningauthorexecentry.req.exec_type;
            msg_data_p->type.result_ui32=
                AAA_OM_GetRunningAuthorExecEntry(&tmep_arg_data_p->getrunningauthorexecentry.entry);
            memcpy(msg_data_p->data.getrunningauthorexecentry.resp.list_name,
                tmep_arg_data_p->getrunningauthorexecentry.entry.list_name,
                sizeof(msg_data_p->data.getrunningauthorexecentry.resp.list_name));
            msg_data_p->data.getrunningauthorexecentry.resp.configure_mode=
                tmep_arg_data_p->getrunningauthorexecentry.entry.configure_mode;
        }
            break;

        case AAA_OM_IPCCMD_GETNEXTRUNNINGAUTHORLISTENTRY:
        {
            ipcmsg_p->msg_size=AAA_OM_SIZEOF(msg_data_p->data.getnextrunningauthorlistentry.resp);
            tmep_arg_data_p->getnextrunningauthorlistentry.entry.list_index
                =msg_data_p->data.getnextrunningauthorlistentry.req.list_index;
            msg_data_p->type.result_ui32=
                AAA_OM_GetNextRunningAuthorListEntry(&tmep_arg_data_p->getnextrunningauthorlistentry.entry);
            msg_data_p->data.getnextrunningauthorlistentry.resp.entry
                =tmep_arg_data_p->getnextrunningauthorlistentry.entry;
        }
            break;
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        case AAA_OM_IPCCMD_GET_AUTHOR_COMMAND_ENTRY:
        {
            ipcmsg_p->msg_size = AAA_OM_SIZEOF(msg_data_p->data.get_author_command_entry.resp);

            tmep_arg_data_p->get_author_command_entry.priv_lvl = msg_data_p->data.get_author_command_entry.req.priv_lvl;
            tmep_arg_data_p->get_author_command_entry.exec_type = msg_data_p->data.get_author_command_entry.req.exec_type;

            msg_data_p->type.result_bool = AAA_OM_GetAuthorCommandEntryIPC(
                                                tmep_arg_data_p->get_author_command_entry.priv_lvl,
                                                tmep_arg_data_p->get_author_command_entry.exec_type,
                                                &msg_data_p->data.get_author_command_entry.resp.entry);
            break;
        }

        case AAA_OM_IPCCMD_GET_RUNNING_AUTHOR_COMMAND_ENTRY:
        {
            ipcmsg_p->msg_size = AAA_OM_SIZEOF(msg_data_p->data.get_author_command_entry.resp);

            tmep_arg_data_p->get_author_command_entry.priv_lvl = msg_data_p->data.get_author_command_entry.req.priv_lvl;
            tmep_arg_data_p->get_author_command_entry.exec_type = msg_data_p->data.get_author_command_entry.req.exec_type;

            msg_data_p->type.result_ui32 = AAA_OM_GetRunningAuthorCommandEntry(
                                                tmep_arg_data_p->get_author_command_entry.priv_lvl,
                                                tmep_arg_data_p->get_author_command_entry.exec_type,
                                                &msg_data_p->data.get_author_command_entry.resp.entry);

            break;
        }
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        default:
            msg_data_p->type.result_ui32=0;
            SYSFUN_Debug_Printf("%s(): Invalid cmd.\n", __FUNCTION__);
            return TRUE;

    }

    /*Check sychronism or asychronism ipc. If it is sychronism(need to respond)then we return true.
     */
    if(cmd<AAA_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#endif /* SYS_CPNT_AAA == TRUE */

