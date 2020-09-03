#include "cgi.h"
#include "cgi_cache.h"
#include "cgi_file.h"
#include "cgi_debug.h"
#include "mm.h"

#define IS_THIS_DEBUG_ON()  (CGI_DEBUG_GetFlag() & CGI_DEBUG_CACHE)

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_HashValue
 *------------------------------------------------------------------------------
 * Function : Compute hash value
 * Input    : key   -   hash key
 *            hash  -   hash
 * Output   : None
 * Return   : hash value
 * Note     : None
 *------------------------------------------------------------------------------
 */
static size_t
CGI_CACHE_HashValue(
    datum_t *key,
    hash_t *hash)
{
    CGI_CACHE_Key_T *k = (CGI_CACHE_Key_T *) key->data;
    char *p;
    size_t hash_val = 0;

    ASSERT(key->size == sizeof(CGI_CACHE_Key_T));
    ASSERT(CGI_CACHE_KEY == k->type);

    for (p = k->file_path->co.ptr; *p; p++, hash_val = (hash_val * 32 + *p) % hash->size);

    return hash_val;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_HashKeyCmp
 *------------------------------------------------------------------------------
 * Function : Compare two keys in hash table
 * Input    : hash  -   hash
 *            key1  -   key 1
 *            key2  -   key 2
 * Output   : None
 * Return   : The same as strcmp
 * Note     : None
 *------------------------------------------------------------------------------
 */
static int
CGI_CACHE_HashKeyCmp(
    hash_t *hash,
    datum_t *key1,
    datum_t *key2)
{
    CGI_CACHE_Key_T *k1 = (CGI_CACHE_Key_T *) key1->data;
    CGI_CACHE_Key_T *k2 = (CGI_CACHE_Key_T *) key2->data;

    ASSERT(key1->size == sizeof(CGI_CACHE_Key_T));
    ASSERT(key2->size == sizeof(CGI_CACHE_Key_T));

    ASSERT(CGI_CACHE_KEY == k1->type);
    ASSERT(CGI_CACHE_KEY == k2->type);

    return strcmp(k1->file_path->co.ptr, k2->file_path->co.ptr);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_HashDataFree
 *------------------------------------------------------------------------------
 * Function : Free data in cache
 * Input    : data  -   cache data
 *            size  -   data size
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
static void
CGI_CACHE_HashDataFree(
    void *data,
    unsigned int size)
{
    ASSERT(CGI_CACHE_KEY == ((CGI_CACHE_Key_T *) data)->type ||
        CGI_CACHE_DATA == ((CGI_CACHE_Key_T *) data)->type);

    if (CGI_CACHE_KEY == ((CGI_CACHE_Key_T *) data)->type)
    {
        CGI_CACHE_Key_T *key = (CGI_CACHE_Key_T *) data;

        CGI_CACHE_MfreeFilePath(key->file_path);
    }
    else if (CGI_CACHE_DATA == ((CGI_CACHE_Key_T *) data)->type)
    {
        CGI_CACHE_Entry_T *ent = (CGI_CACHE_Entry_T *) data;

        CGI_CACHE_MfreeFileData(ent->file_info);
    }
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_LLKeyCmp
 *------------------------------------------------------------------------------
 * Function : Compare two keys in linked list
 * Input    : data  -   cache data
 *            size  -   data size
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
static I32_T
CGI_CACHE_LLKeyCmp(
    void *inlist_element,
    void *input_element)
{
    CGI_CACHE_Key_T *inlist = (CGI_CACHE_Key_T *)inlist_element;
    CGI_CACHE_Key_T *input  = (CGI_CACHE_Key_T *)input_element;

    return strcmp(inlist->file_path->co.ptr, input->file_path->co.ptr);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_LLDataDel
 *------------------------------------------------------------------------------
 * Function : Free data in linked list
 * Input    : element  -   linked list data
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
static void
CGI_CACHE_LLDataDel(
    void *element)
{
    CGI_CACHE_Key_T *e = (CGI_CACHE_Key_T *)element;

    CGI_CACHE_MfreeFilePath(e->file_path);

    MM_free(e, SYSFUN_TaskIdSelf());
    free(e);
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_Evict
 *------------------------------------------------------------------------------
 * Function : Purge the least-recently-used element in the cache
 * Input    : cache -   cache
 *            ent   -   new cache entry
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
static BOOL_T
CGI_CACHE_Evict(
    CGI_CACHE_T *cache,
    const CGI_CACHE_Entry_T *ent)
{
    ASSERT(NULL != cache);
    ASSERT(NULL != ent->file_info);
    ASSERT(ent->file_info->co.size <= cache->capacity);

    while (cache->capacity < cache->size + ent->file_info->co.size)
    {
        CGI_CACHE_Key_T *p = (CGI_CACHE_Key_T *) L_list_pop_back(cache->key_tracker);
        datum_t hkey;
        datum_t *hval;

        ASSERT(NULL != p);

        CGI_DEBUG_LOG("Evict page %s", p->file_path->co.ptr);

        hkey.data = p;
        hkey.size = sizeof(*p);

        hval = hash_delete(&hkey, cache->key_to_value);

        if (NULL != hval)
        {
            CGI_CACHE_Entry_T *e = (CGI_CACHE_Entry_T *) hval->data;

            ASSERT(hval->size == sizeof(*e));
            ASSERT(e->file_info->co.size <= cache->size);

            cache->size -= e->file_info->co.size;

            CGI_CACHE_MfreeFileData(e->file_info);
            datum_free(hval);
        }

        CGI_CACHE_MfreeFilePath(p->file_path);

        MM_free(p, SYSFUN_TaskIdSelf());
        free(p);
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_Insert
 *------------------------------------------------------------------------------
 * Function : Record a fresh key-value pair in the cache
 * Input    : cache -   cache
 *            key   -   key for new cache entry
 *            ent   -   new cache entry
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
static BOOL_T
CGI_CACHE_Insert(
    CGI_CACHE_T *cache,
    CGI_CACHE_Key_T *key,
    CGI_CACHE_Entry_T *ent)
{
    datum_t hkey;
    datum_t hval;
    datum_t *hret;

    ASSERT(NULL != cache);
    ASSERT(NULL != key);
    ASSERT(NULL != ent);

    hkey.data = key;
    hkey.size = sizeof(*key);

    hval.data = ent;
    hval.size = sizeof(*ent);

    /* Method is only called on cache misses
     */
    ASSERT(NULL == hash_lookup(&hkey, cache->key_to_value));
    ASSERT(NULL == L_listnode_lookup(cache->key_tracker, key));

    hret = hash_insert(&hkey, &hval, cache->key_to_value);
    if (NULL == hret)
    {
        return FALSE;
    }

    {
        CGI_CACHE_Key_T *lldata;
        struct L_listnode *llnode;

        lldata = (CGI_CACHE_Key_T *) malloc(sizeof(*lldata));
        if (NULL == lldata)
        {
            hash_delete(&hkey, cache->key_to_value);
            return FALSE;
        }

        MM_alloc(lldata, sizeof(*lldata), SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

        lldata->type = key->type;

        lldata->file_path = key->file_path;
        ++ lldata->file_path->co.ref_count;

        llnode = L_listnode_add_before(cache->key_tracker, NULL, lldata);
        if (NULL == llnode)
        {
            hash_delete(&hkey, cache->key_to_value);
            return FALSE;
        }
    }

    cache->size += ent->file_info->co.size;
    return TRUE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_IsDirty
 *------------------------------------------------------------------------------
 * Function : Check entry is dirty or not
 * Input    : cache         -   cache
 *            key           -   key
 *            val           -   entry value
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
BOOL_T CGI_CACHE_IsDirty(
    CGI_CACHE_T *cache,
    const datum_t *key,
    const datum_t *val)
{
    const CGI_CACHE_Key_T *ckey;
    const CGI_CACHE_Entry_T *ent;
    CGI_FILE_STATS_T file_stats;

    ASSERT(NULL != cache);
    ASSERT(NULL != key);
    ASSERT(NULL != key->data);
    ASSERT(NULL != val);

    ckey = (CGI_CACHE_Key_T *)key->data;
    ent = (CGI_CACHE_Entry_T *)val->data;

    if (TRUE != cache->get_file_stats_fn(ckey->file_path->co.ptr, &file_stats))
    {
        return TRUE;
    }

    if (ent->file_info->stats.mtime != file_stats.mtime)
    {
        return TRUE;
    }

    return FALSE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_Create
 *------------------------------------------------------------------------------
 * Function : Create LRU cache
 * Input    : cache             -   cache
 *            read_file_fn      -   a function to read file from file system
 *            get_file_stats_fn -   a function to get file stats from file system
 *            capacity          -   capacity in bytes
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
BOOL_T
CGI_CACHE_Create(
    CGI_CACHE_T *cache,
    CGI_CACHE_ReadFile_T read_file_fn,
    CGI_CACHE_GetFileStats_T get_file_stats_fn,
    UI32_T capacity)
{
    ASSERT(NULL != cache);
    ASSERT(0 != capacity);

    memset(cache, 0, sizeof(*cache));

    cache->key_tracker = L_list_new();
    if (NULL == cache->key_tracker)
    {
        return FALSE;
    }

    cache->key_tracker->cmp = CGI_CACHE_LLKeyCmp;
    cache->key_tracker->del = CGI_CACHE_LLDataDel;

    cache->key_to_value = hash_create(10, (hashval_t) CGI_CACHE_HashValue, (keycmp_t) CGI_CACHE_HashKeyCmp,
                                                        CGI_CACHE_HashDataFree);
    if (NULL == cache->key_to_value)
    {
        L_list_free(cache->key_tracker);
        cache->key_tracker = NULL;
        return FALSE;
    }

    cache->read_file_fn = read_file_fn;
    cache->get_file_stats_fn = get_file_stats_fn;
    cache->capacity = capacity;
    cache->size = 0;
    return TRUE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_Destroy
 *------------------------------------------------------------------------------
 * Function : Destroy cache
 * Input    : cache         -   cache
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
void
CGI_CACHE_Destroy(
    CGI_CACHE_T *cache)
{
    ASSERT(NULL != cache);

    L_list_delete_all_node(cache->key_tracker);
    L_list_free(cache->key_tracker);
    cache->key_tracker = NULL;

    hash_destroy(cache->key_to_value);
    cache->key_to_value = NULL;

    memset(cache, 0, sizeof(*cache));
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_Lookup
 *------------------------------------------------------------------------------
 * Function : Lookup a file
 * Input    : cache         -   cache
 *            path          -   file path
 *            ent           -   cache entry
 * Output   : None
 * Return   : TRUE/FALSE
 * Note     : None
 *------------------------------------------------------------------------------
 */
BOOL_T
CGI_CACHE_Lookup(
    CGI_CACHE_T *cache,
    const char *path,
    CGI_CACHE_Entry_T *ent)
{
    datum_t key;
    datum_t *val;

    CGI_CACHE_Key_T ckey;
    CGI_FILE_PATH_T _fp;

    ASSERT(NULL != cache);
    ASSERT(NULL != path);
    ASSERT(NULL != ent);

    memset(ent, 0, sizeof(*ent));
    ent->type = CGI_CACHE_DATA;

    ckey.type = CGI_CACHE_KEY;

    ckey.file_path = &_fp;
    ckey.file_path->co.ptr = (char *) path;
    ckey.file_path->co.ref_count = 0x9999;

    key.data = &ckey;
    key.size = sizeof(ckey);

    /* Attempt to find existing record
     */
    val = hash_lookup(&key, cache->key_to_value);

    if (NULL != val)
    {
        if (CGI_CACHE_IsDirty(cache, &key, val) == TRUE)
        {
            // CGI_CACHE_Remove
            datum_t *old_val = hash_delete(&key, cache->key_to_value);
            ASSERT(old_val != NULL);
            ASSERT(CGI_CACHE_DATA == ((CGI_CACHE_Key_T *) old_val->data)->type);

            ASSERT(((CGI_CACHE_Entry_T *) old_val->data)->file_info != NULL);

            CGI_CACHE_MfreeFileData(((CGI_CACHE_Entry_T *) old_val->data)->file_info);

            datum_free(old_val);
        }
        else
        {
            CGI_DEBUG_LOG("Cache hit");

            ++ cache->hit;

            ASSERT(val->size == sizeof(*ent));

            memcpy(ent, val->data, sizeof(*ent));
            ++ ent->file_info->co.ref_count;

            datum_free(val);
            return TRUE;
        }
    }

    CGI_DEBUG_LOG("Cache miss");
    ++ cache->miss;

    /* load file
     */

    ent->file_info = cache->read_file_fn(path);
    if (NULL == ent->file_info)
    {
        CGI_DEBUG_LOG("page %s not found", path);
        return FALSE;
    }

    CGI_DEBUG_LOG("Load page %s from disk", path);
    ent->file_info->co.ref_count = 1;

    /* Store content in cache
     */
    {
        // TODO: add cache.ignore = [".json$", "..."]
        const char *ext = CGI_FILE_GetFileExt(path);
        if (ext && strcmp(ext, ".json") == 0) {
            return TRUE; // !!
        }
    }

    if (ent->file_info->co.size <= cache->capacity)
    {
        CGI_CACHE_Evict(cache, ent);

        ckey.file_path = CGI_CACHE_MallocateFilePath(path);

        if (NULL != ckey.file_path)
        {
            if (TRUE == CGI_CACHE_Insert(cache, &ckey, ent))
            {
                ++ ent->file_info->co.ref_count;
            }
        }
    }

    return TRUE;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_MallocateFilePath
 *------------------------------------------------------------------------------
 * Function : Allocates memory for file path
 * Input    : path  - file path
 * Output   : None
 * Return   : A object for file path. Use CGI_CACHE_MfreeFilePath to free the
 *            memory.
 * Note     : None
 *------------------------------------------------------------------------------
 */
CGI_FILE_PATH_T *
CGI_CACHE_MallocateFilePath(
    const char *path)
{
    CGI_FILE_PATH_T *p = (CGI_FILE_PATH_T *) malloc(sizeof(*p));

    if (NULL == p)
    {
        return FALSE;
    }

    p->co.ptr = strdup(path);
    if (NULL == p->co.ptr)
    {
        free(p);
        return FALSE;
    }

    MM_alloc(p->co.ptr, strlen(p->co.ptr) + 1, SYSFUN_TaskIdSelf(), __FILE__, __LINE__, __builtin_return_address( 0 ));

    p->co.size = strlen(p->co.ptr) + 1;
    p->co.ref_count = 1;
    return p;
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_MfreeFilePath
 *------------------------------------------------------------------------------
 * Function : Free memory for file path object
 * Input    : file_path - file path object
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
void
CGI_CACHE_MfreeFilePath(
    CGI_FILE_PATH_T *file_path)
{
    ASSERT(NULL != file_path);

    if (0 == CGI_CACHE_MfreeObject(&file_path->co))
    {
        MM_free(file_path, SYSFUN_TaskIdSelf());
        free(file_path);
    }
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_MfreeFileData
 *------------------------------------------------------------------------------
 * Function : Free memory for file data object
 * Input    : file_info - file data object
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
void
CGI_CACHE_MfreeFileData(
     CGI_FILE_INFO_T *file_info)
{
    ASSERT(NULL != file_info);

    if (0 == CGI_CACHE_MfreeObject(&file_info->co))
    {
        MM_free(file_info, SYSFUN_TaskIdSelf());
        free(file_info);
    }
}

/*------------------------------------------------------------------------------
 * Routine Name : CGI_CACHE_MfreeObject
 *------------------------------------------------------------------------------
 * Function : Free a CGI_OBJECT_T object
 * Input    : obj   - CGI_OBJECT_T object
 * Output   : None
 * Return   : None
 * Note     : None
 *------------------------------------------------------------------------------
 */
int
CGI_CACHE_MfreeObject(
    CGI_OBJECT_T *obj)
{
    ASSERT(0 < obj->ref_count && NULL != obj->ptr && 0 != obj->size);

    if (0 < obj->ref_count)
    {
        -- obj->ref_count;

        if (0 == obj->ref_count)
        {
            MM_free(obj->ptr, SYSFUN_TaskIdSelf());
            free(obj->ptr);
        }
    }

    return obj->ref_count;
}

