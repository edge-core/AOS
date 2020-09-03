/* =============================================================================
 * MODULE NAME : MLAG_OM.H
 * PURPOSE     : Provide declarations for MLAG public data management.
 * NOTE        : None.
 * HISTORY     :
 *     2014/04/18 -- Timon Chang, Create.
 *
 * Copyright(C)      Accton Corporation, 2014
 * =============================================================================
 */

#ifndef MLAG_OM_H
#define MLAG_OM_H

/* -----------------------------------------------------------------------------
 * INCLUDE FILE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#include "sys_type.h"
#include "sysfun.h"
#include "mlag_type.h"

/* -----------------------------------------------------------------------------
 * NAMING CONSTANT DECLARATIONS
 * -----------------------------------------------------------------------------
 */

#define MLAG_OM_IPCMSG_TYPE_SIZE sizeof(union MLAG_OM_IpcMsg_Type_U)

/* command used in IPC message
 */
enum
{
    MLAG_OM_IPC_GETGLOBALSTATUS,
    MLAG_OM_IPC_GETRUNNINGGLOBALSTATUS,
    MLAG_OM_IPC_GETDOMAINENTRY,
    MLAG_OM_IPC_GETNEXTDOMAINENTRY,
    MLAG_OM_IPC_GETMLAGENTRY,
    MLAG_OM_IPC_GETNEXTMLAGENTRY,
    MLAG_OM_IPC_GETNEXTMLAGENTRYBYDOMAIN,
    MLAG_OM_IPC_ISMLAGPORT,
};

/* -----------------------------------------------------------------------------
 * MACRO FUNCTION DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* Macro function for computation of IPC msg_buf size based on field name
 * used in MLAG_OM_IpcMsg_T.data
 */
#define MLAG_OM_GET_MSG_SIZE(field_name)                        \
            (MLAG_OM_IPCMSG_TYPE_SIZE +                         \
            sizeof(((MLAG_OM_IpcMsg_T*)0)->data.field_name))

/* -----------------------------------------------------------------------------
 * DATA TYPE DECLARATIONS
 * -----------------------------------------------------------------------------
 */

/* IPC message structure
 */
typedef struct
{
    union MLAG_OM_IpcMsg_Type_U
    {
        UI32_T  cmd;
        BOOL_T  ret_bool;
        UI32_T  ret_ui32;
    } type; /* the intended action or return value */

    union
    {
        UI32_T                  arg_ui32;
        MLAG_TYPE_DomainEntry_T arg_domain;
        MLAG_TYPE_MlagEntry_T   arg_mlag;
    } data;
} MLAG_OM_IpcMsg_T;

/* -----------------------------------------------------------------------------
 * EXPORTED SUBPROGRAM SPECIFICATIONS
 * -----------------------------------------------------------------------------
 */

/* FUNCTION NAME - MLAG_OM_GetGlobalStatus
 * PURPOSE : Get global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetGlobalStatus(UI32_T *status_p);

/* FUNCTION NAME - MLAG_OM_GetRunningGlobalStatus
 * PURPOSE : Get running global status of the CSC.
 * INPUT   : None
 * OUTPUT  : *status_p -- MLAG_TYPE_GLOBAL_STATUS_ENABLED /
 *                        MLAG_TYPE_GLOBAL_STATUS_DISABLED
 * RETURN  : SYS_TYPE_GET_RUNNING_CFG_SUCCESS /
 *           SYS_TYPE_GET_RUNNING_CFG_FAIL /
 *           SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE
 * NOTE    : None
 */
SYS_TYPE_Get_Running_Cfg_T MLAG_OM_GetRunningGlobalStatus(UI32_T *status_p);

/* FUNCTION NAME - MLAG_OM_GetDomainEntry
 * PURPOSE : Get a MLAG domain entry.
 * INPUT   : entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetDomainEntryByPort
 * PURPOSE : Get a MLAG domain entry by given peer link.
 * INPUT   : entry_p->peer_link -- peer link
 * OUTPUT  : entry_p -- buffer containing information for a domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetDomainEntryByPort(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetNextDomainEntry
 * PURPOSE : Get next MLAG domain entry.
 * INPUT   : entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next domain
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : Empty string for domain ID to get the first entry
 */
UI32_T MLAG_OM_GetNextDomainEntry(MLAG_TYPE_DomainEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetMlagEntry
 * PURPOSE : Get a MLAG entry.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing information for a MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetMlagEntryByPort
 * PURPOSE : Get a MLAG entry by given local MLAG member.
 * INPUT   : entry_p->local_member -- local MLAG member
 * OUTPUT  : entry_p -- buffer containing the MLAG entry
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : None
 */
UI32_T MLAG_OM_GetMlagEntryByPort(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetNextMlagEntry
 * PURPOSE : Get next MLAG entry.
 * INPUT   : entry_p->mlag_id -- MLAG ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_OM_GetNextMlagEntry(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_GetNextMlagEntryByDomain
 * PURPOSE : Get next MLAG entry in the given domain.
 * INPUT   : entry_p->mlag_id   -- MLAG ID
 *           entry_p->domain_id -- MLAG domain ID
 * OUTPUT  : entry_p -- buffer containing information for next MLAG
 * RETURN  : MLAG_TYPE_RETURN_CODE_E
 * NOTE    : 0 for MLAG ID to get the first entry
 */
UI32_T MLAG_OM_GetNextMlagEntryByDomain(MLAG_TYPE_MlagEntry_T *entry_p);

/* FUNCTION NAME - MLAG_OM_IsMlagPort
 * PURPOSE : Check whether a logical port is peer link or MLAG member.
 * INPUT   : lport -- logical port to be checked
 * OUTPUT  : None
 * RETURN  : TRUE  -- peer link or MLAG member
 *           FALSE -- otherwise
 * NOTE    : None
 */
BOOL_T MLAG_OM_IsMlagPort(UI32_T lport);

/* FUNCTION NAME - MLAG_OM_HandleIPCReqMsg
 * PURPOSE : Handle the IPC request message.
 * INPUT   : msgbuf_p -- IPC request message buffer
 * OUTPUT  : msgbuf_p -- IPC response message buffer
 * RETURN  : TRUE  -- there is a response required to be sent
 *           FALSE -- there is no response required to be sent
 * NOTE    : None
 */
BOOL_T MLAG_OM_HandleIPCReqMsg(SYSFUN_Msg_T* msgbuf_p);

#endif /* End of MLAG_OM_H */
