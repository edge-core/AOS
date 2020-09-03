/* Module Name: traceroute_pom.h
 * Purpose:
 *    Declares the APIs for IPCs with TRACEROUTE OM.
 *
 * Notes:
 *    None.
 * History:
 *      Date        --  Modifier,   Reason
 *      2007/12/19  --  Peter Yu,   Create
 *
 * Copyright(C)      Accton Corporation, 2007
 */
#ifndef TRACEROUTE_POM_H
#define TRACEROUTE_POM_H


/* INCLUDE FILE DECLARATIONS
 */

#include "sys_type.h"
#include "traceroute_type.h"


/* NAMING CONSTANT DECLARATIONS
 */


/* MACRO FUNCTION DECLARATIONS
 */


/* DATA TYPE DECLARATIONS
 */


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: TRACEROUTE_POM_InitiateProcessResources
 * PURPOSE:
 *          Initiate resources for TRACEROUTE_POM in the calling process.
 * INPUT:
 *          None.
 * OUTPUT:
 *          None.
 * RETURN:
 *          TRUE    -- Success
 *          FALSE   -- Fail
 * NOTES:
 *          None.
 */
BOOL_T TRACEROUTE_POM_InitiateProcessResources(void);

/* FUNCTION NAME:TRACEROUTE_POM_GetTraceRouteCtlEntry
 * PURPOSE: 
 *          Get the specific traceroute control entry.
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 * OUTPUT: 
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_POM_GetTraceRouteCtlEntry(char  *owner_index_p,
                                             UI32_T owner_index_len,
                                             char  *test_name_p,
                                             UI32_T test_name_len,
                                             TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p);

/* FUNCTION NAME:TRACEROUTE_POM_GetNextTraceRouteCtlEntry
 * PURPOSE: 
 *          Get the next traceroute control entry.
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 * OUTPUT: 
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_POM_GetNextTraceRouteCtlEntry(char  *owner_index_p,
                                                 UI32_T owner_index_len,
                                                 char  *test_name_p,
                                                 UI32_T test_name_len,
                                                 TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p);

/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry
 * PURPOSE: 
 *          To Get next available prob history entry
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 * OUTPUT:  
 *          prob_history_entry_p - the next probe history entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 */ 
UI32_T  TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                          UI32_T owner_index_len,
                                                          char  *test_name_p,
                                                          UI32_T test_name_len,
                                                          UI32_T history_index,
                                                          UI32_T history_hop_index,
                                                          UI32_T history_probe_index,
                                                          TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p); 

/* FUNCTION NAME: TRACEROUTE_POM_GetTraceRouteResultsEntry
 * PURPOSE: 
 *          To the specific result entry base on the given index
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 * OUTPUT:  
 *          result_entry_p - the result entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_POM_GetTraceRouteResultsEntry(char  *owner_index_p,
                                                 UI32_T owner_index_len,
                                                 char  *test_name_p,
                                                 UI32_T test_name_len,
                                                 TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);
                                                 
/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteResultsEntry
 * PURPOSE: 
 *          To get the next result entry of the specified control entry.
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 * OUTPUT:  
 *          result_entry_p - the next result entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_POM_GetNextTraceRouteResultsEntry(char  *owner_index_p,
                                                     UI32_T owner_index_len,
                                                     char  *test_name_p,
                                                     UI32_T test_name_len,
                                                     TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p);
                                                    
/* FUNCTION NAME: TRACEROUTE_POM_GetTraceRouteProbeHistoryEntry
 * PURPOSE: 
 *          To get the specific probe history entry base on the given index
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 * OUTPUT:  
 *          prob_history_entry_p - the probe history entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_POM_GetTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                      UI32_T owner_index_len,
                                                      char  *test_name_p,
                                                      UI32_T test_name_len,
                                                      UI32_T history_index,
                                                      UI32_T history_hop_index,
                                                      UI32_T history_probe_index,
                                                      TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);
                                                         
/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry
 * PURPOSE: 
 *          To Get next available prob history entry
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 * OUTPUT:  
 *          prob_history_entry_p - the next probe history entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 */ 
UI32_T  TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                          UI32_T owner_index_len,
                                                          char  *test_name_p,
                                                          UI32_T test_name_len,
                                                          UI32_T history_index,
                                                          UI32_T history_hop_index,
                                                          UI32_T history_probe_index,
                                                          TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p); 

/* FUNCTION NAME: TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB
 * PURPOSE: 
 *          To Get next available prob history entry
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 *          history_index - The history index of the TraceRouteProbeHistoryEntry.
 *          history_hop_index - The hop index of the TraceRouteProbeHistoryEntry.
 *          history_probe_index - The probe index of the TraceRouteProbeHistoryEntry.
 *
 * OUTPUT:  
 *          prob_history_entry_p- next available entry that contains prob packet information
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_POM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(char  *owner_index_p,
                                                                  UI32_T owner_index_len,
                                                                  char  *test_name_p,
                                                                  UI32_T test_name_len,
                                                                  UI32_T history_index,
                                                                  UI32_T history_hop_index,
                                                                  UI32_T history_probe_index,
                                                                  TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p);


#endif /* #ifndef TRACEROUTE_POM_H */
