/* MODULE NAME: dns_cache.c
 * PURPOSE:
 *       This module provide functions receive TCP or UDP packets
 *       functions transmit TCP or UDP packets
 *
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier,   Reason
 *       2002-09-06    -- Wiseway , created
 *       2002-10-24    -- Wiseway   modified for convention
 *
 * Copyright(C)      Accton Corporation, 2002
 */

/* isiah.2004-01-06. remove all compile warring message.*/
#include "sys_type.h"
#include "sys_cpnt.h"

#if (SYS_CPNT_DNS == TRUE)

#include <stdlib.h>
#include <string.h>

#include "sys_module.h"
#include "l_mm.h"
#include "dns_type.h"
#include "dns.h"
#include "dns_om.h"
#include "dns_mgr.h"
#include "dns_vm.h"
#include "dns_cmm.h"
#include "dns_cache.h"
#include "dns_resolver.h"

#include "l_stdlib.h"
#include "l_inet.h"

static DNS_CacheConfig_T    dns_cache_config;
static DNS_CacheEntryPtr_T  pDns_cache_table=NULL;  /* a pointer to the memory space allocated in DNS_CACHE_Init through function "malloc" */
static int hash[DNS_CACHE_HASH_TBL_LEN];        /* every element in the array is a int pointer to the head of the hash link */
static int ttlhdr = 0;
static int cache_ent_usenum = 0;
static UI32_T dns_cache_semaphore_id;

/* FUNCTION NAME : DNS_CACHE_Init
 * PURPOSE:
 *     This function is used for initializing dns cache
 *     The contents of cache will be initialized here.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success
 *
 * NOTES:
 *      This function will be called by DNS_Init.
 */
int DNS_CACHE_Init(void)
{
    int cache_statue;           /* 1 enable cache,2 disable cache   */
    UI32_T cache_max_ttl;       /* Maximum Time-To-Live for RRs in this cache*/
    int i = 0;
    int cache_size=0;
    DNS_VM_GetDnsResCacheStatus(&cache_statue);
    DNS_VM_GetDnsResCacheMaxTTL(&cache_max_ttl);
    DNS_VM_GetDnsResCacheMaxEntries(&cache_size);
    dns_cache_config.cache_status = cache_statue;
    dns_cache_config.cache_max_ttl = cache_max_ttl; /* sec */
    dns_cache_config.cache_max_entries = cache_size;
    dns_cache_config.cache_good_caches = 0;
    dns_cache_config.cache_bad_caches = 0;

        /* ES4649-38-00061*/
        /*
    if ((cache_statue == VAL_dnsResCacheStatus_enabled)&&(dns_status==DNS_ENABLE))
    {
    */
        if (pDns_cache_table!=NULL)
            free(pDns_cache_table);

        pDns_cache_table = (DNS_CacheEntry_T *)malloc(sizeof(DNS_CacheEntry_T) * cache_size);
        if(pDns_cache_table == NULL)
        {
            return DNS_ERROR;
        }
        memset(pDns_cache_table,0,sizeof(DNS_CacheEntry_T) * cache_size);
        for(i = 0; i < cache_size; i++)
        {
            pDns_cache_table[i].flag = 0;
            pDns_cache_table[i].type = DNS_RRT_A;
            memset(&pDns_cache_table[i].ip, 0, sizeof(L_INET_AddrIp_T));
            pDns_cache_table[i].arrived_time = 0;
            pDns_cache_table[i].ttl = 0;
            pDns_cache_table[i].ttl_next = -1;
            pDns_cache_table[i].ttl_prev = -1;
            pDns_cache_table[i].hash_next = -1;
            pDns_cache_table[i].hash_prev = -1;
        }
        for(i = 0; i < DNS_CACHE_HASH_TBL_LEN; i++)
        {
            hash[i] = -1;
        }
        ttlhdr = 0;
    /*
    }
    else
    {
      if (NULL!=pDns_cache_table)
        free(pDns_cache_table);
        pDns_cache_table=NULL;
    }
    */
     if (SYSFUN_GetSem(SYS_BLD_SYS_SEMAPHORE_KEY_DNS_CACHE, &dns_cache_semaphore_id) != SYSFUN_OK)
    {
        printf("\n%s:get cachesem id fail.\n", __FUNCTION__);
        return FALSE;
    }
    return DNS_OK;
}


/* FUNCTION NAME : DNS_CACHE_Reset
 *
 * PURPOSE:
 *     This function delete the contents of the dns cache.The default
 *     configuration will be used.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success
 *
 * NOTES:
 *      This function will be called by configuration sub moudle.
 */
int DNS_CACHE_Reset(void)
{
    int i = 0;
    UI32_T orig_priority;

        DNS_MGR_GetDnsResCacheMaxTTL(&(dns_cache_config.cache_max_ttl));
        DNS_MGR_GetDnsResCacheMaxEntries(&(dns_cache_config.cache_max_entries));
        dns_cache_config.cache_good_caches = 0;
        dns_cache_config.cache_bad_caches = 0;
        cache_ent_usenum = 0;
        orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */
        for(i = 0; i < dns_cache_config.cache_max_entries; i++)
        {
            pDns_cache_table[i].flag = 0;
            pDns_cache_table[i].type = DNS_RRT_A;
            memset(&pDns_cache_table[i].ip, 0, sizeof(L_INET_AddrIp_T));
            pDns_cache_table[i].arrived_time = 0;
            pDns_cache_table[i].ttl = 0;
            pDns_cache_table[i].ttl_next = -1;
            pDns_cache_table[i].ttl_prev = -1;
            pDns_cache_table[i].hash_next = -1;
            pDns_cache_table[i].hash_prev = -1;
        }
        for(i = 0; i < DNS_CACHE_HASH_TBL_LEN; i++)
        {
            hash[i] = -1;
        }
        ttlhdr = 0;
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
        return DNS_OK;
}

/* FUNCTION NAME : DNS_CACHE_ComputeHashIndex
 *
 * PURPOSE:
 *      This funciton will compute the hash index to put the RRs in the appropriate
 *      hash link.
 *
 * INPUT:
 *      const char* -- The name whose hash index will be computed.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      >=0, hash index;
 *      DNS_ERROR
 *
 * NOTES:
 *      This function will be called by DNS_CACHE_SetRR and DNS_CACHE_GetRR.
 */
static int DNS_CACHE_ComputeHashIndex(const char* name)
{
    int i = 0;
    int hsval = 0;

    while(name[i] != '\0')
    {
        hsval += (int)name[i++];
    }

    hsval = (hsval * 47) % DNS_CACHE_HASH_TBL_LEN;

    return hsval;
}


/* FUNCTION NAME : DNS_CACHE_SetHashTab
 *
 * PURPOSE:
 *      This function will add the new entry into hash tab.
 *
 * INPUT:
 *      int -- Index in cache table.
 *      int -- Old hash index of this entry.
 *      int -- New has index of this entry.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      This function will be called by DNS_CACHE_AddEntry.
 */
static void DNS_CACHE_SetHashTab(int idx, int oldhashidx, int newhashidx)
{
    int hash_next;
    int hash_prev;

    /* delete from the old hash list */
    hash_next = pDns_cache_table[idx].hash_next;
    if(hash_next >= 0)  /* not tail */
    {
        pDns_cache_table[hash_next].hash_prev = pDns_cache_table[idx].hash_prev;
    }
    hash_prev = pDns_cache_table[idx].hash_prev;
    if(hash_prev >= 0)  /* not head */
    {
        pDns_cache_table[hash_prev].hash_next = pDns_cache_table[idx].hash_next;
    }
    else if(oldhashidx >= 0)    /* head */
    {
        hash[oldhashidx] = pDns_cache_table[idx].hash_next;
    }

    /* add into new hash list */
    if(hash[newhashidx] >= 0)   /* not a empty list */
    {
        pDns_cache_table[hash[newhashidx]].hash_prev = idx;
        pDns_cache_table[idx].hash_next = hash[newhashidx];
        pDns_cache_table[idx].hash_prev = -1;
        hash[newhashidx] = idx;
    }
    else    /* a empty list */
    {
        hash[newhashidx] = idx;
        pDns_cache_table[idx].hash_next = -1;
        pDns_cache_table[idx].hash_prev = -1;
    }

}


/* FUNCTION NAME : DNS_CACHE_AddEntry
 *
 * PURPOSE:
 *      This function will search an unused entry in cache.
 *      The  newly searhed entry will be put in the front of the cache list.
 *
 * INPUT:
 *      DNS_CacheRR_PTR_T -- This RR will be added into the cache table and hash table.
 *
 * OUTPUT:
 *      idx_p -- index of entry in cache table.
 *
 * RETURN:
 *      DNS_OK,
 *      DNS_RR_EXISTED,
 *      DNS_ERROR.
 *
 * NOTES:
 *      This function will be called by DNS_CACHE_SetRR.
 */
static int DNS_CACHE_AddEntry(DNS_CacheRR_PTR_T rrlist_p, int *idx_p)
{
    int i;
    int idx;
    int oldhashidx;
    int newhashidx = DNS_CACHE_ComputeHashIndex(rrlist_p->name);
    UI32_T ttl = rrlist_p->ttl;

    i = hash[newhashidx];

    while(i >= 0)
    {
        if( (strcmp(rrlist_p->name, pDns_cache_table[i].name) == 0) &&
            ( (L_INET_CompareInetAddr((L_INET_Addr_T *) & rrlist_p->ip,
                    (L_INET_Addr_T *) & pDns_cache_table[i].ip, 0) == 0)
                || (rrlist_p->type == DNS_RRT_CNAME) ))
        {
            if(pDns_cache_table[i].ttl > SYSFUN_GetSysTick())
            {
                *idx_p = i;
                return DNS_RR_EXISTED;
            }
        }
        i = pDns_cache_table[i].hash_next;
    }

    /* there are some empty ent */
    if(cache_ent_usenum < dns_cache_config.cache_max_entries)
    {
        idx = cache_ent_usenum++;
        oldhashidx = -1;
        /* the ttl list is empty, it's the first entry */
        if(idx == 0)    /* commented by wiseway*/
        {
            pDns_cache_table[idx].ttl_prev = -1;
            pDns_cache_table[idx].ttl_next = -1;
            DNS_CACHE_SetHashTab(idx, oldhashidx, newhashidx);
            *idx_p = idx;
            return DNS_OK;
        }
    }
    else
    {
        idx = ttlhdr;

        if(pDns_cache_table[idx].ttl > SYSFUN_GetSysTick())
          return DNS_ERROR;
        oldhashidx = DNS_CACHE_ComputeHashIndex(pDns_cache_table[idx].name);
        ttlhdr = pDns_cache_table[ttlhdr].ttl_next;
    }

    i = ttlhdr;

    /* find the first entry which ttl > ttl(argu) and it should not be unused */
    while ((pDns_cache_table[i].ttl < ttl) && (pDns_cache_table[i].ttl_next >= 0))
    {
        i = pDns_cache_table[i].ttl_next;
    }
    if(pDns_cache_table[i].ttl >= ttl)  /* insert before entry i */
    {
        /* insert before ttlhdr */
        if(i == ttlhdr)
        {
            pDns_cache_table[idx].ttl_next = i;
            pDns_cache_table[idx].ttl_prev = -1;
            pDns_cache_table[i].ttl_prev = idx;
            ttlhdr = idx;
        }
        else
        {
            pDns_cache_table[idx].ttl_next = i;
            pDns_cache_table[idx].ttl_prev = pDns_cache_table[i].ttl_prev;
            pDns_cache_table[pDns_cache_table[i].ttl_prev].ttl_next = idx;
            pDns_cache_table[i].ttl_prev = idx;
        }
    }
    else    /* it's the list tail */
    {
        pDns_cache_table[idx].ttl_next = -1;
        pDns_cache_table[idx].ttl_prev = i;
        pDns_cache_table[i].ttl_next = idx;
    }
    DNS_CACHE_SetHashTab(idx, oldhashidx, newhashidx);

    *idx_p = idx;
    return DNS_OK;
}

/* FUNCTION NAME : DNS_CACHE_DelEntry
 *
 * PURPOSE:
 *      This function will search an unused entry in cache.
 *      The  newly searhed entry will be put in the front of the cache list.
 *
 * INPUT:
 *      DNS_CacheRR_PTR_T -- This RR will be added into the cache table and hash table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      >=0, index of entry in cache table;
 *      DNS_ERROR.
 *
 * NOTES:
 *      This function will be called by DNS_CACHE_SetRR.
 */
int DNS_CACHE_DelEntry(const char* name)
{
    I8_T namebuf[256];
    int hlp;    /* hash list pointer */
    int del;
    int i, j;
    UI32_T orig_priority;

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */
    for(i = 0; i < 3; i++)
    {
        hlp = hash[DNS_CACHE_ComputeHashIndex(name)];
        strcpy((char*)namebuf, (char*)name);
        while(hlp >= 0)
        {
            if(strcmp(pDns_cache_table[hlp].name, (char*)namebuf) == 0)
            {
                del=hlp;

                if(pDns_cache_table[hlp].type == DNS_RRT_CNAME)
                {
                    memcpy(&hlp, pDns_cache_table[hlp].ip.addr, sizeof(hlp));
                    strcpy((char*)namebuf, pDns_cache_table[hlp].name);
                    pDns_cache_table[del].ttl = 0;
                    continue;
                }
                else
                {
                    hlp = pDns_cache_table[hlp].hash_next;
                    strcpy((char*)namebuf, pDns_cache_table[hlp].name);
                    pDns_cache_table[del].ttl = 0;

                }
            }/* if strcmp end */
            hlp = pDns_cache_table[hlp].hash_next;
        }   /* while end */

        j = 0;
        while(name[j] != '.' && name[j] != '\0')
        {
            j++;
        }
        if(name[j] == '\0')
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
            return DNS_OK;
        }
        name += j + 1;

    }   /* for end */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
    return DNS_OK;;

}

/* FUNCTION NAME : DNS_CACHE_SetRR
 * PURPOSE:
 *      This funciton will put received RR in the cache.
 *
 * INPUT:
 *      DNS_CacheRR_PTR_T -- This RR will be added into cache table and hash table.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_ERROR :failure,
 *      DNS_OK :success
 *
 * NOTES:
 *      This function will be called by resolver.
 */
int DNS_CACHE_SetRR(DNS_CacheRR_PTR_T rrlist_p)
{
    DNS_CacheRR_PTR_T alias_P[20] = { NULL };
    int count_a = 0;
    int idx=-1;
    int idx_a;
    int expired_time =0;
    UI32_T orig_priority;
    int ret;

    if(rrlist_p->ttl > dns_cache_config.cache_max_ttl)
    {
        rrlist_p->ttl = dns_cache_config.cache_max_ttl;
    }

    expired_time=DNS_Ttl2Ticks(rrlist_p->ttl);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */

    /* add CNAME entry */
    while(rrlist_p != NULL)
    {
        rrlist_p->ttl=expired_time;  /*added by wiseway 2002-10-17 for cache full bug*/
        if(0 == rrlist_p->ip.addrlen && DNS_RRT_CNAME != rrlist_p->type)
        {
            rrlist_p = rrlist_p->next_p;
            continue;
        }

        if(rrlist_p->type == DNS_RRT_CNAME)
        {
            alias_P[count_a] = rrlist_p;
            count_a++;
            rrlist_p = rrlist_p->next_p;
            continue;
        }

        ret = DNS_CACHE_AddEntry(rrlist_p, &idx);
        if (ret == DNS_OK || ret == DNS_RR_EXISTED)
        {
            if (DNS_OK == ret)
            {
                pDns_cache_table[idx].flag = rrlist_p->flag;
                memcpy(&pDns_cache_table[idx].ip, &rrlist_p->ip, sizeof(L_INET_AddrIp_T));
                pDns_cache_table[idx].type = rrlist_p->type;        /* DNS_RRT_A */
                pDns_cache_table[idx].ttl = expired_time;
                pDns_cache_table[idx].arrived_time = rrlist_p->arrived_time;
                strcpy(pDns_cache_table[idx].name, rrlist_p->name);
            }

            rrlist_p = rrlist_p->next_p;
        }
        else
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
            return DNS_ERROR;
        }
    }

    if (idx>-1)
    {
        /* add ALIAS entry */
        while(count_a > 0)
        {
            count_a--;
            ret = DNS_CACHE_AddEntry(alias_P[count_a], &idx_a);
            if (ret == DNS_OK || ret == DNS_RR_EXISTED)
            {
                pDns_cache_table[idx_a].flag = alias_P[count_a]->flag;
                memcpy(pDns_cache_table[idx_a].ip.addr, &idx, sizeof(idx));
                pDns_cache_table[idx_a].type = alias_P[count_a]->type;      /* DNS_RRT_CNAME */
                pDns_cache_table[idx_a].ttl = expired_time;                      /*alias_P[count_a]->ttl;*/
                pDns_cache_table[idx_a].arrived_time = alias_P[count_a]->arrived_time;
                strcpy(pDns_cache_table[idx_a].name, alias_P[count_a]->name);
            }
            else
            {
                SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
                return DNS_ERROR;
            }
        }
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);

    return DNS_OK;
}



/* FUNCTION NAME : DNS_CACHE_GetRR
 * PURPOSE:
 *      This fucntion searchs cache database to find a cache entry according to name.
 *
 * INPUT:
 *      const char* name -- domain name whose related cache entrys will be returned as a list.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_CacheRR_PTR_T --
 *      NULL -- if not found in cache
 *
 * NOTES:
 *      This function will be called by resolver.
 */
DNS_CacheRR_PTR_T DNS_CACHE_GetRR(const char* name)
{
    DNS_CacheRR_PTR_T rrlist_P = NULL;
    DNS_CacheRR_PTR_T rr_P = NULL;
    DNS_CacheRR_PTR_T rrtail_P = NULL;
    char namebuf[256]; /*maggie liu remove warning*/
    int hlp;    /* hash list pointer */
    int i, j = 0;
    int flag_a = 0;
    int bttl = 0;
    UI32_T orig_priority;
    UI32_T          cur_time=SYSFUN_GetSysTick();

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */
    for(i = 0; i < 3; i++)
    {
        hlp = hash[DNS_CACHE_ComputeHashIndex(name)];
        strcpy(namebuf, name);
        while(hlp >= 0)
        {
            if(strcmp(pDns_cache_table[hlp].name, namebuf) == 0)
            {
                if(pDns_cache_table[hlp].type == DNS_RRT_CNAME)
                {
                    flag_a = 1;
                    memcpy(&hlp, pDns_cache_table[hlp].ip.addr, sizeof(hlp));
                    strcpy(namebuf, pDns_cache_table[hlp].name);
                    continue;
                }
                else
                {
                    if (pDns_cache_table[hlp].ttl< cur_time)
                    {
                        break;
                    }
                    rr_P = NULL;
                    rr_P = (DNS_CacheRR_T *)L_MM_Malloc(sizeof(DNS_CacheRR_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_CACHE_GETRR));
                    if(rr_P == NULL)
                    {
                        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
                        return rrlist_P;
                    }

                    memcpy(&rr_P->ip, &pDns_cache_table[hlp].ip, sizeof(L_INET_AddrIp_T));
                    strcpy(rr_P->name, pDns_cache_table[hlp].name);
                    rr_P->flag = pDns_cache_table[hlp].flag;
                    rr_P->arrived_time = pDns_cache_table[hlp].arrived_time;
                    rr_P->ttl = DNS_Tick2Ttl(pDns_cache_table[hlp].ttl);
                    if(0 == rr_P->ttl)
                        rr_P->ttl = 1;
                    if(0 == bttl)
                        bttl = rr_P->ttl;

                    rr_P->next_p = NULL;
                    if(i == 0)
                    {
                        rr_P->type = pDns_cache_table[hlp].type;
                    }
                    else
                    {
                        rr_P->type = DNS_RRT_NS;
                    }

                    if(rrlist_P == NULL)
                    {
                        rrlist_P = rr_P;
                    }
                    else
                    {
                        rrtail_P->next_p = rr_P;  /* pgr0060, rrtail_P has been assigned below */
                    }
                    rrtail_P = rr_P;  /* pgr0060, rrtail_P is assigned here */

                }
            }   /* if strcmp end */
            hlp = pDns_cache_table[hlp].hash_next;
        }   /* while end */

        if(rrlist_P != NULL)
        {
            if(i == 0 && flag_a == 1)
            {
                rr_P = NULL;
                rr_P = (DNS_CacheRR_T *)L_MM_Malloc(sizeof(DNS_CacheRR_T), L_MM_USER_ID2(SYS_MODULE_DNS, DNS_TYPE_TRACE_ID_DNS_CACHE_GETRR));
                if(rr_P == NULL)
                {
                    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
                    return rrlist_P;
                }
                strcpy(rr_P->name, name);
                rr_P->type = DNS_RRT_CNAME;
                memcpy(&rr_P->ip, &rrlist_P->ip, sizeof(L_INET_AddrIp_T));
                rr_P->flag = DNS_CF_NOAUTH;
                rr_P->ttl = bttl;
                rr_P->next_p = rrlist_P;
                rrlist_P = rr_P;
            }
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
            return rrlist_P;
        }
        j = 0;
        while(name[j] != '.' && name[j] != '\0')
        {
            j++;
        }
        if(name[j] == '\0')
        {
            SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
            return NULL;
        }
        name += j + 1;

    }   /* for end */

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
    return NULL;

}

/* FUNCTION NAME : DNS_CACHE_ShowDatabase
 * PURPOSE:
 *      This funciton is used for showing  cache database and status.
 *      Every field except link should be displayed, and the index of this cache
 *      entry should also be displayed.
 *
 * INPUT:
 *      none.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      none.
 *
 * NOTES:
 *      This routine is called by dns configuration  sub module..
 */
void DNS_CACHE_ShowDatabase(void)
{
    int i;
    int j;
    int count = 0;
    UI32_T ttl;

    I8_T type[6][6]={"0", "CNAME", "2", "3", "4", "ALIAS"};

    char ip[L_INET_MAX_IPADDR_STR_LEN+1];

    printf("\ncache status:\n");

    printf("\nNO\tFLAG\tTYPE\tIP\t\tTTL\tDOMAIN\n");
    for(i = 0; i < DNS_CACHE_HASH_TBL_LEN; i++)
    {
        if(hash[i] >= 0)
        {
            j = hash[i];

            while(j >= 0)
            {
                ttl = DNS_Tick2Ttl(pDns_cache_table[j].ttl);
                if( ttl ==0 )
                {
                    j = pDns_cache_table[j].hash_next;
                    continue;
                }

                printf("%-4d\t%-8d%-8s", j,pDns_cache_table[j].flag,type[pDns_cache_table[j].type]);

                if(DNS_RRT_A == pDns_cache_table[j].type)
/*                  printf("%-15s ", inet_ntoa(htonl(pDns_cache_table[j].ip)));*/
                {
                    /* ES3550MO-PoE-FLF-AA-00053
                     * use CmnLib APIs to replace socket ntohs(), htons()... function
                     */
                    if (L_INET_RETURN_SUCCESS == L_INET_InaddrToString((L_INET_Addr_T *)&pDns_cache_table[j].ip,
                                                                       ip,
                                                                       sizeof(ip)))
                    {
                        printf("%-15s ", ip);
                    }
                }
                else
                {
                    UI32_T idx=0;
                    memcpy(&idx, pDns_cache_table[j].ip.addr, 4);
                    printf("POINTER TO:%-4ld\t", (long)idx);
                }


                printf("%-4lu\t",(unsigned long)ttl);
                printf("%-s\n", pDns_cache_table[j].name);

                j = pDns_cache_table[j].hash_next;
                count++;
            }
        }   /* if end */
    }   /* for end */

    printf("\t%d entries are valid.\n", count);
}



/* FUNCTION NAME : DNS_CACHE_GetNextCacheEntey
 * PURPOSE:
 *      This function get next entry from dns cache.
 *
 *
 * INPUT:
 *      UI32_T *index   --  current index of cache entry.
 *
 * OUTPUT:
 *      UI32_T *index                  --  next index of cache entry.
 *      DNS_CacheEntry_t cache_entry   --  next cache entry.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      The initial index value is -1.
 */
BOOL_T DNS_CACHE_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int cache_size;
    I32_T   i;
    UI32_T  ttl;
    UI32_T orig_priority;

    /* BODY */

    DNS_MGR_GetDnsResCacheMaxEntries(&cache_size);
    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */
    for ( i=(*index+1) ; i<cache_size ; i++ )
    {
        ttl = DNS_Tick2Ttl(pDns_cache_table[i].ttl);
        if( ttl ==0 )
        {
            continue;
        }
        *index = i;
        cache_entry->flag = pDns_cache_table[i].flag;
        strcpy(cache_entry->name, pDns_cache_table[i].name);
        memcpy(&cache_entry->ip, &pDns_cache_table[i].ip, sizeof(L_INET_AddrIp_T));
        cache_entry->type = pDns_cache_table[i].type;
        cache_entry->arrived_time = pDns_cache_table[i].arrived_time;
        cache_entry->ttl = ttl;

        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
        return TRUE;
    }
    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
    return FALSE;
}

/* FUNCTION NAME : DNS_CACHE_FreeRR
 * PURPOSE:
 *      This function will free the RR list created by DNS_CACHE_RR.
 *
 * INPUT:
 *      DNS_CacheRR_T* free_cache_p   --  The RR list to be freed.
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *      Remember to free the RR list after using DNS_CACHE_RR.
 */

void DNS_CACHE_FreeRR(DNS_CacheRR_T* cache_p)
{
    DNS_CacheRR_T* free_cache_p = cache_p;

    if (NULL == cache_p)
        return;

    while(cache_p->next_p != NULL)
    {
        free_cache_p=cache_p;
        cache_p=cache_p->next_p;
        L_MM_Free(free_cache_p);
    }

    L_MM_Free(cache_p);
    cache_p=NULL;
}

/* FUNCTION NAME : DNS_CACHE_GetCacheEntryForSNMP
 * PURPOSE:
 *      This function get an entry from dns cache by a specified index .
 *
 *
 * INPUT:
 *      index         --  current index of cache entry. start from 0.
 *
 * OUTPUT:
 *      index         --  specified index of cache entry.
 *      cache_entry   --  specified cache entry.
 *
 * RETURN:
 *      TRUE    -- success.
 *      FALSE   -- failure.
 *
 * NOTES:
 *      This function will be called by snmp module, initial index=0.
 */
BOOL_T DNS_CACHE_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry)
{
    /* LOCAL CONSTANT DECLARATIONS
     */

    /* LOCAL VARIABLES DEFINITION
     */
    int cache_size;
    UI32_T  ttl;
    UI32_T orig_priority;

    /* BODY */
    DNS_MGR_GetDnsResCacheMaxEntries(&cache_size);
    if (index  < 0 || index > cache_size)
    {
        return FALSE;
    }

    orig_priority = SYSFUN_OM_ENTER_CRITICAL_SECTION(dns_cache_semaphore_id);  /* pgr0695, return value of statement block in macro */

    ttl = DNS_Tick2Ttl(pDns_cache_table[index].ttl);
    if( ttl ==0 )
    {
        SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);
        return FALSE;
    }

    cache_entry->flag = pDns_cache_table[index].flag;
    strcpy(cache_entry->name, pDns_cache_table[index].name);
    memcpy(&cache_entry->ip, &pDns_cache_table[index].ip, sizeof(L_INET_AddrIp_T));
    cache_entry->type = pDns_cache_table[index].type;
    cache_entry->arrived_time = pDns_cache_table[index].arrived_time;
    cache_entry->ttl = ttl;

    SYSFUN_OM_LEAVE_CRITICAL_SECTION(dns_cache_semaphore_id, orig_priority);

    return TRUE;
}

#endif /* #if (SYS_CPNT_DNS == TRUE) */

