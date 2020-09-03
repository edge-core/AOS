/* ------------------------------------------------------------------------- 
 * FILE NAME - TraceRoute_OM.C                                                     
 * ------------------------------------------------------------------------- 
 * Purpose: This package provides the sevices to manage/support the RFC2925 MIB                                   
 * Notes: 1. This package shall be a reusable package for all the L2/L3 switchs.
 *
 *                                                                            
 * Modification History:                                        
 *   By            Date      Ver.    Modification Description                
 * ------------ ----------   -----   --------------------------------------- 
 *   Amytu       2003-07-01          First Created
 * ------------------------------------------------------------------------- 
 * Copyright(C)         ACCTON Technology Corp., 2003
 * -------------------------------------------------------------------------
 */
  


/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "sys_bld.h"
#include "sys_type.h"
#include "sys_dflt.h"
#include "sys_adpt.h"
#include "leaf_2925.h"
#include "l_sort_lst.h"
#include "l_rstatus.h"
#include "l_inet.h"
#include "sysfun.h"
#include "netcfg_type.h"
#include "traceroute_type.h"
#include "traceroute_mgr.h"
#include "traceroute_om.h"
#include "netcfg_pmgr_route.h"
#include "ip_lib.h"

/* LOCAL DATATYPE DECLARATION
 */

/* NAMING CONSTANT DECLARATIONS
 */
#define TRACEROUTE_PROB_HISTORY_TABLE_SIZE      (SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY)


/* MACRO FUNCTION DECLARATIONS
 */
#define TRACEROUTE_OM_ENTER_CRITICAL_SECTION() (original_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(traceroute_om_sem_id))
#define TRACEROUTE_OM_LEAVE_CRITICAL_SECTION() SYSFUN_OM_LEAVE_CRITICAL_SECTION(traceroute_om_sem_id, original_priority)


/* TYPE DECLARATIONS
 */
 
typedef struct
{
    UI32_T      prob_history_head;
    UI32_T      prob_history_end;
    UI32_T      current_prob_history;
    
} TRACEROUTE_PROB_HISTORY_INDEX_T; 


/* LOCAL SUBPROGRAM DECLARATIONS 
 */
static int      TRACEROUTE_OM_Compare(void *elm1, void *elm2);
static void     TRACEROUTE_OM_ClearProbHistoryEntry(UI32_T table_index);
static void     TRACEROUTE_OM_InitProbHistoryIndexTable(void);
static UI32_T   TRACEROUTE_OM_FindKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p);
static BOOL_T   TRACEROUTE_OM_SemanticCheck(void  *ctrl_entry_p);
static void     TRACEROUTE_OM_AssociateResultTable(UI32_T table_index, TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctrl_entry_p);
static void     TRACEROUTE_OM_DisAssociateResultTable(UI32_T table_index);
static UI32_T   TRACEROUTE_OM_CreateNewCtrlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  ctrl_entry);
static UI32_T   TRACEROUTE_OM_FindFirstKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p);
/*static UI32_T   TRACEROUTE_OM_FindNextKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p);*/

/* STATIC VARIABLE DECLARATIONS  
 */   
static TRACEROUTE_TYPE_TraceRouteCtlEntry_T             trace_route_control_table[SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE];
static TRACEROUTE_TYPE_TraceRouteResultsEntry_T         trace_route_result_table[SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE];
static TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T    trace_route_prob_history_table[TRACEROUTE_PROB_HISTORY_TABLE_SIZE];
static TRACEROUTE_PROB_HISTORY_INDEX_T                  prob_history_index_table[SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE];
static L_SORT_LST_List_T                                sort_trace_route_list; 
                    
static UI32_T       next_available_index;
static UI8_T       current_control_entry;

/* for OM critical section */
static UI32_T   traceroute_om_sem_id;
static UI32_T   original_priority;

/* EXPORTED SUBPROGRAM BODIES
 */
 
/* FUNCTION NAME : TRACEROUTE_OM_Initiate_System_Resources
 * PURPOSE:
 *      Initialize Traceroute OM
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
void TRACEROUTE_OM_Initiate_System_Resources(void)
{
    UI32_T res;

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_TRACEROUTE_OM, &traceroute_om_sem_id) != SYSFUN_OK)
    {
        printf("\r\n%s: get om sem id fail!\r\n", __FUNCTION__);
    }

    if ((res = L_SORT_LST_Create(&sort_trace_route_list, SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE, sizeof(TRACEROUTE_SORTLST_ELM_T),TRACEROUTE_OM_Compare)) != TRUE)
    {
        DBG_PrintText (" TRACEROUTE_OM_Init : ERR---Can not create sorted list\n");
    }
        
    memset(trace_route_control_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
    memset(trace_route_result_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
    memset(trace_route_prob_history_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T)*TRACEROUTE_PROB_HISTORY_TABLE_SIZE);  
    memset(prob_history_index_table, 0, sizeof(TRACEROUTE_PROB_HISTORY_INDEX_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
        
    next_available_index = 0;
    current_control_entry = 0;
    
    return;
} /* end of TRACEROUTE_OM_Initiate_System_Resources() */

/* FUNCTION NAME : TRACEROUTE_OM_ClearOM
 * PURPOSE:
 *      Clear Traceroute OM
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
void TRACEROUTE_OM_ClearOM (void)
{
    memset(trace_route_control_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
    memset(trace_route_result_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
    memset(trace_route_prob_history_table, 0, sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T)*TRACEROUTE_PROB_HISTORY_TABLE_SIZE);  
    memset(prob_history_index_table, 0, sizeof(TRACEROUTE_PROB_HISTORY_INDEX_T)*SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE);
 
    L_SORT_LST_Delete_All(&sort_trace_route_list);
    next_available_index = 0;
    current_control_entry = 0;
    return;
} /* end of TRACEROUTE_OM_ClearOM() */


/* FUNCTION NAME : TRACEROUTE_OM_EnterMasterMode
 * PURPOSE:
 *      Assign default value Traceroute OM
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
void TRACEROUTE_OM_EnterMasterMode (void)
{
    TRACEROUTE_OM_InitProbHistoryIndexTable();
    return;    
} /* end of TRACEROUTE_OM_EnterMasterMode() */

/* FUNCTION NAME:TRACEROUTE_OM_GetFirstTraceRouteCtlEntry
 * PURPOSE: 
 *          Get the first traceroute control entry.
 * INPUT:  
 *          None.
 * OUTPUT: 
 *          ctrl_entry_p - the first control entry if it exists.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_OM_GetFirstTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;      
    UI32_T      res;
    
    /*  BODY
     */ 
        
    if (ctrl_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
            
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
        
    if ((res = TRACEROUTE_OM_FindFirstKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;        
    
    memcpy(ctrl_entry_p, &trace_route_control_table[list_elm.table_index],
           sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));           

    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_GetFirstTraceRouteCtlEntry() */

/* FUNCTION NAME:TRACEROUTE_OM_GetTraceRouteCtlEntry
 * PURPOSE: 
 *          Get the specific traceroute control entry.
 * INPUT:  
 *          ctrl_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation. 
 *          ctrl_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry_p->trace_route_ctl_test_name - The test name of the trace route operation. 
 *          ctrl_entry_p->trace_route_ctl_test_name_len - The length of the test name.
 * OUTPUT: 
 *          ctrl_entry_p - the specific control entry if it exists.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_OM_GetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;      
    UI32_T      res;
    
    /*  BODY
     */ 
    if (ctrl_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
        
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(list_elm.owner_index,  ctrl_entry_p->trace_route_ctl_owner_index, ctrl_entry_p->trace_route_ctl_owner_index_len);
    memcpy(list_elm.test_name, ctrl_entry_p->trace_route_ctl_test_name, ctrl_entry_p->trace_route_ctl_test_name_len);
    list_elm.owner_index_len = ctrl_entry_p->trace_route_ctl_owner_index_len;
    list_elm.test_name_len = ctrl_entry_p->trace_route_ctl_test_name_len;
    
    if ((res = TRACEROUTE_OM_FindKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;        
    
    memcpy(ctrl_entry_p, &trace_route_control_table[list_elm.table_index],
           sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));           

    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_GetTraceRouteCtlEntry() */

/* FUNCTION NAME:TRACEROUTE_OM_GetNextTraceRouteCtlEntry
 * PURPOSE: 
 *          To get the next traceroute control entry.
 * INPUT:  
 *          ctrl_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation. 
 *          ctrl_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry_p->trace_route_ctl_test_name - The test name of the trace route operation. 
 *          ctrl_entry_p->trace_route_ctl_test_name_len - The length of the test name.
 * OUTPUT: 
 *          ctrl_entry_p - the next control entry if it exists.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_OM_GetNextTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  *ctrl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;      
    
    /*  BODY
     */ 
    if (ctrl_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
        
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(list_elm.owner_index,  ctrl_entry_p->trace_route_ctl_owner_index, ctrl_entry_p->trace_route_ctl_owner_index_len);
    memcpy(list_elm.test_name, ctrl_entry_p->trace_route_ctl_test_name, ctrl_entry_p->trace_route_ctl_test_name_len);
    list_elm.owner_index_len = ctrl_entry_p->trace_route_ctl_owner_index_len;
    list_elm.test_name_len = ctrl_entry_p->trace_route_ctl_test_name_len;
    
    if(L_SORT_LST_Get_Next(&sort_trace_route_list, &list_elm) != TRUE)
        return TRACEROUTE_TYPE_FAIL;        
    
    memcpy(ctrl_entry_p, &trace_route_control_table[list_elm.table_index],
           sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));           

    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_GetNextTraceRouteCtlEntry() */

/* FUNCTION NAME:TRACEROUTE_OM_SetTraceRouteCtlTargetAddress
 * PURPOSE: 
 *          To set the target address field for traceroute control entry
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 *          target_addr_p    -   The target address of the remote host.
 *          target_addr_len -   The length of target_addr_p
 * OUTPUT: 
 *          None
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_INVALID_ARG \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T TRACEROUTE_OM_SetTraceRouteCtlTargetAddress(TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctl_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;      
    UI32_T      res;
    
    /*  BODY
     */ 
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(list_elm.owner_index, ctl_entry_p->trace_route_ctl_owner_index, ctl_entry_p->trace_route_ctl_owner_index_len);
    memcpy(list_elm.test_name, ctl_entry_p->trace_route_ctl_test_name, ctl_entry_p->trace_route_ctl_test_name_len);
    list_elm.owner_index_len = ctl_entry_p->trace_route_ctl_owner_index_len;
    list_elm.test_name_len = ctl_entry_p->trace_route_ctl_test_name_len;
    
    if ((res = TRACEROUTE_OM_FindKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;    

    trace_route_control_table[list_elm.table_index].trace_route_ctl_target_address = ctl_entry_p->trace_route_ctl_target_address;
    
    return TRACEROUTE_TYPE_OK;
} /* end of TRACEROUTE_OM_SetTraceRouteCtlTargetAddress() */

/* FUNCTION NAME:TRACEROUTE_OM_SetTraceRouteCtlEntry
 * PURPOSE: 
 *          To create / destroy contrl entry
 * INPUT:  
 *          ctrl_entry.trace_route_ctl_owner_index - The owner index of the trace route operation. 
 *          ctrl_entry.trace_route_ctl_owner_index_len - The length of the owner index.
 *          ctrl_entry.trace_route_ctl_test_name - The test name of the trace route operation. 
 *          ctrl_entry.trace_route_ctl_test_name_len - The length of the test name.
 *          ctrl_entry.trace_route_ctl_rowstatus - The row status of the control entry.
 *          ctrl_entry.trace_route_ctl_target_address - The target address of the traceroute operation. 
 *          ctrl_entry.trace_route_ctl_admin_status - The admin status of the traceroute entry.
 * OUTPUT: 
 *          None
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \
 *          TRACEROUTE_TYPE_NO_MORE_WORKSPACE \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          1. Currently, only CAG and Destroy is supported.
 *             Operation for CAW and NotInService is not allowed.
 *          2. Currentlt, we only permit the setting of row_status, target_address and admin_status
 */ 
UI32_T TRACEROUTE_OM_SetTraceRouteCtlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T ctrl_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        local_entry;
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    //UI8_T       dest_ip[SYS_ADPT_IPV4_ADDR_LEN], src_ip[SYS_ADPT_IPV4_ADDR_LEN];
    //L_INET_AddrIp_T dest_addr, src_addr, nexthop_addr;
    UI32_T      res, action;
    //, out_ifindex;
    UI32_T      row_status_result, row_status;
    BOOL_T      admin_status_changed = FALSE;
    
    /*  BODY
     */ 
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    
    list_elm.owner_index_len = ctrl_entry.trace_route_ctl_owner_index_len; 
    list_elm.test_name_len = ctrl_entry.trace_route_ctl_test_name_len;
    memcpy(list_elm.owner_index, ctrl_entry.trace_route_ctl_owner_index, list_elm.owner_index_len);
    memcpy(list_elm.test_name, ctrl_entry.trace_route_ctl_test_name, list_elm.test_name_len);
       
    memset(&local_entry, 0, sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));

    memcpy(local_entry.trace_route_ctl_owner_index, ctrl_entry.trace_route_ctl_owner_index, ctrl_entry.trace_route_ctl_owner_index_len);
    memcpy(local_entry.trace_route_ctl_test_name, ctrl_entry.trace_route_ctl_test_name, ctrl_entry.trace_route_ctl_test_name_len);
    local_entry.trace_route_ctl_owner_index_len = ctrl_entry.trace_route_ctl_owner_index_len;
    local_entry.trace_route_ctl_test_name_len = ctrl_entry.trace_route_ctl_test_name_len;

    res = TRACEROUTE_OM_GetTraceRouteCtlEntry(&local_entry);
    
    if(res == TRACEROUTE_TYPE_OK)
    {
        if ((res = TRACEROUTE_OM_FindKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
            return TRACEROUTE_TYPE_FAIL;   
    }
    /* in this switch, we set the original row_status and detect the admin_status changes.*/
    switch (ctrl_entry.trace_route_ctl_rowstatus)
    {
        case VAL_traceRouteCtlRowStatus_notReady:
            if (res != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
            
            row_status = local_entry.trace_route_ctl_rowstatus;
            break;
            
        case VAL_traceRouteCtlRowStatus_createAndWait:
            if (res == TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
                
            row_status = VAL_traceRouteCtlRowStatus_createAndWait;
            break;
            
        case VAL_traceRouteCtlRowStatus_notInService:     
            /*  NOT SUPPORT
             */
            return TRACEROUTE_TYPE_FAIL;
            break;
                  
        case VAL_traceRouteCtlRowStatus_destroy:   
        case VAL_traceRouteCtlRowStatus_active: 
            /* Entry does not exist in control_table
             */
            if (res != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
                
            if (local_entry.trace_route_ctl_admin_status != ctrl_entry.trace_route_ctl_admin_status)
            {
                admin_status_changed = TRUE;    
            } /* end of if */
             
            row_status = local_entry.trace_route_ctl_rowstatus;         
                              
            break;

        case VAL_traceRouteCtlRowStatus_createAndGo:
            /* The given key already exist in control table.
               Cannot create a duplicate entry.
             */
            if (res == TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
            
            //if (ctrl_entry.trace_route_ctl_target_address_len == 0)
            //    return TRACEROUTE_TYPE_FAIL;
            
            if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
            {
                admin_status_changed = TRUE;      /* from dedault value,"disable", to enable. */
            }
            
            row_status = VAL_traceRouteCtlRowStatus_createAndGo;
            break;  
                    
        default:
            return TRACEROUTE_TYPE_FAIL;
            break;          
    } /* end of switch */
    
    action = ctrl_entry.trace_route_ctl_rowstatus;
    /* VAL_RowStatus_notReady is not a valid action in l_rstatus.c, use VAL_RowStatus_notInService instead */
    if(action == VAL_traceRouteCtlRowStatus_notReady)
    {
        action = VAL_traceRouteCtlRowStatus_notInService;
    }
    row_status_result = L_RSTATUS_Fsm(action, 
                            &row_status,
                            TRACEROUTE_OM_SemanticCheck,
                            (void*)&ctrl_entry);

    switch (row_status_result)                          
    {
        case L_RSTATUS_NOTEXIST_2_NOTREADY:
        
            if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;
                
            ctrl_entry.trace_route_ctl_rowstatus = row_status;
            
            if ( (res = TRACEROUTE_OM_CreateNewCtrlEntry(ctrl_entry)) != TRACEROUTE_TYPE_OK) 
                return res;
            
            break;
            
        case L_RSTATUS_NOTEXIST_2_ACTIVE:
        
            ctrl_entry.trace_route_ctl_rowstatus = row_status;
            
            if ( (res = TRACEROUTE_OM_CreateNewCtrlEntry(ctrl_entry)) != TRACEROUTE_TYPE_OK) 
                return TRACEROUTE_TYPE_FAIL;
            
            if ((res = TRACEROUTE_OM_FindKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
                return TRACEROUTE_TYPE_FAIL;
            
            if (admin_status_changed)
            {
                if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                {
                    memcpy(&trace_route_control_table[list_elm.table_index], &ctrl_entry,
                            sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
                            
                    if ((res = TRACEROUTE_MGR_CreateWorkSpace(list_elm.table_index, &ctrl_entry)) != TRACEROUTE_TYPE_OK) 
                        return TRACEROUTE_TYPE_FAIL;
                    
                    TRACEROUTE_OM_ClearProbHistoryEntry(list_elm.table_index);
                    TRACEROUTE_OM_AssociateResultTable(list_elm.table_index, &ctrl_entry);
                    
                    if ((res = TRACEROUTE_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_traceRouteCtlAdminStatus_enabled)) != TRACEROUTE_TYPE_OK)
                        return TRACEROUTE_TYPE_FAIL;   
                }
                /*else  from notexist_2_active, it's impossible to disassociate.
                    it's impossible admin_status_change from disabe to disable.*/
            }
            break;          

        case L_RSTATUS_ACTIVE_2_NOTEXIST:
        case L_RSTATUS_NOTREADY_2_NOTEXIST:

            memset(&trace_route_control_table[list_elm.table_index], 0, 
                   sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
                   
            memset(&trace_route_result_table[list_elm.table_index], 0, 
                   sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
            
            if ((res = TRACEROUTE_MGR_FreeWorkSpace(list_elm.table_index)) != TRACEROUTE_TYPE_OK)
                return res;
            
            if ((res = TRACEROUTE_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_traceRouteCtlAdminStatus_disabled)) != TRACEROUTE_TYPE_OK)
                return res;      
                   
            TRACEROUTE_OM_ClearProbHistoryEntry(list_elm.table_index);

            if (!L_SORT_LST_Delete(&sort_trace_route_list, &list_elm))
                return TRACEROUTE_TYPE_FAIL;
                
            /*
             * If the traceroute control table is full before,
             * set the next_available_index to the index just freed
             */
            if((next_available_index == TRACEROUTE_TYPE_INVALID_WORK_SPACE) 
                    && (current_control_entry == SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE))
            {
                next_available_index = list_elm.table_index;
            }
            
            current_control_entry--;
            break;

        case L_RSTATUS_NOTREADY_2_NOTREADY:
        case L_RSTATUS_ACTIVE_2_NOTREADY:
        
            ctrl_entry.trace_route_ctl_rowstatus = row_status;
            
            if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;
            
            memcpy(&trace_route_control_table[list_elm.table_index], &ctrl_entry,
                   sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
            break;
            
        case L_RSTATUS_NOTREADY_2_ACTIVE:
            //if ( local_entry.trace_route_ctl_target_address_len == 0)
            //    return TRACEROUTE_TYPE_FAIL;
        
            ctrl_entry.trace_route_ctl_rowstatus = row_status;
            
            if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                ctrl_entry.trace_route_ctl_admin_status = VAL_traceRouteCtlAdminStatus_disabled;
            
            memcpy(&trace_route_control_table[list_elm.table_index], &ctrl_entry,
                        sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
            
            if (admin_status_changed)
            {
                if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                {
                    memcpy(&trace_route_control_table[list_elm.table_index], &ctrl_entry,
                            sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
                            
                    if ((res = TRACEROUTE_MGR_CreateWorkSpace(list_elm.table_index, &ctrl_entry)) != TRACEROUTE_TYPE_OK) 
                        return res;
                    
                    TRACEROUTE_OM_ClearProbHistoryEntry(list_elm.table_index);        
                    TRACEROUTE_OM_AssociateResultTable(list_elm.table_index, &ctrl_entry);
                    
                    if ((res = TRACEROUTE_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_traceRouteCtlAdminStatus_enabled)) != TRACEROUTE_TYPE_OK)
                        return res;   
                    
                }
                /*else  from notready_2_active, it's impossible to disassociate.*/
            }
            
            break;
            
        case L_RSTATUS_ACTIVE_2_ACTIVE:
        
            ctrl_entry.trace_route_ctl_rowstatus = row_status;

            if (admin_status_changed)
            {
                if (ctrl_entry.trace_route_ctl_admin_status == VAL_traceRouteCtlAdminStatus_enabled)
                {

                    if ((res = TRACEROUTE_MGR_CreateWorkSpace(list_elm.table_index, &ctrl_entry)) != TRACEROUTE_TYPE_OK) 
                        return res;

                    TRACEROUTE_OM_ClearProbHistoryEntry(list_elm.table_index);
                    TRACEROUTE_OM_AssociateResultTable(list_elm.table_index, &ctrl_entry);
                    
                    if ((res = TRACEROUTE_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_traceRouteCtlAdminStatus_enabled)) != TRACEROUTE_TYPE_OK)
                        return res;
                }
                else
                {
                    TRACEROUTE_OM_DisAssociateResultTable(list_elm.table_index);
                    
                    if ((res = TRACEROUTE_MGR_FreeWorkSpace(list_elm.table_index)) != TRACEROUTE_TYPE_OK)
                        return res;
                    
                    if ((res = TRACEROUTE_MGR_SetWorkSpaceAdminStatus(list_elm.table_index,VAL_traceRouteCtlAdminStatus_disabled)) != TRACEROUTE_TYPE_OK)
                        return res;            
                }
            }
            
            memcpy(&trace_route_control_table[list_elm.table_index], &ctrl_entry,
                    sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));
                    
            break;
        
        default:
            return TRACEROUTE_TYPE_FAIL;
            break;

    } /* end of switch */
    
    return TRACEROUTE_TYPE_OK;
} /* end of TRACEROUTE_OM_SetTraceRouteCtlEntry() */
                                           

/* FUNCTION NAME: TRACEROUTE_OM_AppendProbePacketResult
 * PURPOSE: 
 *          To append specific prob histroy entry to the end of the last histroy entry
 * INPUT:  
 *          table_index - A key index to identify prob history entry for different owner_index and test name
 *          prob_history_entry - contains values to set to prob history entry.
 *
 * OUTPUT:  
 *          None.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_OM_AppendProbePacketResult(UI32_T table_index, TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  prob_packet_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */

    UI32_T      current_index;
	UI32_T      index;

    /*  BODY
     */     
    TRACEROUTE_OM_ENTER_CRITICAL_SECTION();  

    index = prob_packet_entry.trace_route_probe_history_index + prob_history_index_table[table_index].prob_history_head -1;

	if(trace_route_prob_history_table[index].trace_route_ctl_owner_index_len != 0)
	{
		TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
		return TRACEROUTE_TYPE_OK;
	}
	
    current_index = prob_history_index_table[table_index].current_prob_history;

    memcpy(&trace_route_prob_history_table[current_index], &prob_packet_entry,
           sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    
    if (++current_index > prob_history_index_table[table_index].prob_history_end)
    {
        prob_history_index_table[table_index].current_prob_history = 
            prob_history_index_table[table_index].prob_history_head;
    }       
    else
        prob_history_index_table[table_index].current_prob_history++;        

	TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_AppendProbePacketResult() */



/* FUNCTION NAME: TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry
 * PURPOSE: 
 *          To get the specified available prob history entry
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
 *          prob_history_entry_p - entry that contains prob packet information
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE \
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 */ 
UI32_T  TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                     UI32_T owner_index_len,
                                                     char  *test_name_p,
                                                     UI32_T test_name_len,
                                                     UI32_T history_index,
                                                     UI32_T history_hop_index,
                                                     UI32_T history_probe_index,
                                                     TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;    
    UI32_T      res, history_table_index;
    
    /*  BODY
     */ 
    if ((owner_index_p == NULL) || (test_name_p == NULL) || (prob_history_entry_p == NULL))
        return TRACEROUTE_TYPE_INVALID_ARG; 
    
    TRACEROUTE_OM_ENTER_CRITICAL_SECTION(); 

    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  

    memcpy(local_elm.owner_index, owner_index_p, owner_index_len);
    memcpy(local_elm.test_name, test_name_p, test_name_len);
    local_elm.owner_index_len = owner_index_len;
    local_elm.test_name_len = test_name_len;
    
    if ((res = L_SORT_LST_Get(&sort_trace_route_list, &local_elm)) != TRUE)
    {
        TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return TRACEROUTE_TYPE_FAIL;
    }
            
    history_table_index = prob_history_index_table[local_elm.table_index].prob_history_head + (history_index-1);
    
    if ((trace_route_prob_history_table[history_table_index].trace_route_probe_history_index == history_index) &&
        (trace_route_prob_history_table[history_table_index].trace_route_probe_history_hop_index == history_hop_index) &&
        (trace_route_prob_history_table[history_table_index].trace_route_probe_history_probe_index == history_probe_index))
    {
        memcpy(prob_history_entry_p, &trace_route_prob_history_table[history_table_index], 
                       sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
                
        TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return TRACEROUTE_TYPE_OK;
    }
    
    TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
    return TRACEROUTE_TYPE_FAIL;
} /* end of TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry() */
                                                    
 
 
/* FUNCTION NAME: TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry
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
UI32_T TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry(char  *owner_index_p,
                                                         UI32_T owner_index_len,
                                                         char  *test_name_p,
                                                         UI32_T test_name_len,
                                                         UI32_T history_index,
                                                         UI32_T history_hop_index,
                                                         UI32_T history_probe_index,
                                                         TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */    
    TRACEROUTE_SORTLST_ELM_T    local_elm;    
    UI32_T      compare_index = 0;
    BOOL_T      ctl_entry_existed=FALSE, next_elm_existed=FALSE, is_valid_addr = FALSE;

    /*  BODY
     */ 
    if ((owner_index_p == NULL) || (test_name_p == NULL) || (prob_history_entry_p == NULL))
        return TRACEROUTE_TYPE_INVALID_ARG; 
    
    TRACEROUTE_OM_ENTER_CRITICAL_SECTION(); 
    
    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(local_elm.owner_index, owner_index_p, owner_index_len);
    memcpy(local_elm.test_name, test_name_p, test_name_len);
    local_elm.owner_index_len = owner_index_len;
    local_elm.test_name_len = test_name_len;
    
    ctl_entry_existed = L_SORT_LST_Get(&sort_trace_route_list, &local_elm);
    
    /* If the input index is not existed, 
     * find the nearest one which has probe history entry.
     * We also get the first probe history entry here.
     */
    if(ctl_entry_existed == TRUE)
    {
        if(history_index !=  SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY)
        {
            if(history_index == 0)
            {
                compare_index = prob_history_index_table[local_elm.table_index].prob_history_head;
            }
            else if((history_index > 0) && (history_index <  SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY))
            {
                compare_index = prob_history_index_table[local_elm.table_index].prob_history_head + (history_index-1);
                compare_index++;
            }
            
            //if (trace_route_prob_history_table[compare_index].trace_route_probe_history_haddr_len > 0)
            switch(trace_route_prob_history_table[compare_index].trace_route_probe_history_haddr_type)
            {
                case L_INET_ADDR_TYPE_IPV4:
                case L_INET_ADDR_TYPE_IPV4Z:
                    is_valid_addr = memcmp(&trace_route_prob_history_table[compare_index].trace_route_probe_history_haddr.addr, traceroute_ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN);
                    break;  
#if (SYS_CPNT_IPV6 == TRUE)
                case L_INET_ADDR_TYPE_IPV6:
                case L_INET_ADDR_TYPE_IPV6Z:
                    is_valid_addr = memcmp(&trace_route_prob_history_table[compare_index].trace_route_probe_history_haddr.addr, traceroute_ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN);
                  break;
#endif /* #if (SYS_CPNT_IPV6 == TRUE) */
                default:
                    break;
            }
            
            
            if (is_valid_addr)
            {
                /* Get the next probe history entry immediately*/
                memcpy(prob_history_entry_p, &trace_route_prob_history_table[compare_index], 
                           sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
                    
                TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
                return TRACEROUTE_TYPE_OK;
            }
        }
    }

    while((next_elm_existed = L_SORT_LST_Get_Next(&sort_trace_route_list, &local_elm)))
    {
        compare_index = prob_history_index_table[local_elm.table_index].prob_history_head;
        
        if (trace_route_prob_history_table[compare_index].trace_route_probe_history_index >= 1)
            break;
    }
    
    /* If there's no any result entry in the table. */
    if(next_elm_existed != TRUE)
    {
        TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return TRACEROUTE_TYPE_FAIL;
    }
    
    memcpy(prob_history_entry_p, &trace_route_prob_history_table[compare_index], 
           sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
    
    TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
    return TRACEROUTE_TYPE_OK;
} /* end of TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry() */
                                                        
/* FUNCTION NAME: TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB
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
UI32_T  TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(char  *owner_index_p,
                                                                  UI32_T owner_index_len,
                                                                  char  *test_name_p,
                                                                  UI32_T test_name_len,
                                                                  UI32_T history_index,
                                                                  UI32_T history_hop_index,
                                                                  UI32_T history_probe_index,
                                                                  TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T  *prob_history_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */    
    TRACEROUTE_SORTLST_ELM_T    local_elm;    
    UI32_T      compare_index = 0;
    BOOL_T      ctl_entry_existed=FALSE;

    /*  BODY
     */ 
    
    if ((owner_index_p == NULL) || (test_name_p == NULL) || (prob_history_entry_p == NULL))
        return TRACEROUTE_TYPE_INVALID_ARG; 
    
    TRACEROUTE_OM_ENTER_CRITICAL_SECTION(); 
    
    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(local_elm.owner_index, owner_index_p, owner_index_len);
    memcpy(local_elm.test_name, test_name_p, test_name_len);
    local_elm.owner_index_len = owner_index_len;
    local_elm.test_name_len = test_name_len;
    
    ctl_entry_existed = L_SORT_LST_Get(&sort_trace_route_list, &local_elm);
    
    if(ctl_entry_existed == FALSE)
    {
        TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return TRACEROUTE_TYPE_FAIL;
    }
    
    if(history_index <  SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY)
    {
        if(history_index == 0)
        {
            compare_index = prob_history_index_table[local_elm.table_index].prob_history_head;
        }
        else if((history_index > 0) && (history_index <  SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY))
        {
            compare_index = prob_history_index_table[local_elm.table_index].prob_history_head + (history_index-1);
            compare_index++;
        }
        
        /*Check the traceroute operation is completed or the probe is recieved */
        if (trace_route_prob_history_table[compare_index].trace_route_probe_history_index == 0)
        {
            if (trace_route_result_table[local_elm.table_index].trace_route_results_oper_status 
            	== VAL_traceRouteResultsOperStatus_enabled)
            {	
                TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
                return TRACEROUTE_TYPE_PROCESS_NOT_COMPLETE;   
            }				
            else		
            {
            	TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();	
            	return TRACEROUTE_TYPE_NO_MORE_ENTRY;
            }    
        }

        memcpy(prob_history_entry_p, &trace_route_prob_history_table[compare_index], 
                   sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T));
            
        TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return TRACEROUTE_TYPE_OK;
    }
    
    TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
    return TRACEROUTE_TYPE_FAIL;
} /* end of TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB() */

/* FUNCTION NAME: TRACEROUTE_OM_SetTraceRouteResultsEntry
 * PURPOSE: 
 *          To set specific traceroute result entry by the specific entry index
 * INPUT:  
 *          key_index - the index of the result table.
 *          result_entry - contains values to set to prob history entry.
 *
 * OUTPUT:  
 *          None.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */ 
UI32_T  TRACEROUTE_OM_SetTraceRouteResultsEntry(UI32_T  key_index,
                                                TRACEROUTE_TYPE_TraceRouteResultsEntry_T   result_entry)
{
    /* LOCAL VARIABLE DECLARATION
     */

    /*  BODY
     */ 
    memcpy(&trace_route_result_table[key_index], &result_entry, sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
            
    return TRACEROUTE_TYPE_OK;
} /* end of TRACEROUTE_OM_SetTraceRouteResultsEntry() */


/* FUNCTION NAME: TRACEROUTE_OM_GetTraceRouteResultsEntry
 * PURPOSE: 
 *          To the specific result entry base on the given index
 * INPUT:  
 *          result_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation. 
 *          result_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          result_entry_p->trace_route_ctl_test_name - The test name of the trace route operation. 
 *          result_entry_p->trace_route_ctl_test_name_len - The length of the test name
 * OUTPUT:  
 *          result_entry_p - contains values to get to result entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_OM_GetTraceRouteResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;
    UI32_T res;
    /* BODY 
     */
    if (result_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
    
    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    
    memcpy(local_elm.owner_index,  result_entry_p->trace_route_ctl_owner_index, result_entry_p->trace_route_ctl_owner_index_len);
    memcpy(local_elm.test_name, result_entry_p->trace_route_ctl_test_name, result_entry_p->trace_route_ctl_test_name_len);
    local_elm.owner_index_len = result_entry_p->trace_route_ctl_owner_index_len;
    local_elm.test_name_len = result_entry_p->trace_route_ctl_test_name_len;
    
    if ((res = L_SORT_LST_Get(&sort_trace_route_list, &local_elm)) != TRUE)
        return TRACEROUTE_TYPE_FAIL;
    
    memcpy(result_entry_p, &trace_route_result_table[local_elm.table_index], sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));

    return TRACEROUTE_TYPE_OK;

} /* end of TRACEROUTE_OM_GetTraceRouteResultsEntry() */



/* FUNCTION NAME: TRACEROUTE_OM_GetFirstActiveResultsEntry
 * PURPOSE: 
 *          To get the first active result entry which oper status is enabled.
 * INPUT:  
 *          index_p - Index to the result entry.
 * OUTPUT:  
 *          result_entry_p - contains values to set to prob history entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_OM_GetFirstActiveResultsEntry(UI32_T *index_p, TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    /* BODY 
     */  
    if ((index_p == NULL) || (result_entry_p == NULL))
        return TRACEROUTE_TYPE_FAIL; 
     
    for (local_index = 0; local_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; local_index++)
    {
        if (trace_route_result_table[local_index].trace_route_results_oper_status == VAL_traceRouteResultsOperStatus_enabled)
        {
            *index_p = local_index;
            memcpy(result_entry_p, &trace_route_result_table[local_index], sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
            return TRACEROUTE_TYPE_OK;
        } /* end of if */
    } /* end of for */
    return TRACEROUTE_TYPE_FAIL;

} /* end of TRACEROUTE_OM_GetFirstActiveResultsEntry() */


/* FUNCTION NAME: TRACEROUTE_OM_GetNextActiveResultsEntry
 * PURPOSE: 
 *          To get next active result entry which oper status is enabled.
 * INPUT:  
 *          index_p - Index to the result entry.
 * OUTPUT:  
 *          index_p - Index to the result entry.
 *          result_entry_p - next available active result entry
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_OM_GetNextActiveResultsEntry(UI32_T *index_p, TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    /* BODY 
     */  
     
    if ((index_p == NULL) || (result_entry_p == NULL))
        return TRACEROUTE_TYPE_FAIL;
        
    local_index = *index_p;

    local_index++;

    for (; local_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; local_index++)
    {
		
        if (trace_route_result_table[local_index].trace_route_results_oper_status == VAL_traceRouteResultsOperStatus_enabled)
        {
            *index_p = local_index;
            memcpy(result_entry_p, &trace_route_result_table[local_index], sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
            return TRACEROUTE_TYPE_OK;
        }
    }
    return TRACEROUTE_TYPE_NO_MORE_ENTRY;
}

/* FUNCTION NAME: TRACEROUTE_OM_GetFirstResultsEntry
 * PURPOSE: 
 *          To get the first active result entry
 * INPUT:  
 *          None.
 * OUTPUT:  
 *          result_entry_p - contains values to set to prob history entry.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_OM_GetFirstResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    /* BODY 
     */
     
    if (result_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL; 
    
    for (local_index = 0; local_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; local_index++)
    {
        if ((trace_route_result_table[local_index].trace_route_ctl_owner_index_len > 0) && (trace_route_result_table[local_index].trace_route_ctl_test_name_len > 0))
        {
            memcpy(result_entry_p, &trace_route_result_table[local_index], sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
            return TRACEROUTE_TYPE_OK;
        } /* end of if */
    } /* end of for */
    return TRACEROUTE_TYPE_FAIL;

} /* end of TRACEROUTE_OM_GetFirstResultsEntry() */

/* FUNCTION NAME: TRACEROUTE_OM_GetNextResultsEntry
 * PURPOSE: 
 *          To get the next result entry
 * INPUT:  
 *          result_entry_p->trace_route_ctl_owner_index - The owner index of the trace route operation. 
 *          result_entry_p->trace_route_ctl_owner_index_len - The length of the owner index.
 *          result_entry_p->trace_route_ctl_test_name - The test name of the trace route operation. 
 *          result_entry_p->trace_route_ctl_test_name_len - The length of the test name
 * OUTPUT:  
 *          result_entry_p - next available result entry
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */                 
UI32_T  TRACEROUTE_OM_GetNextResultsEntry(TRACEROUTE_TYPE_TraceRouteResultsEntry_T *result_entry_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;
    BOOL_T      res;
    /* BODY 
     */  
    if (result_entry_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
    
    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    
    memcpy(local_elm.owner_index,  result_entry_p->trace_route_ctl_owner_index, result_entry_p->trace_route_ctl_owner_index_len);
    memcpy(local_elm.test_name, result_entry_p->trace_route_ctl_test_name, result_entry_p->trace_route_ctl_test_name_len);
    local_elm.owner_index_len = result_entry_p->trace_route_ctl_owner_index_len;
    local_elm.test_name_len = result_entry_p->trace_route_ctl_test_name_len;
    
    while((res = L_SORT_LST_Get_Next(&sort_trace_route_list, &local_elm)))
    {
        if((trace_route_result_table[local_elm.table_index].trace_route_ctl_owner_index_len != 0) &&
               (trace_route_result_table[local_elm.table_index].trace_route_ctl_test_name_len != 0))
            break;
    }
    
    /* If there's no any result entry in the table. */
    if(res == FALSE)
        return TRACEROUTE_TYPE_FAIL;
    
    memcpy(result_entry_p, &trace_route_result_table[local_elm.table_index], sizeof(TRACEROUTE_TYPE_TraceRouteResultsEntry_T));
    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_GetNextResultsEntry() */

/* FUNCTION NAME: TRACEROUTE_OM_IsTraceRouteControlEntryExist
 * PURPOSE: 
 *          To check if the specific control entry information exist
 *          in OM or not.
 * INPUT:  
 *          owner_index_p - The owner index of the trace route operation. 
 *          owner_index_len - The length of the owner index.
 *          test_name_p - The test name of the trace route operation. 
 *          test_name_len - The length of the test name.
 * OUTPUT:  
 *          None.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */     
UI32_T TRACEROUTE_OM_IsTraceRouteControlEntryExist(char *owner_index_p, UI32_T owner_index_len, 
                                                   char *test_name_p, UI32_T test_name_len)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    list_elm;
    UI32_T      res;

    /*  BODY
     */ 
    if ((owner_index_p == NULL) || (test_name_p == NULL) ||
        (owner_index_len <= 0) || (owner_index_len > SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE) ||
        (test_name_len <= 0) || (test_name_len > SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE))
        return TRACEROUTE_TYPE_INVALID_ARG; 

    
    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    
    memcpy(list_elm.owner_index, owner_index_p, owner_index_len);
    memcpy(list_elm.test_name, test_name_p, test_name_len);
    list_elm.owner_index_len = owner_index_len;
    list_elm.test_name_len = test_name_len;
    
    if ((res = TRACEROUTE_OM_FindKeyIndex(&list_elm)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;
    
    return TRACEROUTE_TYPE_OK;
 
} /* end of TRACEROUTE_OM_IsTraceRouteControlEntryExist() */
 
/* FUNCTION NAME: TRACEROUTE_OM_TraceRouteKeyToTableIndex
 * PURPOSE: 
 *          Find the table index from the key of the table.
 * INPUT:  
 *          elm_p->owner_index - owner index is the task name
 *          elm_p->owner_index_len - the length of the owner index.
 *          elm_p->test_name - the specific test session name
 *          elm_p->test_name_len - the length of the test name index.   
 * OUTPUT:  
 *          table_index_p - table indeex
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */     
UI32_T TRACEROUTE_OM_TraceRouteKeyToTableIndex(TRACEROUTE_SORTLST_ELM_T *elm_p, UI32_T *table_index_p)
{
    /* LOCAL VARIABLE DECLARATION
     */      
    UI32_T      res;
    
    /*  BODY
     */
    if ((elm_p == NULL) || (table_index_p == NULL))
        return TRACEROUTE_TYPE_INVALID_ARG; 

    if ((res = TRACEROUTE_OM_FindKeyIndex(elm_p)) != TRACEROUTE_TYPE_OK)
        return TRACEROUTE_TYPE_FAIL;

    *table_index_p = elm_p->table_index;
    
    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_TraceRouteKeyToTableIndex() */


/* FUNCTION NAME: TRACEROUTE_OM_TraceRouteTableIndexToKey
 * PURPOSE: 
 *          Find the key of the table from the table index.
 * INPUT:  
 *          table_index - table index
 * OUTPUT:  
 *          elm_p - includes owner_index, owner_index_len, test_name, test_name_len, table_index;
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */     
UI32_T TRACEROUTE_OM_TraceRouteTableIndexToKey(UI32_T table_index, TRACEROUTE_SORTLST_ELM_T *elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */    
    TRACEROUTE_SORTLST_ELM_T    list_elm;    
    
    /*  BODY
     */ 
    if (elm_p == NULL)
        return TRACEROUTE_TYPE_INVALID_ARG; 

    memset(&list_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));  
    if (L_SORT_LST_Get_1st(&sort_trace_route_list, &list_elm))
    {
        if (list_elm.table_index == table_index)
        {
            memcpy(elm_p, &list_elm, sizeof(TRACEROUTE_SORTLST_ELM_T));
            return TRACEROUTE_TYPE_OK;
        } /* end of if */
    }
    

    while (L_SORT_LST_Get_Next(&sort_trace_route_list, &list_elm))
    {       
        if (list_elm.table_index == table_index)
        {
            memcpy(elm_p, &list_elm, sizeof(TRACEROUTE_SORTLST_ELM_T));
            return TRACEROUTE_TYPE_OK;
        } /* end of if */        
    } /* end of while */
    
    return TRACEROUTE_TYPE_FAIL;
       
} /* end of TRACEROUTE_OM_TraceRouteKeyToTableIndex() */


/* FUNCTION NAME: TRACEROUTE_MGR_OM_DisplayAllProbeHistory
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
void TRACEROUTE_MGR_OM_DisplayAllProbeHistory(void)
{
    int i;
    //UI8_T str_daytime[50];

    /* DEBUG */
    printf("TRACEROUTE_MGR_OM_DisplayAllProbeHistory\n");
    printf ("rc: \n" \
        "TRACEROUTE_TYPE_NETWORK_UNREACHABLE      1\n" \
        "TRACEROUTE_TYPE_HOST_UNREACHABLE         2\n" \
        "TRACEROUTE_TYPE_PROTOCOL_UNREACHABLE     3\n" \
        "TRACEROUTE_TYPE_FRAGMENTATION_NEEDED     4\n" \
        "TRACEROUTE_TYPE_EXCEED_MAX_TTL           5\n" \
        "TRACEROUTE_TYPE_PORT_UNREACHABLE         6\n" \
        "TRACEROUTE_TYPE_SRT_UNREACHABLE          7\n" \
        "TRACEROUTE_TYPE_NETWORK_UNKNOWN          8\n" \
        "TRACEROUTE_TYPE_HOST_UNKNOWN             9\n" \
        "TRACEROUTE_TYPE_NO_RESPONSE              19\n");

   /* the filed name string may would be truncated */
   printf ("%-10.10s %-10.10s %-10.10s %-10.10s %-10.10s %-10.10s %-10.10s\n", 
    "index", "owner_index", "test_name", "history_index", 
    "history_hop_index", "history_index", "rc");

   for(i=0;i <TRACEROUTE_PROB_HISTORY_TABLE_SIZE-1; i++)
   {

        if (trace_route_prob_history_table[i].trace_route_ctl_owner_index == NULL)
        {
            continue;
        }
#if 0
        /* convert daytime */
        memset(str_daytime, 0, sizeof(str_daytime));

        if(PING_TYPE_OK != PING_OM_ConvertDayTime(str_daytime, trace_route_prob_history_table[i].ping_probe_history_time, trace_route_prob_history_table[i].ping_probe_history_time_len))
        {
            printf("daytime converted failed.\n");
        }
#endif
        printf ("%-10.10d %-10.10s %-10.10s %-10ld %-10ld %-10ld %-10ld\n",
        i,
        trace_route_prob_history_table[i].trace_route_ctl_owner_index,
        trace_route_prob_history_table[i].trace_route_ctl_test_name,      
        (long)trace_route_prob_history_table[i].trace_route_probe_history_index,
        (long)trace_route_prob_history_table[i].trace_route_probe_history_hop_index,
        (long)trace_route_prob_history_table[i].trace_route_probe_history_probe_index,
        (long)trace_route_prob_history_table[i].trace_route_probe_history_last_rc);
   }
}

/* LOCAL SUBPROGRAM DECLARATIONS 
 */
/* FUNCTION NAME: TRACEROUTE_OM_Compare
 * PURPOSE: 
 *          Compare function for traceroute_sort list.  
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
static int TRACEROUTE_OM_Compare(void *elm1_p, void *elm2_p)
{
    TRACEROUTE_SORTLST_ELM_T    *element1, *element2;
    UI32_T length1=0,length2=0;
    int   r1, r2, r3, r4;
    
    if ((elm1_p == NULL) || (elm2_p == NULL))
        return TRACEROUTE_TYPE_INVALID_ARG; 
    
    element1 = (TRACEROUTE_SORTLST_ELM_T *)elm1_p;
    element2 = (TRACEROUTE_SORTLST_ELM_T *)elm2_p;
    
    /* The sorting precedence is ckecked by r1, r2, r3 then r4*/
    length1 = (element2->owner_index_len >= element1->owner_index_len ? element2->owner_index_len : element1->owner_index_len);
    length2 = (element2->test_name_len >= element1->test_name_len ? element2->test_name_len : element1->test_name_len);
    
    r1 = (element1->owner_index_len - element2->owner_index_len);
    r2 = memcmp(element1->owner_index, element2->owner_index, length1);
    r3 = (element1->test_name_len - element2->test_name_len);
    r4 = memcmp(element1->test_name, element2->test_name, length2);

    /* Both owner index and test_name are identical.
     */
    
    if ((r1 == 0) && (r2 == 0) && (r3 == 0) && (r4 == 0))
        return 0;
    else if ((r1 == 0) && (r2 == 0) && (r3 == 0))
        return r4;
    else if ((r1 == 0) && (r2 == 0))
        return r3;
    else if (r1 == 0)
        return r2;
    else
        return r1;
                
} /* end of TRACEROUTE_OM_Compare() */


/* FUNCTION NAME: TRACEROUTE_OM_InitProbHistoryIndexTable
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
static void TRACEROUTE_OM_InitProbHistoryIndexTable(void)
{
    /* LOCAL VARIABLE DECLARATION
     */
    UI32_T  local_index;
    UI32_T  head_index, tail_index;
    
    /* BODY
     */
    head_index = tail_index = 0;     
    
    for (local_index= 0; local_index < SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE; local_index++)
    {
        head_index = local_index * SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY;
        tail_index = head_index + SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY - 1;
        
        prob_history_index_table[local_index].prob_history_head = head_index;
        prob_history_index_table[local_index].prob_history_end = tail_index;
        prob_history_index_table[local_index].current_prob_history = head_index;
    
    } /* end of for */
         
    return;
} /* end of TRACEROUTE_OM_InitProbHistoryIndexTable() */

/* FUNCTION NAME: TRACEROUTE_OM_FindFirstKeyIndex
 * PURPOSE: 
 *          Find the first table index.
 * INPUT:  
 *          None.
 *
 * OUTPUT:  
 *          list_elm->table_index - index to all tables that stores
 *          control info, result info, and prob history info.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */    
static UI32_T   TRACEROUTE_OM_FindFirstKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;
    /*  BODY
     */
    if (list_elm_p == NULL)
        return TRACEROUTE_TYPE_FAIL;

    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T)); 
    
    if (L_SORT_LST_Get_1st(&sort_trace_route_list, &local_elm))
    {
        memcpy(list_elm_p, &local_elm, sizeof(TRACEROUTE_SORTLST_ELM_T));
        return TRACEROUTE_TYPE_OK;
    }
        
    return TRACEROUTE_TYPE_FAIL;
} /* end of TRACEROUTE_OM_FindFirstKeyIndex() */

/* FUNCTION NAME: TRACEROUTE_OM_FindKeyIndex
 * PURPOSE: 
 *          Find the table index of the specific entry.
 * INPUT:  
 *          list_elm_p->owner_index - owner index is the task name
 *          list_elm_p->owner_index_len - the length of the owner index.
 *          list_elm_p->test_name - the specific test session name
 *          list_elm_p->test_name_len - the length of the test name index.         
 *
 * OUTPUT:  
 *          list_elm_p->table_index - index to all tables that stores
 *          control info, result info, and prob history info.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */    
static UI32_T   TRACEROUTE_OM_FindKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;
    /*  BODY
     */
    if (list_elm_p == NULL)
        return TRACEROUTE_TYPE_FAIL;

    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T)); 
    
    if (L_SORT_LST_Get_1st(&sort_trace_route_list, &local_elm))
    {
        if ((!memcmp(local_elm.owner_index, list_elm_p->owner_index, list_elm_p->owner_index_len)) &&
            (!memcmp(local_elm.test_name, list_elm_p->test_name, list_elm_p->test_name_len)) &&
            (local_elm.owner_index_len == list_elm_p->owner_index_len) &&
            (local_elm.test_name_len == list_elm_p->test_name_len))
        {
            list_elm_p->table_index = local_elm.table_index;
            return TRACEROUTE_TYPE_OK;
        }
    }
           
    while (L_SORT_LST_Get_Next(&sort_trace_route_list, &local_elm))
    {
        if ((!memcmp(local_elm.owner_index, list_elm_p->owner_index, list_elm_p->owner_index_len)) &&
            (!memcmp(local_elm.test_name, list_elm_p->test_name, list_elm_p->test_name_len)) &&
            (local_elm.owner_index_len == list_elm_p->owner_index_len) &&
            (local_elm.test_name_len == list_elm_p->test_name_len))
        {
            list_elm_p->table_index = local_elm.table_index;
            return TRACEROUTE_TYPE_OK;
        } /* end of if */
        
    } /* end of while */
    
    return TRACEROUTE_TYPE_FAIL;
} /* end of TRACEROUTE_OM_FindKeyIndex() */

#if 0
/* FUNCTION NAME: TRACEROUTE_OM_FindNextKeyIndex
 * PURPOSE: 
 *          Find the next table index of the specific entry.
 * INPUT:  
 *          list_elm_p->owner_index - owner index is the task name
 *          list_elm_p->owner_index_len - the length of the owner index.
 *          list_elm_p->test_name - the specific test session name
 *          list_elm_p->test_name_len - the length of the test name index.           
 *
 * OUTPUT:  
 *          list_elm_p->table_index - index to all tables that stores
 *          control info, result info, and prob history info.
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          None
 */    
static UI32_T   TRACEROUTE_OM_FindNextKeyIndex(TRACEROUTE_SORTLST_ELM_T *list_elm_p)
{
    /* LOCAL VARIABLE DECLARATION
     */
    TRACEROUTE_SORTLST_ELM_T    local_elm;
    UI32_T      res;
    BOOL_T      record_found = FALSE;
    
    /*  BODY
     */
    if (list_elm_p == NULL)
        return TRACEROUTE_TYPE_FAIL;
        
    memset(&local_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    
    if ((res = L_SORT_LST_Get_1st(&sort_trace_route_list, &local_elm)) != TRUE)
        return TRACEROUTE_TYPE_FAIL;
    
    do
    {
        if (record_found)
        {
            memcpy(list_elm_p->owner_index,local_elm.owner_index,local_elm.owner_index_len);
            memcpy(list_elm_p->test_name,local_elm.test_name,local_elm.test_name_len);
            list_elm_p->owner_index_len = local_elm.owner_index_len;
            list_elm_p->test_name_len = local_elm.test_name_len;
            list_elm_p->table_index = local_elm.table_index;
            return TRACEROUTE_TYPE_OK;
        }
        if ((!memcmp(local_elm.owner_index, list_elm_p->owner_index, list_elm_p->owner_index_len)) &&
            (!memcmp(local_elm.test_name, list_elm_p->test_name, list_elm_p->test_name_len)) &&
            (local_elm.owner_index_len == list_elm_p->owner_index_len) &&
            (local_elm.test_name_len == list_elm_p->test_name_len))
        {
            record_found = TRUE;
            continue;
        } /* end of if */
        
    }while (L_SORT_LST_Get_Next(&sort_trace_route_list, &local_elm)); /* end of while */
    
    return TRACEROUTE_TYPE_FAIL;
} /* end of TRACEROUTE_OM_FindNextKeyIndex() */
#endif


/* FUNCTION NAME: TRACEROUTE_OM_SemanticCheck
 * PURPOSE: 
 *          Validate the given control entry fields before save it
 *          to control table.
 * INPUT:  
 *          ctrl_entry_p - the control entry information to be validate
 *
 * OUTPUT:  
 *          None 
 * RETURN:  
 *          TRUE \ FALSE
 * NOTES:  
 *          None
 */    
static BOOL_T TRACEROUTE_OM_SemanticCheck(void *ctrl_entry_p)
{
#if 0 // temp
    /* LOCAL VARIABLE DECLARATION
     */
    BOOL_T is_valid_addr = FALSE;
    TRACEROUTE_TYPE_TraceRouteCtlEntry_T        *local_ctrl_entry_p;
    /*  BODY
     */     
     
    if (ctrl_entry_p == NULL)
        return FALSE;

    local_ctrl_entry_p = (TRACEROUTE_TYPE_TraceRouteCtlEntry_T *) ctrl_entry_p;
          
    //if (local_ctrl_entry->trace_route_ctl_target_address == 0)
    switch(local_ctrl_entry_p->trace_route_ctl_target_address_type)
    {
        case L_INET_ADDR_TYPE_IPV4:
        case L_INET_ADDR_TYPE_IPV4Z:
            is_valid_addr = !memcmp(local_ctrl_entry_p->trace_route_ctl_target_address.addr, traceroute_ipv4_zero_addr, SYS_ADPT_IPV4_ADDR_LEN);
            break;  
        case L_INET_ADDR_TYPE_IPV6:
        case L_INET_ADDR_TYPE_IPV6Z:
            is_valid_addr = !memcmp(local_ctrl_entry_p->trace_route_ctl_target_address.addr, traceroute_ipv6_zero_addr, SYS_ADPT_IPV6_ADDR_LEN);
            break;
        default:
            break;
    }
        if(!is_valid_addr)
            return FALSE;
#endif            
    return TRUE;
} /* end of TRACEROUTE_OM_SemanticCheck() */

/* FUNCTION NAME: TRACEROUTE_OM_ClearProbHistoryEntry
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
static void TRACEROUTE_OM_ClearProbHistoryEntry(UI32_T table_index)
{
    /* LOCAL VARIABLE DECLARATION
     */ 
    UI32_T      start_index;
    
    /*  BODY
     */          
	TRACEROUTE_OM_ENTER_CRITICAL_SECTION();  
    if (table_index > SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE)
    {
    	TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
        return;
    } /* end of if */
    
    start_index = prob_history_index_table[table_index].prob_history_head;
    
    memset(&trace_route_prob_history_table[start_index], 0, 
           sizeof(TRACEROUTE_TYPE_TraceRouteProbeHistoryEntry_T) * SYS_ADPT_TRACEROUTE_MAX_NBR_OF_PROB_HISTORY_ENTRY);        

    prob_history_index_table[table_index].current_prob_history = start_index;
    
    TRACEROUTE_OM_LEAVE_CRITICAL_SECTION();
    return;    

} /* end of TRACEROUTE_OM_ClearProbHistoryEntry() */


/* FUNCTION NAME: TRACEROUTE_OM_AssociateResultTable
 * PURPOSE: 
 *          Sync entry from result table with the control entry.
 * INPUT:  
 *          table_index - table index to be synchronize.
 *          ctr_entry - ctrl entry information to be sync.
 * OUTPUT:  
 *          None 
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          Need to sync the following three read-only fields from control entry.
 *          1. ctrl_entry.trace_route_ctl_source_address_type
 *          2. ctrl_entry.trace_route_ctl_target_address    
 *          3. ctrl_entry.trace_route_ctl_admin_status
 */
static void TRACEROUTE_OM_AssociateResultTable(UI32_T table_index, TRACEROUTE_TYPE_TraceRouteCtlEntry_T *ctrl_entry_p)
{
    /* BODY
     */          
    memcpy(trace_route_result_table[table_index].trace_route_ctl_owner_index,ctrl_entry_p->trace_route_ctl_owner_index,ctrl_entry_p->trace_route_ctl_owner_index_len);
    trace_route_result_table[table_index].trace_route_ctl_owner_index_len = ctrl_entry_p->trace_route_ctl_owner_index_len;
    memcpy(trace_route_result_table[table_index].trace_route_ctl_test_name,ctrl_entry_p->trace_route_ctl_test_name,ctrl_entry_p->trace_route_ctl_test_name_len);
    trace_route_result_table[table_index].trace_route_ctl_test_name_len = ctrl_entry_p->trace_route_ctl_test_name_len;
    trace_route_result_table[table_index].trace_route_results_oper_status = VAL_traceRouteResultsOperStatus_enabled;    
    trace_route_result_table[table_index].trace_route_results_cur_hop_count  = 1;
    trace_route_result_table[table_index].trace_route_results_cur_probe_count = 0;


/*    trace_route_result_table[table_index].trace_route_results_ip_tgt_addr_type = VAL_traceRouteCtlTargetAddressType_ipv4;
    memcpy(trace_route_result_table[table_index].trace_route_results_ip_tgt_addr,
           ctrl_entry_p->trace_route_ctl_target_address,
           sizeof(UI8_T) * SYS_ADPT_TRACEROUTE_MAX_IP_ADDRESS_STRING_SIZE);
    trace_route_result_table[table_index].trace_route_results_ip_tgt_addr_len = ctrl_entry_p->trace_route_ctl_target_address_len;
*/

    trace_route_result_table[table_index].trace_route_results_ip_tgt_addr_type = ctrl_entry_p->trace_route_ctl_target_address_type;
    memcpy(&trace_route_result_table[table_index].trace_route_results_ip_tgt_addr,
           &ctrl_entry_p->trace_route_ctl_target_address, sizeof(L_INET_AddrIp_T));
           
    return;
    
} /* end of TRACEROUTE_OM_AssociateResultTable() */

/* FUNCTION NAME: TRACEROUTE_OM_DisAssociateResultTable
 * PURPOSE: 
 *          Sync entry from result table with the control entry.
 * INPUT:  
 *          table_index - table index to be synchronize.
 *          ctr_entry - ctrl entry information to be sync.
 * OUTPUT:  
 *          None 
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 *          Need to sync the following three read-only fields from control entry.
 *          1. ctrl_entry.trace_route_ctl_source_address_type
 *          2. ctrl_entry.trace_route_ctl_target_address    
 *          3. ctrl_entry.trace_route_ctl_admin_status
 */
static void TRACEROUTE_OM_DisAssociateResultTable(UI32_T table_index)
{
    /* BODY
     */          
    trace_route_result_table[table_index].trace_route_results_oper_status = VAL_traceRouteResultsOperStatus_disabled;    

    return;
    
} /* end of TRACEROUTE_OM_DisAssociateResultTable() */



/* FUNCTION NAME: TRACEROUTE_OM_CreateNewCtrlEntry
 * PURPOSE: 
 *          Create a new element in to the sorted list and create a new control entry in the talbe. 
 * INPUT:  
 *          ctrl_entry - control entry info to be stored
 * OUTPUT:  
 *          None 
 * RETURN:  
 *          TRACEROUTE_TYPE_OK \ 
 *          TRACEROUTE_TYPE_NO_MORE_ENTRY \ 
 *          TRACEROUTE_TYPE_FAIL
 * NOTES:  
 */
static UI32_T TRACEROUTE_OM_CreateNewCtrlEntry(TRACEROUTE_TYPE_TraceRouteCtlEntry_T  ctrl_entry)
{
    TRACEROUTE_SORTLST_ELM_T    new_elm;
    BOOL_T      next_index_found = FALSE;
    /* BODY
     */          
    if ((ctrl_entry.trace_route_ctl_owner_index == NULL) || (ctrl_entry.trace_route_ctl_test_name == NULL) ||
        (ctrl_entry.trace_route_ctl_owner_index_len <= 0) || (ctrl_entry.trace_route_ctl_owner_index_len > SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE) ||
        (ctrl_entry.trace_route_ctl_test_name_len <= 0) || (ctrl_entry.trace_route_ctl_test_name_len > SYS_ADPT_TRACEROUTE_MAX_NAME_SIZE))
        return TRACEROUTE_TYPE_INVALID_ARG;
        
    /*  reach max control entry limit.
     */
    if (current_control_entry >= SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE)
        return TRACEROUTE_TYPE_FAIL;
        
    memset(&new_elm, 0, sizeof(TRACEROUTE_SORTLST_ELM_T));
    
    memcpy(new_elm.owner_index, ctrl_entry.trace_route_ctl_owner_index, ctrl_entry.trace_route_ctl_owner_index_len);
    memcpy(new_elm.test_name, ctrl_entry.trace_route_ctl_test_name, ctrl_entry.trace_route_ctl_test_name_len);
    new_elm.owner_index_len = ctrl_entry.trace_route_ctl_owner_index_len;
    new_elm.test_name_len = ctrl_entry.trace_route_ctl_test_name_len;
    new_elm.table_index = next_available_index;
    
    if (L_SORT_LST_Set(&sort_trace_route_list, &new_elm) != TRUE)
        return TRACEROUTE_TYPE_FAIL;
    
        memcpy(&trace_route_control_table[new_elm.table_index], &ctrl_entry, 
               sizeof(TRACEROUTE_TYPE_TraceRouteCtlEntry_T));           
    
    ++current_control_entry;
             
    while(!next_index_found)
    {
        if (++next_available_index >= SYS_ADPT_TRACEROUTE_MAX_NBR_OF_TRACE_ROUTE)
            next_available_index = 0;
            
        if (next_available_index != new_elm.table_index)
        {
            if (trace_route_control_table[next_available_index].trace_route_ctl_owner_index[0] == 0)
            {
                next_index_found = TRUE;
            }                        
        }
        else
        {
            next_available_index = TRACEROUTE_TYPE_INVALID_WORK_SPACE;
            /* can not find the nex available index, control table is full */
            return TRACEROUTE_TYPE_OK;
        }
    } /* end of while */
 
    return TRACEROUTE_TYPE_OK;
    
} /* end of TRACEROUTE_OM_DisAssociateResultTable() */

/* FUNCTION NAME : TRACEROUTE_OM_HandleIPCReqMsg
 * PURPOSE:
 *    Handle the ipc request message for TRACEROUTE om.
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
BOOL_T TRACEROUTE_OM_HandleIPCReqMsg(SYSFUN_Msg_T* ipcmsg_p)
{
    TRACEROUTE_OM_IPCMsg_T *traceroute_om_msg_p;
    BOOL_T           need_respond;

    if(ipcmsg_p==NULL)
        return FALSE;

    traceroute_om_msg_p= (TRACEROUTE_OM_IPCMsg_T*)ipcmsg_p->msg_buf;

    switch(traceroute_om_msg_p->type.cmd)
    {
        case TRACEROUTE_OM_IPCCMD_GETTRACEROUTECTLENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetTraceRouteCtlEntry(&traceroute_om_msg_p->data.ctrl_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(ctrl_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTECTLENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetNextTraceRouteCtlEntry(&traceroute_om_msg_p->data.ctrl_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(ctrl_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETTRACEROUTERESULTSENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetTraceRouteResultsEntry(&traceroute_om_msg_p->data.results_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(results_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTERESULTSENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetNextResultsEntry(&traceroute_om_msg_p->data.results_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(results_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETTRACEROUTEPROBEHISTORYENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetTraceRouteProbeHistoryEntry(
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_hop_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_probe_index,
                &traceroute_om_msg_p->data.probe_history_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRY:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntry(
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_hop_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_probe_index,
                &traceroute_om_msg_p->data.probe_history_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
            need_respond = TRUE;
            break;

        case TRACEROUTE_OM_IPCCMD_GETNEXTTRACEROUTEPROBEHISTORYENTRYFORCLIWEB:
            traceroute_om_msg_p->type.result_ui32 = TRACEROUTE_OM_GetNextTraceRouteProbeHistoryEntryForCLIWEB(
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_owner_index_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_ctl_test_name_len,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_hop_index,
                traceroute_om_msg_p->data.probe_history_entry.trace_route_probe_history_probe_index,
                &traceroute_om_msg_p->data.probe_history_entry);
            ipcmsg_p->msg_size = TRACEROUTE_OM_GET_MSG_SIZE(probe_history_entry);
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
