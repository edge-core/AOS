/* ------------------------------------------------------------------------
 * FILE NAME - NMTR_HIST.C
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             30/09/2010      new created
 *   Wakka             10/11/2016      db redesign
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2010
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "sys_type.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sys_dflt.h"
#include "sysfun.h"
#include "l_math.h"
#include "l_pt.h"
#include "l_sort_lst.h"
#include "l_rstatus.h"
#include "backdoor_mgr.h"

#if (SYS_CPNT_NMTR_HISTORY == TRUE)
#include "nmtr_hist.h"

/* NAMING CONSTANT DECLARARTIONS
 */
#define NMTR_HIST_CTRL_ENTRY_NUM        (SYS_ADPT_NMTR_HIST_MAX_NBR_OF_CTRL_ENTRY_PER_PORT * SYS_ADPT_TOTAL_NBR_OF_LPORT)
#define NMTR_HIST_CTRL_BUCKETS_NUM      (NMTR_TYPE_HIST_CTRL_BUCKETS_MAX - NMTR_TYPE_HIST_CTRL_BUCKETS_MIN + 1)
#define NMTR_HIST_PREVIOUS_ENTRY_NUM    (NMTR_HIST_CTRL_ENTRY_NUM * NMTR_HIST_CTRL_BUCKETS_NUM)
#define NMTR_HIST_INTERVAL_TICKS        (1 * SYS_BLD_TICKS_PER_SECOND)

/* Allocable indexes according to actual database size
 */
#define NMTR_HIST_DB_CTRL_INDEX_MIN     NMTR_TYPE_HIST_CTRL_INDEX_MIN
#define NMTR_HIST_DB_CTRL_INDEX_MAX     (NMTR_HIST_DB_CTRL_INDEX_MIN + NMTR_HIST_CTRL_ENTRY_NUM - 1)
#define NMTR_HIST_DB_CTRL_DATA_SOURCE_MIN   NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MIN
#define NMTR_HIST_DB_CTRL_DATA_SOURCE_MAX   (NMTR_HIST_DB_CTRL_DATA_SOURCE_MIN + SYS_ADPT_TOTAL_NBR_OF_LPORT - 1)

#define NMTR_HIST_DEBUG 1

/* TYPE DEFINITIONS
 */
typedef struct
{
    char *name;
    UI32_T interval;        /* by minutes */
    UI32_T buckets_requested;
} NMTR_TYPE_DefaultHistCtrlInfo_T;

typedef struct
{
    NMTR_TYPE_HistCtrlInfo_T *head;
    NMTR_TYPE_HistCtrlInfo_T *tail;
} NMTR_HIST_HistCtrlList_T;

typedef struct
{
    NMTR_TYPE_HistCtrlInfo_T *next;
    NMTR_TYPE_HistCtrlInfo_T *prev;
} NMTR_HIST_HistCtrlListNode_T;

/* MACRO DEFINITIONS
 */
#define NMTR_HIST_ENTER_CRITICAL_SECTION() \
    SYSFUN_ENTER_CRITICAL_SECTION(nmtr_hist_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define NMTR_HIST_LEAVE_CRITICAL_SECTION() \
    SYSFUN_LEAVE_CRITICAL_SECTION(nmtr_hist_sem_id)

#define NMTR_HIST_SET_CTRL_ENTRY_STATUS(ctrl_p, new_status) ( \
            NMTR_HIST_DEBUG_MSG("SET_STATUS: ctrl_idx:%lu, old_status:%lu, new_status:%lu", (ctrl_p)->ctrl_idx, (ctrl_p)->status, (new_status)), \
            ((ctrl_p)->status = (new_status)))

#define NMTR_HIST_CTRL_ENTRY(ctrl_idx) \
            (&nmtr_hist_ctrl_entry_buf[(ctrl_idx) - NMTR_HIST_DB_CTRL_INDEX_MIN])

#define NMTR_HIST_CTRL_ENTRY_BUF_IDX(ctrl_p) \
            (ctrl_p - nmtr_hist_ctrl_entry_buf)

#define NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p) \
            (&nmtr_hist_ctrl_entry_lst_node[NMTR_HIST_CTRL_ENTRY_BUF_IDX(ctrl_p)])

#define NMTR_HIST_CTRL_ENTRY_FREE_LST() \
            (&nmtr_hist_ctrl_entry_free_lst)

#define NMTR_HIST_CTRL_ENTRY_DS_LST(data_srouce) \
            (&nmtr_hist_ctrl_entry_ds_lst[(data_srouce)-1])

#define NMTR_HIST_CUR_ENTRY(ctrl_p) \
            (&nmtr_hist_cur_entry_buf[NMTR_HIST_CTRL_ENTRY_BUF_IDX(ctrl_p)])

#define NMTR_HIST_PREV_ENTRY(ctrl_p, entry_idx) \
            (&nmtr_hist_prev_entry_buf[NMTR_HIST_CTRL_ENTRY_BUF_IDX(ctrl_p)][(entry_idx)])

#define NMTR_HIST_PREV_ENTRY_FREE_IDX(ctrl_p) \
            (nmtr_hist_prev_entry_free_idx[NMTR_HIST_CTRL_ENTRY_BUF_IDX(ctrl_p)])

#define NMTR_HIST_TIMEOUT_TICKS(start_time, interval) \
            ((start_time) + (interval) * nmtr_hist_interval_ticks - 1)


#define NMTR_HIST_PRINT_MSG(fmt, ...) \
            BACKDOOR_MGR_Printf("%s:%d: " fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#if NMTR_HIST_DEBUG
#define NMTR_HIST_DEBUG_MSG(...) \
            (nmtr_hist_debug_msg ? NMTR_HIST_PRINT_MSG(__VA_ARGS__) : 0)
#define NMTR_HIST_ERROR_MSG(...) \
            (nmtr_hist_error_msg ? NMTR_HIST_PRINT_MSG(__VA_ARGS__) : 0)
#else
#define NMTR_HIST_DEBUG_MSG(...) ((void)0)
#define NMTR_HIST_ERROR_MSG(...) ((void)0)
#endif

/* LOCAL FUNCTIONS DECLARATIONS
 */
static BOOL_T NMTR_HIST_SetCtrlEntryField_(UI32_T ctrl_idx, UI32_T field_idx, void *data_p);
static BOOL_T NMTR_HIST_SetCtrlEntry_(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
static BOOL_T NMTR_HIST_DestroyCtrlEntryByIfindex_(UI32_T ifindex);
static int NMTR_HIST_CompareCtrlEntry(void *e1, void *e2);
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_CreateCtrlEntry(UI32_T ctrl_idx);
static void NMTR_HIST_DestroyCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
static void NMTR_HIST_UpdateCtrlEntryDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T data_source);
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetCtrlEntryPtr(UI32_T ctrl_idx);
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetNextCtrlEntryPtr(UI32_T ctrl_idx);
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(char *name, UI32_T data_source);
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(UI32_T ctrl_idx, UI32_T data_source);
static UI32_T NMTR_HIST_GetFreeCtrlIdx(void);
static BOOL_T NMTR_HIST_SetCtrlEntryBucketsGranted(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T buckets_granted);
static BOOL_T NMTR_HIST_ResetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
static BOOL_T NMTR_HIST_StartCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
static BOOL_T NMTR_HIST_StopCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);
static BOOL_T NMTR_HIST_CtrlEntryActiveCheck(void *rec);
static BOOL_T NMTR_HIST_CtrlEntryFsm(UI32_T ctrl_idx, UI32_T action);
static BOOL_T NMTR_HIST_UpdateCtrlEntryCounter(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, NMTR_TYPE_HistCounterInfo_T *counter_p);
static int NMTR_HIST_ComparePreviousEntry(void *e1, void *e2);
static BOOL_T NMTR_HIST_AddPreviousEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, NMTR_TYPE_HistSampleEntry_T *prev_entry_p);
static BOOL_T NMTR_HIST_RemovePreviousEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T num);
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetPreviousEntryPtr(UI32_T ctrl_idx, UI32_T sample_idx);
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetNextPreviousEntryPtr(UI32_T ctrl_idx, UI32_T sample_idx);
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetNextPreviousEntryPtrByCtrlIdx(UI32_T ctrl_idx, UI32_T sample_idx);
static BOOL_T NMTR_HIST_UpdatePreviousEntryCounter(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);


/* LOCAL VARIABLES DECLARATIONS
 */
static const NMTR_TYPE_DefaultHistCtrlInfo_T nmtr_hist_dflt_ctrl_entry[] = { SYS_DFLT_NMTR_HISTORY_CTRL_ENTRY };
static const UI32_T nmtr_hist_nbr_of_dflt_ctrl_entry = sizeof(nmtr_hist_dflt_ctrl_entry) / sizeof(*nmtr_hist_dflt_ctrl_entry);

static UI32_T nmtr_hist_sem_id;

static NMTR_TYPE_HistCtrlInfo_T nmtr_hist_ctrl_entry_buf[NMTR_HIST_CTRL_ENTRY_NUM];
static NMTR_HIST_HistCtrlListNode_T nmtr_hist_ctrl_entry_lst_node[NMTR_HIST_CTRL_ENTRY_NUM];
static NMTR_HIST_HistCtrlList_T nmtr_hist_ctrl_entry_ds_lst[SYS_ADPT_TOTAL_NBR_OF_LPORT];
static NMTR_HIST_HistCtrlList_T nmtr_hist_ctrl_entry_free_lst;
static NMTR_TYPE_HistSampleEntry_T nmtr_hist_cur_entry_buf[NMTR_HIST_CTRL_ENTRY_NUM];
static NMTR_TYPE_HistSampleEntry_T nmtr_hist_prev_entry_buf[NMTR_HIST_CTRL_ENTRY_NUM][NMTR_HIST_CTRL_BUCKETS_NUM];
static UI32_T nmtr_hist_prev_entry_free_idx[NMTR_HIST_CTRL_ENTRY_NUM];

static UI32_T nmtr_hist_interval_ticks = NMTR_HIST_INTERVAL_TICKS;

static BOOL_T nmtr_hist_debug_msg = FALSE;
static BOOL_T nmtr_hist_error_msg = FALSE;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_Init
 *-----------------------------------------------------------------
 * FUNCTION: This function will init resouce for counter history
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_Init(void)
{
    if (SYSFUN_OK != SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &nmtr_hist_sem_id))
    {
        return FALSE;
    }

    memset(nmtr_hist_ctrl_entry_buf, 0, sizeof(nmtr_hist_ctrl_entry_buf));

    memset(nmtr_hist_ctrl_entry_lst_node, 0, sizeof(nmtr_hist_ctrl_entry_lst_node));
    memset(nmtr_hist_ctrl_entry_ds_lst, 0, sizeof(nmtr_hist_ctrl_entry_ds_lst));
    memset(&nmtr_hist_ctrl_entry_free_lst, 0, sizeof(nmtr_hist_ctrl_entry_free_lst));

    memset(nmtr_hist_cur_entry_buf, 0, sizeof(nmtr_hist_cur_entry_buf));

    memset(nmtr_hist_prev_entry_buf, 0, sizeof(nmtr_hist_prev_entry_buf));
    memset(nmtr_hist_prev_entry_free_idx, 0, sizeof(nmtr_hist_prev_entry_free_idx));

    /* init ctrl list
     */
    {
        NMTR_TYPE_HistCtrlInfo_T *next_ctrl_p, *prev_ctrl_p;
        UI32_T ctrl_idx;

        for (ctrl_idx = NMTR_HIST_DB_CTRL_INDEX_MIN; ctrl_idx < NMTR_HIST_DB_CTRL_INDEX_MAX; ctrl_idx++)
        {
            next_ctrl_p = NMTR_HIST_CTRL_ENTRY(ctrl_idx+1);
            prev_ctrl_p = NMTR_HIST_CTRL_ENTRY(ctrl_idx);

            NMTR_HIST_CTRL_ENTRY_LST_NODE(prev_ctrl_p)->next = next_ctrl_p;
            NMTR_HIST_CTRL_ENTRY_LST_NODE(next_ctrl_p)->prev = prev_ctrl_p;
        }

        next_ctrl_p = NMTR_HIST_CTRL_ENTRY(NMTR_HIST_DB_CTRL_INDEX_MIN);
        prev_ctrl_p = NMTR_HIST_CTRL_ENTRY(NMTR_HIST_DB_CTRL_INDEX_MAX);

        NMTR_HIST_CTRL_ENTRY_LST_NODE(prev_ctrl_p)->next = NULL;
        NMTR_HIST_CTRL_ENTRY_LST_NODE(next_ctrl_p)->prev = NULL;

        NMTR_HIST_CTRL_ENTRY_FREE_LST()->head = NMTR_HIST_CTRL_ENTRY(NMTR_HIST_DB_CTRL_INDEX_MIN);
        NMTR_HIST_CTRL_ENTRY_FREE_LST()->tail = NMTR_HIST_CTRL_ENTRY(NMTR_HIST_DB_CTRL_INDEX_MAX);
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_Reset
 *-----------------------------------------------------------------
 * FUNCTION: This function will clear database
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_Reset(void)
{
    return NMTR_HIST_DestroyAllCtrlEntry();
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetDefaultCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetDefaultCtrlEntry(UI32_T ifindex)
{
    NMTR_TYPE_HistCtrlInfo_T ctrl_info;
    int i;

    for (i = 0; i < nmtr_hist_nbr_of_dflt_ctrl_entry; i++)
    {
        memset(&ctrl_info, 0, sizeof(ctrl_info));
        strncpy(ctrl_info.name, nmtr_hist_dflt_ctrl_entry[i].name, sizeof(ctrl_info.name)-1);
        ctrl_info.interval = nmtr_hist_dflt_ctrl_entry[i].interval;
        ctrl_info.buckets_requested = nmtr_hist_dflt_ctrl_entry[i].buckets_requested;
        ctrl_info.data_source = ifindex;

        if (!NMTR_HIST_SetCtrlEntryByNameAndDataSrc(&ctrl_info))
        {
            NMTR_HIST_ERROR_MSG("NMTR_HIST_SetCtrlEntryByNameAndDataSrc failed: name:%s, data_source:%lu", ctrl_info.name, ctrl_info.data_source);
            return FALSE;
        }
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_AccumulateCounter
 *-----------------------------------------------------------------
 * FUNCTION: This function will calculate sum of two counters
 * INPUT   : counter_p
 *           new_counter_p
 * OUTPUT  : counter_p
 * RETURN  : TRUE/FALSE
 * NOTE    : *counter_p += new_counter_p
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_AccumulateCounter(NMTR_TYPE_HistCounterInfo_T *counter_p, NMTR_TYPE_HistCounterInfo_T *new_counter_p)
{
#define COUNTER_ADD(_field_) \
            counter_p->_field_ += new_counter_p->_field_

    COUNTER_ADD(interval);
    COUNTER_ADD(ifInOctets);
    COUNTER_ADD(ifInUcastPkts);
    COUNTER_ADD(ifInMulticastPkts);
    COUNTER_ADD(ifInBroadcastPkts);
    COUNTER_ADD(ifInDiscards);
    COUNTER_ADD(ifInErrors);
    COUNTER_ADD(ifInUnknownProtos);
    COUNTER_ADD(ifOutOctets);
    COUNTER_ADD(ifOutUcastPkts);
    COUNTER_ADD(ifOutMulticastPkts);
    COUNTER_ADD(ifOutBroadcastPkts);
    COUNTER_ADD(ifOutDiscards);
    COUNTER_ADD(ifOutErrors);
    COUNTER_ADD(ifInUtilization.xmit_bytes);
    COUNTER_ADD(ifOutUtilization.xmit_bytes);

    return TRUE;

#undef COUNTER_ADD
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_UpdateCtrlEntryCounterByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will update counter to ctrl entry
 * INPUT   : ifindex
 *           counter_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_UpdateCtrlEntryCounterByIfindex(UI32_T ifindex, NMTR_TYPE_HistCounterInfo_T *counter_p)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    for (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(0, ifindex);
        ctrl_p != NULL;
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_p->ctrl_idx, ifindex))
    {
        if (ctrl_p->status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE)
        {
            NMTR_HIST_UpdateCtrlEntryCounter(ctrl_p, counter_p);
        }
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_UpdateAllCtrlEntryCounter
 *-----------------------------------------------------------------
 * FUNCTION: This function will update counter to all ctrl entries
 * INPUT   : func      - callback function to retrieve diff counter for specified port.
 *                         return TRUE if diff counter is available
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_UpdateAllCtrlEntryCounter(NMTR_HIST_GetDiffCounter_Callback_T func)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    NMTR_TYPE_HistCounterInfo_T *counter_p;
    UI32_T ctrl_idx;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    ctrl_idx = 0;

    while (NULL != (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(ctrl_idx)))
    {
        ctrl_idx = ctrl_p->ctrl_idx;

        if (ctrl_p->status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE)
        {
            if (NULL == (counter_p = func(ctrl_p->data_source)))
            {
                continue;
            }

            NMTR_HIST_UpdateCtrlEntryCounter(ctrl_p, counter_p);
        }
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntryField
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data_p)
{
    BOOL_T ret;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    ret = NMTR_HIST_SetCtrlEntryField_(ctrl_idx, field_idx, data_p);

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    BOOL_T ret;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    ret = NMTR_HIST_SetCtrlEntry_(ctrl_p);

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : ctrl_p->ctrl_idx
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_TYPE_HistCtrlInfo_T *existed_ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (NULL != (existed_ctrl_p = NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(ctrl_p->name, ctrl_p->data_source)))
    {
        ctrl_p->ctrl_idx = existed_ctrl_p->ctrl_idx;
    }
    else
    {
        if (0 == (ctrl_p->ctrl_idx = NMTR_HIST_GetFreeCtrlIdx()))
        {
            NMTR_HIST_ERROR_MSG("NMTR_HIST_GetFreeCtrlIdx failed: name:%s, data_source:%lu", ctrl_p->name, ctrl_p->data_source);
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
        NMTR_HIST_DEBUG_MSG("name:%s, data_source:%lu, free_ctrl_idx:%lu", ctrl_p->name, ctrl_p->data_source, ctrl_p->ctrl_idx);
    }

    if (!NMTR_HIST_SetCtrlEntry_(ctrl_p))
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_PMGR_DestroyHistoryCtrlEntryByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy a ctrl entry without specified ctrl_idx
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_DestroyCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_TYPE_HistCtrlInfo_T *existed_ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (NULL != (existed_ctrl_p = NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(ctrl_p->name, ctrl_p->data_source)))
    {
        if (!NMTR_HIST_StopCtrlEntry(existed_ctrl_p))
        {
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        NMTR_HIST_DestroyCtrlEntry(existed_ctrl_p);
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_DestroyAllCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy all ctrl entries
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_DestroyAllCtrlEntry(void)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    UI32_T ctrl_idx;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    ctrl_idx = 0;

    while (NULL != (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(ctrl_idx)))
    {
        ctrl_idx = ctrl_p->ctrl_idx;

        if (!NMTR_HIST_StopCtrlEntry(ctrl_p))
        {
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        NMTR_HIST_DestroyCtrlEntry(ctrl_p);
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_DestroyCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_DestroyCtrlEntryByIfindex(UI32_T ifindex)
{
    BOOL_T ret;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    ret = NMTR_HIST_DestroyCtrlEntryByIfindex_(ifindex);

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNumOfCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will get number of ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : number of ctrl of entry
 * NOTE    : None.
 *----------------------------------------------------------------*/
UI32_T NMTR_HIST_GetNumOfCtrlEntryByIfindex(UI32_T ifindex)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    UI32_T count = 0;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    for (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(0, ifindex);
        ctrl_p != NULL;
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_p->ctrl_idx, ifindex))
    {
        count++;
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return count;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next)
{
    NMTR_TYPE_HistCtrlInfo_T *found_ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (get_next)
    {
        found_ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(ctrl_p->ctrl_idx);
    }
    else
    {
        found_ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_p->ctrl_idx);
    }
    if (found_ctrl_p == NULL)
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, get_next:%d, found: ctrl_idx:%lu", ctrl_p->ctrl_idx, get_next, found_ctrl_p->ctrl_idx);
    *ctrl_p = *found_ctrl_p;

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get a ctrl entry with specified data source
 * INPUT   : entry_p->ctrl_idx   - for get_next = TRUE
 *           ctrl_p->name        - for get_next = FALSE
 *           ctrl_p->data_source
 *           get_next
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next)
{
    NMTR_TYPE_HistCtrlInfo_T *found_ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (get_next)
    {
        if (NULL == (found_ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_p->ctrl_idx, ctrl_p->data_source)))
        {
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
        NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, data_source:%lu, found: ctrl_idx:%lu", ctrl_p->ctrl_idx, ctrl_p->data_source, found_ctrl_p->ctrl_idx);
    }
    else
    {
        if (NULL == (found_ctrl_p = NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(ctrl_p->name, ctrl_p->data_source)))
        {
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }
        NMTR_HIST_DEBUG_MSG("name:%s, data_source:%lu, found: ctrl_idx:%lu", ctrl_p->name, ctrl_p->data_source, found_ctrl_p->ctrl_idx);
    }

    *ctrl_p = *found_ctrl_p;

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetCurrentEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get current entry
 * INPUT   : entry_p->ctrl_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetCurrentEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (get_next)
    {
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(entry_p->ctrl_idx);
    }
    else
    {
        ctrl_p = NMTR_HIST_GetCtrlEntryPtr(entry_p->ctrl_idx);
    }
    if (ctrl_p == NULL)
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, get_next:%d, found: ctrl_idx:%lu", entry_p->ctrl_idx, get_next, ctrl_p->ctrl_idx);
    *entry_p = *NMTR_HIST_CUR_ENTRY(ctrl_p);

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetPreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will get previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx
 *           get_next
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetPreviousEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next)
{
    NMTR_TYPE_HistSampleEntry_T *found_entry_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (get_next)
    {
        found_entry_p = NMTR_HIST_GetNextPreviousEntryPtr(entry_p->ctrl_idx, entry_p->sample_idx);
    }
    else
    {
        found_entry_p = NMTR_HIST_GetPreviousEntryPtr(entry_p->ctrl_idx, entry_p->sample_idx);
    }
    if (found_entry_p == NULL)
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, sample_idx:%lu, get_next:%d, found: ctrl_idx:%lu, sample_idx:%lu", entry_p->ctrl_idx, entry_p->sample_idx, get_next, found_entry_p->ctrl_idx, found_entry_p->sample_idx);
    *entry_p = *found_entry_p;

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextPreviousEntryByCtrlIdx
 *-----------------------------------------------------------------
 * FUNCTION: This function will get next previous entry
 * INPUT   : entry_p->ctrl_idx
 *           entry_p->sample_idx - 0 to get first entry
 * OUTPUT  : entry_p
 * RETURN  : TRUE/FALSE
 * NOTE    : sort by timestamp
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetNextPreviousEntryByCtrlIdx(NMTR_TYPE_HistSampleEntry_T *entry_p)
{
    NMTR_TYPE_HistSampleEntry_T *found_entry_p;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    if (NULL == (found_entry_p = NMTR_HIST_GetNextPreviousEntryPtrByCtrlIdx(entry_p->ctrl_idx, entry_p->sample_idx)))
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }
    if (found_entry_p->ctrl_idx != entry_p->ctrl_idx)
    {
        NMTR_HIST_LEAVE_CRITICAL_SECTION();
        return FALSE;
    }

    NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, sample_idx:%lu, found: ctrl_idx:%lu, sample_idx:%lu", entry_p->ctrl_idx, entry_p->sample_idx, found_entry_p->ctrl_idx, found_entry_p->sample_idx);
    *entry_p = *found_entry_p;

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextRemovedDefaultCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get removed default ctrl entry
 * INPUT   : ctrl_p->name
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetNextRemovedDefaultCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    BOOL_T flag;
    int i;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    flag = ctrl_p->name[0] == 0;

    for (i = 0; i < nmtr_hist_nbr_of_dflt_ctrl_entry; i++)
    {
        if (flag)
        {
            if (NULL == NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(
                    nmtr_hist_dflt_ctrl_entry[i].name,
                    ctrl_p->data_source))
            {
                strncpy(ctrl_p->name, nmtr_hist_dflt_ctrl_entry[i].name, sizeof(ctrl_p->name)-1);
                ctrl_p->name[sizeof(ctrl_p->name)-1] = 0;
                ctrl_p->interval = nmtr_hist_dflt_ctrl_entry[i].interval;
                ctrl_p->buckets_requested = nmtr_hist_dflt_ctrl_entry[i].buckets_requested;

                NMTR_HIST_LEAVE_CRITICAL_SECTION();
                return TRUE;
            }
        }
        else
        {
            if (0 == strncmp(nmtr_hist_dflt_ctrl_entry[i].name, ctrl_p->name, sizeof(ctrl_p->name)))
            {
                flag = TRUE;
            }
        }
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextUserCfgCtrlEntryByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: This function will get user configured ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           ctrl_p->data_source
 * OUTPUT  : ctrl_p
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_GetNextUserCfgCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_TYPE_HistCtrlInfo_T *found_ctrl_p;
    int i;

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    found_ctrl_p = ctrl_p;

    while (NULL != (found_ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(found_ctrl_p->ctrl_idx, found_ctrl_p->data_source)))
    {
        for (i = 0; i < nmtr_hist_nbr_of_dflt_ctrl_entry; i++)
        {
            if (0 == strncmp(found_ctrl_p->name, nmtr_hist_dflt_ctrl_entry[i].name, sizeof(found_ctrl_p->name)) &&
                found_ctrl_p->interval == nmtr_hist_dflt_ctrl_entry[i].interval &&
                found_ctrl_p->buckets_requested == nmtr_hist_dflt_ctrl_entry[i].buckets_requested)
            {
                break;
            }
        }
        if (i == nmtr_hist_nbr_of_dflt_ctrl_entry)
        {
            *ctrl_p = *found_ctrl_p; 
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return TRUE;
        }
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return FALSE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_FollowAttribute
 *-----------------------------------------------------------------
 * FUNCTION: This function will copy config from another interface
 * INPUT   : to_ifindex
 *           from_ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_FollowAttribute(UI32_T to_ifindex, UI32_T from_ifindex)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    NMTR_HIST_DEBUG_MSG("to_ifindex:%lu, from_ifindex:%lu", to_ifindex, from_ifindex);

    NMTR_HIST_ENTER_CRITICAL_SECTION();

    NMTR_HIST_DestroyCtrlEntryByIfindex_(to_ifindex);

    for (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(0, from_ifindex);
        ctrl_p != NULL;
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_p->ctrl_idx, from_ifindex))
    {
        NMTR_TYPE_HistCtrlInfo_T *new_ctrl_p;
        UI32_T ctrl_idx;

        if (0 == (ctrl_idx = NMTR_HIST_GetFreeCtrlIdx()))
        {
            NMTR_HIST_ERROR_MSG("NMTR_HIST_GetFreeCtrlIdx failed: name:%s, data_source:%lu", ctrl_p->name, to_ifindex);
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        if (NULL == (new_ctrl_p = NMTR_HIST_CreateCtrlEntry(ctrl_idx)))
        {
            NMTR_HIST_ERROR_MSG("NMTR_HIST_CreateCtrlEntry failed: ctrl_idx:%lu", ctrl_idx);
            NMTR_HIST_LEAVE_CRITICAL_SECTION();
            return FALSE;
        }

        new_ctrl_p->interval = ctrl_p->interval;
        new_ctrl_p->buckets_requested = ctrl_p->buckets_requested;
        new_ctrl_p->status = ctrl_p->status;
        memcpy(new_ctrl_p->name, ctrl_p->name, sizeof(new_ctrl_p->name));

        NMTR_HIST_UpdateCtrlEntryDataSrc(new_ctrl_p, to_ifindex);

        NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu name:%s, data_source:%lu,", new_ctrl_p->ctrl_idx, new_ctrl_p->name, new_ctrl_p->data_source);

        if (new_ctrl_p->status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE)
        {
            if (!NMTR_HIST_StartCtrlEntry(new_ctrl_p))
            {
                NMTR_HIST_LEAVE_CRITICAL_SECTION();
                return FALSE;
            }
        }
    }

    NMTR_HIST_LEAVE_CRITICAL_SECTION();
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_Backdoor
 *-----------------------------------------------------------------
 * FUNCTION: backdoor
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_HIST_Backdoor(void)
{
    char buf[12];
    int ch;

    while (1)
    {
        BACKDOOR_MGR_Print("\r\n 0. Exit");
        BACKDOOR_MGR_Printf("\r\n 1. nmtr_hist_debug_msg: %d", nmtr_hist_debug_msg);
        BACKDOOR_MGR_Printf("\r\n 2. nmtr_hist_error_msg: %d", nmtr_hist_error_msg);
        BACKDOOR_MGR_Printf("\r\n 3. nmtr_hist_interval_ticks: %lu", nmtr_hist_interval_ticks);
        BACKDOOR_MGR_Print("\r\n Select> ");
        ch = BACKDOOR_MGR_GetChar();
        BACKDOOR_MGR_Printf("%c\r\n", isprint(ch) ? ch : '?');

        switch (ch)
        {
            case '0':
                return;
            case '1':
                nmtr_hist_debug_msg = !nmtr_hist_debug_msg;
                break;
            case '2':
                nmtr_hist_error_msg = !nmtr_hist_error_msg;
                break;
            case '3':
                BACKDOOR_MGR_Print("nmtr_hist_interval_ticks = ");
                BACKDOOR_MGR_RequestKeyIn(buf, sizeof(buf)-1);

                if (0 == (nmtr_hist_interval_ticks = atoi(buf)))
                {
                    nmtr_hist_interval_ticks = NMTR_HIST_INTERVAL_TICKS;
                }
                break;
        }
    }
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntryField_
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a field of ctrl entry
 * INPUT   : ctrl_idx
 *           field_idx
 *           data_p    - value of field
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_SetCtrlEntryField_(UI32_T ctrl_idx, UI32_T field_idx, void *data_p)
{
#define CHECK_RANGE(value, min, max) ((value) >= (min) && (value) <= (max))

    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    UI32_T action = L_RSTATUS_SET_OTHER_COLUMN;
    UI32_T value_int;
    char *value_str;
    BOOL_T field_updated = FALSE;
    BOOL_T ret = TRUE;

    if (ctrl_idx < NMTR_TYPE_HIST_CTRL_INDEX_MIN ||
        ctrl_idx > NMTR_TYPE_HIST_CTRL_INDEX_MAX)
    {
        NMTR_HIST_ERROR_MSG("wrong paramter: ctrl_idx:%lu, field_idx:%lu", ctrl_idx, field_idx);
        return FALSE;
    }

    value_int = L_CVRT_PTR_TO_UINT(data_p);
    value_str = data_p;

    if (field_idx == NMTR_TYPE_HIST_CTRL_FIELD_STATUS)
    {
        field_updated = TRUE;
        action = value_int;
    }
    else
    {
        if ((ret = (NULL != (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))))
        {
            switch (field_idx)
            {
                case NMTR_TYPE_HIST_CTRL_FIELD_NAME:
                    if ((ret = (value_str && *value_str)))
                    {
                        NMTR_TYPE_HistCtrlInfo_T *existed_ctrl_p;

                        if (NULL != (existed_ctrl_p = NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(value_str, ctrl_p->data_source)))
                        {
                            ret = (existed_ctrl_p->ctrl_idx == ctrl_p->ctrl_idx);
                            NMTR_HIST_DEBUG_MSG("ret:%lu, ctrl_idx:%lu, field_idx:%lu, value_str:%s", ret, ctrl_idx, field_idx, value_str);
                        }
                        else
                        {
                            field_updated = TRUE;
                            strncpy(ctrl_p->name, value_str, NMTR_TYPE_HIST_CTRL_NAME_LEN_MAX);
                            ctrl_p->name[NMTR_TYPE_HIST_CTRL_NAME_LEN_MAX] = 0;
                        }
                    }
                    break;
                case NMTR_TYPE_HIST_CTRL_FIELD_DATA_SOURCE:
                    if ((ret = CHECK_RANGE(value_int,
                            NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MIN,
                            NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MAX)))
                    {
                        field_updated = ctrl_p->data_source != value_int;

                        if (field_updated)
                        {
                            NMTR_HIST_UpdateCtrlEntryDataSrc(ctrl_p, value_int);
                        }
                    }
                    break;
                case NMTR_TYPE_HIST_CTRL_FIELD_INTERVAL:
                    if ((ret = CHECK_RANGE(value_int,
                            NMTR_TYPE_HIST_CTRL_INTERVAL_MIN,
                            NMTR_TYPE_HIST_CTRL_INTERVAL_MAX)))
                    {
                        field_updated = ctrl_p->interval != value_int;
                        ctrl_p->interval = value_int;
                    }
                    break;
                case NMTR_TYPE_HIST_CTRL_FIELD_BUCKETS_REQUESTED:
                    if ((ret = CHECK_RANGE(value_int,
                            NMTR_TYPE_HIST_CTRL_BUCKETS_MIN,
                            NMTR_TYPE_HIST_CTRL_BUCKETS_MAX)))
                    {
                        field_updated = ctrl_p->buckets_requested != value_int;
                        ctrl_p->buckets_requested = value_int;
                    }
                    break;
                default:
                    ret = FALSE;
            } /* end of switch (field_idx) */
        }
        else
        {
            NMTR_HIST_ERROR_MSG("ctrl_p is NULL: ctrl_idx:%lu, field_idx:%lu", ctrl_idx, field_idx);
        }
    } /* end of if (field_idx) */
    if (!ret)
    {
        NMTR_HIST_ERROR_MSG("update field failed: ctrl_idx:%lu, field_idx:%lu, field_updated:%d", ctrl_idx, field_idx, field_updated);
    }

    if (ret && field_updated)
    {
        if (!(ret = NMTR_HIST_CtrlEntryFsm(ctrl_idx, action)))
        {
            NMTR_HIST_ERROR_MSG("NMTR_HIST_CtrlEntryFsm failed: ctrl_idx:%lu, field_idx:%lu, field_updated:%d, action:%lu", ctrl_idx, field_idx, field_updated, action);
        }
    }

    if (ret && field_updated)
    {
        if (NULL != (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
        {
            switch (field_idx)
            {
                case NMTR_TYPE_HIST_CTRL_FIELD_DATA_SOURCE:
                case NMTR_TYPE_HIST_CTRL_FIELD_INTERVAL:
                    ret = NMTR_HIST_ResetCtrlEntry(ctrl_p);
                    break;

                case NMTR_TYPE_HIST_CTRL_FIELD_BUCKETS_REQUESTED:
                    if (ctrl_p->buckets_granted > ctrl_p->buckets_requested)
                    {
                        ret = NMTR_HIST_SetCtrlEntryBucketsGranted(ctrl_p, ctrl_p->buckets_requested);
                    }
                    break;
            } /* end of switch (field_idx) */
        }
        if (!ret)
        {
            NMTR_HIST_ERROR_MSG("postprocess failed: ctrl_idx:%lu, field_idx:%lu, field_updated:%d", ctrl_idx, field_idx, field_updated);
        }
    } /* end of if (field_updated) */

    NMTR_HIST_DEBUG_MSG("ret:%lu, ctrl_idx:%lu, field_idx:%lu, field_updated:%d", ret, ctrl_idx, field_idx, field_updated);

    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntry_
 *-----------------------------------------------------------------
 * FUNCTION: This function will set a ctrl entry
 * INPUT   : ctrl_p->ctrl_idx
 *           ctrl_p->name
 *           ctrl_p->data_source
 *           ctrl_p->interval
 *           ctrl_p->buckets_requested
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetCtrlEntry_(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    return NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_STATUS, L_CVRT_UINT_TO_PTR(NMTR_TYPE_HIST_CTRL_STATUS_CREATE_AND_WAIT))
        && NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_DATA_SOURCE, L_CVRT_UINT_TO_PTR(ctrl_p->data_source))
        && NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_NAME, ctrl_p->name)
        && NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_INTERVAL, L_CVRT_UINT_TO_PTR(ctrl_p->interval))
        && NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_BUCKETS_REQUESTED, L_CVRT_UINT_TO_PTR(ctrl_p->buckets_requested))
        && NMTR_HIST_SetCtrlEntryField_(ctrl_p->ctrl_idx, NMTR_TYPE_HIST_CTRL_FIELD_STATUS, L_CVRT_UINT_TO_PTR(NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE));
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_DestroyCtrlEntryByIfindex_
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy ctrl entries related to 
 *           specified interface
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_DestroyCtrlEntryByIfindex_(UI32_T ifindex)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    UI32_T ctrl_idx;

    ctrl_idx = 0;

    while (NULL != (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_idx, ifindex)))
    {
        ctrl_idx = ctrl_p->ctrl_idx;

        if (!NMTR_HIST_StopCtrlEntry(ctrl_p))
        {
            return FALSE;
        }

        NMTR_HIST_DestroyCtrlEntry(ctrl_p);
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_CompareCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: A compare func to compare two ctrl entry
 * INPUT   : e1, e2
 * OUTPUT  : None.
 * RETURN  : int
 * NOTE    : None.
 *----------------------------------------------------------------*/
static int NMTR_HIST_CompareCtrlEntry(void *e1, void *e2)
{
    NMTR_TYPE_HistCtrlInfo_T *p1 = *(NMTR_TYPE_HistCtrlInfo_T **)e1;
    NMTR_TYPE_HistCtrlInfo_T *p2 = *(NMTR_TYPE_HistCtrlInfo_T **)e2;
    int diff;

    if (p1 == p2)
        return 0;

    if ((diff = !!p1 - !!p2))
        return diff;

    if ((diff = p1->ctrl_idx - p2->ctrl_idx))
        return diff;

    return 0;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_CreateCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: To create a ctrl entry
 * INPUT   : ctrl_idx
 * OUTPUT  : None.
 * RETURN  : pointer to created ctrl entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_CreateCtrlEntry(UI32_T ctrl_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p;

    if (ctrl_idx < NMTR_HIST_DB_CTRL_INDEX_MIN ||
        ctrl_idx > NMTR_HIST_DB_CTRL_INDEX_MAX)
    {
        NMTR_HIST_ERROR_MSG("out of range: ctrl_idx:%lu", ctrl_idx);
        return NULL;
    }

    ctrl_p = NMTR_HIST_CTRL_ENTRY(ctrl_idx);

    if (ctrl_p->status != NMTR_TYPE_HIST_CTRL_STATUS_NOT_EXIST)
    {
        NMTR_HIST_ERROR_MSG("allocate failed: ctrl_idx:%lu", ctrl_idx);
        return NULL;
    }

    memset(ctrl_p, 0, sizeof(*ctrl_p));
    ctrl_p->ctrl_idx = ctrl_idx;
    ctrl_p->interval = NMTR_TYPE_HIST_CTRL_INTERVAL_MIN;
    ctrl_p->buckets_requested = NMTR_TYPE_HIST_CTRL_BUCKETS_MIN;
    ctrl_p->status = NMTR_TYPE_HIST_CTRL_STATUS_NOT_READY;
    sprintf(ctrl_p->name, "entry%lu", ctrl_p->ctrl_idx);

    NMTR_HIST_UpdateCtrlEntryDataSrc(ctrl_p, NMTR_TYPE_HIST_CTRL_DATA_SOURCE_MIN);

    cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);
    memset(cur_entry_p, 0, sizeof(*cur_entry_p));
    cur_entry_p->ctrl_idx = ctrl_p->ctrl_idx;
    cur_entry_p->sample_idx = NMTR_TYPE_HIST_SAMPLE_INDEX_MIN;

    return ctrl_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_DestroyCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: To destroy a ctrl entry
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static void NMTR_HIST_DestroyCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_HIST_UpdateCtrlEntryDataSrc(ctrl_p, 0);
    memset(ctrl_p, 0, sizeof(*ctrl_p));
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_UpdateCtrlEntryDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: To update date source of ctrl entry
 * INPUT   : ctrl_p
 *           data_source
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static void NMTR_HIST_UpdateCtrlEntryDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T data_source)
{
    NMTR_TYPE_HistCtrlInfo_T *next_ctrl_p, *prev_ctrl_p;
    NMTR_HIST_HistCtrlList_T *ctrl_list_p;

    if (ctrl_p->ctrl_idx == 0)
    {
        NMTR_HIST_ERROR_MSG("wrong params: ctrl_idx:%lu, data_source:%lu", ctrl_p->ctrl_idx, data_source);
        return;
    }

    /* remove from origianl list
     */
    if (ctrl_p->data_source >= NMTR_HIST_DB_CTRL_DATA_SOURCE_MIN &&
        ctrl_p->data_source <= NMTR_HIST_DB_CTRL_DATA_SOURCE_MAX)
    {
        ctrl_list_p = NMTR_HIST_CTRL_ENTRY_DS_LST(ctrl_p->data_source);
    }
    else
    {
        ctrl_list_p = NMTR_HIST_CTRL_ENTRY_FREE_LST();
    }

    next_ctrl_p = NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->next;
    prev_ctrl_p = NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->prev;

    if (next_ctrl_p)
    {
        NMTR_HIST_CTRL_ENTRY_LST_NODE(next_ctrl_p)->prev = prev_ctrl_p;
    }
    else
    {
        ctrl_list_p->tail = prev_ctrl_p;
    }

    if (prev_ctrl_p)
    {
        NMTR_HIST_CTRL_ENTRY_LST_NODE(prev_ctrl_p)->next = next_ctrl_p;
    }
    else
    {
        ctrl_list_p->head = next_ctrl_p;
    }

    NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->next = NULL;
    NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->prev = NULL;

    /* add to new list
     */
    if (data_source >= NMTR_HIST_DB_CTRL_DATA_SOURCE_MIN &&
        data_source <= NMTR_HIST_DB_CTRL_DATA_SOURCE_MAX)
    {
        ctrl_list_p = NMTR_HIST_CTRL_ENTRY_DS_LST(data_source);

        for (prev_ctrl_p = NULL, next_ctrl_p = NMTR_HIST_CTRL_ENTRY_DS_LST(data_source)->head;
            next_ctrl_p != NULL;
            prev_ctrl_p = next_ctrl_p, next_ctrl_p = NMTR_HIST_CTRL_ENTRY_LST_NODE(prev_ctrl_p)->next)
        {
            if (next_ctrl_p->ctrl_idx > ctrl_p->ctrl_idx)
            {
                break;
            }
        }
    }
    else
    {
        ctrl_list_p = NMTR_HIST_CTRL_ENTRY_FREE_LST();

        /* add free entry to tail of list
         */
        next_ctrl_p = NULL;
        prev_ctrl_p = NMTR_HIST_CTRL_ENTRY_FREE_LST()->tail;
    }

    if (next_ctrl_p)
    {
        NMTR_HIST_CTRL_ENTRY_LST_NODE(next_ctrl_p)->prev = ctrl_p;
    }
    else
    {
        ctrl_list_p->tail = ctrl_p;
    }

    if (prev_ctrl_p)
    {
        NMTR_HIST_CTRL_ENTRY_LST_NODE(prev_ctrl_p)->next = ctrl_p;
    }
    else
    {
        ctrl_list_p->head = ctrl_p;
    }

    NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->next = next_ctrl_p;
    NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->prev = prev_ctrl_p;

    /* update data_source
     */
    NMTR_HIST_DEBUG_MSG("ctrl_idx:%lu, data_source:%lu -> %lu", ctrl_p->ctrl_idx, ctrl_p->data_source, data_source);
    ctrl_p->data_source = data_source;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetCtrlEntryPtr
 *-----------------------------------------------------------------
 * FUNCTION: To find ctrl entry
 * INPUT   : ctrl_idx
 * OUTPUT  : None.
 * RETURN  : pointer to found ctrl entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetCtrlEntryPtr(UI32_T ctrl_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    if (ctrl_idx < NMTR_HIST_DB_CTRL_INDEX_MIN ||
        ctrl_idx > NMTR_HIST_DB_CTRL_INDEX_MAX)
    {
        return NULL;
    }

    ctrl_p = NMTR_HIST_CTRL_ENTRY(ctrl_idx);

    if (ctrl_p->status == NMTR_TYPE_HIST_CTRL_STATUS_NOT_EXIST)
    {
        return NULL;
    }

    return ctrl_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextCtrlEntryPtr
 *-----------------------------------------------------------------
 * FUNCTION: To find ctrl entry
 * INPUT   : ctrl_idx
 * OUTPUT  : None.
 * RETURN  : pointer to found ctrl entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetNextCtrlEntryPtr(UI32_T ctrl_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    for (ctrl_idx += 1; ctrl_idx <= NMTR_HIST_DB_CTRL_INDEX_MAX; ctrl_idx++)
    {
        if (NULL != (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
        {
            return ctrl_p;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: To find ctrl entry
 * INPUT   : ctrl_idx
 * OUTPUT  : None.
 * RETURN  : pointer to found ctrl entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetCtrlEntryPtrByNameAndDataSrc(char *name, UI32_T data_source)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    for (ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(0, data_source);
        ctrl_p != NULL;
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(ctrl_p->ctrl_idx, data_source))
    {
        if (strncmp(ctrl_p->name, name, sizeof(ctrl_p->name)) == 0)
        {
            return ctrl_p;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextCtrlEntryPtrByDataSrc
 *-----------------------------------------------------------------
 * FUNCTION: To find ctrl entry
 * INPUT   : ctrl_idx
 *           data_source
 * OUTPUT  : None.
 * RETURN  : pointer to found ctrl entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistCtrlInfo_T *NMTR_HIST_GetNextCtrlEntryPtrByDataSrc(UI32_T ctrl_idx, UI32_T data_source)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;

    if (data_source < NMTR_HIST_DB_CTRL_DATA_SOURCE_MIN ||
        data_source > NMTR_HIST_DB_CTRL_DATA_SOURCE_MAX)
    {
        return NULL;
    }

    if (ctrl_idx > NMTR_HIST_DB_CTRL_INDEX_MAX)
    {
        return NULL;
    }

    if (ctrl_idx < NMTR_HIST_DB_CTRL_INDEX_MIN)
    {
        return NMTR_HIST_CTRL_ENTRY_DS_LST(data_source)->head;
    }

    ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx);

    if (ctrl_p == NULL || ctrl_p->data_source != data_source)
    {
        for (ctrl_p = NMTR_HIST_CTRL_ENTRY_DS_LST(data_source)->head;
            ctrl_p != NULL;
            ctrl_p = NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->next)
        {
            if (ctrl_p->ctrl_idx > ctrl_idx)
            {
                return ctrl_p;
            }
        }

        return NULL;
    }

    ctrl_p = NMTR_HIST_CTRL_ENTRY_LST_NODE(ctrl_p)->next;

    return ctrl_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetFreeCtrlIdx
 *-----------------------------------------------------------------
 * FUNCTION: To find a free ctrl index
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : a free ctrl_idx or 0 if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static UI32_T NMTR_HIST_GetFreeCtrlIdx(void)
{
    NMTR_TYPE_HistCtrlInfo_T *free_ctrl_p;
    UI32_T ctrl_idx = 0;

    free_ctrl_p = NMTR_HIST_CTRL_ENTRY_FREE_LST()->head;

    if (free_ctrl_p)
    {
        ctrl_idx = NMTR_HIST_CTRL_ENTRY_BUF_IDX(free_ctrl_p) + NMTR_HIST_DB_CTRL_INDEX_MIN;
    }

    return ctrl_idx;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetCtrlEntryBucketsGranted
 *-----------------------------------------------------------------
 * FUNCTION: To update buckets_granted and adjust previous entries
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_SetCtrlEntryBucketsGranted(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T buckets_granted)
{
    if (ctrl_p->buckets_granted > buckets_granted)
    {
        NMTR_HIST_RemovePreviousEntry(ctrl_p,
            ctrl_p->buckets_granted - buckets_granted);
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_ResetCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: To reset a ctrl entry. All sample entries will be removed
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_ResetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p;
    NMTR_TYPE_HistCounterInfo_T *cur_counter_p;

    /* reset current entry
     */
    cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);
    cur_counter_p = &cur_entry_p->counter;

    memset(cur_counter_p, 0, sizeof(*cur_counter_p));

    /* clear all previous entries
     */
    NMTR_HIST_SetCtrlEntryBucketsGranted(ctrl_p, 0);

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_StartCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: To do something when a ctrl entry is set to active
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_StartCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_StopCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: To do something when a ctrl entry is set to inactive
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_StopCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    return NMTR_HIST_ResetCtrlEntry(ctrl_p);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_CtrlEntryActiveCheck
 *-----------------------------------------------------------------
 * FUNCTION: For L_RSTATUS_Fsm, to check if status of a ctrl entry
 *           can be active
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_CtrlEntryActiveCheck(void *rec)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p = rec;

    return ctrl_p != NULL;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_CtrlEntryFsm
 *-----------------------------------------------------------------
 * FUNCTION: To update status of a ctrl entry
 * INPUT   : ctrl_idx
 *           action
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_CtrlEntryFsm(UI32_T ctrl_idx, UI32_T action)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    UI32_T old_status, new_status;
    BOOL_T ret = TRUE;

    if (NULL != (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
    {
        old_status = ctrl_p->status;
    }
    else
    {
        old_status = L_RSTATUS_NOT_EXIST;
    }
    new_status = old_status;

    switch (L_RSTATUS_Fsm(action, &new_status, NMTR_HIST_CtrlEntryActiveCheck, ctrl_p))
    {
        case L_RSTATUS_NOTEXIST_2_NOTREADY: /* create, */
        case L_RSTATUS_NOTEXIST_2_ACTIVE:   /* create and start */
        case L_RSTATUS_NOTREADY_2_ACTIVE:   /* start */
            if (ret && old_status == L_RSTATUS_NOT_EXIST)
            {
                if (!(ret = (NULL != (ctrl_p = NMTR_HIST_CreateCtrlEntry(ctrl_idx)))))
                {
                    NMTR_HIST_ERROR_MSG("NMTR_HIST_CreateCtrlEntry failed: ctrl_idx:%lu, action:%lu", ctrl_idx, action);
                }
            }
            if (ret && new_status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE)
            {
                ret = NMTR_HIST_StartCtrlEntry(ctrl_p);
            }
            break;

        case L_RSTATUS_NOTREADY_2_NOTEXIST: /* destroy */
        case L_RSTATUS_ACTIVE_2_NOTEXIST:   /* stop and destroy */
        case L_RSTATUS_ACTIVE_2_NOTREADY:   /* stop */
            if (ret && old_status == NMTR_TYPE_HIST_CTRL_STATUS_ACTIVE)
            {
                ret = NMTR_HIST_StopCtrlEntry(ctrl_p);
            }
            if (ret && new_status == L_RSTATUS_NOT_EXIST)
            {
                NMTR_HIST_DestroyCtrlEntry(ctrl_p);
                ctrl_p = NULL;
            }
            break;

        case L_RSTATUS_NOTEXIST_2_NOTEXIST: /* no change, do nothing */
        case L_RSTATUS_NOTREADY_2_NOTREADY: /* no change, do nothing */
        case L_RSTATUS_ACTIVE_2_ACTIVE:     /* no change, do nothing */
            break;

        case L_RSTATUS_TRANSITION_STATE_ERROR:
            NMTR_HIST_ERROR_MSG("L_RSTATUS_Fsm failed: ctrl_idx:%lu, action:%lu", ctrl_idx, action);
            ret = FALSE;
            break;
    }

    if (ret && ctrl_p)
    {
        NMTR_HIST_SET_CTRL_ENTRY_STATUS(ctrl_p, new_status);
    }

    NMTR_HIST_DEBUG_MSG("ret:%d, ctrl_idx:%lu, action:%lu, old_status:%lu, new_status:%lu", ret, ctrl_idx, action, old_status, new_status);

    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_UpdateCtrlEntryCounter
 *-----------------------------------------------------------------
 * FUNCTION: To update diff counter for a ctrl entry
 * INPUT   : ctrl_p
 *           counter_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_UpdateCtrlEntryCounter(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, NMTR_TYPE_HistCounterInfo_T *counter_p)
{
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);
    NMTR_TYPE_HistCounterInfo_T *cur_counter_p = &cur_entry_p->counter;

    if (cur_counter_p->start_time == 0)
    {
        /* for the first time
         */
        memset(cur_counter_p, 0, sizeof(*cur_counter_p));
        cur_counter_p->start_time = counter_p->start_time;
        ctrl_p->refresh_time = NMTR_HIST_TIMEOUT_TICKS(cur_counter_p->start_time, ctrl_p->interval);
    }
    else if (L_MATH_TimeOut32(counter_p->start_time, ctrl_p->refresh_time))
    {
        /* it's time to move current entry to previous entry
         */
        NMTR_HIST_UpdatePreviousEntryCounter(ctrl_p);

        memset(cur_counter_p, 0, sizeof(*cur_counter_p));
        cur_counter_p->start_time = counter_p->start_time;

        if (cur_entry_p->sample_idx < NMTR_TYPE_HIST_SAMPLE_INDEX_MAX)
        {
            cur_entry_p->sample_idx++;
        }
        else
        {
            cur_entry_p->sample_idx = NMTR_TYPE_HIST_SAMPLE_INDEX_MIN;
        }

        ctrl_p->refresh_time = NMTR_HIST_TIMEOUT_TICKS(ctrl_p->refresh_time, ctrl_p->interval);
    }

    NMTR_HIST_AccumulateCounter(cur_counter_p, counter_p);

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_ComparePreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: A compare func to compare two previous entry
 * INPUT   : e1, e2
 * OUTPUT  : None.
 * RETURN  : int
 * NOTE    : None.
 *----------------------------------------------------------------*/
static int NMTR_HIST_ComparePreviousEntry(void *e1, void *e2)
{
    NMTR_TYPE_HistSampleEntry_T *p1 = *(NMTR_TYPE_HistSampleEntry_T **)e1;
    NMTR_TYPE_HistSampleEntry_T *p2 = *(NMTR_TYPE_HistSampleEntry_T **)e2;
    int diff;

    if (p1 == p2)
        return 0;

    if ((diff = !!p1 - !!p2))
        return diff;

    if ((diff = p1->ctrl_idx - p2->ctrl_idx))
        return diff;

    if ((diff = p1->sample_idx - p2->sample_idx))
        return diff;

    return 0;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_AddPreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: To add a new previous entry to specified ctrl entry
 * INPUT   : ctrl_p
 *           prev_entry_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_AddPreviousEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, NMTR_TYPE_HistSampleEntry_T *prev_entry_p)
{
    NMTR_TYPE_HistSampleEntry_T *free_entry_p = NULL;
    UI32_T entry_idx;

    if (ctrl_p->buckets_granted >= ctrl_p->buckets_requested)
    {
        return FALSE;
    }

    entry_idx = NMTR_HIST_PREV_ENTRY_FREE_IDX(ctrl_p);
    free_entry_p = NMTR_HIST_PREV_ENTRY(ctrl_p, entry_idx);

    entry_idx += 1;
    entry_idx %= NMTR_HIST_CTRL_BUCKETS_NUM;

    NMTR_HIST_PREV_ENTRY_FREE_IDX(ctrl_p) = entry_idx;
    ctrl_p->buckets_granted++;

    *free_entry_p = *prev_entry_p;

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_RemovePreviousEntry
 *-----------------------------------------------------------------
 * FUNCTION: To remove the oldest previous entry from specified ctrl entry
 * INPUT   : ctrl_p
 *           num
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_RemovePreviousEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, UI32_T num)
{
    if (num > ctrl_p->buckets_granted)
    {
        num = ctrl_p->buckets_granted;
    }

    ctrl_p->buckets_granted -= num;

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetPreviousEntryPtr
 *-----------------------------------------------------------------
 * FUNCTION: To find previous entry
 * INPUT   : ctrl_idx
 *           sample_idx
 * OUTPUT  : None.
 * RETURN  : pointer to found entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetPreviousEntryPtr(UI32_T ctrl_idx, UI32_T sample_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p, *prev_entry_p;
    UI32_T entry_idx;

    if (NULL == (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
    {
        return NULL;
    }

    cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);

    entry_idx = ctrl_p->buckets_granted - (cur_entry_p->sample_idx - sample_idx);

    if (cur_entry_p->sample_idx < sample_idx)
    {
        entry_idx -= NMTR_TYPE_HIST_SAMPLE_INDEX_MAX - NMTR_TYPE_HIST_SAMPLE_INDEX_MIN + 1;
    }

    if (entry_idx >= ctrl_p->buckets_granted)
    {
        return NULL;
    }

    /* actual entry_idx =
     *   (relative entry_idx + (free_idx - buckets_granted)) % BUCKETS_NUM
     *
     * underflow leads wrong modolo operation,
     * so add BUCKETS_NUM to avoid underflow.
     *
     * actual entry_idx =
     *   (relative entry_idx + (free_idx + BUCKETS_NUM - buckets_granted)) % BUCKETS_NUM
     */
    entry_idx += NMTR_HIST_PREV_ENTRY_FREE_IDX(ctrl_p) +
        (NMTR_HIST_CTRL_BUCKETS_NUM - ctrl_p->buckets_granted);
    entry_idx %= NMTR_HIST_CTRL_BUCKETS_NUM;

    prev_entry_p = NMTR_HIST_PREV_ENTRY(ctrl_p, entry_idx);

    return prev_entry_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextPreviousEntryPtr
 *-----------------------------------------------------------------
 * FUNCTION: To find previous entry
 * INPUT   : ctrl_idx
 *           sample_idx
 * OUTPUT  : None.
 * RETURN  : pointer to found entry or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetNextPreviousEntryPtr(UI32_T ctrl_idx, UI32_T sample_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p, *prev_entry_p;
    UI32_T sample_idx_seg_num;
    UI32_T sample_idx_seg_first[2] = {0};
    UI32_T sample_idx_seg_last[2] = {0};

    if (NULL == (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
    {
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(ctrl_idx);
        sample_idx = 0;
    }

    for (;
        ctrl_p != NULL;
        ctrl_p = NMTR_HIST_GetNextCtrlEntryPtr(ctrl_p->ctrl_idx), sample_idx = 0)
    {
        if (ctrl_p->buckets_granted < 1)
        {
            continue;
        }

        if (sample_idx > NMTR_TYPE_HIST_SAMPLE_INDEX_MAX - 1)
        {
            continue;
        }

        cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);

        /* calculate sample_idx range
         */
        sample_idx_seg_num = 0;

        if ((cur_entry_p->sample_idx - NMTR_TYPE_HIST_SAMPLE_INDEX_MIN + 1) > ctrl_p->buckets_granted)
        {
            /*          cur_entry_p->sample_idx
             *          |
             * [..ppppppc.....]
             *    |    | 
             *    |    sample_idx_seg_last[0]
             *    |
             *    sample_idx_seg_first[0]
             */
            sample_idx_seg_first[sample_idx_seg_num] = cur_entry_p->sample_idx - ctrl_p->buckets_granted;
            sample_idx_seg_last[sample_idx_seg_num] = cur_entry_p->sample_idx - 1;
            sample_idx_seg_num++;
        }
        else if (cur_entry_p->sample_idx == NMTR_TYPE_HIST_SAMPLE_INDEX_MIN)
        {
            /*  cur_entry_p->sample_idx
             *  |
             * [c.......pppppp]
             *          |    | 
             *          |    sample_idx_seg_last[0]
             *          |
             *          sample_idx_seg_first[0]
             */
            sample_idx_seg_first[sample_idx_seg_num] = NMTR_TYPE_HIST_SAMPLE_INDEX_MAX - ctrl_p->buckets_granted + 1;
            sample_idx_seg_last[sample_idx_seg_num] = NMTR_TYPE_HIST_SAMPLE_INDEX_MAX;
            sample_idx_seg_num++;
        }
        else
        {
            /*     cur_entry_p->sample_idx
             *     |
             * [pppc.......ppp]
             *  | |        | | 
             *  | |        | sample_idx_seg_last[1]
             *  | |        |
             *  | |        sample_idx_seg_first[1]
             *  | |
             *  | sample_idx_seg_last[0]
             *  |
             *  sample_idx_seg_first[0]
             */
            sample_idx_seg_first[sample_idx_seg_num] = NMTR_TYPE_HIST_SAMPLE_INDEX_MIN;
            sample_idx_seg_last[sample_idx_seg_num] = cur_entry_p->sample_idx - 1;
            sample_idx_seg_num++;

            sample_idx_seg_first[sample_idx_seg_num] = NMTR_TYPE_HIST_SAMPLE_INDEX_MAX - 
                (ctrl_p->buckets_granted - (cur_entry_p->sample_idx - NMTR_TYPE_HIST_SAMPLE_INDEX_MIN)) + 1;
            sample_idx_seg_last[sample_idx_seg_num] = NMTR_TYPE_HIST_SAMPLE_INDEX_MAX;
            sample_idx_seg_num++;
        }

        sample_idx++;

        if (sample_idx < sample_idx_seg_first[0])
        {
            sample_idx = sample_idx_seg_first[0];
        }
        else if (sample_idx > sample_idx_seg_last[0])
        {
            if (sample_idx_seg_num > 1)
            {
                if (sample_idx < sample_idx_seg_first[1])
                {
                    sample_idx = sample_idx_seg_first[1];
                }
                else if (sample_idx > sample_idx_seg_last[1])
                {
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        prev_entry_p = NMTR_HIST_GetPreviousEntryPtr(ctrl_p->ctrl_idx, sample_idx);

        if (prev_entry_p != NULL)
        {
            return prev_entry_p;
        }
    }

    return NULL;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNextPreviousEntryPtrByCtrlIdx
 *-----------------------------------------------------------------
 * FUNCTION: To find previous entry
 * INPUT   : ctrl_idx
 *           sample_idx - 0 to get first entry
 * OUTPUT  : None.
 * RETURN  : pointer to found entry or NULL if failed
 * NOTE    : sort by entry_idx (timestamp)
 *----------------------------------------------------------------*/
static NMTR_TYPE_HistSampleEntry_T *NMTR_HIST_GetNextPreviousEntryPtrByCtrlIdx(UI32_T ctrl_idx, UI32_T sample_idx)
{
    NMTR_TYPE_HistCtrlInfo_T *ctrl_p;
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p, *prev_entry_p;
    UI32_T entry_idx;

    if (NULL == (ctrl_p = NMTR_HIST_GetCtrlEntryPtr(ctrl_idx)))
    {
        return NULL;
    }

    cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);

    if (sample_idx == 0)
    {
        entry_idx = 0;
    }
    else
    {
        entry_idx = ctrl_p->buckets_granted - (cur_entry_p->sample_idx - sample_idx) + 1;

        if (cur_entry_p->sample_idx < sample_idx)
        {
            entry_idx -= NMTR_TYPE_HIST_SAMPLE_INDEX_MAX - NMTR_TYPE_HIST_SAMPLE_INDEX_MIN + 1;
        }
    }

    if (entry_idx >= ctrl_p->buckets_granted)
    {
        return NULL;
    }

    /* actual entry_idx =
     *   (relative entry_idx + (free_idx - buckets_granted)) % BUCKETS_NUM
     *
     * underflow leads wrong modolo operation,
     * so add BUCKETS_NUM to avoid underflow.
     *
     * actual entry_idx =
     *   (relative entry_idx + (free_idx + BUCKETS_NUM - buckets_granted)) % BUCKETS_NUM
     */
    entry_idx += NMTR_HIST_PREV_ENTRY_FREE_IDX(ctrl_p) +
        (NMTR_HIST_CTRL_BUCKETS_NUM - ctrl_p->buckets_granted);
    entry_idx %= NMTR_HIST_CTRL_BUCKETS_NUM;

    prev_entry_p = NMTR_HIST_PREV_ENTRY(ctrl_p, entry_idx);

    return prev_entry_p;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_UpdatePreviousEntryCounter
 *-----------------------------------------------------------------
 * FUNCTION: To add new prevoius entry by current entry
 * INPUT   : ctrl_p
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T NMTR_HIST_UpdatePreviousEntryCounter(NMTR_TYPE_HistCtrlInfo_T *ctrl_p)
{
    NMTR_TYPE_HistSampleEntry_T *cur_entry_p = NMTR_HIST_CUR_ENTRY(ctrl_p);

    if (ctrl_p->buckets_granted > ctrl_p->buckets_requested - 1)
    {
        NMTR_HIST_RemovePreviousEntry(ctrl_p,
            ctrl_p->buckets_granted - (ctrl_p->buckets_requested - 1));
    }

    return NMTR_HIST_AddPreviousEntry(ctrl_p, cur_entry_p);
}

#endif /* (SYS_CPNT_NMTR_HISTORY == TRUE) */

