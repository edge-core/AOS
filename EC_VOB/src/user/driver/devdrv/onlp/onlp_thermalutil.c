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
/* MODULE NAME:  onlp_thermalutil.c
 * PURPOSE:
 *     This module implements a simple onlp utility program to
 *     access thermal devices through the ONLP library.
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
#include <onlp/oids.h>
#include "onlp/thermal.h"
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
static int parse_thermal_id(const char* thermal_id_str_p, int *thermal_id_p);
static void onlplib_thermal_init(void);

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
    int opt_t = 0;
    int opt;
    int retval=0;
    int mcelsius=0, rc=-1;
    int target_thermal_id=0;
    onlp_oid_t oid;
    onlp_thermal_info_t info;
    const char* optstr="ht:"DEBUG_OPT_CHAR;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 't':
                opt_t=1;
                if (parse_thermal_id(optarg, &target_thermal_id)!=0)
                {
                    printf("Invalid thermal index.\r\n");
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
        printf("   -t <thermal idx> Get the temperature from the specified thermal\n");
        printf("      sensor index\n");
        printf("      The output format is shown below.\n");
        printf("      [rc] [temperature]\n");
        printf("      rc           : return code; the fields behind rc is valid\n");
        printf("                     only when rc is not negative value.\n");
        printf("      temperature  : temperature got from the specified sensor\n");
        printf("                     the unit is milli-celsius\n");
#ifdef DEBUG
        printf(" Optional options:\n");
        printf("   -d Show debug message\n");
#endif
        return retval;
    }

    onlplib_thermal_init();

    if (opt_t)
    {
        oid=ONLP_THERMAL_ID_CREATE(target_thermal_id);
        DEBUG_MSG("THERMAL oid 0x%08lX:", (unsigned long)oid);
        rc=onlp_thermal_info_get(oid, &info);
        if (rc>=0)
        {
            mcelsius=info.mcelsius;
            {
                DEBUG_MSG("%d\n", mcelsius);
            }
        }
        else
        {
            DEBUG_MSG("onlp_thermal_info_get error.(rc=%d, thermal_id=%hu)\r\n", rc, target_thermal_id);
            retval=-1;
        }
    }

    printf("%d %d\n", rc, mcelsius);

    return retval;
}

/* LOCAL SUBPROGRAM BODIES
 */
static int parse_thermal_id(const char* thermal_id_str_p, int *thermal_id_p)
{
    *thermal_id_p=atoi(thermal_id_str_p);
    if (thermal_id_p<=0)
    {
        return -1;
    }
    return 0;
}

static void onlplib_thermal_init(void)
{
#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif
    onlp_thermal_init();
}

