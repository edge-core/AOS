/* Module Name: SYSFUN.H
 * Purpose:
 *      This module is unified system call for all platforms.
 *
 * Notes:
 *
 * HISTORY
 *       Date       --  Modifier,   Reason
 *  0.2 2001.8.16   --  William,    Add semaphore functions and add description.
 *  0.3 2001.8.26   --  William,    VxWorks platform
 *  0.4 2001.9.04   --  William,    Patch event_init. (Notes.5), & named constants
 *  0.5 2001.9.23   --  William,    Change timeAnnounce using signal. (not this version)
 *  0.6 2001.9.25   --  William,    Use Watch-Dog Timer to implement periodic timer.
 *  0.6a2001.9.29   --  William,    Watch-Dog dependent periodic timer.
 *  0.7 2001.10.29  --  William,    Add SYSFUN_EnableRoundRonbin/SYSFUN_DisableRoundRonbin,
 *                                  SYSFUN_SetTaskPriority, DBG_DumpHex, DBG_PrintText
 *  0.8  2001.11.14 --  William,    SYSFUN_SendEvent(), check tid <>0, to avoid mis-use.
 *  0.9  2002.01.08  -- William,    Mask off error message display for DBG_Print.
 *  0.10 2002.01.23 --  William,    Add SYSFUN_TaskIDToName()
 *  1.1  2002.06.20 --  William,    add macro SYSFUN_Om_WriteLock(), SYSFUN_Om_ReleaseWriteLock
 *                                  and SYSFUN_FlushSem(), SYSFUN_Mgr_EnterCriticalSection(),
 *                                  SYSFUN_Mgr_LeaveCriticalSection() for object lock.
 *  1.02 2002.08.06 --  William,    Add (enter/leave MGR macro)
 *  1.04 2002.09.21 --  William,    Add SYSFUN_Register_Ticks_Callback() for LED task to periodical callback,
 *                                  only one function can be registered in the implementation.
 *  1.05 2002.10.23 --  William,    Add SYSFUN_ConvertTaskIdToIndex() to convert task-id to sequential index.
 *                                  the index can be used as error-code log, task-related record.
 *                                  add a global variable for stacking developing issue,SYSFUN_PrintRefCounterFileName.
 *                                  this variable will be removed after stacking developing is done.
 *  1.06 2004.04.20 --  Penny,      in SYSFUN_TASK_HALT(), remove halting system; instead just log error message.
 *
 *       2007.04.12 --  Wakka,      Add SYSFUN_CopyFromUser(), SYSFUN_CopyToUser().
 *       2007.04.20 --  Wakka,      Add SYSFUN_RegisterCallBackFunc(),
 *                                  SYSFUN_KERNEL_MODULE_INIT(), SYSFUN_KERNEL_MODULE_EXIT()
 *
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002, 2004, 2006
 */

#ifndef     _SYSFUN_H
#define     _SYSFUN_H

/* INCLUDE FILE DECLARATIONS
 */
/* linux specific include files
 */
#include <linux/version.h>
#include <linux/kernel.h> /* for printk() */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include <linux/semaphore.h> /* for semaphore */
#else
#include <asm/semaphore.h>
#endif
#include <linux/spinlock.h>
#include <linux/irqflags.h>

/* Accton include files
 */
#include "sys_type.h"
#include "sys_bld.h"
#include "sysfun_type.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

#if (SYS_CPNT_PRINTF == FALSE)
    #define SYSFUN_Debug_Printf   SYSFUN_NullPrintf
#elif SYS_CPNT_PRINTF == TRUE
    #define SYSFUN_Debug_Printf   printk
#endif

/* DATA TYPE DECLARATIONS
 */

typedef struct SYSFUN_CpuRecord_S
{
    unsigned long  used_ticks;          /* ticks used by task between reporting interval
                                  * l_pt will modify this field, so we should not put
                                  * the critical field within first 4 bytes(This 4 bytes
                                  * will be modified by l_pt to maintain free list.
                                  */
    unsigned long  tid;                 /* 0-this utcb is not used, >0- task id */
    unsigned long  accumulated_ticks;   /* ticks used by task from started */
    void*   user_buffer;         /* buffer for user to keep cpu utilization */
}   SYSFUN_CpuRecord_T;

/* Define  user-TCB(task Control Block)
 * This structure will lie in task_struct on Linux.
 *
 * SYSFUN_INIT_UTCB will lie in INIT_TASK on Linux.
 * The purpose is to ensure the access validity of task_name.
 */
typedef struct SYSFUN_UserTcb_S
{
    SYSFUN_CpuRecord_T cpu_rec;
    unsigned long      event_set;            /* events be set and not received */
    unsigned long      event_waited;         /* events be waited by task */
    struct semaphore   event_sem;            /* synchronize mechanism to provide timimg */
    struct semaphore   suspense_sem;         /* the id of the semaphore which is used
                                              * to suspend a task with a timer
                                              */
    int                stack_peak_size;      /* max stack used in sampling interval */
    BOOL_T             event_is_all:1;       /* TRUE-wait all events, FALSE-wait any of events */
    BOOL_T             is_ui_task:1;         /* TRUE-This is a UI task, FALSE- This is not a UI task */
    BOOL_T             pading:6;             /* padding bit-filed to form a complete byte */
    UI8_T              eh_buf_idx;           /* EH error message buffer index bound to the task */
    UI8_T              reserved[2];          /* in order to use l_pt properly, this
                                              * structure need to be 4 byte alignment
                                              */
    char task_name[SYSFUN_TASK_NAME_LENGTH+1];
} SYSFUN_UserTcb_T;
#define SYSFUN_INIT_UTCB { .task_name = {0}, \
                           .eh_buf_idx = SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX \
                         }

typedef unsigned long SYSFUN_IntMask_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#define SYSFUN_NullPrintf(...)

/* FUNCTION NAME : SYSFUN_Init
 * PURPOSE:
 *      Initialize system function working environment; includes
 *      event working environment for VxWorks - hook function on
 *      task creation.
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
 *      1. This function is called by root().
 */
void SYSFUN_Init(void);

/* FUNCTION NAME : SYSFUN_Create_InterCSC_Relation
 * PURPOSE:
 *      This function initializes all function pointer registration operations.
 * INPUT:
 *      None.
 * OUTPUT:
 *      None.
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void SYSFUN_Create_InterCSC_Relation(void);

/* FUNCTION NAME : SYSFUN_TaskIDToName
 * PURPOSE:
 *      Retrieve Task name by task-id.
 * INPUT:
 *      task_id - the specified task ID.
 *				  0 : the caller task.
 *      size    - the size of buffer (task_name) to receive task name.
 *
 * OUTPUT:
 *      task_name - Address of pointer which point to name of task retrieved.
 *
 * RETURN:
 *      SYSFUN_OK                   - Successfully.
 *		SYSFUN_RESULT_INVALID_ID	- specified ID is not valid.
 *      SYSFUN_RESULT_NO_NAMED_TASK - specified task was no name.
 *		SYSFUN_RESULT_INVALID_ARG	- not specified task name buffer space.
 *
 * NOTES:
 *      1. If specified buf_size is smaller than task-name, task-name will be
 *		   truncated to fit the size (buf_size-1).
 *      2. On linux, task id could be thread id or process id.
 */
UI32_T  SYSFUN_TaskIDToName (UI32_T task_id, char *task_name, UI32_T size);

/* FUNCTION NAME : SYSFUN_TaskIdSelf
 * PURPOSE:
 *      get the task ID of a running task.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      The task ID of the calling task.
 *
 * NOTES:
 *      1. The task ID will be invalid if called at interrupt level.
 */
UI32_T SYSFUN_TaskIdSelf (void);

/* The following two functions SYSFUN_NonPreempty() and SYSFUN_Preempty()
 * originates from the SYSFUN in Vxworks and are not used in Linux anymore.
 * The linux kernel functions lock_kernel() and unlock_kernel() are removed
 * from v2.6.37. So just comment out SYSFUN_NonPreempty() and SYSFUN_Preempty().
 */
#if 0
/* FUNCTION NAME : SYSFUN_NonPreempty
 * PURPOSE:
 *      System enter non-preempty mode, no task schdule is occurs.
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
 *      1. When you call reschedule function, system may change to Preempty(), eg. Semaphore,
 *         wait message Q. Please reference each OS about preempty/nonpreempty function.
 */
void SYSFUN_NonPreempty (void);
#define SYSFUN_Lock()   SYSFUN_NonPreempty()


/* FUNCTION NAME : SYSFUN_Preempty
 * PURPOSE:
 *      System leave non-preempty mode, task schedule will be occured based on priority.
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
 *      None.
 */
void SYSFUN_Preempty (void);
#define SYSFUN_UnLock()   SYSFUN_Preempty()
#endif /* end of #if 0 */

/* FUNCTION NAME : SYSFUN_GetSysTick
 * PURPOSE:
 *      Get system accumulated ticks, from system start or restart.
 *      Stack topology do not reset the ticks.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *
 * RETURN:
 *      The accumulated ticks from system start or restart.
 *
 * NOTES:
 *      1. The unit of tick is 0.01 sec, ie. 10 ms. depending on system configuration.
 */
UI32_T  SYSFUN_GetSysTick (void);

/*--------------------------
 *  Interrupt lock/unlock
 *--------------------------
 */

/* FUNCTION NAME : SYSFUN_InterruptLock
 * PURPOSE:
 *      Set interrupt lock, disable hardware interrupt.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      interrupt mask value for disable interrupt.
 * NOTES:
 *      1. For pSos, use SYSFUN_DISABLE_ALL as parameter.
 *         SYSFUN_DISABLE_ALL : mask off all interrupt
 */
#if 1
#define SYSFUN_InterruptLock local_irq_save
#else
SYSFUN_IntMask_T SYSFUN_InterruptLock(void);
#endif


/* FUNCTION NAME : SYSFUN_InterruptUnlock
 * PURPOSE:
 *      Set interrupt lock, enable hardware interrupt.
 *
 * INPUT:
 *      x   --  interrupt mask value, return-value of SYSFUN_InterruptLock().
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 * NOTES:
 *
 */
#if 1
#define SYSFUN_InterruptUnlock local_irq_restore
#else
void SYSFUN_InterruptUnlock(SYSFUN_IntMask_T int_mask);
#endif

/* FUNCTION NAME : SYSFUN_EnterCriticalSection
 * PURPOSE:
 *      This function provides a general protection method for the code
 *      in kernel space using spin lock and interrupt lock.
 *      This function should be called before entering a critical section.
 * INPUT:
 *      lock_p  -  spin lock
 * OUTPUT:
 *      None.
 * RETURN:
 *      Interrupt mask returned by SYSFUN_InterruptLock(). This return value
 *      must be passed as argument "oint" of SYSFUN_LeaveCriticalSection().
 * NOTES:
 *      None.
 */
#if 1
#define SYSFUN_EnterCriticalSection spin_lock_irqsave
#else
static inline SYSFUN_IntMask_T SYSFUN_EnterCriticalSection(spinlock_t *lock_p)
{
    SYSFUN_IntMask_T oint;
    oint=SYSFUN_InterruptLock();
    spin_lock(lock_p);
    return oint;
}
#endif

/* FUNCTION NAME : SYSFUN_LeaveCriticalSection
 * PURPOSE:
 *      This function provides a general protection method for the code
 *      in kernel space using spin lock and interrupt lock.
 *      This function should be called when leaving a critical section.
 * INPUT:
 *      lock_p  -  spin lock
 *      oint    -  return value of SYSFUN_EnterCriticalSection().
 * OUTPUT:
 *      None.
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
#if 1
#define SYSFUN_LeaveCriticalSection spin_unlock_irqrestore
#else
static inline void SYSFUN_LeaveCriticalSection(spinlock_t *lock_p, SYSFUN_IntMask_T oint)
{
    spin_unlock(lock_p);
    SYSFUN_InterruptUnlock(oint);
}
#endif

/* FUNCTION NAME : SYSFUN_SetTaskPriority
 * PURPOSE:
 *      Change task's priority.
 *
 * INPUT:
 *      sched_policy -- scheduling policy setting, valid settings are listed below:
 *                         SYSFUN_SCHED_DEFAULT  -- use default policy
 *                         SYSFUN_SCHED_FIFO     -- use first-in-first-out policy
 *                         SYSFUN_SCHED_RR       -- use round-robin policy
 *      tid          -- the task to be changing priority. Tid can be process id
 *                      or thread id on linux.
 *      priority     -- new priority. Valid range is 1-255. 1 is the highest priority.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK   --  OK;
 *      SYSFUN_RESULT_INVALID_ID    -- tid incorrect; failed validity check.
 *
 * NOTES:
 *      1. This function MUST BE carefully using, the different priority will cause
 *         system generate different result. So, if you want to use this function,
 *         please make sure whole system's priority is correct. NOT take mis-order
 *         result.
 */
UI32_T  SYSFUN_SetTaskPriority (UI32_T sched_policy, UI32_T tid, UI32_T priority);

/* FUNCTION NAME : SYSFUN_GetTaskPriority
 * PURPOSE:
 *      Get task's priority.
 *
 * INPUT:
 *      tid     -- the task to get priority. tid can be process id or thread id
 *                 on linux.
 *
 * OUTPUT:
 *      sched_policy -- scheduling policy setting
 *                         SYSFUN_SCHED_DEFAULT  -- default policy
 *                         SYSFUN_SCHED_FIFO     -- first-in-first-out policy
 *                         SYSFUN_SCHED_RR       -- round-robin policy
 *      priority     -- thread priority or process priority. Valid range is 1-255. 1 is the highest priority.
 * RETURN:
 *      SYSFUN_OK   --  OK;
 *      SYSFUN_RESULT_INVALID_ID    -- tid incorrect; failed validity check.
 *      SYSFUN_RESULT_INVALID_ARG   -- invalid arguments
 *      SYSFUN_RESULT_ERROR         -- error
 *
 * NOTES:
 *      1. This function MUST BE carefully using, the different priority will cause
 *         system generate different result. So, if you want to use this function,
 *         please make sure whole system's priority is correct. NOT take mis-order
 *         result.
 */
UI32_T  SYSFUN_GetTaskPriority (UI32_T tid, UI32_T *sched_policy_p, UI32_T *priority_p);

/*-------------------------
 * Semaphore Function
 *-------------------------
 */

/* FUNCTION NAME : SYSFUN_CreateSem
 * PURPOSE:
 *      Create a binary semaphore.
 *
 * INPUT:
 *      sem_key     -- key for the semaphore.
 *                     If this value is set as SYSFUN_SEMKEY_PRIVATE, the created semaphore is
 *                     only valid in the calling process.
 *                     If this valie is set as value other than SYSFUN_SEMKEY_PRIVATE, the
 *                     created semaphore is valid among all processes.
 *      sem_count   -- 0:Empty, 1:Full.
 *      sem_flag    -- fifo or priority
 *                     SYSFUN_SEM_FIFO : FIFO semaphore;
 *                     SYSFUN_SEM_PRIORITY : Priority semaphore;
 *                                           high priority task will get the semaphore first.
 *
 * OUTPUT:
 *      sem_id      -- created semaphore id.
 *
 * RETURN:
 *      SYSFUN_OK   -- Successfully.
 *      SYSFUN_RESULT_INVALID_ARG   -- Invalid argument(s)
 *      SYSFUN_RESULT_NO_SEMAPHORE  -- Exceeds max number of semaphore.
 *      SYSFUN_RESULT_ERROR         -- Error
 *
 * NOTES:
 *      1. The task ID will be invalid if called at interrupt level.
 *      2. On linux, sem_flag won't take effect.
 *      3. If a semaphore already exists for the key, creation will be failed.
 */
UI32_T SYSFUN_CreateSem (UI32_T sem_key, UI32_T sem_count, UI32_T sem_flag, UI32_T *sem_id);


/* FUNCTION NAME : SYSFUN_GetSem
 * PURPOSE:
 *      Get an existed binary semaphore.
 *
 * INPUT:
 *      sem_key     -- key for the semaphore; must be > 0.
 *                     SYSFUN_SEMKEY_PRIVATE is invalid.
 *
 * OUTPUT:
 *      sem_id      -- created semaphore id.
 *
 * RETURN:
 *      SYSFUN_OK   -- Successfully.
 *      SYSFUN_RESULT_NO_SEMAPHORE  -- No semaphore exists for the key.
 *      SYSFUN_RESULT_ERROR         -- Error
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_GetSem (UI32_T sem_key, UI32_T *sem_id);


/* FUNCTION NAME : SYSFUN_DestroySem
 * PURPOSE:
 *      Destroy a created semaphore.
 *
 * INPUT:
 *      sem_id      -- semaphore ID to destroy.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK    -- Successfully.
 *      SYSFUN_RESULT_INVALID_ID    -- Invalid sem ID.
 *
 * NOTES:
 *      None.
 */
UI32_T  SYSFUN_DestroySem(UI32_T sem_id);


/* FUNCTION NAME : SYSFUN_TakeSem
 * PURPOSE:
 *      Wait for semaphore. (Take or P operation)
 *
 * INPUT:
 *      sem_id      -- semaphore ID to wait.
 *      timeout     -- waiting time;
 *                     0x00 : No wait.
 *                     SYSFUN_TIMEOUT_WAIT_FOREVER : Wait Forever
 *                     other: waiting time.
 *
 * OUTPUT:  none
 *
 * RETURN:
 *      SYSFUN_OK    -- Successfully.
 *      SYSFUN_RESULT_TIMEOUT    -- Timeout.
 *      SYSFUN_RESULT_OBJ_DELETED    -- Semaphore been deleted.
 *      SYSFUN_RESULT_INVALID_ID    -- Invalid sem ID.
 *      SYSFUN_RESULT_SEM_NOSIGNAL    -- No semaphore be signal when No_Wait is called.
 *      SYSFUN_RESULT_SEM_NOSIGNAL    -- Semaphore be deleted when waiting semaphore.
 *      SYSFUN_RESULT_ERROR           -- Error
 *
 * NOTES:
 *      1. This function MUST NOT BE called by an ISR.
 *
 */
UI32_T  SYSFUN_TakeSem (UI32_T sem_id, int timeout);


/* FUNCTION NAME : SYSFUN_GiveSem
 * PURPOSE:
 *      Signal for semaphore. (Give or V operation)
 *
 * INPUT:
 *      sem_id      -- semaphore ID to signal(Give or V).
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK    -- Successfully.
 *      SYSFUN_RESULT_OBJ_DELETED    -- Semaphore been deleted.
 *      SYSFUN_RESULT_INVALID_ID    -- Invalid sem ID.
 *      SYSFUN_RESULT_ERROR         -- Error
 *
 * NOTES:
 *      None.
 */
UI32_T  SYSFUN_GiveSem(UI32_T sem_id);


/* FUNCTION NAME : SYSFUN_LogMsg
 * PURPOSE:
 *      Log error message to system log, only one text length less than 56.
 *
 * INPUT:
 *      msg_text-- message (text) body.
 *      arg1 ~ arg6 - argument.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This function only log one text message to syslog.
 *		2. There is some differenct level message, user can use it.
 */
void SYSFUN_LogMsg(char *msg_text, I32_T arg1, I32_T arg2, I32_T arg3, I32_T arg4, I32_T arg5, I32_T arg6);

void SYSFUN_LogUrgentMsg(char *msg_text);

void SYSFUN_LogAlertMsg(char *msg_text);

void SYSFUN_LogCriticalMsg(char *msg_text);

void SYSFUN_LogErrMsg(char *msg_text);

void SYSFUN_LogWarnMsg(char *msg_text);

void SYSFUN_LogNoticeMsg(char *msg_text);

void SYSFUN_LogInfoMsg(char *msg_text);

void SYSFUN_LogDebugMsg(char *msg_text);

/*-------------------------
 * Event Function
 *-------------------------
 */

/* FUNCTION NAME : SYSFUN_SendEvent
 * PURPOSE:
 *      Send events to a task, if the task is waiting for event,
 *      the task will be waked up when the condition is satified.
 *
 * INPUT:
 *      tid     - the task identifier of the target task.
 *      event  - a list of bit-encoded events.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK                   - Successfully.
 *      SYSFUN_RESULT_OBJ_DELETED   - Task has beed deleted.
 *      SYSFUN_RESULT_INVALID_ID    - tid is incorrect
 *      SYSFUN_RESULT_INVALID_ARG   - no event to be set.
 *      SYSFUN_RESULT_SYSFUN_NOT_INIT - SYSFUN not Init. yet.
 *
 * NOTES:
 *      1. Bit-0 to bit-15 is available for use; bit-16 to bit-31 is reserved
 *         for system usage. (Compatbile with pSos).
 */
UI32_T SYSFUN_SendEvent (UI32_T tid, UI32_T event);

/* FUNCTION NAME : SYSFUN_ReceiveEvent
 * PURPOSE:
 *      Receive one or more events.
 *
 * INPUT:
 *      wait_events - the set of events.
 *      flags       - the event processing attributes.
 *                    SYSFUN_EVENT_WAIT_ALL  -- Wait for all event.
 *                    SYSFUN_EVENT_WAIT_ANY  -- Wait for any one event.
 *      timeout     - timeout in units of ticks,
 *                     SYSFUN_TIMEOUT_NOWAIT       - no wait.
 *                     SYSFUN_TIMEOUT_WAIT_FOREVER - wait forever
 *                     others                      - timeout ticks.
 *
 * OUTPUT:
 *      received_events - Points to the variable where SYSFUN_ReceiveEvent ()
 *                        stores the actual events captured.
 *
 * RETURN:
 *      SYSFUN_OK                   - Successfully.
 *      SYSFUN_RESULT_TIMEOUT       - Timed out for waiting a time.
 *      SYSFUN_RESULT_NO_EVENT_SET  - Selected events not occured for flag is NoWait.
 *      SYSFUN_RESULT_CALL_FROM_ISR - Can't call from ISR.
 *      SYSFUN_RESULT_SYSFUN_NOT_INIT - SYSFUN not Init. yet.
 *
 * NOTES:
 *      1. When SYSFUN_EventReceive is called, previous waiting events is cleared.
 *      2. EV_ALL - all the waiting events are satisfied,
 *         EV_ANY - any one of waiting event is satisfied.
 *      3. If the selected event condition is satisfied by events already
 *         pending, SYSFUN_ReceiveEvent () clears those events and returns.
 *      4. For unknown result, all return Timeout if no better status could be return.
 *
 */
UI32_T SYSFUN_ReceiveEvent (UI32_T wait_events,
                            UI32_T flags,
                            int timeout,
                            UI32_T *received_events);

/*-------------------------
 * Memory Copy Function
 *-------------------------
 */

/* FUNCTION NAME : SYSFUN_CopyFromUser
 * PURPOSE:
 *      This function is copy a memory area in user-space to a memory area in kernel-space.
 * INPUT:
 *      src_p  - original memory area. (in user-space)
 *      size   - the size of destined memory area.
 *
 * OUTPUT:
 *      dst_p  - destined memory area.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void SYSFUN_CopyFromUser(void *dst_p, void *src_p, UI32_T size);

/* FUNCTION NAME : SYSFUN_CopyToUser
 * PURPOSE:
 *      This function is copy a memory area in kernel-space to a memory area in user-space.
 * INPUT:
 *      src_p  - original memory area. (in kernel-space)
 *      size   - the size of destined memory area.
 *
 * OUTPUT:
 *      dst_p  - destined memory area. (in user-space)
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      None.
 */
void SYSFUN_CopyToUser(void *dst_p, void *src_p, UI32_T size);

/*-------------------------
 * Syscall-related
 *-------------------------
 */

/* FUNCTION NAME : SYSFUN_RegisterCallBackFunc
 * PURPOSE:
 *      This function is to register callback function for specific command of
 *      system call.
 * INPUT:
 *      syscall_cmd_id  - the command id.
 *      func            - the callback function.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      The limitations of callback function:
 *      1. The number of argument of callback function must be less than or equal to 5.
 *      2. The type of return value of callback function is UI32_T.
 */
void SYSFUN_RegisterCallBackFunc(UI32_T syscall_cmd_id, void *func);

/*--------------------------
 *  System Resource
 *--------------------------
 */

/* FUNCTION NAME : SYSFUN_GetCpuUsage
 * PURPOSE:
 *      Get CPU busy/idle accumulated ticks, from system start or restart.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      busy_ticks_p  - the accumulated busy ticks.
 *      idle_ticks_p  - the accumulated idle ticks.
 *
 * RETURN:
 *      SYSFUN_OK                     -- Successfully.
 *      SYSFUN_RESULT_INVALID_ARG     -- Failed.
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_GetCpuUsage(UI32_T *busy_ticks_p, UI32_T *idle_ticks_p);

/* FUNCTION NAME : SYSFUN_GetMemoryUsage
 * PURPOSE:
 *      Get usable/available memory size.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      total_bytes_p - total usable memory size.
 *      free_bytes_p  - available memory size.
 *
 * RETURN:
 *      SYSFUN_OK                     -- Successfully.
 *      SYSFUN_RESULT_INVALID_ARG     -- Failed.
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_GetMemoryUsage(MEM_SIZE_T *total_bytes_p, MEM_SIZE_T *free_bytes_p);

/* FUNCTION NAME : SYSFUN_GetTaskIdList
 * PURPOSE:
 *      Get list of current task id.
 *
 * INPUT:
 *      task_id_ar    - the buffer to receive task id list.
 *      task_id_num_p - the max number of task id can be stored in task_id_ar
 *
 * OUTPUT:
 *      task_id_ar    - the task id list.
 *      task_id_num_p - the max of task id stored in task_id_ar
 *
 * RETURN:
 *      SYSFUN_OK                     -- Successfully.
 *      SYSFUN_RESULT_INVALID_ARG     -- Failed.
 *
 * NOTES:
 *      Gives task_id_ar == NULL to
 *      get the number of all task id without filling task_id_ar
 */
UI32_T SYSFUN_GetTaskIdList(UI32_T *task_id_ar, UI32_T *task_id_num_p);

/* FUNCTION NAME : SYSFUN_GetCpuUsageByTaskId
 * PURPOSE:
 *      Get CPU accumulated ticks of specified task.
 *
 * INPUT:
 *      task_id       - specified task id.
 *
 * OUTPUT:
 *      user_ticks_p  - user CPU time used.
 *      sys_ticks_p   - system CPU time used
 *
 * RETURN:
 *      SYSFUN_OK                     -- Successfully.
 *      SYSFUN_RESULT_OBJ_DELETED     -- Task has beed deleted.
 *      SYSFUN_RESULT_INVALID_ID      -- specified ID is not valid.
 *      SYSFUN_RESULT_INVALID_ARG     -- Failed.
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_GetCpuUsageByTaskId(UI32_T task_id, UI32_T *user_ticks_p, UI32_T *sys_ticks_p);

#endif /* _SYSFUN_H */
