/* MODULE NAME:  sys_debug_proc.c
 * PURPOSE:
 *    for sys debug usage process.
 *
 * NOTES:
 *
 * REASON:
 * Description:
 * HISTORY
 *    8/05/2008 - Jin wang, Created
 *    2/19/2016 - Charlie Chen, Add test code for doing timer test for
 *                measuring the accuracy of the SYSFUN Timer Event.
 *   11/03/2016 - Charlie Chen, enhance code to have password validation when
 *                entering linux shell through "Ctrl+l" and no password validation
 *                when entering linux shell through CLI command "linux shell".
 *
 * Copyright(C)      Accton Corporation, 2007
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "sys_type.h"
#include "sys_bld.h"
#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "sysfun.h"

#ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
#include "bdpv_mgr.h"
#include "uc_mgr.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */
#define PASSWD_LEN  16
#define ASCII_MIN_VISABLE_CODE 32
#define ASCII_MAX_VISABLE_CODE 127

 
/* SYSFUN_TIMER_EVENT_TEST:
 *     When this constant is defined, sys_debug_proc will run performance test
 *     of SYSFUN timer event only. The program must be run after "acctonlkm.ko"
 *     had been loaded on the system.
 */
/* #define SYSFUN_TIMER_EVENT_TEST */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static BOOL_T SYS_DEBUG_PROC_Init(void);
static void SYS_DEBUG_PROC_SignalHandler(int sig_num);

/* STATIC VARIABLE DECLARATIONS
 */


/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_DEBUG_PROC_Main_Thread_Function_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *	  This is the entry function of the main thread for sys debug process. 
 *
 * INPUT:
 *	  None.
 *
 * OUTPUT:
 *	  None.
 *
 * RETURN:
 *	  None.
 *
 * NOTES:
 */

#ifndef SYSFUN_TIMER_EVENT_TEST
static void SYS_DEBUG_PROC_Main_Thread_Function_Entry(void)
{
    sigset_t sigset;

    sigfillset(&sigset);
    sigdelset(&sigset, SIGINT);
    sigdelset(&sigset, SIGUSR1);

    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYSFUN_SCHED_RR, SYSFUN_TaskIdSelf(), SYS_BLD_REALTIMEPROCESS_DEFAULT_PRIORITY))
    {
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);
    }

    SYSFUN_OpenDebugUART(SYSFUN_UART_CHANNEL1);

    while(1)
    {
    	//SYSFUN_Sleep (SYS_BLD_IDLE_TASK_SLEEP_PERIOD);
        SYSFUN_Sleep_Interruptible(3600);
    }
}
#else /* #ifndef SYSFUN_TIMER_EVENT_TEST */
#define TIMER_TEST_EVENT BIT_0
/* An arbitrary initial value for received_events to verify that the acctonlkm.ko
 * had been loaded
 */
#define RECEIVED_EVENTS_INIT_VAL 0x1234

/* You may adjust the constant variables defined here for doing timer performance
 * test under different conditions
 */
static const UI32_T time_interval=1; /* timer test interval in tick */
static const long   max_abs_err_in_microsec=30; /* maximum allowed timer error in microsecond */
static const int    total_test_count=100;

static struct timeval prev_time;

static void SYS_DEBUG_PROC_Main_Thread_Function_Entry(void)
{
    void   *timer_id;
    UI32_T received_events=RECEIVED_EVENTS_INIT_VAL, rc;
    int    count=0;
    int    err_timer_count=0;

    if(SYSFUN_OK!=SYSFUN_SetTaskPriority(SYSFUN_SCHED_RR, SYSFUN_TaskIdSelf(), 99))
    {
        printf("%s: SYSFUN_SetTaskPriority fail.\n", __FUNCTION__);
    }

    timer_id = SYSFUN_PeriodicTimer_Create();

    if (SYSFUN_PeriodicTimer_Start(timer_id, time_interval, TIMER_TEST_EVENT)==FALSE)
    {
        printf("\r\n%s: Start timer failed!\r\n", __FUNCTION__);
        return;
    }

    printf("Timer test start\r\n");

    while(1)
    {
        rc=SYSFUN_ReceiveEvent(TIMER_TEST_EVENT,
                               SYSFUN_EVENT_WAIT_ANY,
                               SYSFUN_TIMEOUT_WAIT_FOREVER,
                               &received_events);
        if (rc!=SYSFUN_OK)
        {
            printf("SYSFUN_ReceiveEvent error(rc=%lu)\n", rc);
            continue;
        }
        else if(received_events==RECEIVED_EVENTS_INIT_VAL)
        {
            printf("Error! acctonlkm.ko had not been loaded yet.\r\n");
            return;
        }

        if (received_events & TIMER_TEST_EVENT)
        {
            struct timeval  tv;

            gettimeofday(&tv, NULL);
            //printf("seconds: %lu microseconds: %lu\r\n", tv.tv_sec, tv.tv_usec);
            count++;

            if (count!=1)
            {
                long elapsed_microsec;
                long err_microsec;

                elapsed_microsec=(tv.tv_sec-prev_time.tv_sec)*1000000;
                elapsed_microsec=elapsed_microsec+((long)tv.tv_usec-(long)prev_time.tv_usec);
                err_microsec=abs((int)elapsed_microsec-(time_interval*10*1000));
                if (abs(err_microsec)>max_abs_err_in_microsec)
                {
                    printf("err_in_usec=%lu\n",err_microsec);
                    err_timer_count++;
                }
            }
            prev_time=tv;

            if (count>=total_test_count)
            {
                printf("\n%d inaccurate timer in %d timer events\n", err_timer_count,
                    total_test_count);
                return;
            }
        }
    }
}

#endif /* end of #ifndef SYSFUN_TIMER_EVENT_TEST */

#if 0
/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_DEBUG_PROC_InitiateProcessResources
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Initialize the resource used in sys debug process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_DEBUG_PROC_InitiateProcessResources(void)
{

    L_CMNLIB_INIT_InitiateProcessResources();

    if(UC_MGR_InitiateProcessResources() == FALSE)
    {
       printf(" UC_MGR_InitiateProcessResources fail\n");
       return FALSE;
    }

    return TRUE;
}
#endif

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_DEBUG_PROC_Init
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    Do initialization procedures for sys debug process.
 *
 * INPUT:
 *    None.
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    TRUE  -- Success
 *    FALSE -- Fail
 * NOTES:
 *    None
 *------------------------------------------------------------------------------
 */
static BOOL_T SYS_DEBUG_PROC_Init(void)
{
#ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
    if (UC_MGR_InitiateProcessResources()==FALSE)
    {
        printf("%s: UC_MGR_InitiateProcessResources fail\n", __FUNCTION__);
        return FALSE;
    }
#endif

    return TRUE;
}

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_DEBUG_PROC_Daemonize_Entry
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    After the process has been daemonized, the main thread of the process
 *    will call this function to start the main thread.
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
 *    None.
 *------------------------------------------------------------------------------
 */
static void SYS_DEBUG_PROC_Daemonize_Entry(void* arg)
{
	struct sigaction	act;
	int 				rc;
	
	/* For ctrl+c handle */
	act.sa_handler = SYS_DEBUG_PROC_SignalHandler;
	sigemptyset(&(act.sa_mask));
	act.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGINT, &act, NULL);
	if (rc != 0)
	{
		printf("sigaction for SIGINFO (Ctrl-C) fail, rc %d, errno %d\n", rc, errno);
	}

    /* For the case that need to enter linux shell through the CLI command "linux shell"
     */
    rc = sigaction(SIGUSR1, &act, NULL);
    if (rc != 0)
    {
        printf("sigaction for SIGURS1 fail, rc %d, errno %d\n", rc, errno);
    }

    SYS_DEBUG_PROC_Main_Thread_Function_Entry();
}


/* EXPORTED SUBPROGRAM BODIES
 */
/*------------------------------------------------------------------------------
 * ROUTINE NAME : main
 *------------------------------------------------------------------------------
 * PURPOSE:
 *    the main entry for SYS DEBUG process
 *
 * INPUT:
 *    argc     --  the size of the argv array
 *    argv     --  the array of arguments
 *
 * OUTPUT:
 *    None.
 *
 * RETURN:
 *    0 -- Success
 *   <0 -- Error
 * NOTES:
 *    This function is the entry point for SYS DEBUG process.
 *------------------------------------------------------------------------------
 */
int main(int argc, char* argv[])
{
    UI32_T process_id;

    if(SYS_DEBUG_PROC_Init()==FALSE)
    {
        printf("SYS_DEBUG_PROC_Init fail.\n");
        return -1;
    }

    if (SYSFUN_SpawnProcess(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                            SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                            (char *)"sys_debug_proc",
                            SYS_DEBUG_PROC_Daemonize_Entry,
                            NULL,
                            &process_id) != SYSFUN_OK)
    {
        printf("SYS_DEBUG_Process SYSFUN_SpawnProcess error.\n");
        return -1;
    }
    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */

/*------------------------------------------------------------------------------
 * ROUTINE NAME : SYS_DEBUG_PROC_SignalHandler
 *------------------------------------------------------------------------------
 * PURPOSE:
 *	  Signal handler function used in sys_debug_proc.
 *
 * INPUT:
 *	  sig_num - the signal number caught by this process
 *
 * OUTPUT:
 *	  None.
 *
 * RETURN:
 *	  None.
 *
 * NOTES:
 */
static void SYS_DEBUG_PROC_SignalHandler(int sig_num)
{
    struct termios orig_termios, termios;
    int i=0, ret;
    char ch, passwd[PASSWD_LEN+1];
#ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
    int passwd_len;
    char org_passwd_last_char;
#endif
    BOOL_T is_validated=FALSE;

    if (sig_num==SIGUSR1)
    {
        /* This signal is triggered by cli command "linux shell"
         * Do not need to input password.
         */
        SYSFUN_InvokeLinuxShell();
        return;
    }

    memset(passwd, 0, PASSWD_LEN+1);
    ret = tcgetattr(0, &termios);    if (ret != 0)
    {
        return;
    }

    memcpy(&orig_termios, &termios, sizeof(orig_termios));

    /* Use default TTY settings
     */
    termios.c_iflag |= TTYDEF_IFLAG;
    termios.c_oflag |= TTYDEF_OFLAG;
    termios.c_lflag |= TTYDEF_LFLAG;
    /* Turn off ECHO so that the input password characters will not be displayed
     */
    termios.c_lflag &= ~ECHO;

    tcsetattr(0, TCSANOW, &termios);

    /* Get input password characters
     */
    while ((ch = (char)getchar()) != (char)EOF)
    {
        if (ch == '\r' || ch == '\n')
        {
            break;
        }

        passwd[i] = ch;
        if (++i == PASSWD_LEN)
        {
            break;
        }
    }
#ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
    passwd_len=i;
    org_passwd_last_char=passwd[passwd_len-1];
#endif
    /* Restore the original TTY settings
     */
    tcsetattr(0, TCSANOW, &orig_termios);

#ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
    /* Convert the last char to get the equivalent BPDV password
     * Ctrl+L linux shell password uses the same password as BPDV password
     * except that the last char of the linux shell password is ->
     * ("BPDV password last char" + 1)
     * The range of visible ASCII code is 32 to 127
     */
    passwd[passwd_len-1]-=1;
    if ((passwd[passwd_len-1]<ASCII_MIN_VISABLE_CODE) || (passwd[passwd_len-1]>ASCII_MAX_VISABLE_CODE))
    {
        /* it is impossible to have converted char that is outside of visible
         * ASCII code
         */
        return;
    }
#endif /* end of #ifndef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT */

#ifdef SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT
    if (strcmp(passwd,SYS_ADPT_CLI_BACKDOOR_PASSWORD_COMPONENT)==0)
    {
        SYSFUN_InvokeLinuxShell();
    }
#else
    /* Check the password by BDPV_MGR
     */
    if (BDPV_MGR_Validate(passwd)==TRUE)
    {
        SYSFUN_InvokeLinuxShell();
    }
    else if (org_passwd_last_char==(ASCII_MAX_VISABLE_CODE-2))
    {
        /* Both BPDV last char (ASCII_MAX_VISABLE_CODE) and char (ASCII_MAX_VISABLE_CODE-2) map to Linux char (ASCII_MAX_VISABLE_CODE-1)
         * Try to validate with char ASCII_MAX_VISABLE_CODE-2 in this case
         */
        passwd[passwd_len-1]=ASCII_MAX_VISABLE_CODE;
        if (BDPV_MGR_Validate(passwd)==TRUE)
        {
            SYSFUN_InvokeLinuxShell();
        }
    }
#endif

}

