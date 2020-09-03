/* MODULE NAME:  SYSFUN_BACKDOOR.c
 * PURPOSE:
 *     Backdoor functions of SYSFUN.
 *
 * NOTES:
 *     None.
 *
 * HISTORY
 *    2009/11/23 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2009
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <ctype.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>

/* accton include files */

#include "sys_type.h"
#include "sysfun.h"
#include "backdoor_mgr.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define UPLOAD_EXCEPTION_LOG_SCRIPT_FILE "/etc/upload_exception_log.sh"
#define EXCEPTION_LOG_DIRNAME "/flash/exception_log"
#define EXCEPTION_LOG_FILENAME_PREFIX "exception_log"

#define UPLOAD_ALL_LOG_SCRIPT_FILE "/etc/upload_all_flash_logs.sh"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct {
    UI32_T task_id;
    UI32_T user_ticks;
    UI32_T sys_ticks;
} SYSFUN_BACKDOOR_CpuUsage_T;

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void SYSFUN_BACKDOOR_LoadTestMain(void *arg);
static BOOL_T SYSFUN_BACKDOOR_StartLoadTest(void);
static void SYSFUN_BACKDOOR_CpuUsageMain(void);
static int SYSFUN_BACKDOOR_CpuUsageCompare(const void *e1, const void *e2);

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T load_test_is_running = FALSE;

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME : SYSFUN_BACKDOOR_BackDoorMenu
 * PURPOSE:
 *      Backdoor menu of sysfun backdoor.
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

void SYSFUN_BACKDOOR_BackDoorMenu(void)
{
    int ch;
    BOOL_T  eof=FALSE;
    /*  BODY
     */

    while (!eof)
    {
        BACKDOOR_MGR_Printf("\r\n 0. Exit\r\n");
        BACKDOOR_MGR_Printf(" 1. Upload exception log file by TFTP.\r\n");
        BACKDOOR_MGR_Printf(" 2. Load test. (%s)\r\n", load_test_is_running ? "Running" : "Stop");
        BACKDOOR_MGR_Printf(" 3. CPU usage.\r\n");
        BACKDOOR_MGR_Printf(" 4. Dump EH Buf info.\r\n");
        BACKDOOR_MGR_Printf(" 5. Pack all types of logs on flash and upload by TFTP \r\n");
        BACKDOOR_MGR_Printf("     select =");
        ch = BACKDOOR_MGR_GetChar();
        if ((ch < 0x30)||(ch>0x39))
            continue;
        ch -= 0x30;
        BACKDOOR_MGR_Printf("%d\r\n",ch);
        switch (ch)
        {
            case 0 :
                eof = TRUE;
                break;
            case 1 :
            {
                struct stat file_stat;
                int    rc;
                char   ip_buffer[28];
                char   sys_buffer[64];
                DIR   *dir;
                struct dirent *dirent_p;
                BOOL_T is_found;

                rc = stat(UPLOAD_EXCEPTION_LOG_SCRIPT_FILE, &file_stat);
                if(rc!=0)
                {
                    BACKDOOR_MGR_Print("Do not support yet.\r\n");
                    break;
                }

                /* check whether exception log exists
                 */
                is_found=FALSE;
                dir = opendir(EXCEPTION_LOG_DIRNAME);
                if(dir==NULL)
                {
                    goto DIRECTORY_NOT_EXIST;
                }

                is_found=FALSE;
                while((dirent_p = readdir(dir))!=NULL)
                {
                    if(!strncmp(dirent_p->d_name, EXCEPTION_LOG_FILENAME_PREFIX, sizeof(EXCEPTION_LOG_FILENAME_PREFIX)-1))
                    {
                        is_found=TRUE;
                        break;
                    }
                }

                closedir(dir);

DIRECTORY_NOT_EXIST:
                /* do upload exception log files if they exist
                 */
                if(is_found==TRUE)
                {
                    BACKDOOR_MGR_Print("Please ensure that IP of this device had been configured properly\r\n");
                    BACKDOOR_MGR_Print("TFTP server ip: ");
                    BACKDOOR_MGR_RequestKeyIn(ip_buffer, sizeof(ip_buffer)-1);
                    BACKDOOR_MGR_Print("\r\n");
                    sprintf(sys_buffer, "%s %s", UPLOAD_EXCEPTION_LOG_SCRIPT_FILE, ip_buffer);
                    SYSFUN_ExecuteSystemShell(sys_buffer);
                    BACKDOOR_MGR_Print("Done.\r\n");
                }
                else
                    BACKDOOR_MGR_Print("No exception log on the device\r\n");

            }
                break;
            case 2 :
            {
                if (load_test_is_running)
                {
                    load_test_is_running = FALSE;
                }
                else
                {
                    load_test_is_running = TRUE;
                    SYSFUN_BACKDOOR_StartLoadTest();
                }
            }
                break;
            case 3:
                SYSFUN_BACKDOOR_CpuUsageMain();
                break;
            case 4 :
                SYSFUN_DumpEHBufInfo();
                break;
            case 5 :
            {
                struct stat file_stat;
                int    rc;
                char   ip_buffer[28];
                char   sys_buffer[64];

                rc = stat(UPLOAD_ALL_LOG_SCRIPT_FILE, &file_stat);
                if(rc!=0)
                {
                    BACKDOOR_MGR_Print("Do not support yet.\r\n");
                    break;
                }

                BACKDOOR_MGR_Print("Please ensure that IP of this device had been configured properly\r\n");
                BACKDOOR_MGR_Print("TFTP server ip: ");
                BACKDOOR_MGR_RequestKeyIn(ip_buffer, sizeof(ip_buffer)-1);
                BACKDOOR_MGR_Print("\r\n");
                sprintf(sys_buffer, "%s %s", UPLOAD_ALL_LOG_SCRIPT_FILE, ip_buffer);
                SYSFUN_ExecuteSystemShell(sys_buffer);
                BACKDOOR_MGR_Print("Done.\r\n");
            }
                break;
            default :
                ch = 0;
                break;
        }
    }   /*  end of while    */

    return;
}   /*  SYSFUN_BackDoor_Menu */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME : SYSFUN_BACKDOOR_LoadTestMain
 * PURPOSE:
 *      Main entry for Load Test
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
static void SYSFUN_BACKDOOR_LoadTestMain(void *arg)
{
    unsigned long test_buf_size = SYS_TYPE_1K_BYTES;
    void *test_buf = NULL;
    int try_realloc = 1;
    int pattern;

    while (test_buf_size && !test_buf)
    {
        if (!(test_buf = malloc(test_buf_size)))
        {
            test_buf_size >>= 1;
            continue;
        }

        if (try_realloc)
        {
            do
            {
                test_buf_size <<= 1;
                free(test_buf);
                test_buf = malloc(test_buf_size);
            }
            while (test_buf);

             /* use 1/8 free memory at most to avoid out-of-memory error
              */
            test_buf_size >>= 4;
            try_realloc = 0;
        }
        else
        {
            break;
        }
    }

    if (!test_buf)
    {
        BACKDOOR_MGR_Printf("\r\n %s: failed to alloc buffer. (task_id: %lu)\r\n", __FUNCTION__, SYSFUN_TaskIdSelf());
        return;
    }

    BACKDOOR_MGR_Printf("\r\n %s: Start to load test. (task_id: %lu, mem_size: 0x%08lx)\r\n", __FUNCTION__, SYSFUN_TaskIdSelf(), test_buf_size);

    while (load_test_is_running)
    {
        pattern = SYSFUN_GetSysTick();
        memset(test_buf, pattern, test_buf_size);
    }

    free(test_buf);
    BACKDOOR_MGR_Printf("\r\n %s: Load test finish. (task_id: %lu)\r\n", __FUNCTION__, SYSFUN_TaskIdSelf());
}

/* FUNCTION NAME : SYSFUN_BACKDOOR_StartLoadTest
 * PURPOSE:
 *      Spawn thread to perform Load Test
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
static BOOL_T SYSFUN_BACKDOOR_StartLoadTest(void)
{
    UI32_T task_id;

    if (SYSFUN_SpawnThread(SYS_BLD_PROCESS_DEFAULT_PRIORITY,
                           SYS_BLD_PROCESS_DEFAULT_SCHED_POLICY,
                           "LoadTest",
                           SYS_BLD_TASK_COMMON_STACK_SIZE,
                           SYSFUN_TASK_NO_FP,
                           SYSFUN_BACKDOOR_LoadTestMain,
                           NULL,
                           &task_id) != SYSFUN_OK)
    {
        printf("\n%s: Spawn LoadTest thread fail.\n", __FUNCTION__);
        return FALSE;
    }

    return TRUE;
}

/* FUNCTION NAME : SYSFUN_BACKDOOR_CpuUsageMain
 * PURPOSE:
 *      Display CPU usage.
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
static void SYSFUN_BACKDOOR_CpuUsageMain(void)
{
    static int show_idle, show_deleted;

    while (1)
    {
        /* show cpu util
         */
        {
            static UI32_T *task_id_ar;
            static SYSFUN_BACKDOOR_CpuUsage_T *cpu_usage_ar[2];

            static SYSFUN_BACKDOOR_CpuUsage_T *old_cpu_usage_p, *new_cpu_usage_p, *tgt_cpu_usage_p;
            char task_name[SYSFUN_TASK_NAME_LENGTH+1];
            char *str_note;
            UI32_T max_num_of_task = SYS_ADPT_MAX_NBR_OF_TASK_IN_SYSTEM;
            UI32_T delta_user_ticks, delta_sys_ticks;
            UI32_T task_id_num;
            UI32_T old_task_id, new_task_id;
            UI32_T i, j;
            UI32_T ret;

            /* allocate buffer
             */
            if (task_id_ar == NULL)
            {
                task_id_ar = malloc(max_num_of_task * sizeof(*task_id_ar));
            }

            if (cpu_usage_ar[0] == NULL)
            {
                cpu_usage_ar[0] = calloc(max_num_of_task, sizeof(*cpu_usage_ar[0]));
            }

            if (cpu_usage_ar[1] == NULL)
            {
                cpu_usage_ar[1] = malloc(max_num_of_task * sizeof(*cpu_usage_ar[1]));
            }

            /* out of memory
             */
            if (task_id_ar == NULL || cpu_usage_ar[0] == NULL || cpu_usage_ar[1] == NULL)
            {
                BACKDOOR_MGR_Printf("\r\n %s: out of memory.\r\n", __FUNCTION__);

                if (task_id_ar)
                    free(task_id_ar);
                task_id_ar = NULL;

                if (cpu_usage_ar[0])
                    free(cpu_usage_ar[0]);
                if (cpu_usage_ar[1])
                    free(cpu_usage_ar[1]);
                cpu_usage_ar[0] = cpu_usage_ar[1] = NULL;

                return;
            }

            /* get task id list
             */
            if (SYSFUN_OK != (ret = SYSFUN_GetTaskIdList(NULL, &task_id_num)))
            {
                BACKDOOR_MGR_Printf("\r\n %s: SYSFUN_GetTaskIdList failed. (ret: %lu)\r\n", __FUNCTION__, ret);
                return;
            }

            BACKDOOR_MGR_Printf("\n");
            BACKDOOR_MGR_Printf("num of all tasks: %lu\n", task_id_num);

            task_id_num = max_num_of_task;

            if (SYSFUN_OK != (ret = SYSFUN_GetTaskIdList(task_id_ar, &task_id_num)))
            {
                BACKDOOR_MGR_Printf("\r\n %s: SYSFUN_GetTaskIdList failed. (ret: %lu)\r\n", __FUNCTION__, ret);
                return;
            }

            BACKDOOR_MGR_Printf("num of list tasks: %lu\n", task_id_num);

            /* get task cpu usage
             */
            memset(cpu_usage_ar[1], 0, max_num_of_task * sizeof(*cpu_usage_ar[1]));

            for (i = 0, j = 0; i < task_id_num && j < max_num_of_task; i++)
            {
                if (task_id_ar[i] == 0)
                {
                    continue;
                }

                if (SYSFUN_OK != (ret = SYSFUN_GetCpuUsageByTaskId(task_id_ar[i], &cpu_usage_ar[1][j].user_ticks, &cpu_usage_ar[1][j].sys_ticks)))
                {
                    continue;
                }

                cpu_usage_ar[1][j].task_id = task_id_ar[i];
                j++;
            }

            task_id_num = j;

            qsort(cpu_usage_ar[1], task_id_num, sizeof(*cpu_usage_ar[1]), SYSFUN_BACKDOOR_CpuUsageCompare);

            /* show task cpu usage
             */
            BACKDOOR_MGR_Printf("\n");
            BACKDOOR_MGR_Printf("%-11s %-16s %-11s %-11s %-11s %-11s\n",
                "task_id",
                "task_name",
                "total_utime",
                "total_stime",
                "delta_utime",
                "delta_stime");

            i = 0, j = 0;

            while (1)
            {
                if (i < max_num_of_task && cpu_usage_ar[0][i].task_id != 0)
                {
                    old_cpu_usage_p = &cpu_usage_ar[0][i];
                    old_task_id = cpu_usage_ar[0][i].task_id;
                }
                else
                {
                    old_cpu_usage_p = NULL;
                    old_task_id = 0;
                }

                if (j < task_id_num)
                {
                    new_cpu_usage_p = &cpu_usage_ar[1][j];
                    new_task_id = cpu_usage_ar[1][j].task_id;
                }
                else
                {
                    new_cpu_usage_p = NULL;
                    new_task_id = 0;
                }

                /* exit if all entries traversed
                 */
                if (!old_cpu_usage_p && !new_cpu_usage_p)
                {
                    break;
                }

                /* pick the entry to show
                 */
                if (new_cpu_usage_p && old_cpu_usage_p &&
                    old_task_id == new_task_id)
                {
                    tgt_cpu_usage_p = new_cpu_usage_p;
                    delta_user_ticks = new_cpu_usage_p->user_ticks - old_cpu_usage_p->user_ticks;
                    delta_sys_ticks = new_cpu_usage_p->sys_ticks - old_cpu_usage_p->sys_ticks;
                    str_note = "";
                    i++, j++;
                }
                else if (new_cpu_usage_p && 
                         (old_task_id > new_task_id || old_task_id == 0))
                {
                    /* task created
                     */
                    old_cpu_usage_p = NULL;
                    tgt_cpu_usage_p = new_cpu_usage_p;
                    delta_user_ticks = new_cpu_usage_p->user_ticks;
                    delta_sys_ticks = new_cpu_usage_p->sys_ticks;
                    str_note = " created";
                    j++;
                }
                else if (old_cpu_usage_p)
                {
                    /* task deleted
                     */
                    new_cpu_usage_p = NULL;
                    tgt_cpu_usage_p = old_cpu_usage_p;
                    delta_user_ticks = 0;
                    delta_user_ticks = 0;
                    str_note = " deleted";
                    i++;
                }

                /* options
                 */
                if (!show_idle)
                {
                    if (delta_user_ticks == 0 &&
                        delta_sys_ticks == 0)
                    {
                        tgt_cpu_usage_p = NULL;
                    }
                }

                if (!show_deleted)
                {
                    if (!new_cpu_usage_p)
                    {
                        tgt_cpu_usage_p = NULL;
                    }
                }

                /* output
                 */
                if (tgt_cpu_usage_p)
                {
                    if (new_cpu_usage_p)
                    {
                        if (SYSFUN_OK != SYSFUN_TaskIDToName(new_cpu_usage_p->task_id, task_name, sizeof(task_name)))
                        {
                            task_name[0] = 0;
                        }
                    }
                    else
                    {
                        strcpy(task_name, "N/A");
                    }

                    BACKDOOR_MGR_Printf("%11lu %-*.*s %11lu %11lu %11lu %11lu%s\n",
                        tgt_cpu_usage_p->task_id,
                        sizeof(task_name)-1, sizeof(task_name)-1, task_name,
                        tgt_cpu_usage_p->user_ticks,
                        tgt_cpu_usage_p->sys_ticks,
                        delta_user_ticks,
                        delta_sys_ticks,
                        str_note);
                }
            } /* end of for (i, j) */

            BACKDOOR_MGR_Printf("\n");

            /* backup cpu usage
             */
            memcpy(cpu_usage_ar[0], cpu_usage_ar[1], max_num_of_task * sizeof(*cpu_usage_ar[0]));
        } /* end of show cpu util */

        /* menu
         */
        {
            int ch;

            BACKDOOR_MGR_Print("\r\n 0. exit");
            BACKDOOR_MGR_Printf("\r\n 1. show_idle: %d", show_idle);
            BACKDOOR_MGR_Printf("\r\n 2. show_deleted: %d", show_deleted);
            BACKDOOR_MGR_Print("\r\n select = ");
            ch = BACKDOOR_MGR_GetChar();
            BACKDOOR_MGR_Printf("%c\r\n", (isprint(ch) ? ch : '?'));

            switch (ch)
            {
                case '0':
                    return;
                case '1':
                    show_idle = !show_idle;
                    break;
                case '2':
                    show_deleted = !show_deleted;
                    break;
            }
        }
    }
}

static int SYSFUN_BACKDOOR_CpuUsageCompare(const void *e1, const void *e2)
{
    const SYSFUN_BACKDOOR_CpuUsage_T *cpu_usage_1 = e1;
    const SYSFUN_BACKDOOR_CpuUsage_T *cpu_usage_2 = e2;
    int diff;

    if (0 != (diff = (cpu_usage_1->task_id - cpu_usage_2->task_id)))
        return diff;

    return 0;
}

