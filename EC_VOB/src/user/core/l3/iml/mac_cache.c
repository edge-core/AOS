/* -------------------------------------------------------------------------------------
 * FILE NAME: MAC_CACHE.c
 *
 * PURPOSE: Mac Cache supports fast (mac,vlan, port) learning from ingress packet,
 *          because the AMTR learing is slow and distributed.
 * NOTES:   1. This module supports MAC cache, only control packet or management packet
 *             will use this table.
 *          2. This module is inner component of CSC, not a visible component in system
 *             architecture. So we use MAC_CACHE as file name.
 *          3. This module should use HiSam as inner database code;
 *             if the table full, this module would find proper one to be replaced.
 *
 * MODIFICATION HISOTRY:
 * MODIFIER         DATE             DESCRIPTION
 * -------------------------------------------------------------------------------------
 * Penny Chang      05-08-2003       First created
 * William Chiang   07-09-2003       Change age-out time to 45 sec;
 *                                   If split send function to pktOut/forward,
 *                                   age-out time can be 10 sec.
 * -------------------------------------------------------------------------------------
 * Copyright(C)        Accton Techonology Corporation 2003
 * -------------------------------------------------------------------------------------*/

/* INCLUDE FILE DECLARATIONS
 */ 
#include <string.h>
#include "mac_cache.h"
#include "sysfun.h"
#include "sys_bld.h"

/* NAME CONSTANT DECLARATIONS
 */

/* ------------------------------------------------------------------ */
/* MAC cache for incoming packet, 2048 keys with 4 buckets            */
/* ------------------------------------------------------------------ */
#define MAC_CACHE_ENTRY         4096
#define MAC_ENTRY_BUCKET        5
#define HASH_MASK           	4095
#define MAC_AGE_OUT_TIME        (45*SYS_BLD_TICKS_PER_SECOND)   /* 45 secs */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

static MAC_CACHE_MacEntry_T macCacheTable[MAC_CACHE_ENTRY][MAC_ENTRY_BUCKET]; /* MAC cache table for incoming frame */
static UI32_T mac_cache_age_out_time;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
static UI32_T MAC_CACHE_HashIndex (char mac[6]);


/* NAMING CONSTANT DECLARATION
 */

/* EXPORTED SUBPROGRAM DECLARATION
 */
 
/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_Init
 *--------------------------------------------------------------------------
 * PURPOSE  : This function will initialize system resouce for MAC cache.
 * INPUT    : cache_size -- size of cache; the total number of table entry.
 *            hash_depth -- collision used.
 * OUTPUT   : none
 * RETURN   : None.
 * NOTES    : none
 *--------------------------------------------------------------------------*/
void MAC_CACHE_Init(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY 
     */
    memset(macCacheTable, 0, sizeof(macCacheTable));
    mac_cache_age_out_time = MAC_AGE_OUT_TIME;

}/* end of MAC_CACHE_Init */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_ResetCache
 *--------------------------------------------------------------------------
 * PURPOSE  : Reset Cache to empty cache.
 * INPUT    : none
 * OUTPUT   : none
 * RETURN   : none
 * NOTES    : All space allocated or used should be released.
 *--------------------------------------------------------------------------*/
void MAC_CACHE_ResetCache(void)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    /* BODY 
     */
    memset(macCacheTable, 0, sizeof(macCacheTable));
}/* end of MAC_CACHE_ResetCache */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_SetLearnedMacEntry
 *--------------------------------------------------------------------------
 * PURPOSE  : Set learned MAC entry to MAC cache.
 * INPUT    : mac_entry -- the learning information.
 *                vid_ifindex -- learning interface
 *                mac         -- source mac of this packet.
 *                lport       -- ingress port.
 * OUTPUT   : none
 * RETURN   : MAC_CACHE_OK  -- add/update the MAC cache with key successfully.
 *            MAC_CACHE_RET_MAC_INVALID -- mac is either all 0, or all 1, or multicast address
 * NOTES    : 1. key is (vlan, mac).
 *-------------------------------------------------------------------------- */
UI32_T MAC_CACHE_SetLearnedMacEntry(MAC_CACHE_MacEntry_T  mac_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI16_T  hashKey;
    UI32_T  i;
    UI8_T   zero_t[6] = {0};
    UI32_T  cur_time, current_delta_time=0;
    UI32_T  oldest_bucket = 0;
    UI32_T  blank_bucket = MAC_ENTRY_BUCKET;

    /* BODY 
     */
    /* check MAC can not be all 0, all 1, or multicast 
     */
    if ((mac_entry.mac[0] & 0x01) || (memcmp(mac_entry.mac, zero_t , 6) == 0) )
    {
        return MAC_CACHE_RET_MAC_INVALID;
    }

    /* Get Current time for timestamp 
     */
    cur_time = SYSFUN_GetSysTick();
    
    hashKey = MAC_CACHE_HashIndex ((char *)mac_entry.mac);

    /* Find unused MAC cache entry 
     */
    //kh_shi SYSFUN_NonPreempty();
    for (i=0; i < MAC_ENTRY_BUCKET ; i++)
    {
        /* Check if specific MAC address had been cached  
         */
        /* Penny, compare both mac and vid_ifindex thru all buckets in the specific hashKey 
         */
        if ((memcmp(macCacheTable[hashKey][i].mac, mac_entry.mac, 6) == 0)&&
            (macCacheTable[hashKey][i].vid_ifindex == mac_entry.vid_ifindex) )
        {
            macCacheTable[hashKey][i].lport = mac_entry.lport;
            macCacheTable[hashKey][i].timestamp = cur_time;
            
            //kh_shi SYSFUN_Preempty();
            return MAC_CACHE_OK;
        }
              
        if ( (blank_bucket==MAC_ENTRY_BUCKET) &&
             memcmp(macCacheTable[hashKey][i].mac, zero_t, 6) == 0)
        {
            blank_bucket = i;
        }
                    
        /* find the oldest entry
         */
        if ( blank_bucket == MAC_ENTRY_BUCKET )
        {   
            UI32_T delta_time;
            delta_time = cur_time - macCacheTable[hashKey][i].timestamp;
            
            if ( delta_time > current_delta_time )
            {
                oldest_bucket = i;
                current_delta_time = delta_time;
            }
        }
    } /* End of for */

    if ( blank_bucket != MAC_ENTRY_BUCKET)
    {
        oldest_bucket = blank_bucket;
    }
          
    /* 1) all buckets are full, select the oldest bucket & replace it, or
     * 2) find a blank bucket & replace it
     */
    macCacheTable[hashKey][oldest_bucket] = mac_entry;
    macCacheTable[hashKey][oldest_bucket].timestamp = cur_time;
    
    //kh_shi SYSFUN_Preempty();
    return MAC_CACHE_OK;
    
}/* end of MAC_CACHE_SetLearnedMacEntry */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_GetLport(UI32_T ifindex, UI8_T mac, UI32_T *lport_p)
 *--------------------------------------------------------------------------
 * PURPOSE  : Get associated lport with (ip,mac)
 * INPUT    : ifindex
 *          : mac
 * OUTPUT   : lport_p.
 * RETURN   : MAC_CACHE_OK
 *            MAC_CACHE_NOT_EXIST, can't find the mac.
 *            MAC_CACHE_ENTRY_AGE_OUT, the entry existed but age-out
 * NOTES    : key is (vlan,mac)
 *-------------------------------------------------------------------------- */
UI32_T MAC_CACHE_GetLport(UI32_T ifindex, UI8_T *mac, UI32_T *lport_p)
{
    /* LOCAL CONSTANT DECLARATIONS
     */
    /* LOCAL VARIABLES DEFINITION
     */
    UI16_T hashKey;
    UI32_T bucketIdx;
    UI32_T cur_time, delta_time = 0;

    /* BODY 
     */
    /* Get Current time for timestamp 
     */
    cur_time = SYSFUN_GetSysTick();

    hashKey = MAC_CACHE_HashIndex ((char *)mac);

    //kh_shi SYSFUN_NonPreempty();
    for (bucketIdx=0; bucketIdx<MAC_ENTRY_BUCKET; bucketIdx++)
    {
        if ((memcmp(macCacheTable[hashKey][bucketIdx].mac, mac, 6 ) == 0) &&
            (macCacheTable[hashKey][bucketIdx].vid_ifindex == ifindex) )
        {
            /* MAC cache entry found 
             */
            delta_time = cur_time - macCacheTable[hashKey][bucketIdx].timestamp;

            if (delta_time > mac_cache_age_out_time)
            {
                *lport_p = 0;
                
                /* since this entry is aged out, and out lport is 0, that is invalid,
                 * make this entry to be as blank is good to set, because when collision 
                 * occures replacing blank entry is better then old one.
                 */
                memset(macCacheTable[hashKey][bucketIdx].mac, 0, 6);
                
                //kh_shi SYSFUN_Preempty();
                return MAC_CACHE_ENTRY_AGE_OUT;
            }

            *lport_p = macCacheTable[hashKey][bucketIdx].lport;
            
            //kh_shi SYSFUN_Preempty();
            return MAC_CACHE_OK;
        }
    }

    /* Entry not found 
     */
    *lport_p = 0;
    
    //kh_shi SYSFUN_Preempty();
    return MAC_CACHE_NOT_EXIST;

}/* end of MAC_CACHE_GetLport */

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_SetAgeOutTime
 *--------------------------------------------------------------------------
 * PURPOSE  : Set MAC cache age out time
 * INPUT    : age_out_time   --  age out time
 * OUTPUT   : none
 * RETURN   : MAC_CACHE_OK
 * NOTES    : This api is for IML backdoor use
 *--------------------------------------------------------------------------
 */
UI32_T MAC_CACHE_SetAgeOutTime(UI32_T age_out_time)
{
    mac_cache_age_out_time = age_out_time;
    return MAC_CACHE_OK;
}

/*--------------------------------------------------------------------------
 * FUNCTION NAME - MAC_CACHE_GetAgeOutTime
 *--------------------------------------------------------------------------
 * PURPOSE  : Get MAC cache age out time
 * INPUT    : age_out_time   --  age out time
 * OUTPUT   : age_out_time   --  age out time
 * RETURN   : MAC_CACHE_OK
 *            MAC_CACHE_RET_FAIL
 * NOTES    : This api is for IML backdoor use
 *--------------------------------------------------------------------------
 */
UI32_T MAC_CACHE_GetAgeOutTime(UI32_T *age_out_time)
{
    if(NULL == age_out_time)
        return MAC_CACHE_RET_FAIL;
    *age_out_time = mac_cache_age_out_time;   
    return MAC_CACHE_OK;
}



static UI32_T MAC_CACHE_HashIndex ( char mac[6] )
{
    UI32_T index;

    index = *((UI16_T*) (mac+0)) ^ 
            *((UI16_T*) (mac+2)) ^
            *((UI16_T*) (mac+4));
    
    return ( index & HASH_MASK );
}
