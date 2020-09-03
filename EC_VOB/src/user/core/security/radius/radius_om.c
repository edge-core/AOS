#include "radius_om.h"
#include "stdio.h"
#include "ip_lib.h"
#include "sys_cpnt.h"
#if (SYS_CPNT_EH == TRUE)
#include "sys_module.h"
#include "syslog_type.h"
#include "eh_type.h"
#include "eh_mgr.h"
#endif
#include "mib2_pom.h"
#include "l_stdlib.h"

#include "l_link_lst.h"

#ifndef ASSERT
#define ASSERT(eq)
#endif /* ASSERT */

#ifndef _countof
#define _countof(_Ary)  (sizeof(_Ary) / sizeof(*_Ary))
#endif /* _countof */


#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
#define RADACC_MAX_NBR_OF_REQUEST       FD_SETSIZE

/* session id format
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   boot part   |              user part                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

   RFC2866, example
   the first two digits increment on each reboot
   the next 6 digits counting from 0 for the first person logging in after a reboot up
 */
#define RADACC_MIN_SESSION_ID_FOR_BOOT  0x00000000  /* lower bound */
#define RADACC_MAX_SESSION_ID_FOR_BOOT  0xFF000000  /* upper bound */
#define RADACC_INC_SESSION_ID_FOR_BOOT  0x01000000  /* increment */

#define RADACC_MIN_SESSION_ID_FOR_USER  0x00000000  /* lower bound */
#define RADACC_MAX_SESSION_ID_FOR_USER  0x00FFFFFF  /* upper bound */
#define RADACC_INC_SESSION_ID_FOR_USER  0x00000001  /* increment */

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*#define RADIUS_OM_DEBUG_MODE*/ /* define RADIUS_OM_DEBUG_MODE
                                to build RADIUS_OM with DEBUG version
                                And let following macros print debug messages
                                */


/* MACRO FUNCTION DECLARATIONS
 */
#ifdef RADIUS_OM_DEBUG_MODE

    #define RADIUS_OM_TRACE(msg)                            {printf(msg);printf("\r\n");}
    #define RADIUS_OM_TRACE1(msg, arg)                      {printf(msg, arg);printf("\r\n");}
    #define RADIUS_OM_TRACE2(msg, arg1, arg2)               {printf(msg, arg1, arg2);printf("\r\n");}
    #define RADIUS_OM_TRACE3(msg, arg1, arg2, arg3)         {printf(msg, arg1, arg2, arg3);printf("\r\n");}
    #define RADIUS_OM_TRACE4(msg, arg1, arg2, arg3, arg4)   {printf(msg, arg1, arg2, arg3, arg4);printf("\r\n");}

#else

    #define RADIUS_OM_TRACE(msg)                            ((void)0)
    #define RADIUS_OM_TRACE1(msg, arg)                      ((void)0)
    #define RADIUS_OM_TRACE2(msg, arg1, arg2)               ((void)0)
    #define RADIUS_OM_TRACE3(msg, arg1, arg2, arg3)         ((void)0)
    #define RADIUS_OM_TRACE4(msg, arg1, arg2, arg3, arg4)   ((void)0)

#endif /* RADIUS_OM_DEBUG_MODE */

#define RADIUS_OM_REQUEST_INDEX(request_p)      ( \
    (UI32_T)((((char *)request_p - (char *)&radius_om_request_list_elements[0]) / sizeof(radius_om_request_list_elements[0])) + 1)) /* 1-based */

#define RADIUS_OM_INIT_TIMER_LINKED_LIST(list, elements, data, max_element_count_arg, each_data_size_arg, type) \
    do \
    { \
        int loop_i; \
        \
        memset(&list, 0, sizeof(list)); \
        memset(elements, 0, sizeof(elements)); \
        memset(data, 0, sizeof(data)); \
        \
        list.elements_p = elements; \
        list.data_p = data; \
        list.max_element_count = max_element_count_arg; \
        list.each_data_size = each_data_size_arg; \
        list.head_p = NULL; \
        list.tail_p = NULL; \
        list.free_p = &elements[0]; \
        list.free_p->prev_entry_p = NULL; \
        list.free_p->next_entry_p = &elements[1]; \
        \
        for (loop_i = 1; loop_i < max_element_count_arg - 1; loop_i++) \
        { \
            elements[loop_i].prev_entry_p = &elements[loop_i - 1]; \
            elements[loop_i].next_entry_p = &elements[loop_i + 1]; \
        } \
        \
        elements[max_element_count_arg - 1].prev_entry_p = &elements[max_element_count_arg - 2]; \
        elements[max_element_count_arg - 1].next_entry_p = NULL; \
        \
        for (i = 0; i < max_element_count_arg; i++) \
        { \
            elements[i].data_p = (type *)&data[i]; \
        } \
    } while(0)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    BOOL_T              is_init;
    L_LINK_LST_List_T   list;

    UI32_T              max_auth_req_count;
    UI32_T              nbr_of_auth_req;
    UI32_T              max_acct_req_count;
    UI32_T              nbr_of_acct_req;
} RADIUS_OM_ReqWaitingQueue_T;

typedef struct RADIUS_OM_TimerLinkedListElement_S
{
    UI32_T                                      time;
    void                                        *data_p;
    struct RADIUS_OM_TimerLinkedListElement_S   *prev_entry_p;
    struct RADIUS_OM_TimerLinkedListElement_S   *next_entry_p;
} RADIUS_OM_TimerLinkedListElement_T;

typedef struct RADIUS_OM_TimerLinkedList_S
{
    RADIUS_OM_TimerLinkedListElement_T  *head_p; /* sorted by remaining time in ascending order */
    RADIUS_OM_TimerLinkedListElement_T  *tail_p; /* tail of the request queue */
    RADIUS_OM_TimerLinkedListElement_T  *free_p;

    RADIUS_OM_TimerLinkedListElement_T  *elements_p;
    void                                *data_p;
    UI32_T                              each_data_size;

    UI32_T                              max_element_count;
} RADIUS_OM_TimerLinkedList_T;

typedef struct RADIUS_OM_IdQueueEntry_S
{
    UI8_T                           id;
    UI32_T                          request_index;

    BOOL_T                          is_allocated;

    struct RADIUS_OM_IdQueueEntry_S *next_entry_p;
} RADIUS_OM_IdQueueEntry_T;

typedef struct RADIUS_OM_IdQueue_S
{
    int                         socket_id;

    UI32_T                      num_of_allocated;

    RADIUS_OM_IdQueueEntry_T    *free_head_p;
    RADIUS_OM_IdQueueEntry_T    *free_tail_p;
    RADIUS_OM_IdQueueEntry_T    entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER];
} RADIUS_OM_IdQueue_T;

static BOOL_T RADIUS_OM_InitIdQueue(RADIUS_OM_IdQueue_T *queue_p);

static RADIUS_Server_Host_T *RADIUS_OM_GetServerHostByIpAddress(UI32_T ip_address);

static void RADIUS_OM_CopyServerHost(RADIUS_Server_Host_T *det, RADIUS_Server_Host_T *src);

static BOOL_T RADIUS_OM_LLst_Compare_Auth_Request(void *in_list_element_p,
    void *input_element_p);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
static RADACC_UserInfo_T *RADIUS_OM_GetAccUser(UI16_T user_index);
static BOOL_T RADIUS_OM_GetNextAccUser(RADACC_UserInfo_T **user_entry_pp);
static RADACC_UserInfo_T *RADIUS_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type);
static UI32_T RADIUS_OM_GetNextSessionId();
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

static RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T request_index, UI32_T *id_p);
static RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T id);
static RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T id, UI32_T *request_index_p);
static RADIUS_ReturnValue_T RADIUS_OM_GetFreeTimerLinkedListElement(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T **freed_element_pp);
static RADIUS_ReturnValue_T RADIUS_OM_DeleteTimerLinkedListElement(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T *element_p);
static RADIUS_ReturnValue_T RADIUS_OM_ResortTimerLinkedListByTimeOrder(
    RADIUS_OM_TimerLinkedList_T *list_p, UI32_T index);
static RADIUS_ReturnValue_T RADIUS_OM_AddElementToTimerLinkedList(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T *new_element_p);
static RADIUS_ReturnValue_T RADIUS_OM_DeleteElementFromTimerLinkedList(
    RADIUS_OM_TimerLinkedList_T *list_p,
    UI32_T index);

/* STATIC VARIABLE DECLARATIONS
 */
static RADIUS_Server_Host_T RAIDUS_Server_Host[SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS];

static UI8_T   state_attr[256];
static UI32_T  g_identity;
static UI32_T  state_attr_len;
static R_EAP_DATA      eap_message;
static R_STATE_DATA    state_message;
/********************For MIB*******************/
static UI32_T   radiusAuthClientInvalidServerAddresses ;
static UI32_T   RoundTripTimeCounter ;
static UI32_T   AccessRequestsCounter ;
static UI32_T   AccessAcceptsCounter ;
static UI32_T   AccessRejectsCounter ;
static UI32_T   AccessChallengesCounter ;
static UI32_T   MalformedAccessResponsesCounter ;
static UI32_T   BadAuthenticatorsCounter ;
static UI32_T   PendingRequestsCounter ;
static UI32_T   UnknownTypesCounter ;
static UI32_T   PacketsDroppedCounter;
/********************************************/
static I32_T   filter_id_privilege;
/* MACRO FUNCTIONS DECLARACTION
 */
static UI32_T  radius_om_sem_id;   /* semaphore id */
#define RADIUS_OM_LOCK()       SYSFUN_TakeSem(radius_om_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER)
#define RADIUS_OM_UNLOCK()     SYSFUN_GiveSem(radius_om_sem_id)
static UI32_T radius_om_task_id;
static UI32_T radius_om_timeout;
static UI32_T radius_om_retries;
static UI32_T radius_om_server_ip;
static UI32_T radius_om_server_port;
static UI8_T  radius_om_secret_key[MAXSIZE_radiusServerGlobalKey + 1];
static UI32_T radius_om_debug_flag = SECURITY_DEBUG_TYPE_OPTION_FUNC_LINE;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

static BOOL_T   radacc_om_intialized = FALSE;

static UI32_T   radius_om_acct_port;

static UI32_T   acc_invalid_server_address_counter;

/* follow RFC2866 5.5 Acct-Session-Id Example */
static UI32_T   acc_session_id = RADACC_MIN_SESSION_ID_FOR_BOOT + RADACC_MIN_SESSION_ID_FOR_USER;

static BOOL_T   acc_some_packet_need_to_hook; /* need to hook request flag */

static RADIUS_OM_TimerLinkedList_T          radius_om_acc_user_list;
static RADIUS_OM_TimerLinkedListElement_T   radius_om_acc_user_list_elements[RADIUS_OM_MAX_NBR_OF_REQUEST];
static RADACC_UserInfo_T                    radius_om_acc_user_list_data[RADIUS_OM_MAX_NBR_OF_REQUEST];
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

static RADIUS_OM_TimerLinkedList_T          radius_om_request_list;
static RADIUS_OM_TimerLinkedListElement_T   radius_om_request_list_elements[RADIUS_OM_MAX_NBR_OF_REQUEST];
static RADIUS_OM_RadiusRequest_T            radius_om_request_list_data[RADIUS_OM_MAX_NBR_OF_REQUEST];
static RADIUS_OM_ReqWaitingQueue_T          radius_om_req_waiting_queue;
static RADIUS_OM_IdQueue_T                  radius_om_auth_req_id_queue;
static RADIUS_OM_IdQueue_T                  radius_om_acct_req_id_queue;

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : get the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetDebugFlag()
{
    return radius_om_debug_flag;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetDebugFlag
 *---------------------------------------------------------------------------
 * PURPOSE  : set the debug flag
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SetDebugFlag(UI32_T flag)
{
    RADIUS_OM_LOCK();
    radius_om_debug_flag = flag;
    RADIUS_OM_UNLOCK();
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetDebugPrompt
 *---------------------------------------------------------------------------
 * PURPOSE  : get the debug prompt
 * INPUT    : flag -- debug flag
 * OUTPUT   : none
 * RETURN   : string form of debug flag
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
const char* RADIUS_OM_GetDebugPrompt(UI32_T flag)
{
    if (flag & RADIUS_DEBUG_TASK)
        return "RADIUS/TASK";
    if (flag & RADIUS_DEBUG_MGR)
        return "RADIUS/MGR";
    if (flag & RADIUS_DEBUG_OM)
        return "RADIUS/OM";
    if (flag & RADIUS_DEBUG_RADEXAMPLE)
        return "RADIUS/RADEXAMPLE";
    if (flag & RADIUS_DEBUG_BUILDREQ)
        return "RADIUS/BUILDREQ";
    if (flag & RADIUS_DEBUG_SENDSERVER)
        return "RADIUS/SENDSERVER";
    if (flag & RADIUS_DEBUG_AVPAIR)
        return "RADIUS/AVPAIR";
    return "RADIUS";
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_OM_CreatSem
 *---------------------------------------------------------------------------
 * PURPOSE  :   Initiate the semaphore for RADIUS objects
 * INPUT    :   None
 * OUTPUT   :   None
 * RETURN   :   None
 * NOTE     :   None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_CreatSem(void)
{
    /* create semaphore */
    if (SYSFUN_CreateSem(SYSFUN_SEMKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &radius_om_sem_id) != SYSFUN_OK)
    {
        return FALSE;
    }
    return TRUE;
} /* End of RADIUS_OM_CreatSem */

void RADIUS_OM_EnterSem(void)
{
    RADIUS_OM_LOCK();
    return;
}

void RADIUS_OM_LeaveSem(void)
{
    RADIUS_OM_UNLOCK();
    return;
}

UI32_T RADIUS_OM_Get_TaskId()
{
    return radius_om_task_id;
}

void RADIUS_OM_Set_TaskId(UI32_T task_id)
{
    radius_om_task_id = task_id;
    return;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_InitIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Initialize ID queue.
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_OM_InitIdQueue(RADIUS_OM_IdQueue_T *queue_p)
{
    UI32_T i;

    if (NULL == queue_p)
    {
        return FALSE;
    }

    queue_p->socket_id = -1;
    queue_p->num_of_allocated = 0;
    queue_p->free_head_p = &queue_p->entries[0];
    queue_p->free_tail_p = &queue_p->entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1];

    queue_p->entries[0].id = 0;
    queue_p->entries[0].is_allocated = FALSE;
    queue_p->entries[0].request_index = 0;
    queue_p->entries[0].next_entry_p = &queue_p->entries[1];

    for (i = 1; i < RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1; i++)
    {
        queue_p->entries[i].id = i;
        queue_p->entries[i].is_allocated = FALSE;
        queue_p->entries[i].request_index = 0;
        queue_p->entries[i].next_entry_p = &queue_p->entries[i + 1];
    }

    queue_p->entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1].id = RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1;
    queue_p->entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1].is_allocated = FALSE;
    queue_p->entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1].request_index = 0;
    queue_p->entries[RADIUS_OM_MAX_NBR_OF_IDENTIFIER - 1].next_entry_p = NULL;

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_SetConfigSettingToDefault
 *---------------------------------------------------------------------------
 * PURPOSE: This function will set default value of RADIUS configuration
 * INPUT:  None.
 * OUTPUT: None.
 * RETURN: None.
 * NOTES:  None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetConfigSettingToDefault()
{
     UI32_T i;
     BOOL_T  ret;

     RADIUS_OM_Set_Request_Timeout(SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS);
     RADIUS_OM_Set_Server_Port(SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER);
     RADIUS_OM_Set_Server_Secret((UI8_T *)RADIUS_TYPE_DEFAULT_SERVER_SECRET);
     RADIUS_OM_Set_Retransmit_Times(SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS);
     RADIUS_OM_Set_Server_IP(SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS);

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    radius_om_acct_port = SYS_DFLT_RADIUS_ACC_CLIENT_SERVER_PORT_NUMBER;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    RADIUS_OM_Set_State_Attr_Len(0);
    /* for MIB */
    radiusAuthClientInvalidServerAddresses    = 0;
    RADIUS_OM_MIB_Set_RoundTripTimeCounter(0);
    RADIUS_OM_MIB_Set_AccessRequestsCounter(0);
    RADIUS_OM_MIB_Set_AccessAcceptsCounter(0);
    RADIUS_OM_MIB_Set_AccessRejectsCounter(0);
    RADIUS_OM_MIB_Set_AccessChallengesCounter(0);
    RADIUS_OM_MIB_Set_MalformedAccessResponsesCounter(0);
    RADIUS_OM_MIB_Set_BadAuthenticatorsCounter(0);
    RADIUS_OM_MIB_Set_PendingRequestsCounter(0);
    RADIUS_OM_MIB_Set_UnknownTypesCounter(0);
    RADIUS_OM_MIB_Set_PacketsDroppedCounter(0);

    /* for multi-server */
    for(i=0;i<SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS;i++)
    {
        RAIDUS_Server_Host[i].used_flag   = FALSE;
        RAIDUS_Server_Host[i].server_ip   = SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS;
        RAIDUS_Server_Host[i].server_port = SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER;
        RAIDUS_Server_Host[i].timeout     = SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS;
        RAIDUS_Server_Host[i].retransmit  = SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS;
        memset(RAIDUS_Server_Host[i].secret,0,MAXSIZE_radiusServerGlobalKey);
        RAIDUS_Server_Host[i].server_index = 0;
        memset(&(RAIDUS_Server_Host[i].server_table),0,sizeof(AuthServerEntry));
    }

    if (TRUE == radius_om_req_waiting_queue.is_init)
    {
        L_LINK_LST_Delete_All(&radius_om_req_waiting_queue.list);
        radius_om_req_waiting_queue.nbr_of_auth_req = 0;
        radius_om_req_waiting_queue.nbr_of_acct_req = 0;
    }
    else
    {
        L_LINK_LST_Create(&radius_om_req_waiting_queue.list,
            RADIUS_OM_MAX_NBR_OF_WAIT_REQUEST,
            sizeof(RADIUS_OM_RequestEntry_T),
            RADIUS_OM_LLst_Compare_Auth_Request, TRUE);
        radius_om_req_waiting_queue.is_init = TRUE;

        radius_om_req_waiting_queue.max_auth_req_count = RADIUS_OM_MAX_NBR_OF_WAIT_AUTH_REQUEST;
        radius_om_req_waiting_queue.nbr_of_auth_req = 0;
        radius_om_req_waiting_queue.max_acct_req_count = RADIUS_OM_MAX_NBR_OF_WAIT_ACCT_REQUEST;
        radius_om_req_waiting_queue.nbr_of_acct_req = 0;
    }

    ret = TRUE;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    ret &= RADIUS_OM_AccInitialize();
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    /* Request queue in timer linked-list
     */
    RADIUS_OM_INIT_TIMER_LINKED_LIST(radius_om_request_list,
        radius_om_request_list_elements, radius_om_request_list_data,
        RADIUS_OM_MAX_NBR_OF_REQUEST, sizeof(radius_om_request_list_data[0]),
        RADIUS_OM_RadiusRequest_T);

    for (i = 0; i < RADIUS_OM_MAX_NBR_OF_REQUEST; i++)
    {
        radius_om_request_list_data[i].is_used = FALSE;
    }

    /* ID queues
     */
    if (   (FALSE == RADIUS_OM_InitIdQueue(&radius_om_auth_req_id_queue))
        || (FALSE == RADIUS_OM_InitIdQueue(&radius_om_acct_req_id_queue))
        )
    {
        return FALSE;
    }

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    RADIUS_OM_INIT_TIMER_LINKED_LIST(radius_om_acc_user_list,
        radius_om_acc_user_list_elements, radius_om_acc_user_list_data,
        SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS, sizeof(radius_om_acc_user_list_data[0]),
        RADACC_UserInfo_T);

    for (i = 0; i < SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS; i++)
    {
        radius_om_acc_user_list_data[i].user_index = i + 1;
        radius_om_acc_user_list_data[i].entry_status = RADACC_ENTRY_DESTROYED;
    }
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

    return ret;
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccInitialize
 *---------------------------------------------------------------------------
 * PURPOSE  : (re-)initialize accounting om
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : FALSE - failed
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_AccInitialize()
{
    UI16_T  index;
    RADACC_UserInfo_T       *user_info;
    RADACC_RequestInfo_T    *request_info;

    RADIUS_OM_AccFinalize();

    acc_invalid_server_address_counter = 0;

    acc_some_packet_need_to_hook = FALSE;

    RADIUS_OM_TRACE("\r\n[RADIUS_OM_AccInitialize] done");
    radacc_om_intialized = TRUE;

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccFinalize
 *---------------------------------------------------------------------------
 * PURPOSE  : clean accounting om resource
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_AccFinalize()
{
/*    RADACC_RequestInfo_T   *entry;*/

    if (TRUE != radacc_om_intialized)
    {
        /* om doesn't initialize yet */
        return;
    }

    radacc_om_intialized = FALSE;

#if 0 /* if re-stacking then re-enter transition mode
         and then all opened socks will be free automatically
       */
    for (entry = head_of_acc_request; NULL != entry; entry = entry->next_request)
    {
        if (0 == entry->send_data.sock_id)
            continue;

        s_close(entry->send_data.sock_id);
        entry->send_data.sock_id = 0; /* reset socket to know sock free */
    }
#endif

    RADIUS_OM_TRACE("\r\n[RADIUS_OM_AccFinalize] done");
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_AccIncSessionIdBootPart
 *---------------------------------------------------------------------------
 * PURPOSE  : increase boot part of session id
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_AccIncSessionIdBootPart()
{
    if (TRUE != radacc_om_intialized)
    {
        /* om doesn't initialize yet */
        return;
    }

    if ((acc_session_id & RADACC_MAX_SESSION_ID_FOR_BOOT) == RADACC_MAX_SESSION_ID_FOR_BOOT)
    {
        acc_session_id = RADACC_MIN_SESSION_ID_FOR_BOOT + RADACC_MIN_SESSION_ID_FOR_USER;
        RADIUS_OM_TRACE1("\r\n[RADIUS_OM_AccIncSessionIdBootPart] (boot part reset) session id change to %lu", acc_session_id);
    }
    else
    {
        acc_session_id = ((acc_session_id & RADACC_MAX_SESSION_ID_FOR_BOOT) + RADACC_INC_SESSION_ID_FOR_BOOT) + /* boot part */
            RADACC_MIN_SESSION_ID_FOR_USER;
        RADIUS_OM_TRACE1("\r\n[RADIUS_OM_AccIncSessionIdBootPart] (increment on each reboot) session id change to %lu", acc_session_id);
    }
}

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS request timeout is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: timeout
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningRequestTimeout(UI32_T *timeout)
{
    SYS_TYPE_Get_Running_Cfg_T result;

    /*RADIUS_OM_LOCK();*/
    *timeout=RADIUS_OM_Get_Request_Timeout();
    /*RADIUS_OM_UNLOCK();*/

    if (*timeout != SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS )
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   timeout value.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Request_Timeout(void)
{
    UI32_T timeout;

    RADIUS_OM_LOCK();
    timeout = radius_om_timeout;
    RADIUS_OM_UNLOCK();
    return timeout;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Request_Timeout
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           waits for a reply from RADIUS server
 * INPUT:    timeout value.
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Request_Timeout(UI32_T timeval)
{
    if (timeval < SYS_ADPT_RADIUS_MIN_TIMEOUT || timeval > SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_RADIUS, RADIUS_OM_Set_Request_Timeout_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Request timeout"/*arg_p*/);/*Mercury_V2-00030*/
#endif
        return FALSE;
    }

    RADIUS_OM_LOCK();
    radius_om_timeout = timeval;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningServerPort(UI32_T *serverport)
{
    SYS_TYPE_Get_Running_Cfg_T result;

    /*RADIUS_OM_LOCK();*/
    *serverport = RADIUS_OM_Get_Server_Port();
    /*RADIUS_OM_UNLOCK();*/

    if (*serverport != SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the UDP port number of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   Prot number
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Server_Port(void)
{
    UI32_T serverport;

    RADIUS_OM_LOCK();
    serverport = radius_om_server_port;
    RADIUS_OM_UNLOCK();
    return serverport;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_Port
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the UDP port number of the remote RADIUS server
 * INPUT:    Prot number
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Port(UI32_T serverport)
{
    /*UI8_T *arg_p = "Server port";*/

    if (serverport < MIN_radiusServerGlobalAuthPort|| serverport > MAX_radiusServerGlobalAuthPort)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_RADIUS, RADIUS_OM_Set_Server_Port_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Server port"/*arg_p*/);/*Mercury_V2-00030*/
#endif
        return FALSE;
    }

    RADIUS_OM_LOCK();
    radius_om_server_port = serverport;
    RADIUS_OM_UNLOCK();
    return TRUE;
}



#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server port is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverport
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetRunningServerAcctPort(UI32_T *acct_port)
{
    UI32_T result;

    /*RADIUS_OM_LOCK();*/
    *acct_port = RADIUS_OM_GetServerAcctPort();
    /*RADIUS_OM_UNLOCK();*/

    if (*acct_port != SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get global server accounting port
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : port number
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_GetServerAcctPort()
{
    UI32_T  ret;
    RADIUS_OM_LOCK();
    ret = radius_om_acct_port;
    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetServerAcctPort
 *---------------------------------------------------------------------------
 * PURPOSE  : set global server accounting port
 * INPUT    : acct_port
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetServerAcctPort(UI32_T acct_port)
{
    if ((MIN_radiusServerGlobalAcctPort > acct_port) || (MAX_radiusServerGlobalAcctPort < acct_port))
    {
        RADIUS_OM_TRACE3("\r\n[RADIUS_OM_SetServerAcctPort] invalid acct_port(%lu) {%lu - %lu}",
        acct_port, RADIUS_MIN_SERVER_PORT, RADIUS_MAX_SERVER_PORT);

        return FALSE;
    }

    RADIUS_OM_LOCK();
    radius_om_acct_port = acct_port;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */



/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerSecret
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server secret is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serversecret[MAX_SECRET_LENGTH + 1]
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T  RADIUS_OM_GetRunningServerSecret(UI8_T *serversecret)
{
   SYS_TYPE_Get_Running_Cfg_T result;

    RADIUS_OM_LOCK();
    memcpy(serversecret,radius_om_secret_key,MAXSIZE_radiusServerGlobalKey);
    RADIUS_OM_UNLOCK();

    if ( strcmp((char *)serversecret,RADIUS_TYPE_DEFAULT_SERVER_SECRET) !=0)
       result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
       result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   secret_key
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI8_T * RADIUS_OM_Get_Server_Secret(void)
{
    return radius_om_secret_key;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_Secret
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the shared secret text string used between
 *           the RADIUS client and server.
 * INPUT:    secret text string
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Secret(UI8_T *serversecret)
{
 /*UI8_T *arg_p = "Server secret";*/

    if (strlen((char *)serversecret) > MAXSIZE_radiusServerGlobalKey) /*length can't greater than 48 */ /* validation check */
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_RADIUS, RADIUS_OM_Set_Server_Secret_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Server secret"/*arg_p*/);/*Mercury_V2-00030*/
#endif
        return FALSE;
    }

 RADIUS_OM_LOCK();
 strncpy((char *)radius_om_secret_key,(char *)serversecret, sizeof(radius_om_secret_key));
 RADIUS_OM_UNLOCK();
 return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningRetransmitTimes
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS retransmit times is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: retimes
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningRetransmitTimes(UI32_T *retimes)
{
  SYS_TYPE_Get_Running_Cfg_T result;

    /*RADIUS_OM_LOCK();*/
    *retimes = RADIUS_OM_Get_Retransmit_Times();
    /*RADIUS_OM_UNLOCK();*/

    if (*retimes != SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   retransmit times
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T RADIUS_OM_Get_Retransmit_Times(void)
{
    UI32_T retimes;

    RADIUS_OM_LOCK();
    retimes = radius_om_retries;
    RADIUS_OM_UNLOCK();
    return retimes;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Retransmit_Times
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the number of times the RADIUS client
 *           transmits each RADIUS request to the server before giving up
 * INPUT:    retransmit times
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Retransmit_Times(UI32_T retryval)
{
 /*UI8_T *arg_p = "Retransmit times";*/

    if (retryval < SYS_ADPT_RADIUS_MIN_RETRANSMIT || retryval > SYS_ADPT_RADIUS_MAX_RETRANSMIT)
    {
#if (SYS_CPNT_EH == TRUE)
        EH_MGR_Handle_Exception1(SYS_MODULE_RADIUS, RADIUS_OM_Set_Retransmit_Times_FUNC_NO, EH_TYPE_MSG_VALUE_OUT_OF_RANGE, SYSLOG_LEVEL_INFO ,"Retransmit times"/*arg_p*/);/*Mercury_V2-00030*/
#endif
        return FALSE;
    }
    RADIUS_OM_LOCK();
    radius_om_retries = retryval;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetRunningServerIP
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetRunningServerIP(UI32_T *serverip)
{
   SYS_TYPE_Get_Running_Cfg_T result;

    /*RADIUS_OM_LOCK();*/
    *serverip = RADIUS_OM_Get_Server_IP();
    /*RADIUS_OM_UNLOCK();*/

    if (*serverip != SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS)
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    return result;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Get_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the IP address of the remote RADIUS server
 * INPUT:    None.
 * OUTPUT:   None.
 * RETURN:   RADIUS server IP address
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_OM_Get_Server_IP(void)
{
  UI32_T serverip;

 RADIUS_OM_LOCK();
 serverip = radius_om_server_ip;
 RADIUS_OM_UNLOCK();
 return serverip;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_Set_Server_IP
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the IP address of the remote RADIUS server
 * INPUT:    RADIUS server IP address
 * OUTPUT:   None.
 * RETURN:   TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_IP(UI32_T serverip)
{
    RADIUS_OM_LOCK();
    radius_om_server_ip = serverip;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*  For MIB */
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Get_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   Number of RADIUS Access-Response packets received from unknown addresses
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
UI32_T  RADIUS_OM_SNMP_Get_UnknowAddress_Packets(void)
{
    return radiusAuthClientInvalidServerAddresses;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Set_UnknowAddress_Packets
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the number of RADIUS Access-Response packets
 *           received from unknown addresses.
 * INPUT:    counts
 * OUTPUT:   None.
 * RETURN:   None.
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SNMP_Set_UnknowAddress_Packets(UI32_T counts)
{
    radiusAuthClientInvalidServerAddresses = counts;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_Get_NAS_ID
 *---------------------------------------------------------------------------
 * PURPOSE:  Get he NAS-Identifier of the RADIUS authentication client.
 *           This is not necessarily the same as sysName in MIB II.
 * INPUT:    None
 * OUTPUT:   None.
 * RETURN:   NASID
 *           NASID = NULL  ---No NAS ID
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_Get_NAS_ID(UI8_T *nas_id)
{
  UI8_T sys_name[MAXSIZE_radiusAuthClientIdentifier+1];
  /* MAXSIZE_radiusAuthClientIdentifier =255 , define in leaf_2618.h"*/
  /*RADIUS_OM_LOCK();*/
  memset(sys_name,'\0',MAXSIZE_radiusAuthClientIdentifier+1);
  if(MIB2_POM_GetSysName(sys_name) == TRUE)
    {
    memset(nas_id, 0, MAXSIZE_radiusAuthClientIdentifier+1);
    memcpy(nas_id, sys_name, MAXSIZE_radiusAuthClientIdentifier);
    }
  else
    {
        memset(sys_name,'\0',MAXSIZE_radiusAuthClientIdentifier);
        memcpy(nas_id, sys_name, MAXSIZE_radiusAuthClientIdentifier);
    }
  /*RADIUS_OM_UNLOCK();*/
  return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_GetAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will call RADIUS client MIB to get radiusAuthServerTable
 * INPUT:    adiusAuthServerIndex
 * OUTPUT:   AuthServerEntry pointer
 * RETURN:   Get table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_GetAuthServerTable(UI32_T index,AuthServerEntry *ServerEntry)
{
    RADIUS_Server_Host_T server_host;

    if(RADIUS_OM_Get_Server_Host(index,&server_host) == FALSE)
        return FALSE;

    ServerEntry->radiusAuthServerIndex                    = server_host.server_index;
    ServerEntry->radiusAuthServerAddress                  = server_host.server_ip;
    ServerEntry->radiusAuthClientServerPortNumber         = server_host.server_port;
    ServerEntry->radiusAuthClientRoundTripTime            = server_host.server_table.radiusAuthClientRoundTripTime;
    ServerEntry->radiusAuthClientAccessRequests           = server_host.server_table.radiusAuthClientAccessRequests;
    ServerEntry->radiusAuthClientAccessRetransmissions    = server_host.server_table.radiusAuthClientAccessRetransmissions;
    ServerEntry->radiusAuthClientAccessAccepts            = server_host.server_table.radiusAuthClientAccessAccepts;
    ServerEntry->radiusAuthClientAccessRejects            = server_host.server_table.radiusAuthClientAccessRejects;
    ServerEntry->radiusAuthClientAccessChallenges         = server_host.server_table.radiusAuthClientAccessChallenges;
    ServerEntry->radiusAuthClientMalformedAccessResponses = server_host.server_table.radiusAuthClientMalformedAccessResponses;
    ServerEntry->radiusAuthClientBadAuthenticators        = server_host.server_table.radiusAuthClientBadAuthenticators;
    ServerEntry->radiusAuthClientPendingRequests          = server_host.server_table.radiusAuthClientPendingRequests;
    ServerEntry->radiusAuthClientTimeouts                 = server_host.server_table.radiusAuthClientTimeouts;
    ServerEntry->radiusAuthClientUnknownTypes             = server_host.server_table.radiusAuthClientUnknownTypes;
    ServerEntry->radiusAuthClientPacketsDropped           = server_host.server_table.radiusAuthClientPacketsDropped;

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SNMP_GetNextAuthServerTable
 *---------------------------------------------------------------------------
 * PURPOSE:  This funtion returns true if the next available table entry info
 *           can be successfully retrieved. Otherwise, false is returned.
 * INPUT:    RadiusAuthServerIndex ,
 * OUTPUT:   NextAuthServerEntry pointer
 * RETURN:   Get next table TRUE/FALSE
 * NOTE:     None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SNMP_GetNextAuthServerTable(UI32_T *index,AuthServerEntry *ServerEntry)
{
    RADIUS_Server_Host_T server_host;

    //if(*index >= SYS_ADPT_MAX_NBR_OF_RADIUS_SERVER)
    //  return FALSE;
    //else
    //{
        //*Index=*Index+1;
        //return RADIUS_OM_SNMP_GetAuthServerTable(*Index,ServerEntry);
        if(RADIUS_OM_GetNext_Server_Host(index,&server_host) == FALSE)
            return FALSE;
        ServerEntry->radiusAuthServerIndex                    = server_host.server_index;
        ServerEntry->radiusAuthServerAddress                  = server_host.server_ip;
        ServerEntry->radiusAuthClientServerPortNumber         = server_host.server_port;
        ServerEntry->radiusAuthClientRoundTripTime            = server_host.server_table.radiusAuthClientRoundTripTime;
        ServerEntry->radiusAuthClientAccessRequests           = server_host.server_table.radiusAuthClientAccessRequests;
        ServerEntry->radiusAuthClientAccessRetransmissions    = server_host.server_table.radiusAuthClientAccessRetransmissions;
        ServerEntry->radiusAuthClientAccessAccepts            = server_host.server_table.radiusAuthClientAccessAccepts;
        ServerEntry->radiusAuthClientAccessRejects            = server_host.server_table.radiusAuthClientAccessRejects;
        ServerEntry->radiusAuthClientAccessChallenges         = server_host.server_table.radiusAuthClientAccessChallenges;
        ServerEntry->radiusAuthClientMalformedAccessResponses = server_host.server_table.radiusAuthClientMalformedAccessResponses;
        ServerEntry->radiusAuthClientBadAuthenticators        = server_host.server_table.radiusAuthClientBadAuthenticators;
        ServerEntry->radiusAuthClientPendingRequests          = server_host.server_table.radiusAuthClientPendingRequests;
        ServerEntry->radiusAuthClientTimeouts                 = server_host.server_table.radiusAuthClientTimeouts;
        ServerEntry->radiusAuthClientUnknownTypes             = server_host.server_table.radiusAuthClientUnknownTypes;
        ServerEntry->radiusAuthClientPacketsDropped           = server_host.server_table.radiusAuthClientPacketsDropped;
    //}
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerPendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the pending request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerPendingRequestCounter(UI32_T server_index)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientPendingRequests++;

    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_DecreaseServerPendingRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Decrease the pending request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_DecreaseServerPendingRequestCounter(UI32_T server_index)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientPendingRequests--;

    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerTimeoutRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the timeout request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerTimeoutCounter(UI32_T server_index)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientTimeouts++;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerAccessRetransmissionsCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the access retransmission counter of specified RADIUS
 *            server in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerAccessRetransmissionsCounter(
    UI32_T server_index)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientAccessRetransmissions++;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_IncreaseServerAccessRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the access request counter of specified RADIUS server
 *            in MIB.
 * INPUT    : server_index  - RADIUS server index
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncreaseServerAccessRequestCounter(UI32_T server_index)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientAccessRequests++;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_SetServerRoundTripTime
 *---------------------------------------------------------------------------
 * PURPOSE  : Increase the round trip time of specified RADIUS server in MIB.
 * INPUT    : server_index  - RADIUS server index
 *            time          - Round trip time
 * OUTPUT   : None
 * RETURN   : TRUE/FALSE
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetServerRoundTripTime(UI32_T server_index, UI32_T time)
{
    RADIUS_Server_Host_T * server_host_p;

    if (server_index >= SYS_ADPT_RADIUS_MAX_TIMEOUT)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();
    server_host_p = RADIUS_OM_Get_Server_Host_Entry(server_index + 1);
    if (NULL == server_host_p)
    {
        RADIUS_OM_UNLOCK();

        return FALSE;
    }

    server_host_p->server_table.radiusAuthClientRoundTripTime = time;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*UI32_T RADIUS_OM_Get_Global_Identity ()
{
    return g_identity;
}
*/

BOOL_T RADIUS_OM_Set_Global_Identity (UI32_T identity)
{
    g_identity = identity;
    return TRUE;
}

UI8_T * RADIUS_OM_Get_State_Attr()
{
    return state_attr;
}

BOOL_T RADIUS_OM_Set_State_Attr(UI8_T * attribute,UI32_T attribute_len)
{
    memcpy(state_attr, attribute,attribute_len);
    return TRUE;
}

UI32_T RADIUS_OM_Get_State_Attr_Len()
{
    return state_attr_len;
}

BOOL_T RADIUS_OM_Set_State_Attr_Len(UI32_T attribute_len)
{
    state_attr_len = attribute_len;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_RoundTripTimeCounter()
{
    return RoundTripTimeCounter;
}

BOOL_T RADIUS_OM_MIB_Set_RoundTripTimeCounter(UI32_T mib_counter)
{
    RoundTripTimeCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_AccessRequestsCounter()
{
    return AccessRequestsCounter;
}

BOOL_T RADIUS_OM_MIB_Set_AccessRequestsCounter(UI32_T mib_counter)
{
    AccessRequestsCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_AccessAcceptsCounter()
{
    return AccessAcceptsCounter;
}

BOOL_T RADIUS_OM_MIB_Set_AccessAcceptsCounter(UI32_T mib_counter)
{
    AccessAcceptsCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_AccessRejectsCounter()
{
    return AccessRejectsCounter;
}

BOOL_T RADIUS_OM_MIB_Set_AccessRejectsCounter(UI32_T mib_counter)
{
    AccessRejectsCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_AccessChallengesCounter()
{
    return AccessChallengesCounter;
}

BOOL_T RADIUS_OM_MIB_Set_AccessChallengesCounter(UI32_T mib_counter)
{
    AccessChallengesCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_MalformedAccessResponsesCounter()
{
    return MalformedAccessResponsesCounter;
}

BOOL_T RADIUS_OM_MIB_Set_MalformedAccessResponsesCounter(UI32_T mib_counter)
{
    MalformedAccessResponsesCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_BadAuthenticatorsCounter()
{
    return BadAuthenticatorsCounter;
}

BOOL_T RADIUS_OM_MIB_Set_BadAuthenticatorsCounter(UI32_T mib_counter)
{
    BadAuthenticatorsCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_PendingRequestsCounter()
{
    return PendingRequestsCounter;
}

BOOL_T RADIUS_OM_MIB_Set_PendingRequestsCounter(UI32_T mib_counter)
{
    PendingRequestsCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_UnknownTypesCounter()
{
    return UnknownTypesCounter;
}

BOOL_T RADIUS_OM_MIB_Set_UnknownTypesCounter(UI32_T mib_counter)
{
    UnknownTypesCounter = mib_counter;
    return TRUE;
}

UI32_T RADIUS_OM_MIB_Get_PacketsDroppedCounter()
{
    return PacketsDroppedCounter;
}

BOOL_T RADIUS_OM_MIB_Set_PacketsDroppedCounter(UI32_T mib_counter)
{
    PacketsDroppedCounter = mib_counter;
    return TRUE;
}

R_EAP_DATA* RADIUS_OM_Get_Eap_Message_Ptr()
{
    return &eap_message;
}

R_STATE_DATA* RADIUS_OM_Get_State_Message_Ptr()
{
    return &state_message;
}

BOOL_T RADIUS_OM_Set_Server_Host(UI32_T server_index,RADIUS_Server_Host_T *server_host)
{
    UI32_T  i = 0, network_order_default_ip;

    if(server_index <= 0 || server_index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
    {
        return FALSE;
    }

    /* ES3550MO-PoE-FLF-AA-00077
     * sys_dflt.h define IP address in host order
     * so need to convert it to network order
     */
    network_order_default_ip = L_STDLIB_Hton32(SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS);

    /*ES4649-ZZ-00508*/
    /*Don't check the server IP when the IP is default IP.*/
    if(server_host->server_ip != network_order_default_ip)
    {
        /*check if the server IP has exist */
        for(i=0;i < SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS;i++)
        {
            if((RAIDUS_Server_Host[i].used_flag == TRUE) && ((server_index-1) != i))
            {
                if(server_host->server_ip == RAIDUS_Server_Host[i].server_ip)
                return FALSE;
            }
        }
    }

    server_index --;/*database start at 0*/

    if(server_host->server_port < 0 || server_host->server_port > MAX_radiusMultipleServerPortNumber)
                return FALSE;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    if(server_host->acct_port < 0 || server_host->acct_port > MAX_radiusMultipleServerPortNumber)
        return FALSE;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    if(server_host->timeout < 0 || server_host->timeout > SYS_ADPT_RADIUS_MAX_TIMEOUT)
        return FALSE;
    if(server_host->retransmit < 0 || server_host->retransmit > SYS_ADPT_RADIUS_MAX_RETRANSMIT)
        return FALSE;
    if (strlen((char *)server_host->secret) < 0 || strlen((char *)server_host->secret) > MAXSIZE_radiusServerGlobalKey)
        return FALSE;

    RAIDUS_Server_Host[server_index].server_ip   = server_host->server_ip;

    if(server_host->server_port == 0)
        RAIDUS_Server_Host[server_index].server_port = RADIUS_OM_Get_Server_Port();
    else
        RAIDUS_Server_Host[server_index].server_port = server_host->server_port;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    if (server_host->acct_port == 0)
        RAIDUS_Server_Host[server_index].acct_port = RADIUS_OM_GetServerAcctPort();
    else
        RAIDUS_Server_Host[server_index].acct_port = server_host->acct_port;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    if(server_host->timeout == 0)
        RAIDUS_Server_Host[server_index].timeout = RADIUS_OM_Get_Request_Timeout();
    else
        RAIDUS_Server_Host[server_index].timeout = server_host->timeout;

    if(server_host->retransmit == 0)
        RAIDUS_Server_Host[server_index].retransmit  = RADIUS_OM_Get_Retransmit_Times();
    else
        RAIDUS_Server_Host[server_index].retransmit  = server_host->retransmit;
    memset(RAIDUS_Server_Host[server_index].secret, 0, MAXSIZE_radiusServerGlobalKey);
    if (strlen((char *)server_host->secret) == 0)/*(server_host->secret == NULL)*/
        memcpy(RAIDUS_Server_Host[server_index].secret,RADIUS_OM_Get_Server_Secret(),MAXSIZE_radiusServerGlobalKey);
    else
        memcpy(RAIDUS_Server_Host[server_index].secret,server_host->secret,MAXSIZE_radiusServerGlobalKey);

    RAIDUS_Server_Host[server_index].server_index = server_index + 1;
    RAIDUS_Server_Host[server_index].status = RADIUS_UNKNOWN;
    RAIDUS_Server_Host[server_index].used_flag = TRUE;
    return TRUE;
}

BOOL_T RADIUS_OM_Destroy_Server_Host(UI32_T server_index)
{
    if(server_index <= 0 || server_index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
    {
        return FALSE;
    }
    server_index --;/*database start at 0*/

    memset(&RAIDUS_Server_Host[server_index], 0, sizeof(RAIDUS_Server_Host[server_index]));

    RAIDUS_Server_Host[server_index].used_flag   = FALSE;
    RAIDUS_Server_Host[server_index].server_ip   = SYS_DFLT_RADIUS_AUTH_SERVER_ADDRESS;
    RAIDUS_Server_Host[server_index].server_port = SYS_DFLT_RADIUS_AUTH_CLIENT_SERVER_PORT_NUMBER;
    RAIDUS_Server_Host[server_index].timeout     = SYS_DFLT_RADIUS_AUTH_CLIENT_TIMEOUTS;
    RAIDUS_Server_Host[server_index].retransmit  = SYS_DFLT_RADIUS_AUTH_CLIENT_ACCESS_RETRANSMISSIONS;
    memset(RAIDUS_Server_Host[server_index].secret,0,MAXSIZE_radiusServerGlobalKey);
    RAIDUS_Server_Host[server_index].server_index = 0;
    memset(&(RAIDUS_Server_Host[server_index].server_table),0,sizeof(AuthServerEntry));

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    memset(&(RAIDUS_Server_Host[server_index].std_acc_cli_mib), 0, sizeof(RADACC_Rfc2620_T));
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    return TRUE;
}

/*if server_ip = 0, then get the first data */
BOOL_T
RADIUS_OM_GetNext_Server_Host(
    UI32_T *index,
    RADIUS_Server_Host_T *server_host)
{

    for (; *index < SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS; ++*index)
    {
        if (RAIDUS_Server_Host[*index].used_flag == TRUE)
        {
            RADIUS_OM_CopyServerHost(server_host, &RAIDUS_Server_Host[*index]);
            ++*index;
            return TRUE;
        }
    }

    return FALSE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetNextRunning_Server_Host
 *---------------------------------------------------------------------------
 * PURPOSE: This function returns SYS_TYPE_GET_RUNNING_CFG_SUCCESS if
 *          the non-default RADIUS server IP is successfully retrieved.
 *          Otherwise, SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE is returned.
 * INPUT: None.
 * OUTPUT: serverip
 * RETURN: SYS_TYPE_GET_RUNNING_CFG_SUCCESS/SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE/SYS_TYPE_GET_RUNNING_CFG_FAIL
 * NOTES: 1. This function shall only be invoked by CLI to save the
 *           "running configuration" to local or remote files.
 *        2. Since only non-default configuration will be saved, this
 *           function shall return non-default system name.
 *        3. Caller has to prepare buffer for storing system name
 *---------------------------------------------------------------------------
 */
SYS_TYPE_Get_Running_Cfg_T RADIUS_OM_GetNextRunning_Server_Host(UI32_T *index,RADIUS_Server_Host_T *server_host)
{
    SYS_TYPE_Get_Running_Cfg_T result;

    /*RADIUS_OM_LOCK();*/
    if (RADIUS_OM_GetNext_Server_Host(index,server_host))
        result = SYS_TYPE_GET_RUNNING_CFG_SUCCESS;
    else
        result = SYS_TYPE_GET_RUNNING_CFG_NO_CHANGE;
    /*RADIUS_OM_UNLOCK();*/
    return result;
}

/*server index start 0 */
BOOL_T RADIUS_OM_Get_Server_Host(UI32_T index,RADIUS_Server_Host_T *server_host)
{
    RADIUS_Server_Host_T    *entry;

    if (NULL == server_host)
        return FALSE;

    RADIUS_OM_LOCK();
    /* 1 based */
    entry = RADIUS_OM_Get_Server_Host_Entry(index + 1);
    if ((NULL == entry) || (FALSE == entry->used_flag))
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    RADIUS_OM_CopyServerHost(server_host, entry);

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_Reset_All_Server_Host_Status
 *---------------------------------------------------------------------------
 * PURPOSE: Reset all server status to unknown if all host status are
 *          unreachable
 * INPUT:   None.
 * OUTPUT:  None.
 * RETURN:  None.
 * NOTES:   None.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_Reset_All_Server_Host_Status()
{
    UI32_T i;

    RADIUS_OM_LOCK();

    for (i=0;i < SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS; i++)
    {
        if (RAIDUS_Server_Host[i].used_flag == FALSE)
            continue;

        if (RAIDUS_Server_Host[i].status != RADIUS_UNREACHABLE)
        {
            RADIUS_OM_UNLOCK();
            return;
        }
    }

    for (i=0;i < SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS; i++)
    {
        if (RAIDUS_Server_Host[i].used_flag == FALSE)
            continue;

        RAIDUS_Server_Host[i].status = RADIUS_UNKNOWN;
    }

    RADIUS_OM_UNLOCK();
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_Set_Server_Host_Status
 *---------------------------------------------------------------------------
 * PURPOSE: Set host status
 * INPUT:   index   - Server host index. 1-based.
 *          status  - Server status
 * OUTPUT:  None.
 * RETURN:  TRUE/FALSE
 * NOTES:   None.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_Set_Server_Host_Status(UI32_T index, RADIUS_ServerStatus_T status)
{
    if (index > SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
        return FALSE;

    RADIUS_OM_LOCK();

    index --;
    if (RAIDUS_Server_Host[index].used_flag == TRUE)
    {
        RAIDUS_Server_Host[index].status = status;
    }

    RADIUS_OM_UNLOCK();

    return TRUE;
}



/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetLightServerHost
 *---------------------------------------------------------------------------
 * PURPOSE  : get specified server entry by index
 * INPUT    : server_host->server_index (1-based)
 * OUTPUT   : server_host
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : index RANGE (1..SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS)
 *            fail (1). if out of range (2). server_status == RADIUS_SERVER_STATUS_NOTEXIST
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetLightServerHost(UI32_T server_index, RADIUS_LightServerHost_T *server_host)
{
    RADIUS_Server_Host_T    *entry;

    if (NULL == server_host)
        return FALSE;

    RADIUS_OM_LOCK();

    entry = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == entry) || FALSE == entry->used_flag/*(RADIUS_SERVER_STATUS_NOTEXIST == entry->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->server_index = entry->server_index;
    server_host->server_ip = entry->server_ip;
    server_host->auth_port = entry->server_port;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    server_host->acct_port = entry->acct_port;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

    server_host->timeout = entry->timeout;
    server_host->retransmit = entry->retransmit;
    strcpy((char *)server_host->secret, (char *)entry->secret);
    /*server_host->server_status = entry->server_status;*/

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_Get_Server_Host_Entry
 *---------------------------------------------------------------------------
 * PURPOSE  : get server host entry by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - not found
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
RADIUS_Server_Host_T *RADIUS_OM_Get_Server_Host_Entry(UI32_T server_index)
{
    if ((0 >= server_index) || (SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS < server_index))
        return NULL;

    return &(RAIDUS_Server_Host[server_index - 1]);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetServerHostIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : get the ip address by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : ip_address
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostIpAddress(UI32_T server_index, UI32_T *ip_address)
{
    RADIUS_Server_Host_T    *server_host;

    if (NULL == ip_address)
        return FALSE;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    *ip_address = server_host->server_ip;

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetServerHostTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : get the retransmission_timeout by server_index
 * INPUT    : server_index (1-based)
 * OUTPUT   : retransmission_timeout
 * RETURN   : TRUE -- succeeded, FALSE -- Failed
 * NOTE     : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostTimeout(UI32_T server_index, UI32_T *retransmission_timeout)
{
    RADIUS_Server_Host_T    *server_host;

    if (NULL == retransmission_timeout)
        return FALSE;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    *retransmission_timeout = server_host->timeout;

    RADIUS_OM_UNLOCK();
    return TRUE;
}


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetServerHostMaxRetransmissionTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : query the max of retransmission timeout of server hosts
 * INPUT    : none
 * OUTPUT   : max_retransmission_timeout
 * RETURN   : TRUE - succeeded; FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetServerHostMaxRetransmissionTimeout(UI32_T *max_retransmission_timeout)
{
    UI32_T  server_index;

    RADIUS_Server_Host_T    *server_host;

    if (NULL == max_retransmission_timeout)
        return FALSE;

    *max_retransmission_timeout = 0;

    RADIUS_OM_LOCK();
    for (server_index = 1; SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS >= server_index; ++server_index)
    {
        server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
        if (NULL == server_host)
            break;

        if (FALSE == server_host->used_flag/*RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status*/)
            continue;

        if (server_host->timeout > *max_retransmission_timeout)
            *max_retransmission_timeout = server_host->timeout;
    }
    RADIUS_OM_UNLOCK();
    RADIUS_OM_TRACE1("\r\n[RADIUS_OM_GetServerHostMaxRetransmissionTimeout] value = %lu", *max_retransmission_timeout);
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IsServerHostActive
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is active or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : an entry is invalid entry if server_status != RADIUS_SERVER_STATUS_ACTIVE
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IsServerHostActive(UI32_T server_index)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if (NULL == server_host)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    if (TRUE != server_host->used_flag/*RADIUS_SERVER_STATUS_ACTIVE != server_host->server_status*/)
    {
        RADIUS_OM_TRACE1("\r\n[RADIUS_OM_IsServerHostActive] server_index(%lu) is not active", server_index);
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IsServerHostValid
 *---------------------------------------------------------------------------
 * PURPOSE  : query whether this server host is valid or not
 * INPUT    : server_index (1-based)
 * OUTPUT   : none
 * RETURN   : TRUE - valid; FALSE - invalid
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IsServerHostValid(UI32_T server_index)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) ||
        (FALSE == server_host->used_flag))
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_LookupServerIndexByIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : lookup server host by ip_address
 * INPUT    : ip_address
 * OUTPUT   : server_index (1-based)
 * RETURN   : TRUE - found; FALSE - not found
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_LookupServerIndexByIpAddress(UI32_T ip_address, UI32_T *server_index)
{
    RADIUS_Server_Host_T    *server_host;

    if (NULL == server_index)
        return FALSE;

    server_host = RADIUS_OM_GetServerHostByIpAddress(ip_address);
    if (NULL == server_host)
        return FALSE;

    *server_index = server_host->server_index;
    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_CopyServerHost
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will copy one entry to another
 * INPUT:    src
 * OUTPUT:   det
 * RETURN:   none
 * NOTE:     none
 *---------------------------------------------------------------------------
 */
static void RADIUS_OM_CopyServerHost(RADIUS_Server_Host_T *det, RADIUS_Server_Host_T *src)
{
    memcpy(det, src, sizeof(RADIUS_Server_Host_T)); /* maybe better than one-by-one copy */

    det->server_table.radiusAuthServerAddress = src->server_ip;
    det->server_table.radiusAuthClientServerPortNumber = src->server_port;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
    det->std_acc_cli_mib.radiusAccServerAddress = src->server_ip;
    det->std_acc_cli_mib.radiusAccClientServerPortNumber = src->server_port;

    det->std_acc_cli_mib.radiusAccClientRequests = det->std_acc_cli_mib.radiusAccClientResponses +
        det->std_acc_cli_mib.radiusAccClientPendingRequests + det->std_acc_cli_mib.radiusAccClientTimeouts;
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */
}

I32_T RADIUS_OM_Get_Filter_Id_Privilege()
{
    return filter_id_privilege;
}

BOOL_T RADIUS_OM_Set_Filter_Id_Privilege( I32_T privilege)
{
    filter_id_privilege = privilege;
    return TRUE;
}

/*---------------------------------------------------------------------------
* FUNCTION NAME:  RADIUS_OM_LLst_Compare_Auth_Request
*---------------------------------------------------------------------------
* PURPOSE  : Compare authentication request element in linked list
* INPUT    : in_list_element_p  - inlist element
*            input_element_p    - new element
* OUTPUT   : none
* RETURN   : TRUE/FALSE
* NOTES    : none
*---------------------------------------------------------------------------
*/
static BOOL_T RADIUS_OM_LLst_Compare_Auth_Request(void *in_list_element_p,
    void *input_element_p)
{
    RADIUS_OM_RequestEntry_T *in_list_entry_p = (RADIUS_OM_RequestEntry_T *)in_list_element_p;
    RADIUS_OM_RequestEntry_T *input_entry_p = (RADIUS_OM_RequestEntry_T *)input_element_p;

    if (in_list_entry_p->type != input_entry_p->type)
    {
        return FALSE;
    }

    switch (in_list_entry_p->type)
    {
        case RADIUS_REQUEST_TYPE_IGMPAUTH:
            if (0 != memcmp(in_list_entry_p->igmp_auth_data.auth_mac,
                input_entry_p->igmp_auth_data.auth_mac,
                sizeof(in_list_entry_p->igmp_auth_data.auth_mac)))
            {
                return FALSE;
            }
            if (in_list_entry_p->igmp_auth_data.auth_port !=
                input_entry_p->igmp_auth_data.auth_port)
            {
                return FALSE;
            }
            if (in_list_entry_p->igmp_auth_data.ip_address !=
                input_entry_p->igmp_auth_data.ip_address)
            {
                return FALSE;
            }
            if (in_list_entry_p->igmp_auth_data.vlan_id !=
                input_entry_p->igmp_auth_data.vlan_id)
            {
                return FALSE;
            }
            if (in_list_entry_p->igmp_auth_data.src_ip !=
                input_entry_p->igmp_auth_data.src_ip)
            {
                return FALSE;
            }
            if (in_list_entry_p->igmp_auth_data.msg_type !=
                input_entry_p->igmp_auth_data.msg_type)
            {
                return FALSE;
            }
            break;

        case RADIUS_REQUEST_TYPE_DOT1X_AUTH:
            if (0 != strncmp(in_list_entry_p->dot1x_auth_data.username,
                input_entry_p->dot1x_auth_data.username,
                strlen(in_list_entry_p->dot1x_auth_data.username) + 1))
            {
                return FALSE;
            }
            if (0 != memcmp(in_list_entry_p->dot1x_auth_data.src_mac,
                input_entry_p->dot1x_auth_data.src_mac,
                sizeof(in_list_entry_p->dot1x_auth_data.src_mac)))
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.src_port !=
                input_entry_p->dot1x_auth_data.src_port)
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.src_vid !=
                input_entry_p->dot1x_auth_data.src_vid)
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.cookie !=
                input_entry_p->dot1x_auth_data.cookie)
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.server_ip !=
                input_entry_p->dot1x_auth_data.server_ip)
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.eap_data_len !=
                input_entry_p->dot1x_auth_data.eap_data_len)
            {
                return FALSE;
            }
            if (0 != memcmp(in_list_entry_p->dot1x_auth_data.eap_data,
                input_entry_p->dot1x_auth_data.eap_data,
                in_list_entry_p->dot1x_auth_data.eap_data_len))
            {
                return FALSE;
            }
            if (in_list_entry_p->dot1x_auth_data.state_data_len !=
                input_entry_p->dot1x_auth_data.state_data_len)
            {
                return FALSE;
            }
            if (0 != memcmp(in_list_entry_p->dot1x_auth_data.state_data,
                input_entry_p->dot1x_auth_data.state_data,
                in_list_entry_p->dot1x_auth_data.state_data_len))
            {
                return FALSE;
            }
            break;

        case RADIUS_REQUEST_TYPE_RADA_AUTH:
            if (0 != memcmp(in_list_entry_p->rada_auth_data.src_mac,
                input_entry_p->rada_auth_data.src_mac,
                sizeof(in_list_entry_p->rada_auth_data.src_mac)))
            {
                return FALSE;
            }
            if (in_list_entry_p->rada_auth_data.src_port !=
                input_entry_p->rada_auth_data.src_port)
            {
                return FALSE;
            }
            if (in_list_entry_p->rada_auth_data.cookie !=
                input_entry_p->rada_auth_data.cookie)
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->rada_auth_data.username,
                input_entry_p->rada_auth_data.username,
                strlen(in_list_entry_p->rada_auth_data.username) + 1))
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->rada_auth_data.password,
                input_entry_p->rada_auth_data.password,
                strlen(in_list_entry_p->rada_auth_data.password) + 1))
            {
                return FALSE;
            }
            break;

        case RADIUS_REQUEST_TYPE_WEB_AUTH:
            if (in_list_entry_p->user_auth_data.cookie.web !=
                input_entry_p->user_auth_data.cookie.web)
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->user_auth_data.username,
                input_entry_p->user_auth_data.username,
                strlen(in_list_entry_p->user_auth_data.username) + 1))
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->user_auth_data.password,
                input_entry_p->user_auth_data.password,
                strlen(in_list_entry_p->user_auth_data.password) + 1))
            {
                return FALSE;
            }
            break;

        case RADIUS_REQUEST_TYPE_USER_AUTH:
            if (in_list_entry_p->user_auth_data.cookie.cli.len !=
                input_entry_p->user_auth_data.cookie.cli.len)
            {
                return FALSE;
            }
            if (0 != memcmp(in_list_entry_p->user_auth_data.cookie.cli.value,
                input_entry_p->user_auth_data.cookie.cli.value,
                in_list_entry_p->user_auth_data.cookie.cli.len))
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->user_auth_data.username,
                input_entry_p->user_auth_data.username,
                strlen(in_list_entry_p->user_auth_data.username) + 1))
            {
                return FALSE;
            }
            if (0 != strncmp(in_list_entry_p->user_auth_data.password,
                input_entry_p->user_auth_data.password,
                strlen(in_list_entry_p->user_auth_data.password) + 1))
            {
                return FALSE;
            }
            break;

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
        case RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE:
            if (in_list_entry_p->accounting_create_data.request.ifindex !=
                input_entry_p->accounting_create_data.request.ifindex)
            {
                return FALSE;
            }
            if (in_list_entry_p->accounting_create_data.request.client_type !=
                input_entry_p->accounting_create_data.request.client_type)
            {
                return FALSE;
            }
            if (0 != strncmp(
                in_list_entry_p->accounting_create_data.request.user_name,
                input_entry_p->accounting_create_data.request.user_name,
                strlen(in_list_entry_p->accounting_create_data.request.user_name) + 1))
            {
                return FALSE;
            }
            break;

        case RADIUS_REQUEST_TYPE_ACCOUNTING:
            if (in_list_entry_p->accounting_data.request_type !=
                input_entry_p->accounting_data.request_type)
            {
                return FALSE;
            }
            if (in_list_entry_p->accounting_data.user_index !=
                input_entry_p->accounting_data.user_index)
            {
                return FALSE;
            }
            break;
#endif /* #if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) */

        default:
            break;
    }

    return TRUE;
}

#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccPacketNeedToHook
 *---------------------------------------------------------------------------
 * PURPOSE  : none
 * INPUT    : none
 * OUTPUT   : none.
 * RETURN   : TRUE - need to hook
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccPacketNeedToHook()
{
    return acc_some_packet_need_to_hook;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccPacketNeedToHook
 *---------------------------------------------------------------------------
 * PURPOSE  : set need_to_hook flag
 * INPUT    : need_flag
 * OUTPUT   : none.
 * RETURN   : none
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
void RADIUS_OM_SetAccPacketNeedToHook(BOOL_T need_flag)
{
    RADIUS_OM_LOCK();
    acc_some_packet_need_to_hook = need_flag;
    RADIUS_OM_UNLOCK();
}


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : none.
 * OUTPUT   : invalid_server_address_counter
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccClientInvalidServerAddresses(UI32_T *invalid_server_address_counter)
{
    if (NULL == invalid_server_address_counter)
        return FALSE;
    RADIUS_OM_LOCK();
    *invalid_server_address_counter = acc_invalid_server_address_counter;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : set the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : invalid_server_address_counter
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccClientInvalidServerAddresses(UI32_T invalid_server_address_counter)
{
    RADIUS_OM_LOCK();
    acc_invalid_server_address_counter = invalid_server_address_counter;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientInvalidServerAddresses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Response packets received from unknown addresses.
 * INPUT    : inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientInvalidServerAddresses(UI32_T inc_qty)
{
    RADIUS_OM_LOCK();
    acc_invalid_server_address_counter += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccClientRoundTripTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the time interval between the most recent Accounting-Response
 *            and the Accounting-Request that matched it from this RADIUS accounting server.
 * INPUT    : server_index (1-based), time -- the time interval.
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccClientRoundTripTime(UI32_T server_index, UI32_T time)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }
    server_host->std_acc_cli_mib.radiusAccClientRoundTripTime = time;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientRetransmissions
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Request packets retransmitted
 *            to this RADIUS accounting server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   :  none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientRetransmissions(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientRetransmissions += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientResponses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets received on the
 *            accounting port from this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientResponses(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientResponses += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientMalformedResponses
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of malformed RADIUS Accounting-Response
 *            packets received from this server. Malformed packets
 *            include packets with an invalid length. Bad
 *            authenticators and unknown types are not included as
 *            malformed accounting responses.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientMalformedResponses(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientMalformedResponses += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientBadAuthenticators
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Response
 *            packets which contained invalid authenticators
 *            received from this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientBadAuthenticators(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientBadAuthenticators += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientPendingRequests
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS Accounting-Request packets
 *            sent to this server that have not yet timed out or
 *            received a response.
 * INPUT    : server_index (1-based). inc_qty -- negative qty implies decrease
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientPendingRequests(UI32_T server_index, I32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    if (0 > inc_qty)
    {
        if ((-inc_qty) > server_host->std_acc_cli_mib.radiusAccClientPendingRequests)
            server_host->std_acc_cli_mib.radiusAccClientPendingRequests = 0;
        else
            server_host->std_acc_cli_mib.radiusAccClientPendingRequests += inc_qty;
    }
    else
    {
        server_host->std_acc_cli_mib.radiusAccClientPendingRequests += inc_qty;
    }
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientTimeouts
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of accounting timeouts to this server.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientTimeouts(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientTimeouts += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientUnknownTypes
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets of unknown type which
 *            were received from this server on the accounting port.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientUnknownTypes(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientUnknownTypes += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccClientPacketsDropped
 *---------------------------------------------------------------------------
 * PURPOSE  : increase the number of RADIUS packets which were received from
 *            this server on the accounting port and dropped for some
 *            other reason.
 * INPUT    : server_index (1-based). inc_qty
 * OUTPUT   : none.
 * RETURN   : TRUE - successful, FALSE - failure
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccClientPacketsDropped(UI32_T server_index, UI32_T inc_qty)
{
    RADIUS_Server_Host_T    *server_host;

    RADIUS_OM_LOCK();

    server_host = RADIUS_OM_Get_Server_Host_Entry(server_index);
    if ((NULL == server_host) || (FALSE==server_host->used_flag)/*(RADIUS_SERVER_STATUS_NOTEXIST == server_host->server_status)*/)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    server_host->std_acc_cli_mib.radiusAccClientPacketsDropped += inc_qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}


/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQty
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user entries
 * INPUT    : none.
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQty(UI32_T *qty)
{
    RADACC_UserInfo_T user_info;
    RADACC_UserInfo_T *user_info_p = &user_info;

    if (NULL == qty)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();

    *qty = 0;
    user_info_p->user_index = 0;

    while (TRUE == RADIUS_OM_GetNextAccUser(&user_info_p))
    {
        ++(*qty);
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByNameAndType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified name and client_type
 * INPUT    : name, client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByNameAndType(const char *name, AAA_ClientType_T client_type, UI32_T *qty)
{
    RADACC_UserInfo_T user_info;
    RADACC_UserInfo_T *user_info_p = &user_info;

    if (NULL == qty)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();

    *qty = 0;
    user_info_p->user_index = 0;

    while (TRUE == RADIUS_OM_GetNextAccUser(&user_info_p))
    {
        if (   (client_type == user_info_p->client_type)
            && (0 == strncmp(name, (char *)user_info_p->user_name,
                strlen((char *)user_info_p->user_name) + 1))
            )
        {
            ++(*qty);
        }
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByType
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user with specified client_type
 * INPUT    : client_type
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByType(AAA_ClientType_T client_type, UI32_T *qty)
{
    RADACC_UserInfo_T user_info;
    RADACC_UserInfo_T *user_info_p = &user_info;

    if (NULL == qty)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();

    *qty = 0;
    user_info_p->user_index = 0;

    while (TRUE == RADIUS_OM_GetNextAccUser(&user_info_p))
    {
        if (client_type == user_info_p->client_type)
        {
            ++(*qty);
        }
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryQtyFilterByPort
 *---------------------------------------------------------------------------
 * PURPOSE  : get the number of user from specified port
 * INPUT    : ifindex
 * OUTPUT   : qty.
 * RETURN   : TRUE - success; FALSE - fail
 * NOTES    : exclude "dead" user which connect_status ==
 *            AAA_ACC_CNET_DORMANT, AAA_ACC_CNET_IDLE, AAA_ACC_CNET_FAILED
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryQtyFilterByPort(UI32_T ifindex, UI32_T *qty)
{
    RADACC_UserInfo_T user_info;
    RADACC_UserInfo_T *user_info_p = &user_info;

    if (NULL == qty)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();

    *qty = 0;
    user_info_p->user_index = 0;

    while (TRUE == RADIUS_OM_GetNextAccUser(&user_info_p))
    {
        if (   (AAA_CLIENT_TYPE_DOT1X == user_info_p->client_type)
            && (ifindex == user_info_p->ifindex)
            )
        {
            ++(*qty);
        }
    }

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetNextAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy next accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetNextAccUserEntry(RADACC_UserInfo_T *entry)
{
    RADACC_UserInfo_T user_info;
    RADACC_UserInfo_T *user_info_p = &user_info;

    if (NULL == entry)
    {
        return FALSE;
    }

    RADIUS_OM_LOCK();

    user_info_p->user_index = entry->user_index;
    if (FALSE == RADIUS_OM_GetNextAccUser(&user_info_p))
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    memcpy(entry, user_info_p, sizeof(*entry));

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : copy accounting user to user_entry
 * INPUT    : user_index
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntry(RADACC_UserInfo_T *entry)
{
    RADACC_UserInfo_T   *user_info;

    if (NULL == entry)
        return FALSE;

    user_info = RADIUS_OM_GetAccUser(entry->user_index);
    if (NULL == user_info)
        return FALSE;

    memcpy(entry, user_info, sizeof(RADACC_UserInfo_T));

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryByKey
 *---------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : entry
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : prev_user, next_user are unavailable
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryByKey(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type, RADACC_UserInfo_T *entry)
{
    RADACC_UserInfo_T  *user_info;

    if (NULL == entry)
        return FALSE;

    RADIUS_OM_LOCK();

    user_info = RADIUS_OM_QueryAccUser(ifindex, user_name, client_type);
    if (NULL == user_info)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    memcpy(entry, user_info, sizeof(RADACC_UserInfo_T));

    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryActiveServerIdx
 *---------------------------------------------------------------------------
 * PURPOSE  : get active server index by user index
 * INPUT    : user_index
 * OUTPUT   : active_server_index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryActiveServerIdx(UI16_T user_index, UI32_T *active_server_index)
{
    RADACC_UserInfo_T   *user_info;

    if (NULL == active_server_index)
        return FALSE;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    *active_server_index = user_info->radius_entry_info.active_server_index;
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_DoesAccUserExist
 *---------------------------------------------------------------------------
 * PURPOSE  : check whether the specified accounting user exist or not
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - exist, FALSE - not exist
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_DoesAccUserExist(UI16_T user_index)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_CreateAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE  : create an accounting user for request
 * INPUT    : request, sys_time
 * OUTPUT   : none
 * RETURN   : user_index - succeeded, 0 - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
UI16_T RADIUS_OM_CreateAccUserEntry(const AAA_AccRequest_T *request, UI32_T sys_time)
{
    RADIUS_OM_TimerLinkedListElement_T *new_element_p;
    RADACC_UserInfo_T  *user_info;
    UI32_T session_id = RADIUS_OM_GetNextSessionId();

    if (NULL == request)
        return 0;

    RADIUS_OM_LOCK();

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetFreeTimerLinkedListElement(
        &radius_om_acc_user_list, &new_element_p))
    {
        RADIUS_OM_UNLOCK();
        return 0;
    }

    user_info = (RADACC_UserInfo_T *)new_element_p->data_p;

    user_info->ifindex = request->ifindex;

    strncpy((char *)user_info->user_name, (char *)request->user_name, SYS_ADPT_MAX_USER_NAME_LEN);
    user_info->user_name[SYS_ADPT_MAX_USER_NAME_LEN] = '\0'; /* force to end a string */

    user_info->session_id = session_id; /*RADIUS_OM_GetNextSessionId();*/

    user_info->accounting_start_time = sys_time;
    user_info->session_start_time = sys_time;
    user_info->last_update_time = 0;

#if(SYS_CPNT_SUPPORT_RADIUS_SESSION_LIMITATION == TRUE)
    if( request->acct_interim_interval < RADIUS_MIN_ACCT_INTERIM_INTERVAL ||
        request->acct_interim_interval > RADIUS_MAX_ACCT_INTERIM_INTERVAL )
        user_info->accounting_interim_interval = 0;
    else
        user_info->accounting_interim_interval = request->acct_interim_interval;
#else
    user_info->accounting_interim_interval = 0;
#endif

    user_info->stop_retry_time = 0;
    user_info->stop_retry_flag = FALSE;

    user_info->request_counter = 0;

    user_info->request_index = 0;

    memset(&user_info->radius_entry_info, 0, sizeof(RADACC_AAARadiusEntryInfo_T));
    memset(&user_info->ctrl_bitmap, 0, sizeof(RADACC_UserCtrlBitmap_T));

    user_info->connect_status = AAA_ACC_CNET_IDLE;

    user_info->identifier = request->identifier;
    user_info->call_back_func = request->call_back_func;

    user_info->client_type = request->client_type;

    user_info->auth_by_whom = (AAA_AccAuthentic_T)request->auth_by_whom;
    user_info->terminate_cause = request->terminate_cause;

    memcpy(user_info->auth_mac, request->auth_mac, SYS_ADPT_MAC_ADDR_LEN);

    memset(&user_info->statistic_info, 0, sizeof(RADACC_UserStatisticInfo_T));

    user_info->destroy_flag = FALSE;

    user_info->entry_status = RADACC_ENTRY_READY;

    new_element_p->time = RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME;

    if (RADIUS_RETURN_FAIL == RADIUS_OM_AddElementToTimerLinkedList(
        &radius_om_acc_user_list, new_element_p))
    {
        RADIUS_OM_UNLOCK();
        return 0;
    }

    RADIUS_OM_UNLOCK();

    return user_info->user_index;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_FreeAccUser
 *---------------------------------------------------------------------------
 * PURPOSE: recycle specific user entry from user list
 * INPUT:   user_index (1-based)
 * OUTPUT:  none.
 * RETURN:  TRUE/FALSE
 * NOTES:   none.
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_FreeAccUser(UI16_T user_index)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
    {
        RADIUS_OM_TRACE1("\r\n[RADIUS_OM_FreeAccUser] bad user_index(%d)", user_index);
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    if (RADACC_ENTRY_DESTROYED == radius_om_acc_user_list_data[om_index].entry_status)
    {
        RADIUS_OM_TRACE1("\r\n[RADIUS_OM_FreeAccUser] should not free a destroyed entry (%d)", user_index);

        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    if (RADIUS_RETURN_FAIL == RADIUS_OM_DeleteElementFromTimerLinkedList(
        &radius_om_acc_user_list, om_index))
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    radius_om_acc_user_list_data[om_index].entry_status = RADACC_ENTRY_DESTROYED;

    RADIUS_OM_UNLOCK();

    RADIUS_OM_TRACE1("\r\n[RADIUS_OM_FreeAccUser] free user(%s)", entry->user_name);

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntrySessionStartTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the session start time by specific user index
 * INPUT    : user_index, start_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntrySessionStartTime(UI16_T user_index, UI32_T start_time)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->session_start_time = start_time;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryLastUpdateTime
 *---------------------------------------------------------------------------
 * PURPOSE  : set the last update time by specific user index
 * INPUT    : user_index, update_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryLastUpdateTime(UI16_T user_index, UI32_T update_time)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->last_update_time = update_time;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryAAARadiusInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : set aaa radius info by user index
 * INPUT    : user_index, entry
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryAAARadiusInfo(UI16_T user_index, RADACC_AAARadiusEntryInfo_T *entry)
{
    RADACC_UserInfo_T   *user_info;

    if (NULL == entry)
        return FALSE;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    memcpy(&user_info->radius_entry_info, entry, sizeof(RADACC_AAARadiusEntryInfo_T));
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStartResponseSent
 *---------------------------------------------------------------------------
 * PURPOSE  : set the start package response flag by specific user index
 * INPUT    : user_index, start_response_flag
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStartResponseSent(UI16_T user_index, BOOL_T start_response_flag)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->ctrl_bitmap.start_packet_response = start_response_flag ? 1 : 0;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStopRetry
 *---------------------------------------------------------------------------
 * PURPOSE  : set the stop retry flag and stop time by specific user index
 * INPUT    : user_index, stop_retry, sys_time
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStopRetry(UI16_T user_index, BOOL_T stop_retry, UI32_T sys_time)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->stop_retry_time = stop_retry ? sys_time : 0;
    user_info->stop_retry_flag = stop_retry;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryConnectStatus
 *---------------------------------------------------------------------------
 * PURPOSE  : setup connection status by specific user index
 * INPUT    : user_index, connect_status
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryConnectStatus(UI16_T user_index, AAA_AccConnectStatus_T connect_status)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->connect_status = connect_status;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_IncAccUserEntryRequestCounter
 *---------------------------------------------------------------------------
 * PURPOSE  : increase/decrease request_counter by user_index
 * INPUT    : user_index, qty - negative implies decrease
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_IncAccUserEntryRequestCounter(UI16_T user_index, I16_T qty)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    if ((0 > qty) && ((-qty) > user_info->request_counter))
        qty = -user_info->request_counter;

    RADIUS_OM_LOCK();
    user_info->request_counter += qty;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_ResetAccUserEntryCallbackInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : reset the call_back_func, identifier by specific user index
 * INPUT    : user_index
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_ResetAccUserEntryCallbackInfo(UI16_T user_index)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->call_back_func = NULL;
    user_info->identifier = 0;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryTerminateCause
 *---------------------------------------------------------------------------
 * PURPOSE  : set the terminate_cause by specific user index
 * INPUT    : user_index, terminate_cause
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryTerminateCause(UI16_T user_index, AAA_AccTerminateCause_T terminate_cause)
{
    RADACC_UserInfo_T   *user_info;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    user_info->terminate_cause = terminate_cause;
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryStatisticInfo
 *---------------------------------------------------------------------------
 * PURPOSE  : set the statistic_info by specific user index
 * INPUT    : user_index, statistic_info
 * OUTPUT   : none
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryStatisticInfo(UI16_T user_index, RADACC_UserStatisticInfo_T *statistic_info)
{
    RADACC_UserInfo_T   *user_info;

    if (NULL == statistic_info)
        return FALSE;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    RADIUS_OM_LOCK();
    memcpy(&user_info->statistic_info, statistic_info, sizeof(RADACC_UserStatisticInfo_T));
    RADIUS_OM_UNLOCK();
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntrySessionId
 *---------------------------------------------------------------------------
 * PURPOSE  : get specified user's session id
 * INPUT    : user_index, buffer_size
 * OUTPUT   : id_buffer
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntrySessionId(UI16_T user_index, UI8_T *id_buffer, UI16_T buffer_size)
{
    UI16_T  digit_cnt, div_num;
    RADACC_UserInfo_T   *user_info;

    if (NULL == id_buffer)
        return FALSE;

    user_info = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_info)
        return FALSE;

    for (digit_cnt = 1, div_num = user_info->session_id; 0 < div_num; ++digit_cnt)
    {
        div_num /= 10;
    }

    if (digit_cnt >= buffer_size) /* buffer size must be reserved 1-char for '\0' */
    {
        RADIUS_OM_TRACE2("\r\n[RADIUS_OM_GetAccUserEntrySessionId] buffer size(%d) is not enough (<= %d)",
            buffer_size, digit_cnt);
        return FALSE;
    }

    sprintf((char *)id_buffer, "%lu", user_info->session_id);
    RADIUS_OM_TRACE2("\r\n[RADIUS_OM_GetAccUserEntrySessionId] user(%s) session id(%s)", user_info->user_name, id_buffer);
    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : Get timeout of the specified accounting user.
 * INPUT    : user_index    - User index
 * OUTPUT   : timeout_p     - Timeout
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryTimeout(UI16_T user_index, UI32_T *timeout_p)
{
    UI32_T om_index;
    RADACC_UserInfo_T *user_entry_p;

    if (   (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
        || (NULL == timeout_p)
        )
    {
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    user_entry_p = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_entry_p)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    *timeout_p = radius_om_acc_user_list_elements[om_index].time;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE  : Set timeout of the specified accounting user.
 * INPUT    : user_index    - User index
 *            timeout       - Timeout
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryTimeout(UI16_T user_index, UI32_T timeout)
{
    UI32_T om_index;
    RADACC_UserInfo_T *user_entry_p;

    if (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
    {
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    user_entry_p = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_entry_p)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    radius_om_acc_user_list_elements[om_index].time = timeout;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUserEntryRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Get request index of the specified accounting user.
 * INPUT    : user_index        - User index
 * OUTPUT   : request_index_p   - Request index
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_GetAccUserEntryRequestIndex(UI16_T user_index,
    UI32_T *request_index_p)
{
    UI32_T om_index;
    RADACC_UserInfo_T *user_entry_p;

    if (   (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
        || (NULL == request_index_p)
        )
    {
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    user_entry_p = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_entry_p)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    *request_index_p = radius_om_acc_user_list_data[om_index].request_index;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_SetAccUserEntryRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE  : Set request index of the specified accounting user.
 * INPUT    : user_index    - User index
 *            request_index - Request index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_SetAccUserEntryRequestIndex(UI16_T user_index,
    UI32_T request_index)
{
    UI32_T om_index;
    RADACC_UserInfo_T *user_entry_p;

    if (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
    {
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    user_entry_p = RADIUS_OM_GetAccUser(user_index);
    if (NULL == user_entry_p)
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    radius_om_acc_user_list_data[om_index].request_index = request_index;
    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAccUserEntryDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE: Set the destroy flag of the specified accounting user.
 * INPUT:   user_index - User index
 *          flag            - Destroy flag
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAccUserEntryDestroyFlag(UI32_T user_index,
    BOOL_T flag)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    if (RADACC_ENTRY_DESTROYED ==
        radius_om_acc_user_list_data[om_index].entry_status)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_acc_user_list_data[om_index].destroy_flag = flag;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAccUserEntryDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the destroy flag of the specified accounting user.
 * INPUT:    user_index - User index
 * OUTPUT:   flag_p     - Destroy flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAccUserEntryDestroyFlag(
    UI32_T user_index, BOOL_T *flag_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
        || (NULL == flag_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    if (RADACC_ENTRY_DESTROYED ==
        radius_om_acc_user_list_data[om_index].entry_status)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *flag_p = radius_om_acc_user_list_data[om_index].destroy_flag;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_ResortAccUserEntryByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE  : Resort the accounting users list by ascending timeout order.
 * INPUT    : user_index    - User index
 * OUTPUT   : None
 * RETURN   : TRUE - succeeded, FALSE - failed
 * NOTES    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_ResortAccUserEntryByTimeoutOrder(UI16_T user_index)
{
    UI32_T om_index;
    RADIUS_ReturnValue_T ret;

    if (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
    {
        return FALSE;
    }

    om_index = user_index - 1;

    RADIUS_OM_LOCK();

    if (RADIUS_RETURN_FAIL == RADIUS_OM_ResortTimerLinkedListByTimeOrder(
        &radius_om_acc_user_list, om_index))
    {
        RADIUS_OM_UNLOCK();
        return FALSE;
    }

    RADIUS_OM_UNLOCK();

    return TRUE;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRecentAccUserEntryTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the smallest timeout between all accounting users.
 * INPUT:    None
 * OUTPUT    timeout_p  - Recent timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRecentAccUserEntryTimeout(UI32_T *timeout_p)
{
    if (NULL == timeout_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (NULL == radius_om_acc_user_list.head_p)
    {
        *timeout_p = RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME;

        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_SUCCESS;
    }

    *timeout_p = radius_om_acc_user_list.head_p->time;

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextAccUserByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Get the next accounting user by ascending timeout order.
 * INPUT:   user_index_p    - User index (0 for get first user)
 * OUTPUT:  user_index_p    - Next user index by ascending timeout order
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextAccUserByTimeoutOrder(
    UI32_T *user_index_p)
{
    RADACC_UserInfo_T *user_entry_p;

    if (   (NULL == user_index_p)
        || (   (0 != *user_index_p)
            && (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(*user_index_p))
            )
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (0 == *user_index_p)
    {
        if (NULL == radius_om_acc_user_list.head_p)
        {
            RADIUS_OM_UNLOCK();
            return RADIUS_RETURN_FAIL;
        }

        user_entry_p = (RADACC_UserInfo_T *)radius_om_acc_user_list.head_p->data_p;
        *user_index_p = user_entry_p->user_index;
    }
    else
    {
        UI32_T om_index = *user_index_p - 1;
        RADIUS_OM_RadiusRequest_T *request_p;

        user_entry_p = (RADACC_UserInfo_T *)
            radius_om_acc_user_list_elements[om_index].data_p;

        if (   (RADACC_ENTRY_DESTROYED == user_entry_p->entry_status)
            || (NULL == radius_om_acc_user_list_elements[om_index].next_entry_p)
            )
        {
            RADIUS_OM_UNLOCK();
            return RADIUS_RETURN_FAIL;
        }

        user_entry_p = (RADACC_UserInfo_T *)
            radius_om_acc_user_list_elements[om_index].next_entry_p->data_p;
        *user_index_p = user_entry_p->user_index;
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyAccUserEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the accounting user entry.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyAccUserEntry()
{
    BOOL_T ret;

    RADIUS_OM_LOCK();

    ret = (NULL != radius_om_acc_user_list.head_p);

    RADIUS_OM_UNLOCK();

    return ret;
}
#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * FUNCTION NAME - RADIUS_OM_HandleIPCReqMsg
 *---------------------------------------------------------------------------
 * PURPOSE : Handle the ipc request message for RADIUS om.
 * INPUT   : ipcmsg_p  --  input request ipc message buffer
 * OUTPUT  : ipcmsg_p  --  output response ipc message buffer
 * RETUEN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HandleIPCReqMsg(SYSFUN_Msg_T *ipcmsg_p)
{
    if(ipcmsg_p == NULL)
        return FALSE;

    switch(RADIUS_OM_MSG_CMD(ipcmsg_p))
    {
        case RADIUS_OM_IPC_CMD_GET_RUNNING_REQUEST_TIMEOUT:
        {
            RADIUS_OM_IPCMsg_RequestTimeout_T *data_p = (RADIUS_OM_IPCMsg_RequestTimeout_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningRequestTimeout(&data_p->timeout);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RequestTimeout_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_REQUEST_TIMEOUT:
        {
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_Get_Request_Timeout();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_PORT:
        {
            RADIUS_OM_IPCMsg_ServerPort_T *data_p = (RADIUS_OM_IPCMsg_ServerPort_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningServerPort(&data_p->serverport);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerPort_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_SERVER_PORT:
        {
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_Get_Server_Port();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_SECRET:
        {
            RADIUS_OM_IPCMsg_ServerSecret_T *data_p = (RADIUS_OM_IPCMsg_ServerSecret_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningServerSecret(data_p->serversecret);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_SERVER_SECRET:
        {
            RADIUS_OM_IPCMsg_ServerSecret_T *data_p = (RADIUS_OM_IPCMsg_ServerSecret_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            UI8_T *serversecret = RADIUS_OM_Get_Server_Secret();
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = serversecret ? TRUE : FALSE;
            if (serversecret)
                memcpy(data_p->serversecret, serversecret, sizeof(data_p->serversecret));
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerSecret_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_RUNNING_RETRANSMIT_TIMES:
        {
            RADIUS_OM_IPCMsg_RetransmitTimes_T *data_p = (RADIUS_OM_IPCMsg_RetransmitTimes_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningRetransmitTimes(&data_p->retimes);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_RetransmitTimes_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_RETRANSMIT_TIMES:
        {
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_Get_Retransmit_Times();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_IP:
        {
            RADIUS_OM_IPCMsg_ServerIP_T *data_p = (RADIUS_OM_IPCMsg_ServerIP_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningServerIP(&data_p->serverip);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerIP_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_SERVER_IP:
        {
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_Get_Server_IP();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_UNKNOW_ADDRESS_PACKETS:
        {
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_SNMP_Get_UnknowAddress_Packets();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE_FOR_EMPTY_DATA();
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_NAS_ID:
        {
            RADIUS_OM_IPCMsg_NAS_ID_T *data_p = (RADIUS_OM_IPCMsg_NAS_ID_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_SNMP_Get_NAS_ID(data_p->nas_id);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_NAS_ID_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_AUTH_SERVER_TABLE:
        {
            RADIUS_OM_IPCMsg_AuthServerTable_T *data_p = (RADIUS_OM_IPCMsg_AuthServerTable_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_SNMP_GetAuthServerTable(data_p->index, &data_p->ServerEntry);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_NEXT_AUTH_SERVER_TABLE:
        {
            RADIUS_OM_IPCMsg_AuthServerTable_T *data_p = (RADIUS_OM_IPCMsg_AuthServerTable_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_SNMP_GetNextAuthServerTable(&data_p->index, &data_p->ServerEntry);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_AuthServerTable_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_NEXT_SERVER_HOST:
        {
            RADIUS_OM_IPCMsg_ServerHost_T *data_p = (RADIUS_OM_IPCMsg_ServerHost_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetNext_Server_Host(&data_p->index, &data_p->server_host);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_NEXT_RUNNING_SERVER_HOST:
        {
            RADIUS_OM_IPCMsg_ServerHost_T *data_p = (RADIUS_OM_IPCMsg_ServerHost_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetNextRunning_Server_Host(&data_p->index, &data_p->server_host);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_SERVER_HOST:
        {
            RADIUS_OM_IPCMsg_ServerHost_T *data_p = (RADIUS_OM_IPCMsg_ServerHost_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_Get_Server_Host(data_p->index, &data_p->server_host);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerHost_T);
            break;
        }
#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE) /*maggie liu, 2009-03-09*/
        case RADIUS_OM_IPC_CMD_GET_RUNNING_SERVER_ACCT_PORT:
        {
            RADIUS_OM_IPCMsg_ServerAcctPort_T *data_p = (RADIUS_OM_IPCMsg_ServerAcctPort_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetRunningServerAcctPort(&data_p->acct_port);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T);
            break;
        }
        case RADIUS_OM_IPC_CMD_GET_SERVER_ACCT_PORT:
        {
            RADIUS_OM_IPCMsg_ServerAcctPort_T *data_p = (RADIUS_OM_IPCMsg_ServerAcctPort_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = TRUE;
            data_p->acct_port = RADIUS_OM_GetServerAcctPort();
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_ServerAcctPort_T);
            break;
        }
        case RADIUS_POM_GET_ACC_CLIENT_INVALID_SERVER_ADDRESSES:
        {
            RADIUS_OM_IPCMsg_Counter_T *data_p = (RADIUS_OM_IPCMsg_Counter_T *)RADIUS_OM_MSG_DATA(ipcmsg_p);
            RADIUS_OM_MSG_RETVAL(ipcmsg_p) = RADIUS_OM_GetAccClientInvalidServerAddresses(&data_p->counter);
            ipcmsg_p->msg_size = RADIUS_OM_GET_MSGBUFSIZE(RADIUS_OM_IPCMsg_Counter_T);
            break;
        }
#endif
        default:
            SYSFUN_Debug_Printf("\r\n%s(): Invalid cmd.", __FUNCTION__);
            return FALSE;
    } /* switch ipcmsg_p->cmd */

    return TRUE;
} /* RADIUS_OM_HandleIPCReqMsg */

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetServerHostByIpAddress
 *---------------------------------------------------------------------------
 * PURPOSE  : get server host by ip address
 * INPUT    : ip_address
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static RADIUS_Server_Host_T *RADIUS_OM_GetServerHostByIpAddress(UI32_T ip_address)
{
    UI32_T  index;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_RADIUS_SERVERS > index; ++index)
    {
        if ((FALSE == RAIDUS_Server_Host[index].used_flag)/*RADIUS_SERVER_STATUS_NOTEXIST == RAIDUS_Server_Host[index].server_status*/)
            continue;

        if (ip_address == RAIDUS_Server_Host[index].server_ip)
            return &RAIDUS_Server_Host[index];
    }

    RADIUS_OM_TRACE1("\r\n[RADIUS_OM_GetServerHostByIpAddress] server host with ip_address(%lu) not found", ip_address);
    return NULL;
}


#if (SYS_CPNT_RADIUS_ACCOUNTING == TRUE)
/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetAccUser
 *---------------------------------------------------------------------------
 * PURPOSE  : get specific user entry from user list
 * INPUT    : user_index (1-based)
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : none.
 *---------------------------------------------------------------------------
 */
static RADACC_UserInfo_T *RADIUS_OM_GetAccUser(UI16_T user_index)
{
    RADACC_UserInfo_T   *user_info;

    if ((0 >= user_index) || (SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS < user_index))
        return NULL;

    user_info = (RADACC_UserInfo_T *)&radius_om_acc_user_list_data[user_index - 1];
    if (RADACC_ENTRY_DESTROYED == user_info->entry_status)
        return NULL;

    return user_info;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_GetNextAccUser
 *---------------------------------------------------------------------------
 * PURPOSE  : Get next accounting user.
 * INPUT    : user_entry_p  - Accounting user (0 for get first user)
 * OUTPUT   : user_entry_p  - Next accounting user
 * RETURN   : TRUE/FALSE
 * NOTES    : MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static BOOL_T RADIUS_OM_GetNextAccUser(RADACC_UserInfo_T **user_entry_pp)
{
    UI16_T user_index;
    RADACC_UserInfo_T *next_user_p;

    if (NULL == user_entry_pp)
    {
        return FALSE;
    }

    user_index = (*user_entry_pp)->user_index;

    if (   (0 != user_index)
        && (FALSE == RADIUS_OM_IS_ACC_USER_INDEX_VALID(user_index))
        )
    {
        return FALSE;
    }

    if (0 == user_index)
    {
        if (NULL == radius_om_acc_user_list.head_p)
        {
            return FALSE;
        }

        next_user_p = (RADACC_UserInfo_T *)
            radius_om_acc_user_list.head_p->data_p;
    }
    else
    {
        if (NULL == radius_om_acc_user_list_elements[user_index - 1].next_entry_p)
        {
            return FALSE;
        }

        next_user_p = (RADACC_UserInfo_T *)
            radius_om_acc_user_list_elements[user_index - 1].
            next_entry_p->data_p;
    }

    if (RADACC_ENTRY_DESTROYED == next_user_p->entry_status)
    {
        return FALSE;
    }

    *user_entry_pp = next_user_p;

    return TRUE;
}

/*---------------------------------------------------------------------------
 * FUNCTION NAME:  RADIUS_OM_QueryAccUser
 *---------------------------------------------------------------------------
 * PURPOSE  : query specific user entry from user list
 * INPUT    : ifindex, user_name, client_type
 * OUTPUT   : none
 * RETURN   : NULL - failed
 * NOTES    : MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADACC_UserInfo_T *RADIUS_OM_QueryAccUser(UI32_T ifindex, const char *user_name, AAA_ClientType_T client_type)
{
    RADACC_UserInfo_T  *entry;
    RADIUS_OM_TimerLinkedListElement_T *element_p;

    if (NULL == user_name)
    {
        return NULL;
    }

    element_p = radius_om_acc_user_list.head_p;

    while (element_p)
    {
        RADACC_UserInfo_T *user_entry_p;

        user_entry_p = (RADACC_UserInfo_T *)element_p->data_p;

        if (   (ifindex == user_entry_p->ifindex)
            && (0 == strncmp(user_name, (char *)user_entry_p->user_name,
                strlen((char *)user_entry_p->user_name) + 1))
            && (client_type == user_entry_p->client_type)
            )
        {
            return user_entry_p;
        }

        element_p = element_p->next_entry_p;
    }

    return NULL;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_GetNextSessionId
 *---------------------------------------------------------------------------
 * PURPOSE  : calculate the next available session id
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : session id
 * NOTES    : none
 *---------------------------------------------------------------------------
 */
static UI32_T RADIUS_OM_GetNextSessionId()
{
    UI32_T  current_id = acc_session_id;

    RADIUS_OM_LOCK();
    if ((acc_session_id & RADACC_MAX_SESSION_ID_FOR_USER) == RADACC_MAX_SESSION_ID_FOR_USER)
    {
        RADIUS_OM_TRACE("\r\n[RADIUS_OM_GetNextSessionId] reset session id (user part)");
        acc_session_id = RADACC_MIN_SESSION_ID_FOR_USER;
    }
    else
    {
        RADIUS_OM_TRACE("\r\n[RADIUS_OM_GetNextSessionId] increment (user part))");
        acc_session_id += RADACC_INC_SESSION_ID_FOR_USER;
    }
    RADIUS_OM_UNLOCK();

    RADIUS_OM_TRACE1("\r\n[RADIUS_OM_GetNextSessionId] %lu", current_id);
    return current_id;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_DumpAccUserTable
 *---------------------------------------------------------------------------
 * PURPOSE  : try to dump user table
 * INPUT    : none
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : this func is used to debug only
 *---------------------------------------------------------------------------
 */
#if 0
void RADIUS_OM_DumpAccUserTable()
{
    UI16_T      index;

    RADACC_UserInfo_T  *entry;

    for (index = 0; SYS_ADPT_MAX_NBR_OF_RADIUS_ACC_USERS > index; ++index)
    {
        entry = &acc_user_info[index];

        if (RADACC_ENTRY_DESTROYED == entry->entry_status)
            continue;

        printf("\r\n[RADIUS_OM_DumpAccUserTable] index(%d)", entry->user_index);
        printf("\r\n\tname(%s) ifindex(%lu), start time(%lu)",
            entry->user_name, entry->ifindex, entry->accounting_start_time);

        printf("\r\n%20s(%5lu) %20s(%5lu)",
            "session_start_time", entry->session_start_time,
            "last_update_time", entry->last_update_time);

        printf("\r\n%20s(%5d) %20s(%5d)",
            "aaa_group_index", entry->radius_entry_info.aaa_group_index,
            "aaa_radius_index", entry->radius_entry_info.aaa_radius_index);

        printf("\r\n%20s(%5lu) %20s(%5d)",
            "active_server_index", entry->radius_entry_info.active_server_index,
            "aaa_radius_order", entry->radius_entry_info.aaa_radius_order);

        printf("\r\n%20s(%5s) %20s(%5d)",
            "stop_retry_flag", (TRUE == entry->stop_retry_flag) ? "TRUE" : "FALSE",
            "request_counter", entry->request_counter);

        printf("\r\n%20s(%5lu) %20s(%s)",
            "identifier", entry->identifier,
            "client type",
            (AAA_CLIENT_TYPE_DOT1X == entry->client_type) ? "AAA_CLIENT_TYPE_DOT1X" :
            "unknown client type");

        printf("\r\n%20s(%5s) %20s(%5s)",
            "start_packet_sent", (1 == entry->ctrl_bitmap.start_packet_sent) ? "Yes" : "No",
            "stop_packet_sent", (1 == entry->ctrl_bitmap.stop_packet_sent) ? "Yes" : "No");
    }
}
#endif /* #if 0 */

/*---------------------------------------------------------------------------
 * ROUTINE NAME  - RADIUS_OM_DumpAccRequestTable
 *---------------------------------------------------------------------------
 * PURPOSE  : try to dump request table
 * INPUT    : none
 * OUTPUT   : None
 * RETURN   : none
 * NOTES    : this func is used to debug only
 *---------------------------------------------------------------------------
 */
#if 0
void RADIUS_OM_DumpAccRequestTable()
{
    UI16_T      index;

    RADACC_RequestInfo_T *entry;

    for (index = 0; RADACC_MAX_NBR_OF_REQUEST > index; ++index)
    {
        entry = &acc_request_queue[index];

        if (RADACC_ENTRY_DESTROYED == entry->entry_status)
            continue;

        printf("\r\n[RADIUS_OM_DumpAccRequestTable] index(%d)", entry->request_index);
        printf("\r\n%15s(%5d) %15s(%10s)",
            "user_index", entry->user_index,
            "request_type",
            (RADACC_START == entry->request_type) ? "RADACC_START" :
            (RADACC_STOP == entry->request_type) ? "RADACC_STOP" :
            "unknown request type");

        printf("\r\n%15s(%5lu) %15s(%s)",
            "identifier", entry->identifier,
            "request_state",
            (RADACC_REQUEST_SENDING == entry->request_state) ? "RADACC_REQUEST_SENDING" :
            (RADACC_ACK_RECEIVING == entry->request_state) ? "RADACC_ACK_RECEIVING" :
            "unknown request state");

        printf("\r\n%20s(%5d)", "request identifier", entry->request_data.identifier);
        printf("\r\n%20s(%5lu) %20s(%5lu)",
            "server_index", entry->send_data.server_index,
            "request_sent_time", entry->send_data.request_sent_time);

        printf("\r\n%20s(%5d) %20s(%5lu)",
            "sock_id", entry->send_data.sock_id,
            "retry_times", entry->send_data.retry_times);
    }
}
#endif /* if 0 */

#endif /* SYS_CPNT_RADIUS_ACCOUNTING == TRUE */

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_ResortRequestByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Resort the request in process queue by ascending timeout order.
 * INPUT:   request_index   - Request index
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_ResortRequestByTimeoutOrder(
    UI32_T request_index)
{
    UI32_T om_index;
    RADIUS_ReturnValue_T ret;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_ResortTimerLinkedListByTimeOrder(
        &radius_om_request_list, om_index);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyFreeRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the free request entry in process queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyFreeRequestEntry()
{
    BOOL_T ret;

    RADIUS_OM_LOCK();

    ret = (NULL != radius_om_request_list.free_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestEntry
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether the request entry in process queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestEntry()
{
    BOOL_T ret;

    RADIUS_OM_LOCK();

    ret = (NULL != radius_om_request_list.head_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequest
 *---------------------------------------------------------------------------
 * PURPOSE: Add the request into process queue.
 * INPUT:   request_data_p  - Request data
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequest(
    RADIUS_OM_RequestEntry_T *request_data_p, UI32_T *request_index_p)
{
    RADIUS_OM_TimerLinkedListElement_T *new_element_p;
    RADIUS_OM_RadiusRequest_T *request_p;

    if (   (NULL == request_data_p)
        || (NULL == request_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (RADIUS_RETURN_FAIL == RADIUS_OM_GetFreeTimerLinkedListElement(
        &radius_om_request_list, &new_element_p))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    request_p = (RADIUS_OM_RadiusRequest_T *)new_element_p->data_p;
    request_p->state = RADIUS_REQUEST_STATE_INIT;
    request_p->server_index = 0;
    request_p->identifier = RADIUS_OM_INVALID_IDENTIFIER;
    request_p->is_pending_counter_increased = FALSE;
    request_p->is_used = TRUE;
    memcpy(&request_p->request_data, request_data_p, sizeof(request_p->request_data));

    request_p->response_packet_p = NULL;
    request_p->response_packet_len = 0;

    new_element_p->time = RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME;
    if (RADIUS_RETURN_FAIL == RADIUS_OM_AddElementToTimerLinkedList(
        &radius_om_request_list, new_element_p))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *request_index_p = RADIUS_OM_REQUEST_INDEX(new_element_p);

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequest
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from process queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequest(UI32_T request_index)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }


    if (RADIUS_RETURN_FAIL == RADIUS_OM_DeleteElementFromTimerLinkedList(
        &radius_om_request_list, om_index))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    /* Reset the data.
     */
    memset(&radius_om_request_list_data[om_index], 0, sizeof(radius_om_request_list_data[om_index]));
    radius_om_request_list_data[om_index].is_used = FALSE;

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextRequestByTimeoutOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Get the next request by ascending timeout order.
 * INPUT:   request_index_p - Request index (0 for get first request)
 * OUTPUT:  request_index_p - Next request index by ascending timeout order
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextRequestByTimeoutOrder(
    UI32_T *request_index_p)
{
    if (   (NULL == request_index_p)
        || (   (0 != *request_index_p)
            && (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(*request_index_p))
            )
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (0 == *request_index_p)
    {
        if (NULL == radius_om_request_list.head_p)
        {
            RADIUS_OM_UNLOCK();
            return RADIUS_RETURN_FAIL;
        }

        *request_index_p = RADIUS_OM_REQUEST_INDEX(
            radius_om_request_list.head_p);
    }
    else
    {
        UI32_T om_index = *request_index_p - 1;
        RADIUS_OM_RadiusRequest_T *request_p;

        request_p = (RADIUS_OM_RadiusRequest_T *)
            radius_om_request_list_elements[om_index].data_p;

        if (   (FALSE == request_p->is_used)
            || (NULL == radius_om_request_list_elements[om_index].next_entry_p)
            )
        {
            RADIUS_OM_UNLOCK();
            return RADIUS_RETURN_FAIL;
        }

        *request_index_p = RADIUS_OM_REQUEST_INDEX(
            radius_om_request_list_elements[om_index].next_entry_p);
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE: Set the destroy flag in specified requests.
 * INPUT:   request_index   - Request index
 *          flag            - Destroy flag
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestDestroyFlag(UI32_T request_index,
    BOOL_T flag)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].destroy_flag = flag;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestDestroyFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the destroy flag of the specified request.
 * INPUT:    request_index  - Request index
 * OUTPUT:   flag_p         - Destroy flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestDestroyFlag(UI32_T request_index,
    BOOL_T *flag_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == flag_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *flag_p = radius_om_request_list_data[om_index].destroy_flag;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the request index of the specified request.
 * INPUT:    request_p          - The specified request
 * OUTPUT:   request_index_p    - Found request index
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestIndex(
    RADIUS_OM_RequestEntry_T *request_p, UI32_T *request_index_p)
{
    RADIUS_OM_TimerLinkedListElement_T *traverse_request_p;

    if (   (NULL == request_p)
        || (NULL == request_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    traverse_request_p = radius_om_request_list.head_p;

    while (traverse_request_p)
    {
        RADIUS_OM_RadiusRequest_T *om_request_p;

        om_request_p = (RADIUS_OM_RadiusRequest_T *)traverse_request_p->data_p;
        if (TRUE == RADIUS_OM_LLst_Compare_Auth_Request(request_p,
            &om_request_p->request_data))
        {
            *request_index_p = RADIUS_OM_REQUEST_INDEX(traverse_request_p);

            RADIUS_OM_UNLOCK();
            return RADIUS_RETURN_SUCCESS;
        }

        traverse_request_p = traverse_request_p->next_entry_p;
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_FAIL;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddAuthReqIntoWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Add the authentication request into the waiting queue.
 * INPUT:    request_entry_p    - Request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddAuthReqIntoWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p)
{
    BOOL_T is_auth_request;

    if (NULL == request_entry_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    is_auth_request = (RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE !=
        request_entry_p->type);

    RADIUS_OM_LOCK();

    /* Check for maximum allowed number of authentication/accounting request
     */
    if (   (   (TRUE == is_auth_request)
            && (radius_om_req_waiting_queue.max_auth_req_count <=
                radius_om_req_waiting_queue.nbr_of_auth_req)
            )
        || (   (FALSE == is_auth_request)
            && (radius_om_req_waiting_queue.max_acct_req_count <=
                radius_om_req_waiting_queue.nbr_of_acct_req)
            )
        )
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    if (FALSE == L_LINK_LST_Set(&radius_om_req_waiting_queue.list,
        request_entry_p, L_LINK_LST_APPEND))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    if (TRUE == is_auth_request)
    {
        radius_om_req_waiting_queue.nbr_of_auth_req++;
    }
    else
    {
        radius_om_req_waiting_queue.nbr_of_acct_req++;
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteFirstAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Delete the first authentication request from the waiting queue.
 * INPUT:    None
 * OUTPUT:   request_entry_p    - First request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteFirstAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p)
{
    BOOL_T ret;

    if (NULL == request_entry_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (FALSE == L_LINK_LST_Get_1st(&radius_om_req_waiting_queue.list,
        request_entry_p))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    if (FALSE == L_LINK_LST_Delete(&radius_om_req_waiting_queue.list,
        request_entry_p))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE != request_entry_p->type)
    {
        radius_om_req_waiting_queue.nbr_of_auth_req--;
    }
    else
    {
        radius_om_req_waiting_queue.nbr_of_acct_req--;
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  Delete the specified authentication request from the waiting
 *           queue.
 * INPUT:    request_entry_p    - Specified request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p)
{
    if (NULL == request_entry_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (FALSE == L_LINK_LST_Delete(&radius_om_req_waiting_queue.list,
        request_entry_p))
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_REQUEST_TYPE_ACCOUNTING_CREATE != request_entry_p->type)
    {
        radius_om_req_waiting_queue.nbr_of_auth_req--;
    }
    else
    {
        radius_om_req_waiting_queue.nbr_of_acct_req--;
    }

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetNextAuthReqFromWaitingQueue
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the next authentication request from the
 *           waiting queue.
 * INPUT:    request_entry_p - entry of the request
 * OUTPUT:   request_entry_p - entry of the request to the inputed request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetNextAuthReqFromWaitingQueue(
    RADIUS_OM_RequestEntry_T *request_entry_p)
{
    BOOL_T ret;

    if (NULL == request_entry_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = L_LINK_LST_Get_Next(&radius_om_req_waiting_queue.list,
        request_entry_p);

    RADIUS_OM_UNLOCK();

    return (TRUE == ret) ? RADIUS_RETURN_SUCCESS : RADIUS_RETURN_FAIL;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestState
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will get the state of the specific request
 * INPUT:    request_id - request id
 * OUTPUT:   state_p    - state of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestState(UI32_T request_id,
    RADIUS_OM_RequestState_T *state_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == state_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *state_p = radius_om_request_list_data[om_index].state;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestState
 *---------------------------------------------------------------------------
 * PURPOSE:  This function will set the state of the specific request
 * INPUT:    request_id - request id
 *           state      - state of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestState(UI32_T request_id,
    RADIUS_OM_RequestState_T state)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (FALSE == RADIUS_OM_IS_REQUEST_STATE_VALID(state))
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].state = state;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the identifier of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   identifier_p   - Identifier
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestIdentifier(UI32_T request_id,
    UI32_T *identifier_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == identifier_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *identifier_p = radius_om_request_list_data[om_index].identifier;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestIdentifier
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the identifier of the specified request.
 * INPUT:    request_id - Request id
 *           identifier - Identifier
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestIdentifier(UI32_T request_id,
    UI32_T identifier)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (FALSE == RADIUS_OM_IS_REQUEST_IDENTIFIER_VALID(identifier))
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].identifier = identifier;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the timeout of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   timeout_p  - Timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestTimeout(UI32_T request_id,
    UI32_T *timeout_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == timeout_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *timeout_p = radius_om_request_list_elements[om_index].time;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the timeout of the specified request.
 * INPUT:    request_id - Request id
 *           timeout    - Timeout
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestTimeout(UI32_T request_id,
    UI32_T timeout)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_elements[om_index].time = timeout;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestData
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the data of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   data_p     - Request data
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestData(UI32_T request_id,
    RADIUS_OM_RequestEntry_T *data_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == data_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(data_p, &radius_om_request_list_data[om_index].request_data,
        sizeof(*data_p));
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the retry times of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   retry_times_p  - Retry times of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestRetryTimes(UI32_T request_id,
    UI32_T *retry_times_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == retry_times_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *retry_times_p = radius_om_request_list_data[om_index].retry_times;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestRetryTimes
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the retry times of the specified request.
 * INPUT:    request_id     - Request id
 *           retry_times    - Retry times of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestRetryTimes(UI32_T request_id,
    UI32_T retry_times)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].retry_times = retry_times;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestServerIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the server index of the specified request.
 * INPUT:    request_id     - Request id
 * OUTPUT:   server_index_p - Server index of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestServerIndex(UI32_T request_id,
    UI32_T *server_index_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == server_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *server_index_p = radius_om_request_list_data[om_index].server_index;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestServerIndex
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the server index of the specified request.
 * INPUT:    request_id     - Request id
 *           server_index   - Server index of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestServerIndex(UI32_T request_id,
    UI32_T server_index)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].server_index = server_index;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestType
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the request type of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   type_p     - Request type
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestType(UI32_T request_id,
    RADIUS_OM_RadiusRequestType_T *type_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == type_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *type_p = radius_om_request_list_data[om_index].request_data.type;

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestVector
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the vector of the specified request.
 * INPUT:    request_id - Request id
 * OUTPUT:   vector_p   - Vector
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestVector(UI32_T request_id,
    UI8_T *vector_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == vector_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(vector_p, radius_om_request_list_data[om_index].vector,
        sizeof(radius_om_request_list_data[om_index].vector));

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestVector
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the vector of the specified request.
 * INPUT:    request_id - Request id
 *           vector_p   - Vector
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestVector(UI32_T request_id,
    UI8_T *vector_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == vector_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(radius_om_request_list_data[om_index].vector, vector_p,
        sizeof(radius_om_request_list_data[om_index].vector));

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestSecretKey
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the secret key of the specified request
 * INPUT:    request_id     - Request id
 * OUTPUT    secret_key_p   - Secret key
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestSecretKey(UI32_T request_id,
    char *secret_key_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == secret_key_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    strncpy(secret_key_p, radius_om_request_list_data[om_index].secret_key,
        sizeof(radius_om_request_list_data[om_index].secret_key) - 1);
    secret_key_p[
        sizeof(radius_om_request_list_data[om_index].secret_key) - 1] = '\0';

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestSecretKey
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the secret key of the specified request
 * INPUT:    request_id     - Request id
 *           secret_key_p   - Secret key
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestSecretKey(UI32_T request_id,
    char *secret_key_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == secret_key_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    strncpy(radius_om_request_list_data[om_index].secret_key, secret_key_p,
        sizeof(radius_om_request_list_data[om_index].secret_key) - 1);
    radius_om_request_list_data[om_index].secret_key[
        sizeof(radius_om_request_list_data[om_index].secret_key) - 1] = '\0';

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestServerArray
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the servers of the specified request.
 * INPUT:    request_i]ndex - Request index
 * OUTPUT:   server_array_p - Server array
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestServerArray(UI32_T request_index,
    RADIUS_OM_ServerArray_T *server_array_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == server_array_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(server_array_p, &radius_om_request_list_data[om_index].server_array,
        sizeof(*server_array_p));

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestServerArray
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the servers of the specified request.
 * INPUT:    request_i]ndex - Request index
 *           server_array_p - Server array
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestServerArray(UI32_T request_index,
    RADIUS_OM_ServerArray_T *server_array_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == server_array_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_index - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(&radius_om_request_list_data[om_index].server_array, server_array_p,
        sizeof(radius_om_request_list_data[om_index].server_array));

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestLastSentTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the last sent time of the specified request.
 * INPUT:    request_id         - Request id
 * OUTPUT:   last_sent_time_p   - Last sent time of the request
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestLastSentTime(UI32_T request_id,
    UI32_T *last_sent_time_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == last_sent_time_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *last_sent_time_p = radius_om_request_list_data[om_index].last_sent_time;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestLastSentTime
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the last sent time of the specified request.
 * INPUT:    request_id     - Request id
 *           last_sent_time - Last sent time of the request
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestLastSentTime(UI32_T request_id,
    UI32_T last_sent_time)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].last_sent_time = last_sent_time;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestPendingRequestCounterFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the flag of pending request counter of the specified
 *           request.
 * INPUT:    request_id - Request id
 * OUTPUT:   flag_p     - Flag
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestPendingRequestCounterFlag(
    UI32_T request_id, BOOL_T *flag_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == flag_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *flag_p =
        radius_om_request_list_data[om_index].is_pending_counter_increased;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestPendingRequestCounterFlag
 *---------------------------------------------------------------------------
 * PURPOSE:  Set the flag of pending request counter of the specified
 *           request.
 * INPUT:    request_id - Request id
 *           flag       - Flag
 * OUTPUT:   None
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestPendingRequestCounterFlag(
    UI32_T request_id, BOOL_T flag)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].is_pending_counter_increased =
        flag;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestResponsePacket
 *---------------------------------------------------------------------------
 * PURPOSE: Get the response packet of the specified request.
 * INPUT:   request_id              - Request id
 * OUTPUT:  response_packet_pp      - Pointer of response packet
 *          response_packet_len_p   - Length of response packet
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestResponsePacket(UI32_T request_id,
    UI8_T **response_packet_pp, UI32_T *response_packet_len_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == response_packet_pp)
        || (NULL == response_packet_len_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    *response_packet_pp =
        radius_om_request_list_data[om_index].response_packet_p;
    *response_packet_len_p =
        radius_om_request_list_data[om_index].response_packet_len;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResponsePacket
 *---------------------------------------------------------------------------
 * PURPOSE: Set the response packet of the specified request.
 * INPUT:   request_id          - Request id
 *          response_packet_p   - Pointer of response packet
 *          response_packet_len - Length of response packet
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResponsePacket(UI32_T request_id,
    UI8_T *response_packet_p, UI32_T response_packet_len)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == response_packet_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].response_packet_p =
        response_packet_p;
    radius_om_request_list_data[om_index].response_packet_len =
        response_packet_len;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestResultData
 *---------------------------------------------------------------------------
 * PURPOSE: Get the result data of the specified request.
 * INPUT:   request_id      - Request id
 * OUTPUT:  result_data_p   - Result data
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestResultData(UI32_T request_id,
    RADIUS_OM_RequestResult_T *result_data_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == result_data_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(result_data_p,
        &radius_om_request_list_data[om_index].result_data,
        sizeof(*result_data_p));
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResultData
 *---------------------------------------------------------------------------
 * PURPOSE: Set the result data of the specified request.
 * INPUT:   request_id      - Request id
 *          result_data_p   - Result data
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResultData(UI32_T request_id,
    RADIUS_OM_RequestResult_T *result_data_p)
{
    UI32_T om_index;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
        || (NULL == result_data_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    memcpy(&radius_om_request_list_data[om_index].result_data,
        result_data_p,
        sizeof(radius_om_request_list_data[om_index].result_data));
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetRequestResultServerIp
 *---------------------------------------------------------------------------
 * PURPOSE: Set the server IP address of the last request end of the process.
 * INPUT:   request_id  - Request id
 *          server_ip   - Server IP address
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetRequestResultServerIp(UI32_T request_id,
    UI32_T server_ip)
{
    UI32_T om_index;

    if (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_id))
    {
        return RADIUS_RETURN_FAIL;
    }

    om_index = request_id - 1;

    RADIUS_OM_LOCK();

    if (FALSE == radius_om_request_list_data[om_index].is_used)
    {
        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_FAIL;
    }

    radius_om_request_list_data[om_index].result_data.server_ip = server_ip;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRecentRequestTimeout
 *---------------------------------------------------------------------------
 * PURPOSE:  Get the recent request timeout (closest to current time)
 *           between all processing requests.
 * INPUT:    None
 * OUTPUT    timeout_p  - Recent request timeout
 * RETURN:   RADIUS_RETURN_SUCCESS - succeeded,
 *           RADIUS_RETURN_FAIL - failed
 * NOTE:     None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRecentRequestTimeout(UI32_T *timeout_p)
{
    if (NULL == timeout_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    if (NULL == radius_om_request_list.head_p)
    {
        *timeout_p = RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME;

        RADIUS_OM_UNLOCK();
        return RADIUS_RETURN_SUCCESS;
    }

    *timeout_p = radius_om_request_list.head_p->time;

    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAuthReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Set the socket ID in the authentication request identifier queue.
 * INPUT:   socket_id   - Socket ID.
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAuthReqIdQueueSocketId(int socket_id)
{
    RADIUS_OM_LOCK();
    radius_om_auth_req_id_queue.socket_id = socket_id;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAuthReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Get the socket ID in the authentication request identifier queue.
 * INPUT:   None
 * OUTPUT:  socket_id_p - Socket ID.
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAuthReqIdQueueSocketId(int *socket_id_p)
{
    if (NULL == socket_id_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    *socket_id_p = radius_om_auth_req_id_queue.socket_id;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequestIntoAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Add a new request into the authentication request identifier
 *          queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  id_p            - Identifier
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoAuthReqIdQueue(
    UI32_T request_index, UI32_T *id_p)
{
    RADIUS_ReturnValue_T ret;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == id_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_AddRequestIntoIdQueue(&radius_om_auth_req_id_queue,
        request_index, id_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequestFromAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from the authentication request identifier
 *          queue.
 * INPUT:   id  - Identifier
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromAuthReqIdQueue(UI32_T id)
{
    RADIUS_ReturnValue_T ret;

    if (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_DeleteRequestFromIdQueue(&radius_om_auth_req_id_queue,
        id);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestInAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether there is any request in authentication request
 *          identifier queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestInAuthReqIdQueue()
{
    return (radius_om_auth_req_id_queue.num_of_allocated > 0);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestByIdFromAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Get the belonging request index by identifier from
 *          authentication request ID queue.
 * INPUT:   id              - Identifier
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromAuthReqIdQueue(UI32_T id,
    UI32_T *request_index_p)
{
    RADIUS_ReturnValue_T ret;

    if (   (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
        || (NULL == request_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_GetRequestByIdFromIdQueue(&radius_om_auth_req_id_queue,
        id, request_index_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_SetAcctReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Set the socket ID in the accounting request identifier queue.
 * INPUT:   socket_id   - Socket ID.
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_SetAcctReqIdQueueSocketId(int socket_id)
{
    RADIUS_OM_LOCK();
    radius_om_acct_req_id_queue.socket_id = socket_id;
    RADIUS_OM_UNLOCK();

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetAcctReqIdQueueSocketId
 *---------------------------------------------------------------------------
 * PURPOSE: Get the socket ID in the accounting request identifier queue.
 * INPUT:   None
 * OUTPUT:  socket_id_p - Socket ID.
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetAcctReqIdQueueSocketId(int *socket_id_p)
{
    if (NULL == socket_id_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    *socket_id_p = radius_om_acct_req_id_queue.socket_id;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequestIntoAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Add a new request into the accounting request identifier
 *          queue.
 * INPUT:   request_index   - Request index
 * OUTPUT:  id_p            - Identifier
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoAcctReqIdQueue(
    UI32_T request_index, UI32_T *id_p)
{
    RADIUS_ReturnValue_T ret;

    if (   (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == id_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_AddRequestIntoIdQueue(&radius_om_acct_req_id_queue,
        request_index, id_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequestFromAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from the accounting request identifier
 *          queue.
 * INPUT:   id  - Identifier
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromAcctReqIdQueue(UI32_T id)
{
    RADIUS_ReturnValue_T ret;

    if (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_DeleteRequestFromIdQueue(&radius_om_acct_req_id_queue,
        id);

    RADIUS_OM_UNLOCK();

    return ret;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_HasAnyRequestInAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Check whether there is any request in accounting request
 *          identifier queue.
 * INPUT:   None
 * OUTPUT:  None
 * RETURN:  TRUE/FALSE
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
BOOL_T RADIUS_OM_HasAnyRequestInAcctReqIdQueue()
{
    return (radius_om_acct_req_id_queue.num_of_allocated > 0);
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestByIdFromAcctReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Get the belonging request index by identifier from
 *          accounting request ID queue.
 * INPUT:   id              - Identifier
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    None
 *---------------------------------------------------------------------------
 */
RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromAcctReqIdQueue(UI32_T id,
    UI32_T *request_index_p)
{
    RADIUS_ReturnValue_T ret;

    if (   (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
        || (NULL == request_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    RADIUS_OM_LOCK();

    ret = RADIUS_OM_GetRequestByIdFromIdQueue(&radius_om_acct_req_id_queue,
        id, request_index_p);

    RADIUS_OM_UNLOCK();

    return ret;
}

/* Timer linked-list related functions.
 */
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetFreeTimerLinkedListElement
 *---------------------------------------------------------------------------
 * PURPOSE: Return a free timer linked-list element from free list.
 * INPUT:   list_p          - Timer linked-list
 * OUTPUT:  freed_element_p - Freed element
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_GetFreeTimerLinkedListElement(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T **freed_element_pp)
{
    if (   (NULL == list_p)
        || (NULL == freed_element_pp)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (NULL == list_p->free_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    *freed_element_pp = list_p->free_p;

    /* Updaet free list
     */
    list_p->free_p = list_p->free_p->next_entry_p;
    if (NULL != list_p->free_p)
    {
        list_p->free_p->prev_entry_p = NULL;
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteTimerLinkedListElement
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the specified element from the timer linked-list. =
 * INPUT:   list_p      - Timer linked-list
 *          element_p   - The element to delete
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section. The deleted entry will not be
 *          linked back to free list and the caller need to take care it.
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_DeleteTimerLinkedListElement(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T *element_p)
{
    if (   (NULL == list_p)
        || (NULL == element_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (element_p == list_p->head_p)
    {
        if (NULL != element_p->next_entry_p)
        {
            list_p->head_p = element_p->next_entry_p;
            element_p->next_entry_p->prev_entry_p = NULL;
        }
        else
        {
            list_p->head_p = NULL;
        }
    }
    else if (element_p == list_p->tail_p)
    {
        list_p->tail_p = element_p->prev_entry_p;
        element_p->prev_entry_p->next_entry_p = NULL;
    }
    else
    {
        element_p->next_entry_p->prev_entry_p = element_p->prev_entry_p;
        element_p->prev_entry_p->next_entry_p = element_p->next_entry_p;
    }

    element_p->prev_entry_p = NULL;
    element_p->next_entry_p = NULL;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_ResortTimerLinkedListByTimeOrder
 *---------------------------------------------------------------------------
 * PURPOSE: Resort the timer linked-list by ascending time order.
 * INPUT:   list_p  - Timer linked-list
 *          index   - Entry to resort
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_ResortTimerLinkedListByTimeOrder(
    RADIUS_OM_TimerLinkedList_T *list_p, UI32_T index)
{
    RADIUS_OM_TimerLinkedListElement_T *element_p;

    if (   (NULL == list_p)
        || (index >= list_p->max_element_count)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    element_p = &list_p->elements_p[index];

    if (RADIUS_RETURN_FAIL == RADIUS_OM_DeleteTimerLinkedListElement(list_p,
        element_p))
    {
        return RADIUS_RETURN_FAIL;
    }

    if (RADIUS_RETURN_FAIL == RADIUS_OM_AddElementToTimerLinkedList(list_p,
        element_p))
    {
        return RADIUS_RETURN_FAIL;
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddElementToTimerLinkedList
 *---------------------------------------------------------------------------
 * PURPOSE: Add the new element into the timer linked list based on its time.
 * INPUT:   list_p          - Timer linked-list
 *          new_element_p   - New element
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_AddElementToTimerLinkedList(
    RADIUS_OM_TimerLinkedList_T *list_p,
    RADIUS_OM_TimerLinkedListElement_T *new_element_p)
{
    RADIUS_OM_TimerLinkedListElement_T *traverse_element_p;

    if (   (NULL == list_p)
        || (NULL == new_element_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (NULL == list_p->head_p)
    {
        list_p->head_p = list_p->tail_p = new_element_p;
        new_element_p->prev_entry_p = NULL;
        new_element_p->next_entry_p = NULL;
        return RADIUS_RETURN_SUCCESS;
    }

    if (RADIUS_OM_INVALID_TIMER_LINKED_LIST_TIME == new_element_p->time)
    {
        list_p->tail_p->next_entry_p = new_element_p;
        new_element_p->prev_entry_p = list_p->tail_p;
        new_element_p->next_entry_p = NULL;
        list_p->tail_p = new_element_p;
        return RADIUS_RETURN_SUCCESS;
    }

    traverse_element_p = list_p->head_p;
    while (TRUE)
    {
        if (traverse_element_p->time >= new_element_p->time)
        {
            new_element_p->next_entry_p = traverse_element_p;

            if (NULL != traverse_element_p->prev_entry_p)
            {
                new_element_p->prev_entry_p = traverse_element_p->prev_entry_p;
                traverse_element_p->prev_entry_p->next_entry_p = new_element_p;
            }
            else
            {
                list_p->head_p = new_element_p;
                new_element_p->prev_entry_p = NULL;
            }

            traverse_element_p->prev_entry_p = new_element_p;

            break;
        }
        else
        {
            if (NULL == traverse_element_p->next_entry_p)
            {
                traverse_element_p->next_entry_p = new_element_p;
                list_p->tail_p = new_element_p;
                new_element_p->prev_entry_p = traverse_element_p;
                new_element_p->next_entry_p = NULL;

                break;
            }
            else
            {
                traverse_element_p = traverse_element_p->next_entry_p;
            }
        }
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteElementFromTimerLinkedList
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the element from the timer linked list.
 * INPUT:   list_p  - Timer linked-list
 *          index   - Entry to delete
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_DeleteElementFromTimerLinkedList(
    RADIUS_OM_TimerLinkedList_T *list_p,
    UI32_T index)
{
    if (   (NULL == list_p)
        || (index >= list_p->max_element_count)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    /* Update the pointers of the head and tail in request list
     */
    if (&list_p->elements_p[index] == list_p->head_p)
    {
        list_p->head_p = list_p->head_p->next_entry_p;
    }
    if (&list_p->elements_p[index] == list_p->tail_p)
    {
        list_p->tail_p = list_p->tail_p->prev_entry_p;
    }

    /* Inter-connect between previous and next entry of the deleting entry
     */
    if (NULL != list_p->elements_p[index].prev_entry_p)
    {
        list_p->elements_p[index].prev_entry_p->next_entry_p = list_p->elements_p[index].next_entry_p;
    }
    if (NULL != list_p->elements_p[index].next_entry_p)
    {
        list_p->elements_p[index].next_entry_p->prev_entry_p = list_p->elements_p[index].prev_entry_p;
    }

    /* Link back to the head of the free list.
     */
    list_p->elements_p[index].next_entry_p = list_p->free_p;
    list_p->elements_p[index].prev_entry_p = NULL;
    if (NULL != list_p->free_p)
    {
        list_p->free_p->prev_entry_p = &list_p->elements_p[index];
    }
    list_p->free_p = &list_p->elements_p[index];

    return RADIUS_RETURN_SUCCESS;
}

/* ID queue related functions.
 */
/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_AddRequestIntoIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Add a new request into the identifier queue.
 * INPUT:   queue_p         - ID queue
 *          request_index   - Request index
 * OUTPUT:  id_p            - Identifier
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_AddRequestIntoIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T request_index, UI32_T *id_p)
{
    RADIUS_OM_IdQueueEntry_T *entry_p;

    if (   (NULL == queue_p)
        || (FALSE == RADIUS_OM_IS_REQUEST_ID_VALID(request_index))
        || (NULL == id_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (NULL == queue_p->free_head_p)
    {
        return RADIUS_RETURN_FAIL;
    }

    entry_p = queue_p->free_head_p;

    if (queue_p->free_head_p == queue_p->free_tail_p)
    {
        queue_p->free_tail_p = NULL;
    }

    queue_p->free_head_p = queue_p->free_head_p->next_entry_p;
    queue_p->num_of_allocated++;

    entry_p->is_allocated = TRUE;
    entry_p->request_index = request_index;
    entry_p->next_entry_p = NULL;

    *id_p = entry_p->id;

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_DeleteRequestFromAuthReqIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Delete the request from the authentication request identifier
 *          queue.
 * INPUT:   queue_p - ID queue
 *          id      - Identifier
 * OUTPUT:  None
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_DeleteRequestFromIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T id)
{
    RADIUS_OM_IdQueueEntry_T *entry_p;

    if (   (NULL == queue_p)
        || (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    entry_p = &queue_p->entries[id];

    if (FALSE == entry_p->is_allocated)
    {
        return RADIUS_RETURN_FAIL;
    }

    entry_p->is_allocated = FALSE;
    entry_p->next_entry_p = NULL;
    entry_p->request_index = 0;

    if (NULL != queue_p->free_tail_p)
    {
        queue_p->free_tail_p->next_entry_p = entry_p;
    }
    queue_p->free_tail_p = entry_p;
    queue_p->num_of_allocated--;

    if (NULL == queue_p->free_head_p)
    {
        queue_p->free_head_p = queue_p->free_tail_p;
    }

    return RADIUS_RETURN_SUCCESS;
}

/*---------------------------------------------------------------------------
 * ROUTINE NAME - RADIUS_OM_GetRequestByIdFromIdQueue
 *---------------------------------------------------------------------------
 * PURPOSE: Get the belonging request index by identifier from ID queue.
 * INPUT:   queue_p         - ID queue
 *          id              - Identifier
 * OUTPUT:  request_index_p - Request index
 * RETURN:  RADIUS_RETURN_SUCCESS - succeeded,
 *          RADIUS_RETURN_FAIL - failed
 * NOTE:    MUST be called in critical section
 *---------------------------------------------------------------------------
 */
static RADIUS_ReturnValue_T RADIUS_OM_GetRequestByIdFromIdQueue(
    RADIUS_OM_IdQueue_T *queue_p, UI32_T id, UI32_T *request_index_p)
{
    if (   (id >= RADIUS_OM_MAX_NBR_OF_IDENTIFIER)
        || (NULL == request_index_p)
        )
    {
        return RADIUS_RETURN_FAIL;
    }

    if (FALSE == queue_p->entries[id].is_allocated)
    {
        return RADIUS_RETURN_FAIL;
    }

    *request_index_p = queue_p->entries[id].request_index;

    return RADIUS_RETURN_SUCCESS;
}
