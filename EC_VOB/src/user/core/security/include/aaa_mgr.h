/* MODULE NAME: aaa_mgr.h
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
#ifndef AAA_MGR_H
#define AAA_MGR_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_cpnt.h"

#if (SYS_CPNT_AAA == TRUE)

#include "sys_adpt.h"
#include "aaa_def.h"
#include "security_backdoor.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define AAA_SUPPORT_ACCTON_BACKDOOR      (TRUE && SECURITY_SUPPORT_ACCTON_BACKDOOR) /* support backdoor functions */

#define AAA_MGR_MSGBUF_TYPE_SIZE     sizeof(((AAA_MGR_IPCMsg_T *)0)->type)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

enum
{
    AAA_MGR_IPCCMD_SETRADIUSGROUPENTRY,
    AAA_MGR_IPCCMD_DESTROYRADIUSGROUPENTRY,
    AAA_MGR_IPCCMD_SETTACACSPLUSGROUPENTRY,
    AAA_MGR_IPCCMD_DESTROYTACACSPLUSGROUPENTRY,
    AAA_MGR_IPCCMD_SETRADIUSENTRY,
    AAA_MGR_IPCCMD_SETRADIUSENTRYJOINDEFAULTRADIUSGROUP,
    AAA_MGR_IPCCMD_SETRADIUSENTRYDEPARTDEFAULTRADIUSGROUP,
    AAA_MGR_IPCCMD_SETRADIUSENTRYBYIPADDRESS,
    AAA_MGR_IPCCMD_DESTROYRADIUSENTRY,
    AAA_MGR_IPCCMD_DESTROYRADIUSENTRYBYIPADDRESS,
    AAA_MGR_IPCCMD_SETTACACSPLUSENTRY,
    AAA_MGR_IPCCMD_SETTACACSPLUSENTRYJOINDEFAULTTACACSPLUSGROUP,
    AAA_MGR_IPCCMD_SETTACACSPLUSENTRYDEPARTDEFAULTTACACSPLUSGROUP,
    AAA_MGR_IPCCMD_SETTACACSPLUSENTRYBYIPADDRESS,
    AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRY,
    AAA_MGR_IPCCMD_DESTROYTACACSPLUSENTRYBYIPADDRESS,
    AAA_MGR_IPCCMD_SETACCUPDATEINTERVAL,
    AAA_MGR_IPCCMD_DISABLEACCUPDATEINTERVAL,
    AAA_MGR_IPCCMD_SETACCDOT1XENTRY,
    AAA_MGR_IPCCMD_DISABLEACCDOT1XENTRY,
    AAA_MGR_IPCCMD_SETACCEXECENTRY,
    AAA_MGR_IPCCMD_DISABLEACCEXECENTRY,
    AAA_MGR_IPCCMD_SETACCLISTENTRY,
    AAA_MGR_IPCCMD_DESTROYACCLISTENTRY,
    AAA_MGR_IPCCMD_DESTROYACCLISTENTRY2,
    AAA_MGR_IPCCMD_SETDEFAULTLIST,
    AAA_MGR_IPCCMD_SETMETHODNAME,
    AAA_MGR_IPCCMD_SETMETHODGROUPNAME,
    AAA_MGR_IPCCMD_SETMETHODCLIENTTYPE,
    AAA_MGR_IPCCMD_SETMETHODMODE,
    AAA_MGR_IPCCMD_SETMETHODSTATUS,
    AAA_MGR_IPCCMD_SETRADIUSGROUPNAME,
    AAA_MGR_IPCCMD_SETRADIUSSERVERGROUPBITMAP,
    AAA_MGR_IPCCMD_SETRADIUSGROUPSTATUS,
    AAA_MGR_IPCCMD_SETTACACSPLUSGROUPNAME,
    AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSERVERBITMAP,
    AAA_MGR_IPCCMD_SETTACACSPLUSGROUPSTATUS,
    AAA_MGR_IPCCMD_SETUPDATE,
    AAA_MGR_IPCCMD_SETACCOUNTMETHODNAME,
    AAA_MGR_IPCCMD_SETACCOUNTSTATUS,
    AAA_MGR_IPCCMD_GETACCUSERENTRYQTY,
    AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYNAMEANDTYPE,
    AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYTYPE,
    AAA_MGR_IPCCMD_GETACCUSERENTRYQTYFILTERBYPORT,
    AAA_MGR_IPCCMD_GETNEXTACCUSERENTRY,
    AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYNAMEANDTYPE,
    AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYTYPE,
    AAA_MGR_IPCCMD_GETNEXTACCUSERENTRYFILTERBYPORT,
    AAA_MGR_IPCCMD_AUTHORREQUEST,
    AAA_MGR_IPCCMD_SETAUTHOREXECENTRY,
    AAA_MGR_IPCCMD_DISABLEAUTHOREXECENTRY,
    AAA_MGR_IPCCMD_SET_AUTHOR_COMMAND_ENTRY,
    AAA_MGR_IPCCMD_DISABLE_AUTHOR_COMMAND_ENTRY,
    AAA_MGR_IPCCMD_SETAUTHORDEFAULTLIST,
    AAA_MGR_IPCCMD_SETAUTHORLISTENTRY,
    AAA_MGR_IPCCMD_DESTROYAUTHORLISTENTRY,
    AAA_MGR_IPCCMD_SETACCCOMMANDENTRY,
    AAA_MGR_IPCCMD_DISABLEACCCOMMANDENTRY,
    AAA_MGR_IPCCMD_FOLLOWISASYNCHRONISMIPC,
    AAA_MGR_IPCCMD_ASYNC_ASYNCACCOUNTINGREQUEST
};

#if (SYS_CPNT_ACCOUNTING == TRUE)

typedef enum AAA_AccCpntType_E
{
    AAA_ACC_CPNT_RADIUS,
    AAA_ACC_CPNT_TACACS_PLUS,

    AAA_ACC_CPNT_SUPPORT_NUMBER, /* to calculate the number of accounting components automatically */
} AAA_AccCpntType_T;


typedef BOOL_T (*AAA_AccCpntAsyncNotify_Callback_T)(const AAA_AccRequest_T *request); /* callback to the accounting component like radius or tacacs+ */

#endif /* SYS_CPNT_ACCOUNTING == TRUE */


typedef struct
{
    union
    {
        UI32_T cmd;          /*cmd fnction id*/
        BOOL_T result_bool;  /*respond bool return*/
        UI32_T result_ui32;  /*respond ui32 return*/
        UI32_T result_i32;  /*respond i32 return*/
    }type;

    union
    {
        union
        {
            struct
            {
                AAA_RadiusGroupEntryInterface_T entry;
            }req;

            struct
            {
            }resp;
        }setradiusgroupentry;

        union
        {
            struct
            {
                char  name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
            }req;

            struct
            {
                AAA_WarningInfo_T warning;
            }resp;
        }destroyradiusgroupentry;

        union
        {
            struct
            {
                AAA_TacacsPlusGroupEntryInterface_T entry;
            }req;

            struct
            {
            }resp;
        }settacacsplusgroupentry;


        union
        {
            struct
            {
                char  name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
            }req;

            struct
            {
                AAA_WarningInfo_T warning;
            }resp;
        }destroytacacsplusgroupentry;

        union
        {
            struct
            {
                UI16_T group_index;
                AAA_RadiusEntryInterface_T entry;
            }req;

            struct
            {
                AAA_WarningInfo_T warning;
            }resp;
        }setradiusentry;

        union
        {
            struct
            {
                UI32_T radius_server_index;
            }req;

            struct
            {
            }resp;
        }setradiusentryjoindefaultradiusgroup;

        union
        {
            struct
            {
                UI32_T radius_server_index;
            }req;

            struct
            {
            }resp;
        }setradiusentrydepartdefaultradiusgroup;

        union
        {
            struct
            {
                UI16_T group_index;
                UI32_T ip_address;
            }req;

            struct
            {
            }resp;
        }setradiusentrybyipaddress;

        union
        {
            struct
            {
                UI16_T group_index;
                AAA_RadiusEntryInterface_T entry;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroyradiusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI32_T ip_address;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroyradiusentrybyipaddress;

        union
        {
            struct
            {
                UI16_T group_index;
                AAA_TacacsPlusEntryInterface_T entry;
            }req;

            struct
            {
                AAA_WarningInfo_T warning;
            }resp;
        }settacacsplusentry;

        union
        {
            struct
            {
                UI32_T tacacs_server_index;
            }req;

            struct
            {
            }resp;
        }settacacsplusentryjoindefaulttacacsplusgroup;

        union
        {
            struct
            {
                UI32_T tacacs_server_index;
            }req;

            struct
            {
            }resp;
        }settacacsplusentrydepartdefaulttacacsplusgroup;

        union
        {
            struct
            {
                UI16_T group_index;
                UI32_T ip_address;
            }req;

            struct
            {
            }resp;
        }settacacsplusentrybyipaddress;

        union
        {
            struct
            {
                UI16_T group_index;
                AAA_TacacsPlusEntryInterface_T entry;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroytacacsplusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI32_T ip_address;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroytacacsplusentrybyipaddress;

        union
        {
            struct
            {
                UI32_T update_interval;
            }req;

            struct
            {
            }resp;
        }setaccupdateinterval;

        union
        {
            struct
            {
            }req;

            struct
            {
            }resp;
        }disableaccupdateinterval;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        union
        {
            struct
            {
                AAA_AccDot1xEntry_T entry;
            }req;

            struct
            {
            }resp;
        }setaccdot1xentry;

        union
        {
            struct
            {
                UI32_T ifindex;
            }req;

            struct
            {
            }resp;
        }disableaccdot1xentry;

        union
        {
            struct
            {
                AAA_AccExecEntry_T entry;
            }req;

            struct
            {
            }resp;
        }setaccexecentry;

        union
        {
            struct
            {
                AAA_ExecType_T exec_type;
            }req;

            struct
            {
            }resp;
        }disableaccexecentry;

        union
        {
            struct
            {
                AAA_AccListEntryInterface_T entry;
            }req;

            struct
            {
            }resp;
        }setacclistentry;

        union
        {
            struct
            {
                char  name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ClientType_T client_type;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroyacclistentry;

        union
        {
            struct
            {
                AAA_AccListEntryInterface_T entry;
                AAA_WarningInfo_T warning;
            }req;

            struct
            {
            }resp;
        }destroyacclistentry2;

        union
        {
            struct
            {
                AAA_AccListEntryInterface_T entry;
            }req;

            struct
            {
            }resp;
        }setdefaultlist;

        union
        {
            struct
            {
                AAA_AccCpntType_T cpnt_type;
                AAA_AccCpntAsyncNotify_Callback_T call_back_func;
            }req;

            struct
            {
            }resp;
        }register_acccomponent_callback;

        union
        {
            struct
            {
                AAA_AccRequest_T request;
            }req;

            struct
            {
            }resp;
        }asyncaccountingrequest;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        union
        {
            struct
            {
                AAA_AccCommandEntry_T cmd_entry;
            }req;

            struct
            {
            }resp;
        }setacccmdentry;

        union
        {
            struct
            {
                AAA_AccRequest_T request;
            }req;

            struct
            {
                UI32_T  ser_no;
            }resp;
        }asyncaccountingrequest_for_acccmd;
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */
#endif
        union
        {
            struct
            {
                UI16_T method_index;
                char  method_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1];
            }req;

            struct
            {
            }resp;
        }setmethodname;

        union
        {
            struct
            {
                UI16_T method_index;
                char  method_group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1];
            }req;

            struct
            {
            }resp;
        }setmethodgroupname;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        union
        {
            struct
            {
                UI16_T method_index;
                AAA_ClientType_T client_type;
            }req;

            struct
            {
            }resp;
        }setmethodclienttype;
#endif
        union
        {
            struct
            {
                UI16_T method_index;
                UI8_T method_mode;
            }req;

            struct
            {
            }resp;
        }setmethodmode;

        union
        {
            struct
            {
                UI16_T method_index;
                UI8_T method_status;
            }req;

            struct
            {
            }resp;
        }setmethodstatus;

        union
        {
            struct
            {
                UI16_T radius_group_index;
                char  radius_group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1];
            }req;

            struct
            {
            }resp;
        }setradiusgroupname;

        union
        {
            struct
            {
                UI16_T radius_group_index;
                UI32_T radius_group_status;
            }req;

            struct
            {
            }resp;
        }setradiusgroupstatus;

        union
        {
            struct
            {
                UI16_T tacacsplus_group_index;
                char  tacacsplus_group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME+1];
            }req;

            struct
            {
            }resp;
        }settacacsplusgroupname;

        union
        {
            struct
            {
                UI16_T tacacsplus_group_index;
                UI8_T tacacsplus_group_server_bitmap;
            }req;

            struct
            {
            }resp;
        }settacacsplusgroupserverbitmap;

        union
        {
            struct
            {
                UI16_T tacacsplus_group_index;
                UI32_T tacacsplus_group_status;
            }req;

            struct
            {
            }resp;
        }settacacsplusgroupstatus;

        union
        {
            struct
            {
                UI32_T update_interval;
            }req;

            struct
            {
            }resp;
        }setupdate;

        union
        {
            struct
            {
                UI32_T ifindex;
                char  account_method_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME+1];
            }req;

            struct
            {
            }resp;
        }setaccountmethodname;

        union
        {
            struct
            {
            }req;

            struct
            {
                UI32_T qty;
            }resp;
        }getaccuserentryqty;

        union
        {
            struct
            {
                char  name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
                AAA_ClientType_T client_type;

            }req;

            struct
            {
                UI32_T qty;
            }resp;
        }getaccuserentryqtyfilterbynameandtype;

        union
        {
            struct
            {
                AAA_ClientType_T client_type;
            }req;

            struct
            {
                UI32_T qty;
            }resp;
        }getaccuserentryqtyfilterbytype;

        union
        {
            struct
            {
                UI32_T ifindex;
            }req;

            struct
            {
                UI32_T qty;
            }resp;
        }getaccuserentryqtyfilterbyport;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        union
        {
            struct
            {
                UI16_T  user_index;
            }req;

            struct
            {
                AAA_AccUserInfoInterface_T entry;
            }resp;
        }getnextaccuserentry;

        union
        {
            struct
            {
                UI16_T  user_index;     /* array index + 1 */
                UI32_T  ifindex;        /* if client_type == AAA_CLIENT_TYPE_DOT1X, ifindex == l_port
                               if client_type == AAA_CLIENT_TYPE_EXEC, ifindex implies console or telnet's session id
                             */
                AAA_ClientType_T            client_type;
            }req;

            struct
            {
                AAA_AccUserInfoInterface_T entry;
            }resp;
        }getnextaccuserentryfilterbynameandtype;

        union
        {
            struct
            {
                UI16_T  user_index;     /* array index + 1 */
                AAA_ClientType_T            client_type;
            }req;

            struct
            {
                AAA_AccUserInfoInterface_T entry;
            }resp;
        }getnextaccuserentryfilterbytype;

        union
        {
            struct
            {
                UI16_T  user_index;     /* array index + 1 */
                UI32_T  ifindex;
            }req;

            struct
            {
                AAA_AccUserInfoInterface_T entry;
            }resp;
        }getnextaccuserentryfilterbyport;
#endif
#if (SYS_CPNT_AUTHORIZATION == TRUE)
        union
        {
            struct
            {
                AAA_AuthorRequest_T request;
                AAA_AuthorReply_T reply;
            }req;

            struct
            {
                AAA_AuthorReply_T reply;
            }resp;
        }authorrequest;

        union
        {
            struct
            {
                AAA_AuthorExecEntry_T entry;
            }req;

            struct
            {
            }resp;
        }setauthorexecentry;

        union
        {
            struct
            {
                AAA_AuthorListEntryInterface_T entry;
            }req;

            struct
            {
            }resp;
        }setauthorlistentry;
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        union
        {
            struct
            {
                AAA_AuthorCommandEntry_T entry;
            }req;

            struct
            {
            }resp;
        }set_author_command_entry;

        union
        {
            struct
            {
                AAA_ExecType_T exec_type;
                UI32_T priv_lvl;
            }req;

            struct
            {
            }resp;
        }disable_author_command_entry;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

        union
        {
            struct
            {
                AAA_ExecType_T exec_type;
            }req;

            struct
            {
            }resp;
        }disableauthorexecentry;

        union
        {
            struct
            {
                AAA_ListType_T list_type;
                char  group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
            }req;

            struct
            {
            }resp;
        }setauthordefaultlist;

        union
        {
            struct
            {
                char  list_name[SYS_ADPT_MAX_LEN_OF_AUTHORIZATION_LIST_NAME + 1];
                AAA_ListType_T list_type;
            }req;

            struct
            {
                AAA_WarningInfo_T warning;
            }resp;
        }destroyauthorlistentry;

        union
        {
            struct
            {
                UI16_T radius_grp_idx;
                UI8_T  radius_grp_svr_bmp;
            }req;

            struct
            {
            }resp;
        }setradiusgrpsvrbmp;

        union
        {
            struct
            {
                UI32_T ifindex;
                UI8_T  account_status;
            }req;

            struct
            {
            }resp;
        }setaccountstatus;
    }data;
}  AAA_MGR_IPCMsg_T;


typedef union
{
    struct
    {
        UI32_T qty;
    }getaccuserentryqty;

    struct
    {
        char  name[SYS_ADPT_MAX_USER_NAME_LEN + 1];
        AAA_ClientType_T client_type;
        UI32_T qty;
    }getaccuserentryqtyfilterbynameandtype;

    struct
    {
        AAA_ClientType_T client_type;
        UI32_T qty;
    }getaccuserentryqtyfilterbytype;

    struct
    {
        UI32_T ifindex;
        UI32_T qty;
    }getaccuserentryqtyfilterbyport;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
    struct
    {
        AAA_AccUserInfoInterface_T entry;
    }getnextaccuserentry;

    struct
    {
        AAA_AccUserInfoInterface_T entry;
    }getnextaccuserentryfilterbynameandtype;

    struct
    {
        AAA_AccUserInfoInterface_T entry;
    }getnextaccuserentryfilterbytype;

    struct
    {
        AAA_AccUserInfoInterface_T entry;
    }getnextaccuserentryfilterbyport;
#endif
}   AAA_MGR_IPCMsg_ALL_ARG_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

SYS_TYPE_Stacking_Mode_T AAA_MGR_CurrentOperationMode();

void AAA_MGR_EnterMasterMode();

void AAA_MGR_EnterSlaveMode();

void AAA_MGR_SetTransitionMode();

void AAA_MGR_EnterTransitionMode();

BOOL_T AAA_MGR_Initiate_System_Resources();

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusGroupEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by group_index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusGroupEntryByIndex(AAA_RadiusGroupEntryInterface_T *entry);


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
BOOL_T AAA_MGR_SetRadiusGroupEntry(const AAA_RadiusGroupEntryInterface_T* entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete radius group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyRadiusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlustGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to set the group by name.
 * INPUT    : group_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : group_index will be ignored.
 *            create or modify specified entry
 *            can't modify the default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupEntry(const AAA_TacacsPlusGroupEntryInterface_T* entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete tacacs plus group by name.
 * INPUT    : name (terminated with '\0')
 * OUTPUT   : wanring
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can't delete default "tacacs+" group
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyTacacsPlusGroupEntry(const char *name, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusGroupEntryModifiedTime
 *-------------------------------------------------------------------------
 * PURPOSE  : get the group's modified time by index.
 * INPUT    : group_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusGroupEntryModifiedTime(UI16_T group_index, UI32_T *modified_time);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetRadiusEntryByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th radius entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetRadiusEntryByOrder(UI16_T group_index, UI16_T order, AAA_RadiusEntryInterface_T *entry);

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
BOOL_T AAA_MGR_SetRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry join default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusEntryJoinDefaultRadiusGroup(UI32_T radius_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the radius entry depart default "radius" group
 * INPUT    : radius_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by radius_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusEntryDepartDefaultRadiusGroup(UI32_T radius_server_index);

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
BOOL_T AAA_MGR_SetRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address);

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
BOOL_T AAA_MGR_DestroyRadiusEntry(UI16_T group_index, const AAA_RadiusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

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
BOOL_T AAA_MGR_DestroyRadiusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning);




/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusEntryByOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the n-th tacacs plus entry by group_index, order
 * INPUT    : group_index (1-based), order (1-based)
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusEntryByOrder(UI16_T group_index, UI16_T order, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetTacacsPlusEntryOrder
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry's order in group
 * INPUT    : group_index (1-based), tacacs_index (1-based)
 * OUTPUT   : order (1-based)
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetTacacsPlusEntryOrder(UI16_T group_index, UI16_T tacacs_index, UI16_T *order);

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
BOOL_T AAA_MGR_SetTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry join default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusEntryJoinDefaultTacacsPlusGroup(UI32_T tacacs_server_index);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup
 *-------------------------------------------------------------------------
 * PURPOSE  : set the tacacs plus entry depart default "tacacs+" group
 * INPUT    : tacacs_server_index
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : call by tacacs_mgr
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusEntryDepartDefaultTacacsPlusGroup(UI32_T tacacs_server_index);

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
BOOL_T AAA_MGR_SetTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address);

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
BOOL_T AAA_MGR_DestroyTacacsPlusEntry(UI16_T group_index, const AAA_TacacsPlusEntryInterface_T *entry, AAA_WarningInfo_T *warning);

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
BOOL_T AAA_MGR_DestroyTacacsPlusEntryByIpAddress(UI16_T group_index, UI32_T ip_address, AAA_WarningInfo_T *warning);



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
BOOL_T AAA_MGR_IsTacacsPlusGroupValid(UI16_T group_index, AAA_ClientType_T client_type, UI32_T ifindex);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_IsTacacsPlusEntryValid
 *-------------------------------------------------------------------------
 * PURPOSE  : whether the tacacs plus entry is valid or not according to input params
 * INPUT    : tacacs_index, client_type, ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_IsTacacsPlusEntryValid(UI16_T tacacs_index, AAA_ClientType_T client_type, UI32_T ifindex);

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
BOOL_T AAA_MGR_SetAccUpdateInterval(UI32_T update_interval);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : disable the accounting update interval
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccUpdateInterval();

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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : enable dot1x accounting on specified port
 * INPUT    : ifindex, list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccDot1xEntry(const AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccDot1xEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable dot1x accounting on specified port
 * INPUT    : ifindex
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccDot1xEntry(UI32_T ifindex);

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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec accounting by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccExecEntry(const AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec accounting by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : only manual configured port
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAccExecEntry(AAA_ExecType_T exec_type);

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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetRunningAccCommandEntry(AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next command accounting entry by index.
 * INPUT    : index, use 0 to get first
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAccCommandEntry(UI32_T *index, AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccCommandEntry(AAA_AccCommandEntry_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAccCommandEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup command accounting by specified privilege level
 * INPUT    : privilege, list name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore to change the configure mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccCommandEntry(const AAA_AccCommandEntry_T *entry_p);

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
BOOL_T AAA_MGR_DisableAccCommandEntry(const AAA_AccCommandEntry_T *entry_p);

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
BOOL_T AAA_MGR_GetAccListEntry(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by index.
 * INPUT    : list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccListEntryByIndex(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccListEntryByPort
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting list by ifindex.
 * INPUT    : ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccListEntryByPort(UI32_T ifindex, AAA_AccListEntryInterface_T *entry);

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
BOOL_T AAA_MGR_SetAccListEntry(const AAA_AccListEntryInterface_T *entry);

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
BOOL_T AAA_MGR_DestroyAccListEntry(const char *name, AAA_ClientType_T client_type, AAA_WarningInfo_T *warning);

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
BOOL_T AAA_MGR_DestroyAccListEntry2(AAA_AccListEntryInterface_T *entry, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_DestroyAccListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by index
 * INPUT    : list_index
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyAccListEntryByIndex(UI16_T list_index, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's working mode and group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0'), working_mode
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTE     : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetDefaultList(const AAA_AccListEntryInterface_T *entry_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccountingGroupIndex_ByInterface
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccountingGroupIndex_ByInterface(const AAA_AccInterface_T *entry_p, AAA_QueryGroupIndexResult_T *query_result_p);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAccountingGroupIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAccountingGroupIndex(AAA_ClientType_T client_type, UI32_T ifindex, AAA_QueryGroupIndexResult_T *query_result);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_Register_AccComponent_Callback
 *-------------------------------------------------------------------------
 * PURPOSE  : register the accounting component function
 * INPUT    : cpnt_type, call_back_func
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
void AAA_MGR_Register_AccComponent_Callback(AAA_AccCpntType_T cpnt_type, AAA_AccCpntAsyncNotify_Callback_T call_back_func);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_AsyncAccountingRequest
 *-------------------------------------------------------------------------
 * PURPOSE  : sent accounting request and non-blocking wait accounting server response.
 * INPUT    : ifindex, client_type, request_type, identifier,
 *            user_name     --  User name (terminated with '\0')
 *            call_back_func      --  Callback function pointer.
 * OUTPUT   : none.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_AsyncAccountingRequest(AAA_AccRequest_T *request);

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
BOOL_T AAA_MGR_ConvertToExecType(UI32_T exec_id, AAA_ExecType_T *exec_type);


#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the moethod_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AccListEntryInterface_T
 *            method_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodName(UI16_T method_index, char* method_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the group_name into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AccListEntryInterface_T
              method_group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodGroupName(UI16_T method_index, char* method_group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodClientType(UI16_T method_index, AAA_ClientType_T client_type);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodMode
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set the mode into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    :   method_index   -- the index of AAA_AccListEntryInterface_T
              method_mode    -- VAL_aaaMethodMode_start_stop
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodMode(UI16_T method_index, UI8_T method_mode);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetMethodStatus
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set the status into AAA_AccListEntryInterface_T entry by method_index.
 * INPUT    : method_index   -- the index of AAA_AccListEntryInterface_T
 *            method_status  -- Set to VAL_aaaMethodStatus_valid to initiate the aaaMethodTable,
 *            VAL_aaaMethodStatus_invalid to destroy the table.
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetMethodStatus(UI16_T method_index, UI8_T method_status);
#endif /*SYS_CPNT_ACCOUNTING == TRUE*/

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    : radius_group_index
              radius_group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "radius" group name
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusGroupName(UI16_T radius_group_index, char* radius_group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupServerBitMap
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set group name into AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    : radius_group_index
              radius_group_server_bitmap
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't change the default "radius" group's radius entry
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusGroupServerBitMap(UI16_T radius_group_index, UI8_T radius_group_server_bitmap);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetRadiusGroupStatus
 *------------------------------------------------------------------------
 * PURPOSE  : Set the group status
 * INPUT    : radius_group_index
              radius_group_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : can't delete the default "radius" group
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetRadiusGroupStatus(UI16_T radius_group_index, UI32_T radius_group_status);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set group name into AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    : tacacsplus_group_index
              tacacsplus_group_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupName(UI16_T tacacsplus_group_index, char* tacacsplus_group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupServerBitMap
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set group name into AAA_TacacsPlusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    : tacacsplus_group_index
              tacacsplus_group_server_bitmap
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupServerBitMap(UI16_T tacacsplus_group_index, UI8_T tacacsplus_group_server_bitmap);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetTacacsPlusGroupStatus
 *------------------------------------------------------------------------
 * PURPOSE  : Set the group status of TacacsPlusGroupTable into AAA_MGR_SetTacacsPlusGroupStatus
 *            by tacacsplus_group_index.
 * INPUT    : tacacsplus_group_index
              tacacsplus_group_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetTacacsPlusGroupStatus(UI16_T tacacsplus_group_index, UI32_T tacacsplus_group_status);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetUpdate
 *------------------------------------------------------------------------
 * PURPOSE  : Setting update interval
 * INPUT    : update_interval
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetUpdate(UI32_T update_interval);

#if (SYS_CPNT_ACCOUNTING == TRUE)

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAccountMethodName
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set list-name into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    : ifindex
              account_method_name
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccountMethodName(UI32_T ifindex, char *account_method_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_Setdot1xAccountStatus
 *------------------------------------------------------------------------
 * PURPOSE  : This function to set list-status into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    : ifindex
              account_status
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAccountStatus(UI32_T ifindex, UI8_T account_status);
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
BOOL_T AAA_MGR_GetAccUserEntryQty(UI32_T *qty);

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
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByNameAndType(char *name, AAA_ClientType_T client_type, UI32_T *qty);

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
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty);

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
BOOL_T AAA_MGR_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty);

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
BOOL_T AAA_MGR_GetNextAccUserEntry(AAA_AccUserInfoInterface_T *entry);

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
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByNameAndType(AAA_AccUserInfoInterface_T *entry);

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
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByType(AAA_AccUserInfoInterface_T *entry);

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
BOOL_T AAA_MGR_GetNextAccUserEntryFilterByPort(AAA_AccUserInfoInterface_T *entry);

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
BOOL_T AAA_MGR_AuthorRequest(AAA_AuthorRequest_T *request,AAA_AuthorReply_T *reply);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorizationGroupIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the corresponding group_index according to input params
 * INPUT    : client_type, ifindex
 * OUTPUT   : query_result
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorizationGroupIndex(UI32_T priv_lvl, AAA_ClientType_T client_type, UI32_T ifindex, AAA_QueryGroupIndexResult_T *query_result);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : setup exec authorization by exec_type
 * INPUT    : entry->exec_type, entry->list_name
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : ignore entry->configure_mode
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorExecEntry(const AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DisableAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : disable exec authorization by exec_type
 * INPUT    : exec_type
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DisableAuthorExecEntry(AAA_ExecType_T exec_type);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_SetAuthorDefaultList
 *-------------------------------------------------------------------------
 * PURPOSE  : set the default list's group name by client_type
 * INPUT    : client_type, group_name (terminated with '\0')
 * OUTPUT   : none
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorDefaultList(const AAA_ListType_T *list_type, const char *group_name);

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
BOOL_T AAA_MGR_SetAuthorListEntry(const AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_DestroyAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : delete method-list by name and client_type
 * INPUT    : name (terminated with '\0'), client_type
 * OUTPUT   : warning
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : if not exist then return success
 *            can not delete default method list
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_DestroyAuthorListEntry(const char *list_name, const AAA_ListType_T *list_type, AAA_WarningInfo_T *warning);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetNextAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAuthorListEntry(AAA_AuthorListEntryInterface_T *entry);

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
SYS_TYPE_Get_Running_Cfg_T AAA_MGR_GetNextRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the authorization list by name and client_type
 * INPUT    : entry->list_name,  entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorListEntry(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_MGR_GetAuthorListEntryByIndex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the authorization list by index.
 * INPUT    : list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorListEntryByIndex(AAA_AuthorListEntryInterface_T *entry);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_GetAuthorMethodTable
 *------------------------------------------------------------------------
 * PURPOSE  : This function to get the AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetAuthorMethodTable(UI16_T method_index, AAA_AuthorListEntryInterface_T *entry);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_GetNextAuthorMethodTable
 *------------------------------------------------------------------------
 * PURPOSE  : This function to get the next AAA_AuthorListEntryInterface_T entry by method_index.
 * INPUT    : method_index
 * OUTPUT   : entry
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_GetNextAuthorMethodTable(UI16_T method_index, AAA_AuthorListEntryInterface_T *entry);

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
BOOL_T AAA_MGR_SetAuthorMethodName(UI16_T method_index, char* method_name);

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
BOOL_T AAA_MGR_SetAuthorMethodGroupName(UI16_T method_index, char* method_group_name);

/*------------------------------------------------------------------------
 * FUNCTION NAME - AAA_MGR_SetAuthorMethodClientType
 *------------------------------------------------------------------------
 * PURPOSE  : set the client type by list_index.
 * INPUT    : method_index, client_type
 * OUTPUT   : None
 * RETURN   : TRUE / FALSE
 * NOTE     : None
 *------------------------------------------------------------------------*/
BOOL_T AAA_MGR_SetAuthorMethodClientType(UI16_T method_index, const AAA_ListType_T *list_type);

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
BOOL_T AAA_MGR_SetAuthorMethodStatus(UI16_T method_index, UI8_T method_status);

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
BOOL_T AAA_MGR_SetAuthorCommandEntry(const AAA_AuthorCommandEntry_T *entry);

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
BOOL_T AAA_MGR_DisableAuthorCommandEntry(UI32_T priv_lvl, AAA_ExecType_T exec_type);
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : AAA_MGR_HandleIPCReqMsg
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
BOOL_T AAA_MGR_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif /* SYS_CPNT_AAA == TRUE */

#endif /* End of AAA_MGR_H */
