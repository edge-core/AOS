#include <stdio.h>
//#define _XOPEN_SOURCE 600
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
//#include <pthread.h>
#include <errno.h>
#include <fcntl.h>        /* O_RDWR, O_NOCTTY, O_NONBLOCK */
#include <termios.h>      /* for tcgetattr(), tcsetattr(), ... */
#include <sys/ioctl.h>

#include "sys_cpnt.h"
#include "sys_adpt.h"
#include "cli_api.h"
#include "cli_def.h"
#include "cli_io.h"
#include "sysfun.h"
#include "sys_pmgr.h"

#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
#include "sw_watchdog_mgr.h"
#endif

extern int posix_openpt(int);
extern int grantpt(int);
extern int unlockpt(int);
extern char *ptsname(int);
//extern UI8_T CLI_IO_ReadFromTelnet(CLI_TASK_WorkingArea_T *ctrl_P);

static BOOL_T CLI_API_FileExists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")) != NULL)
    {
        fclose(file);
        return TRUE;
    }
    return FALSE;
}

static int CLI_API_LinuxShell_TelnetSSH(CLI_TASK_WorkingArea_T *ctrl_P)
{
    pid_t pid;
//    char *arg_list[] = {"sh","-l",NULL};    
    int fdm, fds;
    int rc;

    fdm = posix_openpt(O_RDWR);
    if (fdm < 0)
    {
        fprintf(stderr, "Error %d on posix_openpt()\n", errno);
        return -1;
    }

    /* Because grantpt may create a child process, SUSv3 says that the behavior of grantpt()
     * is unspecified if the calling program has installed a handler for SIGCHLD. 
     */
    signal(SIGCHLD, SIG_IGN);
    
    /* changes the ownership and permissions of the slave
     * device corresponding to a pseudoterminal master device 
     */    
    rc = grantpt(fdm); 
    if (rc != 0)
    {
        fprintf(stderr, "Error %d on grantpt()\n", errno);
        close(fdm);        
        return 1;
    }
    signal(SIGCHLD, SIG_DFL); /* ?? whether the default handler will exit the current thread?? */

    /* unlocks the slave pseudo-terminal device corresponding to the master pseudo-terminal 
     * device associated with the file descriptor of master device. 
     * On many systems, the slave can only be opened after unlocking, so portable applications 
     * should always call unlockpt before trying to open the slave. 
     */
    rc = unlockpt(fdm);
    if (rc != 0)
    {
        fprintf(stderr, "Error %d on unlockpt()\n", errno);
        close(fdm);        
        return 1;
    }

    fds = open(ptsname(fdm), O_RDWR); /* Open the slave side ot the PTY */

    pid = fork();
      
    if (pid < 0)
    {
        /* critical error, print error message directly
         */
        CLI_LIB_PrintStr("\r\nfork() fail! \r\n");
        close(fdm);
        close(fds);
        return -1;
    }
    else if (pid > 0)
    {
        /* parent process 
         */
        fd_set ibits;
        BOOL_T end = FALSE;
        struct timeval timeout;
        int len, max_val, flags, c_pid;
        int ret, status=-1;
        UI8_T  ch, ch_lf=10 /* Line Feed */;
        
        close(fds);      /* Close the slave side of the PTY */

        flags = fcntl(fdm, F_GETFL, 0);
        if (fcntl(fdm, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            CLI_LIB_PrintStr("\r\nfcntl set non-block error \r\n"); /* debug print */
            SYSFUN_Kill(pid, 15 /* SIGTERM */);        
            end = TRUE;
        }
        
        max_val = ctrl_P->socket;
        if (fdm > max_val)
            max_val = fdm;
        
        while (end != TRUE)
        {
            timeout.tv_sec  = 1;       //  no. of seconds to block
            timeout.tv_usec = 0;       //  no. of micro seconds to block        
            FD_ZERO(&ibits);
            FD_SET(fdm, &ibits);
            FD_SET(ctrl_P->socket, &ibits);

            if ((ret = select(max_val+1, (fd_set *)&ibits, NULL, NULL, &timeout)) > 0)
            {
               if (FD_ISSET(fdm, &ibits))
                {
                    while ((len = read(fdm, ctrl_P->send_buf, 1)) > 0) 
                    {   
                        if (ctrl_P->send_buf[0] == 10) /* LF */
                        {
                            ctrl_P->send_buf[1] = '\0'; /* LF */
                            CLI_IO_PrintOut(ctrl_P);                        
                            ctrl_P->send_buf[0] = 13; /* CR */
                            ctrl_P->send_buf[1] = '\0';
                            CLI_IO_PrintOut(ctrl_P);
                        }                        
                        else
                        {
                            ctrl_P->send_buf[1] = '\0';
                            CLI_IO_PrintOut(ctrl_P);
                        }
                    }
                    if (errno != EAGAIN /* no data */ && errno != EWOULDBLOCK && len != 0)
                    {
                        end = TRUE;
                    }
                }
               
                if (FD_ISSET(ctrl_P->socket, &ibits))
                {
                    while((ret = recv(ctrl_P->socket, (UI8_T *)&ch, 1, 0)) > 0)
                    {
                        switch (ch)
                        {
                            case 4: /* Ctrl-D */
                            default:
                                if (write(fdm, &ch, 1) == -1)
                                {
                                    end = TRUE;
                                }
                               break;
                        }
                        
                    }
                    if (errno != EAGAIN && errno != EWOULDBLOCK && ret == -1)
                    {
                        end = TRUE;
                    }
                }
            }
            else if (ret == 0 /* select time out */ || errno == EINTR /* interrupted by signal */)
            {
            }
            else /* select error */
            {
                end = TRUE;
            }
        
            c_pid = waitpid(pid, &status, WNOHANG);

            if (c_pid > 0) /* c_pid=child pid --> child is exited, c_pid=0 -> child is not yet exited */
            {
                end = TRUE;            
            }
        }
        close(fdm);
        return (status == 0 ? 0 : -1);
    }
    else 
    {
        /* the new created child process 
         */
        sigset_t block_mask;
        int fd = -1;
        struct termios termios_org, termios_new;

        close(fdm); /* Close the master side of the PTY */

        rc = tcgetattr(fds, &termios_org); /* Save the defaults parameters of the slave side of the PTY */
        termios_new = termios_org; 
        cfmakeraw (&termios_new); /* Set RAW mode on slave side of PTY */
        termios_new.c_lflag |= ECHO | ICANON | IEXTEN | ISIG;
        termios_new.c_oflag |= OPOST;
        termios_new.c_iflag |= ICRNL | IXON;
        tcsetattr (fds, TCSANOW, &termios_new);

        /* The slave side of the PTY becomes the standard input and outputs of the child process
         */
        dup2(fds, 0);
        dup2(fds, 1);
        dup2(fds, 2);        

        setsid(); /* Make the current process a new session leader */

        /* As the child is a session leader, set the controlling terminal to be the slave side of the PTY
         * (Mandatory for programs like the shell to make them manage correctly their outputs)
         */
        ioctl(0, TIOCSCTTY, 1);

        close(fds); /* The original file descriptor is useless */

        sigemptyset(&block_mask);
        sigprocmask(SIG_SETMASK, &block_mask, NULL); /* unblock all signals */         
    }
    char * const envp[] = {"PATH=/sbin:/usr/sbin:/bin:/usr/bin", "HOME=/", "TERM=vt100", NULL};
    execle("/bin/sh", "sh", "-l", (char *)0, envp);    
 
    exit(0);
}

UI32_T CLI_API_LinuxShell(UI16_T cmd_idx, char *arg[], CLI_TASK_WorkingArea_T *ctrl_P)
{
    UI32_T task_id;
    UI32_T time_out_value = 0;
    
    switch (ctrl_P->sess_type)
    {
        case CLI_TYPE_UART:
            SYS_PMGR_GetRunningConsoleExecTimeOut(&time_out_value);
            SYS_PMGR_SetConsoleExecTimeOut(0); /* disable console timeout */
            
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
             SW_WATCHDOG_MGR_UnregisterMonitorThread(SW_WATCHDOG_CLI);
#endif            

            SYSFUN_TaskNameToID("sys_debug_proc", &task_id);
            SYSFUN_Kill(task_id, 10 /*SIGUSR1*/); /*   SYSFUN_InvokeLinuxShell(); */

            SYSFUN_Sleep(200); /* yield CPU resource to "sys_debug_proc" */

            /* while linux shell is still running, CLI should not take over the console control
             */
            while (CLI_API_FileExists(SYSFUN_FLAG_LINUX_SHELL_RUNNING) == TRUE)
            {
                SYSFUN_Sleep(200);
            }
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_CLI, ctrl_P->cli_tid, SYS_ADPT_CLI_SW_WATCHDOG_TIMER);
#endif             
            SYS_PMGR_SetConsoleExecTimeOut(time_out_value);
            break;
            
        case CLI_TYPE_TELNET:
        case CLI_TYPE_SSH:
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            SW_WATCHDOG_MGR_UnregisterMonitorThread(SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM+CLI_TASK_GetMySessId()-1);
#endif            
            CLI_LIB_PrintStr("\r\rEntering Linux Shell...\r\n");

            if (CLI_API_LinuxShell_TelnetSSH(ctrl_P) != 0)
            {
                CLI_LIB_PrintStr("\r\nFailed to create linux shell. \r\n");
            }
            CLI_LIB_PrintStr("\r\r---- Return from Linux Shell ----\r\n");
#if (SYS_CPNT_SW_WATCHDOG_TIMER == TRUE)
            SW_WATCHDOG_MGR_RegisterMonitorThread(SW_WATCHDOG_MAX_MONITOR_ID+SYS_ADPT_MAX_TELNET_NUM+CLI_TASK_GetMySessId()-1, ctrl_P->cli_tid, SYS_ADPT_TELNET_PARENT_SW_WATCHDOG_TIMER);
#endif   
            break;
        default:
            break;
   }
    return CLI_NO_ERROR;
}


