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
/* MODULE NAME:  onlp_query.c
 * PURPOSE:
 *     This module implements a simple onlp utility program to
 *     query the information provided by the ONLP library.
 *
 * NOTES:
 *     None
 *
 * HISTORY
 *    10/23/2005 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2015
 */

#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <onlp/onlp.h>
#include <onlp/oids.h>
#include <onlp/thermal.h>
#include <onlp/fan.h>
#include <onlp/psu.h>
#include <onlp/sys.h>

/* #define DEBUG 1 */

#define PROBE_DEV_NUM_METHOD_BRUTE_FORCE 0
#define PROBE_DEV_NUM_METHOD_INFO_TABLE  1
#define PROBE_DEV_NUM_METHOD PROBE_DEV_NUM_METHOD_INFO_TABLE

#define EEPROM_DISPLAY_FLAG_PRODUCT_NAME      (1<<0)
#define EEPROM_DISPLAY_FLAG_SERIAL_NUMBER     (1<<1)
#define EEPROM_DISPLAY_FLAG_MAC_BASE          (1<<2)
#define EEPROM_DISPLAY_FLAG_MANUFACTURE_DATE  (1<<3)
#define EEPROM_DISPLAY_FLAG_LABEL_REVISION    (1<<4)
#define EEPROM_DISPLAY_FLAG_PLATFORM_NAME     (1<<5)

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

struct onlp_oid_t_lst
{
    onlp_oid_t oid;
    void* next_p;
};

#ifdef DEBUG
static int debug_flag=0;
#endif

static int total_num_of_psu=0;
static int eeprom_display_flag=0;
static struct onlp_oid_t_lst *psu_oid_lst_head_p=NULL;

static void add_psu_oid_to_lst(onlp_oid_t* oidp)
{
    struct onlp_oid_t_lst* new_entry_p=NULL;
    new_entry_p=(struct onlp_oid_t_lst *)malloc(sizeof(struct onlp_oid_t_lst));
    if(new_entry_p==NULL)
    {
        printf("%s(%d)malloc %d bytes for psu_oid_lst entry failed.\n",
            __FUNCTION__, __LINE__, sizeof(struct onlp_oid_t_lst));
        return;
    }
    memset(new_entry_p, 0, sizeof(struct onlp_oid_t_lst));
    new_entry_p->oid = *oidp;
    if (psu_oid_lst_head_p==NULL)
    {
        psu_oid_lst_head_p=new_entry_p;
    }
    else
    {
        struct onlp_oid_t_lst* tmp_entry_p;

        /* add to tail of the list
         */
        tmp_entry_p=psu_oid_lst_head_p;
        while (tmp_entry_p->next_p!=NULL)
        {
            tmp_entry_p=tmp_entry_p->next_p;
        }
        tmp_entry_p->next_p=new_entry_p;
    }

    total_num_of_psu++;
}

int main(int argc, char* argv[])
{
    int opt_h = 0;
    int opt_t = 0;
    int opt_f = 0;
    int opt_p = 0;
    int opt_c = 0;
    int opt_e = 0;
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
    int opt_x = 0;
#endif
    int opt;
    int retval=0, rc;
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE)
    int i;
    onlp_oid_t oid;
#endif
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
    onlp_oid_t* oidp;
#endif
    onlp_sys_info_t si;
    onlp_psu_info_t psu_info;
    const char* optstr="phtfce:"
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
        "x"
#endif
        DEBUG_OPT_CHAR;

    while( (opt = getopt(argc, argv, optstr)) != -1)
    {
        switch(opt)
        {
            case 'h': opt_h=1; break;
            case 't': opt_t=1; break;
            case 'f': opt_f=1; break;
            case 'p': opt_p=1; break;
            case 'c': opt_c=1; break;
            case 'e': opt_e=1;
            {
                switch(*optarg)
                {
                    case 'n':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_PRODUCT_NAME;
                        break;
                    case 'p':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_PLATFORM_NAME;
                        break;
                    case 's':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_SERIAL_NUMBER;
                        break;
                    case 'm':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_MAC_BASE;
                        break;
                    case 'a':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_MANUFACTURE_DATE;
                        break;
                    case 'l':
                        eeprom_display_flag=EEPROM_DISPLAY_FLAG_LABEL_REVISION;
                        break;
                    default:
                        printf("Unknown EEPROM option.\n");
                        return -1;
                        break;
                }
            }
            break;
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
            case 'x': opt_x=1; break;
#endif
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
        printf("   -t Show number of thermal in the system\n");
        printf("   -f Show number of fan in the system\n");
        printf("   -p Show number of psu in the system\n");
        printf("   -c Show cpld version\n");
        printf("   -en Show Product Name from ONIE sys-eeprom TLV\n");
        printf("   -ep Show Platform Name from ONIE sys-eeprom TLV\n");
        printf("   -es Show Serial Number from ONIE sys-eeprom TLV\n");
        printf("   -em Show Mac Base(6-byte binary value) from ONIE sys-eeprom TLV\n");
        printf("   -ea Show Manufacture Date from ONIE sys-eeprom TLV\n");
        printf("   -el Show Label Revision from ONIE sys-eeprom TLV\n");
        printf(" Optional options:\n");
#ifdef DEBUG
        printf("   -d Show debug message\n");
#endif
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
        printf("   -x Include the number of fan/thermal within the psu\n");
#endif
        return retval;
    }

    onlp_init();

    rc=onlp_sys_info_get(&si);
    if(rc < 0)
    {
        DEBUG_MSG("onlp_sys_info_get error(rc=%d)\n", rc);
        return -1;
    }

    if(opt_c)
    {
        if (si.platform_info.cpld_versions)
        {
            printf("%s\n", si.platform_info.cpld_versions);
            return 0;
        }
        else
        {
            printf("NA\n");
            return -1;
        }
    }

    if (opt_p)
    {
#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE)
        /* probe for number of psu through ONLP function
         * note that psu id starts from 1 in ONLP
         */
        for(i=1; i<((unsigned char)(~0)); i++)
        {
            oid=ONLP_PSU_ID_CREATE(i);
            rc=onlp_psu_info_get(oid, &psu_info);
            if (rc>=0)
            {
                DEBUG_MSG("PSU oid 0x%08lX:", (unsigned long)oid);
                if (psu_info.status & ONLP_PSU_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
                add_psu_oid_to_lst(&oid);
            }
            else
                break;
        }
#elif (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)

        ONLP_OID_TABLE_ITER_TYPE(si.hdr.coids, oidp, PSU)
        {
            DEBUG_MSG("PSU oid 0x%08lX:", (unsigned long)*oidp);
            rc=onlp_psu_info_get(*oidp, &psu_info);
            if (rc>=0)
            {
                if (psu_info.status & ONLP_PSU_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
                add_psu_oid_to_lst(oidp);
            }
            else
            {
                DEBUG_MSG("onlp_psu_info_get error(rc=%d)\n", rc);
            }
        }
#endif /* end of #if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE) */

        printf("%d\n", total_num_of_psu);
        return retval;
    }

    if (opt_t)
    {
        onlp_thermal_info_t info;
        int total_nbr_of_thermal=0;

#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE)
        /* probe for number of thermal through ONLP function
         * note that thermal id starts from 1 in ONLP
         */
        for(i=1; i<((unsigned char)(~0)); i++)
        {
            oid=ONLP_THERMAL_ID_CREATE(i);
            rc=onlp_thermal_info_get(oid, &info);
            if (rc>=0)
            {
                total_nbr_of_thermal++;
                DEBUG_MSG("Thermal oid 0x%08lX:", (unsigned long)oid);
                if (info.status & ONLP_THERMAL_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
            }
            else
                break;
        }
#elif (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
        ONLP_OID_TABLE_ITER_TYPE(si.hdr.coids, oidp, THERMAL)
        {
            total_nbr_of_thermal++;
            DEBUG_MSG("Thermal oid 0x%08lX:", (unsigned long)*oidp);
            rc=onlp_thermal_info_get(*oidp, &info);
            if (rc>=0)
            {
                if (info.status & ONLP_THERMAL_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
            }
            else
            {
                DEBUG_MSG("onlp_thermal_info_get error(rc=%d).\n", rc);
            }
        }

        if (opt_x)
        {
            struct onlp_oid_t_lst *iterator_p=NULL;
            for(iterator_p=psu_oid_lst_head_p; iterator_p!=NULL; iterator_p=iterator_p->next_p)
            {
                rc=onlp_psu_info_get(iterator_p->oid, &psu_info);
                if (rc<0)
                {
                    DEBUG_MSG("onlp_psu_info_get error(oid=0x%08lX,rc=%d).\n", (unsigned long)iterator_p->oid, rc);
                    continue;
                }

                ONLP_OID_TABLE_ITER_TYPE(psu_info.hdr.coids, oidp, THERMAL)
                {
                    total_nbr_of_thermal++;
                    DEBUG_MSG("Thermal oid 0x%08lX(In PSU oid 0x%08lX):", (unsigned long)*oidp, (unsigned long)iterator_p->oid);
                    rc=onlp_thermal_info_get(*oidp, &info);
                    if (rc>=0)
                    {
                        if (info.status & ONLP_THERMAL_STATUS_PRESENT)
                        {
                            DEBUG_MSG("Present.\n");
                        }
                        else
                        {
                            DEBUG_MSG("Not Present.\n");
                        }
                    }
                    else
                    {
                        DEBUG_MSG("onlp_thermal_info_get error(rc=%d).\n", rc);
                    }
                }
            }
        }

#endif /* end of #if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE) */

        printf("%d\n", total_nbr_of_thermal);
        return retval;
    }

    if (opt_f)
    {
        onlp_fan_info_t info;
        int total_nbr_of_fan=0;

#if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE)
        /* probe for number of fan through ONLP function
         * note that fan id starts from 1 in ONLP
         */
        for(i=1; i<((unsigned char)(~0)); i++)
        {
            oid=ONLP_FAN_ID_CREATE(i);
            rc=onlp_fan_info_get(oid, &info);
            if (rc>=0)
            {
                total_nbr_of_fan++;
                DEBUG_MSG("Fan oid 0x%08lX:", (unsigned long)oid);
                if (info.status & ONLP_FAN_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
            }
            else
                break;
        }
#elif (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_INFO_TABLE)
        rc=onlp_sys_info_get(&si);
        if(rc < 0)
        {
            DEBUG_MSG("onlp_sys_info_get error(rc=%d)\n", rc);
            return -1;
        }

        ONLP_OID_TABLE_ITER_TYPE(si.hdr.coids, oidp, FAN)
        {
            total_nbr_of_fan++;
            DEBUG_MSG("Fan oid 0x%08lX:", (unsigned long)*oidp);
            rc=onlp_fan_info_get(*oidp, &info);
            if (rc>=0)
            {
                if (info.status & ONLP_FAN_STATUS_PRESENT)
                {
                    DEBUG_MSG("Present.\n");
                }
                else
                {
                    DEBUG_MSG("Not Present.\n");
                }
            }
            else
            {
                DEBUG_MSG("onlp_fan_info_get error(rc=%d).\n", rc);
            }
        }

        if (opt_x)
        {
            struct onlp_oid_t_lst *iterator_p=NULL;
            for(iterator_p=psu_oid_lst_head_p; iterator_p!=NULL; iterator_p=iterator_p->next_p)
            {
                rc=onlp_psu_info_get(iterator_p->oid, &psu_info);
                if (rc<0)
                {
                    DEBUG_MSG("onlp_psu_info_get error(oid=0x%08lX,rc=%d).\n", (unsigned long)iterator_p->oid, rc);
                    continue;
                }

                ONLP_OID_TABLE_ITER_TYPE(psu_info.hdr.coids, oidp, FAN)
                {
                    total_nbr_of_fan++;
                    DEBUG_MSG("Fan oid 0x%08lX(In PSU oid 0x%08lX):", (unsigned long)*oidp, (unsigned long)iterator_p->oid);
                    rc=onlp_fan_info_get(*oidp, &info);
                    if (rc>=0)
                    {
                        if (info.status & ONLP_FAN_STATUS_PRESENT)
                        {
                            DEBUG_MSG("Present.\n");
                        }
                        else
                        {
                            DEBUG_MSG("Not Present.\n");
                        }
                    }
                    else
                    {
                        DEBUG_MSG("onlp_fan_info_get error(rc=%d).\n", rc);
                    }
                }
            }
        }
#endif /* end of #if (PROBE_DEV_NUM_METHOD==PROBE_DEV_NUM_METHOD_BRUTE_FORCE) */

        printf("%d\n", total_nbr_of_fan);
        return retval;
    }

    if (opt_e)
    {
        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_PRODUCT_NAME)
        {
            printf("%s\n", si.onie_info.product_name);
            return retval;
        }

        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_PLATFORM_NAME)
        {
            printf("%s\n", si.onie_info.platform_name);
            return retval;
        }

        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_SERIAL_NUMBER)
        {
            printf("%s\n", si.onie_info.serial_number);
            return retval;
        }

        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_MAC_BASE)
        {
            fwrite(si.onie_info.mac, 6, 1, stdout);
            return retval;
        }

        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_MANUFACTURE_DATE)
        {
            printf("%s\n", si.onie_info.manufacture_date);
            return retval;
        }

        if (eeprom_display_flag & EEPROM_DISPLAY_FLAG_LABEL_REVISION)
        {
            printf("%s\n", si.onie_info.label_revision);
            return retval;
        }
    }

    return retval;
}

