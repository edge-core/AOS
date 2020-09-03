/* MODULE NAME:  l_mm_backdoor.c
 * PURPOSE:
 *     User space l_mm backdoor
 *
 * NOTES:
 *     BACKDOOR_OPEN in kernel L_MM.c must be defined as TRUE when using this
 *     module in user space.
 *     Cares should be taken the definition of L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER
 *     must be consistent with the definition in kernel mode L_MM.c.
 *
 * HISTORY
 *    2006/10/26 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2006
 */
/* define L_MM_BACKDOOR_EXCEPTION_TEST if the backdoor item to cause exception
 * need to be included
 */
/* #define L_MM_BACKDOOR_EXCEPTION_TEST */
 
/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h> /* for strtoul() */
#if defined(L_MM_BACKDOOR_EXCEPTION_TEST)
#include <signal.h>
#include <time.h>
#include <string.h>
#endif

/* accton include files */
#include "sys_module.h"
#include "sys_type.h"
#include "l_mm.h"
#include "l_mm_type.h"
#include "l_ipcmem.h"
#include "sysfun.h"
#include "backdoor_mgr.h"
#include "l_mm_backdoor.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER should be FALSE for
 * release build to avoid hindering system performance.
 */
#define L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER FALSE

#define L_MM_BACKDOOR_MAX_DISPLAY_TASKNAME_LEN 11

#define L_MM_SHOW_ALL_ID               0xFFFFFFFF

/* For L_MM_Backdoor user id display mode definition
 */
enum {
    L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE = 0,
    L_MM_BACKDOOR_DISPLAY_EXT_TRACE,
    L_MM_BACKDOOR_MAX_DISPLAY_MODE
};

/* MACRO FUNCTION DECLARATIONS
 */
#if (L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER==TRUE)
/* for showing backdoor menu item
 */
#define L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM() \
    BACKDOOR_MGR_Print("\r\n 7. Toggle malloc buffer elapsed time check flag");
#define K_L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM() \
    BACKDOOR_MGR_Print("\r\n 8. Toggle malloc buffer elapsed time check flag");

/* for toggling the flag for checking elapsed time of malloc buffer
 */
#define K_L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER() \
{\
    BOOL_T check_elapsed_time_flag=(BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR, (void*)L_MM_BACKDOOR_CMD_TOGGLECHECKELAPSEDTIMEOFMALLOCBUFFERFLAG, NULL, NULL, NULL);\
    BACKDOOR_MGR_Printf("\r\nFlag %s", check_elapsed_time_flag ? "On" : "Off");\
}

#else
#define L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM()
#define K_L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM()
#define K_L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER()
#endif

/*enum defined in src/sysinclude/sys_bld.h*/
#define SYSBLD_LIST(_) \
    _(SYS_BLD_APP_PROTOCOL_PROC_OM_IPCMSGQ_KEY,      "APP_PROTOCOL_PROC")  \
    _(SYS_BLD_AUTH_PROTOCOL_PROC_OM_IPCMSGQ_KEY,     "AUTH_PROC")          \
    _(SYS_BLD_BGP_PROC_OM_IPCMSGQ_KEY,               "BGP_PROC")           \
    _(SYS_BLD_CLI_PROC_OM_IPCMSGQ_KEY,               "CLI_PROC")           \
    _(SYS_BLD_CORE_UTIL_PROC_OM_IPCMSGQ_KEY,         "CORE_UTIL_PROC")     \
    _(SYS_BLD_DOT1X_SUP_PROC_OM_IPCMSGQ_KEY,         "DOT1X_SUP_PROC")     \
    _(SYS_BLD_DRIVER_PROC_OM_IPCMSGQ_KEY,            "DRIVER_PROC")        \
    _(SYS_BLD_IP_SERVICE_PROC_OM_IPCMSGQ_KEY,        "IP_SERVICE_PROCESS") \
    _(SYS_BLD_L2_L4_PROC_OM_IPCMSGQ_KEY,             "L2_L4_PROCESS")      \
    _(SYS_BLD_NETCFG_PROC_OM_IPCMSGQ_KEY,            "NETCFG_PROC")        \
    _(SYS_BLD_NSM_PROC_OM_IPCMSGQ_KEY,               "NSM_PROC")           \
    _(SYS_BLD_OSPF6_PROC_OM_IPCMSGQ_KEY,             "OSPF6_PROCC")        \
    _(SYS_BLD_OSPF_PROC_OM_IPCMSGQ_KEY,              "OSPF_PROC")          \
    _(SYS_BLD_PIM_PROC_OM_IPCMSGQ_KEY,               "PIM_PROC")           \
    _(SYS_BLD_POE_PROC_OM_IPCMSGQ_KEY,               "POE_PROC")           \
    _(SYS_BLD_RIP_PROC_OM_IPCMSGQ_KEY,               "RIP_PROC")           \
    _(SYS_BLD_SFLOW_PROC_OM_IPCMSGQ_KEY,             "SFLOW_PROC")         \
    _(SYS_BLD_SNMP_PROC_OM_IPCMSGQ_KEY,              "SNMP_PROC")          \
    _(SYS_BLD_STKCTRL_PROC_OM_IPCMSGQ_KEY,           "STKCTRL_PROC")       \
    _(SYS_BLD_STKTPLG_PROC_OM_IPCMSGQ_KEY,           "STKTPLG_PROC")       \
    _(SYS_BLD_SYS_MGMT_PROC_OM_IPCMSGQ_KEY,          "SYS_MGMT_PROC")      \
    _(SYS_BLD_VRRP_PROC_OM_IPCMSGQ_KEY,              "VRRP_PROC")          \
    _(SYS_BLD_WEB_PROC_OM_IPCMSGQ_KEY,               "WEB_PROC")           \
    _(SYS_BLD_XFER_PROC_OM_IPCMSGQ_KEY,              "XFER_PROC")          \
    _(SYS_BLD_SYS_CALLBACK_PROC_OM_IPCMSGQ_KEY,      "SYS_CALLBACK_PROC")
    
       
    
/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void L_MM_BACKDOOR_ShowBufferInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time);
static void K_L_MM_BACKDOOR_ShowBufferInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time);
static void K_L_MM_BACKDOOR_ShowMrefInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time);
static void L_MM_BACKDOOR_DumpBuffer(void *p);
static void K_L_MM_BACKDOOR_DumpBuffer(void *p);
static BOOL_T L_MM_BACKDOOR_GetTaskIdFromConsole(UI32_T *value_p);
static BOOL_T L_MM_BACKDOOR_GetModuleIdFromConsole(UI32_T *value_p);
static BOOL_T L_MM_BACKDOOR_GetElapsedTimeFromConsole(UI32_T *value_p);
static BOOL_T L_MM_BACKDOOR_GetAddressFromConsole(void **pp);
static void L_MM_BACKDOOR_ShowProcessName(void);
static BOOL_T L_MM_BACKDOOR_GetProcessNumberFromConsole(UI32_T *value_p);

/* STATIC VARIABLE DECLARATIONS
 */
/* for table header of different display modes
 */
static char*  header_str[L_MM_BACKDOOR_MAX_DISPLAY_MODE] = {"PL_ID TR_ID", "EXT_TR_ID  "};
static UI8_T   uid_display_mode=L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE;

/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_BACKDOOR_BackDoorMenu
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM kernel space backdoor menu entry funtion.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    This function should be called directly by backdoor_mgr which handles backdoor
 *    transactions.
 *------------------------------------------------------------------------------
 */
void K_L_MM_BACKDOOR_BackDoorMenu(void)
{
    UI32_T task_id, module_id, elapsed_time;
    char   str_buf[3];
    BOOL_T is_exit=FALSE;

    /*  BODY
     */

    while(is_exit==FALSE)
    {
        BACKDOOR_MGR_Print("\r\n==========K_L_MM BackDoor Menu================\n");
        BACKDOOR_MGR_Print("\r\n 0. Exit");
        BACKDOOR_MGR_Print("\r\n 1. Show ALL L_MM_Monitor_T info");
        BACKDOOR_MGR_Print("\r\n 2. Show L_MM_Monitor_T info by a specific taskID, moduleID, and elpased time > a given value");
        BACKDOOR_MGR_Print("\r\n 3. Show L_MM_Mref_T info by a specific taskID, moduleID, and elpased time > a given value");
        BACKDOOR_MGR_Print("\r\n 4. Show total number of allocated buffer");
        BACKDOOR_MGR_Print("\r\n 5. Toggle debug message flag");
        BACKDOOR_MGR_Print("\r\n 6. Toggle validate free flag");
        BACKDOOR_MGR_Print("\r\n 7. Toggle user id display mode");
        /* item 8 */ K_L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM();
        BACKDOOR_MGR_Print("\r\n 9. Dump buffer of L_MM_Monitor_T by address");
        BACKDOOR_MGR_Print("\r\n a. Toggle mref error detect flag");
        BACKDOOR_MGR_Print("\r\n b. Corrupt MREF DESC");
        BACKDOOR_MGR_Print("\r\n=================================================");
        BACKDOOR_MGR_Print("\r\nselect =");

        BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

        switch (str_buf[0])
        {
            case '0' :
                is_exit = TRUE;
                break;
            case '1': /* Show ALL L_MM_Monitor_T info */
                K_L_MM_BACKDOOR_ShowBufferInfo(L_MM_SHOW_ALL_ID, L_MM_SHOW_ALL_ID, 0);
                break;
            case '2': /* Show L_MM_Monitor_T info by a specific taskID, moduleID, and elpased time > a given value */
                BACKDOOR_MGR_Print("\r\nTask id(input hex value, '*' means show all):0x");
                if(FALSE==L_MM_BACKDOOR_GetTaskIdFromConsole(&task_id))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for task id.");
                    break;
                }
                BACKDOOR_MGR_Print("\r\nModule id(input dec value, '*' means show all):");
                if(FALSE==L_MM_BACKDOOR_GetModuleIdFromConsole(&module_id))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for module id.");
                    break;
                }
                BACKDOOR_MGR_Print("\r\nElapsed time(input dec value, '0' means show all):");
                if(FALSE==L_MM_BACKDOOR_GetElapsedTimeFromConsole(&elapsed_time))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for elapsed time.\n");
                    break;
                }
                K_L_MM_BACKDOOR_ShowBufferInfo(task_id, module_id, elapsed_time);
                break;
            case '3': /* Show L_MM_Mref_T info by a specific taskID, moduleID, and elpased time > a given value */
                BACKDOOR_MGR_Print("\r\nTask id(input hex value, '*' means show all):0x");
                if(FALSE==L_MM_BACKDOOR_GetTaskIdFromConsole(&task_id))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for task id.");
                    break;
                }
                BACKDOOR_MGR_Print("\r\nModule id(input dec value, '*' means show all):");
                if(FALSE==L_MM_BACKDOOR_GetModuleIdFromConsole(&module_id))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for module id.");
                    break;
                }
                BACKDOOR_MGR_Print("\r\nElapsed time(input dec value, '0' means show all):");
                if(FALSE==L_MM_BACKDOOR_GetElapsedTimeFromConsole(&elapsed_time))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for elapsed time.\n");
                    break;
                }
                K_L_MM_BACKDOOR_ShowMrefInfo(task_id, module_id, elapsed_time);
                break;
            case '4': /* Show total number of allocated buffer */
                BACKDOOR_MGR_Printf("\r\nTotal number of allocated buffer = %ld",
                       SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR, (void*)L_MM_BACKDOOR_CMD_GETTOTALNBROFALLOCATEDBUFFER, NULL, NULL, NULL));
                break;
            case '5': /* Toggle debug message flag */
            {
                BOOL_T dbg_msg_flag = (UI8_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR, (void*)L_MM_BACKDOOR_CMD_TOGGLEDBGMSGFLAG, NULL, NULL, NULL);
                BACKDOOR_MGR_Printf("\r\nDebug message flag = %s", (dbg_msg_flag==TRUE)?"TRUE":"FALSE");
            }
                break;
            case '6': /* Toggle validation free flag */
            {
                BOOL_T validate_free_flag = (UI8_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR, (void*)L_MM_BACKDOOR_CMD_TOGGLEVALIDATEFREEFLAG, NULL, NULL, NULL);
                BACKDOOR_MGR_Printf("\r\nValidate free flag = %s", (validate_free_flag==TRUE)?"TRUE":"FALSE");
            }
                break;
            case '7': /* Toogle uid display mode */
                if(uid_display_mode==L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE)
                {
                    uid_display_mode=L_MM_BACKDOOR_DISPLAY_EXT_TRACE;
                    BACKDOOR_MGR_Print("\r\nDisplay ext trace id.");
                }
                else
                {
                    uid_display_mode=L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE;
                    BACKDOOR_MGR_Print("\r\nDisplay pool id and trace id.");
                }
                break;
            case '8': /* Toggle malloc buffer elapsed time check flag */
                K_L_MM_DEBUG_TOGGLE_FLAG_OF_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER();
                break;
            case '9':
            {
                void *p;

                BACKDOOR_MGR_Print("\r\nL_MM_Monitor_T BUF_ADDR: ");
                if (!L_MM_BACKDOOR_GetAddressFromConsole(&p))
                {
                    BACKDOOR_MGR_Print("\r\nInvalid value for buffer address.\n");
                    break;
                }
                BACKDOOR_MGR_Print("\r\n");
                if (p)
                {
                    K_L_MM_BACKDOOR_DumpBuffer(p);
                }
            }
                break;
            case 'a': /* Toggle mref error detect flag */
            {
                BOOL_T mref_error_detect_flag = (UI8_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR, (void*)L_MM_BACKDOOR_CMD_TOGGLEMREFERRDETECTFLAG, NULL, NULL, NULL);
                BACKDOOR_MGR_Printf("\r\nMREF error detect flag = %s", (mref_error_detect_flag==TRUE)?"TRUE":"FALSE");
            }
                break;
            case 'b':
            {
                SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR,
                        (void*)L_MM_BACKDOOR_CMD_MREF_CORRUPT, NULL, NULL, NULL);
            }
                break;
            default :
                break;
        }
    }   /*  end of while    */

    return;
}


/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_BackDoorMenu
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM user space backdoor menu entry funtion.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    None.
 *
 * NOTES:
 *    This function should be called directly by backdoor_mgr which handles backdoor
 *    transactions.
 *------------------------------------------------------------------------------
 */
void L_MM_BACKDOOR_BackDoorMenu(void)
{
    UI32_T processnum,bd_choose;
    SYSFUN_MsgQ_T ipcmsgq_handle;
    UI32_T task_id, module_id, elapsed_time;
    void *dump_buf_addr_p;
    static const UI32_T msgq_key[]={SYSBLD_LIST(MODULE_ID)};
    const UI32_T msg_size =sizeof(L_MM_BACKDOOR_IpcMsg_T);
    UI8_T ipc_buf[SYSFUN_SIZE_OF_MSG(msg_size)];
    L_MM_BACKDOOR_IpcMsg_T *msg_p=NULL;
    SYSFUN_Msg_T *msgbuf_p = (SYSFUN_Msg_T*)ipc_buf;
    BOOL_T is_exit=FALSE;
    char   str_buf[3];

    /*  BODY
     */

    while(is_exit==FALSE)
    {
        BACKDOOR_MGR_Print("\r\n==========L_MM BackDoor Menu================");
        BACKDOOR_MGR_Print("\r\n 0. Exit");
        BACKDOOR_MGR_Print("\r\n 1. Show all process name");
        BACKDOOR_MGR_Print("\r\n 2. Specify which process to show its buffer information");
        BACKDOOR_MGR_Print("\r\n=================================================");
        BACKDOOR_MGR_Print("\r\nselect =");
        BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

        switch (str_buf[0])
        {
            case '0' :
                is_exit = TRUE;
                break;
            case '1': /* Show all process name */
                L_MM_BACKDOOR_ShowProcessName();
                break;
            case '2': /* Show buffer information of the specified process */
                BACKDOOR_MGR_Print("\r\nEnter the process number:");
                if (FALSE==L_MM_BACKDOOR_GetProcessNumberFromConsole(&processnum) || (processnum>(sizeof(msgq_key)/sizeof(UI32_T))) || (processnum<1))
                {
                    break;
                }

                if (SYSFUN_GetMsgQ(msgq_key[processnum-1], SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
                {
                    BACKDOOR_MGR_Printf("\r\n%s(): SYSFUN_GetMsgQ failed.\r\n", __FUNCTION__);
                }
                /*
                 * Check whether the process exists or not. SYSFUN_GetMsgQOwner()
                 * will return task_id, task_id = 0 stands for "no such process"
                 */
                if(0 == SYSFUN_GetMsgQOwner(ipcmsgq_handle) )
                {
                       BACKDOOR_MGR_Print("\r\nProcess not exist.\r\n");
                       break;
                }
                else
                {
                    BACKDOOR_MGR_Print("\r\n==========L_MM BackDoor Menu================");
                    BACKDOOR_MGR_Print("\r\n 0. Exit");
                    BACKDOOR_MGR_Print("\r\n 1. Show ALL L_MM_Monitor_T info");
                    BACKDOOR_MGR_Print("\r\n 2. Show L_MM_Monitor_T info by a specific taskID,");
                    BACKDOOR_MGR_Print(" moduleID, and elpased time > a given value");
                    BACKDOOR_MGR_Print("\r\n 3. Show total number of allocated buffer");
                    BACKDOOR_MGR_Print("\r\n 4. Toggle debug message flag");
                    BACKDOOR_MGR_Print("\r\n 5. Toggle validate free flag");
                    BACKDOOR_MGR_Print("\r\n 6. Toggle user id display mode");
                    /* item 7 */ L_MM_DEBUG_CHECK_ELAPSED_TIME_OF_MALLOC_BUFFER_SHOW_BACKDOOR_MENU_ITEM();
                    BACKDOOR_MGR_Print("\r\n 8. Dump buffer of L_MM_Monitor_T by address");
                    #if defined(L_MM_BACKDOOR_EXCEPTION_TEST)
                    BACKDOOR_MGR_Print("\r\n 9. Trigger an exception");
                    #endif
                    BACKDOOR_MGR_Print("\r\n=================================================");
                    BACKDOOR_MGR_Print("\r\nselect =");
                    L_MM_BACKDOOR_GetProcessNumberFromConsole(&bd_choose);

                    msgbuf_p->cmd = SYS_MODULE_CMNLIB;
                    msgbuf_p->msg_size = msg_size;

                    msg_p = (L_MM_BACKDOOR_IpcMsg_T*)msgbuf_p->msg_buf;
                    msg_p->msgq_key = msgq_key[processnum-1];

                    switch (bd_choose)
                    {
                        case 1: /* Show ALL L_MM_Monitor_T info */
                            break;
                        case 2: /* Show L_MM_Monitor_T info by a specific taskID, moduleID, and elpased time > a given value */
                            BACKDOOR_MGR_Print("\r\nTask id(input hex value, '*' means show all):0x");
                            if(FALSE==L_MM_BACKDOOR_GetTaskIdFromConsole(&task_id))
                            {
                                BACKDOOR_MGR_Print("\r\nInvalid value for task id.");
                                bd_choose = 0;
                                break;
                            }
                            BACKDOOR_MGR_Print("\r\nModule id(input dec value, '*' means show all):");
                            if(FALSE==L_MM_BACKDOOR_GetModuleIdFromConsole(&module_id))
                            {
                                BACKDOOR_MGR_Print("\r\nInvalid value for module id.");
                                bd_choose = 0;
                                break;
                            }
                            BACKDOOR_MGR_Print("\r\nElapsed time(input dec value, '0' means show all):");
                            if(FALSE==L_MM_BACKDOOR_GetElapsedTimeFromConsole(&elapsed_time))
                            {
                                BACKDOOR_MGR_Print("\r\nInvalid value for elapsed time.\n");
                                bd_choose = 0;
                                break;
                            }
                            msg_p->task_id=task_id;
                            msg_p->module_id=module_id;
                            msg_p->elapsed_time=elapsed_time;
                            break;
                        case 3: /* Show total number of allocated buffer */
                        case 4: /* Toggle debug message flag */
                        case 5: /* Toggle validation free flag */
                        case 6: /* Toogle uid display mode */
                        case 7: /* Toggle malloc buffer elapsed time check flag */
                        #if defined(L_MM_BACKDOOR_EXCEPTION_TEST)
                        case 9: /* Trigger an exception */
                        #endif
                            break;
                        case 8: /* Dump buffer of L_MM_Monitor_T by address */
                        {
                            BACKDOOR_MGR_Print("\r\nL_MM_Monitor_T BUF_ADDR: ");
                            if (!L_MM_BACKDOOR_GetAddressFromConsole(&dump_buf_addr_p))
                            {
                                BACKDOOR_MGR_Print("\r\nInvalid value for buffer address.\n");
                                bd_choose = 0;
                                break;
                            }
                            BACKDOOR_MGR_Print("\r\n");
                            msg_p->dump_buf_addr_p = dump_buf_addr_p;
                        }
                            break;
                        default :
                            break;
                    }
                    if(bd_choose > 9 || bd_choose < 1)
                        break;
                    else
                        msg_p->bd_choose=bd_choose;

                    if(msgq_key[processnum-1]==SYS_BLD_CORE_UTIL_PROC_OM_IPCMSGQ_KEY) /* when the choose is core_util, no need to do IPC */
                    {
                        L_MM_BACKDOOR_BackDoorMenu4Process(msgbuf_p);
                    }
                    else
                    {
                        if (SYSFUN_SendRequestMsg(ipcmsgq_handle, msgbuf_p,
                            SYSFUN_TIMEOUT_WAIT_FOREVER, SYSFUN_SYSTEM_EVENT_IPCMSG, msg_size,
                            msgbuf_p) != SYSFUN_OK)
                        {
                            BACKDOOR_MGR_Print("\r\n: SYSFUN_SendRequestMsg error.\r\n");
                        }
                    }
                }
                SYSFUN_ReleaseMsgQ(ipcmsgq_handle);
                break;
            default:
                break;
        }
    }
}

#if defined(L_MM_BACKDOOR_EXCEPTION_TEST)
#define L_MM_BACKDOOR_EXCEPTION_TEST_SIGNAL_NO (SIGRTMIN + 3)
static void L_MM_BACKDOOR_Exception_Helper(int signo)
{
    char* foo_p=NULL;
    strcpy(foo_p,"Foo");
}

static void L_MM_BACKDOOR_CauseExceptionLatter(void)
{
    int rc;
    UI32_T sysfun_rc;
    struct sigevent timer_event_spec;
    timer_t         timer_id;
    struct itimerspec interval;
    static BOOL_T is_signal_registered=FALSE;

    if(is_signal_registered==TRUE)
    {
        BACKDOOR_MGR_Printf("%s(%d)Signal for exception is registered.\r\n");
        return;
    }

    sysfun_rc=SYSFUN_RegisterSignal(L_MM_BACKDOOR_EXCEPTION_TEST_SIGNAL_NO-SIGRTMIN, L_MM_BACKDOOR_Exception_Helper);
    if(sysfun_rc!=SYSFUN_OK)
    {
        BACKDOOR_MGR_Printf("%s(%d)SYSFUN_RegisterSignal error(rc=%lu).\r\n",
            __FUNCTION__, __LINE__, sysfun_rc);
        return;
    }

    timer_event_spec.sigev_signo = (L_MM_BACKDOOR_EXCEPTION_TEST_SIGNAL_NO);
    timer_event_spec.sigev_notify = SIGEV_SIGNAL;
    timer_event_spec.sigev_value.sival_int = (int)0;

    rc = timer_create(CLOCK_REALTIME, &(timer_event_spec), &(timer_id));
    if (rc!=0)
    {
        BACKDOOR_MGR_Printf("%s(%d)timer_create error(rc=%d)\r\n", __FUNCTION__, __LINE__, rc);
        return;
    }

    memset(&interval, 0, sizeof(interval));
    interval.it_value.tv_sec = 1;
    interval.it_value.tv_nsec = 0;

    rc = timer_settime(timer_id, 0, &interval, NULL);
    if (rc != 0)
    {
        BACKDOOR_MGR_Printf("%s(%d)timer_settime error(rc=%d)\r\n", __FUNCTION__, __LINE__, rc);
        return;
    }
}
#endif /* end of #if defined(L_MM_BACKDOOR_EXCEPTION_TEST) */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_BackDoorMenu4Process
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    L_MM user space backdoor menu entry funtion for process.
 *
 * INPUT:
 *    msgbuf_p
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *  TRUE  -- done
 * NOTES:
 *  According to bd_choose from msgbuf_p, calling the corresponding function.  
 *  (Ex show buff info, toggle flag, dump buffer)
 *------------------------------------------------------------------------------
 */
BOOL_T L_MM_BACKDOOR_BackDoorMenu4Process(SYSFUN_Msg_T* msgbuf_p)
{
    L_MM_BACKDOOR_IpcMsg_T *msg_p;
    UI32_T bd_choose;
    SYSFUN_MsgQ_T ipcmsgq_handle;
    UI32_T task_id, module_id, elapsed_time;

    msg_p = (L_MM_BACKDOOR_IpcMsg_T*)msgbuf_p->msg_buf;
    bd_choose = msg_p->bd_choose;
    if (SYSFUN_GetMsgQ(msg_p->msgq_key, SYSFUN_MSGQ_BIDIRECTIONAL, &ipcmsgq_handle) != SYSFUN_OK)
        BACKDOOR_MGR_Printf("\r\n%s(): fail to get message queue.\r\n", __FUNCTION__);

    /*  BODY
     */
        switch (bd_choose)
        {
            case 1: /* Show ALL L_MM_Monitor_T info */
                L_MM_BACKDOOR_ShowBufferInfo(L_MM_SHOW_ALL_ID, L_MM_SHOW_ALL_ID, 0);
                break;
            case 2: /* Show L_MM_Monitor_T info by a specific taskID, moduleID, and elpased time > a given value */
                task_id = msg_p->task_id;
                module_id = msg_p->module_id;
                elapsed_time = msg_p->elapsed_time;
                L_MM_BACKDOOR_ShowBufferInfo(task_id, module_id, elapsed_time);
                break;
            case 3: /* Show total number of allocated buffer */
                BACKDOOR_MGR_Printf("\r\nTotal number of allocated buffer = %ld", L_MM_GetTotalNBofAllocatedBuffer());
                break;
            case 4: /* Toggle debug message flag */
            {
                BOOL_T dbg_msg_flag = L_MM_ToggleDebugMsgFlag();
                BACKDOOR_MGR_Printf("\r\nDebug message flag = %s", (dbg_msg_flag==TRUE)?"TRUE":"FALSE");
            }
                break;
            case 5: /* Toggle validation free flag */
            {
                BOOL_T validate_free_flag = L_MM_ToggleValidateFreeFlag();
                BACKDOOR_MGR_Printf("\r\nValidate free flag = %s", (validate_free_flag==TRUE)?"TRUE":"FALSE");
            }
                break;
            case 6: /* Toogle uid display mode */
                if(uid_display_mode==L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE)
                {
                    uid_display_mode=L_MM_BACKDOOR_DISPLAY_EXT_TRACE;
                    BACKDOOR_MGR_Print("\r\nDisplay ext trace id.");
                }
                else
                {
                    uid_display_mode=L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE;
                    BACKDOOR_MGR_Print("\r\nDisplay pool id and trace id.");
                }
                break;
            case 7: /* Toggle malloc buffer elapsed time check flag */
                L_MM_ToggleCheckElapsedTimeFlag();
                break;
            case 8:
            {
                void *dump_buf_addr_p;
                dump_buf_addr_p = msg_p->dump_buf_addr_p;

                if (dump_buf_addr_p)
                {
                    L_MM_BACKDOOR_DumpBuffer(dump_buf_addr_p);
                }
            }
		        break;
#if defined(L_MM_BACKDOOR_EXCEPTION_TEST)
            case 9:
                L_MM_BACKDOOR_CauseExceptionLatter();
                break;
#endif
            default :
                break;
        }
    SYSFUN_ReleaseMsgQ(ipcmsgq_handle);
    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_ShowBufferInfo
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Display Monitored Buffer information by given conditions
 *
 * INPUT:
 *  task_id      -- specify the task_id to display (or L_MM_SHOW_ALL_ID  to display all tasks)
 *  module_id    -- specify the module_id to display (or L_MM_SHOW_ALL_ID to display all modules)
 *  elapsed_time -- for those buffer with (current system time -allocation_time > elapsed_time)
 *                  are the condition to display (or 0 to display all)
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *   1.The given conditions (task_id, module_id, elapsed_time) will all be
 *     checked for display (They are ANDed, not ORed)
 *   2.The buffer info is retrieved from kernel one by one. So there are chances
 *     that the retrieved monitored buffer is freed so that there is no way to
 *     proceed to next monitored buffer.
 *------------------------------------------------------------------------------
 */
static void L_MM_BACKDOOR_ShowBufferInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time)
{
    L_MM_Backdoor_BufferInfo_T buffer_info;
    UI32_T current_time = SYSFUN_GetSysTick();
    UI32_T check_time = current_time - elapsed_time;
    UI32_T calc_elapsed_time;
    /*char *taskname_p;*/
    /*char taskname[SYSFUN_TASK_NAME_LENGTH+1];*/
    BOOL_T is_normal_scenario = (current_time>check_time) ? TRUE : FALSE;

    /* There are two scenarios should be considered for elapsed_time checking:
     * The first scenario is Normal Scenario:
     *     If the range of timestamp we're going to check is not
     *     across the maximum value of UI32_T(i.e. 0xFFFFFFFF).
     * The other scenario is Start-over Scenario:
     *     If the range of timestamp we're going to check is across the boundary of the maximum value of
     *     UI32_T.
     *
     * Normal Scenario is illustrated as the chart shown below:
     *  |***********|xxxxxxxxxxxxxxxxx|------------------>
     *  0           ^                 ^
     *              |<--------------->|
     *              |   elapsed_time  |
     *              +->check_time     |
     *                                +->current_time
     * Assume sys tick starts from 0, and the time stamp meet the condition should fall within the range
     * marked as '*'. If the time stamp fall in the range marked as 'x', that means life time of the buffer
     * is smaller than elpased_time. Thus the checking condition for Normal Scenario would be:
     *              time_stamp < check_time
     * Further more, if the sys tick has been across 0xFFFFFFFF, there might be some buffer with timestamp greater
     * than current_time, those buffer should also be included. At last, the checking condition would be:
     *              (time_stamp < check_time) || (time_stamp>current_time)
     * However, if the timestamp is assigned before sys tick was across 0xFFFFFFFF and the timestamp happens to
     * fall in 'x' range, we would consider the buffer doesn't meet the condition but actually it does meet.
     *
     * Start-over Scenario is illustrated as the chart shown below:
     *  |xxxxxx|**************** ... *******************|xxxxxxxxxxxxxxxxxx|
     *  0      ^                                        ^                  ^(0xFFFFFFFF)
     *  |<---->|                                        |<---------------->|
     *     a   |                                        |        b
     *         +->current_time                          +->check_time
     * The meaning of legends '*' and 'x' is the same as in Normal Condition.
     * Assume a + b == elapsed_time.
     * The checking condition for this scenario would be:
     *              (time_stamp < check_time) && (time_stamp > current_time)
     */
    BACKDOOR_MGR_Printf("\r\n   TASK_ID   ELAPSEDTM MAG    BUF_SZ BUF_ADDR  M_ID %s",header_str[uid_display_mode]);
    BACKDOOR_MGR_Printf("\r\n------------------------------------------------------------------------------\r\n");

    buffer_info.last_ref_monitor_p=NULL;

    while(L_MM_GetBufferInfo(&buffer_info) == TRUE)
    {
	/* check whether task_id mached */
        if ( (task_id != L_MM_SHOW_ALL_ID) && (task_id != buffer_info.task_id) )
            continue;

        /* check whether module_id matched */
        if ( (module_id != L_MM_SHOW_ALL_ID) && (module_id != (L_MM_GET_MODULEID_FROM_USERID(buffer_info.uid)) ) )
            continue;

        /* Check whether elapsed time matched */
        if(elapsed_time!=0)
        {
            if ( is_normal_scenario == TRUE )
            {
                if ( (buffer_info.allocation_time> check_time) &&
                     (buffer_info.allocation_time <= current_time)
                   )
                    continue;
            } /* end of if( is_normal_scenario == TRUE ) */
            else /* is_normal_scenario == */
            {
                if ( (buffer_info.allocation_time > check_time) ||
                     (buffer_info.allocation_time <= current_time)
                   )
                   continue;
            } /* end of else */
        } /* end of if(elapsed_time!=0) */

        /* Display buffer info for matched buffer */
        /* Sample print out format:
         *   TASK_ID TASK_NAME    ELAPSEDTM MAG BUF_T M_ID PL_ID TR_ID   BUF_SZ BUF_ADDR
         *------------------------------------------------------------------------------
         *0x12345678 Sample_Task 1234567890 R,R  0,63   123   1     12 12345678 001A0000
         */

        
        /*if(SYSFUN_OK != SYSFUN_TaskIDToName(buffer_info.task_id, &taskname, sizeof(taskname)))
            taskname_p = "Unknown";
        else
            taskname_p=&taskname;
        */
        if(buffer_info.allocation_time<=current_time)
            calc_elapsed_time = current_time - buffer_info.allocation_time;
        else
            calc_elapsed_time = 0xFFFFFFFF - (buffer_info.allocation_time - current_time);

            BACKDOOR_MGR_Printf("\r\n0x%8lx %10lu %1s,%1s %8hu %p  %3hu",
            buffer_info.task_id, /* TASK_ID */
            calc_elapsed_time,   /* ELAPSEDTM */
            (buffer_info.is_valid_leading_magic_byte==TRUE) ? "R":"W", /* leading magic byte check */
            (buffer_info.is_valid_trail_magic_byte==TRUE) ? "R":"W", /* trail magic byte check */
            buffer_info.buf_len,  /*bufer size*/
            buffer_info.buffer_addr, /*buffer address*/
            (UI8_T)L_MM_GET_MODULEID_FROM_USERID(buffer_info.uid) /* MOD_ID */);

        switch(uid_display_mode)
        {
            case L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE:
            default:
                BACKDOOR_MGR_Printf("     %1hu    %2hu \n",
                    (UI8_T)L_MM_GET_POOLID_FROM_USERID(buffer_info.uid), /* POOL_ID */
                    (UI8_T)L_MM_GET_TRACEID_FROM_USERID(buffer_info.uid) /* TRACE_ID */);
                break;
            case L_MM_BACKDOOR_DISPLAY_EXT_TRACE:
                BACKDOOR_MGR_Printf("    %3hu      \n", (UI8_T)L_MM_GET_EXT_TRACEID_FROM_USERID(buffer_info.uid) /* TRACE_ID */);
                break;
        }


    } /* end of while((BOOL_T)... */
} /* end of L_MM_BACKDOOR_ShowBufferInfo() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_BACKDOOR_ShowBufferInfo
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Display Monitored Buffer information by given conditions
 *
 * INPUT:
 *  task_id      -- specify the task_id to display (or L_MM_SHOW_ALL_ID  to display all tasks)
 *  module_id    -- specify the module_id to display (or L_MM_SHOW_ALL_ID to display all modules)
 *  elapsed_time -- for those buffer with (current system time -allocation_time > elapsed_time)
 *                  are the condition to display (or 0 to display all)
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *   1.The given conditions (task_id, module_id, elapsed_time) will all be
 *     checked for display (They are ANDed, not ORed)
 *   2.The buffer info is retrieved from kernel one by one. So there are chances
 *     that the retrieved monitored buffer is freed so that there is no way to
 *     proceed to next monitored buffer.
 *------------------------------------------------------------------------------
 */
static void K_L_MM_BACKDOOR_ShowBufferInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time)
{
    L_MM_Backdoor_BufferInfo_T buffer_info;
    UI32_T current_time = SYSFUN_GetSysTick();
    UI32_T check_time = current_time - elapsed_time;
    UI32_T calc_elapsed_time;
    char *taskname_p;
    char taskname[SYSFUN_TASK_NAME_LENGTH+1];
    BOOL_T is_normal_scenario = (current_time>check_time) ? TRUE : FALSE;

    /* There are two scenarios should be considered for elapsed_time checking:
     * The first scenario is Normal Scenario:
     *     If the range of timestamp we're going to check is not
     *     across the maximum value of UI32_T(i.e. 0xFFFFFFFF).
     * The other scenario is Start-over Scenario:
     *     If the range of timestamp we're going to check is across the boundary of the maximum value of
     *     UI32_T.
     *
     * Normal Scenario is illustrated as the chart shown below:
     *  |***********|xxxxxxxxxxxxxxxxx|------------------>
     *  0           ^                 ^
     *              |<--------------->|
     *              |   elapsed_time  |
     *              +->check_time     |
     *                                +->current_time
     * Assume sys tick starts from 0, and the time stamp meet the condition should fall within the range
     * marked as '*'. If the time stamp fall in the range marked as 'x', that means life time of the buffer
     * is smaller than elpased_time. Thus the checking condition for Normal Scenario would be:
     *              time_stamp < check_time
     * Further more, if the sys tick has been across 0xFFFFFFFF, there might be some buffer with timestamp greater
     * than current_time, those buffer should also be included. At last, the checking condition would be:
     *              (time_stamp < check_time) || (time_stamp>current_time)
     * However, if the timestamp is assigned before sys tick was across 0xFFFFFFFF and the timestamp happens to
     * fall in 'x' range, we would consider the buffer doesn't meet the condition but actually it does meet.
     *
     * Start-over Scenario is illustrated as the chart shown below:
     *  |xxxxxx|**************** ... *******************|xxxxxxxxxxxxxxxxxx|
     *  0      ^                                        ^                  ^(0xFFFFFFFF)
     *  |<---->|                                        |<---------------->|
     *     a   |                                        |        b
     *         +->current_time                          +->check_time
     * The meaning of legends '*' and 'x' is the same as in Normal Condition.
     * Assume a + b == elapsed_time.
     * The checking condition for this scenario would be:
     *              (time_stamp < check_time) && (time_stamp > current_time)
     */

    BACKDOOR_MGR_Printf("\r\n   TASK_ID TASK_NAME    ELAPSEDTM MAG BUF_T M_ID %s   BUF_SZ BUF_ADDR", header_str[uid_display_mode]);
    BACKDOOR_MGR_Print("\r\n------------------------------------------------------------------------------");

    buffer_info.last_ref_monitor_p=NULL;

    while(((BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR,
        (void*)L_MM_BACKDOOR_CMD_GETBUFFERINFO, &buffer_info, (void*)0, (void*)0))
        == TRUE)
    {
        /* check whether task_id mached */
        if ( (task_id != L_MM_SHOW_ALL_ID) && (task_id != buffer_info.task_id) )
            continue;

        /* check whether module_id matched */
        if ( (module_id != L_MM_SHOW_ALL_ID) && (module_id != (L_MM_GET_MODULEID_FROM_USERID(buffer_info.uid)) ) )
            continue;

        /* Check whether elapsed time matched */
        if(elapsed_time!=0)
        {
            if ( is_normal_scenario == TRUE )
            {
                if ( (buffer_info.allocation_time> check_time) &&
                     (buffer_info.allocation_time <= current_time)
                   )
                    continue;
            } /* end of if( is_normal_scenario == TRUE ) */
            else /* is_normal_scenario == */
            {
                if ( (buffer_info.allocation_time > check_time) ||
                     (buffer_info.allocation_time <= current_time)
                   )
                   continue;
            } /* end of else */
        } /* end of if(elapsed_time!=0) */

        /* Display buffer info for matched buffer */
        /* Sample print out format:
         *   TASK_ID TASK_NAME    ELAPSEDTM MAG BUF_T M_ID PL_ID TR_ID   BUF_SZ BUF_ADDR
         *------------------------------------------------------------------------------
         *0x12345678 Sample_Task 1234567890 R,R  0,63   123   1     12 12345678 001A0000
         */

        if(SYSFUN_OK != SYSFUN_TaskIDToName(buffer_info.task_id, &(taskname[0]), sizeof(taskname)))
            taskname_p = "Unknown";
        else
            taskname_p=&(taskname[0]);

        if(buffer_info.allocation_time<=current_time)
            calc_elapsed_time = current_time - buffer_info.allocation_time;
        else
            calc_elapsed_time = 0xFFFFFFFF - (buffer_info.allocation_time - current_time);

        BACKDOOR_MGR_Printf("\r\n0x%8lx %-11s %10lu %1s,%1s  %1hu,%1hu   %3hu",
            buffer_info.task_id, /* TASK_ID */
            taskname_p,          /* TASK_NAME */
            calc_elapsed_time,   /* ELAPSEDTM */
            (buffer_info.is_valid_leading_magic_byte==TRUE) ? "R":"W", /* leading magic byte check */
            (buffer_info.is_valid_trail_magic_byte==TRUE) ? "R":"W", /* trail magic byte check */
            (UI8_T)L_MM_GET_INTERNAL_BUFTYPE(buffer_info.buffer_type), /* BUF_TYPE(Internal) */
            (UI8_T)L_MM_GET_APP_BUFTYPE(buffer_info.buffer_type), /* BUF_TYPE(Application defined) */
            (UI8_T)L_MM_GET_MODULEID_FROM_USERID(buffer_info.uid) /* MOD_ID */);

        switch(uid_display_mode)
        {
            case L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE:
            default:
                BACKDOOR_MGR_Printf("     %1hu    %2hu ", 
                    (UI8_T)L_MM_GET_POOLID_FROM_USERID(buffer_info.uid), /* POOL_ID */
                    (UI8_T)L_MM_GET_TRACEID_FROM_USERID(buffer_info.uid) /* TRACE_ID */);
                break;
            case L_MM_BACKDOOR_DISPLAY_EXT_TRACE:
                BACKDOOR_MGR_Printf("    %3hu      ", (UI8_T)L_MM_GET_EXT_TRACEID_FROM_USERID(buffer_info.uid) /* TRACE_ID */);
                break;
        }

        BACKDOOR_MGR_Printf("%8hu %p",
               buffer_info.buf_len /* buffer length */,
               buffer_info.buffer_addr /* buffer address */);
    } /* end of while((BOOL_T)... */
} /* end of K_L_MM_BACKDOOR_ShowBufferInfo() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_BACKDOOR_ShowMrefInfo
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Display multiple reference information by given conditions
 *------------------------------------------------------------------------------
 * INPUT:
 *  task_id      -- specify the task_id to display (or L_MM_SHOW_ALL_ID  to display all tasks)
 *  module_id    -- specify the module_id to display (or L_MM_SHOW_ALL_ID to display all modules)
 *  elapsed_time -- for those mref with (current system time -allocation_time > elapsed_time)
 *                  are the condition to display (or 0 to display all)
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *   1.The given conditions (task_id, module_id, elapsed_time) will all be
 *     checked for display (They are ANDed, not ORed)
 *   2.SYSFUN_InterruptLock/SYSFUN_InterruptUnlock are needed in this procedure
 */
static void K_L_MM_BACKDOOR_ShowMrefInfo(UI32_T task_id, UI32_T module_id, UI32_T elapsed_time)
{
    L_MM_Backdoor_MrefInfo_T mref_info;
    /*char*  taskname_p;*/
    /*char taskname[SYSFUN_TASK_NAME_LENGTH+1];*/
    UI32_T current_time = SYSFUN_GetSysTick();
    UI32_T check_time = current_time - elapsed_time;
    /*UI32_T calc_elapsed_time;*/
    BOOL_T is_normal_scenario = (current_time>check_time) ? TRUE : FALSE;

    /* See comment in L_MM_BACKDOOR_ShowBufferInfo() for description of Normal Scenario and other scenario. */

    BACKDOOR_MGR_Printf("\r\nM_ID %s CUR_USR NXT_USR REF_CNT BUF_LEN PDU_OFFSET PDU_LEN BUF_ADDR", header_str[uid_display_mode]);
    BACKDOOR_MGR_Print("\r\n----------------------------------------------------------------------------");

    mref_info.buffer_info.last_ref_monitor_p=NULL;
    while(((BOOL_T)SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR,
        (void*)L_MM_BACKDOOR_CMD_GETMREFINFO,
        &mref_info, NULL, NULL)) == TRUE)
    {
        /* check whether buffer type matched */
        if(0==(L_MM_GET_INTERNAL_BUFTYPE(mref_info.buffer_info.buffer_type) & L_MM_MREF_BUFFER_TYPE))
            continue;

        /* check whether task_id matched */
        if ( (task_id != L_MM_SHOW_ALL_ID) && (task_id != mref_info.buffer_info.task_id) )
            continue;

        /* check whether module_id matched */
        if ( (module_id != L_MM_SHOW_ALL_ID) && (module_id != (L_MM_GET_MODULEID_FROM_USERID(mref_info.buffer_info.uid)) ) )
            continue;

        /* Check whether elapsed time matched */
        if(elapsed_time!=0)
        {
            if ( is_normal_scenario == TRUE )
            {
                if ( (mref_info.buffer_info.allocation_time > check_time) &&
                     (mref_info.buffer_info.allocation_time <= current_time)
                   )
                    continue;
            } /* end of if( is_normal_scenario == TRUE ) */
            else /* is_normal_scenario == */
            {
                if ( (mref_info.buffer_info.allocation_time > check_time) ||
                     (mref_info.buffer_info.allocation_time <= current_time)
                   )
                   continue;
            } /* end of else */
        } /* end of if(elapsed_time!=0) */

        /* Display mref info for matched mref */
        /* Sample print out format:
         *M_ID PL_ID TR_ID CUR_USR NXT_USR REF_CNT BUF_LEN PDU_OFFSET PDU_LEN BUF_ADDR
         *----------------------------------------------------------------------------
         * 123   1     12   0x1234  0x1234   12345   12345    12345     12345 010A0000
         */
        /*
        if(SYSFUN_OK != SYSFUN_TaskIDToName(mref_info.buffer_info.task_id, &(taskname[0]), sizeof(taskname)))
            taskname_p = "Unknown";
        else
            taskname_p = &(taskname[0]);
         */

        /*
        if(mref_info.buffer_info.allocation_time<=current_time)
            calc_elapsed_time = current_time - mref_info.buffer_info.allocation_time;
        else
            calc_elapsed_time = 0xFFFFFFFF - (mref_info.buffer_info.allocation_time - current_time);
         */

        BACKDOOR_MGR_Printf("\r\n %3hu ", (UI8_T)L_MM_GET_MODULEID_FROM_USERID(mref_info.buffer_info.uid) /* M_ID(module id) */);

        switch(uid_display_mode)
        {
            case L_MM_BACKDOOR_DISPLAY_POOL_AND_TRACE:
            default:
                BACKDOOR_MGR_Printf("  %1hu    %2hu", 
                    (UI8_T)L_MM_GET_POOLID_FROM_USERID(mref_info.buffer_info.uid), /* POOL_ID */
                    (UI8_T)L_MM_GET_TRACEID_FROM_USERID(mref_info.buffer_info.uid) /* TRACE_ID */);
                break;
            case L_MM_BACKDOOR_DISPLAY_EXT_TRACE:
                BACKDOOR_MGR_Printf("      %3hu",
		(UI8_T)L_MM_GET_EXT_TRACEID_FROM_USERID(mref_info.buffer_info.uid) /* TRACE_ID */);
                break;
        }

        BACKDOOR_MGR_Printf("    0x%.4x  0x%.4x   %5u   %5u      %5u   %5u %p",
            mref_info.current_usr_id, /* CUR_USR(current_usr_id) */
            mref_info.next_usr_id, /* NXT_USR(next_usr_id) */
            mref_info.ref_count, /* REF_CNT(reference count) */
            mref_info.buffer_info.buf_len, /* BUF_LEN(buffer length) */
            mref_info.pdu_offset, /* PDU_OFFSET */
            mref_info.pdu_len /* PDU_LEN */,
            mref_info.buffer_info.buffer_addr);
    } /* end of while((BOOL_T)... */
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_DumpBuffer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Dump specified Monitored Buffer
 *
 * INPUT:
 *  p            -- the physical address of Monitored Buffer
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static void L_MM_BACKDOOR_DumpBuffer(void *p)
{
#define L_MM_BACKDOOR_DUMP_MM_TAIL_MAGIC(p) \
    do {                                    \
        len = L_MM_TRAIL_MAGIC_BYTE_LEN;    \
                                            \
        if (show_mm_tail)                   \
        {                                   \
            BACKDOOR_MGR_Printf("\r\nL_MM_TRAIL_MAGIC: %#010lx (len:%lu)\r\n", p, len);             \
            BACKDOOR_MGR_DumpHex(NULL, len, p);   \
            show_mm_tail = FALSE;           \
        }                                   \
    } while (0)


    L_MM_Backdoor_BufferInfo_T buffer_info;
    L_MM_Monitor_T *monitor_p;
    UI32_T len;
    BOOL_T show_mm_tail;

    buffer_info.last_ref_monitor_p = NULL;

    while(L_MM_GetBufferInfo(&buffer_info) == TRUE)
    {
        if (buffer_info.buffer_addr == p)
        {
            monitor_p = p;
            len = sizeof(*monitor_p);

            BACKDOOR_MGR_Printf("\r\nL_MM_Monitor_T: %#010lx (len:%lu)\r\n", p, len);
            BACKDOOR_MGR_DumpHex(NULL, len, monitor_p);

            p += sizeof(*monitor_p);
            len = buffer_info.buf_len - sizeof(*monitor_p) - L_MM_TRAIL_MAGIC_BYTE_LEN;
            show_mm_tail = TRUE;

            BACKDOOR_MGR_Printf("\r\nBuffer: %#010lx (len:%lu)\r\n", p, len);
            BACKDOOR_MGR_DumpHex(NULL, len, p);

            p += len;
            L_MM_BACKDOOR_DUMP_MM_TAIL_MAGIC(p);

            break;
        }
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : K_L_MM_BACKDOOR_DumpBuffer
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Dump specified Monitored Buffer
 *
 * INPUT:
 *  p            -- the physical address of Monitored Buffer
 *
 * OUTPUT:
 *  None.
 *
 * RETURN:
 *  None.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static void K_L_MM_BACKDOOR_DumpBuffer(void *p)
{
#define K_L_MM_BACKDOOR_DUMP_MM_TAIL_MAGIC(p) \
    do {                                    \
        len = L_MM_TRAIL_MAGIC_BYTE_LEN;    \
                                            \
        if (show_mm_tail)                   \
        {                                   \
            BACKDOOR_MGR_Printf("\r\nL_MM_TRAIL_MAGIC: %#010lx (len:%lu)\r\n", p, len);             \
            BACKDOOR_MGR_DumpHex(NULL, len, L_IPCMEM_GetPtr(L_IPCMEM_GetOffsetOfPhysicalPtr(p)));   \
            show_mm_tail = FALSE;           \
        }                                   \
    } while (0)


    L_MM_Backdoor_BufferInfo_T buffer_info;
    L_MM_Monitor_T *monitor_p;
    L_MM_Mref_T *mref_p;
    UI32_T len;
    BOOL_T show_mm_tail;

    buffer_info.last_ref_monitor_p = NULL;

    while(SYSFUN_Syscall(SYSFUN_SYSCALL_L_MM, (void*)L_MM_CMD_BACKDOOR,
        (void*)L_MM_BACKDOOR_CMD_GETBUFFERINFO, &buffer_info, (void*)0, (void*)0))
    {
        if (buffer_info.buffer_addr == p)
        {
            monitor_p = L_IPCMEM_GetPtr(L_IPCMEM_GetOffsetOfPhysicalPtr(p));
            len = sizeof(*monitor_p);

            BACKDOOR_MGR_Printf("\r\nL_MM_Monitor_T: %#010lx (len:%lu)\r\n", p, len);
            BACKDOOR_MGR_DumpHex(NULL, len, monitor_p);

            p += sizeof(*monitor_p);
            len = buffer_info.buf_len - sizeof(*monitor_p) - L_MM_TRAIL_MAGIC_BYTE_LEN;
            show_mm_tail = TRUE;

            if (monitor_p->buffer_type == L_MM_CONVERT_TO_INTERNAL_BUFTYPE(L_MM_MREF_BUFFER_TYPE))
            {
                mref_p = L_IPCMEM_GetPtr(L_IPCMEM_GetOffsetOfPhysicalPtr(p));

                BACKDOOR_MGR_Printf("\r\nL_MM_Mref_T: %#010lx (len:%lu, pdu_offset: %lu, pdu_len:%lu) \r\n",
                    p, len, mref_p->pdu_offset, mref_p->pdu_len);
                BACKDOOR_MGR_DumpHex(NULL, len, mref_p);

                p += len;
                K_L_MM_BACKDOOR_DUMP_MM_TAIL_MAGIC(p);

                p = mref_p->buffer_addr;
                len = mref_p->buf_len;
            }

            BACKDOOR_MGR_Printf("\r\nBuffer: %#010lx (len:%lu)\r\n", p, len);
            BACKDOOR_MGR_DumpHex(NULL, len, L_IPCMEM_GetPtr(L_IPCMEM_GetOffsetOfPhysicalPtr(p)));

            p += len;
            K_L_MM_BACKDOOR_DUMP_MM_TAIL_MAGIC(p);

            break;
        }
    }
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_GetTaskIdFromConsole
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get task id(hex value) from console.
 * INPUT:
 *  None.
 * OUTPUT:
 *  value_p  -- task id which is input from console will be stored in this variable.
 *
 * RETURN:
 *  TRUE  -- Get valid task id from console.
 *  FALSE -- Get invalid task id from console.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_BACKDOOR_GetTaskIdFromConsole(UI32_T *value_p)
{
    char str_buf[16], *ch_p;

    BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

    if(str_buf[0] == '*')
    {
        *value_p = L_MM_SHOW_ALL_ID;
        return TRUE;
    }

    ch_p = str_buf;
    while( (*ch_p) != 0 )
    {
        /* validate input value */
        if(((*ch_p < '0') || (*ch_p > '9')) &&
           ((*ch_p < 'a') || (*ch_p > 'f')) &&
           ((*ch_p < 'A') || (*ch_p > 'F'))
          )
            return FALSE;
        ch_p++;
    }

    *value_p = (UI32_T)strtoul(str_buf, NULL, 16);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_GetModuleIdFromConsole
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get module id(decimal value) from console.
 * INPUT:
 *  None.
 * OUTPUT:
 *  value_p  -- module id which is input from console will be stored in this variable.
 *
 * RETURN:
 *  TRUE  -- Get valid module id from console.
 *  FALSE -- Get invalid module id from console.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_BACKDOOR_GetModuleIdFromConsole(UI32_T *value_p)
{
    char str_buf[16], *ch_p;

    BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

    if(str_buf[0] == '*')
    {
        *value_p = L_MM_SHOW_ALL_ID;
        return TRUE;
    }

    ch_p = str_buf;
    while( (*ch_p) != 0 )
    {
        /* validate input value */
        if((*ch_p < '0') || (*ch_p > '9'))
            return FALSE;
        ch_p++;
    }

    *value_p = (UI32_T)atol(str_buf);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_GetElapsedTimeFromConsole
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get elapsed time from console.
 * INPUT:
 *  None.
 * OUTPUT:
 *  value_p  -- elapsed time from console will be stored in this variable.
 *
 * RETURN:
 *  TRUE  -- Get valid value from console.
 *  FALSE -- Get invalid value from console.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_BACKDOOR_GetElapsedTimeFromConsole(UI32_T *value_p)
{
    char str_buf[16], *ch_p;

    BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

    ch_p = str_buf;
    while( (*ch_p) != 0 )
    {
        /* validate input value */
        if((*ch_p < '0') || (*ch_p > '9'))
            return FALSE;
        ch_p++;
    }

    *value_p = (UI32_T)atol(str_buf);
    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_GetAddressFromConsole
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get memory address from console.
 * INPUT:
 *  None.
 * OUTPUT:
 *  pp  -- memory address from console will be stored in this variable.
 *
 * RETURN:
 *  TRUE  -- Get valid value from console.
 *  FALSE -- Get invalid value from console.
 *
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_BACKDOOR_GetAddressFromConsole(void **pp)
{
    char str_buf[20];

    BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

    if (1 > sscanf(str_buf, "0x%lx", (unsigned long *)pp) &&
        1 > sscanf(str_buf, "%lx", (unsigned long *)pp))
    {
        return FALSE;
    }

    return TRUE;
}
/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_ShowProcessName
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Show all process name.
 * INPUT:
 *  None.
 * OUTPUT:
 *  None
 * RETURN:
 *  none
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static void L_MM_BACKDOOR_ShowProcessName(void)
{
    UI32_T counter;
    char *sys_bld_string[] = {SYSBLD_LIST(MODULE_NAME)};
    const UI32_T sysbld_size = sizeof(sys_bld_string)/sizeof(char *); 
    for (counter = 0; counter < sysbld_size; counter++)
    BACKDOOR_MGR_Printf("\r\n%d. %s", counter+1, sys_bld_string[counter]);
    BACKDOOR_MGR_Printf("\r\n----------------------------------------------------------------------------\n");
} /* end of L_MM_BACKDOOR_ShowProcessName() */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : L_MM_BACKDOOR_GetProcessNameFromConsole
 *------------------------------------------------------------------------------
 * PURPOSE:
 *  Get the process number from console.
 * INPUT:
 *  None.
 * OUTPUT:
 *  value_p  -- process number which is input from console will be stored in this variable.
 *
 * RETURN:
 *  TRUE   -- Get valid process number from console.
 *  FALSE  -- Get invalid process number from console.
 * NOTES:
 *  None.
 *------------------------------------------------------------------------------
 */
static BOOL_T L_MM_BACKDOOR_GetProcessNumberFromConsole(UI32_T *value_p)
{
    char str_buf[16], *ch_p;

    BACKDOOR_MGR_RequestKeyIn(str_buf, sizeof(str_buf)-1);

    ch_p = str_buf;
    while( (*ch_p) != 0 )
    {
        /* validate input value */
        if((*ch_p < '0') || (*ch_p > '9'))
            return FALSE;
        ch_p++;
    }

    *value_p = (UI32_T)atol(str_buf);
    return TRUE;
}
