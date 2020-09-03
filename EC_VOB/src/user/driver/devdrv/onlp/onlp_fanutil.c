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
/* MODULE NAME:  onlp_fanutil.c
 * PURPOSE:
 *     This module implements a simple onlp utility program to
 *     access fan devices through the ONLP library.
 *
 * NOTES:
 *     None
 *
 * HISTORY
 *    10/26/2005 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

#include "onlp/onlp_config.h"
#include <onlp/oids.h>
#include <onlp/fan.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define DEBUG 1

/* MACRO FUNCTION DECLARATIONS
 */
#ifdef DEBUG
#define DEBUG_MSG(fmtstr, ...) do { \
    if(debug_flag == 1) \
        {printf("%s(%d)"fmtstr, __FUNCTION__, __LINE__, ##__VA_ARGS__);} \
} while (0)

#define DEBUG_OPT_CHAR "d"
#else
#define DEBUG_MSG(...)
#define DEBUG_OPT_CHAR
#endif

extern void onlp_api_lock_init(void);

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static int parse_fan_id(const char* fan_id_str_p, int *fan_id_p);
static void onlplib_fan_init(void);

/* EXPORTED VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef DEBUG
static int debug_flag=0;
#endif

/* EXPORTED SUBPROGRAM BODIES
 */
int main(int argc, char* argv[])
{
    int opt_h = 0;
    int opt_f = 0;
    int opt_r = 0;
    int opt_c = 0;
    int opt_s = 0;
    int opt;
    int retval=0;
    int i, rc;
    int target_fan_id=0, duty_cycle=100;
    int present_status=-1, failed_status=-1;
    onlp_oid_t oid;
    onlp_fan_info_t info;
    const char* optstr="hf:r:c:s:"DEBUG_OPT_CHAR;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 'f': opt_f=1;
            {
                const char* cptr;
                int   dot_pos;

                for (cptr=optarg; *cptr!='\0'; cptr++)
                {
                    if (*cptr=='.')
                        break;
                }
                if (*cptr!='\0')
                {
                    if(cptr[1]=='\0')
                    {
                        printf("Invalid fan duty cycle argument.\r\n");
                        return -1;
                    }
                    dot_pos=(int)(cptr-optarg);
                    rc=parse_fan_id(optarg, &target_fan_id);
                    if (rc<0)
                    {
                        printf("Invalid fan id.\r\n");
                        return -1;
                    }
                    duty_cycle=atoi(optarg+dot_pos+1);
                }
                else
                {
                    duty_cycle=atoi(optarg);
                }
            }
                break;
            case 'r':
                opt_r=1;
                if (parse_fan_id(optarg, &target_fan_id)!=0)
                {
                    printf("Invalid fan index.\r\n");
                    return -1;
                }
                break;
            case 'c':
                opt_c=1;
                if (parse_fan_id(optarg, &target_fan_id)!=0)
                {
                    printf("Invalid fan index.\r\n");
                    return -1;
                }
                break;
            case 's':
                opt_s=1;
                if (parse_fan_id(optarg, &target_fan_id)!=0)
                {
                    printf("Invalid fan index.\r\n");
                    return -1;
                }
                break;
#ifdef DEBUG
            case 'd': debug_flag=1; break;
#endif
            default: opt_h=1; retval = 1; break;
        }
    }

    if(opt_h)
    {
        printf("Usage: %s [OPTIONS]\n", argv[0]);
        printf(" Note that only one required option can be handled at a time.\n");
        printf(" Do not specify more than one required option at a time.\n");
        printf(" Required options:\n");
        printf("   -h Show help message\n");
        printf("   -f Set the specified duty cycle(0-100) to specified fan\n");
        printf("      Two formats are avaiable:\n");
        printf("        Format 1: <duty cycle> e.g. 50\n");
        printf("          Set duty cycle as 50%% to all fans in the system.\n");
        printf("        Format 2: <duty cycle> e.g. 1.50\n");
        printf("          Set duty cycle to 50%% to fan id 1.\n");
        printf("   -r <fan idx> Get the fan speed in RPM of the specified fan\n");
        printf("      index if the info is availabe\n");
        printf("      The output would be negative if the info is not available\n");
        printf("   -c <fan idx> Get the percentage of duty cycle of the specified fan\n");
        printf("      index if the info is availabe\n");
        printf("      The output would be negative if the info is not available\n");
        printf("   -s <fan idx> Get the fan status of the specified fan\n");
        printf("      index if the info is availabe\n");
        printf("      The output format is shown below.\n");
        printf("      [rc] [present] [failed]\n");
        printf("      rc           : return code; the fields behind rc is valid\n");
        printf("                     only when rc is not negative.\n");
        printf("      present      : 1 -> fan is present; 0 -> fan is not present\n");
        printf("      failed       : 1 -> fan is failed; 0 -> fan is OK\n");
        printf("      The output would be negative if the info is not available\n");
#ifdef DEBUG
        printf(" Optional options:\n");
        printf("   -d Show debug message\n");
#endif
        return retval;
    }

    onlplib_fan_init();

    if (opt_f)
    {
        int total_nbr_of_fan=0;

        if (duty_cycle<0 || duty_cycle>100)
        {
            printf("Illegal duty cycle setting %d(Valid range is 0-100).\r\n", duty_cycle);
            return -1;
        }

        /* traverse through oids of fans to set duty cycle to each fan
         * note that fan id starts from 1 in ONLP
         */
        for(i=1; i<((unsigned char)(~0)); i++)
        {
            if (target_fan_id==0 || target_fan_id==i)
            {
                oid=ONLP_FAN_ID_CREATE(i);
                rc=onlp_fan_info_get(oid, &info);
                if (rc>=0)
                {
                    total_nbr_of_fan++;
                    DEBUG_MSG("Fan oid 0x%08lX:", (unsigned long)oid);
                    if (info.status & ONLP_FAN_STATUS_PRESENT)
                    {
                        DEBUG_MSG("Present.\r\n");
                    }
                    else
                    {
                        DEBUG_MSG("Not Present.\r\n");
                    }
                    rc=onlp_fan_percentage_set(oid, duty_cycle);
                    if(rc!=ONLP_STATUS_OK)
                    {
                        printf("Set fan oid 0x%08lx error(rc=%d)\r\n", (unsigned long)oid,
                            rc);
                        retval=-1;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        return retval;
    }

    if (opt_r)
    {
        oid=ONLP_FAN_ID_CREATE(target_fan_id);
        DEBUG_MSG("FAN oid 0x%08lX:", (unsigned long)target_fan_id);
        rc=onlp_fan_info_get(oid, &info);
        if (rc>=0)
        {
            if (info.caps & ONLP_FAN_CAPS_GET_RPM)
            {
                printf("%d\n", info.rpm);
            }
            else
            {
                DEBUG_MSG("Fan %hu does not support.\r\n", target_fan_id);
                printf("%d\n", ONLP_STATUS_E_UNSUPPORTED);
                return -1;
            }
        }
        else
        {
            DEBUG_MSG("onlp_fan_info_get error.(rc=%d, fan id=%hu)\r\n", rc, target_fan_id);
            printf("%d\n", rc);
            return -1;
        }
    }

    if (opt_c)
    {
        oid=ONLP_FAN_ID_CREATE(target_fan_id);
        DEBUG_MSG("FAN oid 0x%08lX:", (unsigned long)target_fan_id);
        rc=onlp_fan_info_get(oid, &info);
        if (rc>=0)
        {
            if (info.caps & ONLP_FAN_CAPS_GET_PERCENTAGE)
            {
                printf("%d\n", info.percentage);
            }
            else
            {
                DEBUG_MSG("Fan %hu does not support.\r\n", target_fan_id);
                printf("%d\n", ONLP_STATUS_E_UNSUPPORTED);
                return -1;
            }
        }
        else
        {
            DEBUG_MSG("onlp_fan_info_get error.(rc=%d, fan id=%hu)\r\n", rc, target_fan_id);
            printf("%d\n", rc);
            return -1;
        }
    }

    if (opt_s)
    {
        oid=ONLP_FAN_ID_CREATE(target_fan_id);
        DEBUG_MSG("FAN oid 0x%08lX:", (unsigned long)target_fan_id);
        rc=onlp_fan_info_get(oid, &info);
        if (rc>=0)
        {
            if (info.status & ONLP_FAN_STATUS_PRESENT)
            {
                DEBUG_MSG("Present\n");
                present_status=1;
            }
            else
            {
                DEBUG_MSG("Not Present\n");
                present_status=0;
            }

            if (info.status & ONLP_FAN_STATUS_FAILED)
            {
                DEBUG_MSG("Failed\n");
                failed_status=1;
            }
            else
            {
                DEBUG_MSG("OK\n");
                failed_status=0;
            }

        }
        else
        {
            DEBUG_MSG("onlp_fan_info_get error.(rc=%d, fan id=%hu)\r\n", rc, target_fan_id);
            retval=-1;
        }
        printf("%d %d %d\n", rc, present_status, failed_status);
    }

    return retval;
}

/* LOCAL SUBPROGRAM BODIES
 */
static int parse_fan_id(const char* fan_id_str_p, int *fan_id_p)
{
    *fan_id_p=atoi(fan_id_str_p);
    if (fan_id_p<=0)
    {
        return -1;
    }
    return 0;
}

static void onlplib_fan_init(void)
{
#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif
    onlp_fan_init();
}

