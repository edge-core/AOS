/* MODULE NAME: dns_cache.h
 * PURPOSE:
 *   Initialize the cache and provide some functions for the resolver cache.
 *
 * NOTES:
 *
 * History:
 *       Date          -- Modifier     ,  Reason
 *       2002-09-06    -- Simon Zhou  , created.
 *       2002-10-23    -- wiseway       modified for convention
 * Copyright(C)      Accton Corporation, 2002
 */

#ifndef DNS_CACHE_H
#define DNS_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif  /* __cpluscplus */


/* the flag values for RR sets in the Cache */

#define DNS_CF_NOPURGE      1       /* Do not purge this record */
#define DNS_CF_LOCAL        2       /* Local zone entry */
#define DNS_CF_NOAUTH       4       /* Non-authoritative record */
#define DNS_CF_NOCACHE      8       /* Only hold for the Cache latency time period, then purge.
                                       not really written to Cache records, but used by add_cent_rr */
#define DNS_CF_ADDITIONAL   16      /* This was fetched as an additional or "off-topic" record. */
#define DNS_CF_NEGATIVE     32      /* this one is for per-RRset negative Cacheing*/

#define DNS_CF_NOINHERIT (DNS_CF_LOCAL | DNS_CF_NOAUTH | DNS_CF_ADDITIONAL) /* not to be inherited on requery */


/* the flag values for whole domains in the Cache */

#define DNS_DF_NEGATIVE    1                    /* this one is for whole-domain negative Cacheing (created on NXDOMAIN)*/
#define DNS_DF_LOCAL       2                    /* local record (in conj. with DF_NEGATIVE) */

#define DNS_DFF_NOINHERIT (DNS_DF_NEGATIVE)     /* not to be inherited on requery */

/* This is the time in secs any record remains at least in the Cache before it is purged. */
/* (exception is that the Cache is full) */

#define DNS_CACHE_LAT               120
#define DNS_CACHE_HASH_TBL_LEN      256         /* the number of hash links */
#define DNS_CACHE_MAX_NAME_LENGTH   64          /* we only cache the domain name with length no longer than 64 bytes */

/* the following cache entry belongs to two links one is sorted by ttl ,another is hash link*/
typedef struct DNS_CacheEntry_S
{
    short   flag;                               /* flag of this Cache node */
    char    name[DNS_CACHE_MAX_NAME_LENGTH];/* domain name */
    L_INET_AddrIp_T ip;                                 /* IP address */
    UI16_T  type;
    UI32_T  arrived_time;                       /* time when RR was received */
    UI32_T  ttl;                                /* This is the initial TTL value at which RR is originally received */

    short   hash_next;                          /* a int type pointer to the next element in the same hash link */
    short   hash_prev;                          /* a int type pointer to the previous element in the same hash link */
    short   ttl_next;                           /* a int type pointer to the next element in the link  sorted by ttl */
    short   ttl_prev;                       /* a int type pointer to the previous element in the link  sorted by ttl */
} DNS_CacheEntry_T, *DNS_CacheEntryPtr_T;

typedef struct DNS_CacheRR_S
{
    int     flag;                               /* flag of this cache node */
    char    name[DNS_CACHE_MAX_NAME_LENGTH]; /* domain name */
    L_INET_AddrIp_T ip;                                 /* IP address if type is ALIAS or pointer to a cache node if rtype is CNAME */
    UI32_T  arrived_time;                       /*  */
    UI32_T  ttl;
    UI16_T  type;                               /* rr type */

    struct DNS_CacheRR_S* next_p;
} DNS_CacheRR_T, *DNS_CacheRR_PTR_T;


#if 0
SEM_ID gdns_cache_sem;                          /* semaphore used for access cache exclusively */
#endif


/* FUNCTION NAME : DNS_CACHE_Init
 *
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
 *      This function will be called by DNS_DNS_Init..
 */
int DNS_CACHE_Init(void);


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
int DNS_CACHE_Reset(void);


/* FUNCTION NAME : DNS_CACHE_SetRR
 *
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
int DNS_CACHE_SetRR(DNS_CacheRR_PTR_T rrlist_p);


/* FUNCTION NAME : DNS_CACHE_GetRR
 * PURPOSE:
 *      This fucntion searchs cache database to find a cache entry according to name.
 *
 * INPUT:
 *      const char* -- domain name whose related cache entrys will be returned as a list.
 *
 * OUTPUT:
 *      none.
 *
 * RETURN:
 *      DNS_CacheRR_PTR_T --
 *
 * NOTES:
 *      This function will be called by resolver.
 */
DNS_CacheRR_PTR_T DNS_CACHE_GetRR(const char* name);


/* FUNCTION NAME : DNS_CACHE_ShowDatabase
 * PURPOSE:
 *      This funciton is used for showing  cache database and status.
 *      Every field except link should be displayed, and the index of this cache
 *      entry should also be displayed.
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
void DNS_CACHE_ShowDatabase(void);


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
BOOL_T DNS_CACHE_GetNextCacheEntry(I32_T *index, DNS_CacheRecord_T *cache_entry);

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

void DNS_CACHE_FreeRR(DNS_CacheRR_T* cache_p);

/* FUNCTION NAME : DNS_CACHE_GetCacheEntryForSNMP
 * PURPOSE:
 *      This function get an entry from dns cache by a specified index .
 *
 *
 * INPUT:
 *      index         --  current index of cache entry.
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
BOOL_T DNS_CACHE_GetCacheEntryForSNMP(I32_T index, DNS_CacheRecord_T *cache_entry);

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
int DNS_CACHE_DelEntry(const char* name);

#ifdef  __cpluscplus

}
#endif  /* __cpluscplus */

#endif  /* #ifndef DNS_CACHE_H */
