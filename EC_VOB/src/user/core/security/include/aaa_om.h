/* MODULE NAME: aaa_om.h
 * PURPOSE:
 *  Implement AAA
 *
 * NOTES:
 *
 * History:
 *    2007/08/06 : eli      Create this file
 *
 * Copyright(C)      Accton Corporation, 2004
 */
#ifndef AAA_OM_H
#define AAA_OM_H

#include "sys_type.h"
#include "aaa_def.h"
#include "aaa_om_private.h"

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
                UI16_T  group_index;
            }req;

            struct
            {
                AAA_RadiusGroupEntryInterface_T entry;
            }resp;

        }getnextrunningradiusgroupentry;

        union
        {
            struct
            {
                UI8_T   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
            }req;

            struct
            {
                UI16_T  group_index;
            }resp;

        }getradiusgroupentry_ex;

        union
        {
            struct
            {
                UI16_T  group_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_RadiusGroupEntryInterface_T entry;
            }resp;

        }getnextradiusgroupentry;


        union
        {
            struct
            {
                UI16_T  group_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_TacacsPlusGroupEntryInterface_T entry;
            }resp;

        }getnextrunningtacacsplusgroupentry;

        union
        {
            struct
            {
                UI8_T   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];
            }req;

            struct
            {
                UI16_T  group_index; /* array index + 1 */
            }resp;

        }gettacacsplusgroupentry_ex;

        union
        {
            struct
            {
                UI16_T  group_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_TacacsPlusGroupEntryInterface_T entry;
            }resp;

        }getnexttacacsplusgroupentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  radius_index; /* array index + 1 */

            }req;

            struct
            {
                AAA_RadiusEntryInterface_T entry;
            }resp;

        }getnextrunningradiusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  radius_index; /* array index + 1 */

            }req;

            struct
            {
                AAA_RadiusEntryInterface_T entry;
            }resp;

        }getnextradiusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  radius_index; /* array index + 1 */

            }req;

            struct
            {
                UI32_T  radius_server_index; /* mapping to radius_om */
            }resp;

        }getradiusentry_ex;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  radius_index; /* array index + 1 */

            }req;

            struct
            {
                UI32_T  order; /* mapping to radius_om */
            }resp;

        }getradiusentryorder;

        union
        {
            struct
            {
                UI16_T group_index;
                AAA_ClientType_T client_type;
                UI32_T ifindex;
            }req;

            struct
            {
            }resp;

        }isradiusgroupvalid;

        union
        {
            struct
            {
                UI16_T radius_index;
                AAA_ClientType_T client_type;
                UI32_T ifindex;
            }req;

            struct
            {
            }resp;

        }isradiusentryvalid;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  tacacs_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_TacacsPlusEntryInterface_T entry;
            }resp;

        }getnextrunningtacacsplusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  tacacs_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_TacacsPlusEntryInterface_T entry;
            }resp;

        }getnexttacacsplusentry;

        union
        {
            struct
            {
                UI16_T group_index;
                UI16_T  tacacs_index; /* array index + 1 */
            }req;

            struct
            {
                UI32_T  tacacs_server_index; /* mapping to tacacs_om */
            }resp;

        }gettacacsplusentry_ex;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        union
        {
            struct
            {
            }req;

            struct
            {
                UI32_T  update_interval; /* mapping to tacacs_om */
            }resp;

        }getaccupdateinterval;

        union
        {
            struct
            {
                AAA_ExecType_T  exec_type;
            }req;

            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode;
            }resp;

        }getrunningaccexecentry;
        union
        {
            struct
            {
                AAA_ExecType_T  exec_type;
            }req;

            struct
            {
                AAA_AccExecEntry_T entry;
            }resp;

        }getnextaccexecentry;

        union
        {
            struct
            {
                AAA_ExecType_T  exec_type;
            }req;

            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode;
            }resp;

        }getaccexecentry_ex;

        union
        {
            struct
            {
                UI32_T  ifindex; /* l_port (array index + 1) */
            }req;

            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode;
            }resp;
        }getrunningaccdot1xentry;

        union
        {
            struct
            {
                UI32_T  ifindex; /* l_port (array index + 1) */
            }req;

            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode;
            }resp;
        }getaccdot1xentry_ex;

        union
        {
            struct
            {
                UI16_T  list_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_AccListEntryInterface_T entry;
            }resp;
        }getnextrunningacclistentry;

        union
        {
            struct
            {
                UI16_T  list_index; /* array index + 1 */
            }req;

            struct
            {
                AAA_AccListEntryInterface_T entry;
            }resp;
        }getnextacclistentry;

        union
        {
            struct
            {
                UI16_T  list_index; /* array index + 1 */
                AAA_ClientType_T        client_type;    /* distinguish between DOT1X and EXEC method-list */
            }req;

            struct
            {
                AAA_AccListEntryInterface_T entry;
            }resp;
        }getnextacclistentryfilterbyclienttype;

        union
        {
            struct
            {
                AAA_ClientType_T client_type;
                UI32_T ifindex;
            }req;

            struct
            {
                AAA_QueryGroupIndexResult_T query_result;
            }resp;
        }getaccountinggroupindex_ex;

        union
        {
            struct
            {
                UI16_T  list_index;
            }req;

            struct
            {
                UI8_T   port_list[SYS_ADPT_TOTAL_NBR_OF_LPORT];
            }resp;
        }queryaccdot1xportlist;

        union
        {
            struct
            {
                UI16_T method_index;
                UI16_T  list_index; /* array index + 1 */
            }req;

            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                UI8_T   group_name[SYS_ADPT_MAX_LEN_OF_AAA_SERVER_GROUP_NAME + 1];

                AAA_ClientType_T        client_type;    /* distinguish between DOT1X and EXEC method-list */
                AAA_ServerGroupType_T   group_type;
                AAA_AccWorkingMode_T    working_mode;
            }resp;
        }getmethodtable;

        union
        {
            struct
            {
                UI16_T method_index;
            }req;

            struct
            {
                UI16_T method_index;
                AAA_AccListEntryInterface_T entry;
            }resp;
        }getnextmethodtable;

#if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE)
        union
        {
            struct
            {
                AAA_AccCommandEntry_T   entry;
            }req;

            struct
            {
                AAA_AccCommandEntry_T   entry;
            }resp;

        }getacccommandentryinf;

        union
        {
            struct
            {
                UI32_T                  index;
                AAA_AccCommandEntry_T   entry;
            }req;

            struct
            {
                UI32_T                  index;
                AAA_AccCommandEntry_T   entry;
            }resp;

        }getnextacccommandentry;
#endif /* #if (SYS_CPNT_ACCOUNTING_COMMAND == TRUE) */
#endif

        union
        {
            struct
            {
                UI16_T radius_group_index;
            }req;
            struct
            {
                AAA_RadiusGroupEntryMIBInterface_T entry;
            }resp;
        }getradiusgrouptable;

        union
        {
            struct
            {
                UI16_T radius_group_index;
            }req;
            struct
            {
                AAA_RadiusGroupEntryMIBInterface_T entry;
            }resp;
        }getnextradiusgrouptable;

        union
        {
            struct
            {
                UI16_T tacacsplus_group_index;
            }req;
            struct
            {
                AAA_TacacsPlusGroupEntryMIBInterface_T entry;
            }resp;
        }gettacacsplusgrouptable;

        union
        {
            struct
            {
                UI16_T tacacsplus_group_index;
            }req;
            struct
            {
                AAA_TacacsPlusGroupEntryMIBInterface_T entry;
            }resp;
        }getnexttacacsplusgrouptable;

        union
        {
            struct
            {

            }req;
            struct
            {
                UI32_T update_interval;
            }resp;
        }getupdate;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        union
        {
            struct
            {
                UI32_T ifindex;
            }req;
            struct
            {
                AAA_AccDot1xEntry_T entry;
            }resp;
        }getaccounttable;

        union
        {
            struct
            {
                UI32_T ifindex;
            }req;
            struct
            {
                AAA_AccDot1xEntry_T entry;
            }resp;
        }getnextaccounttable;
#endif
        union
        {
            struct
            {
            }req;
            struct
            {
            }resp;
        }backdoor_showaccuser;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
        union
        {
            struct
            {
                UI16_T  list_index; /* array index + 1 */
                AAA_ListType_T  list_type;
            }req;
            struct
            {
                AAA_AuthorListEntryInterface_T entry;
            }resp;
        }getnextauthorlistentryfilterbyclienttype;

        union
        {
            struct
            {
                AAA_ExecType_T   exec_type;
            }req;
            struct
            {
                AAA_AuthorExecEntry_T entry;
            }resp;
        }getnextauthorexecentry;

        union
        {
            struct
            {
                UI16_T  list_index; /* array index + 1 */
            }req;
            struct
            {
                AAA_AuthorListEntryInterface_T entry;
            }resp;
        }getnextrunningauthorlistentry;

#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

        union
        {
            struct
            {
                AAA_ExecType_T   exec_type;
            }req;
            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode; /* Auto or Manual */
            }resp;
        }getauthorexecentry_ex;

        union
        {
            struct
            {
                AAA_ExecType_T   exec_type;
            }req;
            struct
            {
                UI8_T   list_name[SYS_ADPT_MAX_LEN_OF_ACCOUNTING_LIST_NAME + 1];
                AAA_ConfigureMode_T configure_mode; /* Auto or Manual */
            }resp;
        }getrunningauthorexecentry;

        union
        {
            struct
            {
            }req;

            struct
            {
                UI32_T update_interval;
            }resp;
        }getrunningaccupdateinterval;

#if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE)
        union
        {
            struct
            {
                UI32_T           priv_lvl;
                AAA_ExecType_T   exec_type;
            }req;

            struct
            {
                AAA_AuthorCommandEntry_T entry;
            }resp;
        }get_author_command_entry;
#endif /* #if (SYS_CPNT_AUTHORIZATION_COMMAND == TRUE) */


    }data;
}   AAA_OM_IPCMsg_T;


typedef union
{
    struct
    {
        AAA_RadiusGroupEntryInterface_T entry;
    }getnextrunningradiusgroupentry;

    struct
    {
        AAA_RadiusGroupEntryInterface_T entry;
    }getradiusgroupentry_ex;

    struct
    {
        AAA_RadiusGroupEntryInterface_T entry;
    }getnextradiusgroupentry;

    struct
    {
        AAA_TacacsPlusGroupEntryInterface_T entry;
    }getnextrunningtacacsplusgroupentry;

    struct
    {
        AAA_TacacsPlusGroupEntryInterface_T entry;
    }gettacacsplusgroupentry_ex;

    struct
    {
        AAA_TacacsPlusGroupEntryInterface_T entry;
    }getnexttacacsplusgroupentry;

    struct
    {
        UI16_T group_index;
        AAA_RadiusEntryInterface_T entry;
    }getnextrunningradiusentry;

    struct
    {
        UI16_T group_index;
        AAA_RadiusEntryInterface_T entry;
    }getnextradiusentry;

    struct
    {
        UI16_T group_index;
        AAA_RadiusEntryInterface_T entry;
    }getradiusentry_ex;

    struct
    {
        UI16_T group_index;
        UI16_T radius_index;
        UI16_T order;
    }getradiusentryorder;

    struct
    {
        UI16_T group_index;
        AAA_ClientType_T client_type;
        UI32_T ifindex;
    }isradiusgroupvalid;

    struct
    {
        UI16_T radius_index;
        AAA_ClientType_T client_type;
        UI32_T ifindex;
    }isradiusentryvalid;

    struct
    {
        UI16_T group_index;
        AAA_TacacsPlusEntryInterface_T entry;
    }getnextrunningtacacsplusentry;

    struct
    {
        UI16_T group_index;
        AAA_TacacsPlusEntryInterface_T entry;
    }getnexttacacsplusentry;

    struct
    {
        UI16_T group_index;
        AAA_TacacsPlusEntryInterface_T entry;
    }gettacacsplusentry_ex;

#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
    struct
    {
        AAA_AccExecEntry_T entry;
    }getrunningaccexecentry;

    struct
    {
        AAA_AccExecEntry_T entry;
    }getnextaccexecentry;

    struct
    {
        AAA_AccExecEntry_T entry;
    }getaccexecentry_ex;

    struct
    {
        AAA_AccDot1xEntry_T entry;
    }getrunningaccdot1xentry;

    struct
    {
        AAA_AccDot1xEntry_T entry;
    }getaccdot1xentry_ex;

    struct
    {
        AAA_AccListEntryInterface_T entry;
    }getnextrunningacclistentry;

    struct
    {
        AAA_AccListEntryInterface_T entry;
    }getnextacclistentry;

    struct
    {
        AAA_AccListEntryInterface_T entry;
    }getnextacclistentryfilterbyclienttype;

    struct
    {
        AAA_ClientType_T client_type;
        UI32_T ifindex;
        AAA_QueryGroupIndexResult_T query_result;
    }getaccountinggroupindex_ex;

    struct
    {
        AAA_QueryAccDot1xPortListResult_T entry;
    }queryaccdot1xportlist;

    struct
    {
        UI16_T method_index;
        AAA_AccListEntryInterface_T entry;
    }getmethodtable;

    struct
    {
        UI16_T method_index;
        AAA_AccListEntryInterface_T entry;
    }getnextmethodtable;

#endif
    struct
    {
        UI16_T radius_group_index;
        AAA_RadiusGroupEntryMIBInterface_T entry;
    }getradiusgrouptable;

    struct
    {
        UI16_T radius_group_index;
        AAA_RadiusGroupEntryMIBInterface_T entry;
    }getnextradiusgrouptable;

    struct
    {
        UI16_T tacacsplus_group_index;
        AAA_TacacsPlusGroupEntryMIBInterface_T entry;
    }gettacacsplusgrouptable;

    struct
    {
        UI16_T tacacsplus_group_index;
        AAA_TacacsPlusGroupEntryMIBInterface_T entry;
    }getnexttacacsplusgrouptable;

    struct
    {
        UI32_T update_interval;
    }getupdate;
#if (SYS_CPNT_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
    struct
    {
        UI32_T ifindex;
        AAA_AccDot1xEntry_T entry;
    }getaccounttable;

    struct
    {
        UI32_T ifindex;
        AAA_AccDot1xEntry_T entry;
    }getnextaccounttable;
#endif
    struct
    {
    }backdoor_showaccuser;

#if (SYS_CPNT_AUTHORIZATION == TRUE)
    struct
    {
        AAA_AuthorListEntryInterface_T entry;
    }getnextauthorlistentryfilterbyclienttype;

    struct
    {
        AAA_AuthorExecEntry_T entry;
    }getnextauthorexecentry;

    struct
    {
        AAA_AuthorExecEntry_T entry;
    }getauthorexecentry_ex;

    struct
    {
        AAA_AuthorExecEntry_T entry;
    }getrunningauthorexecentry;

    struct
    {
        AAA_AuthorListEntryInterface_T entry;
    }getnextrunningauthorlistentry;
#endif /* #if (SYS_CPNT_AUTHORIZATION == TRUE) */

    struct
    {
        UI32_T priv_lvl;
        AAA_ExecType_T exec_type;
    } get_author_command_entry;

    union
    {
        UI32_T update_interval;
    }getrunningaccupdateinterval;

}   AAA_OM_IPCMsg_ALL_ARG_T;

enum
{
    AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSGROUPENTRY,
    AAA_OM_IPCCMD_GETRADIUSGROUPENTRY_EX,
    AAA_OM_IPCCMD_GETNEXTRADIUSGROUPENTRY,
    AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSGROUPENTRY,
    AAA_OM_IPCCMD_GETTACACSPLUSGROUPENTRY_EX,
    AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPENTRY,
    AAA_OM_IPCCMD_GETNEXTRUNNINGRADIUSENTRY,
    AAA_OM_IPCCMD_GETNEXTRADIUSENTRY,
    AAA_OM_IPCCMD_GETRADIUSENTRY_EX,
    AAA_OM_IPCCMD_GETRADIUSENTRYORDER,
    AAA_OM_IPCCMD_ISRADIUSGROUPVALID,
    AAA_OM_IPCCMD_ISRADIUSENTRYVALID,
    AAA_OM_IPCCMD_GETNEXTRUNNINGTACACSPLUSENTRY,
    AAA_OM_IPCCMD_GETNEXTTACACSPLUSENTRY,
    AAA_OM_IPCCMD_GETTACACSPLUSENTRY_EX,
    AAA_OM_IPCCMD_GETRUNNINGACCUPDATEINTERVAL,
    AAA_OM_IPCCMD_GETACCUPDATEINTERVAL,
    AAA_OM_IPCCMD_GETRUNNINGACCEXECENTRY,
    AAA_OM_IPCCMD_GETNEXTACCEXECENTRY,
    AAA_OM_IPCCMD_GETACCEXECENTRY_EX,
    AAA_OM_IPCCMD_GETRUNNINGACCDOT1XENTRY,
    AAA_OM_IPCCMD_GETACCDOT1XENTRY_EX,
    AAA_OM_IPCCMD_GETNEXTRUNNINGACCLISTENTRY,
    AAA_OM_IPCCMD_GETNEXTACCLISTENTRY,
    AAA_OM_IPCCMD_GETNEXTACCLISTENTRYFILTERBYCLIENTTYPE,
    AAA_OM_IPCCMD_QUERYACCDOT1XPORTLIST,
    AAA_OM_IPCCMD_GETMETHODTABLE,
    AAA_OM_IPCCMD_GETNEXTMETHODTABLE,
    AAA_OM_IPCCMD_GETRADIUSGROUPTABLE,
    AAA_OM_IPCCMD_GETNEXTRADIUSGROUPTABLE,
    AAA_OM_IPCCMD_GETTACACSPLUSGROUPTABLE,
    AAA_OM_IPCCMD_GETNEXTTACACSPLUSGROUPTABLE,
    AAA_OM_IPCCMD_GETUPDATE,
    AAA_OM_IPCCMD_GETACCOUNTTABLE,
    AAA_OM_IPCCMD_GETNEXTACCOUNTTABLE,
    AAA_OM_IPCCMD_BACKDOOR_SHOWACCUSER,
    AAA_OM_IPCCMD_GETNEXTAUTHORLISTENTRYFILTERBYCLIENTTYPE,
    AAA_OM_IPCCMD_GETNEXTAUTHOREXECENTRY,
    AAA_OM_IPCCMD_GETAUTHOREXECENTRY_EX,
    AAA_OM_IPCCMD_GETRUNNINGAUTHOREXECENTRY,
    AAA_OM_IPCCMD_GETNEXTRUNNINGAUTHORLISTENTRY,
    AAA_OM_IPCCMD_GETACCCOMMANDENTRYINTERFACE,
    AAA_OM_IPCCMD_GETNEXTACCCOMMANDENTRY,
    AAA_OM_IPCCMD_GETRUNNINGACCCOMMANDENTRY,
    AAA_OM_IPCCMD_GET_AUTHOR_COMMAND_ENTRY,
    AAA_OM_IPCCMD_GET_RUNNING_AUTHOR_COMMAND_ENTRY,
    AAA_OM_IPCCMD_FOLLOWISASYNCHRONISMIPC
};


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccUpdateInterval(UI32_T *update_interval);

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry);


 /*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the radius group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusGroupEntry_Ex(AAA_RadiusGroupEntryInterface_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusGroupEntry(AAA_RadiusGroupEntryInterface_T *entry);


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry);


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusGroupEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the tacacs group entry by name.
 * INPUT    : entry->group_name
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupEntry_Ex(AAA_TacacsPlusGroupEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusGroupEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next group by index.
 * INPUT    : entry->group_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusGroupEntry(AAA_TacacsPlusGroupEntryInterface_T *entry);


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextRadiusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next radius accounting entry by group_index and radius_index.
 * INPUT    : group_index (1-based), entry->radius_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextRadiusEntry(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetRadiusEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the radius entry by group_index, radius_index
 * INPUT    : group_index (1-based), radius_index (1-based);
 * OUTPUT   : radius_server_index (1-based);
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntry_Ex(UI16_T group_index, AAA_RadiusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:
 *--------------------------------AAA_OM_GetRadiusEntryOrder-----------------------------------------
 * PURPOSE  : get the radius entry's order in group
 * INPUT    : group_index (1-based), radius_index (1-based);
 * OUTPUT   : order (1-based);
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetRadiusEntryOrder(UI16_T group_index, UI16_T radius_index, UI16_T *order);

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the next tacacs plus accounting entry by group_index and tacacs_index.
 * INPUT    : group_index (1-based), entry->tacacs_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusEntry(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);


/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetTacacsPlusEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : get the tacacs plus entry by group_index, tacacs_index
 * INPUT    : group_index (1-based), tacacs_index (1-based);
 * OUTPUT   : tacacs_server_index (1-based);
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusEntry_Ex(UI16_T group_index, AAA_TacacsPlusEntryInterface_T *entry);

#if (SYS_CPNT_ACCOUNTING == TRUE)
/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccUpdateInterval
 *-------------------------------------------------------------------------
 * PURPOSE  : get the accounting update interval (minutes);
 * INPUT    : none
 * OUTPUT   : update_interval
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccUpdateInterval(UI32_T *update_interval);

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
 * FUNCTION NAME:  AAA_OM_GetNextAccExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccExecEntry(AAA_AccExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec accounting entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccExecEntry_Ex(AAA_AccExecEntry_T *entry);

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAccDot1xEntry(AAA_AccDot1xEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAccDot1xEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the dot1x accounting entry by index.
 * INPUT    : entry->ifindex
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAccDot1xEntry_Ex(AAA_AccDot1xEntry_T *entry);

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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAccListEntry(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list by index.
 * INPUT    : entry->list_index
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntry(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAccListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next accounting list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAccListEntryFilterByClientType(AAA_AccListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_QueryAccDot1xPortList
 *-------------------------------------------------------------------------
 * PURPOSE  : get the port list associated with specific list_index
 * INPUT    : query_result->list_index
 * OUTPUT   : query_result->port_list
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_QueryAccDot1xPortList(AAA_QueryAccDot1xPortListResult_T *query_result);

#endif

#if (AAA_SUPPORT_ACCTON_AAA_MIB == TRUE) /* the following APIs are defined by Leo Chen 2004.4.19 */
#if (SYS_CPNT_ACCOUNTING == TRUE)
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
BOOL_T AAA_OM_GetMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry);

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
BOOL_T AAA_OM_GetNextMethodTable(UI16_T method_index, AAA_AccListEntryInterface_T *entry);

#endif /* SYS_CPNT_ACCOUNTING == TRUE */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextRadiusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_RadiusGroupEntryMIBInterface_T entry by radius_group_index.
 * INPUT    :   radius_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
BOOL_T AAA_OM_GetNextRadiusGroupTable(UI16_T radius_group_index, AAA_RadiusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry);


/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_GetNextTacacsPlusGroupTable
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to get next AAA_TacacsPlusEntryMIBInterface_T entry by tacacsplus_group_index.
 * INPUT    :   tacacsplus_group_index
 * OUTPUT   :   entry
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextTacacsPlusGroupTable(UI16_T tacacsplus_group_index, AAA_TacacsPlusGroupEntryMIBInterface_T *entry);

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
BOOL_T AAA_OM_GetUpdate(UI32_T *update_interval);


#if (SYS_CPNT_ACCOUNTING == TRUE)
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
BOOL_T AAA_OM_GetAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry);

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
BOOL_T AAA_OM_GetNextAccountTable(UI32_T ifindex, AAA_AccDot1xEntry_T *entry);

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AAA_OM_SetAccountMethodName
 * ------------------------------------------------------------------------
 * PURPOSE  :   This function to set list-name into AAA_AccDot1xEntry_T entry by ifindex.
 * INPUT    :   ifindex
                account_method_name
 * OUTPUT   :   None
 * RETURN   :   TRUE / FALSE
 * NOTE     :   None
 * ------------------------------------------------------------------------
 */
 #endif
#endif /* AAA_SUPPORT_ACCTON_AAA_MIB == TRUE */


#if (SYS_CPNT_AUTHORIZATION == TRUE)

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorListEntryFilterByClientType
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next authorization list with specified client_type by index.
 * INPUT    : entry->list_index, entry->client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorListEntryFilterByClientType(AAA_AuthorListEntryInterface_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetNextAuthorExecEntry
 *-------------------------------------------------------------------------
 * PURPOSE  : This function to get the next exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetNextAuthorExecEntry(AAA_AuthorExecEntry_T *entry);

/*-------------------------------------------------------------------------
 * FUNCTION NAME:  AAA_OM_GetAuthorExecEntry_Ex
 *-------------------------------------------------------------------------
 * PURPOSE  : get the exec authorization entry by exec_type.
 * INPUT    : entry->exec_type
 * OUTPUT   : entry
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : none.
 *-------------------------------------------------------------------------*/
BOOL_T AAA_OM_GetAuthorExecEntry_Ex(AAA_AuthorExecEntry_T *entry);


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetRunningAuthorExecEntry(AAA_AuthorExecEntry_T *entry);


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
SYS_TYPE_Get_Running_Cfg_T AAA_OM_GetNextRunningAuthorListEntry(AAA_AuthorListEntryInterface_T *entry);

#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SWCTRL_OM_HandleIPCReqMsg
 *------------------------------------------------------------------------------
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
 *------------------------------------------------------------------------------
 */
BOOL_T AAA_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

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
BOOL_T AAA_OM_CreatSem(void);

#endif /* End of AAA_OM_H */

