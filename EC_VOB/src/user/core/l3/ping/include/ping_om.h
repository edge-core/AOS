/* Module Name: PING_OM.H
 * Purpose:
 *     This component implements the standard Ping-MIB functionality (rfc2925).
 *
 * Notes:
 *      1. This design references the TRACEROUTE module.
 *      2. Currently support IPv4 address only.
 *
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/3      --  peter_yu    Create to support partial standard ping-MIB.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 *                                  (2) Handle IPC Message.
 *                                  (3) Handle critical section.
 * Copyright(C)      Accton Corporation, 2007
 */

#ifndef _PING_OM_H
#define _PING_OM_H


/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "ping_type.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* command used in IPC message
 */
enum
{
    PING_OM_IPC_GETFORCEVERSION = 1,
    PING_OM_IPCCMD_GETCTLENTRY,
    PING_OM_IPCCMD_GETNEXTCTLENTRY,
    PING_OM_IPCCMD_GETPROBEHISTORYENTRY,
    PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRY,
    PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRYFORCLI,
    PING_OM_IPCCMD_GETRESULTSENTRY,
    PING_OM_IPCCMD_GETNEXTRESULTSENTRY,
};

/* MACRO FUNCTION DECLARATIONS
 */
/* Macro function for computation of IPC msg_buf size based on field name
 * used in PING_OM_IpcMsg_T.data
 */
#define PING_OM_GET_MSG_SIZE(field_name)                      \
            (PING_OM_MSGBUF_TYPE_SIZE +                       \
             sizeof(((PING_OM_IPCMsg_T*)0)->data.field_name))


#define PING_OM_MSGBUF_TYPE_SIZE sizeof(union PING_OM_IPCMsg_Type_U)


/* DATA TYPE DECLARATIONS
 */
/* IPC message structure
 */
typedef struct
{
	union PING_OM_IPCMsg_Type_U
	{
		UI32_T cmd;
		UI32_T result_ui32;
	} type;

	union
	{
        PING_TYPE_PingCtlEntry_T          ctrl_entry;
        PING_TYPE_PingProbeHistoryEntry_T probe_history_entry;
        PING_TYPE_PingResultsEntry_T      results_entry;

/*        struct
        {
            UI32_T                       ui32;
            PING_TYPE_PingResultsEntry_T results_entry;
        } ui32_resultsentry;
*/
        struct
        {
            PING_SORTLST_ELM_T sortlst_elm;
            UI32_T             ui32;
        } sortlst_elm_ui32;

        struct
        {
            UI32_T             ui32;
            PING_SORTLST_ELM_T sortlst_elm;
        } ui32_sortlst_elm;
	} data;
} PING_OM_IPCMsg_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME : PING_OM_Initiate_System_Resources
 * PURPOSE:
 *      Initialize ping OM
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None
 */
void PING_OM_Initiate_System_Resources(void);

/* FUNCTION NAME:PING_OM_GetCtlEntry
 * PURPOSE:
 *          Get the specific ping control entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          ctrl_entry_p    -- the specific control entry if it exists.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

/* FUNCTION NAME:PING_OM_GetNextCtlEntry
 * PURPOSE:
 *          To get the next ping control entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          ctrl_entry_p    -- the next control entry if it exists.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetNextCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p);

/* FUNCTION NAME: PING_OM_GetProbeHistoryEntry
 * PURPOSE:
 *          To get the specified available prob history entry
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *
 * OUTPUT:
 *          prob_history_entry_p    -- entry that contains prob packet information
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 */
UI32_T PING_OM_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

/* FUNCTION NAME: PING_OM_GetNextProbeHistoryEntry
 * PURPOSE:
 *          To get next available prob history entry
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *
 * OUTPUT:
 *          prob_history_entry_p    -- next available entry that contains prob packet information
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Recursive call is used, although not suggested, but currently it works well.
 */
UI32_T PING_OM_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

/* FUNCTION NAME: PING_OM_GetNextProbeHistoryEntryForCli
 * PURPOSE:
 *          To get next available prob history entry (only for CLI use)
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *
 * OUTPUT:
 *          prob_history_entry_p    -- next available entry that contains prob packet information
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 *          Similar to PING_OM_GetNextProbeHistoryEntry() except ping_ctl_owner_index and
 *          ping_ctl_test_name must be provided and are fixed. Only for CLI use.
 */
UI32_T PING_OM_GetNextProbeHistoryEntryForCli(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);

/* FUNCTION NAME: PING_OM_GetResultsEntry
 * PURPOSE:
 *          To the specific result entry base on the given index
 * INPUT:
 *          result_entry_p  -- the specific result entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          result_entry_p  -- contains values to get to result entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p);

/* FUNCTION NAME: PING_OM_GetFirstActiveResultsEntry
 * PURPOSE:
 *          To get the first active result entry which oper status is enabled.
 * INPUT:
 *          index_p - Index to the result entry.
 * OUTPUT:
 *          result_entry_p - contains values to set to prob history entry.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetFirstActiveResultsEntry(UI32_T *index_p, PING_TYPE_PingResultsEntry_T *result_entry_p);

/* FUNCTION NAME: PING_OM_GetNextActiveResultsEntry
 * PURPOSE:
 *          To get next active result entry which oper status is enabled.
 * INPUT:
 *          index_p - Index to the result entry.
 * OUTPUT:
 *          index_p - Index to the result entry.
 *          result_entry_p - next available active result entry
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetNextActiveResultsEntry(UI32_T *index_p, PING_TYPE_PingResultsEntry_T *result_entry_p);

/* FUNCTION NAME: PING_OM_GetNextResultsEntry
 * PURPOSE:
 *          To get the next result entry
 * INPUT:
 *          result_entry_p  -- the specific result entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          result_entry_p  -- next available result entry
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_GetNextResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p);

/* UTILITY FUNCTIONS
 */
/* FUNCTION NAME: PING_OM_IsCtlEntryExist
 * PURPOSE:
 *          To check if the specific control entry information exist
 *          in OM or not.
 * INPUT:
 *          ctl_entry_p     -- the pointer of specific control entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_IsPingCtlEntryExist(PING_TYPE_PingCtlEntry_T *ctl_entry_p);

/* FUNCTION NAME: PING_OM_PingKeyToTableIndex
 * PURPOSE:
 *          Find the table index from the key of the table.
 * INPUT:
 *          elm_p->owner_index - owner index is the task name
 *          elm_p->test_name - the specific test session name
 * OUTPUT:
 *          table_index_p - table indeex
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_PingKeyToTableIndex(PING_SORTLST_ELM_T *elm_p, UI32_T *table_index_p);

/* FUNCTION NAME: PING_OM_PingTableIndexToKey
 * PURPOSE:
 *          Find the key of the table from the table index.
 * INPUT:
 *          table_index -- table index
 * OUTPUT:
 *          elm_p       -- includes owner_index, test_name, table_index;
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_PingTableIndexToKey(UI32_T table_index, PING_SORTLST_ELM_T *elm_p);

/* FUNCTION NAME : PING_OM_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for PING om.
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
 *
 */
BOOL_T PING_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p);

#endif
