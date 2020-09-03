/* ------------------------------------------------------------------------
 * FILE NAME - BACKDOOR_MGR.C
 * ------------------------------------------------------------------------
 * ABSTRACT :
 * Purpose:
 * Note:
 * ------------------------------------------------------------------------
 *  History
 *
 *   Charles Cheng     02/21/2002      new created
 *   Wakka             05/09/2007      design change.
 *
 * ------------------------------------------------------------------------
 * Copyright(C)                             ACCTON Technology Corp. , 2002
 * ------------------------------------------------------------------------
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "sys_type.h"
#include "sys_module.h"
#include "sys_bld.h"
#include "sysfun.h"
#include "l_ipcio.h"
#include "l_string.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARARTIONS
 */
#define BACKDOOR_MGR_DEBUG                          FALSE
#define BACKDOOR_MGR_DEBUG_LOCAL_CS                 FALSE
#define BACKDOOR_MGR_DEBUG_SHARED_CS                FALSE

#define BACKDOOR_OPEN                               FALSE

/* options for callback management
 *
 * BACKDOOR_MGR_MAIN_MENU_CASE_INSENSITIVE
 *   specify the name of callback is case insensitive.
 *
 * BACKDOOR_MGR_MAIN_MENU_SORT
 *   to allow sort callback by name.
 *
 * BACKDOOR_MGR_MAIN_MENU_PARTIAL_MATCH
 *   to parse user keyin with partical match instead exact match.
 *
 * BACKDOOR_MGR_MAIN_MENU_BY_COLUMN
 *   list menu by column instead of by line.
 */
#define BACKDOOR_MGR_MAIN_MENU_CASE_INSENSITIVE     TRUE
#define BACKDOOR_MGR_MAIN_MENU_SORT                 TRUE
#define BACKDOOR_MGR_MAIN_MENU_PARTIAL_MATCH        TRUE
#define BACKDOOR_MGR_MAIN_MENU_BY_COLUMN            TRUE

/* internal command to enable/disable IPC IO
 */
#define BACKDOOR_MGR_RESERVED_WORD_FOR_IPC_IO  "!"

#define BACKDOOR_MGR_INVALID_MSGQ_HANDLE    0
#define BACKDOOR_MGR_INVALID_IDX            ((UI32_T)-1)

/* Backdoor session:
 * When a UI session enters backdoor, it will occupy
 * a backdoor session.
 *
 * User can specify custom IO service for the backdoor session
 * or use stdio.
 */
#define BACKDOOR_MGR_MAX_NBR_OF_CALLBACK        100
#define BACKDOOR_MGR_MAX_NBR_OF_SESSION         5
#define BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE BACKDOOR_MGR_MAX_NBR_OF_SESSION
#define BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE      (BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE + 1)

/* stdio IO service always use the last idx.
 */
#define BACKDOOR_MGR_STDIO_IO_SERVICE_ID        (BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE -1)

#define BACKDOOR_MGR_PARSING_KEYIN_EXIT 0
#define BACKDOOR_MGR_PARSING_SUCCESS    1
#define BACKDOOR_MGR_PARSING_FAIL       2

/* The timeout for backdoor main to wait for event
 */
#define BACKDOOR_MGR_MAIN_TIMEOUT_RUNNING           SYSFUN_TIMEOUT_NOWAIT
#define BACKDOOR_MGR_MAIN_TIMEOUT_INACTIVE          SYS_BLD_TICKS_PER_SECOND

/* TYPE DEFINITIONS
 */
/* The events for backdoor main
 */
typedef enum
{
    BACKDOOR_MGR_MAIN_EVENT_LEAVE_BACKDOOR      = BIT_16,
    BACKDOOR_MGR_MAIN_EVENT_STOP_IO_HANDLING    = BIT_17,
    BACKDOOR_MGR_MAIN_EVENT_START_IO_HANDLING   = BIT_18,
} BACKDOOR_MGR_MainEvent_T;

/* The status of session entries
 *
 * BACKDOOR_MGR_ST_INACTIVE this session is unused.
 * BACKDOOR_MGR_ST_INIT     this session is being initialized.
 * BACKDOOR_MGR_ST_RUNNING  this session is ready now,
 *                          backdoor main thread is running.
 *
 *                     BACKDOOR_MGR_ST_INACTIVE
 *                          /            ^
 *         session created /              \
 *                        v                \
 *          BACKDOOR_MGR_ST_INIT           | session deleted
 *                        \                /
 *    main entry presented \              /
 *                          v            /
 *                     BACKDOOR_MGR_ST_RUNNING
 */
typedef enum
{
    BACKDOOR_MGR_ST_INACTIVE,
    BACKDOOR_MGR_ST_INIT,
    BACKDOOR_MGR_ST_RUNNING,
} BACKDOOR_MGR_Status_T;

/*
 * BACKDOOR_MGR_PENDING_IO_ST_IDLE
 *     do nothing
 *
 * BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ
 *     allowed to handle L_IPCIO_CMD_GETCHAR/L_IPCIO_CMD_GETLINE
 *
 * BACKDOOR_MGR_PENDING_IO_ST_WAIT_KEY
 *     perfom keyin monitoring
 *
 *                     BACKDOOR_MGR_PENDING_IO_ST_IDLE
 *                          /            ^
 *       handle getkey req /              \
 *                        v                \
 * BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ     | CSC backdoor finish
 *                        \                /
 *  start keyin monitoring \              /
 *                          v            /
 *                     BACKDOOR_MGR_PENDING_IO_ST_WAIT_KEY
 */
typedef enum
{
    BACKDOOR_MGR_PENDING_IO_ST_IDLE,
    BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ,
    BACKDOOR_MGR_PENDING_IO_ST_WAIT_KEY,
} BACKDOOR_MGR_PendingIOState_T;

typedef struct
{
    UI32_T msgq_key;
    void   (*func)(void);
    char   csc_name[BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH + 1];
} BACKDOOR_MGR_CallBack_T;

typedef struct
{
    UI32_T msgq_key;
    UI32_T func_task_id;
    int (*func_getchar)(void *cookie);
    int (*func_print)(void *cookie, const char *s);
    BOOL_T *cookie;
    BOOL_T is_valid;
} BACKDOOR_MGR_IOService_T;

typedef struct
{
    BACKDOOR_MGR_PendingIOState_T pio_state;
    UI8_T msgbuf[SYSFUN_SIZE_OF_MSG(sizeof(L_IPCIO_Msg_T))];
    SYSFUN_Msg_T *pio_msg_p;
    L_IPCIO_Msg_T *pio_ipcio_msg_p;
    size_t pio_received_nchar;
    size_t pio_required_nchar;
} BACKDOOR_MGR_PendingIO_T;

typedef struct
{
    /* NOTE
     *   session info only valid when status is
     */
    BACKDOOR_MGR_Status_T status;

    /* for debug, force stdio.
     */
    BOOL_T force_stdio;

    /* these info. are set before session running,
     */
    BOOL_T forked;
    UI32_T io_service_idx;
    void (*func_leave)(void *cookie);
    void   *cookie;
    UI32_T task_id;     /* task id of backdoor main thread */

    /* to store pending_io info, will be allocated when session added.
     * it is in process scope and
     * only IO service handler and backdoor main can access.
     */
    BACKDOOR_MGR_PendingIO_T *pending_io_p;

    /* for track which backdoor is running now
     * only writed by IPC handler that will spawn backdoor thread.
     */
    UI32_T backdoor_callback_idx;

    /* When a CSC backdoor is being started, an IPC
     * msg will be sent to a mgr thread, and the mgr
     * thread will spawn a backdoor thread.
     * this task id is that backdoor thread described
     * above.
     * only writed by backdoor thread.
     */
    UI32_T backdoor_task_id;
} BACKDOOR_MGR_Session_T;

typedef struct
{
    /* for debug, force stdio for all session.
     */
    BOOL_T all_force_stdio;

    /* backdoor callback info.
     * no unregister func supoort now
     * that is, race condition need to take care when callback registering.
     */
    UI32_T nbr_of_callback;
    BACKDOOR_MGR_CallBack_T         callback[BACKDOOR_MGR_MAX_NBR_OF_CALLBACK];

    /* a IO service is an I/O interface used by backdoor threads
     */
    BACKDOOR_MGR_IOService_T  io_service[BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE];

    /* a session that is usually created by an UI, it contains
     *    one backdoor main thread and may invokes another
     *    backdoor sub thread
     */
    BACKDOOR_MGR_Session_T          session[BACKDOOR_MGR_MAX_NBR_OF_SESSION];
} BACKDOOR_MGR_Shmem_Data_T;

typedef struct
{
    UI32_T msgq_key;
    SYSFUN_MsgQ_T msgq_handle;
} BACKDOOR_MGR_IPC_Handle_T;

typedef struct
{
    /* to keep available IPC handles for IO service threads.
     */
    BACKDOOR_MGR_IPC_Handle_T ipc_handle;
} BACKDOOR_MGR_IOServiceContext_T;

/* MACRO DEFINITIONS
 */
/*-----------------------------------------------------------
 * MACRO NAME - BACKDOOR_MGR_CHECK_PROC_INIT_OK
 *-----------------------------------------------------------
 * FUNCTION: check if process resource is init ok
 * INPUT   : value to return if process resource is not init ok
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
#define BACKDOOR_MGR_CHECK_PROC_INIT_OK(...) do { \
            if (!bd_proc_init_ok) return __VA_ARGS__; \
        } while (0)

/*-----------------------------------------------------------
 * MACRO NAME - BACKDOOR_MGR_ENTER_CRITICAL_SECTION
 *              BACKDOOR_MGR_LEAVE_CRITICAL_SECTION
 *-----------------------------------------------------------
 * FUNCTION: To protect local data.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
#define BACKDOOR_MGR_ENTER_CRITICAL_SECTION() ( \
    BACKDOOR_MGR_DEBUG_MSG_LO_CS("BACKDOOR_MGR_ENTER_CRITICAL_SECTION: task_id: %lu", (unsigned long)SYSFUN_TaskIdSelf()), \
    SYSFUN_ENTER_CRITICAL_SECTION(bd_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER))
#define BACKDOOR_MGR_LEAVE_CRITICAL_SECTION() ( \
    BACKDOOR_MGR_DEBUG_MSG_LO_CS("BACKDOOR_MGR_LEAVE_CRITICAL_SECTION: task_id: %lu", (unsigned long)SYSFUN_TaskIdSelf()), \
    SYSFUN_LEAVE_CRITICAL_SECTION(bd_sem_id))

/*-----------------------------------------------------------
 * MACRO NAME - BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION
 *              BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION
 *-----------------------------------------------------------
 * FUNCTION: To protect data stored in shared memory.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : to avoid deadlock, never use these APIs
 *           in local critical section.
 *----------------------------------------------------------*/
#define BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION() ( \
    BACKDOOR_MGR_DEBUG_MSG_SH_CS("BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION: task_id: %lu", (unsigned long)SYSFUN_TaskIdSelf()), \
    SYSFUN_ENTER_CRITICAL_SECTION(bd_shared_sem_id, SYSFUN_TIMEOUT_WAIT_FOREVER))
#define BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION() ( \
    BACKDOOR_MGR_DEBUG_MSG_SH_CS("BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION: task_id: %lu", (unsigned long)SYSFUN_TaskIdSelf()), \
    SYSFUN_LEAVE_CRITICAL_SECTION(bd_shared_sem_id))


/*-----------------------------------------------------------
 * MACRO NAME - BACKDOOR_MGR_IPC_GetChar
 *              BACKDOOR_MGR_IPC_GetLine
 *              BACKDOOR_MGR_IPC_Print
 *              BACKDOOR_MGR_IPC_Printf
 *              BACKDOOR_MGR_IPC_VPrintf
 *-----------------------------------------------------------
 * FUNCTION: To perform standard I/O via IPC
 * INPUT   : ...
 * OUTPUT  : ...
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
#define BACKDOOR_MGR_IPC_GetChar(ipc, ch_p)         L_IPCIO_GetChar((ipc), &bd_ipcio_user_info, (ch_p))
#define BACKDOOR_MGR_IPC_GetLine(ipc, size, buf)    L_IPCIO_GetLine((ipc), &bd_ipcio_user_info, (size), (buf))
#define BACKDOOR_MGR_IPC_Print(ipc, str)            L_IPCIO_Print((ipc), &bd_ipcio_user_info, BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME, (str))
#define BACKDOOR_MGR_IPC_Printf(ipc, fmt, ...)      L_IPCIO_Printf((ipc), &bd_ipcio_user_info, BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME, (fmt), ##__VA_ARGS__)
#define BACKDOOR_MGR_IPC_VPrintf(ipc, fmt, ap)      L_IPCIO_VPrintf((ipc), &bd_ipcio_user_info, BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME, (fmt), (ap))


#define BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx) ( \
            (session_idx) < BACKDOOR_MGR_MAX_NBR_OF_SESSION)

#define BACKDOOR_MGR_SESSION_IS_USED(session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].status != BACKDOOR_MGR_ST_INACTIVE)

#define BACKDOOR_MGR_SESSION_IS_RUNNING(session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].status == BACKDOOR_MGR_ST_RUNNING)

#define BACKDOOR_MGR_SESSION_SET_STATUS(session_idx, new_status) ( \
            BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, status: %d -> %d", (long unsigned)(session_idx), bd_shmem_data_p->session[(session_idx)].status, (new_status)), \
            bd_shmem_data_p->session[(session_idx)].status = (new_status))

#define BACKDOOR_MGR_TASK_IS_BACKDOOR_MAIN(tid, session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].task_id == (tid))

#define BACKDOOR_MGR_TASK_IS_CSC_BACKDOOR(tid, session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].backdoor_task_id == (tid))

#define BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx) ( \
            (io_service_idx) < BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE)

#define BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx) ( \
            bd_shmem_data_p->io_service[(io_service_idx)].is_valid)

#define BACKDOOR_MGR_IO_SERVICE_IS_IPC(io_service_idx) ( \
            bd_shmem_data_p->io_service[(io_service_idx)].msgq_key != BACKDOOR_MGR_INVALID_MSGQ_KEY)

#define BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx) ( \
            io_service_idx == BACKDOOR_MGR_STDIO_IO_SERVICE_ID || \
            bd_shmem_data_p->io_service[(io_service_idx)].func_task_id == SYSFUN_TaskIdSelf())

#define BACKDOOR_MGR_IO_SERVICE_VALIDATE(io_service_idx) ( \
            BACKDOOR_MGR_DEBUG_MSG("io_service_idx: %lu, VALIDATE", (long unsigned)(io_service_idx)), \
            bd_shmem_data_p->io_service[(io_service_idx)].is_valid = TRUE)

#define BACKDOOR_MGR_IO_SERVICE_INVALIDATE(io_service_idx) ( \
            BACKDOOR_MGR_DEBUG_MSG("io_service_idx: %lu, INVALIDATE", (long unsigned)(io_service_idx)), \
            bd_shmem_data_p->io_service[(io_service_idx)].is_valid = FALSE)

#define BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx) ( \
            io_service_idx == BACKDOOR_MGR_STDIO_IO_SERVICE_ID ? \
                bd_stdio_io_service.func_print : \
                bd_shmem_data_p->io_service[(io_service_idx)].func_print)

#define BACKDOOR_MGR_IO_SERVICE_FUNC_GETCHAR(io_service_idx) ( \
            io_service_idx == BACKDOOR_MGR_STDIO_IO_SERVICE_ID ? \
                bd_stdio_io_service.func_getchar : \
                bd_shmem_data_p->io_service[(io_service_idx)].func_getchar)

#define BACKDOOR_MGR_PENDING_IO_IS_IDLE(session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].pending_io_p->pio_state == BACKDOOR_MGR_PENDING_IO_ST_IDLE)
#define BACKDOOR_MGR_PENDING_IO_IS_WAIT_REQ(session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].pending_io_p->pio_state == BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ)
#define BACKDOOR_MGR_PENDING_IO_IS_WAIT_KEY(session_idx) ( \
            bd_shmem_data_p->session[(session_idx)].pending_io_p->pio_state == BACKDOOR_MGR_PENDING_IO_ST_WAIT_KEY)
#define BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, new_state) ( \
            BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, state: %d -> %d", (long unsigned)(session_idx), bd_shmem_data_p->session[(session_idx)].pending_io_p->pio_state, (new_state)), \
            bd_shmem_data_p->session[(session_idx)].pending_io_p->pio_state = (new_state))



/* definitions for strcmp/strncmp
 */
#if BACKDOOR_MGR_MAIN_MENU_CASE_INSENSITIVE
#define BACKDOOR_MGR_Strcmp     strcasecmp
#define BACKDOOR_MGR_Strncmp    strncasecmp
#else
#define BACKDOOR_MGR_Strcmp     strcmp
#define BACKDOOR_MGR_Strncmp    strncmp
#endif

/* for debug/error msg output
 */
#define BACKDOOR_MGR_FATAL(fmt, ...) printf("\r\nFATAL ERROR: " fmt "\r\n", ##__VA_ARGS__)
#define BACKDOOR_MGR_ERROR(fmt, ...) (printf("\r\n" fmt, ##__VA_ARGS__), fflush(stdout))
#define _BACKDOOR_MGR_DEBUG_MSG(fmt, ...)   (printf("\r\n%s:%d: " fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__), fflush(stdout))
#if BACKDOOR_MGR_DEBUG
#define BACKDOOR_MGR_DEBUG_MSG(...)         _BACKDOOR_MGR_DEBUG_MSG(__VA_ARGS__)
#else
#define BACKDOOR_MGR_DEBUG_MSG(...)         (void)0
#endif
#if BACKDOOR_MGR_DEBUG_LOCAL_CS
#define BACKDOOR_MGR_DEBUG_MSG_LO_CS(...)   _BACKDOOR_MGR_DEBUG_MSG(__VA_ARGS__)
#else
#define BACKDOOR_MGR_DEBUG_MSG_LO_CS(...)   (void)0
#endif
#if BACKDOOR_MGR_DEBUG_SHARED_CS
#define BACKDOOR_MGR_DEBUG_MSG_SH_CS(...)   _BACKDOOR_MGR_DEBUG_MSG(__VA_ARGS__)
#else
#define BACKDOOR_MGR_DEBUG_MSG_SH_CS(...)   (void)0
#endif


/* LOCAL FUNCTIONS DECLARATIONS
 */
static BOOL_T BACKDOOR_MGR_InitateProcessResource(void);
static UI32_T BACKDOOR_MGR_AddSession(BOOL_T forked, BACKDOOR_MGR_IOService_T *io_service_p, void (*func_leave)(void *cookie), void *cookie);
static BOOL_T BACKDOOR_MGR_MainMenu(UI32_T session_idx);
static UI32_T BACKDOOR_MGR_ParsingKeyin(char *key_in_string, UI32_T *callback_idx_p);
static BOOL_T BACKDOOR_MGR_WaitForBackdoorFinish(UI32_T session_idx);
static BOOL_T BACKDOOR_MGR_CheckBackdoorAlive(UI32_T session_idx);
static BOOL_T BACKDOOR_MGR_HandlePendingIO(UI32_T session_idx, UI32_T *event_p);
static BOOL_T BACKDOOR_MGR_EnterDebugMode(UI32_T session_idx);
static BOOL_T BACKDOOR_MGR_LeaveDebugMode(UI32_T session_idx);
static BOOL_T BACKDOOR_MGR_LeaveBackdoor(UI32_T session_idx);
static BOOL_T BACKDOOR_MGR_InitMainThread(UI32_T session_idx);
static void BACKDOOR_MGR_MainEntry(void *session_idx_p);
static void BACKDOOR_MGR_BackdoorEntry(void *session_idx_p);
static BOOL_T BACKDOOR_MGR_HandleInvokeBackdoor(BACKDOOR_MGR_Msg_T *bd_msg_p);
static BOOL_T BACKDOOR_MGR_NotifyInvokeBackdoor(UI32_T session_idx, UI32_T callback_idx);
static BOOL_T BACKDOOR_MGR_HandleIPCIOMsg_GetKey(UI32_T msgq_key, SYSFUN_Msg_T *msg_p);
static BOOL_T BACKDOOR_MGR_HandleIPCIOMsg_Print(UI32_T msgq_key, SYSFUN_Msg_T *msg_p);
static void BACKDOOR_MGR_HandleException(UI32_T session_idx, BOOL_T dflt_del_self);
#if BACKDOOR_MGR_MAIN_MENU_SORT
static void BACKDOOR_MGR_SortRegisteredCallback(void);
#endif

/*-----------------------------------------------------------
 * Following functions never take care critical section
 *-----------------------------------------------------------*/
static UI32_T BACKDOOR_MGR_GetRunningSessionByTaskId(UI32_T task_id);
static UI32_T BACKDOOR_MGR_GetIOService(UI32_T session_idx);
static SYSFUN_MsgQ_T BACKDOOR_MGR_GetIOServiceMsgq(UI32_T io_service_idx);
static BOOL_T BACKDOOR_MGR_SendEventToMain(UI32_T session_idx, BACKDOOR_MGR_MainEvent_T event);
static int BACKDOOR_MGR_VPrintf_Local(
    int (*func_print)(void *cookie, const char *s),
    void *cookie,
    char *fmt_p, va_list arg_p);
static char *BACKDOOR_MGR_GetS_Local(
    int (*func_getchar)(void *cookie),
    int (*func_print)(void *cookie, const char *s),
    void *cookie,
    char *buf, int size);
static int BACKDOOR_MGR_StdioGetChar(void *cookie);
static int BACKDOOR_MGR_StdioPrint(void *cookie, const char *s);
#if BACKDOOR_MGR_MAIN_MENU_SORT
static int BACKDOOR_MGR_RegisteredCallbackCmp(BACKDOOR_MGR_CallBack_T *p1, BACKDOOR_MGR_CallBack_T *p2);
#endif

/* LOCAL VARIABLES DECLARATIONS
 */
/* to determine if process is initiated.
 */
BOOL_T bd_proc_init_ok = FALSE;

/* to control access right of data in shared memory
 */
static UI32_T bd_shared_sem_id;

/* data in shared memory
 */
static BACKDOOR_MGR_Shmem_Data_T *bd_shmem_data_p = NULL;

/* to control access right of private data and msg buffer
 */
static UI32_T bd_sem_id;

/* this buffer is used for send msg, needs to be in critical section.
 */
static UI8_T bd_msg_buffer[SYSFUN_SIZE_OF_MSG(sizeof(BACKDOOR_MGR_Msg_T))];

/* data used by user IO service.
 */
static BACKDOOR_MGR_IOServiceContext_T bd_io_service_ctx[BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE];

/* IO service definition for stdio
 */
static BACKDOOR_MGR_IOService_T bd_stdio_io_service =
{
    .msgq_key = BACKDOOR_MGR_INVALID_MSGQ_KEY,
    .func_getchar = BACKDOOR_MGR_StdioGetChar,
    .func_print = BACKDOOR_MGR_StdioPrint,
    .cookie = NULL,
    .is_valid = TRUE,
};

static const L_IPCIO_UserInfo_T bd_ipcio_user_info = {
    .user_id = SYS_MODULE_BACKDOOR,
};

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_InitiateSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Initiate system resource for BACKDOOR_MGR
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
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_InitiateSystemResources(void)
{
    int i;

    bd_shmem_data_p = (BACKDOOR_MGR_Shmem_Data_T *) SYSRSC_MGR_GetShMem(SYSRSC_MGR_BACKDOOR_MGR_SHMEM_SEGID);

    bd_shmem_data_p->all_force_stdio = FALSE;

    bd_shmem_data_p->nbr_of_callback = 0;

    for (i = 0; i < BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE; i++)
    {
        BACKDOOR_MGR_IO_SERVICE_INVALIDATE(i);
    }
    bd_shmem_data_p->io_service[BACKDOOR_MGR_STDIO_IO_SERVICE_ID] = bd_stdio_io_service;

    for (i = 0; i < BACKDOOR_MGR_MAX_NBR_OF_SESSION; i++)
    {
        BACKDOOR_MGR_SESSION_SET_STATUS(i, BACKDOOR_MGR_ST_INACTIVE);
    }
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_AttachSystemResources
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Attach system resource for BACKDOOR_MGR in the context of the
 *      calling process.
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
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_AttachSystemResources(void)
{
    bd_shmem_data_p = (BACKDOOR_MGR_Shmem_Data_T*) SYSRSC_MGR_GetShMem(SYSRSC_MGR_BACKDOOR_MGR_SHMEM_SEGID);

    if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_BACKDOOR, &bd_shared_sem_id) != SYSFUN_OK)
    {
        BACKDOOR_MGR_ERROR("%s(): SYSFUN_GetSem fails.", __FUNCTION__);
        return;
    }

    BACKDOOR_MGR_InitateProcessResource();
}

/*-------------------------------------------------------------------------
 * FUNCTION NAME : BACKDOOR_MGR_GetShMemInfo
 *-------------------------------------------------------------------------
 * PURPOSE:
 *      Provide shared memory information of BACKDOOR_MGR for SYSRSC.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      segid_p  --  shared memory segment id
 *      seglen_p --  length of the shared memroy segment
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      private om API
 *-------------------------------------------------------------------------
 */
void BACKDOOR_MGR_GetShMemInfo(SYSRSC_MGR_SEGID_T *segid_p, UI32_T *seglen_p)
{
    *segid_p = SYSRSC_MGR_BACKDOOR_MGR_SHMEM_SEGID;
    *seglen_p = sizeof(BACKDOOR_MGR_Shmem_Data_T);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack
 *-----------------------------------------------------------------
 * FUNCTION: To register which function wish to appear in backdoor
 *           of CLI.
 * INPUT   : csc_name   - The name of the csc (computer software compoment)
 *                        module that wish to hook to backdoor, and this
 *                        name will be appeared in the main menu of the
 *                        backdoor. The string length of the name should
 *                        <= BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH
 *                        and >= BACKDOOR_MGR_MIN_CSC_NAME_STRING_LENTH.
 *                        If the length > BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH
 *                        The name will be truncated.
 *                        If the length < BACKDOOR_MGR_MIN_CSC_NAME_STRING_LENTH
 *                        registering will fail.
 *                        The name should not be BACKDOOR_MGR_RESERVED_WORD because
 *                        this is reserved word and registering will fail.
 *           msgq_key   - The message queue that waits for backdoor message.
 *           func       - The functional pointer of the function
 *                        that wish to register.
 *
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------------*/
void BACKDOOR_MGR_Register_SubsysBackdoorFunc_CallBack(char *csc_name,
                                                       UI32_T msgq_key,
                                                       void (*func)(void))
{
    BACKDOOR_MGR_CallBack_T *callback_p;
    BOOL_T registered = FALSE;
    int i;

    BACKDOOR_MGR_CHECK_PROC_INIT_OK();

    if (func == NULL || csc_name == NULL || *csc_name == 0)
    {
        return;
    }

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    /* check if registered.
     */
    for (i = 0; i < bd_shmem_data_p->nbr_of_callback; i++)
    {
        if (BACKDOOR_MGR_Strncmp(csc_name, bd_shmem_data_p->callback[i].csc_name, BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH+1) == 0)
        {
            BACKDOOR_MGR_ERROR("%s(): backdoor had registered: %s", __FUNCTION__, csc_name);
            registered = TRUE;
            break;
        }
    }

    if (!registered &&
        bd_shmem_data_p->nbr_of_callback < BACKDOOR_MGR_MAX_NBR_OF_CALLBACK)
    {
        callback_p = &bd_shmem_data_p->callback[bd_shmem_data_p->nbr_of_callback];
        callback_p->msgq_key = msgq_key;
        callback_p->func = func;
        strncpy(callback_p->csc_name, csc_name, BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH);
        callback_p->csc_name[BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH] = 0;
        bd_shmem_data_p->nbr_of_callback++;
        registered = TRUE;

        BACKDOOR_MGR_DEBUG_MSG("register backdoor: %s", callback_p->csc_name);
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    if (!registered)
    {
        BACKDOOR_MGR_ERROR("%s(): no more space for backdoor registration.", __FUNCTION__);
    }
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Main
 *-----------------------------------------------------------------
 * FUNCTION: This function is to enter debug mode.
 * INPUT   : forked       - TRUE - specify a thread will be spawned
 *                          to run debug mode.
 *                          FALSE - debug mode will run in caller's context.
 *           msgq_key     - The msgq key to handle I/O.
 *                          specify BACKDOOR_MGR_INVALID_MSGQ_KEY
 *                          to use stdin/stdout instead of IPC.
 *           func_getchar - The callback func to get char.
 *                          similar to getchar().
 *                          return negative number on failure.
 *           func_print   - The callback func to print string.
 *                          similar to printf("%s", s).
 *                          return negative number on failure.
 *           func_leave   - if a callback func is specified,
 *                          specified func will be called when debug
 *                          mode finish.
 *           cookie       - cookie for func_getchar/func_print/func_leave
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Main(
    BOOL_T forked,
    UI32_T msgq_key,
    int (*func_getchar)(void *cookie),
    int (*func_print)(void *cookie, const char *s),
    void (*func_leave)(void *cookie),
    void *cookie)
{
    BACKDOOR_MGR_IOService_T io_service;
    UI32_T session_idx;
    UI32_T ret = FALSE;

    BACKDOOR_MGR_CHECK_PROC_INIT_OK(FALSE);

    if (msgq_key == BACKDOOR_MGR_INVALID_MSGQ_KEY)
    {
        io_service = bd_shmem_data_p->io_service[BACKDOOR_MGR_STDIO_IO_SERVICE_ID];
    }
    else
    {
        memset(&io_service, 0, sizeof(io_service));
        io_service.msgq_key = msgq_key;
        io_service.func_task_id = SYSFUN_TaskIdSelf();
        io_service.func_getchar = func_getchar;
        io_service.func_print = func_print;
        io_service.cookie = cookie;
    }

    session_idx = BACKDOOR_MGR_AddSession(forked, &io_service, func_leave, cookie);

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, forked: %u, msgq_key: %lu, func_getchar: %p, func_print: %p, func_leave: %p, cookie: %p", (unsigned long)session_idx, forked, (unsigned long)msgq_key, func_getchar, func_print, func_leave, cookie);

    if (BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx))
    {
        ret = BACKDOOR_MGR_EnterDebugMode(session_idx);
    }

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, ret(bool): %lu", (unsigned long)session_idx, (unsigned long)ret);
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCMsg
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle backdoor messages.
 * INPUT   : msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : None.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_HandleIPCMsg(SYSFUN_Msg_T *msg_p)
{
    BACKDOOR_MGR_Msg_T *bd_msg_p = (BACKDOOR_MGR_Msg_T *)msg_p->msg_buf;
    BOOL_T ret = FALSE;

    BACKDOOR_MGR_CHECK_PROC_INIT_OK(FALSE);

    switch (bd_msg_p->backdoor_cmd)
    {
        case BACKDOOR_MGR_CMD_INVOKE_BACKDOOR:
            BACKDOOR_MGR_HandleInvokeBackdoor(bd_msg_p);
            break;

        default:
            printf("\r\n%s: invalid command.\r\n", __FUNCTION__);
            ret = FALSE;
    }
    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCIOMsg
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle L_IPCIO messages.
 * INPUT   : msgq_key - the key of msgq that received the message.
 *           msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : should only be call by IO service thread.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_HandleIPCIOMsg(UI32_T msgq_key, SYSFUN_Msg_T *msg_p)
{
    L_IPCIO_Msg_T *l_ipcio_msg_p = (L_IPCIO_Msg_T *)msg_p->msg_buf;
    BOOL_T need_resp = FALSE;

    BACKDOOR_MGR_CHECK_PROC_INIT_OK(FALSE);

    if (msg_p == NULL)
        return FALSE;

    BACKDOOR_MGR_DEBUG_MSG("msgq_key: %lu, ipcio_cmd: %d", (unsigned long)msgq_key, (int)l_ipcio_msg_p->ipcio_cmd);

    switch (l_ipcio_msg_p->ipcio_cmd)
    {
        case L_IPCIO_CMD_GETCHAR:
        case L_IPCIO_CMD_GETLINE:
            need_resp = BACKDOOR_MGR_HandleIPCIOMsg_GetKey(msgq_key, msg_p);
            break;

        case L_IPCIO_CMD_PRINT:
            need_resp = BACKDOOR_MGR_HandleIPCIOMsg_Print(msgq_key, msg_p);
            break;

        default:
            need_resp = L_IPCIO_DummyHandleIPCIOMsg(msg_p);
    }

    BACKDOOR_MGR_DEBUG_MSG("msgq_key: %lu, ipcio_cmd: %d, need_resp: %u", (unsigned long)msgq_key, (int)l_ipcio_msg_p->ipcio_cmd, need_resp);

    return need_resp;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetChar
 *-----------------------------------------------------------------
 * FUNCTION: To get a character from UI.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : a character or EOF
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
int BACKDOOR_MGR_GetChar(void)
{
    UI32_T session_idx;
    UI32_T io_service_idx;
    SYSFUN_MsgQ_T msgq_handle = BACKDOOR_MGR_INVALID_MSGQ_HANDLE;
    UI32_T task_id = SYSFUN_TaskIdSelf();
    int ch = EOF;
    int (*func_getchar)(void *cookie) = NULL;
    void *cookie = NULL;
#if (SYS_CPNT_DRVP_DBG == TRUE) /* support driver_proc backdoor (lan, dev_nicdrv, dev_nicdrv_gateway) in linux shell without using Simba/CLI backdoor */
    char buf[16];
    static int pre_get='\n';
    if (strcmp("DRVP_DBG", SYSFUN_GetTaskName(0, buf, 16)) == 0)
    {
        if (pre_get == '\n')
        {
        printf(":");
        ch = getchar();
        }
        else
        {
            ch = pre_get;
        }
        pre_get = getchar(); /* to take away \n */
        return ch;
    }
#endif
    BACKDOOR_MGR_CHECK_PROC_INIT_OK(BACKDOOR_MGR_StdioGetChar(NULL));

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(task_id);

    /* getkey is only allowed when session is running.
     */
    if (BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx))
    {
        io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

        if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
        {
            /* if this task is backdoor main, use direct call.
             */
            if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_getchar = BACKDOOR_MGR_IO_SERVICE_FUNC_GETCHAR(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
            }
        }
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu, session_idx: %lu, msgq_handle: %p, func_getchar: %p, cookie: %p", (unsigned long)task_id, (unsigned long)session_idx, msgq_handle, func_getchar, cookie);

    if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
    {
        if (BACKDOOR_MGR_IPC_GetChar(msgq_handle, &ch) != L_IPCIO_ERR_NONE)
        {
            ch = EOF;
        }
    }

    if (func_getchar)
    {
        ch = func_getchar(cookie);
    }

    if (ch == EOF)
    {
        BACKDOOR_MGR_FATAL("EXCEPTION: (GetChar) task_id: %lu, session_idx: %lu", (unsigned long)task_id, (unsigned long)session_idx);
        BACKDOOR_MGR_HandleException(session_idx, TRUE);
    }

    return ch;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_RequestKeyIn
 *-----------------------------------------------------------------
 * FUNCTION: To get the string until enter is pressed from UI.
 * INPUT   : key_in_string - The pointer to the buffer to store
 *                           the key in string.
 *                           The size of buffer must reserve additional
 *                           space for '\0' (null terminate).
 *           max_key_len   - Maxium character user can key in.
 * OUTPUT  : key_in_string - Got string.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : this function is only allowed to be used in backdoor threads.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_RequestKeyIn(void *key_in_string, UI32_T max_key_len)
{
    UI32_T session_idx;
    UI32_T io_service_idx;
    SYSFUN_MsgQ_T msgq_handle = BACKDOOR_MGR_INVALID_MSGQ_HANDLE;
    UI32_T task_id = SYSFUN_TaskIdSelf();
    BOOL_T ret = FALSE;
    int (*func_getchar)(void *cookie) = NULL;
    int (*func_print)(void *cookie, const char *s) = NULL;
    void *cookie = NULL;

#if (SYS_CPNT_DRVP_DBG == TRUE) /* support driver_proc backdoor (lan, dev_nicdrv, dev_nicdrv_gateway) in linux shell without using Simba/CLI backdoor */
    char buf[16], *output_str;
    int  i;
    if (strcmp("DRVP_DBG", SYSFUN_GetTaskName(0, buf, 16)) == 0)
    {
        printf(":");
        output_str = (char *)key_in_string;
        for (i=0; i< max_key_len; i++)
        {
            output_str[i] = getchar();
            printf ("%c",output_str[i]);
            if (output_str[i]=='\r' || output_str[i] == '\n')
            {
                break;
            }
        }
        output_str[i]=0; 
        return TRUE;
    }
#endif
    BACKDOOR_MGR_CHECK_PROC_INIT_OK(
        BACKDOOR_MGR_GetS_Local(
            BACKDOOR_MGR_StdioGetChar,
            BACKDOOR_MGR_StdioPrint,
            NULL, key_in_string, max_key_len+1) != NULL);

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(task_id);

    /* getkey is only allowed when session is running.
     */
    if (BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx))
    {
        io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

        if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
        {
            /* if this task is backdoor main, use direct call.
             */
            if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_getchar = BACKDOOR_MGR_IO_SERVICE_FUNC_GETCHAR(io_service_idx);
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
            }
        }
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu, session_idx: %lu, msgq_handle: %p, func_getchar: %p, func_print: %p, cookie: %p", (unsigned long)task_id, (unsigned long)session_idx, msgq_handle, func_getchar, func_print, cookie);

    if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
    {
        ret = (BACKDOOR_MGR_IPC_GetLine(msgq_handle, max_key_len+1, key_in_string) == L_IPCIO_ERR_NONE);
    }

    if (func_getchar && func_print)
    {
        ret = (BACKDOOR_MGR_GetS_Local(func_getchar, func_print, cookie, key_in_string, max_key_len+1) != NULL);
    }

    if (!ret)
    {
        BACKDOOR_MGR_FATAL("EXCEPTION: (ReqKey) task_id: %lu, session_idx: %lu", (unsigned long)task_id, (unsigned long)session_idx);
        BACKDOOR_MGR_HandleException(session_idx, TRUE);
    }

    return ret;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Print
 *-----------------------------------------------------------------
 * FUNCTION: To print a string.
 * INPUT   : str - The string to print.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : it may spend at most BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME
 *           on IPC.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Print(char *str)
{
    UI32_T session_idx;
    UI32_T io_service_idx;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T task_id = SYSFUN_TaskIdSelf();
    UI32_T sent_count = 0;
    BOOL_T multi_send;
    int i;
    int (*func_print)(void *cookie, const char *s) = NULL;
    void *cookie = NULL;
#if (SYS_CPNT_DRVP_DBG == TRUE) /* support driver_proc backdoor (lan, dev_nicdrv, dev_nicdrv_gateway) in linux shell without using Simba/CLI backdoor */    
    char buf[16];
    if (strcmp("DRVP_DBG", SYSFUN_GetTaskName(0, buf, 16)) == 0)
    {
        printf(":");
        printf("%s", str);
        return TRUE;
    }
#endif
    BACKDOOR_MGR_CHECK_PROC_INIT_OK(BACKDOOR_MGR_StdioPrint(NULL, str) >= 0);

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(task_id);

    multi_send = !BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx);

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu, session_idx: %lu, multi_send: %u", (unsigned long)task_id, (unsigned long)session_idx, multi_send);

    if (!multi_send)
    {
        BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

        io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

        if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
        {
            /* if this task is backdoor main, use direct call.
             */
            if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
                {
                    sent_count += (BACKDOOR_MGR_IPC_Print(msgq_handle, str) == L_IPCIO_ERR_NONE);
                }

                BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
            }
        }

        BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

        if (func_print)
        {
            sent_count += func_print(cookie, str) >= 0;
        }
    }
    else
    {
        UI32_T sent_msgq_key[BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE];
        UI32_T sent_msgq_key_count = 0;

        for (io_service_idx = 0; io_service_idx < BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE; io_service_idx++)
        {
            func_print = NULL;

            BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

            if (!BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
            {
                /* do nothing */;
            }
            else if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                for (i = 0; i < sent_msgq_key_count; i++)
                    if (bd_shmem_data_p->io_service[io_service_idx].msgq_key == sent_msgq_key[i])
                        break;
                if (i == sent_msgq_key_count)
                {
                    sent_msgq_key[sent_msgq_key_count++] = bd_shmem_data_p->io_service[io_service_idx].msgq_key;

                    BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                    msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                    if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
                    {
                        sent_count += (BACKDOOR_MGR_IPC_Print(msgq_handle, str) == L_IPCIO_ERR_NONE);
                    }

                    BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
                }
            }

            BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

            if (func_print)
            {
                sent_count += func_print(cookie, str) >= 0;
            }
        }

        if (sent_count == 0)
        {
            io_service_idx = BACKDOOR_MGR_STDIO_IO_SERVICE_ID;

            func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
            cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;

            sent_count += func_print(cookie, str) >= 0;
        }
    }

    if (sent_count == 0)
    {
        BACKDOOR_MGR_FATAL("EXCEPTION: (Print) task_id: %lu, session_idx: %lu", (unsigned long)task_id, (unsigned long)session_idx);
        BACKDOOR_MGR_HandleException(session_idx, FALSE);
    }

    return (sent_count > 0);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_Printf
 *-----------------------------------------------------------------
 * FUNCTION: To print a formatted string.
 * INPUT   : fmt_p - The format string.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed. (due to invalid parameters/out of memory/IPC operation failure)
 * NOTE    : it may spend at most BACKDOOR_MGR_PRINTF_DFLT_WAIT_TIME
 *           on IPC.
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_Printf(char *fmt_p, ...)
{
    UI32_T session_idx;
    UI32_T io_service_idx;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T task_id = SYSFUN_TaskIdSelf();
    UI32_T sent_count = 0;
    BOOL_T multi_send;
    va_list arg_p, tmp_arg_p;
    int i;
    int (*func_print)(void *cookie, const char *s) = NULL;
    void *cookie = NULL;

    BACKDOOR_MGR_CHECK_PROC_INIT_OK(
        va_start(arg_p, fmt_p),
        (sent_count += BACKDOOR_MGR_VPrintf_Local(BACKDOOR_MGR_StdioPrint, NULL, fmt_p, arg_p) >= 0),
        va_end(arg_p),
        sent_count > 0);

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(task_id);

    multi_send = !BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx);

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu, session_idx: %lu, multi_send: %u", (unsigned long)task_id, (unsigned long)session_idx, multi_send);

    va_start(arg_p, fmt_p);

    if (!multi_send)
    {
        BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

        io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

        if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
        {
            /* if this task is backdoor main, use direct call.
             */
            if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
                {
                    va_copy(tmp_arg_p, arg_p);
                    sent_count += (BACKDOOR_MGR_IPC_VPrintf(msgq_handle, fmt_p, tmp_arg_p) == L_IPCIO_ERR_NONE);
                    va_end(tmp_arg_p);
                }

                BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
            }
        }

        BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

        if (func_print)
        {
            sent_count += BACKDOOR_MGR_VPrintf_Local(func_print, cookie, fmt_p, arg_p) >= 0;
        }
    }
    else
    {
        UI32_T sent_msgq_key[BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE];
        UI32_T sent_msgq_key_count = 0;

        for (io_service_idx = 0; io_service_idx < BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE; io_service_idx++)
        {
            func_print = NULL;

            BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

            if (!BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
            {
                /* do nothing */;
            }
            else if (BACKDOOR_MGR_IO_SERVICE_IS_FUNC(io_service_idx))
            {
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }
            else
            {
                for (i = 0; i < sent_msgq_key_count; i++)
                    if (bd_shmem_data_p->io_service[io_service_idx].msgq_key == sent_msgq_key[i])
                        break;
                if (i == sent_msgq_key_count)
                {
                    sent_msgq_key[sent_msgq_key_count++] = bd_shmem_data_p->io_service[io_service_idx].msgq_key;

                    BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

                    msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

                    if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
                    {
                        va_copy(tmp_arg_p, arg_p);
                        sent_count += (BACKDOOR_MGR_IPC_VPrintf(msgq_handle, fmt_p, tmp_arg_p) == L_IPCIO_ERR_NONE);
                        va_end(tmp_arg_p);
                    }

                    BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
                }
            }

            BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

            if (func_print)
            {
                va_copy(tmp_arg_p, arg_p);
                sent_count += BACKDOOR_MGR_VPrintf_Local(func_print, cookie, fmt_p, tmp_arg_p) >= 0;
                va_end(tmp_arg_p);
            }
        }

        if (sent_count == 0)
        {
            io_service_idx = BACKDOOR_MGR_STDIO_IO_SERVICE_ID;

            func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
            cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;

            sent_count += BACKDOOR_MGR_VPrintf_Local(func_print, cookie, fmt_p, arg_p) >= 0;
        }
    }

    va_end(arg_p);

    if (sent_count == 0)
    {
        BACKDOOR_MGR_FATAL("EXCEPTION: (Printf) task_id: %lu, session_idx: %lu", (unsigned long)task_id, (unsigned long)session_idx);
        BACKDOOR_MGR_HandleException(session_idx, FALSE);
    }

    return (sent_count > 0);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_DumpHex
 *-----------------------------------------------------------------
 * FUNCTION: Dump heximal code.
 * INPUT   : title   -- text displays on the output.
 *           len     -- data length to be displayed.
 *           buffer  -- buffer holding displaying data.
 * OUTPUT  : None.
 * RETURN  : see BACKDOOR_MGR_Print/BACKDOOR_MGR_Printf
 * NOTE    : see BACKDOOR_MGR_Print/BACKDOOR_MGR_Printf
 *----------------------------------------------------------------*/
BOOL_T BACKDOOR_MGR_DumpHex(char *title, int len, void *buf)
{
/*
DISPLAY FORMAT

0         1         2         3         4         5         6
0123456789012345678901234567890123456789012345678901234567890123456789

TITLE
0000 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ................
0010 10 11 12 13                                      ....
 */
#define ADDR_STR_LEN            4
#define HEX_STR_OFFSET          5
#define PRINTABLE_CHAR_OFFSET   54
#define CR_LF_OFFSET            70
#define OUTPUT_BUF_LEN          73

#define HEX_TO_ACSII(hex) ((tmp = (hex)), ((tmp > 9) ? (tmp + 0x37) : (tmp + '0')))

    int i, olen;
    char obuf[OUTPUT_BUF_LEN], tmp;
    unsigned char *p = buf;

    if (title != NULL)
        BACKDOOR_MGR_Printf("%s\r\n", title);

    if (len > (0x1 << (ADDR_STR_LEN << 2)))
        len = (0x1 << (ADDR_STR_LEN << 2));

    for (p = buf, i = 0; i < len;)
    {
        /* fill addr str
         */
        for (olen = 0; olen < ADDR_STR_LEN; olen++)
        {
            obuf[olen] = HEX_TO_ACSII((i >> ((ADDR_STR_LEN - olen - 1) << 2)) & 0xf);
        }

        /* reset hex str
         */
        memset(obuf + ADDR_STR_LEN, ' ', sizeof(obuf) - ADDR_STR_LEN);
        olen = HEX_STR_OFFSET;

        /* fill hex str
         */
        do
        {
            obuf[olen++] = HEX_TO_ACSII(*p >> 4);
            obuf[olen++] = HEX_TO_ACSII(*p & 0xf);
            olen++;

            obuf[PRINTABLE_CHAR_OFFSET + (i & 0xf)] = isprint(*p) ? *p : '.';

            i++;
            p++;
        }
        while ((i & 0xf) && i < len);

        /* print out
         */
        obuf[CR_LF_OFFSET] = '\r';
        obuf[CR_LF_OFFSET + 1] = '\n';
        obuf[CR_LF_OFFSET + 2] = 0;
        BACKDOOR_MGR_Print(obuf);
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_InitateProcessResource
 *-----------------------------------------------------------
 * FUNCTION: To initiate resource used in this process.
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : TRUE   - Successful
 *           FALSE  - Failed
 * NOTE    : None.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_InitateProcessResource(void)
{
    int i;

    if (SYSFUN_CreateSem(SYSFUN_MSGQKEY_PRIVATE, 1, SYSFUN_SEM_FIFO, &bd_sem_id) != SYSFUN_OK)
    {
        BACKDOOR_MGR_ERROR("%s(): SYSFUN_CreateSem fails.", __FUNCTION__);
        return FALSE;
    }

    for (i = 0; i < BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE; i++)
    {
        bd_io_service_ctx[i].ipc_handle.msgq_key = BACKDOOR_MGR_INVALID_MSGQ_KEY;
        bd_io_service_ctx[i].ipc_handle.msgq_handle = BACKDOOR_MGR_INVALID_MSGQ_HANDLE;
    }

    bd_proc_init_ok = TRUE;

    return TRUE;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_AddSession
 *-----------------------------------------------------------
 * FUNCTION: This function is to add session entry.
 * INPUT   : msgq_key
 *           func_leave
 *           cookie
 * OUTPUT  : None.
 * RETURN  : the session id or BACKDOOR_MGR_INVALID_IDX
 *           if no session entry is available.
 * NOTE    : None.
 *----------------------------------------------------------*/
static UI32_T BACKDOOR_MGR_AddSession(BOOL_T forked, BACKDOOR_MGR_IOService_T *io_service_p, void (*func_leave)(void *cookie), void *cookie)
{
    BACKDOOR_MGR_PendingIO_T *pending_io_p;
    UI32_T unused_io_service_idx = BACKDOOR_MGR_INVALID_IDX;
    UI32_T io_service_idx = BACKDOOR_MGR_INVALID_IDX;
    UI32_T session_idx = BACKDOOR_MGR_INVALID_IDX;

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    /* find a io_service entry to store msgq info.
     */
    /* check if io_service is existed.
     */
    for (io_service_idx = 0; io_service_idx < BACKDOOR_MGR_MAX_NBR_OF_IO_SERVICE; io_service_idx++)
    {
        if (!BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
        {
            if (!BACKDOOR_MGR_SESSION_IDX_IS_VALID(unused_io_service_idx))
                unused_io_service_idx = io_service_idx;
            continue;
        }

        if (bd_shmem_data_p->io_service[io_service_idx].msgq_key == io_service_p->msgq_key &&
            bd_shmem_data_p->io_service[io_service_idx].func_task_id == io_service_p->func_task_id &&
            bd_shmem_data_p->io_service[io_service_idx].func_getchar == io_service_p->func_getchar &&
            bd_shmem_data_p->io_service[io_service_idx].func_print == io_service_p->func_print &&
            bd_shmem_data_p->io_service[io_service_idx].cookie == io_service_p->cookie)
        {
            BACKDOOR_MGR_DEBUG_MSG("io_service_idx: %lu", (unsigned long)io_service_idx);
            break;
        }
    }

    /* if not existed, pick a unused entry.
     */
    if (!BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
    {
        BACKDOOR_MGR_DEBUG_MSG("unused_io_service_idx: %lu", (unsigned long)unused_io_service_idx);
        io_service_idx = unused_io_service_idx;
    }

    /* find a unused session entry and then fill info.
     */
    if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
    {
        for (session_idx = 0; session_idx < BACKDOOR_MGR_MAX_NBR_OF_SESSION; session_idx++)
        {
            if (!BACKDOOR_MGR_SESSION_IS_USED(session_idx))
            {
                /* allocate/initialize pending_io
                 */
                if ((pending_io_p = malloc(sizeof(*pending_io_p))) == NULL)
                {
                    BACKDOOR_MGR_FATAL("%s: out of memory.", __FUNCTION__);
                    session_idx = BACKDOOR_MGR_INVALID_IDX;
                    break;
                }
                pending_io_p->pio_msg_p = (SYSFUN_Msg_T *)pending_io_p->msgbuf;
                pending_io_p->pio_ipcio_msg_p = (L_IPCIO_Msg_T *)pending_io_p->pio_msg_p->msg_buf;

                /* initialize IO service
                 */
                if (!BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
                {
                    memset(&bd_shmem_data_p->io_service[io_service_idx], 0, sizeof(*bd_shmem_data_p->io_service));
                    bd_shmem_data_p->io_service[io_service_idx] = *io_service_p;

                    BACKDOOR_MGR_IO_SERVICE_VALIDATE(io_service_idx);
                }

                memset(&bd_shmem_data_p->session[session_idx], 0, sizeof(*bd_shmem_data_p->session));
                bd_shmem_data_p->session[session_idx].force_stdio = FALSE;
                bd_shmem_data_p->session[session_idx].forked = forked;
                bd_shmem_data_p->session[session_idx].io_service_idx = io_service_idx;
                bd_shmem_data_p->session[session_idx].func_leave = func_leave;
                bd_shmem_data_p->session[session_idx].cookie = cookie;
                bd_shmem_data_p->session[session_idx].task_id = 0;
                bd_shmem_data_p->session[session_idx].pending_io_p = pending_io_p;
                bd_shmem_data_p->session[session_idx].backdoor_callback_idx = BACKDOOR_MGR_INVALID_IDX;
                bd_shmem_data_p->session[session_idx].backdoor_task_id = 0;

                BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, BACKDOOR_MGR_PENDING_IO_ST_IDLE);
                BACKDOOR_MGR_SESSION_SET_STATUS(session_idx, BACKDOOR_MGR_ST_INIT);

                break;
            }
        }
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    return session_idx;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_MainMenu
 *-----------------------------------------------------------
 * FUNCTION: This function is to show backdoor main menu.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : should only be call by backdoor main thread.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_MainMenu(UI32_T session_idx)
{
#define MAX_CHARS_PER_ROW   79
#define NUM_OF_COLS     (MAX_CHARS_PER_ROW / (BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH + 4))
#define NUM_OF_ROWS     ((bd_shmem_data_p->nbr_of_callback + NUM_OF_COLS - 1) / NUM_OF_COLS)

    char   key_in_string[BACKDOOR_MGR_MAX_CSC_NAME_STRING_LENTH + 1];
    UI32_T ret;
    UI32_T callback_idx = BACKDOOR_MGR_INVALID_IDX;
    BOOL_T failure_detected = FALSE;
    int i, j;

#if BACKDOOR_MGR_MAIN_MENU_SORT
    BACKDOOR_MGR_SortRegisteredCallback();
#endif

    while (!failure_detected)
    {
        BACKDOOR_MGR_Print(
            "\r\n\r\n\r\n\r\n\r\n"
            "----------------------------------------\r\n"
            "Main menu of backdoor:\r\n");

        for (i = 0; i < NUM_OF_ROWS; i++)
        {
            BACKDOOR_MGR_Printf("\r\n");

            for (j = 0; j < NUM_OF_COLS; j++)
            {
#if BACKDOOR_MGR_MAIN_MENU_BY_COLUMN
                callback_idx = i + j * NUM_OF_ROWS;
#else
                callback_idx = i * NUM_OF_COLS + j;
#endif

                if (callback_idx < bd_shmem_data_p->nbr_of_callback)
                {
                    BACKDOOR_MGR_Printf(" * %-*s ",
                        (MAX_CHARS_PER_ROW/NUM_OF_COLS - 4),
                        bd_shmem_data_p->callback[callback_idx].csc_name);
                }
            }
        }

        BACKDOOR_MGR_Print("\r\n\r\n " BACKDOOR_MGR_RESERVED_WORD_FOR_EXIT " --- To exit from backdoor.\r\n\r\n");

        BACKDOOR_MGR_Print("Key in the CSC name: ");

        if (!BACKDOOR_MGR_RequestKeyIn(key_in_string, sizeof(key_in_string)-1))
        {
            failure_detected = TRUE;
            break;
        }

        BACKDOOR_MGR_Print("\r\n");

        ret = BACKDOOR_MGR_ParsingKeyin(key_in_string, &callback_idx);

        switch(ret)
        {
            case BACKDOOR_MGR_PARSING_SUCCESS:
                if (BACKDOOR_MGR_NotifyInvokeBackdoor(session_idx, callback_idx))
                    failure_detected = !BACKDOOR_MGR_WaitForBackdoorFinish(session_idx);
                else
                    BACKDOOR_MGR_Print("\r\nFailed to invoke backdoor.\r\n");
                BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu callback_idx: %lu", (unsigned long)session_idx, (unsigned long)callback_idx);
                break;

            case BACKDOOR_MGR_PARSING_FAIL:
                BACKDOOR_MGR_Print("\r\nInvalid CSC name.\r\n");
                break;

            case BACKDOOR_MGR_PARSING_KEYIN_EXIT:
                return TRUE;
        }
    }

    if (failure_detected)
    {
        BACKDOOR_MGR_FATAL("EXCEPTION: (MainMenu) session_idx: %lu", (unsigned long)session_idx);
        BACKDOOR_MGR_HandleException(session_idx, FALSE);
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_ParsingKeyin
 *-----------------------------------------------------------
 * FUNCTION: To parse user input string.
 * INPUT   : key_in_string   - User input string.
 * OUTPUT  : callback_idx_p   - The selected backdoor.
 * RETURN  : BACKDOOR_MGR_PARSING_SUCCESS    - The matched CSC name is found.
 *           BACKDOOR_MGR_PARSING_KEYIN_EXIT - User want to leave debug mode.
 *           BACKDOOR_MGR_PARSING_FAIL       - No matched CSC name.
 * NOTE    : None.
 *----------------------------------------------------------*/
static UI32_T BACKDOOR_MGR_ParsingKeyin(char *key_in_string, UI32_T *callback_idx_p)
{
#if BACKDOOR_MGR_MAIN_MENU_PARTIAL_MATCH
    size_t in_len;
    int min_diff_i = -1;
#endif
    int i;

    BACKDOOR_MGR_DEBUG_MSG("key_in_string: %s", key_in_string);

    if (strcmp(key_in_string, BACKDOOR_MGR_RESERVED_WORD_FOR_IPC_IO) == 0)
        bd_shmem_data_p->all_force_stdio = !bd_shmem_data_p->all_force_stdio;

    if (strcmp(key_in_string, BACKDOOR_MGR_RESERVED_WORD_FOR_EXIT) == 0)
        return BACKDOOR_MGR_PARSING_KEYIN_EXIT;

#if BACKDOOR_MGR_MAIN_MENU_PARTIAL_MATCH
    in_len = strlen(key_in_string);

    if (in_len > 0)
    {
        for (i = 0; i < bd_shmem_data_p->nbr_of_callback; i++)
        {
            if (BACKDOOR_MGR_Strncmp(key_in_string, bd_shmem_data_p->callback[i].csc_name, in_len) != 0)
                continue;

            if (bd_shmem_data_p->callback[i].csc_name[in_len] == 0)
            {
                min_diff_i = i;
                break;
            }
            else if (min_diff_i == -1)
            {
                min_diff_i = i;
                BACKDOOR_MGR_DEBUG_MSG("partial match: callback_idx: %d, csc_name: %s", (int)*callback_idx_p, bd_shmem_data_p->callback[i].csc_name);
            }
            else if (BACKDOOR_MGR_Strcmp(
                        bd_shmem_data_p->callback[i].csc_name,
                        bd_shmem_data_p->callback[min_diff_i].csc_name) < 0)
            {
                min_diff_i = i;
                BACKDOOR_MGR_DEBUG_MSG("partial match: callback_idx: %d, csc_name: %s", (int)*callback_idx_p, bd_shmem_data_p->callback[i].csc_name);
            }
        }
    }

    if (min_diff_i != -1)
    {
        *callback_idx_p = min_diff_i;
        BACKDOOR_MGR_DEBUG_MSG("callback_idx: %d, csc_name: %s", (int)*callback_idx_p, bd_shmem_data_p->callback[*callback_idx_p].csc_name);
        return BACKDOOR_MGR_PARSING_SUCCESS;
    }
#else
    for (i = 0; i < bd_shmem_data_p->nbr_of_callback; i++)
    {
        if (BACKDOOR_MGR_Strcmp(key_in_string, bd_shmem_data_p->callback[i].csc_name) == 0)
        {
            *callback_idx_p = i;
            BACKDOOR_MGR_DEBUG_MSG("callback_idx: %d, csc_name: %s", (int)*callback_idx_p, bd_shmem_data_p->callback[i].csc_name);
            return BACKDOOR_MGR_PARSING_SUCCESS;
        }
    }
#endif

    return BACKDOOR_MGR_PARSING_FAIL;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_WaitForBackdoorFinish
 *-----------------------------------------------------------
 * FUNCTION: To wait until CSC backdoor terminate for main menu.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : should only be call by backdoor main thread.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_WaitForBackdoorFinish(UI32_T session_idx)
{
    UI32_T timeout;
    UI32_T wait_events, received_events, local_events = 0;
    BOOL_T io_handling;
    BOOL_T failure_detected = FALSE;

    BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ);

    io_handling = FALSE;
    wait_events =
        BACKDOOR_MGR_MAIN_EVENT_LEAVE_BACKDOOR |
        BACKDOOR_MGR_MAIN_EVENT_STOP_IO_HANDLING |
        BACKDOOR_MGR_MAIN_EVENT_START_IO_HANDLING;

    while (1)
    {
        timeout = io_handling ? BACKDOOR_MGR_MAIN_TIMEOUT_RUNNING : BACKDOOR_MGR_MAIN_TIMEOUT_INACTIVE;

        SYSFUN_ReceiveEvent (
            wait_events,
            SYSFUN_EVENT_WAIT_ANY,
            timeout,
            &received_events);

        local_events |= received_events;

        BACKDOOR_MGR_DEBUG_MSG("received_events: %08lx local_events: %08lx", received_events, local_events);

        if (local_events & BACKDOOR_MGR_MAIN_EVENT_LEAVE_BACKDOOR)
        {
            BACKDOOR_MGR_DEBUG_MSG("received_events: %08lx local_events: %08lx", received_events, local_events);
            BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, BACKDOOR_MGR_PENDING_IO_ST_IDLE);
            break;
        }

        if (local_events & BACKDOOR_MGR_MAIN_EVENT_STOP_IO_HANDLING)
        {
            BACKDOOR_MGR_DEBUG_MSG("received_events: %08lx local_events: %08lx", received_events, local_events);
            io_handling = FALSE;
            local_events ^= BACKDOOR_MGR_MAIN_EVENT_STOP_IO_HANDLING;
        }

        if (local_events & BACKDOOR_MGR_MAIN_EVENT_START_IO_HANDLING)
        {
            BACKDOOR_MGR_DEBUG_MSG("received_events: %08lx local_events: %08lx", received_events, local_events);
            io_handling = TRUE;
            local_events ^= BACKDOOR_MGR_MAIN_EVENT_START_IO_HANDLING;
        }

        if (!BACKDOOR_MGR_CheckBackdoorAlive(session_idx))
        {
            BACKDOOR_MGR_FATAL("EXCEPTION: (BackdoorAlive) session_idx: %lu", (unsigned long)session_idx);
            BACKDOOR_MGR_HandleException(session_idx, FALSE);
            failure_detected = TRUE;
            break;
        }

        if (io_handling)
        {
            if (!BACKDOOR_MGR_HandlePendingIO(session_idx, &local_events))
            {
                BACKDOOR_MGR_FATAL("EXCEPTION: (HandleIO) session_idx: %lu", (unsigned long)session_idx);
                BACKDOOR_MGR_HandleException(session_idx, FALSE);
                failure_detected = TRUE;
                break;
            }
        }
    }

    return !failure_detected;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_CheckBackdoorAlive
 *-----------------------------------------------------------
 * FUNCTION: To check if backdoor is alive.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - backdoor task is running/terminated normally.
 *           FALSE - backoodr task is terminated abnormally.
 * NOTE    : None.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_CheckBackdoorAlive(UI32_T session_idx)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];
    char task_name[SYSFUN_TASK_NAME_LENGTH+1];

    if (session_p->backdoor_task_id == 0)
    {
        /* not ready or terminated normally
         */
        return TRUE;
    }

    if (SYSFUN_OK != SYSFUN_TaskIDToName(session_p->backdoor_task_id, task_name, sizeof(task_name)))
    {
        /* task doesn't exist
         */
        return FALSE;
    }

    if (strncmp(task_name, SYS_BLD_BACKDOOR_CSC_THREAD_NAME, sizeof(SYS_BLD_BACKDOOR_CSC_THREAD_NAME)-1) != 0)
    {
        /* not backdoor task
         */
        return FALSE;
    }

    return TRUE;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandlePendingIO
 *-----------------------------------------------------------
 * FUNCTION: To monitor IO.
 * INPUT   : session_idx
 *           event
 * OUTPUT  : event
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : should only be call by backdoor main thread.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_HandlePendingIO(UI32_T session_idx, UI32_T *event_p)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];
    BACKDOOR_MGR_PendingIO_T *pending_io_p;
    UI32_T io_service_idx;
    SYSFUN_MsgQ_T msgq_handle;
    BOOL_T failure_detected = FALSE;
    BOOL_T need_resp = FALSE;

    const char *output_str = NULL;
    const char *bs_str = "\b \b";
    char output_buf[2];
    int ch = EOF;
    int (*func_getchar)(void *cookie) = NULL;
    int (*func_print)(void *cookie, const char *s) = NULL;
    void *cookie = NULL;

    io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

    if (!BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
    {
        return FALSE;
    }

    if (!BACKDOOR_MGR_IO_SERVICE_IS_IPC(io_service_idx))
    {
        return TRUE;
    }

    if (!BACKDOOR_MGR_PENDING_IO_IS_WAIT_KEY(session_idx))
    {
        return TRUE;
    }

    func_getchar = BACKDOOR_MGR_IO_SERVICE_FUNC_GETCHAR(io_service_idx);
    func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
    cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;

    pending_io_p = session_p->pending_io_p;

    ch = func_getchar(cookie);
    BACKDOOR_MGR_DEBUG_MSG("ch: 0x%02x", ch);

    BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

    {
        switch (pending_io_p->pio_ipcio_msg_p->ipcio_cmd)
        {
            case L_IPCIO_CMD_GETCHAR:
                if (ch == EOF)
                {
                    pending_io_p->pio_ipcio_msg_p->ipcio_ret = FALSE;
                    pending_io_p->pio_msg_p->msg_size = L_IPCIO_SIZE_OF_MSG(0);

                    failure_detected = TRUE;

                    need_resp = TRUE;
                }
                else if (ch != 0)
                {
                    pending_io_p->pio_ipcio_msg_p->ipcio_arg_ch = ch;
                    pending_io_p->pio_ipcio_msg_p->ipcio_ret = TRUE;
                    pending_io_p->pio_msg_p->msg_size = L_IPCIO_SIZE_OF_MSG(
                        sizeof(pending_io_p->pio_ipcio_msg_p->ipcio_arg_ch));

                    need_resp = TRUE;
                }
                break;

            case L_IPCIO_CMD_GETLINE:
                if (isprint(ch) &&
                    pending_io_p->pio_received_nchar < pending_io_p->pio_required_nchar)
                {
                    pending_io_p->pio_ipcio_msg_p->ipcio_arg_buffer[pending_io_p->pio_received_nchar] = ch;
                    pending_io_p->pio_received_nchar++;
                    output_buf[0] = ch;
                    output_buf[1] = 0;
                    output_str = output_buf;
                }
                else if (ch == '\b' && pending_io_p->pio_received_nchar > 0)
                {
                    pending_io_p->pio_received_nchar--;
                    output_str = bs_str;
                }
                else if (ch == '\r' || ch == '\n' || ch == EOF)
                {
                    if (ch == EOF && pending_io_p->pio_received_nchar == 0)
                    {
                        pending_io_p->pio_ipcio_msg_p->ipcio_ret = FALSE;
                        pending_io_p->pio_msg_p->msg_size = L_IPCIO_SIZE_OF_MSG(0);

                        failure_detected = TRUE;

                        need_resp = TRUE;
                    }
                    else
                    {
                        pending_io_p->pio_ipcio_msg_p->ipcio_arg_buffer[pending_io_p->pio_received_nchar] = 0;
                        pending_io_p->pio_ipcio_msg_p->ipcio_ret = TRUE;
                        pending_io_p->pio_msg_p->msg_size = L_IPCIO_SIZE_OF_MSG(
                            pending_io_p->pio_received_nchar + 1);

                        need_resp = TRUE;
                    }
                }
                break;
        }
    }

    BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();

    if (output_str && func_print)
    {
        func_print(cookie, output_str);
    }

    if (need_resp)
    {
        BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

        msgq_handle = BACKDOOR_MGR_GetIOServiceMsgq(io_service_idx);

        if (msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
        {
            if (SYSFUN_SendResponseMsg(msgq_handle, pending_io_p->pio_msg_p) != SYSFUN_OK)
            {
                BACKDOOR_MGR_ERROR("%s: SYSFUN_SendResponseMsg fail.", __FUNCTION__);
                failure_detected = TRUE;
            }
        }
        else
        {
            BACKDOOR_MGR_DEBUG_MSG("%s: BACKDOOR_MGR_GetIOServiceMsgq fail.", __FUNCTION__);
            failure_detected = TRUE;
        }

        *event_p |= BACKDOOR_MGR_MAIN_EVENT_STOP_IO_HANDLING;

        BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, BACKDOOR_MGR_PENDING_IO_ST_WAIT_REQ);

        BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
    }

    return !failure_detected;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_EnterDebugMode
 *-----------------------------------------------------------------
 * FUNCTION: This function is called when enter debug mode.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_EnterDebugMode(UI32_T session_idx)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];
    BOOL_T forked;

    forked = session_p->forked;

    /* if callback func is not specified,
     * use current thread to work as backdoor main thread.
     */
    if (forked)
    {
        /* Spawn the backdoor main thread.
         */
        if (!BACKDOOR_MGR_InitMainThread(session_idx))
        {
            return FALSE;
        }
    }
    else
    {
        /* call main entry directly.
         */
        BACKDOOR_MGR_MainEntry(&session_idx);
    }

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_LeaveDebugMode
 *-----------------------------------------------------------------
 * FUNCTION: This function is called when leave debug mode.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : should only be call by backdoor main thread.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_LeaveDebugMode(UI32_T session_idx)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, func_leave: %p, cookie: %p", (unsigned long)session_idx, session_p->func_leave, session_p->cookie);

    if (session_p->func_leave)
        session_p->func_leave(session_p->cookie);

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();
    BACKDOOR_MGR_ENTER_CRITICAL_SECTION();
    free(session_p->pending_io_p);
    BACKDOOR_MGR_SESSION_SET_STATUS(session_idx, BACKDOOR_MGR_ST_INACTIVE);
    BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_LeaveBackdoor
 *-----------------------------------------------------------------
 * FUNCTION: This function is called by backdoor sub thread
 *           before termination.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_LeaveBackdoor(UI32_T session_idx)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];

    BACKDOOR_MGR_DEBUG_MSG("callback_idx: %lu", (unsigned long)session_p->backdoor_callback_idx);
    session_p->backdoor_callback_idx = BACKDOOR_MGR_INVALID_IDX;

    if (!BACKDOOR_MGR_SendEventToMain(session_idx, BACKDOOR_MGR_MAIN_EVENT_LEAVE_BACKDOOR))
    {
        BACKDOOR_MGR_FATAL("%s: backdoor main task can't wake up.", __FUNCTION__);
        return FALSE;
    }

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu", (unsigned long)session_p->task_id);

    return TRUE;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_InitMainThread
 *-----------------------------------------------------------------
 * FUNCTION: To spwan backdoor main thread.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_InitMainThread(UI32_T session_idx)
{
    UI32_T task_id;
    UI32_T ret;
    char task_name[SYSFUN_TASK_NAME_LENGTH+1];

    snprintf(task_name, sizeof(task_name)-1, "%.*s%02lu", (int)sizeof(task_name)-3, SYS_BLD_BACKDOOR_MGR_THREAD_NAME, (unsigned long)session_idx);
    task_name[sizeof(task_name)-1] = 0;

    ret = SYSFUN_SpawnThread(SYS_BLD_BACKDOOR_THREAD_PRIORITY,
                             SYS_BLD_BACKDOOR_THREAD_SCHED_POLICY,
                             task_name,
                             SYS_BLD_TASK_COMMON_STACK_SIZE,
                             SYSFUN_TASK_NO_FP,
                             (void *)BACKDOOR_MGR_MainEntry,
                             (void *)&session_idx,
                             &task_id);

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, ret(sysfun): %lu", (unsigned long)session_idx, (unsigned long)ret);

    return (ret == SYSFUN_OK);
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_MainEntry
 *-----------------------------------------------------------
 * FUNCTION: backdoor main thread.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
static void BACKDOOR_MGR_MainEntry(void *session_idx_p)
{
    UI32_T session_idx = *(UI32_T *)session_idx_p;
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];

    session_p->task_id = SYSFUN_TaskIdSelf();

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();
    BACKDOOR_MGR_SESSION_SET_STATUS(session_idx, BACKDOOR_MGR_ST_RUNNING);
    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu, task_id: %lu", (unsigned long)session_idx, (unsigned long)session_p->task_id);

    BACKDOOR_MGR_MainMenu(session_idx);

    BACKDOOR_MGR_LeaveDebugMode(session_idx);

    BACKDOOR_MGR_DEBUG_MSG("session_idx: %lu", (unsigned long)session_idx);
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_BackdoorEntry
 *-----------------------------------------------------------
 * FUNCTION: backdoor sub thread.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : should only be call by backdoor sub thread.
 *----------------------------------------------------------*/
static void BACKDOOR_MGR_BackdoorEntry(void *session_idx_p)
{
    UI32_T session_idx = *(UI32_T *)session_idx_p;
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];
    void (*func)(void) = (void (*)(void))bd_shmem_data_p->callback[session_p->backdoor_callback_idx].func;

    /* set session info.
     */
    session_p->backdoor_task_id = SYSFUN_TaskIdSelf();
    BACKDOOR_MGR_DEBUG_MSG("callback_idx:%lu, task_id: %lu", (unsigned long)session_p->backdoor_callback_idx, (unsigned long)session_p->backdoor_task_id);

    func();

    /* clear session info.
     */
    BACKDOOR_MGR_DEBUG_MSG("callback_idx:%lu, task_id: %lu", (unsigned long)session_p->backdoor_callback_idx, (unsigned long)session_p->backdoor_task_id);
    session_p->backdoor_task_id = 0;

    BACKDOOR_MGR_LeaveBackdoor(session_idx);
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleInvokeBackdoor
 *-----------------------------------------------------------
 * FUNCTION: To parse the backdoor message and spawn backdoor sub thread.
 * INPUT   : bd_msg_p - The backdoor message.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_HandleInvokeBackdoor(BACKDOOR_MGR_Msg_T *bd_msg_p)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[bd_msg_p->session_idx];
    UI32_T task_id;
    UI32_T ret;
    char task_name[SYSFUN_TASK_NAME_LENGTH+1];

    BACKDOOR_MGR_DEBUG_MSG("callback_idx: %lu", (unsigned long)bd_msg_p->callback_idx);

    /* invoke backdoor passes session_idx and callback_idx between two threads,
     * here we store callback_idx in session info block,
     * so we need only pass session id to spawned thread.
     */
    session_p->backdoor_callback_idx = bd_msg_p->callback_idx;

    snprintf(task_name, sizeof(task_name)-1, "%.*s%02lu", (int)sizeof(task_name)-3, SYS_BLD_BACKDOOR_CSC_THREAD_NAME, (unsigned long)bd_msg_p->session_idx);
    task_name[sizeof(task_name)-1] = 0;

    ret = SYSFUN_SpawnThread(SYS_BLD_BACKDOOR_THREAD_PRIORITY,
                             SYS_BLD_BACKDOOR_THREAD_SCHED_POLICY,
                             task_name,
                             SYS_BLD_BACKDOOR_THREAD_STACK_SIZE,
                             SYSFUN_TASK_NO_FP,
                             (void *)BACKDOOR_MGR_BackdoorEntry,
                             (void *)&bd_msg_p->session_idx,
                             &task_id);

    /* can't spawn the backdoor sub thread.
     */
    if (ret != SYSFUN_OK)
        goto EXIT_AND_LEAVE_BACKDOOR;

    return TRUE;

EXIT_AND_LEAVE_BACKDOOR:
    BACKDOOR_MGR_LeaveBackdoor(bd_msg_p->session_idx);
    return FALSE;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_NotifyInvokeBackdoor
 *-----------------------------------------------------------
 * FUNCTION: To notify CSC to execute backdoor.
 * INPUT   : session_idx
 *           callback_idx
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : should only be call by backdoor main thread.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_NotifyInvokeBackdoor(UI32_T session_idx, UI32_T callback_idx)
{
    BACKDOOR_MGR_CallBack_T *callback_p = &bd_shmem_data_p->callback[callback_idx];

    SYSFUN_Msg_T *msg_p = (SYSFUN_Msg_T *)bd_msg_buffer;
    BACKDOOR_MGR_Msg_T *bd_msg_p;
    SYSFUN_MsgQ_T msgq_handle;
    UI32_T ret;

    if (SYSFUN_GetMsgQ(callback_p->msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &msgq_handle) != SYSFUN_OK)
    {
        printf("\r\n%s(): fail to get message queue.\r\n", __FUNCTION__);
        return FALSE;
    }

    /* Send asynchronous request to CSC.
     */
    BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

    msg_p->cmd = SYS_MODULE_BACKDOOR;
    msg_p->msg_size = sizeof(BACKDOOR_MGR_Msg_T);
    bd_msg_p = (BACKDOOR_MGR_Msg_T *)msg_p->msg_buf;
    bd_msg_p->backdoor_cmd = BACKDOOR_MGR_CMD_INVOKE_BACKDOOR;
    bd_msg_p->session_idx = session_idx;
    bd_msg_p->callback_idx = callback_idx;

    ret = SYSFUN_SendRequestMsg(msgq_handle,
                                msg_p,
                                SYSFUN_TIMEOUT_NOWAIT,
                                SYSFUN_SYSTEM_EVENT_IPCMSG,
                                0,
                                NULL);

    BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();

    SYSFUN_ReleaseMsgQ(msgq_handle);

    return (ret == SYSFUN_OK);
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCIOMsg_GetKey
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle L_IPCIO messages.
 * INPUT   : msgq_key - the key of msgq that received the message.
 *           msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : should only be call by IO service thread.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_HandleIPCIOMsg_GetKey(UI32_T msgq_key, SYSFUN_Msg_T *msg_p)
{
    L_IPCIO_Msg_T *l_ipcio_msg_p = (L_IPCIO_Msg_T *)msg_p->msg_buf;
    BACKDOOR_MGR_PendingIO_T *pending_io_p;
    UI32_T session_idx;
    UI32_T io_service_idx;
    UI32_T req_is_wrong = TRUE;
    BOOL_T need_resp = FALSE;

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(msg_p->msg_type);

    io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

    BACKDOOR_MGR_DEBUG_MSG("msgq_key: %lu, session_idx: %lu, io_service_idx: %lu", msgq_key, session_idx, io_service_idx);

    if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx) &&
        BACKDOOR_MGR_IO_SERVICE_IS_IPC(io_service_idx) &&
        bd_shmem_data_p->io_service[io_service_idx].msgq_key == msgq_key)
    {
        BACKDOOR_MGR_ENTER_CRITICAL_SECTION();

        if (BACKDOOR_MGR_PENDING_IO_IS_WAIT_REQ(session_idx))
        {
            if (l_ipcio_msg_p->ipcio_cmd != L_IPCIO_CMD_GETLINE ||
                l_ipcio_msg_p->ipcio_arg_size > 0)
            {
                req_is_wrong = FALSE;

                pending_io_p = bd_shmem_data_p->session[session_idx].pending_io_p;

                memcpy(pending_io_p->pio_msg_p, msg_p, SYSFUN_SIZE_OF_MSG(msg_p->msg_size));

                if (l_ipcio_msg_p->ipcio_cmd == L_IPCIO_CMD_GETLINE)
                {
                    pending_io_p->pio_received_nchar = 0;
                    pending_io_p->pio_required_nchar = l_ipcio_msg_p->ipcio_arg_size - 1;
                }

                BACKDOOR_MGR_PENDING_IO_SET_STATE(session_idx, BACKDOOR_MGR_PENDING_IO_ST_WAIT_KEY);

                BACKDOOR_MGR_SendEventToMain(session_idx, BACKDOOR_MGR_MAIN_EVENT_START_IO_HANDLING);
            }
        }

        BACKDOOR_MGR_LEAVE_CRITICAL_SECTION();
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    if (req_is_wrong)
    {
        BACKDOOR_MGR_DEBUG_MSG("msgq_key: %lu, session_idx: %lu, io_service_idx: %lu", msgq_key, session_idx, io_service_idx);
        l_ipcio_msg_p->ipcio_ret = FALSE;
        msg_p->msg_size = L_IPCIO_SIZE_OF_MSG(0);
        need_resp = TRUE;
    }

    return need_resp;
}

/*-----------------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleIPCIOMsg_Print
 *-----------------------------------------------------------------
 * FUNCTION: This function is used to handle L_IPCIO messages.
 * INPUT   : msgq_key - the key of msgq that received the message.
 *           msg_p - The message to process.
 * OUTPUT  : msg_p - The message to response when return value is TRUE
 * RETURN  : TRUE  - need to send response.
 *           FALSE - not need to send response.
 * NOTE    : should only be call by IO service thread.
 *----------------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_HandleIPCIOMsg_Print(UI32_T msgq_key, SYSFUN_Msg_T *msg_p)
{
    L_IPCIO_Msg_T *l_ipcio_msg_p = (L_IPCIO_Msg_T *)msg_p->msg_buf;
    UI32_T session_idx;
    UI32_T io_service_idx;
    BOOL_T multi_send;
    BOOL_T need_resp = FALSE;
    int (*func_print)(void *cookie, const char *s) = NULL;
    void *cookie = NULL;

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    session_idx = BACKDOOR_MGR_GetRunningSessionByTaskId(msg_p->msg_type);

    multi_send = !BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx);

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_DEBUG_MSG("msgq_key: %lu, session_idx: %lu, multi_send: %u", msgq_key, session_idx, multi_send);

    if (!multi_send)
    {
        BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

        io_service_idx = BACKDOOR_MGR_GetIOService(session_idx);

        if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx) &&
            BACKDOOR_MGR_IO_SERVICE_IS_IPC(io_service_idx) &&
            bd_shmem_data_p->io_service[io_service_idx].msgq_key == msgq_key)
        {
            func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
            cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
        }

        BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

        BACKDOOR_MGR_DEBUG_MSG("io_service_idx: %lu, func_print: %p, cookie: %p", io_service_idx, func_print, cookie);

        if (func_print)
            func_print(cookie, l_ipcio_msg_p->ipcio_arg_buffer);
    }
    else
    {
        /* if the task that sent request is not backdoor task,
         * print to all IO service.
         */
        for (io_service_idx = 0; io_service_idx < BACKDOOR_MGR_MAX_NBR_OF_USER_IO_SERVICE; io_service_idx++)
        {
            func_print = NULL;

            BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

            if (BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx) &&
                BACKDOOR_MGR_IO_SERVICE_IS_IPC(io_service_idx) &&
                bd_shmem_data_p->io_service[io_service_idx].msgq_key == msgq_key)
            {
                func_print = BACKDOOR_MGR_IO_SERVICE_FUNC_PRINT(io_service_idx);
                cookie = bd_shmem_data_p->io_service[io_service_idx].cookie;
            }

            BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

            if (func_print)
                func_print(cookie, l_ipcio_msg_p->ipcio_arg_buffer);
        }
    }

    return need_resp;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_HandleException
 *-----------------------------------------------------------
 * FUNCTION: To monitor IO.
 * INPUT   : session_idx
 *           dflt_del_self
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------*/
static void BACKDOOR_MGR_HandleException(UI32_T session_idx, BOOL_T dflt_del_self)
{
    BACKDOOR_MGR_Session_T *session_p = &bd_shmem_data_p->session[session_idx];
    UI32_T io_service_idx;
    UI32_T task_id = SYSFUN_TaskIdSelf();
    BOOL_T del_self = dflt_del_self;
    void (*func_leave)(void *cookie) = NULL;
    void *cookie = NULL;

    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    if (BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx) &&
        BACKDOOR_MGR_SESSION_IS_USED(session_idx))
    {
        if (BACKDOOR_MGR_TASK_IS_BACKDOOR_MAIN(task_id, session_idx))
        {
            if (session_p->forked)
            {
                del_self = TRUE;

                func_leave = session_p->func_leave;
                cookie = session_p->cookie;

                free(session_p->pending_io_p);
                BACKDOOR_MGR_SESSION_SET_STATUS(session_idx, BACKDOOR_MGR_ST_INACTIVE);
            }
            else
            {
                del_self = FALSE;
            }

            /* if no one use the IO service, invalidate it.
             */
            io_service_idx = session_p->io_service_idx;

            if (BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx) &&
                BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
            {
                UI32_T session_idx;

                for (session_idx = 0; session_idx < BACKDOOR_MGR_MAX_NBR_OF_SESSION; session_idx++)
                {
                    if (BACKDOOR_MGR_SESSION_IS_USED(session_idx) &&
                        bd_shmem_data_p->session[session_idx].io_service_idx == io_service_idx)
                    {
                        break;
                    }
                }

                if (!BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx))
                {
                    BACKDOOR_MGR_IO_SERVICE_INVALIDATE(io_service_idx);
                }
            }
        }
        else if (BACKDOOR_MGR_TASK_IS_CSC_BACKDOOR(task_id, session_idx))
        {
            del_self = TRUE;
        }
    }

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();

    BACKDOOR_MGR_FATAL("task_id: %lu, session_idx: %lu, func_leave: %p, dflt_del_self: %u, del_self: %u", (unsigned long)task_id, (unsigned long)session_idx, func_leave, dflt_del_self, del_self);

    if (func_leave)
        func_leave(cookie);

    if (del_self)
        SYSFUN_DelSelfThread();
}

#if BACKDOOR_MGR_MAIN_MENU_SORT
/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_SortRegisteredCallback
 *-----------------------------------------------------------
 * FUNCTION: sort registered callback
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
static void BACKDOOR_MGR_SortRegisteredCallback(void)
{
    BACKDOOR_MGR_ENTER_SHARED_CRITICAL_SECTION();

    qsort(
        bd_shmem_data_p->callback,
        bd_shmem_data_p->nbr_of_callback,
        sizeof(*bd_shmem_data_p->callback),
        (void *)BACKDOOR_MGR_RegisteredCallbackCmp);

    BACKDOOR_MGR_LEAVE_SHARED_CRITICAL_SECTION();
}
#endif


/*-----------------------------------------------------------
 * Following functions never take care critical section
 *-----------------------------------------------------------*/

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetRunningSessionByTaskId
 *-----------------------------------------------------------
 * FUNCTION: To find session by task id
 * INPUT   : task_id - 0 means self task
 * OUTPUT  : None.
 * RETURN  : session_idx
 * NOTE    : None.
 *----------------------------------------------------------*/
static UI32_T BACKDOOR_MGR_GetRunningSessionByTaskId(UI32_T task_id)
{
    UI32_T session_idx;

    if (task_id == 0)
        task_id = SYSFUN_TaskIdSelf();

    for (session_idx = 0; session_idx < BACKDOOR_MGR_MAX_NBR_OF_SESSION; session_idx++)
    {
        if (!BACKDOOR_MGR_SESSION_IS_RUNNING(session_idx))
        {
            continue;
        }

        if (BACKDOOR_MGR_TASK_IS_BACKDOOR_MAIN(task_id, session_idx) ||
            BACKDOOR_MGR_TASK_IS_CSC_BACKDOOR(task_id, session_idx))
        {
            return session_idx;
        }
    }

    return BACKDOOR_MGR_INVALID_IDX;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetIOService
 *-----------------------------------------------------------
 * FUNCTION: This function is to get which io_service to use
 *           for specify session.
 * INPUT   : session_idx
 * OUTPUT  : None.
 * RETURN  : the io_service or NULL if not found.
 * NOTE    : None.
 *----------------------------------------------------------*/
static UI32_T BACKDOOR_MGR_GetIOService(UI32_T session_idx)
{
    UI32_T io_service_idx;

    if (!BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx) ||
        !BACKDOOR_MGR_SESSION_IS_RUNNING(session_idx))
    {
        return BACKDOOR_MGR_INVALID_IDX;
    }

    if (bd_shmem_data_p->all_force_stdio)
    {
        return BACKDOOR_MGR_STDIO_IO_SERVICE_ID;
    }

    if (bd_shmem_data_p->session[session_idx].force_stdio)
    {
        return BACKDOOR_MGR_STDIO_IO_SERVICE_ID;
    }

    io_service_idx = bd_shmem_data_p->session[session_idx].io_service_idx;

    if (!BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(session_idx) ||
        !BACKDOOR_MGR_IO_SERVICE_IS_VALID(io_service_idx))
    {
        return BACKDOOR_MGR_INVALID_IDX;
    }

    return io_service_idx;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetIOServiceMsgq
 *-----------------------------------------------------------
 * FUNCTION: This function is to get msgq handle
 * INPUT   : io_service_idx
 * OUTPUT  : None.
 * RETURN  : msgq handle or 0 if failed.
 * NOTE    : None.
 *----------------------------------------------------------*/
static SYSFUN_MsgQ_T BACKDOOR_MGR_GetIOServiceMsgq(UI32_T io_service_idx)
{
    BACKDOOR_MGR_IOService_T *io_service_p;
    BACKDOOR_MGR_IPC_Handle_T *ipc_handle_p;
    SYSFUN_MsgQ_T msgq_handle;

    if (!BACKDOOR_MGR_IO_SERVICE_IDX_IS_VALID(io_service_idx))
    {
        return BACKDOOR_MGR_INVALID_MSGQ_HANDLE;
    }

    io_service_p = &bd_shmem_data_p->io_service[io_service_idx];
    ipc_handle_p = &bd_io_service_ctx[io_service_idx].ipc_handle;

    /* check if io_service is updated.
     * if yes, release original handle.
     */
    if (ipc_handle_p->msgq_key != io_service_p->msgq_key)
    {
        ipc_handle_p->msgq_key = io_service_p->msgq_key;

        if (ipc_handle_p->msgq_handle != BACKDOOR_MGR_INVALID_MSGQ_HANDLE)
        {
            SYSFUN_ReleaseMsgQ(ipc_handle_p->msgq_handle);
            ipc_handle_p->msgq_handle = BACKDOOR_MGR_INVALID_MSGQ_HANDLE;
        }
    }

    /* get msgq handle
     */
    if (ipc_handle_p->msgq_handle == BACKDOOR_MGR_INVALID_MSGQ_HANDLE &&
        ipc_handle_p->msgq_key != BACKDOOR_MGR_INVALID_MSGQ_KEY)
    {
        SYSFUN_GetMsgQ(ipc_handle_p->msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &ipc_handle_p->msgq_handle);

        BACKDOOR_MGR_DEBUG_MSG("io_service_idx: %lu, msgq_key: %lu, msgq_handle: %p", io_service_idx, ipc_handle_p->msgq_key, ipc_handle_p->msgq_handle);
    }

    msgq_handle = ipc_handle_p->msgq_handle;

    return msgq_handle;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_SendEventToMain
 *-----------------------------------------------------------
 * FUNCTION: To send event to backdoor main
 * INPUT   : session_idx
 *           event - event of backdoor main.
 * OUTPUT  : None.
 * RETURN  : TRUE  - Successful.
 *           FALSE - Failed.
 * NOTE    : None.
 *----------------------------------------------------------*/
static BOOL_T BACKDOOR_MGR_SendEventToMain(UI32_T session_idx, BACKDOOR_MGR_MainEvent_T event)
{
    UI32_T ret = SYSFUN_RESULT_ERROR;

    if (BACKDOOR_MGR_SESSION_IDX_IS_VALID(session_idx) &&
        BACKDOOR_MGR_SESSION_IS_RUNNING(session_idx))
    {
        ret = SYSFUN_SendEvent(bd_shmem_data_p->session[session_idx].task_id, event);
    }

    BACKDOOR_MGR_DEBUG_MSG("task_id: %lu -> %lu, session_idx: %lu, event: %08x, status: %d, ret(sysfun): %lu",
        SYSFUN_TaskIdSelf(),
        bd_shmem_data_p->session[session_idx].task_id,
        session_idx, event,
        bd_shmem_data_p->session[session_idx].status,
        ret);

    return (ret == SYSFUN_OK);
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_VPrintf_Local
 *-----------------------------------------------------------
 * FUNCTION: to get user input.
 * INPUT   : func_print, cookie
 *           fmt_p, arg_p
 * OUTPUT  : None.
 * RETURN  : >= 0 when successful
 * NOTE    : None.
 *----------------------------------------------------------*/
static int BACKDOOR_MGR_VPrintf_Local(
    int (*func_print)(void *cookie, const char *s),
    void *cookie,
    char *fmt_p, va_list arg_p)
{
    va_list tmp_arg_p;
    int ret;

    if (fmt_p == NULL)
        return -1;

    char *buf;
    int len;

    va_copy(tmp_arg_p, arg_p);
    len = vsnprintf(0, 0, fmt_p, tmp_arg_p);
    va_end(tmp_arg_p);

    if (len < 0)
    {
        return -1;
    }

    if ((buf = malloc(len + 1)) == NULL)
    {
        return -1;
    }

    vsnprintf(buf, len + 1, fmt_p, arg_p);

    ret = func_print(cookie, buf);

    free(buf);

    return ret;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_GetS_Local
 *-----------------------------------------------------------
 * FUNCTION: to get user input.
 * INPUT   : func_getchar, func_print, cookie
 *           size - size of buf
 * OUTPUT  : buf  - buffer to store input string
 * RETURN  : the pointer to buf or NULL if failed
 * NOTE    : None.
 *----------------------------------------------------------*/
static char *BACKDOOR_MGR_GetS_Local(
    int (*func_getchar)(void *cookie),
    int (*func_print)(void *cookie, const char *s),
    void *cookie,
    char *buf, int size)
{
    char tmpbuf[2];
    int i = 0;
    int ch;

    if (size == 0)
        return NULL;

    tmpbuf[1] = 0;

    while (1)
    {
        ch = func_getchar(cookie);

        if (i < size-1 && isprint(ch))
        {
            tmpbuf[0] = ch;
            func_print(cookie, tmpbuf);
            buf[i++] = ch;
        }
        else if (ch == '\b' && i > 0)
        {
            tmpbuf[0] = ch;
            func_print(cookie, "\b \b");
            i--;
        }
        else if (ch == '\n' || ch == '\r')
        {
            break;
        }
        else if (ch == EOF)
        {
            if (i == 0)
                return NULL;
            break;
        }
    }

    buf[i] = 0;

    return buf;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_StdioGetChar
 *-----------------------------------------------------------
 * FUNCTION: getchar
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
static int BACKDOOR_MGR_StdioGetChar(void *cookie)
{
    int ch;

    do{
        ch=getchar();
    }while (ch==EOF);

    return ch;
}

/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_StdioPrint
 *-----------------------------------------------------------
 * FUNCTION: print string
 * INPUT   : None.
 * OUTPUT  : None.
 * RETURN  : None.
 * NOTE    : None.
 *----------------------------------------------------------*/
static int BACKDOOR_MGR_StdioPrint(void *cookie, const char *s)
{
    return (printf("%s", s) < 0 ? -1 : (fflush(stdout), 0));
}

#if BACKDOOR_MGR_MAIN_MENU_SORT
/*-----------------------------------------------------------
 * ROUTINE NAME - BACKDOOR_MGR_SortRegisteredCallback
 *-----------------------------------------------------------
 * FUNCTION: compare function for registered callback sorting.
 * INPUT   : p1, p2 - callback info
 * OUTPUT  : None.
 * RETURN  : comparsion result.
 * NOTE    : None.
 *----------------------------------------------------------*/
static int BACKDOOR_MGR_RegisteredCallbackCmp(BACKDOOR_MGR_CallBack_T *p1, BACKDOOR_MGR_CallBack_T *p2)
{
    return BACKDOOR_MGR_Strcmp(p1->csc_name, p2->csc_name);
}
#endif

#if BACKDOOR_OPEN
static void BACKDOOR_MGR_BackdoorMenu(void)
{
}
#endif /* BACKDOOR_OPEN */

