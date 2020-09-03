/* ------------------------------------------------------------------------
 * FILE NAME - NMTR_HIST.H
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Wakka             30/09/2010      new created
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2010
 * ------------------------------------------------------------------------
 */
#ifndef NMTR_HIST_H
#define NMTR_HIST_H

/* INCLUDE FILE DECLARATIONS
 */
#include "sys_type.h"
#include "nmtr_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef NMTR_TYPE_HistCounterInfo_T *(*NMTR_HIST_GetDiffCounter_Callback_T)(UI32_T ifindex);

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
BOOL_T NMTR_HIST_Init(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_Reset
 *-----------------------------------------------------------------
 * FUNCTION: This function will clear database
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_Reset(void);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_SetDefaultCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will create default ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_SetDefaultCtrlEntry(UI32_T ifindex);

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
BOOL_T NMTR_HIST_AccumulateCounter(NMTR_TYPE_HistCounterInfo_T *counter_p, NMTR_TYPE_HistCounterInfo_T *new_counter_p);

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
BOOL_T NMTR_HIST_UpdateCtrlEntryCounterByIfindex(UI32_T ifindex, NMTR_TYPE_HistCounterInfo_T *counter_p);

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
BOOL_T NMTR_HIST_UpdateAllCtrlEntryCounter(NMTR_HIST_GetDiffCounter_Callback_T func);

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
BOOL_T NMTR_HIST_SetCtrlEntryField(UI32_T ctrl_idx, UI32_T field_idx, void *data_p);

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
BOOL_T NMTR_HIST_SetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

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
BOOL_T NMTR_HIST_SetCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

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
BOOL_T NMTR_HIST_DestroyCtrlEntryByNameAndDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_DestroyAllCtrlEntry
 *-----------------------------------------------------------------
 * FUNCTION: This function will destroy all ctrl entries
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T NMTR_HIST_DestroyAllCtrlEntry(void);

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
BOOL_T NMTR_HIST_DestroyCtrlEntryByIfindex(UI32_T ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_GetNumOfCtrlEntryByIfindex
 *-----------------------------------------------------------------
 * FUNCTION: This function will get number of ctrl entry
 * INPUT   : ifindex
 * OUTPUT  : None.
 * RETURN  : number of ctrl of entry
 * NOTE    : None.
 *----------------------------------------------------------------*/
UI32_T NMTR_HIST_GetNumOfCtrlEntryByIfindex(UI32_T ifindex);

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
BOOL_T NMTR_HIST_GetCtrlEntry(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next);

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
BOOL_T NMTR_HIST_GetCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p, BOOL_T get_next);

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
BOOL_T NMTR_HIST_GetCurrentEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next);

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
BOOL_T NMTR_HIST_GetPreviousEntry(NMTR_TYPE_HistSampleEntry_T *entry_p, BOOL_T get_next);

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
BOOL_T NMTR_HIST_GetNextPreviousEntryByCtrlIdx(NMTR_TYPE_HistSampleEntry_T *entry_p);

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
BOOL_T NMTR_HIST_GetNextRemovedDefaultCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

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
BOOL_T NMTR_HIST_GetNextUserCfgCtrlEntryByDataSrc(NMTR_TYPE_HistCtrlInfo_T *ctrl_p);

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
BOOL_T NMTR_HIST_FollowAttribute(UI32_T to_ifindex, UI32_T from_ifindex);

/*-----------------------------------------------------------------
 * ROUTINE NAME - NMTR_HIST_Backdoor
 *-----------------------------------------------------------------
 * FUNCTION: backdoor
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE/FALSE
 * NOTE    : None.
 *----------------------------------------------------------------*/
void NMTR_HIST_Backdoor(void);

#endif /* NMTR_HIST_H */

