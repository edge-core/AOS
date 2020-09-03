/* MODULE NAME:  sysfun.c (Kernel Space)
 * PURPOSE:
 *      This module is unified system call for all platforms.
 * NOTES:
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
 * Copyright(C)      Accton Corporation, 1999, 2000, 2001, 2002, 2004, 2006
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <linux/version.h>
#include "sys_hwcfg.h"

/* linux specific include files
 */
#include <linux/jiffies.h>  /* jiffies, jiffies_to_clock_t(), clock_t_to_jiffies() */
#include <linux/kernel.h>   /* dump_stack() */
#include <linux/sched.h>    /* for current, jiffies */
#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/timer.h>    /* for timer */
#include <linux/kernel_stat.h> /* for kstat_cpu, ... */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
#include <linux/semaphore.h> /* for semaphore */
#else
#include <asm/semaphore.h>  /* for semaphore */
#endif
#include <asm/uaccess.h>    /* for copy_from_user(), copy_to_user() */

/* linux includes files for specific hardware
 */
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
#include <asm/reg.h>        /* mtmsr(), mfspr(), mtspr() */
#endif

#ifdef SYS_HWCFG_CPU_MIPS
#include <asm/pgtable.h>
#include <asm/mipsregs.h>
#endif

#include <linux/syscalls.h>

/* Accton include files
 */
#include "sys_type.h"
#include "k_syscall.h"
#include "k_sysfun.h"
#include "l_cvrt.h"
#include "l_pt.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SYSFUN_DEBUG  FALSE


/* SYSFUN_LOGLEVEL_XXX are used for the argument log_level
 * of the macro function SYSFUN_LOGMSG()
 */
#define SYSFUN_LOGLEVEL_DEFAULT SYSFUN_LOGLEVEL_WARN
#define SYSFUN_LOGLEVEL_URGENT     KERN_EMERG
#define SYSFUN_LOGLEVEL_ALERT      KERN_ALERT
#define SYSFUN_LOGLEVEL_CRITICAL   KERN_CRIT
#define SYSFUN_LOGLEVEL_ERR        KERN_ERR
#define SYSFUN_LOGLEVEL_WARN       KERN_WARNING
#define SYSFUN_LOGLEVEL_NOTICE     KERN_NOTICE
#define SYSFUN_LOGLEVEL_INFO       KERN_INFO
#define SYSFUN_LOGLEVEL_DEBUG      KERN_DEBUG

/* MACRO FUNCTION DECLARATIONS
 */
/* MACRO FUNCTION NAME : SYSFUN_LOGMSG
 * PURPOSE:
 *         Macro used by SYSFUN_LogXXX APIs.
 *
 * INPUT:
 *      log_level  -  log level, valid settings are listed below:
 *                    SYSFUN_LOGLEVEL_URGENT
 *                    SYSFUN_LOGLEVEL_ALERT
 *                    SYSFUN_LOGLEVEL_CRITICAL
 *                    SYSFUN_LOGLEVEL_ERR
 *                    SYSFUN_LOGLEVEL_WARN
 *                    SYSFUN_LOGLEVEL_NOTICE
 *                    SYSFUN_LOGLEVEL_INFO
 *                    SYSFUN_LOGLEVEL_DEBUG
 *      msg        -  message to be logged
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1.Use printk until SYSLOG_MGR is ready
 */
#define SYSFUN_LOGMSG(log_level, msg, arg...) \
({                                            \
    printk(log_level msg "\n", ##arg);        \
})

#if SYSFUN_DEBUG
#define SYSFUN_DEBUG_Print printk
#else
#define SYSFUN_DEBUG_Print(...)
#endif

/* DATA TYPE DECLARATIONS
 */
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
struct my_timer_rec {
    struct timer_list   timer_lst;
    unsigned long       data;
};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
    static void SYSFUN_WakeUpRecvEventTimeout(struct timer_list *t);
#else
    static void SYSFUN_WakeUpRecvEventTimeout(unsigned long event_sem);
#endif

static UI32_T SYSFUN_Operation(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4);
static BOOL_T SYSFUN_SuspendThreadSelf (void);
static UI32_T SYSFUN_ResumeThread (UI32_T task_id);
static BOOL_T SYSFUN_SuspendTaskWithTimeout(UI32_T timeout);
static BOOL_T SYSFUN_ResumeTaskInTimeoutSuspense (UI32_T task_id);
static void SYSFUN_InitUTCB(char *task_name, BOOL_T is_ui_task);
static void SYSFUN_DestroyUTCB(void);
static UI32_T SYSFUN_GetUserTcb(UI32_T task_id, SYSFUN_UserTcb_T **utcb_p);
static UI8_T SYSFUN_AllocateEHBufferIndex(void);
static void SYSFUN_FreeEHBufferIndex(UI8_T eh_buf_idx);
static void SYSFUN_DumpEHBufInfo(void);
static UI32_T SYSFUN_GetEHBufIndex(UI8_T *eh_buf_idx_p);
static UI32_T SYSFUN_SetEHBufIndex(UI8_T eh_buf_idx);
static UI32_T SYSFUN_InvalidateEHBufIndex(void);

/* STATIC VARIABLE DECLARATIONS
 */
/* l_pt descriptor for EH Buffer Index allocation
 */
static L_PT_Descriptor_T eh_buf_idx_pt_desc;
/* EH Buffer Index allocation buffer for eh_buf_idx_pt_desc
 */
static UI32_T eh_buf_idx_pool[SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK];


/* EXPORTED SUBPROGRAM BODIES
 */

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
void SYSFUN_Init ()
{
    /* jerry.du 20080918, add new system call sys_reboot */
    /* charlie_chen 20100728, sys_reboot is exported as GPL symbol
     * and can only be used by GPL kernel module. In order to
     * change acctonlkm as priorietary license, this function
     * call need to be removed.
     */
#if 0
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_REBOOT, sys_reboot);
#endif

    /* Initialize eh_buf_idx_pt_desc for EH Buffer Index allocation
     */
    eh_buf_idx_pt_desc.buffer=(char*)(&eh_buf_idx_pool[0]);
    eh_buf_idx_pt_desc.buffer_len=sizeof(eh_buf_idx_pool);
    eh_buf_idx_pt_desc.partition_len=sizeof(UI32_T);
    L_PT_Create(&eh_buf_idx_pt_desc);

    return;
}

/* FUNCTION NAME : SYSFUN_Create_InterCSC_Relation
 * PURPOSE: Create inter-CSC relationships.
 * INPUT:
 *      None.
 * OUTPUT:
 *      None.
 * RETURN:
 *      None.
 * NOTES:
 *      None.
 */
void SYSFUN_Create_InterCSC_Relation(void)
{
    SYSFUN_RegisterCallBackFunc(SYSFUN_SYSCALL_INTERNAL, SYSFUN_Operation);
}

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
UI32_T SYSFUN_TaskIdSelf (void)
{
    return (UI32_T)current->pid;
}

/* FUNCTION NAME : SYSFUN_TaskNameToID
 * PURPOSE:
 *      Get the numeric identifier of a named task.
 * INPUT:
 *      task_name - the name of task to be searched, if no task name
 *                  specified, i.e. NULL, return current task ID.
 *
 * OUTPUT:
 *      task_id - the specified task ID.
 *
 * RETURN:
 *      SYSFUN_OK                   - Successfully.
 *      SYSFUN_RESULT_NO_NAMED_TASK - Named task was not found.
 *
 * NOTES:
 *      1. If there are duplicate-named tasks, SYSFUN_TaskNameToID()
 *         always returns the first task created with the duplicate name.
 */
UI32_T  SYSFUN_TaskNameToID (char *task_name, UI32_T *task_id)
{
    struct task_struct *task_p;
    struct task_struct *task_g;

    read_lock(&tasklist_lock);
    /*for_each_process(task_p)  this will only traverse each process */
    do_each_thread(task_g, task_p) /* traverse each threads */
    {
        if (strncmp(task_p->utcb.task_name, task_name, sizeof(task_p->utcb.task_name)) == 0)
        {
            *task_id = task_p->pid;
            read_unlock(&tasklist_lock);
            return SYSFUN_OK;
        }
    } while_each_thread(task_g, task_p);

    read_unlock(&tasklist_lock);
    return SYSFUN_RESULT_NO_NAMED_TASK;
}

/* FUNCTION NAME : SYSFUN_TaskIDToName
 * PURPOSE:
 *      Retrieve Task name by task-id.
 * INPUT:
 *      task_id - the specified task ID.
 *                0 : the caller task.
 *      size    - the size of buffer (task_name) to receive task name.
 *
 * OUTPUT:
 *      task_name - Address of pointer which point to name of task retrieved.
 *
 * RETURN:
 *      SYSFUN_OK                   - Successfully.
 *      SYSFUN_RESULT_INVALID_ID    - specified ID is not valid.
 *      SYSFUN_RESULT_NO_NAMED_TASK - specified task was no name.
 *      SYSFUN_RESULT_INVALID_ARG   - not specified task name buffer space.
 *
 * NOTES:
 *      1. If specified buf_size is smaller than task-name, task-name will be
 *         truncated to fit the size (buf_size-1).
 *      2. On linux, task id could be thread id or process id.
 */
UI32_T  SYSFUN_TaskIDToName (UI32_T task_id, char *task_name, UI32_T size)
{
    SYSFUN_UserTcb_T *utcb_p;

    if (SYSFUN_GetUserTcb(task_id, &utcb_p) == SYSFUN_OK)
    {
        strncpy(task_name, utcb_p->task_name, size);
        task_name[size-1] = 0;
        return SYSFUN_OK;
    }

    return SYSFUN_RESULT_INVALID_ID;
}

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
void SYSFUN_NonPreempty (void)
{
    lock_kernel();/*need to fix */
}

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
void SYSFUN_Preempty (void)
{
    unlock_kernel();/*need to fix */
}
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
UI32_T  SYSFUN_GetSysTick (void)
{
/* It is better to call sys_times(), however, it is exported by linux kernel
 * as GPL symbol. To keep acctonlkm as priorietary license, we cannot but
 * envaulate system tick by ourselves.
 */
#if 1
    return (UI32_T)jiffies_64_to_clock_t(get_jiffies_64());
#else
    /* invoke linux kernel API to minimize porting effort */
    return (UI32_T)sys_times(NULL);
#endif
}

// TODO: not support
#if 0
/* FUNCTION NAME : SYSFUN_SetSysTick
 * PURPOSE:
 *      Set system ticks.
 * INPUT:
 *      UI32_T  tick
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
void SYSFUN_SetSysTick(UI32_T tick)
{
    jiffies = (unsigned long)tick;
}
#endif

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
#if 0
SYSFUN_IntMask_T SYSFUN_InterruptLock(void)
{
    SYSFUN_IntMask_T int_mask;

    local_irq_save(int_mask);
    return int_mask;
}
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
#if 0
void SYSFUN_InterruptUnlock(SYSFUN_IntMask_T int_mask)
{
    local_irq_restore(int_mask);
}
#endif

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
 *      2. There is some differenct level message, user can use it.
 */
void SYSFUN_LogMsg(char *msg_text, I32_T arg1, I32_T arg2, I32_T arg3, I32_T arg4, I32_T arg5, I32_T arg6)
{
    printk(msg_text, arg1, arg2, arg3, arg4, arg5, arg6);
}

void SYSFUN_LogUrgentMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_URGENT, "%s", msg_text);
}

void SYSFUN_LogAlertMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_ALERT, "%s", msg_text);
}

void SYSFUN_LogCriticalMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_CRITICAL, "%s", msg_text);
}

void SYSFUN_LogErrMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_ERR, "%s", msg_text);
}

void SYSFUN_LogWarnMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_WARN, "%s", msg_text);
}

void SYSFUN_LogNoticeMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_NOTICE, "%s", msg_text);
}

void SYSFUN_LogInfoMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_INFO, "%s", msg_text);
}

void SYSFUN_LogDebugMsg(char *msg_text)
{
    SYSFUN_LOGMSG(SYSFUN_LOGLEVEL_DEBUG, "%s", msg_text);
}

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
UI32_T SYSFUN_SendEvent (UI32_T tid, UI32_T event)
{
    /* LOCAL VARIABLE DECLARATION
     */
    SYSFUN_IntMask_T int_mask;
    struct task_struct *target_task_p;
    SYSFUN_UserTcb_T   *utcb;

    /* BODY */
    if (0 == tid)
        return SYSFUN_RESULT_INVALID_ID;
    if (0 == event)
        return SYSFUN_RESULT_INVALID_ARG;

    write_lock_irqsave(&tasklist_lock, int_mask);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    target_task_p = find_task_by_vpid(tid);
#else
    target_task_p = find_task_by_pid(tid);
#endif
    if(target_task_p==NULL)
    {

        write_unlock_irqrestore(&tasklist_lock, int_mask);
        return SYSFUN_RESULT_OBJ_DELETED;
    }

    utcb = &(target_task_p->utcb);

    utcb->event_set |= event;
    SYSFUN_DEBUG_Print("\r\n<7>%s: %d to %lu, [event] send: %081x now: %081x", __FUNCTION__, current->pid, tid, event, utcb->event_set);

    if (utcb->event_waited )
    {
        event = utcb->event_set & utcb->event_waited;
        if (utcb->event_is_all)
        {
            if (utcb->event_waited  == event)
                up(&(utcb->event_sem));
        }
        else
        {
            if (event != 0)
                up(&(utcb->event_sem));
        }
    }

    write_unlock_irqrestore(&tasklist_lock, int_mask);

    return  SYSFUN_OK;

}   /* End of SYSFUN_SendEvent () */

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
 *                     SYSFUN_TIMEOUT_NOWAIT       - no wait
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
UI32_T SYSFUN_ReceiveEvent (UI32_T wait_events, UI32_T flags, int timeout,
                            UI32_T *received_events)
{
    /* LOCAL VARIABLE DECLARATION
     */
    SYSFUN_IntMask_T  int_mask;
    UI32_T    all, event;
    SYSFUN_UserTcb_T *utcb = &(current->utcb);
    int       ret;
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
    struct    my_timer_rec timer;
#else
    struct    timer_list timer;
#endif
    unsigned long expired_time;

    /* BODY */
    if ( wait_events == 0 )
        return SYSFUN_RESULT_NO_EVENT_SET;

    all = (flags == SYSFUN_EVENT_WAIT_ALL);

    *received_events = 0;

    expired_time = jiffies + clock_t_to_jiffies((unsigned long)timeout);

RE_GET:
    write_lock_irqsave(&tasklist_lock, int_mask);

    event = utcb->event_set & wait_events;
    if ( (all && (event == wait_events)) || (!all && event!=0) )
    {
        /* wait event is satisfied */
        utcb->event_set ^= (*received_events = event);
        utcb->event_waited = 0;
        SYSFUN_DEBUG_Print("\r\n<7>%s: %d, [event] wait: %08lx  recv: %08lx now: %08lx", __FUNCTION__, current->pid, wait_events, *received_events, utcb->event_set);
        write_unlock_irqrestore(&tasklist_lock, int_mask);

        return SYSFUN_OK;
    }

    /* wait event is not satisfied
     */
    if (timeout==0)     /*  No wait */
    {
        write_unlock_irqrestore(&tasklist_lock, int_mask);
        return  SYSFUN_RESULT_TIMEOUT;
    }

    /* 1. Ready to wait semaphore
     * 2. save wait condition
     */
    utcb->event_waited = wait_events;
    utcb->event_is_all = all;

    /* When the process run to here, it means the condition does not meet
     * in previous if block. We need to clear the semaphore here to assure
     * that the semaphore is set due to the condition meets.
     */
    while (down_trylock(&(utcb->event_sem)) == 0);    /* clear the semaphore */

    write_unlock_irqrestore(&tasklist_lock, int_mask);


    /* need to add a timer to wake up the blocked process
     * if its timeout setting is not "wait forever".
     */
    if(timeout!=(int)SYSFUN_TIMEOUT_WAIT_FOREVER)
    {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
        timer.data = (unsigned long)&(utcb->event_sem);
        timer.timer_lst.expires  = expired_time;
        timer_setup(&timer.timer_lst, SYSFUN_WakeUpRecvEventTimeout, 0);
#else
        init_timer(&timer);
        timer.data     = (unsigned long)&(utcb->event_sem);
        timer.function = SYSFUN_WakeUpRecvEventTimeout;
        timer.expires  = expired_time;
        add_timer(&timer);
#endif
    }

    /* Race condition will occur, but no harmful!
     */
    ret = down_interruptible(&(utcb->event_sem));

    /* The timer might have not been activated yet.
     * Just clean it up here. Nothing wrong even if the timer has been activated.
     */
    if(timeout!=(int)SYSFUN_TIMEOUT_WAIT_FOREVER)
    {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
        del_timer(&timer.timer_lst);
#else
        del_timer(&timer);
#endif
    }

    if(ret==-EINTR)
        return SYSFUN_RESULT_EINTR;

    /* set timeout as 0
     * so the process will be timeout when going to RE_GET
     */
    timeout=0;
    goto RE_GET;
}   /* End of SYSFUN_ReceiveEvent() */

/* FUNCTION NAME : SYSFUN_Debug_DumpTaskStack
 * PURPOSE:
 *      This api will dump the call stack of the caller.
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      Only address of the called functions will be displayed.
 *      It is required to understand the shown call stack with the help of
 *      objdumpppc(For PPC) to know the called functions.
 */
void SYSFUN_Debug_DumpTaskStack(void)
{
    dump_stack();
}

/* FUNCTION NAME : SYSFUN_SetMasterStatusRegister
 * PURPOSE:
 *      Set Master Status Register
 * INPUT:
 *      UI32 val - Value to set to Master Status Register
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
void SYSFUN_SetMasterStatusRegister(UI32_T val)
{
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
    mtmsr(val);
#endif
}

/* FUNCTION NAME : SYSFUN_EnableDataCache
 * PURPOSE:
 *      Enable CPU's data cache.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UI32_T
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_EnableDataCache(void)
{
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
    mtspr(SPRN_HID0, mfspr(SPRN_HID0) | HID0_DCE);
/*
    unsigned long hid0;

    __asm__ __volatile__(
        "mfspr %0, 0x3F0\n\t"
        "ori %0, %0, 0x4000\n\t"
        "sync\n\t"
        "mtspr 0x3F0, %0"
        : "=r" (hid0));
 */
#endif

#ifdef SYS_HWCFG_CPU_MIPS
/* MIPS32 CPU with standard TLB MMU uses the K0 field in CP0 Config register
 * to control the cache attribute of Kseg0. The cache attribute of useg/kuseg
 * is determined by TLB. The K0 field controls both i-cache and d-cache.
 * This implementation will turn on the cache on Kseg0.
 */

    change_c0_config(CONF_CM_CMASK, PAGE_CACHABLE_DEFAULT>>9);
#endif
    return SYSFUN_OK;
}

/* FUNCTION NAME : SYSFUN_DisableDataCache
 * PURPOSE:
 *      Disable CPU's data cache.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UI32_T
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_DisableDataCache(void)
{
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
    mtspr(SPRN_HID0, mfspr(SPRN_HID0) & ~HID0_DCE);
#endif

#ifdef SYS_HWCFG_CPU_MIPS
/* MIPS32 CPU with standard TLB MMU uses the K0 field in CP0 Config register
 * to control the cache attribute of Kseg0. The cache attribute of useg/kuseg
 * is determined by TLB. The K0 field controls both i-cache and d-cache.
 * This implementation will turn off the cache on Kseg0.
 */
    change_c0_config(CONF_CM_CMASK, _CACHE_UNCACHED>>9);
#endif

    return SYSFUN_OK;
}

/* FUNCTION NAME : SYSFUN_EnableInstructionCache
 * PURPOSE:
 *      Enable CPU's instruction cache.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UI32_T
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_EnableInstructionCache(void)
{
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
    mtspr(SPRN_HID0, mfspr(SPRN_HID0) | HID0_ICE);
/*
    unsigned long hid0;

    __asm__ __volatile__(
        "mfspr %0, 0x3F0\n\t"
        "ori %0, %0, 0x8000\n\t"
        "sync\n\t"
        "mtspr 0x3F0, %0"
        : "=r" (hid0));
 */
#endif

#ifdef SYS_HWCFG_CPU_MIPS
/* MIPS32 CPU with standard TLB MMU uses the K0 field in CP0 Config register
 * to control the cache attribute of Kseg0. The cache attribute of useg/kuseg
 * is determined by TLB. The K0 field controls both i-cache and d-cache.
 * This implementation will turn on the cache on Kseg0.
 */
    SYSFUN_EnableDataCache();
#endif
    return SYSFUN_OK;
}

/* FUNCTION NAME : SYSFUN_DisableInstructionCache
 * PURPOSE:
 *      Disable CPU's instruction cache.
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      UI32_T
 *
 * NOTES:
 *      None.
 */
UI32_T SYSFUN_DisableInstructionCache(void)
{
#if defined (SYS_HWCFG_CPU_82XX) || defined (SYS_HWCFG_CPU_85XX)
    mtspr(SPRN_HID0, mfspr(SPRN_HID0) & ~HID0_ICE);
#endif
#ifdef SYS_HWCFG_CPU_MIPS
/* MIPS32 CPU with standard TLB MMU uses the K0 field in CP0 Config register
 * to control the cache attribute of Kseg0. The cache attribute of useg/kuseg
 * is determined by TLB. The K0 field controls both i-cache and d-cache.
 * This implementation will turn off the cache on Kseg0.
 */
    SYSFUN_DisableDataCache();
#endif

    return SYSFUN_OK;
}

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
void SYSFUN_CopyFromUser(void *dst_p, void *src_p, UI32_T size)
{
    copy_from_user(dst_p, src_p, size);
}

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
void SYSFUN_CopyToUser(void *dst_p, void *src_p, UI32_T size)
{
    copy_to_user(dst_p, src_p, size);
}

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
void SYSFUN_RegisterCallBackFunc(UI32_T syscall_cmd_id, void *func)
{
    SYSCALL_RegisterCallBackFunc(syscall_cmd_id, func);
}

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
UI32_T SYSFUN_GetCpuUsage(UI32_T *busy_ticks_p, UI32_T *idle_ticks_p)
{
    UI32_T user, nice, system, idle, iowait, irq, softirq, steal;
    UI32_T total;
    int i;

    user = nice = system = idle = iowait = irq = softirq = steal = 0;

    for_each_possible_cpu(i)
    {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,0,0))
        user += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_USER]);
        nice += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_NICE]);
        system += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM]);
        idle += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IDLE]);
        iowait += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT]);
        irq += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IRQ]);
        softirq += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ]);
        steal += nsec_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_STEAL]);

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,13))
        user += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_USER]);
        nice += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_NICE]);
        system += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM]);
        idle += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IDLE]);
        iowait += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT]);
        irq += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_IRQ]);
        softirq += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ]);
        steal += cputime64_to_clock_t(kcpustat_cpu(i).cpustat[CPUTIME_STEAL]);
#else
        user += cputime64_to_clock_t(kstat_cpu(i).cpustat.user);
        nice += cputime64_to_clock_t(kstat_cpu(i).cpustat.nice);
        system += cputime64_to_clock_t(kstat_cpu(i).cpustat.system);
        idle += cputime64_to_clock_t(kstat_cpu(i).cpustat.idle);
        iowait += cputime64_to_clock_t(kstat_cpu(i).cpustat.iowait);
        irq += cputime64_to_clock_t(kstat_cpu(i).cpustat.irq);
        softirq += cputime64_to_clock_t(kstat_cpu(i).cpustat.softirq);
        steal += cputime64_to_clock_t(kstat_cpu(i).cpustat.steal);
#endif
    }

    total = user + nice + system + idle + iowait + irq + softirq + steal;

    *idle_ticks_p = idle + iowait;
    *busy_ticks_p = total - *idle_ticks_p;

    return SYSFUN_OK;
}

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
UI32_T SYSFUN_GetMemoryUsage(MEM_SIZE_T *total_bytes_p, MEM_SIZE_T *free_bytes_p)
{
/* convert to bytes
 */
#define B(x) ((x) << PAGE_SHIFT)

    struct sysinfo si;

    si_meminfo(&si);

    *total_bytes_p = B(si.totalram);
    *free_bytes_p = B(si.freeram);

    return SYSFUN_OK;
#undef B
}

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
UI32_T SYSFUN_GetTaskIdList(UI32_T *task_id_ar, UI32_T *task_id_num_p)
{
    struct task_struct *task_p;
    struct task_struct *task_g;
    UI32_T n = 0;

    read_lock(&tasklist_lock);

    if (task_id_ar == NULL)
    {
        /* get the number of all task id without filling task_id_ar
         */
        do_each_thread(task_g, task_p)
        {
            if (task_p->utime > 0 || task_p->stime > 0)
            {
                n++;
            }
        }
        while_each_thread(task_g, task_p);
    }
    else
    {
        do_each_thread(task_g, task_p)
        {
            if (n >= *task_id_num_p)
            {
                goto exit_each_thread;
            }

            if (task_p->utime > 0 || task_p->stime > 0)
            {
                task_id_ar[n++] = task_p->pid;
            }
        }
        while_each_thread(task_g, task_p);
    }

exit_each_thread:
    read_unlock(&tasklist_lock);

    *task_id_num_p = n;

    return SYSFUN_OK;
}

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
UI32_T SYSFUN_GetCpuUsageByTaskId(UI32_T task_id, UI32_T *user_ticks_p, UI32_T *sys_ticks_p)
{
    struct task_struct *task;
    UI32_T ret = SYSFUN_RESULT_INVALID_ARG;

    if (task_id == 0)
    {
        return SYSFUN_RESULT_INVALID_ID;
    }

 	rcu_read_lock();

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    task = find_task_by_vpid(task_id);
#else
    task = find_task_by_pid(task_id);
#endif

    if (task == NULL)
    {
        ret = SYSFUN_RESULT_OBJ_DELETED;
    }
    else
    {
        *user_ticks_p = task->utime;
        *sys_ticks_p = task->stime;
        ret = SYSFUN_OK;
    }

	rcu_read_unlock();

    return ret;
}

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME : SYSFUN_Operation
 * PURPOSE:
 *      This function is to dispatch the actions that must be execute in kernel to
 *       thecorresponding handler by command id.
 * INPUT:
 *      cmd  - the command id.
 *      arg1 ~ arg4 - the arguments for the command.
 *
 * OUTPUT:
 *      Depend on the given cmd.
 *
 * RETURN:
 *      Depend on the given cmd.
 *
 * NOTES:
 *      None.
 */
static UI32_T SYSFUN_Operation(void* arg0, void* arg1, void* arg2, void* arg3, void* arg4)
{
    static UI32_T tmp_task_id_ar[SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM];
    UI32_T tmp_arg, tmp_arg2;
    UI32_T *tmp_p;
    UI32_T ret = SYSFUN_RESULT_INVALID_ARG;
    char tmp_task_name[SYSFUN_TASK_NAME_LENGTH+1];
    int cmd = L_CVRT_PTR_TO_UINT(arg0);

    switch(cmd)
    {
        case SYSFUN_INTERNAL_SYSCALL_VALIDITY:
            SYSFUN_DEBUG_Print("\r\n<7>SYSFUN_INTERNAL_SYSCALL_VALIDITY\r\n");
            ret = SYSFUN_INTERNAL_SYSCALL_VALIDITY_VALUE;
            break;
        case SYSFUN_INTERNAL_SYSCALL_INIT_UTCB:
            /* arg1: char *task_name
             * arg2: BOOL_T is_ui_task
             */
            strncpy_from_user(tmp_task_name, (char *)arg1, sizeof(tmp_task_name));
            tmp_task_name[sizeof(tmp_task_name)-1] = 0;
            SYSFUN_InitUTCB(tmp_task_name, L_CVRT_PTR_TO_UINT(arg2));
            break;
        case SYSFUN_INTERNAL_SYSCALL_DESTROY_UTCB:
            SYSFUN_DestroyUTCB();
            break;
        case SYSFUN_INTERNAL_SYSCALL_TASKNAME_TO_ID:
            strncpy_from_user(tmp_task_name, (char *)arg1, sizeof(tmp_task_name));
            tmp_task_name[sizeof(tmp_task_name)-1] = 0;
            ret = SYSFUN_TaskNameToID(tmp_task_name, &tmp_arg);
            SYSFUN_CopyToUser((void*)arg2, &tmp_arg, sizeof(UI32_T));
            break;
        case SYSFUN_INTERNAL_SYSCALL_TASKID_TO_NAME:
            tmp_arg = L_CVRT_PTR_TO_UINT(arg3);
            tmp_arg = tmp_arg < sizeof(tmp_task_name) ? tmp_arg : sizeof(tmp_task_name);
            ret = SYSFUN_TaskIDToName(L_CVRT_PTR_TO_UINT(arg1), tmp_task_name, tmp_arg);
            SYSFUN_CopyToUser((char *)arg2, tmp_task_name, tmp_arg);
            break;
        case SYSFUN_INTERNAL_SYSCALL_SENDEVENT:
            /* arg1: UI32_T tid
             * arg2: UI32_T event
             * return: UI32_T
             */
            ret = SYSFUN_SendEvent(L_CVRT_PTR_TO_UINT(arg1), L_CVRT_PTR_TO_UINT(arg2));
            break;
        case SYSFUN_INTERNAL_SYSCALL_RECEIVEEVENT:
            /* arg1: UI32_T wait_events
             * arg2: UI32_T flags
             * arg3: int    timeout
             * arg4: UI32_T* received_events
             * return: UI32_T
             */
            ret = SYSFUN_ReceiveEvent(L_CVRT_PTR_TO_UINT(arg1), L_CVRT_PTR_TO_UINT(arg2), L_CVRT_PTR_TO_UINT(arg3), &tmp_arg);
            SYSFUN_CopyToUser((UI32_T *)arg4, &tmp_arg, sizeof(UI32_T));
            break;
#if 0 // TODO: not support
        case SYSFUN_INTERNAL_SYSCALL_SETSYSTICK:
            SYSFUN_SetSysTick(arg1);
            break;
#endif
        case SYSFUN_INTERNAL_SYSCALL_SUSPENDTASKSELF:
            ret = (UI32_T)SYSFUN_SuspendThreadSelf();
            break;
        case SYSFUN_INTERNAL_SYSCALL_RESUMETASK:
            ret = SYSFUN_ResumeThread(L_CVRT_PTR_TO_UINT(arg1));
            break;
        case SYSFUN_INTERNAL_SYSCALL_SUSPENDTASKWITHTIMEOUT:
            ret = (UI32_T)SYSFUN_SuspendTaskWithTimeout(L_CVRT_PTR_TO_UINT(arg1));
            break;
        case SYSFUN_INTERNAL_SYSCALL_RESUMETASKWITHTIMEOUT:
            ret = (UI32_T)SYSFUN_ResumeTaskInTimeoutSuspense(L_CVRT_PTR_TO_UINT(arg1));
            break;
        case SYSFUN_INTERNAL_SYSCALL_DUMPTASKSTACK:
            SYSFUN_Debug_DumpTaskStack();
            break;
        case SYSFUN_INTERNAL_SYSCALL_SETDCACHE:
            if(L_CVRT_PTR_TO_UINT(arg1) == TRUE)
                ret = SYSFUN_EnableDataCache();
            else
                ret = SYSFUN_DisableDataCache();
            break;
        case SYSFUN_INTERNAL_SYSCALL_SETICACHE:
            if(L_CVRT_PTR_TO_UINT(arg1) == TRUE)
                ret = SYSFUN_EnableInstructionCache();
            else
                ret = SYSFUN_DisableInstructionCache();
            break;
        case SYSFUN_INTERNAL_SYSCALL_GET_CPU_USAGE:
            /* arg1: UI32_T *busy_ticks_p
             * arg2: UI32_T *idle_ticks_p
             */
            ret = SYSFUN_GetCpuUsage(&tmp_arg, &tmp_arg2);
            SYSFUN_CopyToUser((UI32_T *)arg1, &tmp_arg, sizeof(UI32_T));
            SYSFUN_CopyToUser((UI32_T *)arg2, &tmp_arg2, sizeof(UI32_T));
            break;
        case SYSFUN_INTERNAL_SYSCALL_GET_MEMORY_USAGE:
            /* arg1: MEM_SIZE_T *total_bytes_p
             * arg2: MEM_SIZE_T *free_bytes_p
             */
            {
                MEM_SIZE_T total_bytes, free_bytes;
                ret = SYSFUN_GetMemoryUsage(&total_bytes, &free_bytes);
                SYSFUN_CopyToUser((MEM_SIZE_T *)arg1, &total_bytes, sizeof(MEM_SIZE_T));
                SYSFUN_CopyToUser((MEM_SIZE_T *)arg2, &free_bytes, sizeof(MEM_SIZE_T));
            }
            break;
        case SYSFUN_INTERNAL_SYSCALL_GETSYSTICK:
            /* arg1: UI32_T *tick
             */
            tmp_arg=SYSFUN_GetSysTick();
            SYSFUN_CopyToUser((UI32_T*)arg1, &tmp_arg, sizeof(UI32_T));
            ret = SYSFUN_OK;
            break;
        case SYSFUN_INTERNAL_SYSCALL_DUMP_EHBUF_INFO:
            SYSFUN_DumpEHBufInfo();
            break;
        case SYSFUN_INTERNAL_SYSCALL_GET_EHBUF_INDEX:
        {
            UI8_T eh_buf_index_local;
            /* arg1: UI8_T *eh_buf_idx_p
             */
            ret = SYSFUN_GetEHBufIndex(&eh_buf_index_local);
            SYSFUN_CopyToUser((UI8_T*)arg1, &eh_buf_index_local, sizeof(UI8_T));
        }
            break;
        case SYSFUN_INTERNAL_SYSCALL_SET_EHBUF_INDEX:
            /* arg1: UI8_T eh_buf_idx
             */
            ret = SYSFUN_SetEHBufIndex(L_CVRT_PTR_TO_UINT(arg1));
            break;
        case SYSFUN_INTERNAL_SYSCALL_INVALIDATE_EHBUF_INDEX:
            ret = SYSFUN_InvalidateEHBufIndex();
            break;
        case SYSFUN_INTERNAL_SYSCALL_GET_TASKID_LIST:
            /* arg1: (IN/OUT) UI32_T *task_id_ar
             * arg2: (IN/OUT) UI32_T *task_id_num_p
             */
            tmp_p = (arg1 == 0) ? NULL : tmp_task_id_ar;
            SYSFUN_CopyFromUser(&tmp_arg, (UI32_T *)arg2, sizeof(UI32_T));
            ret = SYSFUN_GetTaskIdList(tmp_p, &tmp_arg);
            if (arg1 && ret == SYSFUN_OK)
                SYSFUN_CopyToUser((UI32_T *)arg1, tmp_p, sizeof(UI32_T) * tmp_arg);
            SYSFUN_CopyToUser((UI32_T *)arg2, &tmp_arg, sizeof(UI32_T));
            break;
        case SYSFUN_INTERNAL_SYSCALL_GET_CPU_USAGE_BY_TASKID:
            /* arg1: (IN)  UI32_T task_id
             * arg2: (OUT) UI32_T *user_ticks_p
             * arg3: (OUT) UI32_T *sys_ticks_p
             */
            ret = SYSFUN_GetCpuUsageByTaskId(L_CVRT_PTR_TO_UINT(arg1), &tmp_arg, &tmp_arg2);
            SYSFUN_CopyToUser((UI32_T *)arg2, &tmp_arg, sizeof(UI32_T));
            SYSFUN_CopyToUser((UI32_T *)arg3, &tmp_arg2, sizeof(UI32_T));
            break;
        default:
            printk("\r\n<4>%s:Unknown cmd(%d).", __FUNCTION__, cmd);
    }

    return ret;
}

#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
static void SYSFUN_WakeUpRecvEventTimeout(struct timer_list *t)
{
    struct my_timer_rec *trec_p;

    trec_p = from_timer(trec_p, t, timer_lst);

    up((struct semaphore*)trec_p->data);
}
#else
static void SYSFUN_WakeUpRecvEventTimeout(unsigned long event_sem)
{
    up((struct semaphore*)event_sem);
}
#endif

/* FUNCTION NAME : SYSFUN_SuspendThreadSelf
 * PURPOSE:
 *      Suspend the calling thread.
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
BOOL_T SYSFUN_SuspendThreadSelf (void)
{
    SYSFUN_UserTcb_T *utcb_p = &(current->utcb);
    SYSFUN_IntMask_T int_mask;

    /* clear the semaphore
     */
    write_lock_irqsave(&tasklist_lock, int_mask);

    if (down_trylock(&(utcb_p->suspense_sem)) == 0)
    {
        while (down_trylock(&(utcb_p->suspense_sem)) == 0);
        write_unlock_irqrestore(&tasklist_lock, int_mask);
        return TRUE;
    }
    write_unlock_irqrestore(&tasklist_lock, int_mask);

    return (down_interruptible(&(utcb_p->suspense_sem)) != -EINTR);
}

/* FUNCTION NAME : SYSFUN_ResumeThread
 * PURPOSE:
 *      Resume a suspended thread.
 *
 * INPUT:
 *      task_id -- the task be resumed.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK                -- OK, wakeup the task.
 *      SYSFUN_RESULT_INVALID_ID -- task can't be resume, invalid task id.
 *
 * NOTES:
 *      task_id is invalid if the thread and calling thread are not
 *      in the same process.
 */
UI32_T SYSFUN_ResumeThread (UI32_T task_id)
{
    SYSFUN_UserTcb_T *utcb_p;

    if (SYSFUN_GetUserTcb(task_id, &utcb_p) == SYSFUN_OK)
    {
        up(&(utcb_p->suspense_sem));
        return SYSFUN_OK;
    }

    return SYSFUN_RESULT_INVALID_ID;
}
/* FUNCTION NAME : SYSFUN_ResumeTimeoutThread
 * PURPOSE:
 *      Resume a thread suspended by timer.
 *
 * INPUT:
 *      task_id -- the task be resumed.
 *
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      SYSFUN_OK                -- OK, wakeup the task.
 *      SYSFUN_RESULT_INVALID_ID -- task can't be resume, invalid task id.
 *
 * NOTES:
 *      task_id is invalid if the thread and calling thread are not
 *      in the same process.
 */
static UI32_T SYSFUN_ResumeTimeoutThread  (UI32_T task_id)
{
   struct task_struct *task;
   /* thread can not resume itself
    * and check the value
    * Modify By Tony.Lei
    */
   if (task_id == 0)
       return SYSFUN_RESULT_INVALID_ID;
   /**/
 	rcu_read_lock();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    task = find_task_by_vpid(task_id);
#else
    task = find_task_by_pid(task_id);
#endif

	rcu_read_unlock();
	if (!task)
		goto out;

   /*wakeup the process that has been delayed by time*/
   wake_up_process(task);

   return SYSFUN_OK;

out:
   return SYSFUN_RESULT_INVALID_ID;

}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_SuspendTaskWithTimeout
 *------------------------------------------------------------------------------
 * Purpose  : This function will suspend the task which invoke this function
 *            with the duration given by timeout. The suspended task will be
 *            resumed when the timer expire or SYSFUN_ResumeTaskInTimeoutSuspense()
 *            is called with id of the suspended task.
 * INPUT    : timeout  -  timer in ticks
 * OUTPUT   : None
 * RETURN   : TRUE  -  The suspended task is resumed before the timer expires
 *            FALSE -  The suspended task is resumed due to time out or interrupt.
 * NOTE     :
 *            1.This function MUST NOT BE called by an ISR.
 *            2.This function may return immediately if
 *              SYSFUN_ResumeTaskInTimeoutSuspense() was called after timeout
 *              last time. Users should take this case into considration.
 *-----------------------------------------------------------------------------*/
static BOOL_T SYSFUN_SuspendTaskWithTimeout(UI32_T timeout)
{
    SYSFUN_UserTcb_T *utcb_p = &(current->utcb);
    SYSFUN_IntMask_T  int_mask;
    UI32_T ret;
    BOOL_T resume_before_expired;

    if (timeout == 0)
        return TRUE;

    /* clear the semaphore
     */
    local_irq_save(int_mask);
    if (down_trylock(&(utcb_p->suspense_sem)) == 0)
    {
        while (down_trylock(&(utcb_p->suspense_sem)) == 0);
        local_irq_restore(int_mask);
        return TRUE;
    }
    local_irq_restore(int_mask);

    {
#if ( LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0) )
        struct my_timer_rec trec;
        trec.data = (unsigned long)&(utcb_p->suspense_sem);
        trec.timer_lst.expires = jiffies + clock_t_to_jiffies((unsigned long)timeout);
        timer_setup(&trec.timer_lst, SYSFUN_WakeUpRecvEventTimeout, 0);

        ret = down_interruptible(&(utcb_p->suspense_sem));
        /* must use macro time_before check timeout. it yields the correct value even if jiffies wraparound occurred.*/
        resume_before_expired = (ret != -EINTR && time_before(jiffies, trec.timer_lst.expires));

        del_timer_sync(&trec.timer_lst);
#else
        struct timer_list timer;

        init_timer(&timer);
        timer.data     = (unsigned long)&(utcb_p->suspense_sem);
        timer.function = SYSFUN_WakeUpRecvEventTimeout;
        /*jiffies is a 32-bit variable, maybe overflow. */
        timer.expires  = jiffies + clock_t_to_jiffies((unsigned long)timeout);
        add_timer(&timer);

        ret = down_interruptible(&(utcb_p->suspense_sem));
        /* must use macro time_before check timeout. it yields the correct value even if jiffies wraparound occurred.*/
        resume_before_expired = (ret != -EINTR && time_before(jiffies, timer.expires));

        del_timer_sync(&timer);
#endif
    }
    return resume_before_expired;
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_ResumeTaskInTimeoutSuspense
 *------------------------------------------------------------------------------
 * Purpose  : This function will resume the task with the specified task_id
 * INPUT    : task_id - id of the task which is going to be resumed if that task
 *                      is suspended.
 * OUTPUT   : None
 * RETURN   : TRUE  -  The task is resumed without error.
 *            FALSE -  Fail to resume the task
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static BOOL_T SYSFUN_ResumeTaskInTimeoutSuspense (UI32_T task_id)
{
    return (SYSFUN_ResumeThread(task_id) == SYSFUN_OK);
}


static void SYSFUN_InitUTCB(char *task_name, BOOL_T is_ui_task)
{
    SYSFUN_UserTcb_T *utcb_p = &(current->utcb);

    /* Initiate sempaphores for event and suspend task
     * suspend with timeout has not been implemented on Linux
     */
    utcb_p->event_set  = utcb_p->event_waited = 0;
    utcb_p->event_is_all = TRUE;

#if 0 /* this init operation had been moved to copy_process function in $(KERNDIR)/kernel/fork.c */
    sema_init(&(utcb_p->event_sem), 0);
    sema_init(&(utcb_p->suspense_sem), 0);
#endif

    /* initiate stack peak size
     * note: stack peak size has not been implemented on Linux
     */
    utcb_p->stack_peak_size = 0;

    /* initiate cpu record
     * note: cpu record has not been implemented on Linux
     */
    utcb_p->cpu_rec.tid = current->pid;
    utcb_p->cpu_rec.used_ticks = utcb_p->cpu_rec.accumulated_ticks = 0;

    strncpy(utcb_p->task_name, task_name, sizeof(utcb_p->task_name)-1);
    utcb_p->task_name[sizeof(utcb_p->task_name)-1] = 0;
    utcb_p->is_ui_task = is_ui_task;
    if (is_ui_task == TRUE)
    {
        utcb_p->eh_buf_idx=SYSFUN_AllocateEHBufferIndex();
    }
    else
    {
        utcb_p->eh_buf_idx=SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX;
    }
}

static void SYSFUN_DestroyUTCB(void)
{
    SYSFUN_UserTcb_T *utcb_p = &(current->utcb);

    utcb_p->task_name[0] = 0;

    if (utcb_p->is_ui_task==TRUE)
    {
        SYSFUN_FreeEHBufferIndex(utcb_p->eh_buf_idx);
        utcb_p->eh_buf_idx = SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX;
    }

    return;
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_GetUserTcb
 *------------------------------------------------------------------------------
 * Purpose  : Get task specific UserTcb_T.
 * INPUT    : task_id - The task identifier of the target task.
 * OUTPUT   : utcb_p  - The task specific UserTcb_T.
 * RETURN   : SYSFUN_OK                   - Successfully.
 *            SYSFUN_RESULT_OBJ_DELETED   - specified task is deleted.
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static UI32_T SYSFUN_GetUserTcb(UI32_T task_id, SYSFUN_UserTcb_T **utcb_p)
{
    struct task_struct *task_p;

    if (task_id == 0)
    {
        *utcb_p = &current->utcb;
        if ((*utcb_p)->cpu_rec.tid != current->pid)
            return SYSFUN_RESULT_INVALID_ID;
        return SYSFUN_OK;
    }

    read_lock(&tasklist_lock);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32))
    if ((task_p = find_task_by_vpid((int)task_id)) != NULL)
#else
    if ((task_p = find_task_by_pid((int)task_id)) != NULL)
#endif

    {
        *utcb_p = &task_p->utcb;
        read_unlock(&tasklist_lock);
        if ((*utcb_p)->cpu_rec.tid != task_id)
            return SYSFUN_RESULT_INVALID_ID;
        return SYSFUN_OK;
    }
    read_unlock(&tasklist_lock);
    return SYSFUN_RESULT_OBJ_DELETED;
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_AllocateEHBufferIndex
 *------------------------------------------------------------------------------
 * Purpose  : Allocate an EH buffer index
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX   :No EH buffer index left for
 *                                                   allocation
 *            0 - SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK :An EH buffer index
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static UI8_T SYSFUN_AllocateEHBufferIndex(void)
{
    UI32_T *eh_buf_idx_p;
    UI8_T   eh_buf_idx = SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX;

    eh_buf_idx_p = L_PT_Allocate(&eh_buf_idx_pt_desc);
    if (eh_buf_idx_p!=NULL)
    {
        eh_buf_idx = eh_buf_idx_p - &(eh_buf_idx_pool[0]);
        *eh_buf_idx_p = current->pid; /* put pid in allocated buffer for debug purpose only */
    }
    return eh_buf_idx;
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_AllocateEHBufferIndex
 *------------------------------------------------------------------------------
 * Purpose  : Free an EH buffer index
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : Pass an eh buf index that has already been freed will corrupts
 *            eh_buf_idx_pt_desc. eh_buf_idx must be an index that is in
 *            allocated status.
 *-----------------------------------------------------------------------------*/
static void SYSFUN_FreeEHBufferIndex(UI8_T eh_buf_idx)
{
    if (eh_buf_idx>=SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK)
    {
        printk("<0>%s:Invalid eh_buf_idx %hu\n", __FUNCTION__, eh_buf_idx);
        return;
    }
    L_PT_Free(&eh_buf_idx_pt_desc, &(eh_buf_idx_pool[eh_buf_idx]));
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_DumpEHBufInfo
 *------------------------------------------------------------------------------
 * Purpose  : Dump debug info for eh buf index allocation.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : None
 * NOTE     : This function is for debug purpose only.
 *-----------------------------------------------------------------------------*/
static void SYSFUN_DumpEHBufInfo(void)
{
    int i;

    printk("<0>EH Buf Idx free number = %lu\n", (unsigned long)eh_buf_idx_pt_desc.free_no);
    printk("<0>eh_buf_idx_pool = %p\n", &(eh_buf_idx_pool[0]));
    printk("<0>free list in l_pt = %p\n", eh_buf_idx_pt_desc.free);
    printk("<0>Dump eh buf idx pool:\n");

    for (i=0; i<SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK; i++)
        printk("<0>[%d]:%08lX\n", i, (unsigned long)eh_buf_idx_pool[i]);
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_GetEHBufIndex
 *------------------------------------------------------------------------------
 * Purpose  : Get EH buffer index bound to the caller thread.
 * INPUT    : None
 * OUTPUT   : eh_buf_idx_p  --  eh buffer index bound to the caller thread.
 * RETURN   : SYSFUN_OK
 *                          --  Get EH buffer index successfully.
 *            SYSFUN_RESULT_INVALID_ID
 *                          --  No EH buffer index bound to the caller thread.
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static UI32_T SYSFUN_GetEHBufIndex(UI8_T *eh_buf_idx_p)
{
    SYSFUN_UserTcb_T *utcb = &(current->utcb);

    if (utcb->eh_buf_idx == SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX)

    *eh_buf_idx_p = utcb->eh_buf_idx;
    return SYSFUN_OK;
}

/*------------------------------------------------------------------------------
 * Function : SYSFUN_SetEHBufIndex
 *------------------------------------------------------------------------------
 * Purpose  : Set EH buffer index bound to the caller thread.
 * INPUT    : eh_buf_idx    --  EH buffer index bound to the caller thread.
 * OUTPUT   : None
 * RETURN   : SYSFUN_OK
 *                          --  Get EH buffer index successfully.
 *            SYSFUN_RESULT_ERROR
 *                          --  The caller thread is a UI thread, the operation
 *                              is not allowed.
 * NOTE     : If the caller thread is a UI thread, it is not allowed to change
 *            the EH buffer index bound to the caller thread.
 *-----------------------------------------------------------------------------*/
static UI32_T SYSFUN_SetEHBufIndex(UI8_T eh_buf_idx)
{
    SYSFUN_UserTcb_T *utcb = &(current->utcb);

    if (utcb->is_ui_task == TRUE)
        return SYSFUN_RESULT_ERROR;

    utcb->eh_buf_idx = eh_buf_idx;
    return SYSFUN_OK;

}
/*------------------------------------------------------------------------------
 * Function : SYSFUN_InvalidateEHBufIndex
 *------------------------------------------------------------------------------
 * Purpose  : Invalidate EH buffer index bound to the caller thread.
 *            EH buffer index cannot be invalidated if the caller thread belongs
 *            to UI thread.
 * INPUT    : None
 * OUTPUT   : None
 * RETURN   : SYSFUN_OK
 *                          --  Invalidate EH buffer index successfully.
 *            SYSFUN_RESULT_ERROR
 *                          --  Failed to invalidate EH buffer index because the
 *                              caller thread belongs to UI thread.
 * NOTE     : None
 *-----------------------------------------------------------------------------*/
static UI32_T SYSFUN_InvalidateEHBufIndex(void)
{
    SYSFUN_UserTcb_T *utcb = &(current->utcb);

    if (utcb->is_ui_task == TRUE )
        return SYSFUN_RESULT_ERROR;

    utcb->eh_buf_idx = SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX;
    return SYSFUN_OK;
}

MODULE_AUTHOR("Edge-Core Networks");
MODULE_DESCRIPTION("Edge-Core lkm");
MODULE_LICENSE("Proprietary");
