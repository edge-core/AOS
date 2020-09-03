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
/* MODULE NAME:  onlp_psuutil.c
 * PURPOSE:
 *     This module implements a simple onlp utility program to
 *     access fan devices through the ONLP library.
 *
 * NOTES:
 *     None
 *
 * HISTORY
 *    11/10/2005 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>

#include "onlp/onlp_config.h"
#include "onlp/psu.h"
#include "onlp/oids.h"
#include "onlp/sys.h"

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
static int parse_psu_id(const char* psu_id_str_p, int *psu_id_p);
static void onlplib_psu_init(void);

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
    int opt_p = 0;
    int opt;
    int retval=0;
    int rc;
    int target_psu_id=0;
    onlp_oid_t oid;
    onlp_psu_info_t info;
    const char* optstr="p:"DEBUG_OPT_CHAR;
    int present_status=-1, failed_status=-1, unplugged_status=-1;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 'p': opt_p=1;
                if (parse_psu_id(optarg, &target_psu_id)!=0)
                {
                    printf("Invalid psu index.\r\n");
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
        printf("   -p <psu idx> Get the psu status of the specified psu index\n");
        printf("      The output format is shown below.\n");
        printf("      [rc] [present] [failed] [unplugged]\n");
        printf("      rc       : return code; the fields behind rc is valid only\n");
        printf("                 when rc is not negative.\n");
        printf("      present  : 1 -> present; 0 -> not present.\n");
        printf("      failed   : 1 -> failed; 0 -> not failed.\n");
        printf("      unplugged: 1 -> unplugged; 0 -> plugged(powered).\n");
        printf("      The values of the above fields are negative when error.");
#ifdef DEBUG
        printf(" Optional options:\n");
        printf("   -d Show debug message\n");
#endif
        return retval;
    }

    onlplib_psu_init();

    if (opt_p)
    {
        oid=ONLP_PSU_ID_CREATE(target_psu_id);
        rc=onlp_psu_info_get(oid, &info);
        if (rc>=0)
        {
            DEBUG_MSG("PSU oid 0x%08lX:", (unsigned long)oid);
            if (info.status & ONLP_PSU_STATUS_PRESENT)
            {
                DEBUG_MSG("Present.\r\n");
                present_status=1;
            }
            else
            {
                DEBUG_MSG("Not Present.\r\n");
                present_status=0;
            }

            if (info.status & ONLP_PSU_STATUS_FAILED)
            {
                DEBUG_MSG("Failed.\r\n");
                failed_status=1;
            }
            else
            {
                DEBUG_MSG("OK.\r\n");
                failed_status=0;
            }

            if (info.status & ONLP_PSU_STATUS_UNPLUGGED)
            {
                DEBUG_MSG("Unplugged.\r\n");
                unplugged_status=1;
            }
            else
            {
                DEBUG_MSG("Plugged.\r\n");
                unplugged_status=0;
            }
        }

        if (rc<0)
        {
            retval=-1;
        }

        printf("%d %d %d %d\n", rc, present_status, failed_status, unplugged_status);
        return retval;
    }

    return retval;
}

/* LOCAL SUBPROGRAM BODIES
 */
static int parse_psu_id(const char* psu_id_str_p, int *psu_id_p)
{
    *psu_id_p=atoi(psu_id_str_p);
    if (psu_id_p<=0)
    {
        return -1;
    }
    return 0;
}

static void onlplib_psu_init(void)
{
#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif
    onlp_psu_init();
}

