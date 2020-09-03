/************************************************************
 *
 *        Copyright 2016 Accton Corporation, 2016
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
/* MODULE NAME:  onlp_ledutil.c
 * PURPOSE:
 *     This module implements a simple onlp utility program to
 *     control leds through the ONLP library.
 *
 * NOTES:
 *     None
 *
 * HISTORY
 *    5/17/2016 - Chiourung Huang, Created
 *
 * Copyright(C)      Accton Corporation, 2016
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "onlp/onlp_config.h"
#include <onlp/oids.h>
#include <onlp/led.h>

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
static void onlplib_led_init(void);
static int onlplib_get_led_oid(char *name, onlp_oid_t *oid, int *cap);
static int onlplib_find_one_light_mode_from_cap(int capability, int* led_light_mode_p);

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
    int opt_l = 0;
    int opt_s = 0;
    int opt;
    int retval=0;
    int rc;
    int loc_led_status=-1;
    onlp_oid_t oid;
    onlp_led_info_t info;
    const char* optstr="hl:s"DEBUG_OPT_CHAR;
    onlp_led_mode_t mode;
    char name[8]={0};
    int cap;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 'l': opt_l=1;

                mode = atoi(optarg);
                sprintf(name,"LOC");

                if(mode != 0 && mode != 1)
                {
                    printf("Error parameter.\r\n");
                    printf("0: off, 1: on.\r\n");
                    return -1;
                }

                break;
            case 's': opt_s=1;
                sprintf(name,"LOC");
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
        printf("   -l Set location LED\n");
        printf("      1  : on\n");
        printf("      0  : off\n");
        printf("   -s Get the LED status\n");
        printf("      Location LED : 1 -> on; 0 -> off\n");
#ifdef DEBUG
        printf(" Optional options:\n");
        printf("   -d Show debug message\n");
#endif
        return retval;
    }

    onlplib_led_init();

    if (opt_l)
    {
        if (onlplib_get_led_oid(name, &oid, &cap) != 0)
        {
            DEBUG_MSG("LED don't support %s\r\n", name);
            return -1;
        }

        DEBUG_MSG("set LED_LOC oid 0x%08lX, mode: %d\r\n", (unsigned long)oid, mode);

        if (mode == 0)
        {
            rc = onlp_led_mode_set(oid, ONLP_LED_MODE_OFF);
        }
        else
        {
            int led_light_mode;

            if (onlplib_find_one_light_mode_from_cap(cap, &led_light_mode)!=0)
            {
                DEBUG_MSG("Unable to find any light mode(cap=0x%X)\r\n", cap);
                return -1;
            }
            DEBUG_MSG("led_light_mode=%d\n", led_light_mode);
            rc = onlp_led_mode_set(oid, led_light_mode);
        }

        if(rc!=ONLP_STATUS_OK)
        {
            DEBUG_MSG("Set led oid 0x%08lx error(rc=%d)\r\n", (unsigned long)oid, rc);
            retval=-1;
        }

        return retval;
    }

    if (opt_s)
    {
        if (onlplib_get_led_oid(name, &oid, &cap) != 0)
        {
            DEBUG_MSG("LED don't support %s\r\n", name);
            return -1;
        }

        DEBUG_MSG("LED_LOC oid 0x%08lX\r\n", (unsigned long)oid);
        rc=onlp_led_info_get(oid, &info);
        if (rc>=0)
        {
            if (info.mode == ONLP_LED_MODE_OFF)
            {
                DEBUG_MSG("LED_LOC: off\n");
                loc_led_status=0;
            }
            else
            {
                DEBUG_MSG("LED_LOC: on\n");
                loc_led_status=1;
            }
        }
        else
        {
            DEBUG_MSG("onlp_led_info_get error.(rc=%d, oid: 0x%081X)\r\n", rc, (unsigned long)oid);
            retval=-1;
        }
        printf("%d\n", loc_led_status);
    }

    return retval;
}

/* LOCAL SUBPROGRAM BODIES
 */
static void onlplib_led_init(void)
{
#if ONLP_CONFIG_INCLUDE_API_LOCK == 1
    onlp_api_lock_init();
#endif
    onlp_led_init();
}

static int onlplib_get_led_oid(char *name, onlp_oid_t *oid, int *cap)
{
    int local_id=1; /*0: reserve*/
    int rc;
    onlp_led_info_t info;
    int match=0;
    int retval;

    while (1)
    {
        *oid = ONLP_LED_ID_CREATE(local_id);
        rc=onlp_led_info_get(*oid, &info);
        if (rc == ONLP_STATUS_OK)
        {
            if (strstr(info.hdr.description, name))
            {
                *cap = info.caps;
                retval = 0;
                break;
            }

            local_id++;
        }
        else
        {
            retval = -1;
            break;
        }
    }

    return retval;
}

static int onlplib_find_one_light_mode_from_cap(int capability, int* led_light_mode_p)
{
    /* try to find light still mode first
     */
    if (capability & ONLP_LED_CAPS_RED)
    {
        *led_light_mode_p=ONLP_LED_MODE_RED;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_ORANGE)
    {
        *led_light_mode_p=ONLP_LED_MODE_ORANGE;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_YELLOW)
    {
        *led_light_mode_p=ONLP_LED_MODE_YELLOW;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_GREEN)
    {
        *led_light_mode_p=ONLP_LED_MODE_GREEN;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_BLUE)
    {
        *led_light_mode_p=ONLP_LED_MODE_BLUE;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_PURPLE)
    {
        *led_light_mode_p=ONLP_LED_MODE_PURPLE;
        return 0;
    }

    /* try to find light blinking mode
     */
    if (capability & ONLP_LED_CAPS_ORANGE_BLINKING)
    {
        *led_light_mode_p=ONLP_LED_MODE_ORANGE_BLINKING;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_YELLOW_BLINKING)
    {
        *led_light_mode_p=ONLP_LED_MODE_YELLOW_BLINKING;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_GREEN_BLINKING)
    {
        *led_light_mode_p=ONLP_LED_MODE_GREEN_BLINKING;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_BLUE_BLINKING)
    {
        *led_light_mode_p=ONLP_LED_MODE_BLUE_BLINKING;
        return 0;
    }

    if (capability & ONLP_LED_CAPS_PURPLE_BLINKING)
    {
        *led_light_mode_p=ONLP_LED_MODE_PURPLE_BLINKING;
        return 0;
    }

    /* try to find auto mode last
     */
    if (capability & ONLP_LED_CAPS_AUTO)
    {
        *led_light_mode_p=ONLP_LED_MODE_AUTO;
        return 0;
    }

    return -1;
}

