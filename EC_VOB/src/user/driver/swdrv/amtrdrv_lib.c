/* MODULE NAME:  amtrdrv_lib.c
 * PURPOSE:
 *         This file provides utility routines which are related
 *         to AMTR/AMTRDRV, such as update counters by a given mac
 *         entry.
 *
 * NOTES:
 *
 * HISTORY
 *    12/8/2013 - Charlie Chen, Created
 *
 * Copyright(C)      Accton Corporation, 2013
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "amtrdrv_lib.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* ------------------------------------------------------------------------
 * FUNCTION NAME - AMTRDRV_LIB_UpdateCount
 * ------------------------------------------------------------------------
 * Purpose: Update AMTR counters by the given AMTR Mac address entry.
 * INPUT  : addr_entry_p    - The Mac address entry to be added or deleted
 *          is_to_increase  - TRUE:  addr_entry_p will be added to OM and
 *                                   will increase the related counters
 *                            FALSE: addr_entry_p will be removed from OM and
 *                                   will decrease the related counters
 * OUTPUT : amtr_counters_p - The corresponding counters will be updated by
 *                            the given addr_entry_p and is_to_increase.
 * RETURN : TRUE  -  Success
 *          FALSE -  Fail
 * NOTES  :
 *   1. Caller need to protect amtr_counters_p for race condition.
 *   2. If the address.life_time = other(under create entry) or source = self
 *      (CPU mac) or ifindex ==0(under create entry), we only update total
 *      counter.
 *
 * ------------------------------------------------------------------------
 */
BOOL_T AMTRDRV_LIB_UpdateCount(AMTR_TYPE_AddrEntry_T *addr_entry_p, BOOL_T is_to_increase, AMTR_TYPE_Counters_T* amtr_counters_p)
{

    if (addr_entry_p==NULL || amtr_counters_p==NULL)
        return FALSE;

    if(is_to_increase)
    {
        if((addr_entry_p->life_time != AMTR_TYPE_ADDRESS_LIFETIME_OTHER)&&   /*Under Create Entry*/
           (addr_entry_p->source != AMTR_TYPE_ADDRESS_SOURCE_SELF)&&  /*CPU MAC*/
           (addr_entry_p->ifindex!=0)&&
           (addr_entry_p->vid!=0))  /*Under Create Entry*/
        {
            if(addr_entry_p->life_time != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                amtr_counters_p->static_address_count_by_port[addr_entry_p->ifindex-1] ++;
                amtr_counters_p->static_address_count_by_vid[addr_entry_p->vid-1] ++;
                amtr_counters_p->total_static_address_count ++;
            }
            else
            {
                amtr_counters_p->dynamic_address_count_by_port[addr_entry_p->ifindex-1] ++;
                amtr_counters_p->dynamic_address_count_by_vid[addr_entry_p->vid-1] ++;
                amtr_counters_p->total_dynamic_address_count ++;
            }

            if(addr_entry_p->source==AMTR_TYPE_ADDRESS_SOURCE_LEARN)
            {
                amtr_counters_p->learnt_address_count_by_port[addr_entry_p->ifindex-1] ++;
            }
            else if(addr_entry_p->source==AMTR_TYPE_ADDRESS_SOURCE_SECURITY)
            {
                amtr_counters_p->security_address_count_by_port[addr_entry_p->ifindex-1] ++;
            }
            else if(addr_entry_p->source == AMTR_TYPE_ADDRESS_SOURCE_CONFIG)
            {
                amtr_counters_p->config_address_count_by_port[addr_entry_p->ifindex-1] ++;
            }
        }
        amtr_counters_p->total_address_count ++;
    }
    else
    {
        if((addr_entry_p->life_time != AMTR_TYPE_ADDRESS_LIFETIME_OTHER)&&
           (addr_entry_p->source != AMTR_TYPE_ADDRESS_SOURCE_SELF)&&
           (addr_entry_p->ifindex!=0)&&
           (addr_entry_p->vid!=0))
        {
            if(addr_entry_p->life_time != AMTR_TYPE_ADDRESS_LIFETIME_DELETE_ON_TIMEOUT)
            {
                amtr_counters_p->static_address_count_by_port[addr_entry_p->ifindex-1] --;
                amtr_counters_p->static_address_count_by_vid[addr_entry_p->vid-1] --;
                amtr_counters_p->total_static_address_count --;
            }
            else
            {
                amtr_counters_p->dynamic_address_count_by_port[addr_entry_p->ifindex-1] --;
                amtr_counters_p->dynamic_address_count_by_vid[addr_entry_p->vid-1] --;
                amtr_counters_p->total_dynamic_address_count --;
            }

            if(addr_entry_p->source==AMTR_TYPE_ADDRESS_SOURCE_LEARN)
            {
                amtr_counters_p->learnt_address_count_by_port[addr_entry_p->ifindex-1] --;
            }
            else if(addr_entry_p->source==AMTR_TYPE_ADDRESS_SOURCE_SECURITY)
            {
                amtr_counters_p->security_address_count_by_port[addr_entry_p->ifindex-1] --;
            }
            else if(addr_entry_p->source == AMTR_TYPE_ADDRESS_SOURCE_CONFIG)
            {
                amtr_counters_p->config_address_count_by_port[addr_entry_p->ifindex-1] --;
            }
        }
        amtr_counters_p->total_address_count --;
    }

    return TRUE;
}

/* LOCAL SUBPROGRAM BODIES
 */

