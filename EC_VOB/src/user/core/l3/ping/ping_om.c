/* Module Name: ping_om.c
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
 *      2007/12     --  peter_yu    Porting to linux platform.
 *                                  (1) IP address format is changed to UI8_T[SYS_ADPT_IPV4_ADDR_LEN].
 *                                  (2) Handle IPC Message.
 *                                  (3) Handle critical section.
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "leaf_2925p.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "l_rstatus.h"
#include "l_sort_lst.h"
#include "l_inet.h"
#include "sysfun.h"
#include "ping_mgr.h"
#include "ping_om.h"
#include "ping_om_private.h"
#include "ping_type.h"
#include "backdoor_mgr.h"

/* LOCAL DATATYPE DECLARATION
 */
extern BOOL_T ping_backdoor_debug; /* declare in ping_mgr.h */

/* NAMING CONSTANT DECLARATIONS
 */
#define PING_PROB_HISTORY_TABLE_SIZE    ( SYS_ADPT_MAX_PING_NUM * SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY )

/* TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T      prob_history_head;
    UI32_T      prob_history_end;
    UI32_T      current_prob_history;
    UI32_T      prob_history_max_index;
} PING_PROB_HISTORY_INDEX_T;


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int      PING_OM_Compare(void *elm1, void *elm2);
static void     PING_OM_ClearProbHistoryEntry(UI32_T table_index);
static void     PING_OM_InitProbHistoryIndexTable(void);
static UI32_T   PING_OM_FindKeyIndex(PING_SORTLST_ELM_T *list_elm_p);
static BOOL_T   PING_OM_SemanticCheck(void  *ctrl_entry_p);
static void     PING_OM_AssociateResultTable(UI32_T table_index, PING_TYPE_PingCtlEntry_T *ctrl_entry_p);
static void     PING_OM_DisAssociateResultTable(UI32_T table_index);
static UI32_T   PING_OM_CreateNewCtrlEntry(PING_TYPE_PingCtlEntry_T  ctrl_entry);
//static UI32_T   PING_OM_ConvertDayTime(UI8_T *str_p, UI8_T *day_and_time, UI32_T day_and_time_len);
static UI32_T   PING_OM_Internal_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p);
static UI32_T   PING_OM_Internal_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p, BOOL_T is_fix_ctl_entry);

/* STATIC VARIABLE DECLARATIONS
 */
static PING_TYPE_PingCtlEntry_T             ping_control_table[SYS_ADPT_MAX_PING_NUM];
static PING_TYPE_PingResultsEntry_T         ping_result_table[SYS_ADPT_MAX_PING_NUM];
static PING_TYPE_PingProbeHistoryEntry_T    ping_prob_history_table[PING_PROB_HISTORY_TABLE_SIZE];
static PING_PROB_HISTORY_INDEX_T            prob_history_index_table[SYS_ADPT_MAX_PING_NUM];
static L_SORT_LST_List_T                    sort_ping_list;

static UI32_T   next_available_index;
static UI8_T    current_control_entry;

/* for OM critical section */
static UI32_T   ping_om_sem_id;

/* EXPORTED SUBPROGRAM BODIES
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
void PING_OM_Initiate_System_Resources(void)
{
    UI32_T res;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_PING_OM, &ping_om_sem_id) != SYSFUN_OK)
    {
        printf("\r\n%s: get om sem id fail!\r\n", __FUNCTION__);
    }

    if ((res = L_SORT_LST_Create(&sort_ping_list, SYS_ADPT_MAX_PING_NUM, sizeof(PING_SORTLST_ELM_T),PING_OM_Compare)) != TRUE)
    {
        printf("\r\n%s: can not create sorted list!\r\n", __FUNCTION__);
    }

    memset(ping_control_table, 0, sizeof(PING_TYPE_PingCtlEntry_T) * SYS_ADPT_MAX_PING_NUM);
    memset(ping_result_table, 0, sizeof(PING_TYPE_PingResultsEntry_T) * SYS_ADPT_MAX_PING_NUM);
    memset(ping_prob_history_table, 0, sizeof(PING_TYPE_PingProbeHistoryEntry_T) * PING_PROB_HISTORY_TABLE_SIZE);
    memset(prob_history_index_table, 0, sizeof(PING_PROB_HISTORY_INDEX_T) * SYS_ADPT_MAX_PING_NUM);

    next_available_index = 0;
    current_control_entry = 0;

    return;
} /* end of PING_OM_Initiate_System_Resources() */

/* FUNCTION NAME : PING_OM_ClearOM
 * PURPOSE:
 *      Clear ping OM
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
void PING_OM_ClearOM (void)
{
    memset(ping_control_table, 0, sizeof(PING_TYPE_PingCtlEntry_T) * SYS_ADPT_MAX_PING_NUM);
    memset(ping_result_table, 0, sizeof(PING_TYPE_PingResultsEntry_T) * SYS_ADPT_MAX_PING_NUM);
    memset(ping_prob_history_table, 0, sizeof(PING_TYPE_PingProbeHistoryEntry_T) * PING_PROB_HISTORY_TABLE_SIZE);
    memset(prob_history_index_table, 0, sizeof(PING_PROB_HISTORY_INDEX_T) * SYS_ADPT_MAX_PING_NUM);

    L_SORT_LST_Delete_All(&sort_ping_list);
    next_available_index = 0;
    current_control_entry = 0;
    return;
} /* end of PING_OM_ClearOM() */


/* FUNCTION NAME : PING_OM_EnterMasterMode
 * PURPOSE:
 *      Apply default value to ping OM
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
void PING_OM_EnterMasterMode (void)
{
    PING_OM_InitProbHistoryIndexTable();
    return;
} /* end of PING_OM_EnterMasterMode() */

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
UI32_T PING_OM_GetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    list_elm;
    UI32_T      res;
    UI32_T      original_priority;

    /* BODY
     */
    /* DEBUG */
    /*printf("%s, %d, ctrl_entry_p->ping_ctl_owner_index: %s\n", __FUNCTION__, __LINE__, ctrl_entry_p->ping_ctl_owner_index);
    printf("%s, %d, ctrl_entry_p->ping_ctl_test_name: %s\n", __FUNCTION__, __LINE__, ctrl_entry_p->ping_ctl_test_name);
    printf("%s, %d, ctrl_entry_p->ping_ctl_owner_index_len: %ld\n", __FUNCTION__, __LINE__, ctrl_entry_p->ping_ctl_owner_index_len);
    printf("%s, %d, ctrl_entry_p->ping_ctl_admin_status: %ld\n", __FUNCTION__, __LINE__, ctrl_entry_p->ping_ctl_admin_status);
    printf("%s, %d, ctrl_entry_p->ping_ctl_target_address: %d.%d.%d.%d\n", __FUNCTION__, __LINE__,
            ctrl_entry_p->ping_ctl_target_address[0],
            ctrl_entry_p->ping_ctl_target_address[1],
            ctrl_entry_p->ping_ctl_target_address[2],
            ctrl_entry_p->ping_ctl_target_address[3]);
    */


    if (ctrl_entry_p == NULL)
    {
        return PING_TYPE_FAIL;
    }

    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(list_elm.owner_index,  ctrl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(list_elm.test_name, ctrl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);
    /* DEBUG */
    //printf("%s, %d, [Enter CS]\n", __FUNCTION__, __LINE__);

    if ((res = PING_OM_FindKeyIndex(&list_elm)) != PING_TYPE_OK)
    {
        /* DEBUG */
        //printf("%s, %d, [Leave CS]\n", __FUNCTION__, __LINE__);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
        return PING_TYPE_FAIL;
    }

    memcpy(ctrl_entry_p, &ping_control_table[list_elm.table_index],
           sizeof(PING_TYPE_PingCtlEntry_T));

    /* DEBUG */
    //printf("%s, %d, [Leave CS]\n", __FUNCTION__, __LINE__);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
    return PING_TYPE_OK;

} /* end of PING_OM_GetCtlEntry() */

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
UI32_T PING_OM_GetNextCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    list_elm;

    /* BODY
     */
    if (ctrl_entry_p == NULL)
    {
        return PING_TYPE_FAIL;
    }

    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(list_elm.owner_index,  ctrl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(list_elm.test_name, ctrl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if(L_SORT_LST_Get_Next(&sort_ping_list, &list_elm) != TRUE)
        return PING_TYPE_FAIL;

    memcpy(ctrl_entry_p, &ping_control_table[list_elm.table_index],
           sizeof(PING_TYPE_PingCtlEntry_T));

    return PING_TYPE_OK;

} /* end of PING_OM_GetNextCtlEntry() */

/* FUNCTION NAME:PING_OM_SetCtlEntry
 * PURPOSE:
 *          To create / destroy contrl entry.
 * INPUT:
 *          ctrl_entry_p    -- the specific control entry, including keys:
 *                             ping_ctl_owner_index, ping_ctl_test_name.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 * NOTES:
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status.
 *          3. The ctrl_entry should not be modified, so we create a local entry for local use.
 */
UI32_T PING_OM_SetCtlEntry(PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_TYPE_PingCtlEntry_T    local_entry;
    PING_SORTLST_ELM_T          list_elm;
    UI32_T      res, action;
    UI32_T      row_status_result, row_status;
    BOOL_T      admin_status_changed = FALSE;

    /* BODY
     */
    /* DEBUG */
    //printf("%s, %d\n", __FUNCTION__, __LINE__);

    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(list_elm.owner_index, ctrl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(list_elm.test_name, ctrl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    memset(&local_entry, 0, sizeof(PING_TYPE_PingCtlEntry_T));

    memcpy(local_entry.ping_ctl_owner_index, ctrl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(local_entry.ping_ctl_test_name, ctrl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    local_entry.ping_ctl_owner_index_len = ctrl_entry_p->ping_ctl_owner_index_len;
    local_entry.ping_ctl_test_name_len = ctrl_entry_p->ping_ctl_test_name_len;

    res = PING_OM_GetCtlEntry(&local_entry);

    if(res == PING_TYPE_OK)
    {
        if ((res = PING_OM_FindKeyIndex(&list_elm)) != PING_TYPE_OK)
        {
            return PING_TYPE_FAIL;
        }
    }
    /* in this switch, we set the original row_status and detect the admin_status changes.*/
    switch (ctrl_entry_p->ping_ctl_rowstatus)
    {
        case VAL_pingCtlRowStatus_notReady:
            if (res != PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            row_status = local_entry.ping_ctl_rowstatus;
            break;

        case VAL_pingCtlRowStatus_createAndWait:
            if (res == PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            row_status = VAL_pingCtlRowStatus_createAndWait;
            break;

        case VAL_pingCtlRowStatus_notInService:
            /*  NOT SUPPORT
             */
            return PING_TYPE_FAIL;
            break;

        case VAL_pingCtlRowStatus_destroy:
        case VAL_pingCtlRowStatus_active:
            /* Entry does not exist in control_table
             */
            if (res != PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            if (local_entry.ping_ctl_admin_status != ctrl_entry_p->ping_ctl_admin_status)
            {
                admin_status_changed = TRUE;
            }

            row_status = local_entry.ping_ctl_rowstatus;

            break;

        case VAL_pingCtlRowStatus_createAndGo:
            /* The given key already exist in control table.
               Cannot create a duplicate entry.
             */
            if (res == PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }

            if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
            {
                admin_status_changed = TRUE; /* from dedault value: "disabled" to "enabled" */
            }

            row_status = VAL_pingCtlRowStatus_createAndGo;
            break;

        default:
            return PING_TYPE_FAIL;
            break;
    } /* end of switch */

    action = ctrl_entry_p->ping_ctl_rowstatus;

    /* VAL_RowStatus_notReady is not a valid action in l_rstatus.c, use VAL_RowStatus_notInService instead */
    if(action == VAL_pingCtlRowStatus_notReady)
    {
        action = VAL_pingCtlRowStatus_notInService;
    }

    row_status_result = L_RSTATUS_Fsm(action,
                            &row_status,
                            PING_OM_SemanticCheck,
                            (void*)ctrl_entry_p);

    switch (row_status_result)
    {
        case L_RSTATUS_NOTEXIST_2_NOTREADY:

            if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
            {
                ctrl_entry_p->ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
            }
            ctrl_entry_p->ping_ctl_rowstatus = row_status;

            if ( (res = PING_OM_CreateNewCtrlEntry(*ctrl_entry_p)) != PING_TYPE_OK)
            {
                return res;
            }
            break;

        case L_RSTATUS_NOTEXIST_2_ACTIVE:

            ctrl_entry_p->ping_ctl_rowstatus = row_status;

            if ( (res = PING_OM_CreateNewCtrlEntry(*ctrl_entry_p)) != PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            if ((res = PING_OM_FindKeyIndex(&list_elm)) != PING_TYPE_OK)
            {
                return PING_TYPE_FAIL;
            }
            if (admin_status_changed)
            {
                if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
                {
                    memcpy(&ping_control_table[list_elm.table_index], ctrl_entry_p,
                            sizeof(PING_TYPE_PingCtlEntry_T));

                    if ((res = PING_MGR_CreateWorkSpace (list_elm.table_index, ctrl_entry_p)) != PING_TYPE_OK)
                    {
                        return PING_TYPE_FAIL;
                    }
                    PING_OM_ClearProbHistoryEntry(list_elm.table_index);
                    PING_OM_AssociateResultTable(list_elm.table_index, ctrl_entry_p);

                    if ((res = PING_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_pingCtlAdminStatus_enabled)) != PING_TYPE_OK)
                    {
                        return PING_TYPE_FAIL;
                    }
                }
                /*else  from notexist_2_active, it's impossible to disassociate.
                    it's impossible admin_status_change from disabe to disable.*/
            }
            break;

        case L_RSTATUS_ACTIVE_2_NOTEXIST:
        case L_RSTATUS_NOTREADY_2_NOTEXIST:

            memset(&ping_control_table[list_elm.table_index], 0,
                   sizeof(PING_TYPE_PingCtlEntry_T));

            memset(&ping_result_table[list_elm.table_index], 0,
                   sizeof(PING_TYPE_PingResultsEntry_T));

            if ((res = PING_MGR_FreeWorkSpace(list_elm.table_index)) != PING_TYPE_OK)
            {
                return res;
            }
            if ((res = PING_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_pingCtlAdminStatus_disabled)) != PING_TYPE_OK)
            {
                return res;
            }
            PING_OM_ClearProbHistoryEntry(list_elm.table_index);

            if (!L_SORT_LST_Delete(&sort_ping_list, &list_elm))
            {
                return PING_TYPE_FAIL;
            }
            /*
             * If the ping control table is full before,
             * set the next_available_index to the index just freed
             */
            if((next_available_index == PING_TYPE_INVALID_WORK_SPACE)
                    && (current_control_entry == SYS_ADPT_MAX_PING_NUM))
            {
                next_available_index = list_elm.table_index;
            }

            current_control_entry--;
            break;

        case L_RSTATUS_NOTREADY_2_NOTREADY:
        case L_RSTATUS_ACTIVE_2_NOTREADY:

            ctrl_entry_p->ping_ctl_rowstatus = row_status;

            if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
            {
                ctrl_entry_p->ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
            }
            memcpy(&ping_control_table[list_elm.table_index], ctrl_entry_p,
                   sizeof(PING_TYPE_PingCtlEntry_T));
            break;

        case L_RSTATUS_NOTREADY_2_ACTIVE:

            ctrl_entry_p->ping_ctl_rowstatus = row_status;

            if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
            {
                ctrl_entry_p->ping_ctl_admin_status = VAL_pingCtlAdminStatus_disabled;
            }
            memcpy(&ping_control_table[list_elm.table_index], ctrl_entry_p,
                        sizeof(PING_TYPE_PingCtlEntry_T));

            if (admin_status_changed)
            {
                if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
                {
                    memcpy(&ping_control_table[list_elm.table_index], ctrl_entry_p,
                            sizeof(PING_TYPE_PingCtlEntry_T));

                    if ((res = PING_MGR_CreateWorkSpace (list_elm.table_index, ctrl_entry_p)) != PING_TYPE_OK)
                    {
                        return res;
                    }
                    PING_OM_ClearProbHistoryEntry(list_elm.table_index);
                    PING_OM_AssociateResultTable(list_elm.table_index, ctrl_entry_p);

                    if ((res = PING_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_pingCtlAdminStatus_enabled)) != PING_TYPE_OK)
                    {
                        return res;
                    }
                }
                /*else  from notready_2_active, it's impossible to disassociate.*/
            }

            break;

        case L_RSTATUS_ACTIVE_2_ACTIVE:

            ctrl_entry_p->ping_ctl_rowstatus = row_status;

            if (admin_status_changed)
            {
                if (ctrl_entry_p->ping_ctl_admin_status == VAL_pingCtlAdminStatus_enabled)
                {
                    if ((res = PING_MGR_CreateWorkSpace (list_elm.table_index, ctrl_entry_p)) != PING_TYPE_OK)
                        return res;

                    PING_OM_ClearProbHistoryEntry(list_elm.table_index);
                    PING_OM_AssociateResultTable(list_elm.table_index, ctrl_entry_p);

                    if ((res = PING_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_pingCtlAdminStatus_enabled)) != PING_TYPE_OK)
                    {
                        return res;
                    }
                }
                else
                {
                    PING_OM_DisAssociateResultTable(list_elm.table_index);

                    if ((res = PING_MGR_FreeWorkSpace(list_elm.table_index)) != PING_TYPE_OK)
                    {
                        return res;
                    }
                    if ((res = PING_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_pingCtlAdminStatus_disabled)) != PING_TYPE_OK)
                    {
                        return res;
                    }
                }
            }

            memcpy(&ping_control_table[list_elm.table_index], ctrl_entry_p,
                    sizeof(PING_TYPE_PingCtlEntry_T));

            break;

        default:
            return PING_TYPE_FAIL;

    } /* end of switch */

    return PING_TYPE_OK;
} /* end of PING_OM_SetCtlEntry() */

/* FUNCTION NAME: PING_OM_AppendProbePacketResult
 * PURPOSE:
 *          To append specific prob histroy entry to the end of the last histroy entry
 * INPUT:
 *          table_index           -- A key index to identify prob history entry for different
 *                                   owner_index and test name.
 *          prob_history_entry_p  -- contains values to set to prob history entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_AppendProbePacketResult(UI32_T table_index, PING_TYPE_PingProbeHistoryEntry_T *prob_packet_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T current_index;
    UI32_T original_priority;

    /* BODY
     */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);
    current_index = prob_history_index_table[table_index].current_prob_history;

    prob_history_index_table[table_index].prob_history_max_index++;
    prob_packet_entry_p->ping_probe_history_index = prob_history_index_table[table_index].prob_history_max_index;
    memcpy(&ping_prob_history_table[current_index], prob_packet_entry_p,
           sizeof(PING_TYPE_PingProbeHistoryEntry_T));

    if (++current_index > prob_history_index_table[table_index].prob_history_end)
    {
        prob_history_index_table[table_index].current_prob_history =
            prob_history_index_table[table_index].prob_history_head;
    }
    else
    {
        prob_history_index_table[table_index].current_prob_history++;
    }

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
    return PING_TYPE_OK;

} /* end of PING_OM_AppendProbePacketResult() */

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
UI32_T PING_OM_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T res;
    UI32_T original_priority;

    /* BODY
     */

    if (prob_history_entry_p == NULL)
    {
        return PING_TYPE_INVALID_ARG;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);


    res = PING_OM_Internal_GetProbeHistoryEntry(prob_history_entry_p);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
    return res;

} /* end of PING_OM_GetProbeHistoryEntry() */

/* FUNCTION NAME: PING_OM_Internal_GetProbeHistoryEntry
 * PURPOSE:
 *          To get the specified available prob history entry, withoug critical section protection.
 *
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
 *      1. This function doesn't implement critical section protection.
 */
static UI32_T PING_OM_Internal_GetProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T local_elm;
    UI32_T  i;

    /* BODY
     */
    memset(&local_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(local_elm.owner_index, prob_history_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(local_elm.test_name, prob_history_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if (!L_SORT_LST_Get(&sort_ping_list, &local_elm))
    {
        return PING_TYPE_FAIL;
    }

    for(i= prob_history_index_table[local_elm.table_index].prob_history_head; i<=prob_history_index_table[local_elm.table_index].prob_history_end; i++)
    {
        if(ping_prob_history_table[i].ping_probe_history_index == prob_history_entry_p->ping_probe_history_index)
        {
            memcpy(prob_history_entry_p, &ping_prob_history_table[i],
                           sizeof(PING_TYPE_PingProbeHistoryEntry_T));

            return PING_TYPE_OK;
        }
    }

    return PING_TYPE_FAIL;
}

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
 *          None.
 */
UI32_T PING_OM_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  original_priority;
    UI32_T  ret;

    /* BODY
     */
    if (prob_history_entry_p == NULL)
    {
        return PING_TYPE_INVALID_ARG;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);

    ret = PING_OM_Internal_GetNextProbeHistoryEntry(prob_history_entry_p, FALSE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);

    return ret;
}

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
UI32_T PING_OM_GetNextProbeHistoryEntryForCli(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  original_priority;
    UI32_T  ret;

    /* BODY
     */
    if (prob_history_entry_p == NULL)
    {
        return PING_TYPE_INVALID_ARG;
    }

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);

    ret = PING_OM_Internal_GetNextProbeHistoryEntry(prob_history_entry_p, TRUE);

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);

    return ret;
}

/* FUNCTION NAME: PING_OM_Internal_GetNextProbeHistoryEntry
 * PURPOSE:
 *          To get next available prob history entry
 * INPUT:
 *          prob_history_entry_p    -- the specific probe history entry, including keys:
 *                                     ping_ctl_owner_index, ping_ctl_test_name, ping_probe_history_index.
 *          is_fix_ctl_entry        -- if TRUE, fix ping_ctl_owner_index, ping_ctl_test_name
 *
 * OUTPUT:
 *          prob_history_entry_p    -- next available entry that contains prob packet information
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_PROCESS_NOT_COMPLETE
 *          PING_TYPE_FAIL
 * NOTES:
 *          None.
 */
static UI32_T PING_OM_Internal_GetNextProbeHistoryEntry(PING_TYPE_PingProbeHistoryEntry_T *prob_history_entry_p, BOOL_T is_fix_ctl_entry)
{
    UI32_T  i, target_history_index;
    I32_T   found_index;
    BOOL_T  ctl_entry_exist;
    PING_SORTLST_ELM_T    local_elm;

    if (prob_history_entry_p == NULL)
    {
        return PING_TYPE_INVALID_ARG;
    }

    memset(&local_elm, 0, sizeof(PING_SORTLST_ELM_T));
    memcpy(local_elm.owner_index, prob_history_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
    memcpy(local_elm.test_name, prob_history_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE);

    if(FALSE == (ctl_entry_exist = L_SORT_LST_Get(&sort_ping_list, &local_elm)))
    {
        if (is_fix_ctl_entry)
            return PING_TYPE_FAIL;

        ctl_entry_exist = L_SORT_LST_Get_Next(&sort_ping_list, &local_elm);
    }

    target_history_index = prob_history_entry_p->ping_probe_history_index;
    while (ctl_entry_exist)
    {
        found_index = -1;

        /* we want to find the smallest ping_probe_history_index which is larger than target_history_index
         */
        for(i= prob_history_index_table[local_elm.table_index].prob_history_head; i<=prob_history_index_table[local_elm.table_index].prob_history_end; i++)
        {
            if (ping_prob_history_table[i].ping_probe_history_index == 0)
                continue;

            if(ping_prob_history_table[i].ping_ctl_owner_index_len == 0)
                continue;

            if (ping_prob_history_table[i].ping_probe_history_index > target_history_index)
            {
                if (found_index == -1 ||
                    ping_prob_history_table[found_index].ping_probe_history_index > ping_prob_history_table[i].ping_probe_history_index)
                {
                    found_index = (I32_T)i;
                }
            }
        }

        if (found_index != -1)
        {
            memcpy(prob_history_entry_p, &ping_prob_history_table[found_index], sizeof(PING_TYPE_PingProbeHistoryEntry_T));
            return PING_TYPE_OK;
        }

        if (is_fix_ctl_entry)
            return PING_TYPE_NO_MORE_ENTRY;

        ctl_entry_exist = L_SORT_LST_Get_Next(&sort_ping_list, &local_elm);
        target_history_index = 0;
    }

    return PING_TYPE_NO_MORE_ENTRY;
}

/* FUNCTION NAME: PING_OM_SetResultsEntry
 * PURPOSE:
 *          To set specific ping result entry by the specific entry index
 * INPUT:
 *          key_index       -- the index of the result table.
 *          result_entry_p  -- the pointer of the result_entry.
 *
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
UI32_T PING_OM_SetResultsEntry(UI32_T key_index, PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */

    /* BODY
     */
    memcpy(&ping_result_table[key_index], result_entry_p, sizeof(PING_TYPE_PingResultsEntry_T));

    return PING_TYPE_OK;
} /* end of PING_OM_SetResultsEntry() */


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
UI32_T PING_OM_GetResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    local_elm;
    UI32_T res;
    UI32_T original_priority;

    /* BODY
     */
    /* DEBUG */
    /* printf("%s, %d, result_entry_p->ping_ctl_owner_index: %s\n", __FUNCTION__, __LINE__, result_entry_p->ping_ctl_owner_index);
    printf("%s, %d, result_entry_p->ping_ctl_test_name: %s\n", __FUNCTION__, __LINE__, result_entry_p->ping_ctl_test_name);
    printf("%s, %d, result_entry_p->ping_ctl_owner_index_len: %ld\n", __FUNCTION__, __LINE__, result_entry_p->ping_ctl_owner_index_len);
    printf("%s, %d, result_entry_p->ping_ctl_admin_status: %ld\n", __FUNCTION__, __LINE__, result_entry_p->ping_results_oper_status);
    printf("%s, %d, result_entry_p->ping_ctl_target_address: %d.%d.%d.%d\n", __FUNCTION__, __LINE__,
            result_entry_p->ping_results_ip_target_address[0],
            result_entry_p->ping_results_ip_target_address[1],
            result_entry_p->ping_results_ip_target_address[2],
            result_entry_p->ping_results_ip_target_address[3]);
    */

    if (result_entry_p == NULL)
        return PING_TYPE_FAIL;

    memset(&local_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(local_elm.owner_index,  result_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(local_elm.test_name, result_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);
    /* DEBUG */
    //printf("%s, %d, [Enter CS]\n", __FUNCTION__, __LINE__);

    if ((res = L_SORT_LST_Get(&sort_ping_list, &local_elm)) != TRUE)
    {
        /* DEBUG */
        //printf("%s, %d, [Leave CS]\n", __FUNCTION__, __LINE__);
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
        return PING_TYPE_FAIL;
    }
    memcpy(result_entry_p, &ping_result_table[local_elm.table_index], sizeof(PING_TYPE_PingResultsEntry_T));

    /* DEBUG */
    //printf("%s, %d, [Leave CS]\n", __FUNCTION__, __LINE__);
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
    return PING_TYPE_OK;

} /* end of PING_OM_GetResultsEntry() */



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
UI32_T PING_OM_GetFirstActiveResultsEntry(UI32_T *index_p, PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    /* BODY
     */
    if ((index_p == NULL) || (result_entry_p == NULL))
        return PING_TYPE_FAIL;

    for (local_index = 0; local_index < SYS_ADPT_MAX_PING_NUM; local_index++)
    {
        if (ping_result_table[local_index].ping_results_oper_status == VAL_pingResultsOperStatus_enabled)
        {
            *index_p = local_index;
            memcpy(result_entry_p, &ping_result_table[local_index], sizeof(PING_TYPE_PingResultsEntry_T));
            return PING_TYPE_OK;
        } /* end of if */
    } /* end of for */
    return PING_TYPE_FAIL;

} /* end of PING_OM_GetFirstActiveResultsEntry() */


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
UI32_T PING_OM_GetNextActiveResultsEntry(UI32_T *index_p, PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    /* BODY
     */

    if ((index_p == NULL) || (result_entry_p == NULL))
        return PING_TYPE_FAIL;

    local_index = *index_p;

    /* fix the problem: if index are 14 & 15 at the same time, both will always time-outed */
    /* if(++local_index >= SYS_ADPT_MAX_PING_NUM)
        local_index=0;
     */
    local_index++;

    for ( ;local_index < SYS_ADPT_MAX_PING_NUM; local_index++)
    {
        if (ping_result_table[local_index].ping_results_oper_status == VAL_pingResultsOperStatus_enabled)
        {
            *index_p = local_index;
            memcpy(result_entry_p, &ping_result_table[local_index], sizeof(PING_TYPE_PingResultsEntry_T));
            return PING_TYPE_OK;
        } /* end of if */
    } /* end of for */
    return PING_TYPE_NO_MORE_ENTRY;
} /* end of PING_OM_GetNextActiveResultsEntry() */

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
UI32_T PING_OM_GetNextResultsEntry(PING_TYPE_PingResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    local_elm;
    BOOL_T      res;
    /* BODY
     */
    if (result_entry_p == NULL)
        return PING_TYPE_FAIL;

    memset(&local_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(local_elm.owner_index,  result_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(local_elm.test_name, result_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    while((res = L_SORT_LST_Get_Next(&sort_ping_list, &local_elm)))
    {
        if((ping_result_table[local_elm.table_index].ping_ctl_owner_index_len != 0) &&
               (ping_result_table[local_elm.table_index].ping_ctl_test_name_len != 0))
        {
            break;
        }
    }

    /* If there's no any result entry in the table. */
    if(res == FALSE)
    {
        return PING_TYPE_FAIL;
    }
    memcpy(result_entry_p, &ping_result_table[local_elm.table_index], sizeof(PING_TYPE_PingResultsEntry_T));
    return PING_TYPE_OK;

} /* end of PING_OM_GetNextResultsEntry() */

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
UI32_T PING_OM_IsPingCtlEntryExist(PING_TYPE_PingCtlEntry_T *ctl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    list_elm;
    UI32_T      res;

    /* BODY
     */
    if ((ctl_entry_p->ping_ctl_owner_index == NULL) || (ctl_entry_p->ping_ctl_test_name == NULL))
    {
        return PING_TYPE_INVALID_ARG;
    }

    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(list_elm.owner_index, ctl_entry_p->ping_ctl_owner_index,  SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(list_elm.test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_FindKeyIndex(&list_elm)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }
    return PING_TYPE_OK;

} /* end of PING_OM_IsPingCtlEntryExist() */

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
UI32_T PING_OM_PingKeyToTableIndex(PING_SORTLST_ELM_T *elm_p, UI32_T *table_index_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T      res;

    /* BODY
     */
    if ((elm_p == NULL) || (table_index_p == NULL))
        return PING_TYPE_INVALID_ARG;

    if ((res = PING_OM_FindKeyIndex(elm_p)) != PING_TYPE_OK)
        return PING_TYPE_FAIL;

    *table_index_p = elm_p->table_index;

    return PING_TYPE_OK;

} /* end of PING_OM_PingKeyToTableIndex() */


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
UI32_T PING_OM_PingTableIndexToKey(UI32_T table_index, PING_SORTLST_ELM_T *elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    list_elm;

    /* BODY
     */
    if (elm_p == NULL)
    {
        return PING_TYPE_INVALID_ARG;
    }
    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));
    if (L_SORT_LST_Get_1st(&sort_ping_list, &list_elm))
    {
        if (list_elm.table_index == table_index)
        {
            memcpy(elm_p, &list_elm, sizeof(PING_SORTLST_ELM_T));
            return PING_TYPE_OK;
        } /* end of if */
    }


    while (L_SORT_LST_Get_Next(&sort_ping_list, &list_elm))
    {
        if (list_elm.table_index == table_index)
        {
            memcpy(elm_p, &list_elm, sizeof(PING_SORTLST_ELM_T));
            return PING_TYPE_OK;
        } /* end of if */
    } /* end of while */

    return PING_TYPE_FAIL;

} /* end of PING_OM_PingKeyToTableIndex() */

#if 0
/* FUNCTION NAME: PING_OM_SetCtlEntryByField
 * PURPOSE:
 *          Set only the field of the entry.
 * INPUT:
 *          ctl_entry_p -- the pointer of the specified ctl entry.
 *          field       -- field.
 * OUTPUT:
 *          None.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_FAIL
 *          PING_TYPE_INVALID_ARG
 * NOTES:
 *          1. set only the field of the entry.
 */
UI32_T PING_OM_SetCtlEntryByField(PING_TYPE_PingCtlEntry_T *ctl_entry_p, PING_TYPE_CtlEntryField_T field)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    list_elm;
    UI32_T      res;

    /* BODY
     */
    memset(&list_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(list_elm.owner_index, ctl_entry_p->ping_ctl_owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1);
    memcpy(list_elm.test_name, ctl_entry_p->ping_ctl_test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1);

    if ((res = PING_OM_FindKeyIndex(&list_elm)) != PING_TYPE_OK)
    {
        return PING_TYPE_FAIL;
    }

    /* DEBUG */
    if(TRUE == ping_backdoor_debug)
    {
        BACKDOOR_MGR_Printf("%s, field:%d\n", __FUNCTION__, field);
    }

    switch (field)
    {
        case PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS_TYPE:
            ping_control_table[list_elm.table_index].ping_ctl_target_address_type = ctl_entry_p->ping_ctl_target_address_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_TARGET_ADDRESS:
            ping_control_table[list_elm.table_index].ping_ctl_target_address =  ctl_entry_p->ping_ctl_target_address;
            break;
        case PING_TYPE_CTLENTRYFIELD_DATA_SIZE:
            ping_control_table[list_elm.table_index].ping_ctl_data_size = ctl_entry_p->ping_ctl_data_size;
            break;
        case PING_TYPE_CTLENTRYFIELD_TIMEOUT:
            ping_control_table[list_elm.table_index].ping_ctl_timeout = ctl_entry_p->ping_ctl_timeout;
            break;
        case PING_TYPE_CTLENTRYFIELD_PROBE_COUNT:
            ping_control_table[list_elm.table_index].ping_ctl_probe_count = ctl_entry_p->ping_ctl_probe_count;
            break;
        case PING_TYPE_CTLENTRYFIELD_ADMIN_STATUS:
            ping_control_table[list_elm.table_index].ping_ctl_admin_status = ctl_entry_p->ping_ctl_admin_status;
            break;
        case PING_TYPE_CTLENTRYFIELD_DATA_FILL:
            memcpy(ping_control_table[list_elm.table_index].ping_ctl_data_fill, ctl_entry_p->ping_ctl_data_fill, sizeof(ctl_entry_p->ping_ctl_data_fill));
            break;
        case PING_TYPE_CTLENTRYFIELD_FREQUENCY:
            ping_control_table[list_elm.table_index].ping_ctl_admin_status = ctl_entry_p->ping_ctl_admin_status;
            break;
        case PING_TYPE_CTLENTRYFIELD_MAX_ROWS:
            ping_control_table[list_elm.table_index].ping_ctl_max_rows = ctl_entry_p->ping_ctl_max_rows;
            break;
        case PING_TYPE_CTLENTRYFIELD_STORAGE_TYPE:
            ping_control_table[list_elm.table_index].ping_ctl_storage_type = ctl_entry_p->ping_ctl_storage_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_GENERATION:
            ping_control_table[list_elm.table_index].ping_ctl_trap_generation = ctl_entry_p->ping_ctl_trap_generation;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_PROBE_FAILURE_FILTER:
            ping_control_table[list_elm.table_index].ping_ctl_trap_probe_failure_filter = ctl_entry_p->ping_ctl_trap_probe_failure_filter;
            break;
        case PING_TYPE_CTLENTRYFIELD_TRAP_TEST_FAILURE_FILTER:
            ping_control_table[list_elm.table_index].ping_ctl_trap_test_failure_filter = ctl_entry_p->ping_ctl_trap_test_failure_filter;
            break;
        case PING_TYPE_CTLENTRYFIELD_TYPE:
            ping_control_table[list_elm.table_index].ping_ctl_type= ctl_entry_p->ping_ctl_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_DESCR:
            memcpy(ping_control_table[list_elm.table_index].ping_ctl_descr, ctl_entry_p->ping_ctl_descr, sizeof(ctl_entry_p->ping_ctl_descr));
            break;
        case PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS_TYPE:
            ping_control_table[list_elm.table_index].ping_ctl_source_address_type = ctl_entry_p->ping_ctl_source_address_type;
            break;
        case PING_TYPE_CTLENTRYFIELD_SOURCE_ADDRESS:
            ping_control_table[list_elm.table_index].ping_ctl_source_address = ctl_entry_p->ping_ctl_source_address;
            break;
        case PING_TYPE_CTLENTRYFIELD_IF_INDEX:
            ping_control_table[list_elm.table_index].ping_ctl_if_index = ctl_entry_p->ping_ctl_if_index;
            break;
        case PING_TYPE_CTLENTRYFIELD_BY_PASS_ROUTE_TABLE:
            ping_control_table[list_elm.table_index].ping_ctl_by_pass_route_table = ctl_entry_p->ping_ctl_by_pass_route_table;
            break;
        case PING_TYPE_CTLENTRYFIELD_DS_FIELD:
            ping_control_table[list_elm.table_index].ping_ctl_ds_field = ctl_entry_p->ping_ctl_ds_field;
            break;
        /* case PING_TYPE_CTLENTRYFIELD_ROWSTATUS:
            ping_control_table[list_elm.table_index].ping_ctl_ds_field = ctl_entry_p->ping_ctl_ds_field;
            break;
           should use PING_OM_SetCtlEntry
        */
        default :
            if(TRUE == ping_backdoor_debug)
            {
                BACKDOOR_MGR_Printf("sorry, not support to change this field yet.\n");
            }
            return PING_TYPE_FAIL;
    }

    return PING_TYPE_OK;
}
#endif

/* FUNCTION NAME: PING_OM_PrintAllProbeHistory
 * PURPOSE:
 *          Print all probe history entry.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          None.
 * NOTES:
 *          1. For debug purpose.
 */
void PING_OM_PrintAllProbeHistory(void)
{
    int i;
    char tmp_buf[L_INET_MAX_IP6ADDR_STR_LEN+1] = {0};

    /* DEBUG */
    BACKDOOR_MGR_Printf("PING_OM_PrintAllPrintAllProbeHistory\n");
    BACKDOOR_MGR_Printf("status: \n" \
            "responseReceived(1)\n" \
            "unknown(2)\n" \
            "internalError(3)\n" \
            "requestTimedOut(4)\n" \
            "unknownDestinationAddress(5)\n" \
            "noRouteToTarget(6)\n" \
            "interfaceInactiveToTarget(7)\n" \
            "arpFailure(8)\n" \
            "maxConcurrentLimitReached(9)\n" \
            "unableToResolveDnsName(10)\n" \
            "invalidHostAddress(11)\n");

   /* the filed name string may would be truncated */
   BACKDOOR_MGR_Printf("%-5.5s %-11.11s %-9.9s %-7.7s %-12.12s %-6.6s %s\n", "index", "owner_index", "test_name", "h_index",
   "response(ms)", "status", "s_addr");

   for(i=0;i <PING_PROB_HISTORY_TABLE_SIZE-1; i++)
   {
        memset(tmp_buf, 0, sizeof(tmp_buf));
        if(ping_prob_history_table[i].ping_ctl_owner_index_len >0)
            L_INET_InaddrToString((L_INET_Addr_T *) &ping_prob_history_table[i].src_addr, tmp_buf, sizeof(tmp_buf));

        BACKDOOR_MGR_Printf("%-5.5d %-11.11s %-9.9s %-7ld %-12ld %-6ld %s\n",
        i,
        ping_prob_history_table[i].ping_ctl_owner_index,
        ping_prob_history_table[i].ping_ctl_test_name,
        (long)ping_prob_history_table[i].ping_probe_history_index,
        (long)ping_prob_history_table[i].ping_probe_history_response,
        (long)ping_prob_history_table[i].ping_probe_history_status,
        tmp_buf);
   }
}

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* FUNCTION NAME: PING_OM_Compare
 * PURPOSE:
 *          Compare function for ping_sort list.
 * INPUT:
 *          elm1 - Element to be compared
 *          elm2 - Element to be compared
 * OUTPUT:
 *          None.
 * RETURN:
 *          None.
 * NOTES:
 *          None
 */
static int PING_OM_Compare(void *elm1_p, void *elm2_p)
{
    PING_SORTLST_ELM_T    *element1, *element2;
    int r1, r2;
    if ((elm1_p == NULL) || (elm2_p == NULL))
        return PING_TYPE_INVALID_ARG;

    element1 = (PING_SORTLST_ELM_T *)elm1_p;
    element2 = (PING_SORTLST_ELM_T *)elm2_p;

    r1 = memcmp(element1->owner_index, element2->owner_index, SYS_ADPT_PING_MAX_NAME_SIZE);
    r2 = memcmp(element1->test_name, element2->test_name, SYS_ADPT_PING_MAX_NAME_SIZE);

    /* Both owner index and test_name are identical.
     */

    if ((r1 == 0) && (r2 == 0))
        return 0;
    else if (r1 == 0)
        return r2;
    else
        return r1;

} /* end of PING_OM_Compare() */


/* FUNCTION NAME: PING_OM_InitProbHistoryIndexTable
 * PURPOSE:
 *          Init Prob history index Table.  Map each entry position to
 *          the correct prob_history_table index.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          None.
 * NOTES:
 *          None
 */
static void PING_OM_InitProbHistoryIndexTable(void)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    UI32_T  head_index, tail_index;

    /* BODY
     */
    head_index = tail_index = 0;

    for (local_index= 0; local_index < SYS_ADPT_MAX_PING_NUM; local_index++)
    {
        head_index = local_index * SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY;
        tail_index = head_index + SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY - 1;

        prob_history_index_table[local_index].prob_history_head = head_index;
        prob_history_index_table[local_index].prob_history_end = tail_index;
        prob_history_index_table[local_index].current_prob_history = head_index;
        prob_history_index_table[local_index].prob_history_max_index = 0;

    } /* end of for */

    return;
} /* end of PING_OM_InitProbHistoryIndexTable() */

/* FUNCTION NAME: PING_OM_FindKeyIndex
 * PURPOSE:
 *          Find the table index of the specific entry.
 * INPUT:
 *          list_elm_p->owner_index - owner index is the task name
 *          list_elm_p->test_name - the specific test session name
 *
 * OUTPUT:
 *          list_elm_p->table_index - index to all tables that stores
 *          control info, result info, and prob history info.
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          None
 */
static UI32_T PING_OM_FindKeyIndex(PING_SORTLST_ELM_T *list_elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    PING_SORTLST_ELM_T    local_elm;
    /* BODY
     */
    if (list_elm_p == NULL)
        return PING_TYPE_FAIL;

    memset(&local_elm, 0, sizeof(PING_SORTLST_ELM_T));

    if (L_SORT_LST_Get_1st(&sort_ping_list, &local_elm))
    {
        if ((!memcmp(local_elm.owner_index, list_elm_p->owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1)) &&
            (!memcmp(local_elm.test_name, list_elm_p->test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1)))
        {
            list_elm_p->table_index = local_elm.table_index;
            return PING_TYPE_OK;
        }
    }

    while (L_SORT_LST_Get_Next(&sort_ping_list, &local_elm))
    {
        if ((!memcmp(local_elm.owner_index, list_elm_p->owner_index, SYS_ADPT_PING_MAX_NAME_SIZE + 1)) &&
            (!memcmp(local_elm.test_name, list_elm_p->test_name, SYS_ADPT_PING_MAX_NAME_SIZE + 1)))
        {
            list_elm_p->table_index = local_elm.table_index;
            return PING_TYPE_OK;
        } /* end of if */

    } /* end of while */

    return PING_TYPE_FAIL;
} /* end of PING_OM_FindKeyIndex() */

/* FUNCTION NAME: PING_OM_SemanticCheck
 * PURPOSE:
 *          Validate the given control entry fields before save it
 *          to control table.
 * INPUT:
 *          ctrl_entry_p - the control entry information to be validate
 *
 * OUTPUT:
 *          None
 * RETURN:
 *          TRUE  FALSE
 * NOTES:
 *          None
 */
static BOOL_T PING_OM_SemanticCheck(void  *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */

    /* BODY
     */

    return TRUE;
} /* end of PING_OM_SemanticCheck() */

/* FUNCTION NAME: PING_OM_ClearProbHistoryEntry
 * PURPOSE:
 *          Clear Prob history table as well as rest index counters
 *          in prob history index table.
 * INPUT:
 *          table_index - table index to be reset.
 * OUTPUT:
 *          None
 * RETURN:
 *          None.
 * NOTES:
 *          None
 */
static void PING_OM_ClearProbHistoryEntry(UI32_T table_index)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T start_index;
    UI32_T original_priority;

    /* BODY
     */
    original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(ping_om_sem_id);

    if (table_index > SYS_ADPT_MAX_PING_NUM)
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
        return;
    } /* end of if */

    start_index = prob_history_index_table[table_index].prob_history_head;

    memset(&ping_prob_history_table[start_index], 0,
           sizeof(PING_TYPE_PingProbeHistoryEntry_T) * SYS_ADPT_PING_MAX_NBR_OF_PROB_HISTORY_ENTRY);

    prob_history_index_table[table_index].current_prob_history = start_index;
    prob_history_index_table[table_index].prob_history_max_index = 0;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(ping_om_sem_id, original_priority);
    return;

} /* end of PING_OM_ClearProbHistoryEntry() */


/* FUNCTION NAME: PING_OM_AssociateResultTable
 * PURPOSE:
 *          Sync entry from result table with the control entry.
 * INPUT:
 *          table_index -- table index to be synchronize.
 *          ctr_entry   -- ctrl entry information to be sync.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          Need to sync the following three read-only fields from control entry.
 *          1. ctrl_entry.ping_ctl_source_address_type
 *          2. ctrl_entry.ping_ctl_target_address
 *          3. ctrl_entry.ping_ctl_admin_status
 */
static void PING_OM_AssociateResultTable(UI32_T table_index, PING_TYPE_PingCtlEntry_T *ctrl_entry_p)
{
    /* BODY
     */
    /* clear result entry */
    memset(&ping_result_table[table_index], 0, sizeof(PING_TYPE_PingResultsEntry_T));
    memcpy(ping_result_table[table_index].ping_ctl_owner_index,ctrl_entry_p->ping_ctl_owner_index,ctrl_entry_p->ping_ctl_owner_index_len);
    ping_result_table[table_index].ping_ctl_owner_index_len = ctrl_entry_p->ping_ctl_owner_index_len;
    memcpy(ping_result_table[table_index].ping_ctl_test_name,ctrl_entry_p->ping_ctl_test_name,ctrl_entry_p->ping_ctl_test_name_len);
    ping_result_table[table_index].ping_ctl_test_name_len = ctrl_entry_p->ping_ctl_test_name_len;
    ping_result_table[table_index].ping_results_oper_status = VAL_pingResultsOperStatus_enabled;
    ping_result_table[table_index].ping_results_ip_target_address_type = ctrl_entry_p->ping_ctl_target_address_type;

    memcpy(&ping_result_table[table_index].ping_results_ip_target_address, &ctrl_entry_p->ping_ctl_target_address, sizeof(L_INET_AddrIp_T));

    return;

} /* end of PING_OM_AssociateResultTable() */

/* FUNCTION NAME: PING_OM_DisAssociateResultTable
 * PURPOSE:
 *          Sync entry from result table with the control entry.
 * INPUT:
 *          table_index -- table index to be synchronize.
 *          ctr_entry   -- ctrl entry information to be sync.
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 *          Need to sync the following three read-only fields from control entry.
 *          1. ctrl_entry.ping_ctl_source_address_type
 *          2. ctrl_entry.ping_ctl_target_address
 *          3. ctrl_entry.ping_ctl_admin_status
 */
static void PING_OM_DisAssociateResultTable(UI32_T table_index)
{
    /* BODY
     */
    ping_result_table[table_index].ping_results_oper_status = VAL_pingResultsOperStatus_disabled;

    return;

} /* end of PING_OM_DisAssociateResultTable() */



/* FUNCTION NAME: PING_OM_CreateNewCtrlEntry
 * PURPOSE:
 *          Create a new element in to the sorted list and create a new control entry in the table.
 * INPUT:
 *          ctrl_entry - control entry info to be stored
 * OUTPUT:
 *          None
 * RETURN:
 *          PING_TYPE_OK
 *          PING_TYPE_NO_MORE_ENTRY
 *          PING_TYPE_FAIL
 * NOTES:
 */
static UI32_T PING_OM_CreateNewCtrlEntry(PING_TYPE_PingCtlEntry_T ctrl_entry)
{
    PING_SORTLST_ELM_T    new_elm;
    BOOL_T      next_index_found = FALSE;
    /* BODY
     */
    if ((ctrl_entry.ping_ctl_owner_index == NULL) || (ctrl_entry.ping_ctl_test_name == NULL) ||
        (ctrl_entry.ping_ctl_owner_index_len <= 0) || (ctrl_entry.ping_ctl_owner_index_len > SYS_ADPT_PING_MAX_NAME_SIZE) ||
        (ctrl_entry.ping_ctl_test_name_len <= 0) || (ctrl_entry.ping_ctl_test_name_len > SYS_ADPT_PING_MAX_NAME_SIZE))
    {
        return PING_TYPE_INVALID_ARG;
    }

    /*  reach max control entry limit.
     */
    if (current_control_entry >= SYS_ADPT_MAX_PING_NUM)
    {
        return PING_TYPE_FAIL;
    }

    memset(&new_elm, 0, sizeof(PING_SORTLST_ELM_T));

    memcpy(new_elm.owner_index, ctrl_entry.ping_ctl_owner_index, ctrl_entry.ping_ctl_owner_index_len);
    memcpy(new_elm.test_name, ctrl_entry.ping_ctl_test_name, ctrl_entry.ping_ctl_test_name_len);
    new_elm.table_index = next_available_index;

    if (L_SORT_LST_Set(&sort_ping_list, &new_elm) != TRUE)
    {
        return PING_TYPE_FAIL;
    }
    memcpy(&ping_control_table[new_elm.table_index], &ctrl_entry,
               sizeof(PING_TYPE_PingCtlEntry_T));

    ++current_control_entry;

    while(!next_index_found)
    {
        if (++next_available_index >= SYS_ADPT_MAX_PING_NUM)
        {
            next_available_index = 0;
        }
        if (next_available_index != new_elm.table_index)
        {
            if (ping_control_table[next_available_index].ping_ctl_owner_index[0] == 0)
            {
                next_index_found = TRUE;
            }
        }
        else
        {
            next_available_index = PING_TYPE_INVALID_WORK_SPACE;
            /* can not find the nex available index, control table is full */
            return PING_TYPE_OK;
        }
    } /* end of while */

    return PING_TYPE_OK;

} /* end of PING_OM_DisAssociateResultTable() */


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
BOOL_T PING_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    PING_OM_IPCMsg_T *ping_om_msg_p;
    BOOL_T           need_respond;

    if(ipcmsg_p==NULL)
        return FALSE;

    ping_om_msg_p= (PING_OM_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(ping_om_msg_p->type.cmd)
    {
        case PING_OM_IPCCMD_GETCTLENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetCtlEntry(&(ping_om_msg_p->data.ctrl_entry));
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(ctrl_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETNEXTCTLENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetNextCtlEntry(&(ping_om_msg_p->data.ctrl_entry));
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(ctrl_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETPROBEHISTORYENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetProbeHistoryEntry(&(ping_om_msg_p->data.probe_history_entry));
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetNextProbeHistoryEntry(&(ping_om_msg_p->data.probe_history_entry));
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETNEXTPROBEHISTORYENTRYFORCLI:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetNextProbeHistoryEntryForCli(&(ping_om_msg_p->data.probe_history_entry));
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(probe_history_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETRESULTSENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetResultsEntry(&ping_om_msg_p->data.results_entry);
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(results_entry);
            need_respond = TRUE;
            break;

        case PING_OM_IPCCMD_GETNEXTRESULTSENTRY:
            ping_om_msg_p->type.result_ui32 = PING_OM_GetNextResultsEntry(&ping_om_msg_p->data.results_entry);
            ipcmsg_p->msg_size = PING_OM_GET_MSG_SIZE(results_entry);
            need_respond = TRUE;
            break;

        default:
            printf("%s(): Invalid cmd.\n", __FUNCTION__);
            /* Unknow command. There is no way to idntify whether this
             * ipc message need or not need a response. If we response to
             * a asynchronous msg, then all following synchronous msg will
             * get wrong responses and that might not easy to debug.
             * If we do not response to a synchronous msg, the requester
             * will be blocked forever. It should be easy to debug that
             * error.
             */
            need_respond=FALSE;
    }
    return need_respond;
}


