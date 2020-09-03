/************************************************************
 *
 *        Copyright 2015 Accton Corporation, 2015
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 ***********************************************************/
/* MODULE NAME:  onlp_platmgrd.c
 * PURPOSE:
 *     This module implements a daemon to run the onlp platform
 *     manager.
 *
 * NOTES:
 *     None
 *
 * HISTORY
 *    10/23/2005 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include <onlp/onlp.h>
#include <onlp/sys.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define EVENT_EMPTY     (0)
#define EVENT_RESTART   (1<<0)
#define EVENT_TERMINATE (1<<1)

#define TERMINATE_IN_MAIN_THREAD 1

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void signal_handler(int sig);

/* STATIC VARIABLE DECLARATIONS
 */
static int event_flag=EVENT_EMPTY;

/* EXPORTED SUBPROGRAM BODIES
 */

int main(int argc, char* argv[])
{
    sigset_t sigset;

    if ( (argc>1) && (strncmp(argv[1], "-d", 2)==0))
    {
        daemon(0, 0);
    }

    signal(SIGHUP,signal_handler); /* hangup signal */
    signal(SIGTERM,signal_handler); /* software termination signal from kill */

    sigfillset(&sigset);
    sigdelset(&sigset, SIGHUP);
    sigdelset(&sigset, SIGTERM);

    onlp_init();
    onlp_sys_platform_manage_start();

    while(1)
    {
        sigsuspend(&sigset);
#if defined(TERMINATE_IN_MAIN_THREAD)
        if (event_flag & EVENT_TERMINATE)
        {
            onlp_sys_platform_manage_stop();
            exit(0);
        }
#endif

        if (event_flag & EVENT_RESTART)
        {
            onlp_sys_platform_manage_stop();
            onlp_sys_platform_manage_start();
        }
    }

    return 0;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void signal_handler(int sig)
{
    switch(sig)
    {
        case SIGHUP:
            /* restart the service */
            event_flag|=EVENT_RESTART;
            break;
        case SIGTERM:
            /* terminate the service */
            #if defined(TERMINATE_IN_MAIN_THREAD)
            event_flag|=EVENT_TERMINATE;
            #else
            onlp_sys_platform_manage_stop();
            exit(0);
            #endif
            break;
    }
}

