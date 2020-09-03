/* MODULE NAME:  Sysfun_type.h
 * PURPOSE:
 *  Private definition for sysfun.
 *
 * NOTES:
 *
 * HISTORY
 *    2006/11/14 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2006
 */
#ifndef SYSFUN_TYPE_H
#define SYSFUN_TYPE_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
/*  TASK option */
#define SYSFUN_TASK_NAME_LENGTH         15

/*  Timeout Parameter   */
#define SYSFUN_TIMEOUT_NOWAIT           0           /* No wait for timeout parameter    */
#define SYSFUN_TIMEOUT_WAIT_FOREVER     0xFFFFFFFFUL/* Wait forever for timeout paramter    */

/*  Event Function  */
#define SYSFUN_EVENT_WAIT_ALL           0x00        /*  Wait all events occur       */
#define SYSFUN_EVENT_NOWAIT             0x01        /*  Check and receive events but no wait    */
#define SYSFUN_EVENT_WAIT_ANY           0x02        /*  Wait any one events occur   */

/*  Return Value of SYSFUN  */
#define SYSFUN_OK                       0x00        /*  Successfully do the function            */
#define SYSFUN_RESULT_TIMEOUT           0x01        /*  It's timeout, no message, semaphore, event is got   */
#define SYSFUN_RESULT_NO_AVAIL_TIMER    0x02        /*  No more timer could be used, exceed max. timer number   */
#define SYSFUN_RESULT_INVALID_ARG       0x03        /*  Invalid argument in calling function    */
#define SYSFUN_RESULT_OBJ_DELETED       0x05        /*  Task already deleted                    */
#define SYSFUN_RESULT_INVALID_ID        0x06        /*  Invalid object ID for task, message, or semaphore   */
#define SYSFUN_RESULT_NO_NAMED_TASK     0x09        /*  Can't find the named task in system */
#define SYSFUN_RESULT_EINTR             0x0A        /*  The operation is interrupted due to a signal being caught */
#define SYSFUN_RESULT_NO_SPACE_4_TASK   0x0F        /*  No more space for task-creation */
#define SYSFUN_RESULT_STACK_TOO_SMALL   0x10        /*  stack size too small to create task */
#define SYSFUN_RESULT_PRIO_OUT_RANGE    0x11        /*  specified priority out of range */
#define SYSFUN_RESULT_CANT_CREATE_MSG_Q 0x33        /*  Can't create the message queue  */
#define SYSFUN_RESULT_MSG_Q_DELETED     0x36        /*  Queue deleted while task waiting.       */
#define SYSFUN_RESULT_NO_MESSAGE        0x37        /*  No message in queue for SYSFUN_TIMEOUT_NOWAIT   */
#define SYSFUN_RESULT_TASK_WAIT_MSG     0x38        /*  there is tasks waiting for message
                                                     *  when Q is deleted. Not all OS could implement this
                                                     *  function, at lease VxWorks do not support
                                                     *  this function, so check it before use it.
                                                     */
#define SYSFUN_RESULT_NO_EVENT_SET      0x3C        /*  No event is set for SYSFUN_EVENT_NOWAIT */
#define SYSFUN_RESULT_NO_SEMAPHORE      0x41        /*  No more available semaphore could be used in this system */
#define SYSFUN_RESULT_SEM_NOSIGNAL      0x42        /*  The waited semaphore is not set/give/v operation */
#define SYSFUN_RESULT_SEM_DELETED       0x43        /*  Semaphore is deleted                    */
#define SYSFUN_RESULT_CANT_CREATE_SHMEM 0x44        /*  No more available memory for shared memory allocation */
#define SYSFUN_RESULT_MSG_Q_FULL        0x80        /*  Message queue is full, can't send message   */
#define SYSFUN_RESULT_CALL_FROM_ISR     0x81        /*  Invalid use system function, call from ISR  */
#define SYSFUN_RESULT_SYSFUN_NOT_INIT   0x82        /*  Not call SYSFUN_Init before call SYSFUN_xxx */
#define SYSFUN_RESULT_Q_FULL            0x83        /*  Queue is full                               */
#define SYSFUN_RESULT_Q_EMPTY           0x84        /*  Queue is empty                              */
#define SYSFUN_RESULT_TOO_MANY_REQUEST  0x85        /*  Can add more request to the function        */
#define SYSFUN_RESULT_BUF_UNACCESSIBLE  0x86        /*  The pointer to buffer isn't accessible      */
#define SYSFUN_RESULT_BUF_TOO_SMALL     0x87        /*  The buffer is too small                     */
#define SYSFUN_RESULT_ACCESS_VIOLATION  0x88        /*  The calling process have no read or write permission */
#define SYSFUN_RESULT_NO_MEMORY         0x89        /*  The system has not enough memory */
#define SYSFUN_RESULT_CHILD_PROC_ERR    0x8A        /*  The child process returns error */
#define SYSFUN_RESULT_SHELL_CMD_ERR     0x8B        /*  The shell command returns error */
#define SYSFUN_RESULT_ERROR             0xFFFFFFFF  /*  error with unknown reason */

/* constant for sched policy
 */
#define SYSFUN_SCHED_DEFAULT    0
#define SYSFUN_SCHED_FIFO       1
#define SYSFUN_SCHED_RR         2


#define SYSFUN_INTERNAL_SYSCALL_VALIDITY_VALUE   0x0000000a

/* Command ID of syscall.
 */
enum SYSFUN_INTERNAL_SYSCALL_CMD_ID_E
{
    SYSFUN_INTERNAL_SYSCALL_VALIDITY,
    SYSFUN_INTERNAL_SYSCALL_INIT_UTCB,
    SYSFUN_INTERNAL_SYSCALL_DESTROY_UTCB,
    SYSFUN_INTERNAL_SYSCALL_TASKNAME_TO_ID,
    SYSFUN_INTERNAL_SYSCALL_TASKID_TO_NAME,
    SYSFUN_INTERNAL_SYSCALL_SENDEVENT,
    SYSFUN_INTERNAL_SYSCALL_RECEIVEEVENT,
    SYSFUN_INTERNAL_SYSCALL_GETSYSTICK,
    SYSFUN_INTERNAL_SYSCALL_SETSYSTICK, /* not support */
    SYSFUN_INTERNAL_SYSCALL_SUSPENDTASKSELF,
    SYSFUN_INTERNAL_SYSCALL_RESUMETASK,
    SYSFUN_INTERNAL_SYSCALL_SUSPENDTASKWITHTIMEOUT,
    SYSFUN_INTERNAL_SYSCALL_RESUMETASKWITHTIMEOUT,
    SYSFUN_INTERNAL_SYSCALL_DUMPTASKSTACK,
    SYSFUN_INTERNAL_SYSCALL_SETDCACHE,
    SYSFUN_INTERNAL_SYSCALL_SETICACHE,
    SYSFUN_INTERNAL_SYSCALL_GET_CPU_USAGE,
    SYSFUN_INTERNAL_SYSCALL_GET_MEMORY_USAGE,
    SYSFUN_INTERNAL_SYSCALL_GET_TASKID_LIST,
    SYSFUN_INTERNAL_SYSCALL_GET_CPU_USAGE_BY_TASKID,
    SYSFUN_INTERNAL_SYSCALL_DUMP_EHBUF_INFO,
    SYSFUN_INTERNAL_SYSCALL_GET_EHBUF_INDEX,
    SYSFUN_INTERNAL_SYSCALL_SET_EHBUF_INDEX,
    SYSFUN_INTERNAL_SYSCALL_INVALIDATE_EHBUF_INDEX,
};

/* Command ID of syscall for external use.
 */
enum SYSFUN_SYSCALL_ID_E
{
    SYSFUN_SYSCALL_INTERNAL = 0,
    SYSFUN_SYSCALL_L_IPCMEM,
    SYSFUN_SYSCALL_L_MM,
    SYSFUN_SYSCALL_PHYADDR_ACCESS,
    SYSFUN_SYSCALL_PHYSICAL_ADDR_ACCESS,/* anzhen.zheng, 2/19/2008 */
    SYSFUN_SYSCALL_I2C,
    SYSFUN_SYSCALL_SYSJOBQ,  /* unused */
    SYSFUN_SYSCALL_IML_MGR,
    SYSFUN_SYSCALL_VLAN_MGR,
    SYSFUN_SYSCALL_AMTRL3_MGR,
    SYSFUN_SYSCALL_ROUTE_MGR,
    SYSFUN_SYSCALL_DYING_GASP,
    SYSFUN_SYSCALL_VLAN_NET,
    SYSFUN_SYSCALL_PTP_ISR,
    SYSFUN_SYSCALL_IPAL_IF
};

/* Constants for EH
 */
#define SYSFUN_TYPE_INVALID_EH_BUFFER_INDEX 0xFF

/* Maximum number of UI task allowed in the system
 */
#define SYSFUN_TYPE_MAX_NUMBER_OF_UI_TASK 32

/* MACRO FUNCTION DECLARATIONS
 */
/* Utility string operation preprocessor macro functions
 */
#define SYSFUN_PREPROCESSOR_STR(s) #s
#define SYSFUN_PREPROCESSOR_JOINSTR(x,y) SYSFUN_PREPROCESSOR_STR(x ## y)
#define SYSFUN_PREPROCESSOR_CONCAT(a, b) a##b

/* Utility macro to enable/disable gcc warnings START */
/* SYSFUN_GCC_DIAG_ON(x): Enable gcc warning for "x".
 *                        For example, the following expression will enable
 *                        the check for "sizeof-pointer-memaccess" after this
 *                        expression.
 *                        GCC_DIAG_ON(sizeof-pointer-memaccess)
 *
 * SYSFUN_GCC_DIAG_OFF(x): Disable gcc warning for "x".
 *                         For example, the following expression will disable
 *                         the check for "sizeof-pointer-memaccess" after this
 *                         expression.
 *                         GCC_DIAG_OFF(sizeof-pointer-memaccess)
 */
#if ((__GNUC__ * 100) + __GNUC_MINOR__) >= 406
#define SYSFUN_GCC_DIAG_DO_PRAGMA(x) _Pragma (#x)
#define SYSFUN_GCC_DIAG_PRAGMA(x) SYSFUN_GCC_DIAG_DO_PRAGMA(GCC diagnostic x)
#define SYSFUN_GCC_DIAG_OFF(x) SYSFUN_GCC_DIAG_PRAGMA(push) \
    SYSFUN_GCC_DIAG_PRAGMA(ignored SYSFUN_PREPROCESSOR_JOINSTR(-W,x))
#define SYSFUN_GCC_DIAG_ON(x) SYSFUN_GCC_DIAG_PRAGMA(pop)
#else
  #define SYSFUN_GCC_DIAG_OFF(x)
  #define SYSFUN_GCC_DIAG_ON(x)
#endif
/* Utility macro to enable/disable gcc warnings END   */

/* MACRO NAME : SYSFUN_STATIC_ASSERT
 * PURPOSE:
 *      Macro function for compile-time assertion.
 *
 * INPUT:
 *      expression - expression to be evaulated. 
 *      err_msg    - error message when the assertion is failed
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      1. This macro helps to evaluate the expression at compile time.
 */
/* Utility macro for static assert(i.e. compile-time assertion) START */
#define SYSFUN_PREPROCESSOR_ASSERT_CONCAT(a, b) SYSFUN_PREPROCESSOR_CONCAT(a, b)
/* These can't be used after statements in c89. */
#ifdef __COUNTER__
  #define SYSFUN_STATIC_ASSERT(expression,err_msg) \
    ;enum { SYSFUN_PREPROCESSOR_ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(!!(expression)) }
#else
  /* This can't be used twice on the same line so ensure if using in headers
   * that the headers are not included twice (by wrapping in #ifndef...#endif)
   * Note it doesn't cause an issue when used on same line of separate modules
   * compiled with gcc -combine -fwhole-program.  */
  #define SYSFUN_STATIC_ASSERT(expression,err_msg) \
    ;enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(expression)) }
#endif
/* Utility macro for static assert(i.e. compile-time assertion) END   */

/* MACRO NAME : SYSFUN_CHECK_CONVERTED_DATA_TYPE_SIZE
 * PURPOSE:
 *      Macro function for checking the data size of the converted data type
 *      is large enough to keep the original data type using compile-time assertion.
 *
 * INPUT:
 *      converted_data_type - the data type used to keep the value
 *                            converted(i.e. type cast) from original_data_type
 *      original_data_type  - the data type of the original data.
 * OUTPUT:
 *      None.
 *
 * RETURN:
 *      None.
 *
 * NOTES:
 *      When the size of the data type 'converted_data_type' is smaller than
 *      data type 'original_data_type', the converted data might not be able
 *      to keep the original value using data type 'original_data_type'.
 *      This macro emits error at compile time when the size of
 *      converted_data_type is smaller than the size of original_data_type.
 */
#define SYSFUN_CHECK_CONVERTED_DATA_TYPE_SIZE(converted_data_type, original_data_type) \
    SYSFUN_STATIC_ASSERT(sizeof(converted_data_type)>=sizeof(original_data_type), "Size of converted data type is smaller than original data type");

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#endif    /* End of SYSFUN_TYPE_H */

