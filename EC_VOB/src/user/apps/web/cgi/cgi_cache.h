#ifndef CGI_CACHE__H
#define CGI_CACHE__H 1

#include "cgi.h"
#include "hash.h"
#include "l_linklist.h"
#include "cgi.h"

typedef enum
{
    CGI_CACHE_KEY          = 1,
    CGI_CACHE_DATA
} CGI_CACHE_TYPE;

typedef struct
{
    CGI_CACHE_TYPE          type;

    /* union
    { */
    CGI_FILE_PATH_T     *file_path;
    /* CGI_FILE_INFO_T     *file_info;
    }; */
} CGI_CACHE_Key_T;

typedef struct
{
    CGI_CACHE_TYPE          type;
    CGI_FILE_INFO_T         *file_info;
} CGI_CACHE_Entry_T;

typedef CGI_FILE_INFO_T * (*CGI_CACHE_ReadFile_T) (const char *path);
typedef BOOL_T (*CGI_CACHE_GetFileStats_T) (const char *path, CGI_FILE_STATS_T *stats);

typedef struct CGI_CACHE_INFO_S
{
    UI32_T                  access;
    UI32_T                  hit;
    UI32_T                  miss;

    UI32_T                  capacity_in_bytes;
    UI32_T                  size_in_bytes;
} CGI_CACHE_INFO_T;

typedef struct
{
    struct L_list           *key_tracker;   /* LRU list. MRU --> LRU */
    hash_t                  *key_to_value;

    CGI_CACHE_ReadFile_T    read_file_fn;
    CGI_CACHE_GetFileStats_T get_file_stats_fn;

    UI32_T                  hit;
    UI32_T                  miss;

    UI32_T                  capacity;
    UI32_T                  size;
} CGI_CACHE_T;

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
    UI32_T capacity
);

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
    CGI_CACHE_T *cache
);

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
    CGI_CACHE_Entry_T *ent
);

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
    const char *path
);

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
    CGI_FILE_PATH_T *file_path
);

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
     CGI_FILE_INFO_T *file_info
);

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
    CGI_OBJECT_T *obj
);

#endif /* CGI_CACHE__H */
